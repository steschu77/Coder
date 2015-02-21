#include <Source/Headers.h>
#include <Source/UTF8Tools.h>
#include <Source/TextPos.h>

#include "TextDoc.h"

// ----------------------------------------------------------------------------
TextDoc::TextDoc()
: Version(0)
{
}

// ----------------------------------------------------------------------------
void TextDoc::setContent(const std::vector<char*>& lines)
{
  Lines.clear();
  Lines.resize(lines.size());

  size_t idx = 0;
  for (auto line : lines) {
    Lines[idx++] = line;
  }

  Version++;
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
const std::string& TextDoc::getLine(size_t idx) const
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
    const std::string& line = getLine(i);

    if (i > 0) {
      text.push_back('\n');
    }

    text.append(line);
  }

  return text;
}

// ----------------------------------------------------------------------------
void TextDoc::insertChars(size_t line, size_t col, const char* ch)
{
  Lines.at(line).insert(col, ch);
  Version++;
}

// ----------------------------------------------------------------------------
void TextDoc::insertNewLine(size_t line, size_t col, size_t indent)
{
  std::string l1 = std::string(indent, ' ') + Lines.at(line).substr(col);
  Lines.insert(Lines.begin() + line + 1, l1);

  Lines[line].erase(col);
  Version++;
}

// ----------------------------------------------------------------------------
void TextDoc::insertNewLineBefore(size_t line, size_t indent)
{
  std::string l1 = std::string(indent, ' ');
  Lines.insert(Lines.begin() + line, l1);
  Version++;
}

// ----------------------------------------------------------------------------
void TextDoc::deleteChar(size_t line, size_t col)
{
  if (col < Lines.at(line).length()) {
    Lines[line].erase(col, 1);
    Version++;
  }
  else if (line < Lines.size()) {
    Lines[line].append(Lines[line + 1]);
    Lines.erase(Lines.begin() + line + 1);
    Version++;
  }
}

// ----------------------------------------------------------------------------
void TextDoc::deleteLine(size_t line)
{
  if (line < Lines.size() && Lines.size() > 1) {
    Lines.erase(Lines.begin() + line);
    Version++;
  }
}

// ----------------------------------------------------------------------------
void TextDoc::deleteLines(size_t first, size_t last)
{
  if (first < Lines.size() && Lines.size() > last-first+1) {
    Lines.erase(Lines.begin() + first, Lines.begin() + last);
    Version++;
  }
}

// ----------------------------------------------------------------------------
void TextDoc::deleteRangeInLine(size_t line, size_t col0, size_t col1)
{
  std::string& text = Lines.at(line);
  text.erase(col0, col1-col0);
  Version++;
}

// ----------------------------------------------------------------------------
TextDoc TextDoc::getContent(const TextPos& p0, const TextPos& p1) const
{
  TextDoc doc;

  for (size_t i = p0.line; i <= p1.line; i++)
  {
    std::string line = Lines.at(i);

    if (i == p0.line) {
      line = line.substr(p0.column, std::string::npos);
    }

    if (i == p1.line) {
      line = line.substr(0, p1.column);
    }

    doc.Lines.push_back(line);
  }

  return doc;
}

