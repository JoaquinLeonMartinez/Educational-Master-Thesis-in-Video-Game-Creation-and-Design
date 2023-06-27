#pragma once

#include "mcv_platform.h"

namespace Input
{
  using Key = int;

  enum MouseButtons 
  { 
    BT_MOUSE_LEFT = 0,
    BT_MOUSE_RIGHT, 
    BT_MOUSE_MIDDLE, 
    BT_MOUSE_COUNT 
  };

  enum GamePadButtons
  {
    BT_A = 0,
    BT_B,
    BT_X,
    BT_Y,
    BT_START,
    BT_SELECT,
    BT_LTRIGGER,
    BT_LBUTTON,
    BT_LTHUMB,
    BT_RTRIGGER,
    BT_RBUTTON,
    BT_RTHUMB,
    BT_DPAD_UP,
    BT_DPAD_DOWN,
    BT_DPAD_LEFT,
    BT_DPAD_RIGHT,
    BT_LANALOG_X,
    BT_LANALOG_Y,
    BT_RANALOG_X,
    BT_RANALOG_Y,
    BT_GAMEPAD_COUNT
  };

  enum EInterface
  {
    INTERFACE_KEYBOARD = 0,
    INTERFACE_MOUSE,
    INTERFACE_GAMEPAD
  };

  static const int BT_KEYBOARD_COUNT = 256;

  struct TButtonDef
  {
    EInterface type;
    int buttonId;
  };
}