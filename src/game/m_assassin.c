//
// m_assassin.c
//
// Copyright 1998 Raven Software
//

#include <float.h>
#include "m_assassin.h"
#include "m_assassin_local.h" //mxd
#include "m_assassin_shared.h" //mxd
#include "m_assassin_anim.h"
#include "m_assassin_moves.h"
#include "c_ai.h"
#include "mg_ai.h" //mxd
#include "mg_guide.h" //mxd
#include "g_debris.h" //mxd
#include "g_DefaultMessageHandler.h"
#include "g_HitLocation.h"
#include "g_monster.h"
#include "g_playstats.h"
#include "m_stats.h"
#include "p_anims.h"
#include "Random.h"
#include "Utilities.h"
#include "Vector.h"

#pragma region ========================== Assassin base info ==========================

static const animmove_t* animations[NUM_ANIMS] =
{
	&assassin_move_daggerl,
	&assassin_move_daggerr,
	&assassin_move_daggerb,
	&assassin_move_daggerc,
	&assassin_move_newdagger,
	&assassin_move_newdaggerb,
	&assassin_move_backflip,
	&assassin_move_frontflip,
	&assassin_move_dodge_right,
	&assassin_move_dodge_left,
	&assassin_move_deatha,
	&assassin_move_deathb,
	&assassin_move_jump,
	&assassin_move_run,
	&assassin_move_pain1,
	&assassin_move_pain2,
	&assassin_move_delay,
	&assassin_move_stand,
	&assassin_move_crouch,
	&assassin_move_uncrouch,
	&assassin_move_evade_jump,
	&assassin_move_evade_backflip,
	&assassin_move_evade_frontflip,
	&assassin_move_inair,
	&assassin_move_land,
	&assassin_move_forcedjump,
	&assassin_move_fjump,
	&assassin_move_bfinair,
	&assassin_move_bfland,
	&assassin_move_ffinair,
	&assassin_move_ffland,
	&assassin_move_evinair,
	&assassin_move_teleport,
	&assassin_move_cloak,
	&assassin_move_walk,
	&assassin_move_walk_loop,
	&assassin_move_backspring,

	// Crouches.
	&assassin_move_crouch_trans,
	&assassin_move_crouch_idle,
	&assassin_move_crouch_look_right,
	&assassin_move_crouch_look_right_idle,
	&assassin_move_crouch_look_l2r,
	&assassin_move_crouch_look_left,
	&assassin_move_crouch_look_left_idle,
	&assassin_move_crouch_look_r2l,
	&assassin_move_crouch_look_r2c,
	&assassin_move_crouch_look_l2c,
	&assassin_move_crouch_poke,
	&assassin_move_crouch_end,

	// Cinematic animations.
	&assassin_move_c_idle1,
	&assassin_move_c_run1,
	&assassin_move_c_attack1,
	&assassin_move_c_attack2,
};

static int sounds[NUM_SOUNDS];

#pragma endregion

#pragma region ========================== Dagger utility functions ==========================

edict_t* AssassinDaggerReflect(edict_t* self, edict_t* other, const vec3_t vel) //mxd. Named 'AssassinArrowReflect' in original logic.
{
	edict_t* dagger = G_Spawn();

	AssassinDaggerInit(dagger);

	dagger->s.modelindex = self->s.modelindex;
	VectorCopy(self->s.origin, dagger->s.origin);
	dagger->owner = other;
	dagger->enemy = self->owner;
	dagger->nextthink = self->nextthink;
	VectorScale(self->avelocity, -0.5f, dagger->avelocity);
	VectorCopy(vel, dagger->velocity);
	VectorNormalize2(vel, dagger->movedir);
	AnglesFromDir(dagger->movedir, dagger->s.angles);
	dagger->reflect_debounce_time = self->reflect_debounce_time - 1;
	dagger->reflected_time = self->reflected_time;

	gi.CreateEffect(&dagger->s, FX_M_EFFECTS, 0, dagger->avelocity, "bv", FX_ASS_DAGGER, dagger->velocity);

	G_LinkMissile(dagger);
	G_SetToFree(self);

	gi.CreateEffect(&dagger->s, FX_LIGHTNING_HIT, CEF_OWNERS_ORIGIN, NULL, "t", vel);

	return dagger;
}

void AssassinDaggerTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surface) //mxd. Named 'assassinDaggerTouch' in original logic.
{
	if (other == self->owner || other->owner == self->owner)
		return;

	// Are we reflecting?
	if (EntReflecting(other, true, true) && self->reflect_debounce_time > 0)
	{
		Create_rand_relect_vect(self->velocity, self->velocity);
		Vec3ScaleAssign(ASSASSIN_DAGGER_SPEED / 2.0f, self->velocity);
		AssassinDaggerReflect(self, other, self->velocity);

		return;
	}

	if (surface != NULL && (surface->flags & SURF_SKY))
	{
		SkyFly(self);
		return;
	}

	// Take into account if angle is within 45 of 0?
	vec3_t normal;
	VectorCopy(vec3_up, normal);

	if (plane != NULL && Vec3NotZero(plane->normal)) //BUGFIX: mxd. Use Vec3NotZero() instead of always-true plane->normal NULL check.
		VectorCopy(plane->normal, normal);

	if (other->takedamage != DAMAGE_NO)
	{
		const int snd_id = ((other->materialtype == MAT_FLESH || other->client != NULL) ? SND_DAGHITF : SND_DAGHITW);
		gi.sound(self, CHAN_AUTO, sounds[snd_id], 1.0f, ATTN_NORM, 0.0f);

		float damage = flrand(ASSASSIN_MIN_DAMAGE, ASSASSIN_MAX_DAMAGE);
		if (SKILL >= SKILL_HARD && fabsf(self->s.angles[PITCH]) < 45.0f) // Up to extra 10 pts damage if pointed correctly AND on hard skill.
			damage += 45.0f / (45.0f - fabsf(self->s.angles[PITCH])) * 10.0f;

		T_Damage(other, self, self->owner, self->movedir, self->s.origin, normal, (int)damage, 0, 0, MOD_DIED);
	}
	else // Spark.
	{
		vec3_t hit_angles;
		vectoangles(normal, hit_angles); //mxd. Removed unnecessary Vec3NotZero() normal check (already done above).

		gi.CreateEffect(NULL, FX_SPARKS, 0, self->s.origin, "d", hit_angles);
		gi.sound(self, CHAN_AUTO, sounds[SND_DAGHITW], 1.0f, ATTN_NORM, 0.0f);
	}

	G_FreeEdict(self);
}

// Create the guts of the dagger.
static void AssassinDaggerInit(edict_t* dagger) //mxd. Named 'create_assassin_dagger' in original logic.
{
	dagger->movetype = MOVETYPE_FLYMISSILE;
	dagger->solid = SOLID_BBOX;
	dagger->classname = "Assassin_Dagger";
	dagger->gravity = 0.0f;
	dagger->clipmask = MASK_SHOT;
	dagger->s.effects |= EF_CAMERA_NO_CLIP;
	dagger->svflags |= SVF_ALWAYS_SEND;
	dagger->s.scale = 0.5f;

	dagger->touch = AssassinDaggerTouch;
	dagger->think = G_FreeEdict;

	VectorSet(dagger->mins, -1.0f, -1.0f, -1.0f);
	VectorSet(dagger->maxs,  1.0f,  1.0f,  1.0f);
}

static void AssassinThrowDagger(edict_t* self, const float right_ofs) //mxd. Named 'assassinThrowDagger' in original logic.
{
	//FIXME: adjust for up/down.
	self->monsterinfo.attack_finished = level.time + 0.4f;

	edict_t* dagger = G_Spawn();

	AssassinDaggerInit(dagger);

	dagger->reflect_debounce_time = MAX_REFLECT;
	dagger->nextthink = level.time + 3.0f;
	dagger->enemy = self->enemy;
	dagger->owner = self;

	vec3_t enemy_pos;
	VectorCopy(self->enemy->s.origin, enemy_pos);
	enemy_pos[2] += (float)self->enemy->viewheight;

	vec3_t forward;
	vec3_t right;
	AngleVectors(self->s.angles, forward, right, NULL);

	VectorCopy(self->s.origin, dagger->s.origin);
	dagger->s.origin[2] += 8.0f;

	VectorMA(dagger->s.origin, 8.0f, forward, dagger->s.origin);
	VectorMA(dagger->s.origin, right_ofs, right, dagger->s.origin);
	VectorCopy(self->movedir, dagger->movedir);
	vectoangles(forward, dagger->s.angles);

	vec3_t check_lead;
	ExtrapolateFireDirection(self, dagger->s.origin, ASSASSIN_DAGGER_SPEED, self->enemy, 0.3f, check_lead);

	vec3_t enemy_dir;
	VectorSubtract(enemy_pos, dagger->s.origin, enemy_dir);
	const float enemy_dist = VectorNormalize(enemy_dir);

	if (Vec3IsZero(check_lead))
	{
		if (DotProduct(enemy_dir, forward) > 0.3f)
			VectorScale(enemy_dir, ASSASSIN_DAGGER_SPEED, dagger->velocity);
		else
			VectorScale(forward, ASSASSIN_DAGGER_SPEED, dagger->velocity);
	}
	else
	{
		VectorScale(check_lead, ASSASSIN_DAGGER_SPEED, dagger->velocity);
	}

	VectorCopy(dagger->velocity, dagger->movedir);
	VectorNormalize(dagger->movedir);
	vectoangles(dagger->movedir, dagger->s.angles);
	dagger->s.angles[PITCH] = -90.0f;

	const float eta = enemy_dist / ASSASSIN_DAGGER_SPEED;
	dagger->avelocity[PITCH] = -1.0f / eta * (360.0f * 3.0f + 30.0f + flrand(-10.0f, 10.0f)); // Ideally, spin @1110 degrees in 1 sec.

	gi.CreateEffect(&dagger->s, FX_M_EFFECTS, 0, dagger->avelocity, "bv", FX_ASS_DAGGER, dagger->velocity);

	G_LinkMissile(dagger);
}

#pragma endregion

#pragma region ========================== Cloaking utility functions ==========================

static qboolean AssassinCheckCloak(const edict_t* self) //mxd. Named 'assassinCheckCloak' in original logic.
{
	if (!self->monsterinfo.awake || self->monsterinfo.misc_debounce_time > level.time || (self->spawnflags & MSF_ASS_NOSHADOW)) // misc_debounce_time == Cloak debounce time.
		return false;

	if (self->ai_mood == AI_MOOD_FLEE)
		return true;

	if (self->enemy == NULL)
		return false;

	const int chance = (AI_IsInfrontOf(self->enemy, self) ? -3 : 0);

	return (irand(0, 10 - SKILL + chance) <= 0);
}

static qboolean AssassinCheckDeCloak(const edict_t* self) //mxd. Named 'assassinCheckDeCloak' in original logic.
{
	if (!self->monsterinfo.awake || self->monsterinfo.misc_debounce_time > level.time) // misc_debounce_time == Cloak debounce time.
		return false;

	if (self->enemy == NULL)
		return !(self->spawnflags & MSF_ASS_STARTSHADOW);

	if (M_DistanceToTarget(self, self->enemy) < ASSASSIN_MIN_CLOAK_RANGE)
		return true;

	const int chance = (AI_IsInfrontOf(self->enemy, self) ? 0 : -3);

	return (irand(0, 10 + SKILL * 2 + chance) <= 0);
}

void AssassinCloakPreThink(edict_t* self) //mxd. Named 'assassinCloakThink' in original logic.
{
	self->next_pre_think = level.time + FRAMETIME;

	// Check cloak or decloak.
	if (!(self->s.renderfx & RF_ALPHA_TEXTURE))
	{
		// Not cloaked.
		if (AssassinCheckCloak(self))
		{
			self->monsterinfo.misc_debounce_time = level.time + 7.0f; // 7 seconds before will willingly uncloak.
			assassin_init_cloak(self);
		}
	}
	else
	{
		// Cloaked.
		if (AssassinCheckDeCloak(self))
			AssassinInitDeCloak(self);
	}

	// Check to teleport.

	// Dumbed down.
	if (SKILL == SKILL_EASY && self->touch_debounce_time > level.time) // Was skill->value < 2.
		return;

	if (self->waterlevel == 3 && self->air_finished <= level.time) // Going to drown!
	{
		// Pick either last buoy or my startspot.
		vec3_t teleport_dest;

		if (self->lastbuoy > NULL_BUOY && !(gi.pointcontents(level.buoy_list[self->lastbuoy].origin) & MASK_WATER))
			VectorCopy(level.buoy_list[self->lastbuoy].origin, teleport_dest);
		else
			VectorCopy(self->assassin_spawn_pos, teleport_dest);

		vec3_t start_pos;
		VectorCopy(teleport_dest, start_pos);

		vec3_t end_pos; //TODO: UNINITIALIZED! Should be self.origin?

		vec3_t mins;
		VectorCopy(self->mins, mins);
		mins[2] = 0.0f;

		vec3_t maxs;
		VectorCopy(self->maxs, maxs);
		maxs[2] = 1.0f;

		start_pos[2] -= self->size[2];

		trace_t trace;
		gi.trace(start_pos, mins, maxs, end_pos, self, MASK_MONSTERSOLID, &trace);

		if (!trace.allsolid && !trace.startsolid)
		{
			VectorCopy(trace.endpos, start_pos);
			start_pos[2] += self->size[2];

			VectorCopy(trace.endpos, end_pos);

			gi.trace(start_pos, self->mins, self->maxs, end_pos, self, MASK_MONSTERSOLID, &trace);

			if (trace.fraction == 1.0f && !trace.allsolid && !trace.startsolid)
			{
				AssassinPrepareTeleportDestination(self, trace.endpos, false);
				return;
			}
		}
	}

	if (SKILL > SKILL_EASY || (self->spawnflags & MSF_ASS_TELEPORTDODGE))
	{
		// Pussies were complaining about assassins teleporting away from certain death, so don't do that unless in hard.
		if (!(self->spawnflags & MSF_ASS_NOTELEPORT) && !(self->spawnflags & MSF_FIXED) && self->groundentity != NULL && irand(0, 4 - SKILL) <= 0)
		{
			// Easy is 40% chance per second, hard is 60% chance to check per second.
			edict_t* found = NULL;
			while ((found = FindInRadius(found, self->s.origin, 200.0f + skill->value * 50.0f)) != NULL)
			{
				if (Q_stricmp(found->classname, "Spell_Maceball") == 0) //mxd. stricmp -> Q_stricmp
				{
					if (self->enemy == NULL && found->owner != NULL)
					{
						self->enemy = found->owner;
						AI_FoundTarget(self, false);
					}

					if (AssassinChooseTeleportDestination(self, ASS_TP_OFF, true, true))
						return;
				}
				else if (Q_stricmp(found->classname, "Spell_RedRain") == 0 || Q_stricmp(found->classname, "Spell_PhoenixArrow") == 0 ||
					Q_stricmp(found->classname, "Spell_FireWall") == 0 || Q_stricmp(found->classname, "Spell_SphereOfAnnihilation") == 0) //mxd. stricmp -> Q_stricmp
				{
					if (self->enemy == NULL && found->owner != NULL)
					{
						self->enemy = found->owner;
						AI_FoundTarget(self, false);
					}

					if (AssassinChooseTeleportDestination(self, ASS_TP_ANY, true, false))
						return;
				}

				if (found == self->enemy && found->client != NULL && M_DistanceToTarget(self, self->enemy) < 128.0f)
				{
					if (AI_IsInfrontOf(self->enemy, self))
					{
						// Is he using his staff or jumping into me?
						switch (found->client->playerinfo.lowerseq)
						{
							case ASEQ_WSWORD_SPIN:
							case ASEQ_WSWORD_SPIN2:
							case ASEQ_WSWORD_STEP2:
							case ASEQ_WSWORD_STEP:
							case ASEQ_POLEVAULT2:
							case ASEQ_POLEVAULT1_W:
							case ASEQ_POLEVAULT1_R:
								if (AssassinChooseTeleportDestination(self, ASS_TP_ANY, true, true))
									return;
								break;
							default:
								break;
						}

						switch (found->client->playerinfo.upperseq) //TODO: why do we need to check both lowerseq and upperseq?..
						{
							case ASEQ_WSWORD_SPIN:
							case ASEQ_WSWORD_SPIN2:
							case ASEQ_WSWORD_STEP2:
							case ASEQ_WSWORD_STEP:
							case ASEQ_POLEVAULT2:
							case ASEQ_POLEVAULT1_W:
							case ASEQ_POLEVAULT1_R:
								if (AssassinChooseTeleportDestination(self, ASS_TP_ANY, true, true))
									return;
								break;

							default:
								break;
						}
					}

					if (found->client->playerinfo.shield_timer > level.time && AssassinChooseTeleportDestination(self, ASS_TP_OFF, true, true))
						return;
				}
			} // while loop end.
		}
	}

	if (self->evade_debounce_time < level.time)
		MG_CheckEvade(self);
}

void AssassinCloakFadePreThink(edict_t* self) //mxd. Named 'assassinCloak' in original logic.
{
#define FADE_INCREMENT	15 //mxd

	for (int i = 0; i < 3; i++) // Fade out RGB.
		if (self->s.color.c_array[i] > 50)
			self->s.color.c_array[i] -= (byte)irand(0, FADE_INCREMENT * 3);

	if (self->s.color.a > 150) // Fade out Alpha.
		self->s.color.a -= (byte)irand(0, FADE_INCREMENT);

	if (self->s.color.r <= 50 && self->s.color.g <= 50 && self->s.color.b <= 50 && self->s.color.a <= 150)
		self->pre_think = AssassinCloakPreThink;

	self->next_pre_think = level.time + FRAMETIME;

	if (self->evade_debounce_time < level.time)
		MG_CheckEvade(self);
}

void AssassinDeCloakFadePreThink(edict_t* self) //mxd. Named 'assassinDeCloak' in original logic.
{
	if (!(self->s.renderfx & RF_ALPHA_TEXTURE))
		return;

	for (int i = 0; i < 3; i++) // Fade in RGB.
	{
		if (self->s.color.c_array[i] < 255 - 10)
			self->s.color.c_array[i] += 10;
		else
			self->s.color.c_array[i] = 255;
	}

	if (self->s.color.a < 255 - 5) // Fade in Alpha.
		self->s.color.a += 5;
	else
		self->s.color.a = 255;

	// Fade-in complete?
	if (self->s.color.c == 0xffffffff)
	{
		self->svflags &= ~SVF_NO_AUTOTARGET;
		self->s.renderfx &= ~RF_ALPHA_TEXTURE;

		if (self->health > 0)
		{
			self->pre_think = AssassinCloakPreThink;
			self->next_pre_think = level.time + FRAMETIME;
		}
		else
		{
			self->pre_think = NULL;
			self->next_pre_think = -1.0f;
		}
	}
	else
	{
		self->pre_think = AssassinDeCloakFadePreThink;
		self->next_pre_think = level.time + FRAMETIME;
	}

	if (self->evade_debounce_time < level.time)
		MG_CheckEvade(self);
}

static void AssassinInitDeCloak(edict_t* self) //mxd. Named 'assassinInitDeCloak' in original logic.
{
	gi.sound(self, CHAN_AUTO, sounds[SND_DECLOAK], 1.0f, ATTN_NORM, 0.0f);

	self->pre_think = AssassinDeCloakFadePreThink;
	self->next_pre_think = level.time + FRAMETIME;
}

#pragma endregion

static void AssassinSmoke(const edict_t* self) //mxd. Named 'assassinSmoke' in original logic.
{
	vec3_t pos;
	VectorCopy(self->s.origin, pos);
	pos[2] += self->mins[2];

	gi.CreateEffect(NULL, FX_TPORTSMOKE, 0, pos, "");
}

#pragma region ========================== Teleport utility functions ==========================

void AssassinPrepareTeleportDestination(edict_t* self, const vec3_t spot, const qboolean instant) //mxd. Named 'assassinPrepareTeleportDest' in original logic.
{
	if ((self->s.renderfx & RF_ALPHA_TEXTURE) && self->pre_think != AssassinDeCloakFadePreThink)
	{
		AssassinInitDeCloak(self);
		self->monsterinfo.misc_debounce_time = level.time + 3.0f;
	}

	VectorCopy(spot, self->assassin_teleport_pos); //TODO: pos2/assassin_teleport_pos is not a saveable field!

	self->placeholder = G_Spawn();
	VectorCopy(self->assassin_teleport_pos, self->placeholder->s.origin);
	self->placeholder->solid = SOLID_BBOX;
	VectorCopy(self->mins, self->placeholder->mins);
	VectorCopy(self->maxs, self->placeholder->maxs);
	self->placeholder->think = G_FreeEdict;
	self->placeholder->nextthink = level.time + 2.0f; // Just in case.

	// Dumbed down.
	if (instant && SKILL > SKILL_MEDIUM)
	{
		assassin_ready_teleport(self);
		assassin_gone(self);
	}
	else
	{
		SetAnim(self, ANIM_TELEPORT);
	}
}

static qboolean AssassinChooseTeleportDestination(edict_t* self, const int type, const qboolean imperative, const qboolean instant) //mxd. Named 'assassinChooseTeleportDestination' in original logic.
{
	if (self->enemy == NULL || (self->spawnflags & MSF_FIXED)) //FIXME: choose my spot?
		return false;

	const int num_tries = (imperative ? (SKILL + 1) * 10 : 1);

	for (int i = 0; i < num_tries; i++)
	{
		int	chance;

		if (type == ASS_TP_OFF)
			chance = irand(0, 66);
		else if (type == ASS_TP_DEF)
			chance = irand(33, 100);
		else // ASS_TP_ANY
			chance = irand(0, 100);

		vec3_t start_pos;
		vec3_t end_pos;
		edict_t* noblock_ent;
		float trace_dist;

		if (chance < 33)
		{
			// ANY, OFF to behind enemy.
			vec3_t forward;
			const vec3_t teleport_angles = { 0.0f, anglemod(self->enemy->s.angles[YAW] + flrand(-90.0f, 90.0f)), 0.0f };
			AngleVectors(teleport_angles, forward, NULL, NULL);

			VectorCopy(self->enemy->s.origin, start_pos);
			start_pos[2] += self->enemy->mins[2] - self->mins[2];

			trace_dist = flrand(self->min_missile_range, self->missile_range); //mxd. irand() in original logic.
			VectorMA(start_pos, -trace_dist, forward, end_pos);
			noblock_ent = self->enemy;
		}
		else if (chance < 66)
		{
			// ANY to anywhere around enemy.
			vec3_t forward;
			const vec3_t teleport_angles = { 0.0f, anglemod(flrand(0.0f, 360.0f)), 0.0f }; //TODO: should be flrand(0.0f, 359.0f)?
			AngleVectors(teleport_angles, forward, NULL, NULL);

			VectorCopy(self->enemy->s.origin, start_pos);
			start_pos[2] += self->enemy->mins[2] - self->mins[2];

			trace_dist = flrand(self->min_missile_range, self->missile_range); //mxd. irand() in original logic.
			VectorMA(start_pos, -trace_dist, forward, end_pos);
			noblock_ent = self->enemy;
		}
		else
		{
			// ANY, DEF to anywhere around me.
			vec3_t forward;
			const vec3_t teleport_angles = { 0.0f, anglemod(flrand(0.0f, 360.0f)), 0.0f }; //TODO: should be flrand(0.0f, 359.0f)?
			AngleVectors(teleport_angles, forward, NULL, NULL);

			VectorCopy(self->s.origin, start_pos);

			trace_dist = flrand(self->min_missile_range, self->missile_range / 2.0f); //mxd. irand() in original logic.
			VectorMA(start_pos, -trace_dist, forward, end_pos);
			noblock_ent = self;
		}

		trace_t trace;
		gi.trace(start_pos, self->mins, self->maxs, end_pos, noblock_ent, MASK_MONSTERSOLID, &trace);

		if (trace.allsolid || trace.startsolid || trace.fraction * trace_dist < 100.0f) // Minimum origin lerp distance.
			continue;

		if (vhlen(trace.endpos, self->enemy->s.origin) >= self->min_missile_range)
		{
			VectorCopy(trace.endpos, start_pos);
			VectorCopy(trace.endpos, end_pos);
			end_pos[2] -= 64.0f;

			gi.trace(start_pos, self->mins, self->maxs, end_pos, noblock_ent, MASK_MONSTERSOLID, &trace);

			if (trace.fraction < 1.0f && !trace.allsolid && !trace.startsolid) // The last two should be false if trace.fraction is < 1.0 but doesn't hurt to check.
			{
				AssassinPrepareTeleportDestination(self, trace.endpos, instant);
				return true;
			}
		}
	}

	return false;
}

static qboolean AssassinCheckTeleport(edict_t* self, const int type) //mxd. Named 'assassinCheckTeleport' in original logic.
{
	if ((self->spawnflags & (MSF_ASS_NOTELEPORT | MSF_FIXED)) || self->groundentity == NULL || !M_ValidTarget(self, self->enemy))
		return false;

	return AssassinChooseTeleportDestination(self, type, false, false);
}

#pragma endregion

#pragma region ========================== Action functions ==========================

// Do melee or ranged attack.
void assassin_attack(edict_t* self, const float flags) //mxd. Named 'assassindagger' in original logic.
{
	if (self->enemy == NULL || self->enemy->health < 0)
	{
		G_PostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
		return;
	}

	vec3_t diff;
	VectorSubtract(self->s.origin, self->enemy->s.origin, diff);
	const float dist = VectorLength(diff);

	if (dist <= (self->maxs[0] + self->enemy->maxs[0] + 56.0f)) // Do melee attack?
	{
		if (AI_IsInfrontOf(self, self->enemy))
		{
			//TODO: play sound regardless of whether AI_IsInfrontOf() check succeeds?
			const int snd_id = (irand(0, 3) == 0 ? SND_SLASH2 : SND_SLASH1); //mxd. 25% chance of SND_SLASH2. Original logic uses only SND_SLASH1. //TODO: use SND_SLASH2 for backstab attack only?
			gi.sound(self, CHAN_WEAPON, sounds[snd_id], 1.0f, ATTN_NORM, 0.0f);

			vec3_t origin;
			const vec3_t offset = { 35.0f, 0.0f, 32.0f };
			VectorGetOffsetOrigin(offset, self->s.origin, self->s.angles[YAW], origin);

			vec3_t angles;
			VectorCopy(self->s.angles, angles);
			angles[YAW] += DEGREE_90;

			vec3_t forward;
			AngleVectors(angles, forward, NULL, NULL);

			int damage = irand(ASSASSIN_MIN_DAMAGE, ASSASSIN_MAX_DAMAGE);

			if (SKILL >= SKILL_HARD && !AI_IsInfrontOf(self->enemy, self))
			{
				// Backstab!
				damage = self->enemy->health + irand(-20, 10);
				damage = max(ASSASSIN_MAX_DAMAGE, damage);
			}

			T_Damage(self->enemy, self, self, forward, origin, vec3_origin, damage, 0, 0, MOD_DIED);
		}
	}
	else // Do ranged attack?
	{
		int throw_num = 0;

		if ((int)flags & BIT_RKNIFE)
		{
			throw_num++;

			// Turn off right dagger model?
			if (!(self->s.fmnodeinfo[MESH__RKNIFE].flags & FMNI_NO_DRAW))
			{
				self->s.fmnodeinfo[MESH__RKNIFE].flags |= FMNI_NO_DRAW;
				AssassinThrowDagger(self, 12.0f);
			}
		}

		if ((int)flags & BIT_LKNIFE)
		{
			throw_num++;

			// Turn off left dagger model?
			if (!(self->s.fmnodeinfo[MESH__LKNIFE].flags & FMNI_NO_DRAW))
			{
				self->s.fmnodeinfo[MESH__LKNIFE].flags |= FMNI_NO_DRAW;
				AssassinThrowDagger(self, -12.0f);
			}
		}

		const int snd_id = (throw_num > 1 ? SND_THROW2 : SND_THROW1); //mxd
		gi.sound(self, CHAN_WEAPON, sounds[snd_id], 1.0f, ATTN_NORM, 0.0f);
	}
}

void assassin_dead(edict_t* self)
{
	self->msgHandler = DeadMsgHandler;
	self->dead_state = DEAD_DEAD;
	M_EndDeath(self);
}

void assassin_growl(edict_t* self) //mxd. Named 'assassingrowl' in original logic.
{
	if (irand(0, 20) == 0)
		gi.sound(self, CHAN_AUTO, sounds[irand(SND_GROWL1, SND_GROWL3)], 1.0f, ATTN_IDLE, 0.0f);
}

void assassin_post_pain(edict_t* self)
{
	if (self->fire_damage_time < level.time && AssassinCheckTeleport(self, ASS_TP_ANY)) // Don't teleport if burning.
		return;

	assassin_pause(self);
}

void assassin_skip_frame_skill_check(edict_t* self) //mxd. Named 'assassinSkipFrameSkillCheck' in original logic.
{
	if (irand(0, 3) < SKILL)
		self->s.frame++;
}

void assassin_pause(edict_t* self)
{
	// This gets stuck on, sometimes.
	self->s.fmnodeinfo[MESH__LKNIFE].flags |= FMNI_NO_DRAW;
	self->s.fmnodeinfo[MESH__RKNIFE].flags |= FMNI_NO_DRAW;

	if (self->monsterinfo.aiflags & AI_OVERRIDE_GUIDE)
		return;

	if ((self->spawnflags & MSF_FIXED) && self->curAnimID == ANIM_DELAY && self->enemy != NULL)
	{
		self->monsterinfo.searchType = SEARCH_COMMON;
		MG_FaceGoal(self, true);
	}

	self->mood_think(self);

	switch (self->ai_mood)
	{
		case AI_MOOD_NORMAL:
		{
			if (FindTarget(self))
			{
				vec3_t diff;
				VectorSubtract(self->s.origin, self->enemy->s.origin, diff);

				const int anim_id = ((VectorLength(diff) > 80.0f || (self->monsterinfo.aiflags & AI_FLEE)) ? MSG_RUN : MSG_MELEE);
				G_PostMessage(self, anim_id, PRI_DIRECTIVE, NULL);
			}
		} break;

		case AI_MOOD_ATTACK:
		{
			const int anim_id = ((self->ai_mood_flags & AI_MOOD_FLAG_MISSILE) ? MSG_MISSILE : MSG_MELEE); //mxd
			G_PostMessage(self, anim_id, PRI_DIRECTIVE, NULL);
		} break;

		case AI_MOOD_PURSUE:
		case AI_MOOD_NAVIGATE:
			G_PostMessage(self, MSG_RUN, PRI_DIRECTIVE, NULL);
			break;

		case AI_MOOD_STAND:
			G_PostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
			break;

		case AI_MOOD_WALK:
			G_PostMessage(self, MSG_WALK, PRI_DIRECTIVE, NULL);
			break;

		case AI_MOOD_DELAY:
			SetAnim(self, ANIM_DELAY);
			break;

		case AI_MOOD_WANDER:
			if (self->spawnflags & MSF_FIXED)
				SetAnim(self, ANIM_DELAY);
			else
				G_PostMessage(self, MSG_WALK, PRI_DIRECTIVE, NULL);
			break;

		case AI_MOOD_JUMP:
		{
			const int anim_id = ((self->spawnflags & MSF_FIXED) ? ANIM_DELAY : ANIM_FJUMP); // ANIM_FJUMP will apply the movedir when he's ready.
			SetAnim(self, anim_id);
		} break;

		default:
			break;
	}
}

void assassin_run(edict_t* self, float dist) //mxd. Named 'assassin_go_run' in original logic.
{
	if (self->maxs[2] == 0.0f)
		assassin_unset_crouched(self);

	if (self->enemy != NULL)
		MG_AI_Run(self, dist);
	else
		ai_walk(self, dist);
}

void assassin_crouch_idle_decision(edict_t* self)
{
	//FIXME: need to uncrouch.
	const int chance = irand(0, 100);

	switch (self->curAnimID)
	{
		case ANIM_CROUCH_IDLE:
			if (chance < 55)
				SetAnim(self, ANIM_CROUCH_IDLE);
			else if (chance < 75)
				SetAnim(self, ANIM_CROUCH_POKE);
			else if (chance < 85)
				SetAnim(self, ANIM_CROUCH_LOOK_RIGHT);
			else if (chance < 95)
				SetAnim(self, ANIM_CROUCH_LOOK_LEFT);
			else
				SetAnim(self, ANIM_CROUCH_END);
			break;

		case ANIM_CROUCH_LOOK_RIGHT:
		case ANIM_CROUCH_LOOK_RIGHT_IDLE:
		case ANIM_CROUCH_LOOK_L2R:
			if (chance < 60)
				SetAnim(self, ANIM_CROUCH_LOOK_RIGHT_IDLE);
			else if (chance < 85)
				SetAnim(self, ANIM_CROUCH_LOOK_R2C);
			else
				SetAnim(self, ANIM_CROUCH_LOOK_R2L);
			break;

		case ANIM_CROUCH_LOOK_LEFT:
		case ANIM_CROUCH_LOOK_LEFT_IDLE:
		case ANIM_CROUCH_LOOK_R2L:
			if (chance < 60)
				SetAnim(self, ANIM_CROUCH_LOOK_LEFT_IDLE);
			else if (chance < 85)
				SetAnim(self, ANIM_CROUCH_LOOK_L2C);
			else
				SetAnim(self, ANIM_CROUCH_LOOK_L2R);
			break;

		case ANIM_CROUCH_TRANS:
			self->monsterinfo.pausetime = FLT_MAX; //mxd. 99999999 in original logic.
			SetAnim(self, ANIM_CROUCH_IDLE);
			break;

		case ANIM_CROUCH_LOOK_R2C:
		case ANIM_CROUCH_LOOK_L2C:
		case ANIM_CROUCH_POKE:
			SetAnim(self, ANIM_CROUCH_IDLE);
			break;

		case ANIM_CROUCH_END:
			self->damage_debounce_time = level.time + 10.0f;
			self->monsterinfo.pausetime = -1.0f;
			SetAnim(self, ANIM_STAND);
			break;

		default:
			SetAnim(self, ANIM_CROUCH_END);
			break;
	}
}

void assassin_ai_walk(edict_t* self, float dist)
{
	if (self->damage_debounce_time < level.time)
	{
		const edict_t* target = ((self->enemy != NULL) ? self->enemy : self->oldenemy); //mxd

		if (target != NULL && vhlen(self->s.origin, target->s.origin) < 48.0f && AI_IsInfrontOf(self, target))
		{
			assassin_enable_fmnode(self, MESH__LKNIFE);
			SetAnim(self, ANIM_CROUCH_TRANS);

			return;
		}
	}

	ai_walk(self, dist);
}

void assassin_walk_loop_go(edict_t* self) //mxd. Named 'assasin_walk_loop_go' in original logic.
{
	SetAnim(self, ANIM_WALK_LOOP);
}

void assassin_crouched_check_attack(edict_t* self, float attack) //mxd. Named 'assassinCrouchedCheckAttack' in original logic.
{
	if (irand(0, 10) < 5 || !AI_IsClearlyVisible(self, self->enemy) || !AI_IsInfrontOf(self, self->enemy))
		return;

	if (attack == 1.0f)
		assassin_attack(self, BIT_RKNIFE);
	else if (attack == 2.0f) // Start crouched attack animation.
		SetAnim(self, ANIM_DAGGERC);
	else // Loop back inside that anim.
		self->monsterinfo.currframeindex = FRAME_ataka1; //mxd. Use define.
}

void assassin_enable_fmnode(edict_t* self, float node) //mxd. Named 'assassinNodeOn' in original logic.
{
	self->s.fmnodeinfo[(int)node].flags &= ~FMNI_NO_DRAW;
}

void assassin_stop(edict_t* self) //mxd. Named 'assassinStop' in original logic.
{
	if (self->evade_debounce_time - level.time > 0.1f)
		self->nextthink = level.time + (self->evade_debounce_time - level.time);
}

void assassin_set_crouched(edict_t* self) //mxd. Named 'assassinSetCrouched' in original logic.
{
	VectorSet(self->maxs, 16.0f, 16.0f, 0.0f);
	self->viewheight = 0;
}

void assassin_unset_crouched(edict_t* self) //mxd. Named 'assassinUndoCrouched' in original logic.
{
	VectorSet(self->maxs, 16.0f, 16.0f, 48.0f);
	self->viewheight = 40;
}

void assassin_sound(edict_t* self, float channel, float sound_num, float attenuation)
{
	gi.sound(self, (int)channel, sounds[(int)sound_num], 1.0f, attenuation, 0.0f);
}

void assassin_jump_go(edict_t* self, float forward_speed, float up_speed, float right_speed) //mxd. Named 'assassinGoJump' in original logic.
{
	//FIXME: do checks and traces first.
	self->monsterinfo.aiflags &= ~AI_OVERRIDE_GUIDE;
	assassin_sound(self, CHAN_VOICE, SND_JUMP, ATTN_NORM);

	vec3_t forward;
	vec3_t right;
	vec3_t up;
	AngleVectors(self->s.angles, forward, right, up);

	VectorMA(self->velocity, up_speed, up, self->velocity);
	VectorMA(self->velocity, forward_speed, forward, self->velocity);
	VectorMA(self->velocity, right_speed, right, self->velocity);
}

void assassin_inair_go(edict_t* self) //mxd. Named 'assassin_go_inair' in original logic.
{
	SetAnim(self, ANIM_INAIR);
}

void assassin_evade_inair_go(edict_t* self) //mxd. Named 'assassin_go_evinair' in original logic.
{
	SetAnim(self, ANIM_EVINAIR);
}

void assassin_fwdflip_inair_go(edict_t* self) //mxd. Named 'assassin_go_ffinair' in original logic.
{
	SetAnim(self, ANIM_FFINAIR);
}

void assassin_backflip_inair_go(edict_t* self) //mxd. Named 'assassin_go_bfinair' in original logic.
{
	SetAnim(self, ANIM_BFINAIR);
}

void assassin_check_loop(edict_t* self, float frame) //mxd. Named 'assassin_go_bfinair' in original logic.
{
#define MELEE_RANGE	64.0f //mxd //TODO: move to m_stats.h?
#define JUMP_RANGE	128.0f //mxd //TODO: move to m_stats.h?

	// See if should fire again.
	if (self->enemy == NULL)
		return;

	ai_charge2(self, 0);

	if (!AI_IsClearlyVisible(self, self->enemy) || !AI_IsInfrontOf(self, self->enemy))
		return;

	if (irand(0, 100) < self->bypass_missile_chance)
	{
		self->monsterinfo.attack_finished = level.time + 3.0f - skill->value;
		return;
	}

	if (self->ai_mood_flags & AI_MOOD_FLAG_BACKSTAB)
		return;

	vec3_t diff;
	VectorSubtract(self->s.origin, self->enemy->s.origin, diff);

	const float dist = VectorLength(diff);
	const float min_separation = self->maxs[0] + self->enemy->maxs[0];

	//mxd. Skip unnecessary AI_IsInfrontOf() check (already checked above).

	// Don't loop if enemy close enough.
	if (dist < min_separation + MELEE_RANGE)
		return;

	if (dist < min_separation + JUMP_RANGE && irand(0, 10) < 3)
		return;

	self->monsterinfo.currframeindex = (int)frame;
}

void assassin_gone(edict_t* self) //mxd. Named 'assassinGone' in original logic.
{
	if (self->placeholder != NULL)
		G_FreeEdict(self->placeholder);

	VectorCopy(self->assassin_teleport_pos, self->s.origin);

	if (self->enemy != NULL)
	{
		// Face enemy.
		vec3_t enemy_dir;
		VectorSubtract(self->enemy->s.origin, self->s.origin, enemy_dir);
		self->s.angles[YAW] = anglemod(VectorYaw(enemy_dir));
	}

	AssassinSmoke(self);

	vec3_t pos;
	VectorCopy(self->assassin_teleport_pos, pos);
	pos[2] += 100.0f;

	if (gi.pointcontents(pos) == CONTENTS_EMPTY && irand(0, 3) == 0)
		SetAnim(self, ANIM_EVFRONTFLIP); //mxd. Inline assassinFrontFlip().
	else
		SetAnim(self, ANIM_UNCROUCH);

	self->monsterinfo.aiflags &= ~AI_OVERRIDE_GUIDE;
	self->svflags &= ~SVF_NO_AUTOTARGET;
	self->touch_debounce_time = level.time + (10.0f - skill->value * 3.0f); // Dumbed down.

	// Should we clear velocity too?
	gi.linkentity(self);
}

void assassin_ready_teleport(edict_t* self) //mxd. Named 'assassinReadyTeleport' in original logic.
{
	AssassinSmoke(self);
	self->svflags |= SVF_NO_AUTOTARGET;
}

void assassin_uncrouch(edict_t* self) //mxd. Named 'assassinUnCrouch' in original logic.
{
	SetAnim(self, ANIM_UNCROUCH);
}

void assassin_init_cloak(edict_t* self) //mxd. Named 'assassinInitCloak' in original logic.
{
	gi.sound(self, CHAN_AUTO, sounds[SND_CLOAK], 1.0f, ATTN_NORM, 0.0f);

	self->svflags |= SVF_NO_AUTOTARGET;
	self->s.renderfx |= RF_ALPHA_TEXTURE;
	self->s.color.c = 0xffffffff;
	self->pre_think = AssassinCloakFadePreThink;
	self->next_pre_think = level.time + FRAMETIME;
}

#pragma endregion

#pragma region ========================== AssassinDismember logic ==========================

static qboolean AssassinCanThrowNode(edict_t* self, const int node_id, int* throw_nodes) //mxd. Named 'canthrownode_as' in original logic.
{
	static const int bit_for_mesh_node[NUM_MESH_NODES] = //mxd. Made local static.
	{
		BIT_DADDYNULL,
		BIT_TORSOFT,
		BIT_TORSOBK,
		BIT_HEAD,
		BIT_LKNIFE,
		BIT_RKNIFE,
		BIT_R4ARM,
		BIT_L4ARM,
		BIT_HIPS,
		BIT_LCALF,
		BIT_RCALF,
		BIT_RTHIGH,
		BIT_LTHIGH,
		BIT_KNIFES,
		BIT_LUPARM,
		BIT_RUPARM
	};

	// See if it's on, if so, add it to throw_nodes. Turn it off on thrower.
	if (!(self->s.fmnodeinfo[node_id].flags & FMNI_NO_DRAW))
	{
		*throw_nodes |= bit_for_mesh_node[node_id];
		self->s.fmnodeinfo[node_id].flags |= FMNI_NO_DRAW;

		return true;
	}

	return false;
}

// THROWS weapon, turns off those nodes, sets that weapon as gone.
static void AssassinDropWeapon(edict_t* self, const int knife_flags) //mxd. Named 'assassin_dropweapon' in original logic.
{
	// NO PART FLY FRAME!
	vec3_t right;
	AngleVectors(self->s.angles, NULL, right, NULL);

	if (!(self->s.fmnodeinfo[MESH__LKNIFE].flags & FMNI_NO_DRAW) && (knife_flags & BIT_LKNIFE))
	{
		vec3_t hand_spot;
		VectorScale(right, -12.0f, hand_spot);

		ThrowWeapon(self, &hand_spot, BIT_LKNIFE, 0, FRAME_prtfly);
		self->s.fmnodeinfo[MESH__LKNIFE].flags |= FMNI_NO_DRAW;
	}

	if (!(self->s.fmnodeinfo[MESH__RKNIFE].flags & FMNI_NO_DRAW) && (knife_flags & BIT_RKNIFE))
	{
		vec3_t hand_spot;
		VectorScale(right, 12.0f, hand_spot);

		ThrowWeapon(self, &hand_spot, BIT_RKNIFE, 0, FRAME_prtfly);
		self->s.fmnodeinfo[MESH__RKNIFE].flags |= FMNI_NO_DRAW;
	}
}

static HitLocation_t AssassinConvertDeadHitLocation(const HitLocation_t hl) //mxd. Named 'assassin_convert_hitloc_dead' in original logic. //mxd. Changed arg and return type from int.
{
	switch (hl)
	{
		case hl_Head:
			return hl_TorsoFront;

		case hl_TorsoFront: // Split in half?
			return (irand(0, 1) == 0 ? hl_LegUpperRight : hl_LegUpperLeft);

		case hl_TorsoBack: // Split in half?
			return hl_Head;

		case hl_ArmUpperLeft:
			return hl_ArmLowerLeft;

		case hl_ArmLowerLeft: // Left arm.
			return hl_ArmUpperLeft;

		case hl_ArmUpperRight:
			return hl_ArmLowerRight;

		case hl_ArmLowerRight: // Right arm.
			return hl_ArmUpperRight;

		case hl_LegUpperLeft:
			return hl_LegLowerLeft;

		case hl_LegLowerLeft: // Left leg.
			return hl_LegUpperLeft;

		case hl_LegUpperRight:
			return hl_LegLowerRight;

		case hl_LegLowerRight: // Right leg.
			return hl_LegUpperRight;

		default:
			return irand(hl_Head, hl_LegLowerRight);
	}
}

static qboolean AssassinThrowHead(edict_t* self, float damage, const qboolean dismember_ok) //mxd. Split from AssassinDismember() to simplify logic.
{
	// Head already gone?
	if (self->s.fmnodeinfo[MESH__HEAD].flags & FMNI_NO_DRAW)
		return false;

	if (self->s.fmnodeinfo[MESH__HEAD].flags & FMNI_USE_SKIN)
		damage *= 1.5f; // Greater chance to cut off if previously damaged.

	if (dismember_ok && flrand(0.0f, (float)self->health) < damage * 0.3f)
	{
		int throw_nodes = 0;
		AssassinCanThrowNode(self, MESH__HEAD, &throw_nodes);

		vec3_t gore_spot = { 0.0f, 0.0f, 18.0f };
		ThrowBodyPart(self, &gore_spot, throw_nodes, damage, FRAME_prtfly);

		VectorAdd(self->s.origin, gore_spot, gore_spot);
		SprayDebris(self, gore_spot, 8, damage);

		if (self->health > 0)
		{
			self->health = 1;
			T_Damage(self, self, self, vec3_origin, vec3_origin, vec3_origin, 10, 20, 0, MOD_DIED);
		}

		return true;
	}

	// Switch head node to damaged skin.
	self->s.fmnodeinfo[MESH__HEAD].flags |= FMNI_USE_SKIN;
	self->s.fmnodeinfo[MESH__HEAD].skin = self->s.skinnum + 1;

	return false;
}

static qboolean AssassinThrowTorso(edict_t* self, float damage, const int mesh_part, const qboolean dismember_ok) //mxd. Split from AssassinDismember() to simplify logic.
{
	// Torso already gone?
	if (self->s.fmnodeinfo[mesh_part].flags & FMNI_NO_DRAW)
		return false;

	if (self->s.fmnodeinfo[mesh_part].flags & FMNI_USE_SKIN)
		damage *= 1.5f; // Greater chance to cut off if previously damaged.

	if (dismember_ok && flrand(0.0f, (float)self->health) < damage * 0.3f)
	{
		int throw_nodes = 0;
		AssassinCanThrowNode(self, MESH__TORSOFT, &throw_nodes);
		AssassinCanThrowNode(self, MESH__TORSOBK, &throw_nodes);
		AssassinCanThrowNode(self, MESH__HEAD, &throw_nodes);
		AssassinCanThrowNode(self, MESH__R4ARM, &throw_nodes);
		AssassinCanThrowNode(self, MESH__L4ARM, &throw_nodes);
		AssassinCanThrowNode(self, MESH__KNIFES, &throw_nodes);
		AssassinCanThrowNode(self, MESH__LUPARM, &throw_nodes);
		AssassinCanThrowNode(self, MESH__RUPARM, &throw_nodes);

		AssassinDropWeapon(self, BIT_LKNIFE | BIT_RKNIFE);

		vec3_t gore_spot = { 0.0f, 0.0f, 12.0f };
		ThrowBodyPart(self, &gore_spot, throw_nodes, damage, FRAME_torsofly);
		VectorAdd(self->s.origin, gore_spot, gore_spot);
		SprayDebris(self, gore_spot, 12, damage);

		if (self->health > 0)
		{
			self->health = 1;
			T_Damage(self, self, self, vec3_origin, vec3_origin, vec3_origin, 10, 20, 0, MOD_DIED);
		}

		return true;
	}

	// Switch torso node to damaged skin.
	self->s.fmnodeinfo[mesh_part].flags |= FMNI_USE_SKIN;
	self->s.fmnodeinfo[mesh_part].skin = self->s.skinnum + 1;

	return false;
}

static void AssassinThrowUpperArm(edict_t* self, float damage, const int mesh_part, const qboolean dismember_ok) //mxd. Split from AssassinDismember() to simplify logic.
{
	// Arm already gone?
	if (self->s.fmnodeinfo[mesh_part].flags & FMNI_NO_DRAW)
		return;

	const qboolean is_left_arm = (mesh_part == MESH__LUPARM); //mxd

	if (is_left_arm && (self->s.fmnodeinfo[mesh_part].flags & FMNI_USE_SKIN))
		damage *= 1.5f; // Greater chance to cut off left arm if previously damaged.

	if (dismember_ok && flrand(0.0f, (float)self->health) < damage * 0.75f)
	{
		int throw_nodes = 0;
		const int forearm_part = (is_left_arm ? MESH__L4ARM : MESH__R4ARM); //mxd
		AssassinCanThrowNode(self, forearm_part, &throw_nodes);

		if (AssassinCanThrowNode(self, mesh_part, &throw_nodes))
		{
			vec3_t right;
			AngleVectors(self->s.angles, NULL, right, NULL);

			vec3_t gore_spot = { 0.0f, 0.0f, self->maxs[2] * 0.3f };
			const float side = (is_left_arm ? -1.0f : 1.0f);
			VectorMA(gore_spot, 10.0f * side, right, gore_spot);

			const int knife_flag = (is_left_arm ? BIT_LKNIFE : BIT_RKNIFE); //mxd
			AssassinDropWeapon(self, knife_flag);
			ThrowBodyPart(self, &gore_spot, throw_nodes, damage, FRAME_prtfly);
		}
	}
	else
	{
		// Switch arm node to damaged skin.
		self->s.fmnodeinfo[mesh_part].flags |= FMNI_USE_SKIN;
		self->s.fmnodeinfo[mesh_part].skin = self->s.skinnum + 1;
	}
}

static void AssassinThrowLowerArm(edict_t* self, float damage, const int mesh_part, const qboolean dismember_ok) //mxd. Split from AssassinDismember() to simplify logic.
{
	if (self->s.fmnodeinfo[mesh_part].flags & FMNI_NO_DRAW)
		return;

	const qboolean is_left_arm = (mesh_part == MESH__L4ARM); //mxd

	if (is_left_arm && self->s.fmnodeinfo[mesh_part].flags & FMNI_USE_SKIN)
		damage *= 1.5f; // Greater chance to cut off left arm if previously damaged.

	if (dismember_ok && flrand(0.0f, (float)self->health) < damage * 0.75f)
	{
		int throw_nodes = 0;

		if (AssassinCanThrowNode(self, mesh_part, &throw_nodes))
		{
			vec3_t right;
			AngleVectors(self->s.angles, NULL, right, NULL);

			vec3_t gore_spot = { 0.0f, 0.0f, self->maxs[2] * 0.3f };
			const float side = (is_left_arm ? -1.0f : 1.0f);
			VectorMA(gore_spot, 10.0f * side, right, gore_spot);

			const int knife_flag = (is_left_arm ? BIT_LKNIFE : BIT_RKNIFE); //mxd
			AssassinDropWeapon(self, knife_flag);
			ThrowBodyPart(self, &gore_spot, throw_nodes, damage, FRAME_prtfly);
		}
	}
	else
	{
		// Switch arm node to damaged skin.
		self->s.fmnodeinfo[mesh_part].flags |= FMNI_USE_SKIN;
		self->s.fmnodeinfo[mesh_part].skin = self->s.skinnum + 1;
	}
}

static void AssassinThrowUpperLeg(edict_t* self, const float damage, const int mesh_part) //mxd. Split from AssassinDismember() to simplify logic.
{
	if (self->health > 0)
	{
		if (self->s.fmnodeinfo[mesh_part].flags & FMNI_USE_SKIN)
			return;

		self->s.fmnodeinfo[mesh_part].flags |= FMNI_USE_SKIN;
		self->s.fmnodeinfo[mesh_part].skin = self->s.skinnum + 1;
	}
	else
	{
		if (self->s.fmnodeinfo[mesh_part].flags & FMNI_NO_DRAW)
			return;

		const qboolean is_left_leg = (mesh_part == MESH__LTHIGH); //mxd

		int throw_nodes = 0;
		const int calf_part = (is_left_leg ? MESH__LCALF : MESH__RCALF); //mxd
		AssassinCanThrowNode(self, calf_part, &throw_nodes);

		if (AssassinCanThrowNode(self, mesh_part, &throw_nodes))
		{
			vec3_t right;
			AngleVectors(self->s.angles, NULL, right, NULL);

			vec3_t gore_spot = { 0.0f, 0.0f, self->maxs[2] * 0.3f };
			const float side = (is_left_leg ? -1.0f : 1.0f);
			VectorMA(gore_spot, 10.0f * side, right, gore_spot);

			ThrowBodyPart(self, &gore_spot, throw_nodes, damage, 0);
		}
	}
}

static void AssassinThrowLowerLeg(edict_t* self, const float damage, const int mesh_part) //mxd. Split from AssassinDismember() to simplify logic.
{
	if (self->health > 0)
	{
		if (self->s.fmnodeinfo[mesh_part].flags & FMNI_USE_SKIN)
			return;

		self->s.fmnodeinfo[mesh_part].flags |= FMNI_USE_SKIN;
		self->s.fmnodeinfo[mesh_part].skin = self->s.skinnum + 1;
	}
	else
	{
		if (self->s.fmnodeinfo[mesh_part].flags & FMNI_NO_DRAW)
			return;

		int throw_nodes = 0;

		if (AssassinCanThrowNode(self, mesh_part, &throw_nodes))
		{
			vec3_t right;
			AngleVectors(self->s.angles, NULL, right, NULL);

			vec3_t gore_spot = { 0.0f, 0.0f, self->maxs[2] * 0.3f };
			const float side = (mesh_part == MESH__LCALF ? -1.0f : 1.0f);
			VectorMA(gore_spot, 10.0f * side, right, gore_spot);

			ThrowBodyPart(self, &gore_spot, throw_nodes, damage, 0);
		}
	}
}

void AssassinDismember(edict_t* self, const int damage, HitLocation_t hl) //mxd. Named 'assassin_dismember' in original logic.
{
	qboolean dismember_ok = false;

	if (hl & hl_MeleeHit)
	{
		dismember_ok = true;
		hl &= ~hl_MeleeHit;
	}

	if (hl <= hl_NoneSpecific || hl >= hl_Max) //mxd. 'hl > hl_Max' in original logic.
		return;

	if (self->health > 0)
	{
		if (self->curAnimID == ANIM_DAGGERL || (self->curAnimID == ANIM_DAGGERB && irand(0, 2) < 1))
		{
			// Hit chest during melee, may have hit arms.
			if (hl == hl_TorsoFront && irand(0, 10) < 4)
				hl = ((irand(0, 10) < 7) ? hl_ArmLowerRight : hl_ArmLowerLeft);
		}

		if (self->curAnimID == ANIM_DAGGERR || self->curAnimID == ANIM_DAGGERC || (self->curAnimID == ANIM_DAGGERB && irand(0, 2) < 1))
		{
			// Hit chest during melee, may have hit arms.
			if (hl == hl_TorsoFront && irand(0, 10) < 4)
				hl = ((irand(0, 10) < 7) ? hl_ArmLowerRight : hl_ArmLowerLeft);
		}

		const qboolean no_left_arm = (self->s.fmnodeinfo[MESH__LUPARM].flags & FMNI_NO_DRAW); //mxd
		const qboolean no_right_arm = (self->s.fmnodeinfo[MESH__RUPARM].flags & FMNI_NO_DRAW); //mxd

		if ((hl == hl_ArmUpperLeft && no_left_arm) || (hl == hl_ArmUpperRight && no_right_arm) ||
			((hl == hl_TorsoFront || hl == hl_TorsoBack) && no_left_arm && no_right_arm && irand(0, 10) < 4))
			hl = hl_Head; // Decapitate.
	}
	else
	{
		hl = AssassinConvertDeadHitLocation(hl);
	}

	switch (hl)
	{
		case hl_Head:
			if (AssassinThrowHead(self, (float)damage, dismember_ok))
				return;
			break;

		case hl_TorsoFront: // Split in half?
			if (AssassinThrowTorso(self, (float)damage, MESH__TORSOFT, dismember_ok))
				return;
			break;

		case hl_TorsoBack: // Split in half?
			if (AssassinThrowTorso(self, (float)damage, MESH__TORSOBK, dismember_ok))
				return;
			break;

		case hl_ArmUpperLeft:
			AssassinThrowUpperArm(self, (float)damage, MESH__LUPARM, dismember_ok);
			break;

		case hl_ArmLowerLeft: // Left arm.
			AssassinThrowLowerArm(self, (float)damage, MESH__L4ARM, dismember_ok);
			break;

		case hl_ArmUpperRight:
			AssassinThrowUpperArm(self, (float)damage, MESH__RUPARM, dismember_ok);
			break;

		case hl_ArmLowerRight: // Right arm.
			AssassinThrowLowerArm(self, (float)damage, MESH__R4ARM, dismember_ok);
			break;

		case hl_LegUpperLeft:
			AssassinThrowUpperLeg(self, (float)damage, MESH__LTHIGH);
			break;

		case hl_LegLowerLeft: // Left leg.
			AssassinThrowLowerLeg(self, (float)damage, MESH__LCALF);
			break;

		case hl_LegUpperRight:
			AssassinThrowUpperLeg(self, (float)damage, MESH__RTHIGH);
			break;

		case hl_LegLowerRight: // Right leg.
			AssassinThrowLowerLeg(self, (float)damage, MESH__RCALF);
			break;

		default:
			if (flrand(0.0f, (float)self->health) < (float)damage * 0.25f)
				AssassinDropWeapon(self, damage); //TODO: 2-nd arg is expected to be knife flags, not damage. Pass BIT_LKNIFE | BIT_RKNIFE instead?..
			break;
	}

	// Can't fight without arms...
	if ((self->s.fmnodeinfo[MESH__L4ARM].flags & FMNI_NO_DRAW) && (self->s.fmnodeinfo[MESH__R4ARM].flags & FMNI_NO_DRAW))
		self->monsterinfo.aiflags |= (AI_COWARD | AI_NO_MELEE | AI_NO_MISSILE);
}

#pragma endregion

#pragma region ========================== Message handler utility functions ==========================

static void AssassinSetRandomAttackAnim(edict_t* self) //mxd. Named 'assassin_random_attack' in original logic.
{
	const int chance = irand(0, 3);
	const qboolean have_left_arm = !(self->s.fmnodeinfo[MESH__L4ARM].flags & FMNI_NO_DRAW); //mxd
	const qboolean have_right_arm = !(self->s.fmnodeinfo[MESH__R4ARM].flags & FMNI_NO_DRAW); //mxd
	int anim_id;

	if ((chance < 1 && have_left_arm) || (!have_right_arm && have_left_arm))
	{
		anim_id = ANIM_DAGGERL;
	}
	else if ((chance < 2 && have_right_arm) || (have_right_arm && !have_left_arm))
	{
		anim_id = (irand(0, 1) == 1 ? ANIM_DAGGERR : ANIM_NEWDAGGER);
	}
	else if (!(self->s.fmnodeinfo[MESH__R4ARM].flags & FMNI_NO_DRAW) && !(self->s.fmnodeinfo[MESH__L4ARM].flags & FMNI_NO_DRAW))
	{
		anim_id = (irand(0, 1) == 1 ? ANIM_DAGGERB : ANIM_NEWDAGGERB);
	}
	else
	{
		self->monsterinfo.aiflags |= AI_COWARD;
		anim_id = ANIM_RUN;
	}

	SetAnim(self, anim_id);
}

static void AssassinChooseJumpAmbush(edict_t* self) //mxd. Named 'assassinChooseJumpAmbush' in original logic.
{
	if (self->enemy == NULL)
	{
		const int anim_id = ((irand(0, 10) < 4) ? ANIM_JUMP : ANIM_FRONTFLIP); //mxd
		SetAnim(self, anim_id);

		return;
	}

	vec3_t forward;
	AngleVectors(self->s.angles, forward, NULL, NULL);

	vec3_t enemy_dir;
	VectorSubtract(self->enemy->s.origin, self->s.origin, enemy_dir);
	VectorNormalize(enemy_dir);

	if (DotProduct(forward, enemy_dir) < 0.0f) // Behind.
	{
		SetAnim(self, ANIM_BACKFLIP);
	}
	else
	{
		const int anim_id = ((irand(0, 3) == 0) ? ANIM_JUMP : ANIM_FRONTFLIP); //mxd
		SetAnim(self, anim_id);
	}
}

static qboolean AssassinChooseSideJumpAmbush(edict_t* self) //mxd. Named 'assassinChooseSideJumpAmbush' in original logic.
{
	//OR: turn and jump?
	if (self->enemy == NULL)
		return false;

	vec3_t right;
	AngleVectors(self->s.angles, NULL, right, NULL);

	vec3_t enemy_dir;
	VectorSubtract(self->enemy->s.origin, self->s.origin, enemy_dir);
	VectorNormalize(enemy_dir);

	const float side = ((DotProduct(right, enemy_dir) > 0.0f) ? 1.0f : -1.0f); //mxd
	VectorScale(right, 300.0f * side, self->movedir);

	self->movedir[2] = 200.0f;
	SetAnim(self, ANIM_FJUMP);

	return true;
}

#pragma endregion

#pragma region ========================== Message handlers ==========================

static void AssassinStandMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'assassin_stand' in original logic.
{
	if (self->ai_mood == AI_MOOD_DELAY)
	{
		SetAnim(self, ANIM_DELAY);
	}
	else
	{
		if ((self->s.renderfx & RF_ALPHA_TEXTURE) && self->pre_think != AssassinDeCloakFadePreThink)
			AssassinInitDeCloak(self);

		SetAnim(self, ANIM_STAND);
	}
}

static void AssassinWalkMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'assassin_walk' in original logic.
{
	if (self->spawnflags & MSF_FIXED)
	{
		SetAnim(self, ANIM_DELAY);
	}
	else
	{
		const int anim_id = ((self->curAnimID == ANIM_WALK_LOOP) ? ANIM_WALK_LOOP : ANIM_WALK); //mxd
		SetAnim(self, anim_id);
	}
}

static void AssassinRunMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'assassin_run' in original logic.
{
	if (self->enemy == NULL && (self->spawnflags & MSF_WANDER))
	{
		SetAnim(self, ANIM_RUN);
		return;
	}

	if (self->spawnflags & MSF_ASS_STARTSHADOW) // De-cloak.
	{
		//FIXME: should I wait until infront of enemy and visible to him?
		self->spawnflags &= ~MSF_ASS_STARTSHADOW;
		assassin_init_cloak(self);
	}

	if (!(self->spawnflags & MSF_FIXED))
	{
		if (self->spawnflags & MSF_ASS_JUMPAMBUSH) // Jump out.
		{
			self->spawnflags &= ~MSF_ASS_JUMPAMBUSH;
			AssassinChooseJumpAmbush(self);

			return;
		}

		if (self->spawnflags & MSF_ASS_SIDEJUMPAMBUSH) // Side-jump out.
		{
			self->spawnflags &= ~MSF_ASS_SIDEJUMPAMBUSH;
			if (AssassinChooseSideJumpAmbush(self))
				return;
		}
	}

	if (self->curAnimID >= ANIM_CROUCH_IDLE && self->curAnimID < ANIM_CROUCH_END)
	{
		SetAnim(self, ANIM_CROUCH_END);
		return;
	}

	if (M_ValidTarget(self, self->enemy))
	{
		if (irand(0, 7) == 0)
		{
			if (AssassinCheckTeleport(self, ASS_TP_OFF))
				return;

			if (irand(0, 3) == 0 && !(self->s.renderfx & RF_ALPHA_TEXTURE) && !(self->spawnflags & MSF_ASS_NOSHADOW))
			{
				SetAnim(self, ANIM_CLOAK);
				return;
			}
		}

		const int anim_id = ((self->spawnflags & MSF_FIXED) ? ANIM_DELAY : ANIM_RUN); //mxd
		SetAnim(self, anim_id);
	}
	else
	{
		G_PostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
	}
}

static void AssassinMeleeMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'assassin_melee' in original logic.
{
	if (M_ValidTarget(self, self->enemy))
	{
		if (irand(0, 7) == 0 && AssassinCheckTeleport(self, ASS_TP_OFF)) // 12.5% chance to try to get away.
			return;

		AssassinSetRandomAttackAnim(self);
	}
	else
	{
		G_PostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
	}
}

static void AssassinMissileMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'assassin_missile' in original logic.
{
	if (M_ValidTarget(self, self->enemy))
	{
		if (irand(0, 10) == 0) // 10% chance try special action. //TODO: 0-10 is actually ~9%. Change to 0-9?
		{
			if (irand(0, 2) == 0) // 25% chance to teleport. //TODO: 0-2 is actually 33.3%. Change to 0-3?
			{
				if (AssassinCheckTeleport(self, ASS_TP_OFF))
					return;
			}
			else if (!(self->s.renderfx & RF_ALPHA_TEXTURE) && !(self->spawnflags & MSF_ASS_NOSHADOW)) // 75% chance to cloak.
			{
				SetAnim(self, ANIM_CLOAK);
				return;
			}
		}

		AssassinSetRandomAttackAnim(self);
	}
	else
	{
		G_PostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
	}
}

static void AssassinPainMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'assassin_pain' in original logic.
{
	if (self->curAnimID == ANIM_TELEPORT)
		return;

	edict_t* inflictor;
	edict_t* attacker;
	qboolean force_pain;
	int damage;
	int temp;
	G_ParseMsgParms(msg, "eeiii", &inflictor, &attacker, &force_pain, &damage, &temp);

	if (inflictor == attacker || Q_stricmp(inflictor->classname, "Spell_RedRain") == 0 || Q_stricmp(inflictor->classname, "Spell_Hellbolt") == 0) //mxd. stricmp -> Q_stricmp
	{
		// Melee hit or constant effect, don't stick around!
		if (!(self->spawnflags & MSF_ASS_NOTELEPORT) && !(self->spawnflags & MSF_FIXED) && self->groundentity != NULL && AssassinChooseTeleportDestination(self, ASS_TP_ANY, true, false))
			return;
	}

	if (!force_pain && irand(0, self->health) > damage) //mxd. flrand() in original logic.
		return;

	self->monsterinfo.aiflags &= ~AI_OVERRIDE_GUIDE;

	if (self->maxs[2] == 0.0f)
		assassin_unset_crouched(self);

	//mxd. Inlined assassin_random_pain_sound().
	gi.sound(self, CHAN_VOICE, sounds[irand(SND_PAIN1, SND_PAIN2)], 1.0f, ATTN_NORM, 0.0f);

	if (force_pain || self->pain_debounce_time < level.time)
	{
		self->pain_debounce_time = level.time + skill->value + 2.0f;
		SetAnim(self, ANIM_PAIN2);

		if (irand(0, 10) > SKILL)
		{
			self->monsterinfo.misc_debounce_time = level.time + 3.0f; // 3 seconds before can re-cloak.
			AssassinInitDeCloak(self);
		}
	}
}

static void AssassinDeathMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'assassin_death' in original logic.
{
	if (self->monsterinfo.aiflags & AI_DONT_THINK)
	{
		const int anim_id = ((irand(0, 10) < 5) ? ANIM_DEATHB : ANIM_DEATHA); //mxd
		SetAnim(self, anim_id);

		return;
	}

	self->msgHandler = DeadMsgHandler;

	if (self->dead_state == DEAD_DEAD) // Dead but still being hit.
		return;

	self->dead_state = DEAD_DEAD;

	self->isBlocked = NULL;
	self->bounced = NULL;

	AssassinDropWeapon(self, BIT_LKNIFE | BIT_RKNIFE);

	if (self->health <= -80) // Gib death?
	{
		gi.sound(self, CHAN_BODY, sounds[SND_GIB], 1.0f, ATTN_NORM, 0.0f);

		if (irand(0, 10) < 5)
		{
			self->s.fmnodeinfo[MESH__TORSOFT].flags |= FMNI_NO_DRAW;
			self->s.fmnodeinfo[MESH__TORSOBK].flags |= FMNI_NO_DRAW;
			self->s.fmnodeinfo[MESH__HEAD].flags |= FMNI_NO_DRAW;
			self->s.fmnodeinfo[MESH__R4ARM].flags |= FMNI_NO_DRAW;
			self->s.fmnodeinfo[MESH__L4ARM].flags |= FMNI_NO_DRAW;
			self->s.fmnodeinfo[MESH__KNIFES].flags |= FMNI_NO_DRAW;
			self->s.fmnodeinfo[MESH__LUPARM].flags |= FMNI_NO_DRAW;
			self->s.fmnodeinfo[MESH__RUPARM].flags |= FMNI_NO_DRAW;
			self->s.fmnodeinfo[MESH__LKNIFE].flags |= FMNI_NO_DRAW;
			self->s.fmnodeinfo[MESH__RKNIFE].flags |= FMNI_NO_DRAW;

			SprayDebris(self, self->s.origin, 12, 100.0f);
		}
		else
		{
			self->think = BecomeDebris;
			self->nextthink = level.time + FRAMETIME;

			return;
		}
	}
	else
	{
		gi.sound(self, CHAN_VOICE, sounds[SND_DIE1], 1.0f, ATTN_NORM, 0.0f); //mxd. Inlined assassin_random_death_sound(). Not very random after all...
		self->msgHandler = DyingMsgHandler;
	}

	const int anim_id = ((self->health <= -80 && irand(0, 10) < 5) ? ANIM_DEATHA : ANIM_DEATHB); //mxd
	SetAnim(self, anim_id);

	self->pre_think = NULL;
	self->next_pre_think = -1.0f;

	if ((self->s.renderfx & RF_ALPHA_TEXTURE) && self->pre_think != AssassinDeCloakFadePreThink)
		AssassinInitDeCloak(self);
}

static void AssassinDeathPainMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'assassin_dead_pain' in original logic.
{
	if (msg == NULL || (self->svflags & SVF_PARTS_GIBBED))
		return;

	//mxd. Inlined assassin_dismember_msg().
	//FIXME: make part fly dir the vector from hit loc to sever loc. Remember - turn on caps!
	int damage;
	HitLocation_t hl;
	G_ParseMsgParms(msg, "ii", &damage, &hl);

	AssassinDismember(self, damage, hl);
}

static void AssassinJumpMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'assassin_jump' in original logic.
{
	SetAnim(self, ANIM_FORCED_JUMP);
	self->monsterinfo.aiflags |= AI_OVERRIDE_GUIDE;
}

static void AssassinEvadeMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'assassin_evade' in original logic.
{
	static const EvadeChance_t evade_chances[] = //mxd. Use struct.
	{
		{.duck_chance = 20, .dodgeleft_chance = 10, .dodgeright_chance = 10, .jump_chance = 10, .backflip_chance = 10, .frontflip_chance = 10 }, // hl_NoneSpecific
		{.duck_chance = 95, .dodgeleft_chance = 50, .dodgeright_chance = 50, .jump_chance = 0,  .backflip_chance = 20, .frontflip_chance = 20 }, // hl_Head
		{.duck_chance = 85, .dodgeleft_chance = 40, .dodgeright_chance = 40, .jump_chance = 0,  .backflip_chance = 60, .frontflip_chance = 0  }, // hl_TorsoFront
		{.duck_chance = 80, .dodgeleft_chance = 40, .dodgeright_chance = 40, .jump_chance = 0,  .backflip_chance = 0,  .frontflip_chance = 60 }, // hl_TorsoBack
		{.duck_chance = 75, .dodgeleft_chance = 0,  .dodgeright_chance = 90, .jump_chance = 0,  .backflip_chance = 20, .frontflip_chance = 20 }, // hl_ArmUpperLeft
		{.duck_chance = 75, .dodgeleft_chance = 0,  .dodgeright_chance = 80, .jump_chance = 30, .backflip_chance = 20, .frontflip_chance = 20 }, // hl_ArmLowerLeft
		{.duck_chance = 60, .dodgeleft_chance = 90, .dodgeright_chance = 0,  .jump_chance = 0,  .backflip_chance = 20, .frontflip_chance = 20 }, // hl_ArmUpperRight
		{.duck_chance = 20, .dodgeleft_chance = 80, .dodgeright_chance = 0,  .jump_chance = 30, .backflip_chance = 20, .frontflip_chance = 20 }, // hl_ArmLowerRight
		{.duck_chance = 0,  .dodgeleft_chance = 0,  .dodgeright_chance = 60, .jump_chance = 50, .backflip_chance = 30, .frontflip_chance = 30 }, // hl_LegUpperLeft
		{.duck_chance = 0,  .dodgeleft_chance = 0,  .dodgeright_chance = 30, .jump_chance = 80, .backflip_chance = 40, .frontflip_chance = 40 }, // hl_LegLowerLeft
		{.duck_chance = 0,  .dodgeleft_chance = 60, .dodgeright_chance = 0,  .jump_chance = 50, .backflip_chance = 30, .frontflip_chance = 30 }, // hl_LegUpperRight
		{.duck_chance = 0,  .dodgeleft_chance = 30, .dodgeright_chance = 0,  .jump_chance = 80, .backflip_chance = 40, .frontflip_chance = 40 }, // hl_LegLowerRight
	};

	if (self->groundentity == NULL)
		return;

	edict_t* projectile;
	HitLocation_t hl;
	float eta;
	G_ParseMsgParms(msg, "eif", &projectile, &hl, &eta);

	self->evade_debounce_time = level.time + min(eta, 2.0f);

	if (SKILL > SKILL_EASY || (self->spawnflags & MSF_ASS_TELEPORTDODGE))
	{
		// Pussies were complaining about assassins teleporting away from certain death, so don't do that unless in hard.
		if (Q_stricmp(projectile->classname, "Spell_PhoenixArrow") == 0 ||
			Q_stricmp(projectile->classname, "Spell_FireWall") == 0 ||
			Q_stricmp(projectile->classname, "Spell_SphereOfAnnihilation") == 0 ||
			Q_stricmp(projectile->classname, "Spell_Maceball") == 0) //mxd. stricmp -> Q_stricmp
		{
			if (AssassinChooseTeleportDestination(self, ASS_TP_OFF, true, true))
				return;
		}
	}

	if (irand(0, 100) < SKILL * 10 && self->pre_think != AssassinCloakFadePreThink)
		assassin_init_cloak(self);

	if (SKILL > SKILL_EASY || (self->spawnflags & MSF_ASS_TELEPORTDODGE))
	{
		// Pussies were complaining about assassins teleporting away from certain death, so don't do that unless in hard.
		if (irand(0, 10) > 8 && !(self->spawnflags & MSF_ASS_NOTELEPORT) && AssassinChooseTeleportDestination(self, ASS_TP_DEF, false, false))
			return;
	}

	//mxd. Get evade info.
	if (hl < hl_NoneSpecific || hl > hl_LegLowerRight)
		hl = hl_NoneSpecific;

	const EvadeChance_t* ec = &evade_chances[hl];

	if (irand(0, 100) < ec->frontflip_chance)
	{
		SetAnim(self, ANIM_EVFRONTFLIP); //mxd. Inline assassinFrontFlip().
		return;
	}

	if (irand(0, 100) < ec->backflip_chance)
	{
		if (self->curAnimID == ANIM_RUN && irand(0, 3) > 0) // Running, do the front flip.
			SetAnim(self, ANIM_EVFRONTFLIP); //mxd. Inline assassinFrontFlip().
		else if (irand(0, 1) == 1)
			SetAnim(self, ANIM_BACKSPRING); //mxd. Inline assassinBackSprings().
		else
			SetAnim(self, ANIM_EVBACKFLIP); //mxd. Inline assassinBackFlip().

		return;
	}

	if (irand(0, 100) < ec->duck_chance)
	{
		self->evade_debounce_time = level.time + eta + 2.0f - skill->value;
		SetAnim(self, ANIM_CROUCH); //mxd. Inline assassinCrouch().

		return;
	}

	if (irand(0, 100) < ec->dodgeleft_chance)
	{
		SetAnim(self, ANIM_DODGE_LEFT); //mxd. Inline assassinDodgeLeft().
		return;
	}

	if (irand(0, 100) < ec->dodgeright_chance)
	{
		SetAnim(self, ANIM_DODGE_RIGHT); //mxd. Inline assassinDodgeRight().
		return;
	}

	if (irand(0, 100) < ec->jump_chance)
	{
		if (self->curAnimID == ANIM_RUN && irand(0, 4) > 0) // Running, do the front flip.
			SetAnim(self, ANIM_EVFRONTFLIP); //mxd. Inline assassinFrontFlip().
		else
			SetAnim(self, ANIM_EVJUMP); //mxd. Inline assassinJump().

		return;
	}

	if (SKILL > SKILL_EASY || (self->spawnflags & MSF_ASS_TELEPORTDODGE))
	{
		// Pussies were complaining about assassins teleporting away from certain death, so don't do that unless in hard.
		if (!(self->spawnflags & MSF_ASS_NOTELEPORT) && AssassinChooseTeleportDestination(self, ASS_TP_DEF, false, false))
			return;
	}

	self->evade_debounce_time = 0.0f;
}

static void AssassinCheckMoodMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'assassin_check_mood' in original logic.
{
	G_ParseMsgParms(msg, "i", &self->ai_mood);
	assassin_pause(self);
}

static void AssassinCinematicActionMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'assassin_c_anims' in original logic.
{
	int curr_anim;

	ReadCinematicMessage(self, msg);
	self->monsterinfo.c_anim_flag = 0;

	switch (msg->ID)
	{
		case MSG_C_ATTACK1:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_ATTACK1;
			break;

		case MSG_C_ATTACK2:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_ATTACK2;
			break;

		case MSG_C_IDLE1:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_IDLE1;
			break;

		case MSG_C_RUN1:
			self->monsterinfo.c_anim_flag |= C_ANIM_MOVE;
			curr_anim = ANIM_C_RUN1;
			break;

		default:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_IDLE1;
			break;
	}

	SetAnim(self, curr_anim);
}

#pragma endregion

#pragma region ========================== Edict callbacks ==========================

// Assigned to 'isBlocked' and 'bounce' callbacks.
void AssassinBlocked(edict_t* self, trace_t* trace) //mxd. Named 'assassin_Touch' in original logic.
{
	if (self->health <= 0 || trace == NULL)
		return;

	edict_t* other = trace->ent;

	if ((self->groundentity != NULL && self->groundentity != other) || Vec3IsZero(self->velocity))
		return;

	const float strength = VectorLength(self->velocity);

	if (strength > 50 && AI_IsMovable(other) && (other->svflags & SVF_MONSTER || other->client != NULL))
	{
		if (self->s.origin[2] + self->mins[2] > other->s.origin[2] + other->maxs[2] * 0.8f)
		{
			if (other->client != NULL)
				P_KnockDownPlayer(&other->client->playerinfo);

			gi.sound(self, CHAN_BODY, sounds[SND_LANDF], 1.0f, ATTN_NORM, 0.0f);

			if (other->takedamage != DAMAGE_NO)
			{
				vec3_t dir;
				VectorCopy(self->velocity, dir);
				dir[2] = max(0.0f, dir[2]);
				VectorNormalize(dir);

				const int damage = min(5, (int)strength);
				T_Damage(other, self, self, dir, trace->endpos, dir, damage, damage * 4, 0, MOD_DIED);
			}

			SetAnim(self, ANIM_EVFRONTFLIP);
		}
	}
	//FIXME: else backflip off walls! Too late to implement.
}

void AssassinCheckDefenseThink(edict_t* self, float enemy_dist, qboolean enemy_vis, qboolean enemy_infront) //mxd. Named 'assassinCheckDefense' in original logic.
{
	if (!enemy_infront && enemy_vis && enemy_dist < self->melee_range)
	{
		AssassinCheckTeleport(self, ASS_TP_DEF);
	}
	else if (!enemy_vis && self->monsterinfo.last_successful_enemy_tracking_time + 6.0f - skill->value < level.time)
	{
		if (irand(0, 10) > 10 - (SKILL + 1) * 3) // Hard = 90%, medium is 40%, easy is 30%.
			AssassinCheckTeleport(self, ASS_TP_OFF);
	}
}

#pragma endregion

void AssassinStaticsInit(void)
{
	static ClassResourceInfo_t res_info; //mxd. Made local static.

	classStatics[CID_ASSASSIN].msgReceivers[MSG_STAND] = AssassinStandMsgHandler;
	classStatics[CID_ASSASSIN].msgReceivers[MSG_WALK] = AssassinWalkMsgHandler;
	classStatics[CID_ASSASSIN].msgReceivers[MSG_RUN] = AssassinRunMsgHandler;
	classStatics[CID_ASSASSIN].msgReceivers[MSG_MELEE] = AssassinMeleeMsgHandler;
	classStatics[CID_ASSASSIN].msgReceivers[MSG_MISSILE] = AssassinMissileMsgHandler;
	classStatics[CID_ASSASSIN].msgReceivers[MSG_PAIN] = AssassinPainMsgHandler;
	classStatics[CID_ASSASSIN].msgReceivers[MSG_DEATH] = AssassinDeathMsgHandler;
	classStatics[CID_ASSASSIN].msgReceivers[MSG_DISMEMBER] = DismemberMsgHandler;
	classStatics[CID_ASSASSIN].msgReceivers[MSG_JUMP] = AssassinJumpMsgHandler;
	classStatics[CID_ASSASSIN].msgReceivers[MSG_EVADE] = AssassinEvadeMsgHandler;
	classStatics[CID_ASSASSIN].msgReceivers[MSG_DEATH_PAIN] = AssassinDeathPainMsgHandler;
	classStatics[CID_ASSASSIN].msgReceivers[MSG_CHECK_MOOD] = AssassinCheckMoodMsgHandler;

	classStatics[CID_ASSASSIN].msgReceivers[MSG_C_IDLE1] = AssassinCinematicActionMsgHandler;
	classStatics[CID_ASSASSIN].msgReceivers[MSG_C_RUN1] = AssassinCinematicActionMsgHandler;
	classStatics[CID_ASSASSIN].msgReceivers[MSG_C_ATTACK1] = AssassinCinematicActionMsgHandler;
	classStatics[CID_ASSASSIN].msgReceivers[MSG_C_ATTACK2] = AssassinCinematicActionMsgHandler;

	res_info.numAnims = NUM_ANIMS;
	res_info.animations = animations;

	// Note that the name is different in the path.
	res_info.modelIndex = gi.modelindex("models/monsters/assassin/tris.fm");

	sounds[SND_PAIN1] = gi.soundindex("monsters/assassin/pain1.wav");
	sounds[SND_PAIN2] = gi.soundindex("monsters/assassin/pain2.wav");
	sounds[SND_DIE1] = gi.soundindex("monsters/assassin/death1.wav");
	sounds[SND_GIB] = gi.soundindex("monsters/assassin/gib.wav");
	sounds[SND_THROW1] = gi.soundindex("monsters/assassin/throw1.wav");
	sounds[SND_THROW2] = gi.soundindex("monsters/assassin/throw2.wav");
	sounds[SND_DAGHITF] = gi.soundindex("monsters/assassin/daghitf.wav");
	sounds[SND_DAGHITW] = gi.soundindex("monsters/assassin/daghitw.wav");
	sounds[SND_JUMP] = gi.soundindex("monsters/assassin/jump.wav");
	sounds[SND_FLIP] = gi.soundindex("monsters/assassin/flip.wav");
	sounds[SND_LAND] = gi.soundindex("monsters/assassin/land.wav");
	sounds[SND_LANDF] = gi.soundindex("monsters/assassin/landf.wav");
	sounds[SND_SLIDE] = gi.soundindex("monsters/assassin/slide.wav");
	sounds[SND_SLASH1] = gi.soundindex("monsters/assassin/slash1.wav");
	sounds[SND_SLASH2] = gi.soundindex("monsters/assassin/slash2.wav");
	sounds[SND_GROWL1] = gi.soundindex("monsters/assassin/growl1.wav");
	sounds[SND_GROWL2] = gi.soundindex("monsters/assassin/growl2.wav");
	sounds[SND_GROWL3] = gi.soundindex("monsters/assassin/growl3.wav");
	sounds[SND_CLOAK] = gi.soundindex("monsters/assassin/cloak.wav");
	sounds[SND_DECLOAK] = gi.soundindex("monsters/assassin/decloak.wav");

	res_info.numSounds = NUM_SOUNDS;
	res_info.sounds = sounds;

	classStatics[CID_ASSASSIN].resInfo = &res_info;
}

// QUAKED monster_assassin (1 .5 0) (-16 -16 -32) (16 16 48) AMBUSH ASLEEP WALKING JUMPAMBUSH NOSHADOW NOTELEPORT CINEMATIC FIXED WANDER MELEE_LEAD STALK COWARD TPORTAMBUSH STARTSHADOW SIDEJUMPAMBUSH TELEPORTDODGE
// The assassin.

// Spawnflags:
// AMBUSH			- Will not be woken up by other monsters or shots from player.
// ASLEEP			- Will not appear until triggered.
// WALKING			- Use WANDER instead.
// JUMPAMBUSH		- Will jump out front or back when triggered (depending on whether player is in front or behind him).
// NOSHADOW			- Can't turn into a shadow.
// NOTELEPORT		- Can't use smoke grenades to trick player and teleport.
// CINEMATIC		- Puts monster into cinematic mode for scripting.
// FIXED			- Will stand in place and attack from afar. Never moves.
// WANDER			- Monster will wander around aimlessly (but follows buoys).
// MELEE_LEAD		- Monster will try to cut you off when you're running and fighting him, works well if there are a few monsters in a group, half doing this, half not.
// STALK			- Monster will only approach and attack from behind. If you're facing the monster it will just stand there.
//					  Once the monster takes pain, it will stop this behaviour and attack normally.
// COWARD			- Monster starts off in flee mode - runs away from you when woken up.
// TPORTAMBUSH		- Will teleport into his origin when triggered (before triggered, is not anywhere at all, like "ASLEEP").
// STARTSHADOW		- Start as a shadow and de-cloak when wakes up.
// SIDEJUMPAMBUSH	- Will jump out to left or right (depending on which side of the assassin the player is).
// TELEPORTDODGE	- Can use teleporting to dodge attacks.

// Variables:
// homebuoy					- Monsters will head to this buoy if they don't have an enemy ("homebuoy" should be targetname of the buoy you want them to go to).
// wakeup_target			- Monsters will fire this target the first time it wakes up (only once).
// pain_target				- Monsters will fire this target the first time it gets hurt (only once).
// mintel					- Monster intelligence - this basically tells a monster how many buoys away an enemy has to be for it to give up (default 64).
// melee_range				- How close the player has to be for the monster to go into melee. If this is zero, the monster will never melee.
//							  If it is negative, the monster will try to keep this distance from the player.
//							  If the monster has a backup, he'll use it if too close, otherwise, a negative value here means the monster will just stop
//							  running at the player at this distance (default 48).
//							 Examples:
//								melee_range = 60 - monster will start swinging it player is closer than 60.
//								melee_range = 0 - monster will never do a melee attack.
//								melee_range = -100 - monster will never do a melee attack and will back away (if it has that ability) when player gets too close.
// missile_range			- Maximum distance the player can be from the monster to be allowed to use it's ranged attack (default 1024).
// min_missile_range		- Minimum distance the player can be from the monster to be allowed to use it's ranged attack (default 64).
// bypass_missile_chance	- Chance that a monster will NOT fire it's ranged attack, even when it has a clear shot. This, in effect, will make the monster
//							  come in more often than hang back and fire. A percentage (0 = always fire/never close in, 100 = never fire/always close in) - must be whole number (default 10).
// jump_chance				- Every time the monster has the opportunity to jump, what is the chance (out of 100) that he will... (100 = jump every time) - must be whole number (default 100).
// wakeup_distance			- How far (max) the player can be away from the monster before it wakes up. This means that if the monster can see the player,
//							  at what distance should the monster actually notice him and go for him (default 1024).
// NOTE: A value of zero will result in defaults, if you actually want zero as the value, use -1.
void SP_monster_assassin(edict_t* self)
{
	if (self->spawnflags & MSF_WALKING)
	{
		self->spawnflags |= MSF_WANDER;
		self->spawnflags &= ~MSF_WALKING;
	}

	if (self->spawnflags & (MSF_ASS_JUMPAMBUSH | MSF_ASS_SIDEJUMPAMBUSH | MSF_ASS_STARTSHADOW))
		self->spawnflags |= MSF_AMBUSH;

	if (self->spawnflags & MSF_ASS_TPORTAMBUSH)
		self->spawnflags |= MSF_ASLEEP;

	if (!M_WalkmonsterStart(self)) // Unsuccessful initialization.
		return;

	self->msgHandler = DefaultMsgHandler;
	self->monsterinfo.dismember = AssassinDismember;

	if (self->health == 0)
		self->health = ASSASSIN_HEALTH * (SKILL + 1) / 3;

	self->mass = ASSASSIN_MASS;
	self->yaw_speed = 20.0f;

	self->movetype = PHYSICSTYPE_STEP;
	VectorClear(self->knockbackvel);

	self->solid = SOLID_BBOX;

	VectorCopy(STDMinsForClass[self->classID], self->mins);
	VectorCopy(STDMaxsForClass[self->classID], self->maxs);
	self->viewheight = 40;

	self->isBlocked = AssassinBlocked;
	self->bounced = AssassinBlocked;

	self->s.modelindex = (byte)classStatics[CID_ASSASSIN].resInfo->modelIndex;
	self->materialtype = MAT_FLESH;

	//FIXME: otherenemy should be more than just *one* kind.
	self->monsterinfo.otherenemyname = "monster_rat";

	// Set up my mood function.
	MG_InitMoods(self);

	self->monsterinfo.aiflags |= AI_NIGHTVISION;

	if (irand(0, 2) == 0)
		self->ai_mood_flags |= AI_MOOD_FLAG_PREDICT;

	self->cant_attack_think = AssassinCheckDefenseThink;

	if (self->spawnflags & MSF_WANDER)
	{
		G_PostMessage(self, MSG_WALK, PRI_DIRECTIVE, NULL);
	}
	else if (self->spawnflags & MSF_ASS_CINEMATIC)
	{
		self->monsterinfo.c_mode = true;
		G_PostMessage(self, MSG_C_IDLE1, PRI_DIRECTIVE, "iiige", 0, 0, 0, NULL, NULL);
	}
	else
	{
		if (self->spawnflags & MSF_ASS_STARTSHADOW)
			assassin_init_cloak(self);

		self->pre_think = AssassinCloakPreThink;
		self->next_pre_think = level.time + FRAMETIME;

		G_PostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
	}

	self->svflags |= SVF_WAIT_NOTSOLID;
	self->s.fmnodeinfo[MESH__KNIFES].flags |= FMNI_NO_DRAW;

	VectorCopy(self->s.origin, self->assassin_spawn_pos); //TODO: pos1/assassin_spawn_pos is not a saveable field!
}