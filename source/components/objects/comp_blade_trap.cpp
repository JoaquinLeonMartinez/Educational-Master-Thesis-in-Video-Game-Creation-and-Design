#include "mcv_platform.h"
#include "engine.h"
#include "comp_blade_trap.h"
#include "components/common/comp_transform.h"
#include "components/common/comp_render.h"
#include "modules/module_physics.h"
#include "components/common/comp_name.h"
using namespace physx;

DECL_OBJ_MANAGER("comp_blade_trap", TCompBladeTrap);

void TCompBladeTrap::debugInMenu() {
}

void TCompBladeTrap::load(const json& j, TEntityParseContext& ctx) {
	damage = j.value("damage", damage);
	impact = j.value("impact", impact);

}

void TCompBladeTrap::registerMsgs() {
	DECL_MSG(TCompBladeTrap, TMsgEntityTriggerEnter, enable);
	DECL_MSG(TCompBladeTrap, TMsgEntityTriggerExit, disable);
	DECL_MSG(TCompBladeTrap, TMsgOnContact, onContact);
	DECL_MSG(TCompBladeTrap, TMsgEntityCreated, onCreate);
}

void TCompBladeTrap::onCreate(const TMsgEntityCreated & msg) {
	TCompRender* c_render = get<TCompRender>();
	c_render->is_visible = true;
	c_render->updateRenderManager();
	TCompCollider* c_col = get<TCompCollider>();
	physx::PxRigidDynamic* rigid_dynamic = static_cast<physx::PxRigidDynamic*>(c_col->actor);
	c_trans = get<TCompTransform>();
	physx::PxBoxGeometry box;
	physx::PxShape* colShape;
	rigid_dynamic->getShapes(&colShape,1, 0);
	colShape->getBoxGeometry(box);
	VEC3 creationWindOffset = VEC3(c_trans->getPosition().x, (c_trans->getPosition().y - (box.halfExtents.y)), c_trans->getPosition().z - (box.halfExtents.z));
	//c_col->setGlobalPose(creationWindOffset, QUAT());
	
}

void TCompBladeTrap::disable(const TMsgEntityTriggerExit & msg) {
	/*if (msg.h_entity == GameController.getPlayerHandle()) {
		_isEnabled = false;
		dbg("Enemy spawner disabled.\n");
	}*/
}


void TCompBladeTrap::enable(const TMsgEntityTriggerEnter & msg) {
	/*if (msg.h_entity == GameController.getPlayerHandle()) {
		_isEnabled = true;
		player = msg.h_entity;
		dbg("Enemy spawner enabled.\n");
		damage = true;
	}*/

}


void TCompBladeTrap::onContact(const TMsgOnContact & msg) {
 	player = GameController.getPlayerHandle();
	if (!player.isValid()) {
		return;
	}
	/*
	TCompTransform* c_trans = get<TCompTransform>();
	TMsgDamage msg_;
	msg_.senderType = EntityType::ENEMIES;
	msg_.targetType = EntityType::ALL;
	msg_.position = c_trans->getPosition();
	msg_.intensityDamage = 10;
	msg_.impactForce = 10;
	player.sendMsg(msg_);
	*/
	TCompName* cname = get<TCompName>();
	PxSphereGeometry geometry(9.0);
	Vector3 damageOrigin = c_trans->getPosition();
	physx::PxFilterData pxFilterData;
	pxFilterData.word0 = EnginePhysics.Player;
	physx::PxQueryFilterData PxPlayerFilterData;
	PxPlayerFilterData.data = pxFilterData;
	PxPlayerFilterData.flags = physx::PxQueryFlag::eDYNAMIC;
	std::vector<physx::PxOverlapHit> hits;

	if (EnginePhysics.Overlap(geometry, damageOrigin, hits, PxPlayerFilterData)) {
		for (int i = 0; i < hits.size(); i++) {
			CHandle hitCollider;
			hitCollider.fromVoidPtr(hits[i].actor->userData);
			if (hitCollider.isValid()) {
				CHandle player_h = hitCollider.getOwner();
				if (player == player_h) {
					_isEnabled = true;
					//Send damage mesage
					CEntity* entityContact = hitCollider.getOwner();
					TCompTransform* c_trans = get<TCompTransform>();
					TMsgDamage msg_;
					msg_.senderType = EntityType::ENVIRONMENT;
					msg_.targetType = EntityType::PLAYER;
					msg_.position = c_trans->getPosition();
					msg_.intensityDamage = damage;
					msg_.impactForce = impact;
					msg_.h_bullet = CHandle(this).getOwner();
					entityContact->sendMsg(msg_);
				}
			}
		}
	}

	
}


void TCompBladeTrap::update(float dt) {
	/*if (!player.isValid())
		return;
	TMsgDamage msg;
	CEntity* player_e = (CEntity*)player;
	if (_isEnabled) {
		c_trans = get<TCompTransform>();
		VEC3 force = c_trans->getFront();
		force *= impact ;
		
		
		msg.senderType = EntityType::ENVIRONMENT;
		msg.targetType = EntityType::ALL;
		msg.position = c_trans->getPosition();
		msg.impactForce = impact * dt;
		msg.intensityDamage =0.f;
		player_e->sendMsg(msg);
	}
	if (damage == true) {
		msg.impactForce = impact * dt;
		msg.intensityDamage = 10.f;
		player_e->sendMsg(msg);
	}
	damage = false;*/
}


/*
template <typename T>
void TCompBladeTrap::send_DamageMessage(CEntity* entity, float dmg) {
	//Send damage mesage
	T msg;
	// Who sent this bullet
	msg.h_sender = h_sender;
	msg.h_bullet = CHandle(this).getOwner();
	msg.intensityDamage = dmg;
	msg.senderType = ENEMIES;
	msg.targetType = PLAYER;
	msg.impactForce = impactForceAttack;
	entity->sendMsg(msg);
}
*/
