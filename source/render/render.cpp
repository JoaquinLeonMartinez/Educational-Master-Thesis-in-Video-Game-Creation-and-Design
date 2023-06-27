#include "mcv_platform.h"
#include "render.h"

#pragma comment(lib,"d3d11")
#pragma comment(lib,"dxguid.lib")   // WKPDID_D3DDebugObjectName in release
#pragma comment(lib,"d3d9")         // For the perf funcs

CRender Render;

bool CRender::resizeBackBuffer(int new_width, int new_height) {
  if (width == new_width && height == new_height)
    return false;
  width = new_width;
  height = new_height;
  destroyRenderTarget();
  dbg("Resizing backbuffer to %dx%d\n", width, height);
  HRESULT hr = swap_chain->ResizeBuffers(0, (UINT)width, (UINT)height, DXGI_FORMAT_UNKNOWN, 0);
  if (FAILED(hr))
    return false;
  createRenderTarget();
  createBackBuffer();
  return true;
}

bool CRender::createBackBuffer() {
  HRESULT hr;
  // Crear un ZBuffer de la resolucion de mi backbuffer
  D3D11_TEXTURE2D_DESC desc;
  ZeroMemory(&desc, sizeof(desc));
  desc.Width = width;
  desc.Height = height;
  desc.MipLevels = 1;
  desc.ArraySize = 1;
  desc.Format = DXGI_FORMAT_R24G8_TYPELESS; // was DXGI_FORMAT_D24_UNORM_S8_UINT;
  desc.SampleDesc.Count = 1;
  desc.SampleDesc.Quality = 0;
  desc.Usage = D3D11_USAGE_DEFAULT;
  desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
  desc.CPUAccessFlags = 0;
  desc.MiscFlags = 0;

  hr = Render.device->CreateTexture2D(&desc, NULL, &depth_texture);
  if (FAILED(hr))
    return false;
  setDXName(depth_texture, "SwapChain.Z");

  // Create the depth stencil view
  D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
  ZeroMemory(&descDSV, sizeof(descDSV));
  descDSV.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
  descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
  hr = Render.device->CreateDepthStencilView(depth_texture, &descDSV, &depth_stencil_view);
  if (FAILED(hr))
    return false;
  setDXName(depth_stencil_view, "SwapChain.Z.dsv");

  // -----------------------------------------
  // Create a resource view so we can use the data in a shader
  D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc;
  ZeroMemory(&srv_desc, sizeof(srv_desc));
  srv_desc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
  srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
  srv_desc.Texture2D.MipLevels = desc.MipLevels;
  hr = Render.device->CreateShaderResourceView(depth_texture, &srv_desc, &depth_srv);
  if (FAILED(hr))
    return false;
  setDXName(depth_srv, "SwapChain.Z.srv");

  return true;
}


void CRender::destroyRenderTarget() {
  SAFE_RELEASE(render_target_view);
}

bool CRender::createRenderTarget() {
  // Create a render target view
  ID3D11Texture2D* pBackBuffer = nullptr;
  HRESULT hr = swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
  if (FAILED(hr))
    return false;
  hr = device->CreateRenderTargetView(pBackBuffer, nullptr, &render_target_view);
  pBackBuffer->Release();
  if (FAILED(hr))
    return false;
  return true;
}

bool CRender::create(HWND hWnd, int new_width, int new_height) {
  PROFILE_FUNCTION("Render::Create");

  width = new_width;
  height = new_height;

  dbg("Creating render device %dx%d\n", width, height);

  UINT createDeviceFlags = 0;
#ifdef DEBUG
  createDeviceFlags = D3D11_CREATE_DEVICE_DEBUG;
#endif

  D3D_FEATURE_LEVEL featureLevels[] =
  {
      D3D_FEATURE_LEVEL_11_0,
  };
  UINT numFeatureLevels = ARRAYSIZE(featureLevels);

  DXGI_SWAP_CHAIN_DESC sd;
  ZeroMemory(&sd, sizeof(sd));
  sd.BufferCount = 1;
  sd.BufferDesc.Width = width;
  sd.BufferDesc.Height = height;
  sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  sd.BufferDesc.RefreshRate.Numerator = 60;
  sd.BufferDesc.RefreshRate.Denominator = 1;
  sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  sd.OutputWindow = hWnd;
  sd.SampleDesc.Count = 1;
  sd.SampleDesc.Quality = 0;
  sd.Windowed = TRUE;

  D3D_FEATURE_LEVEL  featureLevel = D3D_FEATURE_LEVEL_11_0;

  HRESULT hr = D3D11CreateDeviceAndSwapChain(
    nullptr,
    D3D_DRIVER_TYPE_HARDWARE,
    nullptr,
    createDeviceFlags, featureLevels, numFeatureLevels,
    D3D11_SDK_VERSION, &sd, &swap_chain, &device, &featureLevel, &ctx);

  if (FAILED(hr))
    return false;

  if( !createRenderTarget() )
    return false;
  
  if (!createBackBuffer())
    return false;

  // 
  ID3D11SamplerState* samplerstate;
  D3D11_SAMPLER_DESC sampDesc;
  ZeroMemory(&sampDesc, sizeof(sampDesc));
  sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
  sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
  sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
  sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
  sampDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
  sampDesc.MinLOD = 0;
  sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
  hr = Render.device->CreateSamplerState(
    &sampDesc, &samplerstate);
  if (FAILED(hr))
    return false;
  Render.ctx->PSSetSamplers( 0, 1, &samplerstate);

  rmt_BindD3D11(Render.device, Render.ctx);
  return true;
}

void CRender::destroy() {
  rmt_UnbindD3D11();
  if (ctx) ctx->ClearState();
  SAFE_RELEASE(depth_stencil_view);
  SAFE_RELEASE(depth_texture);
  destroyRenderTarget();
  SAFE_RELEASE(swap_chain);
  SAFE_RELEASE(ctx);
  SAFE_RELEASE(device);
}

void CRender::startRenderingBackBuffer() {

  assert(width > 0);
  assert(height > 0);

  ctx->OMSetRenderTargets(1, &render_target_view, depth_stencil_view );

  // Setup the viewport
  D3D11_VIEWPORT vp;
  vp.Width = (FLOAT)width;
  vp.Height = (FLOAT)height;
  vp.MinDepth = 0.0f;
  vp.MaxDepth = 1.0f;
  vp.TopLeftX = 0;
  vp.TopLeftY = 0;
  ctx->RSSetViewports(1, &vp);
}

void CRender::clearBackground( VEC4 color ) {
  // Clear the back buffer 
  float ClearColor[4] = { color.x, color.y, color.z, color.w }; // red,green,blue,alpha
  ctx->ClearRenderTargetView(render_target_view, ClearColor);
  ctx->ClearDepthStencilView(depth_stencil_view, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0 );
}

void CRender::swapChain() {
  PROFILE_FUNCTION("CRender::swapChain");
  // Present the information rendered to the back buffer to the front buffer (the screen)
  swap_chain->Present(0, 0);
}
