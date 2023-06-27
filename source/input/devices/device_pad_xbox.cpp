#pragma once

#include "mcv_platform.h"
#include "input/devices/device_pad_xbox.h"
#include "input/interfaces/gamepad.h"
#include "input/interfaces/rumble.h"
#include "input/enums.h"

#include <Xinput.h>
#pragma comment(lib, "xinput.lib")

namespace
{
  float getButtonState(const XINPUT_STATE& state, int buttonId)
  {
    return (state.Gamepad.wButtons & buttonId) != 0 ? 1.f : 0.f;
  }

  float getTriggerState(const XINPUT_STATE& state, bool left)
  {
    BYTE value = left ? state.Gamepad.bLeftTrigger : state.Gamepad.bRightTrigger;
    if (value <= XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
    {
      return 0.f;
    }

    return static_cast<float>(value) / std::numeric_limits<BYTE>::max();
  }

  VEC2 getAnalogState(const XINPUT_STATE& state, bool left)
  {
    VEC2 result;

    SHORT valueX = left ? state.Gamepad.sThumbLX : state.Gamepad.sThumbRX;
    SHORT valueY = left ? state.Gamepad.sThumbLY : state.Gamepad.sThumbRY;
    SHORT valueDeadzone = left ? XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE : XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE;

    result.x = static_cast<float>(valueX) / std::numeric_limits<SHORT>::max();
    result.y = static_cast<float>(valueY) / std::numeric_limits<SHORT>::max();
    float deadzone = static_cast<float>(valueDeadzone) / std::numeric_limits<SHORT>::max();

    // deal with the dead zone
    float length = result.Length();
    if (length <= deadzone)
    {
      result = VEC2::Zero;
    }
    else
    {
      float validLength = 1.f - deadzone;
      float finalLength = (length - deadzone) / validLength;
      result.Normalize();
      result = result * finalLength;
    }

    return result;
  }
}

void CDevicePadXbox::update(Input::TGamepadData& data)
{
  XINPUT_STATE state;
  data.connected = XInputGetState(_controllerId, &state) == ERROR_SUCCESS;

  if (!data.connected)
    return;

  data.buttons[Input::BT_A] = getButtonState(state, XINPUT_GAMEPAD_A);
  data.buttons[Input::BT_B] = getButtonState(state, XINPUT_GAMEPAD_B);
  data.buttons[Input::BT_X] = getButtonState(state, XINPUT_GAMEPAD_X);
  data.buttons[Input::BT_Y] = getButtonState(state, XINPUT_GAMEPAD_Y);
  data.buttons[Input::BT_LTHUMB] = getButtonState(state, XINPUT_GAMEPAD_LEFT_THUMB);
  data.buttons[Input::BT_RTHUMB] = getButtonState(state, XINPUT_GAMEPAD_RIGHT_THUMB);
  data.buttons[Input::BT_LBUTTON] = getButtonState(state, XINPUT_GAMEPAD_LEFT_SHOULDER);
  data.buttons[Input::BT_RBUTTON] = getButtonState(state, XINPUT_GAMEPAD_RIGHT_SHOULDER);
  data.buttons[Input::BT_DPAD_DOWN] = getButtonState(state, XINPUT_GAMEPAD_DPAD_DOWN);
  data.buttons[Input::BT_DPAD_UP] = getButtonState(state, XINPUT_GAMEPAD_DPAD_UP);
  data.buttons[Input::BT_DPAD_LEFT] = getButtonState(state, XINPUT_GAMEPAD_DPAD_LEFT);
  data.buttons[Input::BT_DPAD_RIGHT] = getButtonState(state, XINPUT_GAMEPAD_DPAD_RIGHT);
  data.buttons[Input::BT_SELECT] = getButtonState(state, XINPUT_GAMEPAD_BACK);
  data.buttons[Input::BT_START] = getButtonState(state, XINPUT_GAMEPAD_START);

  data.buttons[Input::BT_LTRIGGER] = getTriggerState(state, true);
  data.buttons[Input::BT_RTRIGGER] = getTriggerState(state, false);

  VEC2 leftAnalog = getAnalogState(state, true);
  data.buttons[Input::BT_LANALOG_X] = leftAnalog.x;
  data.buttons[Input::BT_LANALOG_Y] = leftAnalog.y;

  VEC2 rightAnalog = getAnalogState(state, false);
  data.buttons[Input::BT_RANALOG_X] = rightAnalog.x;
  data.buttons[Input::BT_RANALOG_Y] = rightAnalog.y;
}

void CDevicePadXbox::feedback(const Input::TRumbleData& data)
{
  XINPUT_VIBRATION vibration;
  vibration.wLeftMotorSpeed = static_cast<WORD>(data.leftVibration * std::numeric_limits<WORD>::max());
  vibration.wRightMotorSpeed = static_cast<WORD>(data.rightVibration * std::numeric_limits<WORD>::max());
  XInputSetState(_controllerId, &vibration);
}
