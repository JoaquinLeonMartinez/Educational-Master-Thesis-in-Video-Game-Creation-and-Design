#pragma once

#include "components/common/comp_base.h"
#include "entity/entity.h"
#include "entity/common_msgs.h"
#include "components/common/physics/comp_collider.h"
#include "components/common/comp_transform.h"

class TCompMeleeTrigger : public TCompBase {


	DECL_SIBLING_ACCESS();
	void onCollision(const TMsgEntityTriggerEnter& msg);
  void onCollisionOut(const TMsgEntityTriggerExit& msg);
  void expansiveWave();
  void expansiveWaveJoints();


public:
	void update(float dt);
	void load(const json& j, TEntityParseContext& ctx);
	void renderDebug();
	void debugInMenu();
	static void registerMsgs();

private:
    std::vector<CHandle> _currentEnemies;
    std::vector<CHandle> _enemiesHit;
    bool before = false;
    bool _isEnabled = true;
    bool hitDisplay = false;
	  TMsgDamage _messageToTarget;
    EntityType targetType;
    std::string _audioOnHit = "event:/Character/Attacks/Melee_Hit";
};

