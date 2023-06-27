#ifndef INC_COMPONENT_RENDER_AO_H_
#define INC_COMPONENT_RENDER_AO_H_

#include "components/common/comp_base.h"
#include "entity/entity.h"
#include "render/intel/ASSAO.h"
#include <vector>

class CRenderToTexture;
class CTechnique;
class CMesh;
class CTexture;

// ------------------------------------
class TCompRenderAO : public TCompBase {
  CRenderToTexture*       rt_output = nullptr;
  const CTexture*         white = nullptr;
  const CTechnique*       tech = nullptr;
  const CMesh*            mesh = nullptr;
  bool                    enabled = true;
  float                   amount = 1.f;
  float                   radius = 3.f;
  float                   zrange_discard = 0.02f;
  float                   amount_spreading = 0.85f;
  int                     xres = 0;
  int                     yres = 0;
  
  bool                    use_assao = true;
  ASSAO_Settings          settings;
  ASSAO_InputsDX11        assao_inputs;
  
  DECL_SIBLING_ACCESS();
  void  prepareRenderTarget();

public:

  void  load(const json& j, TEntityParseContext& ctx);
  void  debugInMenu();
  const CTexture* compute(CTexture* linear_depth_texture, CTexture* normals, ASSAO_Effect* fx);
};

#endif
