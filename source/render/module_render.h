#pragma once

#include "modules/module.h"
#include "render/deferred_renderer.h"

class CModuleRender : public IModule
{
  VEC4 clear_color = VEC4(0.142f, 0.142f, 0.142f, 1);
  void renderEntities();
  float new_ambient = 0.3f;

  CHandle h_camera;
  CCamera camera;
  CCamera camera_ortho;

  CDeferredRenderer* deferred = nullptr;
  CRenderToTexture* deferred_output = nullptr;
  CRenderToTexture* ui_output = nullptr;
  CRenderToTexture* shine_output = nullptr;

  void uploadSkinMatricesToGPU();
  void parsePipelines(const std::string& filename);
  bool setupDeferredOutput();
  void renderShine();

public:
  CModuleRender(const std::string& name)
    : IModule(name)
  {}
  bool start() override;
  void stop() override;
  void renderInMenu() override;
  void update(float dt) override;

  void beginImgui();
  void endImgui();
  void generateFrame();
  void onResolutionUpdated();
  void setNewAmbient(float a) { new_ambient = a; }

  CHandle getCamera() { return h_camera; }
};
