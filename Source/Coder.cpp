#include <Source/Headers.h>

#include <Source/WinCoder.h>
#include <Source/Document.h>
#include <Source/UTF8Tools.h>

// ============================================================================
int doMessageLoop();

#if 0
// ============================================================================
int main(int argc, char* argv[])
{
  retcode rv;
  ghInstance = GetModuleHandle(nullptr);

  if (argc != 2) {
    return -1;
  }
  
  TextDocument* pDoc = new TextDocument(argv[1]);
  pDoc->load();

  WinFactory _WinFactory(ghInstance);
  WinRaster *_pWinRaster;
  
  // create the window
  if (failed(rv = _WinFactory.createRasterWindow(pDoc, &_pWinRaster)))
    return rv;

  HWND hwnd = _pWinRaster->hwnd();

  ShowWindow(hwnd, SW_SHOW);
  UpdateWindow(hwnd);

  doMessageLoop();  
  return 0;
}
#endif

// ----------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int cmdShow)
{
  ghInstance = hInst;

  int argc = 0;
  LPWSTR *pArgs = CommandLineToArgvW(GetCommandLineW(), &argc);

  if (argc != 2) {
    return -1;
  }

  char Arg[1000] = { 0 };

  char* pArg = Arg;
  size_t cArg = 1000;

  const wchar* pwArg = pArgs[1];
  size_t lArgs = wcslen(pArgs[1]);

  UTF16ToUTF8(&pArg, &cArg, &pwArg, &lArgs);

  TextDocument* pDoc = new TextDocument(Arg);
  pDoc->load();

  WinCoderFactory _WinFactory;
  WinCoder* pWinCoder;

  // create the window
  if (failed(_WinFactory.createWindow(pDoc, &pWinCoder)))
    return -1;

  doMessageLoop();
  return 0;
}

// ----------------------------------------------------------------------------
int onIdle()
{
  return 0;
}

// ----------------------------------------------------------------------------
int doMessageLoop()
{
  // application message loop
  MSG msg;
  msg.wParam = (WPARAM)-1;

  for (;;)
  {
    if (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
    {
      int ret = GetMessage(&msg, NULL, 0, 0);
      if (ret == -1) {
        msg.wParam = (WPARAM)-1;
        break;
      }

      if (msg.message == WM_QUIT) {
        break;
      }

      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
    else switch (onIdle())
    {
    // dont spend time to this thread if no messages are waiting
    case 0:  WaitMessage();       break;
    case -1: PostQuitMessage(-1); break;
    }
  }

  return msg.wParam;
}
