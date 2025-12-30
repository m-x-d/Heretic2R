//
// g_monster.c
//
// Copyright 1998 Raven Software
//

#include <float.h> //mxd
#include "g_monster.h"
#include "g_debris.h" //mxd
#include "g_DefaultMessageHandler.h"
#include "g_items.h" //mxd
#include "g_playstats.h"
#include "m_stats.h"
#include "mg_guide.h"
#include "q_Physics.h" //mxd
#include "Random.h"
#include "Vector.h"
#include "g_local.h"

// Returns the modified health for a monster based on the number of players who will be in a game.
int MonsterHealth(int health)
{
	if (SKILL > SKILL_MEDIUM)
		health += (int)((float)health * (skill->value - 1.0f) / 2.0f); // 150% on Armageddon (skill 3).

	return health + (int)((float)health * ((float)(game.maxclients - 1) * 0.25f)); // 175% with 4 players.
}

static void M_CheckGround(edict_t* ent)
{
	if (ent->flags & (FL_SWIM | FL_FLY) || ent->velocity[2] >= 50.0f)
		return;

	// If the hull point one-quarter unit down is solid the entity is on ground.
	const vec3_t point = { ent->s.origin[0], ent->s.origin[1], ent->s.origin[2] - 0.25f };

	trace_t trace;
	gi.trace(ent->s.origin, ent->mins, ent->maxs, point, ent, MASK_MONSTERSOLID, &trace);

	if (!trace.startsolid && !trace.allsolid && trace.plane.normal[2] >= GROUND_NORMAL) // Check steepness. //mxd. Use define.
	{
		VectorCopy(trace.endpos, ent->s.origin);

		ent->groundentity = trace.ent;
		ent->groundentity_linkcount = trace.ent->linkcount;
		ent->velocity[2] = 0.0f;
	}
}

static void M_MonsterCatagorizePosition(edict_t* ent) //mxd. Named 'M_MonsterCatPos' in original logic. MonsterCat! =^.^=
{
	// Get waterlevel.
	vec3_t point = { ent->s.origin[0], ent->s.origin[1], ent->s.origin[2] + ent->mins[2] + 1.0f };

	int content = gi.pointcontents(point);
	if (!(content & MASK_WATER))
	{
		ent->waterlevel = 0;
		ent->watertype = 0;

		return;
	}

	ent->watertype = content;
	ent->waterlevel = 1; // Below knees.
	point[2] += ent->size[2] * 0.25f - 1.0f; // Quarter of way up.

	content = gi.pointcontents(point);
	if (!(content & MASK_WATER))
		return;

	ent->waterlevel = 2; // Between knees and head.
	point[2] = ent->absmax[2]; // Over head.

	content = gi.pointcontents(point);
	if (content & MASK_WATER)
		ent->waterlevel = 3; // All the way in.
}

void M_CatagorizePosition(edict_t* ent)
{
	if (ent->client == NULL)
	{
		M_MonsterCatagorizePosition(ent);
		return;
	}

	// Get waterlevel.
	vec3_t point = { ent->s.origin[0], ent->s.origin[1], ent->s.origin[2] + ent->mins[2] + 1.0f };

	int contents = gi.pointcontents(point);
	if (!(contents & MASK_WATER))
	{
		ent->waterlevel = 0;
		ent->watertype = 0;

		return;
	}

	ent->watertype = contents;
	ent->waterlevel = 1; // Below knees.
	point[2] += 26.0f;

	contents = gi.pointcontents(point);
	if (!(contents & MASK_WATER))
		return;

	ent->waterlevel = 2; // Between knees and head.
	point[2] += 22.0f;

	contents = gi.pointcontents(point);
	if (contents & MASK_WATER)
		ent->waterlevel = 3; // All the way in.
}

void M_WorldEffects(edict_t* ent)
{
	// Apply drowning/suffocation damage?
	if (ent->health > 0)
	{
		if (!(ent->flags & FL_SWIM))
		{
			if (ent->waterlevel < 3 || (ent->monsterinfo.aiflags & AI_SWIM_OK))
			{
				ent->air_finished = level.time + M_HOLD_BREATH_TIME;
			}
			else if (ent->air_finished < level.time && ent->pain_debounce_time < level.time && !(ent->flags & FL_AMPHIBIAN))
			{
				// Drown!
				int dmg = 2 + (int)(floorf(level.time - ent->air_finished)) * 2;
				dmg = min(15, dmg);
				T_Damage(ent, world, world, vec3_origin, ent->s.origin, vec3_origin, dmg, 0, DAMAGE_SUFFOCATION, MOD_WATER);

				ent->pain_debounce_time = level.time + 1.0f;
			}
		}
		else
		{
			if (ent->waterlevel > 0 || (ent->monsterinfo.aiflags & AI_SWIM_OK))
			{
				ent->air_finished = level.time + 9.0f;
			}
			else if (ent->air_finished < level.time && ent->pain_debounce_time < level.time && !(ent->flags & FL_AMPHIBIAN))
			{
				// Suffocate!
				int dmg = 2 + (int)(floorf(level.time - ent->air_finished)) * 2;
				dmg = min(15, dmg);
				T_Damage(ent, world, world, vec3_origin, ent->s.origin, vec3_origin, dmg, 0, DAMAGE_SUFFOCATION, MOD_WATER);

				ent->pain_debounce_time = level.time + 1.0f;
			}
		}
	}

	// Play lava/slime/water exit sound?
	if (ent->waterlevel == 0)
	{
		// FL_INWATER is set whether in lava, slime or water.
		if (ent->flags & FL_INWATER)
		{
			if (ent->flags & FL_INLAVA)
			{
				gi.sound(ent, CHAN_BODY, gi.soundindex("player/inlava.wav"), 1.0f, ATTN_NORM, 0.0f);
				ent->flags &= ~FL_INLAVA;
			}
			else if (ent->flags & FL_INSLIME)
			{
				gi.sound(ent, CHAN_BODY, gi.soundindex("player/muckexit.wav"), 1.0f, ATTN_NORM, 0.0f);
				ent->flags &= ~FL_INSLIME;
			}
			else
			{
				gi.sound(ent, CHAN_BODY, gi.soundindex("player/Water Exit.wav"), 1.0f, ATTN_NORM, 0.0f);
			}

			ent->flags &= ~FL_INWATER;
		}

		return;
	}

	// Apply lava damage?
	if ((ent->watertype & CONTENTS_LAVA) && !(ent->flags & FL_IMMUNE_LAVA) && ent->damage_debounce_time < level.time)
	{
		ent->damage_debounce_time = level.time + 0.2f;
		T_Damage(ent, world, world, vec3_origin, ent->s.origin, vec3_origin, ent->waterlevel * 10, 0, DAMAGE_LAVA, MOD_LAVA);
	}

	// Apply slime damage?
	if ((ent->watertype & CONTENTS_SLIME) && !(ent->flags & FL_IMMUNE_SLIME) && ent->damage_debounce_time < level.time)
	{
		ent->damage_debounce_time = level.time + 1.0f;
		T_Damage(ent, world, world, vec3_origin, ent->s.origin, vec3_origin, ent->waterlevel * 4, 0, DAMAGE_SLIME, MOD_SLIME);
	}

	// Play lava/slime/water enter sound?
	if (!(ent->flags & FL_INWATER))
	{
		if (ent->watertype & CONTENTS_LAVA)
		{
			gi.sound(ent, CHAN_BODY, gi.soundindex("player/inlava.wav"), 1.0f, ATTN_NORM, 0.0f);
			ent->flags |= FL_INLAVA;
		}
		else if (ent->watertype & CONTENTS_SLIME)
		{
			gi.sound(ent, CHAN_BODY, gi.soundindex("player/muckin.wav"), 1.0f, ATTN_NORM, 0.0f);
			ent->flags |= FL_INSLIME;
		}
		else
		{
			gi.sound(ent, CHAN_BODY, gi.soundindex("player/Water Enter.wav"), 1.0f, ATTN_NORM, 0.0f);
		}

		// FL_INWATER is set whether in lava, slime or water.
		ent->flags |= FL_INWATER;
		ent->damage_debounce_time = 0.0f;
	}
}

void M_DropToFloor(edict_t* ent) //mxd. Named 'M_droptofloor' in original logic.
{
	if (Vec3IsZero(ent->mins) && Vec3IsZero(ent->maxs))
	{
		gi.dprintf("ERROR: %s at %s called drop to floor before having size set\n", ent->classname, vtos(ent->s.origin));

		if (ent->think == M_DropToFloor)
			ent->think = NULL; // Don't try again.

		return;
	}

	//TODO: check if ent->think is M_droptofloor before setting nextthink?
	ent->nextthink = level.time + FRAMETIME;
	ent->s.origin[2] += 1.0f;

	trace_t trace;
	const vec3_t end = VEC3_INITA(ent->s.origin, 0.0f, 0.0f, -256.0f);
	gi.trace(ent->s.origin, ent->mins, ent->maxs, end, ent, MASK_MONSTERSOLID, &trace);

	if (trace.allsolid || trace.startsolid)
	{
		gi.dprintf("ERROR: %s at %s started in solid\n", ent->classname, vtos(ent->s.origin));

		if (ent->think == M_DropToFloor)
		{
			ent->think = NULL; // Don't try again.
			gi.linkentity(ent);
			M_CatagorizePosition(ent);
		}

		return;
	}

	if (trace.fraction == 1.0f)
	{
		gi.dprintf("ERROR: %s at %s more than 256 off ground, waiting to fall\n", ent->classname, vtos(ent->s.origin)); //mxd. Print origin.
		return; //TODO: in this case, ent will stay floating in the air. Still copy trace.endpos to ent->s.origin, but don't clear think callback? Increase end offset?
	}

	VectorCopy(trace.endpos, ent->s.origin);

	gi.linkentity(ent);
	M_CheckGround(ent);
	M_CatagorizePosition(ent);

	// No need to think anymore if on the ground.
	ent->think = NULL; //TODO: check if ent->think is M_droptofloor before clearing it?
}

// Unless a nextframe is specified, advance to the next frame listed in the Animation Frame Array.
// Execute any aifunction or think function specified with the given frame.
void M_MoveFrame(edict_t* self)
{
	const qboolean was_new_phys = (self->movetype < NUM_PHYSICSTYPES);
	const animmove_t* move = self->monsterinfo.currentmove;

	if (move == NULL)
	{
		// If move is NULL, then this monster needs to have an anim set on it or all is lost.
		self->think = NULL;
		self->nextthink = THINK_NEVER; //mxd. Use define.

		return;
	}

	//mxd. Add sanity check.
	assert(self->monsterinfo.thinkinc > 0.0f);

	self->nextthink = level.time + self->monsterinfo.thinkinc;

	// There is a voice sound waiting to play.
	if (self->monsterinfo.sound_pending != 0 && self->monsterinfo.sound_start <= level.time)
	{
		G_PostMessage(self, MSG_VOICE_PUPPET, PRI_DIRECTIVE, "i", self->monsterinfo.sound_pending);
		self->monsterinfo.sound_pending = 0;
	}

	// If this is set, the monster runs absolutely no animations or ai.
	if (SV_FREEZEMONSTERS)
		return;

	// Forcing the next frame index - usually the start of an animation.
	if (self->monsterinfo.nextframeindex > -1)
	{
		self->monsterinfo.currframeindex = self->monsterinfo.nextframeindex;
		self->monsterinfo.nextframeindex = -1;
	}
	else
	{
		// Advance animation frame index.
		if (!(self->monsterinfo.aiflags & AI_HOLD_FRAME))
		{
			self->monsterinfo.currframeindex++;

			if (self->monsterinfo.currframeindex >= move->numframes)
				self->monsterinfo.currframeindex = 0;
		}

		// Call endfunc?
		if (self->monsterinfo.currframeindex == move->numframes - 1 && move->endfunc != NULL)
		{
			move->endfunc(self);

			// Re-grab move, endfunc is very likely to change it.
			if (move != self->monsterinfo.currentmove)
			{
				move = self->monsterinfo.currentmove;

				//mxd. If SetAnim() was called from endfunc(), we'll end up with currframeindex:0 and nextframeindex:0 during this and NEXT frame, so let's fix that...
				if (self->monsterinfo.currframeindex == 0 && self->monsterinfo.nextframeindex == 0)
					self->monsterinfo.nextframeindex = -1; // Advance currframeindex during next M_MoveFrame() call.
			}

			// Check for death.
			if (self->svflags & SVF_DEADMONSTER)
				return;
		}
	}

	// Apply animation frame index.
	const int index = self->monsterinfo.currframeindex;
	const animframe_t* frame = &move->frame[index]; //mxd
	self->s.frame = (short)frame->framenum;

	// This is consistent with the animmove_t in the monster anims.
	// Currently all of the *real* movement happens in the "actionfunc" instead of the move func.
	if (!(self->monsterinfo.aiflags & AI_DONT_THINK))
	{
		if (frame->movefunc != NULL)
			frame->movefunc(self, frame->var1, frame->var2, frame->var3);

		if (frame->actionfunc != NULL)
		{
			const float var4 = ((self->monsterinfo.aiflags & AI_HOLD_FRAME) ? 0.0f : move->frame[index].var4); //mxd // Put scaling into SV_Movestep since this isn't ALWAYS the movement function.
			move->frame[index].actionfunc(self, var4);
		}

		if (frame->thinkfunc != NULL)
			frame->thinkfunc(self);
	}

	if (was_new_phys)
		assert(self->movetype < NUM_PHYSICSTYPES);
}

void M_Think(edict_t* self) //mxd. Named 'monster_think' in original logic.
{
	M_MoveFrame(self);
	M_CatagorizePosition(self);
	M_WorldEffects(self);
}

// Using a monster makes it angry at the current activator.
void M_Use(edict_t* self, edict_t* other, edict_t* activator) //mxd. Named 'monster_use' in original logic.
{
	if (self->enemy != NULL || self->health <= 0 || (activator->flags & FL_NOTARGET))
		return;

	if (activator->client == NULL && !(activator->monsterinfo.aiflags & AI_GOOD_GUY))
		return;

	// Delay reaction so if the monster is teleported, its sound is still heard.
	self->enemy = activator;
	AI_FoundTarget(self, true);
}

void M_TriggeredSpawnThink(edict_t* self) //mxd. Named 'monster_triggered_spawn' in original logic.
{
	self->s.origin[2] += 1.0f;
	KillBox(self);

	self->solid = SOLID_BBOX;
	self->movetype = PHYSICSTYPE_STEP;
	self->svflags &= ~SVF_NOCLIENT;
	self->air_finished = level.time + M_HOLD_BREATH_TIME;
	gi.linkentity(self);

	M_MonsterStartGo(self);

	if (self->classID == CID_ASSASSIN && (self->spawnflags & MSF_ASS_TPORTAMBUSH))
	{
		AI_FoundTarget(self, true);

		const vec3_t smoke_pos = VEC3_INITA(self->s.origin, 0.0f, 0.0f, self->mins[2]);
		gi.CreateEffect(NULL, FX_TPORTSMOKE, 0, smoke_pos, "");
	}
	else if (self->enemy != NULL && !(self->spawnflags & MSF_AMBUSH) && !(self->enemy->flags & FL_NOTARGET))
	{
		AI_FoundTarget(self, true);
	}
	else
	{
		self->enemy = NULL;
	}
}

void M_TriggeredSpawnUse(edict_t* self, edict_t* other, edict_t* activator) //mxd. Named 'monster_triggered_spawn_use' in original logic.
{
	// We have a one frame delay here so we don't telefrag the guy who activated us.
	self->spawnflags &= ~MSF_ASLEEP;
	self->think = M_TriggeredSpawnThink;
	self->nextthink = level.time + FRAMETIME;

	if (activator->client != NULL)
		self->enemy = activator;

	self->use = M_Use;
}

static void M_TriggeredStart(edict_t* self) //mxd. Named 'monster_triggered_start' in original logic.
{
	self->solid = SOLID_NOT;
	self->movetype = PHYSICSTYPE_NONE;
	self->svflags |= SVF_NOCLIENT;

	self->think = NULL; //mxd. Not cleared in original logic. Will be reassigned by M_TriggeredSpawnUse() anyway.
	self->nextthink = THINK_NEVER; //mxd. '0' in original logic. Changed for consistency sake.

	self->use = M_TriggeredSpawnUse;
}

// When a monster dies, it fires all of its targets with the current enemy as activator.
void M_DeathUse(edict_t* self) //mxd. Named 'monster_death_use' in original logic.
{
	self->flags &= ~(FL_FLY | FL_SWIM);

	if (self->item != NULL)
	{
		Drop_Item(self, self->item);
		self->item = NULL;
	}

	if (self->target != NULL)
		G_UseTargets(self, self->enemy);

	if (self->deathtarget != NULL)
	{
		self->target = self->deathtarget;
		G_UseTargets(self, self->enemy);
	}
}

static void M_CheckInGround(edict_t* self) //mxd. Named 'MG_CheckInGround' in original logic.
{
	if (gi.pointcontents(self->s.origin) & CONTENTS_SOLID)
	{
		gi.dprintf("%s's origin at %s in solid!\n", self->classname, vtos(self->s.origin));
		return;
	}

	// Check down against world - does not check against entities! Does not check up against ceiling (why would they put one close to a ceiling?).
	const vec3_t top =    VEC3_INITA(self->s.origin, 0.0f, 0.0f, self->maxs[2] - 1.0f);
	const vec3_t bottom = VEC3_INITA(self->s.origin, 0.0f, 0.0f, self->mins[2]);

	const vec3_t mins = { self->mins[0], self->mins[1], 0.0f };
	const vec3_t maxs = { self->maxs[0], self->maxs[1], 1.0f };

	trace_t trace;
	gi.trace(top, mins, maxs, bottom, self, MASK_SOLID, &trace);

	if (trace.allsolid || trace.startsolid) // Monster in solid, can't be fixed.
	{
		gi.dprintf("Top of %s at %s in solid architecture (%s)!\n", self->classname, vtos(self->s.origin), trace.ent->classname);
	}
	else if (trace.fraction < 1.0f)
	{
		// Monster is in the ground.
		const vec3_t new_org = VEC3_INITA(trace.endpos, 0.0f, 0.0f, -self->mins[2]);

		if ((int)new_org[2] != (int)self->s.origin[2]) //mxd. Original logic compares trace.endpos[2] with origin[2].
			gi.dprintf("%s at %s was in ground (%s), moved to %s!\n", self->classname, vtos(self->s.origin), trace.ent->classname, vtos(new_org));

		VectorCopy(new_org, self->s.origin);
	}

	//FIXME: check against other ents too? Same trace or second one?
}

qboolean M_Start(edict_t* self) //mxd. Named 'monster_start' in original logic.
{
	if (DEATHMATCH && !(SV_CHEATS & self_spawn))
	{
		G_FreeEdict(self);
		return false;
	}

	self->monsterinfo.awake = false;
	self->svflags |= SVF_MONSTER;
	self->s.renderfx |= RF_FRAMELERP;
	self->takedamage = DAMAGE_AIM;
	self->air_finished = level.time + M_HOLD_BREATH_TIME;
	self->nextthink = level.time + FRAMETIME;

	self->use = M_Use;
	self->touch = M_Touch;
	self->monsterinfo.alert = GenericMonsterAlerted; // I don't understand why I get a warning here...

	self->max_health = self->health;
	self->clipmask = MASK_MONSTERSOLID;

	if (self->materialtype == 0)
		self->materialtype = MAT_FLESH;

	// Stop the camera clipping with monsters, except the trial beast.
	if (self->classID != CID_TBEAST)
		self->s.effects |= EF_CAMERA_NO_CLIP;

	if (G_MonsterShadow[self->classID].use_shadow)
	{
		self->s.color.c = 0xFFFFFFFF; //mxd. Because ShadowAddToView() now checks owner alpha...
		gi.CreateEffect(&self->s, FX_SHADOW, CEF_OWNERS_ORIGIN, self->s.origin, "f", G_MonsterShadow[self->classID].scale * self->s.scale); //mxd. Scale by s.scale.
	}

	self->s.skinnum = 0;
	self->dead_state = DEAD_NO;
	self->svflags &= ~SVF_DEADMONSTER;
	self->monsterinfo.thinkinc = MONSTER_THINK_INC;
	self->monsterinfo.nextframeindex = -1;

	if (self->monsterinfo.checkattack == NULL)
		self->monsterinfo.checkattack = M_CheckAttack;

	VectorCopy(self->s.origin, self->s.old_origin);

	if (st.item != NULL)
	{
		self->item = P_FindItemByClassname(st.item);

		if (self->item == NULL)
			gi.dprintf("%s at %s has bad item: %s\n", self->classname, vtos(self->s.origin), st.item);
	}

	if (self->mass == 0)
		self->mass = 200;

	self->s.frame = 1;
	self->oldenemy_debounce_time = -1.0f;

	return true;
}

static void M_BBoxAndOriginAdjustForScale(edict_t* self) //mxd. Named 'MG_BBoxAndOriginAdjustForScale' in original logic.
{
	if (self->s.scale == 0.0f)
	{
		if (self->monsterinfo.scale == 0.0f)
			self->monsterinfo.scale = 1.0f;

		self->s.scale = self->monsterinfo.scale; //mxd. Set only when self->monsterinfo.scale is also 0 in original logic.
	}
	else if (self->monsterinfo.scale == 0.0f)
	{
		self->monsterinfo.scale = self->s.scale;
	}

	const float mins_z = self->mins[2];

	Vec3ScaleAssign(self->s.scale, self->mins);
	Vec3ScaleAssign(self->s.scale, self->maxs);

	self->s.origin[2] += mins_z - self->mins[2];

	gi.linkentity(self);
}

static void M_MonsterStartGo(edict_t* self) //mxd. Named 'monster_start_go' in original logic.
{
	self->nextthink = level.time + FRAMETIME;

	if (self->health <= 0)
		return;

	M_BBoxAndOriginAdjustForScale(self);
	M_CheckInGround(self);

	if (self->mass == 0)
		self->mass = 100;

	assert(self->s.scale >= 0.0f); //mxd. Original logic uses 'if (self->s.scale)' check below, so...

	if (self->s.scale > 0.0f)
		self->mass = (int)((float)self->mass * self->s.scale);

	if (self->spawnflags & MSF_COWARD) // Start off running away. //FIXME: let them specify a flee_time and use AI_FLEE if one is set? Would anyone ever use this?
		self->monsterinfo.aiflags |= AI_COWARD;

	if (self->spawnflags & MSF_STALK) // Stalks enemies - only approaches and attacks from behind.
		self->ai_mood_flags |= AI_MOOD_FLAG_BACKSTAB;

	if (self->spawnflags & MSF_MELEE_LEAD) // Lead enemies in melee and tries to cut them off.
		self->ai_mood_flags |= AI_MOOD_FLAG_PREDICT;

	if (self->wakeup_distance == 0.0f)
		self->wakeup_distance = MAX_SIGHT_PLAYER_DIST;

	if (VectorLength(self->size) < 32.0f)
		self->svflags |= SVF_DO_NO_IMPACT_DMG;

	self->jump_time = level.time + 2.0f; // So they don't take damage from the fall after spawning...

	// Check for target to combat_point and change to combattarget.
	self->monsterinfo.coop_check_debounce_time = 0.0f;
	self->monsterinfo.pausetime = -1.0f;

	if (self->enemy != NULL)
	{
		AI_FoundTarget(self, false); // Spawned mad.
	}
	else
	{
		// If self->target is point_combat, change it to self->combattarget.
		if (self->target != NULL)
		{
			edict_t* target = NULL;
			qboolean not_point_combat = false;
			qboolean clear_target = false;

			while ((target = G_Find(target, FOFS(targetname), self->target)) != NULL)
			{
				if (strcmp(target->classname, "point_combat") == 0)
				{
					self->combattarget = self->target;
					clear_target = true;
				}
				else
				{
					not_point_combat = true;
				}
			}

			if (not_point_combat && self->combattarget != NULL)
				gi.dprintf("%s at %s has target with mixed types\n", self->classname, vtos(self->s.origin));

			if (clear_target)
				self->target = NULL;
		}

		// Validate combattarget.
		if (self->combattarget != NULL)
		{
			edict_t* target = NULL;
			while ((target = G_Find(target, FOFS(targetname), self->combattarget)) != NULL)
				if (strcmp(target->classname, "point_combat") != 0)
					gi.dprintf("%s at %s has a bad combattarget %s: %s at %s!\n", self->classname, vtos(self->s.origin), self->combattarget, target->classname, vtos(target->s.origin));
		}

		if (self->target != NULL)
		{
			self->goalentity = G_PickTarget(self->target);
			self->movetarget = self->goalentity;

			if (self->movetarget == NULL)
			{
				gi.dprintf("%s can't find target %s at %s\n", self->classname, self->target, vtos(self->s.origin));

				self->target = NULL;
				self->monsterinfo.pausetime = FLT_MAX; //mxd. 100000000.0f in original logic.

				if (!self->monsterinfo.c_mode) // Not in cinematic mode.
					G_PostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
				else
					G_PostMessage(self, MSG_C_IDLE1, PRI_DIRECTIVE, "iiige", 0, 0, 0, NULL, NULL);
			}
			else if (strcmp(self->movetarget->classname, "path_corner") == 0)
			{
				if (self->classID != CID_SERAPH_OVERLORD)
				{
					vec3_t diff;
					VectorSubtract(self->goalentity->s.origin, self->s.origin, diff);
					self->ideal_yaw = VectorYaw(diff);
					self->s.angles[YAW] = self->ideal_yaw;

					G_PostMessage(self, MSG_WALK, PRI_DIRECTIVE, NULL);
					self->monsterinfo.pausetime = 0.0f;
				}
				else
				{
					self->goalentity = NULL;
					self->movetarget = NULL;
					self->monsterinfo.pausetime = FLT_MAX; //mxd. 100000000.0f in original logic.

					if (!self->monsterinfo.c_mode) // Not in cinematic mode.
						G_PostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
					else
						G_PostMessage(self, MSG_C_IDLE1, PRI_DIRECTIVE, "iiige", 0, 0, 0, NULL, NULL);
				}

				self->target = NULL;
			}
			else
			{
				self->goalentity = NULL;
				self->movetarget = NULL;
				self->monsterinfo.pausetime = FLT_MAX; //mxd. 100000000.0f in original logic.

				if (!self->monsterinfo.c_mode) // Not in cinematic mode.
					G_PostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
				else
					G_PostMessage(self, MSG_C_IDLE1, PRI_DIRECTIVE, "iiige", 0, 0, 0, NULL, NULL);
			}
		}
		else
		{
			self->monsterinfo.pausetime = FLT_MAX; //mxd. 100000000.0f in original logic.

			if (self->monsterinfo.aiflags & AI_EATING)
				G_PostMessage(self, MSG_EAT, PRI_DIRECTIVE, NULL);
			else if (!self->monsterinfo.c_mode) // Not in cinematic mode.
				G_PostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
			else
				G_PostMessage(self, MSG_C_IDLE1, PRI_DIRECTIVE, "iiige", 0, 0, 0, NULL, NULL);
		}
	}

	self->think = M_Think;
}

void M_WalkmonsterStartGo(edict_t* self) //mxd. Named 'walkmonster_start_go' in original logic.
{
	if (!(self->spawnflags & MSF_ASLEEP) && level.time < 1.0f)
	{
		M_DropToFloor(self);

		if (self->groundentity != NULL && !M_walkmove(self, 0.0f, 0.0f))
			gi.dprintf("%s in solid at %s\n", self->classname, vtos(self->s.origin));
	}

	if (self->yaw_speed == 0.0f)
		self->yaw_speed = 20.0f;

	self->viewheight = 25; //TODO: should set only when self->viewheight == 0 (like in M_FlymonsterStartGo())?

	//H2_BUGFIX: mxd. Original logic calls M_MonsterStartGo() regardless of MSF_ASLEEP flag, which results in M_BBoxAndOriginAdjustForScale() called twice for triggered monsters, resulting in incorrect bbox size.
	if (self->spawnflags & MSF_ASLEEP)
		M_TriggeredStart(self); // M_MonsterStartGo() will be called by M_TriggeredSpawnThink() --mxd.
	else
		M_MonsterStartGo(self);
}

qboolean M_WalkmonsterStart(edict_t* self) //mxd. Named 'walkmonster_start' in original logic.
{
	self->think = M_WalkmonsterStartGo;
	return M_Start(self);
}

void M_FlymonsterStartGo(edict_t* self) //mxd. Named 'flymonster_start_go' in original logic.
{
	if (!M_walkmove(self, 0.0f, 0.0f))
		gi.dprintf("%s in solid at %s\n", self->classname, vtos(self->s.origin));

	if (self->yaw_speed == 0.0f)
		self->yaw_speed = 10.0f;

	if (self->viewheight == 0)
		self->viewheight = 25;

	//H2_BUGFIX: mxd. Original logic calls M_MonsterStartGo() regardless of MSF_ASLEEP flag, which results in M_BBoxAndOriginAdjustForScale() called twice for triggered monsters, resulting in incorrect bbox size.
	if (self->spawnflags & MSF_ASLEEP)
		M_TriggeredStart(self); // M_MonsterStartGo() will be called by M_TriggeredSpawnThink() --mxd.
	else
		M_MonsterStartGo(self);
}

qboolean M_FlymonsterStart(edict_t* self) //mxd. Named 'flymonster_start' in original logic.
{
	self->flags |= FL_FLY;
	self->think = M_FlymonsterStartGo;

	return M_Start(self);
}

// This will adjust the pitch and roll of a monster to match a given slope.
// If a non-'0 0 0' slope is passed, it will use that value, otherwise it will use the ground underneath the monster.
// If it doesn't find a surface, it does nothing and returns.
void M_GetSlopePitchRoll(edict_t* ent, vec3_t pass_slope) //mxd. Named 'pitch_roll_for_slope' in original logic.
{
	vec3_t slope;

	if (pass_slope == NULL)
	{
		vec3_t end;
		VectorCopy(ent->s.origin, end);
		end[2] += ent->mins[2] - 300.0f;

		trace_t trace;
		gi.trace(ent->s.origin, vec3_origin, vec3_origin, end, ent, MASK_SOLID, &trace);

		if (trace.fraction == 1.0f || Vec3IsZero(trace.plane.normal))
			return;

		VectorCopy(trace.plane.normal, slope);
	}
	else
	{
		VectorCopy(pass_slope, slope);
	}

	vec3_t old_fwd;
	vec3_t old_right;
	AngleVectors(ent->s.angles, old_fwd, old_right, NULL);

	vec3_t new_angles;
	vectoangles(slope, new_angles);
	const float pitch = new_angles[PITCH] - 90.0f;

	new_angles[PITCH] = 0.0f;
	new_angles[ROLL] = 0.0f;

	vec3_t new_fwd;
	AngleVectors(new_angles, new_fwd, NULL, NULL);

	const float mod = Q_signf(DotProduct(new_fwd, old_right));
	const float dot = DotProduct(new_fwd, old_fwd);

	ent->s.angles[PITCH] = dot * pitch;
	ent->s.angles[ROLL] = (1.0f - fabsf(dot)) * pitch * mod;
}

#pragma region ========================== Monster Helper Functions ==========================

// Tests to see whether the thing touching it is on its head, and if so, it tries to correct that situation.
void M_Touch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf)
{
	// Skip if other is not monster or player.
	if (!(other->svflags & SVF_MONSTER) && Q_stricmp(other->classname, "player") != 0) //mxd. stricmp -> Q_stricmp
		return;

	vec3_t other_bottom;
	VectorCopy(other->s.origin, other_bottom);
	other_bottom[2] += other->mins[2];

	vec3_t self_top;
	VectorCopy(self->s.origin, self_top);
	self_top[2] += self->maxs[2];

	// Not on top?
	if (other_bottom[2] - self_top[2] < 0.0f)
		return;

	vec3_t dir;
	VectorCopy(other->velocity, dir);
	VectorNormalize(dir);

	// 10% chance to do damage.
	if (irand(0, 9) == 0)
	{
		const int damage = irand(1, 3);
		T_Damage(self, other, other, dir, self_top, vec3_origin, damage, 0, DAMAGE_NO_KNOCKBACK | DAMAGE_AVOID_ARMOR, MOD_DIED);
	}

	// Setup a random velocity for the top entity.
	other->velocity[0] = flrand(100.0f, 150.0f);
	other->velocity[1] = flrand(100.0f, 150.0f);
	other->velocity[2] += 110.0f;

	// Randomly reverse those random numbers. //TODO: don't reverse Z-axis?
	if (irand(0, 1) == 1)
		VectorScale(other->velocity, -1.0f, other->velocity);
}

// Test a melee strike to see if it has hit its target.
// Returns:	"trace.ent" if a valid entity is struck (may not be intended target).
//			"NULL" if nothing hit.
//			"attacker" if hit a wall, but no entity (used for spark effects). //TODO: that's not what the logic does...
// Args:
// attacker	- The entity attacking.
// max_dist	- The distance it checks forward.
// trace	- Passed parameter filled with the trace information (can be overkill, or very useful).
edict_t* M_CheckMeleeHit(edict_t* attacker, const float max_dist, trace_t* trace)
{
	// Trace forward the maximum amount of the melee distance.
	vec3_t forward;
	AngleVectors(attacker->s.angles, forward, NULL, NULL);

	vec3_t tgt_pos;
	VectorMA(attacker->s.origin, max_dist, forward, tgt_pos);

	gi.trace(attacker->s.origin, attacker->mins, attacker->maxs, tgt_pos, attacker, MASK_MONSTERSOLID, trace);

	// Check to see if the trace was successful (miss).
	if (trace->fraction < 1.0f)
	{
		// Check an entity collision.
		if (trace->ent != NULL && trace->ent->takedamage != DAMAGE_NO)
			return trace->ent;

		// Hit non-damageable entity or world geometry.
		return attacker;
	}

	// Hit nothing.
	return NULL;
}

// Test a melee attack along a directed line.
// Returns:	"trace.ent" if a valid entity is struck (may not be intended target).
//			"NULL" if nothing hit.
//			"attacker" if hit a wall, but no entity (used for spark effects).
// Args:
// attacker		- What's attacking.
// start		- Starting position of the attack (offsets from the character (f,r,u).
// end			- Ending position of the attack (offsets from the character (f,r,u).
// mins, maxs	- The size of the box to trace by
// trace		- Passed parameter filled with the trace information (can be overkill, or very useful)
// direction	- [out arg] Swipe direction.
edict_t* M_CheckMeleeLineHit(edict_t* attacker, const vec3_t start, const vec3_t end, const vec3_t mins, const vec3_t maxs, trace_t* trace, vec3_t direction)
{
	// Apply the offsets to the positions passed.
	vec3_t forward;
	vec3_t right;
	vec3_t up;
	AngleVectors(attacker->s.angles, forward, right, up);

	vec3_t start_pos;
	VectorMA(attacker->s.origin, start[0], forward, start_pos);
	VectorMA(start_pos, start[1], right, start_pos);
	VectorMA(start_pos, start[2], up, start_pos);

	vec3_t end_pos;
	VectorMA(attacker->s.origin, end[0], forward, end_pos);
	VectorMA(end_pos, end[1], right, end_pos);
	VectorMA(end_pos, end[2], up, end_pos);

	vec3_t swipe_dir;
	VectorSubtract(end_pos, start_pos, swipe_dir);

	// Make sure line to start of swipe is clear.
	gi.trace(attacker->s.origin, mins, maxs, start_pos, attacker, MASK_SHOT, trace);

	if (trace->fraction == 1.0f) // Line to start of trace not blocked.
		gi.trace(start_pos, mins, maxs, end_pos, attacker, MASK_SHOT, trace);

	if (trace->fraction == 1.0f)
	{
		// Hit nothing, trace to middle of line from origin to see if reached too far.
		vec3_t mid_pos;
		VectorMA(start_pos, 0.5f, swipe_dir, mid_pos);
		gi.trace(attacker->s.origin, mins, maxs, mid_pos, attacker, MASK_SHOT, trace);
	}

	if (trace->fraction == 1.0f) // Last chance: trace to end of swipe from origin to see if reached too far.
		gi.trace(attacker->s.origin, mins, maxs, end_pos, attacker, MASK_SHOT, trace);

	VectorNormalize(swipe_dir);

	if (direction != NULL)
		VectorCopy(swipe_dir, direction);

	// Check to see if the trace was successful (miss).
	if (trace->fraction < 1.0f || trace->startsolid || trace->allsolid)
	{
		// Check an entity collision.
		if (trace->ent != NULL && trace->ent->takedamage != DAMAGE_NO)
			return trace->ent;

		// Hit non-damageable entity or world geometry.
		return attacker;
	}

	// Nothing found (missed).
	return NULL;
}

// Make sure we have a live enemy, and then return a distance to him. Returns distance to target.
// Args:
// self		- What's attacking.
// target	- What we're looking for.
float M_DistanceToTarget(const edict_t* self, const edict_t* target)
{
	assert(target != NULL);

	vec3_t diff;
	VectorSubtract(target->s.origin, self->s.origin, diff);

	return VectorLength(diff);
}

// Make sure we have a live enemy, and then return a distance to it. Returns true if the enemy is valid, false if it is dead.
// Args:
// self		- What's attacking.
// target	- What we're looking for.
qboolean M_ValidOldEnemy(edict_t* self)
{
	if (self->oldenemy == NULL || self->oldenemy->health <= 0 || self->oldenemy == self)
		return false;

	if (self->monsterinfo.last_successful_enemy_tracking_time + MONSTER_SEARCH_TIME < level.time && !AI_IsVisible(self, self->oldenemy))
		return false;

	self->monsterinfo.aiflags &= ~AI_STRAIGHT_TO_ENEMY;
	self->enemy = self->oldenemy;
	self->goalentity = self->oldenemy;
	self->oldenemy = NULL;

	return true;
}

qboolean M_ValidTarget(edict_t* self, const edict_t* target)
{
	if (self->oldenemy_debounce_time > 0.0f && self->oldenemy_debounce_time < level.time)
	{
		self->oldenemy_debounce_time = -1.0f;

		if (M_ValidOldEnemy(self))
			return true;
	}

	const qboolean check_old_enemy = (target == self->enemy);

	if (target == NULL)
	{
		self->monsterinfo.aiflags &= ~AI_STRAIGHT_TO_ENEMY;

		// See if there is another valid target to go after.
		if (check_old_enemy && M_ValidOldEnemy(self))
			return true;

		if (!FindTarget(self))
			return false;

		target = self->enemy;
	}

	if (target == NULL)
		return false;

	// See if the target has died.
	if (target->health <= 0 || target == self)
	{
		self->monsterinfo.aiflags &= ~AI_STRAIGHT_TO_ENEMY;

		// See if there is another valid target to go after.
		if (check_old_enemy && M_ValidOldEnemy(self))
			return true;

		if (!FindTarget(self))
		{
			if (self->enemy != NULL)
			{
				self->oldenemy = self->enemy;
				self->enemy = NULL;
			}

			return false;
		}
	}

	if (COOP && self->monsterinfo.awake && self->enemy != NULL && self->monsterinfo.coop_check_debounce_time < level.time)
	{
		float c_dist[MAX_CLIENTS]; //mxd. int[] -> float[].

		// Only do this check once a second per monster.
		self->monsterinfo.coop_check_debounce_time = level.time + 1.0f;

		for (int i = 0; i < game.maxclients; i++) //mxd. 'i <= game.maxclients' in original logic.
		{
			const edict_t* client = &g_edicts[i + 1]; //mxd. '&g_edicts[i]' in original logic.

			if (client->client != NULL && client->health > 0)
				c_dist[i] = M_DistanceToTarget(self, client);
			else
				c_dist[i] = FLT_MAX; //mxd. Original logic uses 9999999999 here.
		}

		edict_t* new_enemy = NULL;
		float enemy_dist = M_DistanceToTarget(self, self->enemy);

		for (int i = 0; i < game.maxclients; i++) //mxd. 'i <= game.maxclients' in original logic.
		{
			if (c_dist[i] < enemy_dist)
			{
				edict_t* client = &g_edicts[i + 1]; //mxd. '&g_edicts[i]' in original logic.

				if (AI_IsVisible(self, client))
				{
					new_enemy = client;
					enemy_dist = c_dist[i];
				}
			}
		}

		if (new_enemy != NULL)
		{
			if (self->enemy->client != NULL && self->enemy->health > 0)
				self->oldenemy = self->enemy;

			self->enemy = new_enemy;
			AI_FoundTarget(self, false);
			self->monsterinfo.searchType = SEARCH_COMMON;
		}
	}

	return true;
}

// Predicts where the target will be a few frames later based on current velocity and facing, and predicts where
// the attacker will be at that same time. It then decides whether or not it will be able to melee from there.
// This is necessary for melee striking creatures who tend to run up to the player, swing, then stand for a few
// frames while the player backs up.
// NOTE: Does not detect whether or not a target and attacker will collide during the course of movement, but ai_run will find this for us.
// Returns:	0 - target will be out of range at end of movements (suggest: run after).
//			1 - target will be within range at the end of the movements at current velocities (suggest: continue motion).
// Args:
// attacker		- The entity pursuing the target.
// target		- What's being pursued.
// pursue_vel	- Attacker's desired movement velocity (passed as parameter so an average velocity for frames can be used).
// evade_vel	- Target's estimated evade velocity (again, passed as parameter in case you have special knowledge of a movement).
// strike_dist	- Maximum distance a melee attack can occur at, this is the range checked at the end of prediction.
// pred_frames	- Number of frames (1/10th second) to predict over (prediction accuracy decreases over large amounts of time).
int M_PredictTargetEvasion(const edict_t* attacker, const edict_t* target, const vec3_t pursue_vel, const vec3_t evade_vel, const float strike_dist, const float pred_frames) //TODO: change return type to qboolean?
{
	// Setup the movement directions.
	vec3_t attack_dir;
	VectorCopy(pursue_vel, attack_dir);

	vec3_t target_dir;
	VectorCopy(evade_vel, target_dir);

	// Setup the distances of attack.
	float attack_dist = VectorNormalize(attack_dir);
	float target_dist = VectorNormalize(target_dir);

	// Obtain movement per frame, then apply it over the number of predicted frames.
	attack_dist = pred_frames * (attack_dist * FRAMETIME);
	target_dist = pred_frames * (target_dist * FRAMETIME);

	vec3_t pred_attack_pos;
	VectorMA(attacker->s.origin, attack_dist, attack_dir, pred_attack_pos);

	vec3_t pred_target_pos;
	VectorMA(target->s.origin, target_dist, target_dir, pred_target_pos);

	// Find the distance between them.
	vec3_t diff;
	VectorSubtract(pred_attack_pos, pred_target_pos, diff);

	// If dist is too far, we won't hit.
	return VectorLength(diff) <= strike_dist;
}

// Predicts where the target will be a few frames later based on current velocity and facing.
// NOTE: Does not detect whether or not a target and attacker will collide during the course of movement, but ai_run will find this for us.
// Returns: position the target may be at in the predicted period.
// Args:
// target			- What's being pursued.
// evade_vel		- Target's estimated evade velocity (again, passed as parameter in case you have special knowledge of a movement).
// pred_frames		- Number of frames (1/10th second) to predict over (prediction accuracy decreases over large amounts of time).
// pred_target_pos	- [out arg] Where the enemy will be.
void M_PredictTargetPosition(const edict_t* target, const vec3_t evade_vel, const float pred_frames, vec3_t pred_target_pos)
{
	// Setup the movement directions.
	vec3_t target_dir;
	VectorCopy(evade_vel, target_dir);

	// Setup the distances of attack.
	float target_dist = VectorNormalize(target_dir);

	// Obtain movement per frame, then apply it over the number of predicted frames.
	target_dist = pred_frames * (target_dist * FRAMETIME);

	VectorMA(target->s.origin, target_dist, target_dir, pred_target_pos);
}

// Sets various states and sets up the monster to play his death frames.
void M_StartDeath(edict_t* self, int animID) //TODO: remove unused arg.
{
	self->msgHandler = DeadMsgHandler;

	// Dead but still being hit.
	if (self->dead_state == DEAD_DEAD)
		return;

	self->dead_state = DEAD_DEAD;

	// Gib death?
	if (self->health <= -80)
	{
		self->think = BecomeDebris; // The monster must check and play its own sound if a gib occured.
		self->nextthink = level.time + FRAMETIME;
	}
}

// The monster is dead completely. Set all information to reflect this.
void M_EndDeath(edict_t* self)
{
	self->mood_nextthink = -1.0f; // Never mood_think again.
	self->maxs[2] = self->mins[2] + 16.0f;

	if (self->s.effects & EF_ON_FIRE)
	{
		self->think = M_EndDeath;
		self->nextthink = level.time + 1.0f;
		self->s.effects &= ~EF_ON_FIRE;
	}
	else
	{
		self->think = NULL;
		self->nextthink = THINK_NEVER; // Stop thinking.
		gi.RemoveEffects(&self->s, FX_REMOVE_EFFECTS);
	}

	self->s.effects |= EF_DISABLE_EXTRA_FX;
	gi.linkentity(self);
}

// Look for monsters of a similar race around the current position of this monster.
// Returns: number of monsters around the current monster.
// Args:
// range - The radius to check inside.
int M_FindSupport(const edict_t* self, const int range)
{
	int num_support = 0;

	edict_t* ent = NULL;
	while ((ent = FindInRadius(ent, self->s.origin, (float)range)) != NULL)
		if (ent != self && ent->classID == self->classID && ent->health > 0)
			num_support++;

	return num_support;
}

// Look for monsters of a similar race and if they are already trying to alert others.
// Returns: whether or not to alert other monsters.
// Args:
// range - The radius to check inside.
qboolean M_CheckAlert(const edict_t* self, const int range)
{
	edict_t* ent = NULL;
	while ((ent = FindInRadius(ent, self->s.origin, (float)range)) != NULL)
	{
		if (ent == self || ent->classID != self->classID || ent->enemy != self->enemy || ent->health <= 0)
			continue;

		if (ent->monsterinfo.sound_finished < level.time || ent->monsterinfo.sound_pending > 0)
			continue;

		return false;
	}

	return true;
}

void M_jump(edict_t* self, G_Message_t* msg) //TODO: used only by Rat. Move to m_rat.c, rename to rat_jump?
{
	if (self->goalentity == NULL || (self->spawnflags & MSF_FIXED) || M_DistanceToTarget(self, self->goalentity) > 256.0f)
		return;

	self->jump_time = level.time + 0.5f;

	vec3_t forward;
	AngleVectors(self->s.angles, forward, NULL, NULL);

	vec3_t jump_vec;
	VectorScale(forward, 256.0f, jump_vec);
	jump_vec[2] += 101.0f;

	if (classStatics[self->classID].msgReceivers[MSG_CHECK_MOOD] != NULL)
	{
		VectorCopy(jump_vec, self->movedir);
		self->ai_mood = AI_MOOD_JUMP; // Don't technically need this line.
		self->mood_nextthink = level.time + 0.5f;

		// As an alternative, call self->forced_jump(self);
		G_PostMessage(self, MSG_CHECK_MOOD, PRI_DIRECTIVE, "i", AI_MOOD_JUMP);
	}
	else
	{
		VectorCopy(jump_vec, self->velocity);
	}
}

// Generic monster reaction to being alerted.
qboolean GenericMonsterAlerted(edict_t* self, alertent_t* alerter, edict_t* enemy) // I don't understand why I get a warning here...
{
	// Not already alerted?
	if (self->alert_time < level.time && SKILL < SKILL_VERYHARD && !(alerter->alert_svflags & SVF_ALERT_NO_SHADE) && !(self->monsterinfo.aiflags & AI_NIGHTVISION))
	{
		if (enemy->light_level < irand(6, 77)) //mxd. flrand in original logic.
			return false;
	}

	if (alerter->lifetime < level.time + 2.0f)
		self->alert_time = level.time + 2.0f; // Be ready for 2 seconds to wake up if alerted again.
	else
		self->alert_time = alerter->lifetime; // Bbe alert as long as the alert sticks around.

	if (enemy->svflags & SVF_MONSTER)
		self->enemy = alerter->enemy;
	else
		self->enemy = enemy;

	AI_FoundTarget(self, true);
	return true;
}

void MG_SetNormalizedVelToGoal(edict_t* self, vec3_t vec) //mxd. Named 'MG_SetNormalizeVelToGoal' in original logic.
{
	const qboolean charge_enemy = ((self->monsterinfo.aiflags & AI_STRAIGHT_TO_ENEMY) && self->enemy != NULL);

	if (self->monsterinfo.searchType == SEARCH_BUOY && !charge_enemy)
	{
		if (self->buoy_index < 0 || self->buoy_index >= level.active_buoys) //mxd. 'self->buoy_index > level.active_buoys' in original logic.
		{
			VectorClear(vec);
			return;
		}

		VectorCopy(level.buoy_list[self->buoy_index].origin, self->monsterinfo.nav_goal);
		VectorSubtract(self->monsterinfo.nav_goal, self->s.origin, vec);
	}
	else if (self->goalentity != NULL && !charge_enemy)
	{
		vec3_t target_pos;

		if (self->goalentity == self->enemy && (self->ai_mood_flags & AI_MOOD_FLAG_PREDICT))
			M_PredictTargetPosition(self->enemy, self->enemy->velocity, 8.0f, target_pos); // Predict where he's going.
		else
			VectorCopy(self->goalentity->s.origin, target_pos);

		VectorSubtract(target_pos, self->s.origin, vec);
	}
	else if (self->enemy != NULL)
	{
		vec3_t target_pos;

		if (self->ai_mood_flags & AI_MOOD_FLAG_PREDICT)
			M_PredictTargetPosition(self->enemy, self->enemy->velocity, 8.0f, target_pos); // Predict where he's going.
		else
			VectorCopy(self->enemy->s.origin, target_pos);

		VectorSubtract(target_pos, self->s.origin, vec);
	}
	else
	{
		VectorClear(vec);
		return;
	}

	VectorNormalize(vec);
}

// Overrides the lung meter and displays the creature's life meter to all clients.
void M_ShowLifeMeter(const int value, const int max_value) //mxd. Removed unused 'self' arg.
{
#define LIFEBAR_SCALE 16

	// Update all clients.
	for (int i = 0; i < MAXCLIENTS; i++)
	{
		player_state_t* ps = &game.clients[i].ps;

		ps->stats[STAT_LIFEBAR_XSIZE] = (short)(max_value / LIFEBAR_SCALE);
		ps->stats[STAT_LIFEBAR_YSIZE] = 16;
		ps->stats[STAT_LIFEBAR_ICON] = (short)gi.imageindex("icons/powerup.m8");
		ps->stats[STAT_LIFEBAR_BACK] = (short)gi.imageindex("icons/lifebar_back.m8");

		if (max_value > 0)
			ps->stats[STAT_LIFEBAR_VALUE] = (short)(value * 100 / max_value);
		else
			ps->stats[STAT_LIFEBAR_VALUE] = 0;
	}
}

void M_DeadBobThink(edict_t* self) //mxd. Named 'fish_deadbob' (in m_fish.c) in original logic.
{
	if (self->velocity[2] > 0.0f)
	{
		if (self->s.origin[2] > self->fish_water_surface_z + flrand(3.0f, 6.0f)) // So it doesn't always go to the same height.
			self->velocity[2] = flrand(-7.0f, -2.0f);
	}
	else
	{
		if (self->s.origin[2] < self->fish_water_surface_z)
			self->velocity[2] = flrand(2.0f, 7.0f);
	}

	self->nextthink = level.time + 0.2f;
}

// Make the monster corpse float to the surface. //TODO: used only by Fish and Plague Ssithra. Use by other monsters, which can potentially be killed in water?
void M_DeadFloatThink(edict_t* self) //mxd. Named 'fish_deadfloat' (in m_fish.c) in original logic.
{
	M_CatagorizePosition(self);

	//TODO: scale velocities by monster mass?
	if (self->waterlevel == 3) // Below water surface.
	{
		if (self->velocity[2] < 10.0f)
			self->velocity[2] += 10.0f;
		else
			self->velocity[2] = 20.0f; // Just in case something blocked it going up.
	}
	else if (self->waterlevel < 2) // Above water surface.
	{
		if (self->velocity[2] > -150.0f)
			self->velocity[2] -= 50.0f; // Fall back in now!
		else
			self->velocity[2] = -200.0f;
	}
	else // On water surface (waterlevel == 2).
	{
		self->fish_water_surface_z = self->s.origin[2];
		self->think = M_DeadBobThink;
	}

	self->nextthink = level.time + FRAMETIME; //mxd. Use define.
}

#pragma endregion