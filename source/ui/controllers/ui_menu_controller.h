#pragma once

#include "mcv_platform.h"
#include "ui/ui_controller.h"

namespace UI
{
  class CButton;

  class CMenuController : public CController
  {
  public:
    using Callback = std::function<void()>;

    void update(float dt) override;

    void registerOption(CButton* button, Callback callback);
	void registerOption(std::string name, Callback callback);
    void setCurrentOption(int idx);
	  int getPosInButton(VEC2 mouse_pos);

  private:
    struct TOption
    {
      CButton* button = nullptr;
      Callback callback = nullptr;
    };
    std::vector<TOption> _options;
    int _currentOption = -1;
	  VEC2 lastPosMouse = VEC2::Zero;
    
  };
}
