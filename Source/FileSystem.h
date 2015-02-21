/*!
** \file    /Source/FileSystem.h
** \brief   File System Helper functions
******************************************************************************/
#pragma once

// ----------------------------------------------------------------------------
//! Creates all directories needed to access 'path'.
retcode createPath(const char* Path);

// ----------------------------------------------------------------------------
//! Returns true if file exists.
bool fileExists(const char* File);

// ----------------------------------------------------------------------------
//! Splits the Path into Dir/Name.Ext parts
//! pDir, pName, pExt can be nullptr if cDir, cName, cExt are 0 accordingly.
retcode splitFilePath(const char* Path, char* pDir, size_t cDir,
  char* pName, size_t cName, char* pExt, size_t cExt);
retcode splitFilePath(const char* Path, char* pDir, size_t cDir,
  char* pName, size_t cName);

// ----------------------------------------------------------------------------
//! Returns the file extension
inline retcode getFileExtension(const char* Path, char* pExt, size_t cExt)
{
  return splitFilePath(Path, nullptr, 0, nullptr, 0, pExt, cExt);
}

// ----------------------------------------------------------------------------
//! Returns the directory to the file
inline retcode getFileDirectory(const char* Path, char* pDir, size_t cDir)
{
  return splitFilePath(Path, pDir, cDir, nullptr, 0, nullptr, 0);
}
