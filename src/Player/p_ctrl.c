//
// p_ctrl.c
//
// Heretic II
// Copyright 1998 Raven Software
//

#include "player.h"
#include "p_types.h"
#include "p_actions.h"
#include "p_anims.h"
#include "p_ctrl.h"
#include "p_main.h"
#include "surfaceprops.h"
#include "random.h"
#include "fx.h"

PLAYER_API void PlayerIntLand(playerinfo_t *playerinfo, float landspeed)
{
	char	LandSound[64];
	char	*material=NULL;
	qboolean	hardfall=false;

	// Initialise the appropriate landing sound.

	material=GetClientGroundSurfaceMaterialName(playerinfo);
	
	strcpy(LandSound,"player/");

	if(material)
	{	
		// Okay, lame, but if a material is not found, then it ignores the string being set up.

		strcat(LandSound, material);
	}

	if (playerinfo->edictflags & FL_CHICKEN)
	{
		playerinfo->flags &= ~ PLAYER_FLAG_FALLING;
	}
	else if (playerinfo->advancedstaff && playerinfo->seqcmd[ACMDU_ATTACK] && 
			(playerinfo->upperseq == ASEQ_WSWORD_DOWNSTAB || playerinfo->upperseq == ASEQ_WSWORD_STABHOLD))
	{
		if (landspeed < 600.0)
		{
			// Land heavy.
			PlayerInterruptAction(playerinfo);
			PlayerAnimSetLowerSeq(playerinfo, ASEQ_WSWORD_LOWERPULLOUT);
			playerinfo->fwdvel = 0.0;
			playerinfo->velocity[0] *= 0.5;
			playerinfo->velocity[1] *= 0.5;
			strcpy(LandSound,"*offwall.wav");
		}
		else 
		{	
			// Land hard, so roll.
			PlayerInterruptAction(playerinfo);
			PlayerAnimSetLowerSeq(playerinfo, ASEQ_ROLLDIVEF_W);
			playerinfo->fwdvel = 150;
			strcat(LandSound,"roll.wav");
			hardfall = true;
		}
	}
	else if (playerinfo->seqcmd[ACMDL_CROUCH])
	{
		PlayerInterruptAction(playerinfo);
		if (landspeed > 50.0)
		{
			PlayerAnimSetLowerSeq(playerinfo, ASEQ_ROLLDIVEF_W);
			strcat(LandSound,"roll.wav");
		}
		else
		{
			PlayerAnimSetLowerSeq(playerinfo, ASEQ_CROUCH_GO);
			strcat(LandSound,"land1.wav");
		}
	}
	else if(playerinfo->lowerseq == ASEQ_FORWARD_FLIP_L ||
		playerinfo->lowerseq == ASEQ_FORWARD_FLIP_R || 
		playerinfo->upperseq == ASEQ_FORWARD_FLIP_L ||
		playerinfo->upperseq == ASEQ_FORWARD_FLIP_R)
	{
		PlayerInterruptAction(playerinfo);
		if(playerinfo->maxs[2] == 25)//fixme!  have a global CROUCHING_MAX_Z and STANDING_MAX_Z
		{//already ready to stand up
			PlayerAnimSetLowerSeq(playerinfo, ASEQ_CROUCH_GO);
			strcat(LandSound,"land1.wav");
		}
		else
		{//still crouch height
			PlayerAnimSetLowerSeq(playerinfo, ASEQ_ROLL_FROM_FFLIP);
			strcat(LandSound,"roll.wav");
		}
	}
	else if (playerinfo->seqcmd[ACMDL_WALK_F])
	{
		if (landspeed > 600)
		{	
			// Can't avoid heavy fall/rolling.

			PlayerInterruptAction(playerinfo);
			PlayerAnimSetLowerSeq(playerinfo, ASEQ_ROLLDIVEF_W);
			strcat(LandSound,"roll.wav");
		}
		else
		{	
			// Drop straight into a walk.

			PlayerAnimSetLowerSeq(playerinfo, ASEQ_WALKF);
			strcat(LandSound,"land1.wav");
		}
	}
	else if (playerinfo->seqcmd[ACMDL_RUN_F])
	{
		if (landspeed > 600)
		{	
			// Can't avoid heavy fall/rolling.
			PlayerInterruptAction(playerinfo);
			PlayerAnimSetLowerSeq(playerinfo, ASEQ_ROLLDIVEF_W);
			strcat(LandSound,"roll.wav");
		}
		else
		{	
			// Drop straight into a run.
			PlayerAnimSetLowerSeq(playerinfo, ASEQ_RUNF);
			strcat(LandSound,"land1.wav");
		}
	}
	else if (playerinfo->seqcmd[ACMDL_JUMP])
	{
		if (landspeed > 600)
		{	
			// Can't avoid heavy fall/rolling.
			PlayerInterruptAction(playerinfo);
			PlayerAnimSetLowerSeq(playerinfo, ASEQ_ROLLDIVEF_W);
			strcat(LandSound,"roll.wav");
		}
		else
		{	
			// Drop straight into another jump.
			PlayerAnimSetLowerSeq(playerinfo, ASEQ_JUMPSTD_GO);
			strcat(LandSound,"walk");
			
			if (irand(0,1))
				strcat(LandSound, "1.wav");
			else
				strcat(LandSound, "2.wav");
		}
	}
	else
	{
		if (landspeed < 50.0)
		{
			PlayerAnimSetLowerSeq(playerinfo, ASEQ_STAND);
			playerinfo->fwdvel = 150;
			strcat(LandSound,"shuffle1.wav");
		}
		else if (landspeed < 300.0)
		{
			// Land light.

			PlayerAnimSetLowerSeq(playerinfo, ASEQ_LANDLIGHT);
			playerinfo->fwdvel = 0.0;
			playerinfo->velocity[0] *= 0.5;
			playerinfo->velocity[1] *= 0.5;
			strcat(LandSound,"land1.wav");
		}
		else if (landspeed < 600.0)
		{
			// Land heavy.

			PlayerAnimSetLowerSeq(playerinfo, ASEQ_LANDHEAVY);
			playerinfo->fwdvel = 0.0;
			playerinfo->velocity[0] *= 0.5;
			playerinfo->velocity[1] *= 0.5;
			strcat(LandSound,"land2.wav");
		}
		else 
		{	
			// Land hard, so roll.

			PlayerInterruptAction(playerinfo);
			PlayerAnimSetLowerSeq(playerinfo, ASEQ_ROLLDIVEF_W);
			playerinfo->fwdvel = 150;
			strcat(LandSound,"roll.wav");
			hardfall = true;
		}
	}

		// Play the appropriate landing sound.

	if(material)
	{
		if(playerinfo->isclient)
		{
			playerinfo->CL_Sound(SND_PRED_ID51,
								 playerinfo->origin,
								 CHAN_VOICE,
								 LandSound,
								 1,
								 ATTN_NORM,
								 0);
		}
		else
		{
			playerinfo->G_Sound(SND_PRED_ID51,
								playerinfo->leveltime,
								playerinfo->self,
								CHAN_VOICE,
								playerinfo->G_SoundIndex(LandSound),
								1,
								ATTN_NORM,
								0);
		}
	}

	if (hardfall)
	{	// Grunt.
		if(playerinfo->isclient)
		{
			playerinfo->CL_Sound(SND_PRED_ID52,
								 playerinfo->origin,
								 CHAN_FOOTSTEP,
								 "*fall.wav",
								 1,
								 ATTN_NORM,
								 0);
		}
		else
		{
			playerinfo->G_Sound(SND_PRED_ID52,
								playerinfo->leveltime,
								playerinfo->self,
								CHAN_FOOTSTEP,
								playerinfo->G_SoundIndex("*fall.wav"),
								1,
								ATTN_NORM,
								0);
		}
	}

	playerinfo->flags &= ~ PLAYER_FLAG_FALLING;

	// Don't do dust in the water!

	if(playerinfo->waterlevel==0)
		if(!playerinfo->isclient)
			playerinfo->G_CreateEffect(EFFECT_PRED_ID15,
									   playerinfo->G_GetEntityStatePtr(playerinfo->self),
									   FX_DUST_PUFF,
									   CEF_OWNERS_ORIGIN, 
									   playerinfo->origin,
									   "");
		else
			playerinfo->CL_CreateEffect(EFFECT_PRED_ID15,
										playerinfo->self,
										FX_DUST_PUFF,
										CEF_OWNERS_ORIGIN, 
										playerinfo->origin,
										"");
}
