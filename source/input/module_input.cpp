#include "mcv_platform.h"
#include "input/module_input.h"
#include "input/devices/device_keyboard.h"
#include "input/devices/device_mouse.h"

namespace Input
{
  std::map<std::string, TButtonDef> CModuleInput::_buttonDefinitions;
  const TButton TButton::dummy;

  CModuleInput::CModuleInput(const std::string& name)
    : IModule(name)
    , _mapping(*this)
  {
    if (_buttonDefinitions.empty())
    {
      registerButtonDefinitions();
    }
  }

  bool CModuleInput::start()
  {
    return true;
  }

  void CModuleInput::update(float dt)
  {
    TKeyboardData keyboardData = {};
    TMouseData mouseData = {};
    TGamepadData gamepadData = {};

    for (auto device : _devices)
    {
      PROFILE_FUNCTION(device->getName().c_str());
      device->update(keyboardData);
      device->update(mouseData);
      device->update(gamepadData);
    }

    _keyboard.update(keyboardData, dt);
    _mouse.update(mouseData, dt);
    _gamepad.update(gamepadData, dt);
    _mapping.update(dt);
  }

  void CModuleInput::registerDevice(IDevice* device)
  {
    _devices.push_back(device);
  }

  IDevice* CModuleInput::getDevice(const std::string& name)
  {
    for (auto& device : _devices)
    {
      if (device->getName() == name)
      {
        return device;
      }
    }
    return nullptr;
  }

  void CModuleInput::assignMapping(const std::string& mappingFile)
  {
    _mapping.load(mappingFile);
  }

  void CModuleInput::feedback(const TRumbleData& data)
  {
    for (auto& device : _devices)
    {
      device->feedback(data);
    }
  }

  const TButton& CModuleInput::operator[](Key key) const
  {
    return _keyboard._buttons[key];
  }

  const TButton& CModuleInput::operator[](MouseButtons bt) const
  {
    return _mouse._buttons[bt];
  }

  const TButton& CModuleInput::operator[](GamePadButtons bt) const
  {
    return _gamepad._buttons[bt];
  }

  const TButton& CModuleInput::operator[](const std::string& name) const
  {
    return _mapping.getMappedButton(name);
  }

  const TButton& CModuleInput::operator[](const TButtonDef& def) const
  {
    if (def.type == EInterface::INTERFACE_KEYBOARD)     return (*this)[(Key)def.buttonId];
    else if (def.type == EInterface::INTERFACE_MOUSE)   return (*this)[(MouseButtons)def.buttonId];
    else if (def.type == EInterface::INTERFACE_GAMEPAD) return (*this)[(GamePadButtons)def.buttonId];
    else                                                return TButton::dummy;
  }

  void CModuleInput::registerButtonDefinitions()
  {
    _buttonDefinitions.clear();

    auto parseButtonDefinitions = [](const json& jData, EInterface type)
    {
      int i = 0;
      for (auto& name : jData)
      {
        _buttonDefinitions[name] = { type, i++ };
      }
    };

    auto jData = loadJson("data/input/button_definitions.json");

    parseButtonDefinitions(jData["keyboard"], INTERFACE_KEYBOARD);
    parseButtonDefinitions(jData["mouse"], INTERFACE_MOUSE);
    parseButtonDefinitions(jData["gamepad"], INTERFACE_GAMEPAD);
  }

  const TButtonDef* CModuleInput::getButtonDefinition(const std::string& name)
  {
    const auto it = _buttonDefinitions.find(name);
    return it != _buttonDefinitions.end() ? &it->second : nullptr;
  }

  const std::string& CModuleInput::getButtonName(EInterface type, int buttonId)
  {
    for (auto& btDef : _buttonDefinitions)
    {
      if (btDef.second.type == type && btDef.second.buttonId == buttonId)
      {
        return btDef.first;
      }
    }
    static std::string dummy;
    return dummy;
  }

  void CModuleInput::renderInMenu()
  {
    const auto printButton = [](const TButton& button, const char* label, bool centerValue = false)
    {
      float value = centerValue ? 0.5f + button.value * 0.5f : button.value;
      ImGui::ProgressBar(value, ImVec2(-1, 0), label);
    };

    if (ImGui::TreeNode("Input"))
    {
      if (ImGui::TreeNode("keyboard"))
      {
        for (int i = 0; i < Input::BT_KEYBOARD_COUNT; ++i)
        {
          printButton(_keyboard._buttons[i], getButtonName(INTERFACE_KEYBOARD, i).c_str());
        }
        ImGui::TreePop();
      }
      if (ImGui::TreeNode("mouse"))
      {
        ImGui::Text("Current position: %.0f %.0f", _mouse._currPosition.x, _mouse._currPosition.y);
        ImGui::Text("Previous position: %.0f %.0f", _mouse._prevPosition.x, _mouse._prevPosition.y);
		ImGui::Text("Delta: %.0f %.0f", _mouse.getDelta().x, _mouse.getDelta().y);
        for (int i = 0; i< Input::BT_MOUSE_COUNT; ++i)
        {
          printButton(_mouse._buttons[i], getButtonName(INTERFACE_MOUSE, i).c_str());
        }
        ImGui::TreePop();
      }
      if (ImGui::TreeNode("pad"))
      {
        ImGui::Text("Connected: %s", _gamepad._connected ? "YES" : "no");
        for (int i = 0; i < Input::BT_GAMEPAD_COUNT; ++i)
        {
          bool isAnalog = i == Input::BT_LANALOG_X || i == Input::BT_LANALOG_Y || i == Input::BT_RANALOG_X || i == Input::BT_RANALOG_Y;
          printButton(_gamepad._buttons[i], getButtonName(INTERFACE_GAMEPAD, i).c_str(), isAnalog);
        }
        ImGui::TreePop();
      }
      if (ImGui::TreeNode("Mapping"))
      {
        for (auto& bt : _mapping._buttons)
        {
          printButton(bt.second.result, bt.first.c_str());
        }
        ImGui::TreePop();
      }
      ImGui::TreePop();
    }
  }

  LRESULT CModuleInput::OnOSMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {

    bool processed = false;
    for (auto& device : _devices)
      processed |= device->processWindowMsg(msg, wParam, lParam) ? true : false;

    return processed;
  }


}
