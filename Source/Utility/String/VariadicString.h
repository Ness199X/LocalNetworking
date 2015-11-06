/*
    Initial author: (https://github.com/)Convery
    License: LGPL 3.0
    Started: 2015-10-22
    Notes:
        Creates a string from variadic arguments.
*/

#pragma once
#include <mutex>
#include <stdarg.h>

static char VariadicBuffer[4][8192];
static uint32_t NextVariadicBufferIndex;
static std::mutex vaThreadSafe;

inline const char *va(const char *fmt, ...)
{
    va_list VarArgs;
    int32_t StringLength = 0;
    char *DestinationBuffer = nullptr;

    vaThreadSafe.lock();
    DestinationBuffer = &VariadicBuffer[NextVariadicBufferIndex][0];
    memset(DestinationBuffer, 0, 8192);							        // Clear any old data..
    NextVariadicBufferIndex = (NextVariadicBufferIndex + 1) % 4;		// Mod by buffercount.

    va_start(VarArgs, fmt);
    StringLength = _vsnprintf_s(DestinationBuffer, 8192, _TRUNCATE, fmt, VarArgs);
    va_end(VarArgs);

    vaThreadSafe.unlock();
    return DestinationBuffer;
}
