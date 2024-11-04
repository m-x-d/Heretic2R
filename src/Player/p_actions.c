//
// p_actions.c
//
// Heretic II
// Copyright 1998 Raven Software
//

#include <windows.h>

#include "player.h"
#include "p_types.h"
#include "p_actions.h"
#include "p_main.h"
#include "p_weapon.h"
#include "p_weapon.h"
#include "g_items.h"
#include "surfaceprops.h"
#include "m_player.h"
#include "fx.h"
#include "random.h"
#include "reference.h"
#include "vector.h"
#include "g_playstats.h"
#include "p_anim_data.h"
#include "q_shared.h"

#define AIRMOVE_AMOUNT		48
#define AIRMOVE_THRESHOLD	64
#define	QUICKTURN_RATE		(-360.0)	// Rotational velocity (degrees/second).

qboolean BranchCheckDismemberAction(playerinfo_t *playerinfo, int weapon);


vec3_t	handmins = {-2.0, -2.0, 0},
		handmaxs = {2.0, 2.0, 2.0};

int		traillength[TRAIL_MAX] =
{
	7,		//	TRAIL_SPIN1,
	6,		//	TRAIL_SPIN2,
	7,		//	TRAIL_STAND,
	6,		//	TRAIL_STEP,
	4,		//	TRAIL_BACK,
	8,		//	TRAIL_STAB,
	6,		//	TRAIL_COUNTERLEFT,
	6,		//	TRAIL_COUNTERRIGHT,
};


/*-----------------------------------------------
	CL_NormaliseAngle
-----------------------------------------------*/

float CL_NormaliseAngle(float Angle)
{
	int Count;

	Count=(int)(Angle/360.0);
		
	Angle=Angle-(Count*360.0);

	if(Angle>180.0)
	{
		Angle+=-360.0;
	}
	else if(Angle<-180.0)
	{
		Angle+=360.0;
	}

	return(Angle);
}

/*-----------------------------------------------
	PlayerActionCheckBranchRunning
-----------------------------------------------*/

void PlayerActionCheckBranchRunningStrafe ( playerinfo_t *playerinfo )
{
	int BranchLwrRunningStrafe(playerinfo_t *playerinfo);
	
	int ret;

	ret = BranchLwrRunningStrafe(playerinfo);

	if (ret)
		PlayerAnimSetLowerSeq(playerinfo, ret);
}

/*-----------------------------------------------
	PlayerActionCheckStrafe
-----------------------------------------------*/

void PlayerActionCheckStrafe ( playerinfo_t *playerinfo )
{
	//Check forward advancement
	if (playerinfo->seqcmd[ACMDL_FWD])
	{
		if (playerinfo->seqcmd[ACMDL_STRAFE_L])
		{
			PlayerAnimSetLowerSeq(playerinfo, ASEQ_WSTRAFE_LEFT);
			return;
		}
		else if (playerinfo->seqcmd[ACMDL_STRAFE_R])
		{
			PlayerAnimSetLowerSeq(playerinfo, ASEQ_WSTRAFE_RIGHT);
			return;
		}
		else
		{
			PlayerAnimSetLowerSeq(playerinfo, ASEQ_WALKF);
			return;
		}
	}
	
	//Check backward advancement
	if (playerinfo->seqcmd[ACMDL_BACK])
	{
		if (playerinfo->seqcmd[ACMDL_STRAFE_L])
		{
			PlayerAnimSetLowerSeq(playerinfo, ASEQ_WSTRAFEB_LEFT);
			return;
		}
		else if (playerinfo->seqcmd[ACMDL_STRAFE_R])
		{
			PlayerAnimSetLowerSeq(playerinfo, ASEQ_WSTRAFEB_RIGHT);
			return;
		}
		else
		{
			PlayerAnimSetLowerSeq(playerinfo, ASEQ_WALKB);
			return;
		}
	}

	//Check for a jump
	if (playerinfo->seqcmd[ACMDL_JUMP])
	{
		if (playerinfo->seqcmd[ACMDL_STRAFE_L])
		{
			PlayerAnimSetLowerSeq(playerinfo, ASEQ_JUMPLEFT_SGO);
			return;
		}
		else if (playerinfo->seqcmd[ACMDL_STRAFE_R])
		{
			PlayerAnimSetLowerSeq(playerinfo, ASEQ_JUMPRIGHT_SGO);
			return;
		}
	}

	//Check for crouching
	if (playerinfo->seqcmd[ACMDL_CROUCH])
	{
		if (playerinfo->seqcmd[ACMDL_STRAFE_L])
		{
			PlayerAnimSetLowerSeq(playerinfo, ASEQ_ROLL_L);
			return;
		}
		else if (playerinfo->seqcmd[ACMDL_STRAFE_R])
		{
			PlayerAnimSetLowerSeq(playerinfo, ASEQ_ROLL_R);
			return;
		}
	}
	
	//Check for change in strafe direction
	if ( playerinfo->seqcmd[ACMDL_STRAFE_L] && playerinfo->lowerseq != ASEQ_STRAFEL)
	{
		PlayerAnimSetLowerSeq(playerinfo, ASEQ_STRAFEL);
		return;
	}

	if ( playerinfo->seqcmd[ACMDL_STRAFE_R] && playerinfo->lowerseq != ASEQ_STRAFER)
	{
		PlayerAnimSetLowerSeq(playerinfo, ASEQ_STRAFER);
		return;
	}

	if ( !(playerinfo->seqcmd[ACMDL_STRAFE_L]) && !(playerinfo->seqcmd[ACMDL_STRAFE_R]) ) 
	{//We're just trying to go forward

		//FORWARD

		if (playerinfo->seqcmd[ACMDL_CREEP_F])
		{
			PlayerAnimSetLowerSeq(playerinfo, ASEQ_CREEPF);
			return;
		}

		if (playerinfo->seqcmd[ACMDL_WALK_F])
		{
			PlayerAnimSetLowerSeq(playerinfo, ASEQ_WALKF_GO);
			return;
		}
		
		if (playerinfo->seqcmd[ACMDL_RUN_F])
		{
			PlayerAnimSetLowerSeq(playerinfo, ASEQ_RUNF_GO);
			return;
		}
		
		//BACKWARD

		if (playerinfo->seqcmd[ACMDL_CREEP_B])
		{
			PlayerAnimSetLowerSeq(playerinfo, ASEQ_CREEPB);
			return;
		}

		if (playerinfo->seqcmd[ACMDL_BACK])
		{
			PlayerAnimSetLowerSeq(playerinfo, ASEQ_WALKB);
			return;
		}
		
		PlayerAnimSetLowerSeq(playerinfo, SeqCtrl[playerinfo->lowerseq].ceaseseq);
		return;
	}
}

/*-----------------------------------------------
	PlayerActionCheckVaultKick
-----------------------------------------------*/

void PlayerActionCheckVaultKick ( playerinfo_t *playerinfo )
{
	if(!playerinfo->isclient)
		playerinfo->G_PlayerVaultKick(playerinfo);
}

/*-----------------------------------------------
	PlayerActionCheckCreepMoveForward
-----------------------------------------------*/

qboolean PlayerActionCheckCreepMoveForward( playerinfo_t *playerinfo )
{
	vec3_t		startpos, endpos, vf, ang, mins;
	trace_t		trace;

	//Scan out and down from the player
	VectorCopy(playerinfo->origin, startpos);
	
	//Ignore the pitch of the player, we only want the yaw
	VectorSet(ang, 0, playerinfo->angles[YAW], 0);
	AngleVectors(ang, vf, NULL, NULL);
	
	//Trace ahead about one step
	VectorMA(playerinfo->origin, CREEP_STEPDIST, vf, startpos);

	//Account for stepheight
	VectorCopy(playerinfo->mins, mins);
	mins[2] += CREEP_MAXFALL;

	//Trace forward to see if the path is clear
	if(playerinfo->isclient)
		playerinfo->CL_Trace(playerinfo->origin,mins,playerinfo->maxs,startpos,MASK_PLAYERSOLID,CEF_CLIP_TO_WORLD,&trace);
	else
		playerinfo->G_Trace(playerinfo->origin, mins, playerinfo->maxs, startpos, playerinfo->self, MASK_PLAYERSOLID,&trace);
	
	//If it is...
	if (trace.fraction == 1)
	{
		//Move the endpoint down the maximum amount
		VectorCopy(startpos, endpos);
		endpos[2] += (playerinfo->mins[2] - CREEP_MAXFALL);

		//Trace down
		if(playerinfo->isclient)
			playerinfo->CL_Trace(startpos,mins,playerinfo->maxs,endpos,MASK_PLAYERSOLID,CEF_CLIP_TO_WORLD,&trace);
		else
			playerinfo->G_Trace(startpos, mins, playerinfo->maxs, endpos, playerinfo->self, MASK_PLAYERSOLID,&trace);

		if (trace.fraction == 1 || (trace.startsolid || trace.allsolid))
		{
			return false;
		}
	}
	else
	{
		return false;
	}

	return true;
}

/*-----------------------------------------------
	PlayerActionCheckCreepMoveBack
-----------------------------------------------*/

qboolean PlayerActionCheckCreepMoveBack( playerinfo_t *playerinfo )
{
	vec3_t		startpos, endpos, vf, ang, mins;
	trace_t		trace;

	//Scan out and down from the player
	VectorCopy(playerinfo->origin, startpos);
	
	//Ignore the pitch of the player, we only want the yaw
	VectorSet(ang, 0, playerinfo->angles[YAW], 0);
	AngleVectors(ang, vf, NULL, NULL);
	
	//Trace ahead about one step
	VectorMA(playerinfo->origin, -CREEP_STEPDIST, vf, startpos);

	//Account for stepheight
	VectorCopy(playerinfo->mins, mins);
	mins[2] += CREEP_MAXFALL;

	//Trace forward to see if the path is clear
	if(playerinfo->isclient)
		playerinfo->CL_Trace(playerinfo->origin,mins,playerinfo->maxs,startpos,MASK_PLAYERSOLID,CEF_CLIP_TO_WORLD,&trace);
	else
		playerinfo->G_Trace(playerinfo->origin, mins, playerinfo->maxs, startpos, playerinfo->self, MASK_PLAYERSOLID,&trace);
	
	//If it is...
	if (trace.fraction == 1)
	{
		//Move the endpoint down the maximum amount
		VectorCopy(startpos, endpos);
		endpos[2] += (playerinfo->mins[2] - CREEP_MAXFALL);

		//Trace down
		if(playerinfo->isclient)
			playerinfo->CL_Trace(startpos,mins,playerinfo->maxs,endpos,MASK_PLAYERSOLID,CEF_CLIP_TO_WORLD,&trace);
		else
			playerinfo->G_Trace(startpos, mins, playerinfo->maxs, endpos, playerinfo->self, MASK_PLAYERSOLID,&trace);

		if (trace.fraction == 1 || (trace.startsolid || trace.allsolid))
		{
			return false;
		}
	}
	else
	{
		return false;
	}

	return true;
}

/*-----------------------------------------------
	PlayerActionSetCrouchHeight
-----------------------------------------------*/

void PlayerActionSetCrouchHeight( playerinfo_t *playerinfo )
{
	playerinfo->maxs[2] = 4;
}

/*-----------------------------------------------
	PlayerActionCheckUncrouchToFinishSeq
-----------------------------------------------*/
qboolean CheckUncrouch(playerinfo_t *playerinfo);
void PlayerActionCheckUncrouchToFinishSeq(playerinfo_t *playerinfo)
{
	int	sequence;

	if(CheckUncrouch(playerinfo))
	{
		playerinfo->maxs[2] = 25;
		return;//ok to finish sequence
	}

//	PlayerActionSetCrouchHeight(playerinfo);

	if(playerinfo->upperseq == ASEQ_NONE)
		sequence = playerinfo->lowerseq;
	else
		sequence = playerinfo->upperseq;
	
	switch(sequence)
	{//choose a proper sequence to go into
	case ASEQ_FORWARD_FLIP_L:
	case ASEQ_FORWARD_FLIP_R:
		PlayerAnimSetUpperSeq(playerinfo, ASEQ_NONE);
		PlayerAnimSetLowerSeq(playerinfo, ASEQ_ROLL_FROM_FFLIP);		
		break;
	default:
		PlayerAnimSetUpperSeq(playerinfo, ASEQ_NONE);
		PlayerAnimSetLowerSeq(playerinfo, ASEQ_CROUCH);		
		break;
	}
}

/*-----------------------------------------------
	PlayerActionTurn180
-----------------------------------------------*/

void PlayerActionTurn180( playerinfo_t *playerinfo )
{
}

// ************************************************************************************************
// PlayerActionSetQTEndTime
// ------------------------
// ************************************************************************************************

void PlayerActionSetQTEndTime(playerinfo_t *playerinfo,float QTEndTime)
{
	playerinfo->quickturn_rate=QUICKTURN_RATE;
}

/*-----------------------------------------------
	PlayerActionCheckDoubleJump
-----------------------------------------------*/

void PlayerActionCheckDoubleJump( playerinfo_t *playerinfo )
{
	//FIXME: Debounce!!!
	//Check to see if the player is still pressing jump, and is not trying to fire or grab a ledge (action)
	if ( playerinfo->seqcmd[ACMDL_JUMP] && (!(playerinfo->seqcmd[ACMDU_ATTACK])) && (!(playerinfo->seqcmd[ACMDL_ACTION])))
	{
		switch(playerinfo->lowerseq)
		{
		case ASEQ_JUMPFWD:
			PlayerAnimSetLowerSeq(playerinfo, ASEQ_FORWARD_FLIP_L_GO);
			break;
		
		case ASEQ_JUMPBACK:
			PlayerAnimSetLowerSeq(playerinfo, ASEQ_JUMPFLIPBACK);
			break;

		case ASEQ_JUMPLEFT:
			PlayerAnimSetLowerSeq(playerinfo, ASEQ_JUMPFLIPLEFT);
			break;

		case ASEQ_JUMPRIGHT:
			PlayerAnimSetLowerSeq(playerinfo, ASEQ_JUMPFLIPRIGHT);
			break;
		}
	}
}

/*-----------------------------------------------
	PlayerActionCheckBowRefire
-----------------------------------------------*/
// This is called during the hold ready bow sequence, so that we may interrupt it if necessary.
void PlayerActionCheckBowRefire( playerinfo_t *playerinfo )
{
	if(playerinfo->switchtoweapon!=playerinfo->pers.weaponready||playerinfo->pers.newweapon)
	{	// Switching weapons is one reason to end the bow refire waiting.
		if (playerinfo->pers.weapon->tag == ITEM_WEAPON_REDRAINBOW)
		{
			PlayerAnimSetUpperSeq(playerinfo, ASEQ_WRRBOW_END);
		}
		else if (playerinfo->pers.weapon->tag == ITEM_WEAPON_PHOENIXBOW)
		{
			PlayerAnimSetUpperSeq(playerinfo, ASEQ_WPHBOW_END);
		}
	}

	if(playerinfo->seqcmd[ACMDU_ATTACK]  && !(playerinfo->edictflags & FL_CHICKEN) && Weapon_CurrentShotsLeft(playerinfo))	// Not a chicken
	{	// Shooting is the other!
		if (playerinfo->pers.weapon->tag == ITEM_WEAPON_REDRAINBOW)
		{
			PlayerAnimSetUpperSeq(playerinfo, ASEQ_WRRBOW_DRAW);
		}
		else if (playerinfo->pers.weapon->tag == ITEM_WEAPON_PHOENIXBOW)
		{
			PlayerAnimSetUpperSeq(playerinfo, ASEQ_WPHBOW_DRAW);
		}
	}
	else
	if (!playerinfo->isclient&&!(Weapon_CurrentShotsLeft(playerinfo)))
		playerinfo->G_WeapNext(playerinfo->self);
		
}

/*-----------------------------------------------
	PlayerActionBreath
-----------------------------------------------*/

void PlayerActionBreath(playerinfo_t *playerinfo, float value)
{
}

/*-----------------------------------------------
	PlayerActionHandFXStart
-----------------------------------------------*/

void PlayerActionHandFXStart(playerinfo_t *playerinfo, float value)
{
	switch((int)value)
	{
	case HANDFX_FIREBALL:
		PlayerSetHandFX(playerinfo,(int)value, 4);
		break;
	case HANDFX_MISSILE:
		PlayerSetHandFX(playerinfo,(int)value, 6);
		break;
	case HANDFX_FIREWALL:
		PlayerSetHandFX(playerinfo,(int)value, 10);
		break;
	}
}

/*-----------------------------------------------
	PlayerActionSphereTrailEnd
-----------------------------------------------*/

void PlayerActionSphereTrailEnd(playerinfo_t *playerinfo, float value)
{
	// The sphere hand trails must be manually shut off.
	playerinfo->effects &= ~EF_TRAILS_ENABLED;
}

/*-----------------------------------------------
	PlayerActionSwordAttack
-----------------------------------------------*/

void PlayerActionSwordAttack(playerinfo_t *playerinfo, float value)
{
	playerinfo->PlayerActionSwordAttack(playerinfo,(int)value);
}

/*-----------------------------------------------
	PlayerActionSpellFireball
-----------------------------------------------*/

void PlayerActionSpellFireball(playerinfo_t *playerinfo, float value)
{
	playerinfo->PlayerActionSpellFireball(playerinfo);
}

/*-----------------------------------------------
	PlayerActionSpellBlast
-----------------------------------------------*/

void PlayerActionSpellBlast(playerinfo_t *playerinfo, float value)
{
	playerinfo->PlayerActionSpellBlast(playerinfo);
}

/*-----------------------------------------------
	PlayerActionSpellArray
-----------------------------------------------*/

void PlayerActionSpellArray(playerinfo_t *playerinfo, float value)
{
	int shotsleft;

	shotsleft=Weapon_CurrentShotsLeft(playerinfo);

	if (shotsleft <= 0)
		return;		// Outta ammo
	else if (value==2.0 && shotsleft <= 3)
		return;		// Three or less shots, use only the inner three projectiles
	else if (value==1.0 && shotsleft <= 1)
		return;		// Only one shot, use the center projectile slot.

	playerinfo->PlayerActionSpellArray(playerinfo,(int)value);
}

/*-----------------------------------------------
	PlayerActionSpellSphereCreate
-----------------------------------------------*/

void PlayerActionSpellSphereCreate(playerinfo_t *playerinfo, float value)
{
	PlayerSetHandFX(playerinfo, HANDFX_SPHERE, -1);
	
	playerinfo->chargingspell=true;
	playerinfo->weaponcharge = 1;
	playerinfo->PlayerActionSpellSphereCreate(playerinfo,&playerinfo->chargingspell);
}

/*-----------------------------------------------
	PlayerActionSpellSphereCharge
-----------------------------------------------*/

void PlayerActionSpellSphereCharge(playerinfo_t *playerinfo, float value)
{
	// Drain mana while charging. If mana depleted, then the branch will set to launch the thing.

	if(playerinfo->seqcmd[ACMDU_ATTACK] && 
			(playerinfo->weaponcharge < SPHERE_MAX_MANA_CHARGE) && 
			((Weapon_CurrentShotsLeft(playerinfo)) || 
					(playerinfo->pers.inventory.Items[playerinfo->weap_ammo_index] >= SPHERE_MANA_PER_CHARGE)))
	{
		if (!(playerinfo->dmflags & DF_INFINITE_MANA))
		{
			playerinfo->pers.inventory.Items[playerinfo->weap_ammo_index] -= SPHERE_MANA_PER_CHARGE;
			playerinfo->weaponcharge++;

			return;
		}
	}

	if ((!Weapon_CurrentShotsLeft(playerinfo) && value != 4.0) || !(playerinfo->seqcmd[ACMDU_ATTACK]))
	{	// If we are out of ammo, or if we have let go of the button, then fire.
		switch((int)value)
		{
		case 1:
			PlayerAnimSetUpperSeq(playerinfo, ASEQ_WSPHERE_FIRE1);		
			break;
		case 2:
			PlayerAnimSetUpperSeq(playerinfo, ASEQ_WSPHERE_FIRE2);
			break;
		case 3:
			PlayerAnimSetUpperSeq(playerinfo, ASEQ_WSPHERE_FIRE3);
			break;
		case 4:
			PlayerAnimSetUpperSeq(playerinfo, ASEQ_WSPHERE_FIRE4);
			break;
		}
	}
}

/*-----------------------------------------------
	PlayerActionSpellSphereRelease
-----------------------------------------------*/

void PlayerActionSpellSphereRelease(playerinfo_t *playerinfo, float value)
{
	if(value==1.0)
	{
		if(playerinfo->seqcmd[ACMDU_ATTACK]==false)
		{
			playerinfo->chargingspell=false;
			playerinfo->weaponcharge = 0;
		}
	}
	else
	{
		playerinfo->chargingspell=false;
		playerinfo->weaponcharge = 0;
	}
}

/*-----------------------------------------------
	PlayerActionSpellBigBall
-----------------------------------------------*/

void PlayerActionSpellBigBall(playerinfo_t *playerinfo, float value)
{
	playerinfo->PlayerActionSpellBigBall(playerinfo);
}

/*-----------------------------------------------
	PlayerActionSpellFirewall
-----------------------------------------------*/

void PlayerActionSpellFirewall(playerinfo_t *playerinfo, float value)
{
	playerinfo->PlayerActionSpellFirewall(playerinfo);
}

/*-----------------------------------------------
	PlayerActionSpellFirewall
-----------------------------------------------*/

void PlayerActionRedRainBowAttack(playerinfo_t *playerinfo, float value)
{
	if (Weapon_CurrentShotsLeft(playerinfo) <= 0)
		return;		// Outta ammo

	playerinfo->PlayerActionRedRainBowAttack(playerinfo);
}

/*-----------------------------------------------
	PlayerActionPhoenixBowAttack
-----------------------------------------------*/

void PlayerActionPhoenixBowAttack(playerinfo_t *playerinfo, float value)
{
	if (Weapon_CurrentShotsLeft(playerinfo) <= 0)
		return;		// Outta ammo

	playerinfo->PlayerActionPhoenixBowAttack(playerinfo);
}

/*-----------------------------------------------
	PlayerActionHellstaffAttack
-----------------------------------------------*/

void PlayerActionHellstaffAttack(playerinfo_t *playerinfo, float value)
{
	if (Weapon_CurrentShotsLeft(playerinfo) <= 0)
		return;		// Outta ammo
	playerinfo->PlayerActionHellstaffAttack(playerinfo);
}

/*-----------------------------------------------
	PlayerActionSpellDefensive
-----------------------------------------------*/
void PlayerActionSpellDefensive(playerinfo_t *playerinfo, float value)
{
}

/*-----------------------------------------------
	PlayerActionSpellChange
-----------------------------------------------*/

void PlayerActionSpellChange(playerinfo_t *playerinfo, float value)
{
	vec3_t	forward, right, spawnpoint;
	int		color;

	assert(playerinfo);

	if (playerinfo->edictflags & FL_CHICKEN)
	{
		// Don't allow us to muck about with spells if we are a chicken.

		return;
	}

	assert(playerinfo->pers.newweapon);

	Weapon_Ready(playerinfo, playerinfo->pers.newweapon);
	playerinfo->pers.newweapon = NULL;

	// Do some fancy effect.

	AngleVectors(playerinfo->angles, forward, right, NULL);
	VectorMA(playerinfo->origin, -2.0, forward, spawnpoint);
	VectorMA(spawnpoint, -7, right, spawnpoint);
	spawnpoint[2] += playerinfo->viewheight - 16.0;

	switch(playerinfo->pers.weapon->tag)
	{
		case ITEM_WEAPON_FLYINGFIST:
			color=1;
			break;

		case ITEM_WEAPON_MAGICMISSILE:
			color=2;
			break;
		
		case ITEM_WEAPON_SPHEREOFANNIHILATION:
			color=3;
			break;
		
		case ITEM_WEAPON_MACEBALLS:
			color=4;
			break;
		
		case ITEM_WEAPON_FIREWALL:
			color=5;
			break;
		
		default:
			color=0;
			break;
	}

	if(playerinfo->isclient)
	{
		playerinfo->CL_Sound(SND_PRED_ID0,
							 playerinfo->origin,
							 CHAN_WEAPON,
							 "Weapons/SpellChange.wav",
							 1.0,
							 ATTN_NORM,
							 0);

		playerinfo->CL_CreateEffect(EFFECT_PRED_ID1,NULL,FX_SPELL_CHANGE,0,spawnpoint,"db",right,color);
	}
	else
	{
		playerinfo->G_Sound(SND_PRED_ID0,
							playerinfo->leveltime,
							playerinfo->self,
							CHAN_WEAPON,
							playerinfo->G_SoundIndex("Weapons/SpellChange.wav"),
							1.0,
							ATTN_NORM,
							0);

		playerinfo->G_CreateEffect(EFFECT_PRED_ID1,NULL,FX_SPELL_CHANGE,0,spawnpoint,"db",right,color);
	}
}

/*-----------------------------------------------
	PlayerActionArrowChange
-----------------------------------------------*/

void PlayerActionArrowChange(playerinfo_t *playerinfo, float value)
{
	vec3_t forward, right, spawnpoint;
	gitem_t *weapon;
	int color=0;

	assert(playerinfo);
	
	if(playerinfo->edictflags & FL_CHICKEN)
	{
		// Don't allow us to muck about with arrows if we are a chicken.

		return;
	}

	assert(playerinfo->pers.newweapon);
	
	Weapon_Ready(playerinfo, playerinfo->pers.newweapon);
	playerinfo->pers.newweapon = NULL;

	// Do some fancy effect.
	
	AngleVectors(playerinfo->angles, forward, right, NULL);
	VectorMA(playerinfo->origin, -2.0, forward, spawnpoint);
	VectorMA(spawnpoint, -7, right, spawnpoint);
	spawnpoint[2] += playerinfo->viewheight - 16.0;

	weapon = playerinfo->pers.weapon;
	
	if (weapon)
	{
		if (weapon->tag == ITEM_WEAPON_PHOENIXBOW)
		{	
			// Set the bow type to phoenix.

			playerinfo->pers.bowtype = BOW_TYPE_PHOENIX;
			color=6;
		}
		else
		{
			// Set the bow type to red-rain.

			playerinfo->pers.bowtype = BOW_TYPE_REDRAIN;
			color=7;
		}

		PlayerUpdateModelAttributes(playerinfo);
	}

	if(playerinfo->isclient)
	{
		playerinfo->CL_Sound(SND_PRED_ID1,
							 playerinfo->origin,
							 CHAN_WEAPON,
							 "Weapons/SpellChange.wav",
							 1.0,
							 ATTN_NORM,
							 0);

		playerinfo->CL_CreateEffect(EFFECT_PRED_ID2,NULL,FX_SPELL_CHANGE,0,spawnpoint,"db",right,color);
	}
	else
	{
		playerinfo->G_Sound(SND_PRED_ID1,
							playerinfo->leveltime,
							playerinfo->self,
							CHAN_WEAPON,
							playerinfo->G_SoundIndex("Weapons/SpellChange.wav"),
							1.0,
							ATTN_NORM,
							0);

		playerinfo->G_CreateEffect(EFFECT_PRED_ID2,NULL,FX_SPELL_CHANGE,0,spawnpoint,"db",right,color);
	}
}

/*-----------------------------------------------
	PlayerActionWeaponChange
-----------------------------------------------*/

// By default changes weapon to the weapon indicated by switchtoweapon. If a value is passed, then
// instead it will simply change the appearance, not the actual weapon.

void PlayerActionWeaponChange(playerinfo_t *playerinfo, float value)
{
	gitem_t *weapon;
	vec3_t	spawnpoint, forward, right;
	int		holdweapon;

	assert(playerinfo);

	if (playerinfo->edictflags & FL_CHICKEN)
	{
		// Don't allow us to muck about with spells if we are a chicken.

		return;
	}

	if (value)
	{
		// Don't REALLY change the weaponready, since the value indicates a cosmetic weapon change only (the real change is later)
		holdweapon = playerinfo->pers.weaponready;

		playerinfo->pers.weaponready = value;
 		PlayerUpdateModelAttributes(playerinfo);

		playerinfo->pers.weaponready = holdweapon;
	}
	else
	{
		assert(playerinfo->pers.newweapon);
		Weapon_Ready(playerinfo, playerinfo->pers.newweapon);
		playerinfo->pers.weaponready = playerinfo->switchtoweapon;
		PlayerUpdateModelAttributes(playerinfo);
		playerinfo->pers.newweapon = NULL;
	}
	
	if (playerinfo->leveltime > 1.0)
	{	
		// Weapon Changing effects.

		switch(playerinfo->pers.weaponready)
		{
		case WEAPON_READY_SWORDSTAFF:

			if(!playerinfo->isclient)
			{
				playerinfo->G_CreateEffect(EFFECT_PRED_ID3,
										   playerinfo->G_GetEntityStatePtr((edict_t *)playerinfo->self),
										   FX_STAFF_CREATEPOOF,
										   CEF_OWNERS_ORIGIN,
										   NULL,
										   "");
			}
			else
				playerinfo->CL_CreateEffect(EFFECT_PRED_ID3,
											playerinfo->self,
											FX_STAFF_CREATEPOOF,
										    CEF_OWNERS_ORIGIN,
										    NULL,
										    "");
			break;

		case WEAPON_READY_HELLSTAFF:

			if(!playerinfo->isclient)
				playerinfo->G_CreateEffect(EFFECT_PRED_ID4,
										   playerinfo->G_GetEntityStatePtr((edict_t *)playerinfo->self),
										   FX_STAFF_CREATEPOOF,
										   CEF_OWNERS_ORIGIN|CEF_FLAG6,
										   NULL,
										   "");
			else
				playerinfo->CL_CreateEffect(EFFECT_PRED_ID4,
											playerinfo->self,
											FX_STAFF_CREATEPOOF,
											CEF_OWNERS_ORIGIN|CEF_FLAG6,
											NULL,
											"");

			break;

		case WEAPON_READY_BOW:
			
			// Make sure we have the right bow color
			
			weapon = playerinfo->pers.weapon;
			
			if (weapon)
			{	
				// There is a weapon.

				if (weapon->tag == ITEM_WEAPON_REDRAINBOW)
				{	
					// Make sure we have the redrain visible.

					if (playerinfo->pers.bowtype == BOW_TYPE_PHOENIX)
					{	
						// Uh oh, change the phoenix into a red rain.

						playerinfo->pers.bowtype = BOW_TYPE_REDRAIN;
						PlayerUpdateModelAttributes(playerinfo);

						if(playerinfo->isclient)
						{
							playerinfo->CL_Sound(SND_PRED_ID2,
												 playerinfo->origin,
												 CHAN_WEAPON,
												 "Weapons/SpellChange.wav",
												 1.0,
												 ATTN_NORM,
												 0);
						}
						else
						{
							playerinfo->G_Sound(SND_PRED_ID2,
												playerinfo->leveltime,
												playerinfo->self,
												CHAN_WEAPON,
												playerinfo->G_SoundIndex("Weapons/SpellChange.wav"),
												1.0,
												ATTN_NORM,
												0);
						}

						// Do some fancy effect.

						AngleVectors(playerinfo->angles, forward, right, NULL);
						VectorMA(playerinfo->origin, -2.0, forward, spawnpoint);
						VectorMA(spawnpoint, -7, right, spawnpoint);
						spawnpoint[2] += playerinfo->viewheight - 16.0;

						if(!playerinfo->isclient)
							playerinfo->G_CreateEffect(EFFECT_PRED_ID5,
													   NULL,
													   FX_SPELL_CHANGE,
													   0,
													   spawnpoint,
													   "db",
													   right,
													   6);
						else
							playerinfo->CL_CreateEffect(EFFECT_PRED_ID5,
														NULL,
														FX_SPELL_CHANGE,
														0,
														spawnpoint,
														"db",
														right,
														6);
					}
				}
				else if (weapon->tag == ITEM_WEAPON_PHOENIXBOW)
				{	
					// Make sure we have the phoenix visible.

					if (playerinfo->pers.bowtype == BOW_TYPE_REDRAIN)
					{	
						// Uh oh, change the red rain to a phoenix.

						playerinfo->pers.bowtype = BOW_TYPE_PHOENIX;
						PlayerUpdateModelAttributes(playerinfo);

						if(playerinfo->isclient)
						{
							playerinfo->CL_Sound(SND_PRED_ID3,
												 playerinfo->origin,
												 CHAN_WEAPON,
												 "Weapons/SpellChange.wav",
												 1.0,
												 ATTN_NORM,
												 0);
						}
						else
						{
							playerinfo->G_Sound(SND_PRED_ID3,
												playerinfo->leveltime,
												playerinfo->self,
												CHAN_WEAPON,
												playerinfo->G_SoundIndex("Weapons/SpellChange.wav"),
												1.0,
												ATTN_NORM,
												0);
						}

						// Do some fancy effect.

						AngleVectors(playerinfo->angles, forward, right, NULL);
						VectorMA(playerinfo->origin, -2.0, forward, spawnpoint);
						VectorMA(spawnpoint, -7, right, spawnpoint);
						spawnpoint[2] += playerinfo->viewheight - 16.0;

						if(!playerinfo->isclient)
							playerinfo->G_CreateEffect(EFFECT_PRED_ID6,
													   NULL,
													   FX_SPELL_CHANGE,
													   0,
													   spawnpoint,
													   "db",
													   right,
													   7);
						else
							playerinfo->CL_CreateEffect(EFFECT_PRED_ID6,
														NULL,
														FX_SPELL_CHANGE,
														0,
														spawnpoint,
														"db",
														right,
														7);
					}
				}
			}
			
			break;

		default:
			
			// No nothing.

			break;
		}
	}
}

/*-----------------------------------------------
	PlayerActionStartStaffGlow
-----------------------------------------------*/

void PlayerActionStartStaffGlow(playerinfo_t *playerinfo, float value)
{
	int flags = CEF_OWNERS_ORIGIN;

	if(playerinfo->isclient)
	{
		if (value == WEAPON_READY_HELLSTAFF)
			flags |= CEF_FLAG6;

		if(playerinfo->pers.stafflevel == STAFF_LEVEL_BASIC || value == WEAPON_READY_HELLSTAFF)
		{
			playerinfo->CL_Sound(SND_PRED_ID5,
								 playerinfo->self,
								 CHAN_WEAPON,
								 "weapons/Staff Ready.wav",
								 1.0,
								 ATTN_NORM,
								 0);
		}
		else if(playerinfo->pers.stafflevel == STAFF_LEVEL_POWER1)//blue
		{
			flags |= CEF_FLAG7;

			playerinfo->CL_Sound(SND_PRED_ID6,
								 playerinfo->self,
								 CHAN_WEAPON,
								 "weapons/Staff2Ready.wav",
								 1.0,
								 ATTN_NORM,
								 0);
		}
		else if(playerinfo->pers.stafflevel == STAFF_LEVEL_POWER2)//flame
		{
			flags |= CEF_FLAG8;

			playerinfo->CL_Sound(SND_PRED_ID7,
								 playerinfo->self,
								 CHAN_WEAPON,
								 "weapons/Staff3Ready.wav",
								 1.0,
								 ATTN_NORM,
								 0);
		}
		else
			playerinfo->CL_Sound(SND_PRED_ID8,
								 playerinfo->self,
								 CHAN_WEAPON,
								 "weapons/Staff Ready.wav",
								 1.0,
								 ATTN_NORM,
								 0);

		playerinfo->CL_CreateEffect(EFFECT_PRED_ID7,
									playerinfo->self,
									FX_STAFF_CREATE,
									flags,
									NULL,
									"");
	}
	else
	{
		if (value == WEAPON_READY_HELLSTAFF)
			flags |= CEF_FLAG6;
	
		if(playerinfo->pers.stafflevel == STAFF_LEVEL_BASIC || value == WEAPON_READY_HELLSTAFF)
		{
			playerinfo->G_Sound(SND_PRED_ID5,
								playerinfo->leveltime,
								playerinfo->self,
								CHAN_WEAPON,
								playerinfo->G_SoundIndex("weapons/Staff Ready.wav"),
								1.0,
								ATTN_NORM,
								0);
		}
		else if(playerinfo->pers.stafflevel == STAFF_LEVEL_POWER1)//blue
		{
			flags |= CEF_FLAG7;

			playerinfo->G_Sound(SND_PRED_ID6,
								playerinfo->leveltime,
								playerinfo->self,
								CHAN_WEAPON,
								playerinfo->G_SoundIndex("weapons/Staff2Ready.wav"),
								1.0,
								ATTN_NORM,
								0);
		}
		else if(playerinfo->pers.stafflevel == STAFF_LEVEL_POWER2)//flame
		{
			flags |= CEF_FLAG8;

			playerinfo->G_Sound(SND_PRED_ID7,
								playerinfo->leveltime,
								playerinfo->self,
								CHAN_WEAPON,
								playerinfo->G_SoundIndex("weapons/Staff3Ready.wav"),
								1.0,
								ATTN_NORM,
								0);
		}
		else
			playerinfo->G_Sound(SND_PRED_ID8,
								playerinfo->leveltime,
								playerinfo->self,
								CHAN_WEAPON,
								playerinfo->G_SoundIndex("weapons/Staff Ready.wav"),
								1.0,
								ATTN_NORM,
								0);

		playerinfo->G_CreateEffect(EFFECT_PRED_ID7,
								   playerinfo->self,
								   FX_STAFF_CREATE,
								   flags,
								   NULL,
								   "");
	}
}

/*-----------------------------------------------
	PlayerActionEndStaffGlow
-----------------------------------------------*/

void PlayerActionEndStaffGlow(playerinfo_t *playerinfo, float value)
{
	int flags = CEF_OWNERS_ORIGIN;


	if(playerinfo->isclient)
	{
		if (value == WEAPON_READY_HELLSTAFF)
			flags |= CEF_FLAG6;

		if(playerinfo->pers.stafflevel == STAFF_LEVEL_BASIC || value == WEAPON_READY_HELLSTAFF)
		{
			playerinfo->CL_Sound(SND_PRED_ID9,
							playerinfo->self,
							CHAN_WEAPON,
							"weapons/Staff Unready.wav",
							1.0,
							ATTN_NORM,
							0);
		}
		else if(playerinfo->pers.stafflevel == STAFF_LEVEL_POWER1)//blue
		{
			flags |= CEF_FLAG7;

			playerinfo->CL_Sound(SND_PRED_ID10,
							playerinfo->self,
							CHAN_WEAPON,
							"weapons/Staff2Unready.wav",
							1.0,
							ATTN_NORM,
							0);
		}
		else if(playerinfo->pers.stafflevel == STAFF_LEVEL_POWER2)//flame
		{
			flags |= CEF_FLAG8;

			playerinfo->CL_Sound(SND_PRED_ID11,
							playerinfo->self,
							CHAN_WEAPON,
							"weapons/Staff3Unready.wav",
							1.0,
							ATTN_NORM,
							0);
		}
		else
		{
			playerinfo->CL_Sound(SND_PRED_ID12,
							playerinfo->self,
							CHAN_WEAPON,
							"weapons/Staff Unready.wav",
							1.0,
							ATTN_NORM,
							0);
		}

		playerinfo->CL_CreateEffect(EFFECT_PRED_ID8,
									playerinfo->self,
									FX_STAFF_REMOVE,
									flags,
									NULL,
									"");
	}
	else
	{
		if (value == WEAPON_READY_HELLSTAFF)
			flags |= CEF_FLAG6;

		if(playerinfo->pers.stafflevel == STAFF_LEVEL_BASIC || value == WEAPON_READY_HELLSTAFF)
		{
			playerinfo->G_Sound(SND_PRED_ID9,
								playerinfo->leveltime,
								playerinfo->self,
								CHAN_WEAPON,
								playerinfo->G_SoundIndex("weapons/Staff Unready.wav"),
								1.0,
								ATTN_NORM,
								0);
		}
		else if(playerinfo->pers.stafflevel == STAFF_LEVEL_POWER1)//blue
		{
			flags |= CEF_FLAG7;

			playerinfo->G_Sound(SND_PRED_ID10,
								playerinfo->leveltime,
								playerinfo->self,
								CHAN_WEAPON,
								playerinfo->G_SoundIndex("weapons/Staff2Unready.wav"),
								1.0,
								ATTN_NORM,
								0);
		}
		else if(playerinfo->pers.stafflevel == STAFF_LEVEL_POWER2)//flame
		{
			flags |= CEF_FLAG8;

			playerinfo->G_Sound(SND_PRED_ID11,
								playerinfo->leveltime,
								playerinfo->self,
								CHAN_WEAPON,
								playerinfo->G_SoundIndex("weapons/Staff3Unready.wav"),
								1.0,
								ATTN_NORM,
								0);
		}
		else
		{
			playerinfo->G_Sound(SND_PRED_ID12,
								playerinfo->leveltime,	
								playerinfo->self,
								CHAN_WEAPON,
								playerinfo->G_SoundIndex("weapons/Staff Unready.wav"),
								1.0,
								ATTN_NORM,
								0);
		}

		playerinfo->G_CreateEffect(EFFECT_PRED_ID8,
								   playerinfo->G_GetEntityStatePtr((edict_t *)playerinfo->self),
								   FX_STAFF_REMOVE,
								   flags,
								   NULL,
								   "");
	}
}

/*-----------------------------------------------
	PlayerActionSwordTrailStart
-----------------------------------------------*/

void PlayerActionSwordTrailSound(playerinfo_t *playerinfo, char *name)
{
	if(playerinfo->isclient)
	{
		playerinfo->CL_Sound(SND_PRED_ID13,
							 playerinfo->origin,
							 CHAN_WEAPON,
							 name,
							 1.0,
							 ATTN_NORM,
							 0);
	}
	else
	{
		playerinfo->G_Sound(SND_PRED_ID13,
							playerinfo->leveltime,
							playerinfo->self,
							CHAN_WEAPON,
							playerinfo->G_SoundIndex(name),
							1.0,
							ATTN_NORM,
							0);
	}
}

void PlayerActionSwordTrailStart(playerinfo_t *playerinfo, float value)
{
	int powerlevel;
	int	length;
	qboolean spin;

	// Add a trail effect to the staff.
	if (playerinfo->powerup_timer > playerinfo->leveltime)
		powerlevel = playerinfo->pers.stafflevel + 1;
	else 
		powerlevel = playerinfo->pers.stafflevel;

	if (powerlevel >= STAFF_LEVEL_MAX)
		powerlevel = STAFF_LEVEL_MAX-1;

	assert((int)value >= 0 && (int)value < TRAIL_MAX);
	length = traillength[(int)value];

	PlayerSetHandFX(playerinfo, HANDFX_STAFF1 + powerlevel - 1, length);

	spin = ((int)value == TRAIL_SPIN1 || (int)value == TRAIL_SPIN2 || (int)value == TRAIL_STAB);

	switch (powerlevel)
	{
	//Normal Staff
	case STAFF_LEVEL_BASIC:
		if (!spin)
			PlayerActionSwordTrailSound(playerinfo, "weapons/staffswing.wav");
		else
			PlayerActionSwordTrailSound(playerinfo, "weapons/stafftwirl.wav");
		break;

	// Energy staff
	case STAFF_LEVEL_POWER1:
		if (!spin)
			PlayerActionSwordTrailSound(playerinfo, "weapons/staffswing_2.wav");
		else
			PlayerActionSwordTrailSound(playerinfo, "weapons/stafftwirl_2.wav");
		break;

	// Fire Staff
	case STAFF_LEVEL_POWER2:
		if (!spin)
			PlayerActionSwordTrailSound(playerinfo, "weapons/staffswing_3.wav");
		else
			PlayerActionSwordTrailSound(playerinfo, "weapons/stafftwirl_3.wav");
		break;

	}
}

/*-----------------------------------------------
	PlayerActionRedRainBowTrailStart
-----------------------------------------------*/

void PlayerActionRedRainBowTrailStart(playerinfo_t *playerinfo, float value)
{
	// Add a trail effect to the red-rain bow.

	if (playerinfo->powerup_timer > playerinfo->leveltime)
	{	// Power up the Arrow FX
		PlayerSetHandFX(playerinfo, HANDFX_POWERREDRAIN, -1);
	}
	else
	{
		PlayerSetHandFX(playerinfo, HANDFX_REDRAIN, -1);
	}

	if(!playerinfo->isclient)
	{
		playerinfo->G_L_Sound(playerinfo->self,playerinfo->G_SoundIndex("weapons/bowReady.wav"));
	}
}

/*-----------------------------------------------
	PlayerActionPhoenixBowTrailStart
-----------------------------------------------*/

void PlayerActionPhoenixBowTrailStart(playerinfo_t *playerinfo, float value)
{
	// Add a trail effect to the phoenix bow.

	if (playerinfo->powerup_timer > playerinfo->leveltime)
	{	// Power up the Arrow FX
		PlayerSetHandFX(playerinfo, HANDFX_POWERPHOENIX, -1);
	}
	else
	{
		PlayerSetHandFX(playerinfo, HANDFX_PHOENIX, -1);
	}

	if(!playerinfo->isclient)
	{
		playerinfo->G_L_Sound(playerinfo->self,playerinfo->G_SoundIndex("weapons/PhoenixReady.wav"));
	}
}

/*-----------------------------------------------
	PlayerActionBowTrailEnd
-----------------------------------------------*/

void PlayerActionBowTrailEnd(playerinfo_t *playerinfo, float value)
{
	playerinfo->effects &= ~EF_TRAILS_ENABLED;
}

/*-----------------------------------------------
	PlayerActionFootstep
-----------------------------------------------*/

void PlayerActionFootstep(playerinfo_t *playerinfo, float value)
{
	char	WalkSound[128];
	int		basestep, absstep;
	int		channel;

	//If we're not on the ground, skip all the rest
	if ( (value==STEP_ROLL) && (!playerinfo->groundentity) )
		return;

	absstep = (int)value;
	if (absstep >= STEP_OFFSET)
	{
		basestep = absstep - STEP_OFFSET;
		channel = CHAN_FOOTSTEP;
	}
	else
	{
		basestep = absstep;
		channel = CHAN_FOOTSTEP2;
	}

	if(!basestep)
		return;

	if (playerinfo->waterlevel)
	{
		if (basestep != STEP_RUN)			// Not running
		{
			if (irand(0,1))
				strcpy(WalkSound,"player/waterwalk1.wav");
			else
				strcpy(WalkSound,"player/waterwalk2.wav");
		}
		else
		{
			if (irand(0,1))
				strcpy(WalkSound,"player/waterrun1.wav");
			else
				strcpy(WalkSound,"player/waterrun2.wav");
		}
	}
	else
	{
		char	*material;

		material=GetClientGroundSurfaceMaterialName(playerinfo);

		if (!material)
			return;

		strcpy(WalkSound,"player/");
		strcat(WalkSound, material);

		switch (basestep)
		{
		case STEP_CREEP:		// Creep
			
			if(irand(0,1))
				strcat(WalkSound,"shuffle1.wav");
			else
				strcat(WalkSound,"shuffle2.wav");
			break;

		case STEP_WALK:		// Walk
			
			if(irand(0,1))
				strcat(WalkSound,"walk1.wav");
			else
				strcat(WalkSound,"walk2.wav");
			break;

		case STEP_RUN:		// Run
			
			if(irand(0,1))
				strcat(WalkSound,"run1.wav");
			else
				strcat(WalkSound,"run2.wav");
			break;

		case STEP_ROLL:		// Roll
			
			strcat(WalkSound,"roll.wav");
			break;

		}
	}

	if(playerinfo->isclient)
	{
		playerinfo->CL_Sound(SND_PRED_ID14,
							 playerinfo->origin,
							 channel,
							 WalkSound,
							 0.75,
							 ATTN_NORM,
							 0);
	}
	else
	{
  		playerinfo->G_Sound(SND_PRED_ID14,
							playerinfo->leveltime,
							playerinfo->self,
							channel,
							playerinfo->G_SoundIndex(WalkSound),
							0.75,
							ATTN_NORM,
							0);
	}
}

/*-----------------------------------------------
	PlayerActionSwimIdleSound
-----------------------------------------------*/

void PlayerActionSwimIdleSound(playerinfo_t *playerinfo, float value)
{
	if(playerinfo->isclient)
	{
		playerinfo->CL_Sound(SND_PRED_ID15,
							 playerinfo->origin,
							 CHAN_VOICE,
							 "player/swim idle.wav",
							 0.4,
							 ATTN_NORM,
							 0);
	}
	else
	{
	  	playerinfo->G_Sound(SND_PRED_ID15,
							playerinfo->leveltime,
							playerinfo->self,
							CHAN_VOICE,
							playerinfo->G_SoundIndex("player/swim idle.wav"),
							0.4,
							ATTN_NORM,
							0);
	}
}

/*-----------------------------------------------
	PlayerActionSwimSound
-----------------------------------------------*/

void PlayerActionSwimSound(playerinfo_t *playerinfo, float value)
{
	char *name;

	switch ( (int) value)
	{
	case SOUND_SWIM_FORWARD:
		name = "player/breaststroke.wav";
		break;

	case SOUND_SWIM_BACK:
		name = "player/Swim Backward.wav";
		break;

	case SOUND_SWIM_SIDE:
		name = (irand(0,1)) ? "player/SwimFreestyle1.wav" : "player/SwimFreestyle2.wav";
		break;

	case SOUND_SWIM_UNDER:
		name = "player/swimunder.wav";
		break;

	default :
		assert(0);
		name = "player/breatstroke.wav";
		break;
	}

	if(playerinfo->isclient)
	{
		playerinfo->CL_Sound(SND_PRED_ID16,
							 playerinfo->origin,
							 CHAN_AUTO,
							 name,
							 0.6,
							 ATTN_NORM,
							 0);
	}
	else
	{
  		playerinfo->G_Sound(SND_PRED_ID16,
							playerinfo->leveltime,
							playerinfo->self,
							CHAN_AUTO,
							playerinfo->G_SoundIndex(name),
							0.6,
							ATTN_NORM,
							0);
	}
}

/*-----------------------------------------------
	PlayerActionSwimBackSound
-----------------------------------------------*/

void PlayerActionSwimBackSound(playerinfo_t *playerinfo, float value)
{
	if(playerinfo->isclient)
	{
		playerinfo->CL_Sound(SND_PRED_ID17,
							 playerinfo->origin,
							 CHAN_VOICE,
							 "swim backward.wav",
							 0.6,
							 ATTN_NORM,
							 0);
	}
	else
	{
		playerinfo->G_Sound(SND_PRED_ID17,
							playerinfo->leveltime,
							playerinfo->self,
							CHAN_VOICE,
							playerinfo->G_SoundIndex("swim backward.wav"),
							0.6,
							ATTN_NORM,
							0);
	}
}

/*-----------------------------------------------
	PlayerActionClimbWallSound
-----------------------------------------------*/

void PlayerActionClimbWallSound(playerinfo_t *playerinfo, float value)
{
	if(playerinfo->isclient)
	{
		playerinfo->CL_Sound(SND_PRED_ID18,
							 playerinfo->origin,
							 CHAN_VOICE,
							 "player/pullup 1.wav",
							 1.0,
							 ATTN_NORM,
							 0);
	}
	else
	{
		playerinfo->G_Sound(SND_PRED_ID18,
							playerinfo->leveltime,
							playerinfo->self,
							CHAN_VOICE,
							playerinfo->G_SoundIndex("player/pullup 1.wav"),
							1.0,
							ATTN_NORM,
							0);
	}
}

/*-----------------------------------------------
	PlayerActionClimbFinishSound
-----------------------------------------------*/

void PlayerActionClimbFinishSound(playerinfo_t *playerinfo, float value)
{
	if(playerinfo->isclient)
	{
		playerinfo->CL_Sound(SND_PRED_ID19,
							 playerinfo->origin,
							 CHAN_VOICE,
							 "player/pullup 2.wav",
							 1.0,
							 ATTN_NORM,
							 0);
	}
	else
	{
		playerinfo->G_Sound(SND_PRED_ID19,
							playerinfo->leveltime,
							playerinfo->self,
							CHAN_VOICE,
							playerinfo->G_SoundIndex("player/pullup 2.wav"),
							1.0,
							ATTN_NORM,
							0);
	}
}

/*-----------------------------------------------
	PlayerActionSwim
-----------------------------------------------*/

void PlayerActionSwim(playerinfo_t *playerinfo, float value)
{
	vec3_t	Origin,
			Dir;

	if(value==1.0)
	{
		VectorCopy(playerinfo->origin,Origin);
		Origin[2]+=playerinfo->waterheight;

		// Fixme: Need to determine the actual water surface normal - if we have any sloping water?

		Dir[0]=0.0;
		Dir[1]=0.0;
		Dir[2]=1.0;

		//

		if(!playerinfo->isclient)
			playerinfo->G_CreateEffect(EFFECT_PRED_ID9,
									   NULL,
									   FX_WATER_ENTRYSPLASH,
									   0,
									   Origin,
									   "bd",
									   32,
									   Dir);
		else
			playerinfo->CL_CreateEffect(EFFECT_PRED_ID9,
										NULL,
										FX_WATER_ENTRYSPLASH,
										0,
										Origin,
										"bd",
										32,
										Dir);
	}
}

/*-----------------------------------------------
	PlayerActionPullup
-----------------------------------------------*/

void PlayerActionPullup(playerinfo_t *playerinfo, float value)
{
}

/*-----------------------------------------------
	PlayerActionSkidSmoke
-----------------------------------------------*/

void PlayerActionSkidSmoke(playerinfo_t *playerinfo, float value)
{
}

#define	MIN_STEP_NORMAL		0.7		// can't step up onto very steep slopes
#define GRAB_HAND_HEIGHT	41
#define GRAB_HAND_WIDTH		8
#define GRAB_HAND_HORZONE	22
#define GRAB_HAND_VERTZONE	15
#define HOLD_HAND_HEIGHT	41

/*-----------------------------------------------
	PlayerActionCheckGrab_
-----------------------------------------------*/

int PlayerActionCheckGrab_(playerinfo_t *playerinfo, float v_adjust)
{
	qboolean	swingable = false;
	trace_t grabtrace;
	trace_t lasttrace, swingtrace;
	vec3_t	mins, maxs;
	vec3_t	planedir;
	vec3_t	forward, right;
	vec3_t	playermin, playermax;
	vec3_t	righthand, lefthand, endpoint;
	vec3_t	player_facing;
	vec3_t	lastcheck_start, lastcheck_end;
	float	vertlength;
	float	yaw;

	assert(playerinfo);

	//	Criteria:
	//	--The wall should be vertical (we already know that any surface collided with is vertical.
	//	--It needs a horizontal (or nearly horizontal) top edge.
	//	--The top edge of the plane must be lower than the "grab" height of the player's arms at the time of impact.
	//	--The plane adjacent to the top edge of the collided-with plane must also be nearly horizontal.
	//	--The player's angle must be almost straight towards the collided-with plane.
	//	--The plane must be at least wide enough at the point of intersection for the whole player.

	// Now we want to cast some rays.  If the two rays moving from the player's hands at "grab"
	// width successfully clear any surface, then at least his hands are free enough to make the
	// grab.

	VectorCopy(playerinfo->angles,player_facing);
	player_facing[PITCH]=player_facing[ROLL]=0;
	AngleVectors(player_facing, forward, right, NULL);
	VectorMA(playerinfo->origin, GRAB_HAND_WIDTH, right, righthand);
	righthand[2] += v_adjust;
	VectorMA(righthand, GRAB_HAND_HORZONE, forward, endpoint);

	if(playerinfo->isclient)
		playerinfo->CL_Trace(righthand,handmins,handmaxs,endpoint,MASK_PLAYERSOLID,CEF_CLIP_TO_WORLD,&grabtrace);
	else
		playerinfo->G_Trace(righthand,handmins,handmaxs,endpoint,playerinfo->self,MASK_PLAYERSOLID,&grabtrace);

	VectorCopy(grabtrace.endpos, righthand);

	if (grabtrace.fraction != 1.0)
	{	
		// Right hand is not clear.

		return(false);
	}

	if (grabtrace.startsolid || grabtrace.allsolid)
	{	
		// Right hand is not clear.
		return(false);
	}

	VectorMA(playerinfo->origin, -GRAB_HAND_WIDTH, right, lefthand);
	lefthand[2] += v_adjust;
	VectorMA(lefthand, GRAB_HAND_HORZONE, forward, endpoint);

	if(playerinfo->isclient)
		playerinfo->CL_Trace(lefthand,handmins,handmaxs,endpoint,MASK_PLAYERSOLID,CEF_CLIP_TO_WORLD,&grabtrace);
	else
		playerinfo->G_Trace(lefthand,handmins,handmaxs,endpoint,playerinfo->self,MASK_PLAYERSOLID,&grabtrace);

	VectorCopy(grabtrace.endpos, lefthand);			
	
	if (grabtrace.fraction != 1.0)
	{	// Left hand is not clear.
		return(false);
	}

	if (grabtrace.startsolid || grabtrace.allsolid)
	{	// Left hand is not clear.
		return(false);
	}
	// If the clear rays from the player's hands, traced down, should hit a legal (almost level)
	// surface within a certain distance, then a grab is possible!

	// First we must figure out how far down to look.

	if (playerinfo->velocity[2] < 0)
	{	
		// If the player is going down, then check his intended speed over the next .1 sec.

		vertlength = (playerinfo->sv_gravity*.5-playerinfo->velocity[2])*.1;
	}
	else
	{	
		// If the player is going up, then check his velocity over the LAST .1 sec.

		vertlength = (playerinfo->sv_gravity*.5+playerinfo->velocity[2])*.1;
	}
	
	vertlength=Q_fabs(vertlength);
	
	if (vertlength < GRAB_HAND_VERTZONE)
	{
		vertlength = GRAB_HAND_VERTZONE;
	}

	VectorCopy(righthand, endpoint);
	endpoint[2] -= vertlength;
	
	if(playerinfo->isclient)
		playerinfo->CL_Trace(righthand,handmins,handmaxs,endpoint,MASK_PLAYERSOLID,CEF_CLIP_TO_WORLD,&grabtrace);
	else
		playerinfo->G_Trace(righthand,handmins,handmaxs,endpoint,playerinfo->self,MASK_PLAYERSOLID,&grabtrace);

	VectorCopy(grabtrace.endpos, righthand);

	if (grabtrace.fraction == 1.0)
	{	
		// Right hand did not connect with a flat surface.
		return(false);
	}

	if (grabtrace.startsolid || grabtrace.allsolid)
	{	
		// Right hand did not connect with a flat surface.
		return(false);
	}

	if (!(grabtrace.contents & MASK_SOLID))
	{	// hand stopped, but not on a grabbable surface.
		return(false);
	}

	VectorCopy(lefthand, endpoint);
	endpoint[2] -= vertlength;
	
	if(playerinfo->isclient)
		playerinfo->CL_Trace(lefthand,handmins,handmaxs,endpoint,MASK_PLAYERSOLID,CEF_CLIP_TO_WORLD,&grabtrace);
	else
		playerinfo->G_Trace(lefthand,handmins,handmaxs,endpoint,playerinfo->self,MASK_PLAYERSOLID,&grabtrace);

	VectorCopy(grabtrace.endpos, lefthand);

	if (grabtrace.fraction == 1.0 || grabtrace.plane.normal[2] < .8 || grabtrace.startsolid || grabtrace.allsolid)
	{	
		// Left hand did not connect with a flat surface.
		return(false);
	}

	if (grabtrace.startsolid || grabtrace.allsolid)
	{	
		// Left hand did not connect with a flat surface.
		return(false);
	}

	if (!(grabtrace.contents & MASK_SOLID))
	{	// hand stopped, but not on a grabbable surface.
		return(false);
	}

	// Now finally, if we try tracing the player blocking forward a tad, we should be hitting an
	// obstruction.

	VectorCopy(playerinfo->mins, playermin);
	VectorCopy(playerinfo->maxs, playermax);
	
	// We need to take the player limits and extend them up to 83 in height.

	playermax[2] = GRAB_HAND_HEIGHT;
	VectorMA(playerinfo->origin, GRAB_HAND_HORZONE, forward, endpoint);

	if(playerinfo->isclient)
		playerinfo->CL_Trace(playerinfo->origin,playermin,playermax,endpoint,MASK_PLAYERSOLID,CEF_CLIP_TO_WORLD,&grabtrace);
	else
		playerinfo->G_Trace(playerinfo->origin,playermin,playermax,endpoint,playerinfo->self,MASK_PLAYERSOLID,&grabtrace);

	if (grabtrace.fraction == 1)
		return(false);

	if (grabtrace.startsolid || grabtrace.allsolid)
		return(false);

	if (!(grabtrace.contents & MASK_SOLID))
	{	// hand stopped, but not on a grabbable surface.
		return(false);
	}

	// Sloped away surfaces are not grabbable.

	if (grabtrace.plane.normal[2] > 0)
		return(false);

	if (grabtrace.ent&&!playerinfo->isclient)
	{
		if(playerinfo->G_EntIsAButton(grabtrace.ent))
		{
			return(false);
		}
	}

	// Now check the angle.  It should be pretty much opposite the player's yaw.

	vectoangles(grabtrace.plane.normal, planedir);
	playerinfo->grabangle = planedir[YAW] - 180.0;
	playerinfo->grabangle = anglemod(playerinfo->grabangle);
	yaw = planedir[YAW] - playerinfo->angles[YAW];
	yaw = anglemod(yaw) - 180.0;

	if (yaw > 30.0 || yaw < -30.0)
	{	
		// Bad angle.  Player should bounce.

		return(false);
	}
	

	//one more check, make sure we can fit in there! -- 
	//so we don't start climbing then fall back down- annoying
	//get the z height
	VectorCopy(playerinfo->origin, lastcheck_start);
	if(lefthand[2] > righthand[2])
		lastcheck_start[2] = lefthand[2] - playerinfo->mins[2];
	else
		lastcheck_start[2] = righthand[2] - playerinfo->mins[2];

	VectorMA(lastcheck_start, 1, forward, lastcheck_end);

	//HEY- should the other checks above check against PLAYERSOLID too?  to include clip brushes?
	if(playerinfo->isclient)
		playerinfo->CL_Trace(lastcheck_start, playerinfo->mins, playerinfo->maxs, lastcheck_end, MASK_PLAYERSOLID, CEF_CLIP_TO_WORLD, &lasttrace);
	else
		playerinfo->G_Trace(lastcheck_start, playerinfo->mins, playerinfo->maxs, lastcheck_end, playerinfo->self, MASK_PLAYERSOLID,&lasttrace);

	if(lasttrace.fraction < 1.0 || lasttrace.startsolid || lasttrace.allsolid)
		return (false);

	//Now see if the surface is eligible for a overhanging swing vault
	//Trace from about the player's waist to his feet to determine this
	VectorCopy(playerinfo->mins, mins);
	VectorCopy(playerinfo->maxs, maxs);
	maxs[2] -= 48;

	AngleVectors(playerinfo->angles, forward, NULL, NULL);
	VectorMA(playerinfo->origin, 32, forward, endpoint);

	if (playerinfo->isclient)
		playerinfo->CL_Trace(playerinfo->origin,mins,maxs,endpoint,MASK_PLAYERSOLID,CEF_CLIP_TO_WORLD,&swingtrace);
	else
		playerinfo->G_Trace(playerinfo->origin,mins,maxs,endpoint,playerinfo->self,MASK_PLAYERSOLID,&swingtrace);
	
	//Did we hit a wall underneath?
	if (swingtrace.fraction == 1.0f && (!swingtrace.startsolid || !swingtrace.allsolid))
		swingable = true;

	// Save the intended grab location (the endpoint).

	playerinfo->grabloc[0] = ((lefthand[0] + righthand[0]) / 2.0);
	playerinfo->grabloc[1] = ((lefthand[1] + righthand[1]) / 2.0);
	playerinfo->grabloc[2] = ((lefthand[2] + righthand[2]) / 2.0);

	endpoint[0] = grabtrace.endpos[0];
	endpoint[1] = grabtrace.endpos[1];
	endpoint[2] = playerinfo->grabloc[2] - v_adjust;

	if(playerinfo->isclient)
		playerinfo->CL_Trace(playerinfo->origin,
							 NULL,
							 NULL,
							 endpoint,
							 MASK_PLAYERSOLID,
							 CEF_CLIP_TO_WORLD,
							 &grabtrace);
	else
		playerinfo->G_Trace(playerinfo->origin,
									  NULL,
									  NULL,
									  endpoint,
									  playerinfo->self,
									  MASK_PLAYERSOLID,&grabtrace);

	if (grabtrace.fraction == 1.0)
	{
		VectorCopy(endpoint, playerinfo->origin);		
		playerinfo->offsetangles[YAW]=-((CL_NormaliseAngle(playerinfo->angles[YAW]))-playerinfo->grabangle);
		playerinfo->angles[YAW] = playerinfo->grabangle;
		
		if (swingable)
			return 2;

		return true;
	}

	return(false);
}

/*-----------------------------------------------
	PlayerActionCheckGrab
-----------------------------------------------*/

void PlayerActionCheckGrab(playerinfo_t *playerinfo, float value)
{
	int		i, type, maxcheck;
	float	v_adjust;

	if (!playerinfo->upperidle)
		return;

	if (!(playerinfo->flags & PLAYER_FLAG_NO_LARM) && !(playerinfo->flags & PLAYER_FLAG_NO_RARM))
		maxcheck = 3;//all checks ok
	else if ( ((playerinfo->flags & PLAYER_FLAG_NO_LARM) && (playerinfo->flags & PLAYER_FLAG_NO_RARM)) || (!playerinfo->upperidle) )
		maxcheck = 1;//only check ankle height
	else
		maxcheck = 2;//up to waist height

	for(i=0;i<maxcheck;i++)
	{
		// Check 3 height zones for 3 results.
		
		switch(i)
		{
			case 0:
				v_adjust = GRAB_HAND_HEIGHT - 58;
				
				break;
			
			case 1:
				v_adjust = GRAB_HAND_HEIGHT - 29;

				break;
			
			case 2:
				v_adjust = GRAB_HAND_HEIGHT;
				
				break;
			
			default:
				v_adjust = GRAB_HAND_HEIGHT;
				
				break;
		}

		if((type=PlayerActionCheckGrab_(playerinfo,v_adjust)))
		{
			switch(i)
			{
				case 2:
					if (type==2)
						PlayerAnimSetVault(playerinfo, ASEQ_OVERHANG);
					else
						PlayerAnimSetVault(playerinfo, ASEQ_PULLUP_WALL);
					
					return;

					break;

				case 0:
				case 1:
					PlayerAnimSetVault(playerinfo, ASEQ_PULLUP_HALFWALL);//
					return;

					break;

				default:
					
					break;
			}
		}
	}
}

void PlayerActionCheckFallingGrab(playerinfo_t *playerinfo, float value)
{
	if (!playerinfo->seqcmd[ACMDL_FWD] && !playerinfo->seqcmd[ACMDL_JUMP] && !playerinfo->seqcmd[ACMDL_ACTION])
		return;

	PlayerActionCheckGrab(playerinfo, value);
}

/*-----------------------------------------------
	PlayerActionBowReadySound
-----------------------------------------------*/

void PlayerActionBowReadySound(playerinfo_t *playerinfo, float value)
{
	if(playerinfo->isclient)
	{
		playerinfo->CL_Sound(SND_PRED_ID20,
							 playerinfo->origin,
							 CHAN_WEAPON,
							 "weapons/bowdraw2.Wav",
							 1.0,
							 ATTN_NORM,
							 0);
	}
	else
	{
		playerinfo->G_Sound(SND_PRED_ID20,
							playerinfo->leveltime,
							playerinfo->self,
							CHAN_WEAPON,
							playerinfo->G_SoundIndex("weapons/bowdraw2.Wav"),
							1.0,
							ATTN_NORM,
							0);
	}
}

/*-----------------------------------------------
	PlayerActionUsePuzzle
-----------------------------------------------*/

qboolean PlayerActionUsePuzzle(playerinfo_t *playerinfo)
{
	if(!playerinfo->isclient)
	{	
		return(playerinfo->G_PlayerActionUsePuzzle(playerinfo));
	}
	else
	{
		// Client does nothing.

		return(false);
	}
}

/*-----------------------------------------------
	PlayerActionCheckPuzzleGrab
-----------------------------------------------*/

qboolean PlayerActionCheckPuzzleGrab(playerinfo_t *playerinfo)
{
	if(!playerinfo->isclient)
	{
		return(playerinfo->G_PlayerActionCheckPuzzleGrab(playerinfo));
	}
	else
	{
		// Client does nothing.

		return(false);
	}
}

/*-----------------------------------------------
	PlayerActionCheckJumpGrab
-----------------------------------------------*/

#define GRAB_JUMP_HORZONE	32
#define GRAB_JUMP_HEIGHT	80

qboolean PlayerActionCheckJumpGrab(playerinfo_t *playerinfo, float value)
{//FIXME: make like others
	vec3_t planedir;
	vec3_t forward, right;
	vec3_t playermin, playermax;
	trace_t grabtrace;
	vec3_t righthand, lefthand, endpoint;
	vec3_t player_facing;
	float yaw, handheight;

	assert(playerinfo);

	// Check for grabbability.
	// Criteria:
	// --The wall should be vertical (we already know that any surface collided with is vertical.
	// --It needs a horizontal (or nearly horizontal) top edge.
	// --The top edge of the plane must be lower than the "grab" height of the player's arms at the time of impact.
	// --The plane adjacent to the top edge of the collided-with plane must also be nearly horizontal.
	// --The player's angle must be almost straight towards the collided-with plane.
	// --The plane must be at least wide enough at the point of intersection for the whole player.

	// Skip it if we're not on the ground.
	
	if (playerinfo->groundentity == NULL)
		return(false);

	// First we need to "move" the player as high as we possibly can.

	VectorCopy(playerinfo->origin, endpoint);
	endpoint[2] += GRAB_JUMP_HEIGHT;
	VectorCopy(playerinfo->mins, playermin);
	VectorCopy(playerinfo->maxs, playermax);
	
	// We need to take the player limits and extend them up to 83 in height (where the hands are).

	playermax[2] = GRAB_HAND_HEIGHT;
	
	if(playerinfo->isclient)
		playerinfo->CL_Trace(playerinfo->origin,playermin,playermax,endpoint,MASK_PLAYERSOLID,CEF_CLIP_TO_WORLD,&grabtrace);
	else			
		playerinfo->G_Trace(playerinfo->origin,playermin,playermax,endpoint,playerinfo->self,MASK_PLAYERSOLID,&grabtrace);

	if (grabtrace.startsolid || grabtrace.allsolid)
	{	// There's no room to jump.
		return(false);
	}

	// Handheight is set to the maximum the hands can go above the current player's height.
	
	handheight = grabtrace.endpos[2] + GRAB_HAND_HEIGHT;

	// Now we want to cast some rays.  If the two rays moving from the player's hands at "grab"
	// width successfully clear any surface, then at least his hands are free enough to make the
	// grab.

	VectorCopy(playerinfo->angles,player_facing);
	player_facing[PITCH]=player_facing[ROLL]=0;
	AngleVectors(player_facing, forward, right, NULL);
	VectorMA(playerinfo->origin, GRAB_HAND_WIDTH, right, righthand);
	righthand[2] = handheight;
	VectorMA(righthand, GRAB_JUMP_HORZONE, forward, endpoint);

	if(playerinfo->isclient)
		playerinfo->CL_Trace(righthand,handmins,handmaxs,endpoint,MASK_PLAYERSOLID,CEF_CLIP_TO_WORLD,&grabtrace);
	else
		playerinfo->G_Trace(righthand,handmins,handmaxs,endpoint,playerinfo->self,MASK_PLAYERSOLID,&grabtrace);

	VectorCopy(grabtrace.endpos, righthand);
	if (grabtrace.fraction != 1.0)
	{	// Right hand is not clear.
		return false;
	}

	VectorMA(playerinfo->origin, -GRAB_HAND_WIDTH, right, lefthand);
	lefthand[2] = handheight;
	VectorMA(lefthand, GRAB_JUMP_HORZONE, forward, endpoint);
	
	if(playerinfo->isclient)
		playerinfo->CL_Trace(lefthand,handmins,handmaxs,endpoint,MASK_PLAYERSOLID,CEF_CLIP_TO_WORLD,&grabtrace);
	else
		playerinfo->G_Trace(lefthand,handmins,handmaxs,endpoint,playerinfo->self,MASK_PLAYERSOLID,&grabtrace);

	VectorCopy(grabtrace.endpos, lefthand);			
	if (grabtrace.fraction != 1.0)
	{	// Left hand is not clear.
		return false;
	}


	// If the clear rays from the player's hands, traced down, should hit a legal (almost level)
	// surface within a certain distance, then a grab is possible!

	VectorCopy(righthand, endpoint);
	endpoint[2] = playerinfo->origin[2] + GRAB_HAND_HEIGHT;
	
	if(playerinfo->isclient)
		playerinfo->CL_Trace(righthand,handmins,handmaxs,endpoint,MASK_PLAYERSOLID,CEF_CLIP_TO_WORLD,&grabtrace);
	else
		playerinfo->G_Trace(righthand,handmins,handmaxs,endpoint,playerinfo->self,MASK_PLAYERSOLID,&grabtrace);

	VectorCopy(grabtrace.endpos, righthand);
	if (grabtrace.fraction == 1.0 || grabtrace.plane.normal[2] < .5)
	{	// Right hand did not connect with a flat surface.
		return false;
	}

	if (!(grabtrace.contents & MASK_SOLID))
	{	// hand stopped, but not on a grabbable surface.
		return(false);
	}

	VectorCopy(lefthand, endpoint);
	endpoint[2] = playerinfo->origin[2] + GRAB_HAND_HEIGHT;
	
	if(playerinfo->isclient)
		playerinfo->CL_Trace(lefthand,handmins,handmaxs,endpoint,MASK_PLAYERSOLID,CEF_CLIP_TO_WORLD,&grabtrace);
	else
		playerinfo->G_Trace(lefthand,handmins,handmaxs,endpoint,playerinfo->self,MASK_PLAYERSOLID,&grabtrace);

	VectorCopy(grabtrace.endpos, lefthand);
	if (grabtrace.fraction == 1.0 || grabtrace.plane.normal[2] < .5)
	{	// Left hand did not connect with a flat surface.
		return false;
	}

	if (!(grabtrace.contents & MASK_SOLID))
	{	// hand stopped, but not on a grabbable surface.
		return(false);
	}


	// Now, finally, if we try tracing the player blocking forward a tad, we should be hitting
	// an obstruction.
	VectorCopy(playerinfo->mins, playermin);
	VectorCopy(playerinfo->maxs, playermax);
	
	// We need to take the player limits and extend them up to 83 in height.

	playermax[2] = GRAB_HAND_HEIGHT;

	VectorMA(playerinfo->origin, GRAB_JUMP_HORZONE, forward, endpoint);
	
	if(playerinfo->isclient)
		playerinfo->CL_Trace(playerinfo->origin,NULL,NULL,endpoint,MASK_PLAYERSOLID,CEF_CLIP_TO_WORLD,&grabtrace);
	else		
		playerinfo->G_Trace(playerinfo->origin,NULL,NULL,endpoint,playerinfo->self,MASK_PLAYERSOLID,&grabtrace);

	// Now, if the player is grabbing an overhang, this will not hit anything.

	if (grabtrace.fraction >= 1.0)
	{	
		// Hit nothing, so do some more checks, by stretching the player up to the outcropping.
		
		// Bottom at the origin
		
		playermin[2] = 0.0;

		// Stretch all the way up to where the grab is made.

		playermax[2] = (lefthand[2]+righthand[2])*0.5 - playerinfo->origin[2];

		if(playerinfo->isclient)
			playerinfo->CL_Trace(playerinfo->origin,playermin,playermax,endpoint,MASK_PLAYERSOLID,CEF_CLIP_TO_WORLD,&grabtrace);
		else
			playerinfo->G_Trace(playerinfo->origin,playermin,playermax,endpoint,playerinfo->self,MASK_PLAYERSOLID,&grabtrace);

		if (grabtrace.fraction >= 1.0)
		{
			return false;
		}
	}

	// Sloped away surfaces are not grabbable.

	if (grabtrace.startsolid || grabtrace.plane.normal[2] > 0)
	{
		return false;
	}

	if (!(grabtrace.contents & MASK_SOLID))
	{	// player stopped, but not on a grabbable surface.
		return(false);
	}

	// Now check the angle.  It should be pretty much opposite the player's yaw.

	vectoangles(grabtrace.plane.normal, planedir);

	playerinfo->grabangle = planedir[YAW] - 180.0;
	playerinfo->grabangle = anglemod(playerinfo->grabangle);
	yaw = planedir[YAW] - playerinfo->angles[YAW];
	yaw = anglemod(yaw) - 180.0;
	if (yaw > 30.0 || yaw < -30.0)
	{	
		// Bad angle. Player should bounce.

		return false;
	}

	// Now that we feel we have a viable jump, let's jump!
	
	return(true);
}

/*-----------------------------------------------
	PlayerActionCheckPushPull
-----------------------------------------------*/

#define PUSH_HAND_WIDTH	16
#define PUSH_HAND_HEIGHT	0
#define PUSH_HAND_HORZONE	48
#define VAULT_HAND_VERTZONE 48
#define VAULT_HEIGHT_CHECK	(-16)

qboolean PlayerActionCheckPushPull(playerinfo_t *playerinfo)
{
	if(!playerinfo->isclient)
	{
		vec3_t player_facing;
		vec3_t forward, right;
		vec3_t righthand, lefthand, endpoint;
		trace_t grabtrace;
		vec3_t planedir;
		float yaw;
		void *holdent;

		assert(playerinfo);

 		VectorCopy(playerinfo->angles,player_facing);
		player_facing[PITCH]=player_facing[ROLL]=0;
		AngleVectors(player_facing, forward, right, NULL);

		VectorMA(playerinfo->origin, PUSH_HAND_WIDTH, right, righthand);
		righthand[2] += PUSH_HAND_HEIGHT;
		VectorMA(righthand, PUSH_HAND_HORZONE, forward, endpoint);
		
		playerinfo->G_Trace(righthand,handmins,handmaxs,endpoint,playerinfo->self,MASK_PLAYERSOLID,&grabtrace);

		if ((grabtrace.fraction == 1) || (!grabtrace.ent))
			return false;

		if(!(playerinfo->G_PlayerActionCheckPushPull_Ent((void*)grabtrace.ent)))
			return(false);

		holdent = (void *)grabtrace.ent;

		VectorMA(playerinfo->origin, -PUSH_HAND_WIDTH, right, lefthand);
		lefthand[2] += PUSH_HAND_HEIGHT;
		VectorMA(lefthand, PUSH_HAND_HORZONE, forward, endpoint);

		playerinfo->G_Trace(lefthand,handmins,handmaxs,endpoint,playerinfo->self,MASK_PLAYERSOLID,&grabtrace);

		VectorCopy(grabtrace.endpos, lefthand);			

		if ((grabtrace.fraction == 1.0) ||  (grabtrace.ent != holdent))
		{	
			// Left hand is not near to pushable object.

			return(false);
		}

		// Parallel to each other?

		vectoangles(grabtrace.plane.normal, planedir);
		yaw = planedir[YAW] - playerinfo->angles[YAW];
		yaw = anglemod(yaw) - 180.0;
		if (yaw > 30.0 || yaw < -30.0)
		{	// 
			return false;
		}

     	playerinfo->target_ent = grabtrace.ent;

		return (true);
	}
	else
	{
		// Client does nothing.

		return(false);
	}
}

/*-----------------------------------------------
	PlayerActionCheckVault
-----------------------------------------------*/

#define VAULT_HAND_WIDTH	4
#define VAULT_HAND_HEIGHT	32
#define VAULT_HAND_VERTZONE 48
#define VAULT_HAND_HORZONE	24
#define VAULT_HEIGHT_CHECK	(-16)

qboolean PlayerActionCheckVault(playerinfo_t *playerinfo, float value)
{
	qboolean	swingable = false;
	trace_t grabtrace;
	trace_t lasttrace;
	trace_t swingtrace;
	vec3_t	mins, maxs, vf;
	vec3_t	forward, right;
	vec3_t	start, end;
	vec3_t	grabloc, planedir;
	vec3_t	righthand, lefthand, endpoint;
	vec3_t	player_facing;
	vec3_t	vaultcheckmins, vaultcheckmaxs;
	vec3_t	lastcheck_start, lastcheck_end;
	float yaw, grabfraction;

	assert(playerinfo);

	// Check in front of the player, and decide if there is a suitable wall here.

	VectorCopy(playerinfo->angles,player_facing);
	player_facing[PITCH]=player_facing[ROLL]=0;
	AngleVectors(player_facing, forward, right, NULL);

	VectorCopy(playerinfo->mins,vaultcheckmins);
	VectorCopy(playerinfo->maxs,vaultcheckmaxs);
	vaultcheckmins[2]+=18;//don't try to vault stairs, man

	VectorCopy(playerinfo->origin, start);
//	start[2] += VAULT_HEIGHT_CHECK;//waist - 16
	VectorMA(start, VAULT_HAND_HORZONE, forward, end);

	if(playerinfo->isclient)
		playerinfo->CL_Trace(start,vaultcheckmins,vaultcheckmaxs,end,MASK_PLAYERSOLID,CEF_CLIP_TO_WORLD,&grabtrace);
	else
		playerinfo->G_Trace(start,vaultcheckmins,vaultcheckmaxs,end,playerinfo->self,MASK_PLAYERSOLID,&grabtrace);

	if (grabtrace.fraction == 1.0)
	{	
		// Nothing in front of us.
		return(false);
	}

	if (!(grabtrace.contents & MASK_SOLID))
	{	// body stopped, but not on a grabbable surface.
		return(false);
	}

	// Sloped surfaces are not grabbable. Question: sloped away or towards?
	
	if (grabtrace.plane.normal[2] > .3)
	{
		return(false);
	}

	if(grabtrace.ent&&!playerinfo->isclient)
	{
		if(playerinfo->G_EntIsAButton(grabtrace.ent))
		{
			return(false);
		}
	}
	// Now check the angle.  It should be pretty much opposite the player's yaw.

	vectoangles(grabtrace.plane.normal, planedir);
	playerinfo->grabangle = planedir[YAW] - 180.0;
	playerinfo->grabangle = anglemod(playerinfo->grabangle);
	yaw = planedir[YAW] - playerinfo->angles[YAW];
	yaw = anglemod(yaw) - 180.0;
	if (yaw > 30.0 || yaw < -30.0)
	{	
		// Bad angle. Player should bounce.
		return(false);
	}

	VectorCopy(grabtrace.endpos, grabloc);

	// Now we want to cast some rays.  If the two rays moving from the player's hands at "grab"
	// width successfully clear any surface, then at least his hands are free enough to make the
	// grab.

	VectorMA(playerinfo->origin, VAULT_HAND_WIDTH, right, righthand);
	righthand[2] += VAULT_HAND_HEIGHT;
	VectorMA(righthand, VAULT_HAND_HORZONE, forward, endpoint);
	
	if(playerinfo->isclient)
		playerinfo->CL_Trace(righthand,handmins,handmaxs,endpoint,MASK_PLAYERSOLID,CEF_CLIP_TO_WORLD,&grabtrace);
	else
		playerinfo->G_Trace(righthand,handmins,handmaxs,endpoint,playerinfo->self,MASK_PLAYERSOLID,&grabtrace);

	VectorCopy(grabtrace.endpos, righthand);
	if (grabtrace.fraction != 1.0 || grabtrace.startsolid || grabtrace.allsolid)
	{	
		// Right hand is not clear.
		return(false);
	}

	VectorMA(playerinfo->origin, -VAULT_HAND_WIDTH, right, lefthand);
	lefthand[2] += VAULT_HAND_HEIGHT;
	VectorMA(lefthand, VAULT_HAND_HORZONE, forward, endpoint);

	if(playerinfo->isclient)
		playerinfo->CL_Trace(lefthand,handmins,handmaxs,endpoint,MASK_PLAYERSOLID,CEF_CLIP_TO_WORLD,&grabtrace);
	else
		playerinfo->G_Trace(lefthand,handmins,handmaxs,endpoint,playerinfo->self,MASK_PLAYERSOLID,&grabtrace);

	VectorCopy(grabtrace.endpos, lefthand);			
	if (grabtrace.fraction != 1.0 || grabtrace.startsolid || grabtrace.allsolid)
	{	
		// Left hand is not clear.
		return(false);
	}

	// If the clear rays from the player's hands, traced down, should hit a legal (almost level)
	// surface within a certain distance, then a grab is possible! First we must figure out how low
	// to look. If the player is going down, then check his intended speed over the next .1 sec.

	VectorCopy(righthand, endpoint);
	endpoint[2] -= VAULT_HAND_VERTZONE;
	
	if(playerinfo->isclient)
		playerinfo->CL_Trace(righthand,handmins,handmaxs,endpoint,MASK_PLAYERSOLID,CEF_CLIP_TO_WORLD,&grabtrace);
	else
		playerinfo->G_Trace(righthand,handmins,handmaxs,endpoint,playerinfo->self,MASK_PLAYERSOLID,&grabtrace);

	VectorCopy(grabtrace.endpos, righthand);
	if (grabtrace.fraction == 1.0 || grabtrace.plane.normal[2] < .8 || grabtrace.startsolid || grabtrace.allsolid)
	{	
		// Right hand did not connect with a flat surface.
		return(false);
	}

	if (!(grabtrace.contents & MASK_SOLID))
	{	// hand stopped, but not on a grabbable surface.
		return(false);
	}

	VectorCopy(lefthand, endpoint);
	endpoint[2] -= VAULT_HAND_VERTZONE;

	if(playerinfo->isclient)
		playerinfo->CL_Trace(lefthand,handmins,handmaxs,endpoint,MASK_PLAYERSOLID,CEF_CLIP_TO_WORLD,&grabtrace);
	else
		playerinfo->G_Trace(lefthand,handmins,handmaxs,endpoint,playerinfo->self,MASK_PLAYERSOLID,&grabtrace);

	VectorCopy(grabtrace.endpos, lefthand);
	if (grabtrace.fraction == 1.0 || grabtrace.plane.normal[2] < .8 || grabtrace.startsolid || grabtrace.allsolid)
	{	
		// Left hand did not connect with a flat surface.
		return(false);
	}

	if (!(grabtrace.contents & MASK_SOLID))
	{	// hand stopped, but not on a grabbable surface.
		return(false);
	}

	// The fit check doesn't work when you are in muck, so check if you are.
	if (!(playerinfo->watertype & (CONTENTS_SLIME|CONTENTS_LAVA)))
	{
		//one more check, make sure we can fit in there! -- 
		//so we don't start climbing then fall back down- annoying
		//get the z height
		VectorCopy(playerinfo->origin, lastcheck_start);
		if(lefthand[2] > righthand[2])
			lastcheck_start[2] = lefthand[2] - playerinfo->mins[2];
		else
			lastcheck_start[2] = righthand[2] - playerinfo->mins[2];

		VectorMA(lastcheck_start, 1, forward, lastcheck_end);

		//HEY- should the other checks above check against PLAYERSOLID too?  to include clip brushes?
		if(playerinfo->isclient)
			playerinfo->CL_Trace(lastcheck_start, playerinfo->mins, playerinfo->maxs, lastcheck_end, MASK_PLAYERSOLID, CEF_CLIP_TO_WORLD, &lasttrace);
		else
			playerinfo->G_Trace(lastcheck_start, playerinfo->mins, playerinfo->maxs, lastcheck_end, playerinfo->self, MASK_PLAYERSOLID,&lasttrace);

		if(lasttrace.fraction < 1.0 || lasttrace.startsolid || lasttrace.allsolid)
		{
			return (false);
		}
	}

	
	grabfraction = grabtrace.fraction;

	//Now see if the surface is eligible for a overhanging swing vault
	//Trace from about the player's waist to his feet to determine this
	VectorCopy(playerinfo->mins, mins);
	VectorCopy(playerinfo->maxs, maxs);
	maxs[2] -= 48;

	AngleVectors(playerinfo->angles, vf, NULL, NULL);
	VectorMA(playerinfo->origin, 32, vf, endpoint);

	if (playerinfo->isclient)
		playerinfo->CL_Trace(playerinfo->origin,mins,maxs,endpoint,MASK_PLAYERSOLID,CEF_CLIP_TO_WORLD,&swingtrace);
	else
		playerinfo->G_Trace(playerinfo->origin,mins,maxs,endpoint,playerinfo->self,MASK_PLAYERSOLID,&swingtrace);
	
	//Did we hit a wall underneath?
	if (swingtrace.fraction == 1.0f && (!swingtrace.startsolid || !swingtrace.allsolid))
		swingable = true;
	
	// Save the intended grab location (the endpoint).
	playerinfo->grabloc[0] = ((lefthand[0] + righthand[0]) / 2.0);
	playerinfo->grabloc[1] = ((lefthand[1] + righthand[1]) / 2.0);
	
	if (lefthand[2] > righthand[2])
		playerinfo->grabloc[2] = lefthand[2];
	else
		playerinfo->grabloc[2] = righthand[2];

	if (grabfraction < 0.5)
	{
		if ( (playerinfo->flags & PLAYER_FLAG_NO_LARM) && (playerinfo->flags & PLAYER_FLAG_NO_RARM) )
			return false;//can't do half pull up with no arms

		// This is strange, but the VAULT_LOW is actually a high wall vault.
		PlayerAnimSetVault(playerinfo, ASEQ_VAULT_LOW);
	}
	else
	{
		// ...and PULLUP_HALFWALL is just a hop.  SO I moved the arm check to the high wall vault.
		PlayerAnimSetVault(playerinfo, ASEQ_PULLUP_HALFWALL);
	}

	return(true);
}

/*-----------------------------------------------
	PlayerActionPushAway
-----------------------------------------------*/

// NOTE: Currently we only push away the width of the player. This will cause problems for
// non-orthogonal grabbing surfaces. --Pat

#define PLAYER_BLOCKING_DIST 17

void PlayerActionPushAway(playerinfo_t *playerinfo, float value)
{	
	// We're letting go from a grab position.

	trace_t trace;
	vec3_t endpos, pushdir;

	// Check in front of the player for the wall position.

	AngleVectors(playerinfo->angles, pushdir, NULL, NULL);
	VectorMA(playerinfo->origin, PLAYER_BLOCKING_DIST, pushdir, endpos);
	
	if(playerinfo->isclient)
		playerinfo->CL_Trace(playerinfo->origin,
							 NULL,
							 NULL,
							 endpos,
							 CEF_CLIP_TO_WORLD,
							 MASK_PLAYERSOLID,
							 &trace);
	else
		playerinfo->G_Trace(playerinfo->origin,
								  NULL,
								  NULL,
								  endpos,
								  playerinfo->self,
								  MASK_PLAYERSOLID,&trace);
	
	// Now push in the opposite direction for a new location.

	VectorMA(trace.endpos, -PLAYER_BLOCKING_DIST, pushdir, endpos);

	// Try placing the entity in the new location.

	if(playerinfo->isclient)
		playerinfo->CL_Trace(endpos,
							 playerinfo->mins,
							 playerinfo->maxs,
							 endpos,
							 MASK_PLAYERSOLID,
							 CEF_CLIP_TO_WORLD,
							 &trace);
	else
		playerinfo->G_Trace(endpos,
								  playerinfo->mins,
								  playerinfo->maxs,
								  endpos,
								  playerinfo->self,
								  MASK_PLAYERSOLID,&trace);

	// If it didn't work, panic!
//		if (trace.startsolid || trace.allsolid)
//			gi.dprintf("PlayerActionPushAway Failed!!!!\n");

	// NOTENOTE: Put it there anyway for now.

	VectorCopy(trace.endpos, playerinfo->origin);
}

/*-----------------------------------------------
	PlayerActionCheckRopeGrab
-----------------------------------------------*/

qboolean PlayerActionCheckRopeGrab(playerinfo_t *playerinfo, float stomp_org)
{
	if(!playerinfo->isclient)
	{	// Check dismemberment before game side rope check.
		if (playerinfo->flags & PLAYER_FLAG_NO_LARM || playerinfo->flags & PLAYER_FLAG_NO_RARM)
			return(false);
		else
			return(playerinfo->G_PlayerActionCheckRopeGrab(playerinfo,stomp_org));
	}
	else
	{
		return(false);
	}
}

/*-----------------------------------------------
	PlayerActionSplashFeet
-----------------------------------------------*/

void PlayerActionSplashFeet(playerinfo_t *playerinfo, float value)
{
}

/*-----------------------------------------------
	PlayerActionBowSound
-----------------------------------------------*/

void PlayerActionBowSound(playerinfo_t *playerinfo, float value)
{
}

/*-----------------------------------------------
	PlayerActionIdleSound
-----------------------------------------------*/

void PlayerActionIdleSound(playerinfo_t *playerinfo, float value)
{
}

/*-----------------------------------------------
	PlayerActionVaultSound
-----------------------------------------------*/

void PlayerActionVaultSound(playerinfo_t *playerinfo, float value)
{
	char VaultSound[64];
	char *Material=NULL;

	Material=GetClientGroundSurfaceMaterialName(playerinfo);

	if(!Material)
		return;

	strcpy(VaultSound,"player/");
	strcat(VaultSound, Material);
	strcat(VaultSound,"vault.wav");

	if(playerinfo->isclient)
	{
		playerinfo->CL_Sound(SND_PRED_ID21,
							 playerinfo->origin,
							 CHAN_WEAPON,
							 VaultSound,
							 1.0,
							 ATTN_NORM,					 
							 0);
	}
	else
	{
		playerinfo->G_Sound(SND_PRED_ID21,
							playerinfo->leveltime,
							playerinfo->self,
							CHAN_WEAPON,
							playerinfo->G_SoundIndex(VaultSound),
							1.0,
							ATTN_NORM,
							0);
	}
}

/*-----------------------------------------------
	PlayerActionJump
-----------------------------------------------*/

void PlayerActionJump(playerinfo_t *playerinfo, float value)
{
	trace_t		trace;
	vec3_t		endpos;

	VectorCopy(playerinfo->origin, endpos);
	endpos[2] += (playerinfo->mins[2] - 2);

	if(playerinfo->isclient)
		playerinfo->CL_Trace(playerinfo->origin,
							 playerinfo->mins,
							 playerinfo->maxs,
							 endpos,
							 CEF_CLIP_TO_WORLD,
							 MASK_PLAYERSOLID,
							 &trace);
	else
		playerinfo->G_Trace(playerinfo->origin,
								  playerinfo->mins,
								  playerinfo->maxs,
								  endpos,
								  playerinfo->self,
								  MASK_PLAYERSOLID,&trace);

	if ((playerinfo->groundentity || trace.fraction < 0.2) && playerinfo->waterlevel < 2)
	{
		playerinfo->upvel = value*10;
	}
}

/*-----------------------------------------------
	PlayerActionJumpBack
-----------------------------------------------*/

void PlayerActionJumpBack(playerinfo_t *playerinfo, float value)
{
	trace_t		trace;
	vec3_t		endpos;

	VectorCopy(playerinfo->origin, endpos);
	endpos[2] += (playerinfo->mins[2] - 2);

	if(playerinfo->isclient)
	{
		playerinfo->CL_Trace(playerinfo->origin,
							 playerinfo->mins,
							 playerinfo->maxs,
							 endpos,
							 CEF_CLIP_TO_WORLD,
							 MASK_PLAYERSOLID,
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

	if ((playerinfo->groundentity || trace.fraction < 0.2) && playerinfo->waterlevel < 2)
	{
		playerinfo->upvel = 150;
	}
}

/*-----------------------------------------------
	PlayerActionFlipForward
-----------------------------------------------*/

void PlayerActionFlipForward(playerinfo_t *playerinfo, float value)
{
	playerinfo->flags |= PLAYER_FLAG_USE_ENT_POS;
	playerinfo->velocity[2] += value;
}

/*-----------------------------------------------
	PlayerActionFlip
-----------------------------------------------*/

void PlayerActionFlip(playerinfo_t *playerinfo, float value)
{
	playerinfo->flags |= PLAYER_FLAG_USE_ENT_POS;
	playerinfo->velocity[2] += value;
}

/*-----------------------------------------------
	PlayerClimbingMoveFunc
-----------------------------------------------*/

void PlayerClimbingMoveFunc(playerinfo_t *playerinfo, float height, float var2, float var3)
{
	assert(playerinfo);
	
	if(!playerinfo->isclient)
		playerinfo->G_PlayerClimbingMoveFunc(playerinfo,height,var2,var3);
}

/*-----------------------------------------------
	PlayerMoveFunc
-----------------------------------------------*/

void PlayerMoveFunc(playerinfo_t *playerinfo, float fwd, float right, float up)
{	
	// Feeds velocity into the character as a thrust value, like player control (no effect if in
	// the air).

	playerinfo->fwdvel = fwd;
	playerinfo->sidevel = right;
	playerinfo->upvel = up;
}

/*-----------------------------------------------
	PlayerSwimMoveFunc
-----------------------------------------------*/

void PlayerSwimMoveFunc(playerinfo_t *playerinfo, float fwd, float right, float up)
{	
	// Feeds velocity into the character as a thrust value, like player control (no effect if in the air).
	playerinfo->fwdvel = fwd;
	playerinfo->sidevel = right;
	playerinfo->upvel = up;

	if ( playerinfo->seqcmd[ACMDL_STRAFE_L] )
	{
		if (fwd > 0.0)
			playerinfo->sidevel -= (fwd / 1.25);
		else
			playerinfo->sidevel += (fwd / 1.25);
	}
	else if ( playerinfo->seqcmd[ACMDL_STRAFE_R] )
	{
		if (fwd > 0.0)
			playerinfo->sidevel += (fwd / 1.25);
		else
			playerinfo->sidevel -= (fwd / 1.25);
	}
}

/*-----------------------------------------------
	PlayerMoveUpperFunc
-----------------------------------------------*/

void PlayerMoveUpperFunc(playerinfo_t *playerinfo, float fwd, float right, float up)
{	
	// Feeds velocity into the character as a thrust value, like player control (not effect if in
	// the air).

	if (playerinfo->loweridle == false)
		return;

	playerinfo->fwdvel = fwd;
	playerinfo->sidevel = right;
	playerinfo->upvel = up;
}

/*-----------------------------------------------
	PlayerMoveForce
-----------------------------------------------*/

void PlayerMoveForce(playerinfo_t *playerinfo, float fwd, float right, float up)
{	
	// For things like jumps and the like, where the velocity is demanded, not a suggestion.

	vec3_t fwdv, rightv;
	
	AngleVectors(playerinfo->angles, fwdv, rightv, NULL);
	
	VectorScale(fwdv, fwd, playerinfo->velocity);

	if(right != 0)
		VectorMA(playerinfo->velocity, right, rightv, playerinfo->velocity);

	playerinfo->velocity[2] += up;
}

/*-----------------------------------------------
	PlayerJumpMoveForce
-----------------------------------------------*/

void PlayerJumpMoveForce(playerinfo_t *playerinfo, float fwd, float right, float up)
{	
	//For things like jumps and the like, where the velocity is demanded, not a suggestion.
	//NOTENOTE: Same as PlayerMoveForce, but uses where the player is looking (torso)

	vec3_t fwdv, rightv, angles;
	
	VectorCopy(playerinfo->aimangles, angles);
	angles[PITCH] = 0;

	AngleVectors(angles, fwdv, rightv, NULL);

	// speed up leap should need be.
	if (playerinfo->effects & EF_SPEED_ACTIVE)
		fwd *=RUN_MULT;

	VectorScale(fwdv, fwd, playerinfo->velocity);

	//Check to see if we should bother
	if(right)
		VectorMA(playerinfo->velocity, right, rightv, playerinfo->velocity);

	// If the player is strafing, move the player in that direction (diagonal jump)
	if (fwd != 0)
	{	// DON'T do this during a normal side jump.
		if (playerinfo->seqcmd[ACMDL_STRAFE_R] && !playerinfo->seqcmd[ACMDL_STRAFE_L])
		{
			if (playerinfo->buttons & BUTTON_RUN)
				VectorMA(playerinfo->velocity, 260, rightv, playerinfo->velocity);
			else
				VectorMA(playerinfo->velocity, 140, rightv, playerinfo->velocity);
		}
		else if (playerinfo->seqcmd[ACMDL_STRAFE_L] && !playerinfo->seqcmd[ACMDL_STRAFE_R])
		{
			if (playerinfo->buttons & BUTTON_RUN)
				VectorMA(playerinfo->velocity, -260, rightv, playerinfo->velocity);
			else
				VectorMA(playerinfo->velocity, -140, rightv, playerinfo->velocity);
		}
	}

	playerinfo->velocity[2] += up;
}

/*-----------------------------------------------
	PlayerJumpNudge
-----------------------------------------------*/

void PlayerJumpNudge(playerinfo_t *playerinfo, float fwd, float right, float up)
{
	vec3_t vf, vr, vu;
	vec3_t vel;
	float ff,fr,fu, df, dr, du;
	
	AngleVectors(playerinfo->angles, vf, vr, vu);

	VectorCopy(playerinfo->velocity, vel);
	VectorNormalize(vel);

	//Get the dot products of the main directions
	df = DotProduct(vf, vel);
	dr = DotProduct(vr, vel);
	du = DotProduct(vu, vel);

	//Forward fraction of the velocity
	VectorScale(playerinfo->velocity, df, vel);
	ff = VectorLength(vel);

	//Right fraction of the velocity
	VectorScale(playerinfo->velocity, dr, vel);
	fr = VectorLength(vel);

	//Up fraction of the velocity
	VectorScale(playerinfo->velocity, du, vel);
	fu = VectorLength(vel);

	//If we're under the minimum, set the velocity to that minimum
	if ( Q_fabs(ff) < Q_fabs(fwd) )
		VectorMA(playerinfo->velocity, fwd, vf, playerinfo->velocity);

	//If we're under the minimum, set the velocity to that minimum
	if ( Q_fabs(fr) < Q_fabs(right) )
		VectorMA(playerinfo->velocity, right, vr, playerinfo->velocity);

	//If we're under the minimum, set the velocity to that minimum
	if ( Q_fabs(fu) < Q_fabs(up) )
		VectorMA(playerinfo->velocity, up, vu, playerinfo->velocity);

	//Cause the player to use this velocity
	playerinfo->flags |= PLAYER_FLAG_USE_ENT_POS;
}

/*-----------------------------------------------
	PlayerMoveALittle
-----------------------------------------------*/

void PlayerMoveALittle(playerinfo_t *playerinfo, float fwd, float right, float up)
{

	vec3_t fwdv;

	if (playerinfo->seqcmd[ACMDL_FWD] || playerinfo->seqcmd[ACMDL_ACTION])
	{
		playerinfo->flags |= PLAYER_FLAG_USE_ENT_POS;
		AngleVectors(playerinfo->angles, fwdv, NULL, NULL);
		VectorMA(playerinfo->velocity, fwd, fwdv, playerinfo->velocity);
	}
	else if (playerinfo->seqcmd[ACMDL_BACK])
	{
		playerinfo->flags |= PLAYER_FLAG_USE_ENT_POS;
		AngleVectors(playerinfo->angles, fwdv, NULL, NULL);
		VectorMA(playerinfo->velocity, -fwd, fwdv, playerinfo->velocity);
	}
}

/*-----------------------------------------------
	PlayerPullupHeight
-----------------------------------------------*/

void PlayerPullupHeight(playerinfo_t *playerinfo, float height, float endseq, float nopushdown)
{
	trace_t		trace;
	vec3_t		endpoint, vf, savepos;
	vec3_t		playermin, playermax;
	float		x, y, diff;
	
	assert(playerinfo);

	if (endseq > 0)
	{
		// End Sequence.

		VectorCopy(playerinfo->grabloc,endpoint);
		endpoint[2] -= playerinfo->mins[2] + 2;

		if(playerinfo->isclient)
			playerinfo->CL_Trace(playerinfo->origin,
								 playerinfo->mins,
								 playerinfo->maxs,
								 endpoint,
								 MASK_PLAYERSOLID,
								 CEF_CLIP_TO_WORLD,
								 &trace);
		else
			playerinfo->G_Trace(playerinfo->origin,
									  playerinfo->mins,
									  playerinfo->maxs,
									  endpoint,
									  playerinfo->self,
									  MASK_PLAYERSOLID,&trace);

		VectorCopy(trace.endpos, playerinfo->origin);

		if (playerinfo->seqcmd[ACMDL_WALK_F])
		{
			PlayerAnimSetLowerSeq(playerinfo, ASEQ_WALKF);
			return;
		}
		else if (playerinfo->seqcmd[ACMDL_RUN_F])
		{
			PlayerAnimSetLowerSeq(playerinfo, ASEQ_RUNF);
			return;
		}
		else if (playerinfo->seqcmd[ACMDL_JUMP])
		{
			PlayerAnimSetLowerSeq(playerinfo, ASEQ_JUMPSTD_GO);
			return;
		}
		else
		{
			PlayerAnimSetLowerSeq(playerinfo, ASEQ_WALKF_GO);
			return;
		}
	}
	else
	{
		if (playerinfo->grabloc[2] - height > playerinfo->origin[2])
		{
			VectorCopy(playerinfo->origin, endpoint);
			endpoint[2] = playerinfo->grabloc[2] - height;

			VectorCopy(playerinfo->mins, playermin);
			VectorCopy(playerinfo->maxs, playermax);

			if(playerinfo->isclient)
				playerinfo->CL_Trace(playerinfo->origin,
									 playermin,
									 playermax,
									 endpoint,
									 MASK_PLAYERSOLID,
									 CEF_CLIP_TO_WORLD,
									 &trace);
			else
				playerinfo->G_Trace(playerinfo->origin,
										  playermin,
										  playermax,
										  endpoint,
										  playerinfo->self,
										  MASK_PLAYERSOLID,&trace);

			if (trace.fraction < 1.0)
			{	
				// We bumped into something so drop down.
				PlayerAnimSetLowerSeq(playerinfo, ASEQ_FALL);
			}
			else
			{
				VectorCopy(trace.endpos, savepos);
				AngleVectors(playerinfo->angles, vf, NULL, NULL);
				VectorMA(trace.endpos, 32, vf, endpoint);
				//playermin[2] -= 2;

				//Move to the correct distance away from the wall
				if(playerinfo->isclient)
					playerinfo->CL_Trace(trace.endpos,
										 playermin,
										 playermax,
										 endpoint,
										 MASK_PLAYERSOLID,
										 CEF_CLIP_TO_WORLD,
										 &trace);
				else
					playerinfo->G_Trace(trace.endpos,
											  playermin,
											  playermax,
											  endpoint,
											  playerinfo->self,
											  MASK_PLAYERSOLID,&trace);

				if (trace.fraction < 1)
				{
					x = fabs(trace.plane.normal[0]);
					y = fabs(trace.plane.normal[1]);

					diff = fabs(x-y); // 0 to 1

					VectorMA(trace.endpos, (4 * diff), trace.plane.normal, trace.endpos);
					VectorCopy(trace.endpos, playerinfo->origin);
				}
				else
				{
					//Restore the old origin
					VectorCopy(savepos, playerinfo->origin);
				}
			}
		}
	}
}


/*-----------------------------------------------
	PlayerActionCheckPushButton
-----------------------------------------------*/

qboolean PlayerActionCheckPushButton(playerinfo_t *playerinfo)
{
	if(!playerinfo->isclient)
		return(playerinfo->G_PlayerActionCheckPushButton(playerinfo));
	else
		return(false);
}

/*-----------------------------------------------
	PlayerActionPushButton
-----------------------------------------------*/

void PlayerActionPushButton(playerinfo_t *playerinfo, float value)
{
	if(!playerinfo->isclient)
		playerinfo->G_PlayerActionPushButton(playerinfo);
}

/*-----------------------------------------------
	PlayerActionCheckPushLever
-----------------------------------------------*/

qboolean PlayerActionCheckPushLever(playerinfo_t *playerinfo)
{
	if(!playerinfo->isclient)
		return(playerinfo->G_PlayerActionCheckPushLever(playerinfo));
	else
		return(false);
}

/*-----------------------------------------------
	PlayerActionPushLever
-----------------------------------------------*/

void PlayerActionPushLever(playerinfo_t *playerinfo, float value)
{
	if(!playerinfo->isclient)
		playerinfo->G_PlayerActionPushLever(playerinfo);
}

/*-----------------------------------------------
	PlayerActionTakePuzzle
-----------------------------------------------*/

void PlayerActionTakePuzzle(playerinfo_t *playerinfo, float value)
{
	if(!playerinfo->isclient)
		playerinfo->G_PlayerActionTakePuzzle(playerinfo);
}

/*-----------------------------------------------
	PlayerActionMoveItem
-----------------------------------------------*/

void PlayerActionMoveItem(playerinfo_t *playerinfo, float distance)
{
	if(!playerinfo->isclient)
		playerinfo->G_PlayerActionMoveItem(playerinfo,distance);
}

/*-----------------------------------------------
	PlayerActionShrineEffect
-----------------------------------------------*/

void PlayerActionShrineEffect(playerinfo_t *playerinfo, float value)
{
	if(!playerinfo->isclient)
		playerinfo->G_PlayerActionShrineEffect(playerinfo);
}

/*-----------------------------------------------
	PlayerMoveAdd
-----------------------------------------------*/

void PlayerMoveAdd(playerinfo_t *playerinfo)
{
	vec3_t	vf, vr, dir;
	float	mag, fmove, rmove;

	//If we're not nudging, then just return (this probably doesn't save us too much time...)
	if ( (!playerinfo->seqcmd[ACMDL_FWD]) && (!playerinfo->seqcmd[ACMDL_BACK]) && (!playerinfo->seqcmd[ACMDL_STRAFE_L]) && (!playerinfo->seqcmd[ACMDL_STRAFE_R]) )
		return;

	//Setup the information
	AngleVectors(playerinfo->angles, vf, vr, NULL);
	VectorCopy(playerinfo->velocity, dir);
	mag = VectorNormalize(dir);
	
	fmove = (DotProduct(dir, vf) * mag);
	rmove = (DotProduct(dir, vr) * mag);

	//Check and apply the nudges

	if (playerinfo->seqcmd[ACMDL_FWD])
	{
		if ( fmove < AIRMOVE_THRESHOLD )
			VectorMA(playerinfo->velocity, AIRMOVE_AMOUNT, vf, playerinfo->velocity);
	}

	if (playerinfo->seqcmd[ACMDL_BACK])
	{
		if ( fmove > -AIRMOVE_THRESHOLD )
			VectorMA(playerinfo->velocity, -AIRMOVE_AMOUNT, vf, playerinfo->velocity);
	}

	if (playerinfo->seqcmd[ACMDL_STRAFE_L])
	{
		if ( rmove > -AIRMOVE_THRESHOLD )
			VectorMA(playerinfo->velocity, -AIRMOVE_AMOUNT, vr, playerinfo->velocity);
	}

	if (playerinfo->seqcmd[ACMDL_STRAFE_R])
	{
		if ( rmove < AIRMOVE_THRESHOLD )
			VectorMA(playerinfo->velocity,  AIRMOVE_AMOUNT, vr, playerinfo->velocity);
	}

	//Use the velocity to move the player
	playerinfo->flags |= PLAYER_FLAG_USE_ENT_POS;
}

/*-----------------------------------------------
	PlayerActionDrownFloatUp
-----------------------------------------------*/

#define ANGLE_RECOVER_AMOUNT	2	//Degrees per tenth of a second

void PlayerActionDrownFloatUp( playerinfo_t *playerinfo )
{
}

/*-----------------------------------------------
	PlayerActionCheckDive
-----------------------------------------------*/

void PlayerActionCheckDive( playerinfo_t *playerinfo )
{
	if (playerinfo->pm_w_flags & WF_DIVE)
	{
		playerinfo->flags&=~PLAYER_FLAG_DIVE;
		playerinfo->pm_w_flags |= WF_DIVING;
		playerinfo->pm_w_flags &= ~(WF_SURFACE|WF_DIVE);

		PlayerAnimSetLowerSeq(playerinfo, ASEQ_DIVE);
	}
}

/*-----------------------------------------------
	PlayerActionCheckRopeMove
-----------------------------------------------*/

void PlayerActionCheckRopeMove( playerinfo_t *playerinfo, float foo )
{
	if (!playerinfo->isclient)
		playerinfo->G_PlayerActionCheckRopeMove(playerinfo);
	else
		return;
}

/*-----------------------------------------------
	PlayerReleaseRope
-----------------------------------------------*/

PLAYER_API void PlayerReleaseRope(playerinfo_t *playerinfo)
{
	assert(playerinfo);
	playerinfo->flags &= ~PLAYER_FLAG_ONROPE;
}

/*-----------------------------------------------
	PlayerReleaseRope
-----------------------------------------------*/

PLAYER_API void KnockDownPlayer(playerinfo_t *playerinfo)
{
	assert(playerinfo);

	//Chicken cannot be knocked down
	if (playerinfo->flags & EF_CHICKEN)
		return;

	if(!(playerinfo->flags&PLAYER_FLAG_KNOCKDOWN))
		playerinfo->flags|=PLAYER_FLAG_KNOCKDOWN;
}

/*-----------------------------------------------
	PlayFly
-----------------------------------------------*/

PLAYER_API void PlayFly(playerinfo_t *playerinfo, float dist)
{
	if(playerinfo->isclient)
	{
		playerinfo->CL_Sound(SND_PRED_ID22,
							 playerinfo->origin,
							 CHAN_BODY,
							 "player/idle buzz.wav",
							 1.0,
							 ATTN_NORM,
							 0);
	}
	else
	{
		playerinfo->G_Sound(SND_PRED_ID22,
							playerinfo->leveltime,
							playerinfo->self,
							CHAN_BODY,
							playerinfo->G_SoundIndex("player/idle buzz.wav"),
							1.0,
							ATTN_NORM,
							0);
	}
}

/*-----------------------------------------------
	PlaySlap
-----------------------------------------------*/

PLAYER_API void PlaySlap(playerinfo_t *playerinfo, float dist)
{
	if(playerinfo->isclient)
	{
		playerinfo->CL_Sound(SND_PRED_ID23,
							 playerinfo->origin,
							 CHAN_BODY,
							 "player/idle slap.wav",
							 1.0,
							 ATTN_NORM,
							 0);
	}
	else
	{
		playerinfo->G_Sound(SND_PRED_ID23,
							playerinfo->leveltime,	
							playerinfo->self,
							CHAN_BODY,
							playerinfo->G_SoundIndex("player/idle slap.wav"),
							1.0,
							ATTN_NORM,
							0);
	}
}

/*-----------------------------------------------
	PlaySigh
-----------------------------------------------*/

PLAYER_API void PlaySigh(playerinfo_t *playerinfo, float dist)
{
	if(playerinfo->isclient)
	{
		playerinfo->CL_Sound(SND_PRED_ID24,
							 playerinfo->origin,
							 CHAN_BODY,
							 "*phew.wav",
							 0.75,
							 ATTN_NORM,
							 0);
	}
	else
	{
		playerinfo->G_Sound(SND_PRED_ID24,
							playerinfo->leveltime,
							playerinfo->self,
							CHAN_BODY,
							playerinfo->G_SoundIndex("*phew.wav"),
							0.75,
							ATTN_NORM,
							0);
	}
}
/*-----------------------------------------------
	PlayScratch
-----------------------------------------------*/

PLAYER_API void PlayScratch(playerinfo_t *playerinfo, float dist)
{
	if(playerinfo->isclient)
	{
		playerinfo->CL_Sound(SND_PRED_ID25,
							 playerinfo->origin,
							 CHAN_BODY,
							 "player/scratch.wav",
							 1.0,
							 ATTN_NORM,
							 0);
	}
	else
	{
		playerinfo->G_Sound(SND_PRED_ID25,
							playerinfo->leveltime,
							playerinfo->self,
							CHAN_BODY,
							playerinfo->G_SoundIndex("player/scratch.wav"),
							1.0,
							ATTN_NORM,
							0);
	}
}

/*-----------------------------------------------
	SpawnDustPuff
-----------------------------------------------*/

PLAYER_API void SpawnDustPuff(playerinfo_t *playerinfo, float dist)
{
	if(playerinfo->waterlevel==0)
	{
		if(!playerinfo->isclient)
			playerinfo->G_CreateEffect(EFFECT_PRED_ID10,
									   playerinfo->G_GetEntityStatePtr(playerinfo->self),
									   FX_DUST_PUFF,
									   CEF_OWNERS_ORIGIN,
									   playerinfo->origin,
									   "");
		else
			playerinfo->CL_CreateEffect(EFFECT_PRED_ID10,
										playerinfo->self,
										FX_DUST_PUFF,
										CEF_OWNERS_ORIGIN,
										playerinfo->origin,
										"");
	}
}

/*-----------------------------------------------
	PlayerActionCheckCreep
-----------------------------------------------*/

void PlayerActionCheckCreep( playerinfo_t *playerinfo )
{
	int	curseq = playerinfo->lowerseq;

	//Check for an autovault (only occurs if upper half of body is idle!)
	if ( (playerinfo->flags & PLAYER_FLAG_COLLISION) &&  (playerinfo->upperidle) && (playerinfo->seqcmd[ACMDL_FWD]) )
	{
		PlayerActionCheckVault(playerinfo, 0);
		
		if (curseq == ASEQ_VAULT_LOW)
		{
			PlayerAnimSetLowerSeq(playerinfo,  ASEQ_VAULT_LOW);
			return;
		}

		if (curseq == ASEQ_PULLUP_HALFWALL)
		{
			PlayerAnimSetLowerSeq(playerinfo,  ASEQ_PULLUP_HALFWALL);
			return;
		}
	}

	//Check for a jump	[High probability]
	if (playerinfo->seqcmd[ACMDL_JUMP])
	{
		//Slime causes skipping, so no jumping in it!
		if (!(playerinfo->watertype & (CONTENTS_SLIME|CONTENTS_LAVA)))
		{
			if (playerinfo->seqcmd[ACMDL_FWD])
			{
				PlayerAnimSetLowerSeq(playerinfo,  ASEQ_JUMPFWD_SGO);
				return;
			}
			else if (playerinfo->seqcmd[ACMDL_BACK])
			{
				PlayerAnimSetLowerSeq(playerinfo,  ASEQ_JUMPBACK_SGO);
				return;
			}
			if (playerinfo->seqcmd[ACMDL_STRAFE_L])
			{
				PlayerAnimSetLowerSeq(playerinfo,  ASEQ_JUMPLEFT_SGO);
				return;
			}
			else if (playerinfo->seqcmd[ACMDL_STRAFE_R])
			{
				PlayerAnimSetLowerSeq(playerinfo,  ASEQ_JUMPRIGHT_SGO);
				return;
			}
		}
	}
	
	//Check for a transition to a creeping strafe		[High probability]
	if ( playerinfo->seqcmd[ACMDL_CREEP_F] && playerinfo->seqcmd[ACMDL_STRAFE_L] && curseq != ASEQ_CSTRAFE_LEFT)
	{
		playerinfo->lowerseq = ASEQ_CSTRAFE_LEFT;
		playerinfo->lowermove = PlayerSeqData[playerinfo->lowerseq].move;
		playerinfo->lowerframeptr = playerinfo->lowermove->frame + playerinfo->lowerframe;
	
		return;
	}
	 
	if ( playerinfo->seqcmd[ACMDL_CREEP_F] && playerinfo->seqcmd[ACMDL_STRAFE_R] && curseq != ASEQ_CSTRAFE_RIGHT)
	{
		playerinfo->lowerseq = ASEQ_CSTRAFE_RIGHT;
		playerinfo->lowermove = PlayerSeqData[playerinfo->lowerseq].move;
		playerinfo->lowerframeptr = playerinfo->lowermove->frame + playerinfo->lowerframe;
	
		return;
	}
	
	//Check for a transition to a creeping strafe		[High probability]
	if ( playerinfo->seqcmd[ACMDL_CREEP_B] && playerinfo->seqcmd[ACMDL_STRAFE_L] && curseq != ASEQ_CSTRAFEB_LEFT)
	{
		playerinfo->lowerseq = ASEQ_CSTRAFEB_LEFT;
		playerinfo->lowermove = PlayerSeqData[playerinfo->lowerseq].move;
		playerinfo->lowerframeptr = playerinfo->lowermove->frame + playerinfo->lowerframe;
	
		return;
	}
	 
	if ( playerinfo->seqcmd[ACMDL_CREEP_B] && playerinfo->seqcmd[ACMDL_STRAFE_R] && curseq != ASEQ_CSTRAFEB_RIGHT)
	{
		playerinfo->lowerseq = ASEQ_CSTRAFEB_RIGHT;
		playerinfo->lowermove = PlayerSeqData[playerinfo->lowerseq].move;
		playerinfo->lowerframeptr = playerinfo->lowermove->frame + playerinfo->lowerframe;
	
		return;
	}

	//Check for a sudden transition to a walk	[Low probability]
	if (playerinfo->seqcmd[ACMDL_WALK_F])
	{
		PlayerAnimSetLowerSeq(playerinfo,  ASEQ_WALKF);
		return;
	}

	//Check for a sudden transition to a run	[Low probability]
	if (playerinfo->seqcmd[ACMDL_RUN_F])
	{
		PlayerAnimSetLowerSeq(playerinfo,  ASEQ_RUNF);
		return;
	}

	//Check for a crouch	[Low probability]
	if (playerinfo->seqcmd[ACMDL_CROUCH])
	{
		if (playerinfo->seqcmd[ACMDL_BACK])
		{
			PlayerAnimSetLowerSeq(playerinfo,  ASEQ_CROUCH_WALK_B);
			return;
		}
		else if (playerinfo->seqcmd[ACMDL_STRAFE_L])
		{
			PlayerAnimSetLowerSeq(playerinfo,  ASEQ_CROUCH_WALK_L);
			return;
		}
		else if (playerinfo->seqcmd[ACMDL_STRAFE_R])
		{
			PlayerAnimSetLowerSeq(playerinfo,  ASEQ_CROUCH_WALK_R);
			return;
		}
		else if (playerinfo->seqcmd[ACMDL_BACK])
		{
			PlayerAnimSetLowerSeq(playerinfo,  ASEQ_CROUCH_WALK_F);
			return;
		}
		else
		{
			PlayerAnimSetLowerSeq(playerinfo,  ASEQ_CROUCH_GO);
			return;
		}
	}

	//Handle an action key press	[Low probability]
	if (playerinfo->seqcmd[ACMDL_ACTION])
	{
		//Climb a rope?
		if ( (playerinfo->targetEnt) && (PlayerActionCheckRopeGrab(playerinfo, 0)) )
		{
			playerinfo->flags |= PLAYER_FLAG_ONROPE;
			
			if(playerinfo->isclient)
				playerinfo->CL_Sound(SND_PRED_ID26, playerinfo->origin, CHAN_VOICE, "player/ropegrab.wav", 0.75, ATTN_NORM, 0 );
			else
				playerinfo->G_Sound(SND_PRED_ID26, playerinfo->leveltime,playerinfo->self, CHAN_VOICE, playerinfo->G_SoundIndex("player/ropegrab.wav"), 0.75, ATTN_NORM, 0 );

			//We're on the rope
			PlayerAnimSetLowerSeq(playerinfo,  ASEQ_CLIMB_ON);
			return;
		}
	}

	//Check for a quickturn		[Low probability]
	if (playerinfo->seqcmd[ACMDL_QUICKTURN])
	{
		PlayerAnimSetLowerSeq(playerinfo,  ASEQ_TURN180);
		return;
	}

	//If we're pressing forward, and nothing else is happening, then we're walking forward
	if ( (playerinfo->seqcmd[ACMDL_CREEP_F]) && (!playerinfo->seqcmd[ACMDL_STRAFE_L]) && (!playerinfo->seqcmd[ACMDL_STRAFE_R]) )
	{
		if (curseq != ASEQ_CREEPF)
		{
			if (PlayerActionCheckCreepMoveForward(playerinfo))
				PlayerAnimSetLowerSeq(playerinfo, ASEQ_CREEPF);
			else
				PlayerAnimSetLowerSeq(playerinfo, SeqCtrl[playerinfo->lowerseq].ceaseseq);

			return;
		}
		else if (!(PlayerActionCheckCreepMoveForward(playerinfo)))
		{
			PlayerAnimSetLowerSeq(playerinfo, SeqCtrl[playerinfo->lowerseq].ceaseseq);
			return;
		}
	}

	//If we're pressing backward, and nothing else is happening, then we're walking backward
	if ( (playerinfo->seqcmd[ACMDL_CREEP_B]) && (!playerinfo->seqcmd[ACMDL_STRAFE_L]) && (!playerinfo->seqcmd[ACMDL_STRAFE_R]) )
	{
		if (curseq != ASEQ_CREEPB)
		{
			if (PlayerActionCheckCreepMoveBack(playerinfo))
				PlayerAnimSetLowerSeq(playerinfo, ASEQ_CREEPB);
			else
				PlayerAnimSetLowerSeq(playerinfo, SeqCtrl[playerinfo->lowerseq].ceaseseq);

			return;
		}
		else if (!(PlayerActionCheckCreepMoveBack(playerinfo)))
		{
			PlayerAnimSetLowerSeq(playerinfo, SeqCtrl[playerinfo->lowerseq].ceaseseq);
			return;
		}
	}
	
	//All else has failed... did we just let go of everthing?
	if (!playerinfo->seqcmd[ACMDL_FWD] && !playerinfo->seqcmd[ACMDL_BACK])
	{
		PlayerAnimSetLowerSeq(playerinfo, SeqCtrl[playerinfo->lowerseq].ceaseseq);
		return;
	}
}

/*-----------------------------------------------
	PlayerActionCheckCreepUnStrafe
-----------------------------------------------*/

void PlayerActionCheckCreepUnStrafe( playerinfo_t *playerinfo )
{
	//Player has started running
	if (playerinfo->buttons & BUTTON_RUN)	
	{
		if (playerinfo->seqcmd[ACMDL_STRAFE_L])
		{
			PlayerAnimSetLowerSeq(playerinfo, ASEQ_RSTRAFE_LEFT);
			return;
		}
		else if (playerinfo->seqcmd[ACMDL_STRAFE_R])
		{
			PlayerAnimSetLowerSeq(playerinfo, ASEQ_RSTRAFE_RIGHT);
			return;
		}
	}
	else if (!(playerinfo->buttons & BUTTON_CREEP))
	{
		if (playerinfo->seqcmd[ACMDL_STRAFE_L])
		{
			PlayerAnimSetLowerSeq(playerinfo, ASEQ_WSTRAFE_LEFT);
			return;
		}
		else if (playerinfo->seqcmd[ACMDL_STRAFE_R])
		{
			PlayerAnimSetLowerSeq(playerinfo, ASEQ_WSTRAFE_RIGHT);
			return;
		}
	}

	//Still pressing the same way and still strafing
	if (playerinfo->seqcmd[ACMDL_FWD] && playerinfo->seqcmd[ACMDL_STRAFE_L] && playerinfo->lowerseq == ASEQ_CSTRAFE_LEFT)
	{
		//Account for coincidental action
		PlayerActionCheckCreep(playerinfo);
		return;
	}

	if (playerinfo->seqcmd[ACMDL_FWD] && playerinfo->seqcmd[ACMDL_STRAFE_R] && playerinfo->lowerseq == ASEQ_CSTRAFE_RIGHT)
	{
		//Account for coincidental action
		PlayerActionCheckCreep(playerinfo);
		return;
	}

	//Stopped moving forward, has gone to a side strafe
	if (!playerinfo->seqcmd[ACMDL_FWD])
	{
		//Check for a transfer to a run
		if (playerinfo->buttons & BUTTON_RUN)
		{
			if (playerinfo->seqcmd[ACMDL_STRAFE_L])
			{
				PlayerAnimSetLowerSeq(playerinfo, ASEQ_DASH_LEFT_GO);
				return;
			}
			else if (playerinfo->seqcmd[ACMDL_STRAFE_R])
			{
				PlayerAnimSetLowerSeq(playerinfo, ASEQ_DASH_RIGHT_GO);
				return;
			}
		}
		else
		{
			if (playerinfo->seqcmd[ACMDL_STRAFE_L])
			{
				PlayerAnimSetLowerSeq(playerinfo, ASEQ_STRAFEL);
				return;
			}
			else if (playerinfo->seqcmd[ACMDL_STRAFE_R])
			{
				PlayerAnimSetLowerSeq(playerinfo, ASEQ_STRAFER);
				return;
			}
		}
	}

	//Have we reversed directions of the strafe?
	if (playerinfo->seqcmd[ACMDL_STRAFE_R] && playerinfo->lowerseq == ASEQ_CSTRAFE_LEFT)
	{
		playerinfo->lowerseq = ASEQ_CSTRAFE_RIGHT;
		playerinfo->lowermove = PlayerSeqData[playerinfo->lowerseq].move;
		playerinfo->lowerframeptr = playerinfo->lowermove->frame + playerinfo->lowerframe;
		
		return;
	}

	if (playerinfo->seqcmd[ACMDL_STRAFE_L] && playerinfo->lowerseq == ASEQ_CSTRAFE_RIGHT)
	{
		playerinfo->lowerseq = ASEQ_CSTRAFE_LEFT;
		playerinfo->lowermove = PlayerSeqData[playerinfo->lowerseq].move;
		playerinfo->lowerframeptr = playerinfo->lowermove->frame + playerinfo->lowerframe;
		
		return;
	}

	//We're doing something else, so run a normal function to determine it
	PlayerActionCheckCreep(playerinfo);
}

/*-----------------------------------------------
	PlayerActionCheckCreepBack
-----------------------------------------------*/

void PlayerActionCheckCreepBack( playerinfo_t *playerinfo )
{
	//We're doing something else, so run a normal function to determine it
	PlayerActionCheckCreep(playerinfo);
}

/*-----------------------------------------------
	PlayerActionCheckCreepBackUnStrafe
-----------------------------------------------*/

void PlayerActionCheckCreepBackUnStrafe( playerinfo_t *playerinfo )
{
	//Still pressing the same way and still strafing
	if (playerinfo->seqcmd[ACMDL_BACK] && playerinfo->seqcmd[ACMDL_STRAFE_L] && playerinfo->lowerseq == ASEQ_CSTRAFEB_LEFT)
	{
		//Account for coincidental action
		PlayerActionCheckCreep(playerinfo);
		return;
	}

	if (playerinfo->seqcmd[ACMDL_BACK] && playerinfo->seqcmd[ACMDL_STRAFE_R] && playerinfo->lowerseq == ASEQ_CSTRAFEB_RIGHT)
	{
		//Account for coincidental action
		PlayerActionCheckCreep(playerinfo);
		return;
	}

	//Stopped moving forward, has gone to a side strafe
	if (!playerinfo->seqcmd[ACMDL_BACK])
	{
		//Check for a transfer to a run
		if (playerinfo->buttons & BUTTON_RUN)
		{
			if (playerinfo->seqcmd[ACMDL_STRAFE_L])
			{
				PlayerAnimSetLowerSeq(playerinfo, ASEQ_DASH_LEFT_GO);
				return;
			}
			else if (playerinfo->seqcmd[ACMDL_STRAFE_R])
			{
				PlayerAnimSetLowerSeq(playerinfo, ASEQ_DASH_RIGHT_GO);
				return;
			}
		}
		else
		{
			if (playerinfo->seqcmd[ACMDL_STRAFE_L])
			{
				PlayerAnimSetLowerSeq(playerinfo, ASEQ_STRAFEL);
				return;
			}
			else if (playerinfo->seqcmd[ACMDL_STRAFE_R])
			{
				PlayerAnimSetLowerSeq(playerinfo, ASEQ_STRAFER);
				return;
			}
		}
	}

	//Have we reversed directions of the strafe?
	if (playerinfo->seqcmd[ACMDL_STRAFE_R] && playerinfo->lowerseq == ASEQ_CSTRAFEB_LEFT)
	{
		playerinfo->lowerseq = ASEQ_CSTRAFEB_RIGHT;
		playerinfo->lowermove = PlayerSeqData[playerinfo->lowerseq].move;
		playerinfo->lowerframeptr = playerinfo->lowermove->frame + playerinfo->lowerframe;
		
		return;
	}

	if (playerinfo->seqcmd[ACMDL_STRAFE_L] && playerinfo->lowerseq == ASEQ_CSTRAFEB_RIGHT)
	{
		playerinfo->lowerseq = ASEQ_CSTRAFEB_LEFT;
		playerinfo->lowermove = PlayerSeqData[playerinfo->lowerseq].move;
		playerinfo->lowerframeptr = playerinfo->lowermove->frame + playerinfo->lowerframe;
		
		return;
	}

	//We're doing something else, so run a normal function to determine it
	PlayerActionCheckCreep(playerinfo);
}






/*

	Walk Forward

*/






/*-----------------------------------------------
	PlayerActionCheckWalk
-----------------------------------------------*/

extern qboolean CheckFall(playerinfo_t *playerinfo);

void PlayerActionCheckWalk( playerinfo_t *playerinfo )
{
	int	curseq = playerinfo->lowerseq;

	if (playerinfo->groundentity==NULL && playerinfo->waterlevel < 2 && !(playerinfo->watertype & (CONTENTS_SLIME|CONTENTS_LAVA)))
	{
		if (CheckFall(playerinfo))
		{
			PlayerAnimSetLowerSeq(playerinfo,  ASEQ_FALLWALK_GO);
			return;
		}
	}

	//Check for an autovault (only occurs if upper half of body is idle!)
	if ( (playerinfo->flags & PLAYER_FLAG_COLLISION) && (playerinfo->upperidle) && (playerinfo->seqcmd[ACMDL_FWD]) )
	{
		PlayerActionCheckVault(playerinfo, 0);
		
		if (curseq == ASEQ_VAULT_LOW)
		{
			PlayerAnimSetLowerSeq(playerinfo,  ASEQ_VAULT_LOW);
			return;
		}

		if (curseq == ASEQ_PULLUP_HALFWALL)
		{
			PlayerAnimSetLowerSeq(playerinfo,  ASEQ_PULLUP_HALFWALL);
			return;
		}
	}

	//Check for a jump	[High probability]
	if (playerinfo->seqcmd[ACMDL_JUMP])
	{
		//Slime causes skipping, so no jumping in it!
		if (!(playerinfo->watertype & (CONTENTS_SLIME|CONTENTS_LAVA)))
		{
			//Check what type of jump it is
			if (playerinfo->seqcmd[ACMDL_FWD])
			{
				PlayerAnimSetLowerSeq(playerinfo,  ASEQ_JUMPFWD_WGO);
				return;
			}
			
			if (playerinfo->seqcmd[ACMDL_BACK])
			{
				PlayerAnimSetLowerSeq(playerinfo,  ASEQ_JUMPBACK_WGO);
				return;
			}
			
			if (playerinfo->seqcmd[ACMDL_STRAFE_L])
			{
				PlayerAnimSetLowerSeq(playerinfo,  ASEQ_JUMPLEFT_WGO);
				return;
			}
			
			if (playerinfo->seqcmd[ACMDL_STRAFE_R])
			{
				PlayerAnimSetLowerSeq(playerinfo,  ASEQ_JUMPRIGHT_WGO);
				return;
			}
		}
	}

	//Check for a crouch	[Low probability]
	if (playerinfo->seqcmd[ACMDL_CROUCH])
	{
		if (playerinfo->buttons & BUTTON_CREEP)
		{
			//Check crouch direction
			if (playerinfo->seqcmd[ACMDL_BACK])
			{
				PlayerAnimSetLowerSeq(playerinfo,  ASEQ_CROUCH_WALK_B);
				return;
			}
			
			if (playerinfo->seqcmd[ACMDL_STRAFE_L])
			{
				PlayerAnimSetLowerSeq(playerinfo,  ASEQ_CROUCH_WALK_L);
				return;
			}
			
			if (playerinfo->seqcmd[ACMDL_STRAFE_R])
			{
				PlayerAnimSetLowerSeq(playerinfo,  ASEQ_CROUCH_WALK_R);
				return;
			}
			
			if (playerinfo->seqcmd[ACMDL_FWD])
			{
				PlayerAnimSetLowerSeq(playerinfo,  ASEQ_CROUCH_WALK_F);
				return;
			}
				
			//All else failed, we just want to crouch to the ground
			PlayerAnimSetLowerSeq(playerinfo,  ASEQ_CROUCH_GO);
			return;
		}
		else
		{
			//Check crouch direction
			if (playerinfo->seqcmd[ACMDL_BACK])
			{
				PlayerAnimSetLowerSeq(playerinfo,  ASEQ_ROLL_B);
				return;
			}
			
			if (playerinfo->seqcmd[ACMDL_STRAFE_L])
			{
				PlayerAnimSetLowerSeq(playerinfo,  ASEQ_ROLL_L);
				return;
			}
			
			if (playerinfo->seqcmd[ACMDL_STRAFE_R])
			{
				PlayerAnimSetLowerSeq(playerinfo,  ASEQ_ROLL_R);
				return;
			}
			
			if (playerinfo->seqcmd[ACMDL_FWD])
			{
				PlayerAnimSetLowerSeq(playerinfo,  ASEQ_ROLLDIVEF_W);
				return;
			}
				
			//All else failed, we just want to crouch to the ground
			PlayerAnimSetLowerSeq(playerinfo,  ASEQ_CROUCH_GO);
			return;
		}
	}

	//Check for a transition to a walking strafe		[High probability]
	if ( playerinfo->seqcmd[ACMDL_FWD] && playerinfo->seqcmd[ACMDL_STRAFE_L] )
	{
		if (playerinfo->buttons & BUTTON_CREEP)
		{
			PlayerAnimSetLowerSeq(playerinfo, ASEQ_CSTRAFE_LEFT);
			return;
		}
		else if (playerinfo->buttons & BUTTON_RUN)
		{
			PlayerAnimSetLowerSeq(playerinfo, ASEQ_RSTRAFE_LEFT);
			return;
		}
		else if ( curseq != ASEQ_WSTRAFE_LEFT )
		{
			playerinfo->lowerseq = ASEQ_WSTRAFE_LEFT;
			playerinfo->lowermove = PlayerSeqData[playerinfo->lowerseq].move;
			playerinfo->lowerframeptr = playerinfo->lowermove->frame + playerinfo->lowerframe;
		
			return;
		}
	}
    
	//Check for a transition to a walking or running strafe		[High probability]
	if ( playerinfo->seqcmd[ACMDL_FWD] && playerinfo->seqcmd[ACMDL_STRAFE_R])
	{
		if (playerinfo->buttons & BUTTON_CREEP)
		{
			PlayerAnimSetLowerSeq(playerinfo, ASEQ_CSTRAFE_RIGHT);
			return;
		}
		else if (playerinfo->buttons & BUTTON_RUN)
		{
			PlayerAnimSetLowerSeq(playerinfo, ASEQ_RSTRAFE_RIGHT);
			return;
		}
		else if ( curseq != ASEQ_WSTRAFE_RIGHT )
		{
			playerinfo->lowerseq = ASEQ_WSTRAFE_RIGHT;
			playerinfo->lowermove = PlayerSeqData[playerinfo->lowerseq].move;
			playerinfo->lowerframeptr = playerinfo->lowermove->frame + playerinfo->lowerframe;
			
			return;
		}
	}
    
	//Check for a transition to a backward walking or running strafe	[High probability]
	if ( playerinfo->seqcmd[ACMDL_BACK] && playerinfo->seqcmd[ACMDL_STRAFE_L] )
	{
		if (playerinfo->buttons & BUTTON_CREEP)
		{
			PlayerAnimSetLowerSeq(playerinfo, ASEQ_CSTRAFEB_LEFT);
			return;
		}
		else if (playerinfo->buttons & BUTTON_RUN)
		{
			//FIXME: There are no backwards run strafes!
			return;
		}
		else if ( curseq != ASEQ_WSTRAFEB_LEFT )
		{
			playerinfo->lowerseq = ASEQ_WSTRAFEB_LEFT;
			playerinfo->lowermove = PlayerSeqData[playerinfo->lowerseq].move;
			playerinfo->lowerframeptr = playerinfo->lowermove->frame + playerinfo->lowerframe;
		
			return;
		}
		return;
	}

   	//Check for a transition to a backward walking or running strafe	[High probability]
	if ( playerinfo->seqcmd[ACMDL_BACK] && playerinfo->seqcmd[ACMDL_STRAFE_R])
	{
		if (playerinfo->buttons & BUTTON_CREEP)
		{
			PlayerAnimSetLowerSeq(playerinfo, ASEQ_CSTRAFEB_RIGHT);
			return;
		}
		else if (playerinfo->buttons & BUTTON_RUN)
		{
			//FIXME: There are no backwards run strafes!
			return;
		}
		else if ( curseq != ASEQ_WSTRAFEB_RIGHT )
		{
			playerinfo->lowerseq = ASEQ_WSTRAFEB_RIGHT;
			playerinfo->lowermove = PlayerSeqData[playerinfo->lowerseq].move;
			playerinfo->lowerframeptr = playerinfo->lowermove->frame + playerinfo->lowerframe;
			
			return;
		}
	}

	//FORWARD

	//Check for a sudden transition to a creep	[Low probability]
	if (playerinfo->seqcmd[ACMDL_CREEP_F])
	{
		PlayerAnimSetLowerSeq(playerinfo,  ASEQ_CREEPF);
		return;
	}

	//Check for a sudden transition to a run	[Low probability]
	if (playerinfo->seqcmd[ACMDL_RUN_F])
	{
		PlayerAnimSetLowerSeq(playerinfo,  ASEQ_RUNF);
		return;
	}

	//BACK

	//Check for a sudden transition to a creep	[Low probability]
	if (playerinfo->seqcmd[ACMDL_CREEP_B])
	{
		PlayerAnimSetLowerSeq(playerinfo,  ASEQ_CREEPB);
		return;
	}

	//Handle an action key press	[Low probability]
	if (playerinfo->seqcmd[ACMDL_ACTION])
	{
		//Climb a rope?
		if ( (playerinfo->targetEnt) && (PlayerActionCheckRopeGrab(playerinfo,0)) )
		{
			playerinfo->flags |= PLAYER_FLAG_ONROPE;
			
			if(playerinfo->isclient)
				playerinfo->CL_Sound(SND_PRED_ID27, playerinfo->origin, CHAN_VOICE, "player/ropegrab.wav", 0.75, ATTN_NORM, 0 );
			else
				playerinfo->G_Sound(SND_PRED_ID27, playerinfo->leveltime,playerinfo->self, CHAN_VOICE, playerinfo->G_SoundIndex("player/ropegrab.wav"), 0.75, ATTN_NORM, 0 );

			//We're on the rope
			PlayerAnimSetLowerSeq(playerinfo,  ASEQ_CLIMB_ON);
			return;
		}

		//Use a puzzle piece
		PlayerActionUsePuzzle(playerinfo);

		if ( (playerinfo->flags & PLAYER_FLAG_COLLISION) && (playerinfo->upperidle) && (PlayerActionCheckJumpGrab(playerinfo, 0)) )
		{	
			PlayerAnimSetLowerSeq(playerinfo, ASEQ_JUMPSTD_GO);
			return;
		}
	}

	//Check for a quickturn		[Low probability]
	if (playerinfo->seqcmd[ACMDL_QUICKTURN])
	{
		PlayerAnimSetLowerSeq(playerinfo,  ASEQ_TURN180);
		return;
	}

	//If we're pressing forward, and nothing else is happening, then we're just walking forward
	if ( (playerinfo->seqcmd[ACMDL_WALK_F]) && (!playerinfo->seqcmd[ACMDL_STRAFE_L]) && (!playerinfo->seqcmd[ACMDL_STRAFE_R]) )
	{
		if (curseq != ASEQ_WALKF)
		{
			PlayerAnimSetLowerSeq(playerinfo, ASEQ_WALKF);
			return;
		}
	}

	//If we're pressing backward, and nothing else is happening, then we're just walking backward
	if ( (playerinfo->seqcmd[ACMDL_BACK]) && (!playerinfo->seqcmd[ACMDL_STRAFE_L]) && (!playerinfo->seqcmd[ACMDL_STRAFE_R]) )
	{
		if (playerinfo->seqcmd[ACMDL_RUN_B])
		{
			if (!(playerinfo->seqcmd[ACMDU_ATTACK]) && playerinfo->upperidle)
			{
				PlayerAnimSetLowerSeq(playerinfo, ASEQ_JUMPSPRINGBGO);
				return;
			}
		}
		else if (curseq != ASEQ_WALKB)
		{
			PlayerAnimSetLowerSeq(playerinfo, ASEQ_WALKB);
			return;
		}
	}

	//All else has failed... did we just let go of everthing?
	if (!playerinfo->seqcmd[ACMDL_FWD] && !playerinfo->seqcmd[ACMDL_BACK])
	{
		PlayerAnimSetLowerSeq(playerinfo, SeqCtrl[playerinfo->lowerseq].ceaseseq);
		return;
	}
}

/*-----------------------------------------------
	PlayerActionCheckWalkUnStrafe
-----------------------------------------------*/

void PlayerActionCheckWalkUnStrafe(playerinfo_t *playerinfo)
{
	//Player has started running
	if (playerinfo->buttons & BUTTON_RUN)	
	{
		if (playerinfo->seqcmd[ACMDL_STRAFE_L])
		{
			PlayerAnimSetLowerSeq(playerinfo, ASEQ_RSTRAFE_LEFT);
			return;
		}
		else if (playerinfo->seqcmd[ACMDL_STRAFE_R])
		{
			PlayerAnimSetLowerSeq(playerinfo, ASEQ_RSTRAFE_RIGHT);
			return;
		}
	}

	//Still pressing the same way and still strafing
	if (playerinfo->seqcmd[ACMDL_FWD] && playerinfo->seqcmd[ACMDL_STRAFE_L] && playerinfo->lowerseq == ASEQ_WSTRAFE_LEFT)
	{
		if (playerinfo->seqcmd[ACMDL_JUMP] )
		{
			PlayerActionCheckWalk(playerinfo);
		}

		//Account for coincidental action
		PlayerActionCheckWalk(playerinfo);
		return;
	}

	if (playerinfo->seqcmd[ACMDL_FWD] && playerinfo->seqcmd[ACMDL_STRAFE_R] && playerinfo->lowerseq == ASEQ_WSTRAFE_RIGHT)
	{
		if (playerinfo->seqcmd[ACMDL_JUMP] )
		{
			PlayerActionCheckWalk(playerinfo);
		}
		
		//Account for coincidental action
		PlayerActionCheckWalk(playerinfo);
		return;
	}

	//Stopped moving forward, has gone to a side strafe
	if (!playerinfo->seqcmd[ACMDL_FWD])
	{
		//Check for a transfer to a run
		if (playerinfo->buttons & BUTTON_RUN)
		{
			if (playerinfo->seqcmd[ACMDL_STRAFE_L])
			{
				PlayerAnimSetLowerSeq(playerinfo, ASEQ_DASH_LEFT_GO);
				return;
			}
			else if (playerinfo->seqcmd[ACMDL_STRAFE_R])
			{
				PlayerAnimSetLowerSeq(playerinfo, ASEQ_DASH_RIGHT_GO);
				return;
			}
		}
		else
		{
			if (playerinfo->seqcmd[ACMDL_STRAFE_L])
			{
				PlayerAnimSetLowerSeq(playerinfo, ASEQ_STRAFEL);
				return;
			}
			else if (playerinfo->seqcmd[ACMDL_STRAFE_R])
			{
				PlayerAnimSetLowerSeq(playerinfo, ASEQ_STRAFER);
				return;
			}
		}
	}

	//Have we reversed directions of the strafe?
	if (playerinfo->seqcmd[ACMDL_STRAFE_R] && playerinfo->lowerseq == ASEQ_WSTRAFE_LEFT)
	{
		playerinfo->lowerseq = ASEQ_WSTRAFE_RIGHT;
		playerinfo->lowermove = PlayerSeqData[playerinfo->lowerseq].move;
		playerinfo->lowerframeptr = playerinfo->lowermove->frame + playerinfo->lowerframe;
		
		return;
	}

	if (playerinfo->seqcmd[ACMDL_STRAFE_L] && playerinfo->lowerseq == ASEQ_WSTRAFE_RIGHT)
	{
		playerinfo->lowerseq = ASEQ_WSTRAFE_LEFT;
		playerinfo->lowermove = PlayerSeqData[playerinfo->lowerseq].move;
		playerinfo->lowerframeptr = playerinfo->lowermove->frame + playerinfo->lowerframe;
		
		return;
	}

	//We're doing something else, so run a normal function to determine it
	PlayerActionCheckWalk(playerinfo);
}







/*

	Walk Back

*/







/*-----------------------------------------------
	PlayerActionCheckWalkBack
-----------------------------------------------*/

void PlayerActionCheckWalkBack( playerinfo_t *playerinfo )
{
	//Check for a transition to a walking or running strafe		[High probability]
	if ( playerinfo->seqcmd[ACMDL_BACK] && playerinfo->seqcmd[ACMDL_STRAFE_L] && playerinfo->lowerseq != ASEQ_WSTRAFEB_LEFT)
	{
		if (playerinfo->buttons & BUTTON_RUN)
		{
			//FIXME: There are no backwards run strafes!
			PlayerAnimSetLowerSeq(playerinfo, ASEQ_WSTRAFEB_LEFT);
			return;
		}
		else
		{
			playerinfo->lowerseq = ASEQ_WSTRAFEB_LEFT;
			playerinfo->lowermove = PlayerSeqData[playerinfo->lowerseq].move;
			playerinfo->lowerframeptr = playerinfo->lowermove->frame + playerinfo->lowerframe;
		
			return;
		}
	}

	//Check for a transition to a walking or running strafe		[High probability]
	if ( playerinfo->seqcmd[ACMDL_BACK] && playerinfo->seqcmd[ACMDL_STRAFE_R] && playerinfo->lowerseq != ASEQ_WSTRAFEB_RIGHT)
	{
		if (playerinfo->buttons & BUTTON_RUN)
		{
			//FIXME: There are no backwards run strafes!
			PlayerAnimSetLowerSeq(playerinfo, ASEQ_WSTRAFEB_RIGHT);
			return;
		}
		else
		{
			playerinfo->lowerseq = ASEQ_WSTRAFEB_RIGHT;
			playerinfo->lowermove = PlayerSeqData[playerinfo->lowerseq].move;
			playerinfo->lowerframeptr = playerinfo->lowermove->frame + playerinfo->lowerframe;
			
			return;
		}
	}

	//We're doing something else, so run a normal function to determine it
	PlayerActionCheckWalk(playerinfo);
}

/*-----------------------------------------------
	PlayerActionCheckWalkBackUnStrafe
-----------------------------------------------*/

void PlayerActionCheckWalkBackUnStrafe(playerinfo_t *playerinfo)
{
	//Still pressing the same way and still 
	if (playerinfo->seqcmd[ACMDL_BACK] && playerinfo->seqcmd[ACMDL_STRAFE_L] && playerinfo->lowerseq == ASEQ_WSTRAFEB_LEFT)
	{
		//Account for coincidental action
		PlayerActionCheckWalk(playerinfo);
		return;
	}

	if (playerinfo->seqcmd[ACMDL_BACK] && playerinfo->seqcmd[ACMDL_STRAFE_R] && playerinfo->lowerseq == ASEQ_WSTRAFEB_RIGHT)
	{
		//Account for coincidental action
		PlayerActionCheckWalk(playerinfo);
		return;
	}

	//Stopped moving forward, has gone to a side strafe
	if (!playerinfo->seqcmd[ACMDL_BACK])
	{
		if (playerinfo->seqcmd[ACMDL_STRAFE_L])
		{
			if (playerinfo->buttons & BUTTON_RUN)
				PlayerAnimSetLowerSeq(playerinfo, ASEQ_DASH_LEFT_GO);
			else
				PlayerAnimSetLowerSeq(playerinfo, ASEQ_STRAFEL);
			return;
		}
		else if (playerinfo->seqcmd[ACMDL_STRAFE_R])
		{
			if (playerinfo->buttons & BUTTON_RUN)
				PlayerAnimSetLowerSeq(playerinfo, ASEQ_DASH_RIGHT_GO);
			else
				PlayerAnimSetLowerSeq(playerinfo, ASEQ_STRAFER);
			return;
		}
	}

	//Have we reversed directions of the strafe?
	if (playerinfo->seqcmd[ACMDL_STRAFE_R] && playerinfo->lowerseq == ASEQ_WSTRAFEB_LEFT)
	{
		playerinfo->lowerseq = ASEQ_WSTRAFEB_RIGHT;
		playerinfo->lowermove = PlayerSeqData[playerinfo->lowerseq].move;
		playerinfo->lowerframeptr = playerinfo->lowermove->frame + playerinfo->lowerframe;
		
		return;
	}

	if (playerinfo->seqcmd[ACMDL_STRAFE_L] && playerinfo->lowerseq == ASEQ_WSTRAFEB_RIGHT)
	{
		playerinfo->lowerseq = ASEQ_WSTRAFEB_LEFT;
		playerinfo->lowermove = PlayerSeqData[playerinfo->lowerseq].move;
		playerinfo->lowerframeptr = playerinfo->lowermove->frame + playerinfo->lowerframe;
		
		return;
	}

	//We're doing something else, so run a normal function to determine it
	PlayerActionCheckWalk(playerinfo);
}








/*

	Run

*/








/*-----------------------------------------------
	PlayerActionCheckRun
-----------------------------------------------*/

void PlayerActionCheckRun(playerinfo_t *playerinfo)
{
	int	curseq = playerinfo->lowerseq;

	//Check for an upper sequence interruption due to a staff attack
	if ((	playerinfo->seqcmd[ACMDU_ATTACK] && 
			playerinfo->seqcmd[ACMDL_RUN_F]) &&  
			(playerinfo->pers.weaponready == WEAPON_READY_SWORDSTAFF) && 
			(!(playerinfo->flags & PLAYER_FLAG_NO_RARM)) &&
			!(playerinfo->edictflags & FL_CHICKEN))	
	{
		if (playerinfo->advancedstaff)
		{
			if (playerinfo->seqcmd[ACMDL_STRAFE_L] || playerinfo->seqcmd[ACMDL_STRAFE_R])
				PlayerAnimSetLowerSeq(playerinfo, ASEQ_WSWORD_SPIN2);
			else
				PlayerAnimSetLowerSeq(playerinfo, ASEQ_WSWORD_SPIN);
		}
		else
		{	// Not advanced staff
			if (playerinfo->irand(playerinfo,0,1))
				PlayerAnimSetLowerSeq(playerinfo, ASEQ_WSWORD_SPIN2);
			else
				PlayerAnimSetLowerSeq(playerinfo, ASEQ_WSWORD_SPIN);
		}
		return;
	}

	//Check for an autovault (only occurs if upper half of body is idle!)
	if ( (playerinfo->flags & PLAYER_FLAG_COLLISION) &&  (playerinfo->upperidle) && (playerinfo->seqcmd[ACMDL_FWD]) )
	{
		PlayerActionCheckVault(playerinfo, 0);
		
		if (curseq == ASEQ_VAULT_LOW)
		{
			PlayerAnimSetLowerSeq(playerinfo,  ASEQ_VAULT_LOW);
			return;
		}

		if (curseq == ASEQ_PULLUP_HALFWALL)
		{
			PlayerAnimSetLowerSeq(playerinfo,  ASEQ_PULLUP_HALFWALL);
			return;
		}
	}

	//Check for a transition to a walking or running strafe		[High probability]
	if ( playerinfo->seqcmd[ACMDL_FWD] && playerinfo->seqcmd[ACMDL_STRAFE_L] && curseq != ASEQ_RSTRAFE_LEFT)
	{
		if (playerinfo->buttons & BUTTON_RUN)
		{
			playerinfo->lowerseq = ASEQ_RSTRAFE_LEFT;
			playerinfo->lowermove = PlayerSeqData[playerinfo->lowerseq].move;
			playerinfo->lowerframeptr = playerinfo->lowermove->frame + playerinfo->lowerframe;
		
			return;
		}
		else
		{
			PlayerAnimSetLowerSeq(playerinfo, ASEQ_WSTRAFE_LEFT);
			return;
		}
	}
    
	//Check for a transition to a walking or running strafe		[High probability]
	if ( playerinfo->seqcmd[ACMDL_FWD] && playerinfo->seqcmd[ACMDL_STRAFE_R] && curseq != ASEQ_RSTRAFE_RIGHT)
	{
		if (playerinfo->buttons & BUTTON_RUN)
		{
			playerinfo->lowerseq = ASEQ_RSTRAFE_RIGHT;
			playerinfo->lowermove = PlayerSeqData[playerinfo->lowerseq].move;
			playerinfo->lowerframeptr = playerinfo->lowermove->frame + playerinfo->lowerframe;
			
			return;
		}
		else
		{
			PlayerAnimSetLowerSeq(playerinfo, ASEQ_WSTRAFE_RIGHT);
			return;
		}
	}

	//Check for a jump	[High probability]
	if (playerinfo->seqcmd[ACMDL_JUMP])
	{
		//Slime causes skipping, so no jumping in it!
		if (!(playerinfo->watertype & (CONTENTS_SLIME|CONTENTS_LAVA)))
		{
			if (playerinfo->seqcmd[ACMDL_FWD])
			{
				if (	(playerinfo->pers.weaponready == WEAPON_READY_SWORDSTAFF) &&
						(!(playerinfo->flags & PLAYER_FLAG_NO_RARM)) )
				{
					PlayerAnimSetLowerSeq(playerinfo,  ASEQ_POLEVAULT1_W);
					return;
				}
				else
				{
					PlayerAnimSetLowerSeq(playerinfo,  ASEQ_JUMPFWD_WGO);
					return;
				}
			}
			else if (playerinfo->seqcmd[ACMDL_BACK])
			{
				PlayerAnimSetLowerSeq(playerinfo,  ASEQ_JUMPBACK_WGO);
				return;
			}
			if (playerinfo->seqcmd[ACMDL_STRAFE_L])
			{
				PlayerAnimSetLowerSeq(playerinfo,  ASEQ_JUMPLEFT_WGO);
				return;
			}
			else if (playerinfo->seqcmd[ACMDL_STRAFE_R])
			{
				PlayerAnimSetLowerSeq(playerinfo,  ASEQ_JUMPRIGHT_WGO);
				return;
			}
		}
	}
    
	//Check for a sudden transition to a walk	[Low probability]
	if (playerinfo->seqcmd[ACMDL_WALK_F])
	{
		PlayerAnimSetLowerSeq(playerinfo,  ASEQ_WALKF);
		return;
	}

	//Check for a crouch	[Low probability]
	if (playerinfo->seqcmd[ACMDL_CROUCH])
	{
		if (playerinfo->seqcmd[ACMDL_BACK])
		{
			PlayerAnimSetLowerSeq(playerinfo,  ASEQ_ROLL_B);
			return;
		}
		else if (playerinfo->seqcmd[ACMDL_STRAFE_L])
		{
			PlayerAnimSetLowerSeq(playerinfo,  ASEQ_ROLL_L);
			return;
		}
		else if (playerinfo->seqcmd[ACMDL_STRAFE_R])
		{
			PlayerAnimSetLowerSeq(playerinfo,  ASEQ_ROLL_R);
			return;
		}
		else	
		{
			PlayerAnimSetLowerSeq(playerinfo,  ASEQ_ROLLDIVEF_W);
			return;
		}
	}
	
	//Handle an action key press	[Low probability]
	if (playerinfo->seqcmd[ACMDL_ACTION])
	{
		//Climb a rope?
		if ( (playerinfo->targetEnt) && (PlayerActionCheckRopeGrab(playerinfo,0)) )
		{
			playerinfo->flags |= PLAYER_FLAG_ONROPE;
			
			if(playerinfo->isclient)
				playerinfo->CL_Sound(SND_PRED_ID28,	playerinfo->origin, CHAN_VOICE, "player/ropegrab.wav", 0.75, ATTN_NORM, 0);
			else
				playerinfo->G_Sound(SND_PRED_ID28, playerinfo->leveltime, playerinfo->self, CHAN_VOICE, playerinfo->G_SoundIndex("player/ropegrab.wav"), 0.75, ATTN_NORM, 0);

			//We're on the rope
			PlayerAnimSetLowerSeq(playerinfo,  ASEQ_CLIMB_ON);
			return;
		}

		//Try and use a puzzle item
		PlayerActionUsePuzzle(playerinfo);
	}

	//Check for a quickturn		[Low probability]
	if (playerinfo->seqcmd[ACMDL_QUICKTURN])
	{
		PlayerAnimSetLowerSeq(playerinfo,  ASEQ_TURN180);
		return;
	}

	//Check for a sudden transition to a creep		[Low probability]
	if (playerinfo->seqcmd[ACMDL_CREEP_F])
	{
		PlayerAnimSetLowerSeq(playerinfo,  ASEQ_CREEPF);
		return;
	}

	//If we're pressing forward, and nothing else is happening, then we're walking forward
	if ( (playerinfo->seqcmd[ACMDL_RUN_F]) && (!playerinfo->seqcmd[ACMDL_STRAFE_L]) && (!playerinfo->seqcmd[ACMDL_STRAFE_R]) )
	{
		if (curseq != ASEQ_RUNF)
		{
			PlayerAnimSetLowerSeq(playerinfo, ASEQ_RUNF);
			return;
		}
	}

	//If we're pressing backward, and nothing else is happening, then we're walking backward
	if (playerinfo->seqcmd[ACMDL_BACK])
	{
		if (playerinfo->seqcmd[ACMDL_RUN_B])
		{
			if (!(playerinfo->seqcmd[ACMDU_ATTACK]) && playerinfo->upperidle)			
			{
				PlayerAnimSetLowerSeq(playerinfo, ASEQ_JUMPSPRINGBGO);
				return;
			}
		}
		else
		{
			PlayerAnimSetLowerSeq(playerinfo, ASEQ_WALKB);
			return;
		}
	}

	//All else has failed... did we just let go of everthing?
	if (!playerinfo->seqcmd[ACMDL_FWD] && !playerinfo->seqcmd[ACMDL_BACK])
	{
		PlayerAnimSetLowerSeq(playerinfo, SeqCtrl[playerinfo->lowerseq].ceaseseq);
		return;
	}
}

/*-----------------------------------------------
	PlayerActionCheckRunUnStrafe
-----------------------------------------------*/

void PlayerActionCheckRunUnStrafe(playerinfo_t *playerinfo)
{
	//Player has stopped running
	if (!(playerinfo->buttons & BUTTON_RUN))
	{
		if (playerinfo->seqcmd[ACMDL_STRAFE_L])
		{
			PlayerAnimSetLowerSeq(playerinfo, ASEQ_WSTRAFE_LEFT);
			return;
		}
		
		if (playerinfo->seqcmd[ACMDL_STRAFE_R])
		{
			PlayerAnimSetLowerSeq(playerinfo, ASEQ_WSTRAFE_RIGHT);
			return;
		}
	}

	//Still pressing the same way and still 
	if (playerinfo->seqcmd[ACMDL_FWD] && playerinfo->seqcmd[ACMDL_STRAFE_L] && playerinfo->lowerseq == ASEQ_RSTRAFE_LEFT)
	{
		//Account for coincidental action
		PlayerActionCheckRun(playerinfo);
		return;
	}

	if (playerinfo->seqcmd[ACMDL_FWD] && playerinfo->seqcmd[ACMDL_STRAFE_R] && playerinfo->lowerseq == ASEQ_RSTRAFE_RIGHT)
	{
		//Account for coincidental action
		PlayerActionCheckRun(playerinfo);
		return;
	}

	//Stopped moving forward, has gone to a side strafe
	if (!playerinfo->seqcmd[ACMDL_FWD])
	{
		if (playerinfo->buttons & BUTTON_RUN)
		{
			if (playerinfo->seqcmd[ACMDL_STRAFE_L])
			{
				PlayerAnimSetLowerSeq(playerinfo, ASEQ_DASH_LEFT);
				return;
			}
			else if (playerinfo->seqcmd[ACMDL_STRAFE_R])
			{
				PlayerAnimSetLowerSeq(playerinfo, ASEQ_DASH_RIGHT);
				return;
			}
		}
		else
		{
			if (playerinfo->seqcmd[ACMDL_STRAFE_L])
			{
				PlayerAnimSetLowerSeq(playerinfo, ASEQ_STRAFEL);
				return;
			}
			else if (playerinfo->seqcmd[ACMDL_STRAFE_R])
			{
				PlayerAnimSetLowerSeq(playerinfo, ASEQ_STRAFER);
				return;
			}
		}
	}

	//Have we reversed directions of the strafe?
	if (playerinfo->seqcmd[ACMDL_STRAFE_R] && playerinfo->lowerseq == ASEQ_RSTRAFE_LEFT)
	{
		playerinfo->lowerseq = ASEQ_RSTRAFE_RIGHT;
		playerinfo->lowermove = PlayerSeqData[playerinfo->lowerseq].move;
		playerinfo->lowerframeptr = playerinfo->lowermove->frame + playerinfo->lowerframe;
		
		return;
	}

	if (playerinfo->seqcmd[ACMDL_STRAFE_L] && playerinfo->lowerseq == ASEQ_RSTRAFE_RIGHT)
	{
		playerinfo->lowerseq = ASEQ_RSTRAFE_LEFT;
		playerinfo->lowermove = PlayerSeqData[playerinfo->lowerseq].move;
		playerinfo->lowerframeptr = playerinfo->lowermove->frame + playerinfo->lowerframe;
		
		return;
	}

	//We're doing something else, so run a normal function to determine it
	PlayerActionCheckRun(playerinfo);
}

void PlayerActionClimbStartSound(playerinfo_t *playerinfo, float value)
{
	assert(playerinfo);

	//1 in 5 chance of playing
	if (irand(0,4))
		return;

	if(playerinfo->isclient)
		playerinfo->CL_Sound(SND_PRED_ID29,playerinfo->origin, CHAN_VOICE, "*grab.wav", 0.75, ATTN_NORM, 0);
	else
		playerinfo->G_Sound(SND_PRED_ID29, playerinfo->leveltime, playerinfo->self, CHAN_VOICE, playerinfo->G_SoundIndex("*grab.wav"), 0.75, ATTN_NORM, 0);
}

void PlayerPlaySlide(playerinfo_t *playerinfo)
{
	assert(playerinfo);

	if(playerinfo->isclient)
		playerinfo->CL_Sound(SND_PRED_ID30,playerinfo->origin, CHAN_VOICE, "player/slope.wav", 0.75, ATTN_NORM, 0);
	else
		playerinfo->G_Sound(SND_PRED_ID30, playerinfo->leveltime, playerinfo->self, CHAN_VOICE, playerinfo->G_SoundIndex("player/slope.wav"), 0.75, ATTN_NORM, 0);
}


PLAYER_API void PlayerInterruptAction(playerinfo_t *playerinfo)
{
	PLAYER_API void TurnOffPlayerEffects(playerinfo_t *playerinfo);

	//Shut off player effects from weapons or the like
	TurnOffPlayerEffects(playerinfo);

	//Remove weapon sounds from the player (technically looping weapons should do this for us, but better safe than annoyed)
	if(playerinfo->isclient)
	{
		playerinfo->CL_Sound(SND_PRED_ID31,
							 playerinfo->origin,
							 CHAN_WEAPON,
							 "misc/null.wav",
							 1.0,
							 ATTN_NORM,
							 0);
	}
	else
	{
		playerinfo->G_Sound(SND_PRED_ID31,
							playerinfo->leveltime,	
							playerinfo->self,
							CHAN_WEAPON,
							playerinfo->G_SoundIndex("misc/null.wav"),
							1.0,
							ATTN_NORM,
							0);
	}

	//Release any held weapons
	if (playerinfo->pers.weapon->tag == ITEM_WEAPON_REDRAINBOW && playerinfo->upperseq == ASEQ_WRRBOW_HOLD)
	{
		playerinfo->PlayerActionRedRainBowAttack(playerinfo);
	}
	else if (playerinfo->pers.weapon->tag == ITEM_WEAPON_PHOENIXBOW && playerinfo->upperseq == ASEQ_WPHBOW_HOLD)
	{
		playerinfo->PlayerActionPhoenixBowAttack(playerinfo);
	}
	else if (playerinfo->pers.weapon->tag == ITEM_WEAPON_SPHEREOFANNIHILATION && playerinfo->chargingspell)
	{
		playerinfo->chargingspell=false;
		playerinfo->PlayerActionSpellSphereCreate(playerinfo, &playerinfo->chargingspell);
	}

	//Clear out any pending animations
	PlayerAnimSetUpperSeq(playerinfo, ASEQ_NONE);
}
