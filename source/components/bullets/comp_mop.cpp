#include "mcv_platform.h"
#include "comp_mop.h"
#include "components/common/comp_transform.h"
#include "entity/entity_parser.h"
#include "engine.h"
#include "input/input.h"
#include "components/common/comp_tags.h"
#include "input/module_input.h"
#include "modules/module_physics.h"

using namespace physx;

DECL_OBJ_MANAGER("mop_controller", TCompMopController);

void TCompMopController::debugInMenu() {
	ImGui::InputFloat("intensityDamage", &intensityDamage);
}

void TCompMopController::load(const json& j, TEntityParseContext& ctx) {
	intensityDamage = j.value("intensityDamage", intensityDamage);
	max_distance_ray = j.value("max_distance_ray", max_distance_ray);
}

void TCompMopController::registerMsgs() {
	DECL_MSG(TCompMopController, TMsgAssignBulletOwner, onMopInfoMsg);
	DECL_MSG(TCompMopController, TMsgOnContact, onCollision);
}


void TCompMopController::onCollision(const TMsgOnContact& msg) {

	CEntity* source_of_impact = (CEntity *)msg.source.getOwner();
	TCompTags* c_tag = source_of_impact->get<TCompTags>();
	if (c_tag) {
		std::string tag = CTagsManager::get().getTagName(c_tag->tags[0]);
		if (strcmp("scenario", tag.c_str()) == 0) {
			flagColision = true;//cambiar a constantes segun lo que colisiona
		}
		//CUANDO TENGAMOS COLLIDER EN FREGONA Y LA FREGONA CAMBIAR POR ESTO LO QUE HAY EN EL UPDATE 
		/*if (strcmp("enemy", tag.c_str()) == 0) {
			TMsgDamageToEnemy msg;
			msg.h_sender = h_sender;      // Who send this bullet
			msg.h_bullet = CHandle(this).getOwner(); // The bullet information
			msg.intensityDamage = intensityDamage;
			CEntity* entityEnemyDamage = source_of_impact;
			entityEnemyDamage->sendMsg(msg);

		}
		else if (strcmp("destroyable_box", tag.c_str()) == 0) {
			TMsgDamageToEnemy msg;
			msg.h_sender = h_sender;      // Who send this bullet
			msg.h_bullet = CHandle(this); // The bullet information
			msg.intensityDamage = intensityDamage;
			CEntity* entityEnemyDamage = source_of_impact;
			entityEnemyDamage->sendMsg(msg);

		}*/



	}

}




void TCompMopController::onMopInfoMsg(const TMsgAssignBulletOwner& msg) {
	h_sender = msg.h_owner;

	TCompTransform* c_trans = get<TCompTransform>();
	c_trans->setPosition(msg.source);
	float yaw = vectorToYaw(msg.front);
	c_trans->setAngles(yaw, 0.f, 0.f);


	c_collider = get<TCompCollider>();
	VEC3 new_pos = c_trans->getFront();
	PxVec3 velocidad = PxVec3(new_pos.x, new_pos.y, new_pos.z);

	physx::PxRigidDynamic* rigid_dynamic = static_cast<physx::PxRigidDynamic*>(c_collider->actor);
	
}

void TCompMopController::update(float delta) {//cambiar la condicion un poco antes de la explosion para indicar que va a explotar
	PROFILE_FUNCTION("Mop");
	if (flagColision) {
		TCompTransform* c_trans = get<TCompTransform>();
		assert(c_trans);
		PxSphereGeometry geometry(radius_hit);
		PxVec3 pos = VEC3_TO_PXVEC3(c_trans->getPosition());
		PxQuat ori = QUAT_TO_PXQUAT(c_trans->getRotation());
		PxTransform shapePose = PxTransform(pos, ori);
		
		
		const PxU32 bufferSize = 256;
		PxOverlapHit hitBuffer[bufferSize];
		PxOverlapBuffer buf(hitBuffer, bufferSize);
	
		bool res = EnginePhysics.gScene->overlap(geometry, shapePose, buf);
		if (res) {
			for (PxU32 i = 0; i < buf.nbTouches; i++) {
				CHandle h_comp_physics;
				h_comp_physics.fromVoidPtr(buf.getAnyHit(i).actor->userData);
				CEntity* entityContact = h_comp_physics.getOwner();
				if (entityContact) {
					TCompTags* c_tag = entityContact->get<TCompTags>();
					if (c_tag) {
						std::string tag = CTagsManager::get().getTagName(c_tag->tags[0]);
						if (strcmp("enemy", tag.c_str()) == 0) {
							TMsgDamageToEnemy msg;
							msg.h_sender = h_sender;      // Who send this bullet
							msg.h_bullet = CHandle(this).getOwner(); // The bullet information
							msg.intensityDamage = intensityDamage;
							CEntity* entityEnemyDamage = entityContact;
							entityEnemyDamage->sendMsg(msg);
						}
						else if (strcmp("destroyable_box", tag.c_str()) == 0) {
							TMsgDamageToEnemy msg;
							msg.h_sender = h_sender;      // Who send this bullet
							msg.h_bullet = CHandle(this); // The bullet information
							msg.intensityDamage = intensityDamage;
							CEntity* entityEnemyDamage = entityContact;
							entityEnemyDamage->sendMsg(msg);
						}
					}
				}
			}
		}
		CHandle(this).getOwner().destroy();
	}
}
		
	

	
	
	 



