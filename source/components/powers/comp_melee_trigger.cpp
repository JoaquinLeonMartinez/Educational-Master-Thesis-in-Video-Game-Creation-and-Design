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
#include "comp_melee_trigger.h"

using namespace physx;

DECL_OBJ_MANAGER("comp_melee_trigger", TCompMeleeTrigger);
void TCompMeleeTrigger::debugInMenu() {
  ImGui::LabelText("Enemies: ", "%d", _currentEnemies.size());
}

void TCompMeleeTrigger::load(const json& j, TEntityParseContext& ctx) {
}

void TCompMeleeTrigger::registerMsgs() {
    DECL_MSG(TCompMeleeTrigger, TMsgEntityTriggerEnter, onCollision);
    DECL_MSG(TCompMeleeTrigger, TMsgEntityTriggerExit, onCollisionOut);
}


void TCompMeleeTrigger::onCollision(const TMsgEntityTriggerEnter& msg) {
  CEntity* player = getEntityByName("Player");
  TCompCharacterController* c_c = player->get<TCompCharacterController>();
  if (c_c == nullptr)
    return;
  bool attacking = c_c->getAttacking();
  if (attacking && std::find(_enemiesHit.begin(), _enemiesHit.end(), msg.h_entity) == _enemiesHit.end()) { //si atacas y no le has golpeado aun...
    TMsgMeleeHit meleeHit;
    meleeHit.h_entity = msg.h_entity;
    player->sendMsg(meleeHit);
    EngineAudio.playEvent("event:/Character/Attacks/Melee_Hit");
    expansiveWave();
    _enemiesHit.push_back(msg.h_entity);
  }
  else if(!attacking && std::find(_currentEnemies.begin(), _currentEnemies.end(), msg.h_entity) == _currentEnemies.end()) { // si no estas atacando y no "estaba" ya dentro, lo metes en los candidatos
    _currentEnemies.push_back(msg.h_entity);
  }
}

void TCompMeleeTrigger::onCollisionOut(const TMsgEntityTriggerExit& msg) {
  for (int i = 0; i < _currentEnemies.size(); i++) {
    if (msg.h_entity == _currentEnemies[i]) {
      _currentEnemies.erase(_currentEnemies.begin() + i);
      return;
    }
  }
}


void TCompMeleeTrigger::update(float delta) {
  CEntity* player = getEntityByName("Player");
  TCompCharacterController* c_c = player->get<TCompCharacterController>();
  if (c_c == nullptr)
    return;
  bool attacking = c_c->getAttacking();
  if (attacking && !before) {
    if (_currentEnemies.size() > 0) {
      EngineAudio.playEvent("event:/Character/Attacks/Melee_Hit");
      expansiveWave();
      expansiveWaveJoints();
      hitDisplay = true;
    }
  }

  if (attacking) { // si estas atacando
    if (_currentEnemies.size() > 0 && !hitDisplay) {
      EngineAudio.playEvent("event:/Character/Attacks/Melee_Hit");
      expansiveWave();
      expansiveWaveJoints();
      hitDisplay = true;
    }

    for (int i = 0; i < _currentEnemies.size(); i++) {
      TMsgMeleeHit meleeHit;
      meleeHit.h_entity = _currentEnemies[i];
      if (meleeHit.h_entity.isValid()) {
        player->sendMsg(meleeHit);
      }
      _currentEnemies.erase(_currentEnemies.begin() + i);
      i--;
      _enemiesHit.push_back(meleeHit.h_entity);
    }
  }
  else if (!attacking && before) {
    _enemiesHit.clear();
    _currentEnemies.clear();
    hitDisplay = false;
  }

  before = attacking;
}

void TCompMeleeTrigger::renderDebug() {
}

void TCompMeleeTrigger::expansiveWave() {
  CEntity* player = getEntityByName("Player");
  TCompTransform* c_trans = player->get<TCompTransform>();

  CEntity* onom_manager = getEntityByName("Onomatopoeia Particles");
  TMsgOnomPet msgonom;
  msgonom.type = 1.0f;
  msgonom.pos = c_trans->getPosition();
  onom_manager->sendMsg(msgonom);


  PxVec3 pos = VEC3_TO_PXVEC3(c_trans->getPosition());
  PxQuat ori = QUAT_TO_PXQUAT(c_trans->getRotation());
  PxSphereGeometry geometry(7.0f);
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
        msg.intensityDamage = 8.0f;
        msg.impactForce = 8.0f;
        msg.damageType = MELEE;
        msg.targetType = ENEMIES;
        entityContact->sendMsg(msg);
      }
    }
  }
}


void TCompMeleeTrigger::expansiveWaveJoints() {
  CEntity* player = getEntityByName("Player");
  TCompTransform* c_trans = player->get<TCompTransform>();

  VEC3 pos3 = c_trans->getPosition() + VEC3(0, 10, 0);

  PxVec3 pos = VEC3_TO_PXVEC3(pos3);
  PxQuat ori = QUAT_TO_PXQUAT(c_trans->getRotation());
  PxSphereGeometry geometry(25.0f);
  const PxU32 bufferSize = 16;
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