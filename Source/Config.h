/*!
** \file    /Source/Config.h
** \brief   Configuration structure definition
**
**          Tune this file if your want to customize your settings.
******************************************************************************/
#pragma once

// ============================================================================
struct ConfigColors
{
  ConfigColors();

  uint32 bkgEditor;
  uint32 bkgCursorLine;
  uint32 colWhiteSpace;
  uint32 colNumber;
  uint32 colString;
  uint32 colIdentifier;
  uint32 colOperator;
  uint32 colComment;
  uint32 colPreProcessor;
  uint32 colKeyword;
  uint32 colLineNumber;
  uint32 bkgLineNumber;
  uint32 bkgFindResult;
  uint32 bkgSelection;
};

// ============================================================================
struct Config
{
  Config();

  int winPosX;
  int winPosY;
  int winSizeX;
  int winSizeY;

  int fontSizeX;
  int fontSizeY;

  int tabSize;
  
  std::vector<std::string> Files;
  
  ConfigColors Colors;
};

// ============================================================================
extern Config gConfig;
