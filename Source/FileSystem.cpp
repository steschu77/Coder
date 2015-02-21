#include <Source/Headers.h>
#include <Source/UTF8Tools.h>

#include "FileSystem.h"

// ============================================================================
retcode createPath(const char *Path)
{
  wchar SubDir[MaxPathLength];
  UTF8ToUTF16(SubDir, MaxPathLength, Path);

  wchar *Work = SubDir;

  for (; wcslen(Work) > 1;)
  {
    wchar *sFSlash = wcschr(Work, '/');
    wchar *sBSlash = wcschr(Work, '\\');
    if (sFSlash == nullptr && sBSlash == nullptr) {
      return rcInvalidFormat;
    }

    wchar *sSlash = (sFSlash == nullptr) ? sBSlash : ((sBSlash == nullptr) ? sFSlash : std::min(sFSlash, sBSlash));
    if (sSlash == Work || sSlash[-1] == ':')
    {
      Work = sSlash + 1;
      continue;
    }

    Work = sSlash + 1;
    sSlash[0] = 0;

    if (!CreateDirectoryW(SubDir, NULL))
    {
      DWORD errorCode = GetLastError();
      if (errorCode != ERROR_ALREADY_EXISTS) {
        return rcFailed;
      }
    }

    sSlash[0] = '\\';
  }

  return rcSuccess;
}

// ----------------------------------------------------------------------------
bool fileExists(const char *File)
{
  wchar Path[MaxPathLength];
  UTF8ToUTF16(Path, MaxPathLength, File);

  HANDLE h = CreateFileW(Path, GENERIC_READ, FILE_SHARE_READ,
    NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );

  if (h == INVALID_HANDLE_VALUE) {
    uint code = GetLastError();
    return (code == ERROR_FILE_NOT_FOUND || code == ERROR_PATH_NOT_FOUND);
  }

  CloseHandle(h);
  return true;
}

// ----------------------------------------------------------------------------
retcode splitFilePath(const char *Path, char *pDir, size_t cDir,
  char *pName, size_t cName, char *pExt, size_t cExt)
{
  if (Path == nullptr) {
    return rcInvalidPointer;
  }

  const char *Name = std::max(strrchr(Path, '/'), strrchr(Path, '\\'));
  const char *Ext  = strrchr(Path, '.');

  if (Name == nullptr) {
    // No '/' -> no directory given
    Name = Path;
  } else {
    // Name doesn't include the '/'
    Name++;
  }

  if (Ext < Name) {
    // '.' found is part of directory
    Ext = nullptr;
  }

  if (Ext == nullptr) {
    // no extension found, inform user if he wants the extension
    if ((pExt != nullptr) && (cExt > 0)) {
      *pExt = 0;
    }
  } else {
    strcpy_s(pExt, cExt, Ext);
  }

  size_t DirLen = std::min<size_t>(cDir, Name-Path);
  if (pDir != nullptr) {
    strncpy_s(pDir, cDir, Path, DirLen);
  }

  size_t NameLen = std::min<size_t>(cName, Ext-Name);
  if (pName != nullptr) {
    strncpy_s(pName, cName, Name, NameLen);
  }

  return rcSuccess;
}

// ----------------------------------------------------------------------------
retcode splitFilePath(const char *Path,
  char *pDir, size_t cDir,
  char *pName, size_t cName)
{
  if (Path == nullptr) {
    return rcInvalidPointer;
  }

  const char *Length = Path + strlen(Path);
  const char *Name = std::max(strrchr(Path, '/'), strrchr(Path, '\\'));

  if (Name == nullptr) {
    // No '/' -> no directory given
    Name = Path;
  } else {
    // Name doesn't include the '/'
    Name++;
  }

  if (pDir != nullptr) {
    size_t DirLen = std::min<size_t>(cDir, Name-Path);
    strncpy_s(pDir, cDir, Path, DirLen);
  }

  if (pName != nullptr) {
    size_t NameLen = std::min<size_t>(cName, Length-Name);
    strncpy_s(pName, cName, Name, NameLen);
  }

  return rcSuccess;
}
