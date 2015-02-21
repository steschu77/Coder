/*!
** \file    /Include/System.h
** \brief   Basic system tools.
******************************************************************************/
#pragma once

#include <Source/BaseTypes.h>

// ============================================================================
// Timer
// ============================================================================
const uint Infinite = 0xffffffff;

// ============================================================================
// File IO
// ============================================================================

// ----------------------------------------------------------------------------
enum FileMode
{
  fileOpenExisting,
  fileCreateNew,
  fileCreateAlways,
};

// ----------------------------------------------------------------------------
enum MoveMode
{
  moveBegin,
  moveCurrent,
  moveEnd,
};

// ============================================================================
// Include platform specific parts
#if defined(_MSC_VER)
# include "Win32Lib.h"
#endif

#if defined(__GNUC__)
# include "LinuxLib.h"
#endif
