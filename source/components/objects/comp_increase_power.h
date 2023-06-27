#pragma once

#include "components/common/comp_base.h"
#include "entity/entity.h"
#include "entity/entity_parser.h"
#include "entity/common_msgs.h"

class TCompIncreasePower : public TCompBase
{
  DECL_SIBLING_ACCESS();

public:
  void debugInMenu();
  void load(const json& j, TEntityParseContext& ctx);
  void update(float dt);

  PowerUpType type = PowerUpType::HEALTH_UP;
  float chunk = 50.0f; //cantidad que se incrementara

  static void registerMsgs();

private:
	bool destroy = false;
	bool _isEnabled = false;
	void enable(const TMsgEntityTriggerEnter & msg);
};

