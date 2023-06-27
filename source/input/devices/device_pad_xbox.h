#pragma once

#include "input/device.h"
#include "input/enums.h"

class CDevicePadXbox : public Input::IDevice
{
public:
  CDevicePadXbox(const std::string& name, int controllerId)
    : Input::IDevice(name)
    , _controllerId(controllerId)
  {}

  void update(Input::TGamepadData& data) override;
  void feedback(const Input::TRumbleData& data) override;

private:
  int _controllerId = 0;
};
