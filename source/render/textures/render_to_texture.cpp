#include "mcv_platform.h"
#include "render_to_texture.h"
#include "resources/resource.h"
#include "render/render_utils.h"
//#include "render/render_objects.h"      // createDepthStencil

CRenderToTexture* CRenderToTexture::current_rt = nullptr;

CRenderToTexture::~CRenderToTexture() {
}

void CRenderToTexture::destroy() {
  SAFE_RELEASE(render_target_view);
  SAFE_RELEASE(depth_stencil_view);
  SAFE_RELEASE(depth_resource);
  CTexture::destroy();
}

void CRenderToTexture::renderInMenu() {
  // If we have a color buffer...
  if(render_target_view)
    CTexture::renderInMenu();
  
  // Show the Depth Buffer if exists
  if (depth_stencil_view && getZTexture())
    getZTexture()->renderInMenu();
}


bool CRenderToTexture::createRT(
  const char* new_name
  , int new_xres
  , int new_yres
  , DXGI_FORMAT new_color_format
  , DXGI_FORMAT new_depth_format
  , bool        uses_depth_of_backbuffer
  , int         mip_level
) {
  PROFILE_FUNCTION("RTCreate");

  destroy();

  xres = new_xres;
  yres = new_yres;

  // Create color buffer
  color_format = new_color_format;
  if (color_format != DXGI_FORMAT_UNKNOWN) {
    if (!create(new_xres, new_yres, color_format, CREATE_RENDER_TARGET, mip_level))
      return false;
    setDXName(texture, new_name);
    setDXName(shader_resource_view, new_name);

    // The part of the render target view
    HRESULT hr = Render.device->CreateRenderTargetView((ID3D11Resource*) texture, nullptr, &render_target_view);
    if (FAILED(hr))
      return false;
    setDXName(render_target_view, new_name);
  }

  // Resource information
  if (name != new_name) {
    setNameAndType(new_name, getResourceTypeFor<CTexture>());
    Resources.registerResource(this);
  }

  // Create ZBuffer 
  depth_format = new_depth_format;
  if (depth_format != DXGI_FORMAT_UNKNOWN) {
    if (!createDepthStencil(getName(), xres, yres, new_depth_format, &depth_resource, &depth_stencil_view, &ztexture))
      return false;
    setDXName(depth_stencil_view, getName().c_str());
  }
  else {
    // Create can have the option to use the ZBuffer of the backbuffer
    if (uses_depth_of_backbuffer) {
      assert(xres == Render.width);
      assert(yres == Render.height);
      depth_stencil_view = Render.depth_stencil_view;
      depth_stencil_view->AddRef();
    }
  }

  return true;
}

// Will return prev rt
CRenderToTexture* CRenderToTexture::activateRT() {
  CRenderToTexture* prev_rt = current_rt;
  Render.ctx->OMSetRenderTargets(1, &render_target_view, depth_stencil_view);
  activateViewport();
  current_rt = this;
  return prev_rt;
}

// used to paint on a RT using the depth/stencil from another one
CRenderToTexture* CRenderToTexture::activateRT(CRenderToTexture* rt){
  CRenderToTexture* prev_rt = current_rt;
  Render.ctx->OMSetRenderTargets(1, &render_target_view, rt->depth_stencil_view);
  activateViewport();
  current_rt = this;
  return prev_rt;
}

void CRenderToTexture::activateViewport() {
  D3D11_VIEWPORT vp;
  vp.Width = (float)xres;
  vp.Height = (float)yres;
  vp.TopLeftX = 0;
  vp.TopLeftY = 0;
  vp.MinDepth = 0.f;
  vp.MaxDepth = 1.f;
  Render.ctx->RSSetViewports(1, &vp);
}

void CRenderToTexture::clear(VEC4 clear_color) {
  assert(render_target_view);
  Render.ctx->ClearRenderTargetView(render_target_view, &clear_color.x);
}

void CRenderToTexture::clearZ() {
  if(depth_stencil_view)
    Render.ctx->ClearDepthStencilView(depth_stencil_view, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

// ---------------------------------------------
bool createDepthStencil(
  const std::string& aname,
  int width, int height,
  DXGI_FORMAT format,
  // outputs
  ID3D11Texture2D** depth_stencil_resource,
  ID3D11DepthStencilView** depth_stencil_view,
  CTexture** out_ztexture
) {

  assert(format == DXGI_FORMAT_R32_TYPELESS
    || format == DXGI_FORMAT_R24G8_TYPELESS
    || format == DXGI_FORMAT_R16_TYPELESS
    || format == DXGI_FORMAT_D24_UNORM_S8_UINT
    || format == DXGI_FORMAT_R8_TYPELESS);

  // Crear un ZBuffer de la resolucion de mi backbuffer
  D3D11_TEXTURE2D_DESC desc;
  ZeroMemory(&desc, sizeof(desc));
  desc.Width = width;
  desc.Height = height;
  desc.MipLevels = 1;
  desc.ArraySize = 1;
  desc.Format = format;
  desc.SampleDesc.Count = 1;
  desc.SampleDesc.Quality = 0;
  desc.Usage = D3D11_USAGE_DEFAULT;
  desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
  desc.CPUAccessFlags = 0;
  desc.MiscFlags = 0;

  // The format 'DXGI_FORMAT_D24_UNORM_S8_UINT' can't be binded to shader resource
  if (format != DXGI_FORMAT_D24_UNORM_S8_UINT)
    desc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;

  // SRV = Shader Resource View
  // DSV = Depth Stencil View
  DXGI_FORMAT texturefmt = DXGI_FORMAT_R32_TYPELESS;
  DXGI_FORMAT SRVfmt = DXGI_FORMAT_R32_FLOAT;       // Stencil format
  DXGI_FORMAT DSVfmt = DXGI_FORMAT_D32_FLOAT;       // Depth format

  switch (format) {
  case DXGI_FORMAT_R32_TYPELESS:
    SRVfmt = DXGI_FORMAT_R32_FLOAT;
    DSVfmt = DXGI_FORMAT_D32_FLOAT;
    break;
  case DXGI_FORMAT_R24G8_TYPELESS:
    SRVfmt = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    DSVfmt = DXGI_FORMAT_D24_UNORM_S8_UINT;
    break;
  case DXGI_FORMAT_R16_TYPELESS:
    SRVfmt = DXGI_FORMAT_R16_UNORM;
    DSVfmt = DXGI_FORMAT_D16_UNORM;
    break;
  case DXGI_FORMAT_R8_TYPELESS:
    SRVfmt = DXGI_FORMAT_R8_UNORM;
    DSVfmt = DXGI_FORMAT_R8_UNORM;
    break;
  case DXGI_FORMAT_D24_UNORM_S8_UINT:
    SRVfmt = desc.Format;
    DSVfmt = desc.Format;
    break;
  default:
    fatal("Unsupported format creating depth buffer\n");
  }

  HRESULT hr = Render.device->CreateTexture2D(&desc, NULL, depth_stencil_resource);
  if (FAILED(hr))
    return false;
  setDXName(*depth_stencil_resource, aname.c_str());

  // Create the depth stencil view
  D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
  ZeroMemory(&descDSV, sizeof(descDSV));
  descDSV.Format = DSVfmt;
  descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
  descDSV.Texture2D.MipSlice = 0;
  hr = Render.device->CreateDepthStencilView(*depth_stencil_resource, &descDSV, depth_stencil_view);
  if (FAILED(hr))
    return false;
  setDXName(*depth_stencil_view, (aname + "_DSV").c_str());

  if (out_ztexture) {
    // Setup the description of the shader resource view.
    D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
    shaderResourceViewDesc.Format = SRVfmt;
    shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
    shaderResourceViewDesc.Texture2D.MipLevels = desc.MipLevels;

    // Create the shader resource view.
    ID3D11ShaderResourceView* depth_resource_view = nullptr;
    hr = Render.device->CreateShaderResourceView(*depth_stencil_resource, &shaderResourceViewDesc, &depth_resource_view);
    if (FAILED(hr))
      return false;

    CTexture* ztexture = new CTexture();
    ztexture->setDXParams(width, height, *depth_stencil_resource, depth_resource_view);
    ztexture->setNameAndType("Z" + aname, getResourceTypeFor<CTexture>());
    Resources.registerResource(ztexture);
    setDXName(*depth_stencil_resource, (ztexture->getName() + "_DSR").c_str());
    setDXName(depth_resource_view, (ztexture->getName() + "_DRV").c_str());

    // The ztexture already got the reference
    depth_resource_view->Release();
    *out_ztexture = ztexture;
  }

  return true;
}

