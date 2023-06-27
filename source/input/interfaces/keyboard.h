#pragma once

#include "mcv_platform.h"
#include "input/button.h"
#include "input/enums.h"

namespace Input
{
  struct TKeyboardData
  {
    bool keys[BT_KEYBOARD_COUNT];
  };

  class CKeyboard
  {
  public:
    void update(const TKeyboardData& data, float delta)
    {
      PROFILE_FUNCTION("CKeyboard");
      for (int i = 0; i < BT_KEYBOARD_COUNT; ++i)
      {
        _buttons[i].update(data.keys[i] ? 1.f : 0.f, delta);
      }
    }

    TButton _buttons[BT_KEYBOARD_COUNT];
  };
}
