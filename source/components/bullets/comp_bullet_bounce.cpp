#include "mcv_platform.h"
#include "comp_bullet_bounce.h"
#include "components/common/comp_transform.h"
#include "components/common/comp_tags.h"
#include "entity/entity_parser.h"
#include "engine.h"
#include "input/input.h"
#include "input/module_input.h"
#include "modules/module_physics.h"

using namespace physx;

DECL_OBJ_MANAGER("bullet_bounce_controller", TCompBulletBounceController);

void TCompBulletBounceController::debugInMenu() {
	ImGui::DragFloat("Speed", &speed, 0.1f, 0.0f, 500.0f);
	ImGui::DragFloat("Damage", &intensityDamage, 0.1f, 0.0f, 100.0f);
	ImGui::DragInt("Bounces", &bounces, 1, 1, 10);
}

void TCompBulletBounceController::load(const json& j, TEntityParseContext& ctx) {
	speed = j.value("speed", speed);
	intensityDamage = j.value("damage", intensityDamage);
	bounces = j.value("bounces", bounces);
}

void TCompBulletBounceController::registerMsgs() {
	DECL_MSG(TCompBulletBounceController, TMsgAssignBulletOwner, onBulletInfoMsg);
	DECL_MSG(TCompBulletBounceController, TMsgOnContact, onCollision);
}

void TCompBulletBounceController::onCollision(const TMsgOnContact& msg) {
	//tratar aqui el numero de rebotes
	//destruir aqui la bala LOGICA DE LAS BALAS
	CEntity* source_of_impact = (CEntity *)msg.source.getOwner();
	if (source_of_impact) {
		TCompTags* c_tag = source_of_impact->get<TCompTags>();
		if (c_tag) {
			std::string tag = CTagsManager::get().getTagName(c_tag->tags[0]);
			if (strcmp("player", tag.c_str()) == 0) { //HERE WE NEVER DO SOMETHING

			}
			else if (strcmp("floor", tag.c_str()) == 0) { //WE WILL DESTROY THE BULLET IF NEEDED ON THE FLOOR
			  //bounces = 0;
			}
			else if (strcmp("enemy", tag.c_str()) == 0) {
				// Notify the entity that he has been hit
				TMsgDamageToEnemy msg;
				msg.h_sender = h_sender;      // Who send this bullet
				msg.h_bullet = CHandle(this).getOwner(); // The bullet information
				msg.intensityDamage = intensityDamage;
				source_of_impact->sendMsg(msg);
				bounces--;

			}
			else if (strcmp("grenade", tag.c_str()) == 0) { //HERE WE NEVER DO SOMETHING
				bounces = 0;
			}
			else if (strcmp("wall", tag.c_str()) == 0) {
				bounces = 0;
			}
			else if (strcmp("scenario", tag.c_str()) == 0) {
				bounces--;
			}
			else if(strcmp("destroyable_box", tag.c_str()) == 0){
				bounces = 0;
				TMsgDamageToEnemy msg;
				msg.h_sender = h_sender;      // Who send this bullet
				msg.h_bullet = CHandle(this).getOwner(); // The bullet information
				msg.intensityDamage = intensityDamage;
				source_of_impact->sendMsg(msg);
			}
			
		}
    }
    
}

void TCompBulletBounceController::onBulletInfoMsg(const TMsgAssignBulletOwner& msg) {
	h_sender = msg.h_owner;

	TCompTransform* c_trans = get<TCompTransform>();
	c_trans->setPosition(msg.source);
	float yaw = vectorToYaw(msg.front);
	c_trans->setAngles(yaw, 0.f, 0.f);

	TCompCollider* c_collider = get<TCompCollider>();
	VEC3 new_pos = msg.front;
	PxVec3 velocidad = PxVec3(new_pos.x, new_pos.y, new_pos.z) * speed;
  
  physx::PxRigidDynamic* rigid_dynamic = static_cast<physx::PxRigidDynamic*>(c_collider->actor);
  rigid_dynamic->setRigidBodyFlag(PxRigidBodyFlag::eENABLE_CCD, true);
  //rigid_dynamic->addForce(velocidad, PxForceMode::eIMPULSE, false);
  rigid_dynamic->setLinearVelocity(velocidad);
}

void TCompBulletBounceController::update(float delta) {
	PROFILE_FUNCTION("BulletBounce");
	if (bounces <= 0) {
    CHandle(this).getOwner().destroy();
    CHandle(this).destroy();
	}
	else {
		if (limitTime > 0.f) {
			limitTime -= delta;
		} 
		else{
      CHandle(this).getOwner().destroy();
      CHandle(this).destroy();
		}
	}
}


