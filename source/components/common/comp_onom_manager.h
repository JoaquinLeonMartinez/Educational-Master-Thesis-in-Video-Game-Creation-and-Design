#pragma once

#include "components/common/comp_base.h"
#include "entity/entity.h"
#include "entity/common_msgs.h"
#include "modules/module_physics.h"

using namespace physx;

class TCompOnomManager : public TCompBase {
  std::vector< TMsgOnomPet> petitions;
  CHandle camera;
  bool wham = true;

  void onPetition(const TMsgOnomPet& msg);

public:
  void load(const json& j, TEntityParseContext& ctx);
  void debugInMenu();
  void renderDebug();
  void update(float delta);

  static void registerMsgs();

  DECL_SIBLING_ACCESS();
};