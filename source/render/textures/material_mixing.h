#pragma once

#include "material.h"

// ----------------------------------------------
class CMaterialMixing : public CMaterial {

  const CMaterial*  mats[3] = { nullptr, nullptr, nullptr };
  const CTexture*   mix_blend_weights = nullptr;

  CCteBuffer<TCtesMaterial> ctes_mixing;

public:
  CMaterialMixing();
  void activate() const override;
  void destroy() override;
  bool create(const json& j) override;
  void renderInMenu() override;
};

