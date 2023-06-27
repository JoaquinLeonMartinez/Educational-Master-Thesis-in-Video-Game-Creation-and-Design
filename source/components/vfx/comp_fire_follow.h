#pragma once

#include "components/common/comp_base.h"
#include "components/common/comp_transform.h"
#include "components/common/comp_camera.h"
#include "entity/entity.h"
#include "render/render.h"
#include "engine.h"


using namespace physx;

class TCompFireFollow : public TCompBase {

  DECL_SIBLING_ACCESS();

public:
  void update(float dt);
  void load(const json& j, TEntityParseContext& ctx);
  void debugInMenu();

};