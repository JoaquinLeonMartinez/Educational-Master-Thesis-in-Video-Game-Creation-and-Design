#ifndef INC_COMPONENT_DAMAGED_EFFECT_H_
#define INC_COMPONENT_DAMAGED_EFFECT_H_

#include "components/common/comp_base.h"
#include "entity/entity.h"
#include "entity/common_msgs.h"

class CTexture;
class CRenderToTexture;

// ------------------------------------
struct TCompDamagedEffect : public TCompBase {
  bool enabled;
  float grade = 0.0f;

  CRenderToTexture* rt = nullptr;
  const CTechnique* tech = nullptr;
  const CMesh* mesh = nullptr;
  const CTexture* dither_tex = nullptr;

  void load(const json& j, TEntityParseContext& ctx);
  void debugInMenu();
  CTexture* apply(CTexture* texture);
  void update(float dt);
  static void registerMsgs();
  void onCollision(const TMsgOnContact& msg);
};

#endif
