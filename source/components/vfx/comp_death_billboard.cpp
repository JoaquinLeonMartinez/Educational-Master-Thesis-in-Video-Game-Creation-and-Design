#include "mcv_platform.h"
#include "comp_death_billboard.h"
#include "components/common/comp_transform.h"
#include "components/common/comp_buffers.h"
#include "components/common/physics/comp_collider.h"

DECL_OBJ_MANAGER("death_billboard", TCompDeathBillboard);

void TCompDeathBillboard::debugInMenu() {

}

void TCompDeathBillboard::load(const json& j, TEntityParseContext& ctx) {

}

void TCompDeathBillboard::renderDebug() {

}

void TCompDeathBillboard::update(float delta) {
  if (!done) {
    done = true;
    if (particles.isValid()) {
      CEntity* e = particles;
      TCompBuffers* c_buff = e->get<TCompBuffers>();
      if (c_buff) {
        auto buf = c_buff->getCteByName("TCtesParticles");
        TCompTransform* c_trans = get<TCompTransform>();
        CCteBuffer<TCtesParticles>* data = dynamic_cast<CCteBuffer<TCtesParticles>*>(buf);
        data->emitter_center = c_trans->getPosition();
        data->updateGPU();
      }
    }
  }

  TCompTransform* cTransform = get<TCompTransform>();

  CEntity* e_cam = getEntityByName("PlayerCamera");
  TCompTransform* cam_trans = e_cam->get<TCompTransform>();
  VEC3 front = cam_trans->getPosition() - cTransform->getPosition();
  front.Normalize();
  cTransform->lookAt(cTransform->getPosition(), cTransform->getPosition() - front, cam_trans->getUp());
}


