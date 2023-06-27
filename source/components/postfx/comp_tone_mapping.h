#ifndef INC_COMPONENT_TONE_H_
#define INC_COMPONENT_TONE_H_

#include "components/common/comp_base.h"
#include <vector>

class CTexture;
class CRenderToTexture;

// ------------------------------------
struct TCompToneMapping : public TCompBase {
  bool enabled;
    
  CRenderToTexture*        rt = nullptr;
  const CTechnique*        tech = nullptr;
  const CMesh*             mesh = nullptr;

  void load(const json& j, TEntityParseContext& ctx);
  void debugInMenu();
  CTexture* apply(CTexture* texture);
};

#endif
