#pragma once

#include "components/common/comp_base.h"
#include "entity/entity.h"
 
class TCompBoomBillboard :  public TCompBase {
  DECL_SIBLING_ACCESS();

  VEC4 constants = VEC4::Zero;
  float timer = 0.0f;

  public:
    void load(const json& j, TEntityParseContext& ctx);
    void debugInMenu();
    void renderDebug();
    void update(float delta);
};