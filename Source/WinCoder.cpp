#include <Source/Headers.h>
#include "WinCoder.h"

#include "WinRaster.h"
#include "WinSearch.h"
#include "Document.h"
#include "FileSystem.h"

#include "Config.h"

// ============================================================================
static const wchar* gCoderClassName = L"CoderClass";

// ----------------------------------------------------------------------------
WinCoderFactory::WinCoderFactory()
{
  _registerClass(gCoderClassName, nullptr, "\"APPICON\"");
}

// ----------------------------------------------------------------------------
retcode WinCoderFactory::createWindow(TextDocument* pDoc, WinCoder** ppWin)
{
  RECT rec = { 0 };
  WinCoder::getWindowClientRect(&rec);

  AdjustWindowRectEx(&rec, WS_OVERLAPPEDWINDOW, FALSE, 0);

  WinCoder* pWin = new WinCoder(pDoc);

  HWND hwnd = CreateWindowW(gCoderClassName, L"", WS_OVERLAPPEDWINDOW,
    CW_USEDEFAULT, 0, rec.right-rec.left, rec.bottom-rec.top,
    nullptr, nullptr, ghInstance, static_cast<BaseWinProc*>(pWin));

  if (hwnd == nullptr) {
    return rcFailed;
  }

  ShowWindow(hwnd, SW_SHOW);
  UpdateWindow(hwnd);

  *ppWin = pWin;
  return rcSuccess;
}

// ----------------------------------------------------------------------------
int gSearchHeight = 50;
int gNavPanelWidth = 0;

// ----------------------------------------------------------------------------
void WinCoder::getWindowClientRect(RECT* prc)
{
  WinRaster::getWindowClientRect(prc);

  prc->right  += gNavPanelWidth;
  prc->bottom += gSearchHeight;
}

// ----------------------------------------------------------------------------
WinCoder::WinCoder(TextDocument* pDoc)
: _pDoc(pDoc)
, _pWinRaster(nullptr)
, _pWinSearch(nullptr)
, _hwndRaster(nullptr)
, _hwndSearch(nullptr)
, _hwndNavPanel(nullptr)
{
}

// ----------------------------------------------------------------------------
LRESULT WinCoder::proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch (msg)
  {
  case WM_CREATE:
    return onCreate();

  case WM_DESTROY:
    PostQuitMessage(0);
    break;
    
  case WM_SETFOCUS:
    SetFocus(_hwndRaster);
    break;

  case WM_SIZE:
    return onSize(LOWORD(lParam), HIWORD(lParam), wParam);
        
  case WM_KEYDOWN:
    return onKeyDown((int)wParam, lParam & 0x0f, lParam >> 4);
  }

  return DefWindowProc(hwnd, msg, wParam, lParam);
}

// ----------------------------------------------------------------------------
LRESULT WinCoder::onCreate()
{
  retcode rv;

  WinRasterFactory WinRasterFab;
  if (failed(rv = WinRasterFab.createWindow(this, &_pWinRaster))) {
    return rv;
  }

  WinSearchFactory WinSearchFab;
  if (failed(rv = WinSearchFab.createWindow(this, &_pWinSearch))) {
    return rv;
  }
  
  _hwndRaster = _pWinRaster->hwnd();
  _hwndSearch = _pWinSearch->hwnd();

  ShowWindow(_hwndRaster, SW_SHOW);
  UpdateWindow(_hwndRaster);

  ShowWindow(_hwndSearch, SW_SHOW);
  UpdateWindow(_hwndSearch);

  onCursorDirty();
  _updateTitle();

  SetFocus(_hwndRaster);
  return 0;
}

// --------------------------------------------------------------------------
LRESULT WinCoder::onSize(int cx, int cy, uint flags)
{
  SetRect(&_rcRaster, 0, 0, cx, cy - gSearchHeight);
  SetRect(&_rcSearchPanel,  0, cy - gSearchHeight, cx, cy);

  SetWindowPos(_hwndRaster, nullptr,
    0, 0, cx, cy - gSearchHeight,
    SWP_NOZORDER);
    
  SetWindowPos(_hwndSearch, nullptr,
    0, cy - gSearchHeight,
    cx, gSearchHeight,
    SWP_NOZORDER);

  return 0;
}

// ----------------------------------------------------------------------------
LRESULT WinCoder::onKeyDown(int chr, int RepCount, int Flags)
{
  bool ControlKey = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
  bool ShiftKey   = (GetKeyState(VK_SHIFT) & 0x8000) != 0;

  switch (chr)
  {
	case VK_ESCAPE:
    if (_Search.getResultCount() > 0)
    {
      _Search.invalidate();
      _pWinSearch->updateResultCount(_Search.getResultCount());
      _pWinRaster->onSearchResultDirty();
    }
    else
    {
      PostQuitMessage(0);
    }
    break;
    
  case VK_F3:
    goToNextResult(ShiftKey);
    onCursorDirty();
    break;

  case 'F':
    if (ControlKey) {
      _pWinSearch->giveFocus(WinSearch::focusSearch);
      onCursorDirty();
    }
    break;

  case 'G':
    if (ControlKey) {
      _pWinSearch->giveFocus(WinSearch::focusGoToLine);
    }
    break;

  case 'O':
    if (ControlKey) {
      char File[MaxFNameLength];
      if (_queryFileName(File, MaxFNameLength)) {
      }
    }
    break;

  case 'R':
    if (ControlKey) {
      _pWinSearch->giveFocus(WinSearch::focusReplace);
      onCursorDirty();
    }
    break;

  case 'S':
    if (ControlKey) {
      _pDoc->save();
      _updateTitle();
    }
    break;

  case 'L':
    if (ControlKey) {
      _pDoc->deleteLine();
      onDocumentDirty();
    }
  }
  return 0;
}

// ----------------------------------------------------------------------------
void WinCoder::onDocumentDirty()
{
  _Search.updateDocument(_pDoc->getDoc());
  _pWinSearch->updateResultCount(_Search.getResultCount());

  _updateTitle();
  
  _pWinRaster->onDocumentDirty();
}

// ----------------------------------------------------------------------------
void WinCoder::onCursorDirty()
{
  TextPos pos = _pDoc->getCursor();
  const TextDoc* pDoc = _pDoc->getDoc();

  _pWinSearch->updateCursorPos(pos, pDoc->getLineCount(), pDoc->getLineLength(pos.line));

  _pWinRaster->onCursorDirty();
}

// ----------------------------------------------------------------------------
void WinCoder::onSearchResultDirty()
{
  _pWinRaster->onSearchResultDirty();
}

// ----------------------------------------------------------------------------
void WinCoder::_updateTitle()
{
  // Update Title Bar
  char FileName[MaxFNameLength];
  char Text[1000];
  splitFilePath(_pDoc->getPath().c_str(), nullptr, 0, FileName, MaxFNameLength);
  sprintf(Text, "%s%s", _pDoc->isDirty() ? "* " : "", FileName);

  SetWindowText(_hWnd, Text);
}

// ----------------------------------------------------------------------------
void WinCoder::goToLine(size_t line)
{
  const TextDoc* pDoc = _pDoc->getDoc();

  size_t cLines = pDoc->getLineCount();
  if (line >= cLines) {
    return;
  }

  size_t column = pDoc->getLineLength(line);
  _pDoc->setCursor(TextPos(line, column));

  onCursorDirty();
  _focusEditor();
}

// ----------------------------------------------------------------------------
void WinCoder::goToNextResult(bool Direction)
{
  TextPos pos = _pDoc->getCursor();

  if (_Search.getNextResult(&pos, Direction)) {
    _pDoc->setCursor(pos);
  }

  onCursorDirty();

  _focusEditor();
}

// ----------------------------------------------------------------------------
void WinCoder::hilightText(const char* Text, bool WholeWord, bool MatchCase)
{
  if (*Text == 0) {
    _Search.invalidate();
  } else {
    _Search.indexDocument(_pDoc->getDoc(), Text, WholeWord, MatchCase);
  }
  _pWinSearch->updateResultCount(_Search.getResultCount());
  _pWinRaster->onSearchResultDirty();
}

// ----------------------------------------------------------------------------
bool WinCoder::_queryFileName(char *pszFile, size_t cszFile)
{
  OPENFILENAMEA ofn = { 0 };

  char szFileTitle[128];

  const char* Title = "Open C++ File";
  const char* Filter = "All Files (*.*)\0*.*\0C++ Files (*.cpp;*.h)\0*.cpp\0\0";
  const char* lpszext = "cpp";

  *pszFile = 0;

  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner = _hWnd;
  ofn.hInstance = ghInstance;
  ofn.lpstrFilter = Filter;
  ofn.nFilterIndex = 0;
  ofn.lpstrFile = pszFile;
  ofn.nMaxFile = cszFile;
  ofn.lpstrFileTitle = szFileTitle;
  ofn.nMaxFileTitle = sizeof(szFileTitle);
  ofn.lpstrTitle = Title;
  ofn.lpstrDefExt = lpszext;

  if (!GetOpenFileNameA(&ofn))
    return false;

  return true;
}

// ----------------------------------------------------------------------------
void WinCoder::_focusEditor()
{
  SetFocus(_hwndRaster);
}
