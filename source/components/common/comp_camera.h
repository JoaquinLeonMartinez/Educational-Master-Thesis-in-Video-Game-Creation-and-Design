#pragma once

#include "comp_base.h"
#include "entity/entity.h"
#include "geometry/camera.h"
 
class TCompCamera : public CCamera, public TCompBase {
public:
  void load(const json& j, TEntityParseContext& ctx);
  void debugInMenu();
  void renderDebug();
  void update(float delta);
  DECL_SIBLING_ACCESS();
};