/*!
** \file    Source/FixedSizeFont.h
** \brief   
\*****************************************************************************/
#pragma once

namespace gfx {

// ============================================================================
struct FixedSizeFont
{
  FixedSizeFont();
  FixedSizeFont(int cx, int cy, int firstChar, int lastChar);

  ~FixedSizeFont();
  
  FixedSizeFont(const FixedSizeFont&) = delete;
  FixedSizeFont operator = (FixedSizeFont) = delete;

  int cx, cy;
  int firstChar;
  int lastChar;
  
  uint8** a;
  uint8* aPels;
};

retcode loadTrueTypeFont(int cx, int cy, FixedSizeFont** pFont);

} // namespace gfx
