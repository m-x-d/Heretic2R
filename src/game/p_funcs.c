//
// p_funcs.c
//
// Heretic II - Raven software
//

#include "p_funcs.h"
#include "p_anims.h"
#include "p_client.h" //mxd
#include "p_main.h"
#include "g_combat.h" //mxd
#include "g_playstats.h"
#include "g_Skeletons.h"
#include "g_teleport.h"
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

// ************************************************************************************************
// G_PlayerActionCheckPushPull_Ent
// -------------------------------
// ************************************************************************************************

qboolean G_PlayerActionCheckPushPull_Ent(void *ent)
{
	if(!(strcmp(((edict_t *)ent)->classname,"func_train")==0)||!(((edict_t *)ent)->spawnflags&32))
		return(false);
	else
		return(true);
}

// ************************************************************************************************
// PushPull_stop
// -------------
// ************************************************************************************************

void PushPull_stop(edict_t *self)
{
/*
	playerinfo_t *playerinfo;

	playerinfo=&self->target_ent->client->playerinfo;

	if((playerinfo->lowerseq!=ASEQ_PUSH)&&(playerinfo->lowerseq!=ASEQ_PULL)) 
		VectorClear(self->velocity);
	else if (Vec3IsZero(self->target_ent->velocity))
		VectorClear(self->target_ent->velocity);
*/
}

// ************************************************************************************************
// G_PlayerActionMoveItem
// ----------------------
// ************************************************************************************************

void G_PlayerActionMoveItem(playerinfo_t *playerinfo,float distance)
{
	vec3_t player_facing,pushdir;

	VectorCopy(playerinfo->angles,player_facing);
	player_facing[PITCH]=player_facing[ROLL]=0;
	AngleVectors(player_facing, pushdir, NULL, NULL);

	VectorScale (pushdir, distance, ((edict_t *)playerinfo->target_ent)->velocity);

	((edict_t *)(playerinfo->self))->target_ent->think = PushPull_stop;
	((edict_t *)(playerinfo->self))->target_ent->nextthink = level.time + 2 * FRAMETIME;
	((edict_t *)(playerinfo->self))->target_ent->target_ent = ((edict_t *)playerinfo->self);
}

// ************************************************************************************************
// G_PlayerActionCheckPushButton
// -----------------------------
// ************************************************************************************************

#define MAX_PUSH_BUTTON_RANGE	80.0

qboolean G_PlayerActionCheckPushButton(playerinfo_t *playerinfo)
{
	edict_t *t;
	vec3_t	v,dir;
	float	len1, dot;
	vec3_t	forward;

	// Are you near a button?

 	if(!((edict_t *)playerinfo->self)->target)
	{
		// No button so return.

		return false;
	}

	// A button is nearby, so look to see if it's in reach.

	t = NULL;
	t = G_Find(t,FOFS(targetname),((edict_t *)playerinfo->self)->target);

	if (!t)  
		return(false);

// 	if (!(strcmp(t->classname,"func_train")==0))
 	if (t->classID == CID_BUTTON)
	{
		// Get center of button
		VectorAverage(t->mins, t->maxs, v);
		// Get distance from player origin to center of button
		Vec3SubtractAssign(playerinfo->origin, v);
		len1 = VectorLength(v);
	}
	else
		return(false);

	if (len1 < MAX_PUSH_BUTTON_RANGE)
	{
		VectorCopy(((edict_t *)playerinfo->self)->client->playerinfo.aimangles, dir);
		dir[PITCH] = 0;

		AngleVectors(dir, forward, NULL, NULL);
		VectorNormalize(v);
		// Both these vectors are normalized so result is cos of angle
		dot = DotProduct(v, forward);

		// 41 degree range either way 
		if (dot > 0.75)
			return(true);
	}

	return(false);
}

// ************************************************************************************************
// G_PlayerActionPushButton
// ------------------------
// ************************************************************************************************

void G_PlayerActionPushButton(playerinfo_t *playerinfo)
{
	G_UseTargets((edict_t *)playerinfo->self,(edict_t *)playerinfo->self);
}

// ************************************************************************************************
// G_PlayerActionCheckPushLever
// -----------------------------
// ************************************************************************************************

#define MAX_PUSH_LEVER_RANGE	80.0

qboolean G_PlayerActionCheckPushLever(playerinfo_t *playerinfo)
{
	edict_t *t;
	vec3_t	v,dir;
	float	len1, dot;
	vec3_t	forward;
	edict_t *self;

	self = (edict_t *) playerinfo->self;

	// Are you near a lever?

 	if(!(self->target))
	{
		// No button so return.

		return false;
	}

	// A button is nearby, so look to see if it's in reach.

	t = NULL;
	t = G_Find(t,FOFS(targetname),self->target);

	if (!t)  
		return(false);

 	if (t->classID == CID_LEVER)
	{
		// Get distance from player origin to center of lever
		VectorSubtract(playerinfo->origin, t->s.origin,v);
		len1 = VectorLength(v);
	}
	else
		return(false);

	if (len1 < MAX_PUSH_LEVER_RANGE)
	{		
		VectorCopy(((edict_t *)playerinfo->self)->client->playerinfo.aimangles, dir);
		dir[PITCH] = 0;

		AngleVectors(dir, forward, NULL, NULL);
		VectorSubtract (t->s.origin, self->s.origin, v);
		VectorNormalize(v);
		// Both these vectors are normalized so result is cos of angle
		dot = DotProduct(v, forward);

		// 41 degree range either way 
		if (dot > 0.70)
			return(true); 
	}

	return(false);
}

// ************************************************************************************************
// G_PlayerActionPushLever
// ------------------------
// ************************************************************************************************

void G_PlayerActionPushLever(playerinfo_t *playerinfo)
{
	G_UseTargets((edict_t *)playerinfo->self,(edict_t *)playerinfo->self);
}

// ************************************************************************************************
// G_HandleTeleport
// ----------------
// ************************************************************************************************

qboolean G_HandleTeleport(playerinfo_t *playerinfo)
{
	// Are we teleporting or morphing?

	if (playerinfo->flags & (PLAYER_FLAG_TELEPORT | PLAYER_FLAG_MORPHING))
	{
		// Are we doing de-materialiZe or...

		if (((edict_t *)playerinfo->self)->client->tele_dest[0]!=-1)
		{
			// Are we done dematerialiZing? Or still fading?

			if (((edict_t *)playerinfo->self)->client->tele_count--)
			{
				((edict_t *)playerinfo->self)->s.color.a -= TELE_FADE_OUT;
				
				return(true);
			}
			else
			{
				// We have finished dematerialiZing, let's move the character.

				if (playerinfo->flags & PLAYER_FLAG_TELEPORT)
				{
					Perform_Teleport((edict_t *)playerinfo->self);
				}
				else
				{	
					if(playerinfo->edictflags & FL_CHICKEN)
					{
						// We're set as a chicken.

						reset_morph_to_elf((edict_t *)playerinfo->self);
					}
					else
					{
						Perform_Morph((edict_t *)playerinfo->self);
					}
				}

				return(true);
			}
		}
		else
		{
			// Are we done dematerialiZing? Or still fading?

			if (((edict_t *)playerinfo->self)->client->tele_count--)
			{
				((edict_t *)playerinfo->self)->s.color.a += TELE_FADE;
			}
			else
			{
				// We are done re-materialiZing, let's kill all this BS and get back to the game.

				if(playerinfo->flags & PLAYER_FLAG_TELEPORT)
					CleanUpTeleport((edict_t *)playerinfo->self);
				else
					CleanUpMorph((edict_t *)playerinfo->self);
			}
		}

		if(!deathmatch->value)
		  	return(true);
	}

	return(false);
}

// ************************************************************************************************
// PlayerChickenDeath
// ------------------
// ************************************************************************************************

void PlayerChickenDeath(edict_t *self)
{
	//FIXME:

	//gi.sound (self, CHAN_BODY, sounds[SND_GIB], 1, ATTN_NORM, 0);
	self->deadflag = DEAD_DEAD;
	self->client->playerinfo.deadflag = DEAD_DEAD;
	gi.CreateEffect(&self->s, FX_CHICKEN_EXPLODE, CEF_OWNERS_ORIGIN, NULL, "" ); 

	// fix that respawning bug
	self->morph_timer = level.time -1;
	
	// Reset our thinking.

	self->think = self->oldthink;
	self->nextthink = level.time + FRAMETIME;

#ifdef COMP_FMOD
	self->model = "models/player/corvette/tris_c.fm";
#else
	self->model = "models/player/corvette/tris.fm";
#endif
	self->pain = player_pain;

	// Reset our skins.

	self->s.effects = 0;

	self->s.skinnum = 0;	// Hey, the skinnum stores the skin now, capiche?
	self->s.clientnum = self - g_edicts - 1;

	self->s.modelindex = 255;		// will use the skin specified model
	self->s.frame = 0;

	// Turn our skeleton back on.

	self->s.skeletalType = SKEL_CORVUS;
	self->s.effects |= (EF_SWAPFRAME|EF_JOINTED);
	self->s.effects &= ~EF_CHICKEN;
	self->flags &= ~FL_CHICKEN;
	self->s.renderfx &= ~RF_IGNORE_REFS;

	// Reset our animations.

	P_PlayerAnimReset(&self->client->playerinfo);
}

// ************************************************************************************************
// G_SetJointAngles
// ------------------
// Set the player model's joint angles.
// ************************************************************************************************

void G_SetJointAngles(playerinfo_t *playerinfo) 
{
	edict_t *self;

	self=(edict_t *)playerinfo->self;

	SetJointAngVel(self->s.rootJoint+CORVUS_HEAD,PITCH,playerinfo->targetjointangles[PITCH],ANGLE_45);
	SetJointAngVel(self->s.rootJoint+CORVUS_HEAD,ROLL,playerinfo->targetjointangles[YAW],ANGLE_45);

	if(!playerinfo->headjointonly)
	{
		SetJointAngVel(self->s.rootJoint+CORVUS_UPPERBACK,PITCH,playerinfo->targetjointangles[PITCH],ANGLE_45);
		SetJointAngVel(self->s.rootJoint+CORVUS_LOWERBACK,PITCH,playerinfo->targetjointangles[PITCH],ANGLE_45);
		SetJointAngVel(self->s.rootJoint+CORVUS_UPPERBACK,ROLL,playerinfo->targetjointangles[YAW],ANGLE_45);
		SetJointAngVel(self->s.rootJoint+CORVUS_LOWERBACK,ROLL,playerinfo->targetjointangles[YAW],ANGLE_45);
	}
	else
	{
		SetJointAngVel(self->s.rootJoint+CORVUS_UPPERBACK,PITCH,0,ANGLE_45);
		SetJointAngVel(self->s.rootJoint+CORVUS_LOWERBACK,PITCH,0,ANGLE_45);
		SetJointAngVel(self->s.rootJoint+CORVUS_UPPERBACK,ROLL,0,ANGLE_45);
		SetJointAngVel(self->s.rootJoint+CORVUS_LOWERBACK,ROLL,0,ANGLE_45);
	}
}

// ************************************************************************************************
// G_ResetJointAngles
// ------------------
// Reset the player model's joint angles.
// ************************************************************************************************

void G_ResetJointAngles(playerinfo_t *playerinfo)
{
	edict_t *self;

	self=(edict_t *)playerinfo->self;

	SetJointAngVel(self->s.rootJoint + CORVUS_HEAD,PITCH,0,ANGLE_45);
	SetJointAngVel(self->s.rootJoint + CORVUS_UPPERBACK,PITCH,0,ANGLE_45);
	SetJointAngVel(self->s.rootJoint + CORVUS_LOWERBACK,PITCH,0,ANGLE_45);

	SetJointAngVel(self->s.rootJoint + CORVUS_HEAD,ROLL,0,ANGLE_45);
	SetJointAngVel(self->s.rootJoint + CORVUS_UPPERBACK,ROLL,0,ANGLE_45);
	SetJointAngVel(self->s.rootJoint + CORVUS_LOWERBACK,ROLL,0,ANGLE_45);
}

// ************************************************************************************************
// G_PlayerActionChickenBite
// -------------------------
// ************************************************************************************************

void G_PlayerActionChickenBite(playerinfo_t *playerinfo)
{
	trace_t	trace;
	vec3_t	endpos, vf, mins;

	AngleVectors(playerinfo->aimangles, vf, NULL, NULL);
	VectorMA(playerinfo->origin, 64, vf, endpos);
	
	
	//Account for step height
	VectorSet(mins, playerinfo->mins[0], playerinfo->mins[1], playerinfo->mins[2] + 18);

	gi.trace(playerinfo->origin, mins, playerinfo->maxs, endpos, ((edict_t *)playerinfo->self), MASK_SHOT,&trace);

	if (trace.ent && trace.ent->takedamage)
	{
		if (playerinfo->edictflags & FL_SUPER_CHICKEN)
			T_Damage(trace.ent,((edict_t *)playerinfo->self),((edict_t *)playerinfo->self),vf,trace.endpos,trace.plane.normal,500,0,DAMAGE_AVOID_ARMOR,MOD_CHICKEN);
		else
			T_Damage(trace.ent,((edict_t *)playerinfo->self),((edict_t *)playerinfo->self),vf,trace.endpos,trace.plane.normal,1,0,DAMAGE_AVOID_ARMOR,MOD_CHICKEN);

		if (playerinfo->edictflags & FL_SUPER_CHICKEN)
		{
			// Sound for hitting.
			if (irand(0,1))
				gi.sound(((edict_t *)playerinfo->self), CHAN_WEAPON, gi.soundindex ("monsters/superchicken/bite1.wav"), 1, ATTN_NORM, 0);
			else
				gi.sound(((edict_t *)playerinfo->self), CHAN_WEAPON, gi.soundindex ("monsters/superchicken/bite2.wav"), 1, ATTN_NORM, 0);
		}
		else
		{
			// Sound for hitting.
			if (irand(0,1))
				gi.sound(((edict_t *)playerinfo->self), CHAN_WEAPON, gi.soundindex ("monsters/chicken/bite1.wav"), 1, ATTN_NORM, 0);
			else
				gi.sound(((edict_t *)playerinfo->self), CHAN_WEAPON, gi.soundindex ("monsters/chicken/bite2.wav"), 1, ATTN_NORM, 0);
		}
	}
	else
	{	// Sound for missing.
		if (playerinfo->edictflags & FL_SUPER_CHICKEN)
		{
			if (irand(0,1))
				gi.sound(((edict_t *)playerinfo->self), CHAN_WEAPON, gi.soundindex ("monsters/superchicken/peck1.wav"), 1, ATTN_NORM, 0);
			else
				gi.sound(((edict_t *)playerinfo->self), CHAN_WEAPON, gi.soundindex ("monsters/superchicken/peck2.wav"), 1, ATTN_NORM, 0);
		}
		else
		{
			if (irand(0,1))
				gi.sound(((edict_t *)playerinfo->self), CHAN_WEAPON, gi.soundindex ("monsters/chicken/peck1.wav"), 1, ATTN_NORM, 0);
			else
				gi.sound(((edict_t *)playerinfo->self), CHAN_WEAPON, gi.soundindex ("monsters/chicken/peck2.wav"), 1, ATTN_NORM, 0);
		}
	}
}

// ************************************************************************************************
// G_PlayerFallingDamage
// ---------------------
// ************************************************************************************************

void G_PlayerFallingDamage(playerinfo_t *playerinfo,float delta)
{
	edict_t *ent;
	vec3_t	dir;
	float	damage;

	ent=(edict_t *)playerinfo->self;

	ent->pain_debounce_time=level.time;
	
	if(delta > 50)
		damage = delta - 30;
	else if((damage = (delta - 30) * 0.8) < 1.0f)
		damage = 1;

	VectorSet(dir,0.0,0.0,1.0);

	T_Damage(ent,world,world,dir,ent->s.origin,vec3_origin,damage,0,DAMAGE_AVOID_ARMOR,MOD_FALLING);

	if(deathmatch->value || coop->value)
	{
		if(ent->groundentity && ent->groundentity->takedamage)
		{
			int		mod;
			vec3_t	victim_dir, impact_spot;

			if (playerinfo->edictflags & FL_SUPER_CHICKEN)
			{
				damage = 500;
				mod = MOD_CHICKEN;
			}
			else
			{
				damage *= 2;
				mod = 0;
			}

			VectorSubtract(ent->groundentity->s.origin, ent->s.origin, victim_dir);
			VectorNormalize(victim_dir);
			VectorMA(ent->s.origin, -1.2 * ent->mins[2], victim_dir, impact_spot);

			T_Damage(ent->groundentity, ent, ent, victim_dir, impact_spot, vec3_origin, damage, 0, DAMAGE_AVOID_ARMOR, 0);
			if(ent->groundentity->client)
			{
				if(ent->groundentity->health > 0)
				{
					if(!irand(0, 1))
					{
						P_KnockDownPlayer(&ent->groundentity->client->playerinfo);
					}
				}
			}
		}
	}
}


// *******************************************************
// G_PlayerVaultKick
// -----------------------------
// Check to kick entities inside the pole vault animation
// *******************************************************
#define VAULTKICK_DIST	30			//Amount to trace outward from the player's origin
#define VAULTKICK_MODIFIER 0.25		//percentage of the velocity magnitude to use as damage

void G_PlayerVaultKick(playerinfo_t *playerinfo)
{
	edict_t *self = ((edict_t *)playerinfo->self);
	trace_t trace;
	vec3_t	endpos, vf;
	float	kick_vel;

	//Ignore pitch
	VectorSet(vf, 0, self->s.angles[YAW], 0);
	AngleVectors(vf, vf, NULL, NULL);
	
	//Move ahead by a small amount
	VectorMA(self->s.origin, VAULTKICK_DIST, vf, endpos);

	//Trace out to see if we've hit anything
	gi.trace(self->s.origin, self->mins, self->maxs, endpos, self, MASK_PLAYERSOLID,&trace);

	//If we have...
	if (trace.fraction < 1 && (!(trace.startsolid || trace.allsolid)) )
	{
		if (trace.ent->takedamage)
		{
			//Find the velocity of the kick
			kick_vel = VectorLength(self->velocity);
			kick_vel *= VAULTKICK_MODIFIER;

			//FIXME: Get a real sound
			gi.sound(self, CHAN_WEAPON, gi.soundindex("monsters/plagueElf/hamhit.wav"), 1, ATTN_NORM, 0);
			T_Damage(trace.ent, self, self, vf, trace.endpos, trace.plane.normal, kick_vel, kick_vel*2, DAMAGE_NORMAL,MOD_KICKED);
			VectorMA(trace.ent->velocity, irand(300,500), vf, trace.ent->velocity);
			trace.ent->velocity[2] = 150;
			if(trace.ent->client)
			{
				if(trace.ent->health > 0)
				{
					if(infront(trace.ent, self) && !irand(0, 2))
					{
						P_KnockDownPlayer(&trace.ent->client->playerinfo);
					}
				}
			}
		}
	}
}

// *******************************************************
// G_PlayerLightningShieldDamage
// -----------------------------
// *******************************************************

extern void SpellLightningShieldAttack(edict_t *self);
void G_PlayerSpellShieldAttack(playerinfo_t *playerinfo)
{
	if (irand(0, (SHIELD_ATTACK_CHANCE-1)) == 0)
		SpellLightningShieldAttack((edict_t *)playerinfo->self);
}

// stop the attack and remove the persistant effect
void G_PlayerSpellStopShieldAttack(playerinfo_t *playerinfo)
{
	edict_t	*self;

	self = playerinfo->self;
	if (self->PersistantCFX)
	{
		gi.RemovePersistantEffect(self->PersistantCFX, REMOVE_SHIELD);
		self->PersistantCFX = 0;
		self->s.sound = 0;
	}

}


// ************************************************************************************************
// G_PlayerActionSwordAttack
// -------------------------
// ************************************************************************************************

void G_PlayerActionSwordAttack(playerinfo_t *playerinfo,int value)
{
	WeaponThink_SwordStaff((edict_t *)playerinfo->self,"i",value);
}

// ************************************************************************************************
// G_PlayerActionSpellFireball
// ---------------------------
// ************************************************************************************************

void G_PlayerActionSpellFireball(playerinfo_t *playerinfo)
{
	WeaponThink_FlyingFist((edict_t *)playerinfo->self,"");
}

// ************************************************************************************************
// G_PlayerActionSpellBlast
// ------------------------
// ************************************************************************************************

void G_PlayerActionSpellBlast(playerinfo_t *playerinfo)
{
	WeaponThink_Blast((edict_t *)playerinfo->self,"");
}

// ************************************************************************************************
// G_PlayerActionSpellArray
// ------------------------
// ************************************************************************************************

void G_PlayerActionSpellArray(playerinfo_t *playerinfo,int value)
{
	WeaponThink_MagicMissileSpread((edict_t *)playerinfo->self,"i",value);
}

// ************************************************************************************************
// G_PlayerActionSpellSphereCreate
// -------------------------------
// ************************************************************************************************

void G_PlayerActionSpellSphereCreate(playerinfo_t *playerinfo,qboolean *Charging)
{
	// Start a glow effect.
	WeaponThink_SphereOfAnnihilation((edict_t *)playerinfo->self,"g",Charging);
}

// ************************************************************************************************
// G_PlayerActionSpellBigBall
// --------------------------
// ************************************************************************************************

void G_PlayerActionSpellBigBall(playerinfo_t *playerinfo)
{
	WeaponThink_Maceballs((edict_t *)playerinfo->self,"");
}

// ************************************************************************************************
// G_PlayerActionSpellFirewall
// ---------------------------
// ************************************************************************************************

void G_PlayerActionSpellFirewall(playerinfo_t *playerinfo)
{
	WeaponThink_Firewall((edict_t *)playerinfo->self,"");
}

// ************************************************************************************************
// G_PlayerActionRedRainBowAttack
// ------------------------------
// ************************************************************************************************

void G_PlayerActionRedRainBowAttack(playerinfo_t *playerinfo)
{
	WeaponThink_RedRainBow((edict_t *)playerinfo->self,"");
}

// ************************************************************************************************
// G_PlayerActionPhoenixBowAttack
// ------------------------------
// ************************************************************************************************

void G_PlayerActionPhoenixBowAttack(playerinfo_t *playerinfo)
{
	WeaponThink_PhoenixBow((edict_t *)playerinfo->self,"");
}

// ************************************************************************************************
// G_PlayerActionHellstaffAttack
// -----------------------------
// ************************************************************************************************

void G_PlayerActionHellstaffAttack(playerinfo_t *playerinfo)
{
	WeaponThink_HellStaff((edict_t *)playerinfo->self,"");
}

// ************************************************************************************************
// G_PlayerActionSpellDefensive
// ----------------------------
// ************************************************************************************************

void G_PlayerActionSpellDefensive(playerinfo_t *playerinfo)
{
	int			index;
	gitem_t	*it;

	if (playerinfo->leveltime > playerinfo->defensive_debounce)
	{
//		playerinfo->pers.defence->use(playerinfo,playerinfo->pers.defence);
		playerinfo->pers.defence->weaponthink((edict_t *)playerinfo->self,"");
		playerinfo->defensive_debounce = playerinfo->leveltime + DEFENSE_DEBOUNCE;

		// if we've run out of defence shots, and we have the ring of repulsion - switch to that.
		it = P_FindItem ("ring");
		index = ITEM_INDEX(it);
		if ((P_Defence_CurrentShotsLeft(playerinfo, 1) <=0) && playerinfo->pers.inventory.Items[index])
		{
			playerinfo->G_UseItem(playerinfo->self,"ring");
		}
	}
}

// ************************************************************************************************
// G_EntIsAButton - this is exceedingly gay that this has to be done this way.
// ----------------------------
// ************************************************************************************************

qboolean G_EntIsAButton(edict_t *ent)
{
	if(ent->classID == CID_BUTTON)
		return (true);
	return (false);
}
