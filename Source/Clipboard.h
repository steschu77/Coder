/*!
** \file    Source/ClipBoard.h
** \brief
\*****************************************************************************/
#pragma once

#include <Source/TextDoc.h>

// ============================================================================
retcode copyTextToClipboard(HWND hwnd, const TextDoc& doc);
retcode pasteTextFromClipboard(HWND hwnd, EditableTextDoc* pDoc);
