#ifndef INC_COMPONENT_ANTIALIASING_H_
#define INC_COMPONENT_ANTIALIASING_H_

#include "components/common/comp_base.h"
#include <vector>

class CTexture;
class CRenderToTexture;

// ------------------------------------
struct TCompAntialiasing : public TCompBase {
  bool enabled;
  float screenWidth;
  float screenHeight;
  float sub_pix = 0.4f;
  float edge_threshold = 0.0f;
  float edge_threshold_min = 0.0f;
  CRenderToTexture*        rt = nullptr;
  const CTechnique*        tech = nullptr;
  const CMesh*             mesh = nullptr;

  void load(const json& j, TEntityParseContext& ctx);
  void debugInMenu();
  CTexture* apply(CTexture* texture);
};

#endif
