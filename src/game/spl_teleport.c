//
// spl_teleport.c
//
// Heretic II
// Copyright 1998 Raven Software
//
// Written by Jake Simpson

#include "g_local.h"
#include "fx.h"
#include "Angles.h"
#include "Utilities.h"
#include "random.h"
#include "vector.h"
#include "g_teleport.h"
#include "p_main.h"
#include "g_playstats.h"

extern void	SelectSpawnPoint (edict_t *ent,vec3_t origin, vec3_t angles);
extern void AlertMonsters (edict_t *self, edict_t *enemy, float lifetime, qboolean ignore_shadows);

// PLAYER ONLY!!!
// make the guy actually teleport from one place to another
// called from ANIMACTOR
void Perform_Teleport(edict_t *self)
{
	int i;

	//Get the player off the rope
	self->client->playerinfo.flags |= PLAYER_FLAG_RELEASEROPE;
	
	//physically move the player, bearing in mind thats what a teleport is
	VectorCopy (self->client->tele_dest, self->client->playerinfo.origin);
	VectorCopy (self->client->tele_dest, self->s.origin);

	// set angles
	for (i=0 ; i<3 ; i++)
		self->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(self->client->tele_angles[i] - self->client->resp.cmd_angles[i]);

	self->s.angles[PITCH] = 0;
	self->s.angles[YAW] = self->client->tele_angles[YAW];
	self->s.angles[ROLL] = 0;
	VectorCopy (self->client->tele_angles, self->client->ps.viewangles);
	VectorCopy (self->client->tele_angles, self->client->v_angle);

	// reset the cvar Farclip dist, incase it was modified by a trigger - there should be no teleport
	// destinations or spawn points anywhere where the far clip has been modified.
	gi.cvar_set("r_farclipdist", FAR_CLIP_DIST);

	// unlink to make sure it can't possibly interfere with KillBox - we don't want to collide with ourselves
	gi.unlinkentity (self);
	
	// kill anything at the destination
	KillBox (self);

	// re link us
	gi.linkentity (self);

	// draw the teleport splash at the destination
	gi.CreateEffect(&self->s, FX_PLAYER_TELEPORT_IN, CEF_BROADCAST|CEF_OWNERS_ORIGIN | ((byte)self->client->tele_type << 5), NULL, "" );

	// restart the loop and tell us next time we aren't de-materialising
	self->client->tele_count = TELE_TIME;
	self->client->tele_dest[0] = self->client->tele_dest[1] = self->client->tele_dest[2] = -1;

	AlertMonsters (self, self, 2, true);
}

// PLAYER ONLY!!!
// Done teleporting, clean up after ourselves
// called from ANIMACTOR
void CleanUpTeleport(edict_t *self)
{
	self->client->tele_dest[0] = self->client->tele_dest[1] = self->client->tele_dest[2] = 0;
	self->client->tele_count = 0;
	self->flags &= ~FL_LOCKMOVE;
	self->client->playerinfo.flags &=~PLAYER_FLAG_TELEPORT;
	self->client->ps.pmove.pm_flags &= ~PMF_LOCKMOVE;
	self->s.color.a = 255;
	self->client->shrine_framenum = level.time - 1;;
}

// Setup the teleporter - from the player hitting a teleport pad
// We could send the teleport type over the flags instead of as a parameter byte
void teleporter_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t	dest_v, dest_v_angles;
	int	rand_targ, i;
	edict_t  *dest = NULL;
	
	// if we aren't a player, forget it
	if (!other->client)
		return;

	// if we are already teleporting, forget it

	if ((other->client->playerinfo.flags & (PLAYER_FLAG_TELEPORT | PLAYER_FLAG_MORPHING)) || (other->deadflag & (DEAD_DYING|DEAD_DEAD)))
		return;

	// if we are in deathmatch, and this teleporter is so flagged, give us a random destination
	if (deathmatch->value && (self->spawnflags & 2))
	{
		// figure out a destination point
		SelectSpawnPoint(other,dest_v, dest_v_angles);
		// dest coors to teleport to
		VectorCopy(dest_v,other->client->tele_dest);
		// angles we should start at 
		VectorCopy(dest_v_angles,other->client->tele_angles);
	}
	// we do have a specific destination in mind
	else
	{
		// setup in player info the dest entity of the teleport

		// do we have multiple destinations ?
		if (self->style)
		{
			rand_targ = irand(1,self->style);
			for (i = 0; i<rand_targ;i++)
			{
				dest = G_Find (dest, FOFS(targetname), self->target);
				if (!dest)
				{
#ifdef _DEVEL
					gi.dprintf ("Couldn't find multiple teleport destination %d\n",rand_targ);
#endif
					return;
				}
			}
		}
		// no - just the one
		else
		{
			dest = G_Find (dest, FOFS(targetname), self->target);
			if (!dest)
			{
#ifdef _DEVEL
				gi.dprintf ("Couldn't find teleport destination  %s\n",self->target);
#endif
				return;
			}
		}
		// dest coors to teleport to
		VectorCopy(dest->last_org,other->client->tele_dest);
		// angles we should start at
		VectorCopy(dest->s.angles,other->client->tele_angles);
	}


	// setup other teleporter information that the character will require
	// when the teleport is actually performed in AnimUpdateFrame

	// set the player as teleporting

	other->client->playerinfo.flags |= PLAYER_FLAG_TELEPORT;

	other->client->ps.pmove.pm_flags |= PMF_LOCKMOVE;

	// time taken over dematerialisation
	other->client->tele_count = TELE_TIME_OUT;

	// save out what kind of solid ability we are
	other->client->old_solid = other->solid;

	// make us invunerable for a couple of seconds
	other->client->shrine_framenum = level.time + 10;

	// tell us how we triggered the teleport
	other->client->tele_type = 0;

	// clear the velocity and hold them in place briefly
	VectorClear (other->velocity);
	other->client->ps.pmove.pm_time = 50;
	// make the player still
	other->flags |= FL_LOCKMOVE;
	// allow the player to fade out
	other->s.color.a = 255;
	other->s.color.r = 255;
	other->s.color.g = 255;
	other->s.color.b = 255;

	// draw the teleport splash at the teleport source
	gi.CreateEffect(&other->s, FX_PLAYER_TELEPORT_OUT, CEF_OWNERS_ORIGIN | ((byte)other->client->tele_type << 5), NULL, "" );
	// do the teleport sound
	gi.sound(other,CHAN_VOICE,gi.soundindex("weapons/teleport.wav"),1,ATTN_NORM,0);

}

// Spawn the Spell teleport effect - from the player
// We could send the teleport type over the flags instead of as a parameter byte
void SpellCastTeleport(edict_t *caster,vec3_t StartPos,vec3_t AimAngles,vec3_t AimDir,float Value)
{

	vec3_t	dest, dest_angles;

	// if we are already teleporting, forget it
	if ((caster->client->playerinfo.flags & (PLAYER_FLAG_TELEPORT | PLAYER_FLAG_MORPHING)) || (caster->deadflag & (DEAD_DYING|DEAD_DEAD)))
		return;

	// setup other teleporter information that the character will require
	// when the teleport is actually performed in AnimUpdateFrame

	// set the player as teleporting
	caster->client->playerinfo.flags |= PLAYER_FLAG_TELEPORT;

	caster->client->ps.pmove.pm_flags |= PMF_LOCKMOVE;

	// figure out a destination point
	SelectSpawnPoint(caster,dest, dest_angles);

	// dest coors to teleport to
	VectorCopy(dest,caster->client->tele_dest);
	// angles we should start at 
	VectorCopy(dest_angles,caster->client->tele_angles);
	// time taken over dematerialisation
	caster->client->tele_count = TELE_TIME_OUT;

	// tell us how we triggered the teleport
	caster->client->tele_type = 1;

	// save out what kind of solid ability we are
	caster->client->old_solid = caster->solid;

	// make us invunerable for a couple of seconds
	caster->client->shrine_framenum = level.time + 10;

	// clear the velocity and hold them in place briefly
	VectorClear (caster->velocity);
	caster->client->ps.pmove.pm_time = 50;
	// make the player still
	caster->flags |= FL_LOCKMOVE;
	// allow the player to fade out
	caster->s.color.a = 255;
	caster->s.color.r = 255;
	caster->s.color.g = 255;
	caster->s.color.b = 255;

	// draw the teleport splash at the teleport source
	gi.CreateEffect(&caster->s, FX_PLAYER_TELEPORT_OUT, CEF_OWNERS_ORIGIN | ((byte)caster->client->tele_type << 5), NULL, "" );
	// do the teleport sound
	gi.sound(caster,CHAN_VOICE,gi.soundindex("weapons/teleport.wav"),1,ATTN_NORM,0);
}
