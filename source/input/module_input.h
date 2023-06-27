#pragma once

#include "modules/module.h"
#include "input/interfaces/gamepad.h"
#include "input/interfaces/keyboard.h"
#include "input/interfaces/mouse.h"
#include "input/interfaces/rumble.h"
#include "input/interfaces/mapping.h"
#include "input/device.h"

// we are treating this as a single controller
// if we want more controllers, create multiple
// instances of this class

namespace Input
{
  class CModuleInput : public IModule
  {
  public:
    CModuleInput(const std::string& name);

    bool start() override;
    void update(float dt) override;
    void renderInMenu() override;

    const CKeyboard& keyboard() const { return _keyboard; }
    const CMouse& mouse() const { return _mouse; }
    const CGamepad& gamepad() const { return _gamepad; }
    const CMapping& mapping() const { return _mapping; }

    void feedback(const TRumbleData& data);

    const TButton& operator[](Key key) const;
    const TButton& operator[](MouseButtons bt) const;
    const TButton& operator[](GamePadButtons bt) const;
    const TButton& operator[](const std::string& name) const;
    const TButton& operator[](const TButtonDef& def) const;

    void registerDevice(IDevice* device);
    IDevice* getDevice(const std::string& name);

    void assignMapping(const std::string& mappingFile);

    static void registerButtonDefinitions();
    static const TButtonDef* getButtonDefinition(const std::string& name);
    static const
      std::string& getButtonName(EInterface type, int buttonId);

    LRESULT OnOSMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

  private:
    VDevices _devices;

    CKeyboard _keyboard;
    CMouse _mouse;
    CGamepad _gamepad;
    CMapping _mapping;

    static std::map<std::string, TButtonDef> _buttonDefinitions;
  };
}
