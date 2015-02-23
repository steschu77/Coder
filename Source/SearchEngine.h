#pragma once

class TextDoc;

#include <Source/TextPos.h>

// ============================================================================
class SearchEngine
// ----------------------------------------------------------------------------
{
public:
  SearchEngine();
  
  void indexDocument(const TextDoc& pDoc, const std::string& Search, bool WholeWord, bool MatchCase);
  void updateDocument(const TextDoc& pDoc);
  void invalidate();

  size_t getPatternLength() const;
  size_t getResultCount() const;
    
  bool getNextResult(TextPos* pPos, bool Direction);

  const std::vector<TextPos>& getResults() const;

  uint getSearchResultVersion() const;

private:
  std::string _Search;

  bool _WholeWord;
  bool _MatchCase;

  uint SearchResultVersion;

  std::vector<TextPos> _Results;
  std::vector<TextPos>::const_iterator _CurrentResult;
};

// ----------------------------------------------------------------------------
inline uint SearchEngine::getSearchResultVersion() const
{
  return SearchResultVersion;
}

// ----------------------------------------------------------------------------
inline size_t SearchEngine::getPatternLength() const
{
  return _Search.length();
}

// ----------------------------------------------------------------------------
inline const std::vector<TextPos>& SearchEngine::getResults() const
{
  return _Results;
}
