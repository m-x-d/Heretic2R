//
// g_misc.c
//
// Copyright 1998 Raven Software
//

#include "g_misc.h"
#include "g_debris.h" //mxd
#include "g_DefaultMessageHandler.h"
#include "g_monster.h"
#include "g_items.h" //mxd
#include "g_target.h" //mxd
#include "m_stats.h"
#include "p_client.h" //mxd
#include "p_teleport.h" //mxd
#include "FX.h"
#include "Random.h"
#include "Vector.h"

#pragma region ========================== func_areaportal ==========================

void FuncAreaportalUse(edict_t* ent, edict_t* other, edict_t* activator) //mxd. Named 'Use_Areaportal' in original logic.
{
	ent->count ^= 1; // Toggle state.
	gi.SetAreaPortalState(ent->style, ent->count);
}

// QUAKED func_areaportal (0 0 0) ?
// This is a non-visible object that divides the world into areas that are separated when this portal is not activated.
// Usually enclosed in the middle of a door.
void SP_func_areaportal(edict_t* ent)
{
	ent->use = FuncAreaportalUse;
	ent->count = 0; // Always start closed.
}

#pragma endregion

#pragma region ========================== path_corner ==========================

void PathCornerTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf) //mxd. Named 'path_corner_touch' in original logic.
{
	if (other->movetarget != self || other->enemy != NULL)
		return;

	if (self->pathtarget != NULL)
	{
		char* save_target = self->target;
		self->target = self->pathtarget;
		G_UseTargets(self, other);
		self->target = save_target;
	}

	edict_t* next = ((self->target != NULL) ? G_PickTarget(self->target) : NULL);

	if (next != NULL && (next->spawnflags & MSF_AMBUSH))
	{
		vec3_t v;
		VectorCopy(next->s.origin, v);
		v[2] += next->mins[2];
		v[2] -= other->mins[2];

		VectorCopy(v, other->s.origin);

		next = G_PickTarget(next->target);
	}

	other->goalentity = next;
	other->movetarget = next;

	if (self->wait > 0.0f)
	{
		other->monsterinfo.pausetime = level.time + self->wait;
		G_PostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
	}
	else if (other->goalentity != NULL)
	{
		vec3_t v;
		VectorSubtract(other->goalentity->s.origin, other->s.origin, v);
		other->ideal_yaw = VectorYaw(v);
	}
	else
	{
		other->monsterinfo.pausetime = level.time + 100000000.0f;
		G_PostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
	}
}

// QUAKED path_corner (.5 .3 0) (-8 -8 -8) (8 8 8) TELEPORT
// Variables:
// target		- Target name of next path corner.
// pathtarget	- Used when an entity that has this path_corner targeted touches it angles - used to make the brush rotate.
//				  The brush MUST have an origin brush in it. It is an accumulative value, so if the value is 0 40 0 the brush rotate
//				  an additional 40 degrees along the y axis before it reaches this path corner.
// wait			- -1 makes trains stop until retriggered.
//				  -3 trains explode upon reaching this path corner.
// noise		- Wav file to play when corner is hit (trains only).
void SP_path_corner(edict_t* self)
{
	if (self->targetname == NULL)
	{
		gi.dprintf("path_corner with no targetname at %s\n", vtos(self->s.origin));
		G_FreeEdict(self);

		return;
	}

	if (st.noise != NULL)
		self->moveinfo.sound_middle = gi.soundindex(st.noise);

	self->solid = SOLID_TRIGGER;
	self->movetype = PHYSICSTYPE_NONE;
	self->svflags |= SVF_NOCLIENT;
	self->touch = PathCornerTouch;

	VectorSet(self->mins, -8.0f, -8.0f, -8.0f);
	VectorSet(self->maxs, 8.0f, 8.0f, 8.0f);

	gi.linkentity(self);
}

#pragma endregion

#pragma region ========================== point_combat ==========================

#define SF_HOLD	1 //mxd

void PointCombatTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf) //mxd. Named 'point_combat_touch' in original logic.
{
	if (other->movetarget != self)
		return; // Wasn't purposely heading for me.

	if (self->target != NULL)
	{
		other->target = self->target;
		other->goalentity = other->movetarget = G_PickTarget(other->target);

		if (other->goalentity == NULL)
		{
			gi.dprintf("%s at %s target %s does not exist\n", self->classname, vtos(self->s.origin), self->target);
			other->movetarget = self;
		}

		self->target = NULL;
	}
	else if ((self->spawnflags & SF_HOLD) && !(other->flags & (FL_SWIM | FL_FLY)))
	{
		other->spawnflags |= MSF_FIXED; // Stay here forever for now.
		G_PostMessage(other, MSG_STAND, PRI_DIRECTIVE, NULL);
	}

	if (other->movetarget == self)
	{
		other->target = NULL;
		other->movetarget = NULL;
		other->goalentity = other->enemy;
		other->monsterinfo.aiflags &= ~AI_COMBAT_POINT;
	}

	if (self->pathtarget != NULL)
	{
		edict_t* activator;
		char* save_target = self->target;
		self->target = self->pathtarget;

		if (other->enemy != NULL && other->enemy->client != NULL)
			activator = other->enemy;
		else if (other->oldenemy != NULL && other->oldenemy->client != NULL)
			activator = other->oldenemy;
		else if (other->activator != NULL && other->activator->client != NULL)
			activator = other->activator;
		else
			activator = other;

		G_UseTargets(self, activator);
		self->target = save_target;
	}
}

// QUAKED point_combat (0.5 0.3 0) (-8 -8 -8) (8 8 8) HOLD
// Makes this the target of a monster and it will head here when first activated before going after the activator.
// Spawnflags:
// HOLD - monster will stay here.
void SP_point_combat(edict_t* self)
{
	self->solid = SOLID_TRIGGER;
	self->svflags = SVF_NOCLIENT;
	self->touch = PointCombatTouch;

	VectorSet(self->mins, -8.0f, -8.0f, -16.0f);
	VectorSet(self->maxs, 8.0f, 8.0f, 16.0f);

	gi.linkentity(self);
}

#pragma endregion

#pragma region ========================== info_null, info_notnull  ==========================

// QUAKED info_null (0 0.5 0) (-4 -4 -4) (4 4 4)
// Used as a positional target for spotlights, etc.
void SP_info_null(edict_t* self)
{
	G_FreeEdict(self);
}

// QUAKED info_notnull (0 0.5 0) (-4 -4 -4) (4 4 4)
// Used as a positional target for lightning.
void SP_info_notnull(edict_t* self)
{
	self->solid = SOLID_TRIGGER;
	self->movetype = PHYSICSTYPE_NONE;

	VectorCopy(self->s.origin, self->absmin);
	VectorCopy(self->s.origin, self->absmax);
}

#pragma endregion

#pragma region ========================== func_wall ==========================

#define SF_TRIGGER_SPAWN	1 //mxd
#define SF_TOGGLE			2 //mxd
#define SF_START_ON			4 //mxd
#define SF_ANIMATED			8 //mxd
#define SF_ANIMATED_FAST	16 //mxd

void FuncWallUse(edict_t* self, edict_t* other, edict_t* activator) //mxd. Named 'func_wall_use' in original logic.
{
	if (self->solid == SOLID_NOT)
	{
		self->solid = SOLID_BSP;
		self->svflags &= ~SVF_NOCLIENT;
		KillBox(self);
	}
	else
	{
		self->solid = SOLID_NOT;
		self->svflags |= SVF_NOCLIENT;
	}

	gi.linkentity(self);

	if (!(self->spawnflags & SF_TOGGLE))
		self->use = NULL;
}

// QUAKED func_wall (0 .5 .8) ? TRIGGER_SPAWN TOGGLE START_ON ANIMATED ANIMATED_FAST
// This is just a solid wall if not inhibited.
// Spawnflags:
// TRIGGER_SPAWN	- The wall will not be present until triggered, it will then blink in to existence; it will kill anything that was in it's way.
// TOGGLE			- Only valid for TRIGGER_SPAWN walls. This allows the wall to be turned on and off.
// START_ON			- Only valid for TRIGGER_SPAWN walls. The wall will initially be present.
void SP_func_wall(edict_t* self)
{
	self->movetype = PHYSICSTYPE_PUSH;
	gi.setmodel(self, self->model);

	if (self->spawnflags & SF_ANIMATED)
		self->s.effects |= EF_ANIM_ALL;

	if (self->spawnflags & SF_ANIMATED_FAST)
		self->s.effects |= EF_ANIM_ALLFAST;

	// Just a wall?
	if (!(self->spawnflags & (SF_TRIGGER_SPAWN | SF_TOGGLE | SF_START_ON)))
	{
		self->solid = SOLID_BSP;
		gi.linkentity(self);

		return;
	}

	// Must have TRIGGER_SPAWN flag.
	self->spawnflags |= SF_TRIGGER_SPAWN;

	// Yell if the spawnflags are odd.
	if ((self->spawnflags & SF_START_ON) && !(self->spawnflags & SF_TOGGLE))
	{
		gi.dprintf("func_wall START_ON without TOGGLE\n");
		self->spawnflags |= SF_TOGGLE;
	}

	self->use = FuncWallUse;

	if (self->spawnflags & SF_START_ON)
	{
		self->solid = SOLID_BSP;
	}
	else
	{
		self->solid = SOLID_NOT;
		self->svflags |= SVF_NOCLIENT;
	}

	gi.linkentity(self);
}

#pragma endregion

#pragma region ========================== func_object ==========================

void FuncObjectTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf) //mxd. Named 'func_object_touch' in original logic.
{
	// Only squash thing we fall on top of.
	if (plane != NULL && plane->normal[2] == 1.0f && other->takedamage != DAMAGE_NO)
		T_Damage(other, self, self, vec3_origin, self->s.origin, vec3_origin, self->dmg, 1, DAMAGE_AVOID_ARMOR, MOD_DIED);
}

void FuncObjectRelease(edict_t* self) //mxd. Named 'func_object_release' in original logic.
{
	self->movetype = PHYSICSTYPE_STEP;
	self->touch = FuncObjectTouch;
}

void FuncObjectUse(edict_t* self, edict_t* other, edict_t* activator) //mxd. Named 'func_object_use' in original logic.
{
	self->solid = SOLID_BSP;
	self->svflags &= ~SVF_NOCLIENT;
	self->use = NULL;

	KillBox(self);
	FuncObjectRelease(self);
}

//mxd. Avoid compiler warnings...
#undef SF_ANIMATED
#undef SF_ANIMATED_FAST

#define SF_TRIGGER_SPAWN	1 //mxd
#define SF_ANIMATED			2 //mxd
#define SF_ANIMATED_FAST	4 //mxd

// QUAKED func_object (0 .5 .8) ? TRIGGER_SPAWN ANIMATED ANIMATED_FAST
// This is solid bmodel that will fall if it's support it removed.
void SP_func_object(edict_t* self)
{
	gi.setmodel(self, self->model);

	VectorInc(self->mins);
	VectorDec(self->maxs);

	if (self->dmg == 0)
		self->dmg = 100;

	if (self->spawnflags == 0) //TODO: a strange way to check for SF_TRIGGER_SPAWN flag?.. Will also trigger when SF_ANIMATED or SF_ANIMATED_FAST are set!
	{
		self->solid = SOLID_BSP;
		self->think = FuncObjectRelease;
		self->nextthink = level.time + FRAMETIME * 2.0f;
	}
	else
	{
		self->solid = SOLID_NOT;
		self->use = FuncObjectUse;
		self->svflags |= SVF_NOCLIENT;
	}

	if (self->spawnflags & SF_ANIMATED)
		self->s.effects |= EF_ANIM_ALL;

	if (self->spawnflags & SF_ANIMATED_FAST)
		self->s.effects |= EF_ANIM_ALLFAST;

	self->movetype = PHYSICSTYPE_PUSH;
	self->clipmask = MASK_MONSTERSOLID;

	gi.linkentity(self);
}

#pragma endregion

#pragma region ========================== item_spitter ==========================

#define SF_NOFLASH	1 //mxd

void ItemSpitterUse(edict_t* self, edict_t* owner, edict_t* attacker) //mxd. Named 'ItemSpitterSpit' in original logic.
{
	if (self->target == NULL || self->style == 0)
		return;

	self->style = 0; // Mark spitter as used.

	const float delta = 360.0f / (float)self->count;
	vec3_t hold_angles = { owner->s.angles[0], 0.0f, owner->s.angles[2] };
	gitem_t* item = P_FindItemByClassname(self->target);

	for (int i = 0; i < self->count; i++)
	{
		edict_t* new_item = G_Spawn();

		vec3_t forward;
		AngleVectors(hold_angles, forward, NULL, NULL);

		VectorCopy(self->s.origin, new_item->s.origin);
		VectorMA(new_item->s.origin, self->dmg_radius, forward, new_item->s.origin);

		if (self->mass != 0)
			new_item->spawnflags |= self->mass;

		if (item == NULL) // Must be an object, not an item.
		{
			new_item->classname = ED_NewString(self->target);
			ED_CallSpawn(new_item);
		}
		else
		{
			new_item->movetype = PHYSICSTYPE_STEP;
			SpawnItem(new_item, item);
		}

		if (!(self->spawnflags & SF_NOFLASH))
			gi.CreateEffect(NULL, FX_PICKUP, 0, new_item->s.origin, "");

		hold_angles[YAW] += delta;
	}
}

// QUAKED item_spitter (0 .5 .8) (-4 -4 -4)  (4 4 4)	NOFLASH
// When targeted it will spit out an number of items in various directions

// Spawnflags:
// NOFLASH - No flash is created when item is 'spit out'.

// Variables:
// target		- Classname of item or object being spit out.
// count		- Number of items being spit out (default 1).
// radius		- Distance from item_spitter origin that items will be spawned.
// spawnflags2	- The spawnflags for the item being created.
void SP_item_spitter(edict_t* self)
{
	self->style = 1; // To show it hasn't been used yet.
	self->movetype = PHYSICSTYPE_NONE;
	self->solid = SOLID_NOT;
	self->count = max(1, self->count);
	self->dmg_radius = (float)st.radius;
	self->mass = st.spawnflags2; //TODO: Why is mass used to store spawnflags? Can't we just use spawnflags for that?..
	self->use = ItemSpitterUse;

	VectorSet(self->mins, -4.0f, -4.0f, -4.0f);
	VectorSet(self->maxs,  4.0f,  4.0f,  4.0f);

	gi.linkentity(self);
}

#pragma endregion

#pragma region ========================== misc_update_spawner ==========================

// Update the spawner so that we will re-materialize in a different position.
void MiscUpdateSpawnerTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf) //mxd. Named 'respawner_touch' in original version,
{
	// If we aren't a player, forget it.
	if (other->client == NULL)
		return;

	edict_t* spot = NULL;
	while ((spot = G_Find(spot, FOFS(classname), "info_player_start")) != NULL)
	{
		if (game.spawnpoint[0] == 0 && spot->targetname == NULL)
			break;

		if (game.spawnpoint[0] == 0 || spot->targetname == NULL)
			continue;

		if (Q_stricmp(game.spawnpoint, spot->targetname) == 0)
			break;
	}

	if (spot == NULL)
	{
		if (game.spawnpoint[0] == 0)
			spot = G_Find(spot, FOFS(classname), "info_player_start"); // There wasn't a spawnpoint without a target, so use any.

		if (spot == NULL)
		{
			gi.error("Couldn't find spawn point %s\n", game.spawnpoint);
			return;
		}
	}

	VectorSet(spot->s.origin, self->mins[0] + self->size[0] / 2.0f, self->mins[1] + self->size[1] / 2.0f, self->mins[2] + self->size[2]);
	VectorCopy(self->s.angles, spot->s.angles);

	G_FreeEdict(self);
}

// QUAKED misc_update_spawner (.5 .5 .5) ?
// This creates the spawner update entity, which updates the spawner position when triggered.
void SP_misc_update_spawner(edict_t* ent)
{
	ent->movetype = PHYSICSTYPE_NONE;
	ent->svflags |= SVF_NOCLIENT;
	ent->solid = SOLID_TRIGGER;
	ent->touch = MiscUpdateSpawnerTouch;

	gi.setmodel(ent, ent->model);
	gi.linkentity(ent);
}

#pragma endregion

#pragma region ========================== misc_teleporter ==========================

#define SF_NO_MODEL				1
#define SF_DEATHMATCH_RANDOM	2
#define SF_START_OFF			4
#define SF_MULT_DEST			8

static void MiscTeleporterCreateEffect(edict_t* self) //mxd. Added to reduce code duplication.
{
	edict_t* effect = G_Spawn();

	VectorCopy(self->maxs, effect->maxs);
	VectorCopy(self->mins, effect->mins);
	effect->solid = SOLID_NOT;
	effect->s.effects |= (EF_NODRAW_ALWAYS_SEND | EF_ALWAYS_ADD_EFFECTS);
	self->enemy = effect;

	gi.linkentity(effect);

	vec3_t fx_origin;
	for (int i = 0; i < 3; i++)
		fx_origin[i] = ((self->maxs[i] - self->mins[i]) / 2.0f) + self->mins[i];

	if (!(self->spawnflags & SF_NO_MODEL))
		effect->PersistantCFX = gi.CreatePersistantEffect(&effect->s, FX_TELEPORT_PAD, CEF_BROADCAST, fx_origin, "");
}

static void MiscTeleporterDeactivate(edict_t* self, G_Message_t* msg) //mxd. Named 'Teleporter_Deactivate' in original logic.
{
	self->touch = NULL;

	// If there's an effect out there, kill it.
	if (self->enemy != NULL)
	{
		gi.RemoveEffects(&self->enemy->s, FX_TELEPORT_PAD);

		if (self->enemy->PersistantCFX > 0)
		{
			gi.RemovePersistantEffect(self->enemy->PersistantCFX, REMOVE_TELEPORT_PAD);
			self->enemy->PersistantCFX = 0;
		}

		self->enemy = NULL;
	}
}

static void MiscTeleporterActivate(edict_t* self, G_Message_t* msg) //mxd. Named 'Teleporter_Activate' in original logic.
{
	self->touch = PlayerTeleporterTouch;

	// If there's no effect already, create a new one.
	if (self->enemy == NULL)
		MiscTeleporterCreateEffect(self); //mxd
}

void MiscTeleporterStaticsInit(void)
{
	classStatics[CID_TELEPORTER].msgReceivers[G_MSG_SUSPEND] = MiscTeleporterDeactivate;
	classStatics[CID_TELEPORTER].msgReceivers[G_MSG_UNSUSPEND] = MiscTeleporterActivate;
}

// QUAKED misc_teleporter (1 0 0) ? NO_MODEL DEATHMATCH_RANDOM START_OFF MULT_DEST
// This creates the teleporter disc that will send us places
// Stepping onto this disc will teleport players to the targeted misc_teleporter_dest object.

// Spawnflags:
// NO_MODEL				- Makes teleporter invisible.
// DEATHMATCH_RANDOM	- Makes the teleporter dump you at random spawn points in deathmatch.
// START_OFF			- Pad has no effect, and won't teleport you anywhere till its activated.
// MULT_DEST			- Pad is targeted at more than one destination.

// Variables:
// style - Number of destinations this pad has.
void SP_misc_teleporter(edict_t* ent)
{
	if (ent->target == NULL && (!(ent->spawnflags & SF_DEATHMATCH_RANDOM) || !DEATHMATCH)) //BUGFIX: '!ent->spawnflags & DEATHMATCH_RANDOM' in original logic.
	{
		gi.dprintf("teleporter without a target.\n");
		G_FreeEdict(ent);

		return;
	}

	ent->msgHandler = DefaultMsgHandler;

	ent->movetype = PHYSICSTYPE_NONE;
	ent->svflags |= SVF_NOCLIENT;
	ent->solid = SOLID_TRIGGER;

	gi.setmodel(ent, ent->model);
	gi.linkentity(ent);

	// If we don't have multiple destinations - probably redundant.
	if (!(ent->spawnflags & SF_MULT_DEST))
		ent->style = 0;

	// If we want an effect on spawn, create it.
	if (!(ent->spawnflags & SF_START_OFF))
	{
		ent->touch = PlayerTeleporterTouch;
		MiscTeleporterCreateEffect(ent); //mxd
	}
}

// QUAKED misc_teleporter_dest (1 0 0) (-32 -32 -24) (32 32 -16)
// Point teleporters at these.
void SP_misc_teleporter_dest(edict_t* ent)
{
	ent->s.skinnum = 0;
	ent->solid = SOLID_NOT;
	VectorSet(ent->mins, -32.0f, -32.0f, -24.0f);
	VectorSet(ent->maxs,  32.0f,  32.0f, -16.0f);

	gi.linkentity(ent);

	vec3_t end_pos;
	VectorCopy(ent->s.origin, end_pos);
	end_pos[2] -= 500.0f;

	trace_t tr;
	gi.trace(ent->s.origin, vec3_origin, vec3_origin, end_pos, NULL, CONTENTS_WORLD_ONLY | MASK_PLAYERSOLID, &tr);

	VectorCopy(tr.endpos, ent->last_org);
	ent->last_org[2] -= player_mins[2];
}

#pragma endregion

#pragma region ========================== misc_magic_portal ==========================

void MiscMagicPortalTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf) //mxd. Named 'misc_magic_portal_touch' in original logic.
{
	if (level.time < self->touch_debounce_time || other->client == NULL) // Not a player.
		return;

	edict_t* ent = NULL;
	ent = G_Find(ent, FOFS(targetname), self->target);

	if (ent != NULL)
		TargetChangelevelUse(ent, self, other);

	self->touch_debounce_time = level.time + 4.0f;
}

void MiscMagicPortalUse(edict_t* self, edict_t* other, edict_t* activator) //mxd. Named 'misc_magic_portal_use' in original logic.
{
	if (level.time < self->impact_debounce_time)
		return;

	if (self->solid == SOLID_NOT)
	{
		// We aren't engaged yet. Make us solid and start the effect.
		self->solid = SOLID_TRIGGER;
		self->touch = MiscMagicPortalTouch;
		self->PersistantCFX = gi.CreatePersistantEffect(&self->s, FX_MAGIC_PORTAL, CEF_BROADCAST, self->s.origin, "vbb", self->s.angles, (byte)self->style, (byte)self->count);
		self->s.effects &= ~EF_DISABLE_EXTRA_FX;
	}
	else
	{
		// We were on, now turn it off.
		self->solid = SOLID_NOT;
		self->touch = NULL;

		// Remove the persistent effect.
		if (self->PersistantCFX > 0)
		{
			gi.RemovePersistantEffect(self->PersistantCFX, REMOVE_PORTAL);
			self->PersistantCFX = 0;
		}

		self->s.effects |= EF_DISABLE_EXTRA_FX;
	}

	gi.linkentity(self);
	self->impact_debounce_time = level.time + 4.0f;
}

#undef SF_START_OFF //mxd. Avoid compiler warnings...
#define SF_START_OFF	1 //mxd

// QUAKED misc_magic_portal (1 .5 0) (-16 -16 -32) (16 16 32)  START_OFF
// A magical glowing portal. Triggerable. In order to be functional as a world teleport, it must target a target_changelevel.

// Spawnflags:
// START_OFF - Portal will start off.

// Variables:
// angles	- Manipulates the facing of the effect as normal.
// style	- 0-blue, 1-red, 2-green.
// count	- Close after 1-255 seconds. 0 means stay until triggered.
void SP_misc_magic_portal(edict_t* self)
{
	// Set up the basics.
	VectorSet(self->mins, -16.0f, -16.0f, -32.0f);
	VectorSet(self->maxs,  16.0f,  16.0f,  32.0f);
	self->s.scale = 1.0f;
	self->mass = 250;
	self->friction = 0.0f;
	self->gravity = 0.0f;
	self->s.effects |= EF_ALWAYS_ADD_EFFECTS;
	self->svflags |= SVF_ALWAYS_SEND;
	self->movetype = PHYSICSTYPE_NONE;
	self->solid = SOLID_NOT;
	self->touch = NULL;

	self->use = MiscMagicPortalUse;

	if (!(self->spawnflags & SF_START_OFF)) //BUGFIX: '!self->spawnflags & SF_START_OFF' in original logic.
		MiscMagicPortalUse(self, NULL, NULL); // Set up the touch function, since this baby is live.

	gi.linkentity(self);
}

#pragma endregion

#pragma region ========================== sound_ambient_xxx ==========================

#undef SF_START_OFF //mxd. Avoid compiler warnings...

#define SF_NON_LOCAL	1 //mxd
#define SF_START_OFF	2 //mxd

void SoundAmbientThink(edict_t* self) //mxd. Named 'soundambient_think' in original logic.
{
	const byte b_attenuation = (byte)self->attenuation;
	const byte b_volume = (byte)(self->volume * 255.0f);

	self->s.sound_data = (b_volume & ENT_VOL_MASK) | b_attenuation;

	// If its a looping sound, create it on this entity.
	switch (self->style)
	{
		case AS_FIRE:
			self->s.sound = (byte)gi.soundindex("ambient/fireplace.wav");
			break;

		case AS_WATERLAPPING:
			self->s.sound = (byte)gi.soundindex("ambient/waterlap.wav");
			break;

		case AS_OCEAN:
			self->s.sound = (byte)gi.soundindex("ambient/ocean.wav");
			break;

		case AS_SMALLFOUNTAIN:
			self->s.sound = (byte)gi.soundindex("ambient/smallfountain.wav");
			break;

		case AS_LARGEFOUNTAIN:
			self->s.sound = (byte)gi.soundindex("ambient/fountainloop.wav");
			break;

		case AS_SEWERWATER:
			self->s.sound = (byte)gi.soundindex("ambient/sewerflow.wav");
			break;

		case AS_OUTSIDEWATERWAY:
			self->s.sound = (byte)gi.soundindex("ambient/river.wav");
			break;

		case AS_CAULDRONBUBBLE:
			self->s.sound = (byte)gi.soundindex("ambient/cauldronbubble.wav");
			break;

		case AS_HUGEWATERFALL:
			self->s.sound = (byte)gi.soundindex("ambient/hugewaterfall.wav");
			break;

		case AS_MUDPOOL:
			self->s.sound = (byte)gi.soundindex("ambient/mudpool.wav");
			break;

		case AS_WINDEERIE:
			self->s.sound = (byte)gi.soundindex("ambient/windeerie.wav");
			break;

		case AS_WINDNOISY:
			self->s.sound = (byte)gi.soundindex("ambient/windnoisy.wav");
			break;

		case AS_WINDSOFTHI:
			self->s.sound = (byte)gi.soundindex("ambient/windsofthi.wav");
			break;

		case AS_WINDSOFTLO:
			self->s.sound = (byte)gi.soundindex("ambient/windsoftlow.wav");
			break;

		case AS_WINDSTRONG1:
			self->s.sound = (byte)gi.soundindex("ambient/windstrong1.wav");
			break;

		case AS_WINDSTRONG2:
			self->s.sound = (byte)gi.soundindex("ambient/windstrong2.wav");
			break;

		case AS_WINDWHISTLE:
			self->s.sound = (byte)gi.soundindex("ambient/windwhistle.wav");
			break;

		case AS_CONVEYOR:
			self->s.sound = (byte)gi.soundindex("objects/conveyor.wav");
			break;

		case AS_BUCKETCONVEYOR:
			self->s.sound = (byte)gi.soundindex("objects/bucketconveyor.wav");
			break;

		case AS_SPIT:
			self->s.sound = (byte)gi.soundindex("objects/spit.wav");
			break;

		default:
		{
			const byte b_style = (byte)self->style;
			const byte b_wait = (byte)self->wait;
			gi.CreatePersistantEffect(&self->s, FX_SOUND, CEF_BROADCAST | CEF_OWNERS_ORIGIN, self->s.origin, "bbbb", b_style, b_attenuation, b_volume, b_wait);
		} break;
	}

	self->count = 1; // This is just a flag to show it's on.
	self->think = NULL;
}

void SoundAmbientUse(edict_t* self, edict_t* other, edict_t* activator) //mxd. Named 'sound_ambient_use' in original logic.
{
	if (self->count != 0) // This is just a flag to show it's on.
	{
		self->count = 0;
		gi.RemoveEffects(&self->s, FX_REMOVE_EFFECTS);
	}
	else
	{
		SoundAmbientThink(self);
	}
}

static void SoundAmbientInit(edict_t* self) //mxd. Named 'sound_ambient_init' in original logic.
{
	VectorSet(self->mins, -4.0f, -4.0f, -4.0f);
	VectorSet(self->maxs,  4.0f,  4.0f,  4.0f);

	self->movetype = PHYSICSTYPE_NONE;

	if (self->attenuation <= 0.01f)
		self->attenuation = 1.0f;

	if (self->volume <= 0.01f)
		self->volume = 0.5f;

	if (self->wait < 1.0f)
		self->wait = 10.0f;

	self->s.effects |= EF_NODRAW_ALWAYS_SEND;

	if (!(self->spawnflags & SF_START_OFF))
	{
		self->think = SoundAmbientThink;
		self->nextthink = level.time + 2.5f;
	}

	// If we are asked to do a sound of type zero, free this edict, since its obviously bogus.
	if (self->style == 0)
	{
		gi.dprintf("Bogus ambient sound at %s\n", vtos(self->s.origin));
		G_SetToFree(self);

		return;
	}

	// If we are non-local, clear the origin of this object.
	if (self->spawnflags & SF_NON_LOCAL)
		VectorClear(self->s.origin);
	else
		assert(Vec3NotZero(self->s.origin)); // If we are here, then this ambient sound should have an origin.

	self->use = SoundAmbientUse;
	gi.linkentity(self);
}

// QUAKED sound_ambient_cloudfortress (1 0 0) (-4 -4 0) (4 4 4) NON_LOCAL START_OFF
// Generates an ambient sound for cloud fortress levels.

// Spawnflags:
// NON_LOCAL - sound occurs everywhere in the level - attenuation is not operative with this type of sound
// START_OFF - starts off, can be triggered on

// Variables:
// style:
//		1 - Cauldron bubbling (looping sound).
//		2 - Wind, low, eerie (looping).
//		3 - Wind, low, noisy (looping).
//		4 - Wind, high, soft (looping).
//		5 - Wind, low, soft (looping).
//		6 - Wind, low, strong (looping).
//		7 - Wind, high, strong (looping);
//		8 - Wind, whistling, strong (looping).
// attenuation	- How quickly sound drops off from origin. 0 - heard over entire level (default), 3 - diminish very rapidly with distance.
// volume		- Range of 0.1 to 1.0 (default 0.5); 0 - silent, 1 - full volume.
// wait			- Amount of seconds to wait + or - 50% before spawning sound again (default is 10 seconds).
void SP_sound_ambient_cloudfortress(edict_t* self)
{
	static const int cloud_sound_ids[AS_MAX] = //mxd. Made local static.
	{
		AS_NOTHING,
		AS_CAULDRONBUBBLE,
		AS_WINDEERIE,
		AS_WINDNOISY,
		AS_WINDSOFTHI,
		AS_WINDSOFTLO,
		AS_WINDSTRONG1,
		AS_WINDSTRONG2,
		AS_WINDWHISTLE
	};

	SoundAmbientInit(self);
	self->style = cloud_sound_ids[self->style];
}

// QUAKED sound_ambient_mine (1 0 0) (-4 -4 0) (4 4 4) NON_LOCAL START_OFF
// Generates an ambient sound for mine levels.

// Spawnflags:
// NON_LOCAL - sound occurs everywhere in the level - attenuation is not operative with this type of sound
// START_OFF - starts off, can be triggered on

// Variables:
// style:
//		1 - Mud pool bubbling (looping).
//		2 - Rocks falling (3 sounds).
//		3 - Wind, low, eerie (looping).
//		4 - Wind, low, soft (looping).
//		5 - Conveyor belt (looping).
//		6 - Bucket conveyor belt (looping).
//		7 - Three different creaks of heavy timbers.
// attenuation	- How quickly sound drops off from origin. 0 - heard over entire level (default), 3 - diminish very rapidly with distance.
// volume		- Range of 0.1 to 1.0 (default 0.5); 0 - silent, 1 - full volume.
// wait			- Amount of seconds to wait + or - 50% before spawning sound again (default is 10 seconds).
void SP_sound_ambient_mine(edict_t* self)
{
	static const int mine_sound_ids[AS_MAX] = //mxd. Made local static.
	{
		AS_NOTHING,
		AS_MUDPOOL,
		AS_ROCKS,
		AS_WINDEERIE,
		AS_WINDSOFTLO,
		AS_CONVEYOR,
		AS_BUCKETCONVEYOR,
		AS_CAVECREAK
	};

	SoundAmbientInit(self);
	self->style = mine_sound_ids[self->style];
}

// QUAKED sound_ambient_hive (1 0 0) (-4 -4 0) (4 4 4) NON_LOCAL START_OFF
// Generates an ambient sound for hive levels.

// Spawnflags:
// NON_LOCAL - sound occurs everywhere in the level - attenuation is not operative with this type of sound
// START_OFF - starts off, can be triggered on

// Variables:
// style:
//		1 - Gong.
//		2 - Wind, low, eerie (looping).
//		3 - Wind, low, noisy (looping).
//		4 - Wind, high, soft (looping).
//		5 - Wind, low, soft (looping).
//		6 - Wind, low, strong (looping).
//		7 - Wind, high, strong (looping).
//		8 - Wind, whistling, strong (looping).
// attenuation	- How quickly sound drops off from origin. 0 - heard over entire level (default), 3 - diminish very rapidly with distance.
// volume		- Range of 0.1 to 1.0 (default 0.5); 0 - silent, 1 - full volume.
// wait			- Amount of seconds to wait + or - 50% before spawning sound again (default is 10 seconds).
void SP_sound_ambient_hive(edict_t* self)
{
	static const int hive_sound_ids[AS_MAX] = //mxd. Made local static.
	{
		AS_NOTHING,
		AS_GONG,
		AS_WINDEERIE,
		AS_WINDNOISY,
		AS_WINDSOFTHI,
		AS_WINDSOFTLO,
		AS_WINDSTRONG1,
		AS_WINDSTRONG2,
		AS_WINDWHISTLE
	};

	SoundAmbientInit(self);
	self->style = hive_sound_ids[self->style];
}

// QUAKED sound_ambient_andoria (1 0 0) (-4 -4 0) (4 4 4) NON_LOCAL START_OFF
// Generates an ambient sound for andoria levels.

// Spawnflags:
// NON_LOCAL - sound occurs everywhere in the level - attenuation is not operative with this type of sound
// START_OFF - starts off, can be triggered on

// Variables:
// style:
//		1 - Small fountain (constant loop).
//		2 - Large fountain (constant loop).
//		3 - Water running out of sewer (constant loop).
//		4 - Rushing waterway outside (constant loop).
//		5 - Wind chime.
// attenuation	- How quickly sound drops off from origin. 0 - heard over entire level (default), 3 - diminish very rapidly with distance.
// volume		- Range of 0.1 to 1.0 (default 0.5); 0 - silent, 1 - full volume.
// wait			- Amount of seconds to wait + or - 50% before spawning sound again (default is 10 seconds).
void SP_sound_ambient_andoria(edict_t* self)
{
	static const int andoria_sound_ids[AS_MAX] = //mxd. Made local static
	{
		AS_NOTHING,
		AS_SMALLFOUNTAIN,
		AS_LARGEFOUNTAIN,
		AS_SEWERWATER,
		AS_OUTSIDEWATERWAY,
		AS_WINDCHIME
	};

	SoundAmbientInit(self);
	self->style = andoria_sound_ids[self->style];
}

// QUAKED sound_ambient_swampcanyon (1 0 0) (-4 -4 0) (4 4 4) NON_LOCAL START_OFF
// Generates an ambient sound for swamp or canyon levels.

// Spawnflags:
// NON_LOCAL - sound occurs everywhere in the level - attenuation is not operative with this type of sound
// START_OFF - starts off, can be triggered on

// Variables:
// style:
//		1 - Bird, quick, high pitch.
//		2 - Bird, low, medium pitch.
//		3 - Huge waterfall.
//		4 - Mud pool bubbling (looping).
//		5 - Wind, low, eerie (looping).
//		6 - Wind, low, noisy (looping).
//		7 - Wind, high, soft (looping).
//		8 - Wind, low, soft (looping).
//		9 - Wind, low, strong (looping).
//		10 - Wind, high, strong (looping).
//		11 - Wind, whistling, strong (looping).
// attenuation	- How quickly sound drops off from origin. 0 - heard over entire level (default), 3 - diminish very rapidly with distance.
// volume		- Range of 0.1 to 1.0 (default 0.5); 0 - silent, 1 - full volume.
// wait			- Amount of seconds to wait + or - 50% before spawning sound again (default is 10 seconds).
void SP_sound_ambient_swampcanyon(edict_t* self)
{
	static const int swamp_canyon_sound_ids[AS_MAX] = //mxd. Made local static.
	{
		AS_NOTHING,
		AS_BIRD1,
		AS_BIRD2,
		AS_HUGEWATERFALL,
		AS_MUDPOOL,
		AS_WINDEERIE,
		AS_WINDNOISY,
		AS_WINDSOFTHI,
		AS_WINDSOFTLO,
		AS_WINDSTRONG1,
		AS_WINDSTRONG2,
		AS_WINDWHISTLE,
	};

	self->style = swamp_canyon_sound_ids[self->style];
	SoundAmbientInit(self);
}

// QUAKED sound_ambient_silverspring (1 0 0) (-4 -4 0) (4 4 4) NON_LOCAL START_OFF
// Generates an ambient sound for silverspring levels.

// Spawnflags:
// NON_LOCAL - sound occurs everywhere in the level - attenuation is not operative with this type of sound
// START_OFF - starts off, can be triggered on

// Variables:
// style:
//		1 - Fire (looping).
//		2 - Water lapping (looping).
//		3 - Seagulls (2 random calls).
//		4 - Ocean.
//		5 - Birds (10 random bird calls).
//		6 - Crickets (3 random chirps).
//		7 - Frogs (2 random ribbits).
//		8 - Distant women/children crying (4 total).
//		9 - Mosquitoes (2 random sounds).
//		10 - Bubbles.
//		11 - Bell tolling.
//		12 - Footsteps (3 random sounds).
//		13 - Moans/screams/coughing (5 random sounds).
//		14 - Sewer drips (3 random sounds).
//		15 - Water drips (3 random sounds).
//		16 - Solid drips - heavy liquid (3 random sounds).
//		17 - Cauldron bubbling (looping sound).
//		18 - Creaking for the spit that's holding the elf over a fire.
// attenuation	- How quickly sound drops off from origin. 0 - heard over entire level (default), 3 - diminish very rapidly with distance.
// volume		- Range of 0.1 to 1.0 (default 0.5); 0 - silent, 1 - full volume.
// wait			- Amount of seconds to wait + or - 50% before spawning sound again (default is 10 seconds).
void SP_sound_ambient_silverspring(edict_t* self)
{
	static const int silver_spring_sound_ids[AS_MAX] = //mxd. Made local static.
	{
		AS_NOTHING,
		AS_FIRE,
		AS_WATERLAPPING,
		AS_SEAGULLS,
		AS_OCEAN,
		AS_BIRDS,
		AS_CRICKETS,
		AS_FROGS,
		AS_CRYING,
		AS_MOSQUITOES,
		AS_BUBBLES,
		AS_BELL,
		AS_FOOTSTEPS,
		AS_MOANS,
		AS_SEWERDRIPS,
		AS_WATERDRIPS,
		AS_HEAVYDRIPS,
		AS_CAULDRONBUBBLE,
		AS_SPIT
	};

	self->style = silver_spring_sound_ids[self->style];
	SoundAmbientInit(self);
}

#pragma endregion

#pragma region ========================== misc_remote_camera ==========================

#define SF_ACTIVATING	1 //mxd
#define SF_SCRIPTED		2 //mxd
#define SF_NO_DELETE	4 //mxd

static void MiscRemoteCameraRemove(edict_t* self) //mxd. Named 'remove_camera' in original logic.
{
	if (self->spawnflags & SF_ACTIVATING)
	{
		// Just for the activator.
		self->activator->client->RemoteCameraLockCount--;
	}
	else
	{
		// For all clients.
		for (int i = 0; i < game.maxclients; i++)
		{
			const edict_t* client = &g_edicts[i + 1];

			if (client->inuse)
				client->client->RemoteCameraLockCount--;
		}
	}

	if (!(self->spawnflags & SF_NO_DELETE))
		G_FreeEdict(self);
}

static void MiscRemoteCameraUpdateViewangles(const edict_t* self) //mxd. Added to reduce code duplication.
{
	if (self->spawnflags & SF_ACTIVATING)
	{
		// Just for the activator.
		if (self->activator->client->RemoteCameraNumber == self->s.number)
			for (int i = 0; i < 3; i++)
				self->activator->client->ps.remote_viewangles[i] = self->s.angles[i];
	}
	else
	{
		// For all clients.
		for (int i = 0; i < game.maxclients; i++)
		{
			const edict_t* client = &g_edicts[i + 1];

			if (client->inuse && client->client->RemoteCameraNumber == self->s.number)
				for (int j = 0; j < 3; j++)
					client->client->ps.remote_viewangles[j] = self->s.angles[j];
		}
	}
}

static void MiscRemoteCameraUpdateVieworigin(const edict_t* self) //mxd. Added to reduce code duplication.
{
	if (self->spawnflags & SF_ACTIVATING)
	{
		// Just for the activator.
		if (self->activator->client->RemoteCameraNumber == self->s.number)
			for (int i = 0; i < 3; i++)
				self->activator->client->ps.remote_vieworigin[i] = self->s.origin[i] * 8.0f;
	}
	else
	{
		// For all clients.
		for (int i = 0; i < game.maxclients; i++)
		{
			const edict_t* client = &g_edicts[i + 1];

			if (client->inuse && client->client->RemoteCameraNumber == self->s.number)
				for (int j = 0; j < 3; j++)
					client->client->ps.remote_vieworigin[j] = self->s.origin[j] * 8.0f;
		}
	}
}

void MiscRemoteCameraThink(edict_t* self) //mxd. Named 'misc_remote_camera_think' in original logic.
{
	// Attempt to find my owner entity (i.e. what I'm fixed to). If nothing is found, then my position will remain unchanged.
	if (self->pathtarget != NULL)
		self->enemy = G_Find(NULL, FOFS(targetname), self->pathtarget);

	if (self->enemy != NULL || (self->spawnflags & SF_SCRIPTED))
	{
		// I am attached to another (possibly moving) entity, so update my position.
		if (self->enemy != NULL)
			VectorCopy(self->enemy->s.origin, self->s.origin);

		// Update the position on client(s).
		MiscRemoteCameraUpdateVieworigin(self); //mxd
	}

	// Find my target entity and then orientate myself to look at it.
	self->targetEnt = G_Find(NULL, FOFS(targetname), self->target);

	if (self->targetEnt != NULL)
	{
		// Calculate the angles from myself to my target.
		vec3_t forward;
		VectorSubtract(self->targetEnt->s.origin, self->s.origin, forward);
		VectorNormalize(forward);
		vectoangles(forward, self->s.angles);
		self->s.angles[PITCH] = -self->s.angles[PITCH];

		// Update the angles on client(s).
		MiscRemoteCameraUpdateViewangles(self); //mxd
	}

	// Think again or remove myself?
	if (self->spawnflags & SF_SCRIPTED)
	{
		self->nextthink = level.time + FRAMETIME;
	}
	else
	{
		self->delay -= FRAMETIME; //mxd. Use define.

		if (self->delay >= 0.0f)
			self->nextthink = level.time + FRAMETIME; //mxd. Use define.
		else
			MiscRemoteCameraRemove(self);
	}
}

void MiscRemoteCameraUse(edict_t* self, edict_t* other, edict_t* activator) //mxd. Named 'Use_misc_remote_camera' in original logic.
{
	// If I am already active, just return, else flag that I am active.
	if (self->count != 0)
	{
		if (self->spawnflags & SF_SCRIPTED)
			MiscRemoteCameraRemove(self); // I am a scripted camera, so free myself before returning.

		return;
	}

	self->count = 1;

	// Signal to client(s) that a remote camera view is active.
	if (self->spawnflags & SF_ACTIVATING)
	{
		// Signal to just the activator (i.e. person who was ultimately responsible for triggering the remote camera)
		// that their camera view has changed to a remote camera view.
		self->activator = activator;
		self->activator->client->RemoteCameraLockCount++;
		self->activator->client->RemoteCameraNumber = self->s.number;
	}
	else
	{
		// Signal to all clients that their camera view has changed to a remote camera view..
		for (int i = 0; i < game.maxclients; i++)
		{
			const edict_t* client = &g_edicts[i + 1];

			if (client->inuse)
			{
				client->client->RemoteCameraLockCount++;
				client->client->RemoteCameraNumber = self->s.number;
			}
		}
	}

	// Attempt to find my owner entity (i.e. what I'm fixed to).
	// If nothing is found, then I am a static camera so set up my position here (it will remain unchanged hereafter).
	if (self->pathtarget == NULL)
	{
		// I am static, so set up my position (which will not change hereafter).
		if (self->spawnflags & SF_ACTIVATING)
		{
			// Just for the activator.
			self->enemy = NULL;

			for (int i = 0; i < 3; i++)
				self->activator->client->ps.remote_vieworigin[i] = self->s.origin[i] * 8.0f;
		}
		else
		{
			// For all clients.
			self->enemy = NULL;

			for (int i = 0; i < game.maxclients; i++)
			{
				const edict_t* client = &g_edicts[i + 1];

				if (client->inuse)
					for (int j = 0; j < 3; j++)
						client->client->ps.remote_vieworigin[j] = self->s.origin[j] * 8.0f;
			}
		}
	}
	else
	{
		self->enemy = G_Find(NULL, FOFS(targetname), self->pathtarget);

		if (self->enemy != NULL || (self->spawnflags & SF_SCRIPTED))
		{
			// I am attached to another (possibly moving) entity, so update my position.
			if (self->enemy != NULL)
				VectorCopy(self->enemy->s.origin, self->s.origin);

			// Update the position on client(s).
			MiscRemoteCameraUpdateVieworigin(self); //mxd
		}
	}

	// Find my target entity and then orientate myself to look at it.
	self->targetEnt = G_Find(NULL, FOFS(targetname), self->target);

	vec3_t forward;
	VectorSubtract(self->targetEnt->s.origin, self->s.origin, forward);
	VectorNormalize(forward);
	vectoangles(forward, self->s.angles);
	self->s.angles[PITCH] = -self->s.angles[PITCH];

	// Update the angles on client(s).
	MiscRemoteCameraUpdateViewangles(self); //mxd

	// Setup next think stuff.
	self->think = MiscRemoteCameraThink;
	self->nextthink = level.time + FRAMETIME;
}

// QUAKED misc_remote_camera (0 0.5 0.8) (-4 -4 -4) (4 4 4) ACTIVATING SCRIPTED NO_DELETE
// Spawnflags:
// ACTIVATING	- Only the activating client will see the remote camera view.
// SCRIPTED		- This is a scripted camera.
// NO_DELETE	- Don't delete camera.

// Variables:
// pathtarget	- Holds the name of the camera's owner entity (if any).
// target		- Holds the name of the entity to be looked at.
void SP_misc_remote_camera(edict_t* self)
{
	self->enemy = NULL;
	self->targetEnt = NULL;

	if (self->target == NULL)
	{
		gi.dprintf("Object 'misc_remote_camera' without a target.\n");
		G_FreeEdict(self);

		return;
	}

	self->movetype = PHYSICSTYPE_NONE;
	self->solid = SOLID_NOT;
	VectorSet(self->mins, -4.0f, -4.0f, -4.0f);
	VectorSet(self->maxs, 4.0f, 4.0f, 4.0f);
	self->count = 0; //TODO: add a dedicated 'qboolean is_active' property to one of many edict_t unions?..
	self->use = MiscRemoteCameraUse;

	gi.linkentity(self);
}

#pragma endregion

#pragma region ========================== misc_fire_sparker ==========================

#define SF_FIREBALL	1 //mxd

void MiscFireSparkerThink(edict_t* self) //mxd. Named 'fire_spark_think' in original logic.
{
	if (self->delay > 0.0f && self->delay < level.time)
	{
		G_FreeEdict(self);
	}
	else
	{
		self->think = MiscFireSparkerThink;
		self->nextthink = level.time + FRAMETIME; //mxd. Use define.
	}
}

void MiscFireSparkerRemove(edict_t* self, edict_t* other, edict_t* activator) //mxd. Named 'fire_spark_gone' in original logic.
{
	self->use = NULL;
	gi.RemoveEffects(&self->s, FX_SPARKS);

	G_FreeEdict(self);
}

void MiscFireSparkerUse(edict_t* self, edict_t* other, edict_t* activator) //mxd. Named 'fire_spark_use' in original logic.
{
	gi.CreateEffect(&self->s, FX_SPARKS, CEF_FLAG6 | CEF_FLAG7 | CEF_FLAG8, self->s.origin, "d", vec3_up);

	self->use = MiscFireSparkerRemove;
	self->think = MiscFireSparkerThink;
	self->nextthink = level.time + FRAMETIME; //mxd. Use define.
}

// QUAKED misc_fire_sparker (0 0 0) (-4 -4 0) (4 4 8) FIREBALL
// Fires sparks when used. Using a second time removes it.
// Spawnflags:
// FIREBALL - More of a poofy fireball trail.
// Variables:
// delay - How long to live for (default is forever).
void SP_misc_fire_sparker(edict_t* self)
{
	if (self->spawnflags & SF_FIREBALL)
		self->s.effects |= EF_MARCUS_FLAG1;

	self->svflags |= SVF_ALWAYS_SEND;
	self->solid = SOLID_NOT;
	self->movetype = PHYSICSTYPE_NOCLIP;
	self->clipmask = 0;

	self->use = MiscFireSparkerUse;
}

#pragma endregion

#pragma region ========================== misc_flag ==========================

void MiscFlagThink(edict_t* self) //mxd. Named 'flag_think' in original logic.
{
	if (++self->s.frame > 10)
		self->s.frame = 0;

	self->nextthink = level.time + FRAMETIME;
}

// QUAKED misc_flag (1 .5 0) (-10 -10 0) (10 10 80)
void SP_misc_flag(edict_t* self) //mxd. Defined in m_FMtest.c in original logic.
{
	VectorSet(self->mins, -10.0f, -10.0f, 0.0f);
	VectorSet(self->maxs, 10.0f, 10.0f, 80.0f);

	gi.setmodel(self, "models/rj5/tris.fm"); //TODO: model does not exist.

	self->movetype = PHYSICSTYPE_NONE;
	self->solid = SOLID_BBOX;

	self->think = MiscFlagThink;
	self->nextthink = level.time + flrand(FRAMETIME, 1.0f); //mxd. flrand(0.0f, 1.0f) in original logic.

	gi.linkentity(self);
}

#pragma endregion