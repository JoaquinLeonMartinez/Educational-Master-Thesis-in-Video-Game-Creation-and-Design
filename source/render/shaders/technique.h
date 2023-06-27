#pragma once

#include "shader.h"
#include "resources/resource.h"
#include "render/render_utils.h"

class CTechnique : public IResource {

  CVertexShader vs;
  CPixelShader  ps;

  // Rasterizer/Blend & Depth status
  RSConfig      rs_config = RSConfig::RSCFG_DEFAULT;
  BlendConfig   blend_config = BlendConfig::BLEND_CFG_DEFAULT;
  ZConfig       z_config = ZConfig::ZCFG_DEFAULT;

  json          jdef;

  bool loadVS(const json& j);
  bool loadPS(const json& j);

  bool          uses_skin = false;

  // Textures to activate when the technique is activated
  struct TTextureSlot {
    const CTexture* texture = nullptr;
    int             slot = 0;
  };
  std::vector< TTextureSlot > textures;

public:
  
  static const CTechnique* current_active_tech;
  const json& getDefinition() const { return jdef; }

  void onFileChanged(const std::string& filename) override;
  void renderInMenu() override;

  bool create(const std::string& filename);
  bool create(const json& jdef);
  void activate() const;
  void destroy();

  bool usesSkin() const {
    return uses_skin;
  }
  const CVertexDeclaration* getVertexDecl() const { return vs.getVertexDecl(); }
};