#pragma once

#include "mcv_platform.h"

namespace UI
{
  void renderBitmap(const MAT44& world, const CTexture* texture, const VEC2& minUV = VEC2::Zero, const VEC2& maxUV = VEC2::One, const VEC4& color = VEC4::One, bool additive = false);
  void renderText(const MAT44& world, const CTexture* texture, const std::string& text, const VEC2& size);
}
