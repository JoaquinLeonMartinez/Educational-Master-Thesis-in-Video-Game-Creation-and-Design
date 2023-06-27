#include "mcv_platform.h"
#include "engine.h"
#include "comp_platform_fire_trap.h"
#include "components/common/comp_transform.h"
#include "components/common/comp_render.h"
#include "components/vfx/comp_air.h"
#include "components/ai/others/self_destroy.h"
#include "random"
/*
std::mt19937 bt_off(1993);
std::uniform_int_distribution<int> bt_range4(0, 100);
*/
DECL_OBJ_MANAGER("comp_fire_platform_trap", TCompFirePlatformTrap);

void TCompFirePlatformTrap::debugInMenu() {
  ImGui::DragFloat("Offset", &_offset, 0.0f, 5.0f);
}

void TCompFirePlatformTrap::load(const json& j, TEntityParseContext& ctx) {
  //_offset = bt_range4(bt_off) / 10.0f;
}

void TCompFirePlatformTrap::registerMsgs() {
  DECL_MSG(TCompFirePlatformTrap, TMsgEntityCreated, onCreate);
  DECL_MSG(TCompFirePlatformTrap, TMsgEntityTriggerEnter, enter);
  DECL_MSG(TCompFirePlatformTrap, TMsgEntityTriggerExit, exit);
}

void TCompFirePlatformTrap::onCreate(const TMsgEntityCreated& msg) {
	
	
}


void TCompFirePlatformTrap::enter(const TMsgEntityTriggerEnter & msg) {
	if (flagActivate){
		if (msg.h_entity == GameController.getPlayerHandle()) {
			damageOn = true;
		}
	}
}

void TCompFirePlatformTrap::exit(const TMsgEntityTriggerExit & msg) {
	if (msg.h_entity == GameController.getPlayerHandle()) {
		damageOn = false;
	}
}



void TCompFirePlatformTrap::enable() {
  TCompBuffers* c_buff = get<TCompBuffers>();
  if (c_buff) {
    auto buf = c_buff->getCteByName("TCtesParticles");
    CCteBuffer<TCtesParticles>* data = dynamic_cast<CCteBuffer<TCtesParticles>*>(buf);
    data->emitter_num_particles_per_spawn = 100;
    data->updateGPU();
  }
}

void TCompFirePlatformTrap::disable() {
  TCompBuffers* c_buff = get<TCompBuffers>();
  if (c_buff) {
    auto buf = c_buff->getCteByName("TCtesParticles");
    CCteBuffer<TCtesParticles>* data = dynamic_cast<CCteBuffer<TCtesParticles>*>(buf);
    data->emitter_num_particles_per_spawn = 0;
    data->updateGPU();
  }
}

void TCompFirePlatformTrap::update(float dt) {
  if (_offset > 0.0f) {
    _offset -= dt;
    return;
  }
  
  
  if (_isEnabled) {
    if (_fireTimer <= 0.f) {
      _fireTimer = _fireDelay;
    }
    else {
      _fireTimer -= dt;
    }

    _disablerTimer -= dt;
    if (_disablerTimer <= 0.f) {
      _isEnabled = false;
	   if(!_isEnabled){
		   disable();
		  _disablerTimer = _disablerDelay;
	   }
    }
  }
  else {
	  if(!flagActivate){
		_enablerTimer -= dt;
		if (_enablerTimer <= 0.f) {
		  _isEnabled = true;
		  //enable();
		  _enablerTimer = _enablerDelay;
		  damageOn = false;

		}
	  }
  }

  if (damageOn) {
	  TCompBuffers* c_buff = get<TCompBuffers>();
	  auto buf = c_buff->getCteByName("TCtesParticles");
	  CCteBuffer<TCtesParticles>* data = dynamic_cast<CCteBuffer<TCtesParticles>*>(buf);
	  VEC3 _targetDirection = data->emitter_dir;
	  CHandle player = GameController.getPlayerHandle();
	  CEntity* player_e = (CEntity*)player;
	  if (player_e != nullptr) {
		  TMsgDamage msgDamage;
		  msgDamage.h_bullet = CHandle(this).getOwner();
		  msgDamage.bullet_front = _targetDirection;
		  msgDamage.damageType = PowerType::FIRE;
		  msgDamage.senderType = EntityType::ENEMIES;
		  msgDamage.targetType = EntityType::PLAYER;
		  msgDamage.intensityDamage = _fireDamage;
		  msgDamage.impactForce = 0.f;
		  player_e->sendMsg(msgDamage);
	  }
  }

}


void TCompFirePlatformTrap::setEnable(bool value) {
	_isEnabled = value;
}
