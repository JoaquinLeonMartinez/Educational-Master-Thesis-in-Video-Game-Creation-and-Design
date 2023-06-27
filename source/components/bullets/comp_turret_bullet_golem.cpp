#include "mcv_platform.h"
#include "components/common/comp_transform.h"
#include "components/common/comp_name.h"
#include "components/ai/others/self_destroy.h"
#include "entity/entity_parser.h"
#include "engine.h"
#include "input/input.h"
#include "components/common/comp_tags.h"
#include "components/common/physics/comp_collider.h"
#include "input/module_input.h"
#include "modules/module_physics.h"
#include "components/controllers/comp_parabolic_launch.h"
#include "comp_turret_bullet_golem.h"
#include "modules/game/audio/audioEvent.h"

using namespace physx;

DECL_OBJ_MANAGER("turret_bullet_golem_controller", TCompTurretBulletGolem);
void TCompTurretBulletGolem::debugInMenu() {
}

void TCompTurretBulletGolem::load(const json& j, TEntityParseContext& ctx) {
  _speed = j.value("_speed", _speed);
  _destroyOnCollission = j.value("_destroyOnCollission", _destroyOnCollission);
  _requestAudioPermission = j.value("_requestAudioPermission", _requestAudioPermission);
  _audioOnHit = j.value("_audioOnHit", _audioOnHit);
}

void TCompTurretBulletGolem::registerMsgs() {
  DECL_MSG(TCompTurretBulletGolem, TMsgAssignBulletOwner, onLaunch);
  DECL_MSG(TCompTurretBulletGolem, TMsgEntityTriggerEnter, onCollision);
}


void TCompTurretBulletGolem::onCollision(const TMsgEntityTriggerEnter& msg) {
  if (!_isEnabled)
    return;

  CEntity* e_contact = (CEntity*)msg.h_entity;
  e_contact->sendMsg(_messageToTarget);
  if (_destroyOnCollission)
    _isEnabled = false;
  if (_audioOnHit != "") {
    dbg(_audioOnHit.c_str());
    //This is for those instances in which a sound would be repeated too many times so we ask someone else to play it
    //Example: Fire
    if (_requestAudioPermission) {
      CEntity* e_entity = getEntityByName("Player");
      if (e_entity) {
        TMsgSoundRequest soundRequest;
        soundRequest.name = _audioOnHit;
        e_entity->sendMsg(soundRequest);
      }
    }
    else {
        TCompTransform* c_trans = get<TCompTransform>();
        AudioEvent audio = EngineAudio.playEvent(_audioOnHit);
        audio.set3DAttributes(*c_trans);
    }
  }
}


void TCompTurretBulletGolem::onLaunch(const TMsgAssignBulletOwner& msg) {
  _targetDirection = msg.front;
  _messageToTarget = msg.messageToTarget;
  targetType = msg.messageToTarget.targetType;

  
}


void TCompTurretBulletGolem::update(float delta) {
  //If it's enabled
  if (_isEnabled) {
    //Fly in the desired direction
    TCompTransform* c_trans = get<TCompTransform>();
    VEC3 c_pos = c_trans->getPosition();
    
    float yaw, pitch, roll;
    c_trans->getAngles(&yaw,&pitch);
    yaw += 6.0 * delta;
    pitch += sin(Time.current * 3.0f) * delta;
    pitch = clamp(pitch, -1.2f, 1.2f);
    c_trans->setAngles(yaw, pitch);

    //Movement control
    c_pos += (_targetDirection * _speed * delta);

    const PxTransform tr(VEC3_TO_PXVEC3(c_pos), QUAT_TO_PXQUAT(c_trans->getRotation()));
    TCompCollider* c_col = get<TCompCollider>();
    physx::PxRigidDynamic* rigid_dynamic = static_cast<physx::PxRigidDynamic*>(c_col->actor);
    if (rigid_dynamic) {
      rigid_dynamic->setKinematicTarget(tr);
    }
  }
  else {
    destroy();
  }
}

void TCompTurretBulletGolem::renderDebug() {
}

void TCompTurretBulletGolem::destroy() {
  CEntity* onom_manager = getEntityByName("Onomatopoeia Particles");
  TMsgOnomPet msgonom;
  msgonom.type = 5.0f;
  TCompTransform* c_trans2 = get<TCompTransform>();
  msgonom.pos = c_trans2->getPosition();
  onom_manager->sendMsg(msgonom);

  CHandle(this).getOwner().destroy();
  CHandle(this).destroy();
}