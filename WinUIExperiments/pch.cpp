#include "pch.h"

void DebugPrint(const PCWSTR message, ...)
{
#ifdef _DEBUG
    WCHAR buffer[256] = {0};
    va_list args;
    va_start(args, message);
    StringCchVPrintf(buffer, ARRAYSIZE(buffer), message, args);
    va_end(args);
    OutputDebugString(buffer);
#endif
}

HINSTANCE g_instance = NULL;