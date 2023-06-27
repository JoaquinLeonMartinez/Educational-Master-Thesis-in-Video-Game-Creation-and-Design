#pragma once

#include "components/common/comp_base.h"
#include "entity/entity.h"
#include "entity/entity_parser.h"
#include "entity/common_msgs.h"


class TCompDoor : public TCompBase
{
  DECL_SIBLING_ACCESS();

public:
  void debugInMenu();
  void load(const json& j, TEntityParseContext& ctx);
  void update(float dt);

  static void registerMsgs();
private:
	float ratio = 0.f;
	float mTravelTime = 0.02f;
	bool enabled = false;
	void onBattery(const TMsgGravity & msg);
	const CCurve* _curve = nullptr;
	VEC3 nextPoint;
	bool announceFlag = false;
	
};

