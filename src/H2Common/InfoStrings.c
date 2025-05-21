//
// InfoStrings.c
//
// Copyright 1998 Raven Software
//

#include "Console.h"
#include "q_shared.h"

// Q2 counterpart. Searches the string for the given key and returns the associated value, or an empty string.
H2COMMON_API char* Info_ValueForKey(const char* s, const char* key)
{
	static char value[2][512]; // Use two buffers so compares work without stomping on each other
	static int valueindex;
	char pkey[512];

	valueindex ^= 1;

	if (*s == '\\')
		s++;

	while (true)
	{
		char* o = pkey;

		while (*s != '\\')
		{
			if (!*s)
				return "";

			*o++ = *s++;
		}

		*o = 0;
		s++;

		o = value[valueindex];

		while (*s != '\\' && *s)
			*o++ = *s++;

		*o = 0;

		if (strcmp(key, pkey) == 0)
			return value[valueindex];

		if (!*s)
			return "";

		s++;
	}
}

// Q2 counterpart
H2COMMON_API void Info_RemoveKey(char* s, const char* key)
{
	char pkey[512];
	char value[512];

	if (strstr(key, "\\"))
		return;

	while (true)
	{
		char* start = s;

		if (*s == '\\')
			s++;

		char* o = pkey;

		while (*s != '\\')
		{
			if (!*s)
				return;

			*o++ = *s++;
		}

		*o = 0;
		s++;

		o = value;

		while (*s != '\\' && *s)
			*o++ = *s++;

		*o = 0;

		if (strcmp(key, pkey) == 0)
		{
			memmove(start, s, strlen(s) + 1); // Remove this part //mxd. strcpy -> memmove
			return;
		}

		if (!*s)
			return;
	}
}

// Q2 counterpart. H2: Com_Printf replaced with com_printf reference
H2COMMON_API void Info_SetValueForKey(char* s, const char* key, const char* value)
{
	char newi[MAX_INFO_STRING];

	if (key == NULL)
		return;

	if (strstr(key, "\\") || (value != NULL && strstr(value, "\\")))
	{
		(*com_printf)("Can't use keys or values with a \\\n");
		return;
	}

	if (strstr(key, ";"))
	{
		(*com_printf)("Can't use keys with a semicolon\n");
		return;
	}

	if (strstr(key, "\"") || (value != NULL && strstr(value, "\"")))
	{
		(*com_printf)("Can't use keys or values with a \"\n");
		return;
	}

	if ((strlen(key) > MAX_INFO_KEY - 1) || (value != NULL && (strlen(value) > MAX_INFO_VALUE - 1)))
	{
		(*com_printf)("Keys and values must be < 64 characters.\n");
		return;
	}

	Info_RemoveKey(s, key);

	if (value == NULL || value[0] == 0) //mxd. strlen(str) -> str[0] check.
		return;

	Com_sprintf(newi, sizeof(newi), "\\%s\\%s", key, value);

	if (strlen(newi) + strlen(s) >= MAX_INFO_STRING)
	{
		(*com_printf)("Info string length exceeded\n");
		return;
	}

	// Only copy ascii values
	s += strlen(s);
	const char* v = newi;

	while (*v != 0)
	{
		const char c = *v++; //mxd. int c -> char c
		if (c >= ' ')
			*s++ = c;
	}

	*s = 0;
}

// Q2 counterpart
// Some characters are illegal in info strings because they can mess up the server's parsing
H2COMMON_API qboolean Info_Validate(const char* s)
{
	if (strstr(s, "\"") || strstr(s, ";"))
		return false;

	return true;
}