#pragma once

#include "comp_base.h"
 
class TCompTransform : public CTransform, public TCompBase {
public:
  void load(const json& j, TEntityParseContext& ctx);
  void debugInMenu();
  void renderDebug();
  void set(const CTransform& new_tmx);
};