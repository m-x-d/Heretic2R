//
// fx_quake.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"

void FXQuake(centity_t* owner, int type, const int flags, vec3_t origin)
{
	byte count;
	byte time;
	byte dir;

	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_QUAKE].formatString, &count, &time, &dir);
	fxi.Activate_Screen_Shake(count, (float)(time * 100), (float)fxi.cl->time, dir); // 'current_time' MUST be cl.time, because that's what used by Perform_Screen_Shake() to calculate effect intensity/timing... --mxd.
}