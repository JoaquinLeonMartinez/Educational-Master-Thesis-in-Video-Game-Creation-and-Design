#pragma once

#include "components/common/comp_base.h"
#include "entity/entity.h"
 
class TCompBillboardXZ :  public TCompBase {
  DECL_SIBLING_ACCESS();

  public:
    void load(const json& j, TEntityParseContext& ctx);
    void debugInMenu();
    void renderDebug();
    void update(float delta);
};