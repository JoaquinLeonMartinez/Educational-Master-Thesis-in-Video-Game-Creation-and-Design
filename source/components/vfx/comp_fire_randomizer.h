#pragma once

#include "components/common/comp_base.h"
#include "entity/entity.h"
#include "entity/common_msgs.h"
 
class TCompFireRandomizer :  public TCompBase {
  DECL_SIBLING_ACCESS();

  VEC4 constants = VEC4::Zero;
  void onCreation(const TMsgEntityCreated& msg);

  public:
    void load(const json& j, TEntityParseContext& ctx);
    void debugInMenu();
    void renderDebug();
    void update(float delta);
    static void registerMsgs();
};