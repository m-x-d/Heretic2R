//
// cl_view.c
//
// Copyright 1998 Raven Software
//

#include "client.h"

cvar_t* crosshair;
static cvar_t* cl_stats;

//mxd. Very stripped down compared to Q2 version.
void V_Init(void)
{
	cl_stats = Cvar_Get("cl_stats", "0", 0);
	crosshair = Cvar_Get("crosshair", "0", CVAR_ARCHIVE);
}

void CL_CleanScreenShake(void)
{
	NOT_IMPLEMENTED
}
