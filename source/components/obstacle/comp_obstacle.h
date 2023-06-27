#pragma once

#include "components/common/comp_base.h"
#include "entity/entity.h"
#include "entity/entity_parser.h"
#include "entity/common_msgs.h"

class TCompObstacle : public TCompBase
{
  DECL_SIBLING_ACCESS();

public:
  void debugInMenu();
  void load(const json& j, TEntityParseContext& ctx);
  void update(float dt);
  void onEntityCreated(const TMsgEntityCreated& msg);
  void addObstacle();
  void removeObstacle();
  static void registerMsgs();

private:
	 VEC3 pos;
	 float radius = 2.5f;
	 float height = 2.5f;

	 int referenciaObstacle = -1;
};

