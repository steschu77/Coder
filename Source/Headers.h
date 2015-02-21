/*!
** \file    Headers.h
** \brief   Collection of header files that do not change. They are compiled
**          as pre-compiled headers for platforms that support it.
\*****************************************************************************/
#pragma once

#include <stdio.h>
#include <memory.h>

#include <cstdlib>
#include <cstring>
#include <cassert>
#include <cstdarg>
#include <cstddef>
#include <ctype.h>

#include <string>
#include <vector>
#include <map>
#include <set>
#include <list>
#include <queue>
#include <algorithm>
#include <sstream>

#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>

#include <iostream>

#if defined(_MSC_VER)
#pragma warning(disable: 4091 4100 4996)

// do not include rare used Win32 Headers, but include WinXP stuff
#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0501

#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <shellapi.h>

#undef min
#undef max

#endif // _MSC_VER

#if defined(__GNUC__)

#include <sys/time.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>

#include <unistd.h>
#include <errno.h>
#include <semaphore.h>

#endif // __GNUC__

#include <Source/System.h>
