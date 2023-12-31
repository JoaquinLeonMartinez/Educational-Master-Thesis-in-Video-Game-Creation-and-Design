#include "mcv_platform.h"
#include "handle/handle.h"
#include "entity.h"
#include "entity_parser.h"
#include "common_msgs.h"
#include "utils/json_resource.h"
#include "components/common/comp_group.h"

// Find in the current list of entities created, the first entity matching
// the given name
CHandle TEntityParseContext::findEntityByName(const std::string& name) const {

  // Search linearly in the list of entity currently loaded
  for (auto h : entities_loaded) {
    CEntity* e = h;
    if (e->getName() == name )
      return h;
  }

  // Delegate it to my parent
  if (parent)
    return parent->findEntityByName(name);

  return getEntityByName(name);
}

TEntityParseContext::TEntityParseContext(TEntityParseContext& another_ctx, const CTransform& delta_transform) {
  parent = &another_ctx;
  recursion_level = another_ctx.recursion_level + 1;
  entity_starting_the_parse = another_ctx.entity_starting_the_parse;
  root_transform = another_ctx.root_transform.combineWith(delta_transform);
  //VEC3 p = root_transform.getPosition(); float y, pitch; root_transform.getYawPitchRoll(&y, &pitch);
  //dbg("New root transform is Pos:%f %f %f Yaw: %f\n", p.x, p.y, p.z, rad2deg(y));
}

// 
bool parseScene(const std::string& filename, TEntityParseContext& ctx) {

  ctx.filename = filename;

  const json& j_scene = Resources.get(filename)->as<CJson>()->getJson();
  assert(j_scene.is_array());

  // For each item in the array...
  for (int i = 0; i < j_scene.size(); ++i ) {
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

        // Get delta transform where we should instantiate this transform
        CTransform delta_transform;
        if (j_entity.count("transform")) 
          delta_transform.load(j_entity["transform"]);

        // Parse the prefab, if any other child is created they will inherit our ctx transform
        TEntityParseContext prefab_ctx(ctx, delta_transform);
        prefab_ctx.parsing_prefab = true;
        if (!parseScene(prefab_src, prefab_ctx))
          return false;

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

      }
      else {

        // Create a new fresh entity
        h_e.create< CEntity >();

        // Cast to entity object
        CEntity* e = h_e;

        // Do the parse
        e->load(j_entity, ctx);

      }

      ctx.entities_loaded.push_back(h_e);
    }
  }

  // Create a comp_group automatically if there is more than one entity
  if (ctx.entities_loaded.size() > 1 ) {
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

  return true;
}
