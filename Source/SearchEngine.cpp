#include <Source/Headers.h>
#include <Source/UTF8Tools.h>
#include <Source/Document.h>

#include "SearchEngine.h"

// ============================================================================
SearchEngine::SearchEngine()
: _WholeWord(false)
, _MatchCase(false)
, SearchResultVersion(0)
{
}

// ----------------------------------------------------------------------------
bool SearchEngine::getNextResult(TextPos* pPos, bool Direction)
{
  if (_Results.empty()) {
    return false;
  }

  if (Direction)
  {
    if (pPos->column > 0) {
      pPos->column--;
    } else {
      pPos->line--;
      pPos->column--;
    }

    std::vector<TextPos>::const_iterator i;
    i = std::lower_bound(_Results.begin(), _Results.end(), *pPos);

    if (i == _Results.begin()) {
      i = _Results.end();
    }

    *pPos = *--i;
    return true;
  }
  else
  {
    pPos->column++;

    std::vector<TextPos>::const_iterator i;
    i = std::upper_bound(_Results.begin(), _Results.end(), *pPos);

    if (i == _Results.end()) {
      i = _Results.begin();
    }

    *pPos = *i;
    return true;
  }
}

// ----------------------------------------------------------------------------
void SearchEngine::indexDocument(const TextDoc* pDoc, const std::string& Search, bool WholeWord, bool MatchCase)
{
  _Search = Search;
  _WholeWord = WholeWord;
  _MatchCase = MatchCase;

  if (!MatchCase) {
    for (auto& chr : _Search) {
      chr = toUpperCase(chr);
    }
  }

  updateDocument(pDoc);
}

// ----------------------------------------------------------------------------
void SearchEngine::updateDocument(const TextDoc* pDoc)
{
  _Results.clear();

  size_t cLines = pDoc->getLineCount();
  for (size_t i = 0; i < cLines; i++)
  {
    std::string line = pDoc->getLine(i);

    if (!_MatchCase) {
      for (auto& chr : line) {
        chr = toUpperCase(chr);
      }
    }

    size_t pos = 0;
    do
    {
      pos = line.find(_Search.c_str(), pos);
      if (pos != std::string::npos) {
        _Results.push_back(TextPos(i, pos));
        pos++;
      }
    } while (pos != std::string::npos);
  }

  _CurrentResult = _Results.begin();
  SearchResultVersion++;
}

// ----------------------------------------------------------------------------
void SearchEngine::invalidate()
{
  _Results.clear();
  SearchResultVersion++;
}

// ----------------------------------------------------------------------------
size_t SearchEngine::getResultCount() const
{
  return _Results.size();
}
