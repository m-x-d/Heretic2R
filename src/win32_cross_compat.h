#pragma once

#ifdef _WIN32
#ifndef _MSC_VER // MinGW

#define _WIN32_WINNT 0x0601
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

// MSVC keywords / calling conventions
#ifndef _inline
#define _inline static inline
#endif
#ifndef __inline
#define __inline static inline
#endif

#ifndef __cplusplus
#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif
#endif

// Secure CRT stubs for MinGW
#ifndef fread_s
#define fread_s(buf, bsz, esz, cnt, fp) fread((buf), (esz), (cnt), (fp))
#endif

// MinGW doesn't have VersionHelpers.h sometimes or we want to avoid it
#ifndef IsWindows7OrGreater
#define IsWindows7OrGreater() 1
#endif

#endif // !_MSC_VER
#endif // _WIN32
