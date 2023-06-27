#include "mcv_platform.h"
#include "comp_aim_controller.h"
#include "components/common/comp_buffers.h"
#include "components/common/comp_render.h"
#include "components/controllers/character/comp_character_controller.h"


DECL_OBJ_MANAGER("aim_controller", TCompAimController);

using namespace physx;

  void TCompAimController::updateFilterData() {
    filter_data.word0 = filter_group; // word0 = own ID
  }

  void TCompAimController::debugInMenu() {
    ImGui::DragFloat("Max Distance", &max_distance, 0.1f, 0.f, 200.f);
    ImGui::DragFloat3("Aimed", &closest_impact.x, 0.1f);
  }
  
  void TCompAimController::load(const json& j, TEntityParseContext& ctx) {
    filter_group = CModulePhysics::FilterGroup::NotPlayer;
    updateFilterData();
    max_distance = j.value("max_distance", max_distance);
  }

  void TCompAimController::registerMsgs() {
  }

  void TCompAimController::renderDebug() {
    drawLine(ray_origin, ray_origin + ray_front * max_distance, VEC4(0, 1, 0, 1));
    drawLine(closest_impact, closest_impact + closest_normal, VEC4(1, 0, 1, 1));
  }

  void TCompAimController::update(float dt ) {
	  PROFILE_FUNCTION("aim_controller");

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

    hit = PxRaycastBuffer(hitBuffer, bufferSize); // [out] Blocking and touching hits stored here

    hit_status = scene->raycast(
      VEC3_TO_PXVEC3(ray_origin),
      VEC3_TO_PXVEC3(ray_front),
      max_distance,
      hit,
      outputFlags,
      filter_data
    );

    closest = 200.f;
    closest_impact = VEC3();
    closest_normal = VEC3();
    for (PxU32 i = 0; i < hit.nbTouches; i++) {
      auto& touch = hit.touches[i];
      VEC3 impact = PXVEC3_TO_VEC3(touch.position);
      float distance = VEC3::Distance(ray_origin, impact);
      if (distance < closest) {
        closest = distance;
        closest_impact = impact;
        closest_normal = PXVEC3_TO_VEC3(touch.normal);
      }
    }

	CEntity* line = getEntityByName("AimLineScanner");
	if (line)
	{
		TCompBuffers* c_buff = line->get<TCompBuffers>();
		auto buf = c_buff->getCteByName("TCtesAimLineScanner");
		constants.x = closest;
		buf->updateGPU(&constants);
	}

	CEntity* en = getEntityByName("Player");
	TCompCharacterController* p_contr = en->get<TCompCharacterController>();
	TCompRender* c_render = line->get<TCompRender>();
	if (p_contr->aiming) {
		c_render->is_visible = true;
	}
	else {
		c_render->is_visible = false;
	}
	c_render->updateRenderManager();
	
  }



