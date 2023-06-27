#pragma once

#include "components/common/comp_base.h"
#include "entity/entity.h"
#include "geometry/curve.h"

struct TMsgEntityCreated;

class TCompTransCurveController : public TCompBase
{
  DECL_SIBLING_ACCESS();

public:
  void debugInMenu();
  void load(const json& j, TEntityParseContext& ctx);
  void renderDebug();

  void setRatio(float ratio);
  void update(float delta);

  void onEntityCreated( const TMsgEntityCreated& msg );
  static void registerMsgs();

private:

  CTransform        base;
  const CTransCurve* _curve = nullptr;
  float _speed = 1.f;
  float _ratio = 0.f;
  bool  _enabled = true;
  bool  _loop = true;
  bool  _base_defined = false;
};

