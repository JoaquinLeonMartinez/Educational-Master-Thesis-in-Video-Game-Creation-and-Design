#include "mcv_platform.h"
#include "comp_battery.h"
#include "components/common/comp_transform.h"
#include "components/common/comp_name.h"
#include "entity/entity_parser.h"
#include "engine.h"
#include "input/input.h"
#include "components/common/comp_tags.h"
#include "components/common/physics/comp_collider.h"
#include "input/module_input.h"
#include "modules/module_physics.h"
#include "components/controllers/comp_parabolic_launch.h"
#include "components/ai/others/self_destroy.h"
#include "components/vfx/comp_bolt_sphere.h"
#include "components/vfx/comp_bolt_billboard.h"
#include "components/vfx/comp_bolt_aura_billboard.h"
#include "components/common/comp_dynamic_instance.h"

using namespace physx;

DECL_OBJ_MANAGER("battery_controller", TCompBatteryController);
PxVec3 vector = PxVec3(1, 2, 3);
void TCompBatteryController::debugInMenu() {
    ImGui::DragFloat("Force", &force, 0.1f, 0.0f, 20.0f);
    ImGui::DragFloat("Height Force", &height_force, 0.1f, 0.0f, 20.0f);
    ImGui::DragFloat("Time effect", &timeEffect, 0.1f, 0.0f, 20.0f);
}

void TCompBatteryController::load(const json& j, TEntityParseContext& ctx) {
    battery_radius_hit = j.value("battery_radius_hit", battery_radius_hit);
    mass = j.value("mass", mass);
    timeEffect = j.value("timeEffect", timeEffect);
    force = j.value("force", force);
    height_force = j.value("height_force", height_force);
    atractionForce = j.value("atraction_force", height_force);
    pulseDelay = j.value("pulseDelay", pulseDelay);
    maxEnemiesAffected = j.value("maxEnemiesAffected", maxEnemiesAffected);
}

void TCompBatteryController::registerMsgs() {
    DECL_MSG(TCompBatteryController, TMsgAssignBulletOwner, onBatteryInfoMsg);
    DECL_MSG(TCompBatteryController, TMsgOnContact, onCollision);
    DECL_MSG(TCompBatteryController, TMsgDamage, onBatteryFire);
}

void TCompBatteryController::onBatteryFire(const TMsgDamage & msg) {
    //Creacion de fuego y dano a enemigos que esten aqui
    if (msg.damageType == FIRE) {
        batteryFire = true; //esto se enviara en el mensaje que le envia al enemy
        //spawn here only once an explosion prefab with pulseTimer left of destruction
    }

}

void TCompBatteryController::onCollision(const TMsgOnContact& msg) {

    CEntity* source_of_impact = (CEntity *)msg.source.getOwner();
    if (source_of_impact) {
        PxShape* colShape;
        TCompCollider* c_collider = get<TCompCollider>();
        if (c_collider->actor == nullptr) return;
        c_collider->actor->getShapes(&colShape, 1, 0);
        PxFilterData col_filter_data = colShape->getSimulationFilterData();



        if (col_filter_data.word0 & EnginePhysics.Scenario && isKinematic) {
            isKinematic = false;
            physx::PxRigidDynamic* rigid_dynamic = static_cast<physx::PxRigidDynamic*>(c_collider->actor);	
        }
      
    }
}

void TCompBatteryController::onBatteryInfoMsg(const TMsgAssignBulletOwner& msg) {

    h_sender = msg.h_owner;
    c_trans = get<TCompTransform>();

    TCompCollider* c_collider = get<TCompCollider>();
    physx::PxRigidDynamic* rigid_dynamic = static_cast<physx::PxRigidDynamic*>(c_collider->actor);
	
	rigid_dynamic->setRigidBodyFlag(PxRigidBodyFlag::eENABLE_CCD, true);

	PxShape* colShape;
	c_collider->actor->getShapes(&colShape, 1, 0);

	PxMaterial* material;
	colShape->getMaterials(&material, 1, 0);
	material->setRestitution(0.2f);

    //VEC3 new_pos = c_trans->getFront();
    CEntity* e_camera = getEntityByName("AimCurve");
    TCompParabolicLaunch* parabolicLaunch = e_camera->get<TCompParabolicLaunch>();
    CCurve* curve_dynamic = parabolicLaunch->curve_dynamic;
    _knots = curve_dynamic->getKnots();
    nextPoint = _knots[i];
    c_trans->setPosition(nextPoint);

}

void TCompBatteryController::update(float delta) {
    PROFILE_FUNCTION("BatteryBullet");
    TCompTransform* p_trans = ((CEntity*)(GameController.getPlayerHandle()))->get<TCompTransform>();
    audioEffect.set3DAttributes(*p_trans);

    if (isKinematic) { //FOLLOW CURVE
        TCompTransform* c_trans = get<TCompTransform>();
        TCompCollider* c_col = get<TCompCollider>();
        physx::PxRigidDynamic* rigid_dynamic = static_cast<physx::PxRigidDynamic*>(c_col->actor);

        VEC3 currentPosition = c_trans->getPosition();
        VEC3 direccion = nextPoint - currentPosition;
        direccion.Normalize();
        Vector3 nextPos = currentPosition + direccion * velocityCurve * delta;
        //dbg("NextPos: %f,%f,%f\n", nextPos.x, nextPos.y, nextPos.z);
        PxVec3 pos = VEC3_TO_PXVEC3(nextPos);
        const PxTransform tr(pos);
        rigid_dynamic->setGlobalPose(tr, false);
        //rigid_dynamic->setKinematicTarget(tr);
        float dist = Vector3::Distance(nextPoint, c_trans->getPosition());
        if (dist < 1.5f) {
            i++;
            if (i < _knots.size()) {
                nextPoint = _knots[i];
            }
        }

		    if (limitTime > 0.f) {
			    limitTime -= delta;
		    }
		    else {
			    TMsgBatteryDeactivates msg;
			    ((CEntity*)GameController.getPlayerHandle())->sendMsg(msg);
			    audioEffect.stop();
			    CHandle(this).getOwner().destroy();
			    CHandle(this).destroy();
		    }
    }
    else { //LA PILA HA COLISIONADO
      TCompCollider* c_col = get<TCompCollider>();
      physx::PxRigidDynamic* rigid_dynamic = static_cast<physx::PxRigidDynamic*>(c_col->actor);
      
			//---------------------
			/*if (firstCollision) {
				firstCollision = false;
				TCompTransform* c_trans = get<TCompTransform>();
				c_trans->setScale(1.0);
				VEC3 position = c_trans->getPosition();
				position.y = position.y + 1.0f;
				GameController.spawnPrefab("data/prefabs/vfx/boom.json", position);
				AudioEvent audio = EngineAudio.playEvent("event:/Enemies/Hazards/Explosion");
				audio.set3DAttributes(*c_trans);
			}*/
			//---------------------


      VEC3 angular_speed = PXVEC3_TO_VEC3(rigid_dynamic->getAngularVelocity());
      VEC3 linear_speed = PXVEC3_TO_VEC3(rigid_dynamic->getLinearVelocity());


      if (angular_speed.Length() < 0.4f && linear_speed.Length() < 0.4f) {
        if (!startedEffect) {
          audioEffect = EngineAudio.playEvent("event:/Character/Powers/Battery/Battery");
          //spawn here the bolt sphere
          TEntityParseContext ctx;
          TCompTransform* c_trans = get<TCompTransform>();
          ctx.root_transform = *c_trans;
          parseScene("data/prefabs/vfx/bolt_sphere.json", ctx);
          CEntity* curr_e = ctx.current_entity;
          if (curr_e) {
            TCompSelfDestroy* c_sdes = curr_e->get<TCompSelfDestroy>();
            c_sdes->setDelay(limitTime);
          }
          //spawn a bolt from the sphere to this
          parseScene("data/prefabs/vfx/bolt.json", ctx);
          CEntity* curr_e2 = ctx.current_entity;
          if (curr_e2) {
            TCompSelfDestroy* c_sdes2 = curr_e2->get<TCompSelfDestroy>();
            c_sdes2->setDelay(limitTime);
            TCompBoltBillboard* c_bbill = curr_e2->get<TCompBoltBillboard>();
            c_bbill->setTargetPosition(curr_e);
            CHandle h(this);
            c_bbill->setTargetAim(h.getOwner());
          }

          CEntity* onom_manager = getEntityByName("Onomatopoeia Particles");
          TMsgOnomPet msgonom;
          msgonom.type = 0.0f;
          TCompTransform* c_trans2 = get<TCompTransform>();
          msgonom.pos = c_trans2->getPosition();
          onom_manager->sendMsg(msgonom);


          TEntityParseContext ctx2;
          ctx2.root_transform = *c_trans;
          parseScene("data/prefabs/vfx/electric_aura.json", ctx2);
          CEntity* curr_e3 = ctx2.current_entity;
          if (curr_e3) {
            TCompSelfDestroy* c_sdes2 = curr_e3->get<TCompSelfDestroy>();
            c_sdes2->setDelay(limitTime);
            TCompBoltAuraBillboard* c_bbill = curr_e3->get<TCompBoltAuraBillboard>();
            c_bbill->setTargetPosition(curr_e);
            CHandle h(this);
          }

          parseScene("data/particles/spark_particles.json", ctx2);
          CEntity* curr_e4 = ctx2.current_entity;
          if (curr_e4) {
            TCompSelfDestroy* c_sdes2 = curr_e4->get<TCompSelfDestroy>();
            c_sdes2->setDelay(limitTime);
          }
        }
        startedEffect = true;

        if (pulseTimer <= 0.f) {
          //Remove non valid enemies from the array
          std::list<CHandle>::iterator i = enemiesCatchBattery.begin();
          while (i != enemiesCatchBattery.end())
          {
            if (!(*i).isValid())
            {
              enemiesCatchBattery.erase(i++);  // alternatively, i = items.erase(i);
            }
            else
            {
              ++i;
            }
          }
          //RADIO DE GRAVEDAD
          TCompTransform* c_trans = get<TCompTransform>();
          PxSphereGeometry geometry(battery_radius_hit);
          Vector3 pos = c_trans->getPosition() + (Vector3().Up * gravityOriginHeight);
          PxQuat ori = QUAT_TO_PXQUAT(c_trans->getRotation());

          const PxU32 bufferSize = 256;
          PxOverlapHit hitBuffer[bufferSize];
          PxOverlapBuffer buf(hitBuffer, bufferSize);
          PxTransform shapePose = PxTransform(VEC3_TO_PXVEC3(pos), ori);

          PxQueryFilterData filter_data = PxQueryFilterData();
          if (enemiesCatchBattery.size() < maxEnemiesAffected) {
            filter_data.data.word0 = EnginePhysics.Enemy;
          }
          filter_data.data.word0 = filter_data.data.word0 | EnginePhysics.Product;

          bool res = EnginePhysics.gScene->overlap(geometry, shapePose, buf, filter_data);
          if (res) {
            for (PxU32 i = 0; i < buf.nbTouches; i++) {
              CHandle h_comp_physics;
              h_comp_physics.fromVoidPtr(buf.getAnyHit(i).actor->userData);
              CEntity* entityContact = h_comp_physics.getOwner();
              if (entityContact) {
                TMsgGravity msg;
                msg.h_sender = h_sender;      // Who send this bullet
                msg.h_bullet = CHandle(this).getOwner(); // The bullet information
                msg.position = PXVEC3_TO_VEC3(pos);
                msg.time_effect = limitTime;
                msg.distance = distance;
                msg.attractionForce = atractionForce;
                CEntity* entityEnemyDamage = entityContact;
                //dbg("Entity Enemy %s\n", entityEnemyDamage->getName());
                TCompDynamicInstance* c_di = entityEnemyDamage->get<TCompDynamicInstance>();
                if (!c_di && enemiesCatchBattery.size() < maxEnemiesAffected) {//si es un enemigo y puedo afectarle
                  enemiesCatchBattery.push_back(h_comp_physics);
                  //spawn here a bolt from this to the enemy
                  //spawn a bolt from the sphere to this
                    bool found = false;
                    std::list<CHandle>::iterator it = enemiesBolt.begin();
                    while (it != enemiesBolt.end())
                    {
                      if ((*it) == h_comp_physics) {
                        found = true;
                        break;
                      }
                      ++it;
                    }
                    PxShape* colShape;
                    buf.getAnyHit(i).actor->getShapes(&colShape, 1, 0);
                    PxFilterData col_filter_data = colShape->getSimulationFilterData();

                    if (!found) {
                      TEntityParseContext ctx;
                      parseScene("data/prefabs/vfx/bolt.json", ctx);
                      CEntity* curr_e2 = ctx.current_entity;
                      if (entityContact) {
                        TCompSelfDestroy* c_sdes2 = curr_e2->get<TCompSelfDestroy>();
                        c_sdes2->setDelay(limitTime);
                        TCompBoltBillboard* c_bbill = curr_e2->get<TCompBoltBillboard>();
                        CEntity* e_bolt = getEntityByName("BoltSphere");
                        c_bbill->setTargetPosition(e_bolt);
                        c_bbill->setTargetAim(entityContact);
                      }
                      enemiesBolt.push_back(h_comp_physics);
                    }
										TCompTransform* c_transEnemy = entityEnemyDamage->get<TCompTransform>();
										if (max_distance_enemy > Vector3::Distance(c_transEnemy->getPosition(), c_trans->getPosition())) { 
											//si la ditancia al enemigo es mayor del limite no le envimos nada
											entityEnemyDamage->sendMsg(msg);
										}
                }
                else if (c_di) {
                  entityEnemyDamage->sendMsg(msg);
                }

                

                //TODO: si fire esta activado enviar mensaje de damage cada x tiempo
                if (batteryFire) {
                  if (fireTime <= 0) {
                    fireTime = fireTimeMax;
                    TMsgDamageToAll msgDamage;
                    //msgDamage.intensityDamage = fireDamage;
                    //entityEnemyDamage->sendMsg(msgDamage);
                    for (auto&& enemy : enemiesCatchBattery) {
                      msgDamage.intensityDamage = fireDamage;
                      CEntity* enemyCaught = enemy;
                      enemyCaught->sendMsg(msgDamage);
                      //dbg("Entity Enemy %s\n", enemy->getName());
                    }
                    //dbg("Entity Enemy %s\n",entityEnemyDamage->getName());
                    //dbg("damage de fuego desde la pila enviado al enemigo \n");
                  }
                  else {
                    fireTime -= delta;
                  }
                }

              }
            }
            if (timeEffect > 0) {
              timeEffect -= delta;
            }
            else {
              timeEffect = 0;
              batteryFire = false;
            }
          }
          pulseTimer = pulseDelay;
        }
        else {
          pulseTimer -= delta;
        }

        //DESTROY WHEN IT HAS ENDED
		
        if (limitTime > 0.f) {
          limitTime -= delta;
        }
        else {
          TMsgBatteryDeactivates msg;
          ((CEntity*)GameController.getPlayerHandle())->sendMsg(msg);
          audioEffect.stop();
          CHandle(this).getOwner().destroy();
          CHandle(this).destroy();
        }
      }
    }
}

void TCompBatteryController::renderDebug() {
    TCompTransform* c_trans = get<TCompTransform>();
    Vector3 front = c_trans->getFront();
    Vector3 pos = c_trans->getPosition();
    if (timeEffect > 0 && flagColisiona) {
        TCompTransform* c_trans = get<TCompTransform>();
        Vector3 gravityOrigin = c_trans->getPosition() + (Vector3().Up * gravityOriginHeight);
        drawWiredSphere(gravityOrigin, battery_radius_hit, VEC4(1, 0, 0, 1));
    }
}

void TCompBatteryController::shoot(VEC3 front) {
    TEntityParseContext ctx;
    CHandle h_player = getEntityByName("Player");
    CEntity* e_player = (CEntity *)h_player;
    TCompTransform* c_trans = e_player->get<TCompTransform>();
    ctx.root_transform = *c_trans;
    parseScene("data/prefabs/bullets/battery.json", ctx);
    TMsgAssignBulletOwner msg;
    msg.h_owner = CHandle(this).getOwner();
    msg.source = c_trans->getPosition();
    msg.front = front;
    ctx.entities_loaded[0].sendMsg(msg);
}