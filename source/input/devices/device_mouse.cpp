#pragma once

#include "mcv_platform.h"
#include "input/devices/device_mouse.h"
#include "input/interfaces/mouse.h"
#include "input/enums.h"
#include <Windowsx.h>         // GET_Y_LPARAM
#include "windows/app.h"

void CDeviceMouse::update(Input::TMouseData& data)
{
  data.position.x = static_cast<float>(_posX);
  data.position.y = static_cast<float>(_posY);
  for (int i = 0; i < Input::BT_MOUSE_COUNT; ++i)
  {
    data.buttons[i] = _buttons[i];
  }
  data.wheelDelta = static_cast<float>(_wheelDeltaSteps);
  data.delta.x = static_cast<float>(_deltaX);
  data.delta.y = static_cast<float>(_deltaY);
  _wheelDeltaSteps = 0;
  _deltaX = _deltaY = 0;
}

CDeviceMouse::CDeviceMouse(const std::string& name)
  : Input::IDevice(name)
{
  startRawInputData();
}

void CDeviceMouse::startRawInputData() {
#ifndef HID_USAGE_PAGE_GENERIC
#define HID_USAGE_PAGE_GENERIC         ((USHORT) 0x01)
#endif
#ifndef HID_USAGE_GENERIC_MOUSE
#define HID_USAGE_GENERIC_MOUSE        ((USHORT) 0x02)
#endif

  RAWINPUTDEVICE Rid[1];
  Rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
  Rid[0].usUsage = HID_USAGE_GENERIC_MOUSE;
  Rid[0].dwFlags = RIDEV_INPUTSINK;
  Rid[0].hwndTarget = CApplication::get().getHandle();
  RegisterRawInputDevices(Rid, 1, sizeof(Rid[0]));
}

int CDeviceMouse::processWindowMsg(UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message)
  {
  case WM_INPUT: {
    UINT dwSize = 64;
    static BYTE lpb[64];

    int rc = ::GetRawInputData((HRAWINPUT)lParam, RID_INPUT,
      lpb, &dwSize, sizeof(RAWINPUTHEADER));
    if (rc == -1)
      return 0;

    RAWINPUT* raw = (RAWINPUT*)lpb;
    if (raw->header.dwType == RIM_TYPEMOUSE)
    {
      // Save mouse relative movement!
      _deltaX = raw->data.mouse.lLastX;
      _deltaY = raw->data.mouse.lLastY;
      //dbg("Mouse delta is %d,%d\n", _deltaX, _deltaY);
      return 1;
    }
    break;
  }
  case WM_MOUSEMOVE:
  {
    _posX = GET_X_LPARAM(lParam);
    _posY = GET_Y_LPARAM(lParam);
    break;
  }
  case WM_MOUSEWHEEL:
  {
    _wheelDeltaSteps = GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
    break;
  }
  case WM_MBUTTONDOWN:
  {
    _buttons[Input::BT_MOUSE_MIDDLE] = true;
    break;
  }
  case WM_MBUTTONUP:
  {
    _buttons[Input::BT_MOUSE_MIDDLE] = false;
    break;
  }
  case WM_LBUTTONDOWN:
  {
    _buttons[Input::BT_MOUSE_LEFT] = true;
    break;
  }
  case WM_LBUTTONUP:
  {
    _buttons[Input::BT_MOUSE_LEFT] = false;
    break;
  }
  case WM_RBUTTONDOWN:
  {
    _buttons[Input::BT_MOUSE_RIGHT] = true;
    break;
  }
  case WM_RBUTTONUP:
  {
    _buttons[Input::BT_MOUSE_RIGHT] = false;
    break;
  }
  default:;
  }
  return 0;
}
