#include "mcv_platform.h"
#include "comp_antialiasing.h"
#include "resources/resource.h"

DECL_OBJ_MANAGER("antialiasing", TCompAntialiasing);

// ---------------------
void TCompAntialiasing::debugInMenu() {
  ImGui::Checkbox("Enabled", &enabled);

  ImGui::DragFloat("Sub pixel", &sub_pix, 0.01f, 0.f, 1);
  ImGui::DragFloat("Edge threshold", &edge_threshold, 0.01f, 0.f, 1);
  ImGui::DragFloat("Edge threshold Minimum", &edge_threshold_min, 0.01f, 0.f, 1);
}

void TCompAntialiasing::load(const json& j, TEntityParseContext& ctx) {
  enabled = j.value( "enabled", true);
  tech = Resources.get("fxaa.tech")->as<CTechnique>();
  mesh = Resources.get("unit_quad_xy.mesh")->as<CMesh>();

  rt = new CRenderToTexture;
  bool is_ok = rt->createRT("FXAA", Render.width, Render.height, DXGI_FORMAT_R16G16B16A16_UNORM);
  assert(is_ok);
}

CTexture* TCompAntialiasing::apply( CTexture* texture) {
  CGpuScope gpu_scope(tech->getName().c_str());
  
  if (!enabled)
    return texture;

  assert(rt);
  assert(mesh);
  assert(tech);
  assert(texture);

  ctes_fxaa.screenWidth = Render.width;
  ctes_fxaa.screenHeight = Render.height;
  ctes_fxaa.sub_pix = sub_pix;
  ctes_fxaa.edge_threshold = edge_threshold;
  ctes_fxaa.edge_threshold_min = edge_threshold_min;
  ctes_fxaa.activate();
  ctes_fxaa.updateGPU();

  rt->activateRT();
  texture->activate(TS_ALBEDO);
  tech->activate();
  mesh->activateAndRender();

  return rt;
}