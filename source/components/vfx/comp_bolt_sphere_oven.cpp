#include "mcv_platform.h"
#include "comp_bolt_sphere_oven.h"
#include "components/common/comp_buffers.h"
#include "components/ai/others/self_destroy.h"
#include "components/common/comp_transform.h"

DECL_OBJ_MANAGER("bolt_sphere_oven", TCompBoltSphereOven);

void TCompBoltSphereOven::debugInMenu() {

}

void TCompBoltSphereOven::load(const json& j, TEntityParseContext& ctx) {
  speed = j.value("speed", speed);
}


void TCompBoltSphereOven::renderDebug() {

}

void TCompBoltSphereOven::registerMsgs() {
	DECL_MSG(TCompBoltSphereOven, TMsgEntityCreated, onCreation);
}

void TCompBoltSphereOven::onCreation(const TMsgEntityCreated& msgC) {
	TCompTransform* c_trans = get<TCompTransform>();
	if (c_trans) {
		c_trans = get<TCompTransform>();
		VEC3 pos = c_trans->getPosition();
		target_position = pos + VEC3(0, 2, 0);
	}

}

void TCompBoltSphereOven::update(float delta) {

	TCompTransform* cTransform = get<TCompTransform>();


	CEntity* e = getEntityByName("SparkParticles_oven");
	if (e) {
		TCompBuffers* c_buff = e->get<TCompBuffers>();
		if (c_buff) {
			auto buf = c_buff->getCteByName("TCtesParticles");
			if (c_buff) {
				CCteBuffer<TCtesParticles>* data = dynamic_cast<CCteBuffer<TCtesParticles>*>(buf);
				data->emitter_center = cTransform->getPosition();
				data->updateGPU();
			}
		}
	}
}