#pragma once

#include "components/common/comp_base.h"
#include "entity/entity.h"
#include "geometry/curve.h"

class TCompRemovableCamera : public TCompBase
{
  DECL_SIBLING_ACCESS();

public:
  void debugInMenu();
  void load(const json& j, TEntityParseContext& ctx);
  void update(float dt);
  void renderDebug();

private:
  
};

