#pragma once

#include "components/common/comp_base.h"
#include "entity/entity.h"
#include "geometry/curve.h"
#include "entity/common_msgs.h"
class TCompTransform;

class TCompCamera3rdPerson : public TCompBase
{
  DECL_SIBLING_ACCESS();

public:
  void debugInMenu();
  void load(const json& j, TEntityParseContext& ctx);
  void update(float dt);
  void renderDebug();
  bool _enabled = true;
  bool mouse_active = true;
  void resetCamera();

  static void registerMsgs();
private:
  const CCurve* _curve = nullptr;
  const CCurve* _curveIdle = nullptr;
  CHandle _target;
  CHandle lastGroundHandle;
  VEC3 _posOffset;
  std::string _targetName;
  bool aiming;
  float aimTransitionTime = 0.5f;
  float actualAimTransitionTime = 0.f;
  VEC3 aimOffset = VEC3(0, 0, 1.0);
  VEC3 pulsePos = VEC3();
  VEC3 lastPos = VEC3();
  bool lastPosWasCollision = false;
  float actualCameraHeight = false;
  bool first_reset = false;

  float distance = 10.f;
  float _ratio = 0.f;
  float _yaw = 0.f;
  float yawOffset = 0.f;
  float ratioOffset = 0.f;

  float timeNoAction = 0.f;
  float maxNoAction = 1.5f;
  float pulseRatio = 0.f;

  float transitionIdleMove = 0.f;
  float maxTransitionIdleMove = 2.5f;
  float maxReturnTransitionIdleMove = 0.2f;

  MAT44 _curveTransform;
  MAT44 _curvePulseTransform;

  float smoothSpeed = 8.f;
  float interpolation = 0.f;

  float lastHeight = 0.f;

  bool isPlayerMoving();
  bool isCameraRotating();
  void treatInput(float &yaw_rotation, float &pitch_rotation);
  void shouldSwapCamera();
  bool putPlayerOnScreen(VEC3 &newPos);
  float cameraMovementOnJump();

  void onPauseCam(const TMsgGamePause & msg);
  bool isPause = false;
};

