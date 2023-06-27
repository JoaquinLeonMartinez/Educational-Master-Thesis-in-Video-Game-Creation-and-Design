#ifndef INC_RENDER_RENDER_TO_TEXTURE_H_
#define INC_RENDER_RENDER_TO_TEXTURE_H_

#include "texture.h"

class CRenderToTexture : public CTexture {

  DXGI_FORMAT              color_format = DXGI_FORMAT_UNKNOWN;
  DXGI_FORMAT              depth_format = DXGI_FORMAT_UNKNOWN;

  ID3D11RenderTargetView*  render_target_view = nullptr;
  ID3D11DepthStencilView*  depth_stencil_view = nullptr;

  // To be able to use the ZBuffer as a texture from our material system
  ID3D11Texture2D*         depth_resource = nullptr;
  CTexture*                ztexture = nullptr;

  int                      xres = 0;
  int                      yres = 0;

  static CRenderToTexture* current_rt;

public:

  ~CRenderToTexture();

  bool createRT(const char* name, int new_xres, int new_yres
    , DXGI_FORMAT new_color_format
    , DXGI_FORMAT new_depth_format = DXGI_FORMAT_UNKNOWN
    , bool        uses_depth_of_backbuffer = false
    , int         mip_level = 1
  );
  void destroy();

  CRenderToTexture* activateRT();
  CRenderToTexture* activateRT(CRenderToTexture* rt);

  void activateViewport();

  void clear(VEC4 clear_color);
  void clearZ();

  ID3D11RenderTargetView* getRenderTargetView() {
    return render_target_view;
  }
  CTexture* getZTexture() { return ztexture; }
  static CRenderToTexture* getCurrentRT() { return current_rt; }

  int getWidth() const { return xres; }
  int getHeight() const { return yres; }
  ID3D11DepthStencilView* getDepthStencilView() const { return depth_stencil_view; }

  void renderInMenu() override;
};

#endif
