#include <Source/Headers.h>
#include "WinFactory.h"

// ============================================================================
HINSTANCE ghInstance = nullptr;

// ----------------------------------------------------------------------------
void WinFactory::_registerClass(const wchar* ClassName, HBRUSH hbrBackground, const char* strIcon, const char* strCursor)
{
  WNDCLASSEXW wcex = { 0 };
  
  wcex.cbSize        = sizeof(WNDCLASSEX); 
  wcex.lpfnWndProc   = (WNDPROC)__procStatic1;
  wcex.hInstance     = ghInstance;
  wcex.hIcon         = strIcon != nullptr ? LoadIcon(ghInstance, strIcon) : nullptr;
  wcex.hCursor       = LoadCursor(nullptr, strCursor);
  wcex.hbrBackground = hbrBackground;
  wcex.lpszClassName = ClassName;
  RegisterClassExW(&wcex);
}

// ----------------------------------------------------------------------------
LRESULT CALLBACK WinFactory::__procStatic1(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  if (msg == WM_CREATE)
  {
    CREATESTRUCT *cs = (CREATESTRUCT*)lParam;
    BaseWinProc* pHandler = (cs != nullptr) ? static_cast<BaseWinProc*>(cs->lpCreateParams) : nullptr;
    
    if (pHandler == nullptr) {
      return 0;
    }
    
    pHandler->_hWnd = hwnd;
    SetWindowLongPtr(hwnd, GWL_WNDPROC, reinterpret_cast<LONG_PTR>(__procStaticN));
    SetWindowLongPtr(hwnd, GWL_USERDATA, reinterpret_cast<LONG_PTR>(pHandler));
    return pHandler->proc(hwnd, msg, wParam, lParam);
  }
  
  return DefWindowProc(hwnd, msg, wParam, lParam);
}

// ----------------------------------------------------------------------------
LRESULT CALLBACK WinFactory::__procStaticN(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  BaseWinProc* pWin = reinterpret_cast<BaseWinProc*>(GetWindowLongPtr(hwnd, GWL_USERDATA));
  return pWin->proc(hwnd, msg, wParam, lParam);
}
