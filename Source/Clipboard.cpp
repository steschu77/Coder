#include <Source/Headers.h>
#include <Source/UTF8Tools.h>

#include "ClipBoard.h"

// ============================================================================
retcode copyTextToClipboard(HWND hwnd, const TextDoc& doc)
{
  if (!OpenClipboard(hwnd)) {
    return rcFailed;
  }

  EmptyClipboard();

  std::string text = doc.exportContent();
  size_t cLength = text.length() + 1;
  
  // Allocate a global memory object for the text.
  HGLOBAL hglbCopy = GlobalAlloc(GMEM_MOVEABLE, cLength * sizeof(char));
  if (hglbCopy == nullptr) {
    CloseClipboard();
    return rcFailed;
  }

  // Lock the handle and copy the text to the buffer.
  void* strCopy = GlobalLock(hglbCopy);
  memcpy(strCopy, text.c_str(), cLength * sizeof(char));

  GlobalUnlock(hglbCopy);

  // Place the handle on the clipboard.
  SetClipboardData(CF_TEXT, hglbCopy);

  CloseClipboard();
  return rcSuccess;
}

// ----------------------------------------------------------------------------
retcode pasteTextFromClipboard(HWND hwnd, TextDoc* pDoc)
{  
  if (!IsClipboardFormatAvailable(CF_TEXT)) {
    return rcFailed;
  }
  
  if (!OpenClipboard(hwnd)) {
    return rcFailed;
  }
}
