#pragma once

#include "mcv_platform.h"
#include "input/button.h"
#include "input/enums.h"
#include "windows/app.h"
#include "modules/game/module_game_controller.h"
#include "engine.h"

namespace Input
{
  struct TMouseData
  {
    bool buttons[BT_MOUSE_COUNT];
    VEC2 position;
    VEC2 delta;
    float wheelDelta = 0.f;
  };

  class CMouse
  {
  public:
    void update(const TMouseData& data, float delta)
    {
      PROFILE_FUNCTION("CMouse");
	 
      _prevPosition = _currPosition;
      _currPosition = data.position;
      _deltaPosition = data.delta;
      for (int i = 0; i < BT_MOUSE_COUNT; ++i)
      {
        float newValue = data.buttons[i] ? 1.f : 0.f;
        _buttons[i].update(newValue, delta);
      }
	  _wheelDelta = data.wheelDelta;
	  /*if(GameController.getInvisibleBlock()){
		  if (CApplication::get().has_focus) {
			  VEC2 centerInScreen = (VEC2(float(CApplication::get().width_app), CApplication::get().height_app) * 0.5);
			  POINT pt{ LONG(centerInScreen.x), LONG(centerInScreen.y) };
			  ClientToScreen(CApplication::get().getHandle(), &pt);
			  SetCursorPos(pt.x,pt.y);
			  _currPosition = centerInScreen;
			  ShowCursor(false);
		  }
	  }
	  else {
		  if (CApplication::get().has_focus) {
			  ShowCursor(true);
		  }
	  }*/
    }

    VEC2 getDelta() const
    {
      return _deltaPosition;
    }
	void setDelta(float deltaX, float deltaY)
	{
		_deltaPosition = VEC2(deltaX, deltaY);
	}

	void setLockMouse(bool locked)
	{
		if (!locked) {
			while (ShowCursor(true) <= 1);
		}
		else {
			while (ShowCursor(false) >= 0);
		}

		_lockCursor = locked;
	}

	void setOnlyLockMouse(bool locked) {
		_lockCursor = locked;
	}



    TButton _buttons[BT_MOUSE_COUNT];
    VEC2 _currPosition;
    VEC2 _prevPosition;
    VEC2 _deltaPosition;
    float _wheelDelta = 0.f;
	bool _lockCursor;
  };
}
