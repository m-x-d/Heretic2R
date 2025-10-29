//
// g_rope.c -- In game, animating rope.
//
// Copyright 1998 Raven Software
//

#include "g_rope.h" //mxd
#include "cl_strings.h"
#include "g_ai.h" //mxd
#include "m_chicken_anim.h"
#include "p_main.h"
#include "p_anims.h"
#include "Vector.h"
#include "Random.h"
#include "g_local.h"

#define SF_VINE				1
#define SF_CHAIN			2
#define SF_TENDRIL			4
#define SF_HANGING_CHICKEN	8

// Corresponds with the model index on the client side DIRECTLY, ie DON'T CHANGE!
enum
{
	RM_ROPE = 0,
	RM_CHAIN,
	RM_VINE,
	RM_TENDRIL
};

#pragma region ========================== Sir Nate the Tutorial Chicken ==========================

#define NATE_HEALTH	10000

enum // Instructions.
{
	NATE_INSTRUCTION_SWIPE = 0,
	NATE_INSTRUCTION_SPIN,
	NATE_INSTRUCTION_JUMP_KICK,
	NATE_INSTRUCTION_FIREBALL,

	NUM_INSTRUCTIONS
};

static qboolean UsedRightAttack(const int instruction, const edict_t* attacker, const edict_t* projectile)
{
	if (attacker->client == NULL)
		return false;

	int sequence;

	if (attacker->client->playerinfo.upperseq == ASEQ_NONE)
		sequence = attacker->client->playerinfo.lowerseq;
	else
		sequence = attacker->client->playerinfo.upperseq;

	switch (instruction)
	{
		case NATE_INSTRUCTION_SWIPE:
			return (sequence == ASEQ_WSWORD_STD1 || sequence == ASEQ_WSWORD_STD2 || sequence == ASEQ_WSWORD_STEP2 || sequence == ASEQ_WSWORD_STEP);

		case NATE_INSTRUCTION_SPIN:
			return (sequence == ASEQ_WSWORD_SPIN || sequence == ASEQ_WSWORD_SPIN2);

		case NATE_INSTRUCTION_JUMP_KICK:
			return (sequence == ASEQ_POLEVAULT2);

		case NATE_INSTRUCTION_FIREBALL:
			return (Q_stricmp(projectile->classname, "Spell_FlyingFist") == 0); //mxd. stricmp -> Q_stricmp

		default:
			return false;
	}
}

static void TutorialChickenHandleAttackSequence(edict_t* self, const edict_t* attacker, const int damage) //mxd. Removed unused 'kick' arg and return type. Named 'sir_nate_of_the_embarassingly_shortshanks_pain' in original logic.
{
	self->health = NATE_HEALTH;
	VectorClear(self->velocity);

	if (self->enemy->use != NULL || attacker->client == NULL) // Still waiting to be triggered.
		return;

	if (self->count == NUM_INSTRUCTIONS)
	{
		gi.gamemsg_centerprintf(attacker, GM_SIR_NATE_END);
		self->count++;

		return;
	}

	if (self->count > NUM_INSTRUCTIONS)
	{
		gi.msgvar_centerprintf(attacker, (short)irand(GM_SIR_NATE_HIT_AGAIN0, GM_SIR_NATE_HIT_AGAIN9), damage);
		return;
	}

	if (UsedRightAttack(self->count, attacker, self->activator))
	{
		if (self->damage_debounce_time > level.time)
			return;

		self->count++;

		if (self->count == NUM_INSTRUCTIONS)
		{
			gi.gamemsg_centerprintf(attacker, GM_SIR_NATE_FINISH);
		}
		else
		{
			gi.msgdual_centerprintf(attacker, GM_SIR_NATE_SUCCESS, (short)(self->count + GM_SIR_NATE_INSTRUCTIONS0));
			self->damage_debounce_time = level.time + 1.0f;
		}
	}
	else
	{
		gi.msgdual_centerprintf(attacker, GM_SIR_NATE_FAILURE, (short)(self->count + GM_SIR_NATE_INSTRUCTIONS0));
	}
}

static void TutorialChickenUse(edict_t* self, edict_t* other, edict_t* activator) //mxd. Named 'sir_nate_of_the_embarassingly_shortshanks_use' in original logic.
{
	if (activator->client != NULL)
	{
		gi.gamemsg_centerprintf(activator, (short)(GM_SIR_NATE_GREETING + self->dmg_radius));

		self->dmg_radius++;
		if (self->dmg_radius == 2.0f) // Last opening message.
			self->use = NULL;
	}
}

static void TutorialChickenPain(edict_t* self, edict_t* other, float kick, int damage) //mxd. Named 'hanging_chicken_pain' in original logic.
{
	self->health = NATE_HEALTH;
	self->svflags &= ~SVF_ONFIRE;

	VectorCopy(self->targetEnt->s.origin, self->s.origin);
	VectorClear(self->knockbackvel);

	self->velocity[2] = 0.0f;
	VectorCopy(self->velocity, self->targetEnt->velocity);
	VectorClear(self->velocity);

	TutorialChickenHandleAttackSequence(self, other, damage);

	int fx_flags = CEF_OWNERS_ORIGIN; //mxd
	if (damage < 100)
		fx_flags |= CEF_FLAG6;

	gi.CreateEffect(&self->s, FX_CHICKEN_EXPLODE, fx_flags, NULL, "");
	gi.sound(self, CHAN_AUTO, gi.soundindex(va("monsters/chicken/pain%i.wav", irand(1, 2))), 1.0f, ATTN_NORM, 0.0f);
}

static void TutorialChickenThink(edict_t* self) //mxd. Named 'hanging_chicken_think' in original logic.
{
	vec3_t vel_xy;
	VectorCopy(self->targetEnt->velocity, vel_xy);
	vel_xy[2] = 0.0f;

	if (VectorLength(vel_xy) > 100.0f)
	{
		self->dmg++;
		self->dmg = self->dmg & ((FRAME_cluck18 - FRAME_cluck14) - 1);
		self->s.frame = (short)(FRAME_cluck14 + self->dmg);
	}
	else
	{
		if (++self->dmg > (FRAME_wait6 - FRAME_wait1))
			self->dmg = 0;

		self->s.frame = (short)(FRAME_wait1 + self->dmg);
	}

	// Knockback.
	trace_t trace;
	gi.trace(self->s.origin, self->mins, self->maxs, self->targetEnt->s.origin, self, self->clipmask, &trace);

	if (trace.ent != NULL && AI_IsMovable(trace.ent))
	{
		vec3_t k_diff;
		VectorSubtract(self->targetEnt->s.origin, self->s.origin, k_diff);

		float force = VectorNormalize(k_diff);
		const float mass = VectorLength(trace.ent->size) * 3.0f;

		force = 600.0f * force / mass;

		// Players are not as affected by velocities when they are on the ground, so increase what players experience.
		if (trace.ent->client != NULL)
		{
			if (trace.ent->groundentity != NULL)
				force *= 4.0f;
			else
				force *= 0.25f;	// Too much knockback.
		}

		force = min(512.0f, force); // Cap this speed so it doesn't get insane.

		vec3_t k_vel;
		VectorScale(k_diff, force, k_vel);

		const float upvel = (trace.ent->client != NULL ? 30.0f : 120.0f); // Don't force players up quite so much as monsters.

		// Now if the player isn't being forced DOWN very far, let's force them UP a bit.
		if ((k_diff[2] > -0.5f || trace.ent->groundentity != NULL) && k_vel[2] < upvel && force > 30.0f)
			k_vel[2] = min(force, upvel); // Don't knock UP the player more than we do across...

		VectorAdd(trace.ent->velocity, k_vel, trace.ent->velocity);

		if (trace.ent->client != NULL) // If player, then set the player flag that will affect this.
		{
			trace.ent->client->playerinfo.flags |= PLAYER_FLAG_USE_ENT_POS;

			float knockback_time;

			// The knockback_time indicates how long this knockback affects the player.
			if (force > 200.0f && trace.ent->health > 0 && trace.ent->client->playerinfo.lowerseq != ASEQ_KNOCKDOWN && AI_IsInfrontOf(trace.ent, self))
			{
				if (self->evade_debounce_time < level.time)
				{
					gi.sound(self, CHAN_BODY, gi.soundindex("monsters/pssithra/land.wav"), 1.0f, ATTN_NORM, 0.0f);
					self->evade_debounce_time = level.time + 3.0f;
				}

				P_PlayerAnimSetLowerSeq(&trace.ent->client->playerinfo, ASEQ_KNOCKDOWN);
				P_PlayerAnimSetUpperSeq(&trace.ent->client->playerinfo, ASEQ_NONE);
				P_TurnOffPlayerEffects(&trace.ent->client->playerinfo);

				VectorMA(trace.ent->velocity, 3.0f, k_vel, trace.ent->velocity);
				knockback_time = level.time + 3.0f;
			}
			else if (force > 500)
			{
				knockback_time = level.time + 1.25f;
			}
			else
			{
				knockback_time = level.time + (force / 400.0f);
			}

			trace.ent->client->playerinfo.knockbacktime = max(knockback_time, trace.ent->client->playerinfo.knockbacktime);
		}

		if (force > 100.0f)
		{
			VectorMA(trace.endpos, -force / 5.0f, k_diff, self->targetEnt->s.origin);
			VectorScale(self->enemy->rope_end->velocity, -0.5f * force / 400.0f, self->enemy->rope_end->velocity);
		}
		else
		{
			VectorScale(self->enemy->rope_end->velocity, -0.5f, self->enemy->rope_end->velocity);
		}
	}

	VectorCopy(self->targetEnt->s.origin, self->s.origin);

	vec3_t diff;
	VectorSubtract(self->targetEnt->owner->s.origin, self->s.origin, diff);
	VectorNormalize(diff);

	vec3_t angles;
	vectoangles(diff, angles);

	// Interpolate the yaw. //TODO: why yaw only?
	const float delta_yaw = angles[YAW] - self->s.angles[YAW];
	if (fabsf(delta_yaw) > 8.0f)
		self->s.angles[YAW] -= 8.0f * Q_signf(delta_yaw);

	if (irand(0, 100) == 0)
		gi.sound(self, CHAN_AUTO, gi.soundindex(va("monsters/chicken/cluck%i.wav", irand(1, 2))), 1.0f, ATTN_NORM, 0.0f);

	VectorClear(self->velocity);

	gi.linkentity(self);
	self->nextthink = level.time + FRAMETIME; //mxd. Use define.
}

static void TutorialChickenRopeEndThink(edict_t* self) //mxd. Named 'rope_end_think2' in original logic.
{
	edict_t* grab = self->rope_end;

	vec3_t end_pos;
	VectorCopy(grab->velocity, end_pos);
	float mag = VectorNormalize(end_pos);
	VectorMA(grab->s.origin, mag * FRAMETIME, end_pos, end_pos);

	// Setup the top of the rope entity (the rope's attach point).
	vec3_t rope_top;
	VectorCopy(self->s.origin, rope_top);

	// Find the length of the end segment.
	const float grab_len = fabsf(self->maxs[2] + self->mins[2]);

	// Find the vector to the rope's point of rest.
	vec3_t end_rest;
	VectorCopy(rope_top, end_rest);
	end_rest[2] -= grab_len;

	// Find the vector towards the middle, and that distance (disregarding height).
	vec3_t end_vec;
	VectorSubtract(end_rest, grab->s.origin, end_vec);
	VectorNormalize(end_vec);
	const float end_len = vhlen(end_rest, grab->s.origin);

	// Subtract away from the rope's velocity based on that distance.
	VectorScale(end_vec, -end_len * 0.75f, end_vec);
	VectorSubtract(grab->velocity, end_vec, grab->velocity);
	VectorScale(grab->velocity, 0.99f, grab->velocity);

	// Move the rope based on the new velocity.
	vec3_t end_vel;
	VectorCopy(grab->velocity, end_vel);

	vec3_t end_dest;
	mag = VectorNormalize(end_vel);
	VectorMA(grab->s.origin, FRAMETIME * mag, end_vel, end_dest);

	// Find the angle between the top of the rope and the bottom.
	VectorSubtract(end_dest, rope_top, end_vel);
	VectorNormalize(end_vel);

	// Move the length of the rope in that direction from the top.
	VectorMA(rope_top, grab_len, end_vel, grab->s.origin);

	self->nextthink = level.time + FRAMETIME; //mxd. Use define.
}

static void TutorialChickenSpawn(edict_t* self) //mxd. Named 'spawn_hanging_chicken' in original logic.
{
	gi.setmodel(self, self->model);
	self->svflags |= SVF_NOCLIENT;

	// We only need the vertical size from the designer.
	self->maxs[0] = 32.0f;
	self->maxs[1] = 32.0f;

	self->mins[0] = -32.0f;
	self->mins[1] = -32.0f;

	self->movetype = PHYSICSTYPE_NONE;
	self->solid = SOLID_NOT;

	VectorClear(self->velocity);

	// Spawn rope end.
	edict_t* end_ent = G_Spawn();

	end_ent->movetype = PHYSICSTYPE_NONE;
	end_ent->solid = SOLID_NOT;
	end_ent->svflags |= SVF_ALWAYS_SEND;
	end_ent->owner = self;
	const short end_id = end_ent->s.number;

	VectorCopy(self->s.origin, end_ent->s.origin);
	end_ent->s.origin[2] += self->mins[2];

	VectorClear(end_ent->velocity);

	gi.linkentity(end_ent);

	self->rope_end = end_ent;

	self->bloodType = RM_ROPE;
	end_ent->bloodType = RM_ROPE;

	gi.CreatePersistantEffect(&self->s, FX_ROPE, CEF_BROADCAST, self->s.origin, "ssbvvv", end_id, end_id, RM_ROPE, end_ent->s.origin, end_ent->s.origin, end_ent->s.origin);

	self->think = TutorialChickenRopeEndThink;
	self->nextthink = level.time + FRAMETIME; //mxd. Use define.

	gi.linkentity(self);

	// Now spawn the poor chicken.
	edict_t* chicken = G_Spawn();

	gi.setmodel(chicken, "models/monsters/chicken2/tris.fm");

	// Hanging by feet.
	chicken->s.angles[PITCH] += 180.0f;
	chicken->health = NATE_HEALTH;
	VectorSet(chicken->mins, -16.0f, -16.0f, -8.0f);
	VectorSet(chicken->maxs, 16.0f, 16.0f, 16.0f);

	chicken->classname = TUTORIAL_CHICKEN_CLASSNAME;
	chicken->materialtype = MAT_FLESH;
	chicken->movetype = PHYSICSTYPE_STEP;
	chicken->gravity = 0.0f;
	chicken->solid = SOLID_BBOX;
	chicken->takedamage = DAMAGE_YES;
	chicken->clipmask = MASK_MONSTERSOLID;
	chicken->svflags = (SVF_DO_NO_IMPACT_DMG | SVF_ALLOW_AUTO_TARGET);
	chicken->s.effects = EF_CAMERA_NO_CLIP;

	VectorCopy(self->rope_end->s.origin, chicken->s.origin);

	chicken->pain = TutorialChickenPain;
	chicken->think = TutorialChickenThink;
	chicken->nextthink = level.time + FRAMETIME; //mxd. Use define.

	chicken->enemy = self;
	chicken->targetEnt = self->rope_end;
	self->targetEnt = chicken;

	gi.linkentity(chicken);
}

#pragma endregion

#pragma region ========================== obj_rope ==========================

static void ObjRopeThink(edict_t* self) //mxd. Named 'rope_think' in original logic.
{
	//FIXME!!! Do a trace down rope to make sure the rope does not clip through stuff!
	if (self->enemy == NULL)
		return;

	// See if the player has chosen this rope as the one to grab.
	if (self->enemy->targetEnt == self)
	{
		// If he's already grabbed it...
		if ((self->enemy->client->playerinfo.flags & PLAYER_FLAG_ONROPE) && !(self->enemy->client->playerinfo.flags & PLAYER_FLAG_RELEASEROPE))
		{
			if (self->count == 0)
			{
				self->count = 1;

				vec3_t rope_top;
				VectorCopy(self->s.origin, rope_top);
				rope_top[2] += self->maxs[2];

				// Create the new rope that's attached to the player.
				self->rope_grab->s.effects |= EF_ALTCLIENTFX;

				gi.CreateEffect(&self->enemy->s, FX_ROPE, CEF_BROADCAST | CEF_OWNERS_ORIGIN | CEF_FLAG6, rope_top,
					"ssbvvv",
					self->rope_grab->s.number,	// ID for the grab entity.
					self->rope_end->s.number,	// ID for the end entity.
					self->bloodType,			// Model type.
					rope_top,					// Top of the rope.
					self->rope_grab->s.origin,	// Grab's current origin (???).
					self->rope_end->s.origin);	// End's current origin	(???).
			}

			trace_t trace;
			gi.trace(self->rope_grab->s.origin, vec3_origin, vec3_origin, self->s.origin, self->enemy, MASK_SOLID, &trace);

			if (trace.fraction == 1.0f && !trace.startsolid && !trace.allsolid)
				gi.trace(self->enemy->s.origin, self->enemy->mins, self->enemy->maxs, self->rope_grab->s.origin, self->enemy, MASK_PLAYERSOLID, &trace);

			// If the rope's movement is clear, move the player and the rope.
			if (trace.fraction == 1.0f && !trace.startsolid && !trace.allsolid)
			{
				VectorCopy(self->rope_grab->s.origin, self->enemy->s.origin);
			}
			else
			{
				// Otherwise stop the player and the rope from entering a solid brush.
				VectorScale(self->rope_grab->velocity, -0.5f, self->rope_grab->velocity);
				VectorCopy(self->enemy->s.origin, self->rope_grab->s.origin);
			}
		}
		else
		{
			self->count = 0;
			self->rope_grab->s.effects &= ~EF_ALTCLIENTFX;
		}
	}
	else
	{
		// This grabber is invalid, clear it.
		self->enemy = NULL;
	}
}

static void ObjRopeEndThink(edict_t* self) //mxd. Named 'rope_end_think' in original logic.
{
	edict_t* grab = self->rope_end;

	// Setup the top of the rope entity (the rope's attach point).
	vec3_t rope_top;
	VectorCopy(self->rope_grab->s.origin, rope_top);

	// Find the length of the end segment.
	const float grab_len = fabsf(self->maxs[2] + self->mins[2]) - self->rope_grab->rope_player_z;

	// Find the vector to the rope's point of rest.
	vec3_t end_rest;
	VectorCopy(rope_top, end_rest);
	end_rest[2] -= grab_len;

	// Find the vector towards the middle, and that distance (disregarding height).
	vec3_t end_vec;
	VectorSubtract(end_rest, grab->s.origin, end_vec);
	VectorNormalize(end_vec);
	const float end_len = vhlen(end_rest, grab->s.origin);

	// Subtract away from the rope's velocity based on that distance.
	VectorScale(end_vec, -end_len, end_vec);
	VectorSubtract(grab->velocity, end_vec, grab->velocity);
	VectorScale(grab->velocity, 0.95f, grab->velocity);

	// Move the rope based on the new velocity.
	vec3_t end_vel;
	VectorCopy(grab->velocity, end_vel);

	vec3_t end_dest;
	const float mag = VectorNormalize(end_vel);
	VectorMA(grab->s.origin, FRAMETIME * mag, end_vel, end_dest);

	// Find the angle between the top of the rope and the bottom.
	VectorSubtract(end_dest, rope_top, end_vel);
	VectorNormalize(end_vel);

	// Move the length of the rope in that direction from the top.
	VectorMA(rope_top, grab_len, end_vel, grab->s.origin);
}

static void ObjRopeSwayThink(edict_t* self) //mxd. Named 'rope_sway' in original logic.
{
	edict_t* grab = self->rope_grab;

	// Setup the top of the rope entity (the rope's attach point).
	vec3_t rope_top;
	VectorCopy(self->s.origin, rope_top);
	rope_top[2] += self->maxs[2];

	if (fabsf(grab->velocity[0]) < 0.13f && fabsf(grab->velocity[1]) < 0.13f)
	{
		// The rope isn't moving enough to run all the math, so just make it sway a little.
		vec3_t rope_rest;
		VectorCopy(self->s.origin, rope_rest);

		rope_rest[0] += sinf(level.time * 2.0f) * 1.25f;
		rope_rest[1] += cosf(level.time * 2.0f) * 1.75f;
		rope_rest[2] += self->mins[2];

		VectorCopy(rope_rest, self->pos1);
		VectorCopy(rope_rest, self->slave->s.origin);

		vec3_t v_rope;
		VectorSubtract(rope_rest, rope_top, v_rope);
		VectorNormalize(v_rope);

		vec3_t grab_end;
		VectorMA(rope_top, grab->rope_player_z, v_rope, grab_end);
		VectorCopy(grab_end, grab->s.origin);

		ObjRopeThink(self);

		self->think = ObjRopeSwayThink;
		self->nextthink = level.time + FRAMETIME;

		return;
	}

	// Find the length of the grab.
	vec3_t v_grab;
	VectorSubtract(rope_top, grab->s.origin, v_grab);
	const float grab_len = VectorLength(v_grab);

	// Find the vector to the rope's point of rest.
	vec3_t rope_rest;
	VectorCopy(self->s.origin, rope_rest);
	rope_rest[2] -= grab_len;

	// Find the vector towards the middle, and that distance (disregarding height).
	vec3_t v_rope;
	VectorSubtract(rope_rest, grab->s.origin, v_rope);
	VectorNormalize(v_rope);
	const float dist = vhlen(rope_rest, grab->s.origin);

	//NOTE: There's a fine line between a real pendulum motion here that comes to rest,
	// and a chaotic one that builds too much and runs amock... So don't monkey with the values in here... ok?  --jweier

	// Subtract away from the rope's velocity based on that distance.
	VectorScale(v_rope, -dist, v_rope);
	VectorSubtract(grab->velocity, v_rope, grab->velocity);
	VectorScale(grab->velocity, 0.95f, grab->velocity);

	// Move the rope based on the new velocity.
	vec3_t v_dir;
	VectorCopy(grab->velocity, v_dir);

	vec3_t v_dest;
	const float mag = VectorNormalize(v_dir);
	VectorMA(grab->s.origin, FRAMETIME * mag, v_dir, v_dest);

	// Move the length of the rope in that direction from the top.
	VectorSubtract(v_dest, rope_top, v_rope);
	VectorNormalize(v_rope);

	vec3_t grab_end;
	VectorMA(rope_top, grab->rope_player_z, v_rope, grab_end);

	// You're done.
	VectorCopy(grab_end, grab->s.origin);
	VectorCopy(grab_end, self->pos1);

	// Make the end of the rope come to the end.
	ObjRopeEndThink(self);
	ObjRopeThink(self);

	self->think = ObjRopeSwayThink;
	self->nextthink = level.time + FRAMETIME;
}

static void ObjRopeTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf) //mxd. Named 'rope_touch' in original logic.
{
	if (Q_stricmp(other->classname, "player") == 0) //mxd. stricmp -> Q_stricmp
	{
		// If the player is already on a rope, forget him as a valid grabber
		if (other->targetEnt != NULL && (other->client->playerinfo.flags & PLAYER_FLAG_ONROPE))
			return;

		// If we've got a player on the rope, and this guy isn't it, then don't let him grab.
		if (self->enemy != NULL && self->enemy != other)
			return;

		self->enemy = other;
		self->rope_player_z = other->s.origin[2];
		other->targetEnt = self;
	}
}

// QUAKED obj_rope (.3 .2 .8) ? VINE CHAIN TENDRIL HANGING_CHICKEN
// A rope to climb or swing (default to rope model).
// Place the top of the entity at the top of the rope, and drag the bottom of the box to the end of the rope.
// THE ENTITY MUST HAVE AN ORIGIN BRUSH.
// Spawnflags:
// ROPE		- Default.
// VINE		- Use a vine model.
// CHAIN	- Use a chain model.
// TENDRIL	- Use a tendril model.
void SP_obj_rope(edict_t* self)
{
	if (self->spawnflags & SF_HANGING_CHICKEN)
	{
		if (self->targetname != NULL)
			self->use = TutorialChickenUse;
		else
			gi.dprintf("Chicken on a Rope with no targetname...\n");

		TutorialChickenSpawn(self);
		return;
	}

	gi.setmodel(self, self->model);
	self->svflags |= SVF_NOCLIENT;

	// We only need the vertical size from the designer.
	self->maxs[0] = 32.0f;
	self->maxs[1] = 32.0f;

	self->mins[0] = -32.0f;
	self->mins[1] = -32.0f;

	self->movetype = PHYSICSTYPE_NONE;
	self->solid = SOLID_TRIGGER;
	self->touch = ObjRopeTouch;

	VectorClear(self->velocity);

	// Create rope end entity.
	edict_t* end_ent = G_Spawn();

	end_ent->movetype = PHYSICSTYPE_NONE;
	end_ent->solid = SOLID_NOT;
	end_ent->svflags |= SVF_ALWAYS_SEND;
	const short end_id = end_ent->s.number;

	VectorCopy(self->s.origin, end_ent->s.origin);
	end_ent->s.origin[2] += self->mins[2];

	VectorClear(end_ent->velocity);

	gi.linkentity(end_ent);

	self->rope_end = end_ent;

	// Create rope grab entity.
	edict_t* grab_ent = G_Spawn();

	grab_ent->movetype = PHYSICSTYPE_NONE;
	grab_ent->solid = SOLID_NOT;
	grab_ent->svflags |= SVF_ALWAYS_SEND;
	const short grab_id = grab_ent->s.number;

	VectorClear(grab_ent->velocity);
	VectorCopy(self->s.origin, grab_ent->s.origin);
	grab_ent->s.origin[2] += self->maxs[2] + 4.0f;

	gi.linkentity(grab_ent);

	self->rope_grab = grab_ent;

	byte model_type;
	if (self->spawnflags & SF_CHAIN)
		model_type = RM_CHAIN;
	else if (self->spawnflags & SF_VINE)
		model_type = RM_VINE;
	else if (self->spawnflags & SF_TENDRIL)
		model_type = RM_TENDRIL;
	else
		model_type = RM_ROPE;

	self->bloodType = model_type;
	grab_ent->bloodType = model_type;
	end_ent->bloodType = model_type;

	ObjRopeSwayThink(self);

	gi.CreatePersistantEffect(&self->s, FX_ROPE, CEF_BROADCAST, self->s.origin, "ssbvvv", grab_id, end_id, model_type, self->s.origin, grab_ent->s.origin, end_ent->s.origin);

	self->think = ObjRopeSwayThink;
	self->nextthink = level.time + 1.0f;

	gi.linkentity(self);
}

#pragma endregion