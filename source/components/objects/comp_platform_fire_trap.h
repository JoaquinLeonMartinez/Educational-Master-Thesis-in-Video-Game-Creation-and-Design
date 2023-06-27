#pragma once

#include "components/common/comp_base.h"
#include "entity/entity.h"
#include "entity/entity_parser.h"
#include "entity/common_msgs.h"
#include "components/common/physics/comp_rigid_body.h"
#include "components/common/comp_transform.h"

class TCompFirePlatformTrap : public TCompBase
{
  DECL_SIBLING_ACCESS();

public:
  void debugInMenu();
 void load(const json& j, TEntityParseContext& ctx);
  void update(float dt);
  void setEnable(bool value);
  bool flagActivate = true;
  static void registerMsgs();

private:
	bool _isEnabled = false;

  float _offset = 0.0f;
  float _fireTimer = 0.0f;
  float _fireDelay = 0.1f;
  float _enablerTimer = 4.0f;
  float _disablerTimer = 0.0f;
  float _enablerDelay = 4.0f;
  float _disablerDelay = 3.0f;
  float _fireDamage = 5.0f;
  void onCreate(const TMsgEntityCreated& msg);
  void enter(const TMsgEntityTriggerEnter & msg);
  void exit(const TMsgEntityTriggerExit & msg);
  void enable();
  void disable();

  bool damageOn = false;


 
};

