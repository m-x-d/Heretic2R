// g_weapon.c - generic weapon handling code for all player weapons

//#include "p_types.h"
#include "player.h"
#include "g_local.h"
#include "p_weapon.h"
#include "m_player.h"
#include "fx.h"
#include "matrix.h"
#include "vector.h"
#include "g_skeletons.h"
#include "angles.h"
#include "Reference.h"
#include "Random.h"
#include "Utilities.h"
#include "g_items.h"
#include "g_playstats.h"
#include "p_main.h"
#include "m_beast.h"
#include "p_anims.h"

extern void SpellCastFlyingFist(edict_t *Caster,vec3_t StartPos,vec3_t AimAngles,vec3_t AimDir,float Value);
extern void SpellCastMagicMissile(edict_t *Caster,vec3_t StartPos,vec3_t AimAngles,vec3_t AimDir);
extern void SpellCastMagicMissileSpread(edict_t *Caster,vec3_t StartPos,vec3_t AimAngles,vec3_t AimDir,
										float NoOfMissiles,float Separation);
extern void SpellCastSphereOfAnnihilation(edict_t *Caster,vec3_t StartPos,vec3_t AimAngles,vec3_t AimDir,
										 float Value,qboolean *ReleaseFlagsPtr);
extern void SpellCastMaceball(edict_t *Caster,vec3_t StartPos,vec3_t AimAngles,vec3_t AimDir,float Value);
extern void SpellCastWall(edict_t *caster, vec3_t startpos, vec3_t aimangles, vec3_t AimDir, float Value);
extern void SpellCastRipper(edict_t *Caster,vec3_t StartPos,vec3_t AimAngles,vec3_t AimDir);
extern void SpellCastBlast(edict_t *Caster,vec3_t StartPos,vec3_t AimAngles,vec3_t AimDir);
extern void SpellCastRedRain(edict_t *Caster,vec3_t StartPos,vec3_t AimAngles,vec3_t AimDir,float Value);
extern void SpellCastPhoenix(edict_t *Caster,vec3_t StartPos,vec3_t AimAngles,vec3_t AimDir,float Value);
extern void SpellCastHellstaff(edict_t *Caster,vec3_t StartPos,vec3_t AimAngles,vec3_t AimDir);

static void Weapon_CalcStartPos(vec3_t OriginToLowerJoint,vec3_t OriginToUpperJoint,
								vec3_t DefaultStartPos,vec3_t ActualStartPos,edict_t *Caster);
extern void AlertMonsters (edict_t *self, edict_t *enemy, float lifetime, qboolean ignore_shadows);

// ************************************************************************************************
// Weapon_CalcStartPos
// -------------------
// Sets up a spell missile's starting position, correct with respect to the player model's angles
// and posture. We want the start position of the missile to be the caster's hand, so we must take
// into account the caster's joint orientations and global model orientation. This is a big-arsed
// hack. We assume that if the caster is standing fully upstraight, the missile will be launched
// from default co-ordinates (relative to the caster's origin) specified in DefaultStartPos. The
// other two inputs, LowerJoint & UpperJoint specify Corvus's bone joint positions (relative to his
// model's origin) for the animation frame in which the weapon is launched.
// *************************************************************************************************

static void Weapon_CalcStartPos(vec3_t OriginToLowerJoint,vec3_t OriginToUpperJoint,
								vec3_t DefaultStartPos,vec3_t ActualStartPos,edict_t *Caster)
{
	matrix3_t	LowerRotationMatrix,UpperRotationMatrix;
	vec3_t		LowerbackJointAngles,UpperbackJointAngles,
				LowerJoint,UpperJoint,LowerJointToUpperJoint,
				Forward,Right,Up,
				StartPos;

	// Get matrices corresponding to the current angles of the upper and lower back joints.

	LowerbackJointAngles[PITCH]=GetJointAngle(Caster->s.rootJoint+CORVUS_LOWERBACK,PITCH);
	LowerbackJointAngles[YAW]=GetJointAngle(Caster->s.rootJoint+CORVUS_LOWERBACK,YAW);
	LowerbackJointAngles[ROLL]=GetJointAngle(Caster->s.rootJoint+CORVUS_LOWERBACK,ROLL);
	Matrix3FromAngles(LowerbackJointAngles,LowerRotationMatrix);

	UpperbackJointAngles[PITCH]=GetJointAngle(Caster->s.rootJoint+CORVUS_UPPERBACK,PITCH);
	UpperbackJointAngles[YAW]=GetJointAngle(Caster->s.rootJoint+CORVUS_UPPERBACK,YAW);	
	UpperbackJointAngles[ROLL]=GetJointAngle(Caster->s.rootJoint+CORVUS_UPPERBACK,ROLL);
	Matrix3FromAngles(UpperbackJointAngles,UpperRotationMatrix);

	// Get vector from player model's origin to lower joint.

	VectorAdd(Caster->s.origin,OriginToLowerJoint,LowerJoint);

	// Get vector from player model's origin to upper joint.

	VectorAdd(Caster->s.origin,OriginToUpperJoint,UpperJoint);

	// Get vector from lower joint to upper joint.

	VectorSubtract(OriginToUpperJoint,OriginToLowerJoint,LowerJointToUpperJoint);

	// Get vector from upper joint to the default flying-fist's start position.

	AngleVectors(Caster->s.angles,Forward,Right,Up);
	VectorMA(Caster->s.origin,DefaultStartPos[0],Right,StartPos);
	VectorMA(StartPos,DefaultStartPos[1],Forward,StartPos);
	VectorMA(StartPos,DefaultStartPos[2],Up,StartPos);
	VectorSubtract(StartPos,UpperJoint,StartPos);

	// Add in the contribution from the 'bone' from the lower joint to upper joint.

	Matrix3MultByVec3(UpperRotationMatrix,StartPos,StartPos);
	VectorAdd(StartPos,LowerJointToUpperJoint,StartPos);
	
	// Add in the contribution from the model's origin to the lower joint.

	Matrix3MultByVec3(LowerRotationMatrix,StartPos,StartPos);
	VectorAdd(OriginToLowerJoint,StartPos,StartPos);
	
	// Finally, add on the model's origin to give the correct start position for the flying-fist.

	VectorAdd(StartPos,Caster->s.origin,StartPos);

	VectorCopy(StartPos,ActualStartPos);
}

// ************************************************************************************************
// WeaponThink_SwordStaff
// ----------------------
// ************************************************************************************************

enum swordpos_e
{
	SWORD_ATK_L,
	SWORD_ATK_R,
	SWORD_SPINATK_L,
	SWORD_SPINATK_R,
	SWORD_ATK_B,
	SWORD_ATK_STAB
};


vec3_t swordpositions[23] =
{
	{	0,		0,		0	},	// 0
	{	-20,	-20,	26	},	// 1	swipeA4
	{	4,		-34,	22	},	// 2	swipeA5
	{	43,		-16,	-10	},	// 3	swipeA6
	{	33,		20,		-32	},	// 4	swipeA7

	{	-16,	12,		20	},	// 5	swipeB4
	{	8,		34,		16	},	// 6	swipeB5
	{	40,		-16,	-10	},	// 7	swipeB6
	{	-8,		-24,	-22	},	// 8	swipeB7

	{	-32,	0,		32	},	// 9	newspin5
	{	24,		-36,	8	},	// 10	newspin6
	{	44,		20,		-20	},	// 11	newspin7
	{	0,		0,		0	},	// 12

	{	-24,	0,		20	},	// 13	spining4
	{	24,		36,		16	},	// 14	spining5
	{	36,		-36,	-20	},	// 15	spining6
	{	0,		0,		0	},	// 16

	{	-12,	-12, 	-12	},	// 17	roundbck2
	{	-20,	28,		-4	},	// 18	roundbck3
	{	16,		36,		0	},	// 19	roundbck4
	{	0,		0,		0	},	// 20

	{	12,		0,		-12	},	// 21	spikedown7
	{	20,		0,		-48	},	// 22	spikedown8
};

int sworddamage[STAFF_LEVEL_MAX][2] = 
{	//	MIN		MAX
	{	0,						0						},		// STAFF_LEVEL_NONE
	{	SWORD_DMG_MIN,			SWORD_DMG_MAX			},		// STAFF_LEVEL_BASIC
	{	SWORD_POWER1_DMG_MIN,	SWORD_POWER1_DMG_MAX	},		// STAFF_LEVEL_POWER1
	{	SWORD_POWER2_DMG_MIN,	SWORD_POWER2_DMG_MAX	},		// STAFF_LEVEL_POWER2
};

void WeaponThink_SwordStaff(edict_t *Caster,char *Format,...)
{
	va_list Marker;
	char CurrentChar;
		int locid;
	vec3_t fwd, right, up;
	vec3_t atkpos, startpos, endpos, hitdir, hitangles, diffangles;
	vec3_t mins={-12, -12, -12};
	vec3_t maxs={12, 12, 12};
	int damage, powerlevel, dflags;
	playerinfo_t *playerinfo;

	trace_t trace;

	assert(Caster->client);
	playerinfo = &Caster->client->playerinfo;
	assert(playerinfo);

	powerlevel = playerinfo->pers.stafflevel;
	if (playerinfo->powerup_timer > level.time)
		powerlevel++;							// powerups now power up your staff, too.

	if (powerlevel <= STAFF_LEVEL_NONE)
		return;
	else if (powerlevel >= STAFF_LEVEL_MAX)
		powerlevel = STAFF_LEVEL_MAX-1;

	assert(strlen(Format));
	va_start(Marker,Format);
	CurrentChar=Format[0];
	assert(CurrentChar=='i');
	locid=va_arg(Marker,int);
	va_end(Marker);

	AngleVectors(Caster->client->aimangles, fwd, right, up);

	// Set up the area to check.
	VectorCopy(swordpositions[locid], atkpos);
	VectorMA(Caster->s.origin, atkpos[0], fwd, endpos);
	VectorMA(endpos, -atkpos[1], right, endpos);
	VectorMA(endpos, atkpos[2], up, endpos);

	// Now if we are the first attack of this sweep (1, 5, 9, 13), starting in solid means a hit.  If not, then we must avoid startsolid entities.
	if ((locid & 0x03) == 0x01)
	{	// First check of the swing.
		Caster->client->lastentityhit = NULL;
		VectorCopy(endpos, startpos);
	}
	else
	{
		VectorCopy(swordpositions[locid-1], atkpos);
		VectorMA(Caster->s.origin, atkpos[0], fwd, startpos);
		VectorMA(startpos, -atkpos[1], right, startpos);
		VectorMA(startpos, atkpos[2], up, startpos);
	}
	startpos[2] += Caster->viewheight;
	endpos[2] += Caster->viewheight;

	// For showing where the sword attacks are.
//	gi.CreateEffect(NULL, FX_TEST_BBOX, 0, endpos, "fff", maxs[0], mins[2], maxs[2]);

	VectorCopy(endpos, Caster->client->laststaffpos);
	Caster->client->laststaffuse = level.time;

	gi.trace(startpos, mins, maxs, endpos, Caster, MASK_PLAYERSOLID|CONTENTS_DEADMONSTER,&trace);
	if(level.fighting_beast)
	{
		edict_t *ent;
		
		if(ent = check_hit_beast(startpos, trace.endpos))
			trace.ent = ent;
	}

	if (trace.ent && trace.ent->takedamage)
	{
		if (!trace.startsolid || trace.ent != Caster->client->lastentityhit)
		{
			if (playerinfo->advancedstaff && trace.ent->client && trace.ent->client->playerinfo.block_timer >= level.time)
			{	// Crimminy, what if they're blocking?
				// Check angle
				VectorSubtract(Caster->s.origin, trace.ent->s.origin, hitdir);
				VectorNormalize(hitdir);
				vectoangles(hitdir, hitangles);
				diffangles[YAW] = hitangles[YAW] - trace.ent->client->aimangles[YAW];
				if (diffangles[YAW] > 180.0)
					diffangles[YAW] -= 360.0;
				else if (diffangles[YAW] < -180.0)
					diffangles[YAW] += 360.0;
				diffangles[PITCH] = hitangles[PITCH] - trace.ent->client->aimangles[PITCH];

				if (diffangles[YAW] > -60.0 && 
						diffangles[YAW] < 60.0 &&
						diffangles[PITCH] > -45.0 &&
						diffangles[PITCH] < 75.0)
				{	// The opponent is indeed facing you...
					if (powerlevel >= STAFF_LEVEL_POWER2)
					{
						gi.CreateEffect(NULL,
										FX_BLOCK_SPARKS,
										CEF_FLAG7,
										trace.endpos,
										"d",
										hitdir);
					}
					else
					{
						gi.CreateEffect(NULL,
										FX_BLOCK_SPARKS,
										0,
										trace.endpos,
										"d",
										hitdir);
					}

					AlertMonsters (Caster, Caster, 1, true);
					switch(irand(1,3))
					{
					case 1:
						gi.sound(Caster, CHAN_AUTO, gi.soundindex("weapons/ArmorRic1.wav"), 1, ATTN_NORM, 0);
						break;
					case 2:
						gi.sound(Caster, CHAN_AUTO, gi.soundindex("weapons/ArmorRic2.wav"), 1, ATTN_NORM, 0);
						break;
					case 3:
						gi.sound(Caster, CHAN_AUTO, gi.soundindex("weapons/ArmorRic3.wav"), 1, ATTN_NORM, 0);
						break;
					}
					Caster->client->lastentityhit = trace.ent;

					// Now we're in trouble, go into the attack recoil...
					switch((locid-1)>>2)
					{
					case SWORD_ATK_L:
						P_PlayerAnimSetUpperSeq(playerinfo, ASEQ_WSWORD_BLOCKED_L);
						// And of course the blocker must react too.
						P_PlayerAnimSetUpperSeq(&trace.ent->client->playerinfo, ASEQ_WSWORD_BLOCK_L);
						return;
						break;
					case SWORD_ATK_R:
						P_PlayerAnimSetUpperSeq(playerinfo, ASEQ_WSWORD_BLOCKED_R);
						// And of course the blocker must react too.
						P_PlayerAnimSetUpperSeq(&trace.ent->client->playerinfo, ASEQ_WSWORD_BLOCK_R);
						return;
						break;
					case SWORD_SPINATK_L:
						P_PlayerAnimSetLowerSeq(playerinfo, ASEQ_WSWORD_SPINBLOCKED);
						// And of course the blocker must react too.
						P_PlayerAnimSetUpperSeq(&trace.ent->client->playerinfo, ASEQ_WSWORD_BLOCK_L);
						return;
						break;
					case SWORD_SPINATK_R:
						P_PlayerAnimSetLowerSeq(playerinfo, ASEQ_WSWORD_SPINBLOCKED2);
						// And of course the blocker must react too.
						P_PlayerAnimSetUpperSeq(&trace.ent->client->playerinfo, ASEQ_WSWORD_BLOCK_R);
						return;
						break;
					case SWORD_ATK_B:
						P_PlayerAnimSetUpperSeq(&trace.ent->client->playerinfo, ASEQ_WSWORD_BLOCK_R);
						return;
						break;
					case SWORD_ATK_STAB:
						P_PlayerAnimSetUpperSeq(&trace.ent->client->playerinfo, ASEQ_WSWORD_BLOCK_L);
						return;
						break;
					}
				}
			}

			if (playerinfo->advancedstaff && trace.ent->client && trace.ent->client->laststaffuse+0.1 >= level.time)
			{	// Check if the staffs collided.
				VectorSubtract(trace.endpos, trace.ent->client->laststaffpos, hitdir);

				if (VectorLength(hitdir) < 48.0)
				{	// Let's make these two staffs collide.
					if (powerlevel >= STAFF_LEVEL_POWER2)
					{
						gi.CreateEffect(NULL,
										FX_BLOCK_SPARKS,
										CEF_FLAG7,
										trace.endpos,
										"d",
										hitdir);
					}
					else
					{
						gi.CreateEffect(NULL,
										FX_BLOCK_SPARKS,
										0,
										trace.endpos,
										"d",
										hitdir);
					}

					AlertMonsters (Caster, Caster, 1, true);
					switch(irand(1,3))
					{
					case 1:
						gi.sound(Caster, CHAN_AUTO, gi.soundindex("weapons/ArmorRic1.wav"), 1, ATTN_NORM, 0);
						break;
					case 2:
						gi.sound(Caster, CHAN_AUTO, gi.soundindex("weapons/ArmorRic2.wav"), 1, ATTN_NORM, 0);
						break;
					case 3:
						gi.sound(Caster, CHAN_AUTO, gi.soundindex("weapons/ArmorRic3.wav"), 1, ATTN_NORM, 0);
						break;
					}
					Caster->client->lastentityhit = trace.ent;
					trace.ent->client->lastentityhit = Caster;

					// Now we're in trouble, go into the attack recoil...
					switch((locid-1)>>2)
					{
					case SWORD_ATK_L:
						P_PlayerAnimSetUpperSeq(playerinfo, ASEQ_WSWORD_BLOCKED_L);
						// And of course the blocker must react too.
						P_PlayerAnimSetUpperSeq(&trace.ent->client->playerinfo, ASEQ_WSWORD_BLOCKED_L);
						return;
						break;
					case SWORD_ATK_R:
						P_PlayerAnimSetUpperSeq(playerinfo, ASEQ_WSWORD_BLOCKED_R);
						// And of course the blocker must react too.
						P_PlayerAnimSetUpperSeq(&trace.ent->client->playerinfo, ASEQ_WSWORD_BLOCKED_R);
						return;
						break;
					case SWORD_SPINATK_L:
						P_PlayerAnimSetLowerSeq(playerinfo, ASEQ_WSWORD_SPINBLOCKED);
						// And of course the blocker must react too.
						P_PlayerAnimSetUpperSeq(&trace.ent->client->playerinfo, ASEQ_WSWORD_BLOCKED_L);
						return;
						break;
					case SWORD_SPINATK_R:
						P_PlayerAnimSetLowerSeq(playerinfo, ASEQ_WSWORD_SPINBLOCKED2);
						// And of course the blocker must react too.
						P_PlayerAnimSetUpperSeq(&trace.ent->client->playerinfo, ASEQ_WSWORD_BLOCKED_R);
						return;
						break;
					case SWORD_ATK_B:
						P_PlayerAnimSetUpperSeq(&trace.ent->client->playerinfo, ASEQ_WSWORD_BLOCKED_R);
						return;
						break;
					case SWORD_ATK_STAB:
						P_PlayerAnimSetUpperSeq(&trace.ent->client->playerinfo, ASEQ_WSWORD_BLOCKED_L);
						return;
						break;
					}
				}
			}

			if (CanDamage (trace.ent, Caster))
			{
				VectorSubtract(endpos, startpos, hitdir);
				VectorNormalize2(hitdir, hitdir);

				if (powerlevel > STAFF_LEVEL_POWER2)
					powerlevel = STAFF_LEVEL_POWER2;

				damage = irand(sworddamage[powerlevel][0], sworddamage[powerlevel][1]);
				// Spin attacks should double damage
				switch((locid-1)>>2)
				{
//				case SWORD_ATK_L:
//					break;
//				case SWORD_ATK_R:
//					break;
				case SWORD_SPINATK_L:
				case SWORD_SPINATK_R:
					damage *= SWORD_SPIN_DMG_MOD;		// 50% more damage from spins.
					break;
				case SWORD_ATK_B:
					damage *= SWORD_BACK_DMG_MOD;		// Half damage from behind.
					break;
				case SWORD_ATK_STAB:
					damage *= SWORD_STAB_DMG_MOD;		// Double damage from stab.
					break;
				}

				if(Caster->client)
				{
					if(playerinfo->flags & PLAYER_FLAG_NO_LARM)
					{
						damage = ceil(damage/3);//only one arm 1/3 the damage
					}
				}

				switch(powerlevel)
				{
				case STAFF_LEVEL_BASIC:
					dflags = DAMAGE_EXTRA_KNOCKBACK|DAMAGE_DISMEMBER;
					break;

				case STAFF_LEVEL_POWER1:
					dflags = DAMAGE_EXTRA_KNOCKBACK|DAMAGE_DISMEMBER|DAMAGE_DOUBLE_DISMEMBER;
					break;

				case STAFF_LEVEL_POWER2:
					dflags = DAMAGE_EXTRA_KNOCKBACK|DAMAGE_DISMEMBER|DAMAGE_FIRE;
					break;
				}

				T_Damage (trace.ent, Caster, Caster, fwd, trace.endpos, hitdir, damage, damage*4, dflags,MOD_STAFF);
				
				// If we hit a monster, stick a trail of blood on the staff...
				if (trace.ent->svflags & SVF_MONSTER)
				{
					if(trace.ent->materialtype == MAT_INSECT)//yellow blood
						gi.CreateEffect(&Caster->s, FX_LINKEDBLOOD, CEF_FLAG8|CEF_OWNERS_ORIGIN, NULL, "bb", 30, CORVUS_BLADE);
					else
						gi.CreateEffect(&Caster->s, FX_LINKEDBLOOD, CEF_OWNERS_ORIGIN, NULL, "bb", 30, CORVUS_BLADE);
				}

				if (trace.ent->svflags & SVF_MONSTER || trace.ent->client)
				{
					Caster->s.effects |= EF_BLOOD_ENABLED;
					playerinfo->effects |= EF_BLOOD_ENABLED;
				}

				//Use special hit puff
				switch (powerlevel)
				{
				case STAFF_LEVEL_BASIC:
					gi.sound(Caster,CHAN_AUTO,gi.soundindex("weapons/staffhit.wav"),1,ATTN_NORM,0);
					break;

				case STAFF_LEVEL_POWER1:
					gi.CreateEffect(NULL,
									FX_WEAPON_STAFF_STRIKE,
									0,
									trace.endpos,
									"db",
									trace.plane.normal,
									powerlevel);
					
					gi.sound(Caster,CHAN_AUTO,gi.soundindex("weapons/staffhit_2.wav"),1,ATTN_NORM,0);
					break;

				case STAFF_LEVEL_POWER2:
					gi.CreateEffect(NULL,
									FX_WEAPON_STAFF_STRIKE,
									0,
									trace.endpos,
									"db",
									trace.plane.normal,
									powerlevel);
					
					gi.sound(Caster,CHAN_AUTO,gi.soundindex("weapons/staffhit_3.wav"),1,ATTN_NORM,0);
					break;
				}

				Caster->client->lastentityhit = trace.ent;
			}
		}
	}
	else if (trace.fraction < 1.0 || trace.startsolid)
	{	// Hit a wall or such...
		if (Caster->client->lastentityhit == NULL && Vec3NotZero(trace.plane.normal))
		{	// Don't do sparks if already hit something
			vectoangles(trace.plane.normal, hitangles);

			if (powerlevel >= STAFF_LEVEL_POWER2)
			{
				gi.CreateEffect(NULL,
								FX_SPARKS,
								CEF_FLAG7,
								trace.endpos,
								"d",
								hitdir);
			}
			else
			{
				gi.CreateEffect(NULL,
								FX_SPARKS,
								0,
								trace.endpos,
								"d",
								hitdir);
			}

			AlertMonsters (Caster, Caster, 1, true);
			gi.sound(Caster, CHAN_AUTO, gi.soundindex("weapons/staffhitwall.wav"), 1, ATTN_NORM, 0);

			// NOTENOTE -1 means that the last entity was a wall...
			Caster->client->lastentityhit = (edict_t *)0xFFFFFFFF;
		}
	}
}

// ************************************************************************************************
// WeaponThink_FlyingFist
// ----------------------
// ************************************************************************************************
void WeaponThink_FlyingFist(edict_t *Caster,char *Format,...)
{
	vec3_t	OriginToLowerJoint={0.945585,2.26076,0.571354},
			OriginToUpperJoint={1.80845,2.98912,3.27800},
			DefaultStartPos={18.0,10.0,15.0},
			StartPos,
			Forward;

	// Set up the Magic-missile's starting position and aiming angles then cast the spell.

	Weapon_CalcStartPos(OriginToLowerJoint,OriginToUpperJoint,DefaultStartPos,StartPos,Caster);
	
	AngleVectors(Caster->client->aimangles,Forward,NULL,NULL);

	StartPos[2] += Caster->viewheight - 14.0;
	SpellCastFlyingFist(Caster,StartPos,Caster->client->aimangles,Forward,0.0);

	// Take off mana, but if there is none, then fire a wimpy fizzle-weapon.
	if (Caster->client->playerinfo.pers.inventory.Items[Caster->client->playerinfo.weap_ammo_index] > 0)
	{
		if (!(deathmatch->value && ((int)dmflags->value & DF_INFINITE_MANA)))
				Caster->client->playerinfo.pers.inventory.Items[Caster->client->playerinfo.weap_ammo_index] -= 
						Caster->client->playerinfo.pers.weapon->quantity;	
	}
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
