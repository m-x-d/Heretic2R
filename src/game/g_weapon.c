//
// g_weapon.c -- Generic weapon handling code for all player weapons.
//
// Copyright 1998 Raven Software
//

#include "g_weapon.h" //mxd
#include "p_anims.h"
#include "p_main.h"
#include "g_playstats.h"
#include "g_skeletons.h"
#include "g_ai.h" //mxd
#include "g_combat.h" //mxd
#include "m_beast.h"
#include "spl_blast.h" //mxd
#include "spl_flyingfist.h" //mxd
#include "spl_HellStaff.h" //mxd
#include "spl_maceballs.h" //mxd
#include "spl_magicmissile.h" //mxd
#include "spl_Phoenix.h" //mxd
#include "spl_RedRain.h" //mxd
#include "spl_ripper.h" //mxd
#include "spl_sphereofannihlation.h" //mxd
#include "spl_wall.h" //mxd
#include "FX.h"
#include "Matrix.h"
#include "Random.h"
#include "Reference.h"
#include "Vector.h"
#include "g_local.h"

// Sets up a spell missile's starting position, correct with respect to the player model's angles and posture.
// We want the start position of the missile to be the caster's hand, so we must take into account the caster's joint orientations
// and global model orientation. This is a big-arsed hack. We assume that if the caster is standing fully upstraight, the missile
// will be launched from default coordinates (relative to the caster's origin) specified in DefaultStartPos.
// The other two inputs, LowerJoint & UpperJoint specify Corvus's bone joint positions (relative to his model's origin)
// for the animation frame in which the weapon is launched.
static void Weapon_CalcStartPos(const vec3_t origin_to_lower_joint, const vec3_t origin_to_upper_joint, const vec3_t default_start_pos, vec3_t actual_start_pos, const edict_t* caster)
{
	// Get matrices corresponding to the current angles of the upper and lower back joints.
	vec3_t lowerback_joint_angles;
	vec3_t upperback_joint_angles;

	for (int i = 0; i < 3; i++)
	{
		lowerback_joint_angles[i] = GetJointAngle(caster->s.rootJoint + CORVUS_LOWERBACK, i);
		upperback_joint_angles[i] = GetJointAngle(caster->s.rootJoint + CORVUS_UPPERBACK, i);
	}

	matrix3_t lower_rotation;
	Matrix3FromAngles(lowerback_joint_angles, lower_rotation);

	matrix3_t upper_rotation;
	Matrix3FromAngles(upperback_joint_angles, upper_rotation);

	// Get vector from player model's origin to lower joint.
	vec3_t lower_joint;
	VectorAdd(caster->s.origin, origin_to_lower_joint, lower_joint);

	// Get vector from player model's origin to upper joint.
	vec3_t upper_joint;
	VectorAdd(caster->s.origin, origin_to_upper_joint, upper_joint);

	// Get vector from lower joint to upper joint.
	vec3_t lower_joint_to_upper_joint;
	VectorSubtract(origin_to_upper_joint, origin_to_lower_joint, lower_joint_to_upper_joint);

	// Get vector from upper joint to the default flying-fist's start position.
	vec3_t forward;
	vec3_t right;
	vec3_t up;
	AngleVectors(caster->s.angles, forward, right, up);

	vec3_t start_pos;
	VectorMA(caster->s.origin, default_start_pos[0], right, start_pos);
	VectorMA(start_pos, default_start_pos[1], forward, start_pos);
	VectorMA(start_pos, default_start_pos[2], up, start_pos);
	VectorSubtract(start_pos, upper_joint, start_pos);

	// Add in the contribution from the 'bone' from the lower joint to upper joint.
	Matrix3MultByVec3(upper_rotation, start_pos, start_pos);
	VectorAdd(start_pos, lower_joint_to_upper_joint, start_pos);

	// Add in the contribution from the model's origin to the lower joint.
	Matrix3MultByVec3(lower_rotation, start_pos, start_pos);
	VectorAdd(origin_to_lower_joint, start_pos, start_pos);

	// Finally, add on the model's origin to give the correct start position for the flying-fist.
	VectorAdd(start_pos, caster->s.origin, start_pos);
	VectorCopy(start_pos, actual_start_pos);
}

enum swordpos_e
{
	SWORD_ATK_L,
	SWORD_ATK_R,
	SWORD_SPINATK_L,
	SWORD_SPINATK_R,
	SWORD_ATK_B,
	SWORD_ATK_STAB
};

static vec3_t sword_positions[23] =
{
	{	0.0f,	0.0f,	0.0f	},	// 0
	{	-20.0f,	-20.0f,	26.0f	},	// 1	swipeA4
	{	4.0f,	-34.0f,	22.0f	},	// 2	swipeA5
	{	43.0f,	-16.0f,	-10.0f	},	// 3	swipeA6
	{	33.0f,	20.0f,	-32.0f	},	// 4	swipeA7

	{	-16.0f,	12.0f,	20.0f	},	// 5	swipeB4
	{	8.0f,	34.0f,	16.0f	},	// 6	swipeB5
	{	40.0f,	-16.0f,	-10.0f	},	// 7	swipeB6
	{	-8.0f,	-24.0f,	-22.0f	},	// 8	swipeB7

	{	-32.0f,	0.0f,	32.0f	},	// 9	newspin5
	{	24.0f,	-36.0f,	8.0f	},	// 10	newspin6
	{	44.0f,	20.0f,	-20.0f	},	// 11	newspin7
	{	0.0f,	0.0f,	0.0f	},	// 12

	{	-24.0f,	0.0f,	20.0f	},	// 13	spining4
	{	24.0f,	36.0f,	16.0f	},	// 14	spining5
	{	36.0f,	-36.0f,	-20.0f	},	// 15	spining6
	{	0.0f,	0.0f,	0.0f	},	// 16

	{	-12.0f,	-12.0f, -12.0f	},	// 17	roundbck2
	{	-20.0f,	28.0f,	-4.0f	},	// 18	roundbck3
	{	16.0f,	36.0f,	0.0f	},	// 19	roundbck4
	{	0.0f,	0.0f,	0.0f	},	// 20

	{	12.0f,	0.0f,	-12.0f	},	// 21	spikedown7
	{	20.0f,	0.0f,	-48.0f	},	// 22	spikedown8
};

static int sword_damage[STAFF_LEVEL_MAX][2] =
{
	//	MIN						MAX
	{	0,						0						}, // STAFF_LEVEL_NONE
	{	SWORD_DMG_MIN,			SWORD_DMG_MAX			}, // STAFF_LEVEL_BASIC
	{	SWORD_POWER1_DMG_MIN,	SWORD_POWER1_DMG_MAX	}, // STAFF_LEVEL_POWER1
	{	SWORD_POWER2_DMG_MIN,	SWORD_POWER2_DMG_MAX	}, // STAFF_LEVEL_POWER2
};

void WeaponThink_SwordStaff(edict_t* caster, char* format, ...)
{
	const vec3_t mins = { -12.0f, -12.0f, -12.0f };
	const vec3_t maxs = {  12.0f,  12.0f,  12.0f };

	assert(caster->client != NULL);

	playerinfo_t* info = &caster->client->playerinfo;
	assert(info != NULL);

	int power_level = info->pers.stafflevel;
	if (info->powerup_timer > level.time)
		power_level++; // Powerups now power up your staff, too.

	if (power_level <= STAFF_LEVEL_NONE)
		return;

	power_level = min(STAFF_LEVEL_MAX - 1, power_level);

	va_list marker;
	va_start(marker, format);
	const int loc_id = va_arg(marker, int);
	va_end(marker);

	vec3_t fwd;
	vec3_t right;
	vec3_t up;
	AngleVectors(caster->client->aimangles, fwd, right, up);

	// Set up the area to check.
	vec3_t attack_pos;
	VectorCopy(sword_positions[loc_id], attack_pos);

	vec3_t end_pos;
	VectorMA(caster->s.origin, attack_pos[0], fwd, end_pos);
	VectorMA(end_pos, -attack_pos[1], right, end_pos);
	VectorMA(end_pos, attack_pos[2], up, end_pos);

	vec3_t start_pos;

	// Now if we are the first attack of this sweep (1, 5, 9, 13), starting in solid means a hit. If not, then we must avoid startsolid entities.
	if ((loc_id & 3) == 1)
	{
		// First check of the swing.
		caster->client->lastentityhit = NULL;
		VectorCopy(end_pos, start_pos);
	}
	else
	{
		VectorCopy(sword_positions[loc_id - 1], attack_pos);
		VectorMA(caster->s.origin, attack_pos[0], fwd, start_pos);
		VectorMA(start_pos, -attack_pos[1], right, start_pos);
		VectorMA(start_pos, attack_pos[2], up, start_pos);
	}

	start_pos[2] += (float)caster->viewheight;
	end_pos[2] += (float)caster->viewheight;

	VectorCopy(end_pos, caster->client->laststaffpos);
	caster->client->laststaffuse = level.time;

	trace_t trace;
	gi.trace(start_pos, mins, maxs, end_pos, caster, MASK_PLAYERSOLID | CONTENTS_DEADMONSTER, &trace);

	if (level.fighting_beast)
	{
		edict_t* ent = TB_CheckHit(start_pos, trace.endpos);

		if (ent != NULL)
			trace.ent = ent;
	}

	const int fx_flags = (power_level >= STAFF_LEVEL_POWER2 ? CEF_FLAG7 : 0); //mxd. Fire sparks when powered, blue otherwise.

	if (trace.ent != NULL && trace.ent->takedamage != DAMAGE_NO)
	{
		if (trace.startsolid && trace.ent == caster->client->lastentityhit)
			return;

		// Hit another player?
		if (info->advancedstaff && trace.ent->client != NULL)
		{
			// Crimminy, what if they're blocking?
			if (trace.ent->client->playerinfo.block_timer >= level.time)
			{
				// Check angle
				vec3_t hit_dir;
				VectorSubtract(caster->s.origin, trace.ent->s.origin, hit_dir);
				VectorNormalize(hit_dir);

				vec3_t hit_angles;
				vectoangles(hit_dir, hit_angles);

				vec3_t diff_angles;
				diff_angles[YAW] = hit_angles[YAW] - trace.ent->client->aimangles[YAW];

				if (diff_angles[YAW] > 180.0f)
					diff_angles[YAW] -= 360.0f;
				else if (diff_angles[YAW] < -180.0f)
					diff_angles[YAW] += 360.0f;

				diff_angles[PITCH] = hit_angles[PITCH] - trace.ent->client->aimangles[PITCH];

				if (diff_angles[YAW] > -60.0f && diff_angles[YAW] < 60.0f && diff_angles[PITCH] > -45.0f && diff_angles[PITCH] < 75.0f)
				{
					// The opponent is indeed facing you...
					gi.CreateEffect(NULL, FX_BLOCK_SPARKS, fx_flags, trace.endpos, "d", hit_dir);
					AlertMonsters(caster, caster, 1.0f, true);
					gi.sound(caster, CHAN_AUTO, gi.soundindex(va("weapons/ArmorRic%i.wav", irand(1, 3))), 1.0f, ATTN_NORM, 0.0f);
					caster->client->lastentityhit = trace.ent;

					// Now we're in trouble, go into the attack recoil...
					switch ((loc_id - 1) >> 2)
					{
						case SWORD_ATK_L:
							P_PlayerAnimSetUpperSeq(info, ASEQ_WSWORD_BLOCKED_L);
							P_PlayerAnimSetUpperSeq(&trace.ent->client->playerinfo, ASEQ_WSWORD_BLOCK_L); // The blocker must react too.
							return;

						case SWORD_ATK_R:
							P_PlayerAnimSetUpperSeq(info, ASEQ_WSWORD_BLOCKED_R);
							P_PlayerAnimSetUpperSeq(&trace.ent->client->playerinfo, ASEQ_WSWORD_BLOCK_R); // The blocker must react too.
							return;

						case SWORD_SPINATK_L:
							P_PlayerAnimSetLowerSeq(info, ASEQ_WSWORD_SPINBLOCKED);
							P_PlayerAnimSetUpperSeq(&trace.ent->client->playerinfo, ASEQ_WSWORD_BLOCK_L); // The blocker must react too.
							return;

						case SWORD_SPINATK_R:
							P_PlayerAnimSetLowerSeq(info, ASEQ_WSWORD_SPINBLOCKED2);
							P_PlayerAnimSetUpperSeq(&trace.ent->client->playerinfo, ASEQ_WSWORD_BLOCK_R); // The blocker must react too.
							return;

						case SWORD_ATK_B:
							P_PlayerAnimSetUpperSeq(&trace.ent->client->playerinfo, ASEQ_WSWORD_BLOCK_R);
							return;

						case SWORD_ATK_STAB:
							P_PlayerAnimSetUpperSeq(&trace.ent->client->playerinfo, ASEQ_WSWORD_BLOCK_L);
							return;
					}
				}
			}

			if (trace.ent->client->laststaffuse + 0.1f >= level.time)
			{
				// Check if the staffs collided.
				vec3_t hit_dir;
				VectorSubtract(trace.endpos, trace.ent->client->laststaffpos, hit_dir);

				if (VectorLength(hit_dir) < 48.0f)
				{
					// Let's make these two staffs collide.
					gi.CreateEffect(NULL, FX_BLOCK_SPARKS, fx_flags, trace.endpos, "d", hit_dir);
					AlertMonsters(caster, caster, 1.0f, true);
					gi.sound(caster, CHAN_AUTO, gi.soundindex(va("weapons/ArmorRic%i.wav", irand(1, 3))), 1.0f, ATTN_NORM, 0.0f);
					caster->client->lastentityhit = trace.ent;
					trace.ent->client->lastentityhit = caster;

					// Now we're in trouble, go into the attack recoil...
					switch ((loc_id - 1) >> 2)
					{
						case SWORD_ATK_L:
							P_PlayerAnimSetUpperSeq(info, ASEQ_WSWORD_BLOCKED_L);
							P_PlayerAnimSetUpperSeq(&trace.ent->client->playerinfo, ASEQ_WSWORD_BLOCKED_L); // The blocker must react too.
							return;

						case SWORD_ATK_R:
							P_PlayerAnimSetUpperSeq(info, ASEQ_WSWORD_BLOCKED_R);
							P_PlayerAnimSetUpperSeq(&trace.ent->client->playerinfo, ASEQ_WSWORD_BLOCKED_R); // The blocker must react too.
							return;

						case SWORD_SPINATK_L:
							P_PlayerAnimSetLowerSeq(info, ASEQ_WSWORD_SPINBLOCKED);
							P_PlayerAnimSetUpperSeq(&trace.ent->client->playerinfo, ASEQ_WSWORD_BLOCKED_L); // The blocker must react too.
							return;

						case SWORD_SPINATK_R:
							P_PlayerAnimSetLowerSeq(info, ASEQ_WSWORD_SPINBLOCKED2);
							P_PlayerAnimSetUpperSeq(&trace.ent->client->playerinfo, ASEQ_WSWORD_BLOCKED_R); // The blocker must react too.
							return;

						case SWORD_ATK_B:
							P_PlayerAnimSetUpperSeq(&trace.ent->client->playerinfo, ASEQ_WSWORD_BLOCKED_R);
							return;

						case SWORD_ATK_STAB:
							P_PlayerAnimSetUpperSeq(&trace.ent->client->playerinfo, ASEQ_WSWORD_BLOCKED_L);
							return;
					}
				}
			}
		}

		if (CanDamage(trace.ent, caster))
		{
			vec3_t hit_dir;
			VectorSubtract(end_pos, start_pos, hit_dir);
			VectorNormalize2(hit_dir, hit_dir);

			float damage = (float)(irand(sword_damage[power_level][0], sword_damage[power_level][1]));

			// Spin attacks should do double damage.
			switch ((loc_id - 1) >> 2)
			{
				case SWORD_SPINATK_L:
				case SWORD_SPINATK_R:
					damage *= SWORD_SPIN_DMG_MOD;	// 175% damage from spins.
					break;

				case SWORD_ATK_B:
					damage *= SWORD_BACK_DMG_MOD;	// 70% damage from behind.
					break;

				case SWORD_ATK_STAB:
					damage *= SWORD_STAB_DMG_MOD;	// 250% damage from stab.
					break;
			}

			if (caster->client != NULL && (info->flags & PLAYER_FLAG_NO_LARM))
				damage = ceilf(damage / 3.0f); // only one arm 1/3 the damage

			int dflags = DAMAGE_EXTRA_KNOCKBACK | DAMAGE_DISMEMBER;

			if (power_level == STAFF_LEVEL_POWER1)
				dflags |= DAMAGE_DOUBLE_DISMEMBER;
			else if (power_level == STAFF_LEVEL_POWER2)
				dflags |= DAMAGE_FIRE;

			T_Damage(trace.ent, caster, caster, fwd, trace.endpos, hit_dir, (int)damage, (int)(damage * 4.0f), dflags, MOD_STAFF);

			// If we hit a monster, stick a trail of blood on the staff...
			if (trace.ent->svflags & SVF_MONSTER)
			{
				const int flags = CEF_OWNERS_ORIGIN | ((trace.ent->materialtype == MAT_INSECT) ? CEF_FLAG8 : 0); //mxd
				gi.CreateEffect(&caster->s, FX_LINKEDBLOOD, flags, NULL, "bb", 30, CORVUS_BLADE);
			}

			if (trace.ent->svflags & SVF_MONSTER || trace.ent->client != NULL)
			{
				caster->s.effects |= EF_BLOOD_ENABLED;
				info->effects |= EF_BLOOD_ENABLED;
			}

			// Use special hit puff.
			switch (power_level)
			{
				case STAFF_LEVEL_BASIC:
					gi.sound(caster, CHAN_AUTO, gi.soundindex("weapons/staffhit.wav"), 1.0f, ATTN_NORM, 0.0f);
					break;

				case STAFF_LEVEL_POWER1:
					gi.CreateEffect(NULL, FX_WEAPON_STAFF_STRIKE, 0, trace.endpos, "db", trace.plane.normal, power_level);
					gi.sound(caster, CHAN_AUTO, gi.soundindex("weapons/staffhit_2.wav"), 1.0f, ATTN_NORM, 0.0f);
					break;

				case STAFF_LEVEL_POWER2:
					gi.CreateEffect(NULL, FX_WEAPON_STAFF_STRIKE, 0, trace.endpos, "db", trace.plane.normal, power_level);
					gi.sound(caster, CHAN_AUTO, gi.soundindex("weapons/staffhit_3.wav"), 1.0f, ATTN_NORM, 0.0f);
					break;
			}

			caster->client->lastentityhit = trace.ent;
		}
	}
	else if (trace.fraction < 1.0f || trace.startsolid)
	{
		// Hit a wall or such...
		if (caster->client->lastentityhit == NULL && Vec3NotZero(trace.plane.normal))
		{
			// Don't do sparks if already hit something.
			vec3_t hit_angles;
			vectoangles(trace.plane.normal, hit_angles);

			gi.CreateEffect(NULL, FX_SPARKS, fx_flags, trace.endpos, "d", trace.plane.normal); //BUGFIX: mxd. Original logic uses uninitialized 'hit_dir' var here.
			AlertMonsters(caster, caster, 1.0f, true);
			gi.sound(caster, CHAN_AUTO, gi.soundindex("weapons/staffhitwall.wav"), 1.0f, ATTN_NORM, 0.0f);

			//NOTENOTE: -1 means that the last entity was a wall...
			caster->client->lastentityhit = (edict_t*)0xFFFFFFFF;
		}
	}
}

static const vec3_t origin_to_lower_joint = { 0.945585f, 2.26076f, 0.571354f }; //mxd. Made global static.
static const vec3_t origin_to_upper_joint = { 1.80845f,  2.98912f, 3.278f }; //mxd. Made global static.

void WeaponThink_FlyingFist(edict_t* caster, char* format, ...)
{
	const vec3_t default_start_pos = { 18.0f, 10.0f, 15.0f };

	// Set up the Magic-missile's starting position and aiming angles, then cast the spell.
	vec3_t start_pos;
	Weapon_CalcStartPos(origin_to_lower_joint, origin_to_upper_joint, default_start_pos, start_pos, caster);
	start_pos[2] += (float)caster->viewheight - 14.0f;

	vec3_t forward;
	AngleVectors(caster->client->aimangles, forward, NULL, NULL);

	SpellCastFlyingFist(caster, start_pos, caster->client->aimangles, forward, 0.0f);

	// Take off mana, but if there is none, then fire a wimpy fizzle-weapon.
	playerinfo_t* info = &caster->client->playerinfo; //mxd
	if (info->pers.inventory.Items[info->weap_ammo_index] > 0 && (!DEATHMATCH || !(DMFLAGS & DF_INFINITE_MANA)))
		info->pers.inventory.Items[info->weap_ammo_index] -= info->pers.weapon->quantity; //TODO: Can info->pers.inventory.Items[] go below zero?
}


// ************************************************************************************************
// WeaponThink_Maceballs
// ----------------------
// ************************************************************************************************

void WeaponThink_Maceballs(edict_t *caster, char *format,...)
{
	vec3_t	OriginToLowerJoint={0.945585,2.26076,0.571354},
			OriginToUpperJoint={1.80845,2.98912,3.27800},
			defaultstartpos={-4.0,15.0,15.0},		// Ripper start position
			defaultstartpos2={13.0,15.0,-15.0},		// Maceball start position
			startpos,
			fwd;

	assert(caster->client);
	
	if (caster->client->playerinfo.powerup_timer > level.time)
	{
		// Set up the ball's starting position and aiming angles then cast the spell.
		Weapon_CalcStartPos(OriginToLowerJoint, OriginToUpperJoint, defaultstartpos2, startpos, caster);

		AngleVectors(caster->client->aimangles, fwd, NULL, NULL);
		startpos[2] += caster->viewheight - 14.0;

		SpellCastMaceball(caster, startpos, caster->client->aimangles, NULL, 0.0);
		// Giant iron dooms require lotsa mana, but yer average ripper needs far less.
		if (!deathmatch->value || (deathmatch->value && !((int)dmflags->value & DF_INFINITE_MANA)))
			caster->client->playerinfo.pers.inventory.Items[caster->client->playerinfo.weap_ammo_index] -= 
					caster->client->playerinfo.pers.weapon->quantity * 2.0;
	}
	else
	{
		// Set up the ball's starting position and aiming angles then cast the spell.
		Weapon_CalcStartPos(OriginToLowerJoint, OriginToUpperJoint,defaultstartpos,startpos,caster);

		AngleVectors(caster->client->aimangles, fwd, NULL, NULL);
		startpos[2] += caster->viewheight - 14.0;

		SpellCastRipper(caster, startpos, caster->client->aimangles, NULL);
		if (!deathmatch->value || (deathmatch->value && !((int)dmflags->value & DF_INFINITE_MANA)))
			caster->client->playerinfo.pers.inventory.Items[caster->client->playerinfo.weap_ammo_index] -= 
					caster->client->playerinfo.pers.weapon->quantity;		// Un-powered
	}
}

// ************************************************************************************************
// WeaponThink_MagicMissileSpread
// ------------------------------
// ************************************************************************************************
#define MISSILE_YAW 7.0
#define MISSILE_PITCH 2.0
#define MISSILE_SEP	4.0
void WeaponThink_MagicMissileSpread(edict_t *caster,char *format,...)
{
	va_list marker;
	int		missilepos,
			count;
	char	curchar;
	vec3_t	OriginToLowerJoint={0.945585,2.26076,0.571354},
			OriginToUpperJoint={1.80845,2.98912,3.27800},
			DefaultStartPos={8.0,0.0,5.0},
			StartPos;
	vec3_t	fireangles, fwd;

	// Get number of missiles to fire off and get the separation between missiles.

	assert(strlen(format));

	missilepos=1;

	count=0;

	va_start(marker,format);
	curchar=format[0];
	assert(curchar=='i');
	missilepos=va_arg(marker,int);
	va_end(marker);

	// Set up the Magic-missile's starting position and aiming angles then cast the spell.

	// Push the start position forward for earlier shots
	DefaultStartPos[0] -= MISSILE_SEP*missilepos;
	DefaultStartPos[1] += MISSILE_SEP*missilepos;
	Weapon_CalcStartPos(OriginToLowerJoint,OriginToUpperJoint,DefaultStartPos,StartPos,caster);
	StartPos[2] += caster->viewheight - 14.0;
	
	VectorCopy(caster->client->aimangles, fireangles);
	fireangles[YAW] += missilepos*MISSILE_YAW;
	fireangles[PITCH] += missilepos*MISSILE_PITCH;
	AngleVectors(fireangles, fwd, NULL, NULL);
	SpellCastMagicMissile(caster, StartPos, fireangles, fwd);

	if (missilepos == -1.0)
		gi.sound(caster,CHAN_WEAPON,gi.soundindex("weapons/MagicMissileSpreadFire.wav"),1,ATTN_NORM,0);

	if (!deathmatch->value || (deathmatch->value && !((int)dmflags->value & DF_INFINITE_MANA)))
		caster->client->playerinfo.pers.inventory.Items[caster->client->playerinfo.weap_ammo_index]--;
}

// ************************************************************************************************
// WeaponThink_SphereOfAnnihilation
// -------------------------------
// ************************************************************************************************

void WeaponThink_SphereOfAnnihilation(edict_t *Caster,char *Format,...)
{
	va_list		Marker;
	qboolean	*ReleaseFlagsPtr;
	vec3_t		Forward;

	// Get pointer to missile's release flag.

	assert(strlen(Format));

	va_start(Marker,Format);

	assert(*Format=='g');

	ReleaseFlagsPtr=va_arg(Marker,qboolean *);

	va_end(Marker);

	// Set up the Sphere-of-annihilation's aiming angles then cast the spell.

	AngleVectors(Caster->client->aimangles,Forward,NULL,NULL);

	SpellCastSphereOfAnnihilation(Caster,
								 NULL,
								 Caster->client->aimangles,		//v_angle,
								 Forward,
								 0.0,
								 ReleaseFlagsPtr);

	if (!deathmatch->value || (deathmatch->value && !((int)dmflags->value & DF_INFINITE_MANA)))
		Caster->client->playerinfo.pers.inventory.Items[Caster->client->playerinfo.weap_ammo_index]-= Caster->client->playerinfo.pers.weapon->quantity;
}

// ************************************************************************************************
// WeaponThink_Firewall
// -------------------------------
// ************************************************************************************************

void WeaponThink_Firewall(edict_t *caster, char *format,...)
{
	SpellCastWall(caster, caster->s.origin, caster->client->aimangles, NULL, 0.0);

	if (!deathmatch->value || (deathmatch->value && !((int)dmflags->value & DF_INFINITE_MANA)))
		caster->client->playerinfo.pers.inventory.Items[caster->client->playerinfo.weap_ammo_index] -= caster->client->playerinfo.pers.weapon->quantity;
}

// ************************************************************************************************
// WeaponThink_RedRainBow
// ----------------------
// ************************************************************************************************

void WeaponThink_RedRainBow(edict_t *caster,char *Format,...)
{
	vec3_t	StartPos, Forward, Right;

	AngleVectors(caster->client->aimangles, Forward, Right, NULL);
	VectorMA(caster->s.origin, 25.0F, Forward, StartPos);
	VectorMA(StartPos, 6.0F, Right, StartPos);
	StartPos[2] += caster->viewheight + 4.0;
	
	SpellCastRedRain(caster, StartPos, caster->client->aimangles, NULL, 0.0F);

	if (!deathmatch->value || (deathmatch->value && !((int)dmflags->value & DF_INFINITE_MANA)))
		caster->client->playerinfo.pers.inventory.Items[caster->client->playerinfo.weap_ammo_index] -= caster->client->playerinfo.pers.weapon->quantity;
}

// ************************************************************************************************
// WeaponThink_PhoenixBow
// ----------------------
// ************************************************************************************************

void WeaponThink_PhoenixBow(edict_t *caster,char *Format,...)
{
	vec3_t	StartPos, Forward, Right;

	AngleVectors(caster->client->aimangles, Forward, Right, NULL);
	VectorMA(caster->s.origin, 25.0F, Forward, StartPos);
	VectorMA(StartPos, 6.0F, Right, StartPos);
	StartPos[2] += caster->viewheight + 4.0;
	
	SpellCastPhoenix(caster, StartPos, caster->client->aimangles, Forward, 0.0F);

	if (!deathmatch->value || (deathmatch->value && !((int)dmflags->value & DF_INFINITE_MANA)))
		caster->client->playerinfo.pers.inventory.Items[caster->client->playerinfo.weap_ammo_index] -= caster->client->playerinfo.pers.weapon->quantity;
}

// ************************************************************************************************
// WeaponThink_HellStaff
// ---------------------
// ************************************************************************************************

void WeaponThink_HellStaff(edict_t *caster,char *Format,...)
{
	vec3_t	StartPos;	//, off;
	vec3_t	fwd, right;
//	vec3_t	startangle;

	// Set up the Hellstaff's starting position and aiming angles then cast the spell.
//	VectorSet(off, 34.0, -6.0, 0.0);
//	VectorGetOffsetOrigin(off, caster->s.origin, caster->client->aimangles[YAW], StartPos);

	// Two-thirds of the player angle is torso movement.
/*	startangle[PITCH] = (caster->client->aimangles[PITCH] - caster->s.angles[PITCH]) * 2.0 / 3.0;
	startangle[YAW] = caster->client->aimangles[YAW] - caster->s.angles[YAW];
	if (startangle[YAW] > 180.0)
		startangle[YAW] -= 360.0;
	else if (startangle[YAW] < -180.0)
		startangle[YAW] += 360;
	startangle[YAW] *= 2.0/3.0;
	startangle[ROLL] = 0.0;
*/
//	VectorAdd(startangle, caster->s.angles, startangle);
//	AngleVectors(startangle, fwd, right, NULL);
	AngleVectors(caster->client->aimangles, fwd, right, NULL);
	VectorMA(caster->s.origin,30,fwd,StartPos);
	VectorMA(StartPos,10,right,StartPos);
	StartPos[2] += caster->viewheight - 14.0;

	SpellCastHellstaff(caster, StartPos, caster->client->aimangles, NULL);

	if (!deathmatch->value || (deathmatch->value && !((int)dmflags->value & DF_INFINITE_MANA)))
		caster->client->playerinfo.pers.inventory.Items[caster->client->playerinfo.weap_ammo_index] -= caster->client->playerinfo.pers.weapon->quantity;
}



// ************************************************************************************************
// WeaponThink_Blast
// ---------------------
// ************************************************************************************************

void WeaponThink_Blast(edict_t *caster,char *Format,...)
{
	vec3_t	startpos;
	vec3_t	fwd, right;
	vec3_t	mins={-3.0, -3.0, -3.0}, maxs={3.0, 3.0, 3.0};
	trace_t	trace;

	assert(caster->client);

	// Find the firing position first.
	AngleVectors(caster->client->aimangles, fwd, right, NULL);
	VectorMA(caster->s.origin,10,fwd,startpos);
	VectorMA(startpos, -4.0F, right, startpos);
	startpos[2] += caster->viewheight;

	// Trace from the player's origin to the casting location to assure not spawning in a wall.
	gi.trace(caster->s.origin, mins, maxs, startpos, caster, MASK_SHOT,&trace);
	if (trace.startsolid || trace.allsolid)
	{	// No way to avoid spawning in a wall.
		return;
	}

	if (trace.fraction < 1.0)
	{
		VectorCopy(trace.endpos, startpos);
	}

	// This weapon does not autotarget
	SpellCastBlast(caster, startpos, caster->client->aimangles, NULL);

	if (!deathmatch->value || (deathmatch->value && !((int)dmflags->value & DF_INFINITE_MANA)))
		caster->client->playerinfo.pers.inventory.Items[caster->client->playerinfo.weap_ammo_index] -= caster->client->playerinfo.pers.weapon->quantity;

	gi.sound(caster,CHAN_WEAPON,gi.soundindex("weapons/BlastFire.wav"),1,ATTN_NORM,0);
}

// end
