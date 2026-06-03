//
// linux_compat.h -- Windows/MSVC compatibility shim for building Heretic2R on Linux.
//
// Force-included (via -include) before every translation unit by the CMake build.
// Provides the MSVC "secure CRT" (*_s) string functions, the handful of Win32 types
// and APIs the engine references outside its platform layer, and maps MSVC keywords
// (__declspec, __forceinline, ...) onto their GCC/Clang equivalents.
//
// Copyright 2026 (Linux port)
//

#pragma once

#ifndef _WIN32

#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>   // strcasecmp / strncasecmp
#include <ctype.h>
#include <limits.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dlfcn.h>

// ---------------------------------------------------------------------------
// MSVC keywords / calling conventions.
// ---------------------------------------------------------------------------

// __declspec(dllexport) / dllimport / noreturn / align(n).
// On a Linux .so all extern symbols are exported by default, so dllexport/dllimport
// reduce to a default-visibility no-op; noreturn maps to the GCC attribute.
#define H2_DECLSPEC_dllexport __attribute__((visibility("default")))
#define H2_DECLSPEC_dllimport
#define H2_DECLSPEC_noreturn  __attribute__((noreturn))
#define __declspec(x)         H2_DECLSPEC_##x

#ifndef WINAPI
#define WINAPI
#endif
#ifndef APIENTRY
#define APIENTRY
#endif
#define __cdecl
#define __fastcall
#define __stdcall
#define __forceinline inline
#ifndef __inline
#define __inline inline
#endif
#ifndef _inline
#define _inline inline
#endif

// ---------------------------------------------------------------------------
// Win32 types / handles referenced in cross-platform engine code.
// ---------------------------------------------------------------------------

typedef void*          HINSTANCE;
typedef void*          HMODULE;
typedef void*          HANDLE;
typedef void*          HWND;
typedef int            BOOL;
typedef uint32_t       DWORD;
typedef unsigned char  BYTE;
typedef unsigned short WORD;
typedef const char*    LPCSTR;
typedef char*          LPSTR;
typedef void*          LPVOID;

#ifndef MB_OK
#define MB_OK          0x0
#define MB_ICONWARNING 0x0
#define MB_ICONERROR   0x0
#endif

// ---------------------------------------------------------------------------
// Win32 APIs used outside the platform layer.
// ---------------------------------------------------------------------------

// Dynamic library loading (see src/linux/dll_unix.c). LoadLibrary appends/translates
// the ".so" extension; FreeLibrary returns non-zero on success (opposite of dlclose).
HINSTANCE Sys_dlopen(const char* name);
int       Sys_dlclose(void* handle);

// Windows multimedia timer (ms since startup); implemented in src/linux/q_shunix.c.
unsigned int timeGetTime(void);

#define LoadLibrary(name)         Sys_dlopen(name)
#define FreeLibrary(h)            Sys_dlclose(h)
#define GetProcAddress(h, name)   dlsym((h), (name))

#define MessageBox(hwnd, text, caption, type) \
    ((void)fprintf(stderr, "[%s] %s\n", (caption), (text)), 0)
#define OutputDebugString(s)      ((void)fputs((s), stderr))

#define _mkdir(path)              mkdir((path), 0755)

// ---------------------------------------------------------------------------
// MSVC "secure CRT" string/IO functions (truncating, bounds-checked variants).
// Return 0 on success like the originals; callers that ignore the result are fine.
// ---------------------------------------------------------------------------

static inline int h2_strcpy_s(char* dst, size_t size, const char* src)
{
    if (dst == NULL || size == 0) return 22; // EINVAL
    if (src == NULL) { dst[0] = '\0'; return 22; }
    size_t len = strlen(src);
    if (len >= size) { dst[0] = '\0'; return 34; } // ERANGE
    memcpy(dst, src, len + 1);
    return 0;
}

static inline int h2_strncpy_s(char* dst, size_t size, const char* src, size_t count)
{
    if (dst == NULL || size == 0) return 22;
    if (src == NULL) { dst[0] = '\0'; return 22; }
    size_t len = strnlen(src, count);
    if (len >= size) { dst[0] = '\0'; return 34; }
    memcpy(dst, src, len);
    dst[len] = '\0';
    return 0;
}

static inline int h2_strcat_s(char* dst, size_t size, const char* src)
{
    if (dst == NULL || size == 0 || src == NULL) return 22;
    size_t dlen = strnlen(dst, size);
    if (dlen == size) return 22; // not null-terminated
    size_t slen = strlen(src);
    if (dlen + slen >= size) return 34;
    memcpy(dst + dlen, src, slen + 1);
    return 0;
}

static inline int h2_strncat_s(char* dst, size_t size, const char* src, size_t count)
{
    if (dst == NULL || size == 0 || src == NULL) return 22;
    size_t dlen = strnlen(dst, size);
    if (dlen == size) return 22;
    size_t slen = strnlen(src, count);
    if (dlen + slen >= size) return 34;
    memcpy(dst + dlen, src, slen);
    dst[dlen + slen] = '\0';
    return 0;
}

static inline int h2_memcpy_s(void* dst, size_t dstsize, const void* src, size_t count)
{
    if (dst == NULL) return 22;
    if (count == 0) return 0;
    if (src == NULL || count > dstsize) { memset(dst, 0, dstsize); return 22; }
    memcpy(dst, src, count);
    return 0;
}

static inline int h2_fopen_s(FILE** f, const char* name, const char* mode)
{
    if (f == NULL) return 22;
    *f = fopen(name, mode);
    return (*f != NULL) ? 0 : 2; // ENOENT-ish
}

static inline int h2_memmove_s(void* dst, size_t dstsize, const void* src, size_t count)
{
    if (dst == NULL) return 22;
    if (count == 0) return 0;
    if (src == NULL || count > dstsize) { memset(dst, 0, dstsize); return 22; }
    memmove(dst, src, count);
    return 0;
}

// MSVC fread_s(buf, bufsize, elemsize, count, stream) returns elements read, like fread.
static inline size_t h2_fread_s(void* buf, size_t bufsize, size_t elemsize, size_t count, FILE* stream)
{
    (void)bufsize;
    return fread(buf, elemsize, count, stream);
}

#ifdef __cplusplus
// C++: MSVC's secure-CRT functions have array-deducing template overloads, e.g.
// strcpy_s(dst_array, src) with the size inferred. Provide those plus the explicit
// (ptr, size, ...) overloads as functions, not macros, so both call forms resolve.
static inline int strcpy_s(char* d, size_t n, const char* s) { return h2_strcpy_s(d, n, s); }
template<size_t N> static inline int strcpy_s(char (&d)[N], const char* s) { return h2_strcpy_s(d, N, s); }

static inline int strncpy_s(char* d, size_t n, const char* s, size_t c) { return h2_strncpy_s(d, n, s, c); }
template<size_t N> static inline int strncpy_s(char (&d)[N], const char* s, size_t c) { return h2_strncpy_s(d, N, s, c); }

static inline int strcat_s(char* d, size_t n, const char* s) { return h2_strcat_s(d, n, s); }
template<size_t N> static inline int strcat_s(char (&d)[N], const char* s) { return h2_strcat_s(d, N, s); }

static inline int strncat_s(char* d, size_t n, const char* s, size_t c) { return h2_strncat_s(d, n, s, c); }
template<size_t N> static inline int strncat_s(char (&d)[N], const char* s, size_t c) { return h2_strncat_s(d, N, s, c); }

static inline int fopen_s(FILE** f, const char* n, const char* m) { return h2_fopen_s(f, n, m); }
static inline int memcpy_s(void* d, size_t dn, const void* s, size_t c) { return h2_memcpy_s(d, dn, s, c); }
static inline int memmove_s(void* d, size_t dn, const void* s, size_t c) { return h2_memmove_s(d, dn, s, c); }
static inline size_t fread_s(void* b, size_t bs, size_t es, size_t c, FILE* f) { return h2_fread_s(b, bs, es, c, f); }

static inline int sprintf_s(char* d, size_t n, const char* fmt, ...)
{ va_list ap; va_start(ap, fmt); int r = vsnprintf(d, n, fmt, ap); va_end(ap); return r; }
template<size_t N, typename... A> static inline int sprintf_s(char (&d)[N], const char* fmt, A... a)
{ return snprintf(d, N, fmt, a...); }

static inline int vsprintf_s(char* d, size_t n, const char* fmt, va_list ap) { return vsnprintf(d, n, fmt, ap); }
template<size_t N> static inline int vsprintf_s(char (&d)[N], const char* fmt, va_list ap) { return vsnprintf(d, N, fmt, ap); }

#define _snprintf   snprintf
#define _vsnprintf  vsnprintf
#else
#define strcpy_s(dst, size, src)        h2_strcpy_s((dst), (size), (src))
#define strncpy_s(dst, size, src, cnt)  h2_strncpy_s((dst), (size), (src), (cnt))
#define strcat_s(dst, size, src)        h2_strcat_s((dst), (size), (src))
#define strncat_s(dst, size, src, cnt)  h2_strncat_s((dst), (size), (src), (cnt))
#define fopen_s(fp, name, mode)         h2_fopen_s((fp), (name), (mode))
#define memcpy_s(dst, dsize, src, cnt)  h2_memcpy_s((dst), (dsize), (src), (cnt))
#define memmove_s(dst, dsize, src, cnt) h2_memmove_s((dst), (dsize), (src), (cnt))
#define fread_s(buf, bsz, esz, cnt, fp) h2_fread_s((buf), (bsz), (esz), (cnt), (fp))

// Formatted output: MSVC's *_s variants truncate and null-terminate like snprintf.
#define sprintf_s(buf, size, ...)       snprintf((buf), (size), __VA_ARGS__)
#define _snprintf_s(buf, size, cnt, ...) snprintf((buf), (size), __VA_ARGS__)
#define vsprintf_s(buf, size, fmt, ap)  vsnprintf((buf), (size), (fmt), (ap))
#define vsnprintf_s(buf, size, cnt, fmt, ap) vsnprintf((buf), (size), (fmt), (ap))
#define _vsnprintf                      vsnprintf
#define _snprintf                       snprintf
#endif

// All sscanf_s uses in the tree are numeric (%x/%u/%i/%f) with no %s/%c/%[,
// so the MSVC size arguments are absent and a direct mapping is safe.
#define sscanf_s                        sscanf

// strtok_s(str, delim, ctx) is identical to POSIX strtok_r.
#define strtok_s(str, delim, ctx)       strtok_r((str), (delim), (ctx))

// localtime_s(result, time) <-> POSIX localtime_r(time, result) (arguments swapped).
#define localtime_s(tm_out, time_in)    (localtime_r((time_in), (tm_out)) ? 0 : 22)
#define gmtime_s(tm_out, time_in)       (gmtime_r((time_in), (tm_out)) ? 0 : 22)

// Misc MSVC CRT spellings.
// MSVC provides these as macros via <stdlib.h>/<windef.h>; the engine uses them.
// NOT in C++: the function-like macros would wreck STL headers (std::min, etc.).
// C++ translation units should use std::min/std::max instead.
#ifndef __cplusplus
#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif
#endif

#define _stricmp                        strcasecmp
#define stricmp                         strcasecmp
#define _strnicmp                       strncasecmp
#define strnicmp                        strncasecmp
#define _strdup                         strdup

#endif // !_WIN32
