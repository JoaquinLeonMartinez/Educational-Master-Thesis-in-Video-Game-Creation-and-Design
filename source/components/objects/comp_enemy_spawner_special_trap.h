#pragma once

#include "components/common/comp_base.h"
#include "entity/entity.h"
#include "entity/entity_parser.h"
#include "entity/common_msgs.h"

class TCompEnemySpawnerSpecialTrap : public TCompBase
{
  DECL_SIBLING_ACCESS();

public:
  void debugInMenu();
  void load(const json& j, TEntityParseContext& ctx);
  void update(float dt);
  void setSpawnDelay(float time);
  static void registerMsgs();
  bool working = false; 

private:
	
	bool _isEnabled = false;
	bool is_destroyed = false;
	int _spawnMaxNumber = 2;
	std::vector<CHandle> _currentEnemies;
	float _spawnDelay = 0.f;
	float _spawnTimer = _spawnDelay;
	std::string _prefab = "data/prefabs/enemies/bt_cupcake.json";
  float _spawnOffset = 2.5f;


	void onBattery(const TMsgGravity & msg);
	void onCheckout(const TMsgSpawnerCheckout & msg);
};

