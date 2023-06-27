#pragma once

#include "components/common/comp_base.h"
#include "entity/entity.h"
#include "entity/entity_parser.h"
#include "entity/common_msgs.h"

class TCompTermoestato : public TCompBase
{
  DECL_SIBLING_ACCESS();

public:
  void debugInMenu();
  void load(const json& j, TEntityParseContext& ctx);
 
  void update(float delta);
  static void registerMsgs();

private:
	
	bool _isEnabled = false;

	void onBattery(const TMsgGravity & msg);

	
	
};

