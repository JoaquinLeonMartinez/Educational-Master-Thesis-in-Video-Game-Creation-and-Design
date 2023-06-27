#ifndef INC_RENDER_RADIAL_BLUR_STEP_H_
#define INC_RENDER_RADIAL_BLUR_STEP_H_

class CRenderToTexture;
class CTechnique;
class CMesh;
class CTexture;

struct CRadialBlurStep {

  CRenderToTexture* rt_half_y = nullptr;
  CRenderToTexture* rt_output = nullptr;    // half x & y

  int   xres = 0, yres = 0;
  const CTechnique* tech = nullptr;
  const CMesh* mesh = nullptr;

  // ---------------------
  bool create(const char* name, int in_xres, int in_yres, DXGI_FORMAT fmt);
  void applyBlur(float dx, float dy);
  CTexture* apply(CTexture* input, float global_distance, VEC4 distances, VEC4 weights);
};

#endif
