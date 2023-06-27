#include "mcv_platform.h"
#include "comp_light_dir.h"
#include "comp_transform.h"
#include "comp_aabb.h"
//#include "render/render_objects.h"    // cb_light
#include "render/textures/texture.h" 
//#include "ctes.h"                     // texture slots
#include "render/textures/render_to_texture.h" 
#include "render/render_manager.h" 
#include "engine.h"

DECL_OBJ_MANAGER("light_dir", TCompLightDir);

DXGI_FORMAT readFormat(const json& j, const std::string& label);

// -------------------------------------------------
void TCompLightDir::debugInMenu() {
  TCompCamera::debugInMenu();
  ImGui::DragFloat("Intensity", &intensity, 0.01f, 0.f, 10.f);
  ImGui::ColorEdit3("Color", &color.x);
  ImGui::DragFloat("Step Size", &shadows_step, 0.01f, 0.f, 5.f);
  ImGui::Checkbox("Shadows Enabled", &shadows_enabled);
}

// -------------------------------------------------
void TCompLightDir::renderDebug() {
  TCompCamera::renderDebug();
  TCompTransform* c_trans = get<TCompTransform>();
}

// -------------------------------------------------
void TCompLightDir::load(const json& j, TEntityParseContext& ctx) {
  TCompCamera::load( j, ctx );
  if(j.count("color"))
    color = loadColor(j, "color");
  intensity = j.value("intensity", intensity);

  offset = loadVEC3(j, "offset");

  if( j.count("projector")) {
    std::string projector_name = j.value("projector", "");
    projector = Resources.get(projector_name)->as<CTexture>();
  }

  // Check if we need to allocate a shadow map
  casts_shadows = j.value("casts_shadows", false);
  if (casts_shadows) {
    shadows_step = j.value("shadows_step", shadows_step);
    shadows_resolution = j.value("shadows_resolution", shadows_resolution);
    auto shadowmap_fmt = readFormat(j, "shadows_fmt");
    assert(shadows_resolution > 0);
    shadows_rt = new CRenderToTexture;
    // Make a unique name to have the Resource Manager happy with the unique names for each resource
    char my_name[64];
    sprintf(my_name, "shadow_map_%08x", CHandle(this).asUnsigned());
    
    // Added a placeholder Color Render Target to be able to do a alpha test when rendering6
    // the grass
    bool is_ok = shadows_rt->createRT(my_name, shadows_resolution, shadows_resolution, DXGI_FORMAT_R8G8B8A8_UNORM, shadowmap_fmt);
    assert(is_ok);
  }

  shadows_enabled = j.value("shadows_enabled", casts_shadows);
}

void TCompLightDir::update(float dt) {
  // Can't use the TCompCamera::update because inside it calls 
  // get<TCompTransform> which tries to convert 'this' to an instance
  // of TCompCamera, but will fail because we are a CompLightDir

  TCompTransform* c = get<TCompTransform>();
  if (!c)
    return;
  this->lookAt(c->getPosition(), c->getPosition() + c->getFront(), c->getUp());


  if (AABBpassed)
    return;
  AABBpassed = true;
  VEC3 pp1 = VEC3(1.f, 1.f, 1.f), pp2 = VEC3(-1.f, -1.f, 0.f), pp3 = VEC3(1.f, 1.f, 0.f), pp4 = VEC3(-1.f, -1.f, 1.f);
  MAT44 inv_view_proj = getViewProjection().Invert();

  VEC3 p1, p2, p3, p4;
  VEC3::Transform(pp1, inv_view_proj, p1);
  VEC3::Transform(pp2, inv_view_proj, p2);
  VEC3::Transform(pp3, inv_view_proj, p3);
  VEC3::Transform(pp4, inv_view_proj, p4);

  AABB              aabb1;
  AABB              aabb2;
  AABB::CreateFromPoints(aabb1, p1, p2);
  AABB::CreateFromPoints(aabb2, p3, p4);

  AABB              finalAABB;
  AABB              entityAbsAABB;
  AABB::CreateMerged(finalAABB, aabb1, aabb2);
  AABB::CreateMerged(entityAbsAABB, entityAbsAABB, finalAABB);

  TCompAbsAABB* c_aabb = get<TCompAbsAABB>();
  if (c_aabb) {
    c_aabb->Center = entityAbsAABB.Center;
    c_aabb->Extents = entityAbsAABB.Extents;
  }

  TCompLocalAABB* l_aabb = get< TCompLocalAABB>();
  if (l_aabb) {
    l_aabb->Center = entityAbsAABB.Center;
    l_aabb->Extents = entityAbsAABB.Extents;
  }
}

// -------------------------------------------------
// Updates the Shader Cte Light with MY information
void TCompLightDir::activate() {
  TCompTransform* c = get<TCompTransform>();
  if (!c)
    return;

  if (projector)
    projector->activate(TS_PROJECTOR);

  // To avoid converting the range -1..1 to 0..1 in the shader
  // we concatenate the view_proj with a matrix to apply this offset
  MAT44 mtx_offset = MAT44::CreateScale(VEC3(0.5f, -0.5f, 1.0f))
                   * MAT44::CreateTranslation(VEC3(0.5f, 0.5f, 0.0f));
                  
  ctes_light.LightColor = color;
  ctes_light.LightIntensity = intensity;
  ctes_light.LightPosition = c->getPosition();
  ctes_light.LightRadius = getFar();
  ctes_light.LightViewProjOffset = getViewProjection() * mtx_offset;
  ctes_light.LightFront = c->getFront();

  // If we have a ZTexture, it's the time to activate it
  if (shadows_rt) {

    ctes_light.LightShadowInverseResolution = 1.0f / (float)shadows_rt->getWidth();
    ctes_light.LightShadowStep = shadows_step;
    ctes_light.LightShadowStepDivResolution = shadows_step / (float)shadows_rt->getWidth();
    ctes_light.LightDummy2 = 1.0f;

    assert(shadows_rt->getZTexture());
    shadows_rt->getZTexture()->activate(TS_LIGHT_SHADOW_MAP);
  }
  
  ctes_light.updateGPU();
}

// ------------------------------------------------------
void TCompLightDir::generateShadowMap() {
  PROFILE_FUNCTION("ShadowMap");
  if (!shadows_rt || !shadows_enabled)
    return;

  // In this slot is where we activate the render targets that we are going
  // to update now. You can't be active as texture and render target at the
  // same time
  CTexture::setNullTexture(TS_LIGHT_SHADOW_MAP);

  CGpuScope gpu_scope(shadows_rt->getName().c_str());
  shadows_rt->activateRT();

  {
    PROFILE_FUNCTION("Clear&SetCommonCtes");
    shadows_rt->clearZ();
    // We are going to render the scene from the light position & orientation
    activateCamera(*this, shadows_rt->getWidth(), shadows_rt->getHeight());
  }


  {
    PROFILE_FUNCTION("GPU Culling for light");
    Engine.getGPUCulling().runWithCustomCamera(*this);
  }

  {
    PROFILE_FUNCTION("CPU Entities");
    CHandle comp(this);
    CHandle entity = comp.getOwner();
    CRenderManager::get().setEntityCamera(entity);
    CRenderManager::get().render(CATEGORY_SHADOWS);
  }
}

void TCompLightDir::registerMsgs() {
  DECL_MSG(TCompLightDir, TMsgDefineLocalAABB, onDefineLocalAABB);
}

void TCompLightDir::onDefineLocalAABB(const TMsgDefineLocalAABB& msg) {
  AABBpassed = false;
}