#include <Source/Headers.h>

#include <Source/WinCoder.h>
#include <Source/Config.h>
#include <Source/UTF8Tools.h>

#include "WinSearch.h"

// ============================================================================
static const wchar* gNavClassName = L"CoderSearchClass";

// ----------------------------------------------------------------------------
WinSearchFactory::WinSearchFactory()
{
  _registerClass(gNavClassName, (HBRUSH)(COLOR_WINDOW));
}

// ----------------------------------------------------------------------------
retcode WinSearchFactory::createWindow(WinCoder* pParent, WinSearch** ppWin)
{
  WinSearch* pWin = new WinSearch(pParent);

  HWND hwnd = CreateWindowW(gNavClassName, L"", WS_CHILD,
    0, 0, 0, 0, pParent->hwnd(), nullptr, ghInstance, static_cast<BaseWinProc*>(pWin));

  if (hwnd == nullptr) {
    return rcFailed;
  }

  *ppWin = pWin;
  return rcSuccess;
}

// ----------------------------------------------------------------------------
const wchar* gComboRange[]
{
  L"Selection",
  L"Current Document",
  L"Whole Project",
};

// ----------------------------------------------------------------------------
void fillComboBox(HWND hwndList, const wchar* Names[], size_t cNames)
{
  SendMessageW(hwndList, CB_RESETCONTENT, 0, 0);

  for (size_t i = 0; i < cNames; i++)
  {
    LRESULT idx = SendMessageW(hwndList, CB_ADDSTRING, 0, (LPARAM)Names[i]);
    if (i == CB_ERR || i == CB_ERRSPACE) {
      return;
    }

    SendMessageW(hwndList, CB_SETITEMDATA, idx, (LPARAM)i);
  }

  SendMessageW(hwndList, CB_SETCURSEL, 1, 0);
}

// ----------------------------------------------------------------------------
LRESULT CALLBACK SubEditProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  if (msg == WM_KEYDOWN) {
    if (wParam == VK_RETURN) {
      return SendMessage(GetParent(hwnd), WM_USER, (WPARAM)hwnd, 0);
    }
    if (wParam == VK_ESCAPE) {
      return SendMessage(GetParent(hwnd), WM_USER+1, (WPARAM)hwnd, 0);
    }
  }

  WinSearch* pWin = (WinSearch*)GetWindowLongW(GetParent(hwnd), GWL_USERDATA);
  return CallWindowProc(pWin->EditProc, hwnd, msg, wParam, lParam);
}

// ----------------------------------------------------------------------------
WinSearch::WinSearch(WinCoder* pParent)
: _pParent(pParent)
{
  memset(_CheckBox, false, sizeof(_CheckBox));
}

// ----------------------------------------------------------------------------
LRESULT WinSearch::proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch (msg)
  {
  case WM_CREATE:
    return onCreate();

  case WM_SIZE:
    return onSize(LOWORD(lParam), HIWORD(lParam), wParam);
        
  case WM_COMMAND:
    return onCommand(LOWORD(wParam), HIWORD(wParam), (HWND)lParam);

  case WM_NOTIFY:
    return onNotify(wParam, (NMHDR*)lParam);

  case WM_USER:
  case WM_USER+1:
    return onCommand(2, msg-WM_USER+1, (HWND)wParam);
  }

  return DefWindowProc(hwnd, msg, wParam, lParam);
}

extern HINSTANCE ghInstance;

// ----------------------------------------------------------------------------
LRESULT WinSearch::onCreate()
{
  // create the fonts for rendering
  LOGFONT fnt = { 0 };
  fnt.lfCharSet = ANSI_CHARSET;
  fnt.lfOutPrecision = OUT_DEFAULT_PRECIS;
  fnt.lfClipPrecision = CLIP_DEFAULT_PRECIS;
  fnt.lfQuality = DEFAULT_QUALITY;
  fnt.lfPitchAndFamily = VARIABLE_PITCH | FF_SWISS;
  strcpy(fnt.lfFaceName, "Tahoma");

  HDC hdc = GetDC(_hWnd);
  fnt.lfHeight = -MulDiv(8, GetDeviceCaps(hdc, LOGPIXELSY), 72);
  fnt.lfWeight = 200;

  _hFont = CreateFontIndirect(&fnt);

  static const int EditStyles[3] = {
    WS_VISIBLE | WS_CHILD | WS_BORDER | ES_LEFT,
    WS_VISIBLE | WS_CHILD | WS_BORDER | ES_LEFT,
    WS_VISIBLE | WS_CHILD | WS_BORDER | ES_RIGHT | ES_NUMBER,
  };

  for (int i = 0; i < 3; i++)
  {
    _hwndSearch[i] = CreateWindowEx(0, "EDIT", "", EditStyles[i],
      0, 0, 0, 0,
      _hWnd,
      (HMENU)(i+2), ghInstance, nullptr);

    EditProc = (WNDPROC)SetWindowLongPtr(_hwndSearch[i], GWLP_WNDPROC, (LONG_PTR)SubEditProc);

    SendMessage(_hwndSearch[i], WM_SETFONT, (WPARAM)_hFont, 0);
  }

  _hwndRange = CreateWindowEx(0L, "ComboBox", nullptr,
    WS_VISIBLE | WS_CHILD | WS_TABSTOP | CBS_DROPDOWNLIST | WS_VSCROLL,
    0, 0, 0, 0,
    _hWnd, (HMENU)6, ghInstance, 0);
  SendMessage(_hwndRange, WM_SETFONT, (WPARAM)_hFont, 0);
  fillComboBox(_hwndRange, gComboRange, countof(gComboRange));

  static const char* CheckBoxTitle[] = {
    "Whole Word",
    "Match Case"
  };

  for (int i = 0; i < 2; i++)
  {
    _hwndCheckBox[i] = CreateWindowEx(0L, "BUTTON", CheckBoxTitle[i],
      WS_VISIBLE | WS_CHILD | BS_CHECKBOX,
      0, 0, 0, 0,
      _hWnd, (HMENU)i, ghInstance, 0);
    SendMessage(_hwndCheckBox[i], WM_SETFONT, (WPARAM)_hFont, 0);
    SendMessage(_hwndCheckBox[i], BM_SETCHECK, BST_UNCHECKED, 0);
  }

  static const int StaticSylesEx[4] = {
    0, 0, WS_EX_STATICEDGE, 0,
  };

  static const int StaticSyles[4] = {
    WS_VISIBLE | WS_CHILD | SS_RIGHT,
    WS_VISIBLE | WS_CHILD | SS_LEFT,
    WS_VISIBLE | WS_CHILD | SS_RIGHT,
    WS_VISIBLE | WS_CHILD | SS_LEFT,
  };

  for (int i = 0; i < 4; i++)
  {
    _hwndStatic[i] = CreateWindowEx(
      StaticSylesEx[i], "STATIC", "",
      StaticSyles[i],
      0, 0, 0, 0,
      _hWnd, (HMENU)i+10, ghInstance, 0);
    SendMessage(_hwndStatic[i], WM_SETFONT, (WPARAM)_hFont, 0);
  }
  return 0;
}

// --------------------------------------------------------------------------
LRESULT WinSearch::onSize(int cx, int cy, uint flags)
{
  RECT rc;
  GetClientRect(_hWnd, &rc);

  int EditHeight = 20;
  int EditWidth  = 150;

  int RangeWidth = 100;
  int ButtonWidth = 100;
  int ButtonHeight = 16;

  SetWindowPos(_hwndSearch[0], nullptr,
    rc.left + 5,
    rc.top  + 4,
    EditWidth, EditHeight,
    SWP_NOZORDER);

  SetWindowPos(_hwndSearch[1], nullptr,
    rc.left + 5,
    rc.top  + 4 + EditHeight + 2,
    EditWidth, EditHeight,
    SWP_NOZORDER);

  SetWindowPos(_hwndRange, nullptr,
    rc.left + 5 + EditWidth + 10,
    rc.top  + 4,
    RangeWidth, EditHeight,
    SWP_NOZORDER);

  SetWindowPos(_hwndStatic[0], nullptr,
    rc.left + 5 + EditWidth + 10,
    rc.top  + 4 + EditHeight + 4,
    RangeWidth, EditHeight,
    SWP_NOZORDER);

  SetWindowPos(_hwndCheckBox[0], nullptr,
    rc.left + 5 + EditWidth + 10 + RangeWidth + 10,
    rc.top  + 5,
    ButtonWidth, ButtonHeight,
    SWP_NOZORDER);

  SetWindowPos(_hwndCheckBox[1], nullptr,
    rc.left + 5 + EditWidth + 10 + RangeWidth + 10,
    rc.top  + 5 + ButtonHeight,
    ButtonWidth, ButtonHeight,
    SWP_NOZORDER);

  SetWindowPos(_hwndSearch[2], nullptr,
    rc.right - 5 - 100 - 10 - 40,
    rc.top  + 5,
    40, EditHeight,
    SWP_NOZORDER);

  SetWindowPos(_hwndStatic[2], nullptr,
    rc.right - 5 - 100 - 10 - 40,
    rc.top  + 5 + EditHeight + 3,
    40, EditHeight-1,
    SWP_NOZORDER);

  SetWindowPos(_hwndStatic[1], nullptr,
    rc.right - 5 - 100,
    rc.top  + 5 + 2,
    100, EditHeight-1,
    SWP_NOZORDER);

  SetWindowPos(_hwndStatic[3], nullptr,
    rc.right - 5 - 100,
    rc.top  + 5 + EditHeight + 3 + 1,
    100, EditHeight-1,
    SWP_NOZORDER);

  return 0;
}

// --------------------------------------------------------------------------
LRESULT WinSearch::onCommand(int Id, int Code, HWND hwndControl)
{
  if (Code == 1)
  {
    if (hwndControl == _hwndSearch[0])
    {
      SetFocus(_pParent->hwnd());
      _pParent->goToNextResult(false);
      return 0;
    }

    if (hwndControl == _hwndSearch[2])
    {
      SetFocus(_pParent->hwnd());

      char Text[1000];
      GetWindowText(_hwndSearch[2], Text, 1000);

      uint Line = 0;
      UTF8ToInt(Text, 1000, &Line);

      _pParent->goToLine(Line);
      return 0;
    }
  }

  if (Code == 2)
  {
    if  (hwndControl == _hwndSearch[0]
      || hwndControl == _hwndSearch[1]
      || hwndControl == _hwndSearch[2])
    {
      SetFocus(_pParent->hwnd());
      return 0;
    }
  }

  switch (Id)
  {
  case 2:
    if (Code == EN_CHANGE) {
      return _updateSearch();
    }
    break;

  case 0: case 1:
    if (Code == BN_CLICKED)
    {
      _CheckBox[Id] = !_CheckBox[Id];
      SendMessage(_hwndCheckBox[Id], BM_SETCHECK, _CheckBox[Id]?BST_CHECKED:BST_UNCHECKED, 0);

      return _updateSearch();
    }
    break;
  }
  return 0;
}

// --------------------------------------------------------------------------
LRESULT WinSearch::onNotify(int, NMHDR *)
{
  return 0;
}

// ----------------------------------------------------------------------------
LRESULT WinSearch::_updateSearch()
{
  char Text[1000];
  GetWindowText(_hwndSearch[0], Text, 1000);
  _pParent->hilightText(Text, _CheckBox[0], _CheckBox[1]);
  return 0;
}

// ----------------------------------------------------------------------------
void WinSearch::giveFocus(FocusTarget Mode)
{
  SetFocus(_hwndSearch[Mode]);
  SendMessage(_hwndSearch[Mode], EM_SETSEL, 0, -1);
}

// ----------------------------------------------------------------------------
void WinSearch::updateResultCount(size_t cResults)
{
  std::stringstream str;
  str << cResults << " Results";
  _strResults = str.str();

  SetWindowText(_hwndStatic[0], _strResults.c_str());
}

// ----------------------------------------------------------------------------
void WinSearch::updateCursorPos(const TextPos& pos, size_t cLines, size_t cLine)
{
  std::stringstream str0;
  str0 << pos.line;
  std::string Line = str0.str();

  SetWindowText(_hwndSearch[2], Line.c_str());

  std::stringstream str;
  str << pos.column << " ";
  _strCursor = str.str();

  std::stringstream str1;
  str1 << "/ " << cLines << " Lines";
  _strLines = str1.str();

  std::stringstream str2;
  str2 << "/ " << cLine << " Columns";
  _strLineLength = str2.str();

  SetWindowText(_hwndStatic[1], _strLines.c_str());
  SetWindowText(_hwndStatic[2], _strCursor.c_str());
  SetWindowText(_hwndStatic[3], _strLineLength.c_str());
}
