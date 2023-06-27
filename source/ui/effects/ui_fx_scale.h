#pragma once

#include "mcv_platform.h"
#include "ui/ui_effect.h"

namespace UI
{
  class CFXScale : public CEffect
  {
  public:
    void start() override;
    void stop() override;
    void update(float dt) override;

  private:
    float interpolate(float ratio) const;

    enum class EMode
    {
      Single = 0,
      Loop,
      PingPong
    };
    VEC2 _scale = VEC2::One;
    float _duration = 0.f;
    EMode _mode = EMode::Single;
    Interpolator::IInterpolator* _interpolator = nullptr;
	void changeDuration(float duration);
    float _time = 0.f;
    VEC2 _initialScale = VEC2::One;

    friend class CParser;
  };
}
