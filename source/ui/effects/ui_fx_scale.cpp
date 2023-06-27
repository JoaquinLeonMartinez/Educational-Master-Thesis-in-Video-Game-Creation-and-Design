#include "mcv_platform.h"
#include "ui/effects/ui_fx_scale.h"
#include "ui/ui_widget.h"

namespace UI
{
  void CFXScale::start()
  {
    TParams* params = _owner->getParams();
    if (params)
    {
      _initialScale = params->scale;
    }
  }

  void CFXScale::stop()
  {
    TParams* params = _owner->getParams();
    if (params)
    {
      params->scale = _initialScale;
      _owner->updateTransform();
    }
  }

  void CFXScale::update(float dt)
  {
    assert(_owner);

    TParams* params = _owner->getParams();
    if (!params || _duration <= 0.f)
    {
      return;
    }

    _time += dt;

    VEC2 finalScale = params->scale;

    switch (_mode)
    {
      case EMode::Single:
      {
        if (_time < _duration)
        {
          const float ratio = _time / _duration;
          finalScale = VEC2::Lerp(_initialScale, _scale, interpolate(ratio));
        }
        else
        {
          finalScale = _scale;
        }
        break;
      }

      case EMode::Loop:
      {
        const float ratio = fmodf(_time, _duration) / _duration;
        finalScale = VEC2::Lerp(_initialScale, _scale, interpolate(ratio));
        break;
      }

      case EMode::PingPong:
      {
        const int direction = static_cast<int>(_time / _duration) % 2;
        const float ratio = fmodf(_time, _duration) / _duration;
        if (direction == 0)
        {
          finalScale = VEC2::Lerp(_initialScale, _scale, interpolate(ratio));
        }
        else
        {
          finalScale = VEC2::Lerp(_scale, _initialScale, interpolate(ratio));
        }
        break;
      }
    }

    params->scale = finalScale;
    _owner->updateTransform();
  }

  float CFXScale::interpolate(float ratio) const
  {
    return _interpolator->blend(0.f, 1.f, ratio);
  }

  void CFXScale::changeDuration(float duration) {
	  _duration = duration;

  }



}
