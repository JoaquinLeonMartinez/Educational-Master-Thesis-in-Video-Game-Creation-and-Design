#pragma once

#include "components/common/comp_base.h"
#include "entity/entity.h"
#include "entity/common_msgs.h"
#include "modules/module_physics.h"
#include "modules/game/audio/audioEvent.h"

using namespace physx;

class TCompDynamicInstance : public TCompBase {
  int my_unique_idx = -1;
  physx::PxRigidDynamic* rigid_dynamic = nullptr;
  CHandle h_coll;

  void onPlayerAttack(const TMsgDamage& msg);
  void onCreate(const TMsgEntityCreated&);
  void onBattery(const TMsgGravity& msg);
  void onContact(const TMsgOnContact& msg);
  float life = 0.f;
  float lastSoundTimer = 0.f;
  float lastSoundDelay = 3.f;

public:
  void load(const json& j, TEntityParseContext& ctx);
  void debugInMenu();
  void renderDebug();
  void update(float delta);
  void set_idx(int idx) { my_unique_idx = idx; }

  static void registerMsgs();

  DECL_SIBLING_ACCESS();
};