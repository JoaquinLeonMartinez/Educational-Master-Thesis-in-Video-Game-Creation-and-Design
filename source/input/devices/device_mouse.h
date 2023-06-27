#pragma once

#include "input/device.h"
#include "input/enums.h"

class CDeviceMouse : public Input::IDevice
{
public:
  CDeviceMouse(const std::string& name);
  void update(Input::TMouseData& data) override;
  int processWindowMsg(UINT message, WPARAM wParam, LPARAM lParam) override;

private:
  void startRawInputData();
  bool _buttons[Input::BT_MOUSE_COUNT] = {};
  int _posX = 0;
  int _posY = 0;
  int _wheelDeltaSteps = 0;
  int _deltaX = 0;
  int _deltaY = 0;
};
