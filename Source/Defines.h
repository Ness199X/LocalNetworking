/*
    Initial author: (https://github.com/)Convery
    License: LGPL 3.0
    Started: 2015-11-01
    Notes:
        General defines to save our santiy.
*/

#pragma once

// VS2015 no longer defines NDEBUG in release mode.
#ifdef _MSC_BUILD
#ifndef _DEBUG
#define NDEBUG
#endif
#endif
