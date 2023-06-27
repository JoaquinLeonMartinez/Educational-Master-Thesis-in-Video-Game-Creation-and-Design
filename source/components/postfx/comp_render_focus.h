#ifndef INC_COMPONENT_RENDER_FOCUS_H_
#define INC_COMPONENT_RENDER_FOCUS_H_

#include <vector>
#include "components/common/comp_base.h"

class CTexture;
class CRenderToTexture;

// ------------------------------------
struct TCompRenderFocus : public TCompBase {
  bool                     enabled;
  CCteBuffer< TCtesFocus > cte_focus;
  CRenderToTexture*        rt = nullptr;
  const CTechnique*        tech = nullptr;
  const CMesh*             mesh = nullptr;

  TCompRenderFocus();
  ~TCompRenderFocus();

  void load(const json& j, TEntityParseContext& ctx);
  void debugInMenu();
  CTexture* apply(CTexture* focus_texture, CTexture* blur_texture);
};

#endif
