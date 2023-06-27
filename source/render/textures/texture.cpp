#include "mcv_platform.h"
#include "texture.h"
#include "DDSTextureLoader.h"
#include "render/render.h"

class CTextureResourceType : public CResourceType {
public:
  const char* getExtension(int idx) const override { return "dds"; }
  const char* getName() const override {
    return "Textures";
  }
  IResource* create(const std::string& name) const override {
    CTexture* new_res = new CTexture();
    bool is_ok = new_res->create(name);
    if (!is_ok)
      return nullptr;
    new_res->setNameAndType(name, this);
    return new_res;
  }
};

template<>
const CResourceType* getResourceTypeFor<CTexture>() {
  static CTextureResourceType resource_type;
  return &resource_type;
}



// ----------------------------------------------------------
bool CTexture::create(const std::string& name) {

  // Convert to UTF16
  wchar_t wFilename[MAX_PATH];
  mbstowcs(wFilename, name.c_str(), name.length() + 1);

  HRESULT hr = DirectX::CreateDDSTextureFromFile(
    Render.device,
    wFilename,
    &texture,
    &shader_resource_view
  );
  if (FAILED(hr))
    return false;

  // Name both objects in DX
  setDXName(texture, name.c_str());
  setDXName(shader_resource_view, name.c_str());

  // Update the resolution from the resource
  return true;
}

void CTexture::destroy() {
  SAFE_RELEASE(shader_resource_view);
  SAFE_RELEASE(texture);
}

void CTexture::setNullTexture(int slot) {
  ID3D11ShaderResourceView* null_texture = nullptr;
  Render.ctx->PSSetShaderResources(slot, 1, &null_texture);
}

void CTexture::activate( int slot ) const {
  Render.ctx->PSSetShaderResources(slot, 1, &shader_resource_view);
}

void CTexture::renderInMenu() {
  //ImGui::Text("Resolution is ..");
  ImGui::Image(shader_resource_view, ImVec2(128, 128));
}

// ------------------------------------------------
bool CTexture::create(
  int nxres
  , int nyres
  , DXGI_FORMAT nformat
  , eCreateOptions options
  , int mip_level
)
{
  //xres = nxres;
  //yres = nyres;

  D3D11_TEXTURE2D_DESC desc;
  ZeroMemory(&desc, sizeof(desc));
  desc.Width = nxres;
  desc.Height = nyres;
  desc.MipLevels = mip_level;
  desc.ArraySize = 1;
  desc.Format = nformat;
  desc.SampleDesc.Count = 1;
  desc.SampleDesc.Quality = 0;
  desc.Usage = D3D11_USAGE_DEFAULT;
  desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
  desc.CPUAccessFlags = 0;
  desc.MiscFlags = 0;

  if (options == CREATE_DYNAMIC) {
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  }
  else if (options == CREATE_RENDER_TARGET) {
    desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
  }
  else {
    assert(options == CREATE_STATIC);
  }

  if (mip_level > 1) {
    desc.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
  }

  ID3D11Texture2D* tex2d = nullptr;
  HRESULT hr = Render.device->CreateTexture2D(&desc, nullptr, &tex2d);
  if (FAILED(hr))
    return false;
  texture = tex2d;
  setDXName(texture, getName().c_str());

  // -----------------------------------------
  // Create a resource view so we can use the data in a shader
  D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc;
  ZeroMemory(&srv_desc, sizeof(srv_desc));
  srv_desc.Format = nformat;
  srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
  srv_desc.Texture2D.MipLevels = desc.MipLevels;
  hr = Render.device->CreateShaderResourceView(texture, &srv_desc, &shader_resource_view);
  if (FAILED(hr))
    return false;
  setDXName(shader_resource_view, getName().c_str());
  return true;
}

void CTexture::setDXParams(int new_xres, int new_yres, ID3D11Texture2D* new_texture, ID3D11ShaderResourceView* new_srv) {
  //xres = new_xres;
  //yres = new_yres;
  texture = new_texture;
  shader_resource_view = new_srv;
  new_texture->AddRef();
  new_srv->AddRef();
}
