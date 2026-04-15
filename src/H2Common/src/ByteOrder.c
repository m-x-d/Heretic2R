//
// ByteOrder.c -- Byte order functions
//
// Copyright 1998 Raven Software
//

#include "q_shared.h"

// Q2 counterpart //TODO: this won't work on Linux...
H2COMMON_API short BigShort(const short l)
{
	const byte b1 = l & 255;
	const byte b2 = (l >> 8) & 255;

	return (short)((b1 << 8) + b2);
}

// Q2 counterpart //TODO: this won't work on Linux...
H2COMMON_API int BigLong(const int l)
{
	const byte b1 = l & 255;
	const byte b2 = (l >> 8) & 255;
	const byte b3 = (l >> 16) & 255;
	const byte b4 = (l >> 24) & 255;

	return ((int)b1 << 24) + ((int)b2 << 16) + ((int)b3 << 8) + b4;
}

// Q2 counterpart //TODO: this won't work on Linux...
H2COMMON_API float BigFloat(const float f)
{
	union
	{
		float f;
		byte b[4];
	} dat1, dat2;

	dat1.f = f;
	dat2.b[0] = dat1.b[3];
	dat2.b[1] = dat1.b[2];
	dat2.b[2] = dat1.b[1];
	dat2.b[3] = dat1.b[0];

	return dat2.f;
}