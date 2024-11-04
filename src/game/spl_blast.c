//
// spl_blast.c
//
// Heretic II
// Copyright 1998 Raven Software
//

#include "g_local.h"
#include "fx.h"
#include "Angles.h"
#include "Utilities.h"
#include "vector.h"
#include "random.h"
#include "g_playstats.h"
#include "m_beast.h"



// ****************************************************************************
// SpellCastBlast
// ****************************************************************************

void SpellCastBlast(edict_t *caster,vec3_t startpos,vec3_t aimangles,vec3_t aimdir)
{
	vec3_t fwd, endpos, mins={-3.0, -3.0, -3.0}, maxs={3.0, 3.0, 3.0};
	trace_t trace;
	int	damage;
	vec3_t	angles, diff;
	int i;
	short	distance[BLAST_NUM_SHOTS];
	short	syaw, spitch;

	// This weapon does not autotarget
	VectorCopy(aimangles, angles);

	angles[YAW] -= BLAST_ANGLE_INC * (BLAST_NUM_SHOTS-1) * 0.5;
	for (i=0; i<BLAST_NUM_SHOTS; i++)
	{
		// Single shot travelling out
		AngleVectors(angles, fwd, NULL, NULL);
		VectorMA(startpos, BLAST_DISTANCE, fwd, endpos);
		gi.trace(startpos, mins, maxs, endpos, caster, MASK_SHOT,&trace);
		if(level.fighting_beast)
		{
			edict_t *ent;
			
			if(ent = check_hit_beast(startpos, trace.endpos))
				trace.ent = ent;
		}

		if (trace.ent && trace.ent->takedamage && !(EntReflecting(trace.ent, true, true)))
		{	
			if(deathmatch->value)
				damage = irand(BLAST_DMG_MIN*0.75, BLAST_DMG_MAX*0.75);
			else
				damage = irand(BLAST_DMG_MIN, BLAST_DMG_MAX);
			T_Damage(trace.ent, caster, caster, fwd, trace.endpos, fwd, damage, damage, 0, MOD_MMISSILE);
		}

		VectorSubtract(trace.endpos, startpos, diff);
		distance[i] = VectorLength(diff);

		angles[YAW] += BLAST_ANGLE_INC;
	}

	// The assumption is that there are 5 shot blasts.
	assert(BLAST_NUM_SHOTS==5);

	// Compress the angles into two shorts.
	syaw = (short)(aimangles[YAW]*(65536.0/360.0));
	spitch = (short)(aimangles[PITCH]*(65536.0/360.0));

	gi.CreateEffect(NULL, FX_WEAPON_BLAST, 0, startpos, 
					"sssssss", syaw, spitch, distance[0], distance[1], distance[2], distance[3], distance[4]);
}




