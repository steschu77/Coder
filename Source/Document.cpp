#include <Source/Headers.h>
#include <Source/UTF8Tools.h>
#include <Source/Config.h>

#include "Document.h"

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
  return _Doc.Version != 0;
}

// ----------------------------------------------------------------------------
retcode TextDocument::load()
{
  size_t Length = 0;
  char* pDoc = readTextFile(_Path.c_str(), &Length);

  if (pDoc == nullptr) {
    return rcFileNotFound;
  }
  
  std::vector<char*> Lines;
  int SearchMode = 0;
  
  for (size_t i = 0; i <= Length; i++)
  {
    if (SearchMode == 0) {
      Lines.push_back(pDoc+i);
    }

    if (pDoc[i] == '\n') {
      SearchMode = 0;
    } else {
      SearchMode = 1;
    }

    if (pDoc[i] == '\n' || pDoc[i] == '\r') {
      pDoc[i] = '\0';
    }
  }

  _Doc.setContent(Lines);
  _Doc.Version = 0;
  
  delete[] pDoc;
  
  return rcSuccess;
}

// ----------------------------------------------------------------------------
retcode TextDocument::save()
{
  FILE* f = fopen(_Path.c_str(), "wb");

  size_t cLines = _Doc.getLineCount();

  for (size_t i = 0; i < cLines; i++)
  {
    size_t cLength = _Doc.getLineLength(i);
    const char* pLine = _Doc.getLine(i).c_str();

    if (i > 0) {
      fwrite("\n", 1, 1, f);
    }
    fwrite(pLine, 1, cLength, f);
  }

  fclose(f);

  _Doc.Version = 0;
  return rcSuccess;
}

// ----------------------------------------------------------------------------
size_t TextDocument::getIndent(size_t line) const
{
  size_t indent = std::string::npos;

  if (line > _Doc.getLineCount()) {
    return 0;
  }

  while (line > 0 && indent == std::string::npos)
  {
    const std::string& strLine = _Doc.getLine(line);
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
retcode TextDocument::setCursor(const TextPos& pos)
{
  size_t cLines = _Doc.getLineCount();
  if (pos.line >= cLines) {
    return rcOutOfRange;
  }

  size_t Length = _Doc.getLineLength(pos.line);
  if (pos.column > Length) {
    return rcOutOfRange;
  }

  _Cursor = pos;
  _CursorVersion++;
  return rcSuccess;
}

// ----------------------------------------------------------------------------
retcode TextDocument::moveLeft(bool Select)
{
  _handleSelection(Select, true);

  if (_Cursor.column > 0) {
    _Cursor.column--;
    _CursorVersion++;
    return rcSuccess;
  } else if (_Cursor.line > 0) {
    _Cursor.line--;
    _Cursor.column = _Doc.getLineLength(_Cursor.line);
    _CursorVersion++;
    return rcSuccess;
  }
  
  return rcOutOfRange;
}

// ----------------------------------------------------------------------------
retcode TextDocument::moveRight(bool Select)
{
  _handleSelection(Select, true);

  if (_Cursor.column < _Doc.getLineLength(_Cursor.line)) {
    _Cursor.column++;
    _CursorVersion++;
    return rcSuccess;
  } else if (_Cursor.line < _Doc.getLineCount()-1) {
    _Cursor.line++;
    _Cursor.column = 0;
    _CursorVersion++;
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
  _moveNextNonIdentifier(&pos);
  _moveNextIdentifier(&pos);

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

  _Cursor.line = _Doc.getLineCount()-1;
  _Cursor.column = _Doc.getLineLength(_Cursor.line);
  _CursorVersion++;
  return rcSuccess;
}

// ----------------------------------------------------------------------------
retcode TextDocument::insert(char ch)
{
  _deleteSelection();

  std::string str(1, ch);
  _Doc.insertChars(_Cursor.line, _Cursor.column, str.c_str());

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

  _Doc.insertNewLine(_Cursor.line, _Cursor.column, indent);
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
  _Cursor.line;
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

  return _Doc.getContent(p0, p1);
}

// ----------------------------------------------------------------------------
static bool isIdentifier(char ch)
{
  return ch  == '_' || isAlpha(ch) || isDigit(ch);
}

// ----------------------------------------------------------------------------
char TextDocument::_getChar(const TextPos& pos) const
{
  if (pos.line < _Doc.getLineCount())
  {
    std::string line = _Doc.getLine(pos.line);

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

  _Doc.insertChars(_Cursor.line, _Cursor.column, "{");
  _Doc.insertNewLine(_Cursor.line, _Cursor.column+1, indent + gConfig.tabSize);
  _Doc.insertNewLine(_Cursor.line+1, _Doc.getLineLength(_Cursor.line+1), indent);
  _Doc.insertChars(_Cursor.line+2, indent, "}");

  _Cursor.column = indent + gConfig.tabSize;
  _Cursor.line++;
  return rcSuccess;
}

// ----------------------------------------------------------------------------
void TextDocument::_handleSelection(bool Select, bool Discard)
{
  _SelectionVersion++;

  if (Select && !_Selecting) {
    _startSelection();
  } else if (!Select && _Selecting) {
    _discardSelection();
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
