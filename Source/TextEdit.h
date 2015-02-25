/*!
** \file    Source/TextEdit.h
** \brief   Text editing helper class
\*****************************************************************************/
#pragma once

#include <Source/TextPos.h>
#include <Source/TextDoc.h>
#include <Source/History.h>

// ============================================================================
class TextEdit
{
public:
  TextEdit();
  
  const TextDoc& getDoc() const;
  uint getVersion() const;

  size_t getLineCount() const;
  size_t getLineLength(size_t line) const;
  size_t getMaxLineLength() const;

  TextPos getEndOfDoc() const;
  TextPos getNextPos(const TextPos& Pos) const;
  TextPos getPrevPos(const TextPos& Pos) const;

  void load(const char* Path);
  void save(const char* Path) const;

  void replaceContent(char* pDoc, size_t Length);

  void insertChars(const TextPos& pos, const char* ch);
  void insertNewLine(const TextPos& pos, size_t indent=0);
  void insertNewLineBefore(size_t line, size_t indent=0);
  void deleteChar(size_t line, size_t col);
  void deleteLine(size_t line);

  void insertContent(const TextPos& p, const TextDoc& Content);
  void deleteContent(const TextPos& p0, const TextPos& p1);

  void undo(TextPos* pPos);
  void redo(TextPos* pPos);

private:
  EditableTextDoc _Doc;
  History _Hist;

  const TextPos& _applyCommand(uint type, const EditCommand& cmd);
};

// ----------------------------------------------------------------------------
inline const TextDoc& TextEdit::getDoc() const
{
  return _Doc;
}

// ----------------------------------------------------------------------------
inline uint TextEdit::getVersion() const
{
  return _Doc.getVersion();
}

// ----------------------------------------------------------------------------
inline size_t TextEdit::getLineCount() const
{
  return _Doc.getLineCount();
}

// ----------------------------------------------------------------------------
inline size_t TextEdit::getLineLength(size_t line) const
{
  return _Doc.getLineLength(line);
}

// ----------------------------------------------------------------------------
inline size_t TextEdit::getMaxLineLength() const
{
  return _Doc.getMaxLineLength();
}

// ----------------------------------------------------------------------------
inline TextPos TextEdit::getEndOfDoc() const
{
  return _Doc.getEndOfDoc();
}

// ----------------------------------------------------------------------------
inline TextPos TextEdit::getNextPos(const TextPos& Pos) const
{
  return _Doc.getNextPos(Pos);
}

// ----------------------------------------------------------------------------
inline TextPos TextEdit::getPrevPos(const TextPos& Pos) const
{
  return _Doc.getPrevPos(Pos);
}
