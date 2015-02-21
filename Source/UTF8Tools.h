/*!
** \file    /Source/UTF8Tools.h
** \brief   Contains UTF-8 char-set support functions.
******************************************************************************/
#pragma once

// ----------------------------------------------------------------------------
enum UTF8CharType
{
  utf8Control = (1<<0),
  utf8Space   = (1<<1),
  utf8Blank   = (1<<2),
  utf8Punct   = (1<<3),
  utf8Digit   = (1<<4),
  utf8Hex     = (1<<5),
  utf8Upper   = (1<<6),
  utf8Lower   = (1<<7),
};

// ----------------------------------------------------------------------------
extern uint8_t gCharTypeData[128];

// ----------------------------------------------------------------------------
inline bool isControl(char c)
{
  uint u = static_cast<uint>(c);
  return (u < 128) && (gCharTypeData[u] & utf8Control) != 0;
}

// ----------------------------------------------------------------------------
inline bool isSpace(uint u)
{
  return (u < 128) && (gCharTypeData[u] & utf8Space) != 0;
}

// ----------------------------------------------------------------------------
inline bool isSpace(int c)
{
  uint u = static_cast<uint>(c);
  return isSpace(u);
}

// ----------------------------------------------------------------------------
inline bool isPunct(char c)
{
  uint u = static_cast<uint>(c);
  return (u < 128) && (gCharTypeData[u] & utf8Punct) != 0;
}

// ----------------------------------------------------------------------------
inline bool isDigit(uint u)
{
  return (u < 128) && (gCharTypeData[u] & utf8Digit) != 0;
}

// ----------------------------------------------------------------------------
inline bool isDigit(int c)
{
  uint u = static_cast<uint>(c);
  return isDigit(u);
}

// ----------------------------------------------------------------------------
inline bool isXDigit(uint u)
{
  return (u < 128) && (gCharTypeData[u] & utf8Hex) != 0;
}

// ----------------------------------------------------------------------------
inline bool isXDigit(int c)
{
  uint u = static_cast<uint>(c);
  return isXDigit(u);
}

// ----------------------------------------------------------------------------
inline bool isAlpha(uint u)
{
  return (u < 128) && (gCharTypeData[u] & (utf8Lower|utf8Upper)) != 0;
}

// ----------------------------------------------------------------------------
inline bool isAlpha(int c)
{
  uint u = static_cast<uint>(c);
  return isAlpha(u);
}

// ----------------------------------------------------------------------------
inline bool isLowerCase(char c)
{
  uint u = static_cast<uint>(c);
  return (u < 128) && (gCharTypeData[u] & utf8Lower) != 0;
}

// ----------------------------------------------------------------------------
inline bool isUpperCase(char c)
{
  uint u = static_cast<uint>(c);
  return (u < 128) && (gCharTypeData[u] & utf8Upper) != 0;
}

// ----------------------------------------------------------------------------
inline char toUpperCase(char c)
{
  return isLowerCase(c) ? c - 32 : c;
}

size_t UTF8ToInt(const char* p, size_t count, int* pVal);
size_t UTF8ToInt(const char* p, size_t count, uint* pVal);

size_t UTF8ToUTF16(wchar *pwc, size_t cwc, const char *pc, size_t cmb, size_t *pcmb);
size_t UTF8ToUTF16(wchar *pwc, size_t cwc, const char *pc);

void UTF16ToUTF8(char** ppmb, size_t* pcmb, const wchar** ppwc, size_t* pcwc);
