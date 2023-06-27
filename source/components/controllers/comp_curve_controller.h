#pragma once

#include "components/common/comp_base.h"
#include "entity/entity.h"
#include "geometry/curve.h"

class TCompCurveController : public TCompBase
{
  DECL_SIBLING_ACCESS();

public:
  void debugInMenu();
  void load(const json& j, TEntityParseContext& ctx);
  void renderDebug();
  void setRatio(float ratio);
  const CCurve* getCurve() {
	  return _curve;
  }

private:
  void applyRatio();

  const CCurve* _curve = nullptr;
  float _speed = 10.f;
  float _ratio = 0.f;
  bool  _enabled = true;
  CHandle _target;
  std::string _targetName;
};

