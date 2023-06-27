#ifndef INC_COMP_RAGDOLL_H_
#define INC_COMP_RAGDOLL_H_

#include "components/common/comp_base.h"
#include "PxPhysicsAPI.h"
#include "entity/entity.h"
#include "entity/common_msgs.h"
#include "physics/ragdoll.h"

class CEntity;

struct TCompRagdoll : public TCompBase {
  TRagdoll ragdoll;
  static void registerMsgs();

  ~TCompRagdoll();

  void renderInMenu();
  void load(const json& j, TEntityParseContext& ctx);
  
  void update(float elapsed);
  void updateRagdollFromSkeleton();
  void activateRagdoll();
  void deactivateRagdoll();

  void onCreated(const TMsgEntityCreated&);
  void debugInMenu();
  void renderDebug();

	  bool active = false;
  DECL_SIBLING_ACCESS();
  //move 
};


#endif
