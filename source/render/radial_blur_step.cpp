#include "mcv_platform.h"
#include "radial_blur_step.h"
#include "textures/render_to_texture.h"
#include "resources/resource.h"

// ---------------------
bool CRadialBlurStep::create(const char* name, int in_xres, int in_yres, DXGI_FORMAT fmt) {

  // Get input resolution
  xres = in_xres;
  yres = in_yres;

  rt_half_y = new CRenderToTexture();
  std::string sname = std::string(name) + "_y";
  bool is_ok = rt_half_y->createRT(sname.c_str(), xres, yres / 1.5, fmt, DXGI_FORMAT_UNKNOWN);
  assert(is_ok);

  sname = std::string(name) + "_xy";
  rt_output = new CRenderToTexture();
  is_ok = rt_output->createRT(sname.c_str(), xres / 1.5, yres / 1.5, fmt, DXGI_FORMAT_UNKNOWN);
  assert(is_ok);

  tech = Resources.get("radial_blur.tech")->as<CTechnique>();
  mesh = Resources.get("unit_quad_xy.mesh")->as<CMesh>();

  return is_ok;
}

// --------------------
// dx/dy are texel units. (1,0) means one texel to the right
void CRadialBlurStep::applyBlur(float dx, float dy) {

  // Normalize UV's
  ctes_blur.blur_step.x = (float)dx / (float)xres;
  ctes_blur.blur_step.y = (float)dy / (float)yres;

  ctes_blur.activate();
  ctes_blur.updateGPU();

  tech->activate();
  mesh->activateAndRender();
}

// ---------------------
CTexture* CRadialBlurStep::apply(
  CTexture* input,
  float global_distance,
  VEC4 distances,
  VEC4 weights
) {

  // Sum( Weights ) = 1 to not loose energy. +2 is to account for left and right taps
  float normalization_factor =
    1 * weights.x
    + 2 * weights.y
    + 2 * weights.z
    + 2 * weights.w
    ;
  ctes_blur.blur_w.x = weights.x / normalization_factor;
  ctes_blur.blur_w.y = weights.y / normalization_factor;
  ctes_blur.blur_w.z = weights.z / normalization_factor;
  ctes_blur.blur_w.w = weights.w / normalization_factor;
  ctes_blur.blur_d.x = distances.x;
  ctes_blur.blur_d.y = distances.y;
  ctes_blur.blur_d.z = distances.z;
  ctes_blur.blur_d.w = distances.w;  // Not used

  rt_half_y->activateRT();
  input->activate(TS_ALBEDO);
  applyBlur(0, global_distance);

  rt_output->activateRT();
  rt_half_y->activate(TS_ALBEDO);
  applyBlur(global_distance, 0);

  return rt_output;
}
