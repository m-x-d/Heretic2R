//
// Message.c
//
// Copyright 1998 Raven Software
//

#include "Message.h"
#include "SinglyLinkedList.h"

void QueueMessage(MsgQueue_t* this_ptr, void* msg)
{
	GenericUnion4_t temp;

	temp.t_void_p = msg;
	SLList_Push(&this_ptr->msgs, temp);
}

#ifdef _GAME_DLL
#include "g_local.h"
size_t SetParms(SinglyLinkedList_t* this_ptr, const char* format, va_list marker, qboolean entsAsNames)
#else
size_t SetParms(SinglyLinkedList_t* this_ptr, const char* format, va_list marker)
#endif
{
	qboolean append = false;
	GenericUnion4_t parm;
	int count = 0;
	size_t bytesParsed = 0;

	SLList_Front(this_ptr);

	while (format[count] != 0)
	{
		switch (format[count])
		{
			case 'b':
				parm.t_byte = va_arg(marker, byte);
				bytesParsed += sizeof(byte);
				break;

			case 's':
				parm.t_short = va_arg(marker, short);
				bytesParsed += sizeof(short);
				break;

			case 'i':
				parm.t_int = va_arg(marker, int);
				bytesParsed += sizeof(int);
				break;

			case 'f':
				parm.t_float = va_arg(marker, double);
				bytesParsed += sizeof(double);
				break;

			case 'e': // A pointer is a pointer is a pointer
#ifdef _GAME_DLL
				// This is pretty nasty, this may be an indication that something needs to be rethought.
				// Currently this is only used in the ICScript_Advance.
				// It does keep the code all in one place, which is nice.
				if (entsAsNames)
				{
					int j = 0;
					edict_t* ent = NULL;
					char entityName[64];

					do
					{
						entityName[j] = *(marker + j);
					} while (*(marker + j++));

					marker += j;

					ent = G_Find(ent, FOFS(targetname), entityName);

#ifdef _DEBUG
					if (ent)
					{
						edict_t* ent2 = NULL;

						ent2 = G_Find(ent, FOFS(targetname), entityName);

						assert(!ent2);	// each entityName should be unique!!!
					}
#endif
					parm.t_void_p = ent;
					break;
				}
#endif

			case 'v': // This better be not be a local variable or this will be bunk when the message is received and parsed.
			case 'g': // g for generic
				parm.t_void_p = va_arg(marker, void*);
				bytesParsed += sizeof(void*);
				break;

			case 'c':
				parm.t_RGBA = va_arg(marker, paletteRGBA_t);
				bytesParsed += sizeof(paletteRGBA_t);
				break;

			default:
				assert(0);
		}

		if (append)
		{
			SLList_InsertAfter(this_ptr, parm);
			SLList_Increment(this_ptr);
		}
		else
		{
			SLList_ReplaceCurrent(this_ptr, parm);

			if (SLList_AtLast(this_ptr))
				append = true;
			else
				SLList_Increment(this_ptr);
		}

		count++;
	}

	return bytesParsed;
}

int GetParms(SinglyLinkedList_t* this_ptr, const char* format, va_list marker)
{
	int count = 0;

	assert(format);

	if (format == NULL)
	{
		Sys_Error("GetParms: null format string");
		return 0;
	}

	SLList_Front(this_ptr);

	assert(!SLList_AtEnd(this_ptr));

	if (SLList_AtEnd(this_ptr))
	{
		Sys_Error("Getthis: empty parameter list");
		return 0;
	}

	while (format[count] != 0)
	{
		switch (format[count])
		{
			case 'b':
				byte* b = va_arg(marker, byte*);
				*b = SLList_PostIncrement(this_ptr).t_byte;
				break;

			case 's':
				short* s = va_arg(marker, short*);
				*s = SLList_PostIncrement(this_ptr).t_short;
				break;

			case 'i':
				int* i = va_arg(marker, int*);
				*i = SLList_PostIncrement(this_ptr).t_int;
				break;

			case 'f':
				float* f = va_arg(marker, float*);
				*f = SLList_PostIncrement(this_ptr).t_float;
				break;

			case 'v':
				float* v = va_arg(marker, float*);
				v = SLList_PostIncrement(this_ptr).t_float_p;
				break;

			case 'e': // A pointer is a pointer is a pointer.
			case 'g':
				void** g = va_arg(marker, void**);
				*g = SLList_PostIncrement(this_ptr).t_void_p;
				break;

			case 'c':
				paletteRGBA_t* c = va_arg(marker, paletteRGBA_t*);
				*c = SLList_PostIncrement(this_ptr).t_RGBA;
				break;

			default:
				assert(0);
				return 0;
		}

		count++;
	}

	return count;
}