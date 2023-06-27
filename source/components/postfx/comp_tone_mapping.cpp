#include "mcv_platform.h"
#include "comp_tone_mapping.h"
#include "resources/resource.h"

DECL_OBJ_MANAGER("tone_mapping", TCompToneMapping);

// ---------------------
void TCompToneMapping::debugInMenu() {
  ImGui::Checkbox("Enabled", &enabled);

  //ImGui::DragFloat("Amount", &amount, 0.01f, 0.f, 1);
}

void TCompToneMapping::load(const json& j, TEntityParseContext& ctx) {
  enabled = j.value( "enabled", true);
  tech = Resources.get("tone_mapping.tech")->as<CTechnique>();
  mesh = Resources.get("unit_quad_xy.mesh")->as<CMesh>();

  rt = new CRenderToTexture;
  bool is_ok = rt->createRT("Tone_Mapping", Render.width, Render.height, DXGI_FORMAT_R16G16B16A16_UNORM);
  assert(is_ok);
}

CTexture* TCompToneMapping::apply( CTexture* texture) {
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