//
// p_funcs.c
//
// Copyright 1998 Raven Software
//

#include "p_funcs.h"
#include "p_anims.h"
#include "p_client.h" //mxd
#include "p_main.h"
#include "p_morph.h" //mxd
#include "p_teleport.h" //mxd
#include "g_ai.h" //mxd
#include "g_combat.h" //mxd
#include "g_playstats.h"
#include "g_Skeletons.h"
#include "g_weapon.h"
#include "spl_shield.h" //mxd
#include "FX.h"
#include "Random.h"
#include "Vector.h"
#include "g_local.h"

entity_state_t* G_GetEntityStatePtr(edict_t* entity)
{
	return &entity->s;
}

static void PlayerClimbSound(playerinfo_t* info, char* name)
{
	if (info->isclient)
		info->CL_Sound(SND_PRED_ID53, info->origin, CHAN_VOICE, name, 0.75f, ATTN_NORM, 0.0f);
	else
		info->G_Sound(SND_PRED_ID53, info->leveltime, info->self, CHAN_VOICE, info->G_SoundIndex(name), 0.75f, ATTN_NORM, 0.0f);
}

void G_PlayerActionCheckRopeMove(playerinfo_t* info)
{
	edict_t* player = info->self; //mxd

	if (info->seqcmd[ACMDL_JUMP])
	{
		info->flags &= ~PLAYER_FLAG_ONROPE;
		VectorCopy(player->targetEnt->rope_grab->velocity, info->velocity);
		const float threshold = VectorLengthSquared(info->velocity);

		if (threshold < 300 * 300)
		{
			vec3_t fwd;
			AngleVectors(info->aimangles, fwd, NULL, NULL);
			VectorMA(info->velocity, 200.0f, fwd, info->velocity);
		}
		else
		{
			VectorScale(info->velocity, 0.75f, info->velocity);
		}

		info->velocity[2] = 250.0f;
		info->flags |= PLAYER_FLAG_USE_ENT_POS;

		player->monsterinfo.jump_time = info->leveltime + 2.0f;

		player->targetEnt->rope_grab->s.effects &= ~EF_ALTCLIENTFX;
		player->targetEnt->enemy = NULL;
		player->targetEnt = NULL;

		P_PlayerAnimSetUpperSeq(info, ASEQ_NONE);
		P_PlayerAnimSetLowerSeq(info, ASEQ_JUMPFWD);

		return;
	}

	if (info->seqcmd[ACMDL_STRAFE_L] || info->seqcmd[ACMDL_STRAFE_R])
	{
		vec3_t right;
		AngleVectors(info->angles, NULL, right, NULL);

		const float scale = 32.0f * (info->seqcmd[ACMDL_STRAFE_L] ? -1.0f : 1.0f); //mxd
		VectorScale(right, scale, right);
		VectorAdd(player->targetEnt->rope_grab->velocity, right, player->targetEnt->rope_grab->velocity);

		switch (info->lowerseq)
		{
			case ASEQ_CLIMB_ON:
			case ASEQ_CLIMB_HOLD_L:
			case ASEQ_CLIMB_SETTLE_L:
			case ASEQ_CLIMB_HOLD_R:
			case ASEQ_CLIMB_SETTLE_R:
				PlayerClimbSound(info, (irand(0, 1) ? "player/ropeto.wav" : "player/ropefro.wav"));
				break;
		}
	}
}

int G_BranchLwrClimbing(playerinfo_t* info)
{
	assert(info != NULL);
	edict_t* player = info->self; //mxd

	// Process attack command.
	if (info->seqcmd[ACMDU_ATTACK])
	{
		if (player->targetEnt->rope_grab->monsterinfo.jump_time < level.time)
		{
			player->targetEnt->rope_grab->monsterinfo.jump_time = level.time + 2.0f;

			vec3_t forward;
			AngleVectors(info->angles, forward, NULL, NULL);
			VectorMA(forward, 400.0f, forward, forward);
			VectorAdd(player->targetEnt->rope_grab->velocity, forward, player->targetEnt->rope_grab->velocity);
		}
	}

	// Process strafe commands.
	if (info->seqcmd[ACMDL_STRAFE_L])
	{
		if (player->targetEnt->rope_grab->monsterinfo.search_time < level.time)
		{
			player->targetEnt->rope_grab->monsterinfo.search_time = level.time + 2.0f;

			vec3_t right;
			AngleVectors(info->angles, NULL, right, NULL);
			VectorScale(right, -64.0f, right);
			VectorAdd(player->targetEnt->rope_grab->velocity, right, player->targetEnt->rope_grab->velocity);

			switch (info->lowerseq)
			{
				case ASEQ_CLIMB_HOLD_R:
				case ASEQ_CLIMB_SETTLE_R:
					PlayerClimbSound(info, (irand(0, 1) ? "player/ropeto.wav" : "player/ropefro.wav"));
					return ASEQ_CLIMB_HOLD_R;

				case ASEQ_CLIMB_ON:
				case ASEQ_CLIMB_HOLD_L:
				case ASEQ_CLIMB_SETTLE_L:
					PlayerClimbSound(info, (irand(0, 1) ? "player/ropeto.wav" : "player/ropefro.wav"));
					return ASEQ_CLIMB_HOLD_L;
			}
		}
	}
	else if (info->seqcmd[ACMDL_STRAFE_R])
	{
		if (player->targetEnt->rope_grab->monsterinfo.flee_finished < level.time)
		{
			player->targetEnt->rope_grab->monsterinfo.flee_finished = level.time + 2.0f;

			vec3_t right;
			AngleVectors(info->angles, NULL, right, NULL);
			VectorScale(right, 64.0f, right);
			VectorAdd(player->targetEnt->rope_grab->velocity, right, player->targetEnt->rope_grab->velocity);

			switch (info->lowerseq)
			{
				case ASEQ_CLIMB_HOLD_R:
				case ASEQ_CLIMB_SETTLE_R:
					PlayerClimbSound(info, (irand(0, 1) ? "player/ropeto.wav" : "player/ropefro.wav"));
					return ASEQ_CLIMB_HOLD_R;

				case ASEQ_CLIMB_ON:
				case ASEQ_CLIMB_HOLD_L:
				case ASEQ_CLIMB_SETTLE_L:
					PlayerClimbSound(info, (irand(0, 1) ? "player/ropeto.wav" : "player/ropefro.wav"));
					return ASEQ_CLIMB_HOLD_L;
			}
		}
	}

	// Process movement commands.
	if (info->seqcmd[ACMDL_FWD])
	{
		vec3_t end_point;
		VectorCopy(info->origin, end_point);
		end_point[2] += 32.0f;

		vec3_t player_min;
		vec3_t player_max;
		VectorCopy(info->mins, player_min);
		VectorCopy(info->maxs, player_max);

		trace_t trace;
		info->G_Trace(info->origin, player_min, player_max, end_point, info->self, MASK_PLAYERSOLID, &trace);

		if (trace.fraction < 1.0f)
		{
			// We bumped into something.
			player->targetEnt->rope_grab->viewheight = (int)player->targetEnt->rope_grab->accel;

			switch (info->lowerseq)
			{
				case ASEQ_CLIMB_HOLD_R:
				case ASEQ_CLIMB_SETTLE_R:
					return ASEQ_CLIMB_HOLD_R;

				case ASEQ_CLIMB_ON:
				case ASEQ_CLIMB_HOLD_L:
				case ASEQ_CLIMB_SETTLE_L:
					return ASEQ_CLIMB_HOLD_L;

				case ASEQ_CLIMB_UP_L:
				case ASEQ_CLIMB_DOWN_R:
				case ASEQ_CLIMB_UP_START_L:
				case ASEQ_CLIMB_DOWN_START_L:
					PlayerClimbSound(info, (irand(0, 1) ? "player/ropeto.wav" : "player/ropefro.wav"));
					return ASEQ_CLIMB_SETTLE_R;

				case ASEQ_CLIMB_UP_R:
				case ASEQ_CLIMB_DOWN_L:
				case ASEQ_CLIMB_UP_START_R:
				case ASEQ_CLIMB_DOWN_START_R:
					PlayerClimbSound(info, (irand(0, 1) ? "player/ropeto.wav" : "player/ropefro.wav"));
					return ASEQ_CLIMB_SETTLE_L;
			}
		}
		else
		{
			const int chance = irand(1, 4);

			switch (info->lowerseq)
			{
				case ASEQ_CLIMB_UP_R:
				case ASEQ_CLIMB_UP_START_R:
					if (chance < 3)
						PlayerClimbSound(info, va("player/ropeclimb%i.wav", chance));
					return ASEQ_CLIMB_UP_L;

				case ASEQ_CLIMB_UP_L:
				case ASEQ_CLIMB_UP_START_L:
					if (chance < 3)
						PlayerClimbSound(info, va("player/ropeclimb%i.wav", chance));
					return ASEQ_CLIMB_UP_R;

				case ASEQ_CLIMB_ON:
				case ASEQ_CLIMB_DOWN_L:
				case ASEQ_CLIMB_HOLD_L:
				case ASEQ_CLIMB_SETTLE_L:
				case ASEQ_CLIMB_DOWN_START_L:
					if (chance < 3)
						PlayerClimbSound(info, va("player/ropeclimb%i.wav", chance));
					return ASEQ_CLIMB_UP_START_L;

				case ASEQ_CLIMB_DOWN_R:
				case ASEQ_CLIMB_HOLD_R:
				case ASEQ_CLIMB_SETTLE_R:
				case ASEQ_CLIMB_DOWN_START_R:
					if (chance < 3)
						PlayerClimbSound(info, va("player/ropeclimb%i.wav", chance));
					return ASEQ_CLIMB_UP_START_R;
			}
		}
	}
	else if (info->seqcmd[ACMDL_BACK])
	{
		vec3_t end_point;
		VectorCopy(info->origin, end_point);
		end_point[2] -= 32.0f;

		vec3_t player_min;
		vec3_t player_max;
		VectorCopy(info->mins, player_min);
		VectorCopy(info->maxs, player_max);

		trace_t trace;
		info->G_Trace(info->origin, player_min, player_max, end_point, info->self, MASK_PLAYERSOLID, &trace);

		if (trace.fraction < 1.0f || trace.endpos[2] < player->targetEnt->rope_end->s.origin[2])
		{
			// We bumped into something or have come to the end of the rope.
			player->targetEnt->rope_grab->viewheight = (int)player->targetEnt->rope_grab->accel;

			switch (info->lowerseq)
			{

				case ASEQ_CLIMB_HOLD_R:
				case ASEQ_CLIMB_SETTLE_R:
					return ASEQ_CLIMB_HOLD_R;

				case ASEQ_CLIMB_ON:
				case ASEQ_CLIMB_HOLD_L:
				case ASEQ_CLIMB_SETTLE_L:
					return ASEQ_CLIMB_HOLD_L;

				case ASEQ_CLIMB_UP_L:
				case ASEQ_CLIMB_DOWN_R:
				case ASEQ_CLIMB_UP_START_L:
				case ASEQ_CLIMB_DOWN_START_L:
					PlayerClimbSound(info, (irand(0, 1) ? "player/ropeto.wav" : "player/ropefro.wav"));
					return ASEQ_CLIMB_SETTLE_R;

				case ASEQ_CLIMB_UP_R:
				case ASEQ_CLIMB_DOWN_L:
				case ASEQ_CLIMB_UP_START_R:
				case ASEQ_CLIMB_DOWN_START_R:
					PlayerClimbSound(info, (irand(0, 1) ? "player/ropeto.wav" : "player/ropefro.wav"));
					return ASEQ_CLIMB_SETTLE_L;
			}
		}
		else
		{
			const int chance = irand(1, 4);

			switch (info->lowerseq)
			{
				case ASEQ_CLIMB_DOWN_R:
				case ASEQ_CLIMB_DOWN_START_R:
					if (chance < 3)
						PlayerClimbSound(info, va("player/ropeclimb%i.wav", chance));
					return ASEQ_CLIMB_DOWN_L;

				case ASEQ_CLIMB_DOWN_L:
				case ASEQ_CLIMB_DOWN_START_L:
					if (chance < 3)
						PlayerClimbSound(info, va("player/ropeclimb%i.wav", chance));
					return ASEQ_CLIMB_DOWN_R;

				case ASEQ_CLIMB_ON:
				case ASEQ_CLIMB_UP_L:
				case ASEQ_CLIMB_HOLD_R:
				case ASEQ_CLIMB_SETTLE_L:
				case ASEQ_CLIMB_UP_START_L:
					if (chance < 3)
						PlayerClimbSound(info, va("player/ropeclimb%i.wav", chance));
					return ASEQ_CLIMB_DOWN_START_L;

				case ASEQ_CLIMB_HOLD_L:
				case ASEQ_CLIMB_UP_R:
				case ASEQ_CLIMB_SETTLE_R:
				case ASEQ_CLIMB_UP_START_R:
					if (chance < 3)
						PlayerClimbSound(info, va("player/ropeclimb%i.wav", chance));
					return ASEQ_CLIMB_DOWN_START_R;
			}
		}
	}
	else if (info->seqcmd[ACMDL_JUMP])
	{
		info->flags &= ~PLAYER_FLAG_ONROPE;
		VectorCopy(player->targetEnt->rope_grab->velocity, info->velocity);
		info->velocity[2] = 150.0f;
		info->flags |= PLAYER_FLAG_USE_ENT_POS;

		player->monsterinfo.jump_time = info->leveltime + 2.0f;

		player->targetEnt->rope_grab->s.effects &= ~EF_ALTCLIENTFX;
		player->targetEnt->enemy = NULL;
		player->targetEnt = NULL;

		P_PlayerAnimSetUpperSeq(info, ASEQ_NONE);

		return ASEQ_JUMPFWD;
	}
	else
	{
		switch (info->lowerseq)
		{
			case ASEQ_CLIMB_HOLD_R:
			case ASEQ_CLIMB_SETTLE_R:
				return ASEQ_CLIMB_HOLD_R;

			case ASEQ_CLIMB_ON:
			case ASEQ_CLIMB_HOLD_L:
			case ASEQ_CLIMB_SETTLE_L:
				return ASEQ_CLIMB_HOLD_L;

			case ASEQ_CLIMB_UP_L:
			case ASEQ_CLIMB_DOWN_R:
			case ASEQ_CLIMB_UP_START_L:
			case ASEQ_CLIMB_DOWN_START_L:
				PlayerClimbSound(info, (irand(0, 1) ? "player/ropeto.wav" : "player/ropefro.wav"));
				return ASEQ_CLIMB_SETTLE_R;

			case ASEQ_CLIMB_UP_R:
			case ASEQ_CLIMB_DOWN_L:
			case ASEQ_CLIMB_UP_START_R:
			case ASEQ_CLIMB_DOWN_START_R:
				PlayerClimbSound(info, (irand(0, 1) ? "player/ropeto.wav" : "player/ropefro.wav"));
				return ASEQ_CLIMB_SETTLE_L;
		}
	}

	return ASEQ_NONE;
}

qboolean G_PlayerActionCheckRopeGrab(playerinfo_t* info, float stomp_org) //TODO: remove 'stomp_org' arg?
{
	assert(info != NULL);

	const edict_t* rope = (edict_t*)info->targetEnt;

	// Get the position of the rope's end.
	vec3_t rope_end;
	VectorCopy(rope->rope_end->s.origin, rope_end);

	vec3_t rope_top;
	VectorCopy(rope->s.origin, rope_top);
	rope_top[2] += rope->maxs[2];

	// If we're above the rope then we can't grab it.
	if (info->origin[2] > rope_top[2])
		return false;

	vec3_t vec;
	VectorSubtract(info->origin, rope_top, vec);
	const float len = VectorLength(vec);

	VectorSubtract(rope_end, rope_top, vec);
	float dist = VectorNormalize(vec);

	// Player is below the rope's length
	if (len > dist)
		return false;

	vec3_t rope_check;
	VectorMA(rope_top, len, vec, rope_check);

	dist = vhlen(info->origin, rope_check);
	const float check_dist = ((info->groundentity != NULL) ? 48.0f : 64.0f);

	if (dist >= check_dist)
		return false;

	// Player is getting on the rope for the first time.
	if (!(info->flags & PLAYER_FLAG_ONROPE))
	{
		VectorCopy(info->velocity, rope->rope_grab->velocity);
		VectorScale(rope->rope_grab->velocity, 2.0f, rope->rope_grab->velocity);
		VectorClear(info->velocity);
		VectorCopy(info->origin, rope->rope_grab->s.origin);
		VectorSubtract(info->origin, rope_top, vec);
		rope->rope_grab->viewheight = (int)(VectorLength(vec));
	}
	else
	{
		trace_t trace;
		info->G_Trace(info->origin, info->mins, info->maxs, rope->rope_grab->s.origin, info->self, MASK_PLAYERSOLID, &trace);

		if (trace.fraction < 1.0f || trace.startsolid || trace.allsolid)
			return false;

		VectorCopy(rope->rope_grab->s.origin, info->origin);
	}

	return true;
}

void G_PlayerClimbingMoveFunc(playerinfo_t* info, const float height, float var2, float var3) //TODO: remove unused var2 and var3 args?
{
	if (info->isclient) // Ignore client-side playerinfo_t.
		return;

	// Pull Corvus into the rope.
	G_PlayerActionCheckRopeGrab(info, 1);

	if (info->targetEnt != NULL)
	{
		const edict_t* rope = info->targetEnt; //mxd

		// Update the rope's information about the player's position.
		rope->rope_grab->accel = (float)rope->rope_grab->viewheight;
		rope->rope_grab->viewheight -= (int)height;
	}
}

qboolean G_PlayerActionCheckPuzzleGrab(playerinfo_t* info)
{
	vec3_t forward;
	const vec3_t player_facing = { 0.0f, info->angles[1], 0.0f };
	AngleVectors(player_facing, forward, NULL, NULL);

	vec3_t end_point;
	VectorMA(info->origin, 32.0f, forward, end_point);

	trace_t tr;
	gi.trace(info->origin, info->mins, info->maxs, end_point, (edict_t*)info->self, MASK_PLAYERSOLID, &tr);

	if (tr.fraction == 1.0f || tr.ent == NULL || tr.ent->item == NULL || tr.ent->item->flags != IT_PUZZLE)
		return false;

	info->targetEnt = tr.ent;

	return true;
}

void G_PlayerActionTakePuzzle(const playerinfo_t* info)
{
	edict_t* item = info->self; //mxd

	if (item->targetEnt->use != NULL)
		item->targetEnt->use(item->targetEnt, item, item);
}

qboolean G_PlayerActionUsePuzzle(const playerinfo_t* info)
{
	edict_t* item = info->self; //mxd

	if (item->target_ent != NULL && strcmp(item->target_ent->classname, "trigger_playerusepuzzle") == 0)
	{
		G_UseTargets(item->target_ent, item);
		return true;
	}

	return false;
}

qboolean G_PlayerActionCheckPushPull_Ent(const edict_t* ent)
{
	return (strcmp(ent->classname, "func_train") == 0 && (ent->spawnflags & 32));
}

void G_PlayerActionMoveItem(const playerinfo_t* info, const float distance) //TODO: unused
{
	vec3_t push_dir;
	const vec3_t player_facing = { 0.0f, info->angles[1], 0.0f };
	AngleVectors(player_facing, push_dir, NULL, NULL);

	edict_t* item = info->target_ent; //mxd
	VectorScale(push_dir, distance, item->velocity);

	edict_t* self = info->self; //mxd
	self->target_ent->nextthink = level.time + 2.0f * FRAMETIME;
	self->target_ent->target_ent = self;
}

qboolean G_PlayerActionCheckPushButton(const playerinfo_t* info)
{
#define MAX_PUSH_BUTTON_RANGE	80.0f

	// Are you near a button?
	const edict_t* self = info->self; //mxd
	if (self->target == NULL)
		return false; // No button, so return.

	// A button is nearby, so look to see if it's in reach.
	edict_t* button = NULL;
	button = G_Find(button, FOFS(targetname), self->target);

	if (button == NULL || button->classID != CID_BUTTON)
		return false;

	// Get center of button
	vec3_t v;
	VectorAverage(button->mins, button->maxs, v);

	// Get distance from player origin to the center of button.
	Vec3SubtractAssign(info->origin, v);
	const float range = VectorLength(v);

	if (range < MAX_PUSH_BUTTON_RANGE)
	{
		vec3_t dir;
		VectorCopy(self->client->playerinfo.aimangles, dir);
		dir[PITCH] = 0.0f;

		vec3_t forward;
		AngleVectors(dir, forward, NULL, NULL);
		VectorNormalize(v);

		// Both these vectors are normalized so result is cos of angle.
		return DotProduct(v, forward) > 0.75f; // 41 degree range either way.
	}

	return false;
}

void G_PlayerActionPushButton(const playerinfo_t* info) //mxd. Same logic as G_PlayerActionPushLever().
{
	edict_t* self = info->self; //mxd
	G_UseTargets(self, self);
}

qboolean G_PlayerActionCheckPushLever(const playerinfo_t* info)
{
#define MAX_PUSH_LEVER_RANGE	80.0f

	// Are you near a lever?
	const edict_t* self = info->self;
	if (self->target == NULL)
		return false; // No level, so return.

	// A level is nearby, so look to see if it's in reach.
	edict_t* lever = NULL;
	lever = G_Find(lever, FOFS(targetname), self->target);

	if (lever == NULL || lever->classID != CID_LEVER)
		return false;

	// Get distance from player origin to center of lever
	vec3_t v;
	VectorSubtract(info->origin, lever->s.origin, v);
	const float range = VectorLength(v);

	if (range < MAX_PUSH_LEVER_RANGE)
	{
		vec3_t dir;
		VectorCopy(self->client->playerinfo.aimangles, dir);
		dir[PITCH] = 0.0f;

		vec3_t forward;
		AngleVectors(dir, forward, NULL, NULL);
		VectorSubtract(lever->s.origin, self->s.origin, v);
		VectorNormalize(v);

		// Both these vectors are normalized so result is cos of angle.
		return DotProduct(v, forward) > 0.7f; // 41 degree range either way.
	}

	return false;
}

void G_PlayerActionPushLever(const playerinfo_t* info) //mxd. Same logic as G_PlayerActionPushButton().
{
	edict_t* self = info->self; //mxd
	G_UseTargets(self, self);
}

qboolean G_HandleTeleport(const playerinfo_t* info)
{
	// Are we teleporting or morphing?
	if (!(info->flags & (PLAYER_FLAG_TELEPORT | PLAYER_FLAG_MORPHING)))
		return false;

	edict_t* self = info->self; //mxd

	// Are we doing de-materialize or...
	if (self->client->tele_dest[0] != -1.0f)
	{
		// Are we done de-materializing? Or still fading?
		if (self->client->tele_count-- > 0)
			self->s.color.a -= TELE_FADE_OUT;
		else if (info->flags & PLAYER_FLAG_TELEPORT) // We have finished dematerializing, let's move the character.
			PerformPlayerTeleport(self);
		else if (info->edictflags & FL_CHICKEN)
			MorphChickenToPlayerEnd(self); // We're set as a chicken.
		else
			MorphPlayerToChickenEnd(self);

		return true;
	}

	// Are we done dematerializing? Or still fading?
	if (self->client->tele_count-- > 0)
		self->s.color.a += TELE_FADE;
	else if (info->flags & PLAYER_FLAG_TELEPORT) // We are done re-materializing, let's kill all this BS and get back to the game.
		CleanUpPlayerTeleport(self);
	else
		CleanUpPlayerMorph(self);

	return !DEATHMATCH;
}

void PlayerChickenDeath(edict_t* self)
{
	//gi.sound(self, CHAN_BODY, sounds[SND_GIB], 1, ATTN_NORM, 0); //TODO: play "Monsters/chicken/die.wav"?
	self->dead_state = DEAD_DEAD;
	self->client->playerinfo.deadflag = DEAD_DEAD;
	gi.CreateEffect(&self->s, FX_CHICKEN_EXPLODE, CEF_OWNERS_ORIGIN, NULL, "");

	// Fix that respawning bug.
	self->morph_timer = (int)level.time - 1;

	// Reset our thinking.
	self->think = self->oldthink;
	self->nextthink = level.time + FRAMETIME;

	self->model = "models/player/corvette/tris.fm"; //TODO: there's no such model!
	self->pain = PlayerPain;

	// Reset our skins.
	self->s.skinnum = 0; // The skinnum stores the skin now.
	self->s.clientnum = (short)(self - g_edicts - 1);

	self->s.modelindex = 255; // Will use the skin specified model.
	self->s.frame = 0;

	// Turn our skeleton back on.
	self->flags &= ~FL_CHICKEN;
	self->s.skeletalType = SKEL_CORVUS;
	self->s.effects = (EF_SWAPFRAME | EF_JOINTED);
	self->s.renderfx &= ~RF_IGNORE_REFS;

	// Reset our animations.
	P_PlayerAnimReset(&self->client->playerinfo);
}

// Set the player model's joint angles.
//mxd. Similar to SK_SetJointAngles() in cl_skeletons.c.
void G_SetJointAngles(const playerinfo_t* info)
{
	const edict_t* self = (edict_t*)info->self;

	SetJointAngVel(self->s.rootJoint + CORVUS_HEAD, PITCH, info->targetjointangles[PITCH], ANGLE_45);
	SetJointAngVel(self->s.rootJoint + CORVUS_HEAD, ROLL,  info->targetjointangles[YAW],   ANGLE_45);

	if (!info->headjointonly)
	{
		SetJointAngVel(self->s.rootJoint + CORVUS_UPPERBACK, PITCH, info->targetjointangles[PITCH], ANGLE_45);
		SetJointAngVel(self->s.rootJoint + CORVUS_LOWERBACK, PITCH, info->targetjointangles[PITCH], ANGLE_45);
		SetJointAngVel(self->s.rootJoint + CORVUS_UPPERBACK, ROLL,  info->targetjointangles[YAW],   ANGLE_45);
		SetJointAngVel(self->s.rootJoint + CORVUS_LOWERBACK, ROLL,  info->targetjointangles[YAW],   ANGLE_45);
	}
	else
	{
		SetJointAngVel(self->s.rootJoint + CORVUS_UPPERBACK, PITCH, 0.0f, ANGLE_45);
		SetJointAngVel(self->s.rootJoint + CORVUS_LOWERBACK, PITCH, 0.0f, ANGLE_45);
		SetJointAngVel(self->s.rootJoint + CORVUS_UPPERBACK, ROLL,  0.0f, ANGLE_45);
		SetJointAngVel(self->s.rootJoint + CORVUS_LOWERBACK, ROLL,  0.0f, ANGLE_45);
	}
}

// Reset the player model's joint angles.
void G_ResetJointAngles(const playerinfo_t* info)
{
	const edict_t* self = (edict_t*)info->self;

	SetJointAngVel(self->s.rootJoint + CORVUS_HEAD,      PITCH, 0.0f, ANGLE_45);
	SetJointAngVel(self->s.rootJoint + CORVUS_UPPERBACK, PITCH, 0.0f, ANGLE_45);
	SetJointAngVel(self->s.rootJoint + CORVUS_LOWERBACK, PITCH, 0.0f, ANGLE_45);

	SetJointAngVel(self->s.rootJoint + CORVUS_HEAD,      ROLL, 0.0f, ANGLE_45);
	SetJointAngVel(self->s.rootJoint + CORVUS_UPPERBACK, ROLL, 0.0f, ANGLE_45);
	SetJointAngVel(self->s.rootJoint + CORVUS_LOWERBACK, ROLL, 0.0f, ANGLE_45);
}

void G_PlayerActionChickenBite(const playerinfo_t* info)
{
	vec3_t vf;
	AngleVectors(info->aimangles, vf, NULL, NULL);

	vec3_t end_pos;
	VectorMA(info->origin, 64.0f, vf, end_pos);

	// Account for step height.
	trace_t tr;
	edict_t* self = (edict_t*)info->self; //mxd
	const vec3_t mins = { info->mins[0], info->mins[1], info->mins[2] + 18.0f };
	gi.trace(info->origin, mins, info->maxs, end_pos, self, MASK_SHOT, &tr);

	char* hit_type; //mxd

	if (tr.ent != NULL && tr.ent->takedamage != DAMAGE_NO)
	{
		const int damage = ((info->edictflags & FL_SUPER_CHICKEN) ? 500 : 1); //mxd
		T_Damage(tr.ent, self, self, vf, tr.endpos, tr.plane.normal, damage, 0, DAMAGE_AVOID_ARMOR, MOD_CHICKEN);
		hit_type = "bite";  // Sound for hitting.
	}
	else
	{
		hit_type = "peck"; // Sound for missing.
	}

	const char* chicken_type = ((info->edictflags & FL_SUPER_CHICKEN) ? "superchicken" : "chicken"); //mxd
	const char* sound_name = va("monsters/%s/%s%i.wav", chicken_type, hit_type, irand(1, 2)); //mxd
	gi.sound(self, CHAN_WEAPON, gi.soundindex(sound_name), 1.0f, ATTN_NORM, 0.0f);
}

void G_PlayerFallingDamage(const playerinfo_t* info, const float delta)
{
	edict_t* ent = (edict_t*)info->self;
	ent->pain_debounce_time = level.time;

	float damage = delta - 30.0f;

	if (delta <= 50.0f)
		damage = max(1.0f, damage * 0.8f);

	T_Damage(ent, world, world, vec3_up, ent->s.origin, vec3_origin, (int)damage, 0, DAMAGE_AVOID_ARMOR, MOD_FALLING);

	// Stomp whatever we landed on?
	if (!DEATHMATCH && !COOP) //TODO: remove this check? Why is this done in COOP/DM only?
		return;

	if (ent->groundentity == NULL || ent->groundentity->takedamage == DAMAGE_NO)
		return;

	vec3_t victim_dir;
	VectorSubtract(ent->groundentity->s.origin, ent->s.origin, victim_dir);
	VectorNormalize(victim_dir);

	vec3_t impact_spot;
	VectorMA(ent->s.origin, -1.2f * ent->mins[2], victim_dir, impact_spot);

	const qboolean is_superchicken = (info->edictflags & FL_SUPER_CHICKEN); //mxd
	const int mod = (is_superchicken ? MOD_CHICKEN : MOD_UNKNOWN);
	const int stomp_damage = (is_superchicken ? 500 : (int)(damage * 2.0f)); //mxd
	T_Damage(ent->groundentity, ent, ent, victim_dir, impact_spot, vec3_origin, stomp_damage, 0, DAMAGE_AVOID_ARMOR, mod); //BUGFIX: original version doesn't use 'mod' value.

	// Knock down player we landed on?
	if (ent->groundentity->client != NULL && ent->groundentity->health > 0 && irand(0, 1) == 0)
		P_KnockDownPlayer(&ent->groundentity->client->playerinfo);
}

// Check to kick entities inside the pole vault animation.
void G_PlayerVaultKick(const playerinfo_t* info)
{
#define VAULTKICK_DIST		30		// Amount to trace outward from the player's origin.
#define VAULTKICK_MODIFIER	0.25f	// Percentage of the velocity magnitude to use as damage.

	edict_t* self = (edict_t*)info->self;

	// Ignore pitch.
	vec3_t forward = { 0.0f, self->s.angles[YAW], 0.0f };
	AngleVectors(forward, forward, NULL, NULL);

	// Move ahead by a small amount.
	vec3_t end_pos;
	VectorMA(self->s.origin, VAULTKICK_DIST, forward, end_pos);

	// Trace out to see if we've hit anything.
	trace_t tr;
	gi.trace(self->s.origin, self->mins, self->maxs, end_pos, self, MASK_PLAYERSOLID, &tr);

	if (tr.fraction == 1.0f || tr.startsolid || tr.allsolid || tr.ent->takedamage == DAMAGE_NO)
		return;

	// Find the velocity of the kick.
	const float kick_vel = VectorLength(self->velocity) * VAULTKICK_MODIFIER;

	//FIXME: Get a real sound.
	gi.sound(self, CHAN_WEAPON, gi.soundindex("monsters/plagueElf/hamhit.wav"), 1.0f, ATTN_NORM, 0.0f);
	T_Damage(tr.ent, self, self, forward, tr.endpos, tr.plane.normal, (int)kick_vel, (int)(kick_vel * 2.0f), DAMAGE_NORMAL, MOD_KICKED);

	VectorMA(tr.ent->velocity, flrand(300.0f, 500.0f), forward, tr.ent->velocity); //mxd. Original logic uses irand() here.
	tr.ent->velocity[2] = 150.0f;

	// Knock down player we kicked?
	if (tr.ent->client != NULL && tr.ent->health > 0 && AI_IsInfrontOf(tr.ent, self) && irand(0, 2) == 0)
		P_KnockDownPlayer(&tr.ent->client->playerinfo);
}

void G_PlayerSpellShieldAttack(const playerinfo_t* info)
{
	if (irand(0, SHIELD_ATTACK_CHANCE - 1) == 0)
		SpellLightningShieldAttack((edict_t*)info->self);
}

// Stop the attack and remove the persistent effect.
void G_PlayerSpellStopShieldAttack(const playerinfo_t* info)
{
	edict_t* self = info->self;

	if (self->PersistantCFX > 0)
	{
		gi.RemovePersistantEffect(self->PersistantCFX, REMOVE_SHIELD);
		self->PersistantCFX = 0;
		self->s.sound = 0;
	}
}

#pragma region ========================== Player actions ==========================

void G_PlayerActionSwordAttack(const playerinfo_t* info, const int value)
{
	WeaponThink_SwordStaff((edict_t*)info->self, "i", value);
}

void G_PlayerActionSpellFireball(const playerinfo_t* info)
{
	WeaponThink_FlyingFist((edict_t*)info->self, "");
}

void G_PlayerActionSpellBlast(const playerinfo_t* info)
{
	WeaponThink_Blast((edict_t*)info->self, "");
}

void G_PlayerActionSpellArray(const playerinfo_t* info, const int value)
{
	WeaponThink_MagicMissileSpread((edict_t*)info->self, "i", value);
}

void G_PlayerActionSpellSphereCreate(const playerinfo_t* info, qboolean* charging)
{
	WeaponThink_SphereOfAnnihilation((edict_t*)info->self, "g", charging); // Start a glow effect.
}

void G_PlayerActionSpellBigBall(const playerinfo_t* info)
{
	WeaponThink_Maceballs((edict_t*)info->self, "");
}

void G_PlayerActionSpellFirewall(const playerinfo_t* info)
{
	WeaponThink_Firewall((edict_t*)info->self, "");
}

void G_PlayerActionRedRainBowAttack(const playerinfo_t* info)
{
	WeaponThink_RedRainBow((edict_t*)info->self, "");
}

void G_PlayerActionPhoenixBowAttack(const playerinfo_t* info)
{
	WeaponThink_PhoenixBow((edict_t*)info->self, "");
}

void G_PlayerActionHellstaffAttack(const playerinfo_t* info)
{
	WeaponThink_HellStaff((edict_t*)info->self, "");
}

void G_PlayerActionSpellDefensive(playerinfo_t* info)
{
	if (info->leveltime <= info->defensive_debounce)
		return;

	info->pers.defence->weaponthink((edict_t*)info->self, "");
	info->defensive_debounce = info->leveltime + DEFENSE_DEBOUNCE;

	if (P_Defence_CurrentShotsLeft(info, 1) > 0) //TODO: do ALL defenses use 1 mana? Even when Tome of Power is active?
		return;

	// If we've run out of defense shots, and we have the ring of repulsion - switch to that.
	const int index = ITEM_INDEX(P_FindItem("ring"));
	if (info->pers.inventory.Items[index] > 0)
		info->G_UseItem(info->self, "ring");
}

#pragma endregion

// This is exceedingly gay that this has to be done this way...
qboolean G_EntIsAButton(const edict_t* ent)
{
	return ent->classID == CID_BUTTON;
}