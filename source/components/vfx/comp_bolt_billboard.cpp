#include "mcv_platform.h"
#include "comp_bolt_billboard.h"
#include "components/common/comp_transform.h"
#include "components/common/comp_buffers.h"
#include "components/common/physics/comp_collider.h"

DECL_OBJ_MANAGER("bolt_billboard", TCompBoltBillboard);

void TCompBoltBillboard::debugInMenu() {

}

void TCompBoltBillboard::load(const json& j, TEntityParseContext& ctx) {

}


void TCompBoltBillboard::renderDebug() {

}

void TCompBoltBillboard::update(float delta) {

  TCompTransform* cTransform = get<TCompTransform>();

  CEntity* e_pos = target_position;
  if (!e_pos)
    return;
  TCompTransform* t_pos = e_pos->get<TCompTransform>();

  CEntity* e_aim = target_aim;
  if (!e_aim)
    return;
  TCompTransform* t_aim = e_aim->get<TCompTransform>();
  VEC3 pos_to_aim = t_aim->getPosition();

  TCompCollider* c_collider = e_aim->get<TCompCollider>();
  if (c_collider->controller)
    pos_to_aim += VEC3(0, c_collider->controller->getHeight() / 2.0f + c_collider->controller->getRadius(), 0);

  //cTransform->rotateTowards(pos_to_aim);
  //cTransform->inclineTo(pos_to_aim);
  
  CEntity* e_cam = getEntityByName("MainCamera");
  TCompTransform* cam_trans = e_cam->get<TCompTransform>();
  VEC3 front = pos_to_aim - t_pos->getPosition();
  front.Normalize();
  cTransform->lookAt(t_pos->getPosition(), t_pos->getPosition() + front, cam_trans->getUp());
  float dist = VEC3::Distance(pos_to_aim, t_pos->getPosition());
  cTransform->setScale(dist);

  if (prev_scale != dist) {
    constants.x = dist;
    TCompBuffers* c_buff = get<TCompBuffers>();
    if (c_buff) {
      auto buf = c_buff->getCteByName("TCtesBolt");
      buf->updateGPU(&constants);
    }
  }

  prev_scale = dist;
}


