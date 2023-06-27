#include "mcv_platform.h"
#include "comp_dynamic_instance.h"
#include "engine.h"
#include "components/common/comp_transform.h"
#include "components/common/comp_tags.h"


DECL_OBJ_MANAGER("dynamic_instance", TCompDynamicInstance);

void TCompDynamicInstance::debugInMenu() {
  ImGui::LabelText("IDX: ", "%d", my_unique_idx);
}

void TCompDynamicInstance::load(const json& j, TEntityParseContext& ctx) {
}

void TCompDynamicInstance::renderDebug() {
}

void TCompDynamicInstance::onPlayerAttack(const TMsgDamage& msg) {
    TCompTransform* c_trans = get<TCompTransform>();

    VEC3 hit_pos = msg.position;
    VEC3 dir = c_trans->getPosition() - hit_pos;
    dir.y *= 0.1f;
    dir.Normalize();
    dir *= msg.impactForce * 2.0f;
    physx::PxVec3 impulse = VEC3_TO_PXVEC3(dir);
    
    TCompCollider* c_coll = get<TCompCollider>();
    physx::PxRigidDynamic* rigid_dynamica = static_cast<physx::PxRigidDynamic*>(c_coll->actor);
    rigid_dynamica->addForce(impulse, PxForceMode::eIMPULSE, false);

}

void TCompDynamicInstance::registerMsgs() {
  DECL_MSG(TCompDynamicInstance, TMsgDamage, onPlayerAttack);
  DECL_MSG(TCompDynamicInstance, TMsgEntityCreated, onCreate);
  DECL_MSG(TCompDynamicInstance, TMsgGravity, onBattery);
  DECL_MSG(TCompDynamicInstance, TMsgOnContact, onContact);
}

void TCompDynamicInstance::onContact(const TMsgOnContact& msg) {
    CEntity* source_of_impact = (CEntity*)msg.source.getOwner();
    TCompCollider* c_tag = source_of_impact->get<TCompCollider>();
    TCompCollider* c_collider = (TCompCollider*)h_coll;
    if (c_collider && life > 5.f && lastSoundTimer < 0.f) {
        rigid_dynamic = static_cast<physx::PxRigidDynamic*>(c_collider->actor);
        if (c_tag) {
            PxShape* colShape;
            c_tag->actor->getShapes(&colShape, 1, 0);
            PxFilterData col_filter_data = colShape->getSimulationFilterData();

            if (col_filter_data.word0 & EnginePhysics.Floor && !rigid_dynamic->isSleeping()) {
                TCompTransform* c_trans = get<TCompTransform>();
                AudioEvent audio = EngineAudio.playEvent("event:/Music/Ambience_Props/Products/Product_DropFloor");
                audio.set3DAttributes(*c_trans);
                lastSoundTimer = lastSoundDelay;
            }
        }
    }
}

void TCompDynamicInstance::onBattery(const TMsgGravity& msg) {
  TCompCollider* c_cc = get<TCompCollider>();
  TCompTransform* c_trans = get<TCompTransform>();
  physx::PxRigidDynamic* rigid_dynamic = static_cast<physx::PxRigidDynamic*>(c_cc->actor);
  VEC3 dir = c_trans->getPosition() - msg.position;
  dir.Normalize();
  dir *= 20.0f;
  rigid_dynamic->addForce(VEC3_TO_PXVEC3(dir), PxForceMode::eIMPULSE);

}

void TCompDynamicInstance::onCreate(const TMsgEntityCreated&) {
  TCompCollider* c_collider = get<TCompCollider>();
  h_coll = CHandle(c_collider);
}

void TCompDynamicInstance::update(float delta) {
  TCompCollider* c_collider = (TCompCollider*) h_coll;
  life += delta;
  lastSoundTimer -= delta;
  if (c_collider) {
    rigid_dynamic = static_cast<physx::PxRigidDynamic*>(c_collider->actor);

    //si no esta en reposo...
    if (!rigid_dynamic->isSleeping()) {
      //cojes el modulo de gpu culling y actualizas los datos de este obj (transform y aabb)
      //assert(my_unique_idx > -1);
      if (my_unique_idx < 0)
        return;
      CHandle h(this);
      CEngine::get().getGPUCulling().updateObjData(my_unique_idx, h.getOwner());
      //pones is_dirty = true
      CEngine::get().getGPUCulling().setDirty();
    }
  }
}
