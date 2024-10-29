//
// Message.c
//
// Copyright 1998 Raven Software
//
// Heretic II
//

#include "Message.h"
#include "SinglyLinkedList.h"

void QueueMessage(MsgQueue_t *this, void *msg)
{
	GenericUnion4_t temp;

	temp.t_void_p = msg;
	SLList_Push(&this->msgs, temp);
}

#ifdef _GAME_DLL
#include "g_local.h"
size_t SetParms(SinglyLinkedList_t *this, char *format, va_list marker, qboolean entsAsNames)
#else
size_t SetParms(SinglyLinkedList_t *this, char *format, va_list marker)
#endif
{
	qboolean append = false;
	GenericUnion4_t parm;
	char current;
	int count = 0;
	size_t bytesParsed = 0;

	SLList_Front(this);

	while(current = format[count])
	{
		switch(current)
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
		case 'e':	// a pointer is a pointer is a pointer
//			parm.t_edict_p = va_arg(marker, edict_t*);
//			break;

#ifdef _GAME_DLL	// this is pretty nasty, this may be an indication that something 
					// needs to be rethought currently this is only used in the 
					// ICScript_Advance

					// It does keep the code all in one place, which is nice

			if(entsAsNames)	
			{
				int j = 0;
				edict_t *ent = NULL;
				char entityName[64];

				do
				{
					entityName[j] = *(marker + j);
				}while(*(marker + j++));

				marker += j;

				ent = G_Find (ent, FOFS(targetname), entityName);

#ifdef _DEBUG
				if(ent)
				{
					edict_t *ent2 = NULL;

					ent2 = G_Find (ent, FOFS(targetname), entityName);

					assert(!ent2);	// each entityName should be unique!!!
				}
#endif
				parm.t_void_p = ent;
				break;
			}
#endif
		case 'v':	// this better be not be a local variable or this will be bunk when
					// the message is received and parsed
//			parm.t_float_p = va_arg(marker, float*);
//			break;
		case 'g':	// g for generic
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

		if(append)
		{
			SLList_InsertAfter(this, parm);
			SLList_Increment(this);
		}
		else
		{
			SLList_ReplaceCurrent(this, parm);

			if(SLList_AtLast(this))
			{
				append = true;
			}
			else
			{
				SLList_Increment(this);
			}
		}

		++count;
	}

	return bytesParsed;
}

int GetParms(SinglyLinkedList_t *this, char *format, va_list marker)
{
	qboolean append = false;
	char current;
	int count = 0;
	byte *b;
	short *s;
	int *i;
	float *f, *v;
//	edict_t **e;
	void **g;
	paletteRGBA_t *c;

	assert(format);

	if(!format)
	{
		Sys_Error("GetParms: null format string");
		return 0;
	}

	SLList_Front(this);

	assert(!SLList_AtEnd(this));

	if(SLList_AtEnd(this))
	{
		Sys_Error("Getthis: empty parameter list");
		return 0;
	}

	while(current = format[count])
	{
		switch(current)
		{

		case 'b':
			b = va_arg(marker, byte*);

			*b = SLList_PostIncrement(this).t_byte;
			break;
		case 's':
			s = va_arg(marker, short*);

			*s = SLList_PostIncrement(this).t_short;
			break;
		case 'i':
			i = va_arg(marker, int*);

			*i = SLList_PostIncrement(this).t_int;
			break;
		case 'f':
			f = va_arg(marker, float*);

			*f = SLList_PostIncrement(this).t_float;
			break;
		case 'v':
			v = va_arg(marker, float*);

			v = SLList_PostIncrement(this).t_float_p;
			break;
		case 'e':	// a pointer is a pointer is a pointer
//			e = va_arg(marker, edict_t**);

//			*e = SLList_PostIncrement(this).t_edict_p;
//			break;
		case 'g':
			g = va_arg(marker, void**);

			*g = SLList_PostIncrement(this).t_void_p;
			break;
		case 'c':
			c = va_arg(marker, paletteRGBA_t*);

			*c = SLList_PostIncrement(this).t_RGBA;
			break;
		default:
			assert(0);
			return 0;
		}

		++count;
	}

	return count;
}