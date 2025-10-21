//
// p_actions.c
//
// Copyright 1998 Raven Software
//

#include "Player.h"
#include "p_types.h"
#include "p_actions.h"
#include "p_main.h"
#include "p_weapon.h"
#include "g_items.h"
#include "SurfaceProps.h"
#include "FX.h"
#include "Random.h"
#include "Vector.h"
#include "g_playstats.h"
#include "p_animactor.h"
#include "p_anim_branch.h" //mxd
#include "p_anim_data.h"
#include "p_utility.h" //mxd
#include "q_shared.h"

#define GRAB_HAND_HEIGHT	41
#define GRAB_HAND_WIDTH		8
#define GRAB_HAND_HORZONE	22
#define GRAB_HAND_VERTZONE	15

typedef enum //mxd
{
	GT_NONE,
	GT_GRAB,
	GT_SWING
} grabtype_e;

static const vec3_t handmins = { -2.0f, -2.0f, 0.0f };
static const vec3_t handmaxs = {  2.0f,  2.0f, 2.0f };

void PlayerActionCheckBranchRunningStrafe(playerinfo_t* info)
{
	const int seq = BranchLwrRunningStrafe(info);

	if (seq != ASEQ_NONE)
		PlayerAnimSetLowerSeq(info, seq);
}

void PlayerActionCheckStrafe(playerinfo_t* info)
{
	// Check forward advancement.
	if (info->seqcmd[ACMDL_FWD])
	{
		if (info->seqcmd[ACMDL_STRAFE_L])
			PlayerAnimSetLowerSeq(info, ASEQ_WSTRAFE_LEFT);
		else if (info->seqcmd[ACMDL_STRAFE_R])
			PlayerAnimSetLowerSeq(info, ASEQ_WSTRAFE_RIGHT);
		else
			PlayerAnimSetLowerSeq(info, ASEQ_WALKF);

		return;
	}

	// Check backward advancement.
	if (info->seqcmd[ACMDL_BACK])
	{
		if (info->seqcmd[ACMDL_STRAFE_L])
			PlayerAnimSetLowerSeq(info, ASEQ_WSTRAFEB_LEFT);
		else if (info->seqcmd[ACMDL_STRAFE_R])
			PlayerAnimSetLowerSeq(info, ASEQ_WSTRAFEB_RIGHT);
		else
			PlayerAnimSetLowerSeq(info, ASEQ_WALKB);

		return;
	}

	// Check for a jump.
	if (info->seqcmd[ACMDL_JUMP])
	{
		if (info->seqcmd[ACMDL_STRAFE_L])
		{
			PlayerAnimSetLowerSeq(info, ASEQ_JUMPLEFT_SGO);
			return;
		}

		if (info->seqcmd[ACMDL_STRAFE_R])
		{
			PlayerAnimSetLowerSeq(info, ASEQ_JUMPRIGHT_SGO);
			return;
		}
	}

	// Check for crouching.
	if (info->seqcmd[ACMDL_CROUCH])
	{
		if (info->seqcmd[ACMDL_STRAFE_L])
		{
			PlayerAnimSetLowerSeq(info, ASEQ_ROLL_L);
			return;
		}

		if (info->seqcmd[ACMDL_STRAFE_R])
		{
			PlayerAnimSetLowerSeq(info, ASEQ_ROLL_R);
			return;
		}
	}

	// Check for change in strafe direction.
	if (info->seqcmd[ACMDL_STRAFE_L] && info->lowerseq != ASEQ_STRAFEL)
	{
		PlayerAnimSetLowerSeq(info, ASEQ_STRAFEL);
		return;
	}

	if (info->seqcmd[ACMDL_STRAFE_R] && info->lowerseq != ASEQ_STRAFER)
	{
		PlayerAnimSetLowerSeq(info, ASEQ_STRAFER);
		return;
	}

	// We're just trying to go forward.
	if (!info->seqcmd[ACMDL_STRAFE_L] && !info->seqcmd[ACMDL_STRAFE_R])
	{
		if (info->seqcmd[ACMDL_CREEP_F])				// FORWARD
			PlayerAnimSetLowerSeq(info, ASEQ_CREEPF);
		else if (info->seqcmd[ACMDL_WALK_F])
			PlayerAnimSetLowerSeq(info, ASEQ_WALKF_GO);
		else if (info->seqcmd[ACMDL_RUN_F])
			PlayerAnimSetLowerSeq(info, ASEQ_RUNF_GO);
		else if (info->seqcmd[ACMDL_CREEP_B])			// BACKWARD
			PlayerAnimSetLowerSeq(info, ASEQ_CREEPB);
		else
			PlayerAnimSetLowerSeq(info, SeqCtrl[info->lowerseq].ceaseseq);
	}
}

void PlayerActionCheckVaultKick(playerinfo_t* info)
{
	if (!info->isclient)
		info->G_PlayerVaultKick(info);
}

//mxd. Added to reduce code duplication.
static qboolean CheckCreepMove(const playerinfo_t* info, const float creep_stepdist)
{
	// Scan out and down from the player.

	// Ignore the pitch of the player, we only want the yaw.
	const vec3_t ang = { 0.0f, info->angles[YAW], 0.0f };

	vec3_t forward;
	AngleVectors(ang, forward, NULL, NULL);

	// Trace ahead about one step.
	vec3_t start_pos;
	VectorMA(info->origin, creep_stepdist, forward, start_pos);

	// Account for stepheight.
	vec3_t mins;
	VectorCopy(info->mins, mins);
	mins[2] += CREEP_MAXFALL;

	// Trace forward to see if the path is clear.
	trace_t trace;
	P_Trace(info, info->origin, mins, info->maxs, start_pos, &trace); //mxd

	// If it is...
	if (trace.fraction == 1.0f)
	{
		// Move the endpoint down the maximum amount.
		vec3_t endpos;
		VectorCopy(start_pos, endpos);
		endpos[2] += info->mins[2] - CREEP_MAXFALL;

		// Trace down.
		P_Trace(info, start_pos, mins, info->maxs, endpos, &trace); //mxd

		return (trace.fraction < 1.0f && !trace.startsolid && !trace.allsolid);
	}

	return false;
}

static qboolean PlayerActionCheckCreepMoveForward(const playerinfo_t* info)
{
	return CheckCreepMove(info, CREEP_STEPDIST);
}

static qboolean PlayerActionCheckCreepMoveBack(const playerinfo_t *info)
{
	return CheckCreepMove(info, -CREEP_STEPDIST);
}

void PlayerActionSetCrouchHeight(playerinfo_t* info)
{
	info->maxs[2] = 4.0f;
}

void PlayerActionCheckUncrouchToFinishSeq(playerinfo_t* info)
{
	if (CheckUncrouch(info))
	{
		info->maxs[2] = 25.0f;
		return; // Ok to finish sequence.
	}

	const int seq = (info->upperseq != ASEQ_NONE ? info->upperseq : info->lowerseq);
	const int lower_seq = ((seq == ASEQ_FORWARD_FLIP_L || seq == ASEQ_FORWARD_FLIP_R) ? ASEQ_ROLL_FROM_FFLIP : ASEQ_CROUCH); // Choose a proper sequence to go into.

	PlayerAnimSetUpperSeq(info, ASEQ_NONE);
	PlayerAnimSetLowerSeq(info, lower_seq);
}

void PlayerActionTurn180(playerinfo_t* info) { } //TODO: remove?

void PlayerActionSetQTEndTime(playerinfo_t* info, float QTEndTime)
{
	info->quickturn_rate = -360.0f;
}

void PlayerActionCheckDoubleJump(playerinfo_t* info)
{
	//FIXME: Debounce!!!
	// Check to see if the player is still pressing jump, and is not trying to fire or grab a ledge (action).
	if (info->seqcmd[ACMDL_JUMP] && !info->seqcmd[ACMDU_ATTACK] && !info->seqcmd[ACMDL_ACTION])
	{
		switch (info->lowerseq)
		{
			case ASEQ_JUMPFWD:
				PlayerAnimSetLowerSeq(info, ASEQ_FORWARD_FLIP_L_GO);
				break;

			case ASEQ_JUMPBACK:
				PlayerAnimSetLowerSeq(info, ASEQ_JUMPFLIPBACK);
				break;

			case ASEQ_JUMPLEFT:
				PlayerAnimSetLowerSeq(info, ASEQ_JUMPFLIPLEFT);
				break;

			case ASEQ_JUMPRIGHT:
				PlayerAnimSetLowerSeq(info, ASEQ_JUMPFLIPRIGHT);
				break;
		}
	}
}

// This is called during the hold ready bow sequence, so that we may interrupt it if necessary.
void PlayerActionCheckBowRefire(playerinfo_t* info)
{
	const int num_shots = Weapon_CurrentShotsLeft(info); //mxd

	if (info->seqcmd[ACMDU_ATTACK] && num_shots > 0 && !(info->edictflags & FL_CHICKEN)) // Not a chicken.
	{
		// Shooting is one reason to end the bow refire waiting.
		if (info->pers.weapon->tag == ITEM_WEAPON_REDRAINBOW)
			PlayerAnimSetUpperSeq(info, ASEQ_WRRBOW_DRAW);
		else if (info->pers.weapon->tag == ITEM_WEAPON_PHOENIXBOW)
			PlayerAnimSetUpperSeq(info, ASEQ_WPHBOW_DRAW);
	}
	else if (info->switchtoweapon != info->pers.weaponready || info->pers.newweapon != NULL)
	{
		// Switching weapons is the other!
		if (info->pers.weapon->tag == ITEM_WEAPON_REDRAINBOW)
			PlayerAnimSetUpperSeq(info, ASEQ_WRRBOW_END);
		else if (info->pers.weapon->tag == ITEM_WEAPON_PHOENIXBOW)
			PlayerAnimSetUpperSeq(info, ASEQ_WPHBOW_END);
	}
	else if (!info->isclient && num_shots == 0)
	{
		info->G_WeapNext(info->self);
	}
}

void PlayerActionHandFXStart(playerinfo_t* info, const float value)
{
	const int handfx_type = (int)value; //mxd

	switch (handfx_type)
	{
		case HANDFX_FIREBALL:
			PlayerSetHandFX(info, handfx_type, 4);
			break;

		case HANDFX_MISSILE:
			PlayerSetHandFX(info, handfx_type, 6);
			break;

		case HANDFX_FIREWALL:
			PlayerSetHandFX(info, handfx_type, 10);
			break;
	}
}

void PlayerActionSphereTrailEnd(playerinfo_t* info, float value)
{
	// The sphere hand trails must be manually shut off.
	info->effects &= ~EF_TRAILS_ENABLED;
}

void PlayerActionSwordAttack(playerinfo_t* info, const float value)
{
	info->PlayerActionSwordAttack(info, (int)value);
}

void PlayerActionSpellFireball(playerinfo_t* info, float value)
{
	info->PlayerActionSpellFireball(info);
}

void PlayerActionSpellBlast(playerinfo_t* info, float value)
{
	info->PlayerActionSpellBlast(info);
}

void PlayerActionSpellArray(playerinfo_t* info, const float value)
{
	const int shots_left = Weapon_CurrentShotsLeft(info);
	const int missile_pos = (int)value; //mxd

	if (shots_left <= 0)
		return; // Outta ammo

	if (missile_pos == 2 && shots_left <= 3)
		return; // Three or less shots, use only the inner three projectiles.

	if (missile_pos == 1 && shots_left <= 1)
		return; // Only one shot, use the center projectile slot.

	info->PlayerActionSpellArray(info, missile_pos);
}

void PlayerActionSpellSphereCreate(playerinfo_t* info, float value)
{
	PlayerSetHandFX(info, HANDFX_SPHERE, -1);

	info->chargingspell = true;
	info->weaponcharge = 1;
	info->PlayerActionSpellSphereCreate(info, &info->chargingspell);
}

void PlayerActionSpellSphereCharge(playerinfo_t* info, const float value)
{
	const int shots_left = Weapon_CurrentShotsLeft(info); //mxd

	// Drain mana while charging. If mana depleted, then the branch will set to launch the thing.
	if (info->seqcmd[ACMDU_ATTACK] && info->weaponcharge < SPHERE_MAX_MANA_CHARGE &&
		(shots_left > 0 || info->pers.inventory.Items[info->weap_ammo_index] >= SPHERE_MANA_PER_CHARGE))
	{
		if (!(info->dmflags & DF_INFINITE_MANA)) //TODO: mxd. Shouldn't weaponcharge still be increased without taking mana when DF_INFINITE_MANA is set?
		{
			info->pers.inventory.Items[info->weap_ammo_index] -= SPHERE_MANA_PER_CHARGE;
			info->weaponcharge++;

			return;
		}
	}

	if (!info->seqcmd[ACMDU_ATTACK] || (shots_left == 0 && value != 4.0f))
	{
		// If we are out of ammo, or if we have let go of the button, then fire.
		switch ((int)value)
		{
			case 1: PlayerAnimSetUpperSeq(info, ASEQ_WSPHERE_FIRE1); break;
			case 2: PlayerAnimSetUpperSeq(info, ASEQ_WSPHERE_FIRE2); break;
			case 3: PlayerAnimSetUpperSeq(info, ASEQ_WSPHERE_FIRE3); break;
			case 4: PlayerAnimSetUpperSeq(info, ASEQ_WSPHERE_FIRE4); break;
		}
	}
}

void PlayerActionSpellSphereRelease(playerinfo_t* info, const float value)
{
	if (value != 1.0f || !info->seqcmd[ACMDU_ATTACK])
	{
		info->chargingspell = false;
		info->weaponcharge = 0;
	}
}

void PlayerActionSpellBigBall(playerinfo_t* info, float value)
{
	info->PlayerActionSpellBigBall(info);
}

void PlayerActionSpellFirewall(playerinfo_t* info, float value)
{
	info->PlayerActionSpellFirewall(info);
}

void PlayerActionRedRainBowAttack(playerinfo_t* info, float value)
{
	if (Weapon_CurrentShotsLeft(info) > 0)
		info->PlayerActionRedRainBowAttack(info);
}

void PlayerActionPhoenixBowAttack(playerinfo_t* info, float value)
{
	if (Weapon_CurrentShotsLeft(info) > 0)
		info->PlayerActionPhoenixBowAttack(info);
}

void PlayerActionHellstaffAttack(playerinfo_t* info, float value)
{
	if (Weapon_CurrentShotsLeft(info) > 0)
		info->PlayerActionHellstaffAttack(info);
}

void PlayerActionSpellDefensive(playerinfo_t* info, float value) { } //TODO: remove?

//mxd. Added to reduce code repetition...
static void SetupSpawnPoint(const playerinfo_t* info, vec3_t spawnpoint, vec3_t spawndir)
{
	vec3_t forward;

	AngleVectors(info->angles, forward, spawndir, NULL);
	VectorMA(info->origin, -2.0f, forward, spawnpoint);
	VectorMA(spawnpoint, -7.0f, spawndir, spawnpoint);
	spawnpoint[2] += info->viewheight - 16.0f;
}

//TODO: currently, this is only used when switching from a spell to another spell (ASEQ_HAND2HAND). Also use when switching to spell from a weapon?
void PlayerActionSpellChange(playerinfo_t* info, float value)
{
	vec3_t spawndir;
	vec3_t spawnpoint;
	int color;

	assert(info);

	if (info->edictflags & FL_CHICKEN) // Don't allow us to muck about with spells if we are a chicken.
		return;

	assert(info->pers.newweapon);

	Weapon_Ready(info, info->pers.newweapon);
	info->pers.newweapon = NULL;

	// Do some fancy effect.
	SetupSpawnPoint(info, spawnpoint, spawndir);

	switch (info->pers.weapon->tag)
	{
		case ITEM_WEAPON_FLYINGFIST:
			color = 1;
			break;

		case ITEM_WEAPON_MAGICMISSILE:
			color = 2;
			break;

		case ITEM_WEAPON_SPHEREOFANNIHILATION:
			color = 3;
			break;

		case ITEM_WEAPON_MACEBALLS:
			color = 4;
			break;

		case ITEM_WEAPON_FIREWALL:
			color = 5;
			break;

		default:
			color = 0;
			break;
	}

	P_Sound(info, SND_PRED_ID0, CHAN_WEAPON, "Weapons/SpellChange.wav", 1.0f); //mxd
	P_CreateEffect(info, EFFECT_PRED_ID1, NULL, FX_SPELL_CHANGE, 0, spawnpoint, "db", spawndir, color); //mxd
}

//TODO: currently, this is only used when switching from a bow to another bow (ASEQ_BOW2BOW). Also use when switching to bow from a spell/weapon?
void PlayerActionArrowChange(playerinfo_t* info, float value)
{
	vec3_t spawndir;
	vec3_t spawnpoint;
	int color;

	assert(info);

	if (info->edictflags & FL_CHICKEN) // Don't allow us to muck about with arrows if we are a chicken.
		return;

	assert(info->pers.newweapon);

	Weapon_Ready(info, info->pers.newweapon);
	info->pers.newweapon = NULL;

	const gitem_t* weapon = info->pers.weapon;
	if (weapon == NULL) //mxd. Don't trigger sound / effects when no weapon.
		return;

	// Do some fancy effect.
	SetupSpawnPoint(info, spawnpoint, spawndir); //mxd

	if (weapon->tag == ITEM_WEAPON_PHOENIXBOW)
	{
		// Set the bow type to phoenix.
		info->pers.bowtype = BOW_TYPE_PHOENIX;
		color = 6;
	}
	else
	{
		// Set the bow type to red-rain.
		info->pers.bowtype = BOW_TYPE_REDRAIN;
		color = 7;
	}

	PlayerUpdateModelAttributes(info);

	P_Sound(info, SND_PRED_ID1, CHAN_WEAPON, "Weapons/SpellChange.wav", 1.0f); //mxd
	P_CreateEffect(info, EFFECT_PRED_ID2, NULL, FX_SPELL_CHANGE, 0, spawnpoint, "db", spawndir, color); //mxd
}

// By default changes weapon to the weapon indicated by switchtoweapon.
// If a value is passed, then instead it will simply change the appearance, not the actual weapon.
void PlayerActionWeaponChange(playerinfo_t* info, const float value)
{
	vec3_t spawndir;
	vec3_t spawnpoint;

	assert(info);

	if (info->edictflags & FL_CHICKEN) // Don't allow us to muck about with spells if we are a chicken.
		return;

	if (value != 0.0f)
	{
		// Don't REALLY change the weaponready, since the value indicates a cosmetic weapon change only (the real change is later).
		const int holdweapon = info->pers.weaponready;

		info->pers.weaponready = (int)value;
		PlayerUpdateModelAttributes(info);

		info->pers.weaponready = holdweapon;
	}
	else
	{
		assert(info->pers.newweapon);
		Weapon_Ready(info, info->pers.newweapon);

		info->pers.weaponready = info->switchtoweapon;
		PlayerUpdateModelAttributes(info);

		info->pers.newweapon = NULL;
	}

	if (info->leveltime <= 1.0f)
		return;

	// Weapon Changing effects.
	switch (info->pers.weaponready)
	{
		case WEAPON_READY_SWORDSTAFF:
			P_CreateEffect(info, EFFECT_PRED_ID3, info->self, FX_STAFF_CREATEPOOF, CEF_OWNERS_ORIGIN, NULL, ""); //mxd
			break;

		case WEAPON_READY_HELLSTAFF:
			P_CreateEffect(info, EFFECT_PRED_ID4, info->self, FX_STAFF_CREATEPOOF, CEF_OWNERS_ORIGIN | CEF_FLAG6, NULL, ""); //mxd
			break;

		case WEAPON_READY_BOW:
			// Make sure we have the right bow color
			if (info->pers.weapon == NULL)
				return;

			// There is a weapon.
			if (info->pers.weapon->tag == ITEM_WEAPON_REDRAINBOW)
			{
				// Make sure we have the redrain visible.
				if (info->pers.bowtype == BOW_TYPE_PHOENIX)
				{
					// Uh oh, change the phoenix into a red rain.
					info->pers.bowtype = BOW_TYPE_REDRAIN;
					PlayerUpdateModelAttributes(info);

					// Do some fancy effect.
					SetupSpawnPoint(info, spawnpoint, spawndir); //mxd
					P_Sound(info, SND_PRED_ID2, CHAN_WEAPON, "Weapons/SpellChange.wav", 1.0f); //mxd
					P_CreateEffect(info, EFFECT_PRED_ID5, NULL, FX_SPELL_CHANGE, 0, spawnpoint, "db", spawndir, 6); //mxd
				}
			}
			else if (info->pers.weapon->tag == ITEM_WEAPON_PHOENIXBOW)
			{
				// Make sure we have the phoenix visible.
				if (info->pers.bowtype == BOW_TYPE_REDRAIN)
				{
					// Uh oh, change the red rain to a phoenix.
					info->pers.bowtype = BOW_TYPE_PHOENIX;
					PlayerUpdateModelAttributes(info);

					// Do some fancy effect.
					SetupSpawnPoint(info, spawnpoint, spawndir); //mxd
					P_Sound(info, SND_PRED_ID3, CHAN_WEAPON, "Weapons/SpellChange.wav", 1.0f); //mxd
					P_CreateEffect(info, EFFECT_PRED_ID6, NULL, FX_SPELL_CHANGE, 0, spawnpoint, "db", spawndir, 7); //mxd
				}
			}
			break;

		default: // No nothing.
			break;
	}
}

void PlayerActionStartStaffGlow(const playerinfo_t* info, const float value)
{
	int flags = CEF_OWNERS_ORIGIN;
	const int ready_weapon = (int)value; //mxd

	if (ready_weapon == WEAPON_READY_HELLSTAFF)
		flags |= CEF_FLAG6;

	if (info->pers.stafflevel == STAFF_LEVEL_BASIC || ready_weapon == WEAPON_READY_HELLSTAFF)
	{
		P_Sound(info, SND_PRED_ID5, CHAN_WEAPON, "weapons/Staff Ready.wav", 1.0f); //mxd
	}
	else if (info->pers.stafflevel == STAFF_LEVEL_POWER1) // Blue
	{
		flags |= CEF_FLAG7;
		P_Sound(info, SND_PRED_ID6, CHAN_WEAPON, "weapons/Staff2Ready.wav", 1.0f); //mxd
	}
	else if (info->pers.stafflevel == STAFF_LEVEL_POWER2) // Flame
	{
		flags |= CEF_FLAG8;
		P_Sound(info, SND_PRED_ID7, CHAN_WEAPON, "weapons/Staff3Ready.wav", 1.0f); //mxd
	}
	else
	{
		P_Sound(info, SND_PRED_ID8, CHAN_WEAPON, "weapons/Staff Ready.wav", 1.0f); //mxd
	}

	P_CreateEffect(info, EFFECT_PRED_ID7, info->self, FX_STAFF_CREATE, flags, NULL, ""); //mxd
}

void PlayerActionEndStaffGlow(const playerinfo_t* info, const float value)
{
	int flags = CEF_OWNERS_ORIGIN;
	const int ready_weapon = (int)value; //mxd

	if (ready_weapon == WEAPON_READY_HELLSTAFF)
		flags |= CEF_FLAG6;

	if (info->pers.stafflevel == STAFF_LEVEL_BASIC || ready_weapon == WEAPON_READY_HELLSTAFF)
	{
		P_Sound(info, SND_PRED_ID9, CHAN_WEAPON, "weapons/Staff Unready.wav", 1.0f); //mxd
	}
	else if (info->pers.stafflevel == STAFF_LEVEL_POWER1) // Blue
	{
		flags |= CEF_FLAG7;
		P_Sound(info, SND_PRED_ID10, CHAN_WEAPON, "weapons/Staff2Unready.wav", 1.0f); //mxd
	}
	else if (info->pers.stafflevel == STAFF_LEVEL_POWER2) // Flame
	{
		flags |= CEF_FLAG8;
		P_Sound(info, SND_PRED_ID11, CHAN_WEAPON, "weapons/Staff3Unready.wav", 1.0f); //mxd
	}
	else
	{
		P_Sound(info, SND_PRED_ID12, CHAN_WEAPON, "weapons/Staff Unready.wav", 1.0f); //mxd
	}

	P_CreateEffect(info, EFFECT_PRED_ID8, info->self, FX_STAFF_REMOVE, flags, NULL, ""); //mxd
}

static void PlayerActionStaffTrailSound(const playerinfo_t* info, const char* name) //mxd. Originally named 'PlayerActionSwordTrailSound'.
{
	P_Sound(info, SND_PRED_ID13, CHAN_WEAPON, name, 1.0f); //mxd
}

void PlayerActionStaffTrailStart(playerinfo_t* info, const float value) //mxd. Originally named 'PlayerActionSwordTrailStart'.
{
	static int traillength[TRAIL_MAX] = //mxd. Made local static.
	{
		7,	//	TRAIL_SPIN1,
		6,	//	TRAIL_SPIN2,
		7,	//	TRAIL_STAND,
		6,	//	TRAIL_STEP,
		4,	//	TRAIL_BACK,
		8,	//	TRAIL_STAB,
		6,	//	TRAIL_COUNTERLEFT,
		6,	//	TRAIL_COUNTERRIGHT,
	};

	const int trail_type = (int)value; //mxd

	// Add a trail effect to the staff.
	int powerlevel = info->pers.stafflevel;
	if (info->powerup_timer > info->leveltime)
		powerlevel++;

	powerlevel = min(powerlevel, STAFF_LEVEL_MAX - 1);

	assert(trail_type >= 0 && trail_type < TRAIL_MAX);
	const int length = traillength[trail_type];

	PlayerSetHandFX(info, HANDFX_STAFF1 + powerlevel - 1, length);

	const qboolean spin = (trail_type == TRAIL_SPIN1 || trail_type == TRAIL_SPIN2 || trail_type == TRAIL_STAB);
	const char* sound_type = (spin ? "stafftwirl" : "staffswing"); //mxd

	switch (powerlevel)
	{
		case STAFF_LEVEL_BASIC: // Normal Staff
			PlayerActionStaffTrailSound(info, va("weapons/%s.wav", sound_type));
			break;

		case STAFF_LEVEL_POWER1: // Energy staff
			PlayerActionStaffTrailSound(info, va("weapons/%s_2.wav", sound_type));
			break;

		case STAFF_LEVEL_POWER2: // Fire Staff
			PlayerActionStaffTrailSound(info, va("weapons/%s_3.wav", sound_type));
			break;
	}
}

void PlayerActionRedRainBowTrailStart(playerinfo_t* info, float value)
{
	// Add a trail effect to the red-rain bow.
	const int fx_type = (info->powerup_timer > info->leveltime ? HANDFX_POWERREDRAIN : HANDFX_REDRAIN); // Power up the Arrow FX?
	PlayerSetHandFX(info, fx_type, -1);
	P_LocalSound(info, "weapons/bowReady.wav"); //mxd
}

void PlayerActionPhoenixBowTrailStart(playerinfo_t* info, float value)
{
	// Add a trail effect to the phoenix bow.
	const int fx_type = (info->powerup_timer > info->leveltime ? HANDFX_POWERPHOENIX : HANDFX_PHOENIX); // Power up the Arrow FX?
	PlayerSetHandFX(info, fx_type, -1);
	P_LocalSound(info, "weapons/PhoenixReady.wav"); //mxd
}

void PlayerActionBowTrailEnd(playerinfo_t* info, float value)
{
	info->effects &= ~EF_TRAILS_ENABLED;
}

void PlayerActionFootstep(const playerinfo_t* info, const float value)
{
	char walk_sound[128];
	int basestep;
	int channel;

	const int absstep = (int)value;

	// If we're not on the ground, skip all the rest.
	if (absstep == STEP_ROLL && info->groundentity == NULL)
		return;

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

	if (basestep == 0)
		return;

	if (info->waterlevel > 0)
	{
		const char* snd_name = (basestep == STEP_RUN ? "waterrun" : "waterwalk");
		strcpy_s(walk_sound, sizeof(walk_sound), va("player/%s%i.wav", snd_name, irand(1, 2)));
	}
	else
	{
		const char* material = GetClientGroundSurfaceMaterialName(info);

		if (material == NULL)
			return;

		strcpy_s(walk_sound, sizeof(walk_sound), "player/");
		strcat_s(walk_sound, sizeof(walk_sound), material);

		switch (basestep)
		{
			case STEP_CREEP: // Creep
				strcat_s(walk_sound, sizeof(walk_sound), va("shuffle%i.wav", irand(1, 2)));
				break;

			case STEP_WALK: // Walk
				strcat_s(walk_sound, sizeof(walk_sound), va("walk%i.wav", irand(1, 2)));
				break;

			case STEP_RUN: // Run
				strcat_s(walk_sound, sizeof(walk_sound), va("run%i.wav", irand(1, 2)));
				break;

			case STEP_ROLL: // Roll
				strcat_s(walk_sound, sizeof(walk_sound), "roll.wav");
				break;
		}
	}

	P_Sound(info, SND_PRED_ID14, channel, walk_sound, 0.75f); //mxd
}

void PlayerActionSwimIdleSound(const playerinfo_t* info, float value)
{
	P_Sound(info, SND_PRED_ID15, CHAN_VOICE, "player/swim idle.wav", 0.4f); //mxd
}

void PlayerActionSwimSound(const playerinfo_t* info, const float value)
{
	char* name;
	const int snd_type = (int)value; //mxd

	switch (snd_type)
	{
		case SOUND_SWIM_FORWARD:
			name = "player/breaststroke.wav";
			break;

		case SOUND_SWIM_BACK:
			name = "player/Swim Backward.wav";
			break;

		case SOUND_SWIM_SIDE:
			name = va("player/SwimFreestyle%i.wav", irand(1, 2));
			break;

		case SOUND_SWIM_UNDER:
			name = "player/swimunder.wav";
			break;

		default:
			assert(0);
			return;
	}

	P_Sound(info, SND_PRED_ID16, CHAN_AUTO, name, 0.6f); //mxd
}

void PlayerActionClimbWallSound(const playerinfo_t* info, float value)
{
	P_Sound(info, SND_PRED_ID18, CHAN_VOICE, "player/pullup 1.wav", 1.0f); //mxd
}

void PlayerActionClimbFinishSound(const playerinfo_t* info, float value)
{
	P_Sound(info, SND_PRED_ID19, CHAN_VOICE, "player/pullup 2.wav", 1.0f); //mxd
}

void PlayerActionSwim(const playerinfo_t* info, const float value)
{
	if (value != 1.0f)
		return;

	vec3_t origin;
	VectorCopy(info->origin, origin);
	origin[2] += info->waterheight;

	// Fixme: Need to determine the actual water surface normal - if we have any sloping water?
	vec3_t dir = { 0.0f, 0.0f, 1.0f };
	P_CreateEffect(info, EFFECT_PRED_ID9, NULL, FX_WATER_ENTRYSPLASH, 0, origin, "bd", 32, dir);
}

static grabtype_e GetGrabType(playerinfo_t* info, const float v_adjust)
{
	trace_t grabtrace;
	vec3_t forward;
	vec3_t right;
	vec3_t endpoint;

	assert(info);

	//	Criteria:
	//	--The wall should be vertical (we already know that any surface collided with is vertical.
	//	--It needs a horizontal (or nearly horizontal) top edge.
	//	--The top edge of the plane must be lower than the "grab" height of the player's arms at the time of impact.
	//	--The plane adjacent to the top edge of the collided-with plane must also be nearly horizontal.
	//	--The player's angle must be almost straight towards the collided-with plane.
	//	--The plane must be at least wide enough at the point of intersection for the whole player.

	// Now we want to cast some rays. If the two rays moving from the player's hands at "grab" width
	// successfully clear any surface, then at least his hands are free enough to make the grab.

	const vec3_t player_facing = { 0.0f, info->angles[YAW], 0.0f};
	AngleVectors(player_facing, forward, right, NULL);

	// Check right hand position.
	vec3_t righthand;
	VectorMA(info->origin, GRAB_HAND_WIDTH, right, righthand);
	righthand[2] += v_adjust;

	VectorMA(righthand, GRAB_HAND_HORZONE, forward, endpoint);

	P_Trace(info, righthand, handmins, handmaxs, endpoint, &grabtrace); //mxd

	// Right hand is not clear.
	if (grabtrace.fraction != 1.0f || grabtrace.startsolid || grabtrace.allsolid)
		return GT_NONE;

	VectorCopy(grabtrace.endpos, righthand);

	// Check left hand position.
	vec3_t lefthand;
	VectorMA(info->origin, -GRAB_HAND_WIDTH, right, lefthand);
	lefthand[2] += v_adjust;

	VectorMA(lefthand, GRAB_HAND_HORZONE, forward, endpoint);

	P_Trace(info, lefthand, handmins, handmaxs, endpoint, &grabtrace); //mxd

	// Left hand is not clear.
	if (grabtrace.fraction != 1.0f || grabtrace.startsolid || grabtrace.allsolid)
		return GT_NONE;

	VectorCopy(grabtrace.endpos, lefthand);

	// If the clear rays from the player's hands, traced down, should hit a legal (almost level)
	// surface within a certain distance, then a grab is possible!

	// First we must figure out how far down to look.

	// If the player is going down, then check his intended speed over the next .1 sec.
	// If the player is going up, then check his velocity over the LAST .1 sec.
	float vertlength = fabsf(info->sv_gravity * 0.5f + fabsf(info->velocity[2])) * 0.1f;
	vertlength = max(GRAB_HAND_VERTZONE, vertlength);

	// Check right hand position.
	VectorCopy(righthand, endpoint);
	endpoint[2] -= vertlength;

	P_Trace(info, righthand, handmins, handmaxs, endpoint, &grabtrace); //mxd

	// Right hand did not connect with a flat surface.
	if (grabtrace.fraction == 1.0f || grabtrace.startsolid || grabtrace.allsolid)
		return GT_NONE;

	// Hand stopped, but not on a grabbable surface.
	if (!(grabtrace.contents & MASK_SOLID))
		return GT_NONE;

	VectorCopy(grabtrace.endpos, righthand);

	// Check left hand position.
	VectorCopy(lefthand, endpoint);
	endpoint[2] -= vertlength;

	P_Trace(info, lefthand, handmins, handmaxs, endpoint, &grabtrace); //mxd

	// Left hand did not connect with a flat surface.
	if (grabtrace.fraction == 1.0f || grabtrace.plane.normal[2] < 0.8f || grabtrace.startsolid || grabtrace.allsolid)
		return GT_NONE;

	// Hand stopped, but not on a grabbable surface.
	if (!(grabtrace.contents & MASK_SOLID))
		return GT_NONE;

	VectorCopy(grabtrace.endpos, lefthand);

	// Now finally, if we try tracing the player blocking forward a tad, we should be hitting an obstruction.

	vec3_t playermin;
	vec3_t playermax;
	VectorCopy(info->mins, playermin);
	VectorCopy(info->maxs, playermax);

	// We need to take the player limits and extend them up to 83 in height.

	playermax[2] = GRAB_HAND_HEIGHT;
	VectorMA(info->origin, GRAB_HAND_HORZONE, forward, endpoint);

	P_Trace(info, info->origin, playermin, playermax, endpoint, &grabtrace); //mxd

	// Player body did not connect with a flat surface.
	if (grabtrace.fraction == 1.0f || grabtrace.startsolid || grabtrace.allsolid)
		return GT_NONE;

	// Body stopped, but not on a grabbable surface.
	if (!(grabtrace.contents & MASK_SOLID) || grabtrace.plane.normal[2] > 0.0f)
		return GT_NONE;

	// Don't grab buttons!
	if (grabtrace.ent != NULL && !info->isclient && info->G_EntIsAButton(grabtrace.ent))
		return GT_NONE;

	// Now check the angle. It should be pretty much opposite the player's yaw.

	vec3_t planedir;
	vectoangles(grabtrace.plane.normal, planedir);
	info->grabangle = anglemod(planedir[YAW] - 180.0f);

	const float yaw = anglemod(planedir[YAW] - info->angles[YAW]) - 180.0f;

	// Bad angle. Player should bounce.
	if (yaw > 30.0f || yaw < -30.0f)
		return GT_NONE;

	// One more check, make sure we can fit in there, so we don't start climbing then fall back down.

	// Get the z height
	vec3_t lastcheck_start;
	VectorCopy(info->origin, lastcheck_start);
	lastcheck_start[2] = max(lefthand[2], righthand[2]) - info->mins[2];

	vec3_t lastcheck_end;
	VectorAdd(lastcheck_start, forward, lastcheck_end);

	trace_t lasttrace;
	P_Trace(info, lastcheck_start, info->mins, info->maxs, lastcheck_end, &lasttrace); //mxd

	if (lasttrace.fraction < 1.0f || lasttrace.startsolid || lasttrace.allsolid)
		return GT_NONE;

	// Now see if the surface is eligible for a overhanging swing vault.
	// Trace from about the player's waist to his feet to determine this.
	vec3_t mins;
	vec3_t maxs;
	VectorCopy(info->mins, mins);
	VectorCopy(info->maxs, maxs);
	maxs[2] -= 48.0f;

	AngleVectors(info->angles, forward, NULL, NULL);
	VectorMA(info->origin, 32.0f, forward, endpoint);

	trace_t swingtrace;
	P_Trace(info, info->origin, mins, maxs, endpoint, &swingtrace); //mxd

	// Did we hit a wall underneath?
	const qboolean swingable = (swingtrace.fraction == 1.0f && (!swingtrace.startsolid || !swingtrace.allsolid));

	// Save the intended grab location (the endpoint).
	for (int i = 0; i < 3; i++)
		info->grabloc[i] = (lefthand[i] + righthand[i]) / 2.0f;

	endpoint[0] = grabtrace.endpos[0];
	endpoint[1] = grabtrace.endpos[1];
	endpoint[2] = info->grabloc[2] - v_adjust;

	P_Trace(info, info->origin, NULL, NULL, endpoint, &grabtrace); //mxd

	if (grabtrace.fraction == 1.0f)
	{
		VectorCopy(endpoint, info->origin);
		info->offsetangles[YAW] = -(ClampAngleDeg(info->angles[YAW]) - info->grabangle);
		info->angles[YAW] = info->grabangle;

		return (swingable ? GT_SWING : GT_GRAB);
	}

	return GT_NONE;
}

void PlayerActionCheckGrab(playerinfo_t* info, float value)
{
	static float v_adjusts[] = { GRAB_HAND_HEIGHT - 58, GRAB_HAND_HEIGHT - 29, GRAB_HAND_HEIGHT }; //mxd. Height adjusts for each height zone.

	if (!info->upperidle)
		return;

	int maxcheck;
	if (!(info->flags & PLAYER_FLAG_NO_LARM) && !(info->flags & PLAYER_FLAG_NO_RARM))
		maxcheck = 3; // All checks ok.
	else if ((info->flags & PLAYER_FLAG_NO_LARM) && (info->flags & PLAYER_FLAG_NO_RARM))
		maxcheck = 1; // Only check ankle height.
	else
		maxcheck = 2; // Up to waist height.

	for (int check = 0; check < maxcheck; check++)
	{
		// Check 3 height zones for 3 results.
		const grabtype_e type = GetGrabType(info, v_adjusts[check]);
		if (type != GT_NONE)
		{
			if (check < 2)
				PlayerAnimSetVault(info, ASEQ_PULLUP_HALFWALL);
			else
				PlayerAnimSetVault(info, (type == GT_SWING ? ASEQ_OVERHANG : ASEQ_PULLUP_WALL));

			return;
		}
	}
}

void PlayerActionCheckFallingGrab(playerinfo_t* info, const float value)
{
	if (info->seqcmd[ACMDL_FWD] || info->seqcmd[ACMDL_JUMP] || info->seqcmd[ACMDL_ACTION])
		PlayerActionCheckGrab(info, value);
}

void PlayerActionBowReadySound(const playerinfo_t* info, float value)
{
	P_Sound(info, SND_PRED_ID20, CHAN_WEAPON, "weapons/bowdraw2.wav", 1.0f);
}

qboolean PlayerActionUsePuzzle(const playerinfo_t* info)
{
	if (!info->isclient)
		return info->G_PlayerActionUsePuzzle(info);

	return false; // Client does nothing.
}

qboolean PlayerActionCheckPuzzleGrab(playerinfo_t* info)
{
	if (!info->isclient)
		return info->G_PlayerActionCheckPuzzleGrab(info);

	return false; // Client does nothing.
}

qboolean PlayerActionCheckJumpGrab(playerinfo_t* info, float value)
{
#define GRAB_JUMP_HORZONE	32
#define GRAB_JUMP_HEIGHT	80

	//FIXME: make like others
	assert(info);

	// Check for grabbability.
	// Criteria:
	// --The wall should be vertical (we already know that any surface collided with is vertical.
	// --It needs a horizontal (or nearly horizontal) top edge.
	// --The top edge of the plane must be lower than the "grab" height of the player's arms at the time of impact.
	// --The plane adjacent to the top edge of the collided-with plane must also be nearly horizontal.
	// --The player's angle must be almost straight towards the collided-with plane.
	// --The plane must be at least wide enough at the point of intersection for the whole player.

	// Skip it if we're not on the ground.
	if (info->groundentity == NULL)
		return false;

	// First we need to "move" the player as high as we possibly can.
	vec3_t endpoint;
	VectorCopy(info->origin, endpoint);
	endpoint[2] += GRAB_JUMP_HEIGHT;

	vec3_t playermin;
	vec3_t playermax;
	VectorCopy(info->mins, playermin);
	VectorCopy(info->maxs, playermax);

	// We need to take the player limits and extend them up to 83 in height (where the hands are).
	playermax[2] = GRAB_HAND_HEIGHT;

	trace_t grabtrace;
	P_Trace(info, info->origin, playermin, playermax, endpoint, &grabtrace); //mxd

	// There's no room to jump.
	if (grabtrace.startsolid || grabtrace.allsolid)
		return false;

	// Handheight is set to the maximum the hands can go above the current player's height.
	const float handheight = grabtrace.endpos[2] + GRAB_HAND_HEIGHT;

	// Now we want to cast some rays. If the two rays moving from the player's hands at "grab" width
	// successfully clear any surface, then at least his hands are free enough to make the grab.
	vec3_t forward;
	vec3_t right;
	const vec3_t player_facing = { 0.0f, info->angles[YAW], 0.0f };
	AngleVectors(player_facing, forward, right, NULL);

	// Check right hand position.
	vec3_t righthand;
	VectorMA(info->origin, GRAB_HAND_WIDTH, right, righthand);
	righthand[2] = handheight;

	VectorMA(righthand, GRAB_JUMP_HORZONE, forward, endpoint);

	P_Trace(info, righthand, handmins, handmaxs, endpoint, &grabtrace); //mxd

	if (grabtrace.fraction != 1.0f) // Right hand is not clear.
		return false;

	VectorCopy(grabtrace.endpos, righthand);

	// Check left hand position.
	vec3_t lefthand;
	VectorMA(info->origin, -GRAB_HAND_WIDTH, right, lefthand);
	lefthand[2] = handheight;

	VectorMA(lefthand, GRAB_JUMP_HORZONE, forward, endpoint);

	P_Trace(info, lefthand, handmins, handmaxs, endpoint, &grabtrace); //mxd

	// Left hand is not clear.
	if (grabtrace.fraction != 1.0f)
		return false;

	VectorCopy(grabtrace.endpos, lefthand);

	// If the clear rays from the player's hands, traced down, should hit a legal (almost level)
	// surface within a certain distance, then a grab is possible!

	// Check right hand position.
	VectorCopy(righthand, endpoint);
	endpoint[2] = info->origin[2] + GRAB_HAND_HEIGHT;

	P_Trace(info, righthand, handmins, handmaxs, endpoint, &grabtrace); //mxd

	// Right hand did not connect with a flat surface.
	if (grabtrace.fraction == 1.0f || grabtrace.plane.normal[2] < 0.5f)
		return false;

	// Right hand stopped, but not on a grabbable surface.
	if (!(grabtrace.contents & MASK_SOLID))
		return false;

	VectorCopy(grabtrace.endpos, righthand);

	// Check left hand position.
	VectorCopy(lefthand, endpoint);
	endpoint[2] = info->origin[2] + GRAB_HAND_HEIGHT;

	P_Trace(info, lefthand, handmins, handmaxs, endpoint, &grabtrace); //mxd

	// Left hand did not connect with a flat surface.
	if (grabtrace.fraction == 1.0f || grabtrace.plane.normal[2] < 0.5f)
		return false;

	// Left hand stopped, but not on a grabbable surface.
	if (!(grabtrace.contents & MASK_SOLID))
		return false;

	VectorCopy(grabtrace.endpos, lefthand);

	// Now, finally, if we try tracing the player blocking forward a tad, we should be hitting an obstruction.
	VectorCopy(info->mins, playermin);
	VectorCopy(info->maxs, playermax);

	// We need to take the player limits and extend them up to 83 in height.
	playermax[2] = GRAB_HAND_HEIGHT;

	VectorMA(info->origin, GRAB_JUMP_HORZONE, forward, endpoint);

	P_Trace(info, info->origin, NULL, NULL, endpoint, &grabtrace); //mxd

	// Now, if the player is grabbing an overhang, this will not hit anything.
	if (grabtrace.fraction == 1.0f)
	{
		// Hit nothing, so do some more checks, by stretching the player up to the outcropping.

		// Bottom at the origin.
		playermin[2] = 0.0f;

		// Stretch all the way up to where the grab is made.
		playermax[2] = (lefthand[2] + righthand[2]) * 0.5f - info->origin[2];

		P_Trace(info, info->origin, playermin, playermax, endpoint, &grabtrace); //mxd

		if (grabtrace.fraction == 1.0f)
			return false;
	}

	// Player stopped, but not on a grabbable surface.
	if (!(grabtrace.contents & MASK_SOLID) || grabtrace.startsolid || grabtrace.plane.normal[2] > 0.0f)
		return false;

	// Now check the angle.  It should be pretty much opposite the player's yaw.
	vec3_t planedir;
	vectoangles(grabtrace.plane.normal, planedir);

	info->grabangle = anglemod(planedir[YAW] - 180.0f);
	const float yaw = anglemod(planedir[YAW] - info->angles[YAW]) - 180.0f;

	// Bad angle. Player should bounce.
	if (yaw > 30.0f || yaw < -30.0f)
		return false;

	// Now that we feel we have a viable jump, let's jump!
	return true;
}

qboolean PlayerActionCheckVault(playerinfo_t* info) //mxd. Removed unused 'value' arg.
{
#define VAULT_HAND_WIDTH	4
#define VAULT_HAND_HEIGHT	32
#define VAULT_HAND_VERTZONE 48
#define VAULT_HAND_HORZONE	24

	assert(info);

	// Check in front of the player, and decide if there is a suitable wall here.
	vec3_t forward;
	vec3_t right;
	const vec3_t player_facing = { 0.0f, info->angles[YAW], 0.0f};
	AngleVectors(player_facing, forward, right, NULL);

	vec3_t vaultcheckmins;
	vec3_t vaultcheckmaxs;
	VectorCopy(info->mins, vaultcheckmins);
	VectorCopy(info->maxs, vaultcheckmaxs);
	vaultcheckmins[2] += 18.0f; // Don't try to vault stairs.

	vec3_t start;
	VectorCopy(info->origin, start);

	vec3_t end;
	VectorMA(start, VAULT_HAND_HORZONE, forward, end);

	trace_t grabtrace;
	P_Trace(info, start, vaultcheckmins, vaultcheckmaxs, end, &grabtrace); //mxd

	// Body stopped, but not on a grabbable surface.
	if (grabtrace.fraction == 1.0f || !(grabtrace.contents & MASK_SOLID))
		return false;

	// Sloped surfaces are not grabbable. Question: sloped away or towards?
	if (grabtrace.plane.normal[2] > 0.3f)
		return false;

	// Don't grab buttons.
	if (grabtrace.ent != NULL && !info->isclient && info->G_EntIsAButton(grabtrace.ent))
		return false;

	// Now check the angle. It should be pretty much opposite the player's yaw.
	vec3_t planedir;
	vectoangles(grabtrace.plane.normal, planedir);
	info->grabangle = anglemod(planedir[YAW] - 180.0f);

	const float yaw = anglemod(planedir[YAW] - info->angles[YAW]) - 180.0f;

	// Bad angle. Player should bounce.
	if (yaw > 30.0f || yaw < -30.0f)
		return false;

	vec3_t grabloc;
	VectorCopy(grabtrace.endpos, grabloc);

	// Now we want to cast some rays. If the two rays moving from the player's hands at "grab" width
	// successfully clear any surface, then at least his hands are free enough to make the grab.

	// Check right hand position.
	vec3_t righthand;
	VectorMA(info->origin, VAULT_HAND_WIDTH, right, righthand);
	righthand[2] += VAULT_HAND_HEIGHT;

	vec3_t endpoint;
	VectorMA(righthand, VAULT_HAND_HORZONE, forward, endpoint);

	P_Trace(info, righthand, handmins, handmaxs, endpoint, &grabtrace); //mxd

	// Right hand is not clear.
	if (grabtrace.fraction != 1.0f || grabtrace.startsolid || grabtrace.allsolid)
		return false;

	VectorCopy(grabtrace.endpos, righthand);

	// Check left hand position.
	vec3_t lefthand;
	VectorMA(info->origin, -VAULT_HAND_WIDTH, right, lefthand);
	lefthand[2] += VAULT_HAND_HEIGHT;

	VectorMA(lefthand, VAULT_HAND_HORZONE, forward, endpoint);

	P_Trace(info, lefthand, handmins, handmaxs, endpoint, &grabtrace); //mxd

	// Left hand is not clear.
	if (grabtrace.fraction != 1.0f || grabtrace.startsolid || grabtrace.allsolid)
		return false;

	VectorCopy(grabtrace.endpos, lefthand);

	// If the clear rays from the player's hands, traced down, should hit a legal (almost level) surface
	// within a certain distance, then a grab is possible! First we must figure out how low to look.
	// If the player is going down, then check his intended speed over the next .1 sec.

	// Check right hand position.
	VectorCopy(righthand, endpoint);
	endpoint[2] -= VAULT_HAND_VERTZONE;

	P_Trace(info, righthand, handmins, handmaxs, endpoint, &grabtrace); //mxd

	// Right hand did not connect with a flat surface.
	if (grabtrace.fraction == 1.0f || grabtrace.plane.normal[2] < 0.8f || grabtrace.startsolid || grabtrace.allsolid)
		return false;

	// Right hand stopped, but not on a grabbable surface.
	if (!(grabtrace.contents & MASK_SOLID))
		return false;

	VectorCopy(grabtrace.endpos, righthand);

	// Check left hand position.
	VectorCopy(lefthand, endpoint);
	endpoint[2] -= VAULT_HAND_VERTZONE;

	P_Trace(info, lefthand, handmins, handmaxs, endpoint, &grabtrace); //mxd

	// Left hand did not connect with a flat surface.
	if (grabtrace.fraction == 1.0f || grabtrace.plane.normal[2] < 0.8f || grabtrace.startsolid || grabtrace.allsolid)
		return false;

	// Left hand stopped, but not on a grabbable surface.
	if (!(grabtrace.contents & MASK_SOLID))
		return false;

	VectorCopy(grabtrace.endpos, lefthand);

	// The fit check doesn't work when you are in muck, so check if you are.
	if (!(info->watertype & (CONTENTS_SLIME | CONTENTS_LAVA)))
	{
		// One more check, make sure we can fit in there, so we don't start climbing then fall back down.

		// Get the z height.
		vec3_t lastcheck_start;
		VectorCopy(info->origin, lastcheck_start);
		lastcheck_start[2] = max(lefthand[2], righthand[2]) - info->mins[2];

		vec3_t lastcheck_end;
		VectorAdd(lastcheck_start, forward, lastcheck_end);

		trace_t lasttrace;
		P_Trace(info, lastcheck_start, info->mins, info->maxs, lastcheck_end, &lasttrace); //mxd

		if (lasttrace.fraction < 1.0f || lasttrace.startsolid || lasttrace.allsolid)
			return false;
	}

	//mxd. Removed unused overhanging swing vault check logic.

	// Save the intended grab location (the endpoint).
	info->grabloc[0] = (lefthand[0] + righthand[0]) / 2.0f;
	info->grabloc[1] = (lefthand[1] + righthand[1]) / 2.0f;
	info->grabloc[2] = max(lefthand[2], righthand[2]);

	if (grabtrace.fraction < 0.5f)
	{
		if ((info->flags & PLAYER_FLAG_NO_LARM) && (info->flags & PLAYER_FLAG_NO_RARM))
			return false; // Can't do half pull up with no arms.

		// This is strange, but the VAULT_LOW is actually a high wall vault.
		PlayerAnimSetVault(info, ASEQ_VAULT_LOW);
	}
	else
	{
		// ...and PULLUP_HALFWALL is just a hop, so I moved the arm check to the high wall vault.
		PlayerAnimSetVault(info, ASEQ_PULLUP_HALFWALL);
	}

	return true;
}

qboolean PlayerActionCheckRopeGrab(playerinfo_t* info, const float stomp_org)
{
	// Check dismemberment before game side rope check.
	if (!info->isclient && !(info->flags & PLAYER_FLAG_NO_LARM) && !(info->flags & PLAYER_FLAG_NO_RARM))
		return info->G_PlayerActionCheckRopeGrab(info, stomp_org);

	return false;
}

void PlayerActionVaultSound(const playerinfo_t* info, float value)
{
	const char* material = GetClientGroundSurfaceMaterialName(info);

	if (material != NULL)
	{
		char vault_sound[64];
		strcpy_s(vault_sound, sizeof(vault_sound), va("player/%svault.wav", material));
		P_Sound(info, SND_PRED_ID21, CHAN_WEAPON, vault_sound, 1.0f);
	}
}

//mxd. Added to reduce code repetition.
static qboolean CanJump(const playerinfo_t* info)
{
	if (info->waterlevel > 1) // Don't jump while under water.
		return false;

	if (info->groundentity != NULL)
		return true;

	trace_t trace;
	const vec3_t endpos = { info->origin[0], info->origin[1], info->origin[2] + info->mins[2] - 2.0f };
	P_Trace(info, info->origin, info->mins, info->maxs, endpos, &trace);

	return trace.fraction < 0.2f;
}

void PlayerActionJump(playerinfo_t* info, const float value)
{
 	if (CanJump(info))
		info->upvel = value * 10.0f;
}

void PlayerActionJumpBack(playerinfo_t* info, float value) //TODO: actually change and use value (currently 250)?
{
	if (CanJump(info))
		info->upvel = 150.0f;
}

void PlayerActionFlip(playerinfo_t* info, const float value)
{
	info->flags |= PLAYER_FLAG_USE_ENT_POS;
	info->velocity[2] += value;
}

void PlayerClimbingMoveFunc(playerinfo_t* info, const float height, const float var2, const float var3)
{
	assert(info);

	if (!info->isclient)
		info->G_PlayerClimbingMoveFunc(info, height, var2, var3);
}

void PlayerMoveFunc(playerinfo_t* info, const float fwd, const float right, const float up)
{
	// Feeds velocity into the character as a thrust value, like player control (no effect if in the air).
	info->fwdvel = fwd;
	info->sidevel = right;
	info->upvel = up;
}

void PlayerSwimMoveFunc(playerinfo_t* info, const float fwd, const float right, const float up)
{
	PlayerMoveFunc(info, fwd, right, up);

	if (info->seqcmd[ACMDL_STRAFE_L])
		info->sidevel -= (fabsf(fwd) / 1.25f);
	else if (info->seqcmd[ACMDL_STRAFE_R])
		info->sidevel += (fabsf(fwd) / 1.25f);
}

void PlayerMoveUpperFunc(playerinfo_t* info, const float fwd, const float right, const float up)
{
	if (info->loweridle)
		PlayerMoveFunc(info, fwd, right, up);
}

void PlayerMoveForce(playerinfo_t* info, const float fwd, const float right, const float up)
{
	// For things like jumps and the like, where the velocity is demanded, not a suggestion.
	vec3_t fwdv;
	vec3_t rightv;

	AngleVectors(info->angles, fwdv, rightv, NULL);
	VectorScale(fwdv, fwd, info->velocity);

	if (right != 0.0f)
		VectorMA(info->velocity, right, rightv, info->velocity);

	info->velocity[2] += up;
}

void PlayerJumpMoveForce(playerinfo_t* info, float fwd, const float right, const float up)
{
	// For things like jumps and the like, where the velocity is demanded, not a suggestion.
	vec3_t fwdv;
	vec3_t rightv;
	vec3_t angles;

	//INFO: Same as PlayerMoveForce, but uses where the player is looking (torso).
	VectorCopy(info->aimangles, angles);
	angles[PITCH] = 0.0f;

	AngleVectors(angles, fwdv, rightv, NULL);

	// Speedup powerup active?
	if (info->effects & EF_SPEED_ACTIVE)
		fwd *= RUN_MULT;

	VectorScale(fwdv, fwd, info->velocity);

	// Check to see if we should bother.
	if (right != 0.0f)
		VectorMA(info->velocity, right, rightv, info->velocity);

	// If the player is strafing, move the player in that direction (diagonal jump).
	if (fwd != 0.0f)
	{
		const float scale = ((info->buttons & BUTTON_RUN) ? 260.0f : 140.0f);

		// DON'T do this during a normal side jump.
		if (info->seqcmd[ACMDL_STRAFE_R] && !info->seqcmd[ACMDL_STRAFE_L])
			VectorMA(info->velocity, scale, rightv, info->velocity);
		else if (info->seqcmd[ACMDL_STRAFE_L] && !info->seqcmd[ACMDL_STRAFE_R])
			VectorMA(info->velocity, -scale, rightv, info->velocity);
	}

	info->velocity[2] += up;
}

void PlayerJumpNudge(playerinfo_t* info, const float fwd, const float right, const float up)
{
	vec3_t vf;
	vec3_t vr;
	vec3_t vu;
	vec3_t vel;

	AngleVectors(info->angles, vf, vr, vu);

	VectorCopy(info->velocity, vel);
	VectorNormalize(vel);

	// Get the dot products of the main directions
	const float df = DotProduct(vf, vel);
	const float dr = DotProduct(vr, vel);
	const float du = DotProduct(vu, vel);

	// Forward fraction of the velocity.
	VectorScale(info->velocity, df, vel);
	const float ff = VectorLength(vel);

	// Right fraction of the velocity.
	VectorScale(info->velocity, dr, vel);
	const float fr = VectorLength(vel);

	// Up fraction of the velocity.
	VectorScale(info->velocity, du, vel);
	const float fu = VectorLength(vel);

	// If we're under the minimum, set the velocity to that minimum.
	if (fabsf(ff) < fabsf(fwd))
		VectorMA(info->velocity, fwd, vf, info->velocity);

	if (fabsf(fr) < fabsf(right))
		VectorMA(info->velocity, right, vr, info->velocity);

	if (fabsf(fu) < fabsf(up))
		VectorMA(info->velocity, up, vu, info->velocity);

	// Cause the player to use this velocity.
	info->flags |= PLAYER_FLAG_USE_ENT_POS;
}

void PlayerMoveALittle(playerinfo_t* info, const float fwd, float right, float up)
{
	float scaler;

	if (info->seqcmd[ACMDL_FWD] || info->seqcmd[ACMDL_ACTION])
		scaler = fwd;
	else if (info->seqcmd[ACMDL_BACK])
		scaler = -fwd;
	else
		return;

	vec3_t fwdv;
	info->flags |= PLAYER_FLAG_USE_ENT_POS;
	AngleVectors(info->angles, fwdv, NULL, NULL);
	VectorMA(info->velocity, scaler, fwdv, info->velocity);
}

void PlayerPullupHeight(playerinfo_t* info, const float height, const float endseq, float nopushdown)
{
	assert(info);

	if (endseq > 0.0f)
	{
		// End Sequence.
		const vec3_t end_pos = { info->grabloc[0], info->grabloc[1], info->grabloc[2] - (info->mins[2] + 2.0f) };

		// Trace towards grabloc.
		trace_t trace;
		P_Trace(info, info->origin, info->mins, info->maxs, end_pos, &trace); //mxd
		VectorCopy(trace.endpos, info->origin);

		if (info->seqcmd[ACMDL_WALK_F])
			PlayerAnimSetLowerSeq(info, ASEQ_WALKF);
		else if (info->seqcmd[ACMDL_RUN_F])
			PlayerAnimSetLowerSeq(info, ASEQ_RUNF);
		else if (info->seqcmd[ACMDL_JUMP])
			PlayerAnimSetLowerSeq(info, ASEQ_JUMPSTD_GO);
		else
			PlayerAnimSetLowerSeq(info, ASEQ_WALKF_GO);
	}
	else if (info->grabloc[2] - height > info->origin[2])
	{
		vec3_t end_pos = { info->origin[0], info->origin[1], info->grabloc[2] - height };

		// Trace upwards.
		trace_t trace;
		P_Trace(info, info->origin, info->mins, info->maxs, end_pos, &trace); //mxd

		if (trace.fraction < 1.0f)
		{
			// We bumped into something so drop down.
			PlayerAnimSetLowerSeq(info, ASEQ_FALL);
			return;
		}

		vec3_t save_pos;
		VectorCopy(end_pos, save_pos);

		// Trace towards the wall.
		vec3_t forward;
		AngleVectors(info->angles, forward, NULL, NULL);
		VectorMA(trace.endpos, 32.0f, forward, end_pos);

		// Try to move to the correct distance away from the wall.
		P_Trace(info, trace.endpos, info->mins, info->maxs, end_pos, &trace); //mxd

		//mxd. Add 'trace.fraction > 0.0f' and trace.plane.normal checks. Both will be zero when brush entity (func_wall, func_plat etc.) was hit...
		if (trace.fraction > 0.0f && trace.fraction < 1.0f && Vec3NotZero(trace.plane.normal))
		{
			const float x = fabsf(trace.plane.normal[0]);
			const float y = fabsf(trace.plane.normal[1]);
			const float diff = fabsf(x - y); // [0.0 .. 1.0]

			VectorMA(trace.endpos, diff * 4.0f, trace.plane.normal, info->origin);
		}
		else
		{
			// Just move upwards.
			VectorCopy(save_pos, info->origin);
		}
	}
}

qboolean PlayerActionCheckPushButton(const playerinfo_t* info)
{
	if (!info->isclient)
		return info->G_PlayerActionCheckPushButton(info);

	return false;
}

void PlayerActionPushButton(playerinfo_t* info, float value)
{
	if (!info->isclient)
		info->G_PlayerActionPushButton(info);
}

qboolean PlayerActionCheckPushLever(const playerinfo_t* info)
{
	if (!info->isclient)
		return info->G_PlayerActionCheckPushLever(info);

	return false;
}

void PlayerActionPushLever(playerinfo_t* info, float value)
{
	if (!info->isclient)
		info->G_PlayerActionPushLever(info);
}

void PlayerActionTakePuzzle(playerinfo_t* info, float value)
{
	if (!info->isclient)
		info->G_PlayerActionTakePuzzle(info);
}

void PlayerActionShrineEffect(playerinfo_t* info, float value)
{
	if (!info->isclient)
		info->G_PlayerActionShrineEffect(info);
}

void PlayerMoveAdd(playerinfo_t* info)
{
#define AIRMOVE_AMOUNT		48
#define AIRMOVE_THRESHOLD	64

	vec3_t vf;
	vec3_t vr;
	vec3_t dir;

	// If we're not nudging, then just return (this probably doesn't save us too much time...)
	if (!info->seqcmd[ACMDL_FWD] && !info->seqcmd[ACMDL_BACK] && !info->seqcmd[ACMDL_STRAFE_L] && !info->seqcmd[ACMDL_STRAFE_R])
		return;

	// Setup the information.
	AngleVectors(info->angles, vf, vr, NULL);
	VectorCopy(info->velocity, dir);
	const float mag = VectorNormalize(dir);

	const float fmove = (DotProduct(dir, vf) * mag);
	const float rmove = (DotProduct(dir, vr) * mag);

	// Check and apply the nudges.
	if (info->seqcmd[ACMDL_FWD] && fmove < AIRMOVE_THRESHOLD)
		VectorMA(info->velocity, AIRMOVE_AMOUNT, vf, info->velocity);

	if (info->seqcmd[ACMDL_BACK] && fmove > -AIRMOVE_THRESHOLD)
		VectorMA(info->velocity, -AIRMOVE_AMOUNT, vf, info->velocity);

	if (info->seqcmd[ACMDL_STRAFE_L] && rmove > -AIRMOVE_THRESHOLD)
		VectorMA(info->velocity, -AIRMOVE_AMOUNT, vr, info->velocity);

	if (info->seqcmd[ACMDL_STRAFE_R] && rmove < AIRMOVE_THRESHOLD)
		VectorMA(info->velocity, AIRMOVE_AMOUNT, vr, info->velocity);

	// Use the velocity to move the player.
	info->flags |= PLAYER_FLAG_USE_ENT_POS;
}

void PlayerActionDrownFloatUp(playerinfo_t* info) { } //TODO: remove?

void PlayerActionCheckRopeMove(playerinfo_t* info, float foo)
{
	if (!info->isclient)
		info->G_PlayerActionCheckRopeMove(info);
}

PLAYER_API void PlayerReleaseRope(playerinfo_t* info)
{
	assert(info);
	info->flags &= ~PLAYER_FLAG_ONROPE;
}

PLAYER_API void KnockDownPlayer(playerinfo_t* info)
{
	assert(info);

	// Chicken cannot be knocked down.
	if (!(info->flags & (EF_CHICKEN | PLAYER_FLAG_KNOCKDOWN)))
		info->flags |= PLAYER_FLAG_KNOCKDOWN;
}

PLAYER_API void PlayFly(const playerinfo_t* info, float dist)
{
	P_Sound(info, SND_PRED_ID22, CHAN_BODY, "player/idle buzz.wav", 1.0f); //mxd
}

PLAYER_API void PlaySlap(const playerinfo_t* info, float dist)
{
	P_Sound(info, SND_PRED_ID23, CHAN_BODY, "player/idle slap.wav", 1.0f); //mxd
}

PLAYER_API void PlaySigh(const playerinfo_t* info, float dist)
{
	P_Sound(info, SND_PRED_ID24, CHAN_BODY, "*phew.wav", 0.75f); //mxd
}

PLAYER_API void PlayScratch(const playerinfo_t* info, float dist)
{
	P_Sound(info, SND_PRED_ID25, CHAN_BODY, "player/scratch.wav", 1.0f); //mxd
}

PLAYER_API void SpawnDustPuff(playerinfo_t* info, float dist)
{
	if (info->waterlevel == 0)
		P_CreateEffect(info, EFFECT_PRED_ID10, info->self, FX_DUST_PUFF, CEF_OWNERS_ORIGIN, info->origin, ""); //mxd
}

//mxd. Added to reduce code repetition...
static void SetLowerSeq(playerinfo_t* info, const int seq)
{
	info->lowerseq = seq;
	info->lowermove = PlayerSeqData[info->lowerseq].move;
	info->lowerframeptr = info->lowermove->frame + info->lowerframe;
}

void PlayerActionCheckCreep(playerinfo_t* info)
{
	const int curseq = info->lowerseq;

	// Check for an autovault (only occurs if upper half of body is idle!).
	if ((info->flags & PLAYER_FLAG_COLLISION) && info->upperidle && info->seqcmd[ACMDL_FWD])
	{
		PlayerActionCheckVault(info);

		if (curseq == ASEQ_VAULT_LOW || curseq == ASEQ_PULLUP_HALFWALL)
		{
			PlayerAnimSetLowerSeq(info, curseq);
			return;
		}
	}

	// Check for a jump [High probability]. Slime causes skipping, so no jumping in it!
	if (info->seqcmd[ACMDL_JUMP] && !(info->watertype & (CONTENTS_SLIME | CONTENTS_LAVA)))
	{
		if (info->seqcmd[ACMDL_FWD])
		{
			PlayerAnimSetLowerSeq(info, ASEQ_JUMPFWD_SGO);
			return;
		}

		if (info->seqcmd[ACMDL_BACK])
		{
			PlayerAnimSetLowerSeq(info, ASEQ_JUMPBACK_SGO);
			return;
		}

		if (info->seqcmd[ACMDL_STRAFE_L])
		{
			PlayerAnimSetLowerSeq(info, ASEQ_JUMPLEFT_SGO);
			return;
		}

		if (info->seqcmd[ACMDL_STRAFE_R])
		{
			PlayerAnimSetLowerSeq(info, ASEQ_JUMPRIGHT_SGO);
			return;
		}
	}

	// Check for a transition to a creeping strafe [High probability].
	if (info->seqcmd[ACMDL_CREEP_F] && info->seqcmd[ACMDL_STRAFE_L] && curseq != ASEQ_CSTRAFE_LEFT)
	{
		SetLowerSeq(info, ASEQ_CSTRAFE_LEFT); //mxd
		return;
	}

	if (info->seqcmd[ACMDL_CREEP_F] && info->seqcmd[ACMDL_STRAFE_R] && curseq != ASEQ_CSTRAFE_RIGHT)
	{
		SetLowerSeq(info, ASEQ_CSTRAFE_RIGHT); //mxd
		return;
	}

	// Check for a transition to a creeping strafe [High probability].
	if (info->seqcmd[ACMDL_CREEP_B] && info->seqcmd[ACMDL_STRAFE_L] && curseq != ASEQ_CSTRAFEB_LEFT)
	{
		SetLowerSeq(info, ASEQ_CSTRAFEB_LEFT); //mxd
		return;
	}

	if (info->seqcmd[ACMDL_CREEP_B] && info->seqcmd[ACMDL_STRAFE_R] && curseq != ASEQ_CSTRAFEB_RIGHT)
	{
		SetLowerSeq(info, ASEQ_CSTRAFEB_RIGHT); //mxd
		return;
	}

	// Check for a sudden transition to a walk [Low probability].
	if (info->seqcmd[ACMDL_WALK_F])
	{
		PlayerAnimSetLowerSeq(info, ASEQ_WALKF);
		return;
	}

	// Check for a sudden transition to a run [Low probability].
	if (info->seqcmd[ACMDL_RUN_F])
	{
		PlayerAnimSetLowerSeq(info, ASEQ_RUNF);
		return;
	}

	// Check for a crouch [Low probability].
	if (info->seqcmd[ACMDL_CROUCH])
	{
		if (info->seqcmd[ACMDL_BACK])
			PlayerAnimSetLowerSeq(info, ASEQ_CROUCH_WALK_B);
		else if (info->seqcmd[ACMDL_STRAFE_L])
			PlayerAnimSetLowerSeq(info, ASEQ_CROUCH_WALK_L);
		else if (info->seqcmd[ACMDL_STRAFE_R])
			PlayerAnimSetLowerSeq(info, ASEQ_CROUCH_WALK_R);
		else if (info->seqcmd[ACMDL_FWD]) //BUGFIX: mxd. ACMDL_BACK in original logic.
			PlayerAnimSetLowerSeq(info, ASEQ_CROUCH_WALK_F);
		else
			PlayerAnimSetLowerSeq(info, ASEQ_CROUCH_GO);

		return;
	}

	// Handle an action key press [Low probability].
	if (info->seqcmd[ACMDL_ACTION])
	{
		// Climb a rope?
		if (info->targetEnt != NULL && PlayerActionCheckRopeGrab(info, 0))
		{
			info->flags |= PLAYER_FLAG_ONROPE;
			P_Sound(info, SND_PRED_ID26, CHAN_VOICE, "player/ropegrab.wav", 0.75f);
			PlayerAnimSetLowerSeq(info, ASEQ_CLIMB_ON); // We're on the rope

			return;
		}
	}

	// Check for a quickturn [Low probability].
	if (info->seqcmd[ACMDL_QUICKTURN])
	{
		PlayerAnimSetLowerSeq(info, ASEQ_TURN180);
		return;
	}

	// If we're pressing forward, and nothing else is happening, then we're walking forward.
	if (info->seqcmd[ACMDL_CREEP_F] && !info->seqcmd[ACMDL_STRAFE_L] && !info->seqcmd[ACMDL_STRAFE_R])
	{
		const qboolean can_move = PlayerActionCheckCreepMoveForward(info); //mxd

		if (curseq != ASEQ_CREEPF && can_move)
		{
			PlayerAnimSetLowerSeq(info, ASEQ_CREEPF);
			return;
		}

		if (!can_move)
		{
			PlayerAnimSetLowerSeq(info, SeqCtrl[info->lowerseq].ceaseseq);
			return;
		}
	}

	// If we're pressing backward, and nothing else is happening, then we're walking backward.
	if (info->seqcmd[ACMDL_CREEP_B] && !info->seqcmd[ACMDL_STRAFE_L] && !info->seqcmd[ACMDL_STRAFE_R])
	{
		const qboolean can_move = PlayerActionCheckCreepMoveBack(info); //mxd

		if (curseq != ASEQ_CREEPB && can_move)
		{
			PlayerAnimSetLowerSeq(info, ASEQ_CREEPB);
			return;
		}

		if (!can_move)
		{
			PlayerAnimSetLowerSeq(info, SeqCtrl[info->lowerseq].ceaseseq);
			return;
		}
	}

	// All else has failed... did we just let go of everything?
	if (!info->seqcmd[ACMDL_FWD] && !info->seqcmd[ACMDL_BACK])
		PlayerAnimSetLowerSeq(info, SeqCtrl[info->lowerseq].ceaseseq);
}

void PlayerActionCheckCreepUnStrafe(playerinfo_t* info)
{
	const qboolean is_running = (info->buttons & BUTTON_RUN); //mxd
	const qboolean is_creeping = (info->buttons & BUTTON_CREEP); //mxd
	int tgtseq = ASEQ_NONE; //mxd

	// Player has started running.
	if (is_running)
	{
		if (info->seqcmd[ACMDL_STRAFE_L])
			tgtseq = ASEQ_RSTRAFE_LEFT;
		else if (info->seqcmd[ACMDL_STRAFE_R])
			tgtseq = ASEQ_RSTRAFE_RIGHT;
	}
	else if (!is_creeping)
	{
		if (info->seqcmd[ACMDL_STRAFE_L])
			tgtseq = ASEQ_WSTRAFE_LEFT;
		else if (info->seqcmd[ACMDL_STRAFE_R])
			tgtseq = ASEQ_WSTRAFE_RIGHT;
	}

	if (tgtseq != ASEQ_NONE)
	{
		PlayerAnimSetLowerSeq(info, tgtseq);
		return;
	}

	// Still pressing the same way and still strafing.
	if (info->seqcmd[ACMDL_FWD])
	{
		if ((info->seqcmd[ACMDL_STRAFE_L] && info->lowerseq == ASEQ_CSTRAFE_LEFT) ||
			(info->seqcmd[ACMDL_STRAFE_R] && info->lowerseq == ASEQ_CSTRAFE_RIGHT))
		{
			// Account for coincidental action.
			PlayerActionCheckCreep(info);
			return;
		}
	}

	// Stopped moving forward, has gone to a side strafe.
	if (!info->seqcmd[ACMDL_FWD])
	{
		if (info->seqcmd[ACMDL_STRAFE_L])
			tgtseq = (is_running ? ASEQ_DASH_LEFT_GO : ASEQ_STRAFEL); // Check for a transfer to a run.
		else if (info->seqcmd[ACMDL_STRAFE_R])
			tgtseq = (is_running ? ASEQ_DASH_RIGHT_GO : ASEQ_STRAFER); // Check for a transfer to a run.

		if (tgtseq != ASEQ_NONE)
		{
			PlayerAnimSetLowerSeq(info, tgtseq);
			return;
		}
	}

	// Have we reversed directions of the strafe?
	if (info->seqcmd[ACMDL_STRAFE_R] && info->lowerseq == ASEQ_CSTRAFE_LEFT)
		tgtseq = ASEQ_CSTRAFE_RIGHT;
	else if (info->seqcmd[ACMDL_STRAFE_L] && info->lowerseq == ASEQ_CSTRAFE_RIGHT)
		tgtseq = ASEQ_CSTRAFE_LEFT;

	if (tgtseq != ASEQ_NONE)
	{
		SetLowerSeq(info, tgtseq); //mxd
		return;
	}

	// We're doing something else, so run a normal function to determine it.
	PlayerActionCheckCreep(info);
}

void PlayerActionCheckCreepBack(playerinfo_t* info) //TODO: replace with PlayerActionCheckCreep()?
{
	// We're doing something else, so run a normal function to determine it.
	PlayerActionCheckCreep(info);
}

void PlayerActionCheckCreepBackUnStrafe(playerinfo_t* info)
{
	// Still pressing the same way and still strafing.
	if (info->seqcmd[ACMDL_BACK])
	{
		if ((info->seqcmd[ACMDL_STRAFE_L] && info->lowerseq == ASEQ_CSTRAFEB_LEFT) ||
			(info->seqcmd[ACMDL_STRAFE_R] && info->lowerseq == ASEQ_CSTRAFEB_RIGHT))
		{
			// Account for coincidental action.
			PlayerActionCheckCreep(info);
			return;
		}
	}

	// Stopped moving forward, has gone to a side strafe.
	if (!info->seqcmd[ACMDL_BACK])
	{
		const qboolean is_running = (info->buttons & BUTTON_RUN); //mxd

		if (info->seqcmd[ACMDL_STRAFE_L])
		{
			PlayerAnimSetLowerSeq(info, (is_running ? ASEQ_DASH_LEFT_GO : ASEQ_STRAFEL)); // Check for a transfer to a run.
			return;
		}

		if (info->seqcmd[ACMDL_STRAFE_R])
		{
			PlayerAnimSetLowerSeq(info, (is_running ? ASEQ_DASH_RIGHT_GO : ASEQ_STRAFER)); // Check for a transfer to a run.
			return;
		}
	}

	// Have we reversed directions of the strafe?
	if (info->seqcmd[ACMDL_STRAFE_R] && info->lowerseq == ASEQ_CSTRAFEB_LEFT)
	{
		SetLowerSeq(info, ASEQ_CSTRAFEB_RIGHT); //mxd
		return;
	}

	if (info->seqcmd[ACMDL_STRAFE_L] && info->lowerseq == ASEQ_CSTRAFEB_RIGHT)
	{
		SetLowerSeq(info, ASEQ_CSTRAFEB_LEFT); //mxd
		return;
	}

	// We're doing something else, so run a normal function to determine it.
	PlayerActionCheckCreep(info);
}

#pragma region ========================== WALK FORWARD ==========================

void PlayerActionCheckWalk(playerinfo_t* info)
{
	const qboolean is_running = (info->buttons & BUTTON_RUN); //mxd
	const qboolean is_creeping = (info->buttons & BUTTON_CREEP); //mxd
	const int curseq = info->lowerseq;
	int tgtseq = ASEQ_NONE; //mxd

	if (info->groundentity == NULL && info->waterlevel < 2 && !(info->watertype & (CONTENTS_SLIME | CONTENTS_LAVA)) && CheckFall(info))
	{
		PlayerAnimSetLowerSeq(info, ASEQ_FALLWALK_GO);
		return;
	}

	// Check for an autovault (only occurs if upper half of body is idle!).
	if ((info->flags & PLAYER_FLAG_COLLISION) && info->upperidle && info->seqcmd[ACMDL_FWD])
	{
		PlayerActionCheckVault(info);

		if (curseq == ASEQ_VAULT_LOW || curseq == ASEQ_PULLUP_HALFWALL)
		{
			PlayerAnimSetLowerSeq(info, curseq);
			return;
		}
	}

	// Check for a jump	[High probability]. Slime causes skipping, so no jumping in it!
	if (info->seqcmd[ACMDL_JUMP] && !(info->watertype & (CONTENTS_SLIME | CONTENTS_LAVA)))
	{
		// Check what type of jump it is.
		if (info->seqcmd[ACMDL_FWD])
			tgtseq = ASEQ_JUMPFWD_WGO;
		else if (info->seqcmd[ACMDL_BACK])
			tgtseq = ASEQ_JUMPBACK_WGO;
		else if (info->seqcmd[ACMDL_STRAFE_L])
			tgtseq = ASEQ_JUMPLEFT_WGO;
		else if (info->seqcmd[ACMDL_STRAFE_R])
			tgtseq = ASEQ_JUMPRIGHT_WGO;

		if (tgtseq != ASEQ_NONE) //mxd
		{
			PlayerAnimSetLowerSeq(info, tgtseq);
			return;
		}
	}

	// Check for a crouch [Low probability].
	if (info->seqcmd[ACMDL_CROUCH])
	{
		// Check crouch direction.
		if (info->seqcmd[ACMDL_BACK])
			tgtseq = (is_creeping ? ASEQ_CROUCH_WALK_B : ASEQ_ROLL_B);
		else if (info->seqcmd[ACMDL_STRAFE_L])
			tgtseq = (is_creeping ? ASEQ_CROUCH_WALK_L : ASEQ_ROLL_L);
		else if (info->seqcmd[ACMDL_STRAFE_R])
			tgtseq = (is_creeping ? ASEQ_CROUCH_WALK_R : ASEQ_ROLL_R);
		else if (info->seqcmd[ACMDL_FWD])
			tgtseq = (is_creeping ? ASEQ_CROUCH_WALK_F : ASEQ_ROLLDIVEF_W);
		else
			tgtseq = ASEQ_CROUCH_GO; // All else failed, we just want to crouch to the ground.

		PlayerAnimSetLowerSeq(info, tgtseq);
		return;
	}

	// Check for a transition to a walking strafe [High probability].
	if (info->seqcmd[ACMDL_FWD] && (info->seqcmd[ACMDL_STRAFE_L] || info->seqcmd[ACMDL_STRAFE_R]))
	{
		const qboolean strafe_left = info->seqcmd[ACMDL_STRAFE_L];

		if (is_creeping)
		{
			PlayerAnimSetLowerSeq(info, (strafe_left ? ASEQ_CSTRAFE_LEFT : ASEQ_CSTRAFE_RIGHT));
			return;
		}

		if (is_running)
		{
			PlayerAnimSetLowerSeq(info, (strafe_left ? ASEQ_RSTRAFE_LEFT : ASEQ_RSTRAFE_RIGHT));
			return;
		}

		const int desired_seq = (strafe_left ? ASEQ_WSTRAFE_LEFT : ASEQ_WSTRAFE_RIGHT);
		if (curseq != desired_seq)
		{
			SetLowerSeq(info, desired_seq); //mxd
			return;
		}
	}

	// Check for a transition to a backward walking or running strafe [High probability].
	if (info->seqcmd[ACMDL_BACK] && (info->seqcmd[ACMDL_STRAFE_L] || info->seqcmd[ACMDL_STRAFE_R]))
	{
		const qboolean strafe_left = info->seqcmd[ACMDL_STRAFE_L];

		if (is_creeping)
		{
			PlayerAnimSetLowerSeq(info, (strafe_left ? ASEQ_CSTRAFEB_LEFT : ASEQ_CSTRAFEB_RIGHT));
			return;
		}

		if (is_running) //FIXME: There are no backwards run strafes!
			return;

		const int desired_seq = (strafe_left ? ASEQ_WSTRAFEB_LEFT : ASEQ_WSTRAFEB_RIGHT);
		if (curseq != desired_seq)
		{
			SetLowerSeq(info, desired_seq); //mxd
			return;
		}

		//return; //TODO: present here in original version, but not in the ACMDL_BACK + ACMDL_STRAFE_R block! Which variant is correct?
	}

	if (info->seqcmd[ACMDL_CREEP_F]) // Check for a sudden transition to a forwards creep [Low probability].
		tgtseq = ASEQ_CREEPF;
	else if (info->seqcmd[ACMDL_RUN_F]) // Check for a sudden transition to a run [Low probability].
		tgtseq = ASEQ_RUNF;
	else if (info->seqcmd[ACMDL_CREEP_B]) // Check for a sudden transition to a backwards creep [Low probability].
		tgtseq = ASEQ_CREEPB;

	if (tgtseq != ASEQ_NONE)
	{
		PlayerAnimSetLowerSeq(info, tgtseq);
		return;
	}

	// Handle an action key press [Low probability].
	if (info->seqcmd[ACMDL_ACTION])
	{
		// Climb a rope?
		if (info->targetEnt != NULL && PlayerActionCheckRopeGrab(info, 0.0f))
		{
			info->flags |= PLAYER_FLAG_ONROPE;
			P_Sound(info, SND_PRED_ID27, CHAN_VOICE, "player/ropegrab.wav", 0.75f); //mxd
			PlayerAnimSetLowerSeq(info, ASEQ_CLIMB_ON); // We're on the rope.
			return;
		}

		// Use a puzzle piece.
		PlayerActionUsePuzzle(info);

		if (info->upperidle && (info->flags & PLAYER_FLAG_COLLISION) && PlayerActionCheckJumpGrab(info, 0.0f))
		{
			PlayerAnimSetLowerSeq(info, ASEQ_JUMPSTD_GO);
			return;
		}
	}

	// Check for a quickturn [Low probability].
	if (info->seqcmd[ACMDL_QUICKTURN])
	{
		PlayerAnimSetLowerSeq(info, ASEQ_TURN180);
		return;
	}

	// If we're pressing forward, and nothing else is happening, then we're just walking forward.
	if (info->seqcmd[ACMDL_WALK_F] && curseq != ASEQ_WALKF && !info->seqcmd[ACMDL_STRAFE_L] && !info->seqcmd[ACMDL_STRAFE_R])
	{
		PlayerAnimSetLowerSeq(info, ASEQ_WALKF);
		return;
	}

	// If we're pressing backward, and nothing else is happening, then we're just walking backward.
	if (info->seqcmd[ACMDL_BACK] && !info->seqcmd[ACMDL_STRAFE_L] && !info->seqcmd[ACMDL_STRAFE_R])
	{
		if (info->seqcmd[ACMDL_RUN_B])
		{
			if (!info->seqcmd[ACMDU_ATTACK] && info->upperidle)
			{
				PlayerAnimSetLowerSeq(info, ASEQ_JUMPSPRINGBGO);
				return;
			}
		}
		else if (curseq != ASEQ_WALKB)
		{
			PlayerAnimSetLowerSeq(info, ASEQ_WALKB);
			return;
		}
	}

	// All else has failed... did we just let go of everything?
	if (!info->seqcmd[ACMDL_FWD] && !info->seqcmd[ACMDL_BACK])
		PlayerAnimSetLowerSeq(info, SeqCtrl[info->lowerseq].ceaseseq);
}

void PlayerActionCheckWalkUnStrafe(playerinfo_t* info)
{
	const qboolean is_running = (info->buttons & BUTTON_RUN); //mxd
	int tgtseq = ASEQ_NONE; //mxd

	// Player has started running.
	if (is_running)
	{
		if (info->seqcmd[ACMDL_STRAFE_L])
			tgtseq = ASEQ_RSTRAFE_LEFT;
		else if (info->seqcmd[ACMDL_STRAFE_R])
			tgtseq = ASEQ_RSTRAFE_RIGHT;

		if (tgtseq != ASEQ_NONE)
		{
			PlayerAnimSetLowerSeq(info, tgtseq);
			return;
		}
	}

	// Still pressing the same way and still strafing.
	if (info->seqcmd[ACMDL_FWD])
	{
		if ((info->seqcmd[ACMDL_STRAFE_L] && info->lowerseq == ASEQ_WSTRAFE_LEFT) ||
			(info->seqcmd[ACMDL_STRAFE_R] && info->lowerseq == ASEQ_WSTRAFE_RIGHT))
		{
			// Account for coincidental action.
			PlayerActionCheckWalk(info);
			return;
		}
	}

	// Stopped moving forward, has gone to a side strafe.
	if (!info->seqcmd[ACMDL_FWD])
	{
		// Check for a transfer to a run.
		if (info->seqcmd[ACMDL_STRAFE_L])
			tgtseq = (is_running ? ASEQ_DASH_LEFT_GO : ASEQ_STRAFEL);
		else if (info->seqcmd[ACMDL_STRAFE_R])
			tgtseq = (is_running ? ASEQ_DASH_RIGHT_GO : ASEQ_STRAFER);

		if (tgtseq != ASEQ_NONE)
		{
			PlayerAnimSetLowerSeq(info, tgtseq);
			return;
		}
	}

	// Have we reversed directions of the strafe?
	if (info->seqcmd[ACMDL_STRAFE_R] && info->lowerseq == ASEQ_WSTRAFE_LEFT)
	{
		SetLowerSeq(info, ASEQ_WSTRAFE_RIGHT); //mxd
		return;
	}

	if (info->seqcmd[ACMDL_STRAFE_L] && info->lowerseq == ASEQ_WSTRAFE_RIGHT)
	{
		SetLowerSeq(info, ASEQ_WSTRAFE_LEFT); //mxd
		return;
	}

	// We're doing something else, so run a normal function to determine it.
	PlayerActionCheckWalk(info);
}

#pragma endregion

#pragma region ========================== WALK BACK ==========================

void PlayerActionCheckWalkBack(playerinfo_t* info)
{
	const qboolean is_running = (info->buttons & BUTTON_RUN); //mxd

	// Check for a transition to a walking or running strafe [High probability].
	if (info->seqcmd[ACMDL_BACK] && info->seqcmd[ACMDL_STRAFE_L] && info->lowerseq != ASEQ_WSTRAFEB_LEFT)
	{
		if (is_running)
			PlayerAnimSetLowerSeq(info, ASEQ_WSTRAFEB_LEFT); //FIXME: There are no backwards run strafes!
		else
			SetLowerSeq(info, ASEQ_WSTRAFEB_LEFT); //mxd

		return;
	}

	// Check for a transition to a walking or running strafe [High probability].
	if (info->seqcmd[ACMDL_BACK] && info->seqcmd[ACMDL_STRAFE_R] && info->lowerseq != ASEQ_WSTRAFEB_RIGHT)
	{
		if (is_running)
			PlayerAnimSetLowerSeq(info, ASEQ_WSTRAFEB_RIGHT); //FIXME: There are no backwards run strafes!
		else
			SetLowerSeq(info, ASEQ_WSTRAFEB_RIGHT); //mxd

		return;
	}

	// We're doing something else, so run a normal function to determine it.
	PlayerActionCheckWalk(info);
}

void PlayerActionCheckWalkBackUnStrafe(playerinfo_t* info)
{
	const qboolean is_running = (info->buttons & BUTTON_RUN); //mxd

	// Still pressing the same way and still strafing.
	if (info->seqcmd[ACMDL_BACK])
	{
		if ((info->seqcmd[ACMDL_STRAFE_L] && info->lowerseq == ASEQ_WSTRAFEB_LEFT) ||
			(info->seqcmd[ACMDL_STRAFE_R] && info->lowerseq == ASEQ_WSTRAFEB_RIGHT))
		{
			// Account for coincidental action.
			PlayerActionCheckWalk(info);
			return;
		}
	}

	// Stopped moving forward, has gone to a side strafe.
	if (!info->seqcmd[ACMDL_BACK])
	{
		if (info->seqcmd[ACMDL_STRAFE_L])
		{
			PlayerAnimSetLowerSeq(info, (is_running ? ASEQ_DASH_LEFT_GO : ASEQ_STRAFEL));
			return;
		}

		if (info->seqcmd[ACMDL_STRAFE_R])
		{
			PlayerAnimSetLowerSeq(info, (is_running ? ASEQ_DASH_RIGHT_GO : ASEQ_STRAFER));
			return;
		}
	}

	// Have we reversed directions of the strafe?
	if (info->seqcmd[ACMDL_STRAFE_R] && info->lowerseq == ASEQ_WSTRAFEB_LEFT)
	{
		SetLowerSeq(info, ASEQ_WSTRAFEB_RIGHT); //mxd
		return;
	}

	if (info->seqcmd[ACMDL_STRAFE_L] && info->lowerseq == ASEQ_WSTRAFEB_RIGHT)
	{
		SetLowerSeq(info, ASEQ_WSTRAFEB_LEFT); //mxd
		return;
	}

	// We're doing something else, so run a normal function to determine it.
	PlayerActionCheckWalk(info);
}

#pragma endregion

#pragma region ========================== RUN ==========================

void PlayerActionCheckRun(playerinfo_t* info)
{
	const qboolean is_running = (info->buttons & BUTTON_RUN); //mxd
	const int curseq = info->lowerseq;

	// Check for an upper sequence interruption due to a staff attack.
	if (info->seqcmd[ACMDU_ATTACK] && info->seqcmd[ACMDL_RUN_F] && info->pers.weaponready == WEAPON_READY_SWORDSTAFF &&
		!(info->flags & PLAYER_FLAG_NO_RARM) && !(info->edictflags & FL_CHICKEN))
	{
		int spin_seq;

		if (info->advancedstaff)
			spin_seq = ((info->seqcmd[ACMDL_STRAFE_L] || info->seqcmd[ACMDL_STRAFE_R]) ? ASEQ_WSWORD_SPIN2 : ASEQ_WSWORD_SPIN);
		else // Not advanced staff
			spin_seq = (info->irand(info, 0, 1) ? ASEQ_WSWORD_SPIN2 : ASEQ_WSWORD_SPIN);

		PlayerAnimSetLowerSeq(info, spin_seq);
		return;
	}

	// Check for an autovault (only occurs if upper half of body is idle!).
	if (info->seqcmd[ACMDL_FWD] && info->upperidle && (info->flags & PLAYER_FLAG_COLLISION))
	{
		PlayerActionCheckVault(info);

		if (curseq == ASEQ_VAULT_LOW || curseq == ASEQ_PULLUP_HALFWALL)
		{
			PlayerAnimSetLowerSeq(info, curseq);
			return;
		}
	}

	// Check for a transition to a walking or running strafe [High probability].
	if (info->seqcmd[ACMDL_FWD] && info->seqcmd[ACMDL_STRAFE_L] && curseq != ASEQ_RSTRAFE_LEFT)
	{
		if (is_running)
			SetLowerSeq(info, ASEQ_RSTRAFE_LEFT); //mxd
		else
			PlayerAnimSetLowerSeq(info, ASEQ_WSTRAFE_LEFT);

		return;
	}

	// Check for a transition to a walking or running strafe [High probability].
	if (info->seqcmd[ACMDL_FWD] && info->seqcmd[ACMDL_STRAFE_R] && curseq != ASEQ_RSTRAFE_RIGHT)
	{
		if (is_running)
			SetLowerSeq(info, ASEQ_RSTRAFE_RIGHT); //mxd
		else
			PlayerAnimSetLowerSeq(info, ASEQ_WSTRAFE_RIGHT);

		return;
	}

	// Check for a jump [High probability]. Slime causes skipping, so no jumping in it!
	if (info->seqcmd[ACMDL_JUMP] && !(info->watertype & (CONTENTS_SLIME | CONTENTS_LAVA)))
	{
		int tgtseq = ASEQ_NONE; //mxd

		if (info->seqcmd[ACMDL_FWD])
		{
			if (info->pers.weaponready == WEAPON_READY_SWORDSTAFF && !(info->flags & PLAYER_FLAG_NO_RARM))
				tgtseq = ASEQ_POLEVAULT1_W;
			else
				tgtseq = ASEQ_JUMPFWD_WGO;
		}
		else if (info->seqcmd[ACMDL_BACK])
		{
			tgtseq = ASEQ_JUMPBACK_WGO;
		}
		else if (info->seqcmd[ACMDL_STRAFE_L])
		{
			tgtseq = ASEQ_JUMPLEFT_WGO;
		}
		else if (info->seqcmd[ACMDL_STRAFE_R])
		{
			tgtseq = ASEQ_JUMPRIGHT_WGO;
		}

		if (tgtseq != ASEQ_NONE)
		{
			PlayerAnimSetLowerSeq(info, tgtseq);
			return;
		}
	}

	// Check for a sudden transition to a walk [Low probability].
	if (info->seqcmd[ACMDL_WALK_F])
	{
		PlayerAnimSetLowerSeq(info, ASEQ_WALKF);
		return;
	}

	// Check for a crouch [Low probability].
	if (info->seqcmd[ACMDL_CROUCH])
	{
		if (info->seqcmd[ACMDL_BACK])
			PlayerAnimSetLowerSeq(info, ASEQ_ROLL_B);
		else if (info->seqcmd[ACMDL_STRAFE_L])
			PlayerAnimSetLowerSeq(info, ASEQ_ROLL_L);
		else if (info->seqcmd[ACMDL_STRAFE_R])
			PlayerAnimSetLowerSeq(info, ASEQ_ROLL_R);
		else
			PlayerAnimSetLowerSeq(info, ASEQ_ROLLDIVEF_W);

		return;
	}

	// Handle an action key press [Low probability].
	if (info->seqcmd[ACMDL_ACTION])
	{
		// Climb a rope?
		if (info->targetEnt != NULL && PlayerActionCheckRopeGrab(info, 0.0f))
		{
			info->flags |= PLAYER_FLAG_ONROPE;
			P_Sound(info, SND_PRED_ID28, CHAN_VOICE, "player/ropegrab.wav", 0.75f); //mxd
			PlayerAnimSetLowerSeq(info, ASEQ_CLIMB_ON); // We're on the rope.

			return;
		}

		// Try and use a puzzle item.
		PlayerActionUsePuzzle(info);
	}

	// Check for a quickturn [Low probability].
	if (info->seqcmd[ACMDL_QUICKTURN])
	{
		PlayerAnimSetLowerSeq(info, ASEQ_TURN180);
		return;
	}

	// Check for a sudden transition to a creep [Low probability].
	if (info->seqcmd[ACMDL_CREEP_F])
	{
		PlayerAnimSetLowerSeq(info, ASEQ_CREEPF);
		return;
	}

	// If we're pressing forward, and nothing else is happening, then we're walking forward.
	if (info->seqcmd[ACMDL_RUN_F] && curseq != ASEQ_RUNF && !info->seqcmd[ACMDL_STRAFE_L] && !info->seqcmd[ACMDL_STRAFE_R])
	{
		PlayerAnimSetLowerSeq(info, ASEQ_RUNF);
		return;
	}

	// If we're pressing backward, and nothing else is happening, then we're walking backward.
	if (info->seqcmd[ACMDL_BACK])
	{
		if (info->seqcmd[ACMDL_RUN_B])
		{
			if (!info->seqcmd[ACMDU_ATTACK] && info->upperidle)
			{
				PlayerAnimSetLowerSeq(info, ASEQ_JUMPSPRINGBGO);
				return;
			}
		}
		else
		{
			PlayerAnimSetLowerSeq(info, ASEQ_WALKB);
			return;
		}
	}

	// All else has failed... did we just let go of everything?
	if (!info->seqcmd[ACMDL_FWD] && !info->seqcmd[ACMDL_BACK])
		PlayerAnimSetLowerSeq(info, SeqCtrl[info->lowerseq].ceaseseq);
}

void PlayerActionCheckRunUnStrafe(playerinfo_t* info)
{
	const qboolean is_running = (info->buttons & BUTTON_RUN); //mxd

	// Player has stopped running
	if (!is_running)
	{
		if (info->seqcmd[ACMDL_STRAFE_L])
		{
			PlayerAnimSetLowerSeq(info, ASEQ_WSTRAFE_LEFT);
			return;
		}

		if (info->seqcmd[ACMDL_STRAFE_R])
		{
			PlayerAnimSetLowerSeq(info, ASEQ_WSTRAFE_RIGHT);
			return;
		}
	}

	// Still pressing the same way and still strafing.
	if (info->seqcmd[ACMDL_FWD])
	{
		if ((info->seqcmd[ACMDL_STRAFE_L] && info->lowerseq == ASEQ_RSTRAFE_LEFT) ||
			(info->seqcmd[ACMDL_STRAFE_R] && info->lowerseq == ASEQ_RSTRAFE_RIGHT))
		{
			// Account for coincidental action.
			PlayerActionCheckRun(info);
			return;
		}
	}

	// Stopped moving forward, has gone to a side strafe.
	if (!info->seqcmd[ACMDL_FWD])
	{
		int tgtseq = ASEQ_NONE;

		if (info->seqcmd[ACMDL_STRAFE_L])
			tgtseq = (is_running ? ASEQ_DASH_LEFT : ASEQ_STRAFEL);
		else if (info->seqcmd[ACMDL_STRAFE_R])
			tgtseq = (is_running ? ASEQ_DASH_RIGHT : ASEQ_STRAFER);

		if (tgtseq != ASEQ_NONE)
		{
			PlayerAnimSetLowerSeq(info, tgtseq);
			return;
		}
	}

	// Have we reversed directions of the strafe?
	if (info->seqcmd[ACMDL_STRAFE_R] && info->lowerseq == ASEQ_RSTRAFE_LEFT)
	{
		SetLowerSeq(info, ASEQ_RSTRAFE_RIGHT); //mxd
		return;
	}

	if (info->seqcmd[ACMDL_STRAFE_L] && info->lowerseq == ASEQ_RSTRAFE_RIGHT)
	{
		SetLowerSeq(info, ASEQ_RSTRAFE_LEFT); //mxd
		return;
	}

	// We're doing something else, so run a normal function to determine it.
	PlayerActionCheckRun(info);
}

#pragma endregion

void PlayerActionClimbStartSound(const playerinfo_t* info, float value)
{
	assert(info);

	// 1 in 5 chance of playing.
	if (irand(0, 4) == 0)
		P_Sound(info, SND_PRED_ID29, CHAN_VOICE, "*grab.wav", 0.75f); //mxd
}

void PlayerPlaySlide(const playerinfo_t *info)
{
	assert(info);
	P_Sound(info, SND_PRED_ID30, CHAN_VOICE, "player/slope.wav", 0.75f); //mxd
}

PLAYER_API void PlayerInterruptAction(playerinfo_t* info)
{
	// Shut off player effects from weapons or the like.
	TurnOffPlayerEffects(info);

	// Remove weapon sounds from the player (technically looping weapons should do this for us, but better safe than annoyed).
	P_Sound(info, SND_PRED_ID31, CHAN_WEAPON, "misc/null.wav", 1.0f); //mxd

	// Release any held weapons.
	if (info->pers.weapon->tag == ITEM_WEAPON_REDRAINBOW && info->upperseq == ASEQ_WRRBOW_HOLD)
	{
		info->PlayerActionRedRainBowAttack(info);
	}
	else if (info->pers.weapon->tag == ITEM_WEAPON_PHOENIXBOW && info->upperseq == ASEQ_WPHBOW_HOLD)
	{
		info->PlayerActionPhoenixBowAttack(info);
	}
	else if (info->pers.weapon->tag == ITEM_WEAPON_SPHEREOFANNIHILATION && info->chargingspell)
	{
		info->chargingspell = false;
		info->PlayerActionSpellSphereCreate(info, &info->chargingspell);
	}

	// Clear out any pending animations.
	PlayerAnimSetUpperSeq(info, ASEQ_NONE);
}

void PlayerActionSetDead(playerinfo_t* info) //mxd. Was PlayerSetDead() in p_anim_data.c in original version.
{
	info->deadflag = DEAD_DEAD;
}