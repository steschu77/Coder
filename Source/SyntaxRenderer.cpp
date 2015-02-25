#include <Source/Headers.h>
#include <Source/Canvas.h>
#include <Source/Config.h>
#include <Source/Tokenizer.h>

// ----------------------------------------------------------------------------
void renderSyntaxHilighting(std::vector<gfx::TextChar>& tl, const std::string& line, Tokenizer::state_t initialState)
{
  uint32 gColors[]
  {
    gConfig.Colors.colNumber,
    gConfig.Colors.colString,
    gConfig.Colors.colIdentifier,
    gConfig.Colors.colOperator,
    gConfig.Colors.colComment,
    gConfig.Colors.colPreProcessor,
    gConfig.Colors.colKeyword,
  };

  size_t cLength = line.length();
  size_t idx = 0;
  
  Tokenizer tk(line, initialState);

  Tokenizer::token_t t;
  while (tk.nextToken(&t) < Tokenizer::sError0)
  {
    while (idx < t.p0) {
      tl[idx].colText = gConfig.Colors.colWhiteSpace;
      tl[idx].colBkg  = gConfig.Colors.bkgEditor;
      switch (line[idx])
      {
      case ' ':
        tl[idx].ch = 0xb7;
        break;
      default:
        tl[idx].ch = line[idx];
        break;
      }
      idx++;
    }
    
    while (idx <= t.p1) {
      tl[idx].colText = gColors[t.type];
      tl[idx].colBkg  = gConfig.Colors.bkgEditor;
      tl[idx].ch = line[idx];
      tl[idx].bold = t.type == Tokenizer::tPreProcessor;
      tl[idx].underline = t.px != std::string::npos;
      tl[idx].colLine = 0x00d50707;
      idx++;
    }
  }

  while (idx < cLength) {
    tl[idx].colText = gConfig.Colors.colWhiteSpace;
    switch (line[idx])
    {
    case ' ':
      tl[idx].ch = 0xb7;
      break;
    default:
      tl[idx].ch = line[idx];
      break;
    }
    idx++;
  }
}
