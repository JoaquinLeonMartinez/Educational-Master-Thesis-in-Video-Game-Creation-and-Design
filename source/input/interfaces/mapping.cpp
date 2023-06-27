#include "mcv_platform.h"
#include "input/module_input.h"

namespace Input
{
  CMapping::CMapping(CModuleInput& controller)
    : _controller(controller)
  {
  }

  void CMapping::update(float delta)
  {
    PROFILE_FUNCTION("CMapping");
    for (auto& mappedBt : _buttons)
    {
      TMappedButton& mapBt = mappedBt.second;

      float value = 0.f;
      bool all = true;
      for (auto& bt : mapBt.buttons)
      {
        all &= bt->isPressed();

        // get the highest value of all the defined buttons
        if (fabsf(bt->value) > fabsf(value))
        {
          value = bt->value;
        }
      }

      if (mapBt.checkAll && !all)
      {
        value = 0.f;
      }

      mapBt.result.update(value, delta);
    }
  }

  const TButton& CMapping::getMappedButton(const std::string& name) const
  {
    // check in the custom mapped buttons
    auto it = _buttons.find(name);
    if (it != _buttons.end())
    {
      return it->second.result;
    }

    // check in the system-registered buttons
    const TButtonDef* btDef = _controller.getButtonDefinition(name);
    if (btDef)
    {
      return _controller[*btDef];
    }

    // return a dummy
    return TButton::dummy;
  }

  void CMapping::load(const std::string& file)
  {
    _buttons.clear();

    auto jData = loadJson(file);
    for (auto& jEntry : jData)
    {
      TMappedButton mappedBt;

      const std::string name = jEntry["name"];
      mappedBt.checkAll = jEntry["check"] == "all";
      for (auto& jButton : jEntry["buttons"])
      {
        const TButtonDef* btDef = _controller.getButtonDefinition(jButton);
        if (!btDef) continue;

        const TButton& bt = _controller[*btDef];
        mappedBt.buttons.push_back(&bt);
      }

      _buttons[name] = std::move(mappedBt);
    }
  }
}
