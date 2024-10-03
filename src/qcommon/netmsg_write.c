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

void MSG_WriteFloat(sizebuf_t* sb, float f)
{
	NOT_IMPLEMENTED
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

void ParseEffectToSizeBuf(sizebuf_t* sb, const char* format, va_list marker) // H2
{
	while (format != NULL && *format != 0)
	{
		switch (*format)
		{
			case 'b':
				MSG_WriteByte(sb, va_arg(marker, byte));
				break;

			case 'd':
				MSG_WriteDir(sb, va_arg(marker, vec3_t));
				break;

			case 'f':
				MSG_WriteFloat(sb, va_arg(marker, float)); //TODO: or double?
				break;

			case 'i':
				MSG_WriteLong(sb, va_arg(marker, int));
				break;

			case 'p':
			case 'v':
				MSG_WritePos(sb, va_arg(marker, vec3_t));
				break;

			case 's':
				MSG_WriteShort(sb, va_arg(marker, short));
				break;

			case 't':
				MSG_WriteShortYawPitch(sb, va_arg(marker, vec3_t));
				break;

			case 'u':
				MSG_WriteDirMag(sb, va_arg(marker, vec3_t));
				break;

			case 'x':
				MSG_WriteYawPitch(sb, va_arg(marker, vec3_t));
				break;

			default:
				break;
		}

		format++;
	}
}

void MSG_WriteDir(sizebuf_t* sb, vec3_t vector)
{
	NOT_IMPLEMENTED
}

void MSG_WriteDirMag(sizebuf_t* sb, vec3_t dir)
{
	NOT_IMPLEMENTED
}

void MSG_WriteYawPitch(sizebuf_t* sb, vec3_t vector)
{
	NOT_IMPLEMENTED
}

void MSG_WriteShortYawPitch(sizebuf_t* sb, vec3_t vector)
{
	NOT_IMPLEMENTED
}