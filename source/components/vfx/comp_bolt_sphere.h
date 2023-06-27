#pragma once

#include "components/common/comp_base.h"
#include "entity/entity.h"
#include "entity/common_msgs.h"
 
class TCompBoltSphere :  public TCompBase {
  DECL_SIBLING_ACCESS();

  VEC3 target_position;
  void onCreation(const TMsgEntityCreated& msg); 
  float speed = 1.0f;

  public:
    void load(const json& j, TEntityParseContext& ctx);
    void debugInMenu();
    void renderDebug();
    void update(float delta);
    void setTargetPos(VEC3 target) { target_position = target; }

    static void registerMsgs();
};