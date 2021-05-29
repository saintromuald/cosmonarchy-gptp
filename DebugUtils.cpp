#include "DebugUtils.h"

void WINAPIV DebugOut(const TCHAR* fmt, ...)
{
	TCHAR s[1025];
	va_list args;
	va_start(args, fmt);
	wvsprintf(s, fmt, args);
	va_end(args);
	OutputDebugString(s);
	OutputDebugString("\n");
}