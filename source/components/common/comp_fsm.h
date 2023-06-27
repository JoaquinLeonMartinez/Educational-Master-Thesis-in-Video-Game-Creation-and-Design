#ifndef INC_COMP_FSM_H_
#define INC_COMP_FSM_H_

#include "geometry/transform.h"
#include "comp_base.h"
#include "entity/entity.h"
#include "fsm/context.h"

struct TMsgEntityCreated;

struct TCompFSM : public TCompBase
{
  DECL_SIBLING_ACCESS();

  void load(const json& j, TEntityParseContext& ctx);
  void update(float dt);
  void debugInMenu();
  static void registerMsgs();

  void onEntityCreated(const TMsgEntityCreated&);

  void setVariable(const TVariable& var);

private:
  const CFSM* _fsm = nullptr;
  CFSMContext _context;
};

#endif
