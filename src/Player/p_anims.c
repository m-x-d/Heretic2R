//
// p_anims.c
//
// Copyright 1998 Raven Software
//

#include "Player.h"
#include "p_types.h"
#include "p_anim_branch.h"
#include "p_anim_data.h"
#include "p_anims.h"
#include "p_main.h"
#include "Random.h"
#include "Vector.h"
#include "EffectFlags.h"
#include "p_utility.h" //mxd

PLAYER_API void PlayerAnimSetUpperSeq(playerinfo_t* info, const int seq)
{
	assert(info);

	if (info->upperseq != seq)
	{
		// We don't set all the data up right because it's up to AnimUpdateFrame to do this.
		info->upperseq = seq;
		info->upperframe = -1;
		info->upperidle = false;
	}

	info->uppermove = PlayerSeqData[seq].move;
	info->uppermove_index = seq;

	assert(info->uppermove);

	if (info->upperseq == ASEQ_NONE)
		info->upperidle = true;
}

PLAYER_API void PlayerAnimSetLowerSeq(playerinfo_t* info, const int seq)
{
	paceldata_t* seqdata;

	assert(info);

	if (info->lowerseq != seq)
	{
		// We don't set all the data up right because it's up to AnimUpdateFrame to do this.
		info->lowerseq = seq;
		info->lowerframe = -1;
		info->loweridle = false;
	}

	if (info->edictflags & FL_CHICKEN)
		info->lowermove = PlayerChickenData[seq].move;
	else
		info->lowermove = PlayerSeqData[seq].move;

	assert(info->lowermove);

	info->lowermove_index = seq;

	// The lower two bytes of the player flags are stomped by the sequences' flags.
	if (info->edictflags & FL_CHICKEN)
	{
		seqdata = &PlayerChickenData[seq];
	}
	else
	{
		seqdata = &PlayerSeqData[seq];
		info->viewheight = (float)PlayerSeqData2[seq].viewheight;
	}

	info->flags = (int)(seqdata->playerflags | (info->flags & PLAYER_FLAG_PERSMASK));

	// Set / reset flag that says I am flying...
	if (seqdata->fly)
		info->edictflags |= FL_FLY;
	else
		info->edictflags &= ~FL_FLY;

	// Set / reset flag that says I am standing still.
	if (info->flags & PLAYER_FLAG_STAND)
		info->pm_flags |= PMF_STANDSTILL;
	else
		info->pm_flags &= ~PMF_STANDSTILL;

	if (!info->isclient)
	{
		// Set/reset flag that says I am movelocked.
		if (seqdata->lockmove)
			info->pm_flags |= PMF_LOCKMOVE;
		else
			info->pm_flags &= ~PMF_LOCKMOVE;

		//mxd. Set/reset flag that says I am roping.
		if (info->flags & PLAYER_FLAG_ONROPE)
			info->pm_flags |= PMF_ON_ROPE;
		else
			info->pm_flags &= ~PMF_ON_ROPE;
	}
}

PLAYER_API void PlayerBasicAnimReset(playerinfo_t* info)
{
	PlayerAnimSetLowerSeq(info, ASEQ_STAND);
	info->lowerframeptr = info->lowermove->frame;

	PlayerAnimSetUpperSeq(info, ASEQ_NONE);
	info->upperframeptr = info->uppermove->frame;

	info->effects |= EF_SWAPFRAME;
	info->effects &= ~(EF_DISABLE_EXTRA_FX | EF_ON_FIRE | EF_TRAILS_ENABLED);

	PlayerSetHandFX(info, HANDFX_NONE, -1);

	if (info->pers.weaponready == WEAPON_READY_NONE) // Just in case we die with WEAPON_READY_NONE.
		info->pers.weaponready = WEAPON_READY_HANDS;

	info->switchtoweapon = info->pers.weaponready;
	info->pers.newweapon = NULL;

	// Straighten out joints, i.e. reset torso twisting.
	if (!(info->edictflags & FL_CHICKEN))
		info->ResetJointAngles(info);

	memset(info->seqcmd, 0, sizeof(info->seqcmd));
}

PLAYER_API void PlayerAnimReset(playerinfo_t* info)
{
	PlayerAnimSetLowerSeq(info, ASEQ_STAND);
	info->lowerframeptr = info->lowermove->frame;

	PlayerAnimSetUpperSeq(info, ASEQ_NONE);
	info->upperframeptr = info->uppermove->frame;

	info->pers.armortype = ARMOR_TYPE_NONE;
	info->pers.bowtype = BOW_TYPE_NONE;
	info->pers.stafflevel = STAFF_LEVEL_BASIC;
	info->pers.helltype = HELL_TYPE_BASIC;
	info->pers.altparts = 0;
	info->pers.weaponready = WEAPON_READY_HANDS;
	info->switchtoweapon = WEAPON_READY_HANDS;
	info->pers.newweapon = NULL;
	PlayerUpdateModelAttributes(info);

	PlayerSetHandFX(info, HANDFX_NONE, -1);

	info->effects |= EF_SWAPFRAME;
	info->effects &= ~(EF_DISABLE_EXTRA_FX | EF_ON_FIRE | EF_TRAILS_ENABLED);

	// Straighten out joints, i.e. no torso aiming.
	if (!(info->edictflags & FL_CHICKEN))
		info->ResetJointAngles(info);

	memset(info->seqcmd, 0, sizeof(info->seqcmd));
}

int PlayerAnimWeaponSwitch(playerinfo_t* info)
{
	int newseq = ASEQ_NONE;

	assert(info);

	// See if we have the arm to do that magic.
	if (info->switchtoweapon != info->pers.weaponready && BranchCheckDismemberAction(info, info->switchtoweapon))
		newseq = PlayerAnimWeaponSwitchSeq[info->pers.weaponready][info->switchtoweapon];
	else if (info->pers.newweapon != NULL && BranchCheckDismemberAction(info, info->pers.newweapon->tag))
		newseq = PlayerAnimWeaponSwitchSeq[info->pers.weaponready][info->pers.weaponready];

	if (newseq != ASEQ_NONE)
		PlayerAnimSetUpperSeq(info, newseq);

	return newseq;
}

PLAYER_API void PlayerAnimUpperIdle(playerinfo_t* info)
{
	const int ret = BranchUprReady(info);

	if (ret != ASEQ_NONE)
		PlayerAnimSetUpperSeq(info, ret);

	assert(info->uppermove);
}

PLAYER_API void PlayerAnimLowerIdle(playerinfo_t* info)
{
	if (info->flags & PLAYER_FLAG_SURFSWIM)
	{
		const int ret = BranchLwrSurfaceSwim(info);
		if (ret != ASEQ_NONE)
			PlayerAnimSetLowerSeq(info, ret);

		return;
	}

	if (info->flags & PLAYER_FLAG_UNDERWATER)
	{
		const int ret = BranchLwrUnderwaterSwim(info);
		if (ret != ASEQ_NONE)
			PlayerAnimSetLowerSeq(info, ret);

		return;
	}

	if (info->flags & PLAYER_FLAG_ONROPE)
	{
		const int ret = BranchLwrClimbing(info);
		if (ret != ASEQ_NONE)
			PlayerAnimSetLowerSeq(info, ret);

		return;
	}

	const int ret = BranchLwrStanding(info);
	if (ret != ASEQ_NONE)
	{
		PlayerAnimSetLowerSeq(info, ret);
		return;
	}

	// Not a time to be idling, yet.
	if (info->leveltime - info->idletime < 15.0f)
		return;

	if (info->lowerseq >= ASEQ_IDLE_READY_GO && info->lowerseq <= ASEQ_IDLE_LOOKR && info->lowerseq != ASEQ_IDLE_READY_END)
	{
		// Only certain idle should be called out of here.
		switch (info->irand(info, 0, 3))
		{
			case 0:  PlayerAnimSetLowerSeq(info, ASEQ_IDLE_LOOKL); break;
			case 1:  PlayerAnimSetLowerSeq(info, ASEQ_IDLE_LOOKR); break;
			case 2:  PlayerAnimSetLowerSeq(info, ASEQ_IDLE_READY_END); break;
			default: PlayerAnimSetLowerSeq(info, ASEQ_IDLE_READY); break;
		}

		return;
	}

	// If we are in a cinematic, always do this idle, since its silent.
	if (info->sv_cinematicfreeze != 0.0f)
	{
		PlayerAnimSetLowerSeq(info, ASEQ_IDLE_LOOKBACK);
		return;
	}

	// Because the bow doesn't look right in some idles.
	if (info->pers.weaponready == WEAPON_READY_BOW || info->isclient) //TODO: why isclient check?..
	{
		switch (info->irand(info, 0, 2))
		{
			case 0:  PlayerAnimSetLowerSeq(info, ASEQ_IDLE_SCRATCH_ASS); break;
			case 1:  PlayerAnimSetLowerSeq(info, ASEQ_IDLE_LOOKBACK); break;
			default: PlayerAnimSetLowerSeq(info, ASEQ_IDLE_READY_GO); break;
		}

		return;
	}

	// Because the staff doesn't look right in some idles.
	if (info->pers.weaponready == WEAPON_READY_SWORDSTAFF)
	{
		switch (info->irand(info, 0, 3))
		{
			case 0:  PlayerAnimSetLowerSeq(info, ASEQ_IDLE_FLY1); break;
			case 1:  PlayerAnimSetLowerSeq(info, ASEQ_IDLE_FLY2); break;
			case 2:  PlayerAnimSetLowerSeq(info, ASEQ_IDLE_WIPE_BROW); break;
			default: PlayerAnimSetLowerSeq(info, ASEQ_IDLE_READY_GO); break;
		}

		return;
	}

	// Default idle animations.
	switch (info->irand(info, 0, 6))
	{
		case 0:  PlayerAnimSetLowerSeq(info, ASEQ_IDLE_FLY1); break;
		case 1:  PlayerAnimSetLowerSeq(info, ASEQ_IDLE_FLY2); break;
		case 2:  PlayerAnimSetLowerSeq(info, ASEQ_IDLE_SCRATCH_ASS); break;
		case 3:  PlayerAnimSetLowerSeq(info, ASEQ_IDLE_LOOKBACK); break;
		case 4:  PlayerAnimSetLowerSeq(info, ASEQ_IDLE_WIPE_BROW); break;
		default: PlayerAnimSetLowerSeq(info, ASEQ_IDLE_READY_GO); break;
	}
}

PLAYER_API void PlayerAnimUpperUpdate(playerinfo_t* info) // Exported function never called --mxd.
{
	// Init some values.
	info->upperidle = false;

	// Grab the sequence ctrl struct.
	const seqctrl_t* seqctrl = &SeqCtrl[info->upperseq];

	// First check the branch function. This evaluates "extra" command flags for a potential modification of the "simple" procedure.
	int newseq = (seqctrl->branchfunc != NULL ? seqctrl->branchfunc(info) : ASEQ_NONE);

	if (newseq == ASEQ_NONE)
	{
		// The seqctrl indicates the control flag that this sequence is dependent on.  
		// We've defined a continue and terminate sequence depending on it.
		if (seqctrl->command != ACMD_NONE && info->seqcmd[seqctrl->command] != 0)
			newseq = seqctrl->continueseq;
		else
			newseq = seqctrl->ceaseseq;
	}

	// Now check for idles. If the upper half has an idle, then the upper half is copied.
	if (newseq == ASEQ_NONE)
	{
		if (info->lowerseq == ASEQ_NONE)
		{
			newseq = BranchIdle(info);
			info->loweridle = true;
		}

		info->upperidle = true;
	}

	PlayerAnimSetUpperSeq(info, newseq);
}

PLAYER_API void PlayerAnimLowerUpdate(playerinfo_t* info) // Exported function never called --mxd.
{
	// Init some values.
	info->loweridle = false;

	// Grab the sequence ctrl struct.
	const seqctrl_t* seqctrl = ((info->edictflags & FL_CHICKEN) ? &ChickenCtrl[info->lowerseq] : &SeqCtrl[info->lowerseq]);

	// First check the branch function. This evaluates "extra" command flags for a potential modification of the "simple" procedure.
	int newseq = (seqctrl->branchfunc != NULL ? seqctrl->branchfunc(info) : ASEQ_NONE);

	// If even after the special-case BranchFunc didn't indicate a new sequence...
	if (newseq == ASEQ_NONE)
	{
		// The seqctrl indicates the control flag that this sequence is dependent on.  
		// We've defined a continue and terminate sequence depending on it.
		if (seqctrl->command != ACMD_NONE && info->seqcmd[seqctrl->command] != 0)
			newseq = seqctrl->continueseq;
		else
			newseq = seqctrl->ceaseseq;
	}

	PlayerAnimSetLowerSeq(info, newseq);
}

PLAYER_API void PlayerAnimSetVault(playerinfo_t* info, const int seq)
{
	assert(info);

	PlayerAnimSetLowerSeq(info, seq);

	info->fwdvel = 0.0f;
	info->sidevel = 0.0f;
	info->upvel = 0.0f;
	info->edictflags |= (FL_FLY | FL_LOCKMOVE);
	info->flags = (int)(PlayerSeqData[ASEQ_VAULT_LOW].playerflags | (info->flags & PLAYER_FLAG_PERSMASK));
	info->pm_flags |= PMF_LOCKMOVE;
	VectorClear(info->velocity);

	info->waterlevel = min(info->waterlevel, 1);
}

PLAYER_API void PlayerPlayPain(const playerinfo_t* info, const int type)
{
	// Chicken plays no pain sound (because it has 1 HP). //TODO: what about super-chicken? There are pain sounds for it.
	if (info->edictflags & FL_CHICKEN)
		return;

	const int chance = irand(0, 100);

	switch (type)
	{
		// Normal.
		case 0:
			if (chance < 50)
				P_Sound(info, SND_PRED_ID40, CHAN_VOICE, "*pain1.wav", 1.0f); //mxd
			else
				P_Sound(info, SND_PRED_ID41, CHAN_VOICE, "*pain2.wav", 1.0f); //mxd
			break;

		// Gas.
		case 1:
			if (chance < 33)
				P_Sound(info, SND_PRED_ID42, CHAN_VOICE, "*cough1.wav", 1.0f); //mxd
			else if (chance < 66)
				P_Sound(info, SND_PRED_ID43, CHAN_VOICE, "*cough2.wav", 1.0f); //mxd
			else
				P_Sound(info, SND_PRED_ID44, CHAN_VOICE, "*cough3.wav", 1.0f); //mxd
			break;

		// Small
		case 2:
			P_Sound(info, SND_PRED_ID45, CHAN_VOICE, "*ow.wav", 1.0f); //mxd
			break;
	}
}