/*!
** \file    Source/Tokenizer.h
** \brief   Simple C++ document tokenizer
\*****************************************************************************/
#pragma once

#include <Source/CPPKeywords.h>

class Tokenizer
{
public:
  enum state_t
  {
    sWhiteSpace,
    sIdentifier,
    sNumber,
    sNumberU,
    sNumberL,
    sNumberLL,
    sHexNumber,
    sOctNumber,
    sFloatNumber,
    sString,
    sChar,
    sPreProc,
    sPrePlus,
    sPreMinus,
    sPreSlash,
    sPreMul,
    sPreModulo,
    sPreLT,
    sPreGT,
    sPreOr,
    sPreAnd,
    sPreNot,
    sPreXor,
    sPreEqual,
    sPreNull,
    sCommentA,
    sCommentB,
    sCommentB1,
    sError0,
    sError1,
    sMax
  };

  enum operator_t
  {
    opAssign,
    opPlus,
    opPlusPlus,
    opPlusAssign,
    opMinus,
    opMinusMinus,
    opMinusAssign,
    opStar,
    opStarAssign,
    opSlash,
    opSlashAssign,
    opModulo,
    opModuloAssign,
    opEqual,
    opUnequal,
    opLT,
    opLTorEqual,
    opGT,
    opGTorEqual,
    opRefOf,
    opMove,
    opDeRef,
    opRef,
    opKomma,
    opColon,
    opSemicolon,
    opQMark,
    opNot,
    opNotAssign,
    opXor,
    opXorAssign,
    opOr,
    opOrAssign,
    opAnd,
    opAndAssign,
    opBoolOr,
    opBoolAnd,
    opNeg,
    opBraceOpen,
    opBraceClose,
    opBracketOpen,
    opBracketClose,
    opParenthesisOpen,
    opParenthesisClose,
  };
  
  enum token_type_t
  {
    tNumber,
    tString,
    tIdentifier,
    tOperator,
    tComment,
    tPreProcessor,
    tKeyword,
  };

  enum token_flags_t
  {
    flagTypeSpecifier         = 1 << 0,
    flagStorageClassSpecifier = 1 << 1,
    flagFunctionSpecifier     = 1 << 2,
  };
  
  struct token_t
  {
    size_t line;
    size_t p0, p1, px;
    token_type_t type;
    std::string id;
    operator_t op;
    Keywords keyword;
    int error;
    unsigned flags;
  };

  struct char_t
  {
    char_t(size_t pos=0, int chr=0) : pos(pos), chr(chr) {}

    size_t pos;
    int chr;
  };
  
  Tokenizer(const std::string& line, state_t initialState=sWhiteSpace);

  bool nextChar(char_t* pChr);
  state_t nextToken(token_t* pToken);

  std::string _line;

  state_t _State;
  token_t _CurrentToken;

  std::deque<char_t> _Stack;
  
  typedef state_t (Tokenizer::*FnState)(const char_t& chr);
  FnState _StateFn[sMax];

  state_t _sWhiteSpace(const char_t& chr);
  state_t _sIdentifier(const char_t& chr);
  state_t _sNumber(const char_t& chr);
  state_t _sNumberU(const char_t& chr);
  state_t _sNumberL(const char_t& chr);
  state_t _sNumberLL(const char_t& chr);
  state_t _sHexNumber(const char_t& chr);
  state_t _sOctNumber(const char_t& chr);
  state_t _sFloatNumber(const char_t& chr);
  state_t _sString(const char_t& chr);
  state_t _sChar(const char_t& chr);
  state_t _sPreProc(const char_t& chr);
  state_t _sPrePlus(const char_t& chr);
  state_t _sPreMinus(const char_t& chr);
  state_t _sPreSlash(const char_t& chr);
  state_t _sPreModulo(const char_t& chr);
  state_t _sPreMul(const char_t& chr);
  state_t _sPreLT(const char_t& chr);
  state_t _sPreGT(const char_t& chr);
  state_t _sPreOr(const char_t& chr);
  state_t _sPreAnd(const char_t& chr);
  state_t _sPreNot(const char_t& chr);
  state_t _sPreXor(const char_t& chr);
  state_t _sPreEqual(const char_t& chr);
  state_t _sPreNull(const char_t& chr);
  state_t _sCommentA(const char_t& chr);
  state_t _sCommentB(const char_t& chr);
  state_t _sCommentB1(const char_t& chr);
  state_t _sError0(const char_t& chr);
  state_t _sError1(const char_t& chr);

  state_t _NonNumberChar(const char_t& chr);

  void push(const char_t& chr);
  void x(token_type_t Token);
  void xOp(operator_t op);
  void z0(size_t pos);
  void z1();
  void z1(size_t pos);
  void zx(size_t pos);
};
