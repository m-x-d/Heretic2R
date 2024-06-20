//
// Common.c -- The new home of COM_ functions!
//
// Copyright 1998 Raven Software
//

#include "q_shared.h"
#include "Console.h"

// Q2 counterpart
H2COMMON_API char* COM_SkipPath(char* pathname)
{
	char* last = pathname;

	while (*pathname)
	{
		if (*pathname == '/')
			last = pathname + 1;

		pathname++;
	}

	return last;
}

// Q2 counterpart
H2COMMON_API void COM_StripExtension(const char* in, char* out)
{
	while (*in && *in != '.')
		*out++ = *in++;

	*out = 0;
}

// Q2 counterpart, replaced with YQ2 version //TODO: not included in H2 toolkit headers. Included where?
H2COMMON_API const char* COM_FileExtension(const char* in)
{
	const char* ext = strrchr(in, '.');

	if (!ext || ext == in)
		return "";

	return ext + 1;
}

// Q2 counterpart
H2COMMON_API void COM_FileBase(const char* in, char* out)
{
	const char* s = in + strlen(in) - 1;
	while (s != in && *s != '.')
		s--;

	const char* s2 = s;
	while (s2 != in && *s2 != '/')
		s2--;

	if (s - s2 < 2)
	{
		out[0] = 0;
	}
	else
	{
		s--;
		memcpy(out, s2 + 1, s - s2); //mxd. strncpy -> memcpy
		out[s - s2] = 0;
	}
}

// Q2 counterpart. Returns the path up to, but not including the last '/'
H2COMMON_API void COM_FilePath(const char* in, char* out)
{
	const char* s = in + strlen(in) - 1;
	while (s != in && *s != '/')
		s--;

	memcpy(out, in, s - in); //mxd. strncpy -> memcpy
	out[s - in] = 0;
}

// Q2 counterpart
H2COMMON_API void COM_DefaultExtension(char* path, const char* extension)
{
	// If path doesn't have a .EXT, append extension (extension should include the '.')
	const char* src = path + strlen(path) - 1;
	while (*src != '/' && src != path)
	{
		if (*src == '.')
			return; // It has an extension

		src--;
	}

	strcat(path, extension);
}

char com_token[MAX_TOKEN_CHARS];

// Q2 counterpart. Parse a token out of a string
H2COMMON_API char* COM_Parse(char** data_p)
{
	char c; //mxd. int -> char

	char* data = *data_p;
	int len = 0;
	com_token[0] = 0;

	if (!data)
	{
		*data_p = NULL;
		return "";
	}

skipwhite:
	// Skip whitespace
	while ((c = *data) <= ' ')
	{
		if (c == 0)
		{
			*data_p = NULL;
			return "";
		}

		data++;
	}

	// Skip '//' comments
	if (c == '/' && data[1] == '/')
	{
		while (*data && *data != '\n')
			data++;

		goto skipwhite;
	}

	// Handle quoted strings specially
	if (c == '\"')
	{
		data++;

		while (true)
		{
			c = *data++;

			if (c == '\"' || !c)
				goto done;

			if (len < MAX_TOKEN_CHARS)
			{
				com_token[len] = c;
				len++;
			}
		}
	}

	// Parse a regular word
	do
	{
		if (len < MAX_TOKEN_CHARS)
		{
			com_token[len] = c;
			len++;
		}

		data++;
		c = *data;
	} while (c > 32);

done:
	if (len == MAX_TOKEN_CHARS)
		len = 0;

	com_token[len] = 0;
	*data_p = data;

	return com_token;
}

// Q2 counterpart
H2COMMON_API void Com_sprintf(char* dest, const int size, const char* fmt, ...)
{
	va_list argptr;

	va_start(argptr, fmt);
	const int len = vsnprintf(dest, size, fmt, argptr); //mxd. vsprintf -> vsnprintf
	va_end(argptr);

	if (len >= size)
		(*com_printf)("Com_sprintf: overflow of %i in %i\n", len, size); //mxd. Q2 uses regular Com_Printf here
}

static int paged_total = 0;

// Q2 counterpart. //TODO: Ancient win32 logic to fetch data from swap file to RAM. No longer needed. Remove?
H2COMMON_API void Com_PageInMemory(const byte* buffer, const int size)
{
	for (int i = size - 1; i > 0; i -= 4096)
		paged_total += buffer[i];
}

// Q2 counterpart. Does a varargs printf into a temp buffer.
H2COMMON_API char* va(const char* format, ...)
{
	va_list argptr;
	static char string[1024];

	va_start(argptr, format);
	vsnprintf(string, 1024, format, argptr);
	va_end(argptr);

	return string;
}