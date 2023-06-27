#pragma once

#include "components/common/comp_base.h"
#include "entity/entity.h"
#include "entity/entity_parser.h"
#include "entity/common_msgs.h"

class TCompTriggerLua : public TCompBase
{
  DECL_SIBLING_ACCESS();

public:
  void debugInMenu();
  void load(const json& j, TEntityParseContext& ctx);
  void update(float dt);


  static void registerMsgs();


  bool active = false;
  int killed_enemies = 0;
  int interval_enemy = 2;
 
	//test lua:
  bool activeEnemies = true;

private:
  void onEnter(const TMsgEntityTriggerEnter & msg);
  void onExit(const TMsgEntityTriggerExit & msg); 
  //void onActiveMsg(const TMsgSetActive & msg);
};

