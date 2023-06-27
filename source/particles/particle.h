#pragma once

#include "mcv_platform.h"

namespace particles
{
  // CPU information (more than what we need to render...)
  struct TParticle
  {
    VEC4 color = VEC4::One;
    VEC3 position = VEC3::Zero;
    float scale = 1.f;
    VEC3 velocity = VEC3::Zero;
    float lifetime = 0.f;
    float duration = 0.f;
  };

  // ----------------------------------------------
  // Basic struct to render each particle
  struct TRenderData {
    float3 center;
    float  time;
    float4 color;
  };

}
