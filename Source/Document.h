/*!
** \file    Source/Document.h
** \brief
\*****************************************************************************/
#pragma once

#include "TextEdit.h"

// ============================================================================
class TextDocument
// ----------------------------------------------------------------------------
{
public:
  TextDocument(const char* Path);
  
  retcode load();
  retcode save();

  const TextDoc& getDoc() const;
  
  const std::string getPath() const;
  bool isDirty() const;
  
  size_t getIndent(size_t line) const;
  TextPos getCursor() const;
  retcode setCursor(const TextPos& pos);
  
  retcode moveLeft(bool Select);
  retcode moveRight(bool Select);
  retcode moveUp(bool Select, size_t cLines=1);
  retcode moveDown(bool Select, size_t cLines=1);
  retcode moveNextWord(bool Select);
  retcode movePrevWord(bool Select);
  retcode moveBeginOfLine(bool Select);
  retcode moveEndOfLine(bool Select);
  retcode moveBeginOfDocument(bool Select);
  retcode moveEndOfDocument(bool Select);
  
  retcode insert(char ch);
  retcode insertNewLine();
  retcode insertNewLineBefore();
  retcode insertContent(const TextDoc& doc);
  
  retcode deleteBefore();
  retcode deleteAfter();
  retcode deleteLine();

  retcode undo();
  retcode redo();

  bool getSelection(TextPos* pP0, TextPos* pP1) const;
  bool hasSelectedText(size_t line) const;
  bool isSelected(size_t line, size_t column) const;

  TextDoc getSelectedText() const;

  uint getDocumentVersion() const;
  uint getCursorVersion() const;
  uint getSelectionVersion() const;

  retcode insertOpeningBracket();

private:
  std::string _Path;

  TextEdit _Doc;

  TextPos _Cursor;
  TextPos _SelectionStart;
  
  bool _Selecting;

  // Versions for fast change detection
  uint _CursorVersion;
  uint _SelectionVersion;

  uint _PersistentVersion;

  char _getChar(const TextPos& pos) const;
  char _getNextChar(TextPos* pPos) const;
  char _getPrevChar(TextPos* pPos) const;

  TextPos _getNextPos(const TextPos& pos) const;
  TextPos _getPrevPos(const TextPos& pos) const;

  void _moveNextNonIdentifier(TextPos* pPos) const;
  void _moveNextIdentifier(TextPos* pPos) const;

  void _movePrevNonIdentifier(TextPos* pPos) const;
  void _movePrevIdentifier(TextPos* pPos) const;

  void _handleSelection(bool Select, bool Discard);

  void _startSelection();
  void _discardSelection();
  void _deleteSelection();
};

// ----------------------------------------------------------------------------
inline const TextDoc& TextDocument::getDoc() const
{
  return _Doc.getDoc();
}

// ----------------------------------------------------------------------------
inline uint TextDocument::getDocumentVersion() const
{
  return _Doc.getVersion();
}

// ----------------------------------------------------------------------------
inline uint TextDocument::getCursorVersion() const
{
  return _CursorVersion;
}

// ----------------------------------------------------------------------------
inline uint TextDocument::getSelectionVersion() const
{
  return _SelectionVersion;
}
