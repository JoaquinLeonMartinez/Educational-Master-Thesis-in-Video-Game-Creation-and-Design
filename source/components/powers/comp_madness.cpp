#include "mcv_platform.h"
#include "entity/entity_parser.h"
#include "comp_madness.h"
#include "components/controllers/character/comp_character_controller.h"
using namespace physx;

DECL_OBJ_MANAGER("comp_madness", TCompMadnessController);

void TCompMadnessController::debugInMenu() {
  ImGui::DragFloat("Madness", &_remainingMadness, 0.10f, 0.f, _maximumMadness);
}

void TCompMadnessController::load(const json& j, TEntityParseContext& ctx) {
	//Spender Values
	_fireSpenderValue = j.value("_chilliSpenderValue", _fireSpenderValue);
	_teleportSpenderValue = j.value("_teleportSpenderValue", _teleportSpenderValue);
	_coffeeSpenderValue = j.value("_coffeeSpenderValue", _coffeeSpenderValue);
	_batterySpenderValue = j.value("_batterySpenderValue", _batterySpenderValue);
	//End Spender Values

	//Generator Values
	_meleeGeneratorValue = j.value("_meleeGeneratorValue", _meleeGeneratorValue);
	_fireGeneratorValue = j.value("_chilliGeneratorValue", _fireGeneratorValue);
	_teleportGeneratorValue = j.value("_teleportSpenderValue", _teleportGeneratorValue);
	_puddleGeneratorValue = j.value("_puddleGeneratorValue", _puddleGeneratorValue);
	_fountainGeneratorValue = j.value("_fountainGeneratorValue", _fountainGeneratorValue);
	//End Generator Values
	
	//Insert in maps
	_spenderValues.emplace(PowerType::FIRE, _fireSpenderValue);
  _spenderValues.emplace(PowerType::FIRECOMBO, _fireComboSpenderValue);
	_spenderValues.emplace(PowerType::TELEPORT, _teleportSpenderValue);
	_spenderValues.emplace(PowerType::COFFEE, _coffeeSpenderValue);
	_spenderValues.emplace(PowerType::BATTERY, _batterySpenderValue);

	_generatorValues.emplace(PowerType::MELEE, _meleeGeneratorValue);
  _generatorValues.emplace(PowerType::FIRE, _fireGeneratorValue);
  _generatorValues.emplace(PowerType::FIRECOMBO, _fireComboGeneratorValue);
	_generatorValues.emplace(PowerType::TELEPORT, _teleportGeneratorValue);
	_generatorValues.emplace(PowerType::PUDDLE, _puddleGeneratorValue);
	_generatorValues.emplace(PowerType::FOUNTAIN, _fountainGeneratorValue);

}

void TCompMadnessController::registerMsgs() {
	DECL_MSG(TCompMadnessController, TMsgOnCinematic, onCinematic);
	DECL_MSG(TCompMadnessController, TMsgOnCinematicSpecial, onCinematicSpecial);

}


void TCompMadnessController::onCinematic(const TMsgOnCinematic& msg) {
	if (msg.cinematic) {
		isCinematic = true;
	}
	else {
		isCinematic = false;
	}
}

void TCompMadnessController::onCinematicSpecial(const TMsgOnCinematicSpecial& msg) {
	if (msg.cinematic) {
		isCinematic = true;
	}
	else {
		isCinematic = false;
	}
}




float TCompMadnessController::getRemainingMadness() {
	return _remainingMadness;
}

float TCompMadnessController::getMaximumMadness() {
	return _maximumMadness;
}

void TCompMadnessController::setMaximumMadness(float new_maximum) {
	_maximumMadness = new_maximum;
}

float TCompMadnessController::getPowerGeneration(PowerType power) {
	return _generatorValues.at(power);
}

float TCompMadnessController::getPowerCost(PowerType power) {
	return _spenderValues.at(power);
}

//To be called when we try to use a power, true if we were able to use it
bool TCompMadnessController::spendMadness(PowerType power) {
	if (_remainingMadness >= _spenderValues.at(power)) {
		//Spend madness
		_remainingMadness -= _spenderValues.at(power);
		//Reset regeneration timer, we won't regenerate passively until it runs out
		_passiveRegenerationTimer = _passiveRegenerationDelay;
		//Reset loss timer, we won't lose passively until it runs out
		_passiveLossTimer = _passiveLossDelay;
		return true;
	}
	else {
		dbg("Not enough madness!\n");
    EngineAudio.playEvent("event:/Character/Other/No_Madness");
    return false;
	}
}

//Warning: Only for cases in which we need to multiply the power cost by the deltaTime
bool TCompMadnessController::spendMadness(float amount) {
	if (_remainingMadness >= amount) {
		//Spend madness
		_remainingMadness -= amount;
		//Reset regeneration timer, we won't regenerate passively until it runs out
		_passiveRegenerationTimer = _passiveRegenerationDelay;
		//Reset loss timer, we won't lose passively until it runs out
		_passiveLossTimer = _passiveLossDelay;
		return true;
	}
	else {
		dbg("Not enough madness!\n");
    EngineAudio.playEvent("event:/Character/Other/No_Madness");
    return false;
	}
}

//To be called when a power collides successfully
void TCompMadnessController::generateMadness(PowerType power) {
	if (_remainingMadness < _maximumMadness) {
		//Generate madness
		_remainingMadness += _generatorValues.at(power);
		//Reset loss timer, we won't lose passively until it runs out
		_passiveLossTimer = _passiveLossDelay;
		//If we have exceeded _maximumMadness, set _remainingMadness to _maximumMadness
		if (_remainingMadness > _maximumMadness) {
			_remainingMadness = _maximumMadness;
		}
	}
}

//Warning: Only for cases in which we need to multiply the power generation by the deltaTime
void TCompMadnessController::generateMadness(float amount) {
	if (_remainingMadness < _maximumMadness) {
		//Generate madness
		_remainingMadness += amount;
		//Reset loss timer, we won't lose passively until it runs out
		_passiveLossTimer = _passiveLossDelay;
		//If we have exceeded _maximumMadness, set _remainingMadness to _maximumMadness
		if (_remainingMadness > _maximumMadness) {
			_remainingMadness = _maximumMadness;
		}
	}
}

void TCompMadnessController::restoreMadness(){
	_remainingMadness = _maximumMadness;
}

void TCompMadnessController::update(float delta) {
	if(!isCinematic){
		if (_remainingMadness > _passiveThreshold) {
			//Above threshold logic
			if (_passiveLossTimer <= 0) {
				//if we haven't gained madness in _passiveLossDelay, start losing until we reach _passiveThreshold
				_remainingMadness -= _madnessPassiveLossRate * delta;
			}
		}
		else if(_remainingMadness < _passiveThreshold){
			//Below threshold logic
			if (_passiveRegenerationTimer <= 0) {
				//if we have less than _passiveThreshold, 
				//and we haven't used it in _passiveRegenerationDelay, start regenerating up to _passiveThreshold
				_remainingMadness += _madnessPassiveRestorationRate * delta;
      _remainingMadness = clamp(_remainingMadness, 0.0f, _passiveThreshold);
			}
		}

		//quitar esto TODO
		if (GameController.getGodMode()) {
			_remainingMadness = _maximumMadness;
		}
		//Update timers
		_passiveLossTimer -= delta;
		_passiveRegenerationTimer -= delta;
	}
}

void TCompMadnessController::renderDebug() {

}