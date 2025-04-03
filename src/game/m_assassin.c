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
static ClassResourceInfo_t res_info;

#pragma endregion

static void AssassinCinematicAnimsMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'assassin_c_anims' in original logic.
{
	int curr_anim;

	ai_c_readmessage(self, msg);
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

#pragma region ========================== Utility functions ==========================

edict_t* AssassinArrowReflect(edict_t* self, edict_t* other, const vec3_t vel) //TODO: rename to AssassinDaggerReflect?
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

static void AssassinDaggerTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surface) //mxd. Named 'assassinDaggerTouch' in original logic.
{
	if (other == self->owner || other->owner == self->owner)
		return;

	// Are we reflecting?
	if (EntReflecting(other, true, true) && self->reflect_debounce_time > 0)
	{
		Create_rand_relect_vect(self->velocity, self->velocity);
		Vec3ScaleAssign(ASSASSIN_DAGGER_SPEED / 2.0f, self->velocity);
		AssassinArrowReflect(self, other, self->velocity);

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
		if (SKILL >= SKILL_HARD && Q_fabs(self->s.angles[PITCH]) < 45.0f) // Up to extra 10 pts damage if pointed correctly AND on hard skill.
			damage += 45.0f / (45 - Q_fabs(self->s.angles[PITCH])) * 10;

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

#pragma endregion

#pragma region ========================== Action functions ==========================

static void AssassinJumpMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'assassin_jump' in original logic.
{
	SetAnim(self, ANIM_FORCED_JUMP);
	self->monsterinfo.aiflags |= AI_OVERRIDE_GUIDE;
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

// Do melee or ranged attack.
void assassindagger(edict_t* self, const float flags) //TODO: rename to assassin_attack.
{
	if (self->enemy == NULL || self->enemy->health < 0)
	{
		QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
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

// Assigned to 'isBlocked' and 'bounce' callbacks.
static void AssassinBounce(edict_t* self, trace_t* trace) //mxd. Named 'assassin_Touch' in original logic.
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

void assassin_dead(edict_t* self)
{
	self->msgHandler = DeadMsgHandler;
	self->deadState = DEAD_DEAD;
	M_EndDeath(self);
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

	if (self->deadflag == DEAD_DEAD) // Dead but still being hit.
		return;

	self->deadflag = DEAD_DEAD;

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

	if ((self->s.renderfx & RF_ALPHA_TEXTURE) && self->pre_think != assassinDeCloak)
		assassinInitDeCloak(self);
}

void assassingrowl(edict_t* self) //TODO: rename to assassin_growl?
{
	if (irand(0, 20) == 0)
		gi.sound(self, CHAN_AUTO, sounds[irand(SND_GROWL1, SND_GROWL3)], 1.0f, ATTN_IDLE, 0.0f);
}

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

static void AssassinMeleeMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'assassin_melee' in original logic.
{
	if (M_ValidTarget(self, self->enemy))
	{
		if (irand(0, 7) == 0 && assassinCheckTeleport(self, ASS_TP_OFF)) // 12.5% chance to try to get away.
			return;

		AssassinSetRandomAttackAnim(self);
	}
	else
	{
		QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
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
				if (assassinCheckTeleport(self, ASS_TP_OFF))
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
		QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
	}
}

void assassin_post_pain(edict_t* self)
{
	if (self->fire_damage_time < level.time && assassinCheckTeleport(self, ASS_TP_ANY)) // Don't teleport if burning.
		return;

	assassin_pause(self);
}

static qboolean CanThrowNode(edict_t* self, const int node_id, int* throw_nodes) //mxd. Named 'canthrownode_as' in original logic.
{
	static const int bit_for_mesh_node[] = //mxd. Made local static.
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

#pragma endregion

#pragma region ========================== DISMEMGER LOGIC ==========================

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
		CanThrowNode(self, MESH__HEAD, &throw_nodes);

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
		CanThrowNode(self, MESH__TORSOFT, &throw_nodes);
		CanThrowNode(self, MESH__TORSOBK, &throw_nodes);
		CanThrowNode(self, MESH__HEAD, &throw_nodes);
		CanThrowNode(self, MESH__R4ARM, &throw_nodes);
		CanThrowNode(self, MESH__L4ARM, &throw_nodes);
		CanThrowNode(self, MESH__KNIFES, &throw_nodes);
		CanThrowNode(self, MESH__LUPARM, &throw_nodes);
		CanThrowNode(self, MESH__RUPARM, &throw_nodes);

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
		CanThrowNode(self, forearm_part, &throw_nodes);

		if (CanThrowNode(self, mesh_part, &throw_nodes))
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

		if (CanThrowNode(self, mesh_part, &throw_nodes))
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
		CanThrowNode(self, calf_part, &throw_nodes);

		if (CanThrowNode(self, mesh_part, &throw_nodes))
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

		if (CanThrowNode(self, mesh_part, &throw_nodes))
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

static void AssassinDismember(edict_t* self, const int damage, HitLocation_t hl) //mxd. Named 'assassin_dismember' in original logic.
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

static void AssassinDeathPainMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'assassin_dead_pain' in original logic.
{
	if (msg == NULL || (self->svflags & SVF_PARTS_GIBBED))
		return;

	//mxd. Inlined assassin_dismember_msg().
	//FIXME: make part fly dir the vector from hit loc to sever loc. Remember - turn on caps!
	int damage;
	HitLocation_t hl;
	ParseMsgParms(msg, "ii", &damage, &hl);

	AssassinDismember(self, damage, hl);
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
	ParseMsgParms(msg, "eeiii", &inflictor, &attacker, &force_pain, &damage, &temp);

	if (inflictor == attacker || Q_stricmp(inflictor->classname, "Spell_RedRain") == 0 || Q_stricmp(inflictor->classname, "Spell_Hellbolt") == 0) //mxd. stricmp -> Q_stricmp
	{
		// Melee hit or constant effect, don't stick around!
		if (!(self->spawnflags & MSF_ASS_NOTELEPORT) && !(self->spawnflags & MSF_FIXED) && self->groundentity != NULL && assassinChooseTeleportDestination(self, ASS_TP_ANY, true, false))
			return;
	}

	if (!force_pain && irand(0, self->health) > damage) //mxd. flrand() in original logic.
		return;

	self->monsterinfo.aiflags &= ~AI_OVERRIDE_GUIDE;

	if (self->maxs[2] == 0.0f)
		assassinUndoCrouched(self);

	//mxd. Inlined assassin_random_pain_sound().
	gi.sound(self, CHAN_VOICE, sounds[irand(SND_PAIN1, SND_PAIN2)], 1.0f, ATTN_NORM, 0.0f);

	if (force_pain || self->pain_debounce_time < level.time)
	{
		self->pain_debounce_time = level.time + skill->value + 2.0f;
		SetAnim(self, ANIM_PAIN2);

		if (irand(0, 10) > SKILL)
		{
			self->monsterinfo.misc_debounce_time = level.time + 3.0f; // 3 seconds before can re-cloak.
			assassinInitDeCloak(self);
		}
	}
}

void assassinSkipFrameSkillCheck(edict_t* self) //TODO: rename to assassin_skip_frame_skill_check.
{
	if (irand(0, 3) < SKILL)
		self->s.frame++;
}

/*-------------------------------------------------------------------------
	assassin_pause
-------------------------------------------------------------------------*/
void assassin_pause (edict_t *self)
{
	vec3_t	v;
	float	len;

//this gets stuck on, sometimes
	self->s.fmnodeinfo[MESH__LKNIFE].flags |= FMNI_NO_DRAW;
	self->s.fmnodeinfo[MESH__RKNIFE].flags |= FMNI_NO_DRAW;

	if(self->monsterinfo.aiflags&AI_OVERRIDE_GUIDE)
		return;

	if(self->spawnflags & MSF_FIXED && self->curAnimID == ANIM_DELAY && self->enemy)
	{
		self->monsterinfo.searchType = SEARCH_COMMON;
		MG_FaceGoal(self, true);
	}

	self->mood_think(self);

	if (self->ai_mood == AI_MOOD_NORMAL)
	{
		FindTarget(self);

		if(self->enemy)
		{
			VectorSubtract (self->s.origin, self->enemy->s.origin, v);
			len = VectorLength (v);

			if ((len > 80) || (self->monsterinfo.aiflags & AI_FLEE))  // Far enough to run after
			{
				QPostMessage(self, MSG_RUN,PRI_DIRECTIVE, NULL);
			}
			else	// Close enough to Attack 
			{
				QPostMessage(self, MSG_MELEE, PRI_DIRECTIVE, NULL);
			}
		}
	}
	else
	{
		switch (self->ai_mood)
		{
		case AI_MOOD_ATTACK:
			if(self->ai_mood_flags & AI_MOOD_FLAG_MISSILE)
				QPostMessage(self, MSG_MISSILE, PRI_DIRECTIVE, NULL);
			else
				QPostMessage(self, MSG_MELEE, PRI_DIRECTIVE, NULL);
			break;
		case AI_MOOD_PURSUE:
		case AI_MOOD_NAVIGATE:
			QPostMessage(self, MSG_RUN, PRI_DIRECTIVE, NULL);
			break;
		case AI_MOOD_STAND:
			QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
			break;

		case AI_MOOD_WALK:
			QPostMessage(self, MSG_WALK, PRI_DIRECTIVE, NULL);
			break;

		case AI_MOOD_DELAY:
			SetAnim(self, ANIM_DELAY);
			break;

		case AI_MOOD_WANDER:
			if(self->spawnflags&MSF_FIXED)
			{
				SetAnim(self, ANIM_DELAY);
				return;
			}
			QPostMessage(self, MSG_WALK, PRI_DIRECTIVE, NULL);
			break;

		case AI_MOOD_JUMP:
			if(self->spawnflags&MSF_FIXED)
				SetAnim(self, ANIM_DELAY);
			else
				SetAnim(self, ANIM_FJUMP);//fjump will apply the movedir when he's ready
			break;

		default :
#ifdef _DEVEL
			gi.dprintf("assassin: Unusable mood %d!\n", self->ai_mood);
#endif
			break;
		}
	}
}

void assassinChooseJumpAmbush(edict_t *self)
{
	float	dot;
	vec3_t	forward, enemy_dir;

	if(!self->enemy)
	{
		if(irand(0, 10)<4)
			SetAnim(self, ANIM_JUMP);
		else
			SetAnim(self, ANIM_FRONTFLIP);
		return;
	}

	AngleVectors(self->s.angles, forward, NULL, NULL);
	VectorSubtract(self->enemy->s.origin, self->s.origin, enemy_dir);
	VectorNormalize(enemy_dir);
	dot = DotProduct(forward, enemy_dir);
	if(dot<0)
	{//behind
		SetAnim(self, ANIM_BACKFLIP);
		return;
	}

	if(!irand(0, 3))
		SetAnim(self, ANIM_JUMP);
	else
		SetAnim(self, ANIM_FRONTFLIP);
	return;
}

qboolean assassinChooseSideJumpAmbush(edict_t *self)
{//OR: turn and jump?
	float	dot;
	vec3_t	right, enemy_dir;

	if(!self->enemy)
		return false;

	AngleVectors(self->s.angles, NULL, right, NULL);
	VectorSubtract(self->enemy->s.origin, self->s.origin, enemy_dir);
	VectorNormalize(enemy_dir);

	dot = DotProduct(right, enemy_dir);
	if(dot>0)
		VectorScale(right, 300, self->movedir);
	else
		VectorScale(right, -300, self->movedir);

	self->movedir[2] = 200;
	SetAnim(self, ANIM_FJUMP);
	return true;
}
/*-------------------------------------------------------------------------
	assassin_run
-------------------------------------------------------------------------*/
void assassin_run(edict_t *self, G_Message_t *msg)
{
	if(!self->enemy&&self->spawnflags&MSF_WANDER)
	{
		SetAnim(self, ANIM_RUN);
		return;
	}

	if(self->spawnflags&MSF_ASS_STARTSHADOW)//decloak
	{//FIXME: should I wait until infront of enemy and visible to him?
		self->spawnflags &= ~MSF_ASS_STARTSHADOW;
		assassinInitCloak(self);
	}

	if(!(self->spawnflags&MSF_FIXED))
	{
		if(self->spawnflags&MSF_ASS_JUMPAMBUSH)//jump out
		{
			self->spawnflags &= ~MSF_ASS_JUMPAMBUSH;
			assassinChooseJumpAmbush(self);
			return;
		}

		if(self->spawnflags&MSF_ASS_SIDEJUMPAMBUSH)//side-jump out
		{
			self->spawnflags &= ~MSF_ASS_SIDEJUMPAMBUSH;
			if(assassinChooseSideJumpAmbush(self))
				return;
		}
	}

	if(self->curAnimID >= ANIM_CROUCH_IDLE && self->curAnimID < ANIM_CROUCH_END)
	{
		SetAnim(self, ANIM_CROUCH_END);
		return;
	}

	if (M_ValidTarget(self, self->enemy))
	{
		if(!irand(0, 7))
		{
			if(assassinCheckTeleport(self, ASS_TP_OFF))
			{
//				gi.dprintf("run->teleport\n");
				return;
			}
			else if(!irand(0, 3))
			{
				if(!(self->s.renderfx & RF_ALPHA_TEXTURE))
				{	
					if(!(self->spawnflags&MSF_ASS_NOSHADOW))
					{	
						SetAnim(self, ANIM_CLOAK);
						return;
					}
				}//else uncloak - unncloak when die
			}
		}
		if(!(self->spawnflags&MSF_FIXED))
			SetAnim(self, ANIM_RUN);
		else
			SetAnim(self, ANIM_DELAY);
	}
	else
		QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
}


void assassin_go_run(edict_t *self, float dist)
{
	if(!self->maxs[2])
		assassinUndoCrouched (self);

	if(self->enemy)
		MG_AI_Run(self, dist);
	else
		ai_walk(self, dist);
}
/*----------------------------------------------------------------------
  assassin runorder - order the assassin to choose an run animation
-----------------------------------------------------------------------*/
void assassin_runorder(edict_t *self)
{
	QPostMessage(self, MSG_RUN, PRI_DIRECTIVE, NULL);
}


/*-------------------------------------------------------------------------
	assassinsqueal
-------------------------------------------------------------------------*/
void assassinsqueal (edict_t *self)
{
/*
	if(irand(0, 1))
		gi.sound(self, CHAN_WEAPON, Sounds[SND_PAIN1], 1, ATTN_NORM, 0);
	else
		gi.sound(self, CHAN_WEAPON, Sounds[SND_PAIN2], 1, ATTN_NORM, 0);
*/
}

/*-------------------------------------------------------------------------
	assassin_stand
-------------------------------------------------------------------------*/
void assassin_stand(edict_t *self, G_Message_t *msg)
{	
	if (self->ai_mood == AI_MOOD_DELAY)
		SetAnim(self, ANIM_DELAY);
	else
	{
		if(self->s.renderfx & RF_ALPHA_TEXTURE)
		{
			if(self->pre_think != assassinDeCloak)
				assassinInitDeCloak(self);
		}
		SetAnim(self, ANIM_STAND);
	}

	return;
}


/*-------------------------------------------------------------------------
	assassin_walk
-------------------------------------------------------------------------*/
void assassin_crouch_idle_decision (edict_t *self)
{//FIXME: need to uncrouch
	int	chance;

	chance = irand(0, 100);
	switch(self->curAnimID)
	{
	case ANIM_CROUCH_IDLE:
		if(chance < 55)
			SetAnim(self, ANIM_CROUCH_IDLE);
		else if(chance < 75)
			SetAnim(self, ANIM_CROUCH_POKE);
		else if(chance < 85)
			SetAnim(self, ANIM_CROUCH_LOOK_RIGHT);
		else if(chance < 95)
			SetAnim(self, ANIM_CROUCH_LOOK_LEFT);
		else
			SetAnim(self, ANIM_CROUCH_END);
		break;
	
	case ANIM_CROUCH_LOOK_RIGHT:
	case ANIM_CROUCH_LOOK_RIGHT_IDLE:
	case ANIM_CROUCH_LOOK_L2R:
		if(chance < 60)
			SetAnim(self, ANIM_CROUCH_LOOK_RIGHT_IDLE);
		else if(chance < 85)
			SetAnim(self, ANIM_CROUCH_LOOK_R2C);
		else
			SetAnim(self, ANIM_CROUCH_LOOK_R2L);
		break;
	
	case ANIM_CROUCH_LOOK_LEFT:
	case ANIM_CROUCH_LOOK_LEFT_IDLE:
	case ANIM_CROUCH_LOOK_R2L:
		if(chance < 60)
			SetAnim(self, ANIM_CROUCH_LOOK_LEFT_IDLE);
		else if(chance < 85)
			SetAnim(self, ANIM_CROUCH_LOOK_L2C);
		else
			SetAnim(self, ANIM_CROUCH_LOOK_L2R);
		break;
	
	case ANIM_CROUCH_TRANS:
		self->monsterinfo.pausetime = 99999999;
		SetAnim(self, ANIM_CROUCH_IDLE);
		break;

	case ANIM_CROUCH_LOOK_R2C:
	case ANIM_CROUCH_LOOK_L2C:
	case ANIM_CROUCH_POKE:
		SetAnim(self, ANIM_CROUCH_IDLE);
		break;

	case ANIM_CROUCH_END:
		self->damage_debounce_time = level.time + 10;
		self->monsterinfo.pausetime = -1;
		SetAnim(self, ANIM_STAND);
		break;
	
	default:
		SetAnim(self, ANIM_CROUCH_END);
		break;
	}
}

void assassin_ai_walk (edict_t *self, float dist)
{
	if(self->damage_debounce_time < level.time)
	{
		if(self->enemy)
		{
			if(vhlen(self->s.origin, self->enemy->s.origin) < 48 && AI_IsInfrontOf(self, self->enemy))
			{
				assassinNodeOn (self, MESH__LKNIFE);
				SetAnim(self, ANIM_CROUCH_TRANS);
				return;
			}
		}
		else if(self->oldenemy)
		{
			if(vhlen(self->s.origin, self->oldenemy->s.origin) < 48 && AI_IsInfrontOf(self, self->oldenemy))
			{
				assassinNodeOn (self, MESH__LKNIFE);
				SetAnim(self, ANIM_CROUCH_TRANS);
				return;
			}
		}
	}
	ai_walk(self, dist);
}

void assassin_walk(edict_t *self, G_Message_t *msg)
{
	if(self->spawnflags&MSF_FIXED)
	{
		SetAnim(self, ANIM_DELAY);
		return;
	}
	if(self->curAnimID == ANIM_WALK_LOOP)
		SetAnim(self, ANIM_WALK_LOOP);
	else
		SetAnim(self, ANIM_WALK);
	return;	
}

void assasin_walk_loop_go (edict_t *self)
{
	SetAnim(self, ANIM_WALK_LOOP);
}
//=============================================================

// EVASION

//=============================================================

void assassinDodgeLeft (edict_t *self)
{
	SetAnim(self, ANIM_DODGE_LEFT);
}

void assassinDodgeRight (edict_t *self)
{
	SetAnim(self, ANIM_DODGE_RIGHT);
}

void assassinFrontFlip (edict_t *self)
{
	SetAnim(self, ANIM_EVFRONTFLIP);
}

void assassinBackFlip (edict_t *self)
{
	SetAnim(self, ANIM_EVBACKFLIP);
}

void assassinBackSprings (edict_t *self)
{
	SetAnim(self, ANIM_BACKSPRING);
}

void assassinJump (edict_t *self)
{
	SetAnim(self, ANIM_EVJUMP);
}

void assassinCrouch (edict_t *self)
{
	SetAnim(self, ANIM_CROUCH);
}

void assassinCrouchedAttack (edict_t *self)
{
	SetAnim(self, ANIM_DAGGERC);
}

void assassin_evade (edict_t *self, G_Message_t *msg)
{
	edict_t			*projectile;
	HitLocation_t	HitLocation;
	int duck_chance, dodgeleft_chance, dodgeright_chance, jump_chance, backflip_chance, frontflip_chance;
	int chance;
	float eta;

	if(!self->groundentity)
		return;

	ParseMsgParms(msg, "eif", &projectile, &HitLocation, &eta);
	
	if(eta < 2)
		self->evade_debounce_time = level.time + eta;
	else
		self->evade_debounce_time = level.time + 2;

	if(skill->value || self->spawnflags & MSF_ASS_TELEPORTDODGE)
	{//Pussies were complaining about assassins teleporting away from certain death, so don't do that unless in hard
		if(!stricmp(projectile->classname, "Spell_PhoenixArrow") ||
			!stricmp(projectile->classname, "Spell_FireWall") ||
			!stricmp(projectile->classname, "Spell_SphereOfAnnihilation") ||
			!stricmp(projectile->classname, "Spell_Maceball"))
		{
			if(assassinChooseTeleportDestination(self, ASS_TP_OFF, true, true))
				return;
		}
	}

	switch(HitLocation)
	{
		case hl_Head:
			duck_chance = 95;
			dodgeleft_chance = 50;
			dodgeright_chance = 50;
			jump_chance = 0;
			backflip_chance = 20;
			frontflip_chance = 20;
		break;
		case hl_TorsoFront://split in half?
			duck_chance = 85;
			dodgeleft_chance = 40;
			dodgeright_chance = 40;
			jump_chance = 0;
			backflip_chance = 60;
			frontflip_chance = 0;
		break;
		case hl_TorsoBack://split in half?
			duck_chance = 80;
			dodgeleft_chance = 40;
			dodgeright_chance = 40;
			jump_chance = 0;
			backflip_chance = 0;
			frontflip_chance = 60;
		break;
		case hl_ArmUpperLeft:
			duck_chance = 75;
			dodgeleft_chance = 0;
			dodgeright_chance = 90;
			jump_chance = 0;
			backflip_chance = 20;
			frontflip_chance = 20;
		break;
		case hl_ArmLowerLeft://left arm
			duck_chance = 75;
			dodgeleft_chance = 0;
			dodgeright_chance = 80;
			jump_chance = 30;
			backflip_chance = 20;
			frontflip_chance = 20;
		break;
		case hl_ArmUpperRight:
			duck_chance = 60;
			dodgeleft_chance = 90;
			dodgeright_chance = 0;
			jump_chance = 0;
			backflip_chance = 20;
			frontflip_chance = 20;
		break;
		case hl_ArmLowerRight://right arm
			duck_chance = 20;
			dodgeleft_chance = 80;
			dodgeright_chance = 0;
			jump_chance = 30;
			backflip_chance = 20;
			frontflip_chance = 20;
		break;
		case hl_LegUpperLeft:
			duck_chance = 0;
			dodgeleft_chance = 0;
			dodgeright_chance = 60;
			jump_chance = 50;
			backflip_chance = 30;
			frontflip_chance = 30;
		break;
		case hl_LegLowerLeft://left leg
			duck_chance = 0;
			dodgeleft_chance = 0;
			dodgeright_chance = 30;
			jump_chance = 80;
			backflip_chance = 40;
			frontflip_chance = 40;
		break;
		case hl_LegUpperRight:
			duck_chance = 0;
			dodgeleft_chance = 60;
			dodgeright_chance = 0;
			jump_chance = 50;
			backflip_chance = 30;
			frontflip_chance = 30;
		break;
		case hl_LegLowerRight://right leg
			duck_chance = 0;
			dodgeleft_chance = 30;
			dodgeright_chance = 0;
			jump_chance = 80;
			backflip_chance = 40;
			frontflip_chance = 40;
		break;
		default:
			duck_chance = 20;
			dodgeleft_chance = 10;
			dodgeright_chance = 10;
			jump_chance = 10;
			backflip_chance = 10;
			frontflip_chance = 10;
		break;
	}

	if(irand(0, 100) < skill->value * 10)
	{
		if(self->pre_think != assassinCloak)
			assassinInitCloak(self);
	}

	chance = irand(0, 10);
	if(skill->value || self->spawnflags & MSF_ASS_TELEPORTDODGE)
	{//Pussies were complaining about assassins teleporting away from certain death, so don't do that unless in hard
		if(chance > 8 && !(self->spawnflags&MSF_ASS_NOTELEPORT))
			if(assassinChooseTeleportDestination(self, ASS_TP_DEF, false, false))
			{
//				gi.dprintf("Assassin teleport evade\n");
				return;
			}
	}

	chance = irand(0, 100);
	if(chance < frontflip_chance)
	{
//		gi.dprintf("Assassin fflip evade\n");
		assassinFrontFlip(self);
		return;
	}

	chance = irand(0, 100);
	if(chance < backflip_chance)
	{
		if(self->curAnimID == ANIM_RUN && irand(0, 3))//running, do the front flip
		{
//			gi.dprintf("Assassin fflip evade\n");
			assassinFrontFlip(self);
		}
		else
		{
			if(irand(0, 1))
			{
//				gi.dprintf("Assassin bspring evade\n");
				assassinBackSprings(self);
			}
			else
			{
//				gi.dprintf("Assassin bflip evade\n");
				assassinBackFlip(self);
			}
		}
		return;
	}

	chance = irand(0, 100);
	if(chance < duck_chance)
	{
		self->evade_debounce_time = level.time + eta + 2 - skill->value;
//		gi.dprintf("Assassin crouch evade\n");
		assassinCrouch(self);
		return;
	}

	chance = irand(0, 100);
	if(chance < dodgeleft_chance)
	{
//		gi.dprintf("Assassin dleft evade\n");
		assassinDodgeLeft(self);
		return;
	}
	
	chance = irand(0, 100);
	if(chance < dodgeright_chance)
	{
//		gi.dprintf("Assassin dright evade\n");
		assassinDodgeRight(self);
		return;
	}
	
	chance = irand(0, 100);
	if(chance < jump_chance)
	{
		if(self->curAnimID == ANIM_RUN && irand(0, 4))//running, do the front flip
		{
//			gi.dprintf("Assassin fflip evade\n");
			assassinFrontFlip(self);
		}
		else
		{
//			gi.dprintf("Assassin jump evade\n");
			assassinJump(self);
		}
		return;
	}

	if(skill->value || self->spawnflags & MSF_ASS_TELEPORTDODGE)
	{//Pussies were complaining about assassins teleporting away from certain death, so don't do that unless in hard
		if(!(self->spawnflags&MSF_ASS_NOTELEPORT))
			if(assassinChooseTeleportDestination(self, ASS_TP_DEF, false, false))
			{
//				gi.dprintf("Assassin tport(desperate) evade\n");
				return;
			}
	}

	self->evade_debounce_time = 0;
//	gi.dprintf("Assassin failed to evade\n");
}

void assassinCrouchedCheckAttack (edict_t *self, float attack)
{
	if(!AI_IsClearlyVisible(self, self->enemy))
		return;

	if(!AI_IsInfrontOf(self, self->enemy))
		return;

	if(irand(0,10)<5)
		return;

	if(attack == true)
		assassindagger(self, BIT_RKNIFE);
	else if(attack == 2)//start crouched attack anim
		SetAnim(self, ANIM_DAGGERC);
	else//loop back inside that anim
		self->monsterinfo.currframeindex = 2;
}

void assassinNodeOn (edict_t *self, float node)
{
	self->s.fmnodeinfo[(int)node].flags &= ~FMNI_NO_DRAW;
}

void assassinStop (edict_t *self)
{
	if(self->evade_debounce_time - level.time> 0.1f)
		self->nextthink = level.time + (self->evade_debounce_time - level.time);
}

void assassinSetCrouched (edict_t *self)
{
	VectorSet (self->maxs, 16, 16, 0);	
	self->viewheight = 0;
}

void assassinUndoCrouched (edict_t *self)
{
	VectorSet (self->maxs, 16, 16, 48);	
	self->viewheight = 40;
}

void assassin_sound(edict_t *self, float channel, float soundnum, float attn)
{
	gi.sound (self, (int)(channel), sounds[(int)(soundnum)], 1, (int)(attn), 0);
}

void assassinGoJump (edict_t *self, float fwdspd,float upspd,float rtspd)
{//fixme: do checks and traces first
	vec3_t up, forward, right;

	self->monsterinfo.aiflags &= ~AI_OVERRIDE_GUIDE;
	assassin_sound(self, CHAN_VOICE, SND_JUMP, ATTN_NORM);
	AngleVectors(self->s.angles, forward, right, up);
	
	VectorMA(self->velocity, upspd, up, self->velocity);
	VectorMA(self->velocity, fwdspd, forward, self->velocity);
	VectorMA(self->velocity, rtspd, right, self->velocity);
}

void assassin_go_inair(edict_t *self)
{
	SetAnim(self, ANIM_INAIR);
}

void assassin_go_evinair(edict_t *self)
{
	SetAnim(self, ANIM_EVINAIR);
}

void assassin_go_ffinair(edict_t *self)
{
	SetAnim(self, ANIM_FFINAIR);
}

void assassin_go_bfinair(edict_t *self)
{
	SetAnim(self, ANIM_BFINAIR);
}

void assassinCheckLoop (edict_t *self, float frame)
{//see if should fire again
	vec3_t	v;
	float	len, melee_range, min_seperation, jump_range;

	if(!self->enemy)
		return;

	ai_charge2(self, 0);

	if(!AI_IsClearlyVisible(self, self->enemy))
		return;

	if(!AI_IsInfrontOf(self, self->enemy))
		return;

	if(irand(0, 100) < self->bypass_missile_chance)
	{
		self->monsterinfo.attack_finished = level.time + 3 - skill->value;
		return;
	}

	if(self->ai_mood_flags&AI_MOOD_FLAG_BACKSTAB)
		return;

	VectorSubtract (self->s.origin, self->enemy->s.origin, v);
	len = VectorLength (v);
	melee_range = 64;
	jump_range = 128;
	min_seperation = self->maxs[0] + self->enemy->maxs[0];

	if (AI_IsInfrontOf(self, self->enemy))
	{//don't loop if enemy close enough
		if (len < min_seperation + melee_range)
			return;
		else if (len < min_seperation + jump_range && irand(0,10)<3)
			return;
	}

	self->monsterinfo.currframeindex = (int)frame;
}

void assassinSmoke(edict_t *self)
{
	vec3_t pos;
	VectorCopy(self->s.origin, pos);
	pos[2]+=self->mins[2];
	gi.CreateEffect(NULL, FX_TPORTSMOKE, 0, pos, "");//, "db", hitangles, 5);
	//gi.CreateEffect(&self->s, FX_DUST_PUFF, CEF_OWNERS_ORIGIN, self->s.origin, NULL);
}

void assassinGone(edict_t *self)
{
	vec3_t enemy_dir;

	if(self->placeholder)
		G_FreeEdict(self->placeholder);

	VectorCopy(self->pos2, self->s.origin);
	
	if(self->enemy)
	{//face enemy
		VectorSubtract(self->enemy->s.origin, self->s.origin, enemy_dir);
		self->s.angles[YAW] = anglemod(VectorYaw(enemy_dir));
	}

	assassinSmoke(self);

	VectorCopy(self->pos2, enemy_dir);//reuse
	enemy_dir[2] += 100;
	if(gi.pointcontents(enemy_dir) == CONTENTS_EMPTY&&!irand(0,3))
		assassinFrontFlip(self);
	else
		SetAnim(self, ANIM_UNCROUCH);

	self->monsterinfo.aiflags &= ~AI_OVERRIDE_GUIDE;
	self->svflags &= ~SVF_NO_AUTOTARGET;

	//dumbed down
	self->touch_debounce_time = level.time + (10 - skill->value*3);

	//Should we clear velocity too?
	//VectorClear(self->velocity);
	gi.linkentity(self);
}

void assassinPrepareTeleportDest(edict_t *self, vec3_t spot, qboolean instant)
{
	if(self->s.renderfx & RF_ALPHA_TEXTURE)
	{
		if(self->pre_think != assassinDeCloak)
		{
			assassinInitDeCloak(self);
			self->monsterinfo.misc_debounce_time = level.time + 3;
		}
	}

	VectorCopy(spot, self->pos2);

	self->placeholder = G_Spawn();
	VectorCopy(self->pos2, self->placeholder->s.origin);
	self->placeholder->solid = SOLID_BBOX;
	VectorCopy(self->mins, self->placeholder->mins);
	VectorCopy(self->maxs, self->placeholder->maxs);
	self->placeholder->think = G_FreeEdict;
	self->placeholder->nextthink = level.time + 2;//just in case

	//dumbed down
	if(instant && skill->value > 1)
	{
		assassinReadyTeleport(self);
		assassinGone(self);
	}
	else
	{
		SetAnim(self, ANIM_TELEPORT);
	}
}

qboolean assassinChooseTeleportDestination(edict_t *self, int type, qboolean imperative, qboolean instant)
{//FIXME: don't teleport into area with red rain or ripper balls!
	vec3_t	teleport_angles, forward, endpos, startpos;
	trace_t trace;
	int	chance, num_tries, i;
	edict_t	*noblockent;
	float	tracedist;

	//Instead of chance, do around self if evade, around other if ambush

	if(!self->enemy)//fixme- choose my spot?
		return false;

	if(self->spawnflags&MSF_FIXED)
		return false;

	if(imperative)
		num_tries = (skill->value + 1) * 10;
	else
		num_tries = 1;

	for(i = 0; i < num_tries; i++)
	{
		switch(type)
		{
		case ASS_TP_OFF:
			chance = irand(0, 66);
			break;
		case ASS_TP_ANY:
			chance = irand(0, 100);
			break;
		case ASS_TP_DEF:
			chance = irand(33, 100);
			break;
		}

		if(chance<33)
		{//ANY, OFF to behind enemy
			VectorSet(teleport_angles, 0, anglemod(self->enemy->s.angles[YAW] + flrand(-90, 90)), 0);
			AngleVectors(teleport_angles, forward, NULL, NULL);
			VectorCopy(self->enemy->s.origin, startpos);
			startpos[2]+=self->enemy->mins[2];
			startpos[2]-=self->mins[2];
			tracedist = irand(self->min_missile_range, self->missile_range);
			VectorMA(startpos, -tracedist, forward, endpos);
			noblockent = self->enemy;
		}
		else if(chance<66)
		{//ANY to anywhere around enemy
			VectorSet(teleport_angles, 0, anglemod(flrand(0, 360)), 0);
			AngleVectors(teleport_angles, forward, NULL, NULL);
			VectorCopy(self->enemy->s.origin, startpos);
			startpos[2]+=self->enemy->mins[2];
			startpos[2]-=self->mins[2];
			tracedist = irand(self->min_missile_range, self->missile_range);
			VectorMA(startpos, -tracedist, forward, endpos);
			noblockent = self->enemy;
		}
		else
		{//ANY, DEF to anywhere around me
			VectorSet(teleport_angles, 0, anglemod(flrand(0, 360)), 0);
			AngleVectors(teleport_angles, forward, NULL, NULL);
			VectorCopy(self->s.origin, startpos);
			tracedist = irand(self->min_missile_range, self->missile_range/2);
			VectorMA(startpos, -tracedist, forward, endpos);
			noblockent = self;
		}
		
		gi.trace(startpos, self->mins, self->maxs, endpos, noblockent, MASK_MONSTERSOLID,&trace);
		
		if(trace.fraction*tracedist < 100)//min origin lerp dist
			continue;

		if(trace.allsolid || trace.startsolid)
			continue;
		
		if(vhlen(trace.endpos, self->enemy->s.origin)>=self->min_missile_range)
		{
			VectorCopy(trace.endpos, startpos);
			VectorCopy(trace.endpos, endpos);
			endpos[2] -=64;
			gi.trace(startpos, self->mins, self->maxs, endpos, noblockent, MASK_MONSTERSOLID,&trace);
			if(trace.fraction<1.0 && !trace.allsolid && !trace.startsolid)//the last two should be false if trace.fraction is < 1.0 but doesn't hurt to check
			{
				assassinPrepareTeleportDest(self, trace.endpos, instant);
				return true;
			}
		}
	}
	return false;
}

void assassinReadyTeleport (edict_t *self)
{
	assassinSmoke(self);
	self->svflags |= SVF_NO_AUTOTARGET;
}

qboolean assassinCheckTeleport (edict_t *self, int type)
{
	if(self->spawnflags&MSF_ASS_NOTELEPORT)
		return false;

	if(self->spawnflags&MSF_FIXED)
		return false;

	if(!self->groundentity)
		return false;

	if(!M_ValidTarget(self, self->enemy))
		return false;

/*	if(!infront(self->enemy, self))
		return false;

	if(!visible(self->enemy, self))
		return false;*/

	return assassinChooseTeleportDestination(self, type, false, false);
}

void assassinUnCrouch (edict_t *self)
{
	SetAnim(self, ANIM_UNCROUCH);
}

qboolean assassinCheckCloak (edict_t *self)
{
	int	chance = 0;

	if(!self->monsterinfo.awake)
		return (false);

	if(self->monsterinfo.misc_debounce_time > level.time)//cloak debounce time
		return (false);

	if(self->spawnflags & MSF_ASS_NOSHADOW)
		return (false);

	if(self->ai_mood == AI_MOOD_FLEE)
		return (true);

	if(!self->enemy)
		return (false);

	if(AI_IsInfrontOf(self->enemy, self))
		chance = -3;

	if(irand(0, 10 - skill->value + chance) <= 0)
		return (true);

	return (false);
}

qboolean assassinCheckDeCloak (edict_t *self)
{
	float	dist;
	int		chance = 0;

	if(!self->monsterinfo.awake)
		return (false);

	if(self->monsterinfo.misc_debounce_time > level.time)//cloak debounce time
		return (false);

	if(!self->enemy)
	{
		if(!(self->spawnflags & MSF_ASS_STARTSHADOW))
			return (true);
		return (false);
	}

	dist = M_DistanceToTarget(self, self->enemy);
	if(dist<ASSASSIN_MIN_CLOAK_RANGE)
		return (true);

	if(!AI_IsInfrontOf(self->enemy, self))
		chance = -3;

	if(irand(0, 10 + skill->value * 2 + chance) <= 0)
		return (true);

	return (false);
}

void assassinCloakThink (edict_t *self)
{
	edict_t *found = NULL;
	int		lowerseq, upperseq;
	vec3_t	mins, maxs, startpos, endpos, tport_dest;
	trace_t	trace;

	self->pre_think = assassinCloakThink;
	self->next_pre_think = level.time + FRAMETIME;
//check cloak or decloak
	if(!(self->s.renderfx & RF_ALPHA_TEXTURE))
	{//not cloaked
		if(assassinCheckCloak(self))
		{
			self->monsterinfo.misc_debounce_time = level.time + 7;//10 seconds before will willingly uncloak
			assassinInitCloak(self);
		}
	}
	else
	{//cloaked
		if(assassinCheckDeCloak(self))
			assassinInitDeCloak(self);
	}
//check to teleport

	//dumbed down
	if(!skill->value)//was < 2 
	{
		if(self->touch_debounce_time > level.time)
		{
			return;
		}
	}

	if(self->waterlevel == 3 && self->air_finished <= level.time)//going to drown!
	{//pick either last buoy or my startspot
		VectorCopy(self->pos1, tport_dest);
		if(self->lastbuoy>NULL_BUOY)
		{
			if(!(gi.pointcontents(level.buoy_list[self->lastbuoy].origin) & MASK_WATER))
				VectorCopy(level.buoy_list[self->lastbuoy].origin, tport_dest);
		}

		VectorCopy(tport_dest, startpos);
		VectorCopy(self->mins, mins);
		mins[2] = 0;
		VectorCopy(self->maxs, maxs);
		maxs[2] = 1;
		startpos[2] -= self->size[2];

		gi.trace(startpos, mins, maxs, endpos, self, MASK_MONSTERSOLID,&trace);
		if(!trace.allsolid && !trace.startsolid)
		{
			VectorCopy(trace.endpos, startpos);
			VectorCopy(trace.endpos, endpos);
			startpos[2] +=self->size[2];
			gi.trace(startpos, self->mins, self->maxs, endpos, self, MASK_MONSTERSOLID,&trace);
			if(trace.fraction == 1.0 && !trace.allsolid && !trace.startsolid)
			{
				assassinPrepareTeleportDest(self, trace.endpos, false);
				return;
			}
		}
	}
	
	if(skill->value || self->spawnflags & MSF_ASS_TELEPORTDODGE)
	{//Pussies were complaining about assassins teleporting away from certain death, so don't do that unless in hard
		if(!(self->spawnflags & MSF_ASS_NOTELEPORT) && !(self->spawnflags&MSF_FIXED) && self->groundentity)
		{
			if(irand(0, 4 - skill->value) <= 0)
			{//easy is 40% chance per second, hard is 60% chance to check per second
				while(found = FindInRadius(found, self->s.origin, 200 + skill->value * 50))
				{
					if(!stricmp(found->classname, "Spell_Maceball"))
					{
						if(!self->enemy)
						{
							if(found->owner)
							{
								self->enemy = found->owner;
								AI_FoundTarget(self, false);
							}
						}
						if(assassinChooseTeleportDestination(self, ASS_TP_OFF, true, true))
							return;
					}

					if(!stricmp(found->classname, "Spell_RedRain") ||
						!stricmp(found->classname, "Spell_PhoenixArrow") ||
						!stricmp(found->classname, "Spell_FireWall") ||
						!stricmp(found->classname, "Spell_SphereOfAnnihilation"))
					{
						if(!self->enemy)
						{
							if(found->owner)
							{
								self->enemy = found->owner;
								AI_FoundTarget(self, false);
							}
						}
						if(assassinChooseTeleportDestination(self, ASS_TP_ANY, true, false))
							return;
					}

					if(found==self->enemy && found->client)
					{
						if(M_DistanceToTarget(self, self->enemy) < 128)
						{
							if(AI_IsInfrontOf(self->enemy, self))
							{
								//is he using his staff or jumping into me?
								lowerseq = found->client->playerinfo.lowerseq;
								switch(lowerseq)
								{
								case ASEQ_WSWORD_SPIN:
								case ASEQ_WSWORD_SPIN2:
								case ASEQ_WSWORD_STEP2:
								case ASEQ_WSWORD_STEP:
								case ASEQ_POLEVAULT2:
								case ASEQ_POLEVAULT1_W:
								case ASEQ_POLEVAULT1_R:
									if(assassinChooseTeleportDestination(self, ASS_TP_ANY, true, true))
										return;
									break;
								default:
									break;
								}

								upperseq = found->client->playerinfo.upperseq;
								switch(upperseq)
								{
								case ASEQ_WSWORD_SPIN:
								case ASEQ_WSWORD_SPIN2:
								case ASEQ_WSWORD_STEP2:
								case ASEQ_WSWORD_STEP:
								case ASEQ_POLEVAULT2:
								case ASEQ_POLEVAULT1_W:
								case ASEQ_POLEVAULT1_R:
									if(assassinChooseTeleportDestination(self, ASS_TP_ANY, true, true))
										return;
									break;
								default:
									break;
								}
							}
					
							if(found->client->playerinfo.shield_timer > level.time)
							{
								if(assassinChooseTeleportDestination(self, ASS_TP_OFF, true, true))
									return;
							}
						}

					}
				}
			}
		}
	}
	
	if(self->evade_debounce_time < level.time)
		MG_CheckEvade(self);
}

void assassinCloak (edict_t *self)
{
	int	interval = 15;
	
	if(self->s.color.r > 50)
		self->s.color.r += irand(-interval*3, 0);
	if(self->s.color.g > 50)
		self->s.color.g += irand(-interval*3, 0);
	if(self->s.color.b > 50)
		self->s.color.b += irand(-interval*3, 0);

	if(self->s.color.a > 150)
		self->s.color.a += irand(-interval, 0);

	if(self->s.color.r > 50||
		self->s.color.g > 50||
		self->s.color.b > 50||
		self->s.color.a > 150)
	{
		self->pre_think = assassinCloak;
		self->next_pre_think = level.time + FRAMETIME;
	}
	else
	{
		self->pre_think = assassinCloakThink;
		self->next_pre_think = level.time + FRAMETIME;
	}

	if(self->evade_debounce_time < level.time)
		MG_CheckEvade(self);
}

void assassinDeCloak (edict_t *self)
{
	if(!(self->s.renderfx & RF_ALPHA_TEXTURE))
		return;

	if(self->s.color.r<255 - 10)
		self->s.color.r += 10;
	else
		self->s.color.r = 255;

	if(self->s.color.g<255 - 10)
		self->s.color.g += 10;
	else
		self->s.color.g = 255;
	
	if(self->s.color.b<255 - 10)
		self->s.color.b += 10;
	else
		self->s.color.b = 255;
	
	if(self->s.color.a<255 - 5)
		self->s.color.a += 5;
	else
		self->s.color.a = 255;

	if(self->s.color.r == 255&&
		self->s.color.g == 255&&
		self->s.color.b == 255&&
		self->s.color.a == 255)
	{
		self->svflags &= ~SVF_NO_AUTOTARGET;
		self->s.renderfx &= ~RF_ALPHA_TEXTURE;
		if(self->health > 0)
		{
			self->pre_think = assassinCloakThink;
			self->next_pre_think = level.time + FRAMETIME;
		}
		else
		{
			self->pre_think = NULL;
			self->next_pre_think = -1;
		}
	}
	else
	{
		self->pre_think = assassinDeCloak;
		self->next_pre_think = level.time + FRAMETIME;
	}

	if(self->evade_debounce_time < level.time)
		MG_CheckEvade(self);
}

void assassinInitDeCloak (edict_t *self)
{
	gi.sound(self,CHAN_AUTO,sounds[SND_DECLOAK],1,ATTN_NORM,0);
	self->pre_think = assassinDeCloak;
	self->next_pre_think = level.time + FRAMETIME;
}

void assassinInitCloak (edict_t *self)
{
	self->s.renderfx |= RF_ALPHA_TEXTURE;
	self->svflags |= SVF_NO_AUTOTARGET;
	gi.sound(self,CHAN_AUTO,sounds[SND_CLOAK],1,ATTN_NORM,0);
	self->s.color.r = 255;
	self->s.color.g = 255;
	self->s.color.b = 255;
	self->s.color.a = 255;
	self->pre_think = assassinCloak;
	self->next_pre_think = level.time + FRAMETIME;
}

void assassin_check_mood (edict_t *self, G_Message_t *msg)
{
	ParseMsgParms(msg, "i", &self->ai_mood);

	assassin_pause(self);
}
//=============================================================

/*-------------------------------------------------------------------------
	AssassinStaticsInit
-------------------------------------------------------------------------*/
void AssassinStaticsInit(void)
{
	classStatics[CID_ASSASSIN].msgReceivers[MSG_STAND] = assassin_stand;
	classStatics[CID_ASSASSIN].msgReceivers[MSG_WALK] = assassin_walk;
	classStatics[CID_ASSASSIN].msgReceivers[MSG_RUN] = assassin_run;
	classStatics[CID_ASSASSIN].msgReceivers[MSG_MELEE] = AssassinMeleeMsgHandler;
	classStatics[CID_ASSASSIN].msgReceivers[MSG_MISSILE] = AssassinMissileMsgHandler;
	classStatics[CID_ASSASSIN].msgReceivers[MSG_PAIN] = AssassinPainMsgHandler;
	classStatics[CID_ASSASSIN].msgReceivers[MSG_DEATH] = AssassinDeathMsgHandler;
	classStatics[CID_ASSASSIN].msgReceivers[MSG_DISMEMBER] = DismemberMsgHandler;
	classStatics[CID_ASSASSIN].msgReceivers[MSG_JUMP] = AssassinJumpMsgHandler;
	classStatics[CID_ASSASSIN].msgReceivers[MSG_EVADE] = assassin_evade;
	classStatics[CID_ASSASSIN].msgReceivers[MSG_DEATH_PAIN] = AssassinDeathPainMsgHandler;
	classStatics[CID_ASSASSIN].msgReceivers[MSG_CHECK_MOOD] = assassin_check_mood;

	classStatics[CID_ASSASSIN].msgReceivers[MSG_C_IDLE1] = AssassinCinematicAnimsMsgHandler;
	classStatics[CID_ASSASSIN].msgReceivers[MSG_C_RUN1] = AssassinCinematicAnimsMsgHandler;
	classStatics[CID_ASSASSIN].msgReceivers[MSG_C_ATTACK1] = AssassinCinematicAnimsMsgHandler;
	classStatics[CID_ASSASSIN].msgReceivers[MSG_C_ATTACK2] = AssassinCinematicAnimsMsgHandler;

	res_info.numAnims = NUM_ANIMS;
	res_info.animations = animations;
	
	//note that the name is different in the path
	res_info.modelIndex = gi.modelindex("models/monsters/assassin/tris.fm");

	sounds[SND_PAIN1]=gi.soundindex("monsters/assassin/pain1.wav");
	sounds[SND_PAIN2]=gi.soundindex("monsters/assassin/pain2.wav");
	sounds[SND_DIE1]=gi.soundindex("monsters/assassin/death1.wav");
	sounds[SND_GIB]=gi.soundindex("monsters/assassin/gib.wav");
	sounds[SND_THROW1]=gi.soundindex("monsters/assassin/throw1.wav");
	sounds[SND_THROW2]=gi.soundindex("monsters/assassin/throw2.wav");
	sounds[SND_DAGHITF]=gi.soundindex("monsters/assassin/daghitf.wav");
	sounds[SND_DAGHITW]=gi.soundindex("monsters/assassin/daghitw.wav");
	sounds[SND_JUMP]=gi.soundindex("monsters/assassin/jump.wav");
	sounds[SND_FLIP]=gi.soundindex("monsters/assassin/flip.wav");
	sounds[SND_LAND]=gi.soundindex("monsters/assassin/land.wav");
	sounds[SND_LANDF]=gi.soundindex("monsters/assassin/landf.wav");
	sounds[SND_SLIDE]=gi.soundindex("monsters/assassin/slide.wav");
	sounds[SND_SLASH1]=gi.soundindex("monsters/assassin/slash1.wav");
	sounds[SND_SLASH2]=gi.soundindex("monsters/assassin/slash2.wav");
	sounds[SND_GROWL1] = gi.soundindex ("monsters/assassin/growl1.wav");
	sounds[SND_GROWL2]=gi.soundindex("monsters/assassin/growl2.wav");
	sounds[SND_GROWL3] = gi.soundindex ("monsters/assassin/growl3.wav");
	sounds[SND_CLOAK]=gi.soundindex("monsters/assassin/cloak.wav");
	sounds[SND_DECLOAK] = gi.soundindex ("monsters/assassin/decloak.wav");

	res_info.numSounds = NUM_SOUNDS;
	res_info.sounds = sounds;

	classStatics[CID_ASSASSIN].resInfo = &res_info;
}

void assassinCheckDefense(edict_t *self, float enemydist, qboolean enemyvis, qboolean enemyinfront)
{
	if(!enemyinfront&&enemyvis&&enemydist<self->melee_range)
	{
#ifdef _DEVEL
		if(assassinCheckTeleport(self, ASS_TP_DEF))
			gi.dprintf("defense->teleport\n");
#else
		assassinCheckTeleport(self, ASS_TP_DEF);
#endif
	}
	else if(!enemyvis && self->monsterinfo.last_successful_enemy_tracking_time + 6 - skill->value < level.time)
	{
		if(irand(0, 10) > 10 - (3 * (skill->value + 1)))//hard = 90%, med is 40%, easy is 30%
		{
#ifdef _DEVEL
			gi.dprintf("Assassin trying to teleport to %s since can't find them...\n", self->classname, self->enemy->classname);
#endif
			assassinCheckTeleport(self, ASS_TP_OFF);
		}
	}
}

/*QUAKED monster_assassin (1 .5 0) (-16 -16 -32) (16 16 48) AMBUSH ASLEEP WALKING FwdJumpAmbush NoCloak NoTeleport CINEMATIC FIXED WANDER MELEE_LEAD STALK COWARD TeleportAmbush CloakAmbush SideJumpAmbush TeleportDodge

The assassin 

SPAWNFLAGS:

AMBUSH - Will not be woken up by other monsters or shots from player

ASLEEP - will not appear until triggered

WALKING - use WANDER instead

FwdJumpAmbush - will jump out front or back when triggered (depending on whether player is in front or behind him)

NoCloak - can't turn into a shadow

NoTeleport - can't use smoke grenades to trick player and teleport

CINEMATIC - puts monster into cinematic mode for scripting

FIXED - Will stand in place and attack from afar.  Never moves.

WANDER - Monster will wander around aimlessly (but follows buoys)

MELEE_LEAD - Monster will tryto cut you off when you're running and fighting him, works well if there are a few monsters in a group, half doing this, half not

STALK - Monster will only approach and attack from behind- if you're facing the monster it will just stand there.  Once the monster takes pain, however, it will stop this behaviour and attack normally

COWARD - Monster starts off in flee mode- runs away from you when woken up

TeleportAmbush - Will teleport into his origin when triggered (before triggered, is not anywhere at all, like "ASLEEP")

CloakAmbush - Start as a shadow and decloak when wakes up

SideJumpAmbush - Will jump out to left or right (depending on which side of the assassin the player is)

TeleportDodge - Can use teleporting to dodge attacks

FIELDS:

"homebuoy" - monsters will head to this buoy if they don't have an enemy ("homebuoy" should be targetname of the buoy you want them to go to)

"wakeup_target" - monsters will fire this target the first time it wakes up (only once)

"pain_target" - monsters will fire this target the first time it gets hurt (only once)

mintel - monster intelligence- this basically tells a monster how many buoys away an enemy has to be for it to give up.

melee_range - How close the player has to be, maximum, for the monster to go into melee.  If this is zero, the monster will never melee.  If it is negative, the monster will try to keep this distance from the player.  If the monster has a backup, he'll use it if too clode, otherwise, a negative value here means the monster will just stop running at the player at this distance.
	Examples:
		melee_range = 60 - monster will start swinging it player is closer than 60
		melee_range = 0 - monster will never do a mele attack
		melee_range = -100 - monster will never do a melee attack and will back away (if it has that ability) when player gets too close

missile_range - Maximum distance the player can be from the monster to be allowed to use it's ranged attack.

min_missile_range - Minimum distance the player can be from the monster to be allowed to use it's ranged attack.

bypass_missile_chance - Chance that a monster will NOT fire it's ranged attack, even when it has a clear shot.  This, in effect, will make the monster come in more often than hang back and fire.  A percentage (0 = always fire/never close in, 100 = never fire/always close in).- must be whole number

jump_chance - every time the monster has the opportunity to jump, what is the chance (out of 100) that he will... (100 = jump every time)- must be whole number

wakeup_distance - How far (max) the player can be away from the monster before it wakes up.  This just means that if the monster can see the player, at what distance should the monster actually notice him and go for him.

DEFAULTS:
mintel					= 64
melee_range				= 48
missile_range			= 1024
min_missile_range		= 64
bypass_missile_chance	= 10
jump_chance				= 100
wakeup_distance			= 1024

NOTE: A value of zero will result in defaults, if you actually want zero as the value, use -1
*/
/*-------------------------------------------------------------------------
	SP_monster_assassin
-------------------------------------------------------------------------*/
void SP_monster_assassin (edict_t *self)
{
	if(self->spawnflags & MSF_WALKING)
	{
		self->spawnflags |= MSF_WANDER;
		self->spawnflags &= ~MSF_WALKING;
	}

	if(self->spawnflags&MSF_ASS_JUMPAMBUSH||
		self->spawnflags&MSF_ASS_SIDEJUMPAMBUSH||
		self->spawnflags&MSF_ASS_STARTSHADOW)
		self->spawnflags |= MSF_AMBUSH;

	if(self->spawnflags & MSF_ASS_TPORTAMBUSH)
		self->spawnflags|=MSF_ASLEEP;

	if (!M_WalkmonsterStart(self))	// Unsuccessful initialization.
		return;
		
	self->msgHandler = DefaultMsgHandler;
	self->monsterinfo.dismember = AssassinDismember;

	if(!self->health)
		self->health = ASSASSIN_HEALTH * (skill->value + 1)/3;

	self->mass = ASSASSIN_MASS;
	self->yaw_speed = 20;

	self->movetype = PHYSICSTYPE_STEP;
	VectorClear(self->knockbackvel);
	
	self->solid=SOLID_BBOX;

	VectorCopy(STDMinsForClass[self->classID], self->mins);
	VectorCopy(STDMaxsForClass[self->classID], self->maxs);	
	self->viewheight = 40;
	
	self->isBlocked = self->bounced = AssassinBounce;

	self->s.modelindex = classStatics[CID_ASSASSIN].resInfo->modelIndex;

	self->materialtype = MAT_FLESH;

	//FIXME (somewhere: otherenemy should be more than just *one* kind
	self->monsterinfo.otherenemyname = "monster_rat";

	//set up my mood function
	MG_InitMoods(self);
	if(!irand(0,2))
		self->ai_mood_flags |= AI_MOOD_FLAG_PREDICT;
	self->cant_attack_think = assassinCheckDefense;

	self->monsterinfo.aiflags |= AI_NIGHTVISION;

	if(self->spawnflags & MSF_WANDER)
	{
		QPostMessage(self, MSG_WALK, PRI_DIRECTIVE, NULL);
	}
	else if(self->spawnflags & MSF_ASS_CINEMATIC)
	{
		self->monsterinfo.c_mode = 1;
		QPostMessage(self, MSG_C_IDLE1, PRI_DIRECTIVE, "iiige",0,0,0,NULL,NULL);
	}
	else
	{
		if(self->spawnflags&MSF_ASS_STARTSHADOW)
			assassinInitCloak (self);

		self->pre_think = assassinCloakThink;
		self->next_pre_think = level.time + FRAMETIME;

		QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
	}

	self->svflags |= SVF_WAIT_NOTSOLID;

	self->s.fmnodeinfo[MESH__KNIFES].flags |= FMNI_NO_DRAW;

	VectorCopy(self->s.origin, self->pos1);
}

