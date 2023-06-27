#pragma once

#include "mcv_platform.h"
#include "engine.h"
#include "ui/module_ui.h"
#include "ui/widgets/ui_button.h"
#include "ui/controllers/ui_menu_controller.h"
#include "engine.h"
#include "input/input.h"
#include "utils/utils.h"

namespace UI
{

  void CMenuController::update(float dt)
  {
	//NUEVO+----------------------
	VEC2 mousePos = getMouseInWindow();
	bool mouseActive = mousePos != lastPosMouse;
	if (mouseActive) {
		lastPosMouse = mousePos;
		int touchButton = getPosInButton(mousePos);
		if (touchButton != -1) {
			setCurrentOption(touchButton);
		}
	}
	//-------------------------------


    if (EngineInput["menu_next"].justPressed())
    {
      setCurrentOption(++_currentOption);
    }
    if (EngineInput["menu_prev"].justPressed())
    {
      setCurrentOption(--_currentOption);
    }
    if (EngineInput["menu_confirm"].justPressed())
    {
      _options[_currentOption].callback();
    }

	//nuevo
	if (EngineInput["btMouseClickLeft"].justPressed() && getPosInButton(mousePos) != -1 ) {//nuevo----
		_options[_currentOption].callback();
	} //---------
  }

  void CMenuController::registerOption(CButton* button, Callback callback)
  {
    _options.emplace_back(TOption{button, callback});
  }
  void CMenuController::registerOption(std::string name, Callback callback)
  {

	  CWidget* wdgt = Engine.getUI().getWidgetByAlias(name);
	  CButton* bt = dynamic_cast<CButton*>(wdgt);
	  if (bt)
	  {
		  _options.emplace_back(TOption({ bt, callback }));
	  }
  }


  void CMenuController::setCurrentOption(int idx)
  {
    _currentOption = clamp(idx, 0, static_cast<int>(_options.size()) - 1);
    
    for (auto option : _options)
    {
      option.button->setCurrentState("enabled");
    }

    if (_currentOption >= 0 && _currentOption < _options.size())
    {
      _options.at(_currentOption).button->setCurrentState("selected");
    }
  }
  //nuevo------------------
  int CMenuController::getPosInButton(VEC2 mouse_pos)
  {
	  bool found = false;
	  int nbutton = -1;
	  int i = 0;
	  for (int i = 0; !found && i < _options.size(); i++) {
		  VEC2 pos = _options[i].button->getParams()->position;
		 // VEC2 size = _options[i].button->getImageParams()->size;
		  VEC2 size = _options[i].button->getCurrentState()->imageParams.size;
		  _options[i].button;

		  VEC3 bscale = VEC3::Zero;
		  QUAT brot = QUAT();
		  VEC3 bpos = VEC3::Zero;

		  _options[i].button->getAbsolute().Decompose(bscale, brot, bpos);

		  if (pointInRectangle(mouse_pos, size, VEC2(bpos.x, bpos.y))) {
			  found = true;
			  nbutton = i;
		  }
	  }
	  return nbutton;
  }
  //---------------------------
}



