#pragma once

#include "components/common/comp_base.h"
#include "entity/entity.h"
#include "entity/entity_parser.h"
#include "entity/common_msgs.h"
#include "components/common/physics/comp_rigid_body.h"
#include "components/common/comp_transform.h"
class TCompBalance : public TCompBase
{
  DECL_SIBLING_ACCESS();

public:
  void debugInMenu();
  void load(const json& j, TEntityParseContext& ctx);
  void update(float dt);
  void balanceo();
  void cambioTexturaJoint(bool apagado);
  static void registerMsgs();

private:
	bool balanceoDone = false;
	TCompTransform* c_trans;
	void onCreate(const TMsgEntityCreated & msg);
  void onDamage(const TMsgDamage& msg);
};

