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

// Q2 counterpart
void MSG_WritePos(sizebuf_t* sb, vec3_t pos)
{
	MSG_WriteShort(sb, (int)(pos[0] * 8.0f));
	MSG_WriteShort(sb, (int)(pos[1] * 8.0f));
	MSG_WriteShort(sb, (int)(pos[2] * 8.0f));
}

void MSG_WriteDeltaUsercmd(sizebuf_t* sb, usercmd_t* from, usercmd_t* cmd)
{
	NOT_IMPLEMENTED
}

void ParseEffectToSizeBuf(sizebuf_t* sb, char* format, va_list marker) // H2
{
	NOT_IMPLEMENTED
}