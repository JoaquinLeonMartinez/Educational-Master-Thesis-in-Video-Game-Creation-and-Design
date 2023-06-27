#pragma once

#include "components/common/comp_base.h"
#include "entity/entity.h"
#include "entity/entity_parser.h"
#include "entity/common_msgs.h"

class TCompMadnessPuddle : public TCompBase
{
  DECL_SIBLING_ACCESS();

public:
  void debugInMenu();
  void load(const json& j, TEntityParseContext& ctx);
  void update(float dt);

  static void registerMsgs();

private:
  bool enabled = false;
  float ratio = 1.0f;
  VEC3 constants = VEC3::Zero;
  void onPlayerAttack(const TMsgDamage & msg);
  void onCreation(const TMsgEntityCreated& msg);
};

