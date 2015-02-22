/*!
** \file    GfxApi/lib/DrawLib/TrueType.h
** \brief   
\*****************************************************************************/
#ifndef GFXAPI_LIB_DRAWLIB_TRUETYPE_H
#define GFXAPI_LIB_DRAWLIB_TRUETYPE_H

struct _bitmap
{
  int w, h, stride;
  uint8 *pixels;
};

enum VertexType
{
  STBTT_vmove=1,
  STBTT_vline,
  STBTT_vcurve
};

struct vertex
{
  int16 x, y, cx, cy;
  uint8 type;
  uint8 padding;
};

class TrueTypeFont
{
public:
  TrueTypeFont(const void* ttf);
  retcode init();

  int findGlyphIndex(int unicode_codepoint) const;
  void getGlyphHorzMetrics(int glyph_index, int *advanceWidth, int *leftSideBearing) const;
  int  getGlyphKernAdvance(int glyph1, int glyph2) const;
  void getFontVMetrics(int *ascent, int *descent, int *lineGap) const;
  int  getGlyphBox(int glyph_index, int *x0, int *y0, int *x1, int *y1) const;

  void getGlyphBitmapBoxSubpixel(int glyph, float scale_x, float scale_y, float shift_x, float shift_y, int *ix0, int *iy0, int *ix1, int *iy1) const;
  int getGlyphShape(int glyph_index, vertex **vertices) const;
  void rasterize(_bitmap *result, float flatness_in_pixels, vertex *vertices, int num_verts, float scale_x, float scale_y, float shift_x, float shift_y, int x_off, int y_off, int invert);

private:
  const void* _TTFData;

  // table locations as offset from start of .ttf
  const uint8 * _loca;
  const uint8 * _head;
  const uint8 * _glyf;
  const uint8 * _hhea;
  const uint8 * _hmtx;
  const uint8 * _kern;
  
  uint _NumGlyphs;
  const uint8 * _IndexMap;                     // a cmap mapping for our chosen character encoding
  int _IndexToLocFormat;              // format needed to map from glyph index to glyph

  const uint8 * _getGlyphOffset(uint glyph_index) const;
};

#endif // GFXAPI_LIB_DRAWLIB_TRUETYPE_H
