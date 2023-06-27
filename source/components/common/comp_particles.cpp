#include "mcv_platform.h"
#include "comp_particles.h"
#include "entity/entity.h"
#include "entity/entity_parser.h"
#include "entity/common_msgs.h"
#include "particles/particle_emitter.h"
#include "particles/module_particles.h"
#include "engine.h"

DECL_OBJ_MANAGER("particles", TCompParticles);

void TCompParticles::load(const json& j, TEntityParseContext& ctx) 
{
  _emitter = Resources.get(j.value("file", ""))->as<particles::TEmitter>();
}

void TCompParticles::registerMsgs()
{
  DECL_MSG(TCompParticles, TMsgEntityCreated, onEntityCreated);
}

void TCompParticles::debugInMenu()
{
  //not safe
  particles::TEmitter* b = const_cast<particles::TEmitter*>(_emitter);
  b->renderInMenu();
} 

void TCompParticles::update(float dt)
{
  
}

void TCompParticles::onEntityCreated(const TMsgEntityCreated&)
{
  Engine.getParticles().launchSystem(_emitter, CHandle(this).getOwner());
}
