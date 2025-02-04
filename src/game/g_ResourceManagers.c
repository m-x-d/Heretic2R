//
// g_ResourceManagers.c
//
// Copyright 1998 Raven Software
//

#include "g_ResourceManagers.h" //mxd
#include "g_Message.h" //mxd

void G_InitResourceManagers(void)
{
	InitMsgMngr();
}

void G_ReleaseResourceManagers(void)
{
	ReleaseMsgMngr();
}