//
// fx_quake.c
//
// Heretic II
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Client Entities.h"
#include "FX.h"



void FXQuake(centity_t *owner, int type, int flags, vec3_t origin)
{
	byte count,time,dir;
	
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_QUAKE].formatString, &count,&time,&dir);

	fxi.Activate_Screen_Shake(count,(time * 100), fxi.cl->time, dir);

}
