/*!
** \file    /Source/BaseTypes.h
** \brief   Handy, tiny basic type definitions
******************************************************************************/
#pragma once

#include <wchar.h>

// ==[ Microsoft C++]==========================================================
#if defined(_MSC_VER)

#include <stdint.h>
#include <atomic>

typedef wchar_t wchar;

#endif // _MSC_VER

// ==[ GCC ]===================================================================
#if defined(__GNUC__)
#include <inttypes.h>

typedef wchar_t wchar;

#endif // __GNUC__

typedef unsigned int   uint;
typedef unsigned long  ulong;

typedef int8_t          int8;
typedef int16_t         int16;
typedef int32_t         int32;
typedef int64_t         int64;

typedef uint8_t         uint8;
typedef uint16_t        uint16;
typedef uint32_t        uint32;
typedef uint64_t        uint64;

typedef        void *   handle;

// to make production builds happy
#define unused(x) ((void)(x))

// ============================================================================
template <bool> struct CompileTimeChecker;
template <>     struct CompileTimeChecker<true> {};
#define CompileTimeCheck(x, msg) { CompileTimeChecker<(x)> msg; (void)msg; }

// ============================================================================
// returns the count of the element of an fixed size array
template<typename T, size_t size> size_t countof(T(&)[size])
{
  return size;
}

// ============================================================================
struct size
{
  uint cx;
  uint cy;

  size();
  size(uint x, uint y);
};

// ============================================================================
struct rectangle
{
  uint x0, y0;
  uint x1, y1;

  rectangle();
  rectangle(uint x0, uint y0, uint x1, uint y1);
};

// ============================================================================
inline size::size()
: cx(0)
, cy(0)
{}

// ----------------------------------------------------------------------------
inline size::size(uint x, uint y)
: cx(x)
, cy(y)
{}

// ============================================================================
inline rectangle::rectangle()
: x0(0)
, y0(0)
, x1(0)
, y1(0)
{}

// ----------------------------------------------------------------------------
inline rectangle::rectangle(uint x0, uint y0, uint x1, uint y1)
: x0(x0)
, y0(y0)
, x1(x1)
, y1(y1)
{}

//! retcode = 0 means success, < 0 means error, > 0 means success but under
//! special conditions.
typedef int retcode;

// ============================================================================
enum ReturnCodeSets
{
  rcsetGenericSuccessCodes   = 0x00000000,
  rcsetGenericErrorCodes     = 0x80000000,
};

// ============================================================================
enum GenericReturnCode
{
  rcSuccess                  = rcsetGenericSuccessCodes | 0x0000,
  rcFailed                   = rcsetGenericErrorCodes   | 0x0000,
  rcNotFound                 = rcsetGenericErrorCodes   | 0x0001,
  rcInvalidFormat            = rcsetGenericErrorCodes   | 0x0002,
  rcFileNotFound             = rcsetGenericErrorCodes   | 0x0003,
  rcOutOfRange               = rcsetGenericErrorCodes   | 0x0004,
  rcInvalidPointer           = rcsetGenericErrorCodes   | 0x0005,
};

// ----------------------------------------------------------------------------
inline bool failed(retcode rc)
{
  return (rc < 0);
}
