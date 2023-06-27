#pragma once

#include "mcv_platform.h"
#include "input/button.h"
#include "input/enums.h"

namespace Input
{
  struct TGamepadData
  {
    float buttons[BT_GAMEPAD_COUNT];
    bool connected = false;
  };

  class CGamepad
  {
  public:
    void update(const TGamepadData& data, float delta)
    {
      PROFILE_FUNCTION("CGamepad");
      _connected = data.connected;
      for (int i = 0; i < BT_GAMEPAD_COUNT; ++i)
      {
        _buttons[i].update(data.buttons[i], delta);
      }

	    //TREAT DEADZONES OF JOYSTICKS
	    VEC2 left = VEC2(_buttons[BT_LANALOG_X].value, _buttons[BT_LANALOG_Y].value);
	    float magnitude = left.Length();
	    if (magnitude < deadzone) {
		    _buttons[BT_LANALOG_X].value = 0.0f;
		    _buttons[BT_LANALOG_Y].value = 0.0f;
	    }
	    else {
		    left.Normalize();
		    left = left * ((magnitude - deadzone) / (1 - deadzone));
		    _buttons[BT_LANALOG_X].value = left.x;
		    _buttons[BT_LANALOG_Y].value = left.y;
	    }

	    VEC2 right = VEC2(_buttons[BT_RANALOG_X].value, _buttons[BT_RANALOG_Y].value);
	    magnitude = right.Length();
	    if (magnitude < deadzone) {
		    _buttons[BT_RANALOG_X].value = 0.0f;
		    _buttons[BT_RANALOG_Y].value = 0.0f;
	    }
	    else {
		    right.Normalize();
		    right = right * ((magnitude - deadzone) / (1 - deadzone));
		    _buttons[BT_RANALOG_X].value = right.x;
		    _buttons[BT_RANALOG_Y].value = right.y;
	    }

	    //TREAT ABSOLUTE AXIS DIRECTIONS
	    double angle = atan2f(left.y, left.x);
	    //RANGE BETWEEN -PI AND PI
	    if (angle > M_PI) { angle -= 2 * M_PI; }
	    else if (angle <= -M_PI) { angle += 2 * M_PI; }
	    if (abs(angle) < absolute_axis_angle || abs(angle) >= M_PI - absolute_axis_angle) { //RIGHT OR LEFT
		    _buttons[BT_LANALOG_Y].value = 0.0f;
	    }
	    else if ((abs(angle) > (M_PI / 2.0f) - absolute_axis_angle) && (abs(angle) < (M_PI / 2.0f) + absolute_axis_angle)) { //UP OR DOWN
		    _buttons[BT_LANALOG_X].value = 0.0f;
	    }


	    angle = atan2f(right.y, right.x);
	    //RANGE BETWEEN -PI AND PI
	    if (angle > M_PI) { angle -= 2 * M_PI; }
	    else if (angle <= -M_PI) { angle += 2 * M_PI; }
	    if (abs(angle) < absolute_axis_angle || abs(angle) >= M_PI - absolute_axis_angle) { //RIGHT OR LEFT
		    _buttons[BT_RANALOG_Y].value = 0.0f;
	    }
	    else if ((abs(angle) > (M_PI/ 2.0f) - absolute_axis_angle) && (abs(angle) < (M_PI / 2.0f) + absolute_axis_angle)) { //UP OR DOWN
		    _buttons[BT_RANALOG_X].value = 0.0f;
	    }
    }

    TButton _buttons[BT_GAMEPAD_COUNT];
    bool _connected = false;
	  float deadzone = 0.10f;
    float absolute_axis_angle = 0.2f; //11 grados
  };
}
