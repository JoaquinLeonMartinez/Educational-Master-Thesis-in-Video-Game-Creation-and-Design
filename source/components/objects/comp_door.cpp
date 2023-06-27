#include "mcv_platform.h"
#include "engine.h"
#include "components/common/comp_transform.h"
#include "components/objects/comp_door.h"
#include "modules/module_physics.h"


using namespace physx;

DECL_OBJ_MANAGER("comp_door", TCompDoor);

void TCompDoor::debugInMenu() {



}

void TCompDoor::load(const json& j, TEntityParseContext& ctx) {
	mTravelTime = j.value("mTravelTime_", mTravelTime);
	_curve = Resources.get(j.value("curve", ""))->as<CCurve>();
}

void TCompDoor::registerMsgs() {
	DECL_MSG(TCompDoor, TMsgGravity, onBattery);
}

void TCompDoor::onBattery(const TMsgGravity & msg) {
	enabled = true;
	//solo es una puerta que se abre con bateria
	TCompName* name = get<TCompName>();
	std::string nameEntity = name->getName();
	if (nameEntity.compare("puerta_suelta") == 0) {
		if(!announceFlag) {
			GameController.playAnnouncement("event:/UI/Announcements/Announcement7");
			announceFlag = true;
		}
	}
}


void TCompDoor::update(float dt) {
	if (enabled) {
		TCompTransform* c_trans = get<TCompTransform>();
		TCompCollider* c_col = get<TCompCollider>();
		physx::PxRigidDynamic* rigid_dynamic = static_cast<physx::PxRigidDynamic*>(c_col->actor);

		/*if (ratio >= 1.0f || ratio < 0.0f) {
			mTravelTime = -mTravelTime;
		}*/
		if (ratio >= 1.0f) {
			//mTravelTime = -mTravelTime;
			//ratio = 1.0f;
			enabled = false;
		}
		if (ratio < 0.0f) {
			mTravelTime = -mTravelTime;
			ratio = 0.0f;
		}
		float yaw, pitch, roll = 0.f;
		c_trans->getAngles(&yaw, &pitch);
		nextPoint = _curve->evaluate(ratio);
		ratio += dt * mTravelTime;
		QUAT quat = c_trans->getRotation();
		PxVec3 pos = VEC3_TO_PXVEC3(nextPoint);
		PxQuat qua = QUAT_TO_PXQUAT(quat);
		const PxTransform tr(pos, qua);
		rigid_dynamic->setKinematicTarget(tr);
	}
}
	

