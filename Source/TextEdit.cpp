#include <Source/Headers.h>
#include "TextEdit.h"

// ----------------------------------------------------------------------------
TextEdit::TextEdit()
{
}

// ----------------------------------------------------------------------------
void TextEdit::load(const char* Path)
{
  _Doc = loadTextDoc(Path);
}

// ----------------------------------------------------------------------------
void TextEdit::save(const char* Path) const
{
  saveTextDoc(Path, _Doc);
}

// ----------------------------------------------------------------------------
void TextEdit::replaceContent(char* pDoc, size_t Length)
{
  _Doc.replaceContent(pDoc, Length);
}

// ----------------------------------------------------------------------------
void TextEdit::insertChars(const TextPos& pos, const char* ch)
{
  TextDoc doc(ch);
  insertContent(pos, doc);
}

// ----------------------------------------------------------------------------
void TextEdit::insertNewLine(const TextPos& pos, size_t indent)
{
  TextDoc doc(2u);
  doc.Lines[1] = std::string(indent, ' ');

  insertContent(pos, doc);
}

// ----------------------------------------------------------------------------
void TextEdit::insertNewLineBefore(size_t line, size_t indent)
{
  TextDoc doc(2u);
  doc.Lines[0] = std::string(indent, ' ');
  insertContent(TextPos(line, 0), doc);
}

// ----------------------------------------------------------------------------
void TextEdit::deleteChar(size_t line, size_t col)
{
  TextPos p0(line, col);
  TextPos p1(getNextPos(p0));

  deleteContent(p0, p1);
}

// ----------------------------------------------------------------------------
void TextEdit::deleteLine(size_t line)
{
  TextPos p0(line, 0);
  TextPos p1(line+1, 0);

  deleteContent(p0, p1);
}

// ----------------------------------------------------------------------------
void TextEdit::deleteContent(const TextPos& p0, const TextPos& p1)
{
  TextDoc undo = _Doc.getContent(p0, p1);
  _Hist.addNewCommand(EditCommand(0, p0, p1, undo));

  _Doc.deleteContent(p0, p1);
}

// ----------------------------------------------------------------------------
void TextEdit::insertContent(const TextPos& p, const TextDoc& Content)
{
  _Hist.addNewCommand(EditCommand(1, p, p + Content.getEndOfDoc(), Content));
  _Doc.insertContent(p, Content);
}

// ----------------------------------------------------------------------------
void TextEdit::undo(TextPos* pPos)
{
  if (_Hist.countUndoCommands() == 0) {
    return;
  }

  *pPos = _applyCommand(1, _Hist.undoCommand());
}

// ----------------------------------------------------------------------------
void TextEdit::redo(TextPos* pPos)
{
  if (_Hist.countRedoCommands() == 0) {
    return;
  }

  *pPos = _applyCommand(0, _Hist.redoCommand());
}

// ----------------------------------------------------------------------------
const TextPos& TextEdit::_applyCommand(uint type, const EditCommand& cmd)
{
  if (cmd.type ^ type) {
    _Doc.insertContent(cmd.p0, cmd.content);
  } else {
    _Doc.deleteContent(cmd.p0, cmd.p1);
  }

  return cmd.p0;
}
