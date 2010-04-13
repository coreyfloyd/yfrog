// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit
// use ATL CString
#define _WTL_NO_CSTRING
#define _WTL_NO_WTYPES

// turn on loggin to file
#define _USE_LOGGING
// Current project name
#define _PROJECT_NAME	_T("ImageShackAPI")
// Http user agent name for API
#define HTTP_USER_AGENT	_T("ImageShackAPI")
// Config Registry Key Name
#define _PROJECT_REGKEY	_T("ImageShackAPI")

#pragma warning (disable: 4251)
#pragma warning (disable: 4995)
#pragma warning (disable: 4996)
#pragma warning (disable: 4244)
#pragma warning (disable: 4267)
#pragma warning (disable: 4312)
#pragma warning (disable: 4800)
#pragma warning (disable: 4311)


#include "../res/resource.h"
#include "resourceDLL.h"
#include <atlbase.h>
#include <atlcom.h>
#include <atlwin.h>
#include <atlutil.h>
#include <atlcomcli.h>
#include <atlstr.h>
#include <strsafe.h>
#include <atltypes.h>
#include <atlsafe.h>

#include <shellapi.h>
#include <shlobj.h>

// WTL
#include <atlapp.h>
#include <atlframe.h>
#include <atlcrack.h>
#include <atlctrls.h>
#include <atlctrlx.h>
#include <atlddx.h>
#include <atlmisc.h>

using namespace ATL;

#include "atlcrackplus.h"

#include "API/Win32/File.h"
#include "API/Win32/RegKey.h"
#include "API/Win32/Global.h"
#include "API/Win32/Process.h"
#include "API/Win32/Internet.h"
#include "API/Win32/Shell.h"
#include "API/Win32/DragAndDrop.h"
#include "API/Win32/Thread.h"
using namespace API::Win32;
#include "templates/SmartPtr.h"
using namespace TEMPL;

#include "Logger.h"
#include "Util.h"
using namespace UTIL;

#include "Configuration.h"

#if defined _M_IX86
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
