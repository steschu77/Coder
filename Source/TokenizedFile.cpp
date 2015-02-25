#include <Source/Headers.h>
#include <Source/TextDoc.h>

#include "TokenizedFile.h"

// ============================================================================
void updateTextDoc(TokenizedFile& file, const TextDoc& doc)
{
  size_t cLines = doc.getLineCount();
  file.initialStates.resize(cLines);

  Tokenizer::state_t last  = Tokenizer::sWhiteSpace;
  for (size_t i = 0; i < cLines; i++)
  {
    const std::string& line = doc.getLineAt(i);
    file.initialStates[i] = last;

    Tokenizer tk(line, last);

    Tokenizer::state_t state;
    Tokenizer::token_t token;
    while ((state = tk.nextToken(&token)) < Tokenizer::sError0) {
      last = state;
    }
  }
}

// ============================================================================
class CPPParser
{
public:
  enum state_t
  {
    sDeclarationSequence,
    sDeclaration,
    sSimpleDeclaration,
    sNamespace,
    sStructDecl,
    sClassDecl,
    sDeclarationType,
    sDeclarationId,
    sVariableDecl,
    sFunctionDecl,
    sFunctionDef,
    sConstructorDef,
    sMax
  };

  CPPParser();

  void parseToken(Tokenizer::token_t& token);

  state_t _State;
  std::string _Namespace;

  typedef state_t (CPPParser::*FnState)(Tokenizer::token_t& token);
  FnState _StateFn[sMax];

  state_t _sDeclarationSequence(Tokenizer::token_t& token);
  state_t _sDeclarationType(Tokenizer::token_t& token);
  state_t _sDeclarationId(Tokenizer::token_t& token);
  state_t _sSimpleDeclaration(Tokenizer::token_t& token);
};

// ----------------------------------------------------------------------------
CPPParser::CPPParser()
: _State(sDeclarationSequence)
{
  _StateFn[sDeclarationSequence] = &CPPParser::_sDeclarationSequence;
  _StateFn[sSimpleDeclaration]  = &CPPParser::_sSimpleDeclaration;
  _StateFn[sDeclarationType]  = &CPPParser::_sDeclarationType;
  _StateFn[sDeclarationId]  = &CPPParser::_sDeclarationId;
}

// ----------------------------------------------------------------------------
void CPPParser::parseToken(Tokenizer::token_t& token)
{
  _State = (this->*_StateFn[_State])(token);
}

// ----------------------------------------------------------------------------
CPPParser::state_t CPPParser::_sDeclarationSequence(Tokenizer::token_t& token)
{
  switch (token.type)
  {
  case Tokenizer::tKeyword:
    if ((token.flags & (Tokenizer::flagStorageClassSpecifier)) != 0) {
      return sDeclarationId;
    }
    if ((token.flags & (Tokenizer::flagTypeSpecifier)) != 0) {
      return sDeclarationType;
    }
    if ((token.flags & (Tokenizer::flagFunctionSpecifier)) != 0) {
      return sDeclarationId;
    }

    switch (token.keyword)
    {
    // decl-specifier
    case cpp_friend:
      break;
    case cpp_typedef:
      break;
    case cpp_constexpr:
      break;

    // type-specifier
    case cpp_enum:
      break;
    case cpp_class:
      break;
    case cpp_struct:
      break;
    case cpp_union:
      break;
    }
    break;

  case Tokenizer::tIdentifier:
    token.flags |= Tokenizer::flagTypeSpecifier;
    return sDeclarationId;

  case Tokenizer::tString:
  case Tokenizer::tNumber:
    token.error = 1;
    break;

  case Tokenizer::tOperator:
    if (token.op != Tokenizer::opSemicolon) {
      token.error = 2;
    }
    break;
  }

  return sDeclarationSequence;
}

// ----------------------------------------------------------------------------
CPPParser::state_t CPPParser::_sSimpleDeclaration(Tokenizer::token_t& token)
{
  switch (token.type)
  {
  case Tokenizer::tKeyword:
    if ((token.flags & (Tokenizer::flagTypeSpecifier)) != 0) {
      return sDeclarationType;
    }
  }

  return sDeclarationSequence;
}

// ----------------------------------------------------------------------------
CPPParser::state_t CPPParser::_sDeclarationType(Tokenizer::token_t& token)
{
  switch (token.type)
  {
  case Tokenizer::tKeyword:
    if ((token.flags & (Tokenizer::flagTypeSpecifier)) != 0) {
      return sDeclarationType;
    } else if ((token.flags & (Tokenizer::flagTypeSpecifier)) != 0) {
      return sDeclarationId;
    } else {
      return sDeclarationSequence;
    }

  case Tokenizer::tIdentifier:
    token.flags |= Tokenizer::flagTypeSpecifier;
    return sDeclarationId;

  default:
    return sDeclarationSequence;
  }
}

// ----------------------------------------------------------------------------
CPPParser::state_t CPPParser::_sDeclarationId(Tokenizer::token_t& token)
{
  switch (token.type)
  {
  }
  return sDeclarationSequence;
}

// ============================================================================
const char* gstrTokenTypes[] =
{
  " number",
  " string",
  "     id",
  "     op",
  "comment",
  "preproc",
  "keyword",
  "type-id",
};

// ============================================================================
const char* gstrOperators[] =
{
  "Assign",
  "Plus",
  "PlusPlus",
  "PlusAssign",
  "Minus",
  "MinusMinus",
  "MinusAssign",
  "Star",
  "StarAssign",
  "Slash",
  "SlashAssign",
  "Modulo",
  "ModuloAssign",
  "Equal",
  "Unequal",
  "LT",
  "LTorEqual",
  "GT",
  "GTorEqual",
  "RefOf",
  "Move",
  "DeRef",
  "Ref",
  "Komma",
  "Colon",
  "Semicolon",
  "QMark",
  "Not",
  "NotAssign",
  "Xor",
  "XorAssign",
  "Or",
  "OrAssign",
  "And",
  "AndAssign",
  "BoolOr",
  "BoolAnd",
  "Neg",
  "BraceOpen",
  "BraceClose",
  "BracketOpen",
  "BracketClose",
  "ParenthesisOpen",
  "ParenthesisClose",
};

// ============================================================================
void parseTextDoc(const TextDoc& doc)
{
  CPPParser parser;
  std::vector<Tokenizer::token_t> Tokens;
  Tokenizer::state_t last  = Tokenizer::sWhiteSpace;

  size_t cLines = doc.getLineCount();

  for (size_t i = 0; i < cLines; i++)
  {
    const std::string& line = doc.getLineAt(i);
    Tokenizer tk(line, last);

    Tokenizer::state_t state;
    Tokenizer::token_t token;
    while ((state = tk.nextToken(&token)) < Tokenizer::sError0)
    {
      parser.parseToken(token);

      token.line = i;
      Tokens.push_back(token);

      last = state;
    }
  }

  FILE* f = fopen("c:\\tokens.txt", "w");
  for (auto token : Tokens)
  {
    fprintf(f, "[%5d,%2d-%2d] %s, ", token.line, token.p0, token.p1, gstrTokenTypes[token.type]);

    switch (token.type)
    {
    case Tokenizer::tKeyword:
      fprintf(f, "%s", gstrKeywords[token.keyword]);
      break;

    case Tokenizer::tOperator:
      fprintf(f, "%s", gstrOperators[token.op]);
      break;

    case Tokenizer::tIdentifier:
      fprintf(f, "%s", token.id.c_str());
      break;
    }

    if (token.flags & Tokenizer::flagTypeSpecifier) {
      fprintf(f, ", %s", "type-spec");
    }

    if (token.flags & Tokenizer::flagStorageClassSpecifier) {
      fprintf(f, ", %s", "storage-spec");
    }

    if (token.flags & Tokenizer::flagFunctionSpecifier) {
      fprintf(f, ", %s", "function-spec");
    }

    fprintf(f, "\n");
  }

  fclose(f);
}
