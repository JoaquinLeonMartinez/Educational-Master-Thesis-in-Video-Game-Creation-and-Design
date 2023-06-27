#include "mcv_platform.h"
#include "components/common/comp_transform.h"
#include "components/common/comp_name.h"
#include "engine.h"
#include "comp_trigger_damage_zone.h"


DECL_OBJ_MANAGER("trigger_damage_zone", TCompTriggerDamageZone);

void TCompTriggerDamageZone::debugInMenu() {
}

void TCompTriggerDamageZone::load(const json& j, TEntityParseContext& ctx) {

	damage = j.value("damage", damage);
	damageInterval = j.value("damageInterval", damageInterval);
	active = j.value("active", active);
}


void TCompTriggerDamageZone::registerMsgs()
{
	DECL_MSG(TCompTriggerDamageZone, TMsgEntityTriggerEnter, onEnter);
	DECL_MSG(TCompTriggerDamageZone, TMsgEntityTriggerExit, onExit);
	DECL_MSG(TCompTriggerDamageZone, TMsgSetActive, onActiveMsg);
}

void  TCompTriggerDamageZone::onActiveMsg(const TMsgSetActive & msg) {
	//active = msg.active;
}


void TCompTriggerDamageZone::onExit(const TMsgEntityTriggerExit & msg) {

	/*CEntity* candidateOut = msg.h_entity;
	//candidates.erase(msg.h_entity);
	for (int i = 0; i < candidates.size(); i++) { //debe de haber alguna forma de hacerlo sin el bucle
		if (candidates[i] == candidateOut) {
			candidates.erase(candidates.begin() + i);
		}
	}*/
	//dbg("el candidate sale de la zona de lava \n");
}

void TCompTriggerDamageZone::onEnter(const TMsgEntityTriggerEnter & msg)
{
	//dbg("el candidate entra en la zona de lava \n");
	//eliminar la entity del vector de entities que haya dentro
	//candidates.push_back(msg.h_entity);
	
	
	/*CEntity* entity = msg.h_entity;
	TMsgDamage msgDamage;
	msgDamage.intensityDamage = damage;
	entity->sendMsg(msgDamage);*/
	CEntity* entity = msg.h_entity;
	TMSgTriggerFalloutDead msgTriggerFalloutDead;
	msgTriggerFalloutDead.damage = damage;
	msgTriggerFalloutDead.h_entity = entity;
	msgTriggerFalloutDead.falloutDead = true;
	entity->sendMsg(msgTriggerFalloutDead);
}

void TCompTriggerDamageZone::update(float dt)
{
	/*if (!candidates.empty() & active) { //si hay alguien le envia damage
		if (damageTimer >= damageInterval) {
			damageTimer = 0;
			TMsgDamageToAll msgDamage;
			msgDamage.intensityDamage = damage;
			for (int i = 0; i < candidates.size(); i++) {
				candidates[i]->sendMsg(msgDamage);
			}
		}
		else {
			damageTimer += dt;
		}
	}*/

}
