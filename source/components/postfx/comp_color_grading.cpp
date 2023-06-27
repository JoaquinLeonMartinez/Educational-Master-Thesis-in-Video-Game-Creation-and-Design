#include "mcv_platform.h"
#include "comp_color_grading.h"
#include "render/textures/render_to_texture.h"
#include "resources/resource.h"

DECL_OBJ_MANAGER("color_grading", TCompColorGrading);

// ---------------------
void TCompColorGrading::debugInMenu() {
  ImGui::Checkbox("Enabled", &enabled);
  ImGui::DragFloat("Amount", &amount, 0.01f, 0.0f, 1.0f);
}

void TCompColorGrading::load(const json& j, TEntityParseContext& ctx) {
  enabled = j.value("enabled", true);
  amount= j.value( "amount", 1.0f);
  std::string lut_name = j.value("lut", "");
  lut1 = Resources.get(lut_name)->as<CTexture>();
}
