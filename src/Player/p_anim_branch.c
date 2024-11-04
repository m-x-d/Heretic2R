//
// p_anim_branch.c
//
// Heretic II
// Copyright 1998 Raven Software
//

#include "player.h"
#include "p_types.h"
#include "p_actions.h"
#include "p_anims.h"
#include "g_items.h"
#include "p_main.h"
#include "p_weapon.h"
#include "fx.h"
#include "random.h"
#include "vector.h"
#include "g_physics.h"
#include "p_anim_data.h"
#include "m_player.h"
#include "g_playstats.h"

#define FALL_MINHEIGHT	34

int BranchIdle(playerinfo_t *playerinfo);
int CheckSlopedStand (playerinfo_t *playerinfo);

vec3_t footmins = {-1.0, -1.0, 0};
vec3_t footmaxs = {1.0, 1.0, 1.0};

float SLIDEZPUSH = 48;
float SLIDEXYPUSH = 128;

/*-----------------------------------------------
	CheckFall
-----------------------------------------------*/

qboolean CheckFall(playerinfo_t *playerinfo)
{
	trace_t checktrace;
	vec3_t endpos;
	int	checksloped = ASEQ_NONE;

	VectorCopy(playerinfo->origin, endpos);
	endpos[2] -= FALL_MINHEIGHT;

	if(playerinfo->isclient)
		playerinfo->CL_Trace(playerinfo->origin,playerinfo->mins,playerinfo->maxs,endpos,MASK_PLAYERSOLID,CEF_CLIP_TO_WORLD,&checktrace);
	else
		playerinfo->G_Trace(playerinfo->origin, playerinfo->mins, playerinfo->maxs, endpos, playerinfo->self, MASK_PLAYERSOLID,&checktrace);

	if (checktrace.fraction >= 1)
		return true;

	return false;
}

/*-----------------------------------------------
	CheckUncrouch
-----------------------------------------------*/

qboolean CheckUncrouch(playerinfo_t *playerinfo)
{
	trace_t trace;
	vec3_t	v;

	VectorCopy(playerinfo->origin,v);
	v[2]+=25.0 - playerinfo->maxs[2];//was 25

	if(playerinfo->isclient)
		playerinfo->CL_Trace(playerinfo->origin, playerinfo->mins, playerinfo->maxs, v, MASK_PLAYERSOLID,CEF_CLIP_TO_WORLD,&trace);
	else
		playerinfo->G_Trace(playerinfo->origin, playerinfo->mins, playerinfo->maxs, v, playerinfo->self, MASK_PLAYERSOLID,&trace);

	if (trace.fraction < 1)
		return(false);

	if (trace.allsolid || trace.startsolid)
		return(false);

	return(true);
}

/*-----------------------------------------------
	CheckCreep
-----------------------------------------------*/

qboolean CheckCreep(playerinfo_t *playerinfo, int dir)
{
	trace_t checktrace;
	vec3_t startpos,endpos, vf, ang, mins;

	//Scan out and down from the player
	VectorCopy(playerinfo->origin, startpos);
	
	//Ignore the pitch of the player, we only want the yaw
	VectorSet(ang, 0, playerinfo->angles[YAW], 0);
	AngleVectors(ang, vf, NULL, NULL);
	
	//Trace ahead about one step (dir is 1 for forward, -1 for back)
	VectorMA(playerinfo->origin, dir*CREEP_STEPDIST, vf, startpos);

	//Account for stepheight
	VectorCopy(playerinfo->mins, mins);
	mins[2] += CREEP_MAXFALL;

	//Trace forward to see if the path is clear
	if(playerinfo->isclient)
		playerinfo->CL_Trace(playerinfo->origin,mins,playerinfo->maxs,startpos,MASK_PLAYERSOLID,CEF_CLIP_TO_WORLD,&checktrace);
	else
		playerinfo->G_Trace(playerinfo->origin, mins, playerinfo->maxs, startpos, playerinfo->self, MASK_PLAYERSOLID,&checktrace);
	
	//If it is...
	if (checktrace.fraction == 1)
	{
		//Move the endpoint down the maximum amount
		VectorCopy(startpos, endpos);
		endpos[2] += (playerinfo->mins[2] - CREEP_MAXFALL);

		//Trace down
		if(playerinfo->isclient)
			playerinfo->CL_Trace(startpos,mins,playerinfo->maxs,endpos,MASK_PLAYERSOLID,CEF_CLIP_TO_WORLD,&checktrace);
		else
			playerinfo->G_Trace(startpos, mins, playerinfo->maxs, endpos, playerinfo->self, MASK_PLAYERSOLID,&checktrace);

		if (checktrace.fraction == 1 || (checktrace.startsolid || checktrace.allsolid))
		{
			//Creep would take us off a ledge
			return false;		
		}
	}
	else
	{
		//Creep would take us off a ledge
		return false;
	}

	//Clear to creep
	return true;
}

/*-----------------------------------------------
	CheckSlopedStand
-----------------------------------------------*/

int CheckSlopedStand (playerinfo_t *playerinfo)
{
	trace_t		leftfoot, rightfoot;
	vec3_t		lspotmax, lspotmin, rspotmax, rspotmin;
	vec3_t		player_facing;
	vec3_t		forward, right;
	float		footdiff;

#if	0
	// When this code is moved to the player dll
	// the init code should grab a cl_predict cvar ptr
	if(cl_predict->value)
	{
	 	return ASEQ_NONE;
	}
#endif

	VectorCopy(playerinfo->angles, player_facing);
	player_facing[PITCH] = player_facing[ROLL] = 0.0;
	AngleVectors(player_facing, forward, right, NULL);

	// Get player origin
	VectorCopy(playerinfo->origin, lspotmax);
	VectorCopy(playerinfo->origin, rspotmax);

	// Magic number calc for foot placement
	VectorMA(lspotmax, -9.8, right, lspotmax);
	VectorMA(lspotmax, 7.2, forward, lspotmax);

	VectorMA(rspotmax, 10.5, right, rspotmax);
	VectorMA(rspotmax, -2.6, forward, rspotmax);

	VectorCopy(lspotmax, lspotmin);
	VectorCopy(rspotmax, rspotmin);

	// Go half player height below player
	lspotmin[2] += playerinfo->mins[2] * 2.0;
	rspotmin[2] += playerinfo->mins[2] * 2.0;

	if(playerinfo->isclient)
	{
		playerinfo->CL_Trace(lspotmax, footmins, footmaxs, lspotmin, MASK_PLAYERSOLID, CEF_CLIP_TO_WORLD, &leftfoot);
	}
	else
	{
		 playerinfo->G_Trace(lspotmax, footmins, footmaxs, lspotmin, playerinfo->self, MASK_PLAYERSOLID,&leftfoot);
	}

	if(playerinfo->isclient)
	{
		playerinfo->CL_Trace(rspotmax, footmins, footmaxs, rspotmin, MASK_PLAYERSOLID, CEF_CLIP_TO_WORLD, &rightfoot);		
	}
	else
	{
		playerinfo->G_Trace(rspotmax, footmins, footmaxs, rspotmin, playerinfo->self, MASK_PLAYERSOLID,&rightfoot);
	}

	if((rightfoot.fraction == 1.0) && !rightfoot.startsolid && !rightfoot.allsolid)
	{
		return ASEQ_LSTAIR16;
	}
	else if((leftfoot.fraction == 1.0) && !leftfoot.startsolid && !leftfoot.allsolid)
	{
		return ASEQ_RSTAIR16;
	}
	else
	{
		footdiff = rightfoot.endpos[2] - leftfoot.endpos[2];

		if(footdiff >= 13.0)			//right foot 13 or more higher
			return ASEQ_RSTAIR16;
		else if(footdiff >= 9.0)		//right foot 9 or more higher
			return ASEQ_RSTAIR12;
		else if(footdiff >= 5.0)		//right foot 8 or more higher
			return ASEQ_RSTAIR8;
		else if(footdiff >= 2.0)		//right foot 2 or more higher
			return ASEQ_RSTAIR4;
		else if(footdiff >= -2.0)		//flat
			return ASEQ_NONE;
		else if(footdiff >= -5.0)		//left foot 4 or less higher
			return ASEQ_LSTAIR4;
		else if(footdiff >= -9.0)		//left foot 8 or less higher
			return ASEQ_LSTAIR8;
		else if(footdiff >= -13.0)		//left foot 12 or less higher
			return ASEQ_LSTAIR12;

		return ASEQ_LSTAIR16;
	}
	return ASEQ_NONE;
}

/*-----------------------------------------------
	BranchCheckDismemberAction
-----------------------------------------------*/

PLAYER_API qboolean BranchCheckDismemberAction(playerinfo_t *playerinfo, int weapon)
{
	//If these nodes are on, then any weapon selection is a valid one
	if ( (!(playerinfo->flags & PLAYER_FLAG_NO_RARM)) && (!(playerinfo->flags & PLAYER_FLAG_NO_LARM)) )
		return true;

	//No arm, no shot
	if (weapon == ITEM_WEAPON_FLYINGFIST)
	{
		if (playerinfo->flags & PLAYER_FLAG_NO_RARM)
			return false;
	}
	else if (weapon == ITEM_WEAPON_MAGICMISSILE || weapon == ITEM_WEAPON_MACEBALLS)
	{
		//Powered up is right arm, non-powered is left
		if (playerinfo->powerup_timer > playerinfo->leveltime)
		{
			if (playerinfo->flags & PLAYER_FLAG_NO_RARM)
				return false;
		}
		else
		{
			if (playerinfo->flags & PLAYER_FLAG_NO_LARM)
				return false;
		}
	}
	else if (weapon == ITEM_WEAPON_HELLSTAFF || weapon == ITEM_WEAPON_SWORDSTAFF)
	{
		if (playerinfo->flags & PLAYER_FLAG_NO_RARM)
			return false;
	}
	else	//Any other weapon will need both hands
	{
		if (playerinfo->flags & PLAYER_FLAG_NO_LARM || playerinfo->flags & PLAYER_FLAG_NO_RARM)
			return false;
	}

	//Player is able to complete the action
	return true;
}


/*-----------------------------------------------
	ChickenBranchLwrStanding
-----------------------------------------------*/

int ChickenBranchLwrStanding(playerinfo_t *playerinfo)
{
	int	temp;
	int	checksloped = ASEQ_NONE;

	assert(playerinfo);

	// Decide if we have just fallen off something, or we are falling down.

	//Since the chicken runs through this always, make sure we don't check for falling in the water!
	if (playerinfo->groundentity==NULL && playerinfo->waterlevel < 2)
	{
		if (CheckFall(playerinfo))
			return ASEQ_FALL;
	}

	// Decide what we should be doing now.
	if (playerinfo->waterlevel >= 2)
	{
		if (playerinfo->seqcmd[ACMDL_FWD])
			return ASEQ_USWIMF_GO;
	}
	else if(playerinfo->seqcmd[ACMDU_ATTACK])
	{
		if (playerinfo->seqcmd[ACMDL_FWD])
			return(ASEQ_RUNF);
		else
			return(ASEQ_WSWORD_SPIN);
	}
	else if(playerinfo->seqcmd[ACMDL_JUMP])
	{
		if(playerinfo->seqcmd[ACMDL_WALK_F])
			return(ASEQ_JUMPFWD_WGO);
		else if (playerinfo->seqcmd[ACMDL_RUN_F])
			return(ASEQ_JUMPFWD_RGO);
		else if (playerinfo->seqcmd[ACMDL_BACK])
			return(ASEQ_JUMPBACK_SGO);
		else if (playerinfo->seqcmd[ACMDL_RUN_B])	
			return(ASEQ_JUMPBACK_SGO);
		else 
			return ASEQ_JUMPUP;
	}
	else if (playerinfo->seqcmd[ACMDL_WALK_F])
		return ASEQ_WALKF_GO;
	else if (playerinfo->seqcmd[ACMDL_RUN_F])
		return ASEQ_RUNF_GO;
	else if (playerinfo->seqcmd[ACMDL_RUN_B])		
		return ASEQ_WSTRAFE_LEFT;
	else if (playerinfo->seqcmd[ACMDL_BACK])
		return ASEQ_WALKB;
	else if (playerinfo->seqcmd[ACMDL_STRAFE_L])
		return ASEQ_STRAFEL;
	else if (playerinfo->seqcmd[ACMDL_STRAFE_R])
		return ASEQ_STRAFER;

	// If we've reached this point, we are still idling - so decide if which one we want to do.
	
// OK NOW?	if(!playerinfo->isclient)
	{
		temp = (playerinfo->irand(playerinfo,0,5));
		
		if(!temp)
			return ASEQ_IDLE_LOOKR;
		
		temp--;
		
		if(!temp)
			return ASEQ_IDLE_LOOKL;
	}
	return(ASEQ_NONE);
}

// This allows the chicken to interupt itself - if its idling.

/*-----------------------------------------------
	ChickenBranchidle
-----------------------------------------------*/

int ChickenBranchidle(playerinfo_t *playerinfo)
{
	// Do we need to attack ?
	if (playerinfo->seqcmd[ACMDU_ATTACK])
	{
		PlayerAnimSetLowerSeq(playerinfo, ASEQ_WSWORD_SPIN);
		return(ASEQ_WSWORD_SPIN);
	}
	else if (playerinfo->seqcmd[ACMDL_WALK_F])
	{
		// We are walking forward... want to jump?
		if (playerinfo->seqcmd[ACMDL_JUMP])
		{
			PlayerAnimSetLowerSeq(playerinfo, ASEQ_JUMPFWD_WGO);
			return ASEQ_JUMPFWD_WGO;
		}
		else
		{
			// No, walk forward.
			PlayerAnimSetLowerSeq(playerinfo, ASEQ_WALKF_GO);
			return ASEQ_WALKF_GO;
		}
	}
	else if (playerinfo->seqcmd[ACMDL_RUN_F])
	{
		// 	We are running forward... want to jump?
		if (playerinfo->seqcmd[ACMDL_JUMP])
		{
			PlayerAnimSetLowerSeq(playerinfo, ASEQ_JUMPFWD_RGO);
			return ASEQ_JUMPFWD_RGO;
		}
		else
		{
			// No, run forward.
			PlayerAnimSetLowerSeq(playerinfo, ASEQ_RUNF_GO);
			return ASEQ_RUNF_GO;
		}
	}
	else if (playerinfo->seqcmd[ACMDL_RUN_B])		
	{
		// We are running backward... want to jump?
		if (playerinfo->seqcmd[ACMDL_JUMP])
		{
			PlayerAnimSetLowerSeq(playerinfo, ASEQ_JUMPFLIPL);
			return ASEQ_JUMPFLIPL;
		}
		else
		{
			// No, run backward
			PlayerAnimSetLowerSeq(playerinfo, ASEQ_WSTRAFE_LEFT);
			return ASEQ_WSTRAFE_LEFT;
		}
	}
	else if (playerinfo->seqcmd[ACMDL_BACK])
	{
		// We are walking backward... want to jump?
		if (playerinfo->seqcmd[ACMDL_JUMP])
		{
			PlayerAnimSetLowerSeq(playerinfo, ASEQ_JUMPBACK);
			return ASEQ_JUMPBACK;
		}
		else
		{
			// No, walk backward.
			PlayerAnimSetLowerSeq(playerinfo, ASEQ_WALKB);
			return ASEQ_WALKB;
		}
	}
	else if (playerinfo->seqcmd[ACMDL_STRAFE_L])
	{
		// Strafing left.
		PlayerAnimSetLowerSeq(playerinfo, ASEQ_STRAFEL);
		return ASEQ_STRAFEL;
	}
	else if (playerinfo->seqcmd[ACMDL_STRAFE_R])
	{
		// Strafing right.
		PlayerAnimSetLowerSeq(playerinfo, ASEQ_STRAFER);
		return ASEQ_STRAFER;
	}
	else if (playerinfo->seqcmd[ACMDL_JUMP])
	{
		// Jumping in place.
		PlayerAnimSetLowerSeq(playerinfo, ASEQ_JUMPUP);
		return ASEQ_JUMPUP;
	}

	return(ASEQ_NONE);
}

/*-----------------------------------------------
	BranchLwrStanding
-----------------------------------------------*/

int BranchLwrStanding(playerinfo_t *playerinfo)
{
	int	checksloped = ASEQ_NONE;

	if (playerinfo->deadflag)
	{
		//assert(0);
		return ASEQ_NONE;
	}

	assert(playerinfo);

	if (playerinfo->advancedstaff &&			// Special move
			playerinfo->seqcmd[ACMDL_ACTION] && 
			playerinfo->pers.weaponready == WEAPON_READY_SWORDSTAFF && 
			playerinfo->seqcmd[ACMDU_ATTACK] &&
			BranchCheckDismemberAction(playerinfo, ITEM_WEAPON_SWORDSTAFF))
	{
		return ASEQ_WSWORD_LOWERDOWNSTAB;
	}

	//Check to update the idles
	if (playerinfo->lowerseq != ASEQ_STAND && playerinfo->lowerseq != ASEQ_IDLE_READY)
		playerinfo->idletime = playerinfo->leveltime;

	//Check for a fall
	if (playerinfo->groundentity==NULL && playerinfo->waterlevel < 2)
	{
		if (CheckFall(playerinfo))
			return ASEQ_FALL;
	}

	//Check for a jump
	if (playerinfo->seqcmd[ACMDL_JUMP])
	{
		if (!(playerinfo->watertype & (CONTENTS_SLIME|CONTENTS_LAVA)))
		{
			if (playerinfo->seqcmd[ACMDL_FWD])
				return ASEQ_JUMPFWD_SGO;
			else if (playerinfo->seqcmd[ACMDL_BACK])
				return ASEQ_JUMPBACK_SGO;
			if (playerinfo->seqcmd[ACMDL_STRAFE_L])
				return ASEQ_JUMPLEFT_SGO;
			else if (playerinfo->seqcmd[ACMDL_STRAFE_R])
				return ASEQ_JUMPRIGHT_SGO;

			return ASEQ_JUMPSTD_GO;
		}
	}

	//Check for a crouch
	if (playerinfo->seqcmd[ACMDL_CROUCH])
	{
		if (playerinfo->seqcmd[ACMDL_FWD])
		{
			playerinfo->maxs[2] = 4;
			return ASEQ_ROLLDIVEF_W;
		}
		else if (playerinfo->seqcmd[ACMDL_BACK])
			return ASEQ_ROLL_B;
		else if (playerinfo->seqcmd[ACMDL_STRAFE_L])
			return ASEQ_ROLL_L;
		else if (playerinfo->seqcmd[ACMDL_STRAFE_R])
			return ASEQ_ROLL_R;

		return ASEQ_CROUCH_GO;
	}
	
	//FORWARD 

	//Check for a walk speed start
	if (playerinfo->seqcmd[ACMDL_WALK_F])
		return ASEQ_WALKF_GO;
	
	//Check for a run start
	if (playerinfo->seqcmd[ACMDL_RUN_F])
		return ASEQ_RUNF_GO;
	
	//Check for creep start
	if (playerinfo->seqcmd[ACMDL_CREEP_F])
	{
		if (CheckCreep(playerinfo, 1))
			return ASEQ_CREEPF;
		else
			return ASEQ_STAND;
	}

	
	//BACKWARD 

	//Check for a creepback
	if (playerinfo->seqcmd[ACMDL_CREEP_B])
	{
		if (CheckCreep(playerinfo, -1))
			return ASEQ_CREEPB;
		else
			return ASEQ_STAND;
	}

	//Check for a walk back
	if (playerinfo->seqcmd[ACMDL_WALK_B])
		return ASEQ_WALKB;

	//Check for a backspring
	if ( (playerinfo->seqcmd[ACMDL_RUN_B]) && ((!playerinfo->seqcmd[ACMDL_STRAFE_L]) && (!playerinfo->seqcmd[ACMDL_STRAFE_R])) )
	{
		if (!(playerinfo->seqcmd[ACMDU_ATTACK]) && playerinfo->upperidle && !(playerinfo->dmflags))
			return ASEQ_JUMPSPRINGBGO;
		else
			return ASEQ_WALKB;
	}
	
	//STRAFES

	//Check for a strafe left
	if (playerinfo->seqcmd[ACMDL_STRAFE_L])
	{
		if (playerinfo->seqcmd[ACMDL_BACK])
			return ASEQ_WSTRAFEB_LEFT;

		if (playerinfo->pcmd.buttons & BUTTON_RUN)
			return ASEQ_DASH_LEFT_GO;

		return ASEQ_STRAFEL;
	}
	
	//Check for a strafe right
	if (playerinfo->seqcmd[ACMDL_STRAFE_R])
	{
		if (playerinfo->seqcmd[ACMDL_BACK])
			return ASEQ_WSTRAFEB_RIGHT;

		if (playerinfo->pcmd.buttons & BUTTON_RUN)
			return ASEQ_DASH_RIGHT_GO;

		return ASEQ_STRAFER;
	}

	//Check for action bein held
	if (playerinfo->seqcmd[ACMDL_ACTION])
	{
		if (playerinfo->upperidle && PlayerActionCheckPushLever (playerinfo))
		{
			playerinfo->target = NULL;
			if (!(playerinfo->fmnodeinfo[MESH__BOWACTV].flags  & FMNI_NO_DRAW))
				return ASEQ_PUSHLEVERRIGHT;
			else
				return ASEQ_PUSHLEVERLEFT;
		}
		else if (playerinfo->upperidle && PlayerActionCheckPushButton (playerinfo))
		{
			playerinfo->target = NULL;
			return ASEQ_PUSHBUTTON_GO;
		}
		else if (PlayerActionCheckPuzzleGrab(playerinfo)) 	// Are you near a puzzle piece? Then try to take it
		{
			return ASEQ_TAKEPUZZLEPIECE;
		}
		else if (PlayerActionUsePuzzle(playerinfo)) 	// Trying to use a puzzle piece
		{
			return ASEQ_NONE;	// Need anim to use puzzle piece
		}
		else if ( (playerinfo->targetEnt) && (PlayerActionCheckRopeGrab(playerinfo,0)) ) //Climb a rope?
		{
			if(playerinfo->isclient)
				playerinfo->CL_Sound(SND_PRED_ID32, playerinfo->origin, CHAN_VOICE, "player/ropegrab.wav", 0.75, ATTN_NORM, 0);
			else
				playerinfo->G_Sound(SND_PRED_ID32, playerinfo->leveltime, playerinfo->self, CHAN_VOICE, playerinfo->G_SoundIndex("player/ropegrab.wav"), 0.75, ATTN_NORM, 0);

			return ASEQ_CLIMB_ON;
		}
		else if (playerinfo->upperidle && PlayerActionCheckJumpGrab(playerinfo, 0))
		{	
			return ASEQ_JUMPSTD_GO;
		}
	}

	//Check for an autovault
	if ( (playerinfo->flags & PLAYER_FLAG_COLLISION) &&  (playerinfo->upperidle) && (playerinfo->seqcmd[ACMDL_FWD]) )
	{
		PlayerActionCheckVault(playerinfo, 0);

		if (playerinfo->lowerseq == ASEQ_VAULT_LOW)
			return ASEQ_VAULT_LOW;

		if (playerinfo->lowerseq == ASEQ_PULLUP_HALFWALL)
			return ASEQ_PULLUP_HALFWALL;
	}

	//Check for a quickturn
	if (playerinfo->seqcmd[ACMDL_QUICKTURN])
		return ASEQ_TURN180;
	
	//Check for a rotation left
	if (playerinfo->seqcmd[ACMDL_ROTATE_L])
	{
		if (!(playerinfo->lowerseq >= ASEQ_PIVOTL_GO && playerinfo->lowerseq <= ASEQ_PIVOTL_END))
		{
			return ASEQ_PIVOTL_GO;
		}
		else
			return ASEQ_NONE;
	}
	
	//Check for a rotation right
	if (playerinfo->seqcmd[ACMDL_ROTATE_R])
	{	
		if (!(playerinfo->lowerseq >= ASEQ_PIVOTR_GO && playerinfo->lowerseq <= ASEQ_PIVOTR_END))
		{
			return ASEQ_PIVOTR_GO;
		}
		else
			return ASEQ_NONE;
	}

	//Check for a sloped stand 
	if (playerinfo->isclient&&((playerinfo->lowerseq >= ASEQ_LSTAIR4 && playerinfo->lowerseq <= ASEQ_RSTAIR16)||playerinfo->lowerseq==ASEQ_STAND))
		return playerinfo->lowerseq;
	
	checksloped = CheckSlopedStand(playerinfo);
	
	if(checksloped)
		return checksloped;

	playerinfo->loweridle = true;
	if (playerinfo->lowerseq >= ASEQ_LSTAIR4 && playerinfo->lowerseq <= ASEQ_RSTAIR16)
		return ASEQ_STAND;	//if was standing on stairs, go to stand

	return ASEQ_NONE;
}

// I call this when I end a move and perhaps want to start running or otherwise moving immediately without delay, such as after a pullup.
/*-----------------------------------------------
	BranchLwrStandingRun
-----------------------------------------------*/

int BranchLwrStandingRun(playerinfo_t *playerinfo)
{
	int	checksloped = false;

	assert(playerinfo);

	if (playerinfo->groundentity==NULL && playerinfo->waterlevel < 2)
	{
		if (CheckFall(playerinfo))
			return ASEQ_FALL;
	}

	if ((playerinfo->seqcmd[ACMDL_FWD]) && playerinfo->upperidle)
	{
		PlayerActionCheckVault(playerinfo, 0);
		if (playerinfo->lowerseq == ASEQ_VAULT_LOW)
			return ASEQ_VAULT_LOW;

		if (playerinfo->lowerseq == ASEQ_PULLUP_HALFWALL)
			return ASEQ_PULLUP_HALFWALL;
	}

	if (playerinfo->seqcmd[ACMDL_JUMP])
		return ASEQ_JUMPSTD_GO;
	else if (playerinfo->seqcmd[ACMDL_CREEP_B])
		return ASEQ_CREEPB;
	else if (playerinfo->seqcmd[ACMDL_CROUCH])
	{
		if (playerinfo->seqcmd[ACMDL_FWD])
			return ASEQ_ROLLDIVEF_W;
		else if (playerinfo->seqcmd[ACMDL_BACK])
			return ASEQ_ROLL_B;
		else if (playerinfo->seqcmd[ACMDL_STRAFE_L])
			return ASEQ_ROLL_L;
		else if (playerinfo->seqcmd[ACMDL_STRAFE_R])
			return ASEQ_ROLL_R;

		return ASEQ_CROUCH_GO;
	}
	else if (playerinfo->seqcmd[ACMDL_WALK_F])
		return ASEQ_WALKF;
	else if (playerinfo->seqcmd[ACMDL_RUN_F])
		return ASEQ_RUNF;
	else if (playerinfo->seqcmd[ACMDL_CREEP_F])
		return ASEQ_CREEPF;
	/*else if (((playerinfo->seqcmd[ACMDL_WALK_B]) && (playerinfo->buttons & BUTTON_RUN)) &&
			((!playerinfo->seqcmd[ACMDL_STRAFE_L]) && (!playerinfo->seqcmd[ACMDL_STRAFE_R])) )
	{
		if (!(playerinfo->seqcmd[ACMDU_ATTACK]) && playerinfo->upperidle)
		{
			return ASEQ_JUMPSPRINGBGO;
		}
	}*/
	else if (playerinfo->seqcmd[ACMDL_BACK])
		return ASEQ_WALKB;
	else if (playerinfo->seqcmd[ACMDL_STRAFE_L])
		return ASEQ_STRAFEL;
	else if (playerinfo->seqcmd[ACMDL_STRAFE_R])
		return ASEQ_STRAFER;
	else if (playerinfo->seqcmd[ACMDL_ROTATE_L])
	{
		if (!(playerinfo->lowerseq >= ASEQ_PIVOTL_GO && playerinfo->lowerseq <= ASEQ_PIVOTL_END))
			return ASEQ_PIVOTL_GO;
	}
	else if (playerinfo->seqcmd[ACMDL_ROTATE_R])
	{
		if (!(playerinfo->lowerseq >= ASEQ_PIVOTR_GO && playerinfo->lowerseq <= ASEQ_PIVOTR_END))
			return ASEQ_PIVOTR_GO;
	}
	else
	{
		checksloped = CheckSlopedStand(playerinfo);
		if(checksloped)
			return checksloped;

		playerinfo->loweridle = true;
		if (playerinfo->lowerseq >= ASEQ_LSTAIR4 && playerinfo->lowerseq <= ASEQ_LSTAIR16)
			return ASEQ_STAND;	//if was stainding on stairs, go to stand
		return ASEQ_NONE;
	}
	return ASEQ_NONE;
}

/*-----------------------------------------------
	BranchLwrWalking
-----------------------------------------------*/

int BranchLwrWalking(playerinfo_t *playerinfo)
{
	int	curseq = playerinfo->lowerseq;

	assert(playerinfo);
	
	//Check for the player falling [LOW PROBABILITY, IMMEDIATE CONCERN]
	if (playerinfo->groundentity==NULL && playerinfo->waterlevel < 2 && !(playerinfo->watertype & (CONTENTS_SLIME|CONTENTS_LAVA)))
	{
		if (CheckFall(playerinfo))
			return ASEQ_FALLWALK_GO;
	}

	//Check for an autovault (only occurs if upper half of body is idle!) [LOW PROBABILITY, IMMEDIATE CONCERN]
	if ( (playerinfo->flags & PLAYER_FLAG_COLLISION) && (playerinfo->upperidle) && (playerinfo->seqcmd[ACMDL_FWD]) )
	{
		PlayerActionCheckVault(playerinfo, 0);
		
		if (curseq == ASEQ_VAULT_LOW)
			return ASEQ_VAULT_LOW;

		if (curseq == ASEQ_PULLUP_HALFWALL)
			return  ASEQ_PULLUP_HALFWALL;

		if ( (playerinfo->seqcmd[ACMDL_ACTION]) && PlayerActionCheckJumpGrab(playerinfo, 0) )
			return ASEQ_JUMPSTD_GO;
	}

	//Check for a jump [LOW PROBABILITY]
	if (playerinfo->seqcmd[ACMDL_JUMP])
	{
		if (!(playerinfo->watertype & (CONTENTS_SLIME|CONTENTS_LAVA)))
		{
			if (playerinfo->buttons & BUTTON_RUN)
			{
				if (playerinfo->seqcmd[ACMDL_FWD])
				{
					if (	(playerinfo->pers.weaponready == WEAPON_READY_SWORDSTAFF) &&
							(!(playerinfo->flags & PLAYER_FLAG_NO_RARM)) )
						return ASEQ_POLEVAULT1_W;
					else
						return ASEQ_JUMPFWD_WGO;
				}

				if (playerinfo->seqcmd[ACMDL_BACK])
					return ASEQ_JUMPBACK_RGO;
				
				if (playerinfo->seqcmd[ACMDL_STRAFE_L])
					return ASEQ_JUMPLEFT_RGO;
				
				if (playerinfo->seqcmd[ACMDL_STRAFE_R])
					return ASEQ_JUMPRIGHT_RGO;
			}
			else if (playerinfo->buttons & BUTTON_CREEP)
			{
				if (playerinfo->seqcmd[ACMDL_FWD])
					return ASEQ_JUMPFWD_SGO;

				if (playerinfo->seqcmd[ACMDL_BACK])
					return ASEQ_JUMPBACK_SGO;
				
				if (playerinfo->seqcmd[ACMDL_STRAFE_L])
					return ASEQ_JUMPLEFT_SGO;
				
				if (playerinfo->seqcmd[ACMDL_STRAFE_R])
					return ASEQ_JUMPRIGHT_SGO;
			}
			else
			{
				if (playerinfo->seqcmd[ACMDL_FWD])
					return ASEQ_JUMPFWD_WGO;

				if (playerinfo->seqcmd[ACMDL_BACK])
					return ASEQ_JUMPBACK_WGO;
				
				if (playerinfo->seqcmd[ACMDL_STRAFE_L])
					return ASEQ_JUMPLEFT_WGO;
				
				if (playerinfo->seqcmd[ACMDL_STRAFE_R])
					return ASEQ_JUMPRIGHT_WGO;
			}
		}
	}

	//Check for creep strafing forward [HIGH PROBABILITY]
	if ( playerinfo->seqcmd[ACMDL_CREEP_F] && playerinfo->seqcmd[ACMDL_STRAFE_L])
	{
		return ASEQ_CSTRAFE_LEFT;
	}
    
	if ( playerinfo->seqcmd[ACMDL_CREEP_F] && playerinfo->seqcmd[ACMDL_STRAFE_R])
	{
		return ASEQ_CSTRAFE_RIGHT;
	}

	//Check for walk strafing forward [HIGH PROBABILITY]
	if ( playerinfo->seqcmd[ACMDL_WALK_F] && playerinfo->seqcmd[ACMDL_STRAFE_L])
	{
		return ASEQ_WSTRAFE_LEFT;
	}
    
	if ( playerinfo->seqcmd[ACMDL_WALK_F] && playerinfo->seqcmd[ACMDL_STRAFE_R])
	{
		return ASEQ_WSTRAFE_RIGHT;
	}
    
	//Check for run strafing forward [HIGH PROBABILITY]
	if ( playerinfo->seqcmd[ACMDL_RUN_F] && playerinfo->seqcmd[ACMDL_STRAFE_L])
	{
		return ASEQ_RSTRAFE_LEFT;
	}
    
	if ( playerinfo->seqcmd[ACMDL_RUN_F] && playerinfo->seqcmd[ACMDL_STRAFE_R])
	{
		return ASEQ_RSTRAFE_RIGHT;
	}
    
	//Check for creep strafing backward [HIGH PROBABILITY]
	if ( playerinfo->seqcmd[ACMDL_CREEP_B] && playerinfo->seqcmd[ACMDL_STRAFE_L])
	{
		return ASEQ_CSTRAFEB_LEFT;
	}
    
	if ( playerinfo->seqcmd[ACMDL_CREEP_B] && playerinfo->seqcmd[ACMDL_STRAFE_R])
	{
		return ASEQ_CSTRAFEB_RIGHT;
	}

	//Check for walk strafing backward [HIGH PROBABILITY]
	if ( playerinfo->seqcmd[ACMDL_WALK_B] && playerinfo->seqcmd[ACMDL_STRAFE_L])
	{
		return ASEQ_WSTRAFEB_LEFT;
	}
    
	if ( playerinfo->seqcmd[ACMDL_WALK_B] && playerinfo->seqcmd[ACMDL_STRAFE_R])
	{
		return ASEQ_WSTRAFEB_RIGHT;
	}

	//Check for a crouch [LOW PROBABILITY]
	if (playerinfo->seqcmd[ACMDL_CROUCH])
	{
		if (playerinfo->buttons & BUTTON_CREEP)
		{
			if (playerinfo->seqcmd[ACMDL_FWD])
				return ASEQ_CROUCH_WALK_F;

			if (playerinfo->seqcmd[ACMDL_BACK])
				return ASEQ_CROUCH_WALK_B;
			
			if (playerinfo->seqcmd[ACMDL_STRAFE_L])
				return ASEQ_CROUCH_WALK_L;
			
			if (playerinfo->seqcmd[ACMDL_STRAFE_R])
				return ASEQ_CROUCH_WALK_R;

			return ASEQ_CROUCH_GO;
		}
		else
		{
			if (playerinfo->seqcmd[ACMDL_FWD])
				return ASEQ_ROLLDIVEF_W;

			if (playerinfo->seqcmd[ACMDL_BACK])
				return ASEQ_ROLL_B;
			
			if (playerinfo->seqcmd[ACMDL_STRAFE_L])
				return ASEQ_ROLL_L;
			
			if (playerinfo->seqcmd[ACMDL_STRAFE_R])
				return ASEQ_ROLL_R;

			return ASEQ_CROUCH_GO;
		}
	}

	//Look for the action key being pressed	[LOW PROBABILITY]
	if (playerinfo->seqcmd[ACMDL_ACTION])
	{
		if ( (playerinfo->targetEnt) && (PlayerActionCheckRopeGrab(playerinfo,0)) ) //Climb a rope?
		{
			playerinfo->flags |= PLAYER_FLAG_ONROPE;
			
			if(playerinfo->isclient)
				playerinfo->CL_Sound(SND_PRED_ID33,playerinfo->origin, CHAN_VOICE, "player/ropegrab.wav", 0.75, ATTN_NORM, 0);
			else
				playerinfo->G_Sound(SND_PRED_ID33, playerinfo->leveltime, playerinfo->self, CHAN_VOICE, playerinfo->G_SoundIndex("player/ropegrab.wav"), 0.75, ATTN_NORM, 0);

			return ASEQ_CLIMB_ON;
		}
	
		//Try and use a puzzle piece
		PlayerActionUsePuzzle(playerinfo);

		//Check for an auto-jump vault
		if ( (playerinfo->flags & PLAYER_FLAG_COLLISION) && (playerinfo->upperidle) && (PlayerActionCheckJumpGrab(playerinfo, 0)) )
				return ASEQ_JUMPSTD_GO;
	}

	//Check for a quickturn [LOW PROBABILITY]
	if (playerinfo->seqcmd[ACMDL_QUICKTURN])
		return ASEQ_TURN180;

	//All actions have been accounted for, now just see if we are ending certain sequences

	//Check for a sudden change of speed [HIGH PROBABILITY]
	if (playerinfo->seqcmd[ACMDL_CREEP_F])
	{
		return ASEQ_CREEPF;
	}
	
	if (playerinfo->seqcmd[ACMDL_CREEP_B])
	{
		return ASEQ_CREEPB;
	}
	
	if (playerinfo->seqcmd[ACMDL_RUN_F])
	{
		return ASEQ_RUNF;
	}

	//If we're pressing forward, and nothing else is happening, then we're just walking forward
	if ( (playerinfo->seqcmd[ACMDL_WALK_F]) && (!playerinfo->seqcmd[ACMDL_STRAFE_L]) && (!playerinfo->seqcmd[ACMDL_STRAFE_R]) )
	{
		if (curseq != ASEQ_WALKF)
		{
			return ASEQ_WALKF;
		}
	}

	//If we're pressing backward, and nothing else is happening, then we're just walking backward
	if ( (playerinfo->seqcmd[ACMDL_WALK_B]) && (!playerinfo->seqcmd[ACMDL_STRAFE_L]) && (!playerinfo->seqcmd[ACMDL_STRAFE_R]) )
	{
		if (curseq != ASEQ_WALKB)
		{
			return ASEQ_WALKB;
		}
	}

	return ASEQ_NONE;
}
	
/*-----------------------------------------------
	BranchLwrRunning
-----------------------------------------------*/

int BranchLwrRunning(playerinfo_t *playerinfo)
{
	return BranchLwrWalking(playerinfo);
/*	assert(playerinfo);

	//Check for the player falling [LOW PROBABILITY, BUT IMMEDIATE CONCERN]
	if (playerinfo->groundentity==NULL && playerinfo->waterlevel < 2 && !(playerinfo->watertype & (CONTENTS_SLIME|CONTENTS_LAVA)))
	{
		if (CheckFall(playerinfo))
			return ASEQ_FALLWALK_GO;
	}

	//Check for an autovault (only occurs if upper half of body is idle!) [LOW PROBABILITY, IMMEDIATE CONCERN]

	if ((playerinfo->seqcmd[ACMDL_FWD]) && playerinfo->upperidle)
	{
		PlayerActionCheckVault(playerinfo, 0);
	
		if (playerinfo->lowerseq == ASEQ_VAULT_LOW && playerinfo->upperidle)
			return ASEQ_VAULT_LOW;
		
		if (playerinfo->lowerseq == ASEQ_PULLUP_HALFWALL)
			return ASEQ_PULLUP_HALFWALL;
	}

	//Check for an upper sequence interruption due to a staff attack
	if ((	playerinfo->seqcmd[ACMDU_ATTACK] && 
			playerinfo->seqcmd[ACMDL_RUN_F]) &&  
			(playerinfo->weaponready == WEAPON_READY_SWORDSTAFF) && 
			!(playerinfo->edictflags & FL_CHICKEN))	
	{
		return ASEQ_WSWORD_SPIN;
	}

	//Check for run strafing forward [HIGH PROBABILITY]
	if ( playerinfo->seqcmd[ACMDL_RUN_F] && playerinfo->seqcmd[ACMDL_STRAFE_L])
	{
		return ASEQ_RSTRAFE_LEFT;
	}
    
	if ( playerinfo->seqcmd[ACMDL_RUN_F] && playerinfo->seqcmd[ACMDL_STRAFE_R])
	{
		return ASEQ_RSTRAFE_RIGHT;
	}
    
	//Check for walk strafing forward [HIGH PROBABILITY]
	if ( playerinfo->seqcmd[ACMDL_WALK_F] && playerinfo->seqcmd[ACMDL_STRAFE_L])
	{
		return ASEQ_WSTRAFE_LEFT;
	}
    
	if ( playerinfo->seqcmd[ACMDL_WALK_F] && playerinfo->seqcmd[ACMDL_STRAFE_R])
	{
		return ASEQ_WSTRAFE_RIGHT;
	}

	//Check for a jump [LOW PROBABILITY]
	if (playerinfo->seqcmd[ACMDL_JUMP])
	{
		if (!(playerinfo->watertype & (CONTENTS_SLIME|CONTENTS_LAVA)))
		{
			if (playerinfo->weaponready == WEAPON_READY_SWORDSTAFF)
				return ASEQ_POLEVAULT1_R;
			else
				return ASEQ_JUMPFWD_RGO;
		}
	}
	
	//Check for a sudden change of speed [HIGH PROBABILITY]
	if (playerinfo->seqcmd[ACMDL_WALK_F])
	{
		return ASEQ_WALKF;
	}
	
	if (playerinfo->seqcmd[ACMDL_CREEP_F])
	{
		return ASEQ_CREEPF;
	}

	//Check for a crouch [LOW PROBABILITY]
	if (playerinfo->seqcmd[ACMDL_CROUCH])
	{
		if (playerinfo->seqcmd[ACMDL_BACK])
			return ASEQ_ROLL_B;
		else if (playerinfo->seqcmd[ACMDL_STRAFE_L])
			return ASEQ_ROLL_L;
		else if (playerinfo->seqcmd[ACMDL_STRAFE_R])
			return ASEQ_ROLL_R;

		playerinfo->maxs[2] = 4;

		return ASEQ_ROLLDIVEF_R;
	}
  
	//Look for the action key being pressed	[LOW PROBABILITY]
	if (playerinfo->seqcmd[ACMDL_ACTION])
	{
		if ( (playerinfo->targetEnt) && (PlayerActionCheckRopeGrab(playerinfo,0)) ) //Climb a rope?
		{
			playerinfo->flags |= PLAYER_FLAG_ONROPE;
			
			if(playerinfo->isclient)
				playerinfo->CL_Sound( playerinfo->origin, CHAN_VOICE, "player/ropegrab.wav", 0.75, ATTN_NORM, 0);
			else
				playerinfo->G_Sound( playerinfo->self, CHAN_VOICE, playerinfo->G_SoundIndex("player/ropegrab.wav"), 0.75, ATTN_NORM, 0);

			//On the rope
			return ASEQ_CLIMB_ON;
		}

		if ( (playerinfo->flags & PLAYER_FLAG_COLLISION) && (playerinfo->upperidle) && (PlayerActionCheckJumpGrab(playerinfo, 0)) )
			return ASEQ_JUMPSTD_GO;
	}

	//Check for a quickturn [LOW PROBABILITY]
	if (playerinfo->seqcmd[ACMDL_QUICKTURN])
		return ASEQ_TURN180;

	//All actions have been accounted for, now just see if we are ending certain sequences

	//If we're pressing forward, and nothing else is happening, then we're just walking forward
	if ( (playerinfo->seqcmd[ACMDL_RUN_F]) && (!playerinfo->seqcmd[ACMDL_STRAFE_L]) && (!playerinfo->seqcmd[ACMDL_STRAFE_R]) )
	{
		if (playerinfo->lowerseq != ASEQ_RUNF)
		{
			return ASEQ_RUNF;
		}
	}

	return ASEQ_NONE;
*/
}

/*-----------------------------------------------
	BranchLwrRunningStrafe
-----------------------------------------------*/

int BranchLwrRunningStrafe(playerinfo_t *playerinfo)
{
	assert(playerinfo);

	//Check for the player falling [LOW PROBABILITY, BUT IMMEDIATE CONCERN]
	if (playerinfo->groundentity==NULL && playerinfo->waterlevel < 2 && !(playerinfo->watertype & (CONTENTS_SLIME|CONTENTS_LAVA)))
	{
		if (CheckFall(playerinfo))
			return ASEQ_FALLWALK_GO;
	}

	//Check for crouching
	if (playerinfo->seqcmd[ACMDL_CROUCH])
	{
		if (playerinfo->seqcmd[ACMDL_BACK])
			return ASEQ_ROLL_B;
		else if (playerinfo->seqcmd[ACMDL_STRAFE_L])
			return ASEQ_ROLL_L;
		else if (playerinfo->seqcmd[ACMDL_STRAFE_R])
			return ASEQ_ROLL_R;

		playerinfo->maxs[2] = 4;

		return ASEQ_ROLLDIVEF_R;
	}
	
	if ( playerinfo->seqcmd[ACMDL_RUN_F] && playerinfo->seqcmd[ACMDL_STRAFE_L])
	{
		return ASEQ_RSTRAFE_LEFT;
	}
    
	if ( playerinfo->seqcmd[ACMDL_RUN_F] && playerinfo->seqcmd[ACMDL_STRAFE_R])
	{
		return ASEQ_RSTRAFE_RIGHT;
	}
    
	//Check for a change in speeds
	if (playerinfo->seqcmd[ACMDL_WALK_F])
	{
		return ASEQ_WALKF;
	}

	if ( playerinfo->seqcmd[ACMDL_STRAFE_L] )
	{
		if (playerinfo->seqcmd[ACMDL_BACK])
			return ASEQ_WSTRAFEB_LEFT;

		if (playerinfo->buttons & BUTTON_RUN)
		{
			if ( playerinfo->lowerseq == ASEQ_DASH_LEFT || playerinfo->lowerseq == ASEQ_DASH_LEFT_GO )
				return ASEQ_NONE;
			
			return ASEQ_DASH_LEFT_GO;
		}
		else
		{
			return ASEQ_STRAFEL;
		}
	}

	if ( playerinfo->seqcmd[ACMDL_STRAFE_R] )
	{
		if (playerinfo->seqcmd[ACMDL_BACK])
			return ASEQ_WSTRAFEB_RIGHT;

		if (playerinfo->buttons & BUTTON_RUN)
		{
			if ( playerinfo->lowerseq == ASEQ_DASH_RIGHT || playerinfo->lowerseq == ASEQ_DASH_RIGHT_GO )
				return ASEQ_NONE;
			
			return ASEQ_DASH_RIGHT_GO;
		}
		else
		{
			return ASEQ_STRAFER;
		}
	}

	if (playerinfo->seqcmd[ACMDL_QUICKTURN])
		return ASEQ_TURN180;
	
	if ( !(playerinfo->seqcmd[ACMDL_STRAFE_L]) && !(playerinfo->seqcmd[ACMDL_STRAFE_R]) && !(playerinfo->seqcmd[ACMDL_FWD]) ) 
	{//We've let go of the important buttons
		return ASEQ_STAND;
	}
	
	if ( !(playerinfo->seqcmd[ACMDL_STRAFE_L]) && !(playerinfo->seqcmd[ACMDL_STRAFE_R]) && (playerinfo->seqcmd[ACMDL_FWD]) ) 
	{//We're just trying to go forward
		return ASEQ_RUNF_GO;
	}

	//Look for the action key being pressed	[LOW PROBABILITY]
	if (playerinfo->seqcmd[ACMDL_ACTION])
	{
		if ( (playerinfo->targetEnt) && (PlayerActionCheckRopeGrab(playerinfo,0)) ) //Climb a rope?
		{
			playerinfo->flags |= PLAYER_FLAG_ONROPE;
			
			if(playerinfo->isclient)
				playerinfo->CL_Sound(SND_PRED_ID34, playerinfo->origin, CHAN_VOICE, "player/ropegrab.wav", 0.75, ATTN_NORM, 0);
			else
				playerinfo->G_Sound(SND_PRED_ID34, playerinfo->leveltime, playerinfo->self, CHAN_VOICE, playerinfo->G_SoundIndex("player/ropegrab.wav"), 0.75, ATTN_NORM, 0);

			//On a rope
			return ASEQ_CLIMB_ON;
		}

		//Check for a jump grab
		if ( (playerinfo->flags & PLAYER_FLAG_COLLISION) && (playerinfo->upperidle) && (PlayerActionCheckJumpGrab(playerinfo, 0)) )
			return ASEQ_JUMPSTD_GO;
	}

	return ASEQ_NONE;
}

/*-----------------------------------------------
	BranchLwrStrafe
-----------------------------------------------*/

int BranchLwrStrafe(playerinfo_t *playerinfo)
{
	int curseq;

	assert(playerinfo);

	if (playerinfo->groundentity==NULL && playerinfo->waterlevel < 2 && !(playerinfo->watertype & (CONTENTS_SLIME|CONTENTS_LAVA)))
	{
		if (CheckFall(playerinfo))
			return ASEQ_FALLWALK_GO;
	}

	if (playerinfo->seqcmd[ACMDL_ACTION])
	{
		if ( (playerinfo->targetEnt) && (PlayerActionCheckRopeGrab(playerinfo,0)) ) //Climb a rope?
		{
			playerinfo->flags |= PLAYER_FLAG_ONROPE;
			
			if(playerinfo->isclient)
				playerinfo->CL_Sound(SND_PRED_ID35,playerinfo->origin, CHAN_VOICE, "player/ropegrab.wav", 0.75, ATTN_NORM, 0);
			else
 				playerinfo->G_Sound(SND_PRED_ID35, playerinfo->leveltime, playerinfo->self, CHAN_VOICE, playerinfo->G_SoundIndex("player/ropegrab.wav"), 0.75, ATTN_NORM, 0);

			return ASEQ_CLIMB_ON;
		}

		if ( (playerinfo->flags & PLAYER_FLAG_COLLISION) && (playerinfo->upperidle) && (PlayerActionCheckJumpGrab(playerinfo, 0)) )
			return ASEQ_JUMPSTD_GO;
	}

	//Check for crouching
	if (playerinfo->seqcmd[ACMDL_CROUCH])
	{
		if (playerinfo->seqcmd[ACMDL_BACK])
			return ASEQ_ROLL_B;
		else if (playerinfo->seqcmd[ACMDL_STRAFE_L])
			return ASEQ_ROLL_L;
		else if (playerinfo->seqcmd[ACMDL_STRAFE_R])
			return ASEQ_ROLL_R;

		playerinfo->maxs[2] = 4;

		return ASEQ_ROLLDIVEF_R;
	}
	else if (playerinfo->seqcmd[ACMDL_WALK_F])
	{
		curseq = playerinfo->lowerseq;
		
		if (curseq == ASEQ_RUNF_GO || curseq == ASEQ_RUNF)
			return ASEQ_WALKF;
	}
    else if ( playerinfo->seqcmd[ACMDL_RUN_F] && playerinfo->seqcmd[ACMDL_STRAFE_L])
	{
		return ASEQ_RSTRAFE_LEFT;
	}
    else if ( playerinfo->seqcmd[ACMDL_RUN_F] && playerinfo->seqcmd[ACMDL_STRAFE_R])
	{
		return ASEQ_RSTRAFE_RIGHT;
	}
    else if ( playerinfo->seqcmd[ACMDL_STRAFE_L] )
	{
		if (playerinfo->buttons & BUTTON_RUN)
		{
			if ( playerinfo->lowerseq == ASEQ_DASH_LEFT || playerinfo->lowerseq == ASEQ_DASH_LEFT_GO )
				return ASEQ_NONE;
			else if ( playerinfo->lowerseq == ASEQ_DASH_RIGHT || playerinfo->lowerseq == ASEQ_DASH_RIGHT_GO )
				return ASEQ_DASH_LEFT_GO;
		}
		else
		{
			if ( playerinfo->lowerseq == ASEQ_DASH_LEFT || playerinfo->lowerseq == ASEQ_DASH_LEFT_GO )
				return ASEQ_STRAFEL;
			else if ( playerinfo->lowerseq == ASEQ_DASH_RIGHT || playerinfo->lowerseq == ASEQ_DASH_RIGHT_GO )
				return ASEQ_STRAFEL;
		}
	}
	
	if (playerinfo->seqcmd[ACMDL_QUICKTURN])
		return ASEQ_TURN180;
	
	if ( !(playerinfo->seqcmd[ACMDL_STRAFE_L]) && !(playerinfo->seqcmd[ACMDL_STRAFE_R]) && !(playerinfo->seqcmd[ACMDL_FWD]) && !(playerinfo->seqcmd[ACMDL_BACK])) 
	{//We've let go of the important buttons
		return ASEQ_STAND;
	}
	
	if ( !(playerinfo->seqcmd[ACMDL_STRAFE_L]) && !(playerinfo->seqcmd[ACMDL_STRAFE_R]) ) 
	{//We're just trying to go forward

		//FORWARD

		if (playerinfo->seqcmd[ACMDL_CREEP_F])
			return ASEQ_CREEPF;

		if (playerinfo->seqcmd[ACMDL_WALK_F])
			return ASEQ_WALKF_GO;
		
		if (playerinfo->seqcmd[ACMDL_RUN_F])
			return ASEQ_RUNF_GO;
		
		//BACKWARD

		if (playerinfo->seqcmd[ACMDL_CREEP_B])
			return ASEQ_CREEPB;

		if (playerinfo->seqcmd[ACMDL_BACK])
			return ASEQ_WALKB;
	}

	return ASEQ_NONE;
}

/*-----------------------------------------------
	BranchLwrShortstep
-----------------------------------------------*/

int BranchLwrShortstep(playerinfo_t *playerinfo)
{
	assert(playerinfo);

	if (playerinfo->groundentity==NULL && playerinfo->waterlevel < 2 && !(playerinfo->watertype & (CONTENTS_SLIME|CONTENTS_LAVA)))
	{
		if (CheckFall(playerinfo))
			return ASEQ_FALLWALK_GO;
	}

	if (playerinfo->seqcmd[ACMDL_ACTION])
	{
		if ( (playerinfo->targetEnt) && (PlayerActionCheckRopeGrab(playerinfo,0)) ) //Climb a rope?
		{
			playerinfo->flags |= PLAYER_FLAG_ONROPE;
			
			if(playerinfo->isclient)
				playerinfo->CL_Sound(SND_PRED_ID36, playerinfo->origin, CHAN_VOICE, "player/ropegrab.wav", 0.75, ATTN_NORM, 0);
			else
				playerinfo->G_Sound(SND_PRED_ID36, playerinfo->leveltime, playerinfo->self, CHAN_VOICE, playerinfo->G_SoundIndex("player/ropegrab.wav"), 0.75, ATTN_NORM, 0);

			return ASEQ_CLIMB_ON;
		}

		if ( (playerinfo->flags & PLAYER_FLAG_COLLISION) && (playerinfo->upperidle) && (PlayerActionCheckJumpGrab(playerinfo, 0)) )
			return ASEQ_JUMPSTD_GO;
	}

	if (playerinfo->seqcmd[ACMDL_JUMP])
	{
		if (!(playerinfo->watertype & (CONTENTS_SLIME|CONTENTS_LAVA)))
		{
			if (playerinfo->seqcmd[ACMDL_FWD])
				return ASEQ_JUMPFWD_SGO;
			
			if (playerinfo->seqcmd[ACMDL_BACK])
				return ASEQ_JUMPBACK_SGO;
			
			if (playerinfo->seqcmd[ACMDL_STRAFE_L])
				return ASEQ_JUMPLEFT_SGO;
			
			if (playerinfo->seqcmd[ACMDL_STRAFE_R])
				return ASEQ_JUMPRIGHT_SGO;

			return ASEQ_JUMPSTD_GO;
		}
	}
	
	//Check for a crouch
	if (playerinfo->seqcmd[ACMDL_CROUCH])
	{
		if (playerinfo->buttons & BUTTON_CREEP)
		{
			if (playerinfo->seqcmd[ACMDL_FWD])
				return ASEQ_CROUCH_WALK_F;

			if (playerinfo->seqcmd[ACMDL_BACK])
				return ASEQ_CROUCH_WALK_B;
			
			if (playerinfo->seqcmd[ACMDL_STRAFE_L])
				return ASEQ_CROUCH_WALK_L;
			
			if (playerinfo->seqcmd[ACMDL_STRAFE_R])
				return ASEQ_CROUCH_WALK_R;

			return ASEQ_CROUCH_GO;
		}
		else
		{
			if (playerinfo->seqcmd[ACMDL_FWD])
				return ASEQ_ROLLDIVEF_W;

			if (playerinfo->seqcmd[ACMDL_BACK])
				return ASEQ_ROLL_B;
			
			if (playerinfo->seqcmd[ACMDL_STRAFE_L])
				return ASEQ_ROLL_L;
			
			if (playerinfo->seqcmd[ACMDL_STRAFE_R])
				return ASEQ_ROLL_R;

			return ASEQ_CROUCH_GO;
		}
	}
	
	//Check for walk strafing forward [HIGH PROBABILITY]
	if ( playerinfo->seqcmd[ACMDL_CREEP_F] && playerinfo->seqcmd[ACMDL_STRAFE_L])
	{
		return ASEQ_CSTRAFE_LEFT;
	}
    
	if ( playerinfo->seqcmd[ACMDL_CREEP_F] && playerinfo->seqcmd[ACMDL_STRAFE_R])
	{
		return ASEQ_CSTRAFE_RIGHT;
	}

	//Check for walk strafing backward [HIGH PROBABILITY]
	if ( playerinfo->seqcmd[ACMDL_CREEP_B] && playerinfo->seqcmd[ACMDL_STRAFE_L])
	{
		return ASEQ_CSTRAFEB_LEFT;
	}
    
	if ( playerinfo->seqcmd[ACMDL_CREEP_B] && playerinfo->seqcmd[ACMDL_STRAFE_R])
	{
		return ASEQ_CSTRAFEB_RIGHT;
	}

	if (playerinfo->seqcmd[ACMDL_WALK_F])
		return ASEQ_WALKF_GO;
	
	if (playerinfo->seqcmd[ACMDL_RUN_F])
		return ASEQ_RUNF_GO;
	
	if (playerinfo->seqcmd[ACMDL_CREEP_B])
		return ASEQ_CREEPB;

	if (playerinfo->seqcmd[ACMDL_BACK])
		return ASEQ_WALKB;

	return ASEQ_NONE;
}

/*-----------------------------------------------
	BranchLwrBackspring
-----------------------------------------------*/

int BranchLwrBackspring(playerinfo_t *playerinfo)
{
	assert(playerinfo);

	if (playerinfo->groundentity==NULL && playerinfo->waterlevel < 2 && !(playerinfo->watertype & (CONTENTS_SLIME|CONTENTS_LAVA)))
	{
		if (CheckFall(playerinfo))
			return ASEQ_FALLWALK_GO;
	}

	if (playerinfo->seqcmd[ACMDL_JUMP])
		return ASEQ_JUMPFLIPB;
	else if (playerinfo->seqcmd[ACMDL_RUN_B])
	{
		if (playerinfo->seqcmd[ACMDL_STRAFE_L])
		{
			return ASEQ_WSTRAFEB_LEFT;
		}
		else if (playerinfo->seqcmd[ACMDL_STRAFE_R])
		{
			return ASEQ_WSTRAFEB_RIGHT;
		}
		else if (!(playerinfo->seqcmd[ACMDU_ATTACK]))
		{
			PlayerAnimSetUpperSeq(playerinfo, ASEQ_NONE);
			return ASEQ_JUMPSPRINGB;
		}
		else
			return ASEQ_WALKB;
	}
	else if (playerinfo->seqcmd[ACMDL_WALK_B])
		return ASEQ_WALKB;
	else if (playerinfo->seqcmd[ACMDL_CREEP_B])
		return ASEQ_CREEPB;
	else if (playerinfo->seqcmd[ACMDL_CROUCH])
		return ASEQ_CROUCH_GO;

	return ASEQ_NONE;
}

/*-----------------------------------------------
	BranchLwrJumping
-----------------------------------------------*/

int BranchLwrJumping(playerinfo_t *playerinfo)
{
	assert(playerinfo);

  	if (playerinfo->seqcmd[ACMDL_ACTION])
	{
		if ( (playerinfo->targetEnt) && (PlayerActionCheckRopeGrab(playerinfo,0)) ) //Climb a rope?
		{
			playerinfo->flags |= PLAYER_FLAG_ONROPE;
			return ASEQ_CLIMB_ON;
		}
	}
	
	if (playerinfo->seqcmd[ACMDL_FWD])
		return ASEQ_JUMPFWD_SGO;
	else if (playerinfo->seqcmd[ACMDL_BACK])
		return ASEQ_JUMPBACK_SGO;
	else if (playerinfo->seqcmd[ACMDL_STRAFE_L])
		return ASEQ_JUMPLEFT_SGO;
	else if (playerinfo->seqcmd[ACMDL_STRAFE_R])
		return ASEQ_JUMPRIGHT_SGO;

	return ASEQ_NONE;
}

/*-----------------------------------------------
	BranchLwrKnockDown
-----------------------------------------------*/

int BranchLwrKnockDown(playerinfo_t *playerinfo)
{
	if (playerinfo->seqcmd[ACMDL_BACK])
		return ASEQ_KNOCKDOWN_EVADE;
 
	return ASEQ_KNOCKDOWN_GETUP;
}

/*-----------------------------------------------
	BranchLwrCrouching
-----------------------------------------------*/

int BranchLwrCrouching(playerinfo_t *playerinfo)
{
	assert(playerinfo);

	if (playerinfo->maxs[2] != 4)
	{
		playerinfo->maxs[2] = 4;
	}

	if (playerinfo->groundentity==NULL && playerinfo->waterlevel < 2)
	{
		if (CheckFall(playerinfo))
			return ASEQ_CROUCH_END;
	}

	// See if we're ending the crouch.
	if (!playerinfo->seqcmd[ACMDL_CROUCH] && !playerinfo->seqcmd[ACMDL_ROTATE_L] &&  !playerinfo->seqcmd[ACMDL_ROTATE_R])
	{
		if (!CheckUncrouch(playerinfo))
		{
			if (playerinfo->buttons & BUTTON_CREEP)
			{
				if (playerinfo->seqcmd[ACMDL_FWD])
					return ASEQ_CROUCH_WALK_F;

				if (playerinfo->seqcmd[ACMDL_BACK])
					return ASEQ_CROUCH_WALK_B;

				if (playerinfo->seqcmd[ACMDL_STRAFE_L])
					return ASEQ_CROUCH_WALK_L;

				if (playerinfo->seqcmd[ACMDL_STRAFE_R])
					return ASEQ_CROUCH_WALK_R;
			}

			if (playerinfo->seqcmd[ACMDL_FWD])
				return ASEQ_ROLLDIVEF_W;
			
			if (playerinfo->seqcmd[ACMDL_BACK])
				return ASEQ_ROLL_B;
			
			if (playerinfo->seqcmd[ACMDL_ROTATE_L])
				return ASEQ_CROUCH_PIVOTL;
			
			if (playerinfo->seqcmd[ACMDL_ROTATE_R])
				return ASEQ_CROUCH_PIVOTR;
			
			if (playerinfo->seqcmd[ACMDL_STRAFE_R])
				return ASEQ_ROLL_R;
			
			if (playerinfo->seqcmd[ACMDL_STRAFE_L])
				return ASEQ_ROLL_L;
			
			return ASEQ_CROUCH;
		}

		playerinfo->maxs[2] = 25;

		return ASEQ_CROUCH_END;
	}

	if (playerinfo->seqcmd[ACMDL_JUMP])
		return ASEQ_JUMPSTD_GO;
	
	if (playerinfo->buttons & BUTTON_CREEP)
	{
		if (playerinfo->seqcmd[ACMDL_FWD])
			return ASEQ_CROUCH_WALK_F;

		if (playerinfo->seqcmd[ACMDL_BACK])
			return ASEQ_CROUCH_WALK_B;

		if (playerinfo->seqcmd[ACMDL_STRAFE_L])
			return ASEQ_CROUCH_WALK_L;

		if (playerinfo->seqcmd[ACMDL_STRAFE_R])
			return ASEQ_CROUCH_WALK_R;
	}

	if (playerinfo->seqcmd[ACMDL_FWD])
		return ASEQ_ROLLDIVEF_W;

	if (playerinfo->seqcmd[ACMDL_BACK])
		return ASEQ_ROLL_B;
	
	if (playerinfo->seqcmd[ACMDL_ROTATE_L])
		return ASEQ_CROUCH_PIVOTL;
	
	if (playerinfo->seqcmd[ACMDL_ROTATE_R])
		return ASEQ_CROUCH_PIVOTR;
	
	if (playerinfo->seqcmd[ACMDL_STRAFE_R])
		return ASEQ_ROLL_R;
	
	if (playerinfo->seqcmd[ACMDL_STRAFE_L])
		return ASEQ_ROLL_L;
	
	return ASEQ_CROUCH;
}

/*-----------------------------------------------
	BranchLwrSurfaceSwim
-----------------------------------------------*/

int BranchLwrSurfaceSwim(playerinfo_t *playerinfo)
{
	gitem_t		*Weapon;

	assert(playerinfo);

	if ((playerinfo->pers.weaponready != WEAPON_READY_HANDS) && ((Weapon=FindItem("fball"))!=NULL))
	{
		Weapon_EquipSpell(playerinfo, Weapon);
	}

	if (playerinfo->seqcmd[ACMDL_ACTION])
	{

		//Try and use a puzzle piece
		PlayerActionUsePuzzle(playerinfo);

//		if (PlayerActionCheckPuzzleGrab(playerinfo)) 	// Are you near a puzzle piece? Then try to take it
//		{
  //			return ASEQ_TAKEPUZZLEUNDERWATER;
	//	}

		PlayerActionCheckVault(playerinfo, 0);

		if (playerinfo->lowerseq == ASEQ_VAULT_LOW)
			return ASEQ_VAULT_LOW;
		
		if (playerinfo->lowerseq == ASEQ_PULLUP_HALFWALL)
			return ASEQ_PULLUP_HALFWALL;
	}
	//FIXME: Make this work!
	/*	
	else if (playerinfo->seqcmd[ACMDL_CROUCH])
	{
		PlayerAnimSetLowerSeq(playerinfo, ASEQ_DIVE);
	}
	*/
	else if (playerinfo->seqcmd[ACMDL_FWD])
	{
		PlayerActionCheckVault(playerinfo, 0);
		
		if (playerinfo->lowerseq == ASEQ_VAULT_LOW)
			return ASEQ_VAULT_LOW;
		
		if (playerinfo->lowerseq == ASEQ_PULLUP_HALFWALL)
			return ASEQ_PULLUP_HALFWALL;

		if (playerinfo->waterlevel > 2)
		{	
			return ASEQ_USWIMF_GO;
		}
		
		if (playerinfo->seqcmd[ACMDL_RUN_F])
		{
			if ( (playerinfo->lowerseq == ASEQ_SSWIM_FAST_GO) || (playerinfo->lowerseq == ASEQ_SSWIM_FAST) )
				return ASEQ_SSWIM_FAST;
			else
				return ASEQ_SSWIM_FAST_GO;
		}

		if ((playerinfo->lowerseq == ASEQ_SSWIMF_GO) || (playerinfo->lowerseq == ASEQ_SSWIMF)) 
			return ASEQ_SSWIMF;
		else
			return ASEQ_SSWIMF_GO;
	}
	else if (playerinfo->seqcmd[ACMDL_BACK])
	{
		if ((playerinfo->lowerseq == ASEQ_SSWIMB_GO) || (playerinfo->lowerseq == ASEQ_SSWIMB)) 
			return ASEQ_SSWIMB;
		else
			return ASEQ_SSWIMB_GO;
	}
	else if (playerinfo->seqcmd[ACMDL_STRAFE_L])
	{
		if ((playerinfo->lowerseq == ASEQ_SSWIML_GO) || (playerinfo->lowerseq == ASEQ_SSWIML)) 
			return ASEQ_SSWIML;
		else
			return ASEQ_SSWIML_GO;
	}
	else if (playerinfo->seqcmd[ACMDL_STRAFE_R])
	{
		if ((playerinfo->lowerseq == ASEQ_SSWIMR_GO) || (playerinfo->lowerseq == ASEQ_SSWIMR)) 
			return ASEQ_SSWIMR;
		else
			return ASEQ_SSWIMR_GO;
	}
	else
	{
		if (playerinfo->waterlevel > 2 && (playerinfo->lowerseq == ASEQ_SSWIM_IDLE))
			return ASEQ_USWIM_IDLE;

		return ASEQ_NONE;
	}

	return ASEQ_NONE;
}

/*-----------------------------------------------
	BranchLwrUnderwaterSwim
-----------------------------------------------*/

int BranchLwrUnderwaterSwim(playerinfo_t *playerinfo)
{
	gitem_t	*Weapon;

	assert(playerinfo);

	if ((playerinfo->pers.weaponready != WEAPON_READY_HANDS) && ((Weapon=FindItem("fball"))!=NULL))
	{
		Weapon_EquipSpell(playerinfo, Weapon);
	}

	if (playerinfo->seqcmd[ACMDL_ACTION])
	{

		//Try and use a puzzle piece
		PlayerActionUsePuzzle(playerinfo);

//		if (PlayerActionCheckPuzzleGrab(playerinfo)) 	// Are you near a puzzle piece? Then try to take it
//		{
  //			return ASEQ_TAKEPUZZLEUNDERWATER;
	//	}

	}
	else if (playerinfo->seqcmd[ACMDL_FWD])
	{
		if (playerinfo->waterlevel <= 2)
		{	
			return ASEQ_SSWIM_RESURFACE;
		}
		
		if ((playerinfo->lowerseq == ASEQ_USWIMF_GO) || (playerinfo->lowerseq == ASEQ_USWIMF)) 
			return ASEQ_USWIMF;
		else
			return ASEQ_USWIMF_GO;
	}
	else if (playerinfo->seqcmd[ACMDL_BACK])
	{
		if ((playerinfo->lowerseq == ASEQ_USWIMB_GO) || (playerinfo->lowerseq == ASEQ_USWIMB)) 
			return ASEQ_USWIMB;
		else
			return ASEQ_USWIMB_GO;
	}
	else if (playerinfo->seqcmd[ACMDL_STRAFE_L])
	{
		if ((playerinfo->lowerseq == ASEQ_USWIML_GO) || (playerinfo->lowerseq == ASEQ_USWIML)) 
			return ASEQ_USWIML;
		else
			return ASEQ_USWIML_GO;
	}
	else if (playerinfo->seqcmd[ACMDL_STRAFE_R])
	{
		if ((playerinfo->lowerseq == ASEQ_USWIMR_GO) || (playerinfo->lowerseq == ASEQ_USWIMR)) 
			return ASEQ_USWIMR;
		else
			return ASEQ_USWIMR_GO;
	}
	else
	{
		if (playerinfo->waterlevel <= 2 && (playerinfo->lowerseq == ASEQ_USWIM_IDLE))
			return ASEQ_SSWIM_IDLE;

		return ASEQ_NONE;
	}

	return ASEQ_NONE;
}

/*-----------------------------------------------
	BranchLwrHanging
-----------------------------------------------*/

int BranchLwrHanging(playerinfo_t *playerinfo)
{
	assert(playerinfo);

	if (playerinfo->seqcmd[ACMDL_FWD])
		return(ASEQ_PULLUP_WALL);
	else if (!playerinfo->seqcmd[ACMDL_ACTION] || playerinfo->seqcmd[ACMDL_JUMP])
		return(ASEQ_FALLARMSUP);
	else
		return(ASEQ_NONE);
}

/*-----------------------------------------------
	BranchLwrClimbing
-----------------------------------------------*/

int BranchLwrClimbing(playerinfo_t *playerinfo)
{
	if(!playerinfo->isclient)
		return(playerinfo->G_BranchLwrClimbing(playerinfo));
	else
		return(ASEQ_NONE);
}


/*-----------------------------------------------
	BranchUprReadyHands
-----------------------------------------------*/

int BranchUprReadyHands(playerinfo_t *playerinfo)
{
	assert(playerinfo);
	
	//See if we have the arm to do that magic
	if (!BranchCheckDismemberAction(playerinfo, playerinfo->pers.weapon->tag))
		return ASEQ_NONE;

	if (playerinfo->seqcmd[ACMDU_ATTACK] && !(playerinfo->edictflags & FL_CHICKEN))	// Not a chicken
	{ 
		playerinfo->idletime=playerinfo->leveltime;

		// Check Offensive mana.
		if (playerinfo->pers.weapon->tag == ITEM_WEAPON_FLYINGFIST || Weapon_CurrentShotsLeft(playerinfo))
		{	
			// Fireballs have free mana, but if powered up, use the alternate animation sequence.
			if (playerinfo->powerup_timer > playerinfo->leveltime)
				return playerinfo->pers.weapon->altanimseq;
			else
				return playerinfo->pers.weapon->playeranimseq;
		}
	}
	else
	{
		playerinfo->upperidle = true;
	}

	return(ASEQ_NONE);
}

/*-----------------------------------------------
	BranchUprReadySwordStaff
-----------------------------------------------*/

int BranchUprReadySwordStaff(playerinfo_t *playerinfo)
{
	//No arm, no shot
	if (playerinfo->flags & PLAYER_FLAG_NO_RARM)
		return ASEQ_NONE;

	if (playerinfo->edictflags & FL_CHICKEN)
	{
		playerinfo->upperidle=true;
		return(ASEQ_NONE);
	}

	if (playerinfo->seqcmd[ACMDU_ATTACK])	// Not a chicken
	{
		if (!strcmp(playerinfo->pers.weapon->classname, "Weapon_SwordStaff"))
		{
			playerinfo->idletime=playerinfo->leveltime;
			
			 // Make sure we're not about to do a spinning attack.
			if(playerinfo->seqcmd[ACMDL_RUN_F] && playerinfo->groundentity)
			{
				return(ASEQ_NONE);
			}
			else if (playerinfo->advancedstaff && playerinfo->seqcmd[ACMDL_ACTION] && playerinfo->seqcmd[ACMDL_BACK])
			{
				return(ASEQ_WSWORD_BACK);
			}
			else if (playerinfo->advancedstaff && playerinfo->upperseq == ASEQ_WSWORD_STABHOLD)
			{
				if (playerinfo->seqcmd[ACMDU_ATTACK])
					return ASEQ_WSWORD_STABHOLD;
				else
					return ASEQ_WSWORD_PULLOUT;
			}
			else if (playerinfo->advancedstaff && playerinfo->lowerseq == ASEQ_JUMPFWD && playerinfo->seqcmd[ACMDL_FWD])
			{
				return ASEQ_WSWORD_DOWNSTAB;
			}
			else if(!playerinfo->irand(playerinfo,0, 4))	
			{
				return(ASEQ_WSWORD_STEP);
			}
			else
			{
				return(ASEQ_WSWORD_STD1);	
			}
		}
	}
	else
	{
		if (!playerinfo->seqcmd[ACMDL_RUN_F])
			playerinfo->block_timer = playerinfo->leveltime + 0.1;		// Auto blocking if not running
		playerinfo->upperidle = true;
	}

	return(ASEQ_NONE);
}

/*-----------------------------------------------
	BranchUprReadyHellStaff
-----------------------------------------------*/

int BranchUprReadyHellStaff(playerinfo_t *playerinfo)
{
	//No arm, no shot
	if (playerinfo->flags & PLAYER_FLAG_NO_RARM)
		return ASEQ_NONE;

	if(playerinfo->seqcmd[ACMDU_ATTACK]  && !(playerinfo->edictflags & FL_CHICKEN) && Weapon_CurrentShotsLeft(playerinfo))	// Not a chicken
	{
		playerinfo->idletime=playerinfo->leveltime;
		
		if (!strcmp(playerinfo->pers.weapon->classname, "item_weapon_hellstaff"))
		{
			// If powered up, use the alternate animation sequence.
			if (playerinfo->powerup_timer > playerinfo->leveltime)
				return playerinfo->pers.weapon->altanimseq;
			else
				return playerinfo->pers.weapon->playeranimseq;	
		}
	}
	else
	{
		playerinfo->upperidle = true;
	}

	return(ASEQ_NONE);
}

/*-----------------------------------------------
	BranchUprReadyBow
-----------------------------------------------*/

int BranchUprReadyBow(playerinfo_t *playerinfo)
{
	int blah;

	//No arm, no shot
	if (playerinfo->flags & PLAYER_FLAG_NO_LARM || playerinfo->flags & PLAYER_FLAG_NO_RARM)
		return ASEQ_NONE;

	blah = Weapon_CurrentShotsLeft(playerinfo);

	if(playerinfo->seqcmd[ACMDU_ATTACK]  && !(playerinfo->edictflags & FL_CHICKEN) && Weapon_CurrentShotsLeft(playerinfo))	// Not a chicken
	{
		playerinfo->idletime=playerinfo->leveltime;

		// If powered up, use the alternate animation sequence.
		if (playerinfo->powerup_timer > playerinfo->leveltime)
			return(playerinfo->pers.weapon->altanimseq);
		else
			return(playerinfo->pers.weapon->playeranimseq);
	}
	else
	{
		playerinfo->upperidle=true;
	}

	return(ASEQ_NONE);
}

// if we are out of bow ammo, then switch us to the next weapon

/*-----------------------------------------------
	BranchCheckAmmo
-----------------------------------------------*/

int BranchCheckAmmo(playerinfo_t *playerinfo)
{
	if (Weapon_CurrentShotsLeft(playerinfo) || playerinfo->isclient)		// The client prediction shouldn't test the weapon.
		return(ASEQ_NONE);

	playerinfo->G_WeapNext(playerinfo->self);
	if (playerinfo->pers.weapon->tag == ITEM_WEAPON_REDRAINBOW)
	{
	   	PlayerAnimSetUpperSeq(playerinfo, ASEQ_WRRBOW_END);
	   	return(ASEQ_WRRBOW_END);
	}
	else 
	{
		PlayerAnimSetUpperSeq(playerinfo, ASEQ_WPHBOW_END);
	   	return(ASEQ_WPHBOW_END);
	}
}

// if we are out of hellstaff ammo, then switch us to the next weapon

/*-----------------------------------------------
	BranchCheckHellAmmo
-----------------------------------------------*/

int BranchCheckHellAmmo(playerinfo_t *playerinfo)
{
	if (Weapon_CurrentShotsLeft(playerinfo) || playerinfo->isclient)		// The client prediction shouldn't test the weapon.
		return(ASEQ_NONE);

	playerinfo->G_WeapNext(playerinfo->self);
   	PlayerAnimSetUpperSeq(playerinfo, ASEQ_WHELL_END);
   	return(ASEQ_WHELL_END);
}


/*-----------------------------------------------
	BranchUprReady
-----------------------------------------------*/

int BranchUprReady(playerinfo_t *playerinfo)
{
	assert(playerinfo);

	if((playerinfo->switchtoweapon!=playerinfo->pers.weaponready||playerinfo->pers.newweapon)&&
		!(playerinfo->edictflags&FL_CHICKEN))
	{
		// Not a chicken, so switch weapons.
		playerinfo->idletime=playerinfo->leveltime;	

		return(PlayerAnimWeaponSwitch(playerinfo));
	}

	switch(playerinfo->pers.weaponready)
	{
		case WEAPON_READY_SWORDSTAFF:
			return BranchUprReadySwordStaff(playerinfo);
			break;
		case WEAPON_READY_HELLSTAFF:
			return BranchUprReadyHellStaff(playerinfo);
			break;
		case WEAPON_READY_BOW:
			return BranchUprReadyBow(playerinfo);
			break;
		case WEAPON_READY_HANDS:
			return BranchUprReadyHands(playerinfo);
			break;
		default:		// In case Weapon_ready_none
			playerinfo->pers.weaponready = WEAPON_READY_HANDS;
			return BranchUprReadyHands(playerinfo);
			break;
	}

	return(ASEQ_NONE);
}

// if we are out of offensive mana, then switch us to the next weapon
int BranchCheckMana(playerinfo_t *playerinfo)
{
	if (Weapon_CurrentShotsLeft(playerinfo) || playerinfo->isclient)		// The client prediction shouldn't test the weapon.
		return(BranchUprReady(playerinfo));

	playerinfo->G_WeapNext(playerinfo->self);

   	return(ASEQ_NONE);
}


/*-----------------------------------------------
	BranchIdle
-----------------------------------------------*/

int BranchIdle(playerinfo_t *playerinfo)
{
	assert(playerinfo);
	
	if(!playerinfo->sv_cinematicfreeze)
	{
		//Run special cases if we're in the ready position
		if (playerinfo->lowerseq == ASEQ_IDLE_READY_GO ||
			playerinfo->lowerseq == ASEQ_IDLE_READY ||
			playerinfo->lowerseq == ASEQ_IDLE_LOOKR ||
			playerinfo->lowerseq == ASEQ_IDLE_LOOKL)
		{
			switch(playerinfo->irand(playerinfo, 0, 6))
			{
			case 0:
				return ASEQ_IDLE_LOOKR;
				break;
			case 1:
				return ASEQ_IDLE_LOOKL;
				break;
			case 2:
				return ASEQ_IDLE_READY_END;
				break;
			default:
				return ASEQ_NONE;
				break;
			}
		}
		else if ((playerinfo->pers.weaponready == WEAPON_READY_BOW) || (playerinfo->isclient))
		{	
			// Because the bow doesn't look right in some idles.
			switch(playerinfo->irand(playerinfo, 0, 10))
			{
				case 0:
					return ASEQ_IDLE_WIPE_BROW;
					break;
				case 1:
					if (playerinfo->irand(playerinfo, 0, 1) == 1)
						return ASEQ_IDLE_SCRATCH_ASS;

					break;
				case 2:
					return ASEQ_IDLE_LOOKBACK;
					break;
				default:
					return ASEQ_IDLE_READY;
					break;
			}
		}
		else if ( (playerinfo->pers.weaponready == WEAPON_READY_SWORDSTAFF) || (playerinfo->pers.weaponready == WEAPON_READY_HELLSTAFF))
		{
			// Because the staff doesn't look right in some idles.
			switch(playerinfo->irand(playerinfo, 0, 10))
			{
				case 0:
					return ASEQ_IDLE_FLY1;
					break;
				case 1:
					return ASEQ_IDLE_FLY2;
					break;
				case 2:
					return ASEQ_IDLE_WIPE_BROW;
					break;
				default:
					return ASEQ_IDLE_READY;
					break;
			}
		}
		else
		{
			switch(playerinfo->irand(playerinfo, 0, 10))
			{
				case 0:
					return ASEQ_IDLE_FLY1;
					break;
				case 1:
					return ASEQ_IDLE_FLY2;
					break;
				case 2:
					if (playerinfo->irand(playerinfo, 0, 1) == 1)
						return ASEQ_IDLE_SCRATCH_ASS;

					break;
				case 3:
					return ASEQ_IDLE_LOOKBACK;
					break;
				case 4:
					return ASEQ_IDLE_WIPE_BROW;
					break;
				default:
					return ASEQ_IDLE_READY;
					break;
			}
		}
	}

	return ASEQ_NONE;
}

