#include "mcv_platform.h"
#include "engine.h"
#include "modules/module_physics.h"
#include "components/common/physics/comp_collider.h"
#include "components/common/comp_transform.h"

using namespace physx;

using FilterGroupNames = NamedValues< CModulePhysics::FilterGroup>;

FilterGroupNames::TEntry filter_group_names[] = {
  {CModulePhysics::FilterGroup::Player, "player"},
  {CModulePhysics::FilterGroup::Enemy, "enemy"},
  {CModulePhysics::FilterGroup::Floor, "floor"},
  {CModulePhysics::FilterGroup::Scenario, "scenario"},
  {CModulePhysics::FilterGroup::Wall, "Wall"},
  {CModulePhysics::FilterGroup::TeleportableObj, "Tp Obj" },
  {CModulePhysics::FilterGroup::TeleportableEnemy, "Tp Enemy" },
  {CModulePhysics::FilterGroup::NotPlayer, "Not Player" },
  {CModulePhysics::FilterGroup::All, "all"},
  {CModulePhysics::FilterGroup::Characters, "characters"},
};
static FilterGroupNames filter_names(filter_group_names, sizeof(filter_group_names)/sizeof(FilterGroupNames::TEntry));

class TCompSensor : public TCompBase {

  CModulePhysics::FilterGroup filter_group;
  CModulePhysics::FilterGroup filter_mask;
  PxFilterData                filter_data;

  bool                        frozen = false;
  bool                        multiple_hits = false;
  VEC3                        ray_origin;
  VEC3                        ray_front;

  // https://docs.nvidia.com/gameworks/content/gameworkslibrary/physx/guide/Manual/SceneQueries.html
  bool                        hit_status = false;
  PxRaycastBuffer             hit;
  PxQueryFilterData           query_filter_data;
  float                       max_distance = 5.0f;

  static const PxU32          bufferSize = 256;        // [in] size of 'hitBuffer'
  PxRaycastHit                hitBuffer[bufferSize];   // [out] User provided buffer for results
  
  DECL_SIBLING_ACCESS();

  void updateFilterData() {
    filter_data.word0 = filter_group; // word0 = own ID
    filter_data.word1 = filter_mask;	  // word1 = ID mask to filter pairs that trigger a contact callback;
  }

  const char* getActorName( PxActor* actor ) {
    CHandle h_collider;
    h_collider.fromVoidPtr(actor->userData);
    CEntity* e_hit = h_collider.getOwner();
    return e_hit ? e_hit->getName() : "<NoActor>";
  }

public:
  
  void debugInMenu() {
    bool changed = false;
    changed |= filter_names.debugInMenu("Filter Group", filter_group);
    changed |= filter_names.debugInMenu("Filter Mask", filter_mask);
    if (changed)
      updateFilterData();
    ImGui::Checkbox("Frozen", &frozen);
    ImGui::Checkbox("Multiple Hits", &multiple_hits);
    ImGui::DragFloat("Max Distance", &max_distance, 0.1f, 0.f, 20.f);
    ImGui::LabelText("Hit", "%s", hit_status ? "Yes" : "No");
    if (ImGui::TreeNode("Touches")) {
      for (PxU32 i = 0; i < hit.nbTouches; i++) {
        auto& touch = hit.touches[i];
        VEC3 impact = PXVEC3_TO_VEC3(touch.position);
        float distance = VEC3::Distance(ray_origin, impact);
        const char* e_name = getActorName(touch.actor);
        ImGui::Text(" %d at %f units [%s]", i, distance, e_name);
      }
      ImGui::TreePop();
    }
    if (ImGui::TreeNode("Blocker")) {
      if (hit.hasBlock) {
        const char* e_name = getActorName(hit.block.actor);
        ImGui::Text(" Block at %f units [%s]", hit.block.distance, e_name);
      }
      ImGui::TreePop();
    }
  }
  
  void load(const json& j, TEntityParseContext& ctx) {
    filter_group = filter_names.valueOf(j.value("group", "all").c_str());
    filter_mask = filter_names.valueOf(j.value("mask", "all").c_str());
    updateFilterData();
    max_distance = j.value("max_distance", max_distance);
  }

  static void registerMsgs() {
  }

  void renderDebug() {
    drawLine(ray_origin, ray_origin + ray_front * max_distance, VEC4(0, 1, 0, 1));
    for (PxU32 i = 0; i < hit.nbTouches; i++) {
      auto& touch = hit.touches[i];
      VEC3 impact = PXVEC3_TO_VEC3(touch.position);
      VEC3 normal = PXVEC3_TO_VEC3(touch.normal);
      drawLine(impact, impact + normal, VEC4(1, 0, 1, 1));
    }
  }

  void update(float dt ) {
	  PROFILE_FUNCTION("sensor");
    if (frozen)
      return;

    TCompTransform* c_trans = get<TCompTransform>();
    assert(c_trans);

    auto scene = EnginePhysics.getScene();

    ray_origin = c_trans->getPosition();
    ray_front = c_trans->getFront();

    PxQueryFilterData filter_data = PxQueryFilterData();
    filter_data.data.word0 = filter_group;

    // [in] Define what parts of PxRaycastHit we're interested in
    const PxHitFlags outputFlags = 
        PxHitFlag::eDISTANCE 
      | PxHitFlag::ePOSITION 
      | PxHitFlag::eNORMAL
      ;

    if (multiple_hits) {
      hit = PxRaycastBuffer(hitBuffer, bufferSize); // [out] Blocking and touching hits stored here
    }
    else {
      hit = PxRaycastBuffer();
    }

    hit_status = scene->raycast(
      VEC3_TO_PXVEC3(ray_origin),
      VEC3_TO_PXVEC3(ray_front),
      max_distance,
      hit,
      outputFlags,
      filter_data
    );


  }

};


DECL_OBJ_MANAGER("sensor", TCompSensor);

