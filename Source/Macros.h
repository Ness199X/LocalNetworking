/*
    Initial author: (https://github.com/)Convery
    License: LGPL 3.0
    Started: 2015-11-01
    Notes:
        General macros to save our santiy.
*/

#pragma once
#include "Defines.h"
#include "Utility\String\DebugString.h"

// Debug information.
#ifdef NDEBUG
#define PrintFunction()
#define DebugPrint(string)
#define InfoPrint(string)
#else
#define PrintFunction() AppendToLogfile(__func__, "Func")
#define DebugPrint(string) AppendToLogfile(string, "Debug")
#define InfoPrint(string) AppendToLogfile(string, "Info")
#define NetworkPrint(string) AppendToLogfile(string, "Net")
#endif
