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
	fxi.Activate_Screen_Shake(count, (float)(time * 100), (float)fx_time, dir);
}