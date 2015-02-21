#pragma once

struct TextDoc;

#include <Source/Canvas.h>
#include <Source/TextPos.h>

// ============================================================================
class SearchEngine
// ----------------------------------------------------------------------------
{
public:
  SearchEngine();
  
  std::vector<gfx::TextChar> filterText(size_t line, std::vector<gfx::TextChar>& text) const;

  void indexDocument(const TextDoc* pDoc, const std::string& Search, bool WholeWord, bool MatchCase);
  void updateDocument(const TextDoc* pDoc);
  void invalidate();

  size_t getResultCount() const;
    
  bool getNextResult(TextPos* pPos, bool Direction);

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
