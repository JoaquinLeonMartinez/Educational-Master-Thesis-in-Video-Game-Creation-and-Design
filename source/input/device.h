#pragma once

#include "mcv_platform.h"

namespace Input
{
  struct TKeyboardData;
  struct TMouseData;
  struct TGamepadData;
  struct TRumbleData;

  class IDevice 
  { 
  public:
    IDevice(const std::string& name)
      : _name(name)
    {}

    virtual void update(TKeyboardData& data) {}
    virtual void update(TMouseData& data) {}
    virtual void update(TGamepadData& data) {}

    virtual void feedback(const TRumbleData& data) {}
    virtual int processWindowMsg(UINT message, WPARAM wParam, LPARAM lParam) {
      return 0;
    }

    const std::string& getName() const { return _name; }

  private:
    std::string _name;
  };

  using VDevices = std::vector<IDevice*>;
}