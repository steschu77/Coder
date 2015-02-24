#include <Source/Headers.h>
#include <Source/ToolBox.h>
#include <Source/FixedSizeFont.h>
#include <Source/Clipboard.h>
#include <Source/Config.h>

#include <Source/WinSearch.h>
#include <Source/WinCoder.h>
#include <Source/Document.h>

#include "WinRaster.h"

// ============================================================================
static const wchar* gRasterClassName = L"CoderRasterClass";

// ----------------------------------------------------------------------------
WinRasterFactory::WinRasterFactory()
{
  _registerClass(gRasterClassName, nullptr, nullptr, IDC_IBEAM);
}

// ----------------------------------------------------------------------------
retcode WinRasterFactory::createWindow(WinCoder* pParent, WinRaster** ppWin)
{
  RECT rec = { 0 };
  WinRaster::getWindowClientRect(&rec);

  AdjustWindowRectEx(&rec, WS_OVERLAPPEDWINDOW, FALSE, 0);

  WinRaster* pWin = new WinRaster(pParent);

  HWND hwnd = CreateWindowW(gRasterClassName, L"", WS_CHILD,
    CW_USEDEFAULT, 0, rec.right-rec.left, rec.bottom-rec.top,
    pParent->hwnd(), nullptr, ghInstance, static_cast<BaseWinProc*>(pWin));

  if (hwnd == nullptr) {
    return rcFailed;
  }

  ShowWindow(hwnd, SW_SHOW);
  UpdateWindow(hwnd);

  *ppWin = pWin;
  return rcSuccess;
}

// ----------------------------------------------------------------------------
void WinRaster::getWindowClientRect(RECT* prc)
{
  prc->left  = 0;
  prc->right = gConfig.winSizeX * gConfig.fontSizeX + 2;

  prc->top    = 0;
  prc->bottom = gConfig.winSizeY * gConfig.fontSizeY + 2;
}

// ----------------------------------------------------------------------------
WinRaster::WinRaster(WinCoder* pParent)
: _pParent(pParent)
, _Width(8)
, _Height(14)
, _Pels(0)
, _Lines(0)
, _TextLines(0)
, _CursorCanvas(0)
, _pFont(nullptr)
, _cyLine(_Height)
, _siVert(_Height)
, _siHorz(_Width)
, _Idle(true)
, _Active(false)
, _Focus(false)
, _Insert(false)
{
  _pCanvas[0] = nullptr;
  _pCanvas[1] = nullptr;
}

// ----------------------------------------------------------------------------
LRESULT WinRaster::proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  LRESULT res;
  PAINTSTRUCT ps;

  switch (msg)
  {
  case WM_CREATE:
    return onCreate();

  case WM_TIMER:
    return onTimer((WPARAM)wParam, 0);
    
  case WM_SIZE:
    return onSize(LOWORD(lParam), HIWORD(lParam), wParam);
        
  case WM_VSCROLL:
    return onVScroll(LOWORD(wParam), HIWORD(wParam), (HWND)lParam);
    
  case WM_HSCROLL:
    return onHScroll(LOWORD(wParam), HIWORD(wParam), (HWND)lParam);
    
  case WM_PAINT:
    BeginPaint(_hWnd, &ps);
    res = onPaint(&ps);
    EndPaint(_hWnd, &ps);
    return res;

  case WM_KEYDOWN:
    return onKeyDown((int)wParam, lParam & 0x0f, lParam >> 4);
    
  case WM_KEYUP:
    return onKeyUp((int)wParam, lParam & 0x0f, lParam >> 4);
    
  case WM_CHAR:
    return onChar((char)wParam);

  case WM_MOUSEMOVE:
    return onMouseMove(LOWORD(lParam), HIWORD(lParam), wParam);

  case WM_LBUTTONDOWN:
    return onLButtonDown(LOWORD(lParam), HIWORD(lParam), wParam);

  case WM_LBUTTONUP:
    return onLButtonUp(LOWORD(lParam), HIWORD(lParam), wParam);

  case WM_LBUTTONDBLCLK:
    return onLButtonDblClk(LOWORD(lParam), HIWORD(lParam), wParam);

  case WM_RBUTTONDOWN:
    return onRButtonDown(LOWORD(lParam), HIWORD(lParam), wParam);

  case WM_RBUTTONUP:
    return onRButtonUp(LOWORD(lParam), HIWORD(lParam), wParam);

  case WM_MOUSEWHEEL:
    return onMouseWheel(LOWORD(lParam), HIWORD(lParam), HIWORD(wParam), LOWORD(wParam));

  case WM_SETFOCUS:
    _Focus = true;
    InvalidateRect(_hWnd, &_rcFocus, false);
    break;

  case WM_KILLFOCUS:
    _Focus = false;
    InvalidateRect(_hWnd, &_rcFocus, false);
    break;
  }

  return DefWindowProc(hwnd, msg, wParam, lParam);
}

// ----------------------------------------------------------------------------
LRESULT WinRaster::onCreate()
{
  //gfx::loadTrueTypeFont(_Width, _Height, &_pFont);
  static const char* FontFiles[4] = {
    "c:\\Windows\\Fonts\\consola.ttf",
    "c:\\Windows\\Fonts\\consolab.ttf",
    "c:\\Windows\\Fonts\\consolai.ttf",
    "c:\\Windows\\Fonts\\consolaz.ttf",
  };
  
  gfx::loadTrueTypeFont(FontFiles, _Width, _Height, &_pFont);
  
  _setupVertScrollBar();
  _setupHorzScrollBar();

  onCursorDirty();
  
  SetTimer(_hWnd, 0, 500, nullptr);
  return 0;
}

// --------------------------------------------------------------------------
LRESULT WinRaster::onTimer(int, uint)
{
  _updateCursor(_RenderedState);
  return 0;
}

// --------------------------------------------------------------------------
LRESULT WinRaster::onSize(int cx, int cy, uint flags)
{
  _setupVertScrollBar();
  _setupHorzScrollBar();

  _setupCanvas(cx - 2, cy - 2);
  _updateState();
  return 0;
}

// --------------------------------------------------------------------------
LRESULT WinRaster::onVScroll(int Request, int Pos, HWND)
{
  _onScroll(Request, Pos, &_siVert, SB_VERT);
  _updateState();
  return 0;
}

// --------------------------------------------------------------------------
LRESULT WinRaster::onHScroll(int Request, int Pos, HWND)
{
  _onScroll(Request, Pos, &_siHorz, SB_HORZ);
  _updateState();
  return 0;
}

// ----------------------------------------------------------------------------
LRESULT WinRaster::onPaint(const PAINTSTRUCT *pPS)
{
  SetDIBitsToDevice(pPS->hdc, 1, 1, _Pels, _Lines, 0, 0, 0, _Lines,
    _pCanvas[_CursorCanvas]->rgbaPels, (BITMAPINFO*)&_bmi256, DIB_RGB_COLORS);

  SelectObject(pPS->hdc, GetStockObject(_Focus ? BLACK_PEN : WHITE_PEN));
  SelectObject(pPS->hdc, GetStockObject(HOLLOW_BRUSH));

  RECT rc;
  GetClientRect(_hWnd, &rc);

  Rectangle(pPS->hdc, rc.left, rc.top, rc.right, rc.bottom);
  return 0;
}

// ----------------------------------------------------------------------------
LRESULT WinRaster::onKeyDown(int chr, int RepCount, int Flags)
{
  TextDocument* pDoc = _pParent->getDocument();
  
  if (pDoc == nullptr) {
    return 0;
  }
  
  bool ControlKey = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
  bool ShiftKey   = (GetKeyState(VK_SHIFT) & 0x8000) != 0;

  _Idle = false;
  
  switch (chr)
  {    
  case VK_LEFT:
    if (ControlKey) {
      pDoc->movePrevWord(ShiftKey);
    } else {
      pDoc->moveLeft(ShiftKey);
    }
    break;
    
  case VK_RIGHT:
    if (ControlKey) {
      pDoc->moveNextWord(ShiftKey);
    } else {
      pDoc->moveRight(ShiftKey);
    }
    break;

  case VK_UP:
    if (ControlKey) {
      _onScroll(SB_LINEUP, 0, &_siVert, SB_VERT);
      _placeCursorInVisibleRange();
    }
    else {
      pDoc->moveUp(ShiftKey);
    }
    break;

  case VK_DOWN:
    if (ControlKey) {
      _onScroll(SB_LINEDOWN, 0, &_siVert, SB_VERT);
      _placeCursorInVisibleRange();
    }
    else {
      pDoc->moveDown(ShiftKey);
    }
    break;

  case VK_PRIOR:
    pDoc->moveUp(ShiftKey, _siVert.Page);
    break;

  case VK_NEXT:
    pDoc->moveDown(ShiftKey, _siVert.Page);
    break;

  case VK_HOME:
    if (ControlKey) {
      pDoc->moveBeginOfDocument(ShiftKey);
    }
    else {
      pDoc->moveBeginOfLine(ShiftKey);
    }
    break;

  case VK_END:
    if (ControlKey) {
      pDoc->moveEndOfDocument(ShiftKey);
    }
    else {
      pDoc->moveEndOfLine(ShiftKey);
    }
    break;

  case VK_RETURN:
    if (ControlKey) {
      pDoc->insertNewLineBefore();
    } else {
      pDoc->insertNewLine();
    }
    break;
    
  case VK_BACK:
    pDoc->deleteBefore();
    break;

  case VK_DELETE:
    pDoc->deleteAfter();
    break;

  case VK_INSERT:
    _Insert = !_Insert;
    break;

  case 'L':
    if (ControlKey) {
      pDoc->deleteLine();
    }
    break;

  case 'C':
    if (ControlKey) {
      TextDoc selText = pDoc->getSelectedText();
      copyTextToClipboard(_hWnd, selText);
    }
    break;
    
  case 'V':
    if (ControlKey) {
      EditableTextDoc doc;
      pasteTextFromClipboard(_hWnd, &doc);

      pDoc->insertContent(doc);
    }
    break;

  case 'X':
    if (ControlKey) {
      TextDoc selText = pDoc->getSelectedText();
      copyTextToClipboard(_hWnd, selText);

      pDoc->deleteSelectedText();
    }
    break;

  case 'Y':
    if (ControlKey) {
      pDoc->redo();
    }
    break;

  case 'Z':
    if (ControlKey) {
      pDoc->undo();
    }
    break;

  default:
    return _pParent->onKeyDown(chr, RepCount, Flags);
  }

  _onDocumentDirty();
  _onCursorDirty();
  return 0;
}

// ----------------------------------------------------------------------------
LRESULT WinRaster::onKeyUp(int chr, int RepCount, int Flags)
{
  _Idle = true;
  return 0;
}

// ----------------------------------------------------------------------------
LRESULT WinRaster::onChar(char chr)
{
  TextDocument* pDoc = _pParent->getDocument();
  
  if (pDoc == nullptr) {
    return 0;
  }
  
  if (chr == '{') {
    pDoc->insertOpeningBracket();
  }
  else if (chr >= 32 && chr <= 128) {
    pDoc->insert(chr);
  }
  else {
    return 0;
  }

  _onDocumentDirty();
  _onCursorDirty();
  return 0;
}

// --------------------------------------------------------------------------
LRESULT WinRaster::onMouseMove(int x, int y, uint flags)
{
  if ((flags & MK_LBUTTON) != 0) {
    _updateMouse(x, y, true);
  }
  return 0;
}

// --------------------------------------------------------------------------
LRESULT WinRaster::onLButtonDown(int x, int y, uint)
{
  SetFocus(_hWnd);
  _updateMouse(x, y, false);

  return 0;
}

// --------------------------------------------------------------------------
LRESULT WinRaster::onLButtonUp(int, int, uint)
{
  return 0;
}

// --------------------------------------------------------------------------
LRESULT WinRaster::onLButtonDblClk(int, int, uint)
{
  return 0;
}

// --------------------------------------------------------------------------
LRESULT WinRaster::onRButtonDown(int, int, uint)
{
  return 0;
}

// --------------------------------------------------------------------------
LRESULT WinRaster::onRButtonUp(int, int, uint)
{
  return 0;
}

// --------------------------------------------------------------------------
LRESULT WinRaster::onMouseWheel(int, int, uint, uint)
{
  return 0;
}

// ----------------------------------------------------------------------------
void WinRaster::onDocumentDirty()
{
  _setupVertScrollBar();
  _setupHorzScrollBar();
  _updateState();
}

// ----------------------------------------------------------------------------
void WinRaster::_onDocumentDirty()
{
  RenderState State = _getCurrentState();

  if (State.DocumentVersion != _RenderedState.DocumentVersion) {
    _pParent->onDocumentDirty();
  }
}

// ----------------------------------------------------------------------------
void WinRaster::onSelectionDirty()
{
  _updateState();
}

// ----------------------------------------------------------------------------
void WinRaster::onCursorDirty()
{
  _ensureCursorVisibility();
  _updateState();
}

// ----------------------------------------------------------------------------
void WinRaster::_onCursorDirty()
{
  _pParent->onCursorDirty();
}

// ----------------------------------------------------------------------------
void WinRaster::onSearchResultDirty()
{
  _updateState();
}

// --------------------------------------------------------------------------
void WinRaster::_updateMouse(int x, int y, bool Select)
{
  TextDocument* pDoc = _pParent->getDocument();
  if (pDoc == nullptr) {
    return;
  }

  size_t xOfsOld = _RenderedState.Offset.column;
  size_t yOfsOld = _RenderedState.Offset.line;

  int xPos = x / _Width;
  int yPos = y / _Height;

  pDoc->setCursor(TextPos(yPos+yOfsOld, xPos+xOfsOld), Select);
  _onCursorDirty();
}

// ----------------------------------------------------------------------------
void WinRaster::_updateState()
{
  RenderState State = _getCurrentState();

  if (State.Offset != _RenderedState.Offset
    || State.DocumentVersion != _RenderedState.DocumentVersion
    || State.SelectionVersion != _RenderedState.SelectionVersion
    || State.SearchResultVersion != _RenderedState.SearchResultVersion
    || State.Cursor.line != _RenderedState.Cursor.line)
  {
    _updateCanvas(State);
  }
  else {
    _updateCursor(State);
  }

  _RenderedState = State;
}

// ----------------------------------------------------------------------------
void WinRaster::_ensureCursorVisibility()
{
  TextDocument* pDoc = _pParent->getDocument();
  if (pDoc == nullptr) {
    return;
  }

  size_t FirstVisibleLine = _siVert.Pos;
  size_t LastVisibleLine = _siVert.Pos + _TextLines;

  TextPos cp = pDoc->getCursor();

  if (cp.line < FirstVisibleLine) {
    _onScroll(SB_THUMBPOSITION, cp.line, &_siVert, SB_VERT);
  }

  if (cp.line >= LastVisibleLine) {
    _onScroll(SB_THUMBPOSITION, cp.line - _TextLines + 1, &_siVert, SB_VERT);
  }
}

// ----------------------------------------------------------------------------
void WinRaster::_placeCursorInVisibleRange()
{
  TextDocument* pDoc = _pParent->getDocument();
  if (pDoc == nullptr) {
    return;
  }

  size_t FirstVisibleLine = _siVert.Pos;
  size_t LastVisibleLine = _siVert.Pos + _TextLines;

  TextPos cp = pDoc->getCursor();

  if (cp.line < FirstVisibleLine) {
    pDoc->setCursor(TextPos(FirstVisibleLine, 0));
  }

  if (cp.line >= LastVisibleLine) {
    pDoc->setCursor(TextPos(LastVisibleLine - _TextLines + 1, 0));
  }
}

// ----------------------------------------------------------------------------
void renderSyntaxHilighting(std::vector<gfx::TextChar>& tl, const std::string& line);
void renderSearchResults(std::vector<gfx::TextChar>& text, const SearchEngine& Search, size_t line);

// ----------------------------------------------------------------------------
void renderTextSelection(std::vector<gfx::TextChar>& text, size_t line, const TextDocument* pDoc)
{
  if (pDoc->hasSelectedText(line))
  {
    for (size_t idx = 0; idx < text.size(); idx++)
    {
      if (pDoc->isSelected(line, idx))
      {
        text[idx].background = true;
        text[idx].colBkg = gConfig.Colors.bkgSelection;
      }
    }
  }
}

// ----------------------------------------------------------------------------
void WinRaster::_updateCanvas(const RenderState& State)
{
  const SearchEngine& Search = _pParent->getSearchEngine();
  TextDocument* pDocument = _pParent->getDocument();
  
  gfx::Canvas* pCanvas = _pCanvas[1-_CursorCanvas];

  if (pDocument == nullptr) {
    pCanvas->clear(gConfig.Colors.bkgLineNumber);
    return;
  }

  size_t xOfs = State.Offset.column;
  size_t yOfs = State.Offset.line;

  size_t xOfsOld = _RenderedState.Offset.column;
  size_t yOfsOld = _RenderedState.Offset.line;

  bool UpdateWholeContent = (xOfs != xOfsOld)
    || _RenderedState.DocumentVersion != State.DocumentVersion
    || _RenderedState.SearchResultVersion != State.SearchResultVersion;

  size_t y0 = 0;
  size_t y1 = UpdateWholeContent ? _TextLines : 0;

  if (!UpdateWholeContent)
  {
    size_t Range = min<size_t>(yOfsOld - yOfs, yOfs - yOfsOld, _TextLines);
    size_t Lines = _TextLines - Range;

    if (yOfs < yOfsOld)
    {
      y0 = 0;
      y1 = Range;
      _scrollContent(Lines, 0, Range);
    }

    if (yOfs > yOfsOld)
    {
      y0 = _TextLines - Range;
      y1 = _TextLines;
      _scrollContent(Lines, Range, 0);
    }
  }

  TextPos p0 = std::min(_RenderedState.SelBegin, State.SelBegin);
  TextPos p1 = std::max(_RenderedState.SelBegin, State.SelBegin);
  TextPos p2 = std::min(_RenderedState.SelEnd, State.SelEnd);
  TextPos p3 = std::max(_RenderedState.SelEnd, State.SelEnd);

  pCanvas = _pCanvas[1-_CursorCanvas];

  const TextDoc& doc = pDocument->getDoc();

  size_t cDocLines = doc.getLineCount() - yOfs;
  size_t cLines = std::min(_TextLines, cDocLines);

  for (size_t y = 0; y < cLines; y++)
  {
    size_t yDoc = y + yOfs;

    if ((y >= y0 && y < y1)
      || (yDoc == State.Cursor.line)
      || (yDoc == _RenderedState.Cursor.line)
      || (p0 != p1 && yDoc >= p0.line && yDoc <= p1.line)
      || (p2 != p3 && yDoc >= p2.line && yDoc <= p3.line))
    {
      if (yDoc == State.Cursor.line) {
        pCanvas->clear(0, y*_Height, _Pels, (y+1)*_Height, gConfig.Colors.bkgCursorLine);
      } else {
        pCanvas->clear(0, y*_Height, _Pels, (y+1)*_Height, gConfig.Colors.bkgEditor);
      }

      const std::string strLine = doc.getLineAt(yDoc);
      const size_t cLength = strLine.length();

      if (cLength > xOfs)
      {
        std::vector<gfx::TextChar> tl(cLength + 1);
        renderSyntaxHilighting(tl, strLine);
        renderTextSelection(tl, y+yOfs, pDocument);
        renderSearchResults(tl, Search, y+yOfs);

        pCanvas->drawText(0, _Height*y, &tl[xOfs], *_pFont);
      }
    }
  }

  pCanvas->clear(0, cLines*_Height, _Pels, _TextLines*_Height, gConfig.Colors.bkgEditor);

  return _updateCursor(State);
}

// ----------------------------------------------------------------------------
void WinRaster::_updateCursor(const RenderState& State)
{  
  if (_pCanvas[0] == nullptr || _pCanvas[1] == nullptr) {
    return;
  }

  _pCanvas[1-_CursorCanvas]->blit(_pCanvas[_CursorCanvas]);

  uint t = GetTickCount();
  if (_Focus && (!_Idle || (t % 1000) > 500))
  {
    TextPos cp = State.Cursor;
    size_t cx = cp.column;
    size_t cy = cp.line;

    size_t xOfs = _siHorz.Pos;
    size_t yOfs = _siVert.Pos;

    if (_Insert) {
      _pCanvas[_CursorCanvas]->fillRectangle((cx-xOfs)*_Width, (cy-yOfs)*_Height, (cx-xOfs+1)*_Width, (cy-yOfs+1)*_Height-1, /*txtLine[cx].colText*/gConfig.Colors.colWhiteSpace, 0);
    } else {
      _pCanvas[_CursorCanvas]->drawHorzLine((cx-xOfs)*_Width, (cx-xOfs+1)*_Width, (cy-yOfs+1)*_Height-2, /*txtLine[cx].colText*/gConfig.Colors.colWhiteSpace, gfx::Canvas::flagBold);
    }
  }

  InvalidateRect(_hWnd, nullptr, FALSE);
}

// ----------------------------------------------------------------------------
void WinRaster::_setupVertScrollBar()
{
  const TextDocument* pDocument = _pParent->getDocument();
  size_t cLines = pDocument != nullptr ? pDocument->getDoc().getLineCount() : 0;

  RECT rc; GetClientRect(_hWnd, &rc);
  size_t cy    = (rc.bottom - rc.top + _siVert.Unit - 1) / _siVert.Unit;
  size_t ySize = cLines;

  _siVert.Max  = ySize > 0 ? ySize : 0;
  _siVert.Page = cy;
  _siVert.Pos  = clip<size_t>(_siVert.Pos, 0, _siVert.Max);
  
  _setupScrollBar(SB_VERT, _siVert);
}

// ----------------------------------------------------------------------------
void WinRaster::_setupHorzScrollBar()
{
  const TextDocument* pDocument = _pParent->getDocument();
  size_t cMaxLineLength = pDocument != nullptr ? pDocument->getDoc().getMaxLineLength() : 0;

  RECT rc; GetClientRect(_hWnd, &rc);
  size_t cx    = (rc.right - rc.left + _siHorz.Unit - 1) / _siHorz.Unit;
  size_t xSize = cMaxLineLength ? cMaxLineLength + _Width - cx : 0;

  _siHorz.Max  = xSize > 0 ? xSize - 1 : 0;
  _siHorz.Page = _Width;
  _siHorz.Pos  = clip<size_t>(_siHorz.Pos, 0, _siHorz.Max);

  _setupScrollBar(SB_HORZ, _siHorz);
}

// ----------------------------------------------------------------------------
void WinRaster::_setupScrollBar(int Direction, const ScrollInfo& siInfo)
{
  SCROLLINFO si = { 0 };
  si.cbSize = sizeof(si);
  si.fMask  = SIF_PAGE | SIF_POS | SIF_RANGE;
  si.nMin   = 0;
  si.nMax   = siInfo.Max;
  si.nPage  = siInfo.Page;
  si.nPos   = siInfo.Pos;

  SetScrollInfo(_hWnd, Direction, &si, FALSE);
}

// --------------------------------------------------------------------------
void WinRaster::_onScroll(int Request, int Pos, ScrollInfo* pInfo, int Direction)
{
  SCROLLINFO si = { 0 };
  si.cbSize = sizeof(si);
  si.fMask  = SIF_POS;

  switch (Request)
  {
  case SB_LINEDOWN:
    pInfo->Pos++;
    break;
    
  case SB_LINEUP:
    pInfo->Pos--;
    break;
    
  case SB_PAGEDOWN:
    pInfo->Pos += pInfo->Page;
    break;
    
  case SB_PAGEUP:
    pInfo->Pos -= pInfo->Page;
    break;

  case SB_THUMBPOSITION:
  case SB_THUMBTRACK:
    pInfo->Pos = Pos;
    break;

  case SB_BOTTOM:
    pInfo->Pos = pInfo->Max;
    break;

  case SB_TOP:
    pInfo->Pos = 0;
    break;
  }

  pInfo->Pos = clip<int>(pInfo->Pos, 0, pInfo->Max - pInfo->Page + 1);

  si.nPos = pInfo->Pos;

  SetScrollInfo(_hWnd, Direction, &si, TRUE);
}

// ----------------------------------------------------------------------------
void WinRaster::_setupCanvas(size_t cx, size_t cy)
{
  _Pels = cx;
  _Lines = cy;
  
  size_t Pels  = (cx + _siHorz.Unit - 1) / _siHorz.Unit * _siHorz.Unit;
  size_t Lines = (cy + _siVert.Unit - 1) / _siVert.Unit * _siVert.Unit;
  
  _TextLines = Lines / _cyLine;
  
  memset(&_bmi256.bmiHeader, 0, sizeof(_bmi256.bmiHeader));
  _bmi256.bmiHeader.biSize = sizeof(_bmi256.bmiHeader);
  _bmi256.bmiHeader.biPlanes = 1;
  _bmi256.bmiHeader.biBitCount = 32;
  _bmi256.bmiHeader.biWidth  =  static_cast<long>(Pels);
  _bmi256.bmiHeader.biHeight = -static_cast<long>(Lines);

  delete _pCanvas[0];
  delete _pCanvas[1];

  _pCanvas[0] = new gfx::Canvas(Pels, Lines);
  _pCanvas[1] = new gfx::Canvas(Pels, Lines);

  _RenderedState.DocumentVersion = ~0u;
}

// ----------------------------------------------------------------------------
void WinRaster::_scrollContent(size_t Lines, size_t y0, size_t y1)
{
  gfx::Canvas* pCanvas = _pCanvas[1-_CursorCanvas];
  gfx::Canvas* pCursorCanvas = _pCanvas[_CursorCanvas];

  size roi(_Pels, Lines*_Height);
  pCanvas->blit(pCursorCanvas, roi, 0, y0*_Height, 0, y1*_Height);

  _CursorCanvas = 1 - _CursorCanvas;
}

// ----------------------------------------------------------------------------
WinRaster::RenderState WinRaster::_getCurrentState() const
{
  RenderState State;

  State.Offset.column = _siHorz.Pos;
  State.Offset.line   = _siVert.Pos;

  TextDocument* pDoc = _pParent->getDocument();

  if (pDoc != nullptr)
  {
    State.DocumentVersion  = pDoc->getDocumentVersion();
    State.CursorVersion    = pDoc->getCursorVersion();
    State.SelectionVersion = pDoc->getSelectionVersion();

    State.Cursor = pDoc->getCursor();
    pDoc->getSelection(&State.SelBegin, &State.SelEnd);
  }

  const SearchEngine& Search = _pParent->getSearchEngine();
  State.SearchResultVersion = Search.getSearchResultVersion();

  return State;
}
