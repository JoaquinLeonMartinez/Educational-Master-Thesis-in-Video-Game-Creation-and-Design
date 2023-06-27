#include "mcv_platform.h"
#include "comp_slow_disappear.h"
#include "components/common/comp_buffers.h"

DECL_OBJ_MANAGER("comp_slow_disappear", TCompSlowDisappear);

void TCompSlowDisappear::debugInMenu() {
}

void TCompSlowDisappear::load(const json& j, TEntityParseContext& ctx) {
	_isEnabled = j.value("_isEnabled", _isEnabled);
	_selfDestructDelay = j.value("_stopSpawning", _selfDestructDelay);
	_selfDestructTimer = _selfDestructDelay;
}

void TCompSlowDisappear::enable() {
	_isEnabled = true;
}

void TCompSlowDisappear::update(float delta)
 {
	if (_isEnabled) {
		if (_selfDestructTimer <= 0) {
      TCompBuffers* c_buff = get<TCompBuffers>();
      if (c_buff) {
        auto buf = c_buff->getCteByName("TCtesParticles");
        CCteBuffer<TCtesParticles>* data = dynamic_cast<CCteBuffer<TCtesParticles>*>(buf);
        data->emitter_num_particles_per_spawn = 0;
        data->updateGPU();
      }
      _isEnabled = false;
		}
		else {
			_selfDestructTimer -= delta;
		}
	}
}
