#pragma once

#include "components/common/comp_base.h"
#include "entity/entity.h"
#include "entity/common_msgs.h"

class TCompBulletController : public TCompBase {

  float speed = 1.0f;
  VEC3  front;
  float collision_radius = 2.f;
  CHandle h_sender;

  DECL_SIBLING_ACCESS();
  void onBulletInfoMsg(const TMsgAssignBulletOwner& msg);

public:
  void update(float dt);
  void load(const json& j, TEntityParseContext& ctx);
  void debugInMenu();
  static void registerMsgs();
};