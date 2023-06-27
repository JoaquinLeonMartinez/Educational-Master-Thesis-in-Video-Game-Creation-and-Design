#include "mcv_platform.h"
#include "comp_vignetting.h"
#include "render/textures/render_to_texture.h"
#include "resources/resource.h"

DECL_OBJ_MANAGER("vignetting", TCompVignetting);

// ---------------------
void TCompVignetting::debugInMenu() {
  ImGui::Checkbox("Enabled", &enabled);
}

void TCompVignetting::load(const json& j, TEntityParseContext& ctx) {
  enabled = j.value("enabled", true);
  tech = Resources.get("vignetting.tech")->as<CTechnique>();
  mesh = Resources.get("unit_quad_xy.mesh")->as<CMesh>();

  rt = new CRenderToTexture;
  bool is_ok = rt->createRT("Vignetting", Render.width, Render.height, DXGI_FORMAT_R16G16B16A16_UNORM);
  assert(is_ok);
}

CTexture* TCompVignetting::apply(CTexture* texture) {
  CGpuScope gpu_scope(tech->getName().c_str());

  if (!enabled)
    return texture;

  assert(rt);
  assert(mesh);
  assert(tech);
  assert(texture);


  rt->activateRT();
  texture->activate(TS_ALBEDO);
  tech->activate();
  mesh->activateAndRender();

  return rt;
}