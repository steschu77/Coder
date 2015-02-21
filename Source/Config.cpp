#include <Source/Headers.h>
#include "Config.h"

// ----------------------------------------------------------------------------
Config gConfig;

// ----------------------------------------------------------------------------
ConfigColors::ConfigColors()
: bkgEditor(0x00fdf6e3)
, bkgCursorLine(0x00f9f0d3)
, colWhiteSpace(0x80a39B85)
, colNumber(0x00078A78)
, colString(0x00078A78)
, colIdentifier(0x00880707)
, colOperator(0x00164B32)
, colComment(0x00078A07)
, colPreProcessor(0x00164BCB)
, colKeyword(0x00164BCB)
, colLineNumber(0x00969483)
, bkgLineNumber(0x80D5E8EE)
, bkgFindResult(0x2807d007)
, bkgSelection(0x807e7b71)
{
};

// ----------------------------------------------------------------------------
Config::Config()
: winPosX(100)
, winPosY(100)
, winSizeX(100)
, winSizeY(50)
, fontSizeX(8)
, fontSizeY(14)
, tabSize(2)
{
};
