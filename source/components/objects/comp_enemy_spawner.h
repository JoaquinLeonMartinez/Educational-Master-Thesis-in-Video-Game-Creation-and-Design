#pragma once

#include "components/common/comp_base.h"
#include "entity/entity.h"
#include "entity/entity_parser.h"
#include "entity/common_msgs.h"
#include "modules/game/module_game_manager.h"
#include "modules/game/audio/audioEvent.h"


class TCompEnemySpawner : public TCompBase
{
  DECL_SIBLING_ACCESS();

public:

   

  void debugInMenu();
  void load(const json& j, TEntityParseContext& ctx);
  void update(float dt);
  void setLifeSpawner(float new_life);
  float getLifeSpawner();
  static void registerMsgs();
  void setLengthCone(float length_cone);
  void setScriptTriggerActivate(bool activate);
  void setCurveForSpawner(std::string curveForCupcake);
  void setComportamentNormal(int value);
  int getComportamentNormal();
  void setSpawnDelay(float newtime);
  void setSpawnMaxNumber(int value);
  bool working = true;
  void setDestroyed(bool destroyed) {
	  is_destroyed = destroyed;
  }
  bool getDestroyed() {
	  return is_destroyed;
  }
 

private:

	#define PREFAB_CUPCAKE_ "data/prefabs/enemies/bt_cupcake.json"

	CEntity* e_ = nullptr;
	bool _isEnabled = false;
	bool firstTime = true;
  bool is_destroyed = false;
	int _spawnMaxNumber = 2;
	float destroyEffectTimer = 1.3f;
	std::vector<CHandle> _currentEnemies;
	float _spawnDelay = 5.f;
	float _spawnTimer = _spawnDelay;
	std::string _prefab = PREFAB_CUPCAKE_;
    float _spawnOffset = 2.5f;
    float lifePrefabSpawner = 50.f;
	float lengthConePrefabSpawner = 60.0f;
	float smoke_position = 3.3f;

	float smokeTimerMax = 0.3f;
	float smokeTimer = 0.0f;
	float smokeOffsetZ = 0.0;
	float smokeOffsetX = 0.0f;

	void enable(const TMsgEntityTriggerEnter & msg);
	void disable(const TMsgEntityTriggerExit & msg);
	void onBattery(const TMsgGravity & msg);
	void onCheckout(const TMsgSpawnerCheckout & msg);
  void onCreation(const TMsgEntityCreated& msgC);
	AudioEvent audio;

	int comportamentNormal = 0;

	bool scriptTriggerActivate = false;
	std::string curveForCupcake = "";

};

