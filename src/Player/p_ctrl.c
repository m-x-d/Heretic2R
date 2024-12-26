//
// p_ctrl.c
//
// Copyright 1998 Raven Software
//

#include "Player.h"
#include "p_actions.h"
#include "p_anims.h"
#include "p_ctrl.h"
#include "p_main.h"
#include "SurfaceProps.h"
#include "Random.h"
#include "FX.h"
#include "p_utility.h"

PLAYER_API void PlayerIntLand(playerinfo_t* info, const float landspeed)
{
	qboolean hardfall = false;

	// Initialise the appropriate landing sound.
	const char* material = GetClientGroundSurfaceMaterialName(info);

	char land_sound[64];
	strcpy_s(land_sound, sizeof(land_sound), "player/"); //mxd. strcpy -> strcpy_s.

	// Okay, lame, but if a material is not found, then it ignores the string being set up.
	if (material != NULL)
		strcat_s(land_sound, sizeof(land_sound), material); //mxd. strcat -> strcat_s.

	if (info->edictflags & FL_CHICKEN)
	{
		info->flags &= ~PLAYER_FLAG_FALLING;
	}
	else if (info->advancedstaff && info->seqcmd[ACMDU_ATTACK] && (info->upperseq == ASEQ_WSWORD_DOWNSTAB || info->upperseq == ASEQ_WSWORD_STABHOLD))
	{
		PlayerInterruptAction(info);

		if (landspeed > 600.0f) //mxd. Flipped check.
		{
			// Land hard, so roll.
			PlayerAnimSetLowerSeq(info, ASEQ_ROLLDIVEF_W);
			info->fwdvel = 150.0f;
			strcat_s(land_sound, sizeof(land_sound), "roll.wav"); //mxd. strcat -> strcat_s.
			hardfall = true;
		}
		else
		{
			// Land heavy.
			PlayerAnimSetLowerSeq(info, ASEQ_WSWORD_LOWERPULLOUT);
			info->fwdvel = 0.0f;
			info->velocity[0] *= 0.5f;
			info->velocity[1] *= 0.5f;
			strcpy_s(land_sound, sizeof(land_sound), "*offwall.wav"); //mxd. strcpy -> strcpy_s.
		}
	}
	else if (info->seqcmd[ACMDL_CROUCH])
	{
		PlayerInterruptAction(info);

		if (landspeed > 50.0f)
		{
			PlayerAnimSetLowerSeq(info, ASEQ_ROLLDIVEF_W);
			strcat_s(land_sound, sizeof(land_sound), "roll.wav"); //mxd. strcat -> strcat_s.
		}
		else
		{
			PlayerAnimSetLowerSeq(info, ASEQ_CROUCH_GO);
			strcat_s(land_sound, sizeof(land_sound), "land1.wav"); //mxd. strcat -> strcat_s.
		}
	}
	else if (info->lowerseq == ASEQ_FORWARD_FLIP_L || info->lowerseq == ASEQ_FORWARD_FLIP_R || 
			 info->upperseq == ASEQ_FORWARD_FLIP_L || info->upperseq == ASEQ_FORWARD_FLIP_R)
	{
		PlayerInterruptAction(info);

		if (info->maxs[2] == 25.0f) //TODO: have a global CROUCHING_MAX_Z and STANDING_MAX_Z
		{
			// Ready to stand up.
			PlayerAnimSetLowerSeq(info, ASEQ_CROUCH_GO);
			strcat_s(land_sound, sizeof(land_sound), "land1.wav"); //mxd. strcat -> strcat_s.
		}
		else
		{
			// Still crouch height.
			PlayerAnimSetLowerSeq(info, ASEQ_ROLL_FROM_FFLIP);
			strcat_s(land_sound, sizeof(land_sound), "roll.wav"); //mxd. strcat -> strcat_s.
		}
	}
	else if (info->seqcmd[ACMDL_WALK_F])
	{
		if (landspeed > 600.0f)
		{
			// Can't avoid heavy fall/rolling.
			PlayerInterruptAction(info);
			PlayerAnimSetLowerSeq(info, ASEQ_ROLLDIVEF_W);
			strcat_s(land_sound, sizeof(land_sound), "roll.wav"); //mxd. strcat -> strcat_s.
		}
		else
		{
			// Drop straight into a walk.
			PlayerAnimSetLowerSeq(info, ASEQ_WALKF);
			strcat_s(land_sound, sizeof(land_sound), "land1.wav"); //mxd. strcat -> strcat_s.
		}
	}
	else if (info->seqcmd[ACMDL_RUN_F])
	{
		if (landspeed > 600.0f)
		{
			// Can't avoid heavy fall/rolling.
			PlayerInterruptAction(info);
			PlayerAnimSetLowerSeq(info, ASEQ_ROLLDIVEF_W);
			strcat_s(land_sound, sizeof(land_sound), "roll.wav"); //mxd. strcat -> strcat_s.
		}
		else
		{
			// Drop straight into a run.
			PlayerAnimSetLowerSeq(info, ASEQ_RUNF);
			strcat_s(land_sound, sizeof(land_sound), "land1.wav"); //mxd. strcat -> strcat_s.
		}
	}
	else if (info->seqcmd[ACMDL_JUMP])
	{
		if (landspeed > 600.0f)
		{
			// Can't avoid heavy fall/rolling.
			PlayerInterruptAction(info);
			PlayerAnimSetLowerSeq(info, ASEQ_ROLLDIVEF_W);
			strcat_s(land_sound, sizeof(land_sound), "roll.wav"); //mxd. strcat -> strcat_s.
		}
		else
		{
			// Drop straight into another jump.
			PlayerAnimSetLowerSeq(info, ASEQ_JUMPSTD_GO);
			strcat_s(land_sound, sizeof(land_sound), va("walk%i.wav", irand(1, 2))); //mxd. strcat -> strcat_s.
		}
	}
	else
	{
		if (landspeed < 50.0f)
		{
			PlayerAnimSetLowerSeq(info, ASEQ_STAND);
			info->fwdvel = 150.0f;
			strcat_s(land_sound, sizeof(land_sound), "shuffle1.wav"); //mxd. strcat -> strcat_s.
		}
		else if (landspeed < 300.0f)
		{
			// Land light.
			PlayerAnimSetLowerSeq(info, ASEQ_LANDLIGHT);
			info->fwdvel = 0.0f;
			info->velocity[0] *= 0.5f;
			info->velocity[1] *= 0.5f;
			strcat_s(land_sound, sizeof(land_sound), "land1.wav"); //mxd. strcat -> strcat_s.
		}
		else if (landspeed < 600.0f)
		{
			// Land heavy.
			PlayerAnimSetLowerSeq(info, ASEQ_LANDHEAVY);
			info->fwdvel = 0.0f;
			info->velocity[0] *= 0.5f;
			info->velocity[1] *= 0.5f;
			strcat_s(land_sound, sizeof(land_sound), "land2.wav"); //mxd. strcat -> strcat_s.
		}
		else
		{
			// Land hard, so roll.
			PlayerInterruptAction(info);
			PlayerAnimSetLowerSeq(info, ASEQ_ROLLDIVEF_W);
			info->fwdvel = 150.0f;
			strcat_s(land_sound, sizeof(land_sound), "roll.wav"); //mxd. strcat -> strcat_s.
			hardfall = true;
		}
	}

	// Play the appropriate landing sound.
	if (material != NULL)
		P_Sound(info, SND_PRED_ID51, CHAN_VOICE, land_sound, 1.0f); //mxd

	// Play grunt sound?
	if (hardfall)
		P_Sound(info, SND_PRED_ID52, CHAN_FOOTSTEP, "*fall.wav", 1.0f); //mxd

	info->flags &= ~PLAYER_FLAG_FALLING;

	// Don't do dust in the water!
	if (info->waterlevel == 0)
		P_CreateEffect(info, EFFECT_PRED_ID15, info->self, FX_DUST_PUFF, CEF_OWNERS_ORIGIN, info->origin, ""); //mxd
}