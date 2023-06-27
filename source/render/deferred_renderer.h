#ifndef INC_RENDER_DEFERRED_RENDERER_H_
#define INC_RENDER_DEFERRED_RENDERER_H_

#include "render/textures/render_to_texture.h"
#include "intel/ASSAO.h"

class CDeferredRenderer {

  int lightPointsRendered = 0, lightDirRendered = 0, lightDirShadowRendered = 0;

public:

  int               xres = 0, yres = 0;
  CRenderToTexture* rt_normals = nullptr;
  CRenderToTexture* rt_albedos = nullptr;
  CRenderToTexture* rt_depth = nullptr;
  CRenderToTexture* rt_self_illum = nullptr;
  CRenderToTexture* rt_acc_light = nullptr;
  ASSAO_Effect*     assao_fx = nullptr;

  void renderGBuffer();
  void renderGBufferDecals();
  void renderAccLight();
  void renderAmbientPass();
  void renderDirectionalLightsWithShadows();
  void renderDirectionalLightsWithoutShadows();
  void renderFakeVolumetricLights();
  void renderPointLights();
  void renderSkyBox() const;
  void renderAO(CHandle h_camera) const;

public:

  CDeferredRenderer();
  bool create( int xres, int yres );
  void destroy();
  void render( CRenderToTexture* rt_destination, CHandle h_camera );
  void renderInMenu();
};


#endif

