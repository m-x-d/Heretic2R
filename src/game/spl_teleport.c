//
// spl_teleport.c
//
// Copyright 1998 Raven Software
//

#include "spl_teleport.h" //mxd
#include "p_main.h"
#include "p_client.h" //mxd
#include "p_teleport.h"
#include "FX.h"
#include "Vector.h"

// Spawn the Spell teleport effect - from the player.
// We could send the teleport type over the flags instead of as a parameter byte.
void SpellCastTeleport(edict_t* caster)
{
	// If we are already teleporting, forget it.
	if ((caster->client->playerinfo.flags & (PLAYER_FLAG_TELEPORT | PLAYER_FLAG_MORPHING)) || (caster->dead_state & (DEAD_DYING | DEAD_DEAD)))
		return;

	// Setup other teleporter information that the character will require when the teleport is actually performed in AnimUpdateFrame.

	// Set the player as teleporting.
	caster->client->playerinfo.flags |= PLAYER_FLAG_TELEPORT;
	caster->client->playerinfo.pm_flags |= (PMF_LOCKMOVE | PMF_LOCKANIM); //mxd. Sets 'caster->client->ps.pmove.pm_flags' in original logic (not transferred to client); +PMF_LOCKANIM;

	// Figure out a destination point.
	vec3_t dest_pos;
	vec3_t dest_angles;
	SelectSpawnPoint(caster, dest_pos, dest_angles);

	VectorCopy(dest_pos, caster->client->tele_dest); // Set destination coordinates to teleport to.
	VectorCopy(dest_angles, caster->client->tele_angles); // Set angles we should start at.

	caster->client->tele_count = TELE_TIME_OUT; // Time taken over de-materialization.
	caster->client->tele_type = 1; // Tell us how we triggered the teleport.
	caster->client->old_solid = caster->solid; // Save out what kind of solid ability we are.
	caster->client->shrine_framenum = level.time + 10.0f; // Make us invulnerable for a couple of seconds.

	// Clear the velocity and hold them in place briefly.
	VectorClear(caster->velocity);
	caster->client->ps.pmove.pm_time = 50;

	caster->flags |= FL_LOCKMOVE; // Make the player still.
	caster->s.color.c = 0xffffffff; // Allow the player to fade out.

	// Draw the teleport splash at the teleport source.
	gi.CreateEffect(&caster->s, FX_PLAYER_TELEPORT_OUT, CEF_OWNERS_ORIGIN | ((byte)caster->client->tele_type << 5), NULL, "");

	// Do the teleport sound.
	gi.sound(caster, CHAN_VOICE, gi.soundindex("weapons/teleport.wav"), 1.0f, ATTN_NORM, 0.0f);
}