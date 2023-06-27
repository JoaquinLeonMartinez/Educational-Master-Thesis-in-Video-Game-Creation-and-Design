#include "mcv_platform.h"
#include "comp_light_point.h"
#include "comp_transform.h"
#include "render/render_manager.h" 

DECL_OBJ_MANAGER("light_point", TCompLightPoint);

// Use nvidia photoshop plugin (dds) to convert the RGBTable16x1.dds recolored to a dds volume texture 
// Volume Texture
// No Mipmaps
// 8.8.8.8 ARGB 32 bpp

DXGI_FORMAT readFormat(const json& j, const std::string& label);

// -------------------------------------------------
void TCompLightPoint::debugInMenu() {
  ImGui::DragFloat("Intensity", &intensity, 0.01f, 0.f, 10.f);
  ImGui::ColorEdit3("Color", &color.x);
  ImGui::DragFloat("Radius", &radius, 0.01f, 0.f, 100.f);
}

MAT44 TCompLightPoint::getWorld() {
  TCompTransform* c = get<TCompTransform>();
  if (!c)
    return MAT44::Identity;
  return MAT44::CreateScale(radius * 1.05f) * c->asMatrix();
}

// -------------------------------------------------
void TCompLightPoint::renderDebug() {
  // Render a wire sphere
  auto mesh = Resources.get("data/meshes/UnitSphere.mesh")->as<CMesh>();
  drawMesh(mesh, getWorld(), VEC4(1, 1, 1, 1));
}

// -------------------------------------------------
void TCompLightPoint::load(const json& j, TEntityParseContext& ctx) {
  if(j.count("color"))
    color = loadColor(j, "color");
  intensity = j.value("intensity", intensity);
  radius = j.value("radius", radius);
}

// -------------------------------------------------
// Updates the Shader Cte Light with MY information
void TCompLightPoint::activate() {
  TCompTransform* c = get<TCompTransform>();
  if (!c)
    return;

  ctes_light.LightColor = color;
  ctes_light.LightIntensity = intensity;
  ctes_light.LightPosition = c->getPosition();
  ctes_light.LightRadius = radius * c->getScale();
  ctes_light.LightViewProjOffset = MAT44::Identity;
  ctes_light.updateGPU();
}

void TCompLightPoint::registerMsgs() {
  DECL_MSG(TCompLightPoint, TMsgDefineLocalAABB, onDefineLocalAABB);
}

void TCompLightPoint::onDefineLocalAABB(const TMsgDefineLocalAABB& msg) {
  BSphere sphere = BSphere::BoundingSphere();
  TCompTransform* c_trans = get<TCompTransform>();
  sphere.Center = VEC3::Zero;
  sphere.Radius = radius;
  AABB::CreateFromSphere(aabb, sphere);
  AABB::CreateMerged(*msg.aabb, *msg.aabb, aabb);
}


