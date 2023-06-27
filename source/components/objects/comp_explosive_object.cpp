#include "mcv_platform.h"
#include "comp_explosive_object.h"
#include "components/ai/others/self_destroy.h"
#include "components/common/comp_transform.h"
#include "engine.h"
#include "modules/game/module_game_controller.h"
#include "components/common/physics/comp_rigid_body.h"
#include "components/common/comp_render.h"
#include "components/common/physics/comp_collider.h"
#include "datatypes.h"

DECL_OBJ_MANAGER("comp_explosive_object", TCompExplosiveObject);


void TCompExplosiveObject::load(const json& j, TEntityParseContext& ctx) {
	_isEnabled = j.value("_isEnabled", _isEnabled);
	_explosionDelay = j.value("_explosionDelay", _explosionDelay);
	_fireAreaDuration = j.value("_fireAreaDuration", _fireAreaDuration);
	_explosionRadius = j.value("_explosionRadius", _explosionRadius);
	_explosionDamage = j.value("_explosionDamage", _explosionDamage);
	_explosionForce = j.value("_explosionForce", _explosionForce);
	
}

void TCompExplosiveObject::registerMsgs() {
	DECL_MSG(TCompExplosiveObject, TMsgDamage, onDamageInfoMsg);
	DECL_MSG(TCompExplosiveObject, TMsgEntityTriggerEnter, onTriggerEnter);
	DECL_MSG(TCompExplosiveObject, TMsgEntityTriggerExit, onTriggerExit);
}

void TCompExplosiveObject::onDamageInfoMsg(const TMsgDamage& msg) {
	if (_hasExploded)
		return;

	dbg("Recibe un impacto\n");
	if (msg.damageType == PowerType::FIRE) {
		//dbg("Explosive object receives CHILLI damage.\n");
		enable();
	}
	else {
		if (msg.senderType == ENVIRONMENT) { //si es otro componente explosivo o algo del entorno
			dbg("Explosive object receives other explosive obj explosion.\n");
			enable();
			_explosionTimer = _delayForExternalExplosion; //explota al momento
		}
	}
}

void TCompExplosiveObject::onTriggerEnter(const TMsgEntityTriggerEnter& msg) {
	if (_isFireEnabled) {
		TMsgFireAreaEnter enterFireMsg;
		TCompTransform* c_trans = get<TCompTransform>();
		enterFireMsg.areaCenter = c_trans->getPosition();
		((CEntity*)msg.h_entity)->sendMsg(enterFireMsg);
		candidates.push_back(msg.h_entity);
	}	
}

void TCompExplosiveObject::onTriggerExit(const TMsgEntityTriggerExit& msg) {
	if (_isFireEnabled) {
		TMsgFireAreaExit exitFireMsg;
		((CEntity*)msg.h_entity)->sendMsg(exitFireMsg);

		CEntity* candidateOut = msg.h_entity;
		for (int i = 0; i < candidates.size(); i++) { 
			if (candidates[i] == candidateOut) {
				candidates.erase(candidates.begin() + i);
			}
		}

	}
}

void TCompExplosiveObject::enable() {
	//dbg("Explosive object begins countdown.\n");
	_isEnabled = true;
}

void TCompExplosiveObject::explode() {
	//dbg("Explosive object explodes.\n");
	//Generate damage sphere
	TCompTransform* c_trans = get<TCompTransform>();
	TMsgDamage msg;
	msg.senderType = EntityType::ENVIRONMENT;
	msg.targetType = EntityType::ALL;
	msg.position = c_trans->getPosition();
	msg.intensityDamage = _explosionDamage;
	msg.impactForce = _explosionForce;
	GameController.generateDamageSphere(c_trans->getPosition(), _explosionRadius, msg, "VulnerableToExplosions");
	//Get rid of the object's collider (the shape)
	TCompCollider* c_col = get<TCompCollider>();
	static const physx::PxU32 max_shapes = 8;
	physx::PxShape* shapes[max_shapes];
	const physx::PxU32 bufferSize = 256;
	c_col->actor->getShapes(shapes, bufferSize);
	physx::PxShape* sh = shapes[0];
	//c_col->actor->detachShape(*sh); //comento para que no se vaya por debajo del suelo
	
	_isFireEnabled = true;
	//TCompRender* crender = get<TCompRender>();  //TODO: COMENTADO DE FORMA TEMPORAL
	//crender->showMeshesWithState(1);

	_hasExploded = true;
	GameController.spawnPrefab("data/prefabs/props/explosion_sphere.json", c_trans->getPosition());
}

void TCompExplosiveObject::update(float delta)
 {
	if (_isEnabled) {
		if (!_isFireEnabled) {
			//Explosion logic
			if (_explosionTimer <= 0) {
				explode();
			}
			else {
				_explosionTimer -= delta;
			}
		}
		else {
			//Fire area logic
			if (_fireAreaDuration <= 0) {
				dbg("Fire area expires.\n");
				_isFireEnabled = false;
				_isEnabled = false;
				TCompSelfDestroy* s_d = get <TCompSelfDestroy>();
				s_d->enable();

				//enviar mensaje a todos los que esten dentro para que se vuelvan pequeños
				TMsgFireAreaExit exitFireMsg;
				//candidates.erase(msg.h_entity);
				for (int i = 0; i < candidates.size(); i++) { 
					candidates[i]->sendMsg(exitFireMsg);
				}

			}
			else {
				_fireAreaDuration -= delta;
			}
		}
	}
}