/*!
** \file    /Include/ToolBox.h
** \brief   Generic tools
******************************************************************************/
#pragma once

// ----------------------------------------------------------------------------
template <typename T>
inline T min(T x0, T x1, T x2)
{
  return std::min(std::min(x0, x1), x2);
}

// ----------------------------------------------------------------------------
template <typename T>
inline T max(T x0, T x1, T x2)
{
  return std::max(std::max(x0, x1), x2);
}

// ----------------------------------------------------------------------------
template <typename T>
inline T clip(T x, T x0, T x1)
{
  return std::min<T>(std::max<T>(x, x0), x1);
}

// ============================================================================
template <typename T>
inline bool inRange(const T& x, const T& x0, const T& x1)
{
  return x >= x0 && x <= x1;
}
