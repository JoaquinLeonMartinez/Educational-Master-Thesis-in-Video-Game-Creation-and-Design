#pragma once

#include "mcv_platform.h"

namespace Input
{
  struct TButton
  {
    float value = 0.f;
    float prevValue = 0.f;
    float timePressed = 0.f;

    bool isPressed() const
    {
      return value != 0.f;
    }

    bool wasPressed() const
    {
      return prevValue != 0.f;
    }

    bool justPressed() const
    {
      return prevValue == 0.f && value != 0.f;
    }

    bool justReleased() const
    {
      return prevValue != 0.f && value == 0.f;
    }

    bool justChanged() const
    {
      return prevValue != value;
    }

    void update(float newValue, float dt)
    {
      prevValue = value;
      value = newValue;

      if (justPressed())
      {
        timePressed = 0.f;
      }
      else if (isPressed())
      {
        timePressed += dt;
      }
    }

    operator bool() const
    {
      return isPressed();
    }

    static const TButton dummy;
  };
}
