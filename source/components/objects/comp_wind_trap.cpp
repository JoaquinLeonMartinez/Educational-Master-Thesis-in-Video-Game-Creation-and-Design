#include "mcv_platform.h"
#include "engine.h"
#include "comp_wind_trap.h"
#include "components/common/comp_transform.h"
#include "components/common/comp_render.h"
#include "components/vfx/comp_air.h"
#include "components/ai/others/self_destroy.h"
#include "components/controllers/character/comp_character_controller.h"

DECL_OBJ_MANAGER("comp_wind_trap", TCompWindTrap);

void TCompWindTrap::debugInMenu() {
  ImGui::DragFloat("Wind Frequency: ", &_windDelay, 0.01f, 0.f, 1.f);
  ImGui::DragFloat("Scale Variation: ", &_scaleVar, 0.1f, 0.1f, 2.f);
  ImGui::DragFloat("Speed Variation: ", &_speedVar, 0.1f, 0.1f, 3.f);
  ImGui::DragFloat("Min Destroy Time: ", &_startDestroy, 0.1f, 0.1f, 5.f);
  ImGui::DragFloat("Destroy Variation: ", &_destroyVar, 0.1f, 0.1f, 2.f);
  ImGui::DragFloat("Wind Visibility: ", &windLength, 0.1f, 0.1f, 3.f);
  ImGui::DragFloat("Wind Decay: ", &windDist, 0.1f, 0.1f, 2.f);
}

void TCompWindTrap::load(const json& j, TEntityParseContext& ctx) {
}

void TCompWindTrap::registerMsgs() {
	DECL_MSG(TCompWindTrap, TMsgEntityTriggerEnter, enable);
	DECL_MSG(TCompWindTrap, TMsgEntityTriggerExit, disable);
	DECL_MSG(TCompWindTrap, TMsgGravity, onBattery);
	DECL_MSG(TCompWindTrap, TMsgEntityCreated, onCreate);
}

void TCompWindTrap::onCreate(const TMsgEntityCreated & msg) {
	TCompCollider* c_col = get<TCompCollider>();
	physx::PxRigidDynamic* rigid_dynamic = static_cast<physx::PxRigidDynamic*>(c_col->actor);
	c_trans = get<TCompTransform>();
	physx::PxBoxGeometry box;
	physx::PxShape* colShape;
	rigid_dynamic->getShapes(&colShape,1, 0);
	colShape->getBoxGeometry(box);
	VEC3 creationWindOffset = VEC3(c_trans->getPosition().x, (c_trans->getPosition().y - (box.halfExtents.y)), c_trans->getPosition().z - (box.halfExtents.z));
	//c_col->setGlobalPose(creationWindOffset, QUAT());
	audio = EngineAudio.playEvent("event:/Enemies/Hazards/Wind_Turbine/Wind_Turbine_Loop");
    audio.set3DAttributes(*c_trans);
}

void TCompWindTrap::enable(const TMsgEntityTriggerEnter & msg) {
	if (msg.h_entity == GameController.getPlayerHandle() && !is_destroyed) {
		_isEnabled = true;
		player = msg.h_entity;
		CEntity* player_e = (CEntity*)player;
		TCompCharacterController* comp_ch= player_e->get<TCompCharacterController>();
		comp_ch->setDashSpeed(5.f);
		timmerDeactivateComodin = 1.0f;
	}
}

void TCompWindTrap::disable(const TMsgEntityTriggerExit & msg) {
	if (msg.h_entity == GameController.getPlayerHandle() && !is_destroyed) {
		_isEnabled = false;
		player = msg.h_entity;
		CEntity* player_e = (CEntity*)player;
		TCompCharacterController* comp_ch = player_e->get<TCompCharacterController>();
		comp_ch->setDashSpeed(35.f);
	}
}

void TCompWindTrap::onBattery(const TMsgGravity & msg) {
	_isEnabled = false;
  is_destroyed = true;
  audio.stop();
  audio = EngineAudio.playEvent("event:/Enemies/Hazards/Wind_Turbine/Wind_Turbine_Shutdown");
  audio.set3DAttributes(*c_trans);
	//Animate or start particle system, do something
	/*CHandle(this).getOwner().destroy();
	CHandle(this).destroy();*/
}

void TCompWindTrap::update(float dt) {
  if (!player.isValid()) {
    player = GameController.getPlayerHandle();
    return;
  }
  if (is_destroyed)
    return;
		
	if (_isEnabled) {
		c_trans = get<TCompTransform>();
		VEC3 force = c_trans->getFront();
		force *= windForce * dt;
		CEntity* player_e = (CEntity*)player;
		TMsgTrapWind msg;
		msg.senderType = EntityType::ENVIRONMENT;
		msg.targetType = EntityType::ALL;
		msg.position = c_trans->getPosition();
		msg.intensityDamage = 0.f;
		msg.impactForce = windForce * dt;
		player_e->sendMsg(msg);
        if (!pushingAudio.isPlaying()) {
            pushingAudio = EngineAudio.playEvent("event:/Enemies/Hazards/Wind_Turbine/Wind_Turbine_Push");
            TCompTransform* c_trans = get<TCompTransform>();
            pushingAudio.set3DAttributes(*c_trans);
        }
	}
	//Minor Fix por si falla el exit del trigger
	if (timmerDeactivateComodin <= 0) {
		if (_isEnabled) {
			CEntity* player_e = (CEntity*)player;
			if (player_e != nullptr)  {
				TCompCharacterController* comp_ch = player_e->get<TCompCharacterController>();
				comp_ch->setDashSpeed(35.f);
				_isEnabled = false;
				timmerDeactivateComodin = 1.0f;
			}
		}
	}
	timmerDeactivateComodin -= dt;
	//...hasta aqui
  generateWind(dt);
}

void TCompWindTrap::generateWind(float dt) {
  if (!activateWind)
    return;
  _windCooldownTimer += dt;
  if (_windCooldownTimer >= _windDelay) {
    _windCooldownTimer = 0.0f;
    
    TCompTransform* c_trans = get<TCompTransform>();
    TEntityParseContext ctx;
    ctx.root_transform = *c_trans;
    //change scale randomly
    ctx.root_transform.setScale( 1.0f + randomFloat(-_scaleVar, _scaleVar));
    //change pitch a bit randomly
    float yaw, pitch;
    ctx.root_transform.getAngles(&yaw, &pitch);
    float rollOffset = randomFloat(-_rollVar, _rollVar);
    ctx.root_transform.setAngles(yaw, pitch, rollOffset);
    //change position a bit randomly
    VEC3 offset = VEC3(randomFloat(-_radius, _radius), randomFloat(0.5f - _radius, 0.5f + _radius), randomFloat(-_radius, _radius));
    ctx.root_transform.setPosition(ctx.root_transform.getPosition() + offset);
    
    parseScene("data/prefabs/vfx/air.json", ctx);

    //change speed a bit randomly
    CEntity* e = ctx.entities_loaded[0];
    TCompAir* c_a = e->get<TCompAir>();
    c_a->speed += randomFloat(-_speedVar, _speedVar);
    float timeToDestroy = randomFloat(_startDestroy, _startDestroy + _destroyVar);
    c_a->destroy = timeToDestroy;
    c_a->len = windLength;
    c_a->d = windDist;
    TCompSelfDestroy* c_sd = e->get<TCompSelfDestroy>();
    c_sd->setDelay(timeToDestroy);
    c_sd->enable();
  }
}