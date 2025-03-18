//
// g_target.c
//
// Copyright 1998 Raven Software
//

#include "g_target.h" //mxd
#include "cl_strings.h"
#include "g_combat.h" //mxd
#include "p_hud.h" //mxd
#include "Random.h"
#include "g_local.h"

#define CROSSLEVEL_TRIGGER_SF_MASK	0x000000ff //mxd. Originally defined in g_local.h. Mask for TRIGGER1 - TRIGGER8 spawnflags.

#pragma region ========================== target_temp_entity ==========================

static void TargetTempEntityUse(edict_t* ent, edict_t* other, edict_t* activator) //mxd. Named 'Use_Target_Tent' in original logic.
{
	if (ent->style <= FX_REMOVE_EFFECTS || ent->style >= NUM_FX) //mxd. Added sanity check.
	{
		gi.dprintf("target_temp_entity with invalid effect index %i at %s\n", ent->style, vtos(ent->s.origin));
		return;
	}

	//TODO: many effects have args...
	gi.CreateEffect(NULL, ent->style, 0, ent->s.origin, NULL);
}

// QUAKED target_temp_entity (1 0 0) (-8 -8 -8) (8 8 8)
// Fire an origin based temp entity event to the clients.
// Variables:
// style - Effect type.
void SP_target_temp_entity(edict_t* ent) //mxd. Unused in original logic.
{
	ent->use = TargetTempEntityUse;
}

#pragma endregion

#pragma region ========================== target_explosion ==========================

static void TargetExplosionExplodeThink(edict_t* self) //mxd. Named 'target_explosion_explode' in original logic.
{
	gi.CreateEffect(NULL, FX_EXPLOSION1, 0, self->s.origin, NULL);

	if (self->dmg > 0) //mxd. Added self->dmg check.
	{
		const float damage = (float)self->dmg;
		T_DamageRadius(self, self->activator, NULL, damage + 40.0f, damage, damage / 4.0f, DAMAGE_NORMAL, MOD_DIED);
	}

	const float delay = self->delay;
	self->delay = 0.0f;
	G_UseTargets(self, self->activator);
	self->delay = delay;

	self->think = NULL; //BUGFIX: mxd. Avoid triggering assert in EntityThink() when self->delay > 0...
}

static void TargetExplosionUse(edict_t* self, edict_t* other, edict_t* activator) //mxd. Named 'use_target_explosion' in original logic.
{
	self->activator = activator;

	if (self->delay > 0.0f)
	{
		self->think = TargetExplosionExplodeThink;
		self->nextthink = level.time + self->delay;
	}
	else
	{
		TargetExplosionExplodeThink(self);
	}
}

// QUAKED target_explosion (1 0 0) (-8 -8 -8) (8 8 8)
// Creates explosion when used.
// Variables:
// dmg		- Explosion damage.
// delay	- Delay before explosion.
void SP_target_explosion(edict_t* ent)
{
	ent->use = TargetExplosionUse;
	ent->svflags = SVF_NOCLIENT;
}

#pragma endregion

#pragma region ========================== target_changelevel ==========================

void TargetChangelevelUse(edict_t* self, edict_t* other, edict_t* activator) //mxd. Named 'use_target_changelevel' in original logic.
{
	if (level.intermissiontime > 0.0f)
		return; // Already activated.

	// Don't allow dead players to change levels in singleplayer. //TODO: but they can do it in coop?
	if (!DEATHMATCH && !COOP && g_edicts[1].health <= 0)
		return;

	if (DEATHMATCH)
	{
		// If noexit, do a ton of damage to other.
		if (!(DMFLAGS & DF_ALLOW_EXIT) && other != world)
		{
			T_Damage(activator, self, self, vec3_origin, other->s.origin, vec3_origin, 10000, 10000, DAMAGE_AVOID_ARMOR, MOD_EXIT);
			return;
		}

		// If multiplayer, let everyone know who hit the exit.
		if (activator != NULL && activator->client != NULL) //TODO: can a non-client activate this?
			gi.Obituary(PRINT_HIGH, GM_EXIT, activator->s.number, 0);
	}

	// If going to a new unit, clear cross-level triggers.
	if (strstr(self->map, "*"))
		game.serverflags &= ~CROSSLEVEL_TRIGGER_SF_MASK;

	gi.dprintf("***\n*** Unit complete. ***\n***\n");

	BeginIntermission(self);
}

// QUAKED target_changelevel (1 0 0) (-8 -8 -8) (8 8 8)
// Changes map player is on.
// Variables:
// map - The map to change to. Format: 'newmap'$'target'.
//		'newmap' is the map the player is changing to.
//		'$' - has to be there.
//		'target' is the targetname of the info_player_start to go to.
// If an info_player_start is not given, a random one on the level is chosen.
void SP_target_changelevel(edict_t* ent)
{
	if (ent->map != NULL)
	{
		ent->use = TargetChangelevelUse;
		ent->svflags = SVF_NOCLIENT;
	}
	else
	{
		gi.dprintf("target_changelevel with no map at %s\n", vtos(ent->s.origin));
		G_FreeEdict(ent);
	}
}

#pragma endregion

#pragma region ========================== target_crosslevel_trigger ==========================

static void TargetCrosslevelTriggerUse(edict_t* self, edict_t* other, edict_t* activator) //mxd. Named 'trigger_crosslevel_trigger_use' in original logic.
{
	game.serverflags |= self->spawnflags;
	G_FreeEdict(self);
}

// QUAKED target_crosslevel_trigger (.5 .5 .5) (-8 -8 -8) (8 8 8) TRIGGER1 TRIGGER2 TRIGGER3 TRIGGER4 TRIGGER5 TRIGGER6 TRIGGER7 TRIGGER8
// Once this trigger is touched/used, any target_crosslevel_target with the same trigger number is automatically used when a level is started within the same unit.
// It is OK to check multiple triggers. Message, delay, target, and killtarget also work.
void SP_target_crosslevel_trigger(edict_t* self)
{
	self->svflags = SVF_NOCLIENT;
	self->use = TargetCrosslevelTriggerUse;
}

#pragma endregion

#pragma region ========================== target_crosslevel_target ==========================

static void TargetCrosslevelTargetThink(edict_t* self) //mxd. Named 'target_crosslevel_target_think' in original logic.
{
	if ((self->spawnflags & (game.serverflags & CROSSLEVEL_TRIGGER_SF_MASK)) == self->spawnflags)
	{
		G_UseTargets(self, self);
		G_FreeEdict(self);
	}

	self->nextthink = level.time + FRAMETIME;
}

// QUAKED target_crosslevel_target (.5 .5 .5) (-8 -8 -8) (8 8 8) TRIGGER1 TRIGGER2 TRIGGER3 TRIGGER4 TRIGGER5 TRIGGER6 TRIGGER7 TRIGGER8
// Triggered by a trigger_crosslevel elsewhere within a unit. If multiple triggers are checked, all must be true.
// Delay, target and killtarget also work.
// Variables:
// delay - Delay before using targets if the trigger has been activated (default 1).
void SP_target_crosslevel_target(edict_t* self)
{
	if (self->delay == 0.0f)
		self->delay = 1.0f;

	self->svflags = SVF_NOCLIENT;

	self->think = TargetCrosslevelTargetThink;
	self->nextthink = level.time + self->delay;
}

#pragma endregion

//==========================================================

/*QUAK-ED target_laser (0 .5 .8) (-8 -8 -8) (8 8 8) START_ON RED GREEN BLUE YELLOW ORANGE FAT
When triggered, fires a laser.  You can either set a target
or a direction.
*/
/*
void target_laser_think (edict_t *self)
{
	edict_t	*ignore;
	vec3_t	start;
	vec3_t	end;
	trace_t	tr;
	vec3_t	point;
	vec3_t	last_movedir;
	int		count;
	static	vec3_t	lmins = {-4, -4, -4};
	static	vec3_t	lmaxs = {4, 4, 4};

	if (self->spawnflags & 0x80000000)
		count = 8;
	else
		count = 4;

	if (self->enemy)
	{
		VectorCopy (self->movedir, last_movedir);
		VectorMA (self->enemy->absmin, 0.5, self->enemy->size, point);
		VectorSubtract (point, self->s.origin, self->movedir);
		VectorNormalize (self->movedir);
		if (!VectorCompare(self->movedir, last_movedir))
			self->spawnflags |= 0x80000000;
	}

	ignore = self;
	VectorCopy (self->s.origin, start);
	VectorMA (start, 2048, self->movedir, end);
	while(1)
	{
		tr = gi.trace (start, NULL, NULL, end, ignore, CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_DEADMONSTER);

		if (!tr.ent)
			break;

		// hurt it if we can
		if ((tr.ent->takedamage) && !(tr.ent->flags & FL_IMMUNE_LASER) && (tr.ent != self->owner))
			T_Damage (tr.ent, self, self->owner, self->movedir, tr.endpos, vec3_origin, self->dmg, 1, 0);

		// if we hit something that's not a monster or player or is immune to lasers, we're done
		if (!(tr.ent->svflags & SVF_MONSTER) && (!tr.ent->client))
		{
			if (self->spawnflags & 0x80000000)
			{
				self->spawnflags &= ~0x80000000;
				gi.WriteByte (svc_temp_entity);
				gi.WriteByte (TE_LASER_SPARKS);
				gi.WriteByte (count);
				gi.WritePosition (tr.endpos);
				gi.WriteDir (tr.plane.normal);
				gi.WriteByte (self->s.skinnum);
				gi.multicast (tr.endpos, MULTICAST_PVS);
			}
			break;
		}

		ignore = tr.ent;
		VectorCopy (tr.endpos, start);
	}

	VectorCopy (tr.endpos, self->s.old_origin);

	self->nextthink = level.time + FRAMETIME;
}

void target_laser_on (edict_t *self)
{
	self->spawnflags |= 0x80000001;
	self->svflags &= ~SVF_NOCLIENT;
	target_laser_think (self);
}

void target_laser_off (edict_t *self)
{
	self->spawnflags &= ~1;
	self->svflags |= SVF_NOCLIENT;
	self->nextthink = 0;
}

void target_laser_use (edict_t *self, edict_t *other, edict_t *activator)
{
	if (self->spawnflags & 1)
		target_laser_off (self);
	else
		target_laser_on (self);
}

void target_laser_start (edict_t *self)
{
	edict_t *ent;

	self->movetype = PHYSICSTYPE_NONE;
	self->solid = SOLID_NOT;
	self->s.renderfx |= RF_BEAM|RF_TRANSLUCENT;
	self->s.modelindex = 1;			// must be non-zero

	// set the beam diameter
	if (self->spawnflags & 64)
		self->s.frame = 16;
	else
		self->s.frame = 4;

	// set the color
	if (self->spawnflags & 2)
		self->s.skinnum = 0xf2f2f0f0;
	else if (self->spawnflags & 4)
		self->s.skinnum = 0xd0d1d2d3;
	else if (self->spawnflags & 8)
		self->s.skinnum = 0xf3f3f1f1;
	else if (self->spawnflags & 16)
		self->s.skinnum = 0xdcdddedf;
	else if (self->spawnflags & 32)
		self->s.skinnum = 0xe0e1e2e3;

	if (!self->owner)
		self->owner = self;

	if (!self->enemy)
	{
		if (self->target)
		{
			ent = G_Find (NULL, FOFS(targetname), self->target);
			if (!ent)
				gi.dprintf ("%s at %s: %s is a bad target\n", self->classname, vtos(self->s.origin), self->target);
			self->enemy = ent;
		}
		else
		{
			G_SetMovedir (self->s.angles, self->movedir);
		}
	}
	self->use = target_laser_use;
	self->think = target_laser_think;

	if (!self->dmg)
		self->dmg = 1;

	VectorSet (self->mins, -8, -8, -8);
	VectorSet (self->maxs, 8, 8, 8);
	gi.linkentity (self);

	if (self->spawnflags & 1)
		target_laser_on (self);
	else
		target_laser_off (self);
}

void SP_target_laser (edict_t *self)
{
	// let everything else get spawned before we start firing
	self->think = target_laser_start;
	self->nextthink = level.time + 1;
}
*/
//==========================================================

/*QUAKED target_lightramp (0 .5 .8) (-8 -8 -8) (8 8 8) TOGGLE
speed		How many seconds the ramping will take
message		two letters; starting lightlevel and ending lightlevel
*/

void target_lightramp_think (edict_t *self)
{
	char	style[2];

	style[0] = 'a' + self->movedir[0] + (level.time - self->timestamp) / FRAMETIME * self->movedir[2];
	style[1] = 0;
	gi.configstring (CS_LIGHTS+self->enemy->style, style);

	if ((level.time - self->timestamp) < self->speed)
	{
		self->nextthink = level.time + FRAMETIME;
	}
	else if (self->spawnflags & 1)
	{
		char	temp;

		temp = self->movedir[0];
		self->movedir[0] = self->movedir[1];
		self->movedir[1] = temp;
		self->movedir[2] *= -1;
	}
}

void target_lightramp_use (edict_t *self, edict_t *other, edict_t *activator)
{
	if (!self->enemy)
	{
		edict_t		*e;

		// check all the targets
		e = NULL;
		while (1)
		{
			e = G_Find (e, FOFS(targetname), self->target);
			if (!e)
				break;
			if (strcmp(e->classname, "light") != 0)
			{
#ifdef _DEVEL
				gi.dprintf("%s at %s ", self->classname, vtos(self->s.origin));
				gi.dprintf("target %s (%s at %s) is not a light\n", self->target, e->classname, vtos(e->s.origin));
#endif
			}
			else
			{
				self->enemy = e;
			}
		}

		if (!self->enemy)
		{
#ifdef _DEVEL
			gi.dprintf("%s target %s not found at %s\n", self->classname, self->target, vtos(self->s.origin));
#endif
			G_FreeEdict (self);
			return;
		}
	}

	self->timestamp = level.time;
	target_lightramp_think (self);
}

void SP_target_lightramp (edict_t *self)
{
	if (!self->message || strlen(self->message) != 2 || self->message[0] < 'a' || self->message[0] > 'z' || self->message[1] < 'a' || self->message[1] > 'z' || self->message[0] == self->message[1])
	{
#ifdef _DEVEL
		gi.dprintf("target_lightramp has bad ramp (%s) at %s\n", self->message, vtos(self->s.origin));
#endif
		G_FreeEdict (self);
		return;
	}

	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	if (!self->target)
	{
#ifdef _DEVEL
		gi.dprintf("%s with no target at %s\n", self->classname, vtos(self->s.origin));
#endif
		G_FreeEdict (self);
		return;
	}

	self->svflags |= SVF_NOCLIENT;
	self->use = target_lightramp_use;
	self->think = target_lightramp_think;

	self->movedir[0] = self->message[0] - 'a';
	self->movedir[1] = self->message[1] - 'a';
	self->movedir[2] = (self->movedir[1] - self->movedir[0]) / (self->speed / FRAMETIME);
}

//==========================================================

/*QUAK-ED target_earthquake (1 0 0) (-8 -8 -8) (8 8 8)
When triggered, this initiates a level-wide earthquake.
All players and monsters are affected.
"speed"		severity of the quake (default:200)
"count"		duration of the quake (default:5)
*/

void target_earthquake_think (edict_t *self)
{
	int		i;
	edict_t	*e;

	if (sv_jumpcinematic->value)	// Don't do this if jumping a cinematic
		return;

	if (self->last_move_time < level.time)
	{
		gi.positioned_sound (self->s.origin, self, CHAN_AUTO, self->noise_index, 1.0, ATTN_NONE, 0);
		self->last_move_time = level.time + 0.5;
	}

	for (i=1, e=g_edicts+i; i < globals.num_edicts; i++,e++)
	{
		if (!e->inuse)
			continue;
		if (!e->client)
			continue;
		if (!e->groundentity)
			continue;

//		e->groundentity = NULL;
		e->velocity[0] += flrand(-150.0F, 150.0F);
		e->velocity[1] += flrand(-150.0F, 150.0F);
		e->velocity[2] = self->speed * (100.0 / e->mass);
	}

	if (level.time < self->timestamp)
		self->nextthink = level.time + FRAMETIME;
}

void target_earthquake_use (edict_t *self, edict_t *other, edict_t *activator)
{

	if (sv_jumpcinematic->value)	// Don't do this if jumping a cinematic
		return;

	self->timestamp = level.time + self->count;
	self->nextthink = level.time + FRAMETIME;
	self->activator = activator;
	self->last_move_time = 0;
}

void SP_target_earthquake (edict_t *self)
{
	if (!self->targetname)
		gi.dprintf("untargeted %s at %s\n", self->classname, vtos(self->s.origin));

	if (!self->count)
		self->count = 5;

	if (!self->speed)
		self->speed = 200;

	self->svflags |= SVF_NOCLIENT;
	self->think = target_earthquake_think;
	self->use = target_earthquake_use;

	self->noise_index = gi.soundindex ("world/quake.wav");
}
