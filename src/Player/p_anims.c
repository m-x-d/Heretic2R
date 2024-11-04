//
// p_anims.c
//
// Heretic II
// Copyright 1998 Raven Software
//

#include "player.h"
#include "p_types.h"
#include "p_animactor.h"
#include "p_anim_branch.h"
#include "p_anim_data.h"
#include "p_anims.h"
#include "p_main.h"
#include "g_Skeletons.h"
#include "angles.h"
#include "fx.h"
#include "random.h"
#include "vector.h"
#include "effectflags.h"

PLAYER_API void PlayerAnimSetUpperSeq(playerinfo_t *playerinfo, int seq)
{
	assert(playerinfo);

	if (playerinfo->upperseq != seq)
	{	
		// We don't set all the data up right because it's up to AnimUpdateFrame to do this.

		playerinfo->upperseq = seq;
		playerinfo->upperframe = -1;
		playerinfo->upperidle = false;
	}

	playerinfo->uppermove = PlayerSeqData[seq].move;
	
	playerinfo->uppermove_index=seq;

	assert(playerinfo->uppermove);

	if (playerinfo->upperseq == ASEQ_NONE)
		playerinfo->upperidle = true;
}

PLAYER_API void PlayerAnimSetLowerSeq(playerinfo_t *playerinfo, int seq)
{
	paceldata_t *seqdata;

	assert(playerinfo);

	if (playerinfo->lowerseq != seq)
	{	
		// We don't set all the data up right because it's up to AnimUpdateFrame to do this.

		playerinfo->lowerseq = seq;
		playerinfo->lowerframe = -1;
		playerinfo->loweridle = false;
	}

	if (playerinfo->edictflags & FL_CHICKEN)
		playerinfo->lowermove = PlayerChickenData[seq].move;
	else
		playerinfo->lowermove = PlayerSeqData[seq].move;

	assert(playerinfo->lowermove);

	playerinfo->lowermove_index=seq;

	// The lower two bytes of the player flags are stomped by the sequences' flags.

	if (playerinfo->edictflags & FL_CHICKEN)
	{
		seqdata = &PlayerChickenData[seq];
	}
	else
	{
		seqdata = &PlayerSeqData[seq];
		playerinfo->viewheight = PlayerSeqData2[seq].viewheight;
	}

	playerinfo->flags = seqdata->playerflags | (playerinfo->flags & PLAYER_FLAG_PERSMASK);

	// Set / reset flag that says I am flying..

	if (seqdata->fly)
		playerinfo->edictflags |= FL_FLY;
	else 
		playerinfo->edictflags &= ~FL_FLY;

	// Set / reset flag that says I am standing still.

	if (playerinfo->flags & PLAYER_FLAG_STAND)
		playerinfo->pm_flags |= PMF_STANDSTILL;
	else
		playerinfo->pm_flags &= ~PMF_STANDSTILL;

	// Set / reset flag that says I am movelocked.

	if(!playerinfo->isclient)
	{
		if (seqdata->lockmove)
			playerinfo->pm_flags |= PMF_LOCKMOVE; 
		else
			playerinfo->pm_flags &= ~PMF_LOCKMOVE;		
	}
}

PLAYER_API void PlayerBasicAnimReset(playerinfo_t *playerinfo)
{
	PlayerAnimSetLowerSeq(playerinfo, ASEQ_STAND);
	playerinfo->lowerframeptr = playerinfo->lowermove->frame;
	
	PlayerAnimSetUpperSeq(playerinfo, ASEQ_NONE);
	playerinfo->upperframeptr = playerinfo->uppermove->frame;
		
	playerinfo->effects|=EF_SWAPFRAME;
	playerinfo->effects &= ~(EF_DISABLE_EXTRA_FX | EF_ON_FIRE | EF_TRAILS_ENABLED);

	PlayerSetHandFX(playerinfo, HANDFX_NONE, -1);

	if (playerinfo->pers.weaponready == WEAPON_READY_NONE)		// Just in case we die with WEAPON_READY_NONE
		playerinfo->pers.weaponready = WEAPON_READY_HANDS;

	playerinfo->switchtoweapon = playerinfo->pers.weaponready;
	playerinfo->pers.newweapon = NULL;

	// Straighten out joints, i.e. reset torso twisting.
	
	if(!(playerinfo->edictflags&FL_CHICKEN))
		playerinfo->ResetJointAngles(playerinfo);

	memset(playerinfo->seqcmd,0,ACMD_MAX*sizeof(int));
}

PLAYER_API void PlayerAnimReset(playerinfo_t *playerinfo)
{
	PlayerAnimSetLowerSeq(playerinfo, ASEQ_STAND);
	playerinfo->lowerframeptr = playerinfo->lowermove->frame;
	
	PlayerAnimSetUpperSeq(playerinfo, ASEQ_NONE);
	playerinfo->upperframeptr = playerinfo->uppermove->frame;

	playerinfo->pers.armortype = ARMOR_TYPE_NONE;
	playerinfo->pers.bowtype = BOW_TYPE_NONE;
	playerinfo->pers.stafflevel = STAFF_LEVEL_BASIC;
	playerinfo->pers.helltype = HELL_TYPE_BASIC;
	playerinfo->pers.altparts = 0;
	playerinfo->pers.weaponready = WEAPON_READY_HANDS;
	playerinfo->switchtoweapon = WEAPON_READY_HANDS;
	playerinfo->pers.newweapon = NULL;
	PlayerUpdateModelAttributes(playerinfo);
	playerinfo->pers.handfxtype = HANDFX_NONE;
	
	PlayerSetHandFX(playerinfo, HANDFX_NONE, -1);
		
	playerinfo->effects|=EF_SWAPFRAME;
	playerinfo->effects &= ~(EF_DISABLE_EXTRA_FX | EF_ON_FIRE | EF_TRAILS_ENABLED);

	// Straighten out joints, i.e. no torso aiming.

	if(!(playerinfo->edictflags&FL_CHICKEN))	
		playerinfo->ResetJointAngles(playerinfo);

	memset(playerinfo->seqcmd,0,ACMD_MAX*sizeof(int));
}

int PlayerAnimWeaponSwitch(playerinfo_t *playerinfo)
{
	qboolean BranchCheckDismemberAction(playerinfo_t *playerinfo, int weapon);

	int newseq;

	assert(playerinfo);
	
	// See if we have the arm to do that magic.

	if (playerinfo->switchtoweapon != playerinfo->pers.weaponready)
	{	
		if (!BranchCheckDismemberAction(playerinfo, playerinfo->switchtoweapon))
			return ASEQ_NONE;

		newseq = PlayerAnimWeaponSwitchSeq[playerinfo->pers.weaponready][playerinfo->switchtoweapon];
		if (newseq)
		{
			PlayerAnimSetUpperSeq(playerinfo, newseq);
			return newseq;
		}
	}
	else if (playerinfo->pers.newweapon)
	{
		if (!BranchCheckDismemberAction(playerinfo, playerinfo->pers.newweapon->tag))
			return ASEQ_NONE;

		newseq = PlayerAnimWeaponSwitchSeq[playerinfo->pers.weaponready][playerinfo->pers.weaponready];
		if (newseq)
		{
			PlayerAnimSetUpperSeq(playerinfo, newseq);
			return newseq;
		}
	}
	return ASEQ_NONE;
}

PLAYER_API void PlayerAnimUpperIdle(playerinfo_t *playerinfo)
{
	int ret;

	if (ret = BranchUprReady(playerinfo))
	{
		PlayerAnimSetUpperSeq(playerinfo, ret);

		assert(playerinfo->uppermove);
	}

	assert(playerinfo->uppermove);
}

PLAYER_API void PlayerAnimLowerIdle(playerinfo_t *playerinfo)
{
	int ret;

	if (playerinfo->flags & PLAYER_FLAG_SURFSWIM)
	{
		if (ret = BranchLwrSurfaceSwim(playerinfo))
		{
			PlayerAnimSetLowerSeq(playerinfo, ret);
		}
	}
	else if (playerinfo->flags & PLAYER_FLAG_UNDERWATER)
	{
		if (ret = BranchLwrUnderwaterSwim(playerinfo))
		{
			PlayerAnimSetLowerSeq(playerinfo, ret);
		}
	}
	else if (playerinfo->flags & PLAYER_FLAG_ONROPE)
	{
		if (ret = BranchLwrClimbing(playerinfo))
		{
			PlayerAnimSetLowerSeq(playerinfo, ret);
		}
	}
	else if (ret = BranchLwrStanding(playerinfo))
	{
		PlayerAnimSetLowerSeq(playerinfo, ret);
	}
	else
	{
		if (playerinfo->leveltime - playerinfo->idletime > 15.0) 
		{
			if (playerinfo->lowerseq >= ASEQ_IDLE_READY_GO && playerinfo->lowerseq <= ASEQ_IDLE_LOOKR && playerinfo->lowerseq != ASEQ_IDLE_READY_END)
			{
				// Only certain idle should be called out of here.

				switch(playerinfo->irand(playerinfo, 0, 3))
				{
					case 0:
						PlayerAnimSetLowerSeq(playerinfo, ASEQ_IDLE_LOOKL);
						break;
					case 1:
						PlayerAnimSetLowerSeq(playerinfo, ASEQ_IDLE_LOOKR);
						break;
					case 2:
						PlayerAnimSetLowerSeq(playerinfo, ASEQ_IDLE_READY_END);
						break;
					default:
						PlayerAnimSetLowerSeq(playerinfo, ASEQ_IDLE_READY);
						break;
				}
			}
			// if we are in a cinematic, always do this idle, since its silent
			else
			if (playerinfo->sv_cinematicfreeze)
				PlayerAnimSetLowerSeq(playerinfo, ASEQ_IDLE_LOOKBACK);
			else if ((playerinfo->pers.weaponready == WEAPON_READY_BOW) || (playerinfo->isclient))
			{	
				// Because the bow doesn't look right in some idles.

				switch(playerinfo->irand(playerinfo, 0, 2))
				{
					case 0:
						PlayerAnimSetLowerSeq(playerinfo, ASEQ_IDLE_SCRATCH_ASS);
						break;
					case 1:
						PlayerAnimSetLowerSeq(playerinfo, ASEQ_IDLE_LOOKBACK);
						break;
					default:
						PlayerAnimSetLowerSeq(playerinfo, ASEQ_IDLE_READY_GO);
						break;
				}
			}
			else if ((playerinfo->pers.weaponready == WEAPON_READY_SWORDSTAFF))
			{
				// Because the staff doesn't look right in some idles.

				switch(playerinfo->irand(playerinfo, 0, 3))
				{
					case 0:
						PlayerAnimSetLowerSeq(playerinfo, ASEQ_IDLE_FLY1);
						break;
					case 1:
						PlayerAnimSetLowerSeq(playerinfo, ASEQ_IDLE_FLY2);
						break;
					case 2:
						PlayerAnimSetLowerSeq(playerinfo, ASEQ_IDLE_WIPE_BROW);
						break;
					default:
						PlayerAnimSetLowerSeq(playerinfo, ASEQ_IDLE_READY_GO);
						break;
				}
			}
			else
			{
				switch(playerinfo->irand(playerinfo, 0, 6))
				{
					case 0:
						PlayerAnimSetLowerSeq(playerinfo, ASEQ_IDLE_FLY1);
						break;
					case 1:
						PlayerAnimSetLowerSeq(playerinfo, ASEQ_IDLE_FLY2);
						break;
					case 2:
						PlayerAnimSetLowerSeq(playerinfo, ASEQ_IDLE_SCRATCH_ASS);
						break;
					case 3:
						PlayerAnimSetLowerSeq(playerinfo, ASEQ_IDLE_LOOKBACK);
						break;
					case 4:
						PlayerAnimSetLowerSeq(playerinfo, ASEQ_IDLE_WIPE_BROW);
						break;
					default:
						PlayerAnimSetLowerSeq(playerinfo, ASEQ_IDLE_READY_GO);
						break;
				}
			}
		}
	}
}

PLAYER_API void PlayerAnimUpperUpdate(playerinfo_t *playerinfo)
{
	seqctrl_t *seqctrl;

	int newseq=ASEQ_NONE;

	/*
	// first check if the lower anim forces the lower anim to lock in sync with it.
	if (PlayerSeqData2[playerinfo->lowerseq].nosplit)
	{
		// A seq value of NONE indicates that it is not asserting a move, copy the companion half.
		playerinfo->upperseq = ASEQ_NONE;
		playerinfo->upperidle = true;
		return;
	}
	*/

	// Init some values.

	playerinfo->upperidle = false;

	// Grab the sequence ctrl struct.

	seqctrl = &SeqCtrl[playerinfo->upperseq];

	// First check the branch function. This evaluates "extra" command flags for a potential
	// modification of the "simple" procedure.

	if (seqctrl->branchfunc)
	{
		newseq = seqctrl->branchfunc(playerinfo);
	}

	if (newseq == ASEQ_NONE)
	{
		if (seqctrl->command != ACMD_NONE)
		{
			if (playerinfo->seqcmd[seqctrl->command])
			{
				newseq = seqctrl->continueseq;
			}
			else 
			{
				newseq = seqctrl->ceaseseq;
			}
		}
		else
		{
			newseq = seqctrl->ceaseseq;
		}
	}

	// Now check for idles.  If the upper half has an idle, then the upper half is copied.

	if(newseq == ASEQ_NONE)
	{
		if (playerinfo->lowerseq == ASEQ_NONE)
		{
			newseq=BranchIdle(playerinfo);
			playerinfo->loweridle = true;
		}
		playerinfo->upperidle = true;
	}

	PlayerAnimSetUpperSeq(playerinfo, newseq);
}

PLAYER_API void PlayerAnimLowerUpdate(playerinfo_t *playerinfo)
{
	seqctrl_t	*seqctrl;
	paceldata_t *seqdata;
	int newseq=ASEQ_NONE;

	/*
	// First check if the lower anim is locked by the upper anim.
	if (PlayerSeqData2[playerinfo->upperseq].nosplit)
	{
		// A NONE sequence indicates that the sequence should just mimic the companion half's anim.
		playerinfo->lowerseq = ASEQ_NONE;
		playerinfo->loweridle = true;
		return;
	}
	*/
	// Init some values.
	
	playerinfo->loweridle = false;

	// Grab the sequence ctrl struct.

	if (playerinfo->edictflags & FL_CHICKEN)
		seqctrl = &ChickenCtrl[playerinfo->lowerseq];
	else
		seqctrl = &SeqCtrl[playerinfo->lowerseq];

	// Check for noclip, just to make things more robust.

	if (playerinfo->movetype == PHYSICSTYPE_NOCLIP)
	{
		if (playerinfo->lowerseq != ASEQ_STAND)
		{
			PlayerAnimSetLowerSeq(playerinfo, ASEQ_STAND);
		}
	}

	if (!newseq)	// That is if that waterseq transition wasn't necessary...
	{
		// First check the branch function. This evaluates "extra" command flags for a potential
		// modification of the "simple" procedure.

		if (seqctrl->branchfunc)
		{
			newseq = seqctrl->branchfunc(playerinfo);
		}
	}  

	// If even after the special-case BranchFunc didn't indicate a new sequence...

	if (!newseq)
	{	
		// The seqctrl indicates the control flag that this sequence is dependent on.  
		// We've defined a continue and terminate sequence depending on it.

		if (seqctrl->command != ACMD_NONE)
		{	
			if (playerinfo->seqcmd[seqctrl->command])
			{
				newseq = seqctrl->continueseq;
			}
			else 
			{
				newseq = seqctrl->ceaseseq;
			}
		}
		else
		{
			newseq = seqctrl->ceaseseq;
		}
	}
	
	// Get the pointer to the correct entry in the SeqData table.

	if (playerinfo->edictflags & FL_CHICKEN)
		seqdata = &PlayerChickenData[newseq];
	else
		seqdata = &PlayerSeqData[newseq];

	// Now check for idles.  If the lower half has an idle, then the upper half is copied.
/*	if (playerinfo->lowerseq == ASEQ_NONE)
	{
		if (playerinfo->upperseq == ASEQ_NONE)
		{
			playerinfo->upperseq = BranchIdle(self);
			playerinfo->upperidle = true;
		}
		playerinfo->loweridle = true;
		playerinfo->flags  = seqdata->playerflags | (playerinfo->flags & PLAYER_FLAG_PERSMASK);
	}
	*/

	PlayerAnimSetLowerSeq(playerinfo, newseq);
}

PLAYER_API void PlayerAnimSetVault(playerinfo_t *playerinfo, int seq)
{
	assert(playerinfo);

	PlayerAnimSetLowerSeq(playerinfo, seq);
	playerinfo->fwdvel	= 0.0;
	playerinfo->sidevel = 0.0;
	playerinfo->upvel	= 0.0;
	playerinfo->edictflags |= FL_FLY | FL_LOCKMOVE;
	playerinfo->flags = PlayerSeqData[ASEQ_VAULT_LOW].playerflags | (playerinfo->flags & PLAYER_FLAG_PERSMASK);
	playerinfo->pm_flags |= PMF_LOCKMOVE; 
	VectorClear(playerinfo->velocity);
	
	if(playerinfo->waterlevel>1)
		playerinfo->waterlevel = 1;
}

PLAYER_API void PlayerPlayPain(playerinfo_t *playerinfo, int type)
{
	int chance = irand(0,100);

	if(playerinfo->isclient)
	{
		if (!playerinfo->edictflags & FL_CHICKEN)
		{	// Chicken plays no pain sound.
			switch (type)
			{
				// Normal.

				case 0:
					if(chance < 50)
						playerinfo->CL_Sound(SND_PRED_ID40,playerinfo->origin, CHAN_VOICE, "*pain1.wav", 1.0,	ATTN_NORM, 0);
					else
						playerinfo->CL_Sound(SND_PRED_ID41,playerinfo->origin, CHAN_VOICE, "*pain2.wav", 1.0, ATTN_NORM, 0);
					
					break;

				// Gas.

				case 1:
					if(chance < 33)
						playerinfo->CL_Sound(SND_PRED_ID42,playerinfo->origin, CHAN_VOICE, "*cough1.wav", 1.0,	ATTN_NORM, 0);
					else if (chance < 66)
						playerinfo->CL_Sound(SND_PRED_ID43,playerinfo->origin, CHAN_VOICE, "*cough2.wav", 1.0, ATTN_NORM, 0);
					else 
						playerinfo->CL_Sound(SND_PRED_ID44,playerinfo->origin, CHAN_VOICE, "*cough3.wav", 1.0, ATTN_NORM, 0);

					break;

				// Small

				case 2:
					playerinfo->CL_Sound(SND_PRED_ID45,playerinfo->origin, CHAN_VOICE, "*ow.wav", 1.0, ATTN_NORM, 0);
					break;
			}
		}
	}
	else
	{
		if (!playerinfo->edictflags & FL_CHICKEN)
		{	// Chicken plays no pain sound.
			switch (type)
			{
				// Normal.

				case 0:
					if(chance < 50)
						playerinfo->G_Sound(SND_PRED_ID40,playerinfo->leveltime,playerinfo->self, CHAN_VOICE, playerinfo->G_SoundIndex("*pain1.wav"), 1.0, ATTN_NORM, 0);
					else
						playerinfo->G_Sound(SND_PRED_ID41,playerinfo->leveltime,playerinfo->self, CHAN_VOICE, playerinfo->G_SoundIndex("*pain2.wav"), 1.0, ATTN_NORM, 0);
					
					break;

				// Gas.

				case 1:
					if(chance < 33)
						playerinfo->G_Sound(SND_PRED_ID42,playerinfo->leveltime,playerinfo->self, CHAN_VOICE, playerinfo->G_SoundIndex("*cough1.wav"), 1.0, ATTN_NORM, 0);
					else if (chance < 66)
						playerinfo->G_Sound(SND_PRED_ID43,playerinfo->leveltime,playerinfo->self, CHAN_VOICE, playerinfo->G_SoundIndex("*cough2.wav"), 1.0, ATTN_NORM, 0);
					else 
						playerinfo->G_Sound(SND_PRED_ID44,playerinfo->leveltime,playerinfo->self, CHAN_VOICE, playerinfo->G_SoundIndex("*cough3.wav"), 1.0, ATTN_NORM, 0);

					break;

				// Small.

				case 2:
					playerinfo->G_Sound(SND_PRED_ID45,playerinfo->leveltime,playerinfo->self, CHAN_VOICE, playerinfo->G_SoundIndex("*ow.wav"), 1.0, ATTN_NORM, 0);
					break;
			}
		}
	}
}