#pragma once

#include "components/common/comp_base.h"
#include "entity/entity.h"
#include "entity/common_msgs.h"
 
class TCompDeathBillboard :  public TCompBase {
  DECL_SIBLING_ACCESS();

  CHandle particles;
  VEC4 constants = VEC4::Zero;
  bool done = false;

  public:
    void load(const json& j, TEntityParseContext& ctx);
    void debugInMenu();
    void renderDebug();
    void update(float delta);
    void setParticles(CHandle t) { particles = t; }
};