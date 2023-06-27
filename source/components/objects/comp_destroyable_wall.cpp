#include "mcv_platform.h"
#include "engine.h"
#include "comp_madness_puddle.h"
#include "components/objects/comp_destroyable_wall.h"
#include "components/common/comp_transform.h"
#include "components/controllers/character/comp_character_controller.h"
#include "modules/module_physics.h"
#include "modules/game/module_game_controller.h"
#include "engine.h"


using namespace physx;
DECL_OBJ_MANAGER("comp_destroyable_wall", TCompDestroyableWall);

void TCompDestroyableWall::debugInMenu() {
}

void TCompDestroyableWall::load(const json& j, TEntityParseContext& ctx) {
	typeWall = j.value("type_wall", typeWall);
	factor = j.value("factor", factor);
    iceAudio = EngineAudio.playEvent("event:/Music/Ambience_Props/Ice/Ice_Melting");
    iceAudio.setPaused(true);
}

void TCompDestroyableWall::registerMsgs() {
	DECL_MSG(TCompDestroyableWall, TMsgDamage, onPlayerAttack);
}

void TCompDestroyableWall::onPlayerAttack(const TMsgDamage & msg) {
	if (msg.damageType == FIRE && typeWall == 0) {//ice wall
		//generar animacion de descongelamiento
    TCompMorphAnimation* c_morph = get<TCompMorphAnimation>();
    if (c_morph) {
      c_morph->setIncrement(0.5f);
      c_morph->playMorph();
      if (!iceAudio.isPlaying()) {
          iceAudio.restart();
          TCompTransform* c_trans = get<TCompTransform>();
          iceAudio.set3DAttributes(*c_trans);
      }
        iceAudioTimer = iceAudioDelay;
    }
	}
	else if (typeWall == 1) { //destroyable wall
		if (msg.damageType == MELEE)  {
			TCompTransform* c_trans = get<TCompTransform>();
			VEC3 positionWall = c_trans->getPosition();
			CHandle(this).getOwner().destroy();
			CHandle(this).destroy();
			//aparicion de prefab de cajas rotas
			TEntityParseContext ctx;
			ctx.root_transform.setPosition(c_trans->getPosition());
			float yaw, pitch, roll;
			ctx.root_transform.getAngles(&yaw, &pitch, &roll);
			ctx.root_transform.setAngles(yaw, pitch, roll);
			parseScene("data/prefabs/structures/cajasRotas.json", ctx);

			

			CEntity* e_trozoCajac3 = getEntityByName("TrozoCaja3");
			VEC3 hit_pos3 = msg.position;
			VEC3 dir3 = c_trans->getPosition() - hit_pos3;
			dir3.Normalize();
			dir3 *= msg.impactForce;
			physx::PxVec3 impulse3 = VEC3_TO_PXVEC3(dir3);
			TCompCollider* c_coll3 = e_trozoCajac3->get<TCompCollider>();
			physx::PxRigidDynamic* rigid_dynamica3 = static_cast<physx::PxRigidDynamic*>(c_coll3->actor);
			rigid_dynamica3->addForce(impulse3, PxForceMode::eIMPULSE, false);

			CEntity* e_trozoCajac2 = getEntityByName("TrozoCaja2");
			VEC3 hit_pos2 = msg.position;
			VEC3 dir2 = c_trans->getPosition() - hit_pos2;
			dir2.Normalize();
			dir2 *= msg.impactForce * factor;
			physx::PxVec3 impulse2 = VEC3_TO_PXVEC3(dir2);
			TCompCollider* c_coll2 = e_trozoCajac2->get<TCompCollider>();
			physx::PxRigidDynamic* rigid_dynamica2 = static_cast<physx::PxRigidDynamic*>(c_coll2->actor);
			rigid_dynamica2->addForce(impulse2, PxForceMode::eIMPULSE, false);

			CEntity* e_trozoCajac1 = getEntityByName("TrozoCaja1");
			VEC3 hit_pos1 = msg.position;
			VEC3 dir1 = c_trans->getPosition() - hit_pos1;
			dir1.Normalize();
			dir1 *= msg.impactForce * factor2;
			physx::PxVec3 impulse1 = VEC3_TO_PXVEC3(dir1);
			TCompCollider* c_coll1 = e_trozoCajac1->get<TCompCollider>();
			physx::PxRigidDynamic* rigid_dynamica1 = static_cast<physx::PxRigidDynamic*>(c_coll1->actor);
			rigid_dynamica1->addForce(impulse1, PxForceMode::eIMPULSE, false);



			//ANTIGUO
			//aplicar fuerza a las a las para enviarlas hacia delante
			/*CHandle h_player = GameController.getPlayerHandle();
			CEntity* e_player = (CEntity*)h_player;
			TCompTransform* player_trans = e_player->get< TCompTransform>();
			physx::PxVec3 velocidad = physx::PxVec3(player_trans->getFront().x, player_trans->getFront().y, player_trans->getFront().z);
			VEC3 front_player = c_trans->getFront();
			
			//trozo arriba
			CEntity* e_trozoCajac = getEntityByName("TrozoCaja3");
			TCompCollider* c_collc = e_trozoCajac->get<TCompCollider>();
			physx::PxRigidDynamic* rigid_dynamicc = static_cast<physx::PxRigidDynamic*>(c_collc->actor);

			float impulso_yc = msg.impactForce;
			float impulso_zc = msg.impactForce;
			rigid_dynamicc->addForce(velocidad * impulso_zc, PxForceMode::eIMPULSE, false);
			PxVec3 velocidadyc = PxVec3(0, sender_trans->getPosition().y + impulso_yc, 0);
			rigid_dynamicc->addForce(velocidadyc, PxForceMode::eIMPULSE, false);

			//trozo medio
			CEntity* e_trozoCajab = getEntityByName("TrozoCaja2");
			TCompCollider* c_collb = e_trozoCajab->get<TCompCollider>();
			physx::PxRigidDynamic* rigid_dynamicb = static_cast<physx::PxRigidDynamic*>(c_collb->actor);

			float impulso_yb = msg.impactForce * 2;
			float impulso_zb = msg.impactForce *2;
			rigid_dynamicb->addForce(velocidad * impulso_zb, PxForceMode::eIMPULSE, false);
			PxVec3 velocidadyb = PxVec3(0, sender_trans->getPosition().y + impulso_yb, 0);
			rigid_dynamicb->addForce(velocidadyb, PxForceMode::eIMPULSE, false);

			//trozo abajo
			CEntity* e_trozoCajaa = getEntityByName("TrozoCaja1");
			TCompCollider* c_colla = e_trozoCajaa->get<TCompCollider>();
			physx::PxRigidDynamic* rigid_dynamica = static_cast<physx::PxRigidDynamic*>(c_colla->actor);
			float impulso_ya = msg.impactForce * 3;
			float impulso_za = msg.impactForce * 3;
			rigid_dynamica->addForce(velocidad * impulso_za, PxForceMode::eIMPULSE, false);
			PxVec3 velocidadya = PxVec3(0, player_trans->getPosition().y + impulso_ya, 0);
			rigid_dynamica->addForce(velocidadya, PxForceMode::eIMPULSE, false);*/
		}
	}
	else if (typeWall == 2){//Normal Wall
		if (msg.damageType == MELEE) {

			/*
			TCompTransform* c_trans = get<TCompTransform>();
			VEC3 positionWall = c_trans->getPosition();
			CHandle h_player = GameController.getPlayerHandle();
			CEntity* e_player = (CEntity*)h_player;
			TCompTransform* player_trans = e_player->get< TCompTransform>();
			
			physx::PxVec3 velocidad = physx::PxVec3(player_trans->getFront().x, player_trans->getFront().y, player_trans->getFront().z);
			TCompCollider* c_coll = get<TCompCollider>();
			physx::PxRigidDynamic* rigid_dynamica = static_cast<physx::PxRigidDynamic*>(c_coll->actor);
			float impulso_z = n;
			rigid_dynamica->addForce(velocidad * impulso_z, PxForceMode::eIMPULSE, false);
			*/
			TCompTransform* c_trans = get<TCompTransform>();
			VEC3 hit_pos = msg.position;
			VEC3 dir = c_trans->getPosition() - hit_pos;
			dir.Normalize();
			dir *= msg.impactForce;

			physx::PxVec3 impulse = VEC3_TO_PXVEC3(dir);
			TCompCollider* c_coll = get<TCompCollider>();
			physx::PxRigidDynamic* rigid_dynamica = static_cast<physx::PxRigidDynamic*>(c_coll->actor);
			rigid_dynamica->addForce(impulse, PxForceMode::eIMPULSE, false);


		}
	}
	else if (typeWall == 3) { //Time ice wall 
		if (msg.damageType == FIRE) {
			//esto debe durar X segundos, o tener un limite de vida
			timer_ice_wall -= msg.intensityDamage;
            last_fire_hit = 0.0f;
            TCompMorphAnimation* c_morph = get<TCompMorphAnimation>();
            if (c_morph) {
                c_morph->setIncrement(0.05f);
                c_morph->playMorph();
            }
            if (iceAudio.getPaused()) {
                iceAudio.setPaused(false);
                TCompTransform* c_trans = get<TCompTransform>();
                iceAudio.set3DAttributes(*c_trans);
            }
            iceAudioTimer = iceAudioDelay;
		}
	}
}

void TCompDestroyableWall::update(float dt) {
  if (typeWall != 3)
    return;
  iceAudioTimer -= dt;
  if (iceAudioTimer <= 0) {
      iceAudio.setPaused(true);
  }
  last_fire_hit += dt;
  if (last_fire_hit > max_last_fire_hit) {
    TCompMorphAnimation* c_morph = get<TCompMorphAnimation>();
    if (c_morph) {
      float actualWeight = c_morph->getWeight();
      if (actualWeight < 0.58f) {
        c_morph->stopMorph();
      }
      else if (!enemies_in_tube_deleted) {
        c_morph->setIncrement(0.5f);
        enemies_in_tube_deleted = true;
        CHandle h = GameController.entityByName("enemies_in_tube");
        if (h.isValid()) {
          CEntity* enemies_in_tube = ((CEntity*)h);
          TCompEnemiesInTube* comp = enemies_in_tube->get<TCompEnemiesInTube>();
          if (comp != nullptr) {//nos aseguramos de que es el del puzzle
            TMSgWallDestroyed msg;
            msg.h_entity = h;
            msg.isDetroyed = true;
            enemies_in_tube->sendMsg(msg);
          }
        }
      }
    }
  }
}

