/*!
** \file    Source/Canvas.h
** \brief   Canvas is a class for rendering graphics primitives like lines,
**          rectangles and text.
\*****************************************************************************/
#pragma once

#include <Source/BaseTypes.h>

namespace gfx {

// ============================================================================
struct rgba_t
{
  uint8 r, g, b, a;
};

// ============================================================================
union pixel_t
{
  pixel_t();
  pixel_t(uint8 r, uint8 g, uint8 b, uint8 a);
  pixel_t(uint32 raw);
  
  uint32 raw;
  rgba_t rgba;
};

// ============================================================================
struct FixedSizeFont;

// ============================================================================
struct TextChar
{
  TextChar();
  TextChar(int ch);
  TextChar(int ch, uint32 color);

  int ch;
  uint32 colText;
  uint32 colLine;
  uint32 colBkg;

  uint32 bold:1;
  uint32 italic:1;
  uint32 underline:1;
  uint32 strikethrough:1;
  uint32 background:1;
};

// ============================================================================
struct Canvas
{
  Canvas(int cx, int cy);
  ~Canvas();

  enum LineFlags
  {
    flagDotted   = 1<<0,
    flagBold     = 1<<1,
    flagBrighten = 1<<2,
    flagDarken   = 1<<3,
  };

  void clear(uint32 color=0);
  void clear(int x0, int y0, int x1, int y1, uint32 color=0);

  void blit(Canvas* pDest);
  void blit(Canvas* pDest, const size& roi, int x0, int y0, int x1, int y1);

  void putPixel(int x, int y, uint32 color);

  void drawHorzLine(int x0, int x1, int y, uint32 color, uint flags);
  void drawVertLine(int x0, int x1, int y, uint32 color, uint flags);
  
  void fillRectangle(int x0, int y0, int x1, int y1, uint32 color, uint flags);
  void drawRectangle(int x0, int y0, int x1, int y1, uint32 color, uint flags);
  
  void drawText(int x, int y, const char* strText, const FixedSizeFont& font, uint32 color);
  void drawText(int x, int y, const TextChar* strText, const FixedSizeFont& font);

  int cx, cy;
  uint32** rgba;
  uint32* rgbaPels;
// ----------------------------------------------------------------------------
};


// ----------------------------------------------------------------------------
inline pixel_t::pixel_t()
: raw(0)
{}

// ----------------------------------------------------------------------------
inline pixel_t::pixel_t(uint8 r, uint8 g, uint8 b, uint8 a)
{
  rgba.r = r;
  rgba.g = g;
  rgba.b = b;
  rgba.a = a;
}

// ----------------------------------------------------------------------------
inline pixel_t::pixel_t(uint32 raw)
: raw(raw)
{}

// ----------------------------------------------------------------------------
inline TextChar::TextChar() : ch(0), colText(0), colLine(0), colBkg(0), bold(0), italic(0), underline(0), strikethrough(0) {}
inline TextChar::TextChar(int ch) : ch(ch), colText(0), colLine(0), colBkg(0), bold(0), italic(0), underline(0), strikethrough(0) {}
inline TextChar::TextChar(int ch, uint32 color) : ch(ch), colText(color), colLine(color), colBkg(color), bold(0), italic(0), underline(0), strikethrough(0) {}
  
// ============================================================================
uint32 blendPixel(uint32 x0, uint32 x1);
uint32 blendPixel(int Alpha, uint32 x0, uint32 x1);
pixel_t blendPixel(int Alpha, const pixel_t& x0, const pixel_t& x1);

} // namespace gfx
