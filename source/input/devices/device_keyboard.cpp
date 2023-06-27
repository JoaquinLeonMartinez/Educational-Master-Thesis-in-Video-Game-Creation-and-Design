#pragma once

#include "mcv_platform.h"
#include "input/devices/device_keyboard.h"
#include "input/interfaces/keyboard.h"
#include "input/enums.h"

void CDeviceKeyboard::update(Input::TKeyboardData& data)
{
  data = my_data;
}

int CDeviceKeyboard::processWindowMsg(UINT message, WPARAM wParam, LPARAM lParam)
{
  if (message == WM_KEYDOWN) {
    my_data.keys[wParam] = true;
    return 1;
  }
  else if (message == WM_KEYUP) {
    my_data.keys[wParam] = false;
    return 1;
  }
  return 0;
}
