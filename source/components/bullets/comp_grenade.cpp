#include "mcv_platform.h"
#include "comp_grenade.h"
#include "components/common/comp_transform.h"
#include "components/common/comp_tags.h"
#include "entity/entity_parser.h"
#include "engine.h"
#include "input/input.h"
#include "input/module_input.h"
#include "modules/module_physics.h"

using namespace physx;

DECL_OBJ_MANAGER("grenade_controller", TCompGrenadeController);

void TCompGrenadeController::debugInMenu() {
	ImGui::DragFloat("explosion_radius_hit", &explosion_radius_hit, 0.1f, 0.0f, 100.0f);
	ImGui::DragFloat("intensityDamage", &intensityDamage, 0.1f, 0.0f, 100.0f);
	ImGui::DragFloat("Force", &force, 0.1f, 0.0f, 20.0f);
  ImGui::DragFloat("Height Force", &height_force, 0.1f, 0.0f, 20.0f);
  ImGui::DragFloat("mass", &mass, 0.1f, 0.0f, 500.0f);
}


void TCompGrenadeController::load(const json& j, TEntityParseContext& ctx) {
	explosion_radius_hit = j.value("explosion_radius", explosion_radius_hit);
	intensityDamage = j.value("intensityDamage", intensityDamage);
  force = j.value("force", force);
  height_force = j.value("height_force", height_force);
	mass = j.value("mass", mass);
}

void TCompGrenadeController::registerMsgs() {
	DECL_MSG(TCompGrenadeController, TMsgAssignBulletOwner, onGrenadeInfoMsg);
	DECL_MSG(TCompGrenadeController, TMsgOnContact, onCollision);
}

void TCompGrenadeController::onCollision(const TMsgOnContact& msg) {

	CEntity* source_of_impact = (CEntity *)msg.source.getOwner();
	TCompTags* c_tag = source_of_impact->get<TCompTags>();
	if (c_tag) {
		std::string tag = CTagsManager::get().getTagName(c_tag->tags[0]);
		if (strcmp("player", tag.c_str()) != 0) {
			flagExplota = true;
		}

	}

}

void TCompGrenadeController::onGrenadeInfoMsg(const TMsgAssignBulletOwner& msg) {
	h_sender = msg.h_owner;

  TCompTransform* c_trans = get<TCompTransform>();
	c_trans->setPosition(msg.source);
	float yaw = vectorToYaw(msg.front);
	c_trans->setAngles(yaw, 0.f, 0.f);

  TCompCollider* c_collider = get<TCompCollider>();
	VEC3 new_pos = c_trans->getFront();
	PxVec3 velocidad = PxVec3(new_pos.x, new_pos.y, new_pos.z);

	physx::PxRigidDynamic* rigid_dynamic = static_cast<physx::PxRigidDynamic*>(c_collider->actor);
	rigid_dynamic->setMass(mass);

	PxVec3 vel_ang = PxVec3(1, 0, 0);
	rigid_dynamic->setAngularVelocity(vel_ang);
	rigid_dynamic->addForce(velocidad * force, PxForceMode::eIMPULSE, false);
	PxVec3 velocidady = PxVec3(0, new_pos.y + height_force, 0);
	rigid_dynamic->addForce(velocidady, PxForceMode::eIMPULSE, false);
}

void TCompGrenadeController::update(float delta) {//cambiar la condicion un poco antes de la explosion para indicar que va a explotar
	PROFILE_FUNCTION("GrenadeBullet");
	if (flagExplota) {
		//RADIO DE EXPLOSION

		TCompTransform* c_trans = get<TCompTransform>();
		PxSphereGeometry geometry(explosion_radius_hit);
		PxVec3 pos = VEC3_TO_PXVEC3(c_trans->getPosition());
		PxQuat ori = QUAT_TO_PXQUAT(c_trans->getRotation());

    const PxU32 bufferSize = 256;
    PxOverlapHit hitBuffer[bufferSize];
    PxOverlapBuffer buf(hitBuffer, bufferSize);
		PxTransform shapePose = PxTransform(pos, ori);

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
            if (strcmp("enemy", tag.c_str()) == 0 ) {
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

		//METRALLA
		/*for (int i = 0; i < 10; i++) {
			TEntityParseContext ctx;
			VEC3 posicion = VEC3(c_trans->getPosition().x, c_trans->getPosition().y, c_trans->getPosition().z);
			c_trans->setPosition(posicion);
			ctx.root_transform = *c_trans;
			parseScene("data/prefabs/bullets/shrapnel.json", ctx); //cambiar prefab

			TMsgAssignBulletOwner msg;
			msg.h_owner = CHandle(this).getOwner();
			msg.source = c_trans->getPosition();
			msg.front = c_trans->getFront();
			ctx.entities_loaded[0].sendMsg(msg);
		}*/
    
    CHandle(this).getOwner().destroy();
    CHandle(this).destroy();
	}
}

void TCompGrenadeController::renderDebug() {
  TCompTransform* c_trans = get<TCompTransform>();
  VEC3 pos = c_trans->getPosition();

  drawCircle(pos, explosion_radius_hit, VEC4(1, 0, 0, 1));


}