#include "mcv_platform.h"
#include "engine.h"
#include "checkpoint.h"
#include "components/common/comp_tags.h"
#include "components/common/comp_name.h"
#include "components/common/comp_transform.h"
#include "components/actions/comp_checkpoint_register.h"
#include "components/ai/bt/bt_cupcake.h"
#include "components/ai/bt/bt_sushi.h"
#include "components/ai/bt/bt_ranged_sushi.h"
#include "components/ai/bt/bt_golem.h"
#include "components/ai/others/comp_blackboard.h"
#include "components/controllers/comp_inventory.h"
#include "entity/common_msgs.h"

CCheckpoint::CCheckpoint() {
    saved = false;
    uint32_t tag_id = getID("checkpoint_registered");
    CTagsManager::get().registerTagName(tag_id, "checkpoint_registered");
}

/*

This function clears the previous checkpoint's data and stores the status of those entities marked to be registered.

*/
bool CCheckpoint::saveCheckPoint(VEC3 playerPos, QUAT playerRotation)
{
    PROFILE_FUNCTION("CCheckpoint::saveCheckPoint");
    dbg("Saving...\n");

    /* Clear previous data */
    bool error = false;
    enemies.clear();
    entities.clear();

    /* Save Player status */
    playerStatus.playerPos = playerPos;
    playerStatus.playerRot = playerRotation;
    playerStatus.saved = true;

	
	CEntity* e_player = EngineEntities.getPlayerHandle();
	CEntity* e_inventory = getEntityByName("Inventory");
	TCompInventory* inventory = e_inventory->get<TCompInventory>();
	playerStatus.battery = inventory->getBattery();
	playerStatus.chilli = inventory->getChilli();
	playerStatus.coffe = inventory->getCoffe();
	playerStatus.teleport = inventory->getTeleport();

	//TCompCharacterController* compCharacterController = e_player->get<TCompCharacterController>();
	//playerStatus.power_selected = compCharacterController->power_selected;


    /* Save Registered Entities' status */
    //VHandles allEntities = getAllEntities();
    //Get all entities with the 'checkpoint_registered' tag. More info in comp_checkpoint_register.
    VHandles allEntities = CTagsManager::get().getAllEntitiesByTag(getID("checkpoint_registered"));
    for (const auto& entity : allEntities) {
        CEntity* e_entity = (CEntity*)entity;
        TCompCheckpointRegister* comp_registry = e_entity->get<TCompCheckpointRegister>();
        //If the entity has the tag but doesn't have the component, we have assigned the tag manually
        //This should not happen, since we would not be storing the prefab's address
        if (!comp_registry) {
            fatal("Do NOT use 'checkpoint_registered' tag manually.\nAdd 'comp_checkpoint_register' component to the entity.");
        }

        //Make sure the entity has what it needs in order to be saved
        EntityStatus status;
        TCompName* comp_name = e_entity->get<TCompName>();
        TCompTransform* comp_transform = e_entity->get<TCompTransform>();
        assert(comp_name);
        assert(comp_transform);

        //Save the entity
        status.entityName = std::string(comp_name->getName());
        status.entityPos = comp_transform->getPosition();
        status.entityRot = comp_transform->getRotation();
        status.entityPrefab = comp_registry->getPrefab();
        status.handle = entity;
        status.saved = true;
		status.entityType = comp_registry->getEntityType();

		if (status.entityType == EntityType::CUPCAKE) {
			CBTCupcake* comp_enemy = e_entity->get<CBTCupcake>();
			status.curve = comp_enemy->getNameCurve();//nombre de la curva
			status.lifeCupcake = comp_enemy->getLife();
			
		}
		else if (status.entityType == EntityType::SUSHI) {
			CBTSushi* comp_enemy = e_entity->get<CBTSushi>();
			status.curve = comp_enemy->getNameCurve();//nombre de la curva
			status.pauseBT = comp_enemy->isPaused();
		}
		else if (status.entityType == EntityType::RANGED_SUSHI) {
			CBTRangedSushi* comp_enemy = e_entity->get<CBTRangedSushi>();
			status.curve = comp_enemy->getNameCurve();
		}
		else if (status.entityType == EntityType::CUPCAKE_SPAWNER) {
			TCompEnemySpawner* comp_enemy = e_entity->get<TCompEnemySpawner>();
			if(comp_enemy != nullptr) {
 				status.lifeCupcake = comp_enemy->getLifeSpawner();
				status.comportamentNormal = comp_enemy->getComportamentNormal();
				status.ovenDestroyed = comp_enemy->getDestroyed();
			}
		}
		else if (status.entityType == EntityType::GOLEM) {
			
		}
        entities.push_back(status);
    }



    saved = true;

    dbg("Game saved.\n");
    return !error;
}

/*

This function deletes those entities marked to be registered and creates them again using their stored status

*/
bool CCheckpoint::loadCheckPoint()
{
    PROFILE_FUNCTION("CCheckpoint::loadCheckPoint");
    if (saved) {
        dbg("Loading Player...\n");
        //Move the player to the saved position and reset it
        CHandle h_player = EngineEntities.getPlayerHandle();
        if (h_player.isValid() && playerStatus.saved) {
            CEntity * e_player = h_player;
            TCompCollider * playerCollider = e_player->get<TCompCollider>();
            playerStatus.playerPos.y += 0.5f;
            playerCollider->controller->setPosition(VEC3_TO_PXEXVEC3(playerStatus.playerPos));
			TCompBlackboard* c_b = e_player->get<TCompBlackboard>();
			c_b->resetBlackBoard();
			CEntity* e_inventory = getEntityByName("Inventory");
			TCompInventory* inventory = e_inventory->get<TCompInventory>();
			inventory->setBattery(playerStatus.battery);
			inventory->setChilli(playerStatus.chilli);
			inventory->setCoffe(playerStatus.coffe);
			inventory->setTeleport(playerStatus.teleport);

			TCompCharacterController* compCharacterController = e_player->get<TCompCharacterController>();
			compCharacterController->power_selected = playerStatus.power_selected;
			
            GameController.healPlayer();
        }

        dbg("Loading Registered Entities...\n");
        //Stop Behavior Trees and State Machines in order to be able to delete them safely
        GameController.stopBehaviorTrees();
        GameController.stopStateMachines();
        //Delete all entities present in the scene with the checkpoint_registered tag
        VHandles allEntities = CTagsManager::get().getAllEntitiesByTag(getID("checkpoint_registered"));
        for (auto& handle : allEntities) {
            if (handle.isValid()) {
              handle.destroy();
            }
        }
        //Load saved entities
        for (auto& entity : entities) {
            //Spawn the entity using it's stored prefab, make sure it spawned properly
            CHandle handle = GameController.spawnPrefab(entity.entityPrefab, entity.entityPos, entity.entityRot);
            assert(handle.isValid());
            CEntity* spawnedEntity = (CEntity*)handle;
            TCompName* spawnedEntity_cname = spawnedEntity->get<TCompName>();
            //Update its name with the stored value
            spawnedEntity_cname->setName(entity.entityName.c_str());

			if (entity.entityType == EntityType::CUPCAKE) {
				CBTCupcake* comp_enemy = spawnedEntity->get<CBTCupcake>();
				comp_enemy->setLife(entity.lifeCupcake);
				if (entity.curve != "") {
					comp_enemy->setCurve(Resources.get(entity.curve)->as<CCurve>());
				}
			}
			else if (entity.entityType == EntityType::SUSHI) {
				CBTSushi* comp_enemy = spawnedEntity->get<CBTSushi>();
				
				comp_enemy->setCurve(Resources.get(entity.curve)->as<CCurve>());
				
				comp_enemy->setPaused(entity.pauseBT);
			}
			else if (entity.entityType == EntityType::RANGED_SUSHI) {
				CBTRangedSushi* comp_enemy = spawnedEntity->get<CBTRangedSushi>();

				comp_enemy->setCurve(Resources.get(entity.curve)->as<CCurve>());
			}
			else if (entity.entityType == EntityType::CUPCAKE_SPAWNER) {
				TCompEnemySpawner* comp_enemy = spawnedEntity->get<TCompEnemySpawner>();
				comp_enemy->setLifeSpawner(entity.lifeCupcake);
				comp_enemy->setComportamentNormal(entity.comportamentNormal);
				comp_enemy->setDestroyed(entity.ovenDestroyed);
				
				VHandles v_spark = CTagsManager::get().getAllEntitiesByTag(getID("spark_particle_oven"));
				for (const auto& entity : v_spark) {
					CEntity* e_entity = (CEntity*)entity;
					if (e_entity != nullptr) {
						TCompSelfDestroy* destroy = e_entity->get<TCompSelfDestroy>();
						if (destroy != nullptr) {
							destroy->setEnabled(true);
							destroy->setDelay(0);
						}
					}
				}
			}
        }
        return true;
    }
    else {
        return false;
    }
}

bool CCheckpoint::deleteCheckPoint()
{
    playerStatus.saved = false;
    enemies.clear();
    entities.clear();
    saved = false;

    return true;
}


PlayerStatus CCheckpoint::getPlayerStatus(){
	return playerStatus;
}

bool CCheckpoint::savePower(PowerType power) {
	CEntity* e_player = GameController.getPlayerHandle();
	TCompCharacterController* compCharacterController = e_player->get<TCompCharacterController>();
	playerStatus.power_selected = power;
	return true;
}


void CCheckpoint::debugInMenu()
{
    if (ImGui::TreeNode("Checkpoint")) {

        ImGui::Text("Saved: ");
        ImGui::SameLine();
        if (saved) {
            ImGui::TextColored(ImVec4(0, 255, 0, 255), "TRUE");
            if (ImGui::TreeNode("Player")) {
                ImGui::Text("Position: (%f, %f, %f)", playerStatus.playerPos.x, playerStatus.playerPos.y, playerStatus.playerPos.z);
                ImGui::Text("Rotation: (%f, %f, %f)", playerStatus.playerRot.x, playerStatus.playerRot.y, playerStatus.playerRot.z, playerStatus.playerRot.w);
                ImGui::TreePop();
            }

            if (ImGui::TreeNode("Enemies")) {
                for (int i = 0; i < enemies.size(); i++) {
                    if (ImGui::TreeNode(enemies[i].enemyName.c_str())) {
                        ImGui::Text("Position: (%f, %f, %f)", enemies[i].enemyPosition.x, enemies[i].enemyPosition.y, enemies[i].enemyPosition.z);
                        ImGui::Text("State: %s", enemies[i].enemyIAStateName);
                        switch (enemies[i].enemyType) {
                        case EntityType::CUPCAKE:
                        {
                            ImGui::Text("Type: CUPCAKE");
                            break;
                        }
                        case EntityType::SUSHI:
                        {
                            ImGui::Text("Type: SUSHI");
                            break;
                        }
                        case EntityType::RANGED_SUSHI:
                        {
                            ImGui::Text("Type: RANGED_SUSHI");
                            break;
                        }
                        case EntityType::GOLEM:
                        {
                            ImGui::Text("Type: GOLEM");
                            break;
                        }
						case EntityType::EXPLOSIVE_CUPCAKE:
						{
							ImGui::Text("Type: EXPLOSIVE_CUPCAKE");
							break;
						}
                        }
                        ImGui::TreePop();
                    }
                }
                ImGui::TreePop();
            }

            if (ImGui::TreeNode("Entities")) {
                for (int i = 0; i < entities.size(); i++) {
                    if (ImGui::TreeNode(entities[i].entityName.c_str())) {
                        ImGui::Text("Position: (%f, %f, %f)", entities[i].entityPos.x, entities[i].entityPos.y, entities[i].entityPos.z);
						ImGui::Text("Curve: (%f, %f, %f)", entities[i].curve);
                        ImGui::TreePop();
                    }
                }
                ImGui::TreePop();
            }
        }
        else {
            ImGui::TextColored(ImVec4(255, 0, 0, 255), "FALSE");

        }

        ImGui::TreePop();
    }
}
