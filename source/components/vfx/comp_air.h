#pragma once

#include "components/common/comp_base.h"
#include "entity/entity.h"
#include "entity/common_msgs.h"
 
class TCompAir :  public TCompBase {
  DECL_SIBLING_ACCESS();

  VEC4 constants = VEC4::Zero;

  public:
    float d = 0.0f;
    float len = 0.0f;
    float speed = 9.0f;
    float destroy = 0.0f;
    float ratio = 0.0f;

    void load(const json& j, TEntityParseContext& ctx);
    void debugInMenu();
    void renderDebug();
    void update(float delta);
};