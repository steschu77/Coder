/*!
** \file    /Source/WinCoder.h
** \brief   The WinFactory class provides a base implementation for window
**          creation on Win32 systems.
**
**          This file is a Windows specific implementation.
******************************************************************************/
#pragma once

// ============================================================================
extern HINSTANCE ghInstance;

// ============================================================================
class BaseWinProc
// ----------------------------------------------------------------------------
{
  friend class WinFactory;

public:
  HWND hwnd() const;

protected:
  virtual LRESULT proc(HWND hwnd, UINT cmd, WPARAM wParam, LPARAM lParam) = 0;

  HWND _hWnd;
};

// ----------------------------------------------------------------------------
inline HWND BaseWinProc::hwnd() const
{
  return _hWnd;
}

// ============================================================================
class WinFactory
// ----------------------------------------------------------------------------
{
protected:
  void _registerClass(const wchar* ClassName, HBRUSH hbrBackground=nullptr, const char* strIcon=nullptr, const char* Cursor=IDC_ARROW);

private:
  static LRESULT CALLBACK __procStatic1(HWND hwnd, UINT cmd, WPARAM wParam, LPARAM lParam);
  static LRESULT CALLBACK __procStaticN(HWND hwnd, UINT cmd, WPARAM wParam, LPARAM lParam);
};
