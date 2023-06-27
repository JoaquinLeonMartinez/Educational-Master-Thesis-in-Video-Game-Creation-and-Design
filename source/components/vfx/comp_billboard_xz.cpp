#include "mcv_platform.h"
#include "comp_billboard_xz.h"
#include "components/common/comp_transform.h"
#include "components/common/comp_buffers.h"
#include "components/common/physics/comp_collider.h"

DECL_OBJ_MANAGER("billboard_xz", TCompBillboardXZ);

void TCompBillboardXZ::debugInMenu() {

}

void TCompBillboardXZ::load(const json& j, TEntityParseContext& ctx) {

}


void TCompBillboardXZ::renderDebug() {

}

void TCompBillboardXZ::update(float delta) {

  TCompTransform* cTransform = get<TCompTransform>();

  CEntity* e_pos = getEntityByName("PlayerCamera");
  if (!e_pos)
    return;
  TCompTransform* t_pos = e_pos->get<TCompTransform>();

  float deltaYaw = cTransform->getDeltaYawToAimTo(t_pos->getPosition());

  float yaw, pitch;
  cTransform->getAngles(&yaw, &pitch);

  cTransform->setAngles(yaw + deltaYaw + 1.57f, pitch); //+1.57f because of plane mesh problems...
}


