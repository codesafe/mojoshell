#ifndef _TRACE___
#define _TRACE___


#include <windows.h>
#include <tchar.h>
#include <wchar.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <string>
#include <crtdbg.h>

typedef wchar_t wchar;

#if 1

inline void _TRACE(const wchar *fmt, ...)
{
	wchar temp[1024];
	va_list ap;
	int r;
	va_start(ap, fmt);
	r = vswprintf(temp, _countof(temp), fmt, ap);
	va_end(ap);
	OutputDebugString(temp);
}

extern _declspec(thread) wchar g_TraceBuf[2048];

#define TRACE_TIME_FORMAT     _T("%02d:%02d:%02d.%03d")

inline const wchar* GetTimeString()
{
	_declspec(thread) static wchar s_szBuf[64];
	SYSTEMTIME time;
	GetLocalTime(&time);
	_snwprintf_s(s_szBuf, 64, _TRUNCATE, TRACE_TIME_FORMAT, time.wHour, time.wMinute, time.wSecond, time.wMilliseconds);

	return s_szBuf;
}

#endif


#endif