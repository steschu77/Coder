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
  explicit TextDoc(const char* ch);
  explicit TextDoc(size_t cLines);

  std::vector<std::string> Lines;
  uint Version;

  void setContent(char* lines, size_t Length);
  void setContent(const std::vector<char*>& lines);

  size_t getLineCount() const;
  size_t getLineLength(size_t idx) const;
  size_t getMaxLineLength() const;
  const std::string& getLine(size_t idx) const;
  std::string exportContent() const;

  void insertChars(const TextPos& pos, const char* ch);
  void insertNewLine(const TextPos& pos, size_t indent=0);
  void insertNewLineBefore(size_t line, size_t indent=0);
  void deleteChar(size_t line, size_t col);
  void deleteLine(size_t line);
  void deleteLines(size_t line, size_t count);
  void deleteRangeInLine(size_t line, size_t col0, size_t col1);

  TextDoc getContent(const TextPos& p0, const TextPos& p1) const;
  void insertContent(const TextPos& p, const TextDoc& Content);

  void deleteContent(const TextPos& p0, const TextPos& p1);

  TextPos getEndOfDoc() const;
  TextPos getNextPos(const TextPos& Pos) const;
  TextPos getPrevPos(const TextPos& Pos) const;

  char getCharAt(const TextPos& pos) const;
};
