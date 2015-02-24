/*!
** \file    /Source/WinRaster.h
** \brief   The WinRaster class defines the editor window procedure.
**
**          This file is a Windows specific implementation.
**
**          WinRaster provides the rendering engine that takes a TextDocument
**          along with cursor and selection information and renders that onto
**          a bitmap (canvas). It takes responsibility for displaying the
**          bitmap whenever the window is dirty.
**
**          WinRaster also captures input and directs that to the TextDocument
**          in order to make the right changes to the document.
******************************************************************************/
#pragma once

#include "Canvas.h"
#include "SearchEngine.h"
#include "WinFactory.h"

// ============================================================================
class WinCoder;
class TextDocument;

// ============================================================================
class WinRaster : public BaseWinProc
// ----------------------------------------------------------------------------
{
public:
  WinRaster(WinCoder* pParent);

  static void getWindowClientRect(RECT* prc);

  void onDocumentDirty();
  void onSelectionDirty();
  void onCursorDirty();
  void onSearchResultDirty();

protected:
  LRESULT proc(HWND hwnd, UINT cmd, WPARAM wParam, LPARAM lParam);
  LRESULT onCreate();
  LRESULT onTimer(int, uint);
  LRESULT onSize(int, int, uint);
  LRESULT onVScroll(int Request, int Pos, HWND);
  LRESULT onHScroll(int Request, int Pos, HWND);
  
  LRESULT onPaint(const PAINTSTRUCT *pPS);
  LRESULT onKeyDown(int chr, int RepCount, int Flags);
  LRESULT onKeyUp(int chr, int RepCount, int Flags);
  LRESULT onChar(char chr);

  LRESULT onMouseMove(int, int, uint);
  LRESULT onLButtonDown(int, int, uint);
  LRESULT onLButtonUp(int, int, uint);
  LRESULT onLButtonDblClk(int, int, uint);
  LRESULT onRButtonDown(int, int, uint);
  LRESULT onRButtonUp(int, int, uint);
  LRESULT onMouseWheel(int, int, uint, uint);

  void _onDocumentDirty();
  void _onCursorDirty();
  void _onSelectionDirty();

  TextPos _getTextPos(int x, int y);

  void _ensureCursorVisibility();
  void _placeCursorInVisibleRange();
  void _updateTitle();

  WinCoder* _pParent;

  struct BitmapInfo256
  {
    BITMAPINFOHEADER bmiHeader;
    RGBQUAD          bmiColors[256];
  };

  BitmapInfo256 _bmi256;

  size_t _Width;
  size_t _Height;

  size_t _Pels;
  size_t _Lines;
  
  uint _TextLines;
      
  gfx::Canvas* _pCanvas[2];
  uint _CursorCanvas;
  
  gfx::FixedSizeFont *_pFont;
  
  size_t _cyLine;
  size_t _cxScroll;
  
  struct ScrollInfo
  {
    ScrollInfo(size_t Unit) : Unit(Unit), Max(0), Page(0), Pos(0) {}
    
    int Unit;
    int Max;
    int Page;
    int Pos;
  };
  
  ScrollInfo _siVert;
  ScrollInfo _siHorz;
  
  void _setupVertScrollBar();
  void _setupHorzScrollBar();
  void _setupScrollBar(int Direction, const ScrollInfo& siInfo);
  void _onScroll(int Request, int Pos, ScrollInfo* pInfo, int Direction);

  struct RenderState
  {
    uint DocumentVersion;
    uint CursorVersion;
    uint SelectionVersion;
    uint SearchResultVersion;

    TextPos Offset;

    TextPos Cursor;
    TextPos SelBegin;
    TextPos SelEnd;
  };

  void _updateMouse(int x, int y, bool Select);
  void _updateState();

  void _updateCursor(const RenderState& State);
  void _updateCanvas(const RenderState& State);
  void _setupCanvas(size_t cx, size_t cy);
  void _scrollContent(size_t Lines, size_t y0, size_t y1);

  RECT _rcFocus;
  RECT _rcEdit;
  
  bool _Idle;
  bool _Active;
  bool _Focus;
  bool _Insert;

  RenderState _RenderedState;
  RenderState _getCurrentState() const;
};

// ============================================================================
class WinRasterFactory : public WinFactory
// ----------------------------------------------------------------------------
{
public:
  WinRasterFactory();

  retcode createWindow(WinCoder* pParent, WinRaster** ppWin);
};
