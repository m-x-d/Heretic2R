//==============================================================================
//
// p_chicken.c
//
// Heretic II
// Copyright 1998 Raven Software
//
//
//	AI :
//
//	STAND1		: Looking straight ahead
//
//  Ported from m_chicken.c by Marcus Whitlock
//
//==============================================================================

#include "player.h"
#include "g_Physics.h"
#include "g_teleport.h"
#include "g_Skeletons.h"
#include "p_types.h"
#include "p_anims.h"
#include "p_chicken.h"
#include "fx.h"
#include "random.h"
#include "vector.h"
#include "Utilities.h"
#include "p_main.h"

#define CHICKEN_GLIDE			150
#define CHICKEN_GLIDE_FORWARD	200

void ChickenStepSound(playerinfo_t *playerinfo, float value)
{
	char *name;

	if (playerinfo->edictflags & FL_SUPER_CHICKEN)
	{
		name = (irand(0,1)) ? "monsters/tbeast/step1.wav" : "monsters/tbeast/step2.wav";

		if(playerinfo->isclient)
		{
			playerinfo->CL_Sound(SND_PRED_ID46,
								 playerinfo->origin,
								 CHAN_WEAPON,
								 name,
								 1.0,
								 ATTN_NORM,
								 0);
		}
		else
		{
			playerinfo->G_Sound(SND_PRED_ID46,
								playerinfo->leveltime,
								playerinfo->self,
								CHAN_WEAPON,
								playerinfo->G_SoundIndex(name),
								1.0,
								ATTN_NORM,
								0);
		}
	}
}

void ChickenAssert(playerinfo_t *playerinfo)
{
	//This should never be called, if it is, a sequence has been selected that cannot be addressed by the chicken
	PlayerAnimSetLowerSeq(playerinfo, ASEQ_STAND);
	PlayerAnimSetUpperSeq(playerinfo, ASEQ_NONE);

	assert(0);
}

// ***********************************************************************************************
// PlayerChickenBite
// -----------------
// ************************************************************************************************

void PlayerChickenBite(playerinfo_t *playerinfo)
{
	if(!playerinfo->isclient)
		playerinfo->G_PlayerActionChickenBite(playerinfo);
}

// ***********************************************************************************************
// PlayerChickenSqueal
// -------------------
// ************************************************************************************************

void PlayerChickenSqueal(playerinfo_t *playerinfo)
{
	if(playerinfo->isclient)
	{
		playerinfo->CL_Sound(SND_PRED_ID47,
							 playerinfo->origin,
							 CHAN_WEAPON,
							 "",
							 1.0,
							 ATTN_NORM,
							 0);
	}
	else
	{
		playerinfo->G_Sound(SND_PRED_ID47,
							playerinfo->leveltime,
							playerinfo->self,
							CHAN_WEAPON,
							playerinfo->G_SoundIndex(""),
							1.0,
							ATTN_NORM,
							0);
	}
}

// ***********************************************************************************************
// PlayerChickenNoise
// -------------------
// ************************************************************************************************

void PlayerChickenCluck(playerinfo_t *playerinfo, float force)
{
	char *soundname;

	assert(playerinfo);

	if ( (!force) && (irand(0,10)) )
		return;

	if (playerinfo->edictflags & FL_SUPER_CHICKEN)
		soundname = (irand(0,1)) ? "monsters/superchicken/cluck1.wav" : "monsters/superchicken/cluck2.wav";
	else
		soundname = (irand(0,1)) ? "monsters/chicken/cluck1.wav" : "monsters/chicken/cluck2.wav";

	if(playerinfo->isclient)
		playerinfo->CL_Sound(SND_PRED_ID48,playerinfo->origin, CHAN_WEAPON, soundname, 1.0, ATTN_NORM, 0);
	else
		playerinfo->G_Sound(SND_PRED_ID48,playerinfo->leveltime,playerinfo->self, CHAN_WEAPON, playerinfo->G_SoundIndex(soundname), 1.0, ATTN_NORM, 0);
}

// ***********************************************************************************************
// PlayerChickenJump
// -----------------
// ************************************************************************************************

int PlayerChickenJump(playerinfo_t *playerinfo)
{
	trace_t		trace;
	vec3_t		endpos;
	char		*soundname;
	int			id;

	VectorCopy(playerinfo->origin,endpos);
	endpos[2]+=(playerinfo->mins[2]-2.0);

	if(playerinfo->isclient)
	{
		playerinfo->CL_Trace(playerinfo->origin,
							 playerinfo->mins,
							 playerinfo->maxs,
							 endpos,
							 MASK_PLAYERSOLID,
							 CEF_CLIP_TO_WORLD,
							 &trace);
	}
	else
	{
		playerinfo->G_Trace(playerinfo->origin,
								  playerinfo->mins,
								  playerinfo->maxs,
								  endpos,
								  playerinfo->self,
								  MASK_PLAYERSOLID,&trace);
	}

	if((playerinfo->groundentity||trace.fraction<0.2)&&playerinfo->waterlevel<2)
		playerinfo->upvel=200;

	PlayerAnimSetLowerSeq(playerinfo,ASEQ_FALL);

	id = irand(0,6);

	if (playerinfo->edictflags & FL_SUPER_CHICKEN)
	{
		switch ( id )
		{
		case 0:
			soundname = "monsters/superchicken/jump1.wav";
			break;
		
		case 1:
			soundname = "monsters/superchicken/jump2.wav";
			break;
		
		case 2:
			soundname = "monsters/superchicken/jump3.wav";
			break;

		default:
			return ASEQ_FALL;
			break;
		}
	}
	else
	{
		switch ( id )
		{
		case 0:
			soundname = "monsters/chicken/jump1.wav";
			break;
		
		case 1:
			soundname = "monsters/chicken/jump2.wav";
			break;
		
		case 2:
			soundname = "monsters/chicken/jump3.wav";
			break;

		default:
			return ASEQ_FALL;
			break;
		}
	}

	if(playerinfo->isclient)
		playerinfo->CL_Sound(SND_PRED_ID49,playerinfo->origin, CHAN_WEAPON, soundname, 1.0, ATTN_NORM, 0);
	else
		playerinfo->G_Sound(SND_PRED_ID49,playerinfo->leveltime,playerinfo->self, CHAN_WEAPON, playerinfo->G_SoundIndex(soundname), 1.0, ATTN_NORM, 0);
	
	return ASEQ_FALL;
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
