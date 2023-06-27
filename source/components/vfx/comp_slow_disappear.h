#pragma once

#include "components/common/comp_base.h"
#include "entity/entity.h"


class TCompSlowDisappear : public TCompBase {
	DECL_SIBLING_ACCESS();

public:
	void update(float dt);
	void load(const json& j, TEntityParseContext& ctx);
	void debugInMenu();
  void setDelay(float delay) { _selfDestructDelay = delay; _selfDestructTimer = delay; }

	void enable();

private:
	bool _isEnabled = false;
	float _selfDestructDelay = 10.f;
	float _selfDestructTimer = _selfDestructDelay;
};