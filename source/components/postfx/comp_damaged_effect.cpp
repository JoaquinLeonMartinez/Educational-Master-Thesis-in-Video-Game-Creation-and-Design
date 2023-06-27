#include "mcv_platform.h"
#include "comp_damaged_effect.h"
#include "render/textures/render_to_texture.h"
#include "resources/resource.h"

DECL_OBJ_MANAGER("damaged_effect", TCompDamagedEffect);

// ---------------------
void TCompDamagedEffect::debugInMenu() {
  ImGui::Checkbox("Enabled", &enabled);
  ImGui::DragFloat("Amount", &grade, 0.01f, 0.f, 1);
}

void TCompDamagedEffect::load(const json& j, TEntityParseContext& ctx) {
  enabled = j.value("enabled", true);
  tech = Resources.get("damaged_effect.tech")->as<CTechnique>();
  mesh = Resources.get("unit_quad_xy.mesh")->as<CMesh>();
  dither_tex = Resources.get("data/textures/vfx/shadowcomic.dds")->as<CTexture>();

  rt = new CRenderToTexture;
  bool is_ok = rt->createRT("Damaged_effect", Render.width, Render.height, DXGI_FORMAT_R16G16B16A16_UNORM);
  assert(is_ok);
}

void TCompDamagedEffect::registerMsgs() {
  DECL_MSG(TCompDamagedEffect, TMsgOnContact, onCollision);
}

void TCompDamagedEffect::onCollision(const TMsgOnContact& msg) {
  grade = 1.0f;
}

void TCompDamagedEffect::update(float dt) {
  grade = clamp(grade - dt * 4.0f, 0.0f, 1.0f);
}

CTexture* TCompDamagedEffect::apply(CTexture* texture) {
  CGpuScope gpu_scope(tech->getName().c_str());

  if (!enabled)
    return texture;

  assert(rt);
  assert(mesh);
  assert(tech);
  assert(texture);

  ctes_dam.grade = grade;
  ctes_dam.activate();
  ctes_dam.updateGPU();

  rt->activateRT();
  texture->activate(TS_ALBEDO);
  dither_tex->activate(TS_NORMAL);
  tech->activate();
  mesh->activateAndRender();

  return rt;
}