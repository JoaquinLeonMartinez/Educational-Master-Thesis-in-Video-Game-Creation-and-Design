#include "mcv_platform.h"
#include "comp_chromatic_aberration.h"
#include "resources/resource.h"
#include "components/controllers/character/comp_character_controller.h"

DECL_OBJ_MANAGER("chromatic_aberration", TCompChromaticAberration);

// ---------------------
void TCompChromaticAberration::debugInMenu() {
  ImGui::Checkbox("Enabled", &enabled);

  ImGui::DragFloat("Amount", &amount, 0.01f, 0.f, 1);
  ImGui::DragFloat("Distortion Speed", &distortion_speed, 0.01f, 0.f, 3);
  ImGui::DragFloat("Water Effect Speed", &water_effect_speed, 0.01f, 0.f, 3);
}

void TCompChromaticAberration::load(const json& j, TEntityParseContext& ctx) {
  enabled = j.value( "enabled", true);
  tech = Resources.get("chromatic_aberration.tech")->as<CTechnique>();
  mesh = Resources.get("unit_quad_xy.mesh")->as<CMesh>();

  rt = new CRenderToTexture;
  bool is_ok = rt->createRT("Chromatic_aberration", Render.width, Render.height, DXGI_FORMAT_R16G16B16A16_UNORM);
  assert(is_ok);
}

CTexture* TCompChromaticAberration::apply( CTexture* texture) {
  CGpuScope gpu_scope(tech->getName().c_str());
  
  if (!enabled)
    return texture;

  assert(rt);
  assert(mesh);
  assert(tech);
  assert(texture);

  ctes_chr_abr.WaterEffectSpeed = water_effect_speed;
  ctes_chr_abr.DistortionSpeed = distortion_speed;
  ctes_chr_abr.DistortionAmount = amount;
  ctes_chr_abr.activate();
  ctes_chr_abr.updateGPU();

  rt->activateRT();
  texture->activate(TS_ALBEDO);
  tech->activate();
  mesh->activateAndRender();

  return rt;
}

void TCompChromaticAberration::update(float delta) {
  if (!update_amount)
    return;

  if (!h_player.isValid()) {
    h_player = getEntityByName("Player");
  }
  
  CEntity* e_player = h_player;
  TCompCharacterController* c_char = e_player->get<TCompCharacterController>();
  if (c_char) {
    amount = 1 - (c_char->life / c_char->maxLife);
  }
}