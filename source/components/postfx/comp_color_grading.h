#ifndef INC_COMPONENT_COLOR_GRADING_H_
#define INC_COMPONENT_COLOR_GRADING_H_

#include "components/common/comp_base.h"

class CTexture;
class CRenderToTexture;

// ------------------------------------
struct TCompColorGrading : public TCompBase {
  bool                          enabled = true;
  float                         amount = 1.f;
  const CTexture*               lut1 = nullptr;

  void load(const json& j, TEntityParseContext& ctx);
  void debugInMenu();
  float getAmount() const {
    return enabled ? amount : 0.f;
  }

};

#endif
