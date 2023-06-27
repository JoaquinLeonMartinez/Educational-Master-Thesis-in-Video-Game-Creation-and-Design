#pragma once

#include "components/common/comp_base.h"
#include "entity/entity.h"
#include "entity/common_msgs.h"
 
class TCompDust :  public TCompBase {
  DECL_SIBLING_ACCESS();

  bool _enabled = true;

  float ratio = 0.0f;
  float speed = 3.0f;
  VEC4 constants = VEC4::Zero;

  public:
    void load(const json& j, TEntityParseContext& ctx);
    void debugInMenu();
    void renderDebug();
    void update(float delta);
};