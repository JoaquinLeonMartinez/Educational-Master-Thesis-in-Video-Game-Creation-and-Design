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
#include "comp_kinematic_bullet.h"
#include "modules/game/audio/audioEvent.h"

using namespace physx;

DECL_OBJ_MANAGER("kinematic_bullet", TCompKinematicBullet);
void TCompKinematicBullet::debugInMenu() {
}

void TCompKinematicBullet::load(const json& j, TEntityParseContext& ctx) {
    _speed = j.value("_speed", _speed);
    _destroyOnCollission = j.value("_destroyOnCollission", _destroyOnCollission);
    _requestAudioPermission = j.value("_requestAudioPermission", _requestAudioPermission);
    _audioOnHit = j.value("_audioOnHit", _audioOnHit);
    comicfeed = j.value("comicFeed", comicfeed);
}

void TCompKinematicBullet::registerMsgs() {
    DECL_MSG(TCompKinematicBullet, TMsgAssignBulletOwner, onLaunch);
    DECL_MSG(TCompKinematicBullet, TMsgEntityTriggerEnter, onCollision);
}


void TCompKinematicBullet::expansiveWave() {
  TCompTransform* c_trans = get<TCompTransform>();
  PxVec3 pos = VEC3_TO_PXVEC3(c_trans->getPosition());
  PxQuat ori = QUAT_TO_PXQUAT(c_trans->getRotation());
  PxSphereGeometry geometry(5.0f);
  const PxU32 bufferSize = 256;
  PxOverlapHit hitBuffer[bufferSize];
  PxOverlapBuffer buf(hitBuffer, bufferSize);
  PxTransform shapePose = PxTransform(pos, ori);
  PxQueryFilterData filter_data = PxQueryFilterData();
  filter_data.data.word0 = EnginePhysics.Product;
  bool res = EnginePhysics.gScene->overlap(geometry, shapePose, buf, filter_data);
  if (res) {
    for (PxU32 i = 0; i < buf.nbTouches; i++) {
      CHandle h_comp_physics;
      h_comp_physics.fromVoidPtr(buf.getAnyHit(i).actor->userData);
      CEntity* entityContact = h_comp_physics.getOwner();
      if (entityContact) {
        TMsgDamage msg;
        // Who sent this bullet
        msg.h_sender = CHandle(this).getOwner();
        msg.h_bullet = CHandle(this).getOwner();
        msg.position = c_trans->getPosition() + VEC3::Up;
        msg.senderType = PLAYER;
        msg.intensityDamage = 5.0f;
        msg.impactForce = 5.0f;
        msg.damageType = MELEE;
        msg.targetType = ENEMIES;
        entityContact->sendMsg(msg);
      }
    }
  }
}


void TCompKinematicBullet::onCollision(const TMsgEntityTriggerEnter& msg) {
    if (!_isEnabled)
        return;

    CEntity* e_contact = (CEntity*)msg.h_entity;
    e_contact->sendMsg(_messageToTarget);
    expansiveWave();
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


void TCompKinematicBullet::onLaunch(const TMsgAssignBulletOwner& msg) {
    _targetDirection = msg.front;
    _messageToTarget = msg.messageToTarget;
    targetType = msg.messageToTarget.targetType;
}


void TCompKinematicBullet::update(float delta) {
    //If it's enabled
    if (_isEnabled) {
        //Fly in the desired direction
        TCompTransform* c_trans = get<TCompTransform>();
        VEC3 c_pos = c_trans->getPosition();

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

void TCompKinematicBullet::renderDebug() {
}

void TCompKinematicBullet::destroy() {
  if (comicfeed) {
    CEntity* onom_manager = getEntityByName("Onomatopoeia Particles");
    TMsgOnomPet msgonom;
    msgonom.type = 5.0f;
    TCompTransform* c_trans2 = get<TCompTransform>();
    msgonom.pos = c_trans2->getPosition();
    onom_manager->sendMsg(msgonom);
  }

  CHandle(this).getOwner().destroy();
  CHandle(this).destroy();
}