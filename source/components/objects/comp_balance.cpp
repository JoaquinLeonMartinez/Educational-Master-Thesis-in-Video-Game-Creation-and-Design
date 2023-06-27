#include "mcv_platform.h"
#include "engine.h"
#include "comp_balance.h"
#include "components/common/comp_tags.h"
#include "components/common/comp_transform.h"
#include "components/common/comp_render.h"
#include "modules/module_physics.h"
#include "components/common/comp_name.h"
#include "render/textures/material.h"
using namespace physx;
std::mt19937 ba_mt_ba(std::random_device{}());
std::uniform_int_distribution<int> ba_mt_ba_dist(-4, 4);
std::uniform_int_distribution<int> ba_mt_ba_dist2(-2, 2);


DECL_OBJ_MANAGER("comp_balance", TCompBalance);

void TCompBalance::debugInMenu() {
}

void TCompBalance::load(const json& j, TEntityParseContext& ctx) {


}

void TCompBalance::registerMsgs() {
  DECL_MSG(TCompBalance, TMsgEntityCreated, onCreate);
  DECL_MSG(TCompBalance, TMsgDamage, onDamage);
}

void TCompBalance::onDamage(const TMsgDamage& msg) {
  TCompCollider* c_col = get<TCompCollider>();
  physx::PxRigidDynamic* rigid_dynamic = static_cast<physx::PxRigidDynamic*>(c_col->actor);

  float velocitiRandom = ba_mt_ba_dist2(ba_mt_ba);
  rigid_dynamic->addForce(PxVec3(0, velocitiRandom, 0), PxForceMode::eVELOCITY_CHANGE);
}


void TCompBalance::onCreate(const TMsgEntityCreated& msg) {
  TCompCollider* c_col = get<TCompCollider>();
  physx::PxRigidDynamic* rigid_dynamic = static_cast<physx::PxRigidDynamic*>(c_col->actor);

  float velocitiRandom = ba_mt_ba_dist(ba_mt_ba);
  rigid_dynamic->addForce(PxVec3(0, velocitiRandom, 0), PxForceMode::eVELOCITY_CHANGE);

}

void TCompBalance::balanceo() {

	if (!balanceoDone) {
		getObjectManager<TCompBalance>()->forEach([](TCompBalance* di) {
			TCompCollider* c_col = di->get<TCompCollider>();
			physx::PxRigidDynamic* rigid_dynamic = static_cast<physx::PxRigidDynamic*>(c_col->actor);
			float velocitiRandom = ba_mt_ba_dist(ba_mt_ba);
			VEC3 linear_speed = PXVEC3_TO_VEC3(rigid_dynamic->getLinearVelocity());
			float velLineal = linear_speed.Length();
			if (velLineal <= 0.00) {
				
				//dbg("VEL_LINEAL:%f\n",velLineal);
				rigid_dynamic->addForce(PxVec3(0, -900, 0), PxForceMode::eFORCE);
				//rigid_dynamic->addForce(PxVec3(0, velocitiRandom, 0), PxForceMode::eVELOCITY_CHANGE);
			}
		});
		balanceoDone = true;
	}
}

void TCompBalance::cambioTexturaJoint(bool apagado) {
	VHandles v_tp_joints = CTagsManager::get().getAllEntitiesByTag(getID("joints"));
	for (const auto& entity : v_tp_joints) {
		CEntity* e_entity = (CEntity*)entity;

		TCompRender* comp_render = e_entity->get<TCompRender>();
		for (auto& p : comp_render->parts) {
			if(apagado) {
				comp_render->setMaterial("data/materials/LAMPARA_PALITOS/mat_lampara_med_apagado.material", false);
			}
			else {
				comp_render->setMaterial("data/materials/LAMPARA_PALITOS/mat_lampara_med.material", false);
			}
		}

	}
}


void TCompBalance::update(float dt) {
 getObjectManager<TCompBalance>()->forEach([](TCompBalance* di) {
    TCompCollider* c_col = di->get<TCompCollider>();
    physx::PxRigidDynamic* rigid_dynamic = static_cast<physx::PxRigidDynamic*>(c_col->actor);

    VEC3 linear_speed = PXVEC3_TO_VEC3(rigid_dynamic->getLinearVelocity());
    float velLineal = linear_speed.Length();
    //dbg("VEL_LINEAL:%f\n", velLineal);
    if (velLineal <= 0.00) {
      float velocitiRandom = ba_mt_ba_dist2(ba_mt_ba);
      rigid_dynamic->addForce(PxVec3(0, velocitiRandom, 0), PxForceMode::eVELOCITY_CHANGE);
    }
  });
}