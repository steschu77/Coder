#include <Source/Headers.h>
#include "History.h"

// ============================================================================
EditCommand::EditCommand(uint type, const TextPos& p0, const TextPos& p1, const TextDoc& doc)
: type(type)
, p0(p0)
, p1(p1)
, content(doc)
{
}

// ============================================================================
void History::addNewCommand(const EditCommand& cmd)
{
  _Redo.clear();
  _Undo.push_back(cmd);
}

// ----------------------------------------------------------------------------
EditCommand History::undoCommand()
{
  EditCommand cmd = _Undo.back();
  _Redo.push_back(cmd);
  _Undo.pop_back();

  return cmd;
}

// ----------------------------------------------------------------------------
EditCommand History::redoCommand()
{
  EditCommand cmd = _Redo.back();
  _Undo.push_back(cmd);
  _Redo.pop_back();

  return cmd;
}
