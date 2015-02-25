/*!
** \file    Source/TokenizedFile.h
** \brief   Structure taking start states for tokenization of text lines
\*****************************************************************************/
#pragma once

#include <Source/Tokenizer.h>

// ============================================================================
struct TokenizedFile
{
  std::vector<Tokenizer::state_t> initialStates;
};

// ============================================================================
class TextDoc;

// ============================================================================
void updateTextDoc(TokenizedFile& file, const TextDoc& doc);
