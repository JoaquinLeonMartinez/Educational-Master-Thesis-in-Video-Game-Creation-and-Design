#include "mcv_platform.h"
#include "deferred_renderer.h"
#include "render_manager.h"
#include "render_utils.h"
#include "resources/resource.h"
#include "render/primitives.h"
#include "components/common/comp_light_dir.h"
#include "components/common/comp_light_point.h"
#include "components/postfx/comp_render_ao.h"
#include "render/intel/ASSAO.h"
#include "components/common/comp_aabb.h"
#include "components/common/comp_culling.h"

CDeferredRenderer::CDeferredRenderer() {
  dbg("Creating CDeferredRenderer\n");
}


void CDeferredRenderer::renderGBuffer() {
  CGpuScope gpu_scope("Deferred.GBuffer");

  // Disable the gbuffer textures as we are going to update them
  // Can't render to those textures and have them active in some slot...
  CTexture::setNullTexture(TS_DEFERRED_ALBEDOS);
  CTexture::setNullTexture(TS_DEFERRED_NORMALS);
  CTexture::setNullTexture(TS_DEFERRED_LINEAR_DEPTH);
  CTexture::setNullTexture(TS_DEFERRED_SELF_ILLUM);

  // Activate el multi-render-target MRT
  const int nrender_targets = 4;
  ID3D11RenderTargetView* rts[nrender_targets] = {
    rt_albedos->getRenderTargetView(),
    rt_normals->getRenderTargetView(),
    rt_depth->getRenderTargetView(),
    rt_self_illum->getRenderTargetView(),
  };

  // We use our 3 rt's and the Zbuffer of the backbuffer
  Render.ctx->OMSetRenderTargets(nrender_targets, rts, Render.depth_stencil_view);
  rt_albedos->activateViewport();   // Any rt will do...

  // Clear output buffers, some can be removed if we intend to fill all the screen
  // with new data.
  rt_albedos->clear(VEC4(1, 0, 0, 1));
  rt_normals->clear(VEC4(0, 0, 1, 1));
  rt_depth->clear(VEC4(1, 1, 1, 1));
  rt_self_illum->clear(VEC4(0, 0, 0, 1));

  // Clear ZBuffer with the value 1.0 (far)
  Render.ctx->ClearDepthStencilView(Render.depth_stencil_view, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

  // Render the solid objects that output to the G-Buffer
  CRenderManager::get().render(eRenderCategory::CATEGORY_SOLIDS);

  // Disable rendering to all render targets.
  ID3D11RenderTargetView* rt_nulls[nrender_targets];
  for (int i = 0; i < nrender_targets; ++i) rt_nulls[i] = nullptr;
  Render.ctx->OMSetRenderTargets(nrender_targets, rt_nulls, nullptr);

  // Activate the gbuffer textures to other shaders
  rt_albedos->activate(TS_DEFERRED_ALBEDOS);
  rt_normals->activate(TS_DEFERRED_NORMALS);
  rt_depth->activate(TS_DEFERRED_LINEAR_DEPTH);
  rt_self_illum->activate(TS_DEFERRED_SELF_ILLUM);
}

// --------------------------------------------------------------
void CDeferredRenderer::renderGBufferDecals() {
  CGpuScope gpu_scope("Deferred.GBuffer.Decals");

  // Disable the gbuffer textures as we are going to update them
  // Can't render to those textures and have them active in some slot...
  CTexture::setNullTexture(TS_DEFERRED_ALBEDOS);
  //CTexture::setNullTexture(TS_DEFERRED_NORMALS);

  // Activate el multi-render-target MRT
  const int nrender_targets = 1;
  ID3D11RenderTargetView* rts[nrender_targets] = {
    rt_albedos->getRenderTargetView(),
    //rt_normals->getRenderTargetView()
    // No Z as we need to read to reconstruct the position
  };

  // We use our 3 rt's and the Zbuffer of the backbuffer
  Render.ctx->OMSetRenderTargets(nrender_targets, rts, Render.depth_stencil_view);
  rt_albedos->activateViewport();   // Any rt will do...

  // Render blending layer on top of gbuffer before adding lights
  CRenderManager::get().render(eRenderCategory::CATEGORY_DECALS);

  // Disable rendering to all render targets.
  ID3D11RenderTargetView* rt_nulls[nrender_targets];
  for (int i = 0; i < nrender_targets; ++i) rt_nulls[i] = nullptr;
  Render.ctx->OMSetRenderTargets(nrender_targets, rt_nulls, nullptr);

  // Activate the gbuffer textures to other shaders
  //rt_normals->activate(TS_DEFERRED_NORMALS);
  rt_albedos->activate(TS_DEFERRED_ALBEDOS);
}

// -----------------------------------------------------------------
void CDeferredRenderer::destroy() {
  if (rt_albedos) {
    rt_albedos->destroy();
    rt_normals->destroy();
    rt_depth->destroy();
    rt_acc_light->destroy();
    rt_self_illum->destroy();
  }

  if (assao_fx) {
    ASSAO_Effect::DestroyInstance(assao_fx);
    assao_fx = nullptr;
  }
}

// -----------------------------------------------------------------
bool CDeferredRenderer::create(int new_xres, int new_yres) {

  xres = new_xres;
  yres = new_yres;

  dbg("Initializing deferred to %dx%d\n", xres, yres);

  destroy();

  if (!rt_albedos) {
    rt_albedos = new CRenderToTexture;
    rt_normals = new CRenderToTexture;
    rt_depth = new CRenderToTexture;
    rt_acc_light = new CRenderToTexture;
    rt_self_illum = new CRenderToTexture;
  }

  if (!rt_albedos->createRT("g_albedos.dds", xres, yres, DXGI_FORMAT_R16G16B16A16_UNORM))
    return false;

  if (!rt_normals->createRT("g_normals.dds", xres, yres, DXGI_FORMAT_R16G16B16A16_UNORM))
    return false;

  if (!rt_depth->createRT("g_depths.dds", xres, yres, DXGI_FORMAT_R32_FLOAT))
    return false;

  if (!rt_acc_light->createRT("acc_light.dds", xres, yres, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_UNKNOWN, true))
    return false;

  if (!rt_self_illum->createRT("g_self_illum.dds", xres, yres, DXGI_FORMAT_R8G8B8A8_UNORM))
    return false;

  if (assao_fx) {
    ASSAO_Effect::DestroyInstance(assao_fx); 
    assao_fx = nullptr;
  }

  CMemoryDataProvider mdp("data/shaders/ASSAO.hlsl");
  ASSAO_CreateDescDX11 desc(Render.device, mdp.data(), mdp.size());
  assao_fx = ASSAO_Effect::CreateInstance(&desc);

  return true;
}

// -----------------------------------------------------------------

void CDeferredRenderer::renderAmbientPass() {
  CGpuScope gpu_scope("renderAmbientPass");
  drawFullScreenQuad("pbr_ambient.tech", nullptr);
}

// -------------------------------------------------------------------------
void CDeferredRenderer::renderSkyBox() const {
  CGpuScope gpu_scope("renderSkyBox");
  drawFullScreenQuad("pbr_skybox.tech", nullptr);
}

// -------------------------------------------------------------------------
void CDeferredRenderer::renderAccLight() {
  CGpuScope gpu_scope("Deferred.AccLight");
  rt_acc_light->activateRT();
  rt_acc_light->clear(VEC4(0, 0, 0, 0));
  renderAmbientPass();
  renderPointLights();
  renderDirectionalLightsWithoutShadows();
  renderDirectionalLightsWithShadows();
  //renderFakeVolumetricLights();
  renderSkyBox();
}

// -------------------------------------------------------------------------
void CDeferredRenderer::renderPointLights() {
  CGpuScope gpu_scope("renderPointLights");

  // Activate tech for the light dir
  auto* tech = Resources.get("pbr_point_lights.tech")->as<CTechnique>();
  tech->activate();

  // All light directional use the same mesh
  auto* mesh = Resources.get("data/meshes/UnitSphere.mesh")->as<CMesh>();
  mesh->activate();

  // Check if we have culling information from the camera source
  CEntity* e_camera = CRenderManager::get().getEntityCamera();
  const TCompCulling* culling = nullptr;
  if (e_camera)
    culling = e_camera->get<TCompCulling>();
  const TCompCulling::TCullingBits* culling_bits = culling ? &culling->bits : nullptr;

  int i = 0;
  // Para todas las luces... pintala
  getObjectManager<TCompLightPoint>()->forEach([mesh, culling_bits, &i](TCompLightPoint* c) {

    // Do the culling
    if (culling_bits) {
      TCompAbsAABB* aabb = c->get<TCompAbsAABB>();
      if (aabb) {
        CHandle h_aabb(aabb);
        auto idx = h_aabb.getExternalIndex();
        if (!culling_bits->test(idx)) {
          return;
        }
      }
    }

    i++;
    // subir las contantes de la posicion/dir
    // activar el shadow map...
    c->activate();

    activateObject(c->getWorld());

    // mandar a pintar una geometria que refleje los pixeles que potencialmente
    // puede iluminar esta luz.... El Frustum solido
    mesh->render();
  });
  lightPointsRendered = i;
}

// -------------------------------------------------------------------------
void CDeferredRenderer::renderDirectionalLightsWithoutShadows() {
  CGpuScope gpu_scope("renderDirectionalLightsWithoutShadows");

  // Activate tech for the light dir
  auto* tech = Resources.get("pbr_dir_lights_no_shadows.tech")->as<CTechnique>();
  tech->activate();

  // All light directional use the same mesh
  auto* mesh = Resources.get("unit_frustum_solid.mesh")->as<CMesh>();
  mesh->activate();

  // Check if we have culling information from the camera source
  CEntity* e_camera = CRenderManager::get().getEntityCamera();
  const TCompCulling* culling = nullptr;
  if (e_camera)
    culling = e_camera->get<TCompCulling>();
  const TCompCulling::TCullingBits* culling_bits = culling ? &culling->bits : nullptr;

  // Para todas las luces... pintala
  int i = 0;
  getObjectManager<TCompLightDir>()->forEach([mesh, culling_bits, &i](TCompLightDir* c) {

    if (c->isShadowEnabled())
      return;

    // Do the culling
    if (culling_bits) {
      TCompAbsAABB* aabb = c->get<TCompAbsAABB>();
      if (aabb) {
        CHandle h_aabb(aabb);
        auto idx = h_aabb.getExternalIndex();
        if (!culling_bits->test(idx)) {
          return;
        }
      }
    }

    // subir las contantes de la posicion/dir
    // activar el shadow map...
    c->activate();

    activateObject(c->getViewProjection().Invert());

    i++;
    // mandar a pintar una geometria que refleje los pixeles que potencialmente
    // puede iluminar esta luz.... El Frustum solido
    mesh->render();
  });

  lightDirRendered = i;
}

void CDeferredRenderer::renderDirectionalLightsWithShadows() {
  CGpuScope gpu_scope("renderDirectionalLightsWithShadows");

  // Activate tech for the light dir
  auto* tech = Resources.get("pbr_dir_lights.tech")->as<CTechnique>();
  tech->activate();

  // All light directional use the same mesh
  auto* mesh = Resources.get("unit_frustum_solid.mesh")->as<CMesh>();
  mesh->activate();

  // Check if we have culling information from the camera source
  CEntity* e_camera = CRenderManager::get().getEntityCamera();
  const TCompCulling* culling = nullptr;
  if (e_camera)
    culling = e_camera->get<TCompCulling>();
  const TCompCulling::TCullingBits* culling_bits = culling ? &culling->bits : nullptr;

  int i = 0;
  // Para todas las luces... pintala
  getObjectManager<TCompLightDir>()->forEach([mesh, culling_bits, &i](TCompLightDir* c) {

    if (!c->isShadowEnabled())
      return;

    // Do the culling
    if (culling_bits) {
      TCompAbsAABB* aabb = c->get<TCompAbsAABB>();
      if (aabb) {
        CHandle h_aabb(aabb);
        auto idx = h_aabb.getExternalIndex();
        if (!culling_bits->test(idx)) {
          return;
        }
      }
    }

    // subir las contantes de la posicion/dir
    // activar el shadow map...
    c->activate();

    activateObject(c->getViewProjection().Invert());

    i++;
    // mandar a pintar una geometria que refleje los pixeles que potencialmente
    // puede iluminar esta luz.... El Frustum solido
    mesh->render();
  });
  lightDirShadowRendered = i;
}

void CDeferredRenderer::renderFakeVolumetricLights() {
  CGpuScope gpu_scope("renderFakeVolumetricLights");

  // Activate tech for the volumetric light
  auto* tech = Resources.get("fake_volumetric_lights.tech")->as<CTechnique>();
  tech->activate();

  // All light directional use the same mesh
  auto* mesh = Resources.get("unit_frustum_solid.mesh")->as<CMesh>();
  mesh->activate();

  // Check if we have culling information from the camera source
  CEntity* e_camera = CRenderManager::get().getEntityCamera();
  const TCompCulling* culling = nullptr;
  if (e_camera)
    culling = e_camera->get<TCompCulling>();
  const TCompCulling::TCullingBits* culling_bits = culling ? &culling->bits : nullptr;

  // Para todas las luces... pintala
  getObjectManager<TCompLightDir>()->forEach([mesh, culling_bits](TCompLightDir * c) {
    
    // Do the culling
    if (culling_bits) {
      TCompAbsAABB* aabb = c->get<TCompAbsAABB>();
      if (aabb) {
        CHandle h_aabb(aabb);
        auto idx = h_aabb.getExternalIndex();
        if (!culling_bits->test(idx)) {
          return;
        }
      }
    }

    // subir las contantes de la posicion/dir
    // activar el shadow map...
    c->activate();

    activateObject(c->getViewProjection().Invert());

    // mandar a pintar una geometria que refleje los pixeles que potencialmente
    // puede iluminar esta luz.... El Frustum solido
    mesh->render();
    });
}

// --------------------------------------
void CDeferredRenderer::renderAO(CHandle h_camera) const {

  CEntity* e_camera = h_camera;
  if (!e_camera)
  {
    return;
  }
  TCompRenderAO* comp_ao = e_camera->get<TCompRenderAO>();
  if (!comp_ao) {
    // As there is no comp AO, use a white texture as substitute
    const CTexture* white_texture = Resources.get("data/textures/white.dds")->as<CTexture>();
    white_texture->activate(TS_DEFERRED_AO);
    return;
  }
  // As we are going to update the RenderTarget AO
  // it can NOT be active as a texture while updating it.
  CTexture::setNullTexture(TS_DEFERRED_AO);

  auto ao = comp_ao->compute(rt_depth, rt_normals, assao_fx);
  // Activate the updated AO texture so everybody else can use it
  // Like the AccLight (Ambient pass or the debugger)
  ao->activate(TS_DEFERRED_AO);
}

// --------------------------------------
void CDeferredRenderer::render(CRenderToTexture* rt_destination, CHandle h_camera) {
  PROFILE_FUNCTION("Deferred");

  assert(rt_destination);
  assert(xres);
  assert(rt_destination->getWidth() == xres);
  assert(rt_destination->getHeight() == yres);

  assert(xres);
  assert(rt_destination->getWidth() == xres);
  assert(rt_destination->getHeight() == yres);

  renderGBuffer();
  renderGBufferDecals();
  renderAO(h_camera);

  // Do the same with the acc light
  CTexture::setNullTexture(TS_DEFERRED_ACC_LIGHTS);
  renderAccLight();

  // Render blending layer on top of gbuffer before adding lights
  CRenderManager::get().render(eRenderCategory::CATEGORY_TRANSPARENTS);

  // Now dump contents to the destination buffer.
  rt_destination->activateRT();
  rt_acc_light->activate(TS_DEFERRED_ACC_LIGHTS);

  // Combine the results
  drawFullScreenQuad("gbuffer_resolve.tech", nullptr);
}

void CDeferredRenderer::renderInMenu() {
  auto hm = getObjectManager<TCompLightPoint>();
  ImGui::Text("%d / %d LightPoint's visible", lightPointsRendered, hm->size());
  auto hm2 = getObjectManager<TCompLightDir>();
  ImGui::Text("%d / %d LightDir's visible", lightDirRendered, hm2->size());
  ImGui::Text("%d / %d LightDirShadow's visible", lightDirShadowRendered, hm2->size());
}