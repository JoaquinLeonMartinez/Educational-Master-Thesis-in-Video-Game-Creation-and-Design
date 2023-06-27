#pragma once

#include "components/common/comp_base.h"
#include "entity/entity.h"
#include "entity/common_msgs.h"
#include "components/common/physics/comp_collider.h"
#include "components/common/comp_transform.h"
#include "modules/game/audio/audioEvent.h"

class TCompCoffeeController : public TCompBase {


	DECL_SIBLING_ACCESS();
	void onBatteryInfoMsg(const TMsgAssignBulletOwner& msg);
	void onCollision(const TMsgOnContact& msg);

public:
	void update(float dt);
	void load(const json& j, TEntityParseContext& ctx);
	void renderDebug();
	void debugInMenu();
	static void registerMsgs();

	//Coffee Public Functions
	void setScales();
	void switchState();
	bool getIsEnabled();
	//End Coffee Public Functions

private:
	bool _isEnabled = false;
	float _worldTimeScaleModifier = 0.2f;
	float _playerTimeScaleModifier = 6.f;
	float _playerSpeedModifier = 1.2f;

  bool _enabling = false;
  bool _disabling = false;

  float proportion = 0.f;
  float time_to_enable_disable = 1.f;
  float time_switching = 0.f;

  AudioEvent audioEffect;
};

