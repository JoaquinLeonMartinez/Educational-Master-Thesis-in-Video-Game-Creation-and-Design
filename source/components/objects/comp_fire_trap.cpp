#include "mcv_platform.h"
#include "engine.h"
#include "comp_fire_trap.h"
#include "components/common/comp_transform.h"
#include "components/common/comp_render.h"
#include "components/vfx/comp_air.h"
#include "components/ai/others/self_destroy.h"
#include "random"

std::mt19937 bt_off(1993);
std::uniform_int_distribution<int> bt_range4(0, 100);

DECL_OBJ_MANAGER("comp_fire_trap", TCompFireTrap);

void TCompFireTrap::debugInMenu() {
  ImGui::DragFloat("Offset", &_offset, 0.0f, 5.0f);
}

void TCompFireTrap::load(const json& j, TEntityParseContext& ctx) {
  _offset = bt_range4(bt_off) / 10.0f;
}

void TCompFireTrap::registerMsgs() {
  DECL_MSG(TCompFireTrap, TMsgEntityCreated, onCreate);
}

void TCompFireTrap::onCreate(const TMsgEntityCreated& msg) {
  TCompTransform* c_trans = get<TCompTransform>();
  
  //emision position and direction
  TCompBuffers* c_buff = get<TCompBuffers>();
  if (c_buff) {
    auto buf = c_buff->getCteByName("TCtesParticles");
    CCteBuffer<TCtesParticles>* data = dynamic_cast<CCteBuffer<TCtesParticles>*>(buf);
    data->emitter_center = c_trans->getPosition();
    data->emitter_dir = c_trans->getFront();  
    data->emitter_num_particles_per_spawn = 30;
    data->updateGPU();
  }
  audio = EngineAudio.playEvent("event:/Enemies/Hazards/Fire/Fire_Loop");
  audioSlow = EngineAudio.playEvent("event:/Enemies/Hazards/Fire/Fire_Loop_Slow");
  audio.setPaused(true);
  audioSlow.setPaused(true);
  audio.set3DAttributes(*c_trans);
  audioSlow.set3DAttributes(*c_trans);
}

void TCompFireTrap::enable() {
  TCompBuffers* c_buff = get<TCompBuffers>();
  if (c_buff) {
    auto buf = c_buff->getCteByName("TCtesParticles");
    CCteBuffer<TCtesParticles>* data = dynamic_cast<CCteBuffer<TCtesParticles>*>(buf);
    data->emitter_num_particles_per_spawn = 30;
    data->updateGPU();
    if (GameController.getTimeScale() < 1.0f) {
        audioSlow.setPaused(false);
    }
    else {
        audio.setPaused(false);
    }
  }
}

void TCompFireTrap::disable() {
  TCompBuffers* c_buff = get<TCompBuffers>();
  if (c_buff) {
    auto buf = c_buff->getCteByName("TCtesParticles");
    CCteBuffer<TCtesParticles>* data = dynamic_cast<CCteBuffer<TCtesParticles>*>(buf);
    data->emitter_num_particles_per_spawn = 0;
    data->updateGPU();
    audio.setPaused(true);
    audioSlow.setPaused(true);
  }
}

void TCompFireTrap::update(float dt) {
  if (_offset > 0.0f) {
    _offset -= dt;
    return;
  }
    

  if (_isEnabled) {
      if (GameController.getTimeScale() < 1.0f && audioSlow.getPaused()) {
          audioSlow.setPaused(false);
          audio.setPaused(true);
      }
      else if(GameController.getTimeScale() == 1.0f && audio.getPaused()){
          audio.setPaused(false);
          audioSlow.setPaused(true);
      }
    if (_fireTimer <= 0.f) {
      TCompTransform* c_trans = get<TCompTransform>();
      TEntityParseContext ctx;
      ctx.root_transform.setPosition(c_trans->getPosition());
      float yaw, pitch, roll;
      c_trans->getAngles(&yaw, &pitch, &roll);

      ctx.root_transform.setAngles(yaw, pitch, roll);
      parseScene("data/prefabs/bullets/enemy_fireball.json", ctx);

      //Bullet direction
      VEC3 _targetDirection = c_trans->getFront();

      //Message to the enemies
      TMsgDamage msgDamage;
      msgDamage.h_bullet = CHandle(this).getOwner();
      msgDamage.bullet_front = _targetDirection;
      msgDamage.damageType = PowerType::FIRE;
      msgDamage.senderType = EntityType::ENEMIES;
      msgDamage.targetType = EntityType::PLAYER;
      msgDamage.intensityDamage = _fireDamage;
      msgDamage.impactForce = 0.f;

      //Message to the bullet
      TMsgAssignBulletOwner msg;
      msg.h_owner = CHandle(this).getOwner();
      msg.source = c_trans->getPosition();
      msg.front = _targetDirection;
      msg.messageToTarget = msgDamage;
      ctx.entities_loaded[0].sendMsg(msg);

      _fireTimer = _fireDelay;
    }
    else {
      _fireTimer -= dt;
    }

    _disablerTimer -= dt;
    if (_disablerTimer <= 0.f) {
      _isEnabled = false;
      disable();
      _disablerTimer = _disablerDelay;
    }
  }
  else {
    _enablerTimer -= dt;
    if (_enablerTimer <= 0.f) {
      _isEnabled = true;
      enable();
      _enablerTimer = _enablerDelay;
    }
  }
}
