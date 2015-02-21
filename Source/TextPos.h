/*!
** \file    Source/TextDoc.h
** \brief   Text position specification and operations.
\*****************************************************************************/
#pragma once

// ============================================================================
struct TextPos
{
  TextPos(size_t line=0, size_t column=0) : line(line), column(column) {}

  size_t line;
  size_t column;
};

// ============================================================================
bool operator < (const TextPos& rhs, const TextPos& lhs);
bool operator == (const TextPos& rhs, const TextPos& lhs);
bool operator != (const TextPos& rhs, const TextPos& lhs);

// ----------------------------------------------------------------------------
inline bool operator < (const TextPos& rhs, const TextPos& lhs)
{
  if (rhs.line != lhs.line) return rhs.line < lhs.line;
  return rhs.column < lhs.column;
}

// ----------------------------------------------------------------------------
inline bool operator == (const TextPos& rhs, const TextPos& lhs)
{
  return rhs.line == lhs.line
    || rhs.column == lhs.column;
}

// ----------------------------------------------------------------------------
inline bool operator != (const TextPos& rhs, const TextPos& lhs)
{
  return rhs.line != lhs.line
    || rhs.column != lhs.column;
}
