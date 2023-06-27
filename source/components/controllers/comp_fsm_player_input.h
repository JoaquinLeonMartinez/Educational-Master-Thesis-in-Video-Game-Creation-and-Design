#pragma once

#include "components/common/comp_base.h"
#include "entity/entity.h"

class TCompFSMPlayerInput : public TCompBase
{
  DECL_SIBLING_ACCESS();

public:
  void update(float dt);
};
