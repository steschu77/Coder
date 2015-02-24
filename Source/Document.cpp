#include <Source/Headers.h>
#include <Source/UTF8Tools.h>
#include <Source/Config.h>
#include <Source/History.h>

#include "Document.h"

// ----------------------------------------------------------------------------
static bool isIdentifier(char ch)
{
  return ch  == '_' || isAlpha(ch) || isDigit(ch);
}

// ============================================================================
char* readTextFile(const char* Path, size_t* pLength)
{
  FILE* f = fopen(Path, "rb");

  if (f == nullptr) {
    return nullptr;
  }

  fseek(f, 0, SEEK_END);
  size_t FileSize = ftell(f);
  fseek(f, 0, SEEK_SET);
  
  char* pFile = new char [FileSize+1];
  fread(pFile, FileSize, 1, f);
  fclose(f);
  
  pFile[FileSize] = '\0';
  *pLength = FileSize;
  
  return pFile;
}

// ----------------------------------------------------------------------------
TextDocument::TextDocument(const char* Path)
: _Path(Path)
, _Cursor(0, 0)
, _Selecting(false)
, _CursorVersion(0)
, _SelectionVersion(0)
, _DocumentVersion(0)
, _PersistentVersion(0)
{
}

// ----------------------------------------------------------------------------
const std::string TextDocument::getPath() const
{
  return _Path;
}

// ----------------------------------------------------------------------------
bool TextDocument::isDirty() const
{
  return _Doc.getVersion() != _PersistentVersion;
}

// ----------------------------------------------------------------------------
retcode TextDocument::load()
{
  size_t Length = 0;
  char* pDoc = readTextFile(_Path.c_str(), &Length);

  if (pDoc == nullptr) {
    return rcFileNotFound;
  }
  
  _Doc.replaceContent(pDoc, Length);
  _PersistentVersion = _Doc.getVersion();
  
  delete[] pDoc;
  
  updateTextDoc(_File, _Doc.getDoc());
  return rcSuccess;
}

// ----------------------------------------------------------------------------
retcode TextDocument::save()
{
  FILE* f = fopen(_Path.c_str(), "wb");
  const TextDoc& doc = _Doc.getDoc();

  size_t cLines = doc.getLineCount();

  for (size_t i = 0; i < cLines; i++)
  {
    size_t cLength = doc.getLineLength(i);
    const char* pLine = doc.getLineAt(i).c_str();

    if (i > 0) {
      fwrite("\n", 1, 1, f);
    }
    fwrite(pLine, 1, cLength, f);
  }

  fclose(f);

  _PersistentVersion = _Doc.getVersion();
  return rcSuccess;
}

// ----------------------------------------------------------------------------
size_t TextDocument::getIndent(size_t line) const
{
  const TextDoc& doc = _Doc.getDoc();
  size_t indent = std::string::npos;

  if (line > doc.getLineCount()) {
    return 0;
  }

  while (line > 0 && indent == std::string::npos)
  {
    const std::string& strLine = doc.getLineAt(line);
    indent = strLine.find_first_not_of(' ');
    line--;
  }

  return (indent == std::string::npos) ? 0u : indent;
}

// ----------------------------------------------------------------------------
TextPos TextDocument::getCursor() const
{
  return _Cursor;
}

// ----------------------------------------------------------------------------
retcode TextDocument::setCursor(const TextPos& p, bool Select)
{
  TextPos pos(p);

  size_t cLines = _Doc.getLineCount();
  if (pos.line >= cLines) {
    pos.line = cLines - 1;
  }

  size_t cLine = _Doc.getLineLength(pos.line);
  if (pos.column > cLine) {
    pos.column = cLine;
  }

  _Cursor = pos;
  _CursorVersion++;

  _handleSelection(Select, true);
  return rcSuccess;
}

// ----------------------------------------------------------------------------
retcode TextDocument::moveLeft(bool Select)
{
  _handleSelection(Select, true);

  if (_Cursor > TextPos(0, 0)) {
    _Cursor = _Doc.getPrevPos(_Cursor);
    return rcSuccess;
  } else {
    return rcOutOfRange;
  }
}

// ----------------------------------------------------------------------------
retcode TextDocument::moveRight(bool Select)
{
  _handleSelection(Select, true);

  if (_Cursor < _Doc.getEndOfDoc()) {
    _Cursor = _Doc.getNextPos(_Cursor);
    return rcSuccess;
  } else {
    return rcOutOfRange;
  }
}

// ----------------------------------------------------------------------------
retcode TextDocument::moveUp(bool Select, size_t cLines)
{
  _handleSelection(Select, true);

  if (_Cursor.line >= cLines) {
    _Cursor.line -= cLines;
    _Cursor.column = std::min(_Cursor.column, _Doc.getLineLength(_Cursor.line));
    _CursorVersion++;
    return rcSuccess;
  } else {
    moveBeginOfDocument(Select);
    return rcOutOfRange;
  }
}

// ----------------------------------------------------------------------------
retcode TextDocument::moveDown(bool Select, size_t cLines)
{
  _handleSelection(Select, true);

  if (_Cursor.line < _Doc.getLineCount()-cLines) {
    _Cursor.line += cLines;
    _Cursor.column = std::min(_Cursor.column, _Doc.getLineLength(_Cursor.line));
    _CursorVersion++;
    return rcSuccess;
  } else {
    moveEndOfDocument(Select);
    return rcOutOfRange;
  }
}

// ----------------------------------------------------------------------------
retcode TextDocument::moveNextWord(bool Select)
{
  _handleSelection(Select, true);

  TextPos pos = getCursor();


  if (Select) {
    if (isIdentifier(_getChar(_Cursor))) {
      _moveNextNonIdentifier(&pos);
    } else {
      _moveNextIdentifier(&pos);
      _moveNextNonIdentifier(&pos);
    }
  } else {
    _moveNextNonIdentifier(&pos);
    _moveNextIdentifier(&pos);
  }

  if (pos.line >= _Doc.getLineCount()) {
    pos.line = _Doc.getLineCount() - 1;
    pos.column = _Doc.getLineLength(pos.line);
  }

  _Cursor = pos;
  _CursorVersion++;
  return rcSuccess;
}

// ----------------------------------------------------------------------------
retcode TextDocument::movePrevWord(bool Select)
{
  _handleSelection(Select, true);

  TextPos pos = _getPrevPos(getCursor());

  _movePrevIdentifier(&pos);
  _movePrevNonIdentifier(&pos);

  if (pos.line >= _Doc.getLineCount()) {
    pos.line = 0;
    pos.column = 0;
  } else {
    pos = _getNextPos(pos);
  }

  _Cursor = pos;
  _CursorVersion++;
  return rcSuccess;
}

// ----------------------------------------------------------------------------
retcode TextDocument::moveBeginOfLine(bool Select)
{
  _handleSelection(Select, true);

  _Cursor.column = 0;
  _CursorVersion++;
  return rcSuccess;
}

// ----------------------------------------------------------------------------
retcode TextDocument::moveEndOfLine(bool Select)
{
  _handleSelection(Select, true);

  _Cursor.column = _Doc.getLineLength(_Cursor.line);
  _CursorVersion++;
  return rcSuccess;
}

// ----------------------------------------------------------------------------
retcode TextDocument::moveBeginOfDocument(bool Select)
{
  _handleSelection(Select, true);

  _Cursor.line = 0;
  _Cursor.column = 0;
  _CursorVersion++;
  return rcSuccess;
}

// ----------------------------------------------------------------------------
retcode TextDocument::moveEndOfDocument(bool Select)
{
  _handleSelection(Select, true);

  _Cursor = _Doc.getEndOfDoc();
  _CursorVersion++;
  return rcSuccess;
}

// ----------------------------------------------------------------------------
retcode TextDocument::insert(char ch)
{
  _deleteSelection();

  std::string str(1, ch);
  _Doc.insertChars(_Cursor, str.c_str());

  _Cursor.column++;
  _CursorVersion++;
  return rcSuccess;
}

// ----------------------------------------------------------------------------
retcode TextDocument::insertNewLine()
{
  _deleteSelection();

  size_t indent = getIndent(_Cursor.line);
  
  if (_Cursor.column < indent) {
    indent = 0;
  }

  _Doc.insertNewLine(_Cursor, indent);
  _Cursor.column = indent;
  _Cursor.line++;
  return rcSuccess;
}

// ----------------------------------------------------------------------------
retcode TextDocument::insertNewLineBefore()
{
  _deleteSelection();

  size_t indent = getIndent(_Cursor.line);

  _Doc.insertNewLineBefore(_Cursor.line, indent);
  _Cursor.column = indent;
  return rcSuccess;
}

// ----------------------------------------------------------------------------
retcode TextDocument::insertContent(const TextDoc& doc)
{
  _deleteSelection();
  _Doc.insertContent(_Cursor, doc);
  return rcSuccess;
}

// ----------------------------------------------------------------------------
retcode TextDocument::deleteBefore()
{
  if (_Selecting) {
    _deleteSelection();
  }
  else {
    moveLeft(false);
    _Doc.deleteChar(_Cursor.line, _Cursor.column);
  }

  return rcSuccess;
}

// ----------------------------------------------------------------------------
retcode TextDocument::deleteAfter()
{
  if (_Selecting) {
    _deleteSelection();
  }
  else {
    _Doc.deleteChar(_Cursor.line, _Cursor.column);
  }
  return rcSuccess;
}

// ----------------------------------------------------------------------------
retcode TextDocument::deleteLine()
{
  _Doc.deleteLine(_Cursor.line);
  return rcSuccess;
}

// ----------------------------------------------------------------------------
retcode TextDocument::undo()
{
  _Doc.undo(&_Cursor);
  return rcSuccess;
}

// ----------------------------------------------------------------------------
retcode TextDocument::redo()
{
  _Doc.redo(&_Cursor);
  return rcSuccess;
}

// ----------------------------------------------------------------------------
retcode TextDocument::selectWord(const TextPos& pos)
{
  setCursor(pos, false);
  movePrevWord(false);
  moveNextWord(true);
  return rcSuccess;
}

// ----------------------------------------------------------------------------
bool TextDocument::getSelection(TextPos* pP0, TextPos* pP1) const
{
  if (!_Selecting) {
    return false;
  }

  if (_SelectionStart < _Cursor)
  {
    *pP0 = _SelectionStart;
    *pP1 = _Cursor;
  }
  else
  {
    *pP0 = _Cursor;
    *pP1 = _SelectionStart;
  }

  return true;
}

// ----------------------------------------------------------------------------
bool TextDocument::hasSelectedText(size_t line) const
{
  if (!_Selecting) return false;
  return (line >= _SelectionStart.line && line <= _Cursor.line)
      || (line <= _SelectionStart.line && line >= _Cursor.line);
}

// ----------------------------------------------------------------------------
bool TextDocument::isSelected(size_t line, size_t column) const
{
  TextPos p0, p1;
  if (!getSelection(&p0, &p1)) {
    return false;
  }

  if (line < p0.line || line > p1.line) {
    return false;
  }

  if (line == p0.line && column < p0.column) {
    return false;
  }

  if (line == p1.line && column >= p1.column) {
    return false;
  }

  return true;
}

// ----------------------------------------------------------------------------
TextDoc TextDocument::getSelectedText() const
{
  TextPos p0 = _Cursor;
  TextPos p1 = _Cursor;

  p0.column = 0;
  p1.column = _Doc.getLineLength(p1.line);

  getSelection(&p0, &p1);

  return _Doc.getDoc().getContent(p0, p1);
}

// ----------------------------------------------------------------------------
void TextDocument::deleteSelectedText()
{
  _deleteSelection();
}

// ----------------------------------------------------------------------------
Tokenizer::state_t TextDocument::getInitialStateAt(size_t line) const
{
  return _File.initialStates.at(line);
}

// ----------------------------------------------------------------------------
char TextDocument::_getChar(const TextPos& pos) const
{
  if (pos.line < _Doc.getLineCount())
  {
    std::string line = _Doc.getDoc().getLineAt(pos.line);

    if (pos.column < _Doc.getLineLength(pos.line)) {
      return line[pos.column];
    } else if (pos.column == _Doc.getLineLength(pos.line)) {
      return '\n';
    }
  }

  return '\0';
}

// ----------------------------------------------------------------------------
char TextDocument::_getNextChar(TextPos* pPos) const
{
  *pPos = _getNextPos(*pPos);
  return _getChar(*pPos);
}

// ----------------------------------------------------------------------------
char TextDocument::_getPrevChar(TextPos* pPos) const
{
  *pPos = _getPrevPos(*pPos);
  return _getChar(*pPos);
}

// ----------------------------------------------------------------------------
TextPos TextDocument::_getNextPos(const TextPos& Pos) const
{
  TextPos pos(Pos);

  if (++pos.column >= _Doc.getLineLength(pos.line)) {
    ++pos.line;
    pos.column = 0;
  }

  return pos;
}

// ----------------------------------------------------------------------------
TextPos TextDocument::_getPrevPos(const TextPos& Pos) const
{
  TextPos pos(Pos);
  if (--pos.column >= _Doc.getLineLength(pos.line)) {
    --pos.line;
    pos.column = _Doc.getLineLength(pos.line);
  }

  return pos;
}

// ----------------------------------------------------------------------------
void TextDocument::_moveNextNonIdentifier(TextPos* pPos) const
{
  for (char ch = _getChar(*pPos); ch != '\0' && isIdentifier(ch); ch = _getNextChar(pPos));
}

// ----------------------------------------------------------------------------
void TextDocument::_moveNextIdentifier(TextPos* pPos) const
{
  for (char ch = _getChar(*pPos); ch != '\0' && !isIdentifier(ch); ch = _getNextChar(pPos));
}

// ----------------------------------------------------------------------------
void TextDocument::_movePrevNonIdentifier(TextPos* pPos) const
{
  for (char ch = _getChar(*pPos); ch != '\0' && isIdentifier(ch); ch = _getPrevChar(pPos));
}

// ----------------------------------------------------------------------------
void TextDocument::_movePrevIdentifier(TextPos* pPos) const
{
  for (char ch = _getChar(*pPos); ch != '\0' && !isIdentifier(ch); ch = _getPrevChar(pPos));
}

// C++ Specific

// ----------------------------------------------------------------------------
retcode TextDocument::insertOpeningBracket()
{
  size_t indent = getIndent(_Cursor.line);

  _Doc.insertChars(_Cursor, "{");
  _Doc.insertNewLine(TextPos(_Cursor.line, _Cursor.column+1), indent + gConfig.tabSize);
  _Doc.insertNewLine(TextPos(_Cursor.line+1, _Doc.getLineLength(_Cursor.line+1)), indent);
  _Doc.insertChars(TextPos(_Cursor.line+2, indent), "}");

  _Cursor.column = indent + gConfig.tabSize;
  _Cursor.line++;
  return rcSuccess;
}

// ----------------------------------------------------------------------------
void TextDocument::_handleSelection(bool Select, bool Discard)
{
  if (Select && !_Selecting) {
    _startSelection();
  } else if (!Select && _Selecting) {
    _discardSelection();
  } else if (Select) {
    _SelectionVersion++;
  }
}

// ----------------------------------------------------------------------------
void TextDocument::_startSelection()
{
  _Selecting = true;
  _SelectionStart = _Cursor;

  _SelectionVersion++;
}

// ----------------------------------------------------------------------------
void TextDocument::_discardSelection()
{
  _Selecting = false;
  _SelectionVersion++;
}

// ----------------------------------------------------------------------------
void TextDocument::_deleteSelection()
{
  TextPos p0, p1;
  if (!getSelection(&p0, &p1)) {
    return;
  }

  _Doc.deleteContent(p0, p1);

  _Selecting = false;
  _SelectionVersion++;

  _Cursor = p0;
}

// ----------------------------------------------------------------------------
uint TextDocument::getDocumentVersion()
{
  uint Version = _Doc.getVersion();
  if (Version != _DocumentVersion)
  {
    updateTextDoc(_File, _Doc.getDoc());
    _DocumentVersion = Version;
  }

  return Version;
}

