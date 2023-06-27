#pragma once

#include "components/common/comp_base.h"
#include "entity/entity.h"
#include "entity/entity_parser.h"
#include "entity/common_msgs.h"
#include "modules/game/audio/audioEvent.h"

class TCompInterruptor : public TCompBase
{
  DECL_SIBLING_ACCESS();

public:
  void debugInMenu();
  void load(const json& j, TEntityParseContext& ctx);
 

  static void registerMsgs();
private:
	bool isOn = false;
	std::string nombre;
	std::string elementoAmover;
	void onActivate(const TMsgDamage & msg);
	
};

