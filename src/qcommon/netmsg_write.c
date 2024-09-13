//
// netmsg_write.c
//
// Copyright 1998 Raven Software
//

#include "client.h"

// Q2 counterpart
void MSG_WriteChar(sizebuf_t* sb, const int c)
{
	byte* buf = SZ_GetSpace(sb, 1);
	*buf = (char)c;
}

// Q2 counterpart
void MSG_WriteByte(sizebuf_t* sb, const int c)
{
	byte* buf = SZ_GetSpace(sb, 1);
	buf[0] = (byte)c;
}

// Q2 counterpart
void MSG_WriteShort(sizebuf_t* sb, const int c)
{
	byte* buf = SZ_GetSpace(sb, 2);
	buf[0] = (byte)(c);
	buf[1] = (byte)(c >> 8);
}

// Q2 counterpart
void MSG_WriteLong(sizebuf_t* sb, const int c)
{
	byte* buf = SZ_GetSpace(sb, 4);
	buf[0] = (byte)(c);
	buf[1] = (byte)(c >> 8);
	buf[2] = (byte)(c >> 16);
	buf[3] = (byte)(c >> 24);
}

// Q2 counterpart
void MSG_WriteString(sizebuf_t* sb, const char* s)
{
	if (s != NULL)
		SZ_Write(sb, s, (int)strlen(s) + 1);
	else
		SZ_Write(sb, "", 1);
}

void MSG_WriteDeltaUsercmd(sizebuf_t* buf, usercmd_t* from, usercmd_t* cmd)
{
	NOT_IMPLEMENTED
}