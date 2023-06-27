#pragma once

#include "mcv_platform.h"
#include "particles/particle.h"

namespace particles
{
  struct TEmitter;

  class CSystem
  {
  public:
    CSystem(const TEmitter* emitter);
    void launch();
    // Returns number of particles written to target_render_data
    TRenderData* update(float dt, TRenderData* target_render_data);
    void setOwner(CHandle owner) { _owner = owner; }

    friend class CModuleParticles;

  private:
    TRenderData* emit(TRenderData *out);
    CTransform* getOwnerTransform();

    std::vector<TParticle> _particles;
    const TEmitter* _emitter = nullptr;
    CHandle _owner;
    float _time = 0.f;

    friend class CParser;
  };
}
