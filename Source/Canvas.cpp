#include <Source/Headers.h>
#include <Source/ToolBox.h>
#include <Source/FixedSizeFont.h>
#include "Canvas.h"

// ============================================================================
inline uint8 blend256(int a, int x0, int x1)
{
  return static_cast<uint8>((a*x0 + (256-a)*x1 + 128) / 256);
}

// ============================================================================
inline void memfill32(void* ptr, uint32 val, size_t count)
{
  for (size_t i = 0; i < count; i++) {
    static_cast<uint32*>(ptr)[i] = val;
  }
}

// ============================================================================
gfx::pixel_t gfx::blendPixel(int a, const gfx::pixel_t& a0, const gfx::pixel_t& a1)
{
  gfx::pixel_t b;
  b.rgba.r = blend256(a, a0.rgba.r, a1.rgba.r);
  b.rgba.g = blend256(a, a0.rgba.g, a1.rgba.g);
  b.rgba.b = blend256(a, a0.rgba.b, a1.rgba.b);
  b.rgba.a = blend256(a, a0.rgba.a, a1.rgba.a);

  return b;
}

// ============================================================================
uint32 gfx::blendPixel(uint32 x0, uint32 x1)
{
  gfx::pixel_t a0, a1;
  a0.raw = x0;
  a1.raw = x1;

  return blendPixel(a0.rgba.a, a0, a1).raw;
}

// ============================================================================
uint32 gfx::blendPixel(int a, uint32 x0, uint32 x1)
{
  gfx::pixel_t a0, a1;
  a0.raw = x0;
  a1.raw = x1;

  return blendPixel(a, a0, a1).raw;
}

// ============================================================================
gfx::Canvas::Canvas(int cx, int cy)
: cx(cx)
, cy(cy)
{
  rgbaPels = new uint32[cx * cy];
  rgba = new uint32*[cy];

  for (int y = 0; y < cy; y++) {
    rgba[y] = rgbaPels + y * cx;
  }

  clear();
}

// ----------------------------------------------------------------------------
gfx::Canvas::~Canvas()
{
  delete[] rgba;
  delete[] rgbaPels;
}

// ----------------------------------------------------------------------------
void gfx::Canvas::clear(uint32 color)
{
  for (int y = 0; y < cy; y++) {
    memfill32(rgba[y], color, cx);
  }
}

// ----------------------------------------------------------------------------
void gfx::Canvas::clear(int x0, int y0, int x1, int y1, uint32 color)
{
  x0 = std::max(x0, 0);
  y0 = std::max(y0, 0);
  x1 = std::min(x1, cx);
  y1 = std::min(y1, cy);

  if (x0 >= x1 || y0 >= y1) {
    return;
  }

  for (int y = y0; y < y1; y++) {
    memfill32(&rgba[y][x0], color, x1-x0);
  }
}

// ----------------------------------------------------------------------------
void gfx::Canvas::blit(Canvas* pDest)
{
  for (int y = 0; y < cy; y++) {
    for (int x = 0; x < cx; x++) {
      pDest->rgba[y][x] = rgba[y][x];
    }
  }
}

// ----------------------------------------------------------------------------
void gfx::Canvas::blit(Canvas* pDest, const size& roi, int x0, int y0, int x1, int y1)
{
  size range(roi);
  int xOfs = - min(0, x0, x1);
  int yOfs = - min(0, y0, y1);

  x0 += xOfs;
  y0 += yOfs;
  x1 += xOfs;
  y1 += yOfs;
  range.cx -= xOfs;
  range.cy -= yOfs;

  range.cx -= std::max<size_t>(cx, x0 + range.cx) - cx;
  range.cy -= std::max<size_t>(cy, y0 + range.cy) - cy;
  range.cx -= std::max<size_t>(pDest->cx, x1 + range.cx) - pDest->cx;
  range.cy -= std::max<size_t>(pDest->cy, y1 + range.cy) - pDest->cy;

  for (size_t y = 0; y < range.cy; y++) {
    for (size_t x = 0; x < range.cx; x++) {
      pDest->rgba[y+y1][x+x1] = rgba[y+y0][x+x0];
    }
  }
}

// ----------------------------------------------------------------------------
void gfx::Canvas::putPixel(int x, int y, uint32 color)
{
  if (x >= 0 && x < cx && y >= 0 && y < cy) {
    rgba[y][x] = color;
  }
}

// ----------------------------------------------------------------------------
void gfx::Canvas::drawHorzLine(int x0, int x1, int y, uint32 color, uint flags)
{
  x0 = std::max(x0, 0);
  x1 = std::min(x1, cx-1);

  if (x0 >= x1 || y < 0 || y > cy) {
    return;
  }

  int bold = (flags & flagBold) != 0 ? 2 : 1;

  for (int b = 0; b < bold; b++)
  {
    for (int x = x0; x <= x1; x++)
    {
      if ((flags & flagDotted) == 0 || ((x+y+b) & 1) != 0) {
        putPixel(x, y+b, color);
      }
    }
  }
}

// ----------------------------------------------------------------------------
void gfx::Canvas::drawVertLine(int x, int y0, int y1, uint32 color, uint flags)
{
  y0 = std::max(y0, 0);
  y1 = std::min(y1, cy-1);

  if (y0 >= y1 || x < 0 || x > cx) {
    return;
  }

  int bold = (flags & flagBold) != 0 ? 2 : 1;

  for (int b = 0; b < bold; b++)
  {
    for (int y = y0; y <= y1; y++)
    {
      if ((flags & flagDotted) == 0 || ((x+y+b) & 1) != 0) {
        putPixel(x+b, y, color);
      }
    }
  }
}

// ----------------------------------------------------------------------------
void gfx::Canvas::fillRectangle(int x0, int y0, int x1, int y1, uint32 color, uint flags)
{
  x0 = std::max(x0, 0);
  y0 = std::max(y0, 0);
  x1 = std::min(x1, cx-1);
  y1 = std::min(y1, cy-1);

  if (x0 >= x1 || y0 >= y1) {
    return;
  }

  for (int y = y0; y <= y1; y++)
  {
    for (int x = x0; x <= x1; x++)
    {
      if ((flags & flagDotted) == 0 || ((x+y) & 1) != 0)
      {
        pixel_t a0, a1, b;
        a0.raw = color;
        a1.raw = rgba[y][x];

        b.rgba.r = blend256(a0.rgba.a, a0.rgba.r, a1.rgba.r);
        b.rgba.g = blend256(a0.rgba.a, a0.rgba.g, a1.rgba.g);
        b.rgba.b = blend256(a0.rgba.a, a0.rgba.b, a1.rgba.b);

        rgba[y][x] = b.raw;
      }
    }
  }
}

// ----------------------------------------------------------------------------
void gfx::Canvas::drawRectangle(int x0, int y0, int x1, int y1, uint32 color, uint flags)
{
  drawHorzLine(x0, x1, y0, color, flags);
  drawHorzLine(x0, x1, y1, color, flags);
  drawVertLine(x0, y0, y1, color, flags);
  drawVertLine(x1, y0, y1, color, flags);
}

// ----------------------------------------------------------------------------
int drawFixedSizeFontChar(gfx::Canvas& s, const gfx::FixedSizeFont& font, int sx, int sy, int ch, uint32 color, uint bold, uint italic)
{
  int c = ch;

  if (c < font.firstChar || c >= font.lastChar) {
    return 0;
  }
  
  if (sx >= s.cx || sy >= s.cy) {
    return 0;
  }

  c -= font.firstChar;
  
  int cChars = font.lastChar - font.firstChar;
  c += (bold + 2 * italic) * cChars;

  int x0 = 0;
  int y0 = c * font.cy;

  gfx::pixel_t d;
  d.raw = color;

  for (int y = 0; y < font.cy; y++) {
    for (int x = 0; x < font.cx; x++) {
      s.rgba[sy+y][sx+x] = gfx::blendPixel(font.a[y0+y][x0+x], d.raw, s.rgba[sy+y][sx+x]);
    }
  }

  return font.cx;
}

// ----------------------------------------------------------------------------
void gfx::Canvas::drawText(int x0, int y0, const char *strText, const gfx::FixedSizeFont& font, uint32 color)
{
  while (*strText)
  {
    int ch = (uint8)(*strText++);
    x0 += drawFixedSizeFontChar(*this, font, x0, y0, ch, color, 0, 0);
  }
}

// ----------------------------------------------------------------------------
void gfx::Canvas::drawText(int x, int y, const gfx::TextChar* strText, const gfx::FixedSizeFont& font)
{
  while (strText->ch)
  {
    int x0 = x;
    int x1 = x + font.cx - 1;

    int y0 = y;
    int y1 = y + font.cy - 1;

    if (strText->background) {
      fillRectangle(x0, y0, x1, y1, strText->colBkg, 0);
    }

    x += drawFixedSizeFontChar(*this, font, x, y, strText->ch, strText->colText, strText->bold, strText->italic);

    if (strText->underline) {
      drawHorzLine(x0, x1, y + font.cy-2, strText->colLine, flagBold|flagDotted);
    }

    strText++;
  }
}
