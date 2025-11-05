//
// Message.c
//
// Copyright 1998 Raven Software
//

#include "Message.h"

void MSG_Queue(MsgQueue_t* self, void* msg) //mxd. Named 'QueueMessage' in original logic.
{
	const GenericUnion4_t temp = { .t_void_p = msg };
	SLList_Push(&self->msgs, temp);
}

size_t MSG_SetParms(SinglyLinkedList_t* parms, const char* format, va_list marker) //mxd. Named 'SetParms' in original logic.
{
	qboolean append = false;
	int count = 0;
	size_t bytesParsed = 0;

	SLList_Front(parms);

	while (format[count] != 0)
	{
		GenericUnion4_t parm;

		switch (format[count])
		{
			case 'b':
				parm.t_byte = va_arg(marker, byte);
				bytesParsed += sizeof(parm.t_byte);
				break;

			case 's':
				parm.t_short = va_arg(marker, short);
				bytesParsed += sizeof(parm.t_short);
				break;

			case 'i':
				parm.t_int = va_arg(marker, int);
				bytesParsed += sizeof(parm.t_int);
				break;

			case 'f':
				parm.t_float = (float)va_arg(marker, double);
				bytesParsed += sizeof(parm.t_float); //mxd. sizeof(double) in original logic.
				break;

			case 'e': // A pointer is a pointer is a pointer.
			case 'v': // This better be not be a local variable or this will be bunk when the message is received and parsed.
			case 'g': // g for generic.
				parm.t_void_p = va_arg(marker, void*);
				bytesParsed += sizeof(parm.t_void_p);
				break;

			case 'c':
				parm.t_RGBA = va_arg(marker, paletteRGBA_t);
				bytesParsed += sizeof(parm.t_RGBA);
				break;

			default:
				assert(0);
				return 0; //mxd
		}

		if (append)
		{
			SLList_InsertAfter(parms, parm);
			SLList_Increment(parms);
		}
		else
		{
			SLList_ReplaceCurrent(parms, parm);

			if (SLList_AtLast(parms))
				append = true;
			else
				SLList_Increment(parms);
		}

		count++;
	}

	return bytesParsed;
}

int MSG_GetParms(SinglyLinkedList_t* parms, const char* format, va_list marker) //mxd. Named 'GetParms' in original logic.
{
	int count = 0;

	assert(format);

	if (format == NULL)
		Sys_Error("MSG_GetParms: null format string");

	SLList_Front(parms);

	assert(!SLList_AtEnd(parms));

	if (SLList_AtEnd(parms))
		Sys_Error("MSG_GetParms: empty parameter list");

	while (format[count] != 0)
	{
		switch (format[count])
		{
			case 'b':
			{
				byte* b = va_arg(marker, byte*);
				*b = SLList_PostIncrement(parms).t_byte;
			} break;

			case 's':
			{
				short* s = va_arg(marker, short*);
				*s = SLList_PostIncrement(parms).t_short;
			} break;

			case 'i':
			{
				int* i = va_arg(marker, int*);
				*i = SLList_PostIncrement(parms).t_int;
			} break;

			case 'f':
			{
				float* f = va_arg(marker, float*);
				*f = SLList_PostIncrement(parms).t_float;
			} break;

			case 'v':
			{
				float* v = va_arg(marker, float*);
				v = SLList_PostIncrement(parms).t_float_p;
			} break;

			case 'e': // A pointer is a pointer is a pointer.
			case 'g':
			{
				void** g = va_arg(marker, void**);
				*g = SLList_PostIncrement(parms).t_void_p;
			} break;

			case 'c':
			{
				paletteRGBA_t* c = va_arg(marker, paletteRGBA_t*);
				*c = SLList_PostIncrement(parms).t_RGBA;
			} break;

			default:
				assert(0);
				return 0;
		}

		count++;
	}

	return count;
}