// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // 从 Windows 头中排除极少使用的资料
// Windows 头文件: 
#include <windows.h>

#ifdef _DEBUG
#pragma push_macro("new")
#undef new
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)  
#endif

// C 运行时头文件
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include <intrin.h>
#include <utility>
#include <future>

#include <vector>
#include <list>
#include <map>
#include <unordered_map>
#include <set>
#include <regex>
#include <algorithm>
#include <bitset>
#include <numeric>
#include <utility>

#include <ctime>

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

#include <crtdbg.h>

#ifdef _DEBUG
#pragma pop_macro("new")
#endif

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // 某些 CString 构造函数将是显式的

#include <atlbase.h>
#include <atlstr.h>

// TODO:  在此处引用程序需要的其他头文件

#include "base/defines.h"
#include "lua/lua.hpp"

#include <Shlwapi.h>
#include <ShlObj.h>
#include <math.h>
#include <winhttp.h>
#include <WinCodec.h>
#include <Commdlg.h>
#include <MMSystem.h>
#include <Vfw.h>
#include <d2d1.h>
#include <dwrite.h>
#include <usp10.h>
#include <gdiplus.h>
#include <initguid.h>
#include <dxgi.h>

#pragma comment(lib, "Gdiplus")
#pragma comment(lib, "WindowsCodecs")
#pragma comment(lib, "Msimg32")
#pragma comment(lib, "WinHttp")
#pragma comment(lib, "d2d1")
#pragma comment(lib, "dwrite")
#pragma comment(lib, "Comdlg32")
#pragma comment(lib, "Vfw32")
#pragma comment(lib, "imm32")
#pragma comment(lib, "dxgi")
#pragma comment(lib, "winmm")

#pragma comment(lib, "lib\\libevent")
#pragma comment(lib, "lib\\libevent_core")
#pragma comment(lib, "lib\\libevent_extras")
#pragma comment(lib, "lib\\libcurl")