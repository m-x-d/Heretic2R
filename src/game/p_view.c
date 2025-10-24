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

static edict_t* current_player;
static gclient_t* current_client;

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

static void P_WorldEffects(void)
{
	if (current_player->client->playerinfo.deadflag != DEAD_NO)
		return;

	// If we are in no clip, we don't need air.
	if (current_player->movetype == PHYSICSTYPE_NOCLIP)
	{
		current_player->air_finished = level.time + HOLD_BREATH_TIME;
		return;
	}

	const int waterlevel = current_player->waterlevel;
	const int old_waterlevel = current_client->old_waterlevel;
	current_client->old_waterlevel = waterlevel;

	// If the current player just entered a water volume, play a sound and start a water-ripple client effect.
	if (old_waterlevel == 0 && waterlevel > 0)
	{
		// Clear damage_debounce_time, so the pain sound will play immediately.
		current_player->damage_debounce_time = level.time - 1.0f;

		if (current_player->watertype & CONTENTS_LAVA)
		{
			gi.sound(current_player, CHAN_BODY, gi.soundindex("player/inlava.wav"), 1.0f, ATTN_NORM, 0.0f);
			current_player->flags |= FL_INLAVA;
		}
		else if (current_player->watertype & CONTENTS_SLIME)
		{
			gi.sound(current_player, CHAN_BODY, gi.soundindex("player/muckin.wav"), 1.0f, ATTN_NORM, 0.0f);
			current_player->flags |= FL_INSLIME;
		}
		else
		{
			gi.sound(current_player, CHAN_BODY, gi.soundindex("player/Water Enter.wav"), 1.0f, ATTN_NORM, 0.0f);
		}

		current_player->flags |= FL_INWATER;

		vec3_t origin;
		VectorCopy(current_player->s.origin, origin);
		origin[2] += current_player->client->playerinfo.waterheight;

		VectorCopy(origin, current_player->client->playerinfo.LastWatersplashPos);

		//TODO: different entry splashes for lava and slime.
		// Fixme: Need to determine the actual water surface normal - if we have any sloping water??
		gi.CreateEffect(NULL, FX_WATER_ENTRYSPLASH, CEF_FLAG7, origin, "bd", 128 | 96, vec3_up); // FIXME: Size propn. to entry velocity?
	}
	else if (old_waterlevel > 0 && waterlevel == 0)
	{
		// If the current player just completely exited a water volume, play a sound.

		// FL_INWATER is set whether in lava, slime or water.
		if (current_player->flags & FL_INLAVA)
		{
			gi.sound(current_player, CHAN_BODY, gi.soundindex("player/inlava.wav"), 1.0f, ATTN_NORM, 0.0f);
			current_player->flags &= ~FL_INLAVA;
		}
		else if (current_player->flags & FL_INSLIME)
		{
			gi.sound(current_player, CHAN_BODY, gi.soundindex("player/muckexit.wav"), 1.0f, ATTN_NORM, 0.0f);
			current_player->flags &= ~FL_INSLIME;
		}
		else
		{
			gi.sound(current_player, CHAN_BODY, gi.soundindex("player/Water Exit.wav"), 1.0f, ATTN_NORM, 0.0f);
		}

		current_player->flags &= ~FL_INWATER;

		vec3_t origin;
		VectorCopy(current_player->s.origin, origin);
		origin[2] = current_player->client->playerinfo.LastWatersplashPos[2];

		VectorCopy(origin, current_player->client->playerinfo.LastWatersplashPos);

		//TODO: different entry splashes for lava and slime.
		// Fixme: Need to determine the actual water surface normal - if we have any sloping water??
		gi.CreateEffect(NULL, FX_WATER_ENTRYSPLASH, 0, origin, "bd", 96, vec3_up); // FIXME: Size propn. to exit velocity.
	}

	//TODO: move 'Handle lava sizzle damage' block here, abort when in lava?

	// Start a waterwake effect if the current player has been in water and still is in water.
	if (waterlevel > 0 && (old_waterlevel > 1 && waterlevel < 3) && VectorLength(current_player->velocity) != 0.0f)
	{
		// No ripples while in cinematics.
		if (SV_CINEMATICFREEZE) //TODO: shouldn't this check be at the beginning of the function?
			return;

		if ((int)(current_client->bobtime + bob_move) != bob_cycle) //TODO: skip when in lava.
		{
			// FIXME: Doing more work then we need to here????
			// How about re-writing this so that it is always active on the client and does water tests itself?
			// We'll see - currently not enough info is available on the client to try this.
			vec3_t temp;
			VectorCopy(current_player->velocity, temp);
			VectorNormalize(temp);

			vec3_t angles;
			vectoangles(temp, angles);

			vec3_t origin;
			VectorCopy(current_player->s.origin, origin);
			origin[2] += current_player->client->playerinfo.waterheight;

			const byte angle_byte = (byte)((angles[YAW] + DEGREE_180) / 360.0f * 255.0f);
			gi.CreateEffect(NULL, FX_WATER_WAKE, 0, origin, "sbv", current_player->s.number, angle_byte, current_player->velocity);
		}
	}

	// Check for head just coming out of water.
	if (old_waterlevel == 3 && waterlevel != 3)
	{
		if (current_player->air_finished < level.time)
			gi.sound(current_player, CHAN_BODY, gi.soundindex(va("*gasp%i.wav", irand(1, 2))), 1.0f, ATTN_NORM, 0.0f); // Gasp for air.
		else if (current_player->air_finished < level.time + 11)
			gi.sound(current_player, CHAN_BODY, gi.soundindex("*waterresurface.wav"), 1.0f, ATTN_NORM, 0.0f); // Broke surface, low on air.
	}

	// Handle drowning.
	if (waterlevel == 3)
	{
		if (current_player->watertype & CONTENTS_SLIME)
		{
			// Slime should kill really quick.
			current_player->dmg = 25;

			// Play a gurp sound instead of a normal pain sound.
			gi.sound(current_player, CHAN_VOICE, gi.soundindex(va("*drowning%i.wav", irand(1, 2))), 1.0f, ATTN_NORM, 0.0f);
			current_player->pain_debounce_time = level.time;

			T_Damage(current_player, world, world, vec3_origin, current_player->s.origin, vec3_origin, current_player->dmg, 0, DAMAGE_SUFFOCATION, MOD_SLIME);
		}
		else if (current_player->air_finished + current_player->client->playerinfo.lungs_timer < level.time)
		{
			// If out of air, start drowning.
			if (current_player->client->next_drown_time < level.time && current_player->health > 0)
			{
				current_player->client->next_drown_time = level.time + 1.0f;

				// Take more damage the longer underwater.
				current_player->dmg = min(15, current_player->dmg + 2);

				// Play a gurp sound instead of a normal pain sound.
				gi.sound(current_player, CHAN_VOICE, gi.soundindex(va("*drowning%i.wav", irand(1, 2))), 1.0f, ATTN_NORM, 0.0f);

				current_player->pain_debounce_time = level.time;

				T_Damage(current_player, world, world, vec3_origin, current_player->s.origin, vec3_origin, current_player->dmg, 0, DAMAGE_SUFFOCATION, MOD_WATER);
			}

		}
		else
		{
			// We aren't drowning yet, but we may well need to decrement the amount of extra lungs we have from shrines.

			// Since we still have lungs, reset air finished till we don't anymore.
			if (current_player->client->playerinfo.lungs_timer > 0.0f)
			{
				current_player->client->playerinfo.lungs_timer -= 0.1f;
				current_player->air_finished = level.time + HOLD_BREATH_TIME;

				// Floating point inaccuracy never lets us equal zero by ourselves
				if (current_player->client->playerinfo.lungs_timer < 0.1f)
					current_player->client->playerinfo.lungs_timer = 0.0f;
			}
		}

		// We weren't underwater before this, so play a submerged sound. //TODO: don't play when submerged in lava!
		if (old_waterlevel != 3)
			gi.sound(current_player, CHAN_VOICE, gi.soundindex("player/underwater.wav"), 1.0f, ATTN_IDLE, 0.0f);
		else if ((int)(level.time / 4.0f) * 4 == (int)level.time) // Play an underwater sound every 4 seconds! //BUGFIX, kinda: mxd. Separate if case in original logic.
			gi.sound(current_player, CHAN_BODY, gi.soundindex("player/underwater.wav"), 1.0f, ATTN_IDLE, 0.0f); // Play local only?
	}
	else
	{
		current_player->air_finished = level.time + HOLD_BREATH_TIME;
		current_player->dmg = 2;
	}

	// Handle lava sizzle damage.
	if (waterlevel > 0 && (current_player->watertype & CONTENTS_LAVA))
	{
		if (current_player->health > 0 && current_player->pain_debounce_time <= level.time)
		{
			gi.sound(current_player, CHAN_VOICE, gi.soundindex("player/lavadamage.wav"), 1.0f, ATTN_NORM, 0.0f);
			current_player->pain_debounce_time = level.time + 1.0f;
		}

		const int damage = (waterlevel > 2 ? 25 : waterlevel * 3); //mxd
		T_Damage(current_player, world, world, vec3_origin, current_player->s.origin, vec3_origin, damage, 0, DAMAGE_LAVA, MOD_LAVA);
	}
}

// Called for each player at the end of the server frame and right after spawning.
void ClientEndServerFrame(edict_t* ent)
{
	current_player = ent;
	current_client = ent->client;

	// When in intermission, don't give the player any normal movement attributes.
	if (level.intermissiontime > 0.0f)
	{
		current_client->ps.fov = 75.0f;
		G_SetStats(ent);

		return;
	}

	// Apply world effect, e.g. burn from lava, etc.
	P_WorldEffects();

	// Set the player entity's model angles.
	if (ent->dead_state == DEAD_NO)
	{
		// PITCH.
		if (ent->client->ps.pmove.w_flags & (WF_DIVING | WF_SWIMFREE))
		{
			if (ent->client->v_angle[PITCH] > 180.0f)
				ent->s.angles[PITCH] = -(-360.0f + ent->client->v_angle[PITCH]);
			else
				ent->s.angles[PITCH] = -ent->client->v_angle[PITCH];
		}
		else
		{
			ent->s.angles[PITCH] = 0.0f;
		}

		// YAW and ROLL.
		ent->s.angles[YAW] = ent->client->v_angle[YAW];
		ent->s.angles[ROLL] = 0.0f;
	}

	// Handle calcs for cyclic effects like walking / swimming.
	const float xy_speed = sqrtf(ent->velocity[0] * ent->velocity[0] + ent->velocity[1] * ent->velocity[1]);

	if (xy_speed < 5.0f)
	{
		// Start at beginning of cycle again.
		bob_move = 0.0f;
		current_client->bobtime = 0.0f;
	}
	else if (ent->groundentity != NULL && current_player->waterlevel == 0)
	{
		// So bobbing only cycles when on ground.
		if (xy_speed > 210.0f)
			bob_move = 0.25f;
		else if (xy_speed > 100.0f)
			bob_move = 0.125f;
		else
			bob_move = 0.0625f;
	}
	else if (current_player->waterlevel > 0)
	{
		// So bobbing only cycles when in water.
		if (xy_speed > 100.0f)
			bob_move = 1.0f;
		else if (xy_speed > 50.0f)
			bob_move = 0.5f;
		else
			bob_move = 0.25f;
	}

	current_client->bobtime += bob_move;
	bob_cycle = (int)current_client->bobtime;

	// Calculate damage (if any) from hitting the floor and apply the damage taken this frame from ALL sources.
	SetupPlayerinfo(ent);

	P_PlayerFallingDamage(&ent->client->playerinfo);
	P_DamageFeedback(ent);

	WritePlayerinfo(ent);

	// Generate client-side status display data.
	G_SetStats(ent);

	// Handle player animation.
	SetupPlayerinfo(ent);

	P_PlayerUpdateCmdFlags(&ent->client->playerinfo);
	P_PlayerUpdate(&ent->client->playerinfo);
	P_AnimUpdateFrame(&ent->client->playerinfo);
	PlayerTimerUpdate(ent);

	WritePlayerinfo(ent);

	// Save velocity and viewangles away for use next game frame.
	VectorCopy(ent->velocity, ent->client->playerinfo.oldvelocity);
	VectorCopy(ent->client->ps.viewangles, ent->client->oldviewangles);

	// If the deathmatch scoreboard is up then update it.
	if (ent->client->playerinfo.showscores && DEATHMATCH && !(level.framenum & 31))
	{
		DeathmatchScoreboardMessage(false);
		gi.unicast(ent, false);
	}

	// Reflect remote camera views(s) in the client's playerstate.
	if (current_client->RemoteCameraLockCount > 0)
		current_client->ps.remote_id = current_client->RemoteCameraNumber;
	else
		current_client->ps.remote_id = -1;

	// Reflect inventory changes in the client's playetstate.
	current_client->ps.NoOfItems = 0;
	int items_count = 0;

	for (int i = 0; i < MAX_ITEMS; i++)
	{
		if (current_client->playerinfo.pers.inventory.Items[i] != current_client->playerinfo.pers.old_inventory.Items[i])
		{
			current_client->ps.inventory_changes[items_count] = (byte)i;
			current_client->ps.inventory_remaining[items_count] = (byte)current_client->playerinfo.pers.inventory.Items[i];
			current_client->playerinfo.pers.old_inventory.Items[i] = current_client->playerinfo.pers.inventory.Items[i];

			items_count++;
		}
	}

	current_client->ps.NoOfItems = (byte)items_count;

	// Reflect changes to the client's origin and velocity due to the current player animation, in the client's playerstate.
	for (int i = 0; i < 3; i++)
	{
		current_client->ps.pmove.origin[i] = (short)(ent->s.origin[i] * 8.0f);
		current_client->ps.pmove.velocity[i] = (short)(ent->velocity[i] * 8.0f);
	}

	// Reflect viewheight changes in client's playerstate.
	current_client->ps.viewheight = (short)ent->viewheight;

	// Write all the shit that animation system modifies out to the playerstate (for prediction).
	VectorCopy(current_client->playerinfo.mins, current_client->ps.mins);
	VectorCopy(current_client->playerinfo.maxs, current_client->ps.maxs);

	current_client->ps.NonNullgroundentity = (byte)(current_client->playerinfo.groundentity != NULL ? 1 : 0);
	current_client->ps.GroundPlane = current_client->playerinfo.GroundPlane;
	current_client->ps.GroundContents = current_client->playerinfo.GroundContents;
	current_client->ps.GroundSurface.flags = (current_client->playerinfo.GroundSurface != NULL) ? current_client->playerinfo.GroundSurface->flags : 0;

	current_client->ps.watertype = current_client->playerinfo.watertype;
	current_client->ps.waterlevel = current_client->playerinfo.waterlevel;
	current_client->ps.waterheight = current_client->playerinfo.waterheight;

	VectorCopy(current_client->playerinfo.grabloc, current_client->ps.grabloc);
	current_client->ps.grabangle = current_client->playerinfo.grabangle;

	current_client->ps.fwdvel = current_client->playerinfo.fwdvel;
	current_client->ps.sidevel = current_client->playerinfo.sidevel;
	current_client->ps.upvel = current_client->playerinfo.upvel;

	current_client->ps.flags = current_client->playerinfo.flags;

	current_client->ps.edictflags = current_client->playerinfo.edictflags;

	current_client->ps.oldvelocity_z = current_client->playerinfo.oldvelocity[2];

	current_client->ps.upperseq = current_client->playerinfo.upperseq;
	current_client->ps.lowerseq = current_client->playerinfo.lowerseq;

	current_client->ps.upperframe = current_client->playerinfo.upperframe;
	current_client->ps.lowerframe = current_client->playerinfo.lowerframe;

	current_client->ps.upperidle = (byte)(current_client->playerinfo.upperidle ? 1 : 0);
	current_client->ps.loweridle = (byte)(current_client->playerinfo.loweridle ? 1 : 0);

	current_client->ps.uppermove_index = current_client->playerinfo.uppermove_index;
	current_client->ps.lowermove_index = current_client->playerinfo.lowermove_index;

	current_client->ps.weapon = (byte)ITEM_INDEX(current_client->playerinfo.pers.weapon);
	current_client->ps.defense = (byte)ITEM_INDEX(current_client->playerinfo.pers.defence);
	current_client->ps.lastweapon = (byte)ITEM_INDEX(current_client->playerinfo.pers.lastweapon);
	current_client->ps.lastdefense = (byte)ITEM_INDEX(current_client->playerinfo.pers.lastdefence);
	current_client->ps.weaponready = (byte)current_client->playerinfo.pers.weaponready;
	current_client->ps.switchtoweapon = (byte)current_client->playerinfo.switchtoweapon;
	current_client->ps.newweapon = (byte)ITEM_INDEX(current_client->playerinfo.pers.newweapon);
	current_client->ps.weap_ammo_index = (byte)current_client->playerinfo.weap_ammo_index;
	current_client->ps.def_ammo_index = (byte)current_client->playerinfo.def_ammo_index;
	current_client->ps.weaponcharge = (byte)current_client->playerinfo.weaponcharge;
	current_client->ps.armortype = current_client->playerinfo.pers.armortype;
	current_client->ps.bowtype = current_client->playerinfo.pers.bowtype;
	current_client->ps.stafflevel = current_client->playerinfo.pers.stafflevel;
	current_client->ps.helltype = current_client->playerinfo.pers.helltype;
	current_client->ps.meteor_count = current_client->playerinfo.meteor_count;
	current_client->ps.handfxtype = current_client->playerinfo.pers.handfxtype;
	current_client->ps.plaguelevel = current_client->playerinfo.plaguelevel;
	current_client->ps.skintype = (byte)current_client->playerinfo.pers.skintype;
	current_client->ps.altparts = (byte)current_client->playerinfo.pers.altparts;
	current_client->ps.deadflag = current_client->playerinfo.deadflag;
	current_client->ps.ideal_yaw = ent->ideal_yaw;
	current_client->ps.leveltime = level.time;
	current_client->ps.idletime = current_client->playerinfo.idletime;
	current_client->ps.quickturnEndTime = current_client->playerinfo.quickturnEndTime;
	current_client->ps.powerup_timer = current_client->playerinfo.powerup_timer;
	current_client->ps.quickturn_rate = current_client->playerinfo.quickturn_rate;

	current_client->ps.dmflags = current_client->playerinfo.dmflags;
	current_client->ps.advancedstaff = (byte)current_client->playerinfo.advancedstaff;

	current_client->ps.cinematicfreeze = (byte)current_client->playerinfo.sv_cinematicfreeze;
}