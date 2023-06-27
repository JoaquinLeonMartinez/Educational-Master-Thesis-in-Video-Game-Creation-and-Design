#pragma once

#include "mcv_platform.h"
#include "particles/particle_system.h"
#include "particles/particle_emitter.h"
#include "components/common/comp_transform.h"
#include "components/common/comp_camera.h"
#include "entity/entity.h"
#include "engine.h"
#include "render/module_render.h"
#include "render/render_manager.h"
#include <random>
#include <ctime>



namespace particles
{


  float TRange::getRandom() const
  {
    static std::mt19937_64 generator(std::time(0));
    static std::uniform_real_distribution<float> uniformDistribution;
    const float unitRandomValue = uniformDistribution(generator);
    return minValue + (maxValue - minValue) * unitRandomValue;
  }

  CSystem::CSystem(const TEmitter* emitter)
    : _emitter(emitter)
  {
  }

  TRenderData* CSystem::update(float dt, TRenderData* out)
  {
    const VEC3 kGravity(0.f, -9.8f, 0.f);

    CTransform* cTransform = getOwnerTransform();
    const float ownerScale = cTransform ? cTransform->getScale() : 1.f;

    for (auto it = _particles.begin(); it != _particles.end(); )
    {
      TParticle& p = *it;

      p.lifetime += dt;

      if (p.duration > 0.f && p.lifetime >= p.duration)
      {
        it = _particles.erase(it);
      }
      else
      {
        const float timeRatio = p.duration != 0.f ? p.lifetime / p.duration : 1.f;

        p.velocity += kGravity * _emitter->gravityFactor * dt;
        p.position += p.velocity * dt;
        p.scale = _emitter->sizes.get(timeRatio) * ownerScale;
        p.color = _emitter->colors.get(timeRatio);    // Not used anymore

        ++it;

        // Save render information -----------
        out->center = p.position;
        out->color = p.color;
        out->time = timeRatio;
        ++out;
        // -----------------------------------
      }
    }

    _time += dt;
    if (_time >= _emitter->interval)
    {
      out = emit(out);
      _time -= _emitter->interval;
    }

    return out;
  }

  void CSystem::launch()
  {
    emit(nullptr);
  }

  TRenderData* CSystem::emit(TRenderData *out)
  {
    CTransform* cTransform = getOwnerTransform();
    
    const VEC3 ownerPosition = cTransform ? cTransform->getPosition() : VEC3::Zero;
    const float ownerScale = cTransform ? cTransform->getScale() : 1.f;
    
    for (size_t i = 0; i < _emitter->count && _particles.size() < _emitter->maxCount; ++i)
    {
      TParticle p;
      p.position = ownerPosition;
      p.velocity = _emitter->direction * _emitter->speed.getRandom();
      p.duration = _emitter->duration.getRandom();
      p.color = _emitter->colors.get(0.f);
      p.scale = _emitter->sizes.get(0.f) * ownerScale;

      _particles.emplace_back(std::move(p));

      // Save render information -----------
      if (out) {
        out->center = p.position;
        out->color = p.color;
        out->time = 0.f;
        ++out;
      }
      // -----------------------------------

    }
    return out;
  }

  CTransform* CSystem::getOwnerTransform()
  {
    TCompTransform* cTransform = nullptr;
    if (!_owner.isValid())
    {
      return nullptr;
    }

    CEntity* e = _owner;
    cTransform = e->get<TCompTransform>();

    return cTransform;
  }

}
