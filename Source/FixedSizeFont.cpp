#include <Source/Headers.h>
#include "FixedSizeFont.h"

// ============================================================================
gfx::FixedSizeFont::FixedSizeFont()
: cx(0)
, cy(0)
, firstChar(0)
, lastChar(0)
, a(nullptr)
, aPels(nullptr)
{
}

// ----------------------------------------------------------------------------
gfx::FixedSizeFont::FixedSizeFont(int cx, int cy, int firstChar, int lastChar)
: cx(cx)
, cy(cy)
, firstChar(firstChar)
, lastChar(lastChar)
{
  int cChars = lastChar - firstChar;

  aPels = new uint8[cx * cy * cChars * 4];
  a = new uint8*[cy * cChars * 4];

  for (int y = 0; y < cy * cChars * 4; y++) {
    a[y] = aPels + y * cx;
  }
}

// ----------------------------------------------------------------------------
gfx::FixedSizeFont::~FixedSizeFont()
{
  delete[] a;
  delete[] aPels;
}

// ----------------------------------------------------------------------------
void renderAntialiasedFont(const gfx::FixedSizeFont* pFont, int idx)
{
  int cx = pFont->cx;
  int cy = pFont->cy;

  int cChars = pFont->lastChar - pFont->firstChar;

  const int BitmapWidth = cx;
  const int BitmapHeight = cy * cChars;

  // create the fonts for rendering
  LOGFONT fnt = { 0 };
  fnt.lfCharSet = ANSI_CHARSET;
  fnt.lfOutPrecision = OUT_DEFAULT_PRECIS;
  fnt.lfClipPrecision = CLIP_DEFAULT_PRECIS;
  fnt.lfQuality = DEFAULT_QUALITY;
  fnt.lfPitchAndFamily = VARIABLE_PITCH | FF_SWISS;
  strcpy(fnt.lfFaceName, "Consolas");

  fnt.lfHeight = cy;
  fnt.lfWeight = 400 + 400 * (idx&1);

  HFONT hFont = CreateFontIndirect(&fnt);

  BITMAPINFO bmi = { 0 };
  bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
  bmi.bmiHeader.biWidth = BitmapWidth;
  bmi.bmiHeader.biHeight = BitmapHeight;
  bmi.bmiHeader.biPlanes = 1;
  bmi.bmiHeader.biBitCount = 32;

  BITMAP bm;
  bm.bmType = 0;
  bm.bmWidth = BitmapWidth;
  bm.bmHeight = BitmapHeight;
  bm.bmWidthBytes = BitmapWidth * 4;
  bm.bmPlanes = 1;
  bm.bmBitsPixel = 32;
  bm.bmBits = pFont->aPels;

  //HBITMAP hBmp = CreateBitmapIndirect(&bm);

  uint8* pBits = nullptr;
  HBITMAP hDIB = CreateDIBSection(0, &bmi, DIB_RGB_COLORS, (LPVOID*)&pBits, 0, 0);

  HDC hScreenDC = GetDC(0);
  HDC hDC = CreateCompatibleDC(hScreenDC);
  ReleaseDC(0, hScreenDC);

  HBITMAP hOldBMP = (HBITMAP)SelectObject(hDC, hDIB);
  HFONT hOldFont = (HFONT)SelectObject(hDC, hFont);

  RECT rect = {0, 0, BitmapWidth, BitmapHeight};
  FillRect(hDC, &rect, (HBRUSH)BLACK_BRUSH);

  SetTextColor(hDC, 0x00ffffff);
  SetBkColor(hDC, 0x00000000);

  for (int i = 0; i < cChars; ++i)
  {
    wchar text[2] = { (wchar)(pFont->firstChar + i), 0 };
    TextOutW(hDC, 0, i * cy, text, 1);
  }

  GdiFlush(); // Flush drawing

  int yOfs = idx * cChars * cy;

  for (int y = 0; y < BitmapHeight; y++)
  {
    for (int x = 0; x < BitmapWidth; x++)
    {
      pFont->a[y+yOfs][x] = pBits[(BitmapHeight-y-1) * BitmapWidth*4 + x*4 + 1];
    }
  }

  SelectObject(hDC, hOldFont);
  SelectObject(hDC, hOldBMP);

  DeleteDC(hDC);

  DeleteObject(hFont);
  DeleteObject(hDIB);
}

// ----------------------------------------------------------------------------
retcode gfx::loadTrueTypeFont(int cx, int cy, gfx::FixedSizeFont** ppFont)
{
  const int FirstChar = 32;
  const int LastChar = 256;

  gfx::FixedSizeFont* pFont = new gfx::FixedSizeFont(cx, cy, FirstChar, LastChar);

  for (int i = 0; i < 4; i++) {
    renderAntialiasedFont(pFont, i);
  }
  
  *ppFont = pFont;
  return rcSuccess;
}
