#include <Source/Headers.h>
#include "UTF8Tools.h"

// ----------------------------------------------------------------------------
size_t UTF8ToInt(const char* p, size_t count, int* pVal)
{
  size_t c = 0;
  uint Val = 0;
  uint Sign = 0;

  if (count > 0 && p[c] == '-') {
    Sign = ~0u;
    c++;
  }

  while (c < count && isDigit(p[c])) {
    Val = 10*Val + (p[c++] - '0');
  }

  *pVal = static_cast<int>((Val ^ Sign) - Sign);
  return c;
}

// ----------------------------------------------------------------------------
size_t UTF8ToInt(const char* p, size_t count, uint* pVal)
{
  size_t c = 0;
  uint Val = 0;

  while (c < count && isDigit(p[c])) {
    Val = 10*Val + (p[c++] - '0');
  }

  *pVal = Val;
  return c;
}
