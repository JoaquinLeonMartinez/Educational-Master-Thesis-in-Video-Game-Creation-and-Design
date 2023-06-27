#pragma once

#include "components/common/comp_base.h"
#include "entity/entity.h"
#include "geometry/curve.h"

class TCompCamera3rdPersonAim : public TCompBase
{
  DECL_SIBLING_ACCESS();

public:
  void debugInMenu();
  void load(const json& j, TEntityParseContext& ctx);
  void update(float dt);
  void renderDebug();
  bool _enabled = true;
  bool mouse_active = true;
  void setTimeScale(float timeScale);
  float getTimeScale();

private:
  const CCurve* _curve = nullptr;
  CHandle _target;
  VEC3 _targetOffset;
  VEC3 _posOffset;
  std::string _targetName;
  float _ratio = 0.f;
  float _pitchSensitivity = 1.f;
  float _yawSensitivity = 1.f;
  float _timeScale = 1.0f;
  CCurve* curve_dynamic = nullptr;

  float _yaw = 0.f;
  MAT44 _curveTransform;

  float startEndTime = 1.0f;
  float timeMovingCamera = 0.f;
  float threshold = 0.9f;
  float smoothSpeed = 10.f;
  float interpolation = 0.f;

  void treatInput(float &yaw_rotation, float &pitch_rotation);
  void shouldSwapCamera();
  bool isPlayerMoving();
  void treatCollisionWithScenario(VEC3 &newPos, VEC3 &targetPos);
};

