#pragma once

#include "comp_base.h"
#include "entity/entity.h"

struct TMsgEntityCreated;

namespace particles
{
  struct TEmitter;
}

struct TCompParticles : public TCompBase
{
  DECL_SIBLING_ACCESS();

  void load(const json& j, TEntityParseContext& ctx);
  void update(float dt);
  void debugInMenu();
  static void registerMsgs();

  void onEntityCreated(const TMsgEntityCreated&);
  
private:
  const particles::TEmitter* _emitter = nullptr;
};
