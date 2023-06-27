#pragma once

#include "components/common/comp_base.h"
#include "entity/entity.h"
#include "geometry/curve.h"

class TCompMorphAnimation : public TCompBase
{
  DECL_SIBLING_ACCESS();

  struct TMorphWeights {
	  float a = 0, b = 0, c = 0, d = 0;
  };
  TMorphWeights morph_weights;

public:
  void debugInMenu();
  void load(const json& j, TEntityParseContext& ctx);
  void renderDebug();
  void update(float dt);
  void playMorph();
  void stopMorph();
  void setIncrement(float i);
  void updateMorphData(float amount);
  float getWeight();
  bool play_ = false;

private:
	 
	 float increment = 0.5;	 
	 
};

