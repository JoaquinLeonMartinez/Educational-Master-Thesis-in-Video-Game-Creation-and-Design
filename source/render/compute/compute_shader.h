#ifndef INC_RENDER_PIXEL_SHADER_H_
#define INC_RENDER_PIXEL_SHADER_H_

#include <d3d11shader.h>
#include "resources/resource.h"

// -----------------------------------------
class CComputeShader : public IResource {
  
  ID3D11ComputeShader*     cs = nullptr;
  json                     jdef;

  bool create(
    const std::string& filename
  , const std::string& fn_entry_name
  , const std::string& profile
  );

public:
  std::vector<D3D11_SHADER_INPUT_BIND_DESC> bound_resources;
  uint32_t                                  thread_group_size[3];

  void onFileChanged(const std::string& filename) override;
  bool create(const json& j);
  void destroy();
  void activate() const;
  static void deactivate();
  void renderInMenu();
};

#endif
