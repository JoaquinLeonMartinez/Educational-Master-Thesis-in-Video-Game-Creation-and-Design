#include "mcv_platform.h"
#include "self_destroy.h"

DECL_OBJ_MANAGER("comp_self_destroy", TCompSelfDestroy);

void TCompSelfDestroy::debugInMenu() {
	ImGui::LabelText("Is enabled: ", "%s", _isEnabled ? "true" : "false");
	ImGui::LabelText("TTL: ", "%f", _selfDestructTimer);
}

void TCompSelfDestroy::load(const json& j, TEntityParseContext& ctx) {
	_isEnabled = j.value("_isEnabled", _isEnabled);
	_selfDestructDelay = j.value("_selfDestructDelay", _selfDestructDelay);
	_selfDestructTimer = _selfDestructDelay;
}

void TCompSelfDestroy::enableWithDelay(float delay) {
	if (delay >= 0) {
		_selfDestructTimer = delay;
	}
	_isEnabled = true;
}

void TCompSelfDestroy::enable() {
	_isEnabled = true;
}

void TCompSelfDestroy::update(float delta)
 {
	if (_isEnabled) {
		if (_selfDestructTimer <= 0) {
			CHandle(this).getOwner().destroy();
			CHandle(this).destroy();
		}
		else {
			_selfDestructTimer -= delta;
		}
	}
}
