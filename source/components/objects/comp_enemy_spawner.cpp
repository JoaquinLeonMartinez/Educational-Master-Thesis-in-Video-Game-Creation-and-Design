#include "mcv_platform.h"
#include "engine.h"
#include "comp_enemy_spawner.h"
#include "components/common/comp_transform.h"
#include "components/ai/others/comp_blackboard.h"
#include "components/animation/comp_prop_animation.h"
#include "components/ai/bt/bt_cupcake.h"
#include "components/common/comp_tags.h"

DECL_OBJ_MANAGER("comp_enemy_spawner", TCompEnemySpawner);


using namespace physx;
std::mt19937 spawner_x(3942);
std::uniform_int_distribution<int> spawner_x_range(-100, 100);

std::mt19937 spawner_z(942);
std::uniform_int_distribution<int> spawner_z_range(-100, 100);

void TCompEnemySpawner::debugInMenu() {



}

void TCompEnemySpawner::load(const json& j, TEntityParseContext& ctx) {
	_prefab = j.value("_prefab", _prefab);
	_spawnMaxNumber = j.value("_spawnMaxNumber", _spawnMaxNumber);
	working = j.value("working", working);
	audio = EngineAudio.playEvent("event:/Enemies/Hazards/Oven/Oven_Loop");
}

void TCompEnemySpawner::registerMsgs() {
	DECL_MSG(TCompEnemySpawner, TMsgEntityTriggerEnter, enable);
	DECL_MSG(TCompEnemySpawner, TMsgEntityTriggerExit, disable);
	DECL_MSG(TCompEnemySpawner, TMsgGravity, onBattery);
	DECL_MSG(TCompEnemySpawner, TMsgSpawnerCheckout, onCheckout);
	DECL_MSG(TCompEnemySpawner, TMsgEntityCreated, onCreation);
}

void TCompEnemySpawner::onCreation(const TMsgEntityCreated& msgC) {
  
}

void TCompEnemySpawner::enable(const TMsgEntityTriggerEnter & msg) {
	if (msg.h_entity == GameController.getPlayerHandle()) {
		_isEnabled = true;
		dbg("Enemy spawner enabled.\n");
	}
}

void TCompEnemySpawner::disable(const TMsgEntityTriggerExit & msg) {
	if (msg.h_entity == GameController.getPlayerHandle()) {
		_isEnabled = false;
		dbg("Enemy spawner disabled.\n");
	}
}


void TCompEnemySpawner::onBattery(const TMsgGravity & msg) {
	if (!is_destroyed) {
		_isEnabled = false;
		EngineAudio.playEvent("event:/Character/Powers/Battery/Glitch");
		is_destroyed = true;
		// ----- soltar chispas:
		TCompTransform* c_trans = get<TCompTransform>();
		TEntityParseContext ctx;
		ctx.root_transform = *c_trans;
		ctx.root_transform.setPosition(ctx.root_transform.getPosition() + VEC3(0, 1.8f, 0));

		parseScene("data/particles/spark_particles_oven.json", ctx);
		CEntity* e = ctx.entities_loaded[0];
		TCompBuffers* c_b = e->get<TCompBuffers>();
		if (c_b) {
			auto buf = c_b->getCteByName("TCtesParticles");
			CCteBuffer<TCtesParticles>* data = dynamic_cast<CCteBuffer<TCtesParticles>*>(buf);
			data->emitter_center = ctx.root_transform.getPosition() + (ctx.root_transform.getFront() * 2.0f) + ctx.root_transform.getLeft() * 1.7f;
			data->updateGPU();
		}

		TEntityParseContext ctx2;
		VEC3 pos = c_trans->getPosition() + VEC3(0, 1.8f, 0) - c_trans->getLeft() * 0.1f;
		
		parseScene("data/particles/oven_particles.json", ctx2);
		CEntity* e2 = ctx2.entities_loaded[0];
		TCompBuffers* c_b2 = e2->get<TCompBuffers>();
		if (c_b2) {
		  auto buf2 = c_b2->getCteByName("TCtesParticles");
		  CCteBuffer<TCtesParticles>* data2 = dynamic_cast<CCteBuffer<TCtesParticles>*>(buf2);
		  data2->emitter_center = pos;
		  data2->updateGPU();
		}

		audio.stop();
		audio = EngineAudio.playEvent("event:/Enemies/Hazards/Oven/Oven_Broken_Loop");
		TCompPropAnimator* animator = get<TCompPropAnimator>();
		animator->playAnimation(TCompPropAnimator::OVEN_MEDIO_ABIERTO, 1.f);
	}

}

void TCompEnemySpawner::onCheckout(const TMsgSpawnerCheckout & msg) {
	for (auto it = _currentEnemies.begin(); it != _currentEnemies.end(); ++it) {
		if (*it == msg.enemyHandle) {
			_currentEnemies.erase(it);
			_spawnTimer = _spawnDelay;
			return;
		}
	}
}


void TCompEnemySpawner::update(float dt) {
	/*if (is_destroyed) {
		CEntity* handlePlayer = GameController.getPlayerHandle();
		if (handlePlayer != nullptr) {
			TCompCharacterController* compCharacter = handlePlayer->get<TCompCharacterController>();
			if (compCharacter->life <= 0) {
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
					
					
				}*/
				/*if (e_ != nullptr) {
					TCompName* name = e_->get<TCompName>();
					dbg("%s", name->getName());
					TCompBuffers* c_b = e_->get<TCompBuffers>();
					if (c_b) {
						TCompSelfDestroy* destroy = e_->get<TCompSelfDestroy>();
						if(destroy != nullptr){
							destroy->setEnabled(true);
							destroy->setDelay(0);
						}
					}
				}*/
			/*}
		}
	}*/


	TCompTransform* c_trans = get<TCompTransform>();
	if (c_trans)
		audio.set3DAttributes(*c_trans);
	if (comportamentNormal == 0) {
		if (working && !is_destroyed) {
			if (scriptTriggerActivate) {//codigo de cupcake que sale del horno , poner a false cuando acabe script
				TCompPropAnimator* animator = get<TCompPropAnimator>();
				animator->playAnimation(TCompPropAnimator::OVEN_OPEN, 1.f);
				VEC3 spawnPoint = c_trans->getPosition() + (c_trans->getFront() * _spawnOffset);
				CHandle enemy = GameController.spawnPrefab(_prefab, spawnPoint);
				AudioEvent aux_audio = EngineAudio.playEvent("event:/Enemies/Hazards/Oven/Oven_Spawn");
				aux_audio.set3DAttributes(*c_trans);

				CEntity* player = GameController.getPlayerHandle();
				CEntity* e_enemy = (CEntity*)enemy;
				TCompTransform* c_trans_enemy = e_enemy->get<TCompTransform>();
				if (player != nullptr) {
					TCompTransform* c_trans_player = player->get<TCompTransform>();
					c_trans_enemy->rotateTowards(c_trans_player->getPosition());
				}
				if (_prefab == PREFAB_CUPCAKE_) {
					CBTCupcake* cbt = e_enemy->get<CBTCupcake>();
					cbt->setLife(300.f);
				}


				GameController.updateCupcakeCurveByHandle(curveForCupcake, e_enemy);

				scriptTriggerActivate = false;
				_currentEnemies.push_back(enemy);
				TMsgSpawnerCheckin checkin;
				checkin.spawnerHandle = CHandle(this).getOwner();
				((CEntity*)enemy)->sendMsg(checkin);
			}


			if (_isEnabled) {
				//TODO:destruir el trigger asociado a la aparicion, porque ya no tiene sentido
				if (_spawnTimer <= 0 && _currentEnemies.size() < _spawnMaxNumber) {
					dbg("Spawning %s\n", _prefab);
					TCompPropAnimator* animator = get<TCompPropAnimator>();
					animator->playAnimation(TCompPropAnimator::OVEN_OPEN, 1.f);
					TCompTransform* c_trans = get<TCompTransform>();
					VEC3 spawnPoint = c_trans->getPosition() + VEC3(0, 1.5, 0) + (c_trans->getFront() * _spawnOffset);
					CHandle enemy = GameController.spawnPrefab(_prefab, spawnPoint);
					AudioEvent aux_audio = EngineAudio.playEvent("event:/Enemies/Hazards/Oven/Oven_Spawn");
					aux_audio.set3DAttributes(*c_trans);

					//rotamos el cupcake hacia la posicion del jugador al spawnear para que te vea siempre
					CEntity* player = GameController.getPlayerHandle();
					CEntity* e_enemy = (CEntity*)enemy;
					TCompTransform* c_trans_enemy = e_enemy->get<TCompTransform>();
					if (player != nullptr) {
						TCompTransform* c_trans_player = player->get<TCompTransform>();
						c_trans_enemy->rotateTowards(c_trans_player->getPosition());
					}

					if (_prefab == PREFAB_CUPCAKE_) {
						CBTCupcake* cbt = e_enemy->get<CBTCupcake>();
						cbt->setLife(lifePrefabSpawner);
					}

					_currentEnemies.push_back(enemy);
					TMsgSpawnerCheckin checkin;
					checkin.spawnerHandle = CHandle(this).getOwner();
					((CEntity*)enemy)->sendMsg(checkin);
					_spawnTimer = _spawnDelay;
				}
				else {
					_spawnTimer -= dt;
				}
			}
		}

	}
	else {
		if (working && !is_destroyed) {
			if (_spawnTimer <= 0 && _currentEnemies.size() < _spawnMaxNumber) {
				dbg("Spawning %s\n", _prefab);
				TCompPropAnimator* animator = get<TCompPropAnimator>();
				animator->playAnimation(TCompPropAnimator::OVEN_OPEN, 1.f);
				TCompTransform* c_trans = get<TCompTransform>();
				VEC3 spawnPoint = c_trans->getPosition() + (c_trans->getFront() * _spawnOffset);
				CHandle enemy = GameController.spawnPrefab(_prefab, spawnPoint);
				AudioEvent aux_audio = EngineAudio.playEvent("event:/Enemies/Hazards/Oven/Oven_Spawn");
				aux_audio.set3DAttributes(*c_trans);

				//rotamos el cupcake hacia la posicion del jugador al spawnear para que te vea siempre
				CEntity* player = GameController.getPlayerHandle();
				CEntity* e_enemy = (CEntity*)enemy;
				TCompTransform* c_trans_enemy = e_enemy->get<TCompTransform>();
				if (player != nullptr) {
					TCompTransform* c_trans_player = player->get<TCompTransform>();
					c_trans_enemy->rotateTowards(c_trans_player->getPosition());
				}
				if (_prefab == PREFAB_CUPCAKE_) {
					CBTCupcake* cbt = e_enemy->get<CBTCupcake>();
					cbt->setLengthCone(lengthConePrefabSpawner);
				}



				_currentEnemies.push_back(enemy);
				TMsgSpawnerCheckin checkin;
				checkin.spawnerHandle = CHandle(this).getOwner();
				((CEntity*)enemy)->sendMsg(checkin);
				_spawnTimer = _spawnDelay;
			}
			else {
				_spawnTimer -= dt;
			}

		}
	}
}

void TCompEnemySpawner::setLifeSpawner(float new_value) {
	lifePrefabSpawner = new_value;
}

float TCompEnemySpawner::getLifeSpawner() {
	return lifePrefabSpawner;
}


void TCompEnemySpawner::setScriptTriggerActivate(bool activate) {
	scriptTriggerActivate = activate;
}


void TCompEnemySpawner::setCurveForSpawner(std::string curveForCupcake_) {
	curveForCupcake = curveForCupcake_;
}


void TCompEnemySpawner::setComportamentNormal(int value) {
	comportamentNormal = value;
}

int TCompEnemySpawner::getComportamentNormal() {
	return comportamentNormal;
}

void TCompEnemySpawner::setSpawnDelay(float newtime) {
	_spawnDelay = newtime;
}


void TCompEnemySpawner::setLengthCone(float newValue) {
	lengthConePrefabSpawner = newValue;
}


void TCompEnemySpawner::setSpawnMaxNumber(int value) {
	_spawnMaxNumber = value;
}

