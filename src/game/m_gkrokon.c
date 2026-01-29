//
// m_gkrokon.c
//
// Copyright 1998 Raven Software
//

#include "m_gkrokon.h"
#include "m_gkrokon_shared.h"
#include "m_gkrokon_anim.h"
#include "m_gkrokon_moves.h"
#include "decals.h"
#include "g_debris.h" //mxd
#include "g_DefaultMessageHandler.h"
#include "g_monster.h"
#include "g_playstats.h"
#include "m_stats.h"
#include "mg_ai.h" //mxd
#include "mg_guide.h" //mxd
#include "Random.h"
#include "Utilities.h"
#include "Vector.h"
#include "g_local.h"

#define GKROKON_SPOO_SPEED	450.0f
#define GKROKON_SPOO_ARC	150.0f

#pragma region ========================== Gkrokon base info ==========================

static const animmove_t* animations[NUM_ANIMS] =
{
	&gkrokon_move_stand1,
	&gkrokon_move_stand2,
	&gkrokon_move_stand3,
	&gkrokon_move_stand4,
	&gkrokon_move_crouch1,
	&gkrokon_move_crouch2,
	&gkrokon_move_crouch3,
	&gkrokon_move_walk1,
	&gkrokon_move_run1,
	&gkrokon_move_run2,
	&gkrokon_move_jump1,
	&gkrokon_move_forced_jump,
	&gkrokon_move_melee_attack1,
	&gkrokon_move_melee_attack2,
	&gkrokon_move_missile_attack1,
	&gkrokon_move_eat1,
	&gkrokon_move_eat2,
	&gkrokon_move_eat3,
	&gkrokon_move_pain1,
	&gkrokon_move_death1,
	&gkrokon_move_hop1,
	&gkrokon_move_run_away,
	&gkrokon_move_missile_attack2,
	&gkrokon_move_delay,
};

static int sounds[NUM_SOUNDS];

#pragma endregion

#pragma region ========================== Spoo functions ==========================

void GkrokonSpooTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surface)
{
	if (surface != NULL && (surface->flags & SURF_SKY))
	{
		G_FreeEdict(self);
		return;
	}

	// Are we reflecting?
	if (EntReflecting(other, true, true) && self->reflect_debounce_time > 0)
	{
		Create_rand_relect_vect(self->velocity, self->velocity);
		Vec3ScaleAssign(GKROKON_SPOO_SPEED / 2.0f, self->velocity);
		GkrokonSpooReflect(self, other, self->velocity);

		return;
	}

	// Apply damage.
	const int damage = irand(GKROKON_DMG_SPOO_MIN, GKROKON_DMG_SPOO_MAX); //mxd. Don't use self->dmg; Calculate here, so this val is used by T_DamageRadius() even when other->takedamage is DAMAGE_NO.

	if (other->takedamage != DAMAGE_NO)
		T_Damage(other, self, self->owner, self->movedir, other->s.origin, plane->normal, damage, damage, 0, MOD_DIED);
	else
		VectorMA(self->s.origin, -16.0f, self->movedir, self->s.origin);

	T_DamageRadius(self, self->owner, self, 5.0f, (float)damage, (float)damage / 4.0f, DAMAGE_NO_KNOCKBACK | DAMAGE_ATTACKER_IMMUNE, MOD_DIED);

	gi.RemoveEffects(&self->s, FX_REMOVE_EFFECTS);

	// Create decal?
	vec3_t plane_dir;
	VectorCopy(vec3_up, plane_dir); //mxd. Initialize, so we pass a valid vector when creating FX_SPOO_SPLAT even when IsDecalApplicable() returns false.

	if (IsDecalApplicable(other, self->s.origin, surface, plane, plane_dir))
		gi.CreateEffect(NULL, FX_SCORCHMARK, 0, self->s.origin, "d", plane_dir);

	// Splatter some spoo over the surface.
	gi.CreateEffect(NULL, FX_SPOO_SPLAT, 0, self->s.origin, "d", plane_dir);
	gi.sound(self, CHAN_WEAPON, gi.soundindex("monsters/rat/gib.wav"), 1.0f, ATTN_NORM, 0.0f);

	G_SetToFree(self);
}

void GkrokonSpooIsBlocked(edict_t* self, trace_t* trace) //mxd. Named 'GkrokonSpooTouch2' in original logic.
{
	GkrokonSpooTouch(self, trace->ent, &trace->plane, trace->surface);
}

static void SpooInit(edict_t* spoo) //mxd. Named 'create_gkrokon_spoo' in original logic. //TODO: rename to GkrokonSpooInit.
{
	spoo->movetype = PHYSICSTYPE_FLY;
	spoo->solid = SOLID_BBOX;
	spoo->classname = "Gkrokon_Spoo";
	spoo->clipmask = MASK_SHOT;
	spoo->s.scale = 0.5f;
	spoo->s.effects = EF_CAMERA_NO_CLIP;

	spoo->touch = GkrokonSpooTouch;
	spoo->isBlocked = GkrokonSpooIsBlocked;

	VectorSet(spoo->mins, -1.0f, -1.0f, -1.0f);
	VectorSet(spoo->maxs, 1.0f, 1.0f, 1.0f);
}

edict_t* GkrokonSpooReflect(edict_t* self, edict_t* other, const vec3_t vel)
{
	edict_t* spoo = G_Spawn();

	SpooInit(spoo);

	spoo->enemy = self->owner;
	spoo->owner = other;
	spoo->reflect_debounce_time = self->reflect_debounce_time - 1;
	spoo->reflected_time = self->reflected_time;

	VectorCopy(self->s.origin, spoo->s.origin);
	VectorCopy(vel, spoo->velocity);
	VectorNormalize2(vel, spoo->movedir);
	AnglesFromDir(spoo->movedir, spoo->s.angles);

	gi.linkentity(spoo);
	gi.CreateEffect(&spoo->s, FX_SPOO, CEF_OWNERS_ORIGIN, spoo->s.origin, "");

	G_SetToFree(self);

	return spoo;
}

void gkrokon_spoo_attack(edict_t* self) //mxd. Named 'GkrokonSpoo' in original logic.
{
	if (self->enemy == NULL)
		return;

	vec3_t forward;
	AngleVectors(self->s.angles, forward, NULL, NULL);

	vec3_t diff;
	VectorSubtract(self->enemy->s.origin, self->s.origin, diff);
	const float dist = VectorNormalize(diff);

	// Skip when enemy not within 90-deg forward cone.
	if (DotProduct(diff, forward) < 0.5f) //mxd. Was done in the middle of spoo init logic in original version, before gi.linkentity() call.
		return;

	// Spawn the spoo globbit.
	edict_t* spoo = G_Spawn();

	SpooInit(spoo);

	spoo->reflect_debounce_time = MAX_REFLECT;
	spoo->enemy = self->enemy;
	spoo->owner = self;

	VectorCopy(self->s.origin, spoo->s.origin);
	VectorMA(spoo->s.origin, -16.0f, forward, spoo->s.origin);
	spoo->s.origin[2] += 12.0f;

	VectorCopy(self->movedir, spoo->movedir);
	vectoangles(forward, spoo->s.angles);

	VectorScale(diff, GKROKON_SPOO_SPEED, spoo->velocity);
	spoo->velocity[2] += GKROKON_SPOO_ARC + (dist - 256.0f);
	vectoangles(spoo->velocity, spoo->s.angles);

	gi.linkentity(spoo);
	gi.sound(spoo, CHAN_WEAPON, sounds[SND_SPOO], 1.0f, ATTN_NORM, 0.0f);
	gi.CreateEffect(&spoo->s, FX_SPOO, CEF_OWNERS_ORIGIN, spoo->s.origin, "");

	self->monsterinfo.misc_debounce_time = level.time + flrand(0.5f, 3.0f);
}

#pragma endregion

#pragma region ========================== Message handlers ==========================

static void GkrokonFallbackMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'beetle_skitter' in original logic.
{
	if (self->spawnflags & MSF_FIXED)
	{
		SetAnim(self, ANIM_DELAY);
	}
	else if (irand(0, 10) < 2 && self->monsterinfo.attack_finished < level.time) //mxd. Inline SkitterAway().
	{
		self->monsterinfo.attack_finished = level.time + 5.0f;
		SetAnim(self, ANIM_SNEEZE);
	}
	else
	{
		SetAnim(self, ANIM_RUNAWAY);
	}
}

static void GkrokonCheckMoodMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'beetle_check_mood' in original logic.
{
	G_ParseMsgParms(msg, "i", &self->ai_mood);
	gkrokon_pause(self);
}

static void GkrokonRunMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'beetle_run' in original logic.
{
	// Make sure we're not getting up from laying down without the proper animations.
	if (self->ai_mood == AI_MOOD_FLEE)
	{
		SetAnim(self, irand(ANIM_RUN1, ANIM_RUN2));
		return;
	}

	if (self->curAnimID == ANIM_STAND1)
	{
		SetAnim(self, ANIM_STAND2);
		return;
	}

	if (M_ValidTarget(self, self->enemy))
	{
		if (M_DistanceToTarget(self, self->enemy) < 300.0f)
		{
			const int chance = irand(0, 100);

			if (chance < 5)
				SetAnim(self, ANIM_STAND3);
			else if (chance < 20)
				SetAnim(self, ANIM_RUN2);
			else
				SetAnim(self, ANIM_RUN1);
		}
		else
		{
			SetAnim(self, ANIM_RUN1);
		}

		return;
	}

	// If our enemy is dead, we need to stand.
	G_PostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
}

static void GkrokonStandMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'beetle_stand' in original logic.
{
	SetAnim(self, ((self->spawnflags & MSF_EXTRA1) ? ANIM_STAND1 : ANIM_STAND3));
}

static void GkrokonMissileMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'beetle_missile' in original logic.
{
	if (self->enemy == NULL)
	{
		G_PostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
		return;
	}

	if (self->monsterinfo.misc_debounce_time < level.time)
	{
		const int chance = irand(0, 100);
		const float dist = M_DistanceToTarget(self, self->enemy);

		if (self->spawnflags & MSF_FIXED)
		{
			if (dist < self->missile_range)
				SetAnim(self, ANIM_MISSILE1);
			else if (self->curAnimID == ANIM_CROUCH1 && !irand(0, 2))
				SetAnim(self, ANIM_CROUCH2); // Go into stand.
			else if (self->curAnimID == ANIM_STAND3 && !irand(0, 10))
				SetAnim(self, ANIM_CROUCH3); // Go into a crouch.
			else
				SetAnim(self, ANIM_DELAY); // Stand around.

			return;
		}

		if (dist < 64.0f)
		{
			if (chance < 10)
			{
				SetAnim(self, ANIM_MISSILE1);
			}
			else if (chance < 30)
			{
				SetAnim(self, ANIM_RUNAWAY);
				self->monsterinfo.flee_finished = level.time + 1.0f;
			}
			else
			{
				SetAnim(self, irand(ANIM_MELEE1, ANIM_MELEE2));
			}

			return;
		}

		if (dist > 64.0f && dist < 200.0f)
		{
			if (chance < 40)
				SetAnim(self, ANIM_MISSILE1);
			else if (self->monsterinfo.flee_finished < level.time && dist > 100.0f)
				SetAnim(self, ANIM_RUN2);
			else
				SetAnim(self, ANIM_STAND3);

			return;
		}

		if (dist < 300.0f)
		{
			if (chance < 5)
				SetAnim(self, ANIM_STAND3);
			else if (chance < 20)
				SetAnim(self, ANIM_RUN2);
			else
				SetAnim(self, ANIM_RUN1);

			return;
		}

		SetAnim(self, ANIM_RUN1);
		return;
	}

	if (irand(0, 10) < 6)
		G_PostMessage(self, MSG_RUN, PRI_DIRECTIVE, NULL);
	else
		GkrokonFallbackMsgHandler(self, msg); //mxd. Use function.
}

static void GkrokonPainMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'beetle_pain' in original logic.
{
	int	temp;
	int damage;
	int	force_damage;
	G_ParseMsgParms(msg, "eeiii", &temp, &temp, &force_damage, &damage, &temp);

	// Weighted random based on health compared to maximum health.
	if (force_damage || (irand(0, self->max_health + 50) > self->health && irand(0, 2) > 0)) //mxd. flrand() in original logic.
	{
		gi.sound(self, CHAN_WEAPON, sounds[irand(SND_PAIN1, SND_PAIN2)], 1.0f, ATTN_NORM, 0.0f);
		SetAnim(self, ANIM_PAIN1);
	}

	if (irand(0, 2) == 0)
	{
		self->monsterinfo.aiflags |= AI_FLEE;
		self->monsterinfo.flee_finished = level.time + 15.0f;
	}
}

static void GkrokonEatMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'beetle_eat' in original logic.
{
	SetAnim(self, irand(ANIM_EAT1, ANIM_EAT3));
}

static void GkrokonDeathMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'beetle_death' in original logic.
{
	edict_t* target;
	edict_t* inflictor;
	edict_t* attacker;
	float damage;
	G_ParseMsgParms(msg, "eeei", &target, &inflictor, &attacker, &damage);

	M_StartDeath(self, ANIM_DIE1);

	if (self->health < -80)
	{
		BecomeDebris(self);
		return;
	}

	if (self->health < -10)
	{
		if (self->svflags & SVF_MONSTER)
			for (int i = 0; i < irand(3, 10); i++)
				self->monsterinfo.dismember(self, irand(80, 160), irand(hl_Head, hl_LegLowerRight) | hl_MeleeHit); //mxd. flrand() in original logic.

		vec3_t forward;
		AngleVectors(self->s.angles, forward, NULL, NULL);

		VectorMA(self->velocity, -400.0f, forward, self->velocity);
		self->velocity[2] += 150.0f;
	}

	SetAnim(self, ANIM_DIE1);
	gi.sound(self, CHAN_BODY, sounds[SND_DIE], 1.0f, ATTN_NORM, 0.0f);
}

static void GkrokonDeathPainMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'beetle_dead_pain' in original logic.
{
	if (self->health <= -80)
	{
		// We got gibbed.
		gi.sound(self, CHAN_BODY, sounds[SND_GIB], 1.0f, ATTN_NORM, 0.0f);

		self->think = BecomeDebris;
		self->nextthink = level.time + FRAMETIME;
	}
	else if (msg != NULL && !(self->svflags & SVF_PARTS_GIBBED))
	{
		DismemberMsgHandler(self, msg);
	}
}

#pragma endregion

#pragma region ========================== GkrokonDismember logic ==========================

static qboolean GkrokonCanThrowNode(edict_t* self, const int node_id, int* throw_nodes) //mxd. Named 'canthrownode_gk' in original logic.
{
	static const int bit_for_mesh_node[NUM_MESH_NODES] = //mxd. Made local static.
	{
		BIT_WAIT1,
		BIT_SHELLA_P1,
		BIT_SPIKE_P1,
		BIT_HEAD_P1,
		BIT_RPINCHERA_P1,
		BIT_RPINCHERB_P1,
		BIT_LPINCHERA_P1,
		BIT_LPINCHERB_P1,
		BIT_LARM_P1,
		BIT_RARM_P1,
		BIT_ABDOMEN_P1,
		BIT_LTHIGH_P1,
		BIT_RTHIGH_P1,
		BIT_SHELLB_P1,
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

static void GkrokonThrowBodyPart(edict_t* self, const int mesh_id) //mxd. Split from GkrokonDismember() to simplify logic.
{
	if (!(self->s.fmnodeinfo[mesh_id].flags & FMNI_NO_DRAW))
	{
		self->s.fmnodeinfo[mesh_id].flags |= FMNI_USE_SKIN;
		self->s.fmnodeinfo[mesh_id].skin = self->s.skinnum + 1;
	}
}

static qboolean GkrokonThrowTorsoFront(edict_t* self, const float damage) //mxd. Split from GkrokonDismember() to simplify logic.
{
	static const int mesh_ids[] = //mxd
	{
		MESH__HEAD_P1,
		MESH__SPIKE_P1,
		MESH__RPINCHERA_P1,
		MESH__LPINCHERA_P1,
		MESH__RPINCHERB_P1,
		MESH__LPINCHERB_P1
	};

	int throw_nodes = 0;

	vec3_t forward; //mxd
	vec3_t right;
	AngleVectors(self->s.angles, forward, right, NULL);

	for (uint i = 0; i < ARRAY_SIZE(mesh_ids); i++)
	{
		const int mesh_id = mesh_ids[i];

		if (!(self->s.fmnodeinfo[mesh_id].flags & FMNI_USE_SKIN))
		{
			if (irand(0, 4) > 0)
			{
				self->s.fmnodeinfo[mesh_id].flags |= FMNI_USE_SKIN;
				self->s.fmnodeinfo[mesh_id].skin = self->s.skinnum + 1;
			}
		}
		else if (GkrokonCanThrowNode(self, mesh_id, &throw_nodes))
		{
			vec3_t gore_spot = { 0.0f, 0.0f, self->maxs[2] * 0.3f }; //mxd. In original logic, gore_spot[2] is incremented every time this branch is executed in for loop.
			VectorMA(gore_spot, flrand( -10.0f, 10.0f), forward, gore_spot); //mxd
			VectorMA(gore_spot, flrand( -10.0f, 10.0f), right, gore_spot); //mxd. Randomize position.

			ThrowBodyPart(self, gore_spot, throw_nodes, (int)damage, FRAME_birth1);

			if (mesh_id == MESH__HEAD_P1)
			{
				T_Damage(self, self->enemy, self->enemy, vec3_origin, vec3_origin, vec3_origin, 9999, 200, DAMAGE_NORMAL, MOD_DIED);
				return true;
			}
		}
	}

	return false;
}

static void GkrokonThrowArm(edict_t* self, float damage, const int mesh_part, const qboolean dismember_ok) //mxd. Split from GkrokonDismember() to simplify logic.
{
	if (self->s.fmnodeinfo[mesh_part].flags & FMNI_NO_DRAW)
		return;

	if (self->s.fmnodeinfo[mesh_part].flags & FMNI_USE_SKIN) // Original logic does this only for left arm --mxd.
		damage *= 1.5f; // Greater chance to cut off if previously damaged.

	if (dismember_ok && flrand(0, (float)self->health) < damage * 0.75f)
	{
		int throw_nodes = 0;

		if (GkrokonCanThrowNode(self, mesh_part, &throw_nodes))
		{
			vec3_t right;
			AngleVectors(self->s.angles, NULL, right, NULL);

			vec3_t gore_spot = { 0.0f, 0.0f, self->maxs[2] * 0.3f };
			const float side = ((mesh_part == MESH__LARM_P1) ? -1.0f : 1.0f);
			VectorMA(gore_spot, 10.0f * side, right, gore_spot);

			ThrowBodyPart(self, gore_spot, throw_nodes, (int)damage, FRAME_birth1);
		}
	}
	else
	{
		self->s.fmnodeinfo[mesh_part].flags |= FMNI_USE_SKIN;
		self->s.fmnodeinfo[mesh_part].skin = self->s.skinnum + 1;
	}
}

static void GkrokonThrowLeg(edict_t* self, const float damage, const int mesh_part) //mxd. Split from GkrokonDismember() to simplify logic.
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

		if (GkrokonCanThrowNode(self, mesh_part, &throw_nodes))
		{
			vec3_t right;
			AngleVectors(self->s.angles, NULL, right, NULL);

			vec3_t gore_spot = { 0.0f, 0.0f, self->maxs[2] * 0.3f };
			const float side = ((mesh_part == MESH__LTHIGH_P1) ? -1.0f : 1.0f);
			VectorMA(gore_spot, 10.0f * side, right, gore_spot); //BUGFIX: mxd. scaled by -10 for both legs in original logic.

			ThrowBodyPart(self, gore_spot, throw_nodes, (int)damage, FRAME_birth1);
		}
	}
}

void GkrokonDismember(edict_t* self, int damage, HitLocation_t hl) //mxd. Named 'beetle_dismember' in original logic.
{
	qboolean dismember_ok = false;

	if (hl & hl_MeleeHit)
	{
		dismember_ok = true;
		hl &= ~hl_MeleeHit;
	}

	if (hl <= hl_NoneSpecific || hl >= hl_Max) //mxd. 'hl > hl_Max' in original logic.
		return;

	switch (hl)
	{
		case hl_Head:
			GkrokonThrowBodyPart(self, MESH__SHELLB_P1);
			break;

		case hl_TorsoFront: // Split in half?
			if (GkrokonThrowTorsoFront(self, (float)damage))
				return;
			break;

		case hl_TorsoBack: // Split in half?
			GkrokonThrowBodyPart(self, MESH__ABDOMEN_P1);
			break;

		case hl_ArmLowerLeft: // Left arm.
		case hl_ArmUpperLeft:
			GkrokonThrowArm(self, (float)damage, MESH__LARM_P1, dismember_ok);
			break;

		case hl_ArmUpperRight: // Right arm.
		case hl_ArmLowerRight:
			GkrokonThrowArm(self, (float)damage, MESH__RARM_P1, dismember_ok);
			break;

		case hl_LegUpperLeft: // Left leg.
		case hl_LegLowerLeft:
			GkrokonThrowLeg(self, (float)damage, MESH__LTHIGH_P1);
			break;

		case hl_LegUpperRight: // Right leg.
		case hl_LegLowerRight:
			GkrokonThrowLeg(self, (float)damage, MESH__RTHIGH_P1);
			break;

		default:
			break;
	}

	if (self->s.fmnodeinfo[MESH__SPIKE_P1].flags & FMNI_NO_DRAW)
		self->monsterinfo.aiflags |= (AI_COWARD | AI_NO_MELEE | AI_NO_MISSILE);
}

#pragma endregion

#pragma region ========================== Action functions ==========================

void gkrokon_bite(edict_t* self, float right_side) //mxd. Named 'GkrokonBite' in original logic.
{
	vec3_t start_offset;
	vec3_t end_offset;

	// From the right side.
	if (right_side != 0.0f)
	{
		VectorSet(start_offset, 32.0f, 48.0f, 32.0f);
		VectorSet(end_offset, 32.0f, 16.0f, 24.0f);
	}
	else
	{
		VectorSet(start_offset, 32.0f, -24.0f, 32.0f);
		VectorSet(end_offset, 24.0f, 32.0f, 24.0f);
	}

	const vec3_t mins = { -2.0f, -2.0f, -2.0f };
	const vec3_t maxs = {  2.0f,  2.0f,  2.0f };

	trace_t trace;
	vec3_t direction;
	edict_t* victim = M_CheckMeleeLineHit(self, start_offset, end_offset, mins, maxs, &trace, direction);

	if (victim != NULL && victim != self)
	{
		// Hurt whatever we were whacking away at.
		vec3_t blood_dir;
		VectorSubtract(start_offset, end_offset, blood_dir);
		VectorNormalize(blood_dir);

		const int damage = irand(GKROKON_DMG_BITE_MIN, GKROKON_DMG_BITE_MAX);
		T_Damage(victim, self, self, direction, trace.endpos, blood_dir, damage, damage, DAMAGE_NORMAL, MOD_DIED);

		// Play hit sound.
		gi.sound(self, CHAN_WEAPON, sounds[irand(SND_BITEHIT1, SND_BITEHIT2)], 1.0f, ATTN_NORM, 0.0f);
	}
	else
	{
		// Play swoosh sound.
		gi.sound(self, CHAN_WEAPON, sounds[irand(SND_BITEMISS1, SND_BITEMISS2)], 1.0f, ATTN_NORM, 0.0f);
	}
}

void gkrokon_sound(edict_t* self, float channel, float sound_index, float attenuation) //mxd. Named 'gkrokonSound' in original logic.
{
	gi.sound(self, (int)channel, sounds[(int)sound_index], 1.0f, attenuation, 0.0f);
}

void gkrokon_walk_sound(edict_t* self) //mxd. Named 'gkrokonRandomWalkSound' in original logic.
{
	if (irand(0, 3) == 0)
		gi.sound(self, CHAN_BODY, sounds[irand(SND_WALK1, SND_WALK2)], 1.0f, ATTN_NORM, 0.0f);
}

void gkrokon_idle_sound(edict_t* self) //mxd. Named 'beetle_ai_stand' in original logic.
{
	if (irand(0, 20) < 2)
		gi.sound(self, CHAN_BODY, sounds[irand(SND_IDLE1, SND_IDLE2)], 1.0f, ATTN_NORM, 0.0f);
}

void gkrokon_dead(edict_t* self) //mxd. Named 'GkrokonDead' in original logic.
{
	M_EndDeath(self);
}

void gkrokon_ai_stand(edict_t* self, float dist) //mxd. Named 'beetle_ai_stand' in original logic.
{
	if (M_ValidTarget(self, self->enemy))
		MG_FaceGoal(self, true);
}

void gkrokon_pause(edict_t* self) //mxd. Named 'GkrokonPause' in original logic.
{
	if ((self->spawnflags & MSF_FIXED) && self->curAnimID == ANIM_DELAY && self->enemy != NULL)
	{
		self->monsterinfo.searchType = SEARCH_COMMON;
		MG_FaceGoal(self, true);
	}

	self->mood_think(self);

	switch (self->ai_mood)
	{
		case AI_MOOD_ATTACK:
			G_PostMessage(self, MSG_MISSILE, PRI_DIRECTIVE, NULL);
			break;

		case AI_MOOD_PURSUE:
		case AI_MOOD_NAVIGATE:
		case AI_MOOD_FLEE:
			G_PostMessage(self, MSG_RUN, PRI_DIRECTIVE, NULL);
			break;

		case AI_MOOD_WALK:
		case AI_MOOD_WANDER:
			G_PostMessage(self, MSG_WALK, PRI_DIRECTIVE, NULL);
			break;

		case AI_MOOD_BACKUP:
			G_PostMessage(self, MSG_FALLBACK, PRI_DIRECTIVE, NULL);
			break;

		case AI_MOOD_STAND:
			G_PostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
			break;

		case AI_MOOD_JUMP:
			if ((self->spawnflags & MSF_FIXED) || self->jump_chance <= 0)
				SetAnim(self, ANIM_DELAY);
			else
				SetAnim(self, ANIM_FJUMP);
			break;

		case AI_MOOD_EAT:
			G_PostMessage(self, MSG_EAT, PRI_DIRECTIVE, NULL);
			break;

		default:
			G_PostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
			break;
	}
}

void gkrokon_set_stand_anim(edict_t* self) //mxd. Named 'beetle_to_stand' in original logic.
{
	SetAnim(self, ANIM_STAND3);
	gkrokon_pause(self);
}

void gkrokon_set_crouch_anim(edict_t* self) //mxd. Named 'beetle_to_crouch' in original logic.
{
	SetAnim(self, ANIM_CROUCH1);
	gkrokon_pause(self);
}

#pragma endregion

void GkrokonStaticsInit(void)
{
	static ClassResourceInfo_t res_info; //mxd. Made local static.

	classStatics[CID_GKROKON].msgReceivers[MSG_STAND] = GkrokonStandMsgHandler;
	classStatics[CID_GKROKON].msgReceivers[MSG_RUN] = GkrokonRunMsgHandler;
	classStatics[CID_GKROKON].msgReceivers[MSG_CHECK_MOOD] = GkrokonCheckMoodMsgHandler;
	classStatics[CID_GKROKON].msgReceivers[MSG_FALLBACK] = GkrokonFallbackMsgHandler;
	classStatics[CID_GKROKON].msgReceivers[MSG_MELEE] = GkrokonMissileMsgHandler;
	classStatics[CID_GKROKON].msgReceivers[MSG_MISSILE] = GkrokonMissileMsgHandler;
	classStatics[CID_GKROKON].msgReceivers[MSG_PAIN] = GkrokonPainMsgHandler;
	classStatics[CID_GKROKON].msgReceivers[MSG_EAT] = GkrokonEatMsgHandler;
	classStatics[CID_GKROKON].msgReceivers[MSG_DEATH] = GkrokonDeathMsgHandler;
	classStatics[CID_GKROKON].msgReceivers[MSG_DEATH_PAIN] = GkrokonDeathPainMsgHandler;
	classStatics[CID_GKROKON].msgReceivers[MSG_DISMEMBER] = DismemberMsgHandler;

	res_info.numAnims = NUM_ANIMS;
	res_info.animations = animations;
	res_info.modelIndex = gi.modelindex("models/monsters/gkrokon/tris.fm");

	sounds[SND_PAIN1] = gi.soundindex("monsters/beetle/pain1.wav");
	sounds[SND_PAIN2] = gi.soundindex("monsters/beetle/pain2.wav");
	sounds[SND_DIE] = gi.soundindex("monsters/beetle/death.wav");
	sounds[SND_GIB] = gi.soundindex("monsters/insect/gib.wav");
	sounds[SND_SPOO] = gi.soundindex("monsters/beetle/spoo.wav");
	sounds[SND_IDLE1] = gi.soundindex("monsters/beetle/idle1.wav");
	sounds[SND_IDLE2] = gi.soundindex("monsters/beetle/idle2.wav");
	//sounds[SND_SIGHT] = gi.soundindex("monsters/beetle/sight.wav");
	sounds[SND_WALK1] = gi.soundindex("monsters/beetle/walk1.wav");
	sounds[SND_WALK2] = gi.soundindex("monsters/beetle/walk2.wav");
	//sounds[SND_FLEE] = gi.soundindex("monsters/beetle/flee.wav");
	//sounds[SND_ANGRY] = gi.soundindex("monsters/beetle/angry.wav");
	//sounds[SND_EATING] = gi.soundindex("monsters/beetle/eating.wav");
	sounds[SND_BITEHIT1] = gi.soundindex("monsters/beetle/meleehit1.wav");
	sounds[SND_BITEHIT2] = gi.soundindex("monsters/beetle/meleehit2.wav");
	sounds[SND_BITEMISS1] = gi.soundindex("monsters/beetle/meleemiss1.wav");
	sounds[SND_BITEMISS2] = gi.soundindex("monsters/beetle/meleemiss2.wav");

	res_info.numSounds = NUM_SOUNDS;
	res_info.sounds = sounds;

	classStatics[CID_GKROKON].resInfo = &res_info;
}

// QUAKED monster_gkrokon (1 .5 0) (-20 -20 -0) (20 20 32) AMBUSH ASLEEP EATING 8 16 32 64 FIXED WANDER MELEE_LEAD STALK COWARD RESTING EXTRA2 EXTRA3 EXTRA4

// Spawnflags:
// AMBUSH		- Will not be woken up by other monsters or shots from player.
// ASLEEP		- will not appear until triggered.
// EATING		- Chomp chomp... chewie chomp.
// WANDER		- Monster will wander around aimlessly (but follows buoys).
// MELEE_LEAD	- Monster will try to cut you off when you're running and fighting him, works well if there are a few monsters in a group, half doing this, half not.
// STALK		- Monster will only approach and attack from behind. If you're facing the monster it will just stand there.
//				  Once the monster takes pain, however, it will stop this behaviour and attack normally.
// COWARD		- Monster starts off in flee mode- runs away from you when woken up.

// Variables:
// homebuoy					- Monsters will head to this buoy if they don't have an enemy ("homebuoy" should be targetname of the buoy you want them to go to).
// wakeup_target			- Monsters will fire this target the first time it wakes up (only once).
// pain_target				- Monsters will fire this target the first time it gets hurt (only once).
// mintel					- Monster intelligence - this basically tells a monster how many buoys away an enemy has to be for it to give up (default 12).
// melee_range				- How close the player has to be for the monster to go into melee. If this is zero, the monster will never melee.
//							  If it is negative, the monster will try to keep this distance from the player.
//							  If the monster has a backup, he'll use it if too close, otherwise, a negative value here means the monster will just stop
//							  running at the player at this distance (default 0).
//							 Examples:
//								melee_range = 60 - monster will start swinging it player is closer than 60.
//								melee_range = 0 - monster will never do a melee attack.
//								melee_range = -100 - monster will never do a melee attack and will back away (if it has that ability) when player gets too close.
// missile_range			- Maximum distance the player can be from the monster to be allowed to use it's ranged attack (default 256).
// min_missile_range		- Minimum distance the player can be from the monster to be allowed to use it's ranged attack (default 48).
// bypass_missile_chance	- Chance that a monster will NOT fire it's ranged attack, even when it has a clear shot. This, in effect, will make the monster
//							  come in more often than hang back and fire. A percentage (0 = always fire/never close in, 100 = never fire/always close in) - must be whole number (default 0).
// jump_chance				- Every time the monster has the opportunity to jump, what is the chance (out of 100) that he will... (100 = jump every time) - must be whole number (default 100).
// wakeup_distance			- How far (max) the player can be away from the monster before it wakes up. This means that if the monster can see the player,
//							  at what distance should the monster actually notice him and go for him (default 1024).
// NOTE: A value of zero will result in defaults, if you actually want zero as the value, use -1.
void SP_Monster_Gkrokon(edict_t* self)
{
	if (!M_WalkmonsterStart(self)) // Failed initialization.
		return;

	self->msgHandler = DefaultMsgHandler;

	if (self->health == 0)
		self->health = GKROKON_HEALTH;

	self->health = MonsterHealth(self->health);
	self->max_health = self->health;

	self->materialtype = MAT_INSECT;
	self->mass = GKROKON_MASS;
	self->yaw_speed = 20.0f;
	self->movetype = PHYSICSTYPE_STEP;
	VectorClear(self->knockbackvel);
	self->solid = SOLID_BBOX;

	VectorCopy(STDMinsForClass[self->classID], self->mins);
	VectorCopy(STDMaxsForClass[self->classID], self->maxs);

	self->s.modelindex = (byte)classStatics[CID_GKROKON].resInfo->modelIndex;
	self->s.skinnum = ((irand(0, 1) == 1) ? 0 : 2);
	self->monsterinfo.otherenemyname = "";
	self->monsterinfo.searchType = SEARCH_COMMON;
	self->monsterinfo.attack_finished = level.time;

	self->monsterinfo.dismember = GkrokonDismember;

	MG_InitMoods(self);

	// Check our spawnflags to see what our initial behaviour should be.
	if (self->spawnflags & MSF_EATING)
	{
		self->monsterinfo.aiflags |= AI_EATING;
		G_PostMessage(self, MSG_EAT, PRI_DIRECTIVE, NULL);
	}
	else
	{
		G_PostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
	}
}