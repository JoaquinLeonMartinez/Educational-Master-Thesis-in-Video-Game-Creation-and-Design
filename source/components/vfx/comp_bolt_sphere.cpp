#include "mcv_platform.h"
#include "comp_bolt_sphere.h"
#include "components/common/comp_buffers.h"
#include "components/common/comp_transform.h"

DECL_OBJ_MANAGER("bolt_sphere", TCompBoltSphere);

void TCompBoltSphere::debugInMenu() {

}

void TCompBoltSphere::load(const json& j, TEntityParseContext& ctx) {
  speed = j.value("speed", speed);
}


void TCompBoltSphere::renderDebug() {

}

void TCompBoltSphere::registerMsgs() {
  DECL_MSG(TCompBoltSphere, TMsgEntityCreated, onCreation);
}

void TCompBoltSphere::onCreation(const TMsgEntityCreated& msgC) {
  TCompTransform* c_trans = get<TCompTransform>();
  if (c_trans) {
    c_trans = get<TCompTransform>();
    VEC3 pos = c_trans->getPosition();
    target_position = pos + VEC3(0,2,0);
  }
}

void TCompBoltSphere::update(float delta) {

  TCompTransform* cTransform = get<TCompTransform>();

  VEC3 dir = target_position - cTransform->getPosition();
  if (dir.Length() < 0.1f)
    return;

  dir.Normalize();
  VEC3 newPos = cTransform->getPosition() + dir * delta * speed;
  cTransform->setPosition(newPos);

  CEntity* e = getEntityByName("SparkParticles");
  if (e) {
    TCompBuffers* c_buff = e->get<TCompBuffers>();
    if (c_buff) {
      auto buf = c_buff->getCteByName("TCtesParticles");
      CCteBuffer<TCtesParticles>* data = dynamic_cast<CCteBuffer<TCtesParticles>*>(buf);
      data->emitter_center = newPos;
      data->updateGPU();
    }
  }
}