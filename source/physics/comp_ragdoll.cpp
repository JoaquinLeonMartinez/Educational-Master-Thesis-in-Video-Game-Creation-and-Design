#include "mcv_platform.h"
#include "comp_ragdoll.h"
#include "components/common/comp_transform.h"
#include "skeleton/comp_skeleton.h"
#include "modules/module_manager.h"
#include "entity/entity.h"
#include "modules/module_physics.h"
#include "render/meshes/mesh.h"
//#include "render/render_objects.h"
#include "physics/ragdoll.h"
#include "physics/ragdoll_core.h"
#include "cal3d/cal3d.h"
#include "skeleton/cal3d2engine.h"
#include "engine.h"

//DECL_OBJ_MANAGER("ragdoll", TCompRagdoll);

// Simplify this
#include "resources/resource.h"
#include "render/render.h"

TCompRagdoll::~TCompRagdoll() {
	//CModulePhysics& module_physics = CEngine::get().getPhysics();
  //module_physics.destroyActor(*this);
}

void TCompRagdoll::renderInMenu() {
}

void TCompRagdoll::registerMsgs() {
  DECL_MSG(TCompRagdoll, TMsgEntityCreated, onCreated);
}

void TCompRagdoll::load(const json& j, TEntityParseContext& ctx) {
  
}

void TCompRagdoll::onCreated(const TMsgEntityCreated&) {
	//EnginePhysics.createRagdoll(*this);
}

void TCompRagdoll::updateRagdollFromSkeleton() {
	CHandle h_comp_ragdoll(this);
	CEntity* ent = h_comp_ragdoll.getOwner();
	TCompTransform* comp_transform = ent->get<TCompTransform>();
	CTransform* extra_trans = comp_transform;
	TCompSkeleton* comp_skel = ent->get<TCompSkeleton>();
	if (comp_skel) {
		auto model = comp_skel->model;
		if (model) {
			auto core_model = model->getCoreModel();
			if (core_model) {
				auto skel = model->getSkeleton();
				if (skel) {
					auto core_skel = skel->getCoreSkeleton();
					if (core_skel) {

						int root_core_bone_id = core_skel->getVectorRootCoreBoneId()[0];

						for (int i = 0; i < ragdoll.num_bones; ++i) {
							auto& ragdoll_bone = ragdoll.bones[i];

							CalBone *cal_bone = skel->getBone(ragdoll_bone.idx);

							CTransform transform;
							transform.setPosition(Cal2DX(cal_bone->getTranslationAbsolute()));
							transform.setRotation(Cal2DX(cal_bone->getRotationAbsolute()));
							transform.setPosition(transform.getPosition() + (transform.getLeft()*ragdoll_bone.core->height*0.5f));

							physx::PxTransform px_transform;
							px_transform.p = VEC3_TO_PXVEC3(transform.getPosition());
							px_transform.q = QUAT_TO_PXQUAT(transform.getRotation());

							ragdoll_bone.actor->setGlobalPose(px_transform);
						}
					}
				}
			}
		}
	}
}

void TCompRagdoll::activateRagdoll() {
	if (active)
		return;
	active = true;
	updateRagdollFromSkeleton();
  CHandle h_comp_ragdoll(this);
  for (int i = 0; i < ragdoll.num_bones; ++i) {
	  auto& ragdoll_bone = ragdoll.bones[i];

	  EnginePhysics.getScene()->addActor(*ragdoll_bone.actor);
  }
}

void TCompRagdoll::deactivateRagdoll() {
	if (!active)
		return;
	active = false;
	CHandle h_comp_ragdoll(this);
	for (int i = 0; i < ragdoll.num_bones; ++i) {
		auto& ragdoll_bone = ragdoll.bones[i];

		EnginePhysics.getScene()->removeActor(*ragdoll_bone.actor);
	}
	updateRagdollFromSkeleton();

}

void TCompRagdoll::update(float elapsed) {
  /*
  //TODO: DEACTIVATE SKEL UPDATE
  //TODO: initially not simualted
  static float accum = 0.f;
  accum += elapsed;
  if (accum > 10.f) {
    static bool first_time = true;
    if (first_time) {
      first_time = false;
      activateRagdoll();

      for (int i = 0; i < ragdoll.num_bones; ++i) {
        auto& ragdoll_bone = ragdoll.bones[i];

        if (ragdoll_bone.core->bone== "Bip001 Head") {
          ragdoll_bone.actor->addForce(VEC3_TO_PXVEC3(VEC3(0.f, 0.016f, 0.f)), physx::PxForceMode::eIMPULSE);
        }
      }
    }
  }
  else {
    return;
  }
  */

	if (!active)
	{
		updateRagdollFromSkeleton();
		return;
	}
  CHandle h_comp_ragdoll(this);
  CEntity* ent = h_comp_ragdoll.getOwner();
  TCompSkeleton* comp_skel = ent->get<TCompSkeleton>();
  if (comp_skel) {
    auto model = comp_skel->model;
    if (model) {
      auto core_model = model->getCoreModel();
      if (core_model) {
        auto skel = model->getSkeleton();
        if (skel) {
          auto& cal_bones = skel->getVectorBone();
          assert(cal_bones.size() < MAX_SUPPORTED_BONES);

          for (int i = 0; i < cal_bones.size(); ++i) {
            bool found = false;
            for (int j = 0; j < ragdoll.num_bones; ++j) {
              const auto& ragdoll_bone = ragdoll.bones[j];
              if (ragdoll_bone.idx == i) {
                CalBone* bone = cal_bones[ragdoll_bone.idx];

                CTransform trans;
                auto t = ragdoll_bone.actor->getGlobalPose();

                trans.setPosition(PXVEC3_TO_VEC3(t.p));
                trans.setRotation(PXQUAT_TO_QUAT(t.q));

                bone->clearState();
                bone->blendState(1.f, DX2Cal(trans.getPosition()), DX2Cal(trans.getRotation()));
                bone->calculateBoneSpace();
                found = true;
                break;
              }
            }

            if (!found) {
              CalBone* bone = cal_bones[i];
              bone->setCoreState();
              bone->calculateBoneSpace();
            }
          }
        }
      }
    }
  }
}

void TCompRagdoll::debugInMenu() {
	static bool activated = false;
	ImGui::Checkbox("activated", &activated);
	if (activated)
		activateRagdoll();
	else
		deactivateRagdoll();

}

void TCompRagdoll::renderDebug()  {
  for (int i = 0; i < ragdoll.num_bones; ++i) {
    TRagdoll::TRagdollBone& ragdoll_bone =  ragdoll.bones[i];
    physx::PxTransform px_transform = ragdoll_bone.actor->getGlobalPose();
    CTransform transform = toTransform(px_transform);

    //transform.setRotation(PXQUAT_TO_QUAT(px_transform.q));
	//VEC3 offset = ragdoll_bone.core->height*0.5f*transform.getFront();
    //transform.setPosition(PXVEC3_TO_VEC3(px_transform.p)- offset);
	MAT44 world1 = transform.asMatrix();
	//transform.setPosition(PXVEC3_TO_VEC3(px_transform.p) + offset);
	MAT44 world2 = transform.asMatrix();
	
	// Increase the radius of our unit sphere to match the radius of the physics object
	drawWiredSphere(world1, ragdoll_bone.core->radius, VEC4(1, 1, 1, 1));
	drawWiredSphere(world2, ragdoll_bone.core->radius, VEC4(1, 1, 1, 1));
	drawLine(world1.Translation(), world2.Translation(), VEC4(1, 1, 1, 1));
  }
}
