#include <Source/Headers.h>
#include "UTF8Tools.h"

typedef unsigned char uchar;

// ----------------------------------------------------------------------------
// Specification: RFC 2279
size_t UTF8ToUTF16(wchar *pwc, size_t cwc, const char *pc, size_t cmb, size_t *pcmb)
{
  const uchar *pmb = reinterpret_cast<const uchar*>(pc);
  size_t cmb0 = cmb;
  size_t cwc0 = cwc;

  while (cwc > 0 && cmb > 0)
  {
    if (pmb[0] < 0x80)
    {
      *pwc = pmb[0];
      pmb += 1;
      cmb -= 1;
    } else
    if (pmb[0] < 0xe0)
    {
      if (cmb < 2) break;
      if ((pmb[1] ^ 0x80) >= 0x40)
        *pwc = 0xfffc;
      else
        *pwc = ((wchar)(pmb[0] & 0x1f) << 6) | (wchar)(pmb[1] ^ 0x80);
      pmb += 2;
      cmb -= 2;
    }
    else if (pmb[0] < 0xf0)
    {
      if (cmb < 3) break;
      if  ((pmb[1] ^ 0x80) >= 0x40 || (pmb[2] ^ 0x80) >= 0x40)
        *pwc = 0xfffc;
      else
        *pwc = ((wchar)(pmb[0] & 0x0f) << 12) | ((wchar)(pmb[1] ^ 0x80) << 6) | (wchar)(pmb[2] ^ 0x80);
      pmb += 3;
      cmb -= 3;
    }
    else if (pmb[0] < 0xf8)
    {
      if (cmb < 4) break;
      *pwc = 0xfffc;
      pmb += 4;
      cmb -= 4;
    } else if (pmb[0] < 0xfc) {
      if (cmb < 5) break;
      *pwc = 0xfffc;
      pmb += 5;
      cmb -= 5;
    } else if (pmb[0] < 0xfe) {
      if (cmb < 6) break;
      *pwc = 0xfffc;
      pmb += 6;
      cmb -= 6;
    } else {
      *pwc = 0xfffc;
      pmb += 1;
      cmb -= 1;
    }

    pwc += 1;
    cwc -= 1;
  }

  if (pcmb != nullptr) *pcmb = cmb0 - cmb;
  return cwc0 - cwc;
}

// ----------------------------------------------------------------------------
// Specification: RFC 2279
size_t UTF8ToUTF16(wchar *pwc, size_t cwc, const char *pc)
{
  size_t cwc0 = cwc;
  const uchar *pmb = reinterpret_cast<const uchar*>(pc);

  while (cwc > 0)
  {
    if (pmb[0] == 0) {
      break;
    }

    if (pmb[0] < 0x80)
    {
      *pwc = pmb[0];
      pmb += 1;
    } else if (pmb[0] < 0xe0)
    {
      if (pmb[1] == 0) {
        break;
      }

      if ((pmb[1] ^ 0x80) >= 0x40)
        *pwc = 0xfffc;
      else
        *pwc = ((wchar)(pmb[0] & 0x1f) << 6) | (wchar)(pmb[1] ^ 0x80);

      pmb += 2;
    }
    else if (pmb[0] < 0xf0)
    {
      if (pmb[1] == 0 || pmb[2] == 0) {
        break;
      }

      if  ((pmb[1] ^ 0x80) >= 0x40 || (pmb[2] ^ 0x80) >= 0x40)
        *pwc = 0xfffc;
      else
        *pwc = ((wchar)(pmb[0] & 0x0f) << 12) | ((wchar)(pmb[1] ^ 0x80) << 6) | (wchar)(pmb[2] ^ 0x80);

      pmb += 3;
    }
    else if (pmb[0] < 0xf8)
    {
      if (pmb[1] == 0 || pmb[2] == 0 || pmb[3] == 0) {
        break;
      }

      *pwc = 0xfffc;
      pmb += 4;
    } else if (pmb[0] < 0xfc)
    {
      if (pmb[1] == 0 || pmb[2] == 0 || pmb[3] == 0 || pmb[4] == 0) {
        break;
      }

      *pwc = 0xfffc;
      pmb += 5;
    } else if (pmb[0] < 0xfe)
    {
      if (pmb[1] == 0 || pmb[2] == 0 || pmb[3] == 0 || pmb[4] == 0 || pmb[5] == 0) {
        break;
      }

      *pwc = 0xfffc;
      pmb += 6;
    } else {
      *pwc = 0xfffc;
      pmb += 1;
    }

    pwc += 1;
    cwc -= 1;
  }

  if (cwc > 0) {
    pwc[0] = '\0';
  }

  return cwc0 - cwc;
}

// ----------------------------------------------------------------------------
void UTF16ToUTF8(char** ppmb, size_t* pcmb, const wchar** ppwc, size_t* pcwc)
{
  char*   pmb = *ppmb;
  size_t  cmb = *pcmb;
  const wchar* pwc = *ppwc;
  size_t  cwc = *pcwc;

  while (cwc > 0 && cmb > 0)
  {
    uint wc = *pwc;
    uint j = 1;

    if (wc >= 0x800)
    {
      if (cmb <= 2) break;
      pmb[2] = (uint8)(0x80 | (wc & 0x3f));
      wc = (wc >> 6) | 0x800;
      j++;
    }
    if (wc >= 0x80)
    {
      if (cmb <= 1) break;
      pmb[1] = (uint8)(0x80 | (wc & 0x3f));
      wc = (wc >> 6) | 0xc0;
      j++;
    }
    pmb[0] = (uint8)wc;

    pmb += j;
    cmb -= j;

    pwc += 1;
    cwc -= 1;
  }

  *ppmb = pmb;
  *pcmb = cmb;
  *ppwc = pwc;
  *pcwc = cwc;
}
