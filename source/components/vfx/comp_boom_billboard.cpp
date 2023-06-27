#include "mcv_platform.h"
#include "comp_boom_billboard.h"
#include "components/common/comp_transform.h"
#include "components/common/comp_buffers.h"

DECL_OBJ_MANAGER("boom_billboard", TCompBoomBillboard);

void TCompBoomBillboard::debugInMenu() {

}

void TCompBoomBillboard::load(const json& j, TEntityParseContext& ctx) {

}


void TCompBoomBillboard::renderDebug() {

}

void TCompBoomBillboard::update(float delta) {

  timer += delta;

  TCompTransform* cTransform = get<TCompTransform>();

  TCompTransform* t_pos = get<TCompTransform>();
  
  CEntity* e_cam = getEntityByName("PlayerCamera");
  TCompTransform* cam_trans = e_cam->get<TCompTransform>();
  VEC3 front = cam_trans->getPosition() - t_pos->getPosition();
  front.Normalize();
  cTransform->lookAt(t_pos->getPosition(), t_pos->getPosition() - front, cam_trans->getUp());


  TCompBuffers* c_buff = get<TCompBuffers>();
  if (c_buff) {
    constants.x = 1.0f - clamp((timer - 0.8f) / 0.2f, 0.0f, 1.0f);

    auto buf = c_buff->getCteByName("TCtesBoom");
    buf->updateGPU(&constants);
  }
}


