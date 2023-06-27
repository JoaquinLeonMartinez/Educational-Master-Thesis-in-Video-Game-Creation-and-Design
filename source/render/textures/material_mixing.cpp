#include "mcv_platform.h"
#include "material_mixing.h"

// ---------------------------------------------------------
CMaterialMixing::CMaterialMixing()
: ctes_mixing(CTE_BUFFER_SLOT_MATERIAL)
{
}

// ---------------------------------------------------------
void CMaterialMixing::destroy() {
  ctes_mixing.destroy();
}

// ---------------------------------------------------------
void CMaterialMixing::activate() const {

  // Upload ctes
  ctes_mixing.activate();

  // Tech
  tech->activate();

  // 3 materials sets
  mats[0]->activateTextures(TS_FIRST_SLOT_MATERIAL_0);
  mats[1]->activateTextures(TS_FIRST_SLOT_MATERIAL_1);
  mats[2]->activateTextures(TS_FIRST_SLOT_MATERIAL_2);

  mix_blend_weights->activate(TS_MIX_BLEND_WEIGHTS);

}

bool CMaterialMixing::create(const json& j) {
  
  if (!CMaterial::create(j))
    return false;

  bool is_ok = ctes_mixing.create("Mixing");
  assert(is_ok);

  for (int i = 0; i < 3; ++i) {
    std::string mat_name = j["mats"][i];
    mats[i] = Resources.get(mat_name)->as< CMaterial >();
    assert(mats[i]);
  }

  mix_blend_weights = Resources.get(j.value("mix_blend_weights", "data/textures/black.dds"))->as<CTexture>();
  assert(mix_blend_weights);

  return true;
}

void CMaterialMixing::renderInMenu() {
  ((CTechnique*)tech)->renderInMenu();
  bool changed = false;
  changed |= ImGui::DragFloat("Boost R", &ctes_mixing.mix_boost_r, 0.01f, 0.f, 1.f);
  changed |= ImGui::DragFloat("Boost G", &ctes_mixing.mix_boost_g, 0.01f, 0.f, 1.f);
  changed |= ImGui::DragFloat("Boost B", &ctes_mixing.mix_boost_b, 0.01f, 0.f, 1.f);
  for (int i = 0; i < 3; ++i)
    ((CMaterial*)mats[i])->renderInMenu();
  if( changed )
    ctes_mixing.updateGPU();
}

