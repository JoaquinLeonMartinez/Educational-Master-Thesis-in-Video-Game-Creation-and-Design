#pragma once

#include "components/common/comp_base.h"
#include "entity/entity.h"
#include "entity/common_msgs.h"
#include "PxPhysicsAPI.h"



namespace physx {
  class PxRigidActor;
}

class TCompRigidBody: public TCompBase {
  bool is_enabled;
  float _ascensionGravity = -14.81f;
  float _descentGravity = -18.81f;
  float mass = 1.f;
  float time_on_air = 0.f;
  float jump_time_offset = 0.1f;
	float smokeTimer;
	float smokeTimerMax = 0.5;
	int smoke_counter = 0;

  float lossRatio = 4.5f;
  VEC3 impulse = VEC3();
  bool _usingUnscaledTime = false;

public:
  DECL_SIBLING_ACCESS();

  bool is_grounded;
  VEC3 ground_normal = VEC3();

  ~TCompRigidBody();

  void load(const json& j, TEntityParseContext& ctx);
  void update(float dt);
  void manageImpulseXZ(float delta);
  void jump(const VEC3 dir);
  void doubleJump(const VEC3 dir);
  void addForce(const VEC3 dir);
  void getGroundNormal(VEC3 &value);
  void enableGravity(bool gravity);
  const VEC3 getImpulse();

  void debugInMenu();
  void renderDebug();
  void onEntityCreated(const TMsgEntityCreated& msg);
  bool isGrounded();

  static void registerMsgs();

  void setUsingUnscaledTime(bool usingUnscaledTime);
};