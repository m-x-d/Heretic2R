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

#define CHICKEN_GLIDE			150
#define CHICKEN_GLIDE_FORWARD	200

void ChickenStepSound(const playerinfo_t* info, float value)
{
	if (info->edictflags & FL_SUPER_CHICKEN)
		P_Sound(info, SND_PRED_ID46, CHAN_WEAPON, va("monsters/tbeast/step%i.wav", irand(1, 2)), 1.0f); //mxd
}

// This should never be called, if it is, a sequence has been selected that cannot be addressed by the chicken.
void ChickenAssert(playerinfo_t* info)
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
			info->upvel = 200;
	}

	PlayerAnimSetLowerSeq(info, ASEQ_FALL);

	const int id = irand(1, 7);
	if (id < 4)
		P_Sound(info, SND_PRED_ID49, CHAN_WEAPON, va("monsters/%s/jump%i.wav", GetChickenType(info), id), 1.0f); //mxd
}

void PlayerChickenCheckFlap ( playerinfo_t *playerinfo )
{
	vec3_t	vf;

	if (playerinfo->seqcmd[ACMDL_JUMP])
	{
		playerinfo->flags |= PLAYER_FLAG_USE_ENT_POS;
		
		AngleVectors(playerinfo->angles, vf, NULL, NULL);
		vf[2] = 0;

		VectorScale(vf, CHICKEN_GLIDE_FORWARD, playerinfo->velocity);

		playerinfo->velocity[2] += CHICKEN_GLIDE;

		if(!playerinfo->isclient)
			playerinfo->G_CreateEffect(EFFECT_PRED_ID13,
									   playerinfo->G_GetEntityStatePtr((edict_t *)playerinfo->self),
									   FX_CHICKEN_EXPLODE,
									   CEF_OWNERS_ORIGIN | CEF_FLAG6,
									   NULL,
									   "");
		else
			playerinfo->CL_CreateEffect(EFFECT_PRED_ID13,
										playerinfo->self,
									    FX_CHICKEN_EXPLODE,
									    CEF_OWNERS_ORIGIN | CEF_FLAG6,
										NULL,
										"");

		PlayerAnimSetLowerSeq(playerinfo,ASEQ_JUMPFWD);
	}
}

void PlayerChickenFlap ( playerinfo_t *playerinfo )
{
	vec3_t	vf;

	playerinfo->flags |= PLAYER_FLAG_USE_ENT_POS;
	
	AngleVectors(playerinfo->angles, vf, NULL, NULL);
	vf[2] = 0;

	VectorScale(vf, CHICKEN_GLIDE_FORWARD, playerinfo->velocity);

	playerinfo->velocity[2] += CHICKEN_GLIDE;

	if(!playerinfo->isclient)
		playerinfo->G_CreateEffect(EFFECT_PRED_ID14,
								   playerinfo->self,
								   FX_CHICKEN_EXPLODE,
								   CEF_OWNERS_ORIGIN | CEF_FLAG6,
								   NULL,
								   "");
	else
		playerinfo->CL_CreateEffect(EFFECT_PRED_ID14,
									playerinfo->self,
								    FX_CHICKEN_EXPLODE,
								    CEF_OWNERS_ORIGIN | CEF_FLAG6,
								    NULL,
								    "");
}
