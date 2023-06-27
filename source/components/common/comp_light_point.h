#pragma once

#include "comp_base.h"
#include "entity/entity.h"
#include "entity/common_msgs.h"

class TCompLightPoint : public TCompBase {

  // Light params
  VEC4            color = VEC4(1, 1, 1, 1);
  float           intensity = 1.0f;
  float           radius = 1.0f;
  AABB            aabb;

  void onDefineLocalAABB(const TMsgDefineLocalAABB& msg);

public:
  void debugInMenu();
  void renderDebug();
  void load(const json& j, TEntityParseContext& ctx);

  static void registerMsgs();
  DECL_SIBLING_ACCESS();

  void activate();
  MAT44 getWorld();
  void setIntensity(float i) {
	  intensity = i; 
  }
};