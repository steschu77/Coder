// Credits: stb_truetype.h - v0.2 - public domain - 2009 Sean Barrett / RAD Game Tools
// See https://github.com/nothings/stb

#include <Source/Headers.h>
#include <Source/FixedSizeFont.h>
#include "TrueType.h"

inline int ifloor(float x)
{
  return (int)floor(x);
}

inline int iceil(float x)
{
  return (int)ceil(x);
}

#define STBTT_MACSTYLE_DONTCARE     0
#define STBTT_MACSTYLE_BOLD         1
#define STBTT_MACSTYLE_ITALIC       2
#define STBTT_MACSTYLE_UNDERSCORE   4
#define STBTT_MACSTYLE_NONE         8   // <= not same as 0, this makes us check the bitfield is 0

enum
{ // platformID
  STBTT_PLATFORM_ID_UNICODE   =0,
  STBTT_PLATFORM_ID_MAC       =1,
  STBTT_PLATFORM_ID_ISO       =2,
  STBTT_PLATFORM_ID_MICROSOFT =3
};

enum
{ // encodingID for STBTT_PLATFORM_ID_UNICODE
  STBTT_UNICODE_EID_UNICODE_1_0    =0,
  STBTT_UNICODE_EID_UNICODE_1_1    =1,
  STBTT_UNICODE_EID_ISO_10646      =2,
  STBTT_UNICODE_EID_UNICODE_2_0_BMP=3,
  STBTT_UNICODE_EID_UNICODE_2_0_FULL=4
};

enum
{ // encodingID for STBTT_PLATFORM_ID_MICROSOFT
  STBTT_MS_EID_SYMBOL        =0,
  STBTT_MS_EID_UNICODE_BMP   =1,
  STBTT_MS_EID_SHIFTJIS      =2,
  STBTT_MS_EID_UNICODE_FULL  =10
};

enum
{ // encodingID for STBTT_PLATFORM_ID_MAC; same as Script Manager codes
  STBTT_MAC_EID_ROMAN        =0, STBTT_MAC_EID_ARABIC       =4,
  STBTT_MAC_EID_JAPANESE     =1, STBTT_MAC_EID_HEBREW       =5,
  STBTT_MAC_EID_CHINESE_TRAD =2, STBTT_MAC_EID_GREEK        =6,
  STBTT_MAC_EID_KOREAN       =3, STBTT_MAC_EID_RUSSIAN      =7
};

enum
{ // languageID for STBTT_PLATFORM_ID_MICROSOFT; same as LCID...
  // problematic because there are e.g. 16 english LCIDs and 16 arabic LCIDs
  STBTT_MS_LANG_ENGLISH     =0x0409, STBTT_MS_LANG_ITALIAN     =0x0410,
  STBTT_MS_LANG_CHINESE     =0x0804, STBTT_MS_LANG_JAPANESE    =0x0411,
  STBTT_MS_LANG_DUTCH       =0x0413, STBTT_MS_LANG_KOREAN      =0x0412,
  STBTT_MS_LANG_FRENCH      =0x040c, STBTT_MS_LANG_RUSSIAN     =0x0419,
  STBTT_MS_LANG_GERMAN      =0x0407, STBTT_MS_LANG_SPANISH     =0x0409,
  STBTT_MS_LANG_HEBREW      =0x040d, STBTT_MS_LANG_SWEDISH     =0x041D
};

enum
{ // languageID for STBTT_PLATFORM_ID_MAC
  STBTT_MAC_LANG_ENGLISH      =0, STBTT_MAC_LANG_JAPANESE     =11,
  STBTT_MAC_LANG_ARABIC       =12, STBTT_MAC_LANG_KOREAN       =23,
  STBTT_MAC_LANG_DUTCH        =4, STBTT_MAC_LANG_RUSSIAN      =32,
  STBTT_MAC_LANG_FRENCH       =1, STBTT_MAC_LANG_SPANISH      =6,
  STBTT_MAC_LANG_GERMAN       =2, STBTT_MAC_LANG_SWEDISH      =5,
  STBTT_MAC_LANG_HEBREW       =10, STBTT_MAC_LANG_CHINESE_SIMPLIFIED =33,
  STBTT_MAC_LANG_ITALIAN      =3, STBTT_MAC_LANG_CHINESE_TRAD =19
};

#define ttCHAR(p)     (* (int8 *) (p))
uint16 ttUSHORT(const uint8 *p) { return p[0]*256 + p[1]; }
int16 ttSHORT(const uint8 *p) { return p[0]*256 + p[1]; }
uint32 ttULONG(const uint8 *p) { return (p[0]<<24) + (p[1]<<16) + (p[2]<<8) + p[3]; }
int32 ttLONG(const uint8 *p) { return (p[0]<<24) + (p[1]<<16) + (p[2]<<8) + p[3]; }

#define fourcc(a,b,c,d) (uint32)((a)|((b)<<8)|((c)<<16)|((d)<<24))

static int _isfont(uint32 font)
{
  // check the version number
  if (font == fourcc('1', 0,  0,  0))  return 1; // TrueType 1
  if (font == fourcc('t','y','p','1')) return 1; // TrueType with type 1 font -- we don't support this!
  if (font == fourcc('O','T','T','O')) return 1; // OpenType with CFF
  if (font == fourcc( 0,  1,  0,  0))  return 1; // OpenType 1.0
  return 0;
}

static const uint8 * _find_table(const uint8 *data, uint32 tag)
{
  int32 num_tables = ttUSHORT(data+4);
  uint32 tabledir = 12;
  for (int32 i = 0; i < num_tables; ++i) {
    const uint8 * loc = data + tabledir + 16*i;
    if (tag == fourcc(loc[0], loc[1], loc[2], loc[3])) {
      return data + ttULONG(loc + 8);
    }
  }
  return nullptr;
}

int getFontOffsetForIndex(const uint8 *data, int index)
{
  // if it's just a font, there's only one valid index
  uint32 fccFontCollection = fourcc(data[0], data[1], data[2], data[3]);
  if (_isfont(fccFontCollection))
    return index == 0 ? 0 : -1;

  // check if it's a TTC
  if (fccFontCollection == fourcc('t','t','c','f')) {
    // version 1?
    if (ttULONG(data+4) == 0x00010000 || ttULONG(data+4) == 0x00020000) {
      int32 n = ttLONG(data+8);
      return (index >= n) ? -1 : ttULONG(data+12+index*14);
    }
  }
  return -1;
}

TrueTypeFont::TrueTypeFont(const void* ttf)
: _TTFData(ttf)
, _NumGlyphs(0)
, _IndexMap(0)
, _IndexToLocFormat(0)
{
}

// ----------------------------------------------------------------------------
retcode TrueTypeFont::init()
{
  const uint8 * data = (const uint8*)_TTFData;
  const uint8 * cmap = _find_table(data, fourcc('c','m','a','p')); // required
  _loca = _find_table(data, fourcc('l','o','c','a')); // required
  _head = _find_table(data, fourcc('h','e','a','d')); // required
  _glyf = _find_table(data, fourcc('g','l','y','f')); // required
  _hhea = _find_table(data, fourcc('h','h','e','a')); // required
  _hmtx = _find_table(data, fourcc('h','m','t','x')); // required
  _kern = _find_table(data, fourcc('k','e','r','n')); // not required

  if (!cmap || !_loca || !_head || !_glyf || !_hhea || !_hmtx) {
    return rcInvalidFormat;
  }

  const uint8 * maxp = _find_table(data, fourcc('m','a','x','p'));
  _NumGlyphs = maxp ? ttUSHORT(maxp+4) : 0xffff;

  // find a cmap encoding table we understand to avoid searching later.
  int numTables = ttUSHORT(cmap + 2);
  for (int i = 0; i < numTables; ++i)
  {
    const uint8 * encoding_record = cmap + 4 + 8 * i;
    switch (ttUSHORT(encoding_record)) {
    case STBTT_PLATFORM_ID_MICROSOFT:
      switch (ttUSHORT(encoding_record+2)) {
      case STBTT_MS_EID_UNICODE_BMP:
      case STBTT_MS_EID_UNICODE_FULL:
        _IndexMap = cmap + ttULONG(encoding_record+4); // MS/Unicode
        break;
      }
      break;
    }
  }

  if (_IndexMap == 0)
    return 0;

  _IndexToLocFormat = ttUSHORT(_head + 50);
  return rcSuccess;
}

// ----------------------------------------------------------------------------
int TrueTypeFont::findGlyphIndex(int unicode_codepoint) const
{
  uint16 format = ttUSHORT(_IndexMap + 0);
  if (format == 0) { // apple byte encoding
    int32 bytes = ttUSHORT(_IndexMap + 2);
    if (unicode_codepoint < bytes-6)
      return *(_IndexMap + 6 + unicode_codepoint);
    return 0;
  }
  else if (format == 6) {
    uint32 first = ttUSHORT(_IndexMap + 6);
    uint32 count = ttUSHORT(_IndexMap + 8);
    if ((uint32)unicode_codepoint >= first && (uint32)unicode_codepoint < first+count)
      return ttUSHORT(_IndexMap + 10 + (unicode_codepoint - first)*2);
    return 0;
  }
  else if (format == 2) {
    return 0; // @TODO: high-byte mapping for japanese/chinese/korean
  }
  else if (format == 4) { // standard mapping for windows fonts: binary search collection of ranges
    uint16 segcount = ttUSHORT(_IndexMap+6) >> 1;
    uint16 searchRange = ttUSHORT(_IndexMap+8) >> 1;
    uint16 entrySelector = ttUSHORT(_IndexMap+10);
    uint16 rangeShift = ttUSHORT(_IndexMap+12) >> 1;

    // do a binary search of the segments
    const uint8 * endCount = _IndexMap + 14;
    const uint8 * search = endCount;

    if (unicode_codepoint > 0xffff)
      return 0;

    // they lie from endCount .. endCount + segCount
    // but searchRange is the nearest power of two, so...
    if (unicode_codepoint >= ttUSHORT(search + rangeShift*2))
      search += rangeShift*2;

    // now decrement to bias correctly to find smallest
    search -= 2;
    while (entrySelector) {
      searchRange >>= 1;
      uint16 end = ttUSHORT(search + searchRange*2);
      if (unicode_codepoint > end)
        search += searchRange*2;
      --entrySelector;
    }
    search += 2;

    uint16 item = (uint16)((search - endCount) >> 1);

    assert(unicode_codepoint <= ttUSHORT(endCount + 2*item));
    uint16 start = ttUSHORT(_IndexMap + 14 + segcount*2 + 2 + 2*item);
    if (unicode_codepoint < start)
      return 0;

    uint16 offset = ttUSHORT(_IndexMap + 14 + segcount*6 + 2 + 2*item);
    if (offset == 0)
      return (uint16)(unicode_codepoint + ttSHORT(_IndexMap + 14 + segcount*4 + 2 + 2*item));

    return ttUSHORT(_IndexMap + 14 + segcount*6 + 2 + 2*item + offset + (unicode_codepoint-start)*2);
  }
  else if (format == 12 || format == 13) {
    uint32 ngroups = ttULONG(_IndexMap+12);
    int32 low, high;
    low = 0; high = (int32)ngroups;
    // Binary search the right group.
    while (low < high) {
      int32 mid = low + ((high-low) >> 1); // rounds down, so low <= mid < high
      uint32 start_char = ttULONG(_IndexMap+16+mid*12);
      uint32 end_char = ttULONG(_IndexMap+16+mid*12+4);
      if ((uint32)unicode_codepoint < start_char)
        high = mid;
      else if ((uint32)unicode_codepoint > end_char)
        low = mid+1;
      else {
        uint32 start_glyph = ttULONG(_IndexMap+16+mid*12+8);
        if (format == 12)
          return start_glyph + unicode_codepoint-start_char;
        else // format == 13
          return start_glyph;
      }
    }
    return 0; // not found
  }
  // @TODO
  assert(0);
  return 0;
}

static void setvertex(vertex *v, uint8 type, int16 x, int16 y, int16 cx, int16 cy)
{
  v->type = type;
  v->x = x;
  v->y = y;
  v->cx = cx;
  v->cy = cy;
}

const uint8 * TrueTypeFont::_getGlyphOffset(uint glyph_index) const
{
  if (glyph_index >= _NumGlyphs) return nullptr; // glyph index out of range
  if (_IndexToLocFormat >= 2)    return nullptr; // unknown index->glyph map format

  if (_IndexToLocFormat == 0) {
    int g1 = ttUSHORT(_loca + glyph_index * 2) * 2;
    int g2 = ttUSHORT(_loca + glyph_index * 2 + 2) * 2;
    return g1==g2 ? nullptr : _glyf + g1; // if length is 0, return -1
  }
  else {
    int g1 = ttULONG(_loca + glyph_index * 4);
    int g2 = ttULONG(_loca + glyph_index * 4 + 4);
    return g1==g2 ? nullptr : _glyf + g1; // if length is 0, return -1
  }
}

// ----------------------------------------------------------------------------
int TrueTypeFont::getGlyphBox(int glyph_index, int *x0, int *y0, int *x1, int *y1) const
{
  const uint8 * g = _getGlyphOffset(glyph_index);
  if (g == nullptr) return 0;

  if (x0) *x0 = ttSHORT(g + 2);
  if (y0) *y0 = ttSHORT(g + 4);
  if (x1) *x1 = ttSHORT(g + 6);
  if (y1) *y1 = ttSHORT(g + 8);
  return 1;
}

// ----------------------------------------------------------------------------
int TrueTypeFont::getGlyphShape(int glyph_index, vertex **pvertices) const
{
  *pvertices = nullptr;

  const uint8 * g = _getGlyphOffset(glyph_index);
  if (g == nullptr) return 0;

  vertex *vertices=0;
  int num_vertices=0;

  int16 numberOfContours = ttSHORT(g);

  if (numberOfContours > 0) {
    uint8 flags=0;
    int32 i, j=0, n, was_off=0;
    int16 x, y, cx, cy, sx, sy;
    const uint8* endPtsOfContours = (g + 10);
    int32 ins = ttUSHORT(g + 10 + numberOfContours * 2);
    const uint8* points = g + 10 + numberOfContours * 2 + 2 + ins;

    n = 1+ttUSHORT(endPtsOfContours + numberOfContours*2-2);

    int32 m = n + numberOfContours;  // a loose bound on how many vertices we might need
    vertices = (vertex *)malloc(m * sizeof(vertices[0]));
    if (vertices == 0)
      return 0;

    int32 next_move = 0;
    uint8 flagcount = 0;

    // in first pass, we load uninterpreted data into the allocated array
    // above, shifted to the end of the array so we won't overwrite it when
    // we create our final data starting from the front

    int32 off = m - n; // starting offset for uninterpreted data, regardless of how m ends up being calculated

    // first load flags

    for (i=0; i < n; ++i) {
      if (flagcount == 0) {
        flags = *points++;
        if (flags & 8)
          flagcount = *points++;
      }
      else
        --flagcount;
      vertices[off+i].type = flags;
    }

    // now load x coordinates
    x=0;
    for (i=0; i < n; ++i) {
      flags = vertices[off+i].type;
      if (flags & 2) {
        int16 dx = *points++;
        x += (flags & 16) ? dx : -dx; // ???
      }
      else {
        if (!(flags & 16)) {
          x = x + (int16)(points[0]*256 + points[1]);
          points += 2;
        }
      }
      vertices[off+i].x = x;
    }

    // now load y coordinates
    y=0;
    for (i=0; i < n; ++i) {
      flags = vertices[off+i].type;
      if (flags & 4) {
        int16 dy = *points++;
        y += (flags & 32) ? dy : -dy; // ???
      }
      else {
        if (!(flags & 32)) {
          y = y + (int16)(points[0]*256 + points[1]);
          points += 2;
        }
      }
      vertices[off+i].y = y;
    }

    // now convert them to our format
    num_vertices=0;
    sx = sy = cx = cy = 0;
    for (i=0; i < n; ++i) {
      flags = vertices[off+i].type;
      x     = (int16)vertices[off+i].x;
      y     = (int16)vertices[off+i].y;
      if (next_move == i) {
        // when we get to the end, we have to close the shape explicitly
        if (i != 0) {
          if (was_off)
            setvertex(&vertices[num_vertices++], STBTT_vcurve, sx, sy, cx, cy);
          else
            setvertex(&vertices[num_vertices++], STBTT_vline, sx, sy, 0, 0);
        }

        // now start the new one               
        setvertex(&vertices[num_vertices++], STBTT_vmove, x, y, 0, 0);
        next_move = 1 + ttUSHORT(endPtsOfContours+j*2);
        ++j;
        was_off = 0;
        sx = x;
        sy = y;
      }
      else {
        if (!(flags & 1)) { // if it's a curve
          if (was_off) // two off-curve control points in a row means interpolate an on-curve midpoint
            setvertex(&vertices[num_vertices++], STBTT_vcurve, (cx+x)>>1, (cy+y)>>1, cx, cy);
          cx = x;
          cy = y;
          was_off = 1;
        }
        else {
          if (was_off)
            setvertex(&vertices[num_vertices++], STBTT_vcurve, x, y, cx, cy);
          else
            setvertex(&vertices[num_vertices++], STBTT_vline, x, y, 0, 0);
          was_off = 0;
        }
      }
    }
    if (i != 0) {
      if (was_off)
        setvertex(&vertices[num_vertices++], STBTT_vcurve, sx, sy, cx, cy);
      else
        setvertex(&vertices[num_vertices++], STBTT_vline, sx, sy, 0, 0);
    }
  }
  else if (numberOfContours == -1) {
    // Compound shapes.
    int more = 1;
    const uint8 *comp = g + 10;
    num_vertices = 0;
    vertices = 0;
    while (more) {
      uint16 flags, gidx;
      int comp_num_verts = 0, i;
      vertex *comp_verts = 0, *tmp = 0;
      float mtx[6] ={1, 0, 0, 1, 0, 0}, m, n;

      flags = ttSHORT(comp); comp+=2;
      gidx = ttSHORT(comp); comp+=2;

      if (flags & 2) { // XY values
        if (flags & 1) { // shorts
          mtx[4] = ttSHORT(comp); comp+=2;
          mtx[5] = ttSHORT(comp); comp+=2;
        }
        else {
          mtx[4] = ttCHAR(comp); comp+=1;
          mtx[5] = ttCHAR(comp); comp+=1;
        }
      }
      else {
        // @TODO handle matching point
        assert(0);
      }
      if (flags & (1<<3)) { // WE_HAVE_A_SCALE
        mtx[0] = mtx[3] = ttSHORT(comp)/16384.0f; comp+=2;
        mtx[1] = mtx[2] = 0;
      }
      else if (flags & (1<<6)) { // WE_HAVE_AN_X_AND_YSCALE
        mtx[0] = ttSHORT(comp)/16384.0f; comp+=2;
        mtx[1] = mtx[2] = 0;
        mtx[3] = ttSHORT(comp)/16384.0f; comp+=2;
      }
      else if (flags & (1<<7)) { // WE_HAVE_A_TWO_BY_TWO
        mtx[0] = ttSHORT(comp)/16384.0f; comp+=2;
        mtx[1] = ttSHORT(comp)/16384.0f; comp+=2;
        mtx[2] = ttSHORT(comp)/16384.0f; comp+=2;
        mtx[3] = ttSHORT(comp)/16384.0f; comp+=2;
      }

      // Find transformation scales.
      m = (float)sqrt(mtx[0]*mtx[0] + mtx[1]*mtx[1]);
      n = (float)sqrt(mtx[2]*mtx[2] + mtx[3]*mtx[3]);

      // Get indexed glyph.
      comp_num_verts = getGlyphShape(gidx, &comp_verts);
      if (comp_num_verts > 0) {
        // Transform vertices.
        for (i = 0; i < comp_num_verts; ++i) {
          vertex* v = &comp_verts[i];
          int16 x, y;
          x=v->x; y=v->y;
          v->x = (int16)(m * (mtx[0]*x + mtx[2]*y + mtx[4]));
          v->y = (int16)(n * (mtx[1]*x + mtx[3]*y + mtx[5]));
          x=v->cx; y=v->cy;
          v->cx = (int16)(m * (mtx[0]*x + mtx[2]*y + mtx[4]));
          v->cy = (int16)(n * (mtx[1]*x + mtx[3]*y + mtx[5]));
        }
        // Append vertices.
        tmp = (vertex*)malloc((num_vertices+comp_num_verts)*sizeof(vertex));
        if (!tmp) {
          if (vertices) free(vertices);
          if (comp_verts) free(comp_verts);
          return 0;
        }
        if (num_vertices > 0) memcpy(tmp, vertices, num_vertices*sizeof(vertex));
        memcpy(tmp+num_vertices, comp_verts, comp_num_verts*sizeof(vertex));
        if (vertices) free(vertices);
        vertices = tmp;
        free(comp_verts);
        num_vertices += comp_num_verts;
      }
      // More components ?
      more = flags & (1<<5);
    }
  }
  else if (numberOfContours < 0) {
    // @TODO other compound variations?
    assert(0);
  }
  else {
    // numberOfCounters == 0, do nothing
  }

  *pvertices = vertices;
  return num_vertices;
}

// ----------------------------------------------------------------------------
void TrueTypeFont::getGlyphHorzMetrics(int glyph_index, int *advanceWidth, int *leftSideBearing) const
{
  uint16 numOfLongHorMetrics = ttUSHORT(_hhea + 34);
  if (glyph_index < numOfLongHorMetrics) {
    if (advanceWidth)     *advanceWidth    = ttSHORT(_hmtx + 4*glyph_index);
    if (leftSideBearing)  *leftSideBearing = ttSHORT(_hmtx + 4*glyph_index + 2);
  }
  else {
    if (advanceWidth)     *advanceWidth    = ttSHORT(_hmtx + 4*(numOfLongHorMetrics-1));
    if (leftSideBearing)  *leftSideBearing = ttSHORT(_hmtx + 4*numOfLongHorMetrics + 2*(glyph_index - numOfLongHorMetrics));
  }
}

// ----------------------------------------------------------------------------
int TrueTypeFont::getGlyphKernAdvance(int glyph1, int glyph2) const
{
  // we only look at the first table. it must be 'horizontal' and format 0.
  // number of tables, need at least 1
  // horizontal flag must be set in format
  if (!_kern || ttUSHORT(_kern+2) < 1 || ttUSHORT(_kern+8) != 1) {
    return 0;
  }

  int l = 0;
  int r = ttUSHORT(_kern+10) - 1;
  uint32 needle = glyph1 << 16 | glyph2;
  while (l <= r) {
    int m = (l + r) >> 1;
    uint32 straw = ttULONG(_kern+18+(m*6)); // note: unaligned read
    if (needle < straw)
      r = m - 1;
    else if (needle > straw)
      l = m + 1;
    else
      return ttSHORT(_kern+18+(m*6)+4);
  }
  return 0;
}

void TrueTypeFont::getFontVMetrics(int *ascent, int *descent, int *lineGap) const
{
  if (ascent)  *ascent  = ttSHORT(_hhea + 4);
  if (descent) *descent = ttSHORT(_hhea + 6);
  if (lineGap) *lineGap = ttSHORT(_hhea + 8);
}

void TrueTypeFont::getGlyphBitmapBoxSubpixel(int glyph, float scale_x, float scale_y, float shift_x, float shift_y, int *ix0, int *iy0, int *ix1, int *iy1) const
{
  int x0, y0, x1, y1;
  if (!getGlyphBox(glyph, &x0, &y0, &x1, &y1)) {
    x0=y0=x1=y1=0; // e.g. space character
  }
  // now move to integral bboxes (treating pixels as little squares, what pixels get touched)?
  if (ix0) *ix0 =  ifloor(x0 * scale_x + shift_x);
  if (iy0) *iy0 = -iceil(y1 * scale_y + shift_y);
  if (ix1) *ix1 =  iceil(x1 * scale_x + shift_x);
  if (iy1) *iy1 = -ifloor(y0 * scale_y + shift_y);
}

struct _edge
{
  float x0, y0, x1, y1;
  int invert;
};

struct _active_edge
{
  int x, dx;
  float ey;
  struct _active_edge *next;
  int valid;
};

#define FIXSHIFT   10
#define FIX        (1 << FIXSHIFT)
#define FIXMASK    (FIX-1)

static _active_edge *new_active(_edge *e, int off_x, float start_point)
{
  _active_edge *z = (_active_edge *)malloc(sizeof(*z)); // @TODO: make a pool of these!!!
  float dxdy = (e->x1 - e->x0) / (e->y1 - e->y0);
  assert(e->y0 <= start_point);
  if (!z) return z;
  // round dx down to avoid going too far
  if (dxdy < 0)
    z->dx = -ifloor(FIX * -dxdy);
  else
    z->dx = ifloor(FIX * dxdy);
  z->x = ifloor(FIX * (e->x0 + dxdy * (start_point - e->y0)));
  z->x -= off_x * FIX;
  z->ey = e->y1;
  z->next = 0;
  z->valid = e->invert ? 1 : -1;
  return z;
}

// note: this routine clips fills that extend off the edges... ideally this
// wouldn't happen, but it could happen if the truetype glyph bounding boxes
// are wrong, or if the user supplies a too-small bitmap
static void _fill_active_edges(uint8 *scanline, int len, _active_edge *e, int max_weight)
{
  // non-zero winding fill
  int x0=0, w=0;

  while (e) {
    if (w == 0) {
      // if we're currently at zero, we need to record the edge start point
      x0 = e->x; w += e->valid;
    }
    else {
      int x1 = e->x; w += e->valid;
      // if we went to zero, we need to draw
      if (w == 0) {
        int i = x0 >> FIXSHIFT;
        int j = x1 >> FIXSHIFT;

        if (i < len && j >= 0) {
          if (i == j) {
            // x0,x1 are the same pixel, so compute combined coverage
            scanline[i] = scanline[i] + (uint8)((x1 - x0) * max_weight >> FIXSHIFT);
          }
          else {
            if (i >= 0) // add antialiasing for x0
              scanline[i] = scanline[i] + (uint8)(((FIX - (x0 & FIXMASK)) * max_weight) >> FIXSHIFT);
            else
              i = -1; // clip

            if (j < len) // add antialiasing for x1
              scanline[j] = scanline[j] + (uint8)(((x1 & FIXMASK) * max_weight) >> FIXSHIFT);
            else
              j = len; // clip

            for (++i; i < j; ++i) // fill pixels between x0 and x1
              scanline[i] = scanline[i] + (uint8)max_weight;
          }
        }
      }
    }

    e = e->next;
  }
}

static void _rasterize_sorted_edges(_bitmap *result, _edge *e, int n, int vsubsample, int off_x, int off_y)
{
  _active_edge *active = nullptr;
  int y, j=0;
  int max_weight = (255 / vsubsample);  // weight per vertical scanline
  int s; // vertical subsample index
  uint8 scanline_data[512], *scanline;

  if (result->w > 512)
    scanline = (uint8 *)malloc(result->w);
  else
    scanline = scanline_data;

  y = off_y * vsubsample;
  e[n].y0 = (off_y + result->h) * (float)vsubsample + 1;

  while (j < result->h) {
    memset(scanline, 0, result->w);
    for (s=0; s < vsubsample; ++s) {
      // find center of pixel for this scanline
      float scan_y = y + 0.5f;
      _active_edge **step = &active;

      // update all active edges;
      // remove all active edges that terminate before the center of this scanline
      while (*step) {
        _active_edge * z = *step;
        if (z->ey <= scan_y) {
          *step = z->next; // delete from list
          assert(z->valid);
          z->valid = 0;
          free(z);
        }
        else {
          z->x += z->dx; // advance to position for current scanline
          step = &((*step)->next); // advance through list
        }
      }

      // resort the list if needed
      for (;;) {
        int changed=0;
        step = &active;
        while (*step && (*step)->next) {
          if ((*step)->x > (*step)->next->x) {
            _active_edge *t = *step;
            _active_edge *q = t->next;

            t->next = q->next;
            q->next = t;
            *step = q;
            changed = 1;
          }
          step = &(*step)->next;
        }
        if (!changed) break;
      }

      // insert all edges that start before the center of this scanline -- omit ones that also end on this scanline
      while (e->y0 <= scan_y) {
        if (e->y1 > scan_y) {
          _active_edge *z = new_active(e, off_x, scan_y);
          // find insertion point
          if (active == nullptr)
            active = z;
          else if (z->x < active->x) {
            // insert at front
            z->next = active;
            active = z;
          }
          else {
            // find thing to insert AFTER
            _active_edge *p = active;
            while (p->next && p->next->x < z->x)
              p = p->next;
            // at this point, p->next->x is NOT < z->x
            z->next = p->next;
            p->next = z;
          }
        }
        ++e;
      }

      // now process all active edges in XOR fashion
      if (active)
        _fill_active_edges(scanline, result->w, active, max_weight);

      ++y;
    }
    memcpy(result->pixels + j * result->stride, scanline, result->w);
    ++j;
  }

  while (active) {
    _active_edge *z = active;
    active = active->next;
    free(z);
  }

  if (scanline != scanline_data)
    free(scanline);
}

static int _edge_compare(const void *p, const void *q)
{
  _edge *a = (_edge *)p;
  _edge *b = (_edge *)q;

  if (a->y0 < b->y0) return -1;
  if (a->y0 > b->y0) return  1;
  return 0;
}

struct _point
{
  float x, y;
};

static void _rasterize(_bitmap *result, _point *pts, int *wcount, int windings, float scale_x, float scale_y, float shift_x, float shift_y, int off_x, int off_y, int invert)
{
  float y_scale_inv = invert ? -scale_y : scale_y;
  _edge *e;
  int n, i, j, k, m;
  int vsubsample = result->h < 8 ? 15 : 5;
  // vsubsample should divide 255 evenly; otherwise we won't reach full opacity

  // now we have to blow out the windings into explicit edge lists
  n = 0;
  for (i=0; i < windings; ++i)
    n += wcount[i];

  e = (_edge *)malloc(sizeof(*e) * (n+1)); // add an extra one as a sentinel
  if (e == 0) return;
  n = 0;

  m=0;
  for (i=0; i < windings; ++i) {
    _point *p = pts + m;
    m += wcount[i];
    j = wcount[i]-1;
    for (k=0; k < wcount[i]; j=k++) {
      int a=k, b=j;
      // skip the edge if horizontal
      if (p[j].y == p[k].y)
        continue;
      // add edge from j to k to the list
      e[n].invert = 0;
      if (invert ? p[j].y > p[k].y : p[j].y < p[k].y) {
        e[n].invert = 1;
        a=j, b=k;
      }
      e[n].x0 = p[a].x * scale_x + shift_x;
      e[n].y0 = p[a].y * y_scale_inv * vsubsample + shift_y;
      e[n].x1 = p[b].x * scale_x + shift_x;
      e[n].y1 = p[b].y * y_scale_inv * vsubsample + shift_y;
      ++n;
    }
  }

  // now sort the edges by their highest point (should snap to integer, and then by x)
  qsort(e, n, sizeof(e[0]), _edge_compare);

  // now, traverse the scanlines and find the intersections on each scanline, use xor winding rule
  _rasterize_sorted_edges(result, e, n, vsubsample, off_x, off_y);

  free(e);
}

static void _add_point(_point *points, int n, float x, float y)
{
  if (!points) return; // during first pass, it's unallocated
  points[n].x = x;
  points[n].y = y;
}

// tesselate until threshhold p is happy... @TODO warped to compensate for non-linear stretching
static int _tesselate_curve(_point *points, int *num_points, float x0, float y0, float x1, float y1, float x2, float y2, float objspace_flatness_squared, int n)
{
  // midpoint
  float mx = (x0 + 2*x1 + x2)/4;
  float my = (y0 + 2*y1 + y2)/4;
  // versus directly drawn line
  float dx = (x0+x2)/2 - mx;
  float dy = (y0+y2)/2 - my;
  if (n > 16) // 65536 segments on one curve better be enough!
    return 1;
  if (dx*dx+dy*dy > objspace_flatness_squared) { // half-pixel error allowed... need to be smaller if AA
    _tesselate_curve(points, num_points, x0, y0, (x0+x1)/2.0f, (y0+y1)/2.0f, mx, my, objspace_flatness_squared, n+1);
    _tesselate_curve(points, num_points, mx, my, (x1+x2)/2.0f, (y1+y2)/2.0f, x2, y2, objspace_flatness_squared, n+1);
  }
  else {
    _add_point(points, *num_points, x2, y2);
    *num_points = *num_points+1;
  }
  return 1;
}

// returns number of contours
_point *FlattenCurves(vertex *vertices, int num_verts, float objspace_flatness, int **contour_lengths, int *num_contours)
{
  _point *points=0;
  int num_points=0;

  float objspace_flatness_squared = objspace_flatness * objspace_flatness;
  int i, n=0, start=0, pass;

  // count how many "moves" there are to get the contour count
  for (i=0; i < num_verts; ++i)
    if (vertices[i].type == STBTT_vmove)
      ++n;

  *num_contours = n;
  if (n == 0) return 0;

  *contour_lengths = (int *)malloc(sizeof(**contour_lengths) * n);

  if (*contour_lengths == 0) {
    *num_contours = 0;
    return 0;
  }

  // make two passes through the points so we don't need to realloc
  for (pass=0; pass < 2; ++pass) {
    float x=0, y=0;
    if (pass == 1) {
      points = (_point *)malloc(num_points * sizeof(points[0]));
      if (points == nullptr) goto error;
    }
    num_points = 0;
    n= -1;
    for (i=0; i < num_verts; ++i) {
      switch (vertices[i].type) {
      case STBTT_vmove:
        // start the next contour
        if (n >= 0)
          (*contour_lengths)[n] = num_points - start;
        ++n;
        start = num_points;

        x = vertices[i].x, y = vertices[i].y;
        _add_point(points, num_points++, x, y);
        break;
      case STBTT_vline:
        x = vertices[i].x, y = vertices[i].y;
        _add_point(points, num_points++, x, y);
        break;
      case STBTT_vcurve:
        _tesselate_curve(points, &num_points, x, y,
          vertices[i].cx, vertices[i].cy,
          vertices[i].x, vertices[i].y,
          objspace_flatness_squared, 0);
        x = vertices[i].x, y = vertices[i].y;
        break;
      }
    }
    (*contour_lengths)[n] = num_points - start;
  }

  return points;
error:
  free(points);
  free(*contour_lengths);
  *contour_lengths = 0;
  *num_contours = 0;
  return nullptr;
}

void Rasterize(_bitmap *result, float flatness_in_pixels, vertex *vertices, int num_verts, float scale_x, float scale_y, float shift_x, float shift_y, int x_off, int y_off, int invert)
{
  float scale = scale_x > scale_y ? scale_y : scale_x;
  int winding_count, *winding_lengths;
  _point *windings = FlattenCurves(vertices, num_verts, flatness_in_pixels / scale, &winding_lengths, &winding_count);
  if (windings) {
    _rasterize(result, windings, winding_lengths, winding_count, scale_x, scale_y, shift_x, shift_y, x_off, y_off, invert);
    free(winding_lengths);
    free(windings);
  }
}

uint8 *GetGlyphBitmapSubpixel(const TrueTypeFont& ttf, float scale_x, float scale_y, float shift_x, float shift_y, int glyph, int *width, int *height, int *xoff, int *yoff)
{
  int ix0, iy0, ix1, iy1;
  _bitmap gbm;
  vertex *vertices;
  int num_verts = ttf.getGlyphShape(glyph, &vertices);

  if (scale_x == 0) scale_x = scale_y;
  if (scale_y == 0) {
    if (scale_x == 0) return nullptr;
    scale_y = scale_x;
  }

  ttf.getGlyphBitmapBoxSubpixel(glyph, scale_x, scale_y, 0.0f, 0.0f, &ix0, &iy0, &ix1, &iy1);

  // now we get the size
  gbm.w = (ix1 - ix0);
  gbm.h = (iy1 - iy0);
  gbm.pixels = nullptr; // in case we error

  if (width) *width  = gbm.w;
  if (height) *height = gbm.h;
  if (xoff) *xoff   = ix0;
  if (yoff) *yoff   = iy0;

  if (gbm.w && gbm.h) {
    gbm.pixels = (uint8 *)malloc(gbm.w * gbm.h);
    if (gbm.pixels) {
      gbm.stride = gbm.w;

      Rasterize(&gbm, 0.35f, vertices, num_verts, scale_x, scale_y, shift_x, shift_y, ix0, iy0, 1);
    }
  }
  free(vertices);
  return gbm.pixels;
}

// ----------------------------------------------------------------------------
void MakeGlyphBitmapSubpixel(const TrueTypeFont& ttf, uint8 *output, int out_w, int out_h, int out_stride, float scale_x, float scale_y, float shift_x, float shift_y, int glyph)
{
  int ix0, iy0;
  vertex *vertices;
  int num_verts = ttf.getGlyphShape(glyph, &vertices);
  _bitmap gbm;

  ttf.getGlyphBitmapBoxSubpixel(glyph, scale_x, scale_y, shift_x, shift_y, &ix0, &iy0, 0, 0);
  gbm.pixels = output;
  gbm.w = out_w;
  gbm.h = out_h;
  gbm.stride = out_stride;

  if (out_w && out_h) {
    Rasterize(&gbm, 0.35f, vertices, num_verts, scale_x, scale_y, shift_x, shift_y, ix0, iy0, 1);
  }
  free(vertices);
}

// ----------------------------------------------------------------------------
retcode loadTrueTypeFontFile(const char* FileName, int idx, int cx, int cy, gfx::FixedSizeFont* pFont)
{
  int cChars = pFont->lastChar - pFont->firstChar;

  void* buf = malloc(1<<20);

  uint8* fontfile = (uint8*)(buf);

  FILE* f = fopen(FileName, "rb");
  fread(fontfile, 1, 1<<20, f);
  fclose(f);

  TrueTypeFont ttf(buf);
  ttf.init();
  
  int ascent, descent, lineGap;
  ttf.getFontVMetrics(&ascent, &descent, &lineGap);

  float fheight = (float)(ascent - descent) / cy;
  float scale = 1.0f / fheight;

  int y = 1 + idx * cChars * cy;

  for (int i = 0; i < cChars; ++i)
  {
    int x0, y0, x1, y1;
    int g = ttf.findGlyphIndex(i + pFont->firstChar);

    ttf.getGlyphBitmapBoxSubpixel(g, scale, scale, 0.0f, 0.0f, &x0, &y0, &x1, &y1);
    int gw = x1-x0;
    int gh = y1-y0;

    y0 += y + cy + scale * descent;
    MakeGlyphBitmapSubpixel(ttf, &pFont->a[y0][x0], gw, gh, cx, scale, scale, 0.0f, 0.0f, g);
    
    y += cy;
  }
  
  free(buf);
  return rcSuccess;
}

// ----------------------------------------------------------------------------
retcode gfx::loadTrueTypeFont(const char* FileNames[4], int width, int height, gfx::FixedSizeFont** ppFont)
{
  retcode rv;

  int FirstChar = 32;
  int LastChar = 256;
  int cChars = LastChar - FirstChar;

  int cx = width;
  int cy = height;
  gfx::FixedSizeFont* pFont = new gfx::FixedSizeFont(cx, cy, FirstChar, LastChar);

  memset(pFont->aPels, 0, 4*cx*cy*cChars); // background of 0 around pixels
  
  for (int i = 0; i < 4; i++)
  {
    if (failed(rv = loadTrueTypeFontFile(FileNames[i], i, cx, cy, pFont))) {
      return rv;
    }
  }
  
  *ppFont = pFont;
  return rcSuccess;
}
