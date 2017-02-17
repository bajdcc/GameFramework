#ifndef DEFINES_H
#define DEFINES_H

using cint8 = signed __int8;
using uint8 = unsigned __int8;
using cint16 = signed __int16;
using uint16 = unsigned __int16;
using cint32 = signed __int32;
using uint32 = unsigned __int32;
using cint64 = signed __int64;
using uint64 = unsigned __int64;

#ifdef WIN32
using cint = cint32;
using uint = uint32;
#else
using cint = cint64;
using uint = uint64;
#endif

using byte = uint8;
using size_t = uint;

#ifndef WM_SYSTIMER
#define WM_SYSTIMER 0x0118 //(caret blink)
#endif

#endif