#ifndef INC_COMPONENT_CHROMATICABRR_H_
#define INC_COMPONENT_CHROMATICABRR_H_

#include "components/common/comp_base.h"
#include "entity/entity.h"
#include <vector>

class CTexture;
class CRenderToTexture;

// ------------------------------------
struct TCompChromaticAberration : public TCompBase {
  bool enabled = true;
  bool update_amount = true;
  float amount = 0.f;
  float water_effect_speed = 0.2f;
  float distortion_speed = 1.f;
  
  CRenderToTexture*        rt = nullptr;
  const CTechnique*        tech = nullptr;
  const CMesh*             mesh = nullptr;

  void load(const json& j, TEntityParseContext& ctx);
  void debugInMenu();
  CTexture* apply(CTexture* texture);

  CHandle h_player;

public:
  void update(float delta);
};

#endif
