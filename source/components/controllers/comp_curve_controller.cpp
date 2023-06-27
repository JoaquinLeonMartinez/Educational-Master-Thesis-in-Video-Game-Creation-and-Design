#include "mcv_platform.h"
#include "comp_curve_controller.h"
#include "components/common/comp_transform.h"
//#include "utils/utils.h"
//#include "engine.h"
//#include "input/input.h"
//#include "input/module_input.h"

DECL_OBJ_MANAGER("curve_controller", TCompCurveController);

void TCompCurveController::debugInMenu() {
  ImGui::DragFloat("Max Speed", &_speed, 0.1f, 1.f, 100.f);

  /*ImGui::DragFloat("Sensitivity", &_sensitivity, 0.001f, 0.001f, 0.1f);
  ImGui::DragFloat("Inertia", &_ispeed_reduction_factor, 0.001f, 0.7f, 1.f);
  ImGui::LabelText("Curr Speed", "%f", _ispeed);*/
}

void TCompCurveController::load(const json& j, TEntityParseContext& ctx)
{
  _curve = Resources.get(j.value("curve", ""))->as<CCurve>();
  _ratio = j.value("ratio", _ratio);
  _enabled = j.value("enabled", _enabled);
  _speed = j.value("speed", _speed);
  _targetName = j.value("target", "");
}

void TCompCurveController::setRatio(float ratio)
{
  _ratio = ratio;
  applyRatio();
}

void TCompCurveController::applyRatio()
{
  if (!_curve)
    return;

  if (!_target.isValid())
  {
    _target = getEntityByName(_targetName);

    if (!_target.isValid())
      return;
  }

  TCompTransform* c_transform = get<TCompTransform>();
  if (!c_transform)
    return;
  CEntity* eTarget = _target;
  TCompTransform* cTargetTransform = eTarget->get<TCompTransform>();
  if (!cTargetTransform)
    return;
  
  const VEC3 pos = VEC3::Transform(_curve->evaluate(_ratio), c_transform->asMatrix());
  cTargetTransform->setPosition(pos);
}

void TCompCurveController::renderDebug()
{
  if (_curve)
  {
    TCompTransform* c_transform = get<TCompTransform>();
    if (!c_transform)
      return;
    _curve->renderDebug(*c_transform);
  }
}


