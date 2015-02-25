/*!
** \file    Source/TextDoc.h
** \brief   Low level text editing support using std::vector<std::string>.
\*****************************************************************************/
#pragma once

// ============================================================================
struct TextPos;

// ============================================================================
class TextDoc
{
  friend class EditableTextDoc;

public:
  TextDoc();
  explicit TextDoc(const char* ch);
  explicit TextDoc(size_t cLines);

  size_t getLineCount() const;
  size_t getLineLength(size_t line) const;
  size_t getMaxLineLength() const;

  TextPos getEndOfDoc() const;
  TextPos getNextPos(const TextPos& Pos) const;
  TextPos getPrevPos(const TextPos& Pos) const;

  char getCharAt(const TextPos& pos) const;
  const std::string& getLineAt(size_t line) const;
  
  std::string exportContent() const;

  TextDoc getContent(const TextPos& p0, const TextPos& p1) const;

  std::vector<std::string> Lines;
};

// ============================================================================
class EditableTextDoc : public TextDoc
{
public:
  EditableTextDoc();

  void replaceContent(char* lines, size_t Length);

  void insertContent(const TextPos& p, const TextDoc& Content);
  void deleteContent(const TextPos& p0, const TextPos& p1);

  uint getVersion() const;

private:
  uint Version;
};

// ----------------------------------------------------------------------------
inline uint EditableTextDoc::getVersion() const
{
  return Version;
}

EditableTextDoc loadTextDoc(const char* Path);
void saveTextDoc(const char* Path, const TextDoc& doc);
