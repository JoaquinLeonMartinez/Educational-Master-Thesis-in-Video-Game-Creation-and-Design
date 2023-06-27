#pragma once

#include "render/meshes/vertex_declarations.h"
#include "render/shaders/technique.h"
#include "render/gpu_trace.h"
#include "render/meshes/mesh.h"

class CTexture;

class CRender {

  IDXGISwapChain*           swap_chain = nullptr;
  ID3D11RenderTargetView*   render_target_view = nullptr;
  bool createRenderTarget();
  void destroyRenderTarget();
  bool createBackBuffer();

public:

  ID3D11Texture2D*          depth_texture = nullptr;
  ID3D11DepthStencilView*   depth_stencil_view = nullptr;
  ID3D11ShaderResourceView* depth_srv = nullptr;

  ID3D11Device*           device = nullptr;
  ID3D11DeviceContext*    ctx = nullptr;

  int  width = 0;
  int  height = 0;

  bool create(HWND hWnd, int width, int height);
  void destroy();
  void clearBackground(VEC4 color);
  void swapChain();
  bool resizeBackBuffer(int width, int height);

  void startRenderingBackBuffer();
};

#define SAFE_RELEASE(x) if(x) { (x)->Release(); x = nullptr; }

// To assign a name to each DX object
#define setDXName(dx_obj,new_name) \
        (dx_obj)->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)strlen(new_name), new_name);

extern CRender Render;

enum eRenderCategory {
  CATEGORY_SOLIDS,
  CATEGORY_DISTORSIONS,
  CATEGORY_TRANSPARENTS,
  CATEGORY_DECALS,
  CATEGORY_UI,
  CATEGORY_SHADOWS,
  CATEGORY_SHINE,
  // ...
};

#include "render/render_utils.h"
#include "render/primitives.h"
#include "render/textures/render_to_texture.h"

