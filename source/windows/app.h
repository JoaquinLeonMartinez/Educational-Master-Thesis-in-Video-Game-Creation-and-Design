#pragma once

class CApplication {
  HWND             hWnd;
  CTimer           time_since_last_render;
  bool             has_focus = false;
  bool			   lost_focus = false;
  HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow);

  static CApplication* the_app;
  static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

public:
  HWND getHandle() const { return hWnd; }
  static CApplication& get();

  bool create(HINSTANCE hInstance, int nCmdShow, int w, int h);
  void runMainLoop();
  void generateFrame();
  bool hasFocus() const {
    return has_focus;
  }
  bool lostFocus() const {
	  return lost_focus;
  }
  void setLostFocus(bool val) {
	  lost_focus = val;
  }
  int width_app;
  int height_app; 
  bool cursorIngame = false;
  void showCursorInMenu(bool show);
};




