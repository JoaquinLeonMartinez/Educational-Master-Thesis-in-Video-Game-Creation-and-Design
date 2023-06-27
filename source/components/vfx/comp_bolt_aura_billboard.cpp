#include "mcv_platform.h"
#include "comp_bolt_aura_billboard.h"
#include "components/common/comp_transform.h"

DECL_OBJ_MANAGER("bolt_aura_billboard", TCompBoltAuraBillboard);

void TCompBoltAuraBillboard::debugInMenu() {

}

void TCompBoltAuraBillboard::load(const json& j, TEntityParseContext& ctx) {

}


void TCompBoltAuraBillboard::renderDebug() {

}

void TCompBoltAuraBillboard::update(float delta) {

  TCompTransform* cTransform = get<TCompTransform>();

  CEntity* e_pos = target_position;
  if (!e_pos)
    return;
  TCompTransform* t_pos = e_pos->get<TCompTransform>();
  
  CEntity* e_cam = getEntityByName("PlayerCamera");
  TCompTransform* cam_trans = e_cam->get<TCompTransform>();
  VEC3 front = cam_trans->getPosition() - t_pos->getPosition();
  front.Normalize();
  cTransform->lookAt(t_pos->getPosition(), t_pos->getPosition() - front, cam_trans->getUp());
}


