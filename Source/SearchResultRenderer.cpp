#include <Source/Headers.h>
#include <Source/UTF8Tools.h>
#include <Source/Document.h>
#include <Source/Config.h>
#include <Source/Canvas.h>
#include <Source/SearchEngine.h>

// ----------------------------------------------------------------------------
void renderSearchResults(std::vector<gfx::TextChar>& text, const SearchEngine& Search, size_t line)
{
  const std::vector<TextPos>& Results = Search.getResults();
  std::vector<TextPos>::const_iterator i = std::lower_bound(Results.begin(), Results.end(), TextPos(line,0));

  size_t cSearch = Search.getPatternLength();
  for (; i != Results.end() && i->line == line; ++i)
  {
    for (size_t j = 0; j < cSearch; j++) {
      text[i->column + j].background = true;
      text[i->column + j].colBkg = gfx::blendPixel(gConfig.Colors.bkgFindResult, text[i->column + j].colBkg);
    }
  }
}
