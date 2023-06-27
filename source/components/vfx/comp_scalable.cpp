#include "mcv_platform.h"
#include "comp_scalable.h"
#include "components/common/comp_transform.h"

DECL_OBJ_MANAGER("scalable", TCompScalable);

void TCompScalable::debugInMenu() {

}

void TCompScalable::load(const json& j, TEntityParseContext& ctx) {
  //get final scale and time
  final_scale = j.value("final_scale", final_scale);
  time_to_scale = j.value("time_to_scale", time_to_scale);
  wait_til_return = j.value("wait_til_return", wait_til_return);
  has_to_return = j.value("return", has_to_return);
  enabled = j.value("enabled", enabled);
}

void TCompScalable::registerMsgs() {
  DECL_MSG(TCompScalable, TMsgEntityCreated, onCreation);
}

void TCompScalable::onCreation(const TMsgEntityCreated& msgC) {
  //get initial scale
  TCompTransform* cTransform = get<TCompTransform>();
  initial_scale = cTransform->getScale();
  //calculate speed with all the data
  speed = (final_scale - initial_scale) / time_to_scale;
}

void TCompScalable::renderDebug() {

}

void TCompScalable::update(float delta) {

  TCompTransform* cTransform = get<TCompTransform>();
  float actual_scale = cTransform->getScale();
  if (!end_reached) {
    actual_scale = clamp(actual_scale + speed * delta, -final_scale, final_scale);
  }
  else if (end_reached && has_to_return) {
    actual_scale = clamp(actual_scale - speed * delta, initial_scale, final_scale);
  }
 
  if (actual_scale == final_scale) {
    timer += delta;
    if (timer >= wait_til_return) {
      end_reached = true;
    }
  }
  if (actual_scale == initial_scale) {
    has_to_return = false;
  }
  cTransform->setScale(actual_scale);
}


