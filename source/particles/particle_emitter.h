#pragma once

#include "mcv_platform.h"
#include "resources/resource.h"
#include "utils/track.h"

class CMaterial;

namespace particles
{
  struct TRange
  {
    float minValue = 0.f;
    float maxValue = 0.f;
    float getRandom() const;
  };

  struct TEmitter : public IResource
  {
    unsigned count = 1;
    unsigned maxCount = 1;
    float interval = 0.f;
    TRange duration;
    float gravityFactor = 0.f;
    VEC3 direction = VEC3::Up;
    TRange speed;
    TTrack<VEC4> colors;
    TTrack<float> sizes;
    VEC2 frameSize = VEC2::One;
    float frameTime = 0.f;
    int initialFrame = 0;
    int frameCount = 1;

    const CMaterial* material = nullptr;
    CCteBuffer<TCtesParticles>* ctes = nullptr;

    static const uint32_t max_sizes = 8;
    static const uint32_t max_colors = 8;
    void sampleColorsOverTime();

    // How to render this emitter...
    static const uint32_t invalid_id = ~0;   // All ones
    uint32_t         render_group = invalid_id;
    uint32_t         particles_type_idx = invalid_id;
    bool isRegistered() const { return render_group != invalid_id; }

    void renderInMenu() override;
    //void onFileChanged(const std::string& filename) override;
  };
}
