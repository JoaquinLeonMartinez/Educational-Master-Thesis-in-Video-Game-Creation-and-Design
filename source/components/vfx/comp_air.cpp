#include "mcv_platform.h"
#include "comp_air.h"
#include "components/common/comp_buffers.h"
#include "components/common/comp_transform.h"


DECL_OBJ_MANAGER("air", TCompAir);

void TCompAir::debugInMenu() {

}

void TCompAir::load(const json& j, TEntityParseContext& ctx) {
}

void TCompAir::renderDebug() {
}

void TCompAir::update(float delta) {
  TCompBuffers* c_buff = get<TCompBuffers>();
  if (c_buff) {
    //constants.x = Interpolator::cubicInOut(0.0f, 1.0f, ratio);
    ratio += delta;
    constants.x = len;
    constants.y = d;
    constants.z = Interpolator::quadOut(-d - len, 1.0f, ratio/ destroy); //time 

    auto buf = c_buff->getCteByName("TCtesAir");
    buf->updateGPU(&constants);
  }

  TCompTransform* c_trans = get<TCompTransform>();
  if (c_trans == nullptr) return;
  float factor = 1.0f - ratio/destroy;
  c_trans->setPosition(c_trans->getPosition() + (c_trans->getFront() * delta * ((speed * factor) + 0.2f)) );
}


