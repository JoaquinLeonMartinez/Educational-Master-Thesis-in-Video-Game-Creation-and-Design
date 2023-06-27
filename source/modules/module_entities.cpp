#include "mcv_platform.h"
#include "module_entities.h"
#include "entity/entity.h"
//#include "render/render_objects.h"
//#include "render/texture/texture.h"
//#include "render/texture/material.h"
//#include "components/comp_render.h"
//#include "components/comp_transform.h"
//#include "components/comp_name.h"
#include "components/common/comp_tags.h"
//#include "components/comp_light_dir.h"
#include "render/render_manager.h"

void CModuleEntities::loadListOfManagers(const json& j, std::vector< CHandleManager* > &managers) {
  managers.clear();
  // For each entry in j["update"] add entry to om_to_update
  std::vector< std::string > names = j;
  for (auto& n : names) {
    auto om = CHandleManager::getByName(n.c_str());
    assert(om || fatal("Can't find a manager of components of type %s to update. Check file components.json\n", n.c_str()));
    managers.push_back(om);
  }
}

bool CModuleEntities::start()
{
  json j = loadJson("data/components.json");

  // Initialize the ObjManager preregistered in their constructors
  // with the amount of components defined in the data/components.json
  std::map< std::string, int > comp_sizes = j["sizes"];;
  int default_size = comp_sizes["default"];

  // Reorder the init manager based on the json
  // The bigger the number in the init_order section, the lower comp_type id you get
  std::map< std::string, int > init_order = j["init_order"];;
  std::sort(CHandleManager::predefined_managers
    , CHandleManager::predefined_managers + CHandleManager::npredefined_managers
    , [&init_order](CHandleManager* m1, CHandleManager* m2) {
    int priority_m1 = init_order[m1->getName()];
    int priority_m2 = init_order[m2->getName()];
    return priority_m1 > priority_m2;
  });
  // Important that the entity is the first one for the chain destruction of components
  assert(strcmp(CHandleManager::predefined_managers[0]->getName(), "entity") == 0);

  // Now with the sorted array
  for (size_t i = 0; i < CHandleManager::npredefined_managers; ++i) {
    auto om = CHandleManager::predefined_managers[i];
    auto it = comp_sizes.find(om->getName());
    int sz = (it == comp_sizes.end()) ? default_size : it->second;
    dbg("Initializing obj manager %s with %d\n", om->getName(), sz);
    om->init(sz, false);
  }

  loadListOfManagers(j["update"], om_to_update);
  loadListOfManagers(j["render_debug"], om_to_render_debug);

  std::vector< std::string > names = j["multithread"];
  for (auto& n : names) {
    auto om = CHandleManager::getByName(n.c_str());
    assert(om || fatal("Can't find a manager of components of type %s to update. Check file components.json\n", n.c_str()));
    om->multithreaded = true;
  }

  return true;
}

void CModuleEntities::update(float delta) {
  PROFILE_FUNCTION("CModuleEntities::update");

  for (auto om : om_to_update) {
    PROFILE_FUNCTION(om->getName());
    om->updateAll(Time.delta);
  }

  CHandleManager::destroyAllPendingObjects();
}

void CModuleEntities::stop() {
  //CHandleManager::destroyAllPendingObjects();

  // Destroy all entities, should destroy all components in chain
  auto hm = getObjectManager<CEntity>();
  hm->forEach([](CEntity* e) {
    CHandle h(e);
    h.destroy();
  });

  CHandleManager::destroyAllPendingObjects();
}

void CModuleEntities::renderInMenu()
{
  //Resources.debugInMenu();

  ImGui::DragFloat("Time Factor", &Time.scale_factor, 0.01f, 0.f, 1.0f);

  if (ImGui::TreeNode("All Entities...")) {

    ImGui::SameLine();
    static bool flat = false;
    ImGui::Checkbox("Flat", &flat);

    static ImGuiTextFilter Filter;
    ImGui::SameLine();
    Filter.Draw("Filter");

    auto om = getObjectManager<CEntity>();
    om->forEach([](CEntity* e) {
      CHandle h_e(e);
      if (!flat && h_e.getOwner().isValid())
        return;
      if (Filter.IsActive() && !Filter.PassFilter(e->getName()))
        return;
      ImGui::PushID(e);
      e->debugInMenu();
      ImGui::PopID();
    });
    ImGui::TreePop();
  }

  if (ImGui::TreeNode("All Components...")) {
    for (uint32_t i = 1; i < CHandleManager::getNumDefinedTypes(); ++i)
      CHandleManager::getByType(i)->debugInMenuAll();
    ImGui::TreePop();
  }

  CHandle h_player = getEntityByName("Player");
  if (h_player.isValid())
    h_player.debugInMenu();

  CTagsManager::get().renderInMenu();

  CRenderManager::get().renderInMenu();

  CHandleManager::destroyAllPendingObjects();
}

void CModuleEntities::renderDebug() {
  renderDebugOfComponents();
}

void CModuleEntities::renderDebugOfComponents() {
  CGpuScope gpu_scope("renderDebugOfComponents");
  PROFILE_FUNCTION("renderDebugOfComponents");
  for (auto om : om_to_render_debug) {
    PROFILE_FUNCTION(om->getName());
    CGpuScope gpu_scope(om->getName());
    om->renderDebugAll();
  }

}

CHandle CModuleEntities::getPlayerHandle() {

  if (player_handle.isValid()) {
    return player_handle;
  }
  else {
    player_handle = getEntityByName("Player");
    return player_handle;
  }
}

CHandle CModuleEntities::getInventoryHandle() {

	if (inventory_handle.isValid()) {
		return inventory_handle;
	}
	else {
		inventory_handle = getEntityByName("Inventory");
		return inventory_handle;
	}
}

void CModuleEntities::destroyEntities() {
	auto om = getObjectManager<CEntity>();
	om->forEach([](CEntity* e) {
		CHandle h_e(e);
		h_e.destroy();
	});
	CHandleManager::destroyAllPendingObjects();
}