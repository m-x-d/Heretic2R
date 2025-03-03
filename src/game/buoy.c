//
// buoy.c -- BUOYAH! Navigation System. Mike Gummelt & Josh Weier
//
// Copyright 1998 Raven Software
//

#include "buoy.h"
#include "m_stats.h"
#include "Random.h"
#include "Vector.h"
#include "g_local.h"

#define SF_ONEWAY	8	// Don't back-link.
#define SF_BROKEN	64	// A bad bad buoy.
#define SF_DONT_TRY	128	// Don't allow monster to head here.

#define MAX_BUOY_BRANCH_CHECKS		1024	// Only check 1024 branches before giving up.
#define MAX_PROGRESSIVE_CHECK_DEPTH	20		// Only search 20 buoys deep if doing a progressive depth check.

static int check_depth;
static int buoy_depth;
static int branch_counter; // Short circuit check if 1024.

// Returns the buoy's id.
static int InsertBuoy(const edict_t* self) //mxd. Named 'insert_buoy' in original version.
{
	buoy_t* buoy = &level.buoy_list[level.active_buoys];

	// Init these values to -1 so we know what's filled.
	for (int i = 0; i < MAX_BUOY_BRANCHES; i++)
		buoy->nextbuoy[i] = NULL_BUOY;

	buoy->jump_target_id = NULL_BUOY;

	// Copy all the other important info.
	buoy->opflags = self->ai_mood_flags;
	buoy->modflags = self->spawnflags;

	if (buoy->modflags & BUOY_JUMP)
	{
		buoy->jump_target = self->jumptarget;
		buoy->jump_fspeed = self->speed;
		buoy->jump_yaw = self->s.angles[YAW];
		buoy->jump_uspeed = self->movedir[2];
	}
	else
	{
		buoy->jump_fspeed = 0.0f;
		buoy->jump_yaw = 0.0f;
		buoy->jump_uspeed = 0.0f;
	}

	VectorCopy(self->s.origin, buoy->origin);

	// Get the contents so that we know if this is a water buoy or not.
	buoy->contents = gi.pointcontents(buoy->origin);

	// Are we going to be opening anything?
	buoy->pathtarget = self->pathtarget;

	// Save the connections for easier debugging.
	buoy->target = self->target;
	buoy->jump_target = self->jumptarget;
	buoy->targetname = self->targetname;

	buoy->wait = self->wait;
	buoy->delay = self->delay;

	// Post incremented on purpose, thank you.
	buoy->id = level.active_buoys++;

	return buoy->id;
}

static void AssignNextBuoy(edict_t* self, const edict_t* ent) //mxd. Named 'assign_nextbuoy' in original version.
{
	buoy_t* buoy = &level.buoy_list[self->count];

	for (int i = 0; i < MAX_BUOY_BRANCHES; i++)
	{
		if (buoy->nextbuoy[i] != NULL_BUOY)
		{
			if (i == MAX_BUOY_BRANCHES - 1)
			{
				gi.dprintf("Buoy %d: too many connections on buoy %s (%s)\n", self->count, self->targetname, vtos(self->s.origin));
				self->ai_mood_flags |= SF_BROKEN;

				return;
			}

			continue;
		}

		buoy->nextbuoy[i] = ent->count;
		return;
	}
}

// self is supposed to make monsters jump at ent.
static void AssignJumpBuoy(edict_t* self, const edict_t* ent) //mxd. Named 'assign_jumpbuoy' in original version.
{
	buoy_t* buoy = &level.buoy_list[self->count];

	if (buoy->jump_target_id != NULL_BUOY)
	{
		gi.dprintf("Buoy %s (%s) already has a jump_target (%s), tried to assign another %s!\n", buoy->targetname, vtos(buoy->origin), buoy->jump_target, ent->targetname);
		self->ai_mood_flags |= SF_BROKEN;
	}
	else
	{
		buoy->jump_target_id = ent->count;
	}
}

// Link the buoys after all entities have been spawned.
static void LinkBuoyInfo(edict_t* self) //mxd. Named 'info_buoy_link' in original version.
{
	if ((self->spawnflags & BUOY_ACTIVATE) && self->pathtarget == NULL)
	{
		gi.dprintf("Buoy %s at %s is an ACTIVATE buoy but has no pathtarget!\n", self->targetname, vtos(self->s.origin));
		self->ai_mood_flags |= SF_BROKEN; //TODO: set only when BUOY_DEBUG is enabled in original version. Is this still required without the cvar?
		self->spawnflags &= ~BUOY_ACTIVATE;
	}

	// Make sure we have a target to link to.
	if (self->target != NULL)
	{
		edict_t* ent = NULL;
		ent = G_Find(ent, FOFS(targetname), self->target);

		if (ent == NULL)
		{
			gi.dprintf("info_buoy_link: %s(%s) failed to find target buoy %s\n", self->targetname, vtos(self->s.origin), self->target);
			self->ai_mood_flags |= SF_BROKEN; //TODO: set only when BUOY_DEBUG is enabled in original version. Is this still required without the cvar?
		}
		else if (ent == self)
		{
			gi.dprintf("info_buoy_link: %s(%s) target(%s) is self!\n", self->targetname, vtos(self->s.origin), self->target);
			self->ai_mood_flags |= SF_BROKEN;
		}
		else
		{
			// Link this buoy to it's target.
			AssignNextBuoy(self, ent);

			// If it's not one way, then back link it as well.
			if (!(self->spawnflags & SF_ONEWAY))
				AssignNextBuoy(ent, self);
		}
	}

	// Also check for a secondary target.
	if (self->target2 != NULL)
	{
		if (self->target != NULL && Q_stricmp(self->target2, self->target) == 0) //mxd. stricmp -> Q_stricmp.
		{
			gi.dprintf("info_buoy_link2: %s(%s) has target2 same as target %s\n", self->targetname, vtos(self->s.origin), self->target2);
			self->ai_mood_flags |= SF_BROKEN; //TODO: set only when BUOY_DEBUG is enabled in original version. Is this still required without the cvar?
		}
		else
		{
			edict_t* ent = NULL;
			ent = G_Find(ent, FOFS(targetname), self->target2);

			if (ent == NULL)
			{
				gi.dprintf("info_buoy_link2: %s(%s) failed to find target2 buoy %s\n", self->targetname, vtos(self->s.origin), self->target2);
				self->ai_mood_flags |= SF_BROKEN; //TODO: set only when BUOY_DEBUG is enabled in original version. Is this still required without the cvar?
			}
			else if (ent == self)
			{
				gi.dprintf("info_buoy_link2: %s(%s) target2(%s) is self!!!\n", self->targetname, vtos(self->s.origin), self->target2);
				self->ai_mood_flags |= SF_BROKEN;
			}
			else
			{
				// Link this buoy to it's target.
				AssignNextBuoy(self, ent);

				// If it's not one way, then back link it as well.
				if (!(self->spawnflags & SF_ONEWAY))
					AssignNextBuoy(ent, self);
			}
		}
	}

	if (self->spawnflags & BUOY_JUMP)
	{
		if (self->jumptarget == NULL)
		{
			gi.dprintf("Buoy %s at %s is a JUMP buoy but has no jumptarget!\n", self->targetname, vtos(self->s.origin));
			self->ai_mood_flags |= SF_BROKEN; //TODO: set only when BUOY_DEBUG is enabled in original version. Is this still required without the cvar?
			self->spawnflags &= ~BUOY_JUMP;
		}
		else
		{
			edict_t* ent = NULL;
			ent = G_Find(ent, FOFS(targetname), self->jumptarget);

			if (ent != NULL)
			{
				AssignJumpBuoy(self, ent);
			}
			else
			{
				gi.dprintf("Buoy %s(%s) could not find jumptarget buoy %s\n", self->targetname, vtos(self->s.origin), self->jumptarget);
				self->ai_mood_flags |= SF_BROKEN; //TODO: set only when BUOY_DEBUG is enabled in original version. Is this still required without the cvar?
			}
		}
	}

	// Put an effect on broken buoys?
	if (self->ai_mood_flags & SF_BROKEN)
	{
		level.buoy_list[self->count].opflags |= SF_BROKEN;
		level.fucked_buoys++;
	}

	if (self->count == level.active_buoys - 1)
	{
		// See if any buoys are loners.
		for (int i = 0; i < level.active_buoys; i++)
			if (level.buoy_list[i].nextbuoy[0] == 0 && level.buoy_list[i].nextbuoy[1] == 0 && level.buoy_list[i].nextbuoy[2] == 0)
				gi.dprintf("G.D.E. WARNING: buoy %s(%s) has no connections\n", level.buoy_list[i].targetname, vtos(level.buoy_list[i].origin));

		Com_Printf("%d buoys processed by BUOYAH! Navigation System(tm) (%d bad : %d fixed)\n", level.active_buoys, level.fucked_buoys, level.fixed_buoys);
	}

	G_SetToFree(self);
}

/*QUAKED info_buoy(0.6 0 0.8) (-24 -24 -24) (24 24 24) JUMP ACTIVATE x ONEWAY
BUOYAH! Navigation System Usage Manual:
  0) You shall not give a buoy more than one targetname.
  1) You shall have a buoy target up to two OTHER buoys, therefore:
  2) You shall connect each buoy to up to three other buoys (only three lines can come off a buoy).
  3) Direction of connection does not matter, unless you are trying to make a one-way buoy (see ONEWAY below).
  4) You shall place your buoy in an area that a monster can fit into.
  5) You shall place your buoys such that each buoy can "see" each buoy it's connected to (have a clear line of sight).
  6) Buoys do not need to be placed throughout wide open areas, monsters can get around fine there.
  7) You shall not place buoys in the ground or walls or any other world object.
  8) You shall not give any two buoys the same targetname, and each buoy should have a targetname, even if it is not targeted (this is for debug purposes).
  9) You shall not have any other AI system above the BUOYAH! Navigation System.

Keep in mind that when choosing a buoy, monsters need to be able to find a buoy withing 1024 map units of them.
So make sure buoys are placed so that wherever they can get, they are within 1024 of a buoy.

"cheating_monsters" - At the console, set this to 1 to allow monsters to teleport to a buoy it's having a hard time getting to.

Lots of info and useful instructions here:
JUMP - Will make monster jump ("angle": the direction to go in (default = 0), "speed": the forward velocity in this dir (default = 400),
"height": the height of the jump (default = 400)).
ACTIVATE - Will allow monster to activate doors, triggers, plats, etc. NOTE: the activated object's "pathtargetname" must match the buoy's "pathtarget" field.
ONEWAY - This buoy will not allow buoys it's targeting to send monsters backwards along the path.
Basically, does not back-link, paths from it to buoys it's targeting become one-way.

"jumptarget" - used with JUMP - this buoy will only make monsters jump at the buoy whose "targetname" is the same as "jumpbuoy".
Without this, the buoy WILL NOT MAKE MONSTERS JUMP!
"wait" - used with ACTIVATE. Will make the buoy wait this many seconds before allowing a monster to activate the targeted ent again.
"delay" - used with ACTIVATE. Will make the monster stand and wait this long after activating the target ent
(so it stands and waits on a lift or for the door to open).
*/
void SP_info_buoy(edict_t* self)
{
	static const vec3_t mins = { -24.0f, -24.0f, 0.0f }; //mxd. Made local static.
	static const vec3_t maxs = {  24.0f,  24.0f, 1.0f }; //mxd. Made local static.

	// 1-st buoy, initialize a couple arrays.
	if (level.active_buoys == 0)
	{
		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			level.player_buoy[i] = NULL_BUOY; // Stores current bestbuoy for a player enemy (if any).
			level.player_last_buoy[i] = NULL_BUOY; // When player_buoy is invalid, saves it here so monsters can check it first instead of having to do a whole search.
		}
	}

	if (self->spawnflags & BUOY_JUMP)
	{
		if (self->speed == 0.0f)
			self->speed = 400.0f;

		if (st.height == 0)
			st.height = 400;

		if (self->s.angles[YAW] == 0.0f)
			self->s.angles[YAW] = 360.0f;

		self->movedir[2] = (float)st.height;
	}

	if (self->targetname == NULL)
		gi.dprintf("Buoy with no targetname at %s!\n", vtos(self->s.origin));

	// Make sure it's not in the ground at all.
	if (gi.pointcontents(self->s.origin) & CONTENTS_SOLID)
	{
		gi.dprintf("Buoy %s(%s) in ground!!!\n", self->targetname, vtos(self->s.origin));
		self->ai_mood_flags |= SF_BROKEN;
	}
	else
	{
		// Check down against world. Does not check against entities! Does not check up against ceiling (why would they put one close to a ceiling???).
		vec3_t top;
		VectorCopy(self->s.origin, top);
		top[2] += 23.0f; //BUGFIX: mxd. Increments 'bottom' in original version.

		vec3_t bottom;
		VectorCopy(self->s.origin, bottom);
		bottom[2] -= 24.0f;

		trace_t trace;
		gi.trace(top, mins, maxs, bottom, self, MASK_SOLID, &trace);

		if (trace.allsolid || trace.startsolid) // Bouy in solid, can't be fixed.
		{
			gi.dprintf("Buoy %s(%s) in solid(%s)!\n", self->targetname, vtos(self->s.origin), trace.ent->classname);
			self->ai_mood_flags |= SF_BROKEN;
		}
		else if (trace.fraction < 1.0f)
		{
			// Buoy is in the ground.
			VectorCopy(trace.endpos, bottom);
			bottom[2] += 24.0f;

			gi.dprintf("Buoy %s was in ground(%s), moved it from %s to %s...\n", self->targetname, trace.ent->classname, vtos(self->s.origin), vtos(bottom));
			VectorCopy(bottom, self->s.origin);

			self->ai_mood_flags |= SF_BROKEN;
			level.fixed_buoys++;
		}
	}

	self->movetype = PHYSICSTYPE_NONE;
	self->solid = SOLID_NOT;
	self->clipmask = 0;
	self->classname = "info_buoy";
	self->think = LinkBuoyInfo;
	self->nextthink = level.time + FRAMETIME;
	self->count = InsertBuoy(self);

	if (self->count == NULL_BUOY)
		gi.dprintf("ERROR! SP_info_buoy: failed to insert buoy into map list!\n");

	gi.linkentity(self);
}

qboolean check_buoy_path(edict_t *self, int lb_id, int sb_id, int fb_id)
{
	buoy_t		*check_buoy = NULL;
	buoy_t		*last_buoy, *start_buoy, *final_buoy;
	int			i, branch;
	qboolean	branch_checked[MAX_BUOY_BRANCHES];
	int			num_branches_checked = 0;
	int			infinite_loop_short_circuit = 0;

	if(branch_counter++ >= MAX_BUOY_BRANCH_CHECKS)
	{
#ifdef _DEVEL
		if(BUOY_DEBUG)
			gi.dprintf("Passed MAX_BUOY_DEPTH %d!!!\n", branch_counter);
		else
#endif
			return false;//going too deep into buoys
	}

	if(check_depth < buoy_depth+1)
	{
#ifdef _DEVEL
		if(BUOY_DEBUG_LITE||BUOY_DEBUG)
			gi.dprintf("Hit max buoy check_depth (%d/%d) & failed\n", check_depth, buoy_depth);
#endif
		return false;//going too deep into buoys
	}

	for(i = 0; i < MAX_BUOY_BRANCHES; i++)
		branch_checked[i] = false;

	last_buoy = &level.buoy_list[lb_id];
	start_buoy = &level.buoy_list[sb_id];
	final_buoy = &level.buoy_list[fb_id];

	start_buoy->opflags |= SF_DONT_TRY;

	buoy_depth++;//add a level to buoy search depth
	
	for (i = 0; num_branches_checked < MAX_BUOY_BRANCHES; i++)
	{
		do
		{
			if(infinite_loop_short_circuit++ > 1000)
				assert(0);

			branch = irand(0, MAX_BUOY_BRANCHES - 1);
		} 
		while(branch_checked[branch] == true);

		branch_checked[branch] = true;
		num_branches_checked++;

		if (start_buoy->nextbuoy[branch] == NULL_BUOY)
		{
#ifdef _DEVEL
			if (BUOY_DEBUG>2)
				gi.dprintf("No #%d Branch off of %s\n", branch + 1, start_buoy->targetname);
#endif
			if(num_branches_checked == MAX_BUOY_BRANCHES)
			{
				start_buoy->opflags &= ~SF_DONT_TRY;
				buoy_depth--;//take last level off
				return false;
			}
			else
				continue;//check others
		}

		check_buoy = &level.buoy_list[start_buoy->nextbuoy[branch]];

#ifdef _DEVEL
		if(BUOY_DEBUG>2)
			gi.dprintf("Checking buoy %s off of %s\n", check_buoy->targetname, start_buoy->targetname);
#endif

		if (check_buoy == final_buoy)
		{
#ifdef _DEVEL
			if (BUOY_DEBUG)
				gi.dprintf("buoy found...\n");
#endif			
			start_buoy->opflags &= ~SF_DONT_TRY;
			return true;
		}

		if ((check_buoy->opflags&SF_DONT_TRY))
//	Gil suggestion: unimplemented
// || check_buoy->failed_depth <= buoy_depth)
		{
#ifdef _DEVEL
			if(BUOY_DEBUG>2)
				gi.dprintf("Buoy %s marked as don't try, skipping\n", check_buoy->targetname);
#endif
			continue;
		}

		if (check_buoy == last_buoy)
		{
			continue;
		}

		if(check_buoy_path(self, start_buoy->id, check_buoy->id, final_buoy->id))
		{
			start_buoy->opflags &= ~SF_DONT_TRY;
			return true;
		}

//	Gil suggestion: unimplemented
/*
		//this buoy cannot reach goal at this depth, so don't try it again this search unless come across it at a lower depth
		if(BUOY_DEBUG>2)
			gi.dprintf("Buoy %s marked as failed at depth %d, will skip for rest of checks of lower depth\n", check_buoy->targetname, buoy_depth);
		//NOTE: EXPERIMENTAL - remove if can't prove it works!
		check_buoy->failed_depth = buoy_depth;
*/
  }

	start_buoy->opflags &= ~SF_DONT_TRY;
	buoy_depth--;//take last level off
	return false;
}

buoy_t	*find_next_buoy_2(edict_t *self, int sb_id, int fb_id)
{
	buoy_t		*check_buoy = NULL, *save_buoy = NULL, *start_buoy, *final_buoy;
	int			i, branch;
	qboolean	branch_checked[MAX_BUOY_BRANCHES];
	int			num_branches_checked = 0;
	int			infinite_loop_short_circuit = 0;

	for(i = 0; i < MAX_BUOY_BRANCHES; i++)
		branch_checked[i] = false;

	start_buoy = &level.buoy_list[sb_id];
	final_buoy = &level.buoy_list[fb_id];

	buoy_depth = 1;
	start_buoy->opflags |= SF_DONT_TRY;
	if(self->lastbuoy > NULL_BUOY)
	{//don't loop back around, the save_buoy last branch check will be a shorter path
#ifdef _DEVEL
		if(BUOY_DEBUG)
			gi.dprintf("Last buoy was %s...\n", level.buoy_list[self->lastbuoy].targetname);
#endif
		level.buoy_list[self->lastbuoy].opflags |= SF_DONT_TRY;
	}

//fixme: make my last_buoy also a dont_try?
	
	for (i = 0; num_branches_checked < MAX_BUOY_BRANCHES; i++)
	{
		do
		{
			if(infinite_loop_short_circuit++ > 1000)
				assert(0);

			branch = irand(0, MAX_BUOY_BRANCHES - 1);
		} 
		while(branch_checked[branch] == true);

		branch_checked[branch] = true;
		num_branches_checked++;

		if (start_buoy->nextbuoy[branch] == NULL_BUOY)
		{
			if(num_branches_checked == MAX_BUOY_BRANCHES)
			{
				start_buoy->opflags &= ~SF_DONT_TRY;
				if(self->lastbuoy > NULL_BUOY)
					level.buoy_list[self->lastbuoy].opflags &= ~SF_DONT_TRY;
				return NULL;//hasn't found one before here, and last branch was false and next is null, failed!
			}
			else
				continue;//check others
		}

		if(self->lastbuoy == start_buoy->nextbuoy[branch])
		{
#ifdef _DEVEL
			if (BUOY_DEBUG>2)
				gi.dprintf("Saving %s's last (previous) buoy %s for last path check\n", self->classname, level.buoy_list[self->lastbuoy].targetname);
#endif			
			save_buoy = &level.buoy_list[self->lastbuoy];
			continue;
		}
			
		check_buoy = &level.buoy_list[start_buoy->nextbuoy[branch]];

#ifdef _DEVEL
		if(BUOY_DEBUG>2)
			gi.dprintf("(Start) Checking buoy %s off of %s\n", check_buoy->targetname, start_buoy->targetname);
#endif
		if (check_buoy == final_buoy)
		{
#ifdef _DEVEL
			if(BUOY_DEBUG_LITE||BUOY_DEBUG)
				gi.dprintf("%s FOUND CONNECTION FROM %s TO %s IN 1 STEP, %d BRANCHES CHECKED\n", self->classname, start_buoy->targetname, final_buoy->targetname, branch_counter);
#endif			
			start_buoy->opflags &= ~SF_DONT_TRY;
			if(self->lastbuoy > NULL_BUOY)
				level.buoy_list[self->lastbuoy].opflags &= ~SF_DONT_TRY;
			return check_buoy;
		}

		if ((check_buoy->opflags & SF_DONT_TRY))
//	Gil suggestion: unimplemented
//	|| check_buoy->failed_depth <= buoy_depth)
		{
#ifdef _DEVEL
			if(BUOY_DEBUG>2)
				gi.dprintf("Buoy %s marked as don't try, skipping\n", check_buoy->targetname);
#endif
			continue;
		}

		if(check_buoy_path(self, start_buoy->id, check_buoy->id, final_buoy->id))
		{
#ifdef _DEVEL
			if(BUOY_DEBUG_LITE||BUOY_DEBUG)
				gi.dprintf("%s FOUND CONNECTION FROM %s TO %s IN %d STEPS, %d BRANCHES CHECKED\n", self->classname, start_buoy->targetname, final_buoy->targetname, buoy_depth, branch_counter);
#endif			
			start_buoy->opflags &= ~SF_DONT_TRY;
			if(self->lastbuoy > NULL_BUOY)
				level.buoy_list[self->lastbuoy].opflags &= ~SF_DONT_TRY;

			if(check_buoy->id == self->lastbuoy)
				assert(0);//should NEVER happen!!!

			return check_buoy;
		}
		else
			continue;
	}

	if(save_buoy)
	{
		save_buoy->opflags &= ~SF_DONT_TRY;
		check_buoy = save_buoy;

#ifdef _DEVEL
		if (BUOY_DEBUG>2)
			gi.dprintf("Now checking saved buoy %s for last path check\n", check_buoy->targetname);
#endif
		if (check_buoy == final_buoy)
		{
#ifdef _DEVEL
			if(BUOY_DEBUG_LITE||BUOY_DEBUG)
				gi.dprintf("%s FOUND CONNECTION FROM %s TO %s IN 1 STEP, %d BRANCHES CHECKED\n", self->classname, start_buoy->targetname, final_buoy->targetname, branch_counter);
#endif
			start_buoy->opflags &= ~SF_DONT_TRY;
			if(self->lastbuoy > NULL_BUOY)
				level.buoy_list[self->lastbuoy].opflags &= ~SF_DONT_TRY;

			return check_buoy;
		}

		if (!(check_buoy->opflags & SF_DONT_TRY))
//	Gil suggestion: unimplemented
//	&& check_buoy->failed_depth > buoy_depth)
		{
			if(check_buoy_path(self, start_buoy->id, check_buoy->id, final_buoy->id))
			{
#ifdef _DEVEL
				if(BUOY_DEBUG_LITE||BUOY_DEBUG)
					gi.dprintf("%s FOUND CONNECTION FROM %s TO %s IN %d STEPS, %d BRANCHES CHECKED\n", self->classname, start_buoy->targetname, final_buoy->targetname, buoy_depth, branch_counter);
#endif				
				start_buoy->opflags &= ~SF_DONT_TRY;
				if(self->lastbuoy > NULL_BUOY)
					level.buoy_list[self->lastbuoy].opflags &= ~SF_DONT_TRY;

				return check_buoy;
			}
		}
	}

	start_buoy->opflags &= ~SF_DONT_TRY;
	if(self->lastbuoy > NULL_BUOY)
		level.buoy_list[self->lastbuoy].opflags &= ~SF_DONT_TRY;

	return NULL;
}

buoy_t	*find_next_buoy(edict_t *self, int sb_id, int fb_id)
{
	buoy_t	*found = NULL, *start_buoy, *final_buoy;
	int		i;

	if(!self->mintel)
	{
#ifdef _DEVEL
		if(BUOY_DEBUG_LITE||BUOY_DEBUG)
			gi.dprintf("Can't use buoys- no mintel!...\n");
#endif	
		return NULL;
	}

	start_buoy = &level.buoy_list[sb_id];
	final_buoy = &level.buoy_list[fb_id];

#ifdef _DEVEL
	if(BUOY_DEBUG)
		gi.dprintf("********************************************************\n    %s Beginning search from %s to %s\n********************************************************\n", self->classname, start_buoy->targetname, final_buoy->targetname);
#endif
	branch_counter = 0;

	if(irand(0, 1))
	{//progressive_depth- finds shortest
#ifdef _DEVEL
		if(BUOY_DEBUG)
			gi.dprintf("%s starting progressive depth buoy path check\n", self->classname);
#endif
		check_depth = 0;

		while(check_depth < self->mintel && check_depth < MAX_PROGRESSIVE_CHECK_DEPTH)
		{//only search to max of 20 buoys deep if doing a progressive depth check
			check_depth++;
			found = find_next_buoy_2(self, start_buoy->id, final_buoy->id);
			if(found)
			{
				check_depth = 0;
				return found;
			}
		}
	}
	else
	{//start at max depth- finds first
#ifdef _DEVEL
		if(BUOY_DEBUG)
			gi.dprintf("%s starting max depth(%d) buoy path check\n", self->classname, self->mintel);
#endif
		check_depth = self->mintel;
		found = find_next_buoy_2(self, start_buoy->id, final_buoy->id);
		if(found)
		{
			check_depth = 0;
			return found;
		}
	}

	for(i = 0; i <= level.active_buoys; i++)
	{
		level.buoy_list[i].opflags &= ~SF_DONT_TRY;
//	Gil suggestion: unimplemented
//	level.buoy_list[i].failed_depth = 999999999;
	}

#ifdef _DEVEL
	if(BUOY_DEBUG_LITE||BUOY_DEBUG)
		if(check_depth == self->mintel)
			gi.dprintf("Hit my max buoy depth (%d) & failed\n", check_depth);
#endif	
#ifdef _DEVEL
	if(BUOY_DEBUG_LITE||BUOY_DEBUG)
		gi.dprintf("Path from buoy %s(%s) to buoy %s(%s) not possible at depth of %d!\n", start_buoy->targetname, vtos(start_buoy->origin), final_buoy->targetname, vtos(final_buoy->origin), check_depth);
#endif
	check_depth = 0;
	return NULL;
}
