#pragma once

#include "components/common/comp_base.h"
#include "entity/entity.h"
#include "entity/common_msgs.h"

class TCompScaleGradually : public TCompBase {

	DECL_SIBLING_ACCESS();

public:
	void update(float dt);
	void load(const json& j, TEntityParseContext& ctx);
	void debugInMenu();
	static void registerMsgs();
  bool _enabled = false;

private:
	float _finalScale = 1.f;
  float _time = 1.0f;
};