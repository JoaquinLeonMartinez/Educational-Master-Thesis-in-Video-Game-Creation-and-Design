#include "mcv_platform.h"
#include "entity/entity_parser.h"
#include "engine.h"
#include "comp_coffee.h"
#include "components/controllers/character/comp_character_controller.h"
#include "components/controllers/camera/comp_camera_3rd_person.h"
#include "components/controllers/camera/comp_camera_3rd_person_aim.h"
#include "components/common/physics/comp_rigid_body.h"
#include "components/powers/comp_madness.h"

using namespace physx;

DECL_OBJ_MANAGER("comp_coffee", TCompCoffeeController);

void TCompCoffeeController::debugInMenu() {

}

void TCompCoffeeController::load(const json& j, TEntityParseContext& ctx) {

}

void TCompCoffeeController::registerMsgs() {
}

void TCompCoffeeController::onCollision(const TMsgOnContact& msg) {
}

void TCompCoffeeController::onBatteryInfoMsg(const TMsgAssignBulletOwner& msg) {

}

void TCompCoffeeController::setScales() {
	GameController.setTimeScale(1.0f - (1.f - _worldTimeScaleModifier) * proportion);
	TCompCharacterController* c_cc = get<TCompCharacterController>();
	c_cc->setSpeed(c_cc->getBaseSpeed() * (1.f + (_playerSpeedModifier - 1.f) * proportion));
	TCompRigidBody* c_rb = get<TCompRigidBody>();

	if (proportion > 0.f) {
	c_rb->setUsingUnscaledTime(true);
	}
	else {
	c_rb->setUsingUnscaledTime(false);
	}
}

void TCompCoffeeController::switchState() {
	if (_isEnabled) {
    _isEnabled = false;
    _disabling = true;
    _enabling = false;
    audioEffect.stop();
	}
	else {
    _isEnabled = true;
    _disabling = false;
    _enabling = true;
    audioEffect = EngineAudio.playEvent("event:/Character/Powers/Coffee/Coffee");
	}
}

void TCompCoffeeController::update(float delta) {
  //dbg("--------------\n");
	//If it's enabled, try to spend madness, if we can't, disable
	TCompCharacterController* c_cc = get<TCompCharacterController>();
	if(c_cc != nullptr) {
		delta = Time.delta_unscaled;
		if (_enabling) {
			time_switching += delta;
			if (time_switching > time_to_enable_disable)
				time_switching = time_to_enable_disable;
			//proportion = clamp(0.f, 1.f, time_switching / time_to_enable);
			//dbg("enabling %f\n", time_switching);
			proportion = Interpolator::quadInOut(0.f, 1.f, time_switching / time_to_enable_disable);

			setScales();
		}
		else if (_disabling) {
			time_switching -= delta;
		if (time_switching < 0.f)
			time_switching = 0.f;
		//proportion = clamp(0.f, 1.f, time_switching / time_to_enable_disable);
		//dbg("desabling %f\n", time_switching);
		proportion = Interpolator::quadInOut(0.f, 1.f, time_switching / time_to_enable_disable);

		setScales();
		}

		ctes_shared.CoffeeRatio = proportion;

		if (_isEnabled) {
			TCompMadnessController* m_c = get<TCompMadnessController>();
			if (!m_c->spendMadness(m_c->getPowerCost(PowerType::COFFEE) * delta) && !GameController.getGodMode()) {
				_isEnabled = false;
				_disabling = true;
				_enabling = false;
				audioEffect.stop();
			}
		}
	}
}


bool TCompCoffeeController::getIsEnabled() {
	return _isEnabled;
}

void TCompCoffeeController::renderDebug() {

}