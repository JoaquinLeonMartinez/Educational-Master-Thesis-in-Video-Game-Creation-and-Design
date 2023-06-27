#pragma once

#include "input/button.h"

namespace Input
{
  class CModuleInput;

  struct TMappedButton
  {
    bool checkAll = false;
    std::vector<const TButton*> buttons;
    TButton result;
  };

  class CMapping
  {
  public:
    CMapping(CModuleInput& controller);
    void update(float delta);
    void load(const std::string& file);

    const TButton& getMappedButton(const std::string& name) const;

    std::map<std::string, TMappedButton> _buttons;
    CModuleInput& _controller;
  };
}
