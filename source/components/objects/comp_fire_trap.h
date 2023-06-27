#pragma once

#include "components/common/comp_base.h"
#include "entity/entity.h"
#include "entity/entity_parser.h"
#include "entity/common_msgs.h"
#include "components/common/physics/comp_rigid_body.h"
#include "components/common/comp_transform.h"
#include "modules/game/audio/audioEvent.h"

class TCompFireTrap : public TCompBase
{
  DECL_SIBLING_ACCESS();

public:
  void debugInMenu();
  void load(const json& j, TEntityParseContext& ctx);
  void update(float dt);

  static void registerMsgs();

private:
	bool _isEnabled = false;

  float _offset = 0.0f;
  float _fireTimer = 0.0f;
  float _fireDelay = 0.25f;
  float _enablerTimer = 0.0f;
  float _disablerTimer = 0.0f;
  float _enablerDelay = 2.0f;
  float _disablerDelay = 4.0f;
  float _fireDamage = 15.0f;
  AudioEvent audio;
  AudioEvent audioSlow;

	void enable();
	void disable();
  void onCreate(const TMsgEntityCreated& msg);
};

