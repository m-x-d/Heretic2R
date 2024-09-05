//
// sv_send.c
//
// Copyright 1998 Raven Software
//

#include "server.h"

void SV_BroadcastPrintf(int level, char* fmt, ...)
{
	NOT_IMPLEMENTED
}

void SV_BroadcastCaption(int printlevel, short stringid)
{
	NOT_IMPLEMENTED
}

void SV_BroadcastObituary(int printlevel, short stringid, short client1, short client2)
{
	NOT_IMPLEMENTED
}

// Q2 counterpart
void SV_BroadcastCommand(char* fmt, ...)
{
	va_list argptr;
	char string[1024];

	if (sv.state != ss_dead)
	{
		va_start(argptr, fmt);
		vsprintf_s(string, sizeof(string), fmt, argptr); //mxd. vsprintf -> vsprintf_s
		va_end(argptr);

		MSG_WriteByte(&sv.multicast, svc_stufftext);
		MSG_WriteString(&sv.multicast, string);
		SV_Multicast(NULL, MULTICAST_ALL_R);
	}
}

void SV_Multicast(vec3_t origin, multicast_t to)
{
	NOT_IMPLEMENTED
}

void SV_StartSound(vec3_t origin, edict_t* entity, int channel,	int soundindex, float volume, float attenuation, float timeofs)
{
	NOT_IMPLEMENTED
}

void SV_SendClientMessages(qboolean send_client_data)
{
	NOT_IMPLEMENTED
}