#pragma once

#include "mcv_platform.h"
#include "particles/particle_emitter.h"

namespace particles
{
  class CParser
  {
  public:
    bool loadEmitter(TEmitter& emitter, const json& jData);

    TRange loadRange(const json& jData, const std::string& attr, const TRange& defaultValue);
  };
}
