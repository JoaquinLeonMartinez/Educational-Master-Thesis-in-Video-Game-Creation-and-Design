#include "mcv_platform.h"
#include "engine.h"
#include "components/common/comp_transform.h"
#include "components/objects/comp_rotator.h"
#include "modules/module_physics.h"


using namespace physx;

DECL_OBJ_MANAGER("comp_rotator", TCompRotator);

void TCompRotator::debugInMenu() {



}

void TCompRotator::load(const json& j, TEntityParseContext& ctx) {
	
}


void TCompRotator::update(float dt) {
	if(activateRotator){
		TCompTransform* c_trans = get<TCompTransform>();
		if (axis.x == 1 && axis.y == 0 && axis.z == 0) {
			QUAT angle = QUAT::CreateFromAxisAngle(VEC3(1, 0, 0), dt * rotationTime);
			c_trans->setRotation(angle * c_trans->getRotation());
		}
		else if (axis.x == 0 && axis.y == 1 && axis.z == 0) {
			QUAT angle = QUAT::CreateFromAxisAngle(VEC3(0, 1, 0), dt * rotationTime);
			c_trans->setRotation(angle * c_trans->getRotation());

		}
		else if (axis.x == 0 && axis.y == 0 && axis.z == 1) {
			QUAT angle = QUAT::CreateFromAxisAngle(VEC3(0, 0, 1), dt * rotationTime);
			c_trans->setRotation(angle * c_trans->getRotation());
		}
	}
}
	

