/*!
** \file    /Source/WinSearch.h
** \brief   The WinSearch class defines the search and goto window procedure.
******************************************************************************/
#pragma once

#include "WinFactory.h"
#include "SearchEngine.h"

// ============================================================================
class WinCoder;

// ============================================================================
class WinSearch : public BaseWinProc
// ----------------------------------------------------------------------------
{
public:
  WinSearch(WinCoder* pParent);

  enum FocusTarget
  {
    focusSearch,
    focusReplace,
    focusGoToLine,
  };

  void giveFocus(FocusTarget Mode);
  void updateResultCount(size_t cResults);
  void updateCursorPos(const TextPos& pos, size_t cLines, size_t cLine);

  WNDPROC EditProc;

protected:
  LRESULT proc(HWND hwnd, UINT cmd, WPARAM wParam, LPARAM lParam);
  LRESULT onCreate();
  LRESULT onSize(int, int, uint);
  
  LRESULT onCommand(int, int, HWND);
  LRESULT onNotify(int, NMHDR *);

  WinCoder* _pParent;

  HWND _hwndSearch[3];
  HWND _hwndRange;
  HWND _hwndCheckBox[2];
  HWND _hwndStatic[4];

  HFONT _hFont;

  bool _CheckBox[2];

  LRESULT _updateSearch();

  std::string _strResults;
  std::string _strCursor;
  std::string _strLines;
  std::string _strLineLength;
};

// ============================================================================
class WinSearchFactory : public WinFactory
  // ----------------------------------------------------------------------------
{
public:
  WinSearchFactory();

  retcode createWindow(WinCoder* pParent, WinSearch** ppWin);
};
