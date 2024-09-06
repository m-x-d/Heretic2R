//
// netmsg_write.c
//
// Copyright 1998 Raven Software
//

#include "client.h"

// Q2 counterpart
void MSG_WriteByte(sizebuf_t* sb, const int c)
{
	byte* buf = SZ_GetSpace(sb, 1);
	buf[0] = (byte)c;
}

// Q2 counterpart
void MSG_WriteString(sizebuf_t* sb, const char* s)
{
	if (s != NULL)
		SZ_Write(sb, s, (int)strlen(s) + 1);
	else
		SZ_Write(sb, "", 1);
}