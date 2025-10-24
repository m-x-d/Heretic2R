//
// p_morph.c -- mxd. Corvus <-> Chicken transmutation logic. Originally part of spl_morph.c
//
// Copyright 1998 Raven Software
//

#include "p_morph.h"
#include "g_Physics.h"
#include "g_playstats.h"
#include "g_Shrine.h" //mxd
#include "g_Skeletons.h"
#include "m_chicken_anim.h"
#include "p_main.h"
#include "p_anims.h"
#include "p_client.h" //mxd
#include "p_teleport.h" //mxd
#include "FX.h"
#include "Random.h"
#include "Vector.h"
#include "g_local.h"

// Morphing is done in 3 steps:
// 1: Spawn teleport sfx, start fading out current player model.
// 2: After fade-out is finished, swap to target player model and start fading it in.
// 3: Cleanup morph-related player settings.

// Done morphing, clean up after ourselves - for PLAYER only.
void CleanUpPlayerMorph(edict_t* self) //mxd. Named 'CleanUpMorph' in original version.
{
	VectorClear(self->client->tele_dest);

	self->client->tele_count = 0;
	self->client->playerinfo.edictflags &= ~FL_LOCKMOVE;
	self->client->playerinfo.renderfx &= ~RF_TRANSLUCENT;
	self->client->playerinfo.flags &= ~PLAYER_FLAG_MORPHING;
	self->client->playerinfo.pm_flags &= ~(PMF_LOCKMOVE | PMF_LOCKANIM); //mxd
	self->client->shrine_framenum = level.time - 1.0f;

	self->s.color.a = 255;
}

#pragma region ========================== Chicken -> Corvus morph ==========================

// We are done being a chicken, let's be Corvus again - switch models from chicken back to Corvus and do teleport fade in - for PLAYER only.
void MorphChickenToPlayerEnd(edict_t* self) //mxd. Named 'reset_morph_to_elf' in original version.
{
	// We have no damage, and no motion type.
	self->takedamage = DAMAGE_AIM;
	self->movetype = PHYSICSTYPE_STEP;
	self->health = self->max_health;

	// Move the camera back to where it should be, and reset our lungs and stuff.
	self->viewheight = 0;
	self->mass = 200;
	self->dead_state = DEAD_NO;
	self->air_finished = level.time + HOLD_BREATH_TIME;

	self->s.scale = 1.0f;

	// Set the model back to Corvus.
	self->model = "players/male/tris.fm"; //mxd. "models/player/corvette/tris.fm" in original logic. //TODO: unused by player model logic?
	self->pain = PlayerPain;
	self->die = PlayerDie;
	self->flags &= ~FL_NO_KNOCKBACK;
	self->gravity = 1.0f;

	// Reset our skins.
	self->client->playerinfo.effects = 0;
	self->client->playerinfo.skinnum = 0;
	self->client->playerinfo.clientnum = self - g_edicts - 1;
	self->s.modelindex = 255; // Will use the skin-specified model.

	// Turn our skeleton back on.
	self->s.skeletalType = SKEL_CORVUS;
	self->client->playerinfo.effects |= (EF_SWAPFRAME | EF_JOINTED | EF_CAMERA_NO_CLIP | EF_PLAYER);
	self->client->playerinfo.effects &= ~EF_CHICKEN;
	self->client->playerinfo.edictflags &= ~FL_CHICKEN;
	self->client->playerinfo.renderfx &= ~RF_IGNORE_REFS;

	// Reset our mins and maxs. And then let the physics move us out of anyone else's bounding box.
	VectorCopy(player_mins, self->intentMins);
	VectorCopy(player_maxs, self->intentMaxs);
	self->physicsFlags |= PF_RESIZE;

	// Reset our thinking.
	self->think = self->oldthink;
	self->nextthink = level.time + FRAMETIME; //mxd. Use define.

	// Reset our animations.
	P_PlayerBasicAnimReset(&self->client->playerinfo);
	self->client->playerinfo.frame = 1; //mxd. FRAME_180turn1 from m_player.h (can't include because of define overlaps with m_chicken_anim.h...).

	P_PlayerUpdateModelAttributes(&self->client->playerinfo);
	P_PlayerAnimSetLowerSeq(&self->client->playerinfo, ASEQ_IDLE_WIPE_BROW);

	// Draw the teleport splash at the destination.
	gi.CreateEffect(&self->s, FX_PLAYER_TELEPORT_IN, CEF_BROADCAST | CEF_OWNERS_ORIGIN | CEF_FLAG6, self->s.origin, "");

	// Restart the loop and tell us next time we aren't de-materializing.
	self->client->tele_count = TELE_TIME;
	VectorSet(self->client->tele_dest, -1.0f, -1.0f, -1.0f);
}

// Modify a chicken into a player - first call. Start the teleport effect on the chicken. For PLAYER only.
static void MorphChickenToPlayerStart(edict_t* self) //mxd. Named 'MorphChickenToPlayer' in original version.
{
	// If we are teleporting or morphing, forget it.
	if (self->client->playerinfo.flags & (PLAYER_FLAG_TELEPORT | PLAYER_FLAG_MORPHING))
		return;

	self->client->playerinfo.flags |= PLAYER_FLAG_MORPHING; // Set the player as morphing.
	self->client->ps.pmove.pm_flags |= (PMF_LOCKMOVE | PMF_LOCKANIM); //mxd. Actually make player stand still...

	self->client->tele_count = TELE_TIME_OUT; // Time taken over de-materialization.
	self->client->shrine_framenum = level.time + 10.0f; // Make us invulnerable for a couple of seconds.
	self->client->tele_type = 1; // Tell us how we triggered the teleport.
	self->flags |= FL_LOCKMOVE; // Make the player stand still. //TODO: sets 'self->client->playerinfo.flags' in MorphPlayerToChicken().

	// Clear the velocity and hold them in place briefly.
	VectorClear(self->velocity);
	self->client->ps.pmove.pm_time = 50;

	// Allow the player to fade out.
	self->s.color.c = 0xffffffff;
	self->s.renderfx |= RF_TRANSLUCENT;

	// Make us not think at all.
	self->think = NULL;

	// Make it so that the stuff that does the de-materialization in G_ANIM_ACTOR knows we are fading out, not in.
	VectorClear(self->client->tele_dest);

	// Draw the teleport splash at the teleport source.
	gi.CreateEffect(&self->s, FX_PLAYER_TELEPORT_OUT, CEF_OWNERS_ORIGIN | CEF_FLAG6, NULL, "");

	// Do the teleport sound.
	gi.sound(self, CHAN_WEAPON, gi.soundindex("weapons/teleport.wav"), 1.0f, ATTN_NORM, 0.0f);
}

#pragma endregion

#pragma region ========================== Corvus -> Chicken morph ==========================

// Watch the chicken to see if we should become the elf again. For PLAYER only.
static void ChickenPlayerThink(edict_t* self) //mxd. Named 'watch_chicken' in original version.
{
	// Are we done yet?
	if (self->morph_timer <= (int)level.time)
		MorphChickenToPlayerStart(self);

	self->nextthink = level.time + FRAMETIME; //mxd. Use define.
}

// Switch the models from player to chicken and then make us re-appear ala teleport. For PLAYER only.
void MorphPlayerToChickenEnd(edict_t* self) //mxd. Named 'Perform_Morph' in original version.
{
	static const vec3_t mins = { -16.0f, -16.0f, -36.0f }; //mxd. Made local static.
	static const vec3_t maxs = {  16.0f,  16.0f,  36.0f }; //mxd. Made local static.

	// Change out our model.
	self->model = "models/monsters/chicken2/tris.fm";
	self->s.modelindex = (byte)gi.modelindex("models/monsters/chicken2/tris.fm");
	self->client->playerinfo.frame = FRAME_wait1; //mxd. Set fade-in frame...

	self->client->playerinfo.effects &= ~(EF_JOINTED | EF_SWAPFRAME);
	self->client->playerinfo.effects |= EF_CHICKEN;
	self->s.skeletalType = SKEL_NULL;
	self->client->playerinfo.renderfx |= RF_IGNORE_REFS;

	qboolean super_chicken = (irand(0, 10) == 0);

	// Check if super-chicken can fit here.
	if (super_chicken)
	{
		vec3_t pos;
		VectorCopy(self->s.origin, pos);
		pos[2] += 2.0f;

		trace_t trace;
		gi.trace(pos, mins, maxs, pos, self, MASK_PLAYERSOLID, &trace);

		if (trace.fraction < 1.0f || trace.startsolid || trace.allsolid)
			super_chicken = false;
	}

	if (super_chicken)
	{
		// Reset our motion stuff.
		self->health = 999;
		self->mass = 3000;
		self->yaw_speed = 30.0f;
		self->gravity = 1.0f;

		self->monsterinfo.scale = 2.5f;
		self->s.scale = 2.5f;

		VectorSet(self->mins, -16.0f, -16.0f, -48.0f);
		VectorSet(self->maxs,  16.0f,  16.0f,  64.0f);

		self->client->playerinfo.edictflags |= FL_SUPER_CHICKEN;
	}
	else
	{
		self->health = 1;
		self->mass = 30;
		self->yaw_speed = 20.0f;
		self->gravity = 0.6f;

		self->monsterinfo.scale = MODEL_SCALE;

		// Set new mins and maxs.
		VectorSet(self->intentMins, -8.0f, -8.0f, -12.0f);
		VectorSet(self->intentMaxs,  8.0f,  8.0f,  12.0f);

		self->client->playerinfo.edictflags |= FL_AVERAGE_CHICKEN;
	}

	// Not being knocked back, and stepping like a chicken.
	self->movetype = PHYSICSTYPE_STEP;
	VectorClear(self->knockbackvel);

	// Reset which skin we use.
	self->client->playerinfo.skinnum = 0;
	self->client->playerinfo.clientnum = self - g_edicts - 1;

	// Reset our thinking.
	self->oldthink = self->think;
	self->think = ChickenPlayerThink;
	self->nextthink = level.time + FRAMETIME; //mxd. Use define.

	self->physicsFlags |= PF_RESIZE;

	for (int i = 0; i < MAX_FM_MESH_NODES; i++)
		self->client->playerinfo.fmnodeinfo[i].flags &= ~FMNI_NO_DRAW;

	// Reset our animation.
	P_PlayerAnimSetLowerSeq(&self->client->playerinfo, ASEQ_STAND);

	// Draw the teleport splash at the destination.
	gi.CreateEffect(&self->s, FX_PLAYER_TELEPORT_IN, CEF_BROADCAST | CEF_OWNERS_ORIGIN | CEF_FLAG6, self->s.origin, "");

	// Restart the loop and tell us next time we aren't de-materializing.
	self->client->tele_count = TELE_TIME;
	VectorSet(self->client->tele_dest, -1.0f, -1.0f, -1.0f);
}

// Modify a player into a chicken - first call. Start the teleport effect on the player. For PLAYER only.
void MorphPlayerToChickenStart(edict_t* self) //mxd. Named 'MorphPlayerToChicken' in original version.
{
	// If we are teleporting or morphing, forget it.
	if (self->client->playerinfo.flags & (PLAYER_FLAG_TELEPORT | PLAYER_FLAG_MORPHING))
		return;

	// Remove any hand or weapon effects.
	P_TurnOffPlayerEffects(&self->client->playerinfo);

	// Remove any shrine effects he has.
	PlayerKillShrineFX(self);

	self->client->playerinfo.flags |= PLAYER_FLAG_MORPHING; // Set the player as morphing.
	self->client->ps.pmove.pm_flags |= (PMF_LOCKMOVE | PMF_LOCKANIM); //mxd. Actually make player stand still...

	self->client->tele_count = TELE_TIME_OUT; // Time taken over de-materialization.
	self->client->shrine_framenum = level.time + 10.0f; // Make us invulnerable for a couple of seconds.
	self->client->tele_type = 1; // Tell us how we triggered the teleport.
	self->client->playerinfo.flags |= FL_LOCKMOVE; // Make the player still.

	// Clear the velocity and hold them in place briefly.
	VectorClear(self->velocity);
	self->client->ps.pmove.pm_time = 50;

	// Allow the player to fade out.
	self->s.color.c = 0xffffffff;
	self->s.renderfx |= RF_TRANSLUCENT;

	// Make it so that the stuff that does the de-materialization in G_ANIM_ACTOR knows we are fading out, not in.
	VectorClear(self->client->tele_dest);

	// Tell us how long we have to be a chicken.
	self->morph_timer = (int)level.time + MORPH_DURATION;

	// Draw the teleport splash at the teleport source.
	gi.CreateEffect(&self->s, FX_PLAYER_TELEPORT_OUT, CEF_OWNERS_ORIGIN | CEF_FLAG6, NULL, "");

	// Do the teleport sound.
	gi.sound(self, CHAN_WEAPON, gi.soundindex("weapons/teleport.wav"), 1.0f, ATTN_NORM, 0.0f);
}

#pragma endregion