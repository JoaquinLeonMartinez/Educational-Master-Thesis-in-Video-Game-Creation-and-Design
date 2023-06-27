#pragma once

class CTexture : public IResource {

protected:
  ID3D11Resource*           texture = nullptr;
  ID3D11ShaderResourceView* shader_resource_view = nullptr;

public:
  ~CTexture() {
    destroy();
  }
  bool create(const std::string& filename);

  void destroy();
  void activate(int slot) const;
  void renderInMenu() override;

  static void setNullTexture(int slot);

  // Create a new texture from params
  enum eCreateOptions {
    CREATE_STATIC
    , CREATE_DYNAMIC
    , CREATE_RENDER_TARGET
  };
  bool create(int new_xres, int new_yres, DXGI_FORMAT new_color_format, eCreateOptions create_options = CREATE_STATIC, int mip_level = 1);
  void setDXParams(int new_xres, int new_yres, ID3D11Texture2D* new_texture, ID3D11ShaderResourceView* new_srv);
  ID3D11ShaderResourceView* getShaderResourceView() const { return shader_resource_view; }
};
