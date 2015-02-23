/*!
** \file    Source/History.h
** \brief   
\*****************************************************************************/
#pragma once

struct TextPos;
class TextDoc;

#include <Source/TextPos.h>
#include <Source/TextDoc.h>

// ============================================================================
enum EditCommandType
{
  cmdDelete,
  cmdInsert,
};

// ============================================================================
struct EditCommand
{
  EditCommand(uint type, const TextPos& p0, const TextPos& p1, const TextDoc& doc);
  
  uint type;
  TextPos p0, p1;
  TextDoc content;
};

// ============================================================================
class History
{
public:
  History() {}

  History(const History&) = delete;
  History operator = (History) = delete;

  void addNewCommand(const EditCommand& cmd);
  
  EditCommand undoCommand();
  EditCommand redoCommand();
  
  size_t countUndoCommands() const;
  size_t countRedoCommands() const;
  
private:
  std::vector<EditCommand> _Undo;
  std::vector<EditCommand> _Redo;
};

// ----------------------------------------------------------------------------
inline size_t History::countUndoCommands() const
{
  return _Undo.size();
}

// ----------------------------------------------------------------------------
inline size_t History::countRedoCommands() const
{
  return _Redo.size();
}
