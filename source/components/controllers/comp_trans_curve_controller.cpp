#include "mcv_platform.h"
#include "comp_trans_curve_controller.h"
#include "components/common/comp_transform.h"
#include "entity/common_msgs.h"

DECL_OBJ_MANAGER("trans_curve_controller", TCompTransCurveController);

void TCompTransCurveController::debugInMenu() {
  ImGui::Checkbox("Enabled", &_enabled);
  ImGui::Checkbox("Loops", &_loop);
  if (ImGui::DragFloat("Ratio", &_ratio, 0.01f, 0.f, 1.0f))
    setRatio(_ratio);
  ImGui::DragFloat("Speed", &_speed, 0.01f, 0.f, 10.0f);
  if (!_curve)
    ImGui::Text("Curve is not valid!");
  if (ImGui::TreeNode("Base")) {
    base.renderInMenu();
    ImGui::TreePop();
  }
}

void TCompTransCurveController::registerMsgs() {
  DECL_MSG(TCompTransCurveController, TMsgEntityCreated, onEntityCreated);
}

void TCompTransCurveController::onEntityCreated(const TMsgEntityCreated& msg) {
  if (!_base_defined) {
    // On Start capture base from entity position unless defined in the json
    TCompTransform* c = get<TCompTransform>();
    base = *(CTransform*)c;
  }
}

void TCompTransCurveController::load(const json& j, TEntityParseContext& ctx) {
  _curve = Resources.get(j.value("curve", ""))->as<CTransCurve>();
  _enabled = j.value("enabled", _enabled);
  _loop = j.value("loop", _loop);
  _ratio = j.value("ratio", _ratio);
  _speed = j.value("speed", _speed);
  if (j.count("base")) {
    _base_defined = true;
    base.load(j["base"]);
  }
}

void TCompTransCurveController::update(float delta) {
  if (!_enabled )
    return;
  float new_ratio = _ratio + delta * _speed;
  setRatio(new_ratio);
}

void TCompTransCurveController::setRatio(float ratio)
{
  if (_loop) {
    // Wrap in the range 0..1
    ratio = fmodf(ratio, 1.0f);
    if (ratio < 0)
      ratio += 1;
  }
  else {
    // Keep ratio in valid range
    ratio = clamp(ratio, 0.f, 1.f);
  }
  _ratio = ratio;
  if (!_curve)
    return;

  // delta curve
  CTransform t = _curve->evaluate(_ratio);

  // Destination component
  TCompTransform* c_transform = get<TCompTransform>();
  assert(c_transform);

  // transfrom = base + delta
  c_transform->set(base.combineWith(t));
}

void TCompTransCurveController::renderDebug() {
  if (!_curve)
    return;
  TCompTransform* c_transform = get<TCompTransform>();
  assert(c_transform);
  _curve->renderDebug(base.asMatrix());
}
