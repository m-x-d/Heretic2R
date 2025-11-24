//
// p_teleport.c -- mxd. Originally part of spl_teleport.c
//
// Copyright 1998 Raven Software
//

#include "p_teleport.h"
#include "g_ai.h"
#include "g_playstats.h"
#include "p_main.h"
#include "p_client.h" //mxd
#include "FX.h"
#include "Random.h"
#include "Vector.h"

// Make the guy actually teleport from one place to another.
void PerformPlayerTeleport(edict_t* self) //mxd. Named 'Perform_Teleport' in original version.
{
	// Get the player off the rope.
	self->client->playerinfo.flags |= PLAYER_FLAG_RELEASEROPE;

	// Physically move the player, bearing in mind that's what a teleport is.
	VectorCopy(self->client->tele_dest, self->client->playerinfo.origin);
	VectorCopy(self->client->tele_dest, self->s.origin);

	// Set angles.
	for (int i = 0; i < 3; i++)
		self->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(self->client->tele_angles[i] - self->client->resp.cmd_angles[i]);

	VectorSet(self->s.angles, 0.0f, self->client->tele_angles[YAW], 0.0f);
	VectorCopy(self->client->tele_angles, self->client->ps.viewangles);
	VectorCopy(self->client->tele_angles, self->client->v_angle);

	// Reset the r_farclipdist cvar, in case it was modified by a trigger - there should be no teleport
	// destinations or spawn points anywhere where the far clip has been modified.
	gi.cvar_set("r_farclipdist", FAR_CLIP_DIST);

	// Unlink to make sure it can't possibly interfere with KillBox - we don't want to collide with ourselves.
	gi.unlinkentity(self);

	// Kill anything at the destination.
	KillBox(self);

	// Re-link us.
	gi.linkentity(self);

	// Draw the teleport splash at the destination.
	gi.CreateEffect(&self->s, FX_PLAYER_TELEPORT_IN, CEF_BROADCAST | CEF_OWNERS_ORIGIN | ((byte)self->client->tele_type << 5), NULL, "");

	// Restart the loop and tell us next time we aren't de-materializing.
	self->client->tele_count = TELE_TIME;
	VectorSet(self->client->tele_dest, -1.0f, -1.0f, -1.0f);

	AlertMonsters(self, self, 2.0f, true);
}

// Done teleporting, clean up after ourselves.
void CleanUpPlayerTeleport(edict_t* self) //mxd. Named 'CleanUpTeleport' in original version.
{
	VectorClear(self->client->tele_dest);

	self->client->tele_count = 0;
	self->flags &= ~FL_LOCKMOVE;
	self->client->playerinfo.flags &= ~PLAYER_FLAG_TELEPORT;
	self->client->playerinfo.pm_flags &= ~PMF_LOCKMOVE; //mxd. Set on 'self->client->ps.pmove.pm_flags' in original logic (not transferred to client logic);
	self->s.color.a = 255;
	self->client->shrine_framenum = level.time - 1.0f;
}

// Setup the teleporter - from the player hitting a teleport pad.
// We could send the teleport type over the flags instead of as a parameter byte.
void PlayerTeleporterTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf) //mxd. Named 'teleporter_touch' in original logic.
{
	// If we aren't a player, dead or already teleporting, forget it.
	if (other->client == NULL || (other->client->playerinfo.flags & (PLAYER_FLAG_TELEPORT | PLAYER_FLAG_MORPHING)) || (other->dead_state & (DEAD_DYING | DEAD_DEAD)))
		return;

	// If we are in deathmatch, and this teleporter is so flagged, give us a random destination.
	if (DEATHMATCH && (self->spawnflags & DEATHMATCH_RANDOM))
	{
		// Figure out a destination point.
		vec3_t dest_pos;
		vec3_t dest_angles;
		SelectSpawnPoint(other, dest_pos, dest_angles);

		VectorCopy(dest_pos, other->client->tele_dest); // Set destination coordinates to teleport to.
		VectorCopy(dest_angles, other->client->tele_angles); // Set angles we should start at.
	}
	else // We do have a specific destination in mind.
	{
		// Setup in player info the destination entity of the teleport.
		edict_t* dest = NULL;

		// Do we have multiple destinations?
		if (self->style > 0)
		{
			const int rand_targ = irand(1, self->style);

			for (int i = 0; i < rand_targ; i++)
			{
				dest = G_Find(dest, FOFS(targetname), self->target);

				if (dest == NULL)
				{
					gi.dprintf("Couldn't find multiple teleport destination %d\n", rand_targ);
					return;
				}
			}
		}
		else // No - just one target.
		{
			dest = G_Find(dest, FOFS(targetname), self->target);

			if (dest == NULL)
			{
				gi.dprintf("Couldn't find teleport destination %s\n", self->target);
				return;
			}
		}

		VectorCopy(dest->last_org, other->client->tele_dest); // Set destination coordinates to teleport to.
		VectorCopy(dest->s.angles, other->client->tele_angles); // Set angles we should start at.
	}

	// Setup other teleporter information that the character will require when the teleport is actually performed in AnimUpdateFrame.

	// Set the player as teleporting.
	other->client->playerinfo.flags |= PLAYER_FLAG_TELEPORT;
	other->client->ps.pmove.pm_flags |= (PMF_LOCKMOVE | PMF_TIME_TELEPORT); //mxd. +PMF_TIME_TELEPORT.

	other->client->tele_count = TELE_TIME_OUT; // Time taken over de-materialization.
	other->client->tele_type = 0; // Tell us how we triggered the teleport.
	other->client->old_solid = other->solid; // Save out what kind of solid ability we are.
	other->client->shrine_framenum = level.time + 10.0f; // Make us invulnerable for a couple of seconds.

	// Clear the velocity and hold them in place briefly.
	VectorClear(other->velocity);
	other->client->ps.pmove.pm_time = TELE_PM_DURATION; //mxd. 50 in original logic.

	other->flags |= FL_LOCKMOVE; // Make the player still.
	other->s.color.c = 0xffffffff; // Allow the player to fade out.

	// Draw the teleport splash at the teleport source.
	gi.CreateEffect(&other->s, FX_PLAYER_TELEPORT_OUT, CEF_OWNERS_ORIGIN | ((byte)other->client->tele_type << 5), NULL, "");

	// Do the teleport sound.
	gi.sound(other, CHAN_VOICE, gi.soundindex("weapons/teleport.wav"), 1.0f, ATTN_NORM, 0.0f);
}