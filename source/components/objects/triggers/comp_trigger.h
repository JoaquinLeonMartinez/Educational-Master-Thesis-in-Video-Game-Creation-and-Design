#pragma once

#include "components/common/comp_base.h"
#include "entity/entity.h"
#include "entity/common_msgs.h"

class TCompTrigger : public TCompBase
{
  DECL_SIBLING_ACCESS();

public:
  void debugInMenu();
  void load(const json& j, TEntityParseContext& ctx);
  void update(float dt);

  static void registerMsgs();
  //este componente tendra un componente Transform
	//int type = 0;
 
private:
  //std::string in_script;
  //std::string out_script;

  std::string nameTrigger;
  std::string onEnter_ = "";
  std::string onExit_ = "";
  int delay = 0;
  void onEnter(const TMsgEntityTriggerEnter & msg);
  void onExit(const TMsgEntityTriggerExit & msg); 
};

