#include "mcv_platform.h"
#include "render/render.h"
#include "compile.h"
#include "shader.h"
#include "render/meshes/vertex_declarations.h"

bool CVertexShader::create(const std::string& source
  , const std::string& entry_fn
  , const std::string& vertex_decl_name
) {
  HRESULT hr;

  // Compile the vertex shader
  ID3DBlob* pVSBlob = nullptr;
  hr = CompileShaderFromFile(source, entry_fn, "vs_4_0", &pVSBlob);
  if (FAILED(hr))
  {
    MessageBox(nullptr,
      "The FX file cannot be compiled. Please run this executable from the directory that contains the FX file.", "Error", MB_OK);
    return hr;
  }

  // Create the vertex shader
  hr = Render.device->CreateVertexShader(
    pVSBlob->GetBufferPointer(), 
    pVSBlob->GetBufferSize(), 
    nullptr,
    &vs
  );
  if (FAILED(hr))
  {
    pVSBlob->Release();
    return false;
  }

  vertex_decl = getVertexDeclByName(vertex_decl_name);
  assert(vertex_decl || fatal("Unsupported vertex declaration %s\n", vertex_decl_name.c_str()));


  // Create the input layout
  hr = Render.device->CreateInputLayout(
    vertex_decl->layout, 
    vertex_decl->numElements, 
    pVSBlob->GetBufferPointer(),
    pVSBlob->GetBufferSize(), 
    &vertex_layout
  );
  pVSBlob->Release();
  if (FAILED(hr))
    return false;

  setDXName(vs, (entry_fn + "@" + source).c_str());
  setDXName(vertex_layout, vertex_decl_name.c_str());

  return true;
}

void CVertexShader::destroy() {
  SAFE_RELEASE(vertex_layout);
  SAFE_RELEASE(vs);
}

void CVertexShader::activate() const {
  assert(vs);
  // Set the input layout
  Render.ctx->IASetInputLayout(vertex_layout);
  Render.ctx->VSSetShader(vs, nullptr, 0);
}

void CVertexShader::deactivateResources() {
  // Can't update a buffer if it's still bound in a vs. So unbound it.
  ID3D11ShaderResourceView* srv_nulls[3] = { nullptr, nullptr, nullptr };
  Render.ctx->VSSetShaderResources(0, 3, srv_nulls);
}

bool CPixelShader::create(const std::string& source, const std::string& entry_fn) {
  HRESULT hr;
  // Compile the pixel shader
  ID3DBlob* pPSBlob = nullptr;
  hr = CompileShaderFromFile(source, entry_fn, "ps_4_0", &pPSBlob);
  if (FAILED(hr))
  {
    MessageBox(nullptr,
      "The FX file cannot be compiled. Please run this executable from the directory that contains the FX file.", "Error", MB_OK);
    return false;
  }

  // Create the pixel shader
  hr = Render.device->CreatePixelShader(
    pPSBlob->GetBufferPointer(), 
    pPSBlob->GetBufferSize(), 
    nullptr,
    &ps
  );
  pPSBlob->Release();
  if (FAILED(hr))
    return false;

  setDXName(ps, (entry_fn + "@" + source ).c_str());
  return true;
}

void CPixelShader::destroy() {
  SAFE_RELEASE(ps);
}

void CPixelShader::activate() const {
  assert(ps);
  Render.ctx->PSSetShader(ps, nullptr, 0);
}

