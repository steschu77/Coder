#include <Source/Headers.h>
#include <Source/TextPos.h>

#include "TextDoc.h"

// ----------------------------------------------------------------------------
TextDoc::TextDoc()
{
}

// ----------------------------------------------------------------------------
TextDoc::TextDoc(const char* ch)
: Lines(1)
{
  Lines[0] = ch;
}

// ----------------------------------------------------------------------------
TextDoc::TextDoc(size_t cLines)
: Lines(cLines)
{
}

// ----------------------------------------------------------------------------
size_t TextDoc::getLineCount() const
{
  return Lines.size();
}

// ----------------------------------------------------------------------------
size_t TextDoc::getLineLength(size_t idx) const
{
  return idx < Lines.size() ? Lines[idx].length() : 0;
}

// ----------------------------------------------------------------------------
size_t TextDoc::getMaxLineLength() const
{
  size_t cLines = Lines.size();
  size_t MaxLineLength = 0;
  for (size_t i = 0; i < cLines; i++) {
    MaxLineLength = std::max(MaxLineLength, Lines[i].length());
  }

  return MaxLineLength;
}

// ----------------------------------------------------------------------------
TextPos TextDoc::getEndOfDoc() const
{
  size_t lastLine = getLineCount()-1;
  return TextPos(lastLine, getLineLength(lastLine));
}

// ----------------------------------------------------------------------------
TextPos TextDoc::getNextPos(const TextPos& Pos) const
{
  TextPos pos(Pos);

  if (++pos.column > getLineLength(pos.line)) {
    ++pos.line;
    pos.column = 0;
  }

  return pos;
}

// ----------------------------------------------------------------------------
TextPos TextDoc::getPrevPos(const TextPos& Pos) const
{
  TextPos pos(Pos);
  if (--pos.column >= getLineLength(pos.line)) {
    --pos.line;
    pos.column = getLineLength(pos.line);
  }

  return pos;
}

// ----------------------------------------------------------------------------
char TextDoc::getCharAt(const TextPos& pos) const
{
  if (pos.line < getLineCount())
  {
    std::string line = getLineAt(pos.line);

    if (pos.column < getLineLength(pos.line)) {
      return line[pos.column];
    } else if (pos.column == getLineLength(pos.line)) {
      return '\n';
    }
  }

  return '\0';
}

// ----------------------------------------------------------------------------
const std::string& TextDoc::getLineAt(size_t idx) const
{
  return Lines.at(idx);
}

// ----------------------------------------------------------------------------
std::string TextDoc::exportContent() const
{
  std::string text;

  size_t cLines = getLineCount();

  for (size_t i = 0; i < cLines; i++)
  {
    const std::string& line = getLineAt(i);

    if (i > 0) {
      text.push_back('\n');
    }

    text.append(line);
  }

  return text;
}

// ----------------------------------------------------------------------------
TextDoc TextDoc::getContent(const TextPos& p0, const TextPos& p1) const
{
  TextDoc doc;

  for (size_t i = p0.line; i <= p1.line; i++)
  {
    std::string line = Lines.at(i);

    if (i == p0.line && i == p1.line) {
      line = line.substr(p0.column, p1.column - p0.column);
    } else if (i == p0.line) {
      line = line.substr(p0.column, std::string::npos);
    } else if (i == p1.line) {
      line = line.substr(0, p1.column);
    }

    doc.Lines.push_back(line);
  }

  return doc;
}

// ----------------------------------------------------------------------------
EditableTextDoc::EditableTextDoc()
: Version(0)
{
}

// ----------------------------------------------------------------------------
void EditableTextDoc::replaceContent(char* pDoc, size_t Length)
{
  if (pDoc == nullptr) {
    return;
  }

  std::vector<char*> lines;
  int SearchMode = 0;
  
  for (size_t i = 0; i <= Length; i++)
  {
    if (SearchMode == 0) {
      lines.push_back(pDoc+i);
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

  Lines.clear();
  Lines.resize(lines.size());

  size_t idx = 0;
  for (auto line : lines) {
    Lines[idx++] = line;
  }

  Version = 0;
}

// ----------------------------------------------------------------------------
void EditableTextDoc::deleteContent(const TextPos& p0, const TextPos& p1)
{
  EditableTextDoc doc;

  size_t cLines = getLineCount();

  doc.Lines.reserve(cLines);

  for (size_t i = 0; i < cLines; i++)
  {
    if (i < p0.line || i > p1.line) {
      doc.Lines.push_back(Lines[i]);
      continue;
    }

    if (i > p0.line && i <= p1.line) {
      continue;
    }

    std::string line0 = Lines[p0.line].substr(0, p0.column);
    std::string line1 = Lines[p1.line].substr(p1.column, std::string::npos);

    doc.Lines.push_back(line0 + line1);
  }

  Lines.swap(doc.Lines);
  Version++;
}

// ----------------------------------------------------------------------------
void EditableTextDoc::insertContent(const TextPos& p, const TextDoc& Content)
{
  if (Content.getLineCount() == 1) {
    Lines.at(p.line).insert(p.column, Content.Lines[0]);
    return;
  }

  std::string l1 = Lines.at(p.line).substr(p.column);

  Lines[p.line].erase(p.column);
  Lines[p.line].append(Content.Lines[0]);
  
  std::vector<std::string>::iterator it = Lines.insert(Lines.begin() + p.line + 1, Content.Lines.begin() + 1, Content.Lines.end());
  (it + Content.Lines.size() - 2)->append(l1);

  Version++;
}
