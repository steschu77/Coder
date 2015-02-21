/*!
** \file    /Source/WinCoder.h
** \brief   The WinCoder class defines the main application window procedure.
**
**          This file is a Windows specific implementation.
******************************************************************************/
#pragma once

#include "SearchEngine.h"
#include "WinFactory.h"

// ============================================================================
class WinSearch;
class WinRaster;
class TextDocument;

// ============================================================================
class WinCoder : public BaseWinProc
// ----------------------------------------------------------------------------
{
public:
  WinCoder(TextDocument* pDoc);

  static void getWindowClientRect(RECT* prc);
  
  TextDocument* getDocument();
  const TextDocument* getDocument() const;

  const SearchEngine& getSearchEngine() const;

  void goToNextResult(bool Direction);
  void goToLine(size_t line);

  void hilightText(const char* Text, bool WholeWord, bool MatchCase);

  void onDocumentDirty();
  void onCursorDirty();
  void onSearchResultDirty();

  LRESULT onKeyDown(int chr, int RepCount, int Flags);

protected:
  LRESULT proc(HWND hwnd, UINT cmd, WPARAM wParam, LPARAM lParam);
  LRESULT onCreate();
  LRESULT onSize(int, int, uint);
  
  void _updateTitle();

  bool _queryFileName(char *pszFile, size_t cszFile);

  void _focusEditor();

  TextDocument* _pDoc;

  RECT _rcRaster;
  RECT _rcNavPanel;
  RECT _rcSearchPanel;
  
  SearchEngine _Search;

  WinRaster* _pWinRaster;
  WinSearch* _pWinSearch;

  HWND _hwndRaster;
  HWND _hwndSearch;
  HWND _hwndNavPanel;
};

// ============================================================================
class WinCoderFactory : public WinFactory
// ----------------------------------------------------------------------------
{
public:
  WinCoderFactory();

  retcode createWindow(TextDocument* pDoc, WinCoder** ppWin);
};

// ----------------------------------------------------------------------------
inline TextDocument* WinCoder::getDocument()
{
  return _pDoc;
}

// ----------------------------------------------------------------------------
inline const TextDocument* WinCoder::getDocument() const
{
  return _pDoc;
}

// ----------------------------------------------------------------------------
inline const SearchEngine& WinCoder::getSearchEngine() const
{
  return _Search;
}
