#pragma once

#include "components/common/comp_base.h"
 
class TCompUi :  public TCompBase {
	float windowSize = 20.f;
	float thick = 10.f;
	ImVec4 col = ImVec4(1, 1, 1, 1);
	float sizeExteriorCross = 1.f;
	float sizeInteriorCross = 1.f;

public:
  void load(const json& j, TEntityParseContext& ctx);
  void debugInMenu();
  void renderDebug();
  void update(float delta);
};