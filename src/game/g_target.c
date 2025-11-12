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

void TargetTempEntityUse(edict_t* ent, edict_t* other, edict_t* activator) //mxd. Named 'Use_Target_Tent' in original logic.
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

void TargetExplosionExplodeThink(edict_t* self) //mxd. Named 'target_explosion_explode' in original logic.
{
	gi.CreateEffect(NULL, FX_EXPLOSION1, 0, self->s.origin, NULL); //TODO: sfx is kinda lame... Use whatever obj_barrel uses.

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

void TargetExplosionUse(edict_t* self, edict_t* other, edict_t* activator) //mxd. Named 'use_target_explosion' in original logic.
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
	if (strchr(self->map, '*') != NULL) //mxd. strstr() -> strchr().
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

void TargetCrosslevelTriggerUse(edict_t* self, edict_t* other, edict_t* activator) //mxd. Named 'trigger_crosslevel_trigger_use' in original logic.
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

void TargetCrosslevelTargetThink(edict_t* self) //mxd. Named 'target_crosslevel_target_think' in original logic.
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

#pragma region ========================== target_lightramp ==========================

#define SF_TOGGLE 1 //mxd

void TargetLightrampThink(edict_t* self) //mxd. Named 'target_lightramp_think' in original logic.
{
	char style[2];

	style[0] = 'a' + (char)(self->movedir[0] + ((level.time - self->timestamp) / FRAMETIME * self->movedir[2]));
	style[1] = 0;
	gi.configstring(CS_LIGHTS + self->enemy->style, style);

	if (level.time - self->timestamp < self->speed)
	{
		self->nextthink = level.time + FRAMETIME;
	}
	else
	{
		if (self->spawnflags & SF_TOGGLE)
		{
			const char temp = (char)self->movedir[0];
			self->movedir[0] = self->movedir[1];
			self->movedir[1] = temp;
			self->movedir[2] *= -1.0f;
		}

		self->think = NULL; //BUGFIX: mxd. Avoid triggering assert in EntityThink()...
	}
}

void TargetLightrampUse(edict_t* self, edict_t* other, edict_t* activator) //mxd. Named 'target_lightramp_use' in original logic.
{
	self->timestamp = level.time;
	self->think = TargetLightrampThink;
	self->nextthink = level.time + FRAMETIME;
}

// QUAKED target_lightramp (0 .5 .8) (-8 -8 -8) (8 8 8) TOGGLE
// Spawnflags:
// TOGGLE - Reverse ramping direction each time used.
// Variables:
// speed	- How many seconds the ramping will take.
// message	- Two letters: starting lightlevel and ending lightlevel.
void SP_target_lightramp(edict_t* self)
{
	if (self->message == NULL || strlen(self->message) != 2 || self->message[0] < 'a' || self->message[0] > 'z' || self->message[1] < 'a' || self->message[1] > 'z' || self->message[0] == self->message[1])
	{
		gi.dprintf("target_lightramp has bad ramp (%s) at %s\n", self->message, vtos(self->s.origin));
		G_FreeEdict(self);

		return;
	}

	if (DEATHMATCH)
	{
		G_FreeEdict(self);
		return;
	}

	if (self->target == NULL)
	{
		gi.dprintf("%s with no target at %s\n", self->classname, vtos(self->s.origin));
		G_FreeEdict(self);

		return;
	}

	//mxd. Check all the targets. (originally done in TargetLightrampUse()).
	edict_t* light = NULL;
	while ((light = G_Find(light, FOFS(targetname), self->target)) != NULL)
	{
		if (strcmp(light->classname, "light") == 0)
			self->enemy = light;
		else
			gi.dprintf("%s at %s target %s (%s at %s) is not a light\n", self->classname, vtos(self->s.origin), self->target, light->classname, vtos(light->s.origin));
	}

	if (self->enemy == NULL)
	{
		gi.dprintf("%s target %s not found at %s\n", self->classname, self->target, vtos(self->s.origin));
		G_FreeEdict(self);

		return;
	}

	self->svflags |= SVF_NOCLIENT;
	self->use = TargetLightrampUse;

	self->movedir[0] = (float)(self->message[0] - 'a');
	self->movedir[1] = (float)(self->message[1] - 'a');
	self->movedir[2] = (self->movedir[1] - self->movedir[0]) / (self->speed / FRAMETIME);
}

#pragma endregion

#pragma region ========================== target_earthquake ==========================

void TargetEarthquakeThink(edict_t* self) //mxd. Named 'target_earthquake_think' in original logic.
{
	if ((int)sv_jumpcinematic->value) // Don't do this if jumping a cinematic
	{
		self->think = NULL; //BUGFIX: mxd. Avoid triggering assert in EntityThink()...
		return;
	}

	if (self->last_move_time < level.time)
	{
		gi.positioned_sound(self->s.origin, self, CHAN_AUTO, self->noise_index, 1.0f, ATTN_NONE, 0.0f); //TODO: the sound never stops! Should be manually stopped instead?
		self->last_move_time = level.time + FRAMETIME * 5.0f; //mxd. '+ 0.5f' in original logic. //TODO: use bigger delay? Sound is ~2 seconds long! 
	}

	edict_t* cl = &g_edicts[1];
	for (int i = 1; i < globals.num_edicts; i++, cl++)
	{
		if (cl->inuse && cl->client != NULL && cl->groundentity != NULL) //TODO: this doesn't affect player in any way... //TODO: also affect monsters (mentioned in SP_target_earthquake() description). 
		{
			cl->velocity[0] += flrand(-150.0f, 150.0f);
			cl->velocity[1] += flrand(-150.0f, 150.0f);
			cl->velocity[2] += self->speed * (100.0f / (float)cl->mass);
		}
	}

	if (level.time < self->timestamp)
		self->nextthink = level.time + FRAMETIME;
	else
		self->think = NULL; //BUGFIX: mxd. Avoid triggering assert in EntityThink()...
}

void TargetEarthquakeUse(edict_t* self, edict_t* other, edict_t* activator) //mxd. Named 'target_earthquake_use' in original logic.
{
	if ((int)sv_jumpcinematic->value) // Don't do this if jumping a cinematic.
		return;

	self->activator = activator;
	self->last_move_time = 0.0f;
	self->timestamp = level.time + (float)self->count;

	self->think = TargetEarthquakeThink;
	self->nextthink = level.time + FRAMETIME;
}

// QUAKED target_earthquake (1 0 0) (-8 -8 -8) (8 8 8)
// When triggered, this initiates a level-wide earthquake. All players and monsters are affected.
// Variables:
// speed - Severity of the quake (default 200).
// count - Duration of the quake (default 5).
void SP_target_earthquake(edict_t* self) //TODO: non-functional. Also, there's trigger_quake.
{
	if (self->targetname == NULL)
		gi.dprintf("untargeted %s at %s\n", self->classname, vtos(self->s.origin));

	if (self->count == 0)
		self->count = 5;

	if (self->speed == 0.0f)
		self->speed = 200.0f;

	self->noise_index = gi.soundindex("world/quake.wav");
	self->svflags |= SVF_NOCLIENT;
	self->use = TargetEarthquakeUse;
}

#pragma endregion