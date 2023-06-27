#pragma once

#include "mcv_platform.h"

namespace UI
{
  struct TParams
  {
    VEC2 pivot = VEC2::Zero;
    VEC2 position = VEC2::Zero;
    VEC2 scale = VEC2::One;
    float rotation = 0.f;
    bool visible = true;
  };

  struct TImageParams
  {
    const CTexture* texture = nullptr;
    VEC2 size = VEC2::One;
    bool additive = false;
    VEC4 color = VEC4::One;
    VEC2 minUV = VEC2::Zero;
    VEC2 maxUV = VEC2::One;
  };

  struct TTextParams
  {
    std::string text;
    const CTexture* texture = nullptr;
    VEC2 size = VEC2::One;
  };

  struct TProgressParams
  {
    float ratio = 1.f;
    //const std::string varName;
  };
  struct TBarParams
  {
	 float ratio = 1.f;
	 //float value;
  };
}
