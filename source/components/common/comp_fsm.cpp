#include "mcv_platform.h"
#include "comp_fsm.h"
#include "entity/entity.h"
#include "entity/entity_parser.h"
#include "entity/common_msgs.h"
#include "fsm/fsm.h"

DECL_OBJ_MANAGER("fsm", TCompFSM);

void TCompFSM::load(const json& j, TEntityParseContext& ctx) 
{
  _fsm = Resources.get(j.value("file", ""))->as<CFSM>();
}

void TCompFSM::registerMsgs()
{
  DECL_MSG(TCompFSM, TMsgEntityCreated, onEntityCreated);
}

void TCompFSM::debugInMenu()
{
  _context.debugInMenu();
} 

void TCompFSM::update(float dt)
{
  _context.update(dt);
}

void TCompFSM::onEntityCreated(const TMsgEntityCreated&)
{
  _context.init(_fsm);
}

void TCompFSM::setVariable(const TVariable& var)
{
  _context.setVariable(var);
}

