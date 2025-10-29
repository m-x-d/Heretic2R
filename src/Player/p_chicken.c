//
// p_chicken.c
//
// Copyright 1998 Raven Software
// Ported from m_chicken.c by Marcus Whitlock.
//

#include "Player.h"
#include "p_main.h"
#include "p_anims.h"
#include "p_chicken.h"
#include "p_utility.h" //mxd
#include "FX.h"
#include "Random.h"
#include "Vector.h"

void PlayerChickenStepSound(const playerinfo_t* info, float value)
{
	if (info->edictflags & FL_SUPER_CHICKEN)
		P_Sound(info, SND_PRED_ID46, CHAN_WEAPON, va("monsters/tbeast/step%i.wav", irand(1, 2)), 1.0f); //mxd
}

// This should never be called, if it is, a sequence has been selected that cannot be addressed by the chicken.
void PlayerChickenAssert(playerinfo_t* info)
{
	PlayerAnimSetLowerSeq(info, ASEQ_STAND);
	PlayerAnimSetUpperSeq(info, ASEQ_NONE);
	assert(0);
}

void PlayerChickenBite(playerinfo_t* info)
{
	if (!info->isclient)
		info->G_PlayerActionChickenBite(info);
}

//mxd
static char* GetChickenType(const playerinfo_t* info)
{
	return ((info->edictflags & FL_SUPER_CHICKEN) ? "superchicken" : "chicken");
}

void PlayerChickenCluck(const playerinfo_t* info, const float force)
{
	assert(info);

	if (force != 0.0f || irand(0, 10) == 0)
		P_Sound(info, SND_PRED_ID48, CHAN_WEAPON, va("monsters/%s/cluck%i.wav", GetChickenType(info), irand(1, 2)), 1.0f); //mxd
}

void PlayerChickenJump(playerinfo_t* info)
{
	if (info->waterlevel < 2)
	{
		vec3_t endpos;
		VectorCopy(info->origin, endpos);
		endpos[2] += (info->mins[2] - 2.0f);

		trace_t trace;
		P_Trace(info, info->origin, info->mins, info->maxs, endpos, &trace); //mxd

		if (info->groundentity != NULL || trace.fraction < 0.2f)
			info->upvel = 200.0f;
	}

	PlayerAnimSetLowerSeq(info, ASEQ_FALL);

	const int id = irand(1, 7);
	if (id < 4)
		P_Sound(info, SND_PRED_ID49, CHAN_WEAPON, va("monsters/%s/jump%i.wav", GetChickenType(info), id), 1.0f); //mxd
}

//mxd. Added to reduce code repetition.
static void FlapSetup(playerinfo_t* info, const byte event_id)
{
#define CHICKEN_GLIDE_UPWARDS	150.0f
#define CHICKEN_GLIDE_FORWARD	200.0f

	info->flags |= PLAYER_FLAG_USE_ENT_POS;

	vec3_t forward;
	AngleVectors(info->angles, forward, NULL, NULL);
	forward[2] = 0.0f;

	VectorScale(forward, CHICKEN_GLIDE_FORWARD, info->velocity);
	info->velocity[2] += CHICKEN_GLIDE_UPWARDS;

	P_CreateEffect(info, event_id, info->self, FX_CHICKEN_EXPLODE, CEF_OWNERS_ORIGIN | CEF_FLAG6, NULL, ""); //mxd
}

void PlayerChickenCheckFlap(playerinfo_t* info)
{
	if (info->seqcmd[ACMDL_JUMP])
	{
		FlapSetup(info, EFFECT_PRED_ID13); //mxd
		PlayerAnimSetLowerSeq(info, ASEQ_JUMPFWD);
	}
}

void PlayerChickenFlap(playerinfo_t* info)
{
	FlapSetup(info, EFFECT_PRED_ID14); //mxd
}