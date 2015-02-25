#include <Source/Headers.h>
#include <Source/Canvas.h>
#include <Source/UTF8Tools.h>
#include <Source/Config.h>

#include "Tokenizer.h"

// ----------------------------------------------------------------------------
static int compareStrings(const void* p0, const void* p1)
{
  const char** s0 = (const char**)p0;
  const char** s1 = (const char**)p1;
  
  return strcmp(*s0, *s1);
}

// ----------------------------------------------------------------------------
static int compareKeywords(const void* p0, const void* p1)
{
  const Keywords* k0 = (const Keywords*)p0;
  const Keywords* k1 = (const Keywords*)p1;
  
  return *k0 - *k1;
}

// ----------------------------------------------------------------------------
size_t find(const char* s)
{
  const char** pBase = &gstrKeywords[0];
  const char** p = static_cast<const char**>(std::bsearch(&s, pBase, cpp_cKeywords, sizeof(const char*), compareStrings));
  return (p != nullptr) ? p - pBase : cpp_cKeywords;
}

// ----------------------------------------------------------------------------
bool find(Keywords key, const Keywords* base, size_t count)
{
  const Keywords* p = static_cast<const Keywords*>(std::bsearch(&key, base, count, sizeof(Keywords*), compareKeywords));
  return (p != nullptr);
}

// ----------------------------------------------------------------------------
Tokenizer::Tokenizer(const std::string& line, state_t initialState)
: _line(line)
, _State(initialState)
{
  _StateFn[sWhiteSpace]  = &Tokenizer::_sWhiteSpace;
  _StateFn[sIdentifier]  = &Tokenizer::_sIdentifier;
  _StateFn[sNumber]      = &Tokenizer::_sNumber;
  _StateFn[sNumberU]     = &Tokenizer::_sNumberU;
  _StateFn[sNumberL]     = &Tokenizer::_sNumberL;
  _StateFn[sNumberLL]    = &Tokenizer::_sNumberLL;
  _StateFn[sHexNumber]   = &Tokenizer::_sHexNumber;
  _StateFn[sOctNumber]   = &Tokenizer::_sOctNumber;
  _StateFn[sFloatNumber] = &Tokenizer::_sFloatNumber;
  _StateFn[sString]      = &Tokenizer::_sString;
  _StateFn[sChar]        = &Tokenizer::_sChar;
  _StateFn[sPreProc]     = &Tokenizer::_sPreProc;
  _StateFn[sPrePlus]     = &Tokenizer::_sPrePlus;
  _StateFn[sPreMinus]    = &Tokenizer::_sPreMinus;
  _StateFn[sPreSlash]    = &Tokenizer::_sPreSlash;
  _StateFn[sPreMul]      = &Tokenizer::_sPreMul;
  _StateFn[sPreModulo]   = &Tokenizer::_sPreModulo;
  _StateFn[sPreLT]       = &Tokenizer::_sPreLT;
  _StateFn[sPreGT]       = &Tokenizer::_sPreGT;
  _StateFn[sPreOr]       = &Tokenizer::_sPreOr;
  _StateFn[sPreAnd]      = &Tokenizer::_sPreAnd;
  _StateFn[sPreNot]      = &Tokenizer::_sPreNot;
  _StateFn[sPreXor]      = &Tokenizer::_sPreXor;
  _StateFn[sPreEqual]    = &Tokenizer::_sPreEqual;
  _StateFn[sPreNull]     = &Tokenizer::_sPreNull;
  _StateFn[sCommentA]    = &Tokenizer::_sCommentA;
  _StateFn[sCommentB]    = &Tokenizer::_sCommentB;
  _StateFn[sCommentB1]   = &Tokenizer::_sCommentB1;
  _StateFn[sError0]      = &Tokenizer::_sError0;
  _StateFn[sError1]      = &Tokenizer::_sError1;

  size_t pos = 0;

  for (char chr : line) {
    _Stack.push_back(char_t(pos++, chr));
  }

  _Stack.push_back(char_t(pos++, '\n'));
}

// ----------------------------------------------------------------------------
bool Tokenizer::nextChar(char_t* pChr)
{
  if (!_Stack.empty())
  {
    *pChr = _Stack.front();
    _Stack.pop_front();
    return true;
  }
  else {
    return false;
  }
}

// ----------------------------------------------------------------------------
Tokenizer::state_t Tokenizer::nextToken(Tokenizer::token_t* pToken)
{
  _CurrentToken.p0 = std::string::npos;
  _CurrentToken.p1 = std::string::npos;
  _CurrentToken.px = std::string::npos;
  _CurrentToken.id.clear();
  _CurrentToken.flags = 0;
  
  char_t chr;

  while (nextChar(&chr))
  {
    if ((_State = (this->*_StateFn[_State])(chr)) == sMax) {
      return sMax;
    }

    token_t& t = _CurrentToken;
    if (t.p1 != std::string::npos)
    {
      if (t.type == Tokenizer::tIdentifier)
      {
        t.id = _line.substr(t.p0, t.p1-t.p0+1);
        t.keyword = static_cast<Keywords>(find(t.id.c_str()));

        if (t.keyword < cpp_cKeywords)
        {
          t.type = Tokenizer::tKeyword;

          if (find(t.keyword, gSimpleTypeSpecifier, gcSimpleTypeSpecifier)) {
            t.flags |= flagTypeSpecifier;
          }
          if (find(t.keyword, gStorageClassSpecifier, gcStorageClassSpecifier)) {
            t.flags |= flagStorageClassSpecifier;
          }
          if (find(t.keyword, gFunctionSpecifier, gcFunctionSpecifier)) {
            t.flags |= flagFunctionSpecifier;
          }
        }
      }

      *pToken = _CurrentToken;
      return _State;
    }
  }
  
  return sMax;
}

// ----------------------------------------------------------------------------
Tokenizer::state_t Tokenizer::_sWhiteSpace(const Tokenizer::char_t& chr)
{
  if (isSpace(chr.chr)) {
    return sWhiteSpace;
  }

  if (isAlpha(chr.chr) || chr.chr == '_') {
    x(tIdentifier);
    z0(chr.pos);
    return sIdentifier;
  }

  switch (chr.chr)
  {
  case '#':  z0(chr.pos); x(tPreProcessor); return sPreProc;

  case '+':  z0(chr.pos); return sPrePlus;
  case '-':  z0(chr.pos); return sPreMinus;
  case '/':  z0(chr.pos); return sPreSlash;
  case '*':  z0(chr.pos); return sPreMul;
  case '%':  z0(chr.pos); return sPreModulo;
  case '<':  z0(chr.pos); return sPreLT;
  case '>':  z0(chr.pos); return sPreGT;
  case '|':  z0(chr.pos); return sPreOr;
  case '&':  z0(chr.pos); return sPreAnd;
  case '^':  z0(chr.pos); return sPreXor;
  case '!':  z0(chr.pos); return sPreNot;
  case '=':  z0(chr.pos); return sPreEqual;
  case '0':  z0(chr.pos); x(tNumber); return sPreNull;
  case '1': case '2': case '3':
  case '4': case '5': case '6':
  case '7': case '8': case '9':
             z0(chr.pos); x(tNumber); return sNumber;
  case '~':  z0(chr.pos); xOp(opNeg); z1(); return sWhiteSpace;
  case '{':  z0(chr.pos); xOp(opBraceOpen); z1(); return sWhiteSpace;
  case '}':  z0(chr.pos); xOp(opBraceClose); z1(); return sWhiteSpace;
  case '[':  z0(chr.pos); xOp(opBracketOpen); z1(); return sWhiteSpace;
  case ']':  z0(chr.pos); xOp(opBracketClose); z1(); return sWhiteSpace;
  case '(':  z0(chr.pos); xOp(opParenthesisOpen); z1(); return sWhiteSpace;
  case ')':  z0(chr.pos); xOp(opParenthesisClose); z1(); return sWhiteSpace;
  case ',':  z0(chr.pos); xOp(opKomma); z1(); return sWhiteSpace;
  case ';':  z0(chr.pos); xOp(opSemicolon); z1(); return sWhiteSpace;
  case '?':  z0(chr.pos); xOp(opQMark); z1(); return sWhiteSpace;
  case ':':  z0(chr.pos); xOp(opColon); z1(); return sWhiteSpace;
  case '"':  z0(chr.pos); x(tString); return sString;
  case '\'': z0(chr.pos); x(tNumber); return sChar;

  case '\\': z0(chr.pos); x(tOperator); return sError1;

  default: return sWhiteSpace;
  }
}

// ----------------------------------------------------------------------------
Tokenizer::state_t Tokenizer::_NonNumberChar(const char_t& chr)
{
  if (isDigit(chr.chr) || isAlpha(chr.chr) || chr.chr == '_') {
    zx(chr.pos);
    return sError0;
  }
  else {
    z1(chr.pos-1);
    push(chr);
    return sWhiteSpace;
  }
}

// ----------------------------------------------------------------------------
Tokenizer::state_t Tokenizer::_sNumber(const char_t& chr)
{
  if (isDigit(chr.chr)) {
    return sNumber;
  }

  switch (chr.chr)
  {
  case '.': x(tNumber); return sFloatNumber;
  case 'e': x(tNumber); return sFloatNumber;
  case 'E': x(tNumber); return sFloatNumber;
  case 'l': return sNumberL;
  case 'L': return sNumberL;
  case 'u': return sNumberU;
  case 'U': return sNumberU;

  default:
    return _NonNumberChar(chr);
  }
}

// ----------------------------------------------------------------------------
Tokenizer::state_t Tokenizer::_sNumberU(const char_t& chr)
{
  switch (chr.chr)
  {
  case 'l': x(tNumber); return sNumberL;
  case 'L': x(tNumber); return sNumberL;

  default:
    return _NonNumberChar(chr);
  }
}

// ----------------------------------------------------------------------------
Tokenizer::state_t Tokenizer::_sNumberL(const char_t& chr)
{
  switch (chr.chr)
  {
  case 'l': x(tNumber); return sNumberLL;
  case 'L': x(tNumber); return sNumberLL;

  default:
    return _NonNumberChar(chr);
  }
}

// ----------------------------------------------------------------------------
Tokenizer::state_t Tokenizer::_sNumberLL(const char_t& chr)
{
  return _NonNumberChar(chr);
}

// ----------------------------------------------------------------------------
Tokenizer::state_t Tokenizer::_sHexNumber(const char_t& chr)
{
  if (isXDigit(chr.chr)) {
    return sHexNumber;
  }

  switch (chr.chr)
  {
  case 'u': return sNumberU;
  case 'U': return sNumberU;
  case 'l': return sNumberL;
  case 'L': return sNumberL;
  default:
    return _NonNumberChar(chr);
  }
}

// ----------------------------------------------------------------------------
Tokenizer::state_t Tokenizer::_sOctNumber(const char_t& chr)
{
  if (chr.chr >= '0' && chr.chr <= '7') {
    return sOctNumber;
  }

  switch (chr.chr)
  {
  case 'u': return sNumberU;
  case 'U': return sNumberU;
  case 'l': return sNumberL;
  case 'L': return sNumberL;
  default:
    return _NonNumberChar(chr);
  }
}

// ----------------------------------------------------------------------------
Tokenizer::state_t Tokenizer::_sFloatNumber(const char_t& chr)
{
  if (isDigit(chr.chr)) {
    return sFloatNumber;
  }

  switch (chr.chr)
  {
  case '.': return sFloatNumber;
  case 'e': return sFloatNumber;
  case 'E': return sFloatNumber;
  default:
    return _NonNumberChar(chr);
  }
}

// ----------------------------------------------------------------------------
Tokenizer::state_t Tokenizer::_sPreProc(const char_t& chr)
{
  switch (chr.chr)
  {
  case '\n': z1(chr.pos); return sWhiteSpace;
  default:  return sPreProc;
  }
}

// ----------------------------------------------------------------------------
Tokenizer::state_t Tokenizer::_sPrePlus(const char_t& chr)
{
  switch (chr.chr)
  {
  case '+': xOp(opPlusPlus); z1(chr.pos); return sWhiteSpace;
  case '=': xOp(opPlusAssign); z1(chr.pos); return sWhiteSpace;
  default:  xOp(opPlus); z1(chr.pos-1); push(chr); return sWhiteSpace;
  }
}

// ----------------------------------------------------------------------------
Tokenizer::state_t Tokenizer::_sPreMinus(const char_t& chr)
{
  switch (chr.chr)
  {
  case '-': xOp(opMinusMinus); z1(chr.pos); return sWhiteSpace;
  case '=': xOp(opMinusAssign); z1(chr.pos); return sWhiteSpace;
  case '>': xOp(opDeRef); z1(chr.pos); return sWhiteSpace;
  default:  xOp(opMinus); z1(chr.pos-1); push(chr); return sWhiteSpace;
  }
}

// ----------------------------------------------------------------------------
Tokenizer::state_t Tokenizer::_sPreSlash(const char_t& chr)
{
  switch (chr.chr)
  {
  case '/': x(tComment); return sCommentA;
  case '*': x(tComment); return sCommentB;
  case '=': xOp(opSlashAssign); z1(chr.pos); return sWhiteSpace;
  default:  xOp(opSlash); z1(chr.pos-1); push(chr); return sWhiteSpace;
  }
}

// ----------------------------------------------------------------------------
Tokenizer::state_t Tokenizer::_sPreModulo(const char_t& chr)
{
  switch (chr.chr)
  {
  case '=': xOp(opModuloAssign); z1(chr.pos); return sWhiteSpace;
  default:  xOp(opModulo); z1(chr.pos-1); push(chr); return sWhiteSpace;
  }
}

// ----------------------------------------------------------------------------
Tokenizer::state_t Tokenizer::_sPreMul(const char_t& chr)
{
  switch (chr.chr)
  {
  case '=': xOp(opStarAssign); z1(chr.pos); return sWhiteSpace;
  default:  xOp(opStar); z1(chr.pos-1); push(chr); return sWhiteSpace;
  }
}

// ----------------------------------------------------------------------------
Tokenizer::state_t Tokenizer::_sPreLT(const char_t& chr)
{
  switch (chr.chr)
  {
  case '=': xOp(opLTorEqual); z1(chr.pos); return sWhiteSpace;
  default:  xOp(opLT); z1(chr.pos-1); push(chr); return sWhiteSpace;
  }
}

// ----------------------------------------------------------------------------
Tokenizer::state_t Tokenizer::_sPreGT(const char_t& chr)
{
  switch (chr.chr)
  {
  case '=': xOp(opGTorEqual); z1(chr.pos); return sWhiteSpace;
  default:  xOp(opGT); z1(chr.pos-1); push(chr); return sWhiteSpace;
  }
}

// ----------------------------------------------------------------------------
Tokenizer::state_t Tokenizer::_sPreOr(const char_t& chr)
{
  switch (chr.chr)
  {
  case '|': xOp(opBoolOr); z1(chr.pos); return sWhiteSpace;
  case '=': xOp(opOrAssign); z1(chr.pos); return sWhiteSpace;
  default:  xOp(opOr); z1(chr.pos-1); push(chr); return sWhiteSpace;
  }
}

// ----------------------------------------------------------------------------
Tokenizer::state_t Tokenizer::_sPreAnd(const char_t& chr)
{
  switch (chr.chr)
  {
  case '&': xOp(opBoolAnd); z1(chr.pos); return sWhiteSpace;
  case '=': xOp(opAndAssign); z1(chr.pos); return sWhiteSpace;
  default:  xOp(opAnd); z1(chr.pos-1); push(chr); return sWhiteSpace;
  }
}

// ----------------------------------------------------------------------------
Tokenizer::state_t Tokenizer::_sPreXor(const char_t& chr)
{
  switch (chr.chr)
  {
  case '=': xOp(opXorAssign); z1(chr.pos); return sWhiteSpace;
  default:  xOp(opXor); z1(chr.pos-1); push(chr); return sWhiteSpace;
  }
}

// ----------------------------------------------------------------------------
Tokenizer::state_t Tokenizer::_sPreNot(const char_t& chr)
{
  switch (chr.chr)
  {
  case '=': xOp(opUnequal); z1(chr.pos); return sWhiteSpace;
  default:  xOp(opNot); z1(chr.pos-1); push(chr); return sWhiteSpace;
  }
}

// ----------------------------------------------------------------------------
Tokenizer::state_t Tokenizer::_sPreEqual(const char_t& chr)
{
  switch (chr.chr)
  {
  case '=': xOp(opEqual); z1(chr.pos); return sWhiteSpace;
  default:  xOp(opAssign); z1(chr.pos-1); push(chr); return sWhiteSpace;
  }
}

// ----------------------------------------------------------------------------
Tokenizer::state_t Tokenizer::_sPreNull(const char_t& chr)
{
  if (chr.chr >= '0' && chr.chr <= '7') {
    return sOctNumber;
  }

  switch (chr.chr)
  {
  case 'x': return sHexNumber;
  case 'X': return sHexNumber;
  case '.': return sFloatNumber;
  case 'e': return sFloatNumber;
  case 'E': return sFloatNumber;
  case 'u': return sNumberU;
  case 'U': return sNumberU;
  case 'l': return sNumberL;
  case 'L': return sNumberL;
  default:
    return _NonNumberChar(chr);
  }
}

// ----------------------------------------------------------------------------
Tokenizer::state_t Tokenizer::_sString(const char_t& chr)
{
  switch (chr.chr)
  {
  case '"': z1(chr.pos); return sWhiteSpace;
  default:  return sString;
  }
}

// ----------------------------------------------------------------------------
Tokenizer::state_t Tokenizer::_sChar(const char_t& chr)
{
  switch (chr.chr)
  {
  case '\'': z1(chr.pos); return sWhiteSpace;
  default:  return sChar;
  }
}

// ----------------------------------------------------------------------------
Tokenizer::state_t Tokenizer::_sIdentifier(const char_t& chr)
{
  if (isDigit(chr.chr) || isAlpha(chr.chr) || chr.chr == '_') {
    return sIdentifier;
  }

  z1(chr.pos-1);

  push(chr);
  return sWhiteSpace;
}

// ----------------------------------------------------------------------------
Tokenizer::state_t Tokenizer::_sCommentA(const char_t& chr)
{
  switch (chr.chr)
  {
  case '\n': z1(chr.pos); return sWhiteSpace;
  default:  return sCommentA;
  }
}

// ----------------------------------------------------------------------------
Tokenizer::state_t Tokenizer::_sCommentB(const char_t& chr)
{
  if (chr.pos == 0) {
    x(tComment); z0(chr.pos);
  }

  switch (chr.chr)
  {
  case '*': return sCommentB1;
  case '\n': z1(chr.pos); return sCommentB;
  default:  return sCommentB;
  }
}

// ----------------------------------------------------------------------------
Tokenizer::state_t Tokenizer::_sCommentB1(const char_t& chr)
{
  switch (chr.chr)
  {
  case '*': return sCommentB1;
  case '/': z1(chr.pos); return sWhiteSpace;
  case '\n': z1(chr.pos); return sCommentB;
  default:  return sCommentB;
  }
}

// ----------------------------------------------------------------------------
Tokenizer::state_t Tokenizer::_sError0(const char_t& chr)
{
  if (isDigit(chr.chr) || isAlpha(chr.chr) || chr.chr == '_') {
    return sError0;
  }

  z1(chr.pos-1);

  push(chr);
  return sWhiteSpace;
}

// ----------------------------------------------------------------------------
Tokenizer::state_t Tokenizer::_sError1(const char_t& chr)
{
  if (isSpace(chr.chr) || isDigit(chr.chr) || isAlpha(chr.chr) || chr.chr == '_') {
    return sError1;
  }

  z1(chr.pos-1);

  push(chr);
  return sWhiteSpace;
}

// ----------------------------------------------------------------------------
void Tokenizer::push(const char_t& chr)
{
  _Stack.push_front(chr);
}

// ----------------------------------------------------------------------------
void Tokenizer::x(token_type_t Token)
{
  _CurrentToken.type = Token;
}

// ----------------------------------------------------------------------------
void Tokenizer::xOp(operator_t op)
{
  _CurrentToken.type = tOperator;
  _CurrentToken.op = op;
}

// ----------------------------------------------------------------------------
void Tokenizer::z0(size_t pos)
{
  _CurrentToken.p0 = pos;
}

// ----------------------------------------------------------------------------
void Tokenizer::z1()
{
  z1(_CurrentToken.p0);
}

// ----------------------------------------------------------------------------
void Tokenizer::z1(size_t pos)
{
  _CurrentToken.p1 = pos;
}

// ----------------------------------------------------------------------------
void Tokenizer::zx(size_t pos)
{
  _CurrentToken.px = pos;
}
