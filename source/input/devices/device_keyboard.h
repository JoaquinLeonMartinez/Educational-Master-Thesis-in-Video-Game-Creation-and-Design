#pragma once

#include "input/device.h"
#include "input/interfaces/keyboard.h"

class CDeviceKeyboard : public Input::IDevice
{
  Input::TKeyboardData my_data;
public:
  CDeviceKeyboard(const std::string& name)
    : Input::IDevice(name)
  {
    my_data = {};
  }
  void update(Input::TKeyboardData& data) override;
  int processWindowMsg(UINT message, WPARAM wParam, LPARAM lParam) override;
};
