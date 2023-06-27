#pragma once

#include "components/common/comp_base.h"
#include "components/common/comp_name.h"
#include "entity/entity.h"
#include "entity/common_msgs.h"


class TCompEnemiesInButcher : public TCompBase
{
  DECL_SIBLING_ACCESS();

public:
  void load(const json& j, TEntityParseContext& ctx);//meter en jason la velocidad de lanzamiento
  void debugInMenu();
  void update(float dt);
  static void registerMsgs();
  bool activateTrap = false;
  
  
private:
	int count = 0;

	void onSushiTrap(const TMSgEnemyDead & msg);
	
};

