#pragma once

#include "components/common/comp_base.h"
#include "entity/entity.h"

class TCompCameraFlyover : public TCompBase
{
  DECL_SIBLING_ACCESS();

public:
  void debugInMenu();
  void load(const json& j, TEntityParseContext& ctx);
  void update(float dt);
  bool  _enabled = false;

private:
  float _speed = 10.f;
  float _sensitivity = 0.005f;
  float _maxPitch = (float)M_PI_2 - 1e-4f;
  VEC3  _ispeed;
  float _ispeed_reduction_factor = 0.95f;
  int   _key_toggle_enabled = 0;

};

