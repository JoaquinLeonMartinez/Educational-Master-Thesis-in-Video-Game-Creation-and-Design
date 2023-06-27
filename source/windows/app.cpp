#include "mcv_platform.h"
#include "app.h"
#include "engine.h"
#include "utils/directory_watcher.h"
#include "input/module_input.h"
#include "render/module_render.h"


#include <windowsx.h>
#define IDI_ICON1 102

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

CApplication* CApplication::the_app = nullptr;

//--------------------------------------------------------------------------------------
// Called every time the application receives a message
//--------------------------------------------------------------------------------------
LRESULT CALLBACK CApplication::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  PAINTSTRUCT ps;
  HDC hdc;

  if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
    return true;

  // If the OS processes it, do not process anymore
  auto* input = &EngineInput;
  if (input && input->OnOSMsg(hWnd, message, wParam, lParam))
    return 1;

  switch (message)
  {
  case WM_PAINT:
    hdc = BeginPaint(hWnd, &ps);
    EndPaint(hWnd, &ps);
    break;

  case WM_DESTROY:
    PostQuitMessage(0);
    break;

  case CDirectoyWatcher::WM_FILE_CHANGED: 
  {
    const char* filename = (const char*)(lParam);
    assert(filename);
    std::string strfilename(filename);
    dbg("File %s has changed!!\n", filename);
    Resources.onFileChanged(strfilename);
    delete[] filename;
    break; 
  }

  case WM_EXITSIZEMOVE:
    // Only when the resize finishes, notify the update to the ModuleRender, which will update the deferred
    Engine.getRender().onResolutionUpdated();
    // Then, send a notification to all resources with a fake name, so all materials which where referencing
    // textures like the water of the deferred update the srv.
    Resources.onFileChanged("screen_size");
    break;

  case WM_SIZE:
    if (Render.device != NULL && wParam != SIZE_MINIMIZED) {
      Render.resizeBackBuffer((UINT)LOWORD(lParam), (UINT)HIWORD(lParam));
      if (wParam) { //WHEN SCREEN IS MAXIMIZED OR MINIMIZED
        Engine.getRender().onResolutionUpdated();
        Resources.onFileChanged("screen_size");
      }
    }
      

    return 0;

  case WM_SETFOCUS:
    if (CApplication::the_app)
      CApplication::the_app->has_focus = true;
  case WM_MOVE:
	RECT rect;
	GetClientRect(hWnd, &rect);
	CApplication::get().height_app = float(rect.bottom - rect.top);
	CApplication::get().width_app = float(rect.right - rect.left);
	break;

  case WM_KILLFOCUS:
    if (the_app)
      the_app->has_focus = false;
	  the_app->lost_focus = true;
    break;
	
  default:
    return DefWindowProc(hWnd, message, wParam, lParam);
  }

  return 0;
}

CApplication& CApplication::get() {
  assert(the_app);
  return *the_app;
}

bool CApplication::create(HINSTANCE hInstance, int nCmdShow, int w, int h) {
  PROFILE_FUNCTION("App::Crate");

  the_app = this;

  // Register class
  WNDCLASSEX wcex;
  wcex.cbSize = sizeof(WNDCLASSEX);
  wcex.style = CS_CLASSDC; // CS_HREDRAW | CS_VREDRAW;
  wcex.lpfnWndProc = WndProc;
  wcex.cbClsExtra = 0;
  wcex.cbWndExtra = 0;
  wcex.hInstance = hInstance;
  wcex.hIcon = (HICON)LoadImage(hInstance,
    MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON,
    ::GetSystemMetrics(SM_CXICON),
    ::GetSystemMetrics(SM_CYICON), 0);
  wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
  wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  wcex.lpszMenuName = NULL;
  wcex.lpszClassName = "MCVWindowsClass";
  wcex.hIconSm = (HICON)LoadImage(hInstance,
    MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON,
    ::GetSystemMetrics(SM_CXSMICON),
    ::GetSystemMetrics(SM_CYSMICON), 0);
  if (!RegisterClassEx(&wcex))
    return false;

  // Create window
  //g_hInst = hInstance;
  RECT rc = { 0, 0, w, h };
  width_app = w;
  height_app = h;
  AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
  hWnd = CreateWindow("MCVWindowsClass", "Madness: In Stock",
    WS_OVERLAPPEDWINDOW,
    CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance,
    NULL);
  if (!hWnd)
    return false;

  ShowWindow(hWnd, nCmdShow);
  
  w = GetSystemMetrics(SM_CXSCREEN);
  h = GetSystemMetrics(SM_CYSCREEN);
  SetWindowLongPtr(hWnd, GWL_STYLE, WS_VISIBLE | WS_POPUP);
  SetWindowPos(hWnd, HWND_TOP, 0, 0, w, h, SWP_FRAMECHANGED);
  

  json cfg = loadJson("data/config.json");
  cursorIngame = cfg.value("cursorInGame", false);
  return true;

}

void CApplication::runMainLoop() {

  CEngine::get().start();

  // Main message loop
  MSG msg = { 0 };
  while (WM_QUIT != msg.message)
  {
    if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
	else
	{

		if (!cursorIngame){
			if (GameController.getInvisibleBlock()) {
				if (hasFocus()) {
					VEC2 centerInScreen = (VEC2(float(CApplication::get().width_app), CApplication::get().height_app) * 0.5);
					POINT pt{ LONG(centerInScreen.x), LONG(centerInScreen.y) };
					ClientToScreen(CApplication::get().getHandle(), &pt);
					SetCursorPos(pt.x, pt.y);
					Input::CMouse mouse = EngineInput.mouse();
					mouse.setLockMouse(true);

					//ShowCursor(false);
					//_currPosition = centerInScreen;
				}
			}
			else {
				Input::CMouse mouse = EngineInput.mouse();
				mouse.setLockMouse(false);
			}
		}
        generateFrame();
    }
  }

  CEngine::get().stop();
}


void CApplication::showCursorInMenu(bool show) {
	if (hasFocus()) {
		VEC2 centerInScreen = (VEC2(float(CApplication::get().width_app), CApplication::get().height_app) * 0.5);
		POINT pt{ LONG(centerInScreen.x), LONG(centerInScreen.y) };
		ClientToScreen(CApplication::get().getHandle(), &pt);
		SetCursorPos(pt.x, pt.y);
		Input::CMouse mouse = EngineInput.mouse();
		mouse.setLockMouse(show);
	}
}
