/*!
** \file    Source/TextDoc.h
** \brief   Low level text editing support using std::vector<std::string>.
\*****************************************************************************/
#pragma once

// ============================================================================
struct TextPos;

// ============================================================================
struct TextDoc
{
  TextDoc();

  std::vector<std::string> Lines;
  uint Version;

  void setContent(const std::vector<char*>& lines);

  size_t getLineCount() const;
  size_t getLineLength(size_t idx) const;
  size_t getMaxLineLength() const;
  const std::string& getLine(size_t idx) const;
  std::string exportContent() const;

  void insertChars(size_t line, size_t col, const char* ch);
  void insertNewLine(size_t line, size_t col, size_t indent=0);
  void insertNewLineBefore(size_t line, size_t indent=0);
  void deleteChar(size_t line, size_t col);
  void deleteLine(size_t line);
  void deleteLines(size_t line, size_t count);
  void deleteRangeInLine(size_t line, size_t col0, size_t col1);

  TextDoc getContent(const TextPos& p0, const TextPos& p1) const;
};
