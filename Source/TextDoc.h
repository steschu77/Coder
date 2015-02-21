/*!
** \file    Source/TextDoc.h
** \brief   Low level text editing support.
\*****************************************************************************/
#pragma once

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

  void insertChars(size_t line, size_t col, const char* ch);
  void insertNewLine(size_t line, size_t col, size_t indent=0);
  void insertNewLineBefore(size_t line, size_t indent=0);
  void deleteChar(size_t line, size_t col);
  void deleteLine(size_t line);
  void deleteLines(size_t line, size_t count);
  void deleteRangeInLine(size_t line, size_t col0, size_t col1);
};
