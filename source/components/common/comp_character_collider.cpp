#include "mcv_platform.h"
#include "comp_character_collider.h"
#include "entity/entity_parser.h"
#include "engine.h"
#include "modules/module_physics.h"

DECL_OBJ_MANAGER("character_collider", TCompCharacterCollider);

using namespace physx;

void TCompCharacterCollider::debugInMenu() {
}

void TCompCharacterCollider::load(const json& j, TEntityParseContext& ctx) {
  jconfig = j;
  
}

void TCompCharacterCollider::renderDebug() {
}

void TCompCharacterCollider::registerMsgs() {
  DECL_MSG(TCompCharacterCollider, TMsgEntityCreated, onEntityCreated);
}

void TCompCharacterCollider::onEntityCreated(const TMsgEntityCreated& msg) {
  dbg("Hi from onEntityCreated\n");
  EnginePhysics.CreateCharacterActor(*this);
}