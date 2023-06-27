#include "mcv_platform.h"
#include "render/render.h"
#include "module_render.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"
#include "windows/app.h"
#include "render/primitives.h"
#include "engine.h"
#include "components/common/comp_render.h"
#include "components/common/comp_transform.h"
#include "components/common/comp_camera.h"
#include "components/common/comp_light_dir.h"
#include "components/postfx/comp_render_blur.h"
#include "components/postfx/comp_render_radial_blur.h"
#include "components/postfx/comp_render_focus.h"
#include "components/postfx/comp_render_bloom.h"
#include "components/postfx/comp_color_grading.h"
#include "components/postfx/comp_antialiasing.h"
#include "components/postfx/comp_chromatic_aberration.h"
#include "components/postfx/comp_tone_mapping.h"
#include "components/postfx/comp_damaged_effect.h"
#include "components/postfx/comp_vignetting.h"
#include "components/common/comp_culling.h"
#include "components/common/comp_aabb.h"
#include "skeleton/comp_skeleton.h"
#include "render/render.h"
#include "render/meshes/mesh.h"
#include "render/textures/material.h"
#include "render/render_manager.h"
#include "ui/module_ui.h"
#include "render/compute/compute_shader.h"


bool CModuleRender::start() { 

  auto& app = CApplication::get();

  deferred = new CDeferredRenderer();
  deferred_output = new CRenderToTexture();
  ui_output = new CRenderToTexture();
  shine_output = new CRenderToTexture();

  setupDeferredOutput();
  onResolutionUpdated();

  createRenderUtils();
  parsePipelines("data/shaders/techniques.json");
  createRenderPrimitives();

  // Setup Platform/Renderer bindings
  {
    PROFILE_FUNCTION("Imgui");
    ImGui::CreateContext();
    ImGui_ImplWin32_Init(app.getHandle());
    ImGui_ImplDX11_Init(Render.device, Render.ctx);
  }

  // Initial values
  ctes_shared.GlobalRenderOutput = RO_COMPLETE;
  ctes_shared.GlobalAmbientBoost = 0.3f;
  ctes_shared.GlobalExposureAdjustment = 1.0f;
  ctes_shared.CoffeeRatio = 0.f;
  ctes_shared.Actual_dt = 0.f;

  ctes_shared.ao_power = 2.0f;
  ctes_shared.shadow_ramp = 0.3f;
  ctes_shared.color_intensity = 1.0f;
  ctes_shared.brush_size = 1.0f;

  ctes_shared.brush_rotation = 0.0f;
  ctes_shared.specular_ramp = 0.3f;
  ctes_shared.specular_brush_ramp = 0.3f;
  ctes_shared.specular_strength = 1.0f;

  return true;
}

void CModuleRender::onResolutionUpdated() {
  assert(deferred);
  deferred->create(Render.width, Render.height);
}

void CModuleRender::parsePipelines(const std::string& filename) {
  PROFILE_FUNCTION("parsePipelines");
  TFileContext fc(filename);

  json j = loadJson(filename);

  for (auto it : j.items()) {
    PROFILE_FUNCTION_COPY_TEXT(it.key().c_str());
    TFileContext fce(it.key());

    const json& j = it.value();

    const CResourceType* res_type = nullptr;
    IResource* new_res = nullptr;
    if (j.count("cs")) {
      res_type = getResourceTypeFor<CComputeShader>();
      CComputeShader* cs = new CComputeShader();
      if (cs->create(j)) 
        new_res = cs;
    }
    else {
      assert(j.count("vdecl") > 0);
      res_type = getResourceTypeFor<CTechnique>();
      CTechnique* tech = new CTechnique();
      if (tech->create(j))
        new_res = tech;
    }

    std::string suffix = std::string(".") + res_type->getExtension();
    std::string name = it.key() + suffix;

    if (new_res == nullptr) {
      fatal("Failed to create %s %s\n", res_type->getName(), name.c_str());
      continue;
    }

    // Create a new tech, configure it and register it as resource
    new_res->setNameAndType(name, res_type);
    dbg("Registering %s %s\n", res_type->getName(), name.c_str());
    Resources.registerResource(new_res);
  }
}

void CModuleRender::update(float dt) {
  
}

void CModuleRender::stop() {

  // Render targets are owned by the Resources obj
  deferred = nullptr;
  deferred_output = nullptr;
  ui_output = nullptr;
  shine_output = nullptr;

  ImGui_ImplDX11_Shutdown();
  ImGui_ImplWin32_Shutdown();
  ImGui::DestroyContext();

  destroyRenderPrimitives();
  destroyRenderUtils();
}

void CModuleRender::beginImgui() {

  // Start the Dear ImGui frame
  ImGui_ImplDX11_NewFrame();
  ImGui_ImplWin32_NewFrame();
  ImGui::NewFrame();

}

// Rendering ImGui
void CModuleRender::endImgui() {
  PROFILE_FUNCTION("endImgui");
  CGpuScope gpu_scope("imgui");
  ImGui::Render();
  ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void CModuleRender::renderInMenu() {

  static int nframes = 5;
  ImGui::DragInt("NumFrames To Capture", &nframes, 0.1f, 1, 20);
  if (ImGui::SmallButton("Start CPU Trace Capturing")) {
    PROFILE_SET_NFRAMES(nframes);
    PROFILE_FRAME_BEGINS();
  }

  // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
  static bool show_demo_window = false;
  if (show_demo_window)
    ImGui::ShowDemoWindow(&show_demo_window);
  ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
  ImGui::ColorEdit3("clear color", (float*)&clear_color.x); // Edit 3 floats representing a color
  ImGui::Text("Application average %.3f ms/frame (%.1f FPS) %f", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate, Time.current);

  if (ImGui::TreeNode("Render Control")) {
    ImGui::DragFloat("Exposure Adjustment", &ctes_shared.GlobalExposureAdjustment, 0.01f, 0.1f, 32.f);
    ImGui::DragFloat("Ambient Boost", &new_ambient, 0.01f, 0.0f, 2.f);


    ImGui::Text("Toon Shading");
    ImGui::DragFloat("AO Power", &ctes_shared.ao_power, 0.1f, 0.0f, 10.f);
    ImGui::DragFloat("Shadow Ramp", &ctes_shared.shadow_ramp, 0.01f, 0.01f, 1.f);
    ImGui::DragFloat("Color Intensity", &ctes_shared.color_intensity, 0.1f, 1.0f, 4.f);
    ImGui::DragFloat("Brush Size", &ctes_shared.brush_size, 0.1f, 0.1f, 10.f);

    ImGui::DragFloat("Brush Rotation", &ctes_shared.brush_rotation, 0.01f, 0.0f, 3.14f);
    ImGui::DragFloat("Specular Ramp", &ctes_shared.specular_ramp, 0.05f, 0.0f, 0.95f);
    ImGui::DragFloat("Brush Ramp", &ctes_shared.specular_brush_ramp, 0.01f, 0.1f, 1.f);
    ImGui::DragFloat("Specular Strength", &ctes_shared.specular_strength, 0.1f, 0.0f, 4.f);

    //ImGui::DragFloat("HDR", &ctes_shared.global_hdr_enabled, 0.01f, 0.0f, 1.f);
    //ImGui::DragFloat("Gamma Correction", &ctes_shared.global_gamma_correction_enabled, 0.01f, 0.0f, 1.f);
    //ImGui::DragFloat("Reinhard vs Uncharted2", &ctes_shared.global_tone_mapping_mode, 0.01f, 0.0f, 1.f);

    // Must be in the same order as the RO_* ctes
    static const char* render_output_str =
      "Complete\0"
      "Albedo\0"        // RO_ALBEDO
      "Normals\0"
      "Normals ViewSpace\0"
      "Roughness\0"
      "Metallic\0"
      "World Pos\0"
      "Linear Depth\0"
      "AO\0"
      "\0";
    ImGui::Combo("Output", &ctes_shared.GlobalRenderOutput, render_output_str);

    deferred->renderInMenu();

    ImGui::TreePop();
  }


}

void CModuleRender::renderEntities() {
  CRenderManager::get().render(eRenderCategory::CATEGORY_SOLIDS);
}

void CModuleRender::uploadSkinMatricesToGPU() {
  PROFILE_FUNCTION("uploadSkinMatricesToGPU");
  getObjectManager<TCompSkeleton>()->forEach([](TCompSkeleton* cs) {
    cs->updateCtesBones();
    });
}

// ------------------------------------------
bool CModuleRender::setupDeferredOutput() {

  assert(deferred_output);
  if (deferred_output->getWidth() != Render.width || deferred_output->getHeight() != Render.height) {
    if (!deferred_output->createRT("g_deferred_output.dds", Render.width, Render.height, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_UNKNOWN, true, 1 ))
      return false;
  }

  //do the same for the ui_output
  if (ui_output->getWidth() != Render.width || ui_output->getHeight() != Render.height) {
    if (!ui_output->createRT("g_ui_output.dds", Render.width, Render.height, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_UNKNOWN))
      return false;
  }

  //do the same for the shine_output
  if (shine_output->getWidth() != Render.width || shine_output->getHeight() != Render.height) {
    if (!shine_output->createRT("g_shine_output.dds", Render.width, Render.height, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_UNKNOWN))
      return false;
  }

  return true;
}

// ----------------------------------------------
void CModuleRender::generateFrame() {
  CGpuScope gpu_trace("Module::Render");
  PROFILE_FUNCTION("CModuleRender::generateFrame");
  
  ctes_shared.GlobalWorldTime = (float)Time.current;
  ctes_shared.Actual_dt = (float)Time.delta;
  ctes_shared.GlobalDeltaTime = Time.delta;
  ctes_shared.GlobalDeltaUnscaledTime = Time.delta_unscaled;
  if (ctes_shared.GlobalAmbientBoost != new_ambient ) {
    ctes_shared.GlobalAmbientBoost += sign(new_ambient - ctes_shared.GlobalAmbientBoost) * Time.delta;
    ctes_shared.GlobalAmbientBoost = clamp(ctes_shared.GlobalAmbientBoost, new_ambient, 1.0f);
  }
  ctes_shared.updateGPU();

  activateObject(MAT44::Identity, VEC4(1, 1, 1, 1));

  uploadSkinMatricesToGPU();

  // Check if we have culling information from the camera source
  CEntity* e_camera = CRenderManager::get().getEntityCamera();
  const TCompCulling* culling = nullptr;
  if (e_camera)
    culling = e_camera->get<TCompCulling>();
  const TCompCulling::TCullingBits* culling_bits = culling ? &culling->bits : nullptr;

  // Activate all the lights... but only the last one will be used!!!!
  getObjectManager<TCompLightDir>()->forEach([culling_bits](TCompLightDir* cs) {
    // Do the culling
    if (culling_bits) {
      TCompAbsAABB* aabb = cs->get<TCompAbsAABB>();
      if (aabb) {
        CHandle h_aabb(aabb);
        auto idx = h_aabb.getExternalIndex();
        if (!culling_bits->test(idx)) {
          return;
        }
      }
    }
    cs->generateShadowMap();
  });

  h_camera = getEntityByName("MainCamera");
  if (h_camera.isValid()) {
    CEntity* e_camera = h_camera;
    TCompCamera* c_camera = e_camera->get<TCompCamera>();
    assert(c_camera);
    // Here we are updating the Viewport of the given camera
    c_camera->setViewport(0, 0, Render.width, Render.height);
    // Copy current state of the given camera 
    camera = *c_camera;
    CRenderManager::get().setEntityCamera(h_camera);
  }
  else {
    camera.setViewport(0, 0, Render.width, Render.height);
  }

  activateCamera(camera, Render.width, Render.height);

  setupDeferredOutput();

  deferred->render(deferred_output, h_camera);

  CTexture* current_output = deferred_output;

  // Still rendering over deferred_output....
  CRenderManager::get().render(eRenderCategory::CATEGORY_DISTORSIONS);

  //it should render over deferred_output and shine_output
  renderShine();  

  //back to render to deferred_output
  deferred_output->activateRT();

  // Apply post FX effects
  e_camera = h_camera;
  if (e_camera) {
    TCompRenderBlur* render_blur = e_camera->get<TCompRenderBlur>();
    if (render_blur)
      current_output = render_blur->apply(current_output);

    TCompRenderRadialBlur* render_radial_blur = e_camera->get<TCompRenderRadialBlur>();
    if (render_radial_blur)
      current_output = render_radial_blur->apply(current_output);

    // Render focus requires render blur to be enabled also
    TCompRenderFocus* render_focus = e_camera->get<TCompRenderFocus>();
    if (render_focus && render_blur && render_blur->enabled && render_focus->enabled)
      current_output = render_focus->apply(deferred_output, current_output);

    TCompRenderBloom* render_bloom = e_camera->get<TCompRenderBloom>();
    if (render_bloom) {
      render_bloom->setShineTexture(shine_output);
      render_bloom->generateHighlights(deferred_output);
      render_bloom->addBloom();
    }

    TCompDamagedEffect* render_damaged_effect = e_camera->get<TCompDamagedEffect>();
    if (render_damaged_effect) {
      current_output = render_damaged_effect->apply(current_output);
    }

    TCompVignetting* render_vignetting = e_camera->get<TCompVignetting>();
    if (render_vignetting) {
      current_output = render_vignetting->apply(current_output);
    }

    //WE PASS TO LDR BECAUSE FXAA NEEDS IT
    TCompToneMapping* render_tone_map = e_camera->get<TCompToneMapping>();
    assert(render_tone_map); //this should never jump
    if (render_tone_map) {
      current_output = render_tone_map->apply(current_output);
    }

    TCompAntialiasing* render_antialiasing = e_camera->get<TCompAntialiasing>();
    if (render_antialiasing) 
      current_output = render_antialiasing->apply(current_output);

    //ultimo postfx a hacer ya que activa la textura para el presentation.tech
    TCompColorGrading* color_grading = e_camera->get<TCompColorGrading>();
    ctes_shared.GlobalLUTAmount = color_grading ? color_grading->getAmount() : 0.f;
    ctes_shared.updateGPU();
    if (color_grading)
      color_grading->lut1->activate(TS_LUT_COLOR_GRADING);

  }

  // 
  //change output rendertarget for the next frame
  Render.startRenderingBackBuffer();

  assert(current_output);
  {
    drawFullScreenQuad("presentation.tech", current_output);
  }

  {
    //PROFILE_FUNCTION("RenderInMenu");
    //CEngine::get().getModules().renderInMenu();
    //Resources.renderInMenu();
  }

  // call render method of all active modules... Probably for debug only
  CEngine::get().renderDebug();

  // Render UI section using an ortho camera
  // Vertical size is 1
  // Horizontal size is aspect ratio. Top/left is 0,0
  camera_ortho.setOrthoParams(false, 0.0f, (float)Render.width / (float)Render.height, 0.f, 1.0f, -1.0f, 1.0f);
  activateCamera(camera_ortho, Render.width, Render.height);
  //CRenderManager::get().render(eRenderCategory::CATEGORY_UI);
  CGpuScope gpu_scope("UI");
  ui_output->activateRT();
  ui_output->clear(VEC4(0.0f, 0.0f, 0.0f, 0.0f));
  CEngine::get().getUI().render();

  CTexture* current_ui_output = ui_output;
  if (e_camera) {
    TCompChromaticAberration* render_chromatic = e_camera->get<TCompChromaticAberration>();
    if (render_chromatic) {
      current_ui_output = render_chromatic->apply(ui_output);
    }
  }

  Render.startRenderingBackBuffer();

  assert(current_ui_output);
  {
    drawFullScreenQuad("presentation_ui.tech", current_ui_output);
  }

}

void CModuleRender::renderShine() {
  //shine_output->activateRT(deferred_output);


  //CTexture::setNullTexture(TS_DEFERRED_ALBEDOS);

  // Activate el multi-render-target MRT
  const int nrender_targets = 2;
  ID3D11RenderTargetView* rts[nrender_targets] = {
    deferred_output->getRenderTargetView(),
    shine_output->getRenderTargetView()
  };

  // We use our 2 rt's and the Zbuffer of the Deferred Buffer
  Render.ctx->OMSetRenderTargets(nrender_targets, rts, deferred_output->getDepthStencilView());
  deferred_output->activateViewport();   // Any rt will do...

  // Clear only the shine buffer, we will write over the deferred buffer
  shine_output->clear(VEC4(0, 0, 0, 0));  

  // Render the shining objects that output to the Deferred buffer and Shine buffer
  CRenderManager::get().render(eRenderCategory::CATEGORY_SHINE);

  // Disable rendering to all render targets.
  ID3D11RenderTargetView* rt_nulls[nrender_targets];
  for (int i = 0; i < nrender_targets; ++i) rt_nulls[i] = nullptr;
  Render.ctx->OMSetRenderTargets(nrender_targets, rt_nulls, nullptr);

}