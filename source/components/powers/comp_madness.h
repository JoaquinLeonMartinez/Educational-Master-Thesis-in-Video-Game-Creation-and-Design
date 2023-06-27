#pragma once

#include <map>
#include "components/common/comp_base.h"
#include "entity/entity.h"
#include "entity/common_msgs.h"
#include "components/common/physics/comp_collider.h"
#include "components/common/comp_transform.h"
#include "engine.h"

class TCompMadnessController : public TCompBase {


	DECL_SIBLING_ACCESS();

public:
	void update(float dt);
	void load(const json& j, TEntityParseContext& ctx);
	void renderDebug();
	void debugInMenu();
	static void registerMsgs();

	//enum PowerType { MELEE = 0, CHILLI, COFFEE, TELEPORT, BATTERY, PUDDLE, FOUNTAIN};

	void generateMadness(float amount);
	void generateMadness(PowerType power);
	bool spendMadness(float amount);
	bool spendMadness(PowerType power);

	float getRemainingMadness();
	float getMaximumMadness();
	float getPowerCost(PowerType power);
	float getPowerGeneration(PowerType power);
	void setMaximumMadness(float new_maximum);

	void restoreMadness();



private:
	//General Values
	float _maximumMadness = 100.f;
	float _remainingMadness = 0.f;
	float _madnessPassiveRestorationRate = 3.5f;
	float _madnessPassiveLossRate = 1.f;
	float _passiveThreshold = 20.f;
	std::map<PowerType, float> _generatorValues;
	std::map<PowerType, float> _spenderValues;
	//End General Values

	//Generator & Spender Default Values
	float _meleeSpenderValue = 0.f;
	float _meleeGeneratorValue = 10.f;

	float _fireSpenderValue = 12.f;
	float _fireGeneratorValue = 20.f;

  float _fireComboSpenderValue = 15.f;
  float _fireComboGeneratorValue = 0.f;

	float _teleportSpenderValue = 20.f;
	float _teleportGeneratorValue = 25.f;

	float _coffeeSpenderValue = 10.f;

	float _batterySpenderValue = 40.f;

	float _puddleGeneratorValue = 15.f;

	float _fountainGeneratorValue = 45.f;
	//End Generator & Spender Default Values

	//Time
	//Time we have to wait after using madness until we start regenerating it
	float _passiveRegenerationDelay = 2.f;
	float _passiveRegenerationTimer = _passiveRegenerationDelay;
	//Time that will have to pass after gaining madness until we start losing it
	float _passiveLossDelay = 5.f;
	float _passiveLossTimer = _passiveLossDelay;
	//End Time

	void onCinematic(const TMsgOnCinematic& msg);
	void onCinematicSpecial(const TMsgOnCinematicSpecial& msg);
	bool isCinematic = false;
};

