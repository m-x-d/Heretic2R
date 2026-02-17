//
// p_view.c
//
// Copyright 1998 Raven Software
//

#include "p_view.h" //mxd
#include "g_cmds.h" //mxd
#include "g_combat.h" //mxd
#include "g_playstats.h"
#include "g_Shrine.h" //mxd
#include "p_anims.h"
#include "p_funcs.h"
#include "p_hud.h" //mxd
#include "FX.h"
#include "Random.h"
#include "Vector.h"
#include "g_local.h" //mxd

static float bob_move;
static int bob_cycle; // Odd cycles are right foot going forward.

// Setup a looping sound on the client.
static void G_set_looping_sound(edict_t* self, const int sound_num)
{
	self->s.sound = (byte)sound_num;
	self->s.sound_data = (255 & ENT_VOL_MASK) | ATTN_NORM;
}

static int ClientServerRand(const playerinfo_t* info, const int min, const int max)
{
	if (min >= max)
		return min;

	int rand = (int)(info->leveltime * 10.0f);
	rand = (rand >> 7) ^ (rand >> 10) ^ (rand >> 5);
	rand %= (1 + max - min);

	return rand + min;
}

void InitPlayerinfo(const edict_t* ent)
{
	// Client side function callbacks (approximating functionality of server function callbacks).
	ent->client->playerinfo.CL_Sound = NULL;
	ent->client->playerinfo.CL_Trace = NULL;
	ent->client->playerinfo.CL_CreateEffect = NULL;

	// Server (game) function callbacks (approximating functionality of client-side function callbacks).
	ent->client->playerinfo.G_L_Sound = G_set_looping_sound;
	ent->client->playerinfo.G_Sound = gi.soundevent;
	ent->client->playerinfo.G_Trace = gi.trace;
	ent->client->playerinfo.G_CreateEffect = gi.CreateEffectEvent;
	ent->client->playerinfo.G_RemoveEffects = gi.RemoveEffectsEvent;

	// Server (game) function callbacks that have no client side equivalent.
	ent->client->playerinfo.G_SoundIndex = gi.soundindex;
	ent->client->playerinfo.G_SoundRemove = gi.soundremove;
	ent->client->playerinfo.G_UseTargets = G_UseTargets;
	ent->client->playerinfo.G_GetEntityStatePtr = G_GetEntityStatePtr;
	ent->client->playerinfo.G_BranchLwrClimbing = G_BranchLwrClimbing;
	ent->client->playerinfo.G_PlayerActionCheckRopeGrab = G_PlayerActionCheckRopeGrab;
	ent->client->playerinfo.G_PlayerClimbingMoveFunc = G_PlayerClimbingMoveFunc;
	ent->client->playerinfo.G_PlayerActionUsePuzzle = G_PlayerActionUsePuzzle;
	ent->client->playerinfo.G_PlayerActionCheckPuzzleGrab = G_PlayerActionCheckPuzzleGrab;
	ent->client->playerinfo.G_PlayerActionTakePuzzle = G_PlayerActionTakePuzzle;
	ent->client->playerinfo.G_PlayerActionCheckPushPull_Ent = G_PlayerActionCheckPushPull_Ent;
	ent->client->playerinfo.G_PlayerActionMoveItem = G_PlayerActionMoveItem; //TODO: unused!
	ent->client->playerinfo.G_PlayerActionCheckPushButton = G_PlayerActionCheckPushButton;
	ent->client->playerinfo.G_PlayerActionPushButton = G_PlayerActionPushButton;
	ent->client->playerinfo.G_PlayerActionCheckPushLever = G_PlayerActionCheckPushLever;
	ent->client->playerinfo.G_PlayerActionPushLever = G_PlayerActionPushLever;
	ent->client->playerinfo.G_HandleTeleport = G_HandleTeleport;
	ent->client->playerinfo.G_PlayerActionShrineEffect = G_PlayerActionShrineEffect;
	ent->client->playerinfo.G_PlayerActionChickenBite = G_PlayerActionChickenBite;
	ent->client->playerinfo.G_PlayerFallingDamage = G_PlayerFallingDamage;
	ent->client->playerinfo.G_PlayerSpellShieldAttack = G_PlayerSpellShieldAttack;
	ent->client->playerinfo.G_PlayerSpellStopShieldAttack = G_PlayerSpellStopShieldAttack;
	ent->client->playerinfo.G_PlayerVaultKick = G_PlayerVaultKick;
	ent->client->playerinfo.G_PlayerActionCheckRopeMove = G_PlayerActionCheckRopeMove;
	ent->client->playerinfo.G_gamemsg_centerprintf = gi.gamemsg_centerprintf;
	ent->client->playerinfo.G_levelmsg_centerprintf = gi.levelmsg_centerprintf;
	ent->client->playerinfo.G_WeapNext = Cmd_WeapPrev_f; //TODO: mxd. BUT WHYYYYY?..
	ent->client->playerinfo.G_UseItem = Cmd_Use_f;

	// Common client & server (game) function callbacks.
	ent->client->playerinfo.PointContents = gi.pointcontents;
	ent->client->playerinfo.SetJointAngles = G_SetJointAngles;
	ent->client->playerinfo.ResetJointAngles = G_ResetJointAngles;
	ent->client->playerinfo.PlayerActionSwordAttack = G_PlayerActionSwordAttack;
	ent->client->playerinfo.PlayerActionSpellFireball = G_PlayerActionSpellFireball;
	ent->client->playerinfo.PlayerActionSpellBlast = G_PlayerActionSpellBlast;
	ent->client->playerinfo.PlayerActionSpellArray = G_PlayerActionSpellArray;
	ent->client->playerinfo.PlayerActionSpellSphereCreate = G_PlayerActionSpellSphereCreate;
	ent->client->playerinfo.PlayerActionSpellBigBall = G_PlayerActionSpellBigBall;
	ent->client->playerinfo.PlayerActionSpellFirewall = G_PlayerActionSpellFirewall;
	ent->client->playerinfo.PlayerActionRedRainBowAttack = G_PlayerActionRedRainBowAttack;
	ent->client->playerinfo.PlayerActionPhoenixBowAttack = G_PlayerActionPhoenixBowAttack;
	ent->client->playerinfo.PlayerActionHellstaffAttack = G_PlayerActionHellstaffAttack;
	ent->client->playerinfo.PlayerActionSpellDefensive = G_PlayerActionSpellDefensive;
	ent->client->playerinfo.G_EntIsAButton = G_EntIsAButton;
	ent->client->playerinfo.irand = ClientServerRand;

	// So we know we are server (game) side.
	ent->client->playerinfo.isclient = false;
	ent->client->playerinfo.ishistory = false;
}

void SetupPlayerinfo(edict_t* ent)
{
	// Inputs only.

	// Pointer to the associated player's edict_t.
	ent->client->playerinfo.self = ent;

	// Game .dll variables.
	ent->client->playerinfo.leveltime = level.time;

	// Server variables.
	ent->client->playerinfo.sv_gravity = sv_gravity->value;
	ent->client->playerinfo.sv_cinematicfreeze = sv_cinematicfreeze->value;
	ent->client->playerinfo.sv_jumpcinematic = sv_jumpcinematic->value;

	// From edict_t.
	ent->client->playerinfo.ideal_yaw = ent->ideal_yaw;
	ent->client->playerinfo.groundentity = ent->groundentity;

	// Pointer to entity_state_t of player's enemy edict.
	ent->client->playerinfo.enemystate = (ent->enemy != NULL ? &ent->enemy->s : NULL);

	// Spell / weapon aiming direction.
	VectorCopy(ent->client->aimangles, ent->client->playerinfo.aimangles);

	// Deathmatch flags - only set this if we are in deathmatch.
	ent->client->playerinfo.dmflags = (DEATHMATCH ? (int)(DMFLAGS | DF_DEATHMATCH_SET) : 0); // Send the high bit if deathmatch.
	ent->client->playerinfo.advancedstaff = (int)(advancedstaff->value);

	// Inputs & outputs.

	memcpy(&ent->client->playerinfo.pcmd, &ent->client->pcmd, sizeof(usercmd_t));

	// If we are in a cinematic, remove certain commands from the ucmd_t the server received from the client.
	// NOTE: THIS IS HIGHLY SUBJECTIVE. REQUIRES VIGOUROUS TESTING.
	// Basically, just killing all buttons pressed while a cinematic is running - Probably not the best way to do this -- Jake 9/28/98.
	// Need to re-get this constantly, since it changes on the fly.
	if (SV_CINEMATICFREEZE)
	{
		ent->client->pcmd.buttons = 0;
		ent->client->pcmd.sidemove = 0;
		ent->client->pcmd.forwardmove = 0;
		ent->client->playerinfo.pcmd.buttons = 0;
		ent->client->playerinfo.pcmd.sidemove = 0;
		ent->client->playerinfo.pcmd.forwardmove = 0;
		ent->client->playerinfo.buttons = 0;
		ent->client->playerinfo.remember_buttons = 0;
	}

	// From edict_t.
	VectorCopy(ent->s.origin, ent->client->playerinfo.origin);
	VectorCopy(ent->s.angles, ent->client->playerinfo.angles);
	VectorCopy(ent->velocity, ent->client->playerinfo.velocity);
	VectorCopy(ent->mins, ent->client->playerinfo.mins);
	VectorCopy(ent->maxs, ent->client->playerinfo.maxs);
	ent->client->playerinfo.enemy = ent->enemy;
	ent->client->playerinfo.target = ent->target;
	ent->client->playerinfo.targetEnt = ent->targetEnt;
	ent->client->playerinfo.target_ent = ent->target_ent;
	ent->client->playerinfo.nextthink = ent->nextthink;
	ent->client->playerinfo.viewheight = (float)ent->viewheight;
	ent->client->playerinfo.watertype = ent->watertype;
	ent->client->playerinfo.waterlevel = ent->waterlevel;
	ent->client->playerinfo.deadflag = ent->dead_state;
	ent->client->playerinfo.movetype = ent->movetype;
	ent->client->playerinfo.edictflags = ent->flags;

	// From entity_state_t.
	ent->client->playerinfo.frame = ent->s.frame;
	ent->client->playerinfo.swapFrame = ent->s.swapFrame;
	ent->client->playerinfo.effects = ent->s.effects;
	ent->client->playerinfo.renderfx = ent->s.renderfx;
	ent->client->playerinfo.skinnum = ent->s.skinnum;
	ent->client->playerinfo.clientnum = ent->s.clientnum;

	for (int i = 0; i < MAX_FM_MESH_NODES; i++)
		ent->client->playerinfo.fmnodeinfo[i] = ent->s.fmnodeinfo[i];

	// From pmove_state_t.
	ent->client->playerinfo.pm_flags = ent->client->ps.pmove.pm_flags;
	ent->client->playerinfo.pm_w_flags = ent->client->ps.pmove.w_flags;
}

void WritePlayerinfo(edict_t* ent)
{
	// Inputs & outputs.

	memcpy(&ent->client->pcmd, &ent->client->playerinfo.pcmd, sizeof(usercmd_t));

	// From edict_t.
	VectorCopy(ent->client->playerinfo.origin, ent->s.origin);
	VectorCopy(ent->client->playerinfo.angles, ent->s.angles);
	VectorCopy(ent->client->playerinfo.velocity, ent->velocity);
	VectorCopy(ent->client->playerinfo.mins, ent->mins);
	VectorCopy(ent->client->playerinfo.maxs, ent->maxs);
	ent->enemy = ent->client->playerinfo.enemy;
	ent->targetEnt = ent->client->playerinfo.targetEnt;
	ent->target_ent = ent->client->playerinfo.target_ent;
	ent->target = ent->client->playerinfo.target;
	ent->nextthink = ent->client->playerinfo.nextthink;
	ent->viewheight = (int)ent->client->playerinfo.viewheight;
	ent->watertype = ent->client->playerinfo.watertype;
	ent->waterlevel = ent->client->playerinfo.waterlevel;
	ent->dead_state = ent->client->playerinfo.deadflag;
	ent->movetype = ent->client->playerinfo.movetype;
	ent->flags = ent->client->playerinfo.edictflags;

	// From entity_state_t.
	ent->s.frame = (short)ent->client->playerinfo.frame;
	ent->s.swapFrame = (short)ent->client->playerinfo.swapFrame;
	ent->s.effects = ent->client->playerinfo.effects;
	ent->s.renderfx = ent->client->playerinfo.renderfx;
	ent->s.skinnum = ent->client->playerinfo.skinnum;
	ent->s.clientnum = (short)ent->client->playerinfo.clientnum;

	for (int i = 0; i < MAX_FM_MESH_NODES; i++)
		ent->s.fmnodeinfo[i] = ent->client->playerinfo.fmnodeinfo[i];

	// From pmove_state_t.
	ent->client->ps.pmove.pm_flags = (byte)ent->client->playerinfo.pm_flags;
	ent->client->ps.pmove.w_flags = (byte)ent->client->playerinfo.pm_w_flags;

	// Outputs only.

	// From playerstate_t.
	VectorCopy(ent->client->playerinfo.offsetangles, ent->client->ps.offsetangles);
}

static void SetupPlayerinfo_effects(const edict_t* ent)
{
	ent->client->playerinfo.effects = ent->s.effects;
	ent->client->playerinfo.renderfx = ent->s.renderfx;
	ent->client->playerinfo.skinnum = ent->s.skinnum;
	ent->client->playerinfo.clientnum = ent->s.clientnum;

	for (int i = 0; i < MAX_FM_MESH_NODES; i++)
		ent->client->playerinfo.fmnodeinfo[i] = ent->s.fmnodeinfo[i];
}

static void WritePlayerinfo_effects(edict_t* ent)
{
	ent->s.effects = ent->client->playerinfo.effects;
	ent->s.renderfx = ent->client->playerinfo.renderfx;
	ent->s.skinnum = ent->client->playerinfo.skinnum;
	ent->s.clientnum = (short)ent->client->playerinfo.clientnum;

	for (int i = 0; i < MAX_FM_MESH_NODES; i++)
		ent->s.fmnodeinfo[i] = ent->client->playerinfo.fmnodeinfo[i];
}

void Player_UpdateModelAttributes(edict_t* ent) //mxd. 'ClientSetSkinType' in p_client.c in original version.
{
	SetupPlayerinfo_effects(ent);
	P_PlayerUpdateModelAttributes(&ent->client->playerinfo);
	WritePlayerinfo_effects(ent);
}

// Deal with incidental player stuff, like setting the personal light to OFF if its should be.
static void PlayerTimerUpdate(const edict_t* ent)
{
	playerinfo_t* info = &ent->client->playerinfo;

	// Disable light when we should.
	if (info->light_timer < level.time)
		info->effects &= ~EF_LIGHT_ENABLED;

	// Disable speed when we should.
	if (info->speed_timer < level.time)
		info->effects &= ~EF_SPEED_ACTIVE;

	// Disable max speed when we should.
	if (info->knockbacktime < level.time)
		info->effects &= ~EF_HIGH_MAX;

	// Disable powerup when we should.
	if (info->powerup_timer < level.time)
		info->effects &= ~EF_POWERUP_ENABLED;

	// Disable reflection when we should. Were we reflective last frame?
	if (info->reflect_timer < level.time && (info->renderfx & RF_REFLECTION))
	{
		info->renderfx &= ~RF_REFLECTION;
		P_PlayerUpdateModelAttributes(&ent->client->playerinfo); // Unset the skin.
	}

	// Disable ghosting when we should.
	if (info->ghost_timer < level.time)
		info->renderfx &= ~RF_TRANS_GHOST;
}

// Handles color blends and view kicks.
static void P_DamageFeedback(edict_t* player)
{
	gclient_t* client = player->client;

	// Flash the backgrounds behind the status numbers?
	client->ps.stats[STAT_FLASHES] = (short)((client->damage_blood > 0) ? 1 : 0);

	// Total up the points of damage shot at the player this frame.
	if (client->damage_blood == 0)
		return; // Didn't take any damage.

	// Check for gasssss damage.
	if (player->pain_debounce_time < level.time && client->damage_gas)
	{
		if (client->playerinfo.loweridle && client->playerinfo.upperidle)
			P_PlayerAnimSetLowerSeq(&client->playerinfo, ASEQ_PAIN_A);

		P_PlayerPlayPain(&client->playerinfo, 1);
	}
	else if ((irand(0, 4) == 0 || client->damage_blood > 8) && player->pain_debounce_time < level.time) // Play pain animation.
	{
		if (client->playerinfo.loweridle && client->playerinfo.upperidle)
			P_PlayerAnimSetLowerSeq(&client->playerinfo, ASEQ_PAIN_A);

		const int snd_type = (client->damage_blood > 4 ? 0 : 2); //mxd. 0 - normal, 1 - gas, 2 - small.
		P_PlayerPlayPain(&client->playerinfo, snd_type);
		player->pain_debounce_time = level.time + 0.5f;
	}

	// Reset the player's pain_debounce_time.
	if (level.time > player->pain_debounce_time)
		player->pain_debounce_time = level.time + 0.7f;

	// Clear damage totals.
	client->damage_blood = 0;
	client->damage_knockback = 0;
}

static void P_WorldEffects(edict_t* player) //mxd. +player arg.
{
#define WATERSPLASH_TRACE_DIST	56.0f //mxd

	if (player->client->playerinfo.deadflag != DEAD_NO)
		return;

	// If we are in no clip, we don't need air.
	if (player->movetype == PHYSICSTYPE_NOCLIP)
	{
		player->air_finished = level.time + HOLD_BREATH_TIME;
		return;
	}

	const int waterlevel = player->waterlevel;
	const int old_waterlevel = player->client->old_waterlevel;
	player->client->old_waterlevel = waterlevel;

	// If the current player just entered a water volume, play a sound and start a water-ripple client effect.
	if (old_waterlevel == 0 && waterlevel > 0)
	{
		// Clear damage_debounce_time, so the pain sound will play immediately.
		player->damage_debounce_time = level.time - 1.0f;

		if (player->watertype & CONTENTS_LAVA)
		{
			gi.sound(player, CHAN_BODY, gi.soundindex("player/inlava.wav"), 1.0f, ATTN_NORM, 0.0f);
			player->flags |= FL_INLAVA;
		}
		else if (player->watertype & CONTENTS_SLIME)
		{
			gi.sound(player, CHAN_BODY, gi.soundindex("player/muckin.wav"), 1.0f, ATTN_NORM, 0.0f);
			player->flags |= FL_INSLIME;
		}
		else
		{
			gi.sound(player, CHAN_BODY, gi.soundindex("player/Water Enter.wav"), 1.0f, ATTN_NORM, 0.0f);
		}

		player->flags |= FL_INWATER;

		//mxd. Get water surface direction (because there are enterable horizontal water surfaces in the game).
		const vec3_t origin = VEC3_INITA(player->s.origin, 0.0f, 0.0f, player->client->playerinfo.waterheight - 1.0f); // Reduce waterheight a bit to make sure origin is underwater...

		vec3_t dir;
		VectorSubtract(player->client->playerinfo.LastWatersplashPos, player->s.origin, dir);
		VectorNormalize(dir);

		vec3_t old_origin;
		VectorMA(origin, WATERSPLASH_TRACE_DIST, dir, old_origin);

		trace_t trace;
		gi.trace(old_origin, vec3_origin, vec3_origin, origin, player, MASK_WATER, &trace);

		//TODO: different entry splashes for lava and slime.
		if (!trace.startsolid && trace.fraction < 1.0f)
			gi.CreateEffect(NULL, FX_WATER_ENTRYSPLASH, CEF_FLAG7, trace.endpos, "bd", 128 | 96, trace.plane.normal); // FIXME: Size propn. to entry velocity?
	}
	else if (old_waterlevel > 0 && waterlevel == 0)
	{
		// If the current player just completely exited a water volume, play a sound.

		// FL_INWATER is set whether in lava, slime or water.
		if (player->flags & FL_INLAVA)
		{
			gi.sound(player, CHAN_BODY, gi.soundindex("player/inlava.wav"), 1.0f, ATTN_NORM, 0.0f);
			player->flags &= ~FL_INLAVA;
		}
		else if (player->flags & FL_INSLIME)
		{
			gi.sound(player, CHAN_BODY, gi.soundindex("player/muckexit.wav"), 1.0f, ATTN_NORM, 0.0f);
			player->flags &= ~FL_INSLIME;
		}
		else
		{
			gi.sound(player, CHAN_BODY, gi.soundindex("player/Water Exit.wav"), 1.0f, ATTN_NORM, 0.0f);
		}

		player->flags &= ~FL_INWATER;

		//mxd. Get water surface direction (because there are enterable horizontal water surfaces in the game).
		const vec3_t origin = VEC3_INIT(player->s.origin);

		vec3_t dir;
		VectorSubtract(player->client->playerinfo.LastWatersplashPos, player->s.origin, dir);
		VectorNormalize(dir);

		vec3_t old_origin;
		VectorMA(origin, WATERSPLASH_TRACE_DIST, dir, old_origin);

		trace_t trace;
		gi.trace(origin, vec3_origin, vec3_origin, old_origin, player, MASK_WATER, &trace);

		//TODO: different exit splashes for lava and slime.
		if (!trace.startsolid && trace.fraction < 1.0f)
			gi.CreateEffect(NULL, FX_WATER_ENTRYSPLASH, CEF_FLAG7, trace.endpos, "bd", 128 | 96, trace.plane.normal); // FIXME: Size propn. to exit velocity.
	}
	else
	{
		//mxd. Store position when we weren't breaking water surface.
		VectorCopy(player->s.origin, player->client->playerinfo.LastWatersplashPos);
	}

	//TODO: move 'Handle lava sizzle damage' block here, abort when in lava?

	// Start a waterwake effect if the current player has been in water and still is in water.
	if (old_waterlevel > 0 && waterlevel > 0 && waterlevel < 3 && VectorLength(player->velocity) != 0.0f)
	{
		// No ripples while in cinematics.
		if (SV_CINEMATICFREEZE) //TODO: shouldn't this check be at the beginning of the function?
			return;

		if ((int)(player->client->bobtime + bob_move) != bob_cycle) //TODO: skip when in lava.
		{
			// FIXME: Doing more work than we need to here???
			// How about re-writing this so that it is always active on the client and does water tests itself?
			// We'll see - currently not enough info is available on the client to try this.
			vec3_t dir;
			VectorNormalize2(player->velocity, dir);

			vec3_t angles;
			vectoangles(dir, angles);

			const vec3_t origin = VEC3_INITA(player->s.origin, 0.0f, 0.0f, player->client->playerinfo.waterheight);

			const byte b_yaw = (byte)((angles[YAW] + 180.0f) * DEG_TO_BYTEANGLE);
			gi.CreateEffect(NULL, FX_WATER_WAKE, 0, origin, "sbv", player->s.number, b_yaw, player->velocity);
		}
	}

	// Check for head just coming out of water.
	if (old_waterlevel == 3 && waterlevel != 3)
	{
		if (player->air_finished < level.time)
			gi.sound(player, CHAN_BODY, gi.soundindex(va("*gasp%i.wav", irand(1, 2))), 1.0f, ATTN_NORM, 0.0f); // Gasp for air.
		else if (player->air_finished < level.time + 11)
			gi.sound(player, CHAN_BODY, gi.soundindex("*waterresurface.wav"), 1.0f, ATTN_NORM, 0.0f); // Broke surface, low on air.
	}

	// Handle drowning.
	if (waterlevel == 3)
	{
		if (player->watertype & CONTENTS_SLIME)
		{
			// Slime should kill really quick.
			player->dmg = 25;

			// Play a gurp sound instead of a normal pain sound.
			gi.sound(player, CHAN_VOICE, gi.soundindex(va("*drowning%i.wav", irand(1, 2))), 1.0f, ATTN_NORM, 0.0f);
			player->pain_debounce_time = level.time;

			T_Damage(player, world, world, vec3_origin, player->s.origin, vec3_origin, player->dmg, 0, DAMAGE_SUFFOCATION, MOD_SLIME);
		}
		else if (player->air_finished + player->client->playerinfo.lungs_timer < level.time)
		{
			// If out of air, start drowning.
			if (player->client->next_drown_time < level.time && player->health > 0)
			{
				player->client->next_drown_time = level.time + 1.0f;

				// Take more damage the longer underwater.
				player->dmg = min(15, player->dmg + 2);

				// Play a gurp sound instead of a normal pain sound.
				gi.sound(player, CHAN_VOICE, gi.soundindex(va("*drowning%i.wav", irand(1, 2))), 1.0f, ATTN_NORM, 0.0f);

				player->pain_debounce_time = level.time;

				T_Damage(player, world, world, vec3_origin, player->s.origin, vec3_origin, player->dmg, 0, DAMAGE_SUFFOCATION, MOD_WATER);
			}

		}
		else
		{
			// We aren't drowning yet, but we may well need to decrement the amount of extra lungs we have from shrines.

			// Since we still have lungs, reset air finished till we don't anymore.
			if (player->client->playerinfo.lungs_timer > 0.0f)
			{
				player->client->playerinfo.lungs_timer -= 0.1f;
				player->air_finished = level.time + HOLD_BREATH_TIME;

				// Floating point inaccuracy never lets us equal zero by ourselves.
				if (player->client->playerinfo.lungs_timer < 0.1f)
					player->client->playerinfo.lungs_timer = 0.0f;
			}
		}

		// We weren't underwater before this, so play a submerged sound. //TODO: don't play when submerged in lava!
		if (old_waterlevel != 3)
			gi.sound(player, CHAN_VOICE, gi.soundindex("player/underwater.wav"), 1.0f, ATTN_IDLE, 0.0f);
		else if ((int)(level.time / 4.0f) * 4 == (int)level.time) // Play an underwater sound every 4 seconds! //BUGFIX, kinda: mxd. Separate if case in original logic.
			gi.sound(player, CHAN_BODY, gi.soundindex("player/underwater.wav"), 1.0f, ATTN_IDLE, 0.0f); // Play local only?
	}
	else
	{
		player->air_finished = level.time + HOLD_BREATH_TIME;
		player->dmg = 2;
	}

	// Handle lava sizzle damage.
	if (waterlevel > 0 && (player->watertype & CONTENTS_LAVA))
	{
		if (player->health > 0 && player->pain_debounce_time <= level.time)
		{
			gi.sound(player, CHAN_VOICE, gi.soundindex("player/lavadamage.wav"), 1.0f, ATTN_NORM, 0.0f);
			player->pain_debounce_time = level.time + 1.0f;
		}

		const int damage = (waterlevel > 2 ? 25 : waterlevel * 3); //mxd
		T_Damage(player, world, world, vec3_origin, player->s.origin, vec3_origin, damage, 0, DAMAGE_LAVA, MOD_LAVA);
	}
}

// Called for each player at the end of the server frame and right after spawning.
void ClientEndServerFrame(edict_t* player)
{
	gclient_t* cl = player->client;

	// When in intermission, don't give the player any normal movement attributes.
	if (level.intermissiontime > 0.0f)
	{
		cl->ps.fov = FOV_DEFAULT; //mxd. Use define.
		G_SetStats(player);

		return;
	}

	// Apply world effect, e.g. burn from lava, etc.
	P_WorldEffects(player);

	// Set the player entity's model angles.
	if (player->dead_state == DEAD_NO)
	{
		// PITCH.
		if (cl->ps.pmove.w_flags & (WF_DIVING | WF_SWIMFREE))
		{
			if (cl->v_angle[PITCH] > 180.0f)
				player->s.angles[PITCH] = -(cl->v_angle[PITCH] - 360.0f);
			else
				player->s.angles[PITCH] = -cl->v_angle[PITCH];
		}
		else
		{
			player->s.angles[PITCH] = 0.0f;
		}

		// YAW and ROLL.
		player->s.angles[YAW] = cl->v_angle[YAW];
		player->s.angles[ROLL] = 0.0f;
	}

	// Handle calcs for cyclic effects like walking / swimming.
	const float xy_speed = sqrtf(player->velocity[0] * player->velocity[0] + player->velocity[1] * player->velocity[1]);

	if (xy_speed < 5.0f)
	{
		// Start at beginning of cycle again.
		bob_move = 0.0f;
		cl->bobtime = 0.0f;
	}
	else if (player->groundentity != NULL && player->waterlevel == 0)
	{
		// So bobbing only cycles when on ground.
		if (xy_speed > 210.0f)
			bob_move = 0.25f;
		else if (xy_speed > 100.0f)
			bob_move = 0.125f;
		else
			bob_move = 0.0625f;
	}
	else if (player->waterlevel > 0)
	{
		// So bobbing only cycles when in water.
		if (xy_speed > 100.0f)
			bob_move = 1.0f;
		else if (xy_speed > 50.0f)
			bob_move = 0.5f;
		else
			bob_move = 0.25f;
	}

	cl->bobtime += bob_move;
	bob_cycle = (int)cl->bobtime;

	// Calculate damage (if any) from hitting the floor and apply the damage taken this frame from ALL sources.
	SetupPlayerinfo(player);

	P_PlayerFallingDamage(&cl->playerinfo);
	P_DamageFeedback(player);

	WritePlayerinfo(player);

	// Generate client-side status display data.
	G_SetStats(player);

	// Handle player animation.
	SetupPlayerinfo(player);

	P_PlayerUpdateCmdFlags(&cl->playerinfo);
	P_PlayerUpdate(&cl->playerinfo);
	P_AnimUpdateFrame(&cl->playerinfo);
	PlayerTimerUpdate(player);

	WritePlayerinfo(player);

	// Save velocity and viewangles away for use next game frame.
	VectorCopy(player->velocity, cl->playerinfo.oldvelocity);
	VectorCopy(cl->ps.viewangles, cl->oldviewangles);

	// If the deathmatch scoreboard is up then update it.
	if (cl->playerinfo.showscores && DEATHMATCH && !(level.framenum & 31))
	{
		DeathmatchScoreboardMessage(false);
		gi.unicast(player, false);
	}

	// Reflect remote camera views(s) in the client's playerstate.
	if (cl->RemoteCameraLockCount > 0)
		cl->ps.remote_id = cl->RemoteCameraNumber;
	else
		cl->ps.remote_id = REMOTE_ID_NONE; //mxd. Use define.

	// Reflect inventory changes in the client's playetstate.
	cl->ps.NoOfItems = 0;
	int items_count = 0;

	for (int i = 0; i < MAX_ITEMS; i++)
	{
		if (cl->playerinfo.pers.inventory.Items[i] != cl->playerinfo.pers.old_inventory.Items[i])
		{
			cl->ps.inventory_changes[items_count] = (byte)i;
			cl->ps.inventory_remaining[items_count] = (byte)cl->playerinfo.pers.inventory.Items[i];
			cl->playerinfo.pers.old_inventory.Items[i] = cl->playerinfo.pers.inventory.Items[i];

			items_count++;
		}
	}

	cl->ps.NoOfItems = (byte)items_count;

	// Reflect changes to the client's origin and velocity due to the current player animation, in the client's playerstate.
	for (int i = 0; i < 3; i++)
	{
		cl->ps.pmove.origin[i] = POS2SHORT(player->s.origin[i]); //mxd. Use define.
		cl->ps.pmove.velocity[i] = POS2SHORT(player->velocity[i]); //mxd. Use define.
	}

	// Reflect viewheight changes in client's playerstate.
	cl->ps.viewheight = (short)player->viewheight;

	// Write all the shit that animation system modifies out to the playerstate (for prediction).
	VectorCopy(cl->playerinfo.mins, cl->ps.mins);
	VectorCopy(cl->playerinfo.maxs, cl->ps.maxs);

	cl->ps.NonNullgroundentity = (byte)(cl->playerinfo.groundentity != NULL ? 1 : 0);
	cl->ps.GroundPlane = cl->playerinfo.GroundPlane;
	cl->ps.GroundContents = cl->playerinfo.GroundContents;
	cl->ps.GroundSurface.flags = ((cl->playerinfo.GroundSurface != NULL) ? cl->playerinfo.GroundSurface->flags : 0);

	cl->ps.watertype = cl->playerinfo.watertype;
	cl->ps.waterlevel = cl->playerinfo.waterlevel;
	cl->ps.waterheight = cl->playerinfo.waterheight;

	VectorCopy(cl->playerinfo.grabloc, cl->ps.grabloc);
	cl->ps.grabangle = cl->playerinfo.grabangle;

	cl->ps.fwdvel = cl->playerinfo.fwdvel;
	cl->ps.sidevel = cl->playerinfo.sidevel;
	cl->ps.upvel = cl->playerinfo.upvel;

	cl->ps.flags = cl->playerinfo.flags;

	cl->ps.edictflags = cl->playerinfo.edictflags;

	cl->ps.oldvelocity_z = cl->playerinfo.oldvelocity[2];

	cl->ps.upperseq = cl->playerinfo.upperseq;
	cl->ps.lowerseq = cl->playerinfo.lowerseq;

	cl->ps.upperframe = cl->playerinfo.upperframe;
	cl->ps.lowerframe = cl->playerinfo.lowerframe;

	cl->ps.upperidle = (byte)(cl->playerinfo.upperidle ? 1 : 0);
	cl->ps.loweridle = (byte)(cl->playerinfo.loweridle ? 1 : 0);

	cl->ps.uppermove_index = cl->playerinfo.uppermove_index;
	cl->ps.lowermove_index = cl->playerinfo.lowermove_index;

	cl->ps.weapon = (byte)ITEM_INDEX(cl->playerinfo.pers.weapon);
	cl->ps.defense = (byte)ITEM_INDEX(cl->playerinfo.pers.defence);
	cl->ps.lastweapon = (byte)ITEM_INDEX(cl->playerinfo.pers.lastweapon);
	cl->ps.lastdefense = (byte)ITEM_INDEX(cl->playerinfo.pers.lastdefence);
	cl->ps.weaponready = (byte)cl->playerinfo.pers.weaponready;
	cl->ps.switchtoweapon = (byte)cl->playerinfo.switchtoweapon;
	cl->ps.newweapon = (byte)ITEM_INDEX(cl->playerinfo.pers.newweapon);
	cl->ps.weap_ammo_index = (byte)cl->playerinfo.weap_ammo_index;
	cl->ps.def_ammo_index = (byte)cl->playerinfo.def_ammo_index;
	cl->ps.weaponcharge = (byte)cl->playerinfo.weaponcharge;
	cl->ps.armortype = cl->playerinfo.pers.armortype;
	cl->ps.bowtype = cl->playerinfo.pers.bowtype;
	cl->ps.stafflevel = cl->playerinfo.pers.stafflevel;
	cl->ps.helltype = cl->playerinfo.pers.helltype;
	cl->ps.meteor_count = cl->playerinfo.meteor_count;
	cl->ps.handfxtype = cl->playerinfo.pers.handfxtype;
	cl->ps.plaguelevel = cl->playerinfo.plaguelevel;
	cl->ps.skintype = (byte)cl->playerinfo.pers.skintype;
	cl->ps.altparts = (byte)cl->playerinfo.pers.altparts;
	cl->ps.deadflag = cl->playerinfo.deadflag;
	cl->ps.ideal_yaw = player->ideal_yaw;
	cl->ps.leveltime = level.time;
	cl->ps.idletime = cl->playerinfo.idletime;
	cl->ps.quickturnEndTime = cl->playerinfo.quickturnEndTime;
	cl->ps.powerup_timer = cl->playerinfo.powerup_timer;
	cl->ps.quickturn_rate = cl->playerinfo.quickturn_rate;

	cl->ps.dmflags = cl->playerinfo.dmflags;
	cl->ps.advancedstaff = (byte)cl->playerinfo.advancedstaff;

	cl->ps.cinematicfreeze = (byte)cl->playerinfo.sv_cinematicfreeze;
}