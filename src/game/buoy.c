// ****************************************************************************
//	BUOYAH! Navigation System
//
//	Heretic II
//	Copyright 1998 Raven Software
//
//  Mike Gummelt & Josh Weier
// ****************************************************************************

#include "g_local.h"
#include "buoy.h"
#include "vector.h"
#include "random.h"
#include "fx.h"
#include "m_stats.h"

#define SF_JUMP		1//make monster jump
#define SF_ACTIVATE	2//turn something on
#define SF_TURN		4
#define SF_ONEWAY	8//don't back-link

#define SF_BROKEN	64//a bad bad buoy	
#define SF_DONT_TRY	128//don't allow monster to head here

#define BUOY_PRINT_INFO_DIST 64
#define MAX_BUOY_BRANCH_CHECKS 1024//only check 1024 branches before giving up
#define MAX_PROGRESSIVE_CHECK_DEPTH 20//only search 20 buoys deep if doing a progressive depth check

static int	check_depth;
static int	buoy_depth;
static int	branch_counter;//short circuit check if 1024

//Returns the buoy's id
int insert_buoy(edict_t *self)
{
	buoy_t	*buoy = &level.buoy_list[level.active_buoys];
	int	i;
	
	//Init these values to -1 so we know what's filled
	for (i=0;i<MAX_BUOY_BRANCHES;i++)
		buoy->nextbuoy[i] = NULL_BUOY;
	buoy->jump_target_id = NULL_BUOY;

	//Copy all the other important info
	buoy->opflags = self->ai_mood_flags;
	buoy->modflags = self->spawnflags;
	if(buoy->modflags & BUOY_JUMP)
	{
		buoy->jump_target = self->jumptarget;
		buoy->jump_fspeed = self->speed;
		buoy->jump_yaw = self->s.angles[YAW];
		buoy->jump_uspeed = self->movedir[2];
	}
	else
	{
		buoy->jump_fspeed = buoy->jump_yaw = buoy->jump_uspeed = 0;
	}
	VectorCopy(self->s.origin, buoy->origin);
	
	//Get the contents so that we know if this is a water buoy or not
	buoy->contents = gi.pointcontents(buoy->origin);
	//are we going to be opening anything?
	buoy->pathtarget = self->pathtarget;

	//save the connections for easier debugging
	buoy->target = self->target;
	buoy->jump_target = self->jumptarget;
	buoy->targetname = self->targetname;

	buoy->wait = self->wait;
	buoy->delay = self->delay;
	
//	Gil suggestion: unimplemented
//	buoy->failed_depth = 999999999;

	//Post incremented on purpose, thank you
	buoy->id = level.active_buoys++;
	//when done, level.active_buoys will be the total number of buoys,
	//level.active_buoys - 1 will be the last valid buoy

	return buoy->id;
}

void assign_nextbuoy(edict_t *self, edict_t *ent)
{
	buoy_t	*buoy = &level.buoy_list[self->count];
	int i;
	
	for (i = 0; i < MAX_BUOY_BRANCHES; i++)
	{
		if (buoy->nextbuoy[i] > NULL_BUOY)
		{
			if (i==2)
			{
				gi.dprintf("Buoy %d: Too many connections on buoy %s (%s)\n", self->count, self->targetname, vtos(self->s.origin));
				self->ai_mood_flags |= SF_BROKEN;
				return;
			}

			continue;
		}

		buoy->nextbuoy[i] = ent->count;
		return;
	}

	return;
}

void assign_jumpbuoy(edict_t *self, edict_t *ent)
{//self is supposed to make monsters jump at ent
	buoy_t	*buoy = &level.buoy_list[self->count];
	int i;
	
	for (i = 0; i < MAX_BUOY_BRANCHES; i++)
	{
		if (buoy->jump_target_id > NULL_BUOY)
		{
			gi.dprintf("Buoy %s (%s): already has a jump_target(%s), tried to assign another %s!\n", buoy->targetname, vtos(buoy->origin), buoy->jump_target, ent->targetname);
			self->ai_mood_flags |= SF_BROKEN;
			return;
			continue;
		}

		buoy->jump_target_id = ent->count;
		return;
	}

	return;
}

// ****************************************************************************
//	Link the buoys after all entities have been spawned
// ****************************************************************************
void info_buoy_link(edict_t *self)
{
	edict_t *ent = NULL;
	int		i;
	
	if(self->spawnflags & BUOY_ACTIVATE)
	{
		if(!self->pathtarget)
		{
			if (BUOY_DEBUG)
			{
				gi.dprintf("Buoy %s at %s is an ACTIVATE buoy but has no pathtarget!!!\n", self->targetname, vtos(self->s.origin));
				self->ai_mood_flags |= SF_BROKEN;
			}
			self->spawnflags &= ~BUOY_ACTIVATE;
		}
	}

	//Make sure we have a target to link to
	if (self->target)
	{
		ent	= NULL;
		ent = G_Find(ent, FOFS(targetname), self->target);

		if (ent == NULL)
		{
			if (BUOY_DEBUG)
			{
				gi.dprintf("info_buoy_link: %s(%s) failed to find target buoy %s\n", self->targetname, vtos(self->s.origin), self->target);
				self->ai_mood_flags |= SF_BROKEN;
			}
		}
		else if(ent == self)
		{
			gi.dprintf("info_buoy_link: %s(%s) target(%s) is self!!!\n", self->targetname, vtos(self->s.origin), self->target);
			self->ai_mood_flags |= SF_BROKEN;
		}
		else
		{
			if (BUOY_DEBUG)
				gi.dprintf("info_buoy_link: linked %s to %s\n", self->targetname, ent->targetname);

			//Link this buoy to it's target
			assign_nextbuoy(self, ent);

			if(BUOY_DEBUG > 1)
			{
				gi.CreatePersistantEffect(NULL,
						FX_M_EFFECTS,//green arrows
						CEF_BROADCAST,
						self->s.origin,
						"bv",
						FX_BUOY_PATH,
						ent->s.origin);
			}
			//If it's not one way, then back link it as well
			if(!(self->spawnflags&SF_ONEWAY))
				assign_nextbuoy(ent, self);
		}
	}

	//Also check for a secondary target
	if (self->target2)
	{
		if ( (self->target) && (!stricmp(self->target2, self->target)) )
		{
			if (BUOY_DEBUG)
			{
				gi.dprintf("info_buoy_link2: %s(%s) has target2 same as target %s\n", self->targetname, vtos(self->s.origin), self->target2);
				self->ai_mood_flags |= SF_BROKEN;
			}
		}
		else
		{
			ent	= NULL;
			ent = G_Find(ent, FOFS(targetname), self->target2);

			if (ent == NULL)
			{
				if (BUOY_DEBUG)
				{
					gi.dprintf("info_buoy_link2: %s(%s) failed to find target2 buoy %s\n", self->targetname, vtos(self->s.origin), self->target2);
					self->ai_mood_flags |= SF_BROKEN;
				}
			}
			else if(ent == self)
			{
				gi.dprintf("info_buoy_link2: %s(%s) target2(%s) is self!!!\n", self->targetname, vtos(self->s.origin), self->target2);
				self->ai_mood_flags |= SF_BROKEN;
			}
			else
			{
				//Link this buoy to it's target
				assign_nextbuoy(self, ent);
				
				if(BUOY_DEBUG > 1)
				{
					gi.CreatePersistantEffect(NULL,
							FX_M_EFFECTS,//green arrows
							CEF_BROADCAST,
							self->s.origin,
							"bv",
							FX_BUOY_PATH,
							ent->s.origin);
				}
				//If it's not one way, then back link it as well
				if(!(self->spawnflags&SF_ONEWAY))
					assign_nextbuoy(ent, self);
			}
		}
	}

	if(self->spawnflags&BUOY_JUMP)
	{
		if(!self->jumptarget)
		{
			if (BUOY_DEBUG)
			{
				gi.dprintf("Buoy %s at %s is a JUMP buoy but has no jumptarget!!!\n", self->targetname, vtos(self->s.origin));
				self->ai_mood_flags |= SF_BROKEN;
			}
			self->spawnflags &= ~BUOY_JUMP;
		}
		else
		{
			ent = NULL;
			if(ent = G_Find(ent, FOFS(targetname), self->jumptarget))
			{
				assign_jumpbuoy(self, ent);
				if(BUOY_DEBUG>1)
				{
					gi.CreatePersistantEffect(NULL,
						FX_M_EFFECTS,//blue particles
						CEF_FLAG8|CEF_BROADCAST,
						ent->s.origin,
						"bv",
						FX_BUOY,
						vec3_origin);
				}
			}
			else if(BUOY_DEBUG)
			{
				gi.dprintf("Buoy %s(%s) could not find jumptarget buoy %s\n", self->targetname, vtos(self->s.origin), self->jumptarget);
				self->ai_mood_flags |= SF_BROKEN;
			}
		}
	}

	if(self->ai_mood_flags & SF_BROKEN)
	{//put an effect on broken buoys?
		level.buoy_list[self->count].opflags |= SF_BROKEN;
		level.fucked_buoys++;
	}

	if(self->count == level.active_buoys - 1)
	{
		for(i = 0; i < level.active_buoys; i++)
		{//see if any buoys are loners
			if(!level.buoy_list[i].nextbuoy[0]&&
				!level.buoy_list[i].nextbuoy[1]&&
				!level.buoy_list[i].nextbuoy[2])
			{
				gi.dprintf("G.D.E. WARNING: buoy %s(%s) has no connections\n", level.buoy_list[i].targetname, vtos(level.buoy_list[i].origin));
			}
		}
		Com_Printf("%d buoys processed by BUOYAH! Navigation System(tm) (%d bad : %d fixed)\n", level.active_buoys, level.fucked_buoys, level.fixed_buoys);
	}

	if(!BUOY_DEBUG)
	{
		G_SetToFree(self);
	}
	else
	{//buoys don't need to think
		self->think = NULL;
		self->nextthink = -1;
	}
}

void PrintLocalBuoyInfo(vec3_t org)
{
	int		i, j;
	vec3_t	dir;
	float	dist;

	for(i = 0; i<level.active_buoys; i++)
	{
		if(level.buoy_list[i].print_debounce_time<level.time)
		{
			VectorSubtract(level.buoy_list[i].origin, org, dir);
			dist = VectorLength(dir);
			if(dist<BUOY_PRINT_INFO_DIST)
			{
				gi.dprintf("==============================\n");
				gi.dprintf("BUOY %s\n", level.buoy_list[i].targetname);
				gi.dprintf("at %s\n", vtos(level.buoy_list[i].origin));
				if(level.buoy_list[i].opflags & SF_BROKEN)
					gi.dprintf("BROKEN!!!\n");
				gi.dprintf("connections: ");
				for(j = 0; j < MAX_BUOY_BRANCHES; j++)
				{
					if(level.buoy_list[i].nextbuoy[j] > NULL_BUOY)
					{
						if(j!=0)
							gi.dprintf(", ");
						gi.dprintf("%s", level.buoy_list[level.buoy_list[i].nextbuoy[j]].targetname);
					}
				}
				gi.dprintf("\n");


				if(level.buoy_list[i].modflags & BUOY_JUMP)
				{
					gi.dprintf("\nJUMP\n");
					gi.dprintf("jump_target buoy: %s\n", level.buoy_list[i].jump_target);
					gi.dprintf("angle: %4.2f\n", level.buoy_list[i].jump_yaw);
					gi.dprintf("height: %4.2f\n", level.buoy_list[i].jump_uspeed);
					gi.dprintf("speed: %4.2f\n", level.buoy_list[i].jump_fspeed);
					level.buoy_list[i].print_debounce_time = level.time + 1;
				}

				if(level.buoy_list[i].modflags & BUOY_ACTIVATE)
				{
					edict_t *found;

					gi.dprintf("\nACTIVATE\n");
					gi.dprintf("pathtarget: %s\n", level.buoy_list[i].pathtarget);
					if(found = G_Find(NULL, FOFS(pathtargetname), level.buoy_list[i].pathtarget))
						gi.dprintf("entity to activate: %s\n", found->classname);
					else
						gi.dprintf("ERROR: no entity found to activate!!!\n");
					gi.dprintf("wait: %4.2f\n", level.buoy_list[i].wait);
					gi.dprintf("delay: %4.2f\n", level.buoy_list[i].delay);
					level.buoy_list[i].print_debounce_time = level.time + 1;
				}

				if(level.buoy_list[i].modflags & SF_ONEWAY)
				{
					gi.dprintf("\nONE WAY\n");
					level.buoy_list[i].print_debounce_time = level.time + 1;
				}
				gi.dprintf("==============================\n");
			}
		}
	}
}
/*QUAKED info_buoy(0.6 0 0.8) (-24 -24 -24) (24 24 24) JUMP ACTIVATE TURN ONEWAY
BUOYAH! Navigation System
Mike Gummelt & Josh Weier

THOU SHALT NOT COMMIT GRIEVOUS DESIGN ERRORS FOR THEY ARE AN ABOMINATION BEFORE THE EYES OF THE PROGRAMMER AND THE PROGRAMMER'S WORD IS INFALLIBLE!

THE BUOY TEN COMMANDMENTS as Handed Down to M0535 on Mount Sine-AI:
  0) Thou shalt not give a buoy more than one targetname
  1) Thou shalt have a buoy target up to two OTHER buoies
	therefore:
  2) Thou shalt connect each buoy to up to three other buoies (only three lines can come off a buoy)
  3) Thou shalt knowest that direction of connection does not matter unless you are trying to make a one-way buoy (see ONEWAY) below
  4) Thou shalt place thine buoy in an area that a monster can fit into
  5) Thou shalt place thine buoies such that each buoy can "see" each buoy it's conencted to (have a clear line of sight)
  6) Thou shalt knowest that buoies do not need to be placed throughout wide open areas, monsters can get around fine there.
  7) Thou shalt not place buoies in the ground or walls or any other world object
  8) Thou shalt not give any two buoies the same targetname, and each buoy should have a targetname, even if it is not targeted (this is for debug purposes)
  9) Thou shalt not have any other AI system above the BUOYAH! Navigation System.

Keep in mind that when choosing a buoy, monsters need to be able to find a buoy withing 1024 map units of them.  So make sure buoies are placed so that wherever they can get, they are within 1024 of a buoy.

"showbuoys" - At the console, setting "showbuoys" to 1 and restarting the map will allow you to see each buoy.  The flags you will see are each monster's indicator of where they are trying to go at the minute.  In addition you will get buoy debug messages on the console in this mode, telling you if a monster has a hard time getting to a buoy (it times out) or if, for some reason, a connection cannot be made between two buoies.
"cheating_monsters" - At the console, set this to 1 to allow monsters to teleport to a buoy it's having a hard time getting to.

Lots of info and useful instructions here:
JUMP - Will make monster jump ("angle" is the direction to go in (default = 0), "speed" if the forward velocity in this dir (default = 400), "height" is the height of the jump (default = 400))
ACTIVATE - Will allow monster to activate doors, triggers, plats, etc.  NOTE: the activated object's "pathtargetname" must match the buoy's "pathtarget" field.
(not implemented) TURN - Will make monster turn to buoy's angles
ONEWAY - This buoy will not allow buoys it's targeting to send monsters backwards along the path.  Basically, does not back-link, paths from it to buoys it's targeting become one-way.

"jumptarget" - used with JUMP - this buoy will only make monsters jump at the buoy whose "targetname" is the same as "jumpbuoy"- without this, the buoy WILL NOT MAKE MONSTERS JUMP!
"wait" - used with ACTIVATE- will make the buoy wait this many seconds before allowing a monster to activate the targeted ent again
"delay" - used with ACTIVATE - will make the monster stand and wait this long after activating the target ent (so it stands and waits on a lift or for the door to open)

NOTE: AVOID GRIEVOUS DESIGN ERRORS!

DEBUG INFO:
For "showbuoys" = 2, in addition to the arrow paths indicating target->targetname direction, particles appear on buoys, here's what each color indicates:
red - the buoy a monster has determined is your closest buoy
green - the buoy a monster has determined is it's next buoy to get to
cyan - this buoy has a jump flag
blue - this buoy is the jumptarget of a jump flagged buoy
magenta - this buoy has an activate flag
white - this buoy has an oneway flag

Standing over a buoy that has a spawnflag of BUOY_JUMP or BUOY_ACTIVATE and hitting "action" will print it's info to the console is in "showbuoys" mode
*/

void SP_info_buoy(edict_t *self)
{
	int i;

	if(!level.active_buoys)
	{//1st buoy, initialize a couple arrays
		for(i = 0; i < MAX_CLIENTS; i++)
		{
			level.player_buoy[i] = NULL_BUOY;				//stores current bestbuoy for a player enemy (if any)
			level.player_last_buoy[i] = NULL_BUOY;		//when player_buoy is invalid, saves it here so monsters can check it first instead of having to do a whole search
		}
	}

	if(self->spawnflags&BUOY_JUMP)
	{
		if (!self->speed)
			self->speed = 400;

		if (!st.height)
			st.height = 400;

		if (self->s.angles[YAW] == 0)
			self->s.angles[YAW] = 360;

		self->movedir[2] = st.height;

		if(BUOY_DEBUG>1)
		{
			gi.CreatePersistantEffect(NULL,
				FX_M_EFFECTS,//cyan particles
				CEF_FLAG7|CEF_BROADCAST,
				self->s.origin,
				"bv",
				FX_BUOY,
				vec3_origin);
		}
	}

	if(BUOY_DEBUG>1)
	{
		if(self->spawnflags&BUOY_ACTIVATE)
		{
			gi.CreatePersistantEffect(NULL,
				FX_M_EFFECTS,
				CEF_DONT_LINK|CEF_BROADCAST,//magenta particles
				self->s.origin,
				"bv",
				FX_BUOY,
				vec3_origin);
		}

		if(self->spawnflags&SF_ONEWAY)
		{
			vec3_t	extra_shit;

			VectorSet(extra_shit, 1, 0, 0);//really hacky way to send a diff effect
			gi.CreatePersistantEffect(NULL,
				FX_M_EFFECTS,
				CEF_BROADCAST,
				self->s.origin,
				"bv",
				FX_BUOY,
				extra_shit);//white particles
		}
	}

	if(!self->targetname)
	{
		if (BUOY_DEBUG)
			gi.dprintf("Buoy with no targetname at %s!!!\n", vtos(self->s.origin));
	}

	//make sure it's not in the ground at all
	if(gi.pointcontents(self->s.origin)&CONTENTS_SOLID)
	{
		gi.dprintf("Buoy %s(%s) in ground!!!\n", self->targetname, vtos(self->s.origin));
		self->ai_mood_flags |= SF_BROKEN;
	}
	else
	{//check down against world- does not check against entities! Does not check up against cieling (why would they put one close to a cieling???)
		vec3_t		top, bottom, mins, maxs;
		trace_t		trace;

		VectorCopy(self->s.origin, top);
		VectorCopy(self->s.origin, bottom);
		bottom[2] += 23;
		bottom[2] -= 24;

		VectorSet(mins, -24, -24, 0);
		VectorSet(maxs, 24, 24, 1);

		gi.trace(top, mins, maxs, bottom, self, MASK_SOLID,&trace);
		if(trace.allsolid || trace.startsolid)//bouy in solid, can't be fixed
		{
			gi.dprintf("Buoy %s(%s) in solid(%s)!!!\n", self->targetname, vtos(self->s.origin), trace.ent->classname);
			self->ai_mood_flags |= SF_BROKEN;
		}
		else if(trace.fraction<1.0)
		{//buoy is in the ground
			VectorCopy(trace.endpos, bottom);
			bottom[2] += 24;
			gi.dprintf("Buoy %s was in ground(%s), moved it from %s to %s...\n", self->targetname, trace.ent->classname, vtos(self->s.origin), vtos(bottom));
			VectorCopy(bottom, self->s.origin);
			self->ai_mood_flags |= SF_BROKEN;
			level.fixed_buoys++;
		}
	}

	self->movetype=PHYSICSTYPE_NONE;

	self->solid = SOLID_NOT;
	self->clipmask = 0;

	self->classname = "info_buoy";

	if (BUOY_DEBUG)
	{
		self->s.renderfx = RF_GLOW;
		self->s.angles[2] = 180;
		self->s.modelindex = gi.modelindex("models/objects/lights/bug/tris.fm");
	}
	
	self->think = info_buoy_link;
	self->nextthink = level.time + FRAMETIME;

	if ((self->count = insert_buoy(self)) == NULL_BUOY)
		gi.dprintf("ERROR! SP_info_buoy : Failed to insert buoy into map list!\n");
	
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
