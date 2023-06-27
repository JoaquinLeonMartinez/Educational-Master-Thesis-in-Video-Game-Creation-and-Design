#include "mcv_platform.h"
#include "comp_fire.h"
#include "components/common/comp_transform.h"
#include "entity/entity_parser.h"
#include "engine.h"
#include "input/input.h"
#include "components/controllers/character/comp_character_controller.h"
#include "input/module_input.h"
#include "comp_madness.h"
#include "components/common/physics/comp_collider.h"

#include "skeleton/comp_skeleton.h"
#include "cal3d/cal3d.h"
#include "skeleton/cal3d2engine.h"

using namespace physx;

DECL_OBJ_MANAGER("comp_fire", TCompFireController);

void TCompFireController::debugInMenu() {
}

void TCompFireController::load(const json& j, TEntityParseContext& ctx) {
    _fireWidth = j.value("_fireWidth", _fireWidth);
    _fireHeight = j.value("_fireHeight", _fireHeight);
    _fireDistance = j.value("_fireDistance", _fireDistance);
    _fireDamage = j.value("_fireDamage", _fireDamage);

    _buffDuration = j.value("_buffDuration", _buffDuration);

    bone_name = j.value("bone", "Bone_indice01_der");
    parent_name = j.value("parent", "Player");
    assert(!bone_name.empty());
    assert(!parent_name.empty());

    CEntity* e_parent = ctx.findEntityByName("Player");
    if (e_parent)
        h_skeleton = e_parent->get<TCompSkeleton>();
    srand(1234);

    _hitAudio = EngineAudio.playEvent(_hitAudioName);
    _hitAudio.stop();
}

void TCompFireController::registerMsgs() {
    DECL_MSG(TCompFireController, TMsgSoundRequest, onSoundRequest);
}

void TCompFireController::onSoundRequest(const TMsgSoundRequest& msg) {
    if (msg.name == _hitAudioName && !_hitAudio.isPlaying()) {
        _hitAudio = EngineAudio.playEvent(_hitAudioName);
    }
}

void TCompFireController::renderDebug() {
    
}

void TCompFireController::enable() {
    TCompMadnessController* m_c = get<TCompMadnessController>();
    if (!_enabled && m_c->getRemainingMadness() > m_c->getPowerCost(PowerType::FIRE)) {
        _enabled = true;
        audioEffect = EngineAudio.playEvent("event:/Character/Powers/Fire/Fire");
        TCompCharacterController* c_c = get<TCompCharacterController>();
        if (c_c->getMovmentDirection() != VEC3().Zero) {
          TCompPlayerAnimator* playerAnima = get<TCompPlayerAnimator>();
          playerAnima->playAnimation(TCompPlayerAnimator::FIRE_START_PARTIAL, 1.0f);
        }
        else {
          TCompPlayerAnimator* playerAnima = get<TCompPlayerAnimator>();
          playerAnima->playAnimation(TCompPlayerAnimator::FIRE_START, 1.0f);
        }
    }
}

void TCompFireController::disable() {
    if (_enabled) {
        _enabled = false;
        audioEffect.stop();
        TCompCharacterController* c_c = get<TCompCharacterController>();
        if (c_c->getMovmentDirection() != VEC3().Zero) {
          TCompPlayerAnimator* playerAnima = get<TCompPlayerAnimator>();
          playerAnima->playAnimation(TCompPlayerAnimator::FIRE_END_PARTIAL, 1.0f);
        }
        else {
          TCompPlayerAnimator* playerAnima = get<TCompPlayerAnimator>();
          playerAnima->playAnimation(TCompPlayerAnimator::FIRE_END, 1.0f);
        }
    }
}

void TCompFireController::update(float dt) {

    if (_buffRemaining > 0.f) {
        _buffRemaining -= Time.delta_unscaled;
    }
    else {
        _isBuffed = false;
    }


    //If we're not enabled
    if (!_enabled) {
        return;
    }

    //Update spheres' position if there are any
    TCompSkeleton* c_skel = h_skeleton;
    if (c_skel == nullptr) {
        // Search the parent entity by name
        CEntity* e_entity = getEntityByName("Player");
        if (!e_entity)
            return;
        // The entity I'm tracking should have an skeleton component.
        h_skeleton = e_entity->get<TCompSkeleton>();
        assert(h_skeleton.isValid());
        c_skel = h_skeleton;
    }
    if (bone_id == -1) {
        bone_id = c_skel->model->getCoreModel()->getCoreSkeleton()->getCoreBoneId(bone_name);
        // The skeleton don't have the bone with name 'bone_name'
        assert(bone_id != -1);
    }
    // Access to the bone 'bone_id' of the skeleton
    auto cal_bone = c_skel->model->getSkeleton()->getBone(bone_id);
    QUAT boneRotation = Cal2DX(cal_bone->getRotationAbsolute());
    VEC3 bonePosition = Cal2DX(cal_bone->getTranslationAbsolute());

    //If trigger just released or we've run out of madness, disable
    TCompMadnessController* m_c = get<TCompMadnessController>();
    if (EngineInput["fire_attack_"].justReleased() || (m_c->getRemainingMadness() < m_c->getPowerCost(PowerType::FIRE) * Time.delta_unscaled && !GameController.getGodMode())) {
        disable();
        return;
    }

    //At this point we're enabled and the trigger is being held
    if (_fireTimer <= 0.f) {
        attack(bonePosition);
        _fireTimer = _fireDelay;
        TCompCharacterController* c_c = get<TCompCharacterController>();
        if (c_c->getMovmentDirection() != VEC3().Zero) {
          TCompPlayerAnimator* playerAnima = get<TCompPlayerAnimator>();
          playerAnima->playAnimation(TCompPlayerAnimator::FIRE_LOOP_PARTIAL, 1.0f);
        }
        else {
          TCompPlayerAnimator* playerAnima = get<TCompPlayerAnimator>();
          playerAnima->playAnimation(TCompPlayerAnimator::FIRE_LOOP, 1.0f);
        }
    }
    else {
        _fireTimer -= Time.delta_unscaled;
    }
}

bool TCompFireController::isBuffActive() {
    return _isBuffed;
}

void TCompFireController::comboAttack(VEC3 pos) {
    dbg("TELEPORT-CHILI COMBO.\n");
    //Generate damage sphere
    TMsgDamage msg;
    msg.h_bullet = CHandle(this).getOwner();
    msg.senderType = EntityType::PLAYER;
    msg.targetType = EntityType::ENEMIES;
    msg.damageType = PowerType::FIRECOMBO;
    msg.position = pos;
    msg.intensityDamage = 50.0f;
    msg.impactForce = 20.f;
    float radius = 5.f;
    GameController.generateDamageSphere(pos, radius, msg, "enemy");
    CHandle c = GameController.spawnPrefab("data/prefabs/props/explosion_sphere.json", pos, QUAT().Identity, radius);
    CEntity* e_sphere = c;

    TCompBuffers* c_buff = e_sphere->get<TCompBuffers>();
    if (c_buff) {
      auto buf = c_buff->getCteByName("TCtesParticles");
      CCteBuffer<TCtesParticles>* data = dynamic_cast<CCteBuffer<TCtesParticles>*>(buf);
      data->emitter_center = pos + VEC3::Up;
      data->updateGPU();
    }
}

void TCompFireController::attack(VEC3 origin) {
    _isBuffed = true;
    _buffRemaining = _buffDuration;
    TCompTransform* c_trans = get<TCompTransform>();

    //Bullet origin
    VEC3 firingPosition = origin;

    TEntityParseContext ctx;
    ctx.root_transform.setPosition(firingPosition);
    float yaw, pitch, roll;
    ctx.root_transform.getAngles(&yaw, &pitch, &roll);

    yaw = rand();
    pitch = rand();
    ctx.root_transform.setAngles(yaw, pitch, roll);
    parseScene("data/prefabs/bullets/fireball.json", ctx);

    //Bullet direction
    VEC3 _targetDirection = c_trans->getFront();
    TCompCharacterController* c_controller = get< TCompCharacterController>();
    if (c_controller->aiming) {
        TCompTransform* camera_transform = ((CEntity*)getEntityByName("PlayerCamera"))->get<TCompTransform>();
        _targetDirection = camera_transform->getFront();
    }

    //Message to the enemies
    TMsgDamage msgDamage;
    msgDamage.h_bullet = CHandle(this).getOwner();
    msgDamage.bullet_front = _targetDirection;
    msgDamage.damageType = PowerType::FIRE;
    msgDamage.senderType = EntityType::PLAYER;
    msgDamage.targetType = EntityType::ENEMIES;
    msgDamage.intensityDamage = _fireDamage;
    msgDamage.impactForce = _fireImpactForce;

    //Message to the bullet
    TMsgAssignBulletOwner msg;
    msg.h_owner = CHandle(this).getOwner();
    msg.source = firingPosition;
    msg.front = _targetDirection;
    msg.messageToTarget = msgDamage;
    ctx.entities_loaded[0].sendMsg(msg);
}