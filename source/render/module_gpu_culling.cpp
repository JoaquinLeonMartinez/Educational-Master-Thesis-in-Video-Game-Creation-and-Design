#include "mcv_platform.h"
#include "engine.h"
#include "module_gpu_culling.h"
#include "engine.h"
#include "modules/module_camera_mixer.h"
#include "render/compute/gpu_buffer.h"
#include "components/common/comp_camera.h"
#include "components/common/comp_light_dir.h"
#include "components/common/comp_transform.h"
#include "components/common/comp_render.h"
#include "components/common/comp_lod.h"
#include "components/common/comp_aabb.h"
#include "components/common/comp_dynamic_instance.h"
#include "components/common/comp_group.h"
#include "components/common/comp_name.h"
#include "components/objects/triggers/comp_trigger.h"
#include "components/objects/comp_increase_power.h"
#include "components/objects/comp_wind_trap.h"
#include "components/objects/comp_destroyable_wall.h"
#include "components/actions/comp_checkpoint_register.h"
#include "components/ai/bt/bt_golem.h"
#include "render/textures/material.h"
#include "utils/utils.h"
#include "entity/entity.h"
#include "entity/entity_parser.h"
#include "render/compute/compute_shader.h"
#include "utils/json_resource.h"
#include "components/ai/others/self_destroy.h"
#include "components/common/comp_tags.h"

AABB getRotatedBy(AABB src, const MAT44 &model);

struct TSampleDataGenerator {
  CModuleGPUCulling* mod = nullptr;

  VEC3         pmin;
  VEC3         pmax;
  float        scale = 1.0f;
  uint32_t     num_instances = 0;

  VHandles     prefabs; //for trees
  std::map<const std::string, CHandle> scene_prefabs; //for scenes

  bool         prefab_handles_loaded = false;

  void createProductPrefabs(const json& j) {
	  
    float radius = j.value("radius", 30.f);
    pmin = VEC3(-radius, 0.f, -radius);
    pmax = VEC3(radius, 1.0f, radius);
    num_instances = j.value("num_instances", num_instances);
    
    #ifndef NDEBUG
        return;
    #endif

    std::vector< std::string > prefab_names = j["prefabs"].get< std::vector< std::string > >();
    for (auto& prefab_name : prefab_names) {
      TEntityParseContext ctx;
      bool is_ok = parseScene(prefab_name, ctx);
      assert(is_ok);
      prefabs.push_back( ctx.entities_loaded[0] );
    }
  }

  void deleteProductPrefabs() {
    for (auto& pref : prefabs) {
      pref.destroy();
    }
    prefabs.clear();
  }

  bool isInstantiable(const json j_entity) {
    //is instantiable if
    return j_entity.count("render") > 0 && j_entity.count("abs_aabb") > 0  //has render and aabb
      && j_entity.count("comp_increase_power") == 0 && j_entity.count("ai_platform_mobile") == 0 && j_entity.count("comp_door") == 0  //is not a platform and not a power up and not a door 
      && j_entity.count("morph_animation") == 0 && j_entity.count("comp_destroyable_wall") == 0 && j_entity.count("comp_interruptor") == 0   //is not a morph and not a destroyable wall
      && j_entity.count("bt_sushi") == 0 && j_entity.count("bt_cupcake") == 0 && j_entity.count("bt_golem") == 0 && j_entity.count("bt_ranged_sushi") == 0 //is not an enemy
      && j_entity.count("comp_madness_puddle") == 0 && j_entity.count("comp_wind_trap") == 0 && j_entity.count("local_aabb") == 0; //is not a madness puddle or a wind trap or has a local aabb
  }

  bool isInstantiable(CHandle ch) {
    //is instantiable if
    CEntity* entity = ch;
    TCompTrigger* c_tr = entity->get<TCompTrigger>();
    TCompIncreasePower* c_ip = entity->get<TCompIncreasePower>();
    TCompWindTrap* c_wt = entity->get<TCompWindTrap>();
    CBTGolem* c_g = entity->get<CBTGolem>();
    TCompDestroyableWall* c_dw = entity->get<TCompDestroyableWall>();
    TCompCharacterController* c_cc = entity->get<TCompCharacterController>();
    TCompCheckpointRegister* c_cr = entity->get<TCompCheckpointRegister>();

    return !c_tr && !c_ip && !c_wt && !c_g && !c_dw && !c_cc && !c_cr;
  }

  void createProducts(const std::string& filename, TEntityParseContext& ctx) {
    ctx.filename = filename;

    const json& j_scene = Resources.get(filename)->as<CJson>()->getJson();
    assert(j_scene.is_array());

    // For each item in the array...
    for (int i = 0; i < j_scene.size(); ++i) {
      auto& j_item = j_scene[i];

      assert(j_item.is_object());

      if (j_item.count("entity")) {
        auto& j_entity = j_item["entity"];

        CHandle h_e;

        // Do we have the prefab key in the json?
        if (j_entity.count("prefab")) {

          // Get the src/id of the prefab
          std::string prefab_src = j_entity["prefab"];
          assert(!prefab_src.empty());

          //this prefab is a product on a shelve, so we will load it only on RELEASE because performance issues
          std::size_t found = prefab_src.find("/products/");
          if (found != std::string::npos) {
            #ifndef NDEBUG
              continue;
            #endif
            // Get delta transform where we should instantiate this transform
            CTransform delta_transform;
            if (j_entity.count("transform"))
              delta_transform.load(j_entity["transform"]);

            int rand_idx = rand();
            if (rand_idx % 5 == 0 || (mod->last_prod_index - mod->first_prod_index) + 1 > 8000) { //instantiate without being an entity

              //ADD DATA TO MODULE GPU CULLING
              std::string prefab_name = j_entity["prefab"].get<std::string>();
              prefab_name.erase(0, 29);
              prefab_name.replace(prefab_name.end() - 5, prefab_name.end(), "");
              int idx = std::stoi(prefab_name);
              //int idx = rand() % prefabs.size();//change this
              CHandle prefab = prefabs[idx - 1];
              CEntity* ep = prefab;
              assert(ep);
              mod->getObjSize(); // so we increase the variables that hold the limits containing the number of products
              addPrefabToModule(ep, j_entity["transform"]);
            }
            else { //create it as an entity so we can hit it
              // Parse the prefab, if any other child is created they will inherit our ctx transform
              TEntityParseContext prefab_ctx(ctx, delta_transform);
              prefab_ctx.parsing_prefab = true;
              if (!parseScene(prefab_src, prefab_ctx))
                continue;

              assert(!prefab_ctx.entities_loaded.empty());

              // Create a new fresh entity
              h_e = prefab_ctx.entities_loaded[0];

              // Cast to entity object
              CEntity* e = h_e;

              // We give an option to 'reload' the prefab by modifying existing components, 
              // like changing the name, add other components, etc, but we don't want to parse again 
              // the comp_transform, because it was already parsed as part of the root
              // As the json is const as it's a resouce, we make a copy of the prefab section and
              // remove the transform
              json j_entity_without_transform = j_entity;
              j_entity_without_transform.erase("transform");

              // Do the parse now outside the 'prefab' context
              prefab_ctx.parsing_prefab = false;
              e->load(j_entity_without_transform, prefab_ctx);

              std::string prefab_name = j_entity["prefab"].get<std::string>();
              prefab_name.erase(0, 29);
              prefab_name.replace(prefab_name.end() - 5, prefab_name.end(), "");
              int idx = std::stoi(prefab_name);
              //int idx = rand() % prefabs.size();
              CHandle prefab = prefabs[idx - 1];
              CEntity* ep = prefab;
              assert(e);

              TCompRender* c_render = e->get<TCompRender>();
              CHandle h(c_render);
              h.destroy();

              TCompAbsAABB* c_absaabb = e->get<TCompAbsAABB>();
              CHandle h2(c_absaabb);
              h2.destroy();

              //IF IS A DYNAMIC INSTANCE (NOT PREFAB) SET UNIQUE IDX BINDING
              TCompDynamicInstance* c_di = e->get<TCompDynamicInstance>();
              if (c_di) {
                c_di->set_idx(mod->getObjSize());
              }

              //ADD DATA TO MODULE GPU CULLING
              addPrefabToModule(ep, j_entity["transform"]);
              ctx.entities_loaded.push_back(h_e);
            }
          }
        }
      }
    }

    // Create a comp_group automatically if there is more than one entity
    if (ctx.entities_loaded.size() > 1) {
      // The first entity becomes the head of the group. He is NOT in the group
      CHandle h_root_of_group = ctx.entities_loaded[0];
      CEntity* e_root_of_group = h_root_of_group;
      assert(e_root_of_group);
      // Create a new instance of the TCompGroup
      CHandle h_group = getObjectManager<TCompGroup>()->createHandle();
      // Add it to the entity
      e_root_of_group->set(h_group.getType(), h_group);
      // Now add the rest of entities created to the group, starting at 1 because 0 is the head
      TCompGroup* c_group = h_group;
      for (size_t i = 1; i < ctx.entities_loaded.size(); ++i)
        c_group->add(ctx.entities_loaded[i]);
    }

    // Notify each entity created that we have finished
    // processing this file
    TMsgEntitiesGroupCreated msg = { ctx };
    for (auto h : ctx.entities_loaded)
      h.sendMsg(msg);


    //REMOVE PREFABS FROM SCENE, THE MODULE WILL RENDER THEM
    for (auto h_prefab : scene_prefabs) {
      CEntity* e = h_prefab.second;
      TCompRender* c_render = e->get<TCompRender>();
      CHandle h(c_render);
      h.destroy();

      TCompAbsAABB* c_absaabb = e->get<TCompAbsAABB>();
      CHandle h2(c_absaabb);
      h2.destroy();

      //h_prefab.second.destroy(); //probably not destroy, just remove render and AABB components so we mantain the collider
    }
    //scene_prefabs.clear();

  }

  void create(const std::string& filename, TEntityParseContext& ctx) {
    ctx.filename = filename;

    const json& j_scene = Resources.get(filename)->as<CJson>()->getJson();
    assert(j_scene.is_array());

    // For each item in the array...
    for (int i = 0; i < j_scene.size(); ++i) {
      auto& j_item = j_scene[i];

      assert(j_item.is_object());

      if (j_item.count("entity")) {
        auto& j_entity = j_item["entity"];

        CHandle h_e;

        // Do we have the prefab key in the json?
        if (j_entity.count("prefab")) {

          // Get the src/id of the prefab
          std::string prefab_src = j_entity["prefab"];
          assert(!prefab_src.empty());

          //this prefab can be a product on a shelve, if it is we will load it only on RELEASE because performance issues on another call
          std::size_t found = prefab_src.find("/products/");
          if (found != std::string::npos) {
              continue;
          }

          // Get delta transform where we should instantiate this transform
          CTransform delta_transform;
          if (j_entity.count("transform"))
            delta_transform.load(j_entity["transform"]);

          // Parse the prefab, if any other child is created they will inherit our ctx transform
          TEntityParseContext prefab_ctx(ctx, delta_transform);
          prefab_ctx.parsing_prefab = true;
          if (!parseScene(prefab_src, prefab_ctx))
            continue;

          assert(!prefab_ctx.entities_loaded.empty());

          // Create a new fresh entity
          for (int ij = 0; ij < prefab_ctx.entities_loaded.size(); ij++)
          {
            h_e = prefab_ctx.entities_loaded[ij];

            // Cast to entity object
            CEntity* e = h_e;

            // We give an option to 'reload' the prefab by modifying existing components, 
            // like changing the name, add other components, etc, but we don't want to parse again 
            // the comp_transform, because it was already parsed as part of the root
            // As the json is const as it's a resouce, we make a copy of the prefab section and
            // remove the transform
            json j_entity_without_transform = j_entity;
            j_entity_without_transform.erase("transform");

            // Do the parse now outside the 'prefab' context
            prefab_ctx.parsing_prefab = false;
            e->load(j_entity_without_transform, prefab_ctx);
          }
          h_e = prefab_ctx.entities_loaded[0];
        }
        else if(!isInstantiable(j_entity)){

          // Create a new fresh entity
          h_e.create< CEntity >();

          // Cast to entity object
          CEntity* e = h_e;

          // Do the parse
          e->load(j_entity, ctx);

          //ASSERT JUST FOR DEBUG; MUST DELETE LATER
          TCompRender* c_render = e->get<TCompRender>();
          if (c_render) {
            if (strcmp(c_render->parts[0].material->tech->getName().c_str(), "objs_culled_by_gpu.tech") == 0) {
              TCompName* c_name = e->get<TCompName>();
              fatal("%s mesh will not be instantiated while its material %s wants it\n", c_name->getName(), c_render->parts[0].material->getName().c_str());
            }
          }
        }
        else {
          //this can be instantiable, by discard
          json j_transform = j_entity["transform"];
          std::string mesh_name = j_entity["render"].value("mesh", "none");
          std::string mat_name = j_entity["render"]["materials"][0];
          const std::string temp_prefab_name = mesh_name + mat_name;
          //FIRST TIME GPU PREFAB
          if (scene_prefabs.count(temp_prefab_name) == 0) { 
            //CREATE GPU PREFAB HANDLE
            // Create a new fresh entity
            h_e.create< CEntity >();
            // Cast to entity object
            CEntity* e = h_e;

            // Do the parse
            e->load(j_entity, ctx);


            //ASSERT JUST FOR DEBUG; MUST DELETE LATER
            TCompRender* c_render = e->get<TCompRender>();
            if (c_render) {
              if (strcmp(c_render->parts[0].material->tech->getName().c_str(), "objs_culled_by_gpu.tech") != 0) {
                TCompName* c_name = e->get<TCompName>();
                fatal("%s mesh will be instantiated while its material %s does not want it\n", c_name->getName(), c_render->parts[0].material->getName().c_str());
              }
            }

            scene_prefabs.insert(std::pair<const std::string, CHandle>(temp_prefab_name, h_e));

            //ADD DATA TO MODULE GPU CULLING
            addPrefabToModule(h_e, j_transform);
          }
          else { //ALREADY PARSED GPU PREFAB
            h_e.create< CEntity >();
            CEntity* e = h_e;
            e->load(j_entity, ctx);

            TCompRender* c_render = e->get<TCompRender>();
            CHandle h(c_render);
            h.destroy();

            TCompAbsAABB* c_absaabb = e->get<TCompAbsAABB>();
            CHandle h2(c_absaabb);
            h2.destroy();
            TCompName* n = e->get<TCompName>();
            std::string a = n->getName();

            //ADD DATA TO MODULE GPU CULLING
            addPrefabToModule(scene_prefabs.at(temp_prefab_name), j_transform);
          }
        }

        ctx.entities_loaded.push_back(h_e);
      }
    }

    // Create a comp_group automatically if there is more than one entity
    if (ctx.entities_loaded.size() > 1) {
      // The first entity becomes the head of the group. He is NOT in the group
      CEntity* e_root_of_group;
      int idx_father = -1;
      do {
        idx_father++;
        CHandle h_root_of_group = ctx.entities_loaded[idx_father];
        e_root_of_group = h_root_of_group;
      } while (!isInstantiable(e_root_of_group));
      assert(e_root_of_group);
      // Create a new instance of the TCompGroup
      CHandle h_group = getObjectManager<TCompGroup>()->createHandle();
      // Add it to the entity
      e_root_of_group->set(h_group.getType(), h_group);
      // Now add the rest of entities created to the group, starting at 1 because 0 is the head
      TCompGroup* c_group = h_group;

      for (size_t i = 0; i < ctx.entities_loaded.size(); ++i) {
        if (i == idx_father)
          continue;
        CEntity* e = ctx.entities_loaded[i];
        TCompName* c_name = e->get<TCompName>();
        if (strcmp(c_name->getName(), "Player") == 0)
          continue;
        c_group->add(ctx.entities_loaded[i]);
      }
    }

    // Notify each entity created that we have finished
    // processing this file
    TMsgEntitiesGroupCreated msg = { ctx };
    for (auto h : ctx.entities_loaded) {
      h.sendMsg(msg);
    }

    //REMOVE PREFABS FROM SCENE, THE MODULE WILL RENDER THEM
    for (auto h_prefab : scene_prefabs) {
      CEntity* e = h_prefab.second;
      TCompRender* c_render = e->get<TCompRender>();
      CHandle h(c_render);
      h.destroy();

      TCompAbsAABB* c_absaabb = e->get<TCompAbsAABB>();
      CHandle h2(c_absaabb);
      h2.destroy();

      //h_prefab.second.destroy(); //probably not destroy, just remove render and AABB components so we mantain the collider
    }
    scene_prefabs.clear();
    
    TFileContext fc(filename);
    TEntityParseContext ctx2;
    createProducts(filename, ctx2);
  }
  
  void addPrefabToModule(CHandle handl, json j) {
    //HERE WE RECEIVE THE RENDER COMPONENT HANDLE, SO WE GET ITS ENTITY
    CHandle prefab = handl;
    CEntity* e = prefab;
    assert(e);

    MAT44 rot_yaw = MAT44::CreateFromQuaternion(loadQUAT(j, "rotation"));
    VEC3 pos = loadVEC3(j, "pos");
    float scale = j.value("scale", 1.0f);
    MAT44 world = rot_yaw * MAT44::CreateScale(scale) * MAT44::CreateTranslation(pos);

    // Find AABB in world space
    TCompRender* cr = e->get< TCompRender >();
    const TCompRender::MeshPart& mp = cr->parts[0];
    AABB aabb_local = mp.mesh->getAABB();
    AABB aabb_abs = getRotatedBy(aabb_local, world);

    mod->addToRender(prefab, aabb_abs, world);
  }

};

TSampleDataGenerator sample_data;

void CModuleGPUCulling::parseEntities(const std::string& filename, TEntityParseContext& ctx) {
  sample_data.create(filename, ctx);
}

void CModuleGPUCulling::parseProducts(const std::string& filename, TEntityParseContext& ctx) {
  #ifndef NDEBUG
    return;
  #endif
  sample_data.createProducts(filename, ctx);
}

void CModuleGPUCulling::createPrefabProducts() {
  #ifndef NDEBUG
    return;
  #endif
  json j = loadJson("data/gpu_culling.json");
  sample_data.createProductPrefabs(j["sample_data"]);
}

void CModuleGPUCulling::clear() {
  #ifndef NDEBUG
    return;
  #endif
  sample_data.deleteProductPrefabs();
}

int CModuleGPUCulling::getObjSize() { 
  //this is called to bind the component to the index of the product 
  int val = objs.size();

  if (first_prod_index > val)
    first_prod_index = val;
  if (last_prod_index < val)
    last_prod_index = val;

  return val; 
}

// ---------------------------------------------------------------
bool CModuleGPUCulling::start() {

  json j = loadJson("data/gpu_culling.json");

  show_debug = j.value("show_debug", show_debug);

  TEntityParseContext ctx;
  comp_compute.load(j["compute"], ctx);
  comp_buffers.load(j["buffers"], ctx);

  // Access buffer to hold the instances in the gpu
  gpu_objs = comp_buffers.getBufferByName("instances");
  assert(gpu_objs || fatal("Missing required buffer to hold the instances to be culled\n"));
  max_objs = gpu_objs->num_elems;
  assert(gpu_objs->bytes_per_elem == sizeof(TObj) || fatal("GPU/CPU struct size don't match for instances %d vs %d\n", gpu_objs->bytes_per_elem, (uint32_t)sizeof(TObj)));

  auto gpu_culled_instances = comp_buffers.getBufferByName("culled_instances");
  assert(gpu_culled_instances);
  assert(gpu_culled_instances->bytes_per_elem == sizeof(MAT44) || fatal("GPU/CPU struct size don't match for culled_instances %d vs %d\n", gpu_culled_instances->bytes_per_elem, (uint32_t)sizeof(MAT44)));

  assert(comp_buffers.getCteByName("TCullingPlanes")->size() == sizeof(TCullingPlanes));

  gpu_ctes_instancing = comp_buffers.getCteByName("TCtesInstancing");
  assert(gpu_ctes_instancing);
  assert(gpu_ctes_instancing->size() == sizeof(TCtesInstancing));

  gpu_prefabs = comp_buffers.getBufferByName("prefabs");
  max_prefabs = gpu_prefabs->num_elems;
  assert(gpu_prefabs->bytes_per_elem == sizeof(TPrefab) || fatal("GPU/CPU struct size don't match for prefabs %d vs %d\n", gpu_prefabs->bytes_per_elem, (uint32_t)sizeof(TPrefab)));

  gpu_draw_datas = comp_buffers.getBufferByName("draw_datas");
  max_render_types = gpu_draw_datas->num_elems;
  assert(gpu_draw_datas->bytes_per_elem == sizeof(TDrawData) || fatal("GPU/CPU struct size don't match for draw_datas %d vs %d\n", gpu_draw_datas->bytes_per_elem, (uint32_t)sizeof(TDrawData)));

  // Reserve in CPU all the memory that we might use, so when we upload cpu data to gpu, we read from valid memory
  // as we upload/read the full buffer size.
  objs.reserve(max_objs);
  prefabs.reserve(max_prefabs);
  render_types.reserve(max_render_types);
  draw_datas.reserve(max_render_types);

  // populate with some random generated data
  sample_data.mod = this;
  //sample_data.createTrees(j["sample_data"]);
  //sample_data.create();

  entity_camera_name = j["camera"];
  assert(!entity_camera_name.empty());

  return true;
}

void CModuleGPUCulling::stop() {
  objs.clear();
}

void CModuleGPUCulling::updateObjData(int idx, CHandle entity) {
  if (objs.size() <= idx) {
    return;
  }

  CEntity* e = entity;
  TCompTransform* c_trans = e->get<TCompTransform>();
  objs.at(idx).world = c_trans->asMatrix();

  TCompLocalAABB* c_aabb = e->get<TCompLocalAABB>();
  AABB aabb_abs = getRotatedBy(*c_aabb, c_trans->asMatrix());


  objs[idx].aabb_center = aabb_abs.Center;
  objs[idx].aabb_half = aabb_abs.Extents;
}

// ---------------------------------------------------------------
// Returns the index position in the array of prefabs 
uint32_t CModuleGPUCulling::addPrefabInstance(CHandle new_id) {
  uint32_t idx = 0;
  for (auto& prefab : prefabs) {
    if (prefab.id == new_id) {
      prefab.num_objs++;
      return idx;
    }
    ++idx;
  }
  idx = registerPrefab(new_id);
  prefabs[idx].num_objs++;
  return idx;
}

uint32_t CModuleGPUCulling::registerPrefab(CHandle new_id) {

  // Register 
  TPrefab prefab;
  prefab.id = new_id;
  prefab.num_objs = 0;
  prefab.num_render_type_ids = 0;

  // Check how many draw calls (instance_types) we have
  CEntity* e = new_id;
  assert(e);
  TCompRender* cr = e->get<TCompRender>();
  assert(cr);
  for (auto& p : cr->parts) {

    // This is the identifier of a draw call
    TRenderTypeID tid;
    tid.group = p.mesh_group;
    tid.mesh = p.mesh;
    tid.material = p.material;

    // Find which type of instance type is this draw call
    uint32_t render_type_id = addRenderType(tid);

    // Save it
    assert(prefab.num_render_type_ids < TPrefab::max_render_types_per_prefab);
    prefab.render_type_ids[prefab.num_render_type_ids] = render_type_id;
    ++prefab.num_render_type_ids;
  }

  assert(prefabs.size() + 1 < max_prefabs || fatal( "We need more space in the gpu buffer 'prefabs'. Current size is %d\n", max_prefabs));
  prefabs.push_back(prefab);
  uint32_t idx = (uint32_t)prefabs.size() - 1;

  // Register a lod if exists a complod in the hi-quality prefab
  TCompLod* c_lod = e->get<TCompLod>();
  if (c_lod) {

    // Load the low-quality prefab
    TEntityParseContext ctx;
    bool is_ok = parseScene(c_lod->replacement_prefab, ctx);
    CHandle h_lod = ctx.entities_loaded[0];

    uint32_t lod_idx = registerPrefab(h_lod);
    setPrefabLod(idx, lod_idx, c_lod->threshold);

    h_lod.destroy();
  }

  return idx;
}

// ---------------------------------------------------------------
void CModuleGPUCulling::setPrefabLod(uint32_t high_prefab_idx, uint32_t low_prefab_idx, float threshold) {
  assert(high_prefab_idx < prefabs.size());
  TPrefab& hq = prefabs[high_prefab_idx];
  hq.lod_prefab = low_prefab_idx;
  hq.lod_threshold = threshold;
}

// ---------------------------------------------------------------
uint32_t CModuleGPUCulling::addRenderType(const TRenderTypeID& new_render_type) {

  uint32_t idx = 0;
  for (auto& render_type : render_types) {
    if (render_type == new_render_type)
      return idx;
    ++idx;
  }

  assert((render_types.size() + 1 < max_render_types) || fatal("Too many (%d) render_types registered. The GPU Buffer 'draw_datas' need to be larger.\n", max_render_types));

  // Register. Copy the key
  render_types.push_back(new_render_type);

  // Create a new name for the prefab
  std::string mesh_name = new_render_type.mesh->getName();
  auto off = mesh_name.find_last_of("/");
  mesh_name = mesh_name.substr(off);
  std::string mat_name = new_render_type.material->getName();
  off = mat_name.find_last_of("/");
  mat_name = mat_name.substr(off);
  snprintf(render_types.back().title, sizeof(TRenderTypeID::title), "%s:%d %s", mesh_name.c_str(), new_render_type.group, mat_name.c_str());

  // Collect the range of triangles we need to render
  TDrawData draw_data = {};
  const TMeshGroup& g = new_render_type.mesh->getGroups()[new_render_type.group];
  draw_data.args.indexCount = g.num_indices;
  draw_data.args.firstIndex = g.first_idx;
  draw_datas.push_back(draw_data);

  return idx;
}

// ---------------------------------------------------------------
void CModuleGPUCulling::addToRender(
  CHandle h_prefab
, const AABB aabb
, const MAT44 world
) {

  assert((objs.size()+1 < max_objs) || fatal("Too many (%d) instances registered. The GPU Buffers 'gpu_instances' need to be larger.\n", max_objs));

  TObj obj;
  obj.aabb_center = aabb.Center;
  obj.prefab_idx = addPrefabInstance(h_prefab);
  obj.aabb_half = aabb.Extents;
  obj.world = world;
  obj.unique_id = objs.size();
  objs.push_back(obj);

  is_dirty = true;
}

// ---------------------------------------------------------------
void CModuleGPUCulling::renderDebug() {
  PROFILE_FUNCTION("GPUCulling");
  if (show_debug) {
    for (auto& obj : objs)
      drawWiredAABB(obj.aabb_center, obj.aabb_half, MAT44::Identity, VEC4(1, 0, 0, 1));
  }
}

void CModuleGPUCulling::update(float delta) {
  // Nothing to be done
}

void CModuleGPUCulling::updateCamera() {
  if (!h_camera.isValid()) {
    h_camera = getEntityByName(entity_camera_name);
    if (!h_camera.isValid())
      return;
  }

  CEntity* e_camera = h_camera;
  if (!e_camera) {
	  return;
  }
		
  TCompCamera* c_camera = e_camera->get<TCompCamera>();
  if (!c_camera) {
    TCompLightDir* c_light_dir = e_camera->get<TCompLightDir>();
    assert(c_light_dir);
    culling_camera = *(CCamera*)c_light_dir;
  }
  else {
    culling_camera = *(CCamera*)c_camera;
    culling_camera = Engine.getCameraMixer().saved_result;
  }

  culling_camera.setViewport(0, 0, Render.width, Render.height);
  updateCullingPlanes(culling_camera);
}

// ---------------------------------------------------------------
void CModuleGPUCulling::updateCullingPlanes(const CCamera& camera) {
  MAT44 m = camera.getViewProjection().Transpose();
  VEC4 mx(m._11, m._12, m._13, m._14);
  VEC4 my(m._21, m._22, m._23, m._24);
  VEC4 mz(m._31, m._32, m._33, m._34);
  VEC4 mw(m._41, m._42, m._43, m._44);
  culling_planes.planes[0] = (mw + mx);
  culling_planes.planes[1] = (mw - mx);
  culling_planes.planes[2] = (mw + my);
  culling_planes.planes[3] = (mw - my);
  culling_planes.planes[4] = (mw + mz);      // + mz if frustum is 0..1
  culling_planes.planes[5] = (mw - mz);
  culling_planes.CullingCameraPos = camera.getPosition();
}

// ---------------------------------------------------------------
void CModuleGPUCulling::renderInMenu() {
  if (ImGui::TreeNode("GPU Culling")) {
    ImGui::Text("%ld objects", (uint32_t)objs.size());
    ImGui::Checkbox("Show Debug", &show_debug);

    if (ImGui::TreeNode("All objs...")) {
      for (auto& obj : objs)
        ImGui::Text("Prefab:%d at %f %f %f", obj.prefab_idx, obj.aabb_center.x, obj.aabb_center.y, obj.aabb_center.z);
      ImGui::TreePop();
    }

    if (ImGui::TreeNode("Prefabs")) {
      int idx = 0;
      for (auto& p : prefabs) {
        char txt[256];
        sprintf( txt, "[%2d] %3d num_objs with %d render types Lod:%d at %f Total:%d", idx, p.num_objs, p.num_render_type_ids, p.lod_prefab, p.lod_threshold, p.total_num_objs);
        if (ImGui::TreeNode(txt)) {
          if (ImGui::TreeNode("Render Types")) {
            for (uint32_t i = 0; i < p.num_render_type_ids; ++i)
              ImGui::Text("[%d] %d", i, p.render_type_ids[i]);
            ImGui::TreePop();
          }
          if (ImGui::TreeNode("Objs...")) {
            for (auto& obj : objs) {
              if( obj.prefab_idx == idx )
                ImGui::Text("Prefab:%d at %f %f %f", obj.prefab_idx, obj.aabb_center.x, obj.aabb_center.y, obj.aabb_center.z);
            }
            ImGui::TreePop();
          }
          ImGui::TreePop();
        }
        ++idx;
      }
      ImGui::TreePop();
    }
    if (ImGui::TreeNode("Render Types")) {
      int idx = 0;
      for (auto& rt : render_types) {
        ImGui::Text("[%d] %d M:%s Mat:%s", idx, rt.group, rt.mesh->getName().c_str(), rt.material->getName().c_str());
        ++idx;
      }
      ImGui::TreePop();
    }
    if (ImGui::TreeNode("Draw Datas")) {
      for (auto& dd: draw_datas)
        ImGui::Text("Base:%3d indices:%4d from:%d", dd.base, dd.args.indexCount, dd.args.firstIndex);
      ImGui::TreePop();
    }
    if (ImGui::TreeNode("GPU Draw Datas")) {
      gpu_draw_datas->copyGPUtoCPU();
      TDrawData* dd = (TDrawData*)gpu_draw_datas->cpu_data.data();
      for (uint32_t i = 0; i < draw_datas.size(); ++i, ++dd ) {
        ImGui::Text("Base:%3d Draw Instances:%3d #Idxs:%4d From:%d max_instances:%d dummy:%d", dd->base, dd->args.instanceCount, dd->args.indexCount, dd->args.firstIndex, dd->max_instances, dd->dummy);
      }
      ImGui::TreePop();
    }
    ImGui::TreePop();
  }
}

// ---------------------------------------------------------------
// Every frame, we need to clear the #instances in the gpu
// We could do that in a cs also.
void CModuleGPUCulling::clearRenderDataInGPU() {

  for (auto& dd : draw_datas) 
    dd.args.instanceCount = 0;

  // This line has to be done at least once
  gpu_draw_datas->copyCPUtoGPUFrom(draw_datas.data());
}

// ---------------------------------------------------------------
void CModuleGPUCulling::preparePrefabs() {

  // Count my official number of objects
  for (auto& p : prefabs)
    p.total_num_objs = p.num_objs;

  // Then, if I can become a lod
  for (auto& p : prefabs) {
    if (p.lod_prefab != -1) {
      auto& p_low = prefabs[p.lod_prefab];
      assert((p_low.lod_prefab == -1) || fatal( "We don't support yet, lod of lod"));
      p_low.total_num_objs += p.total_num_objs;
    }
  }

  // Clear counts
  for (auto& dd : draw_datas)
    dd.max_instances = 0;

  for (auto& p : prefabs) {
    // Each prefab will render potencially in several render types
    for (uint32_t idx = 0; idx < p.num_render_type_ids; ++idx) {
      uint32_t render_type_id = p.render_type_ids[idx];
      draw_datas[render_type_id].max_instances += p.total_num_objs;
    }
  }

  // Set the base now that we now how many instances of each render type we have.
  uint32_t base = 0;
  for (auto& dd : draw_datas) {
    dd.base = base;
    base += dd.max_instances;
  }

  uint32_t max_culled_instances = comp_buffers.getBufferByName("culled_instances")->num_elems;
  assert((base <= max_culled_instances) || fatal("We require more space in the buffer %d. Current %d", base, max_culled_instances));

  assert(gpu_prefabs);
  gpu_prefabs->copyCPUtoGPUFrom(prefabs.data());
}

// ---------------------------------------------------------------
// This is called when we run the Compute Shaders
void CModuleGPUCulling::run() {
  CGpuScope gpu_scope("GPU Culling");

  // Just before starting the cs tasks, get the current data of the camera
  updateCamera();

  // Upload culling planes to GPU
  comp_buffers.getCteByName("TCullingPlanes")->updateGPU(&culling_planes);

  if (is_dirty) {
    gpu_objs->copyCPUtoGPUFrom(objs.data());

    preparePrefabs();

    // Notify total number of objects we must try to cull
    ctes_instancing.total_num_objs = (uint32_t) objs.size();
    gpu_ctes_instancing->updateGPU(&ctes_instancing);

    is_dirty = false;
  }

  clearRenderDataInGPU();

  // Run the culling in the GPU
  comp_compute.executions[0].sizes[0] = (uint32_t) objs.size();
  comp_compute.executions[0].run(&comp_buffers);
}

void CModuleGPUCulling::runWithCustomCamera(const CCamera& light_camera) {
  CVertexShader::deactivateResources();
  CGpuScope gpu_scope("GPU Culling Lights");

  // Just before starting the cs tasks, get the current data of the camera
  MAT44 m = light_camera.getViewProjection().Transpose();
  VEC4 mx(m._11, m._12, m._13, m._14);
  VEC4 my(m._21, m._22, m._23, m._24);
  VEC4 mz(m._31, m._32, m._33, m._34);
  VEC4 mw(m._41, m._42, m._43, m._44);
  culling_planes.planes[0] = (mw + mx);
  culling_planes.planes[1] = (mw - mx);
  culling_planes.planes[2] = (mw + my);
  culling_planes.planes[3] = (mw - my);
  culling_planes.planes[4] = (mw + mz);      // + mz if frustum is 0..1
  culling_planes.planes[5] = (mw - mz);
  culling_planes.CullingCameraPos = light_camera.getPosition();

  // Upload culling planes to GPU
  comp_buffers.getCteByName("TCullingPlanes")->updateGPU(&culling_planes);

  if (is_dirty) {
    gpu_objs->copyCPUtoGPUFrom(objs.data());

    preparePrefabs();

    // Notify total number of objects we must try to cull
    ctes_instancing.total_num_objs = (uint32_t)objs.size();
    gpu_ctes_instancing->updateGPU(&ctes_instancing);

    is_dirty = false;
  }

  clearRenderDataInGPU();

  // Run the culling in the GPU
  comp_compute.executions[0].sizes[0] = (uint32_t)objs.size();
  comp_compute.executions[0].run(&comp_buffers);
  CComputeShader::deactivate();
}

// ---------------------------------------------------------------
// This is called when from the RenderManager
void CModuleGPUCulling::renderCategory(eRenderCategory category) {
  CGpuScope gpu_scope("GPU Culling");

  // Right now only supporting solids
  if (category != eRenderCategory::CATEGORY_SOLIDS && category != eRenderCategory::CATEGORY_SHADOWS)
    return;

  if (category == eRenderCategory::CATEGORY_SOLIDS) {
    CGpuScope gpu_scope("Compute Shaders AGAIN");
    // Can't update a buffer if it's still bound as vb. So unbound it.
    CVertexShader::deactivateResources();
    Engine.getGPUCulling().run();
    CComputeShader::deactivate();
  }

  // Activate in the vs
  assert(gpu_ctes_instancing->slotIndex() == 13);
  gpu_ctes_instancing->activate();

  if (category == eRenderCategory::CATEGORY_SHADOWS) {
    render_types[0].material->getShadowsMaterial()->activate();
    render_types[0].material->getShadowsMaterial()->activateCompBuffers(&comp_buffers);
  }

  // Offset to the args of the draw indexed instanced args in the draw_datas gpu buffer
  uint32_t offset = 0;
  uint32_t idx = 0;
  for( auto& render_type : render_types ) {
    CGpuScope gpu_render_type(render_type.title);

    // Setup material & meshes
    if (category != eRenderCategory::CATEGORY_SHADOWS) {
      render_type.material->activate();
      render_type.material->activateCompBuffers(&comp_buffers);
    }
    else if (!render_type.material->castsShadows()){
      // The offset is in bytes
      offset += sizeof(TDrawData);
      ++idx;
      continue;
    }

    // Because SV_InstanceID always start at zero, but the matrices
    // of each group have different starting offset
    ctes_instancing.instance_base = draw_datas[idx].base;
    gpu_ctes_instancing->updateGPU(&ctes_instancing);
    
    render_type.mesh->activate();
    Render.ctx->DrawIndexedInstancedIndirect(gpu_draw_datas->buffer, offset);

    // The offset is in bytes
    offset += sizeof(TDrawData);
    ++idx;
  }

}

