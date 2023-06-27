#ifndef INC_COMPONENT_RENDER_BLOOM_H_
#define INC_COMPONENT_RENDER_BLOOM_H_

#include "comp_render_blur.h"

// ------------------------------------
struct TCompRenderBloom : public TCompRenderBlur {
  CCteBuffer< TCtesBloom >      cte_bloom;
  CRenderToTexture*             rt_highlights = nullptr;
  CTexture*                     t_shine = nullptr;
  const CTechnique*             tech_filter = nullptr;
  const CTechnique*             tech_add = nullptr;
  const CTechnique*             tech_add_shines = nullptr;
  const CMesh*                  mesh = nullptr;
  VEC4                          add_weights;
  float                         threshold_min = 0.8f;
  float                         threshold_max = 1.f;
  float                         multiplier = 1.f;
  float                         emissive_bloom = 4.0f;

  TCompRenderBloom();
  ~TCompRenderBloom();

  void generateHighlights(CTexture* in_texture);
  void load(const json& j, TEntityParseContext& ctx);
  void debugInMenu();
  void addBloom();
  void setShineTexture(CTexture* tex);

};

#endif
