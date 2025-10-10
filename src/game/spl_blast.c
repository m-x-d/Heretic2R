//
// spl_blast.c
//
// Copyright 1998 Raven Software
//

#include "spl_blast.h" //mxd
#include "g_ai.h" //mxd
#include "g_combat.h" //mxd
#include "g_playstats.h"
#include "m_beast.h"
#include "FX.h"
#include "Random.h"
#include "Utilities.h" //mxd
#include "Vector.h"
#include "g_local.h"

void SpellCastBlast(edict_t* caster, const vec3_t start_pos, const vec3_t aim_angles)
{
	static const vec3_t mins = { -3.0f, -3.0f, -3.0f }; //mxd. Made const static.
	static const vec3_t maxs = {  3.0f,  3.0f,  3.0f }; //mxd. Made const static.

	short distance[BLAST_NUM_SHOTS];

	// This weapon does not auto-target.
	vec3_t angles;
	AdjustAimAngles(caster, start_pos, aim_angles, BLAST_DISTANCE, 18.0f, angles); //mxd

	// Compress the angles into two shorts.
	const short s_yaw = ANGLE2SHORT(angles[YAW]);
	const short s_pitch = ANGLE2SHORT(angles[PITCH]);

	angles[YAW] -= BLAST_ANGLE_INC * (BLAST_NUM_SHOTS - 1) * 0.5f;

	for (int i = 0; i < BLAST_NUM_SHOTS; i++)
	{
		// Single shot traveling out.
		vec3_t forward;
		AngleVectors(angles, forward, NULL, NULL);

		vec3_t end_pos;
		VectorMA(start_pos, BLAST_DISTANCE, forward, end_pos);

		trace_t trace;
		gi.trace(start_pos, mins, maxs, end_pos, caster, MASK_SHOT, &trace);

		if (level.fighting_beast)
		{
			edict_t* ent = TBeastCheckHit(start_pos, trace.endpos);

			if (ent != NULL)
				trace.ent = ent;
		}

		if (trace.ent != NULL && trace.ent->takedamage != DAMAGE_NO && !EntReflecting(trace.ent, true, true))
		{
			int	damage = irand(BLAST_DMG_MIN, BLAST_DMG_MAX);

			if (DEATHMATCH)
				damage = (int)((float)damage * 0.75f);

			T_Damage(trace.ent, caster, caster, forward, trace.endpos, forward, damage, damage, 0, MOD_MMISSILE);
		}

		AlertMonstersAt(trace.endpos, caster, 1.0f, 0); //mxd. Don't forget to annoy enemies.

		vec3_t diff;
		VectorSubtract(trace.endpos, start_pos, diff);
		distance[i] = (short)(VectorLength(diff));

		angles[YAW] += BLAST_ANGLE_INC;
	}

	// The assumption is that there are 5 shot blasts.
	assert(BLAST_NUM_SHOTS == 5);
	gi.CreateEffect(NULL, FX_WEAPON_BLAST, 0, start_pos, "sssssss", s_yaw, s_pitch, distance[0], distance[1], distance[2], distance[3], distance[4]);
}
