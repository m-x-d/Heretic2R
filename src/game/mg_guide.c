// ****************************************************************************
//	mg_guide
//
//	High level monster guide information using BUOYAH! Navigation System(tm)
//
//	Heretic II
//	Copyright 1998 Raven Software
//
//  Mike Gummelt & Josh Weier
//
//	FIXME:
//
//	1) Way to send monsters to a buoy out of water/lava if they are drowning
//		or burning.
//
//	2) When you get to a buoy, do a trace to the next if it's blocked, you
//		need to find another way...
//	
//	3) Way to handle lots of monsters gathered around the same buoy... the one
//		that is at it can't get out, the others cant get to it, they just crowd it.
//
// ****************************************************************************

#include "g_local.h"
#include "Angles.h"
#include "Utilities.h"
#include "random.h"
#include "vector.h"
#include "buoy.h"
#include "g_monster.h"
#include "m_stats.h"
#include "fx.h"
#include "mg_guide.h"

#define	BUOY_SEARCH_TIME		10//10 seconds between choosing a buoy and getting there

#define BUOY_SEARCH_PASSES 6//sfs--number of passes through buoy list when searching
							//		(only accept buoys closer than 1/BUOY_SEARCH_PASSES * MAX_BUOY_DIST
							//		for first pass, etc.). if a buoy is found after a pass,
							//		we know we've got the closest buoy, and further passes can
							//		be skipped.

buoy_t	*find_next_buoy(edict_t *self, int sb_id, int fb_id);
qboolean Clear_Path(edict_t *self, vec3_t end);
void MG_AddBuoyEffect(edict_t *self, qboolean endbuoy);
qboolean MG_MakeConnection(edict_t *self, buoy_t *first_buoy, qboolean skipjump);
void assassinPrepareTeleportDest(edict_t *self, vec3_t spot);
qboolean MG_CheckClearPathToSpot(edict_t *self, vec3_t spot);

/* 
 *
 *
 *	Helper functions
 *
 *
 */

qboolean MG_ReachedBuoy (edict_t *self, vec3_t pspot)
{
	float	len, radius, z_diff, center;
	vec3_t	spot;

	if(!pspot)
		VectorCopy(self->monsterinfo.nav_goal, spot);
	else
		VectorCopy(pspot, spot);

	center = (self->absmin[2] + self->absmax[2]) * 0.5;
	z_diff = Q_fabs(spot[2] - center);
	if(z_diff > self->size[2])
		return false;

	len = vhlen(spot, self->s.origin);
	
	if(self->maxs[0]>16)
		radius = 24 + self->maxs[0];
	else
		radius = 40;//24 + 16

	if (len < (24+radius))
		return true;

	return false;
}

qboolean Clear_Path(edict_t *self, vec3_t end)
{
	trace_t	trace;
	vec3_t	mins, maxs;

	if(DEACTIVATE_BUOYS)
		return false;

	VectorCopy(self->mins, mins);
	VectorCopy(self->maxs, maxs);

	//sfs--i guess it's an ob/com thing, since msdev can probably be relied on to
	//		optimize it during compile, but mults are faster than divides.

	mins[0] *= 0.5;
	mins[1] *= 0.5;
	mins[2] *= 0.5;

	maxs[0] *= 0.5;
	maxs[1] *= 0.5;
	maxs[2] *= 0.5;

	if(self->mins[2] + 18 > mins[2])//need to account for stepheight
	{//took off less than 18
		mins[2] = self->mins[2] + 18;
		if(mins[2] > maxs[2])
			maxs[2] = mins[2];
	}
	
	//quicker way to discard points that are very not in a clear path
	if (!gi.inPVS(self->s.origin, end))
		return false;

	gi.trace(self->s.origin, mins, maxs, end, self, MASK_SOLID,&trace);

	if ((trace.fraction < 1) && (trace.ent != self->enemy))
		return false;

	return true;
}

/*
=============
clear_visible_pos

returns 1 if the spot is visible, but not through transparencies
=============
*/
qboolean clear_visible_pos (edict_t *self, vec3_t spot2)
{
	vec3_t	spot1;
	trace_t	trace;

	if (!self)
		return false;

	VectorCopy (self->s.origin, spot1);
	spot1[2] += self->viewheight;
	if(self->classID == CID_TBEAST)
	{
		vec3_t	forward;
	
		AngleVectors(self->s.angles, forward, NULL, NULL);
		VectorMA(spot1, self->maxs[0], forward, spot1);
	}

	//quicker way to discard points that are very not visible
	if (!gi.inPVS(self->s.origin, spot2))
		return false;

	gi.trace (spot1, vec3_origin, vec3_origin, spot2, self, MASK_SOLID,&trace);
	
	if (trace.fraction == 1.0)
		return true;
	return false;
}

int MG_SetFirstBuoy(edict_t *self)
{
	buoy_t		*found_buoy = NULL;
	buoy_t		*bestbuoy = NULL;
	qboolean	vis;
	vec3_t		vec;
	float		bestdist, len;
	int			i = 0;
	int			tracecount=0;
	float		buoy_passes = BUOY_SEARCH_PASSES;

	float search_pass_interval = MAX_BUOY_DIST / buoy_passes;
	float j;

	bestdist    = 9999999;

	if(!self->client)
	{
		if(!(self->monsterinfo.aiflags & AI_USING_BUOYS))
		{
			self->ai_mood = AI_MOOD_PURSUE;
			return NULL_BUOY;
		}
	}

	if(DEACTIVATE_BUOYS)
		return NULL_BUOY;
	//first, precalc all distances
	for(i = 0; i <= level.active_buoys; i++)
	{
		found_buoy = &level.buoy_list[i];
		VectorSubtract(self->s.origin, found_buoy->origin, vec);
		found_buoy->temp_dist = VectorLength(vec);
	}

	//now, do all the passes, going from closest to farthest
	for(j = 0; (j < buoy_passes)&&(!bestbuoy); j++)
	{
		for(i = 0; i <= level.active_buoys; i++)
		{
			vis	  = false;

			found_buoy = &level.buoy_list[i];
			len = found_buoy->temp_dist;

			//only consider buoys in the current interval--closer ones have already been
			//checked, and we'll save farther ones for later
			if (len < bestdist && len > search_pass_interval*j && len < search_pass_interval*(j+1.0))
			{
				tracecount++;
				vis = Clear_Path(self, found_buoy->origin);
			}

			if (vis)
			{
				bestdist = len;
				bestbuoy = found_buoy;
			}
		}
	}

	if (!bestbuoy)
		return NULL_BUOY;

	if(!self->client)
	{
		self->lastbuoy = NULL_BUOY;
		self->buoy_index = bestbuoy->id;
		VectorCopy(bestbuoy->origin, self->monsterinfo.nav_goal);
	}

	return bestbuoy->id;
}

qboolean MG_GoToRandomBuoy(edict_t *self)
{
	buoy_t		*found_buoy;
	qboolean	searching;
	qboolean	dead_end = false;
	int			i, nextbranch, j;
	int			branches_checked;
	qboolean	branch_checked[MAX_BUOY_BRANCHES];
	int			last_buoy = NULL_BUOY;
	
	if(MG_SetFirstBuoy(self) == NULL_BUOY)
		return false;

	found_buoy = &level.buoy_list[self->buoy_index];
	for(i = 0; i < self->mintel; i++)
	{
		searching = true;
		for(j = 0; j<MAX_BUOY_BRANCHES; j++)
			branch_checked[j] = false;

		while(searching)
		{
			nextbranch = irand(0, 2);
			branch_checked[nextbranch] = true;

			branches_checked = false;
			for(j = 0; j<MAX_BUOY_BRANCHES; j++)
			{
				if(branch_checked[j])
				{
					branches_checked++;
				}
			}
			
			if(found_buoy->nextbuoy[nextbranch] > NULL_BUOY &&
				found_buoy->nextbuoy[nextbranch] != self->buoy_index &&
				(found_buoy->nextbuoy[nextbranch] != self->lastbuoy || branches_checked >= MAX_BUOY_BRANCHES)&&
				(found_buoy->nextbuoy[nextbranch] != last_buoy || branches_checked >= MAX_BUOY_BRANCHES))
			{//nextbuoy off this one is not null, not my start buoy, not my last buoy, and not last buoy found
				last_buoy = found_buoy->id;
				found_buoy = &level.buoy_list[found_buoy->nextbuoy[nextbranch]];
				searching = false;
			}
			else if (branches_checked >= MAX_BUOY_BRANCHES)
			{//a dead end, checked all 3 branches
				if(i < 3)//can't run away far enough
					return false;
				searching = false;
				dead_end = true;
			}
		}
		if(dead_end)
			break;
	}

	if(self->ai_mood == AI_MOOD_FLEE)
		self->ai_mood_flags|=AI_MOOD_FLAG_IGNORE_ENEMY;
	else
		self->ai_mood = AI_MOOD_NAVIGATE;//wander?

	self->ai_mood_flags &= ~AI_MOOD_FLAG_DUMB_FLEE;
	self->ai_mood_flags|=AI_MOOD_FLAG_FORCED_BUOY;
	self->forced_buoy = found_buoy->id;

	QPostMessage(self, MSG_RUN,PRI_DIRECTIVE, NULL);

	MG_RemoveBuoyEffects(self);
	MG_MakeConnection(self, NULL, false);

	return true;
}

/*
MG_AssignMonsterNextBuoy(edict_t *self, buoy_t *startbuoy, buoy_t *endbuoy)

Actually assigns the bstartbuoy as the monster's buoy
*/

void MG_AssignMonsterNextBuoy(edict_t *self, buoy_t *startbuoy, buoy_t *endbuoy)
{
	edict_t		*showme;

	VectorCopy(startbuoy->origin, self->monsterinfo.nav_goal);
	if(self->buoy_index!=NULL_BUOY)
		self->lastbuoy = self->buoy_index;
	else
		self->lastbuoy = NULL_BUOY;
	self->buoy_index = startbuoy->id;
	self->last_buoy_time = level.time;
	if(BUOY_DEBUG>1)
	{
		showme = G_Find(NULL, FOFS(targetname), startbuoy->targetname);
		if(showme)
		{
			self->nextbuoy[0] = showme;
			MG_AddBuoyEffect(self, false);
		}

		if(endbuoy)
		{
			showme = G_Find(NULL, FOFS(targetname), endbuoy->targetname);
			if(showme)
			{
				self->nextbuoy[1] = showme;
				MG_AddBuoyEffect(self, true);
			}
		}
	}
}

/*
========================

MG_ValidBestBuoyForEnt

 Just see if this entity and this buoy are ok to be associated (clear path, etc.)

========================
*/
qboolean MG_ValidBestBuoyForEnt (edict_t *ent, buoy_t *test_buoy)
{
	vec3_t	v;
	float	dist;

	VectorSubtract(ent->s.origin, test_buoy->origin, v);
	dist = VectorLengthSquared(v);
	if(dist > 250000)//500 squared
		return false;//to damn far!

	return MG_CheckClearPathToSpot(ent, test_buoy->origin);
//	return Clear_Path(ent, test_buoy->origin);
}

/*
========================

MG_ResolveBuoyConnection

  Actually makes the connection between two buoys

========================
*/
qboolean MG_ResolveBuoyConnection(edict_t *self, buoy_t *bestbuoy, buoy_t *e_bestbuoy, vec3_t goalpos, qboolean dont_use_last, qboolean skipjump)
{//When called directly, this does not and should not set player_buoy
	buoy_t		*found_buoy = NULL;
	buoy_t		*dest = NULL;
	vec3_t		vec;
	float		len, e_len;
	edict_t		*showme = NULL;

	//FIXME:  Allow assassins to take any buoy even if can;t make a connection, since they can teleport
	//- Basically, pick the player's buoy and randomly pick one off of it or even that one...?

	if(self->lastbuoy == e_bestbuoy->id)
	{
		if(!MG_ReachedBuoy(self, e_bestbuoy->origin) && !clear_visible_pos(self, goalpos))
		{
			self->lastbuoy = NULL_BUOY;
			dont_use_last = false;
		}
	}

	if (bestbuoy->modflags&BUOY_JUMP && bestbuoy->jump_target_id == e_bestbuoy->id)
	{
		if(skipjump)
			dest = e_bestbuoy;
		else
			dest = bestbuoy;

		self->monsterinfo.searchType = SEARCH_BUOY;

		if(self->ai_mood != AI_MOOD_FLEE)
			self->ai_mood = AI_MOOD_NAVIGATE;

		VectorCopy(dest->origin, self->monsterinfo.nav_goal);

#ifdef _DEVEL
		if (BUOY_DEBUG)
			gi.dprintf("Found 1-step JUMP connection at %s\n", dest->targetname);
#endif
		if(self->buoy_index!=NULL_BUOY)
			self->lastbuoy = self->buoy_index;
		else
			self->lastbuoy = NULL_BUOY;
		
		self->buoy_index =dest->id;

		self->last_buoy_time = level.time;
		return true;
	}

	//hey what if we're touching our e_bestbuoy buoy? - also, if this is a jump buoy and e_bestboy is the target of it, foce use of bestbuoy
	if (bestbuoy == e_bestbuoy)
	{
		if(dont_use_last)
			if(self->lastbuoy == bestbuoy->id)//found same buoy just touched as next buoy
				return false;

		self->monsterinfo.searchType = SEARCH_BUOY;

		if(self->ai_mood != AI_MOOD_FLEE)
			self->ai_mood = AI_MOOD_NAVIGATE;

		VectorCopy(bestbuoy->origin, self->monsterinfo.nav_goal);

#ifdef _DEVEL
		if (BUOY_DEBUG)
			gi.dprintf("Found 1-step connection at %s\n", bestbuoy->targetname);
#endif
		if(self->buoy_index!=NULL_BUOY)
			self->lastbuoy = self->buoy_index;
		else
			self->lastbuoy = NULL_BUOY;
		self->buoy_index = bestbuoy->id;
		self->last_buoy_time = level.time;
		return true;
	}

	if (bestbuoy && e_bestbuoy)
	{
		VectorSubtract(goalpos, self->s.origin, vec);
		e_len = VectorLength(vec);

		VectorSubtract(e_bestbuoy->origin, goalpos, vec);
		len = VectorLength(vec);

		dest = find_next_buoy(self, bestbuoy->id, e_bestbuoy->id);

		if (dest != NULL)	
		{
#ifdef _DEVEL
			if (BUOY_DEBUG)
			{
				gi.dprintf("Found connection from %s to %s\n", bestbuoy->targetname, e_bestbuoy->targetname);
			}
#endif			
			if(dont_use_last)
			{
				if(self->lastbuoy == dest->id)//found same buoy just touched as next buoy
					return false;
			}

			self->monsterinfo.searchType = SEARCH_BUOY;

			if(self->ai_mood != AI_MOOD_FLEE)
				self->ai_mood = AI_MOOD_NAVIGATE;

			if((!(bestbuoy->modflags&BUOY_JUMP)||skipjump) && MG_ReachedBuoy(self, bestbuoy->origin))
			{
				MG_AssignMonsterNextBuoy(self, dest, e_bestbuoy);
			}
			else
			{
				MG_AssignMonsterNextBuoy(self, bestbuoy, e_bestbuoy);
			}
			return true;
		}
#ifdef _DEVEL
		else if (BUOY_DEBUG)
		{
			gi.dprintf("Failed to find a path\n");
		}
#endif
	}

	//EXPERIMENTAL
	//ok, now you can backtrack since couldn't get there
	//self->lastbuoy = NULL_BUOY;

#ifdef _DEVEL
	if(BUOY_DEBUG_LITE||BUOY_DEBUG)
	{
		if (!bestbuoy)
			gi.dprintf("%s COULDN'T FIND BUOYS FOR SELF!!!\n", self->classname);
		else if (!e_bestbuoy)
			gi.dprintf("%s COULDN'T FIND BUOYS FOR %s!!!\n", self->classname, self->enemy->classname);
	}
#endif
	return false;
}

/*
========================

MG_MakeStartForcedConnection

  Attempts to make a connection between a buoy and a monster's enemy
  This function itself just finds the two buoys to attempt to make the connection between,
  MG_ResolveBuoyConnection actually makes the connection between two buoys

========================
*/
qboolean MG_MakeStartForcedConnection(edict_t *self, int sforced_buoy, qboolean dont_use_last, qboolean skipjump)
{
	buoy_t		*found_buoy = NULL;
	buoy_t		*e_bestbuoy = NULL;
	buoy_t		*bestbuoy = NULL;
	buoy_t		*dest = NULL;
	qboolean	e_vis;
	vec3_t		vec, goalpos, e_buoyvec;
	float		bestdist, e_bestdist, e_len, e_buoydist;
	int			i;
	int			tracecount = 0;
	float		tracedist_total=0;
	float		buoy_passes = BUOY_SEARCH_PASSES;
	float		e_radius;

	float search_pass_interval = MAX_BUOY_DIST / buoy_passes;
	float k;

	if(DEACTIVATE_BUOYS)
		return false;

	bestbuoy	= &level.buoy_list[sforced_buoy];
	bestdist    = 0;
	e_bestdist  = 9999999;

	self->last_buoyed_enemy = self->enemy;//Remember the last enemy I looked for

	VectorCopy(self->enemy->s.origin, goalpos);
	goalpos[2] += self->viewheight;

	if(self->enemy->maxs[0]>16)
		e_radius = 24 + self->enemy->maxs[0];
	else
		e_radius = 40;//24 + 16

	//first, precalc all distances
	for(i = 0; i <= level.active_buoys; i++)
	{
		found_buoy = &level.buoy_list[i];
		VectorSubtract(goalpos, found_buoy->origin, vec);
		found_buoy->temp_e_dist = VectorLength(vec);
		if (found_buoy->temp_e_dist < (24+e_radius))
		{
			e_bestbuoy = found_buoy;
			e_bestdist = found_buoy->temp_dist;
			break;
		}
	}

	//now, do all the passes, going from closest to farthest
	for(k = 0; (k < buoy_passes)&&(!bestbuoy || !e_bestbuoy); k++)
	{
		for(i = 0; i <= level.active_buoys; i++)
		{
			found_buoy = &level.buoy_list[i];

			e_vis = false;
			e_len = found_buoy->temp_e_dist;

			//only consider buoys in the current interval--closer ones have already been
			//checked, and we'll save farther ones for later
			if (e_len < e_bestdist && e_len > search_pass_interval*k && e_len < search_pass_interval*(k+1.0))
			{
				tracecount++;
				tracedist_total+=e_len;
				e_vis = Clear_Path(self->enemy, found_buoy->origin);
			}

			if (e_vis)
			{
				e_bestdist = e_len;
				e_bestbuoy = found_buoy;
			}
		}
	}

	tracecount=0;
	tracedist_total=0;

	if (!e_bestbuoy && irand(0,10) < 5)
	{//ok, Clear_Path too restrictive, try just clear_visible

		//distances precalced already, so skip that step

		//now, do all the passes, going from closest to farthest
		for(k = 0; (k < buoy_passes)&&(!e_bestbuoy); k++)
		{
			for(i = 0; i <= level.active_buoys; i++)
			{
				found_buoy = &level.buoy_list[i];
				e_vis = false;

				e_len = found_buoy->temp_e_dist;

				//only consider buoys in the current interval--closer ones have already been
				//checked, and we'll save farther ones for later
				if (e_len < e_bestdist && e_len > search_pass_interval*k && e_len < search_pass_interval*(k+1.0))
				{
					tracecount++;
					tracedist_total+=e_len;
					e_vis = clear_visible_pos(self->enemy, found_buoy->origin);
				}

				if (e_vis)
				{
					e_bestdist = e_len;
					e_bestbuoy = found_buoy;
				}
			}
		}
	}

	if (e_bestdist > MAX_BUOY_DIST)
	{
#ifdef _DEVEL
		if(BUOY_DEBUG_LITE||BUOY_DEBUG)
			gi.dprintf("%s's %s CLOSEST BUOY TOO FAR AWAY (%4.2f)\n", self->enemy->classname, vtos(self->s.origin), e_bestdist);
#endif
		return false;
	}

	if(!(bestbuoy->modflags&BUOY_JUMP)||skipjump)
	{//don't skip jump buoys, they're crucial
		if(e_bestbuoy)
		{
			VectorSubtract(e_bestbuoy->origin, self->s.origin, e_buoyvec);
			e_buoydist = VectorLength(e_buoyvec);
			if (bestbuoy != e_bestbuoy && e_buoydist > bestdist)
			{//enemy best buoy is farther away from me and not my buoy
				if(Clear_Path(self, e_bestbuoy->origin))
				{//can go straight at enemy best buoy even though farther away
#ifdef _DEVEL
					if(BUOY_DEBUG_LITE||BUOY_DEBUG)
						gi.dprintf("%s going after %s's buoy even though farther\n", self->classname, self->enemy->classname);
#endif
					bestbuoy = e_bestbuoy;
					bestdist = e_bestdist;
				}
			}
		}
	}


	if(e_bestbuoy)
	{//if going after a player, set his buoy for other monsters this frame
		if(self->enemy->client)
		{
#ifdef _DEVEL
			if(BUOY_DEBUG)
				gi.dprintf("%s setting player_buoy %d to %s\n", self->classname, self->enemy->s.number, e_bestbuoy->targetname);
#endif
			level.player_buoy[self->enemy->s.number - 1] = e_bestbuoy->id;
		}
	}

	return MG_ResolveBuoyConnection(self, bestbuoy, e_bestbuoy, goalpos, dont_use_last, skipjump);
}

/*
========================

MG_MakeForcedConnection

  Attempts to make a connection between a monster and its forced_buoy
  This function itself just finds the two buoys to attempt to make the connection between,
  MG_ResolveBuoyConnection actually makes the connection between two buoys

========================
*/
qboolean MG_MakeForcedConnection(edict_t *self, int forced_buoy, qboolean dont_use_last, qboolean skipjump)
{
	buoy_t		*found_buoy = NULL;
	buoy_t		*e_bestbuoy = NULL;
	buoy_t		*bestbuoy = NULL;
	buoy_t		*dest = NULL;
	qboolean	vis;
	vec3_t		vec, goalpos, e_buoyvec;
	float		bestdist, e_bestdist, len, e_buoydist;
	int			i;
	int			tracecount = 0;
	float		tracedist_total=0;
	float		buoy_passes = BUOY_SEARCH_PASSES;
	float		radius;

	float search_pass_interval = MAX_BUOY_DIST / buoy_passes;
	float k;

	bestdist    = 9999999;
	e_bestdist  = 9999999;

	if(DEACTIVATE_BUOYS)
		return false;

	e_bestbuoy = &level.buoy_list[forced_buoy];
	e_bestdist = 0;
	VectorCopy(e_bestbuoy->origin, goalpos);

#ifdef _DEVEL
	if(!e_bestbuoy)
		gi.dprintf("ERROR: forced_buoy not a valid buoy!!!\n");
#endif
	if(self->maxs[0]>16)
		radius = 24 + self->maxs[0];
	else
		radius = 40;//24 + 16


	//first, precalc all distances
	for(i = 0; i <= level.active_buoys; i++)
	{
		found_buoy = &level.buoy_list[i];
		VectorSubtract(self->s.origin, found_buoy->origin, vec);
		found_buoy->temp_dist = VectorLength(vec);
		if (found_buoy->temp_dist < (24+radius))
		{
			bestbuoy = found_buoy;
			bestdist = found_buoy->temp_dist;
			break;
		}
	}

	//now, do all the passes, going from closest to farthest
	for(k = 0; (k < buoy_passes)&&(!bestbuoy || !e_bestbuoy); k++)
	{
		for(i = 0; i <= level.active_buoys; i++)
		{
			found_buoy = &level.buoy_list[i];
			len = found_buoy->temp_dist;

			vis	  = false;

			//only consider buoys in the current interval--closer ones have already been
			//checked, and we'll save farther ones for later
			if (len < bestdist && len > search_pass_interval*k && len < search_pass_interval*(k+1.0))
			{
				tracecount++;
				tracedist_total+=len;
				vis = Clear_Path(self, found_buoy->origin);
			}

			if (vis)
			{
				bestdist = len;
				bestbuoy = found_buoy;
			}
		}
	}

	tracecount=0;
	tracedist_total=0;

	if (bestdist > MAX_BUOY_DIST)
	{
#ifdef _DEVEL
		if(BUOY_DEBUG_LITE||BUOY_DEBUG)
			gi.dprintf("%s's at %s CLOSEST BUOY TOO FAR AWAY (%4.2f)\n", self->classname, vtos(self->s.origin), bestdist);
#endif
		self->pathfind_nextthink = level.time + 3;//wait 3 seconds before trying to use buoys again
		return false;
	}

	if(!(bestbuoy->modflags&BUOY_JUMP)||skipjump)
	{//don't skip jump buoys, they're crucial
		VectorSubtract(e_bestbuoy->origin, self->s.origin, e_buoyvec);
		e_buoydist = VectorLength(e_buoyvec);
		if (bestbuoy != e_bestbuoy && e_buoydist > bestdist)
		{//enemy best buoy is farther away from me and not my buoy
			if(Clear_Path(self, e_bestbuoy->origin))
			{//can go straight at enemy best buoy even though farther away
#ifdef _DEVEL
				if(BUOY_DEBUG_LITE||BUOY_DEBUG)
					gi.dprintf("%s going after forced_buoy %s even though farther\n", self->classname, e_bestbuoy->targetname);
#endif
				bestbuoy = e_bestbuoy;
				bestdist = e_bestdist;
			}
		}
	}

	return MG_ResolveBuoyConnection(self, bestbuoy, e_bestbuoy, goalpos, dont_use_last, skipjump);
}

/*
========================

MG_MakeNormalConnection

  Attempts to make a buoy connection between a monster and its enemy
  This function itself just finds the two buoys to attempt to make the connection between,
  MG_ResolveBuoyConnection actually makes the connection between two buoys

========================
*/
qboolean MG_MakeNormalConnection(edict_t *self, qboolean dont_use_last, qboolean skipjump)
{
	buoy_t		*found_buoy = NULL;
	buoy_t		*e_bestbuoy = NULL;
	buoy_t		*bestbuoy = NULL;
	buoy_t		*dest = NULL;
	qboolean	e_vis, vis;
	vec3_t		vec, goalpos, e_buoyvec;
	float		bestdist, e_bestdist, len, e_len, e_buoydist;
	int			i;
	int			tracecount = 0;
	float		tracedist_total=0;
	float		buoy_passes = BUOY_SEARCH_PASSES;
	float		radius, e_radius;

	float search_pass_interval = MAX_BUOY_DIST / buoy_passes;
	float k;

	bestdist    = 9999999;
	e_bestdist  = 9999999;

	if(DEACTIVATE_BUOYS)
		return false;

	VectorCopy(self->enemy->s.origin, goalpos);
	goalpos[2] += self->viewheight;

	if(self->maxs[0]>16)
		radius = 24 + self->maxs[0];
	else
		radius = 40;//24 + 16

	if(self->enemy->maxs[0]>16)
		e_radius = 24 + self->enemy->maxs[0];
	else
		e_radius = 40;//24 + 16

	//first, precalc all distances
	for(i = 0; i <= level.active_buoys; i++)
	{
		found_buoy = &level.buoy_list[i];
		
		if(!bestbuoy)
		{
			VectorSubtract(self->s.origin, found_buoy->origin, vec);
			found_buoy->temp_dist = VectorLength(vec);
			if (found_buoy->temp_dist < (24+radius))
			{
				bestbuoy = found_buoy;
				bestdist = found_buoy->temp_dist;
			}
		}

		if(!e_bestbuoy)
		{
			VectorSubtract(goalpos, found_buoy->origin, vec);
			found_buoy->temp_e_dist = VectorLength(vec);
			if (found_buoy->temp_e_dist < (24+e_radius))
			{
				e_bestbuoy = found_buoy;
				e_bestdist = found_buoy->temp_dist;
			}
		}
		if(e_bestbuoy && bestbuoy)
			break;
	}

	//now, do all the passes, going from closest to farthest
	for(k = 0; (k < buoy_passes)&&(!bestbuoy || !e_bestbuoy); k++)
	{
		for(i = 0; i <= level.active_buoys; i++)
		{
			found_buoy = &level.buoy_list[i];
			len = found_buoy->temp_dist;

			vis	  = false;
			e_vis = false;

			//only consider buoys in the current interval--closer ones have already been
			//checked, and we'll save farther ones for later
			if (len < bestdist && len > search_pass_interval*k && len < search_pass_interval*(k+1.0))
			{
				tracecount++;
				tracedist_total+=len;
				vis = Clear_Path(self, found_buoy->origin);
			}

			if (vis)
			{
				bestdist = len;
				bestbuoy = found_buoy;
			}

			e_len = found_buoy->temp_e_dist;

			//only consider buoys in the current interval--closer ones have already been
			//checked, and we'll save farther ones for later
			if (e_len < e_bestdist && e_len > search_pass_interval*k && e_len < search_pass_interval*(k+1.0))
			{
				tracecount++;
				tracedist_total+=e_len;
				e_vis = Clear_Path(self->enemy, found_buoy->origin);
			}

			if (e_vis)
			{
				e_bestdist = e_len;
				e_bestbuoy = found_buoy;
			}
		}
	}

	tracecount=0;
	tracedist_total=0;

	if (bestdist > MAX_BUOY_DIST)
	{
#ifdef _DEVEL
		if(BUOY_DEBUG_LITE||BUOY_DEBUG)
			gi.dprintf("%s's %s CLOSEST BUOY TOO FAR AWAY (%4.2f)\n", self->classname, vtos(self->s.origin), bestdist);
#endif
		return false;
	}

	if (!bestbuoy && !e_bestbuoy)
	{
#ifdef _DEVEL
		if(BUOY_DEBUG_LITE||BUOY_DEBUG)
			gi.dprintf("%s' %s COULDN'T FIND BUOYS FOR SELF OR %s!!!\n", self->classname, vtos(self->s.origin), self->enemy->classname);
#endif
		return false;
	}

	if (!e_bestbuoy && irand(0,10) < 5)
	{//ok, Clear_Path too restrictive, try just clear_visible

		//distances precalced already, so skip that step

		//now, do all the passes, going from closest to farthest
		for(k = 0; (k < buoy_passes)&&(!e_bestbuoy); k++)
		{
			for(i = 0; i <= level.active_buoys; i++)
			{
				found_buoy = &level.buoy_list[i];
				e_vis = false;

				e_len = found_buoy->temp_e_dist;

				//only consider buoys in the current interval--closer ones have already been
				//checked, and we'll save farther ones for later
				if (e_len < e_bestdist && e_len > search_pass_interval*k && e_len < search_pass_interval*(k+1.0))
				{
					tracecount++;
					tracedist_total+=e_len;
					e_vis = clear_visible_pos(self->enemy, found_buoy->origin);
				}

				if (e_vis)
				{
					e_bestdist = e_len;
					e_bestbuoy = found_buoy;
				}
			}
		}
	}

	if (e_bestdist > MAX_BUOY_DIST)
	{
#ifdef _DEVEL
		if(BUOY_DEBUG_LITE||BUOY_DEBUG)
			gi.dprintf("%s's %s CLOSEST BUOY TOO FAR AWAY (%4.2f)\n", self->enemy->classname, vtos(self->s.origin), e_bestdist);
#endif
		return false;
	}

	if(!(bestbuoy->modflags&BUOY_JUMP)||skipjump)
	{//don't skip jump buoys, they're crucial
		if(e_bestbuoy)
		{
			VectorSubtract(e_bestbuoy->origin, self->s.origin, e_buoyvec);
			e_buoydist = VectorLength(e_buoyvec);
			if (bestbuoy != e_bestbuoy && e_buoydist > bestdist)
			{//enemy best buoy is farther away from me and not my buoy
				if(Clear_Path(self, e_bestbuoy->origin))
				{//can go straight at enemy best buoy even though farther away
#ifdef _DEVEL
					if(BUOY_DEBUG_LITE||BUOY_DEBUG)
						gi.dprintf("%s going after %s's buoy even though farther\n", self->classname, self->enemy->classname);
#endif
					bestbuoy = e_bestbuoy;
					bestdist = e_bestdist;
				}
			}
		}
	}

	if(e_bestbuoy)
	{//if going after a player, set his buoy for other monsters this frame
		if(self->enemy->client)
		{
#ifdef _DEVEL
			if(BUOY_DEBUG)
				gi.dprintf("%s setting player_buoy %d to %s\n", self->classname, self->enemy->s.number, e_bestbuoy->targetname);
#endif
			level.player_buoy[self->enemy->s.number - 1] = e_bestbuoy->id;
		}
	}

	return MG_ResolveBuoyConnection(self, bestbuoy, e_bestbuoy, goalpos, dont_use_last, skipjump);
}


/*
========================

MG_MakeConnection

	Determines if monster should be pursuing a forced_buoy, if not, calls normal
	enemy-tracking buoy connection finding function

========================
*/
int MG_MakeConnection_Go(edict_t *self, buoy_t *first_buoy, qboolean skipjump)
{
	qboolean	found_path = false;
	qboolean	dont_use_last;
	buoy_t		*forced_buoy = NULL;
	buoy_t		*found_buoy = NULL;
	int			k;
	qboolean	last_buoy_clear = false;

	if(self->spawnflags & MSF_FIXED)
		return false;

	MG_RemoveBuoyEffects(self);

	if(self->enemy && !(self->ai_mood_flags&AI_MOOD_FLAG_IGNORE_ENEMY) && self->ai_mood != AI_MOOD_FLEE)
	{
		self->ai_mood_flags &= ~AIMF_CANT_FIND_ENEMY;

		if(self->enemy!=self->last_buoyed_enemy)//Current enemywasn't the last enemy I looked for...
		{
			dont_use_last = false;
			self->lastbuoy = NULL_BUOY;//so forget last buoy I used
		}
		else if(self->monsterinfo.searchType == SEARCH_BUOY)
			dont_use_last = true;
		else
			dont_use_last = false;

		self->last_buoyed_enemy = self->enemy;//Remember the last enemy I looked for

		if(self->enemy->client)
		{//see if this player already has a bestbuoy found by a previous monster this frame
			if(level.player_buoy[self->enemy->s.number - 1]>NULL_BUOY)//entity numbers 0 - 8 should be players
			{//FIXME: if not, try his last player_buoy first!
				if(BUOY_DEBUG)
				{
					for(k = 0; k<level.active_buoys; k++)
					{
						if(k == level.player_buoy[self->enemy->s.number - 1])
						{
							found_buoy = &level.buoy_list[k];
							break;
						}
					}
					if(found_buoy)
					{
#ifdef _DEVEL
						gi.dprintf("%s using player_buoy %d (%s) set previously this frame\n", 
								self->classname, self->enemy->s.number, found_buoy->targetname);
#endif
					}
				}
				self->forced_buoy = level.player_buoy[self->enemy->s.number - 1];//just stores id in player_buoy
				if(first_buoy)
				{
					forced_buoy = &level.buoy_list[self->forced_buoy];
#ifdef _DEVEL
					if(BUOY_DEBUG)
						gi.dprintf("%s trying connection to player_buoy %d (%s) from first_buoy %s\n", 
								self->classname, self->enemy->s.number, found_buoy->targetname, first_buoy->targetname);
#endif
					found_path = MG_ResolveBuoyConnection(self, first_buoy, forced_buoy, forced_buoy->origin, dont_use_last, skipjump);
				}
				else
				{
#ifdef _DEVEL
					if(BUOY_DEBUG)
						gi.dprintf("%s trying normal connection to player_buoy %d (%s)\n", 
								self->classname, self->enemy->s.number, found_buoy->targetname);
#endif
					found_path = MG_MakeForcedConnection(self, self->forced_buoy, dont_use_last, skipjump);
				}
			}
		}
	}
	else
	{
		dont_use_last = false;
		self->last_buoyed_enemy = NULL;

		if(self->ai_mood_flags&AI_MOOD_FLAG_FORCED_BUOY && self->forced_buoy != -1)
		{
			if(first_buoy)
			{
				forced_buoy = &level.buoy_list[self->forced_buoy];
#ifdef _DEVEL
				if(BUOY_DEBUG)
					gi.dprintf("%s trying non-enemy forced_buoy connection to %s from first_buoy %s\n", 
							self->classname, forced_buoy->targetname, first_buoy->targetname);
#endif
				return MG_ResolveBuoyConnection(self, first_buoy, forced_buoy, forced_buoy->origin, dont_use_last, skipjump);
			}
			else
			{
#ifdef _DEVEL
				if(BUOY_DEBUG)
					gi.dprintf("%s trying non-enemy forced_buoy connection to %s\n", 
							self->classname, level.buoy_list[self->forced_buoy].targetname);
#endif
				return MG_MakeForcedConnection(self, self->forced_buoy, dont_use_last, skipjump);
			}
		}
		else
		{
			self->lastbuoy = NULL_BUOY;//so forget last buoy I used
			return false;
		}
	}

	if(!found_path)
	{
		if(self->ai_mood == AI_MOOD_FLEE)
			return false;

		if(self->enemy->client)
		{
			if(level.player_last_buoy[self->enemy->s.number - 1] > NULL_BUOY)
			{//see if the player_last_buoy is a valid buoy for the enemy, if so, go for it
				if(MG_ValidBestBuoyForEnt(self->enemy, &level.buoy_list[level.player_last_buoy[self->enemy->s.number - 1]]))
				{
					last_buoy_clear = true;
					goto last_resort;
				}
			}
		}
	
#ifdef _DEVEL
		if(BUOY_DEBUG)
			gi.dprintf("%s searching for player(%d)'s bestbuoy\n", self->classname, self->enemy->s.number);
#endif
		if(first_buoy)
		{
#ifdef _DEVEL
			if(BUOY_DEBUG)
				gi.dprintf("%s making connection from forced start %s to %s\n", 
						self->classname, first_buoy->targetname, self->enemy->classname);
#endif
			found_path = MG_MakeStartForcedConnection(self, first_buoy->id, dont_use_last, skipjump);
		}
		else
		{
#ifdef _DEVEL
			if(BUOY_DEBUG)
				gi.dprintf("%s making normal connection to %s\n", self->classname, self->enemy->classname);
#endif
			found_path = MG_MakeNormalConnection(self, dont_use_last, skipjump);
		}
	}

	if(found_path)
		return true;

//OK you!  Can't find ANY buoy connections, let's go with player_lastbuoy even if it can't connect to you!
	self->ai_mood_flags |= AIMF_CANT_FIND_ENEMY;

last_resort:
	if(self->enemy->client)
	{
		if(level.player_last_buoy[self->enemy->s.number - 1] > NULL_BUOY)
		{//try the player_last_buoy, don't care if it can connect to player!
		//FIXME: require that the player be withing 250 of this buoy at least?
			if(BUOY_DEBUG)
			{
				for(k = 0; k<level.active_buoys; k++)
				{
					if(k == level.player_last_buoy[self->enemy->s.number - 1])
					{
						found_buoy = &level.buoy_list[k];
						break;
					}
				}
			}
			//we dont actually set self->forced_buoy since we want to find a better buoy next time we look
			forced_buoy = &level.buoy_list[level.player_last_buoy[self->enemy->s.number - 1]];
			if(!last_buoy_clear)
			{
				if(self->ai_mood_flags & AIMF_SEARCHING || MG_ReachedBuoy(self, forced_buoy->origin))
				{
					return 3;
				}
			}

			if(first_buoy)
			{
#ifdef _DEVEL
				if(BUOY_DEBUG)
				{
					if(last_buoy_clear)
						gi.dprintf("%s using clear-path player_last_buoy %d (%s) from first_buoy %s\n", 
								self->classname, self->enemy->s.number, found_buoy->targetname, first_buoy->targetname);
					else
						gi.dprintf("%s using player_last_buoy %d (%s) from first_buoy %s even though it doesn't connect to player!\n", 
								self->classname, self->enemy->s.number, found_buoy->targetname, first_buoy->targetname);
				}
#endif
				if(MG_ResolveBuoyConnection(self, first_buoy, forced_buoy, forced_buoy->origin, dont_use_last, skipjump))
				{
					return 2;
				}
			}
			else
			{
#ifdef _DEVEL
				if(BUOY_DEBUG)
				{
					if(last_buoy_clear)
						gi.dprintf("%s using clear-path player_last_buoy %d (%s)\n", 
								self->classname, self->enemy->s.number, found_buoy->targetname);
					else
						gi.dprintf("%s using player_last_buoy %d (%s) even though it doesn't connect to player!\n", 
								self->classname, self->enemy->s.number, found_buoy->targetname);
				}
#endif
				if(MG_MakeForcedConnection(self, forced_buoy->id, dont_use_last, skipjump))
				{
					return 2;
				}
			}
		}
	}
	//damn, lost him!  Currrzzze you Corvuzzz!!!
#ifdef _DEVEL
	if(BUOY_DEBUG)
		gi.dprintf("FAILURE: %s Could not find %s in any way shape or form!!!\n", self->classname, self->enemy->classname);
#endif
	return false;
}

qboolean MG_MakeConnection(edict_t *self, buoy_t *first_buoy, qboolean skipjump)
{//just for debug info
	qboolean	result;

#ifdef _DEVEL
	if(BUOY_DEBUG)
		gi.dprintf("========================================================\n    %s Start MakeConnection    \n========================================================\n", self->classname);
#endif	
	result = MG_MakeConnection_Go (self, first_buoy, skipjump);

	if(!(self->ai_mood_flags&AIMF_CANT_FIND_ENEMY))
	{
		self->monsterinfo.last_successful_enemy_tracking_time = level.time;
		self->ai_mood_flags &= ~AIMF_SEARCHING;
	}

	if(result != true)
	{//If can't find him(not including player_last_buoys) for 5 - 10 seconds, go into wander mode...
#ifdef _DEVEL
		if (BUOY_DEBUG && !result)
		{
			gi.dprintf("MG_MakeConnection: failed\n");
		}
#endif
		if(result == 3 && !(self->ai_mood_flags&AIMF_SEARCHING))
		{
#ifdef _DEVEL
			if(BUOY_DEBUG)
				gi.dprintf("%s got to %s's last_buoy, searching normally...\n", self->classname, self->enemy->classname);
#endif
			self->monsterinfo.last_successful_enemy_tracking_time = level.time;
			self->monsterinfo.searchType = SEARCH_COMMON;
			self->ai_mood = AI_MOOD_PURSUE;
			self->ai_mood_flags |= AIMF_SEARCHING;
		}
		else if(self->enemy &&
			self->ai_mood != AI_MOOD_FLEE &&
			!(self->ai_mood_flags & AI_MOOD_FLAG_IGNORE_ENEMY) &&
			self->monsterinfo.last_successful_enemy_tracking_time + MONSTER_SEARCH_TIME < level.time)
		{//give up, can't see him or find path to him for ten seconds now...
			if(self->classID == CID_ASSASSIN && self->monsterinfo.last_successful_enemy_tracking_time + MONSTER_SEARCH_TIME + 20 > level.time)
			{//assassins get an extra 20 seconds to look for the enemy and try to teleport to him
			}
			else
			{
#ifdef _DEVEL
				if(BUOY_DEBUG)
					gi.dprintf("%s giving up finding %s, wandering around\n", self->classname, self->enemy->classname);
#endif
				if(self->enemy->client)
					self->oldenemy = self->enemy;
				self->enemy = NULL;

				if(!result && self->ai_mood == AI_MOOD_WANDER)
					self->ai_mood = AI_MOOD_STAND;
				else
					self->ai_mood = AI_MOOD_WANDER;
			}
		}
		else if(!result && self->ai_mood != AI_MOOD_FLEE && self->enemy)
		{
			self->monsterinfo.searchType = SEARCH_COMMON;
			self->ai_mood = AI_MOOD_PURSUE;
		}
		
		if(!result && self->ai_mood == AI_MOOD_WANDER)
		{
			self->monsterinfo.pausetime = 0;
			self->ai_mood = AI_MOOD_STAND;
		}

		if(result == 3)
			result = false;
	}

#ifdef _DEVEL
	if(BUOY_DEBUG)
		gi.dprintf("========================================================\n    %s End MakeConnection    \n========================================================\n", self->classname);
#endif
	return result;
}

qboolean MG_CheckClearPathToEnemy(edict_t *self)
{
	trace_t	trace;
	vec3_t	mins, checkspot, enemy_dir, center;
	float	dist, i;
	qboolean	close_ok = false;
	
	if(!self->enemy)
		return false;

	VectorCopy(self->mins, mins);
	mins[2] += 18;
	gi.trace(self->s.origin, mins, self->maxs, self->enemy->s.origin, self, MASK_SOLID,&trace);

	if(trace.ent)
	{
		if(trace.ent == self->enemy)
			return true;//bumped into our enemy!
	}

	if(trace.allsolid||trace.startsolid)//trace is through a wall next to me?
		return false;

	if(trace.fraction<1.0)
	{//couldn't get to enemy
		VectorSubtract(self->enemy->s.origin, trace.endpos, enemy_dir);
		dist = VectorLength(enemy_dir);
		if(dist>48 || !visible(self, self->enemy))
			return false;//couldn't even get close to a visible enemy
	}

	if(!self->groundentity || (self->flags & FL_INWATER && self->enemy->flags & FL_INWATER))
		return true;

	if(self->flags & FL_FLY || self->movetype == PHYSICSTYPE_FLY || !self->gravity || self->classID == CID_GORGON)
		return true;

	if((self->flags & FL_INWATER || (self->flags & FL_SWIM) || (self->flags & FL_AMPHIBIAN))&&
		self->enemy->flags & FL_INWATER)
		return true;

	//Now lets see if there is a solid ground or steps path to the enemy

	//FIXME: what about jumping monsters?  Call a jump message?

	VectorAverage(self->absmin, self->absmax, center);

	VectorSubtract(self->enemy->s.origin, center, enemy_dir);
	dist = VectorNormalize(enemy_dir);

	VectorCopy(self->s.origin, mins);
	mins[2] += self->mins[2];

	for(i = 0; i < dist; i += 8)
	{//check to see if ground is there all the way to enemy
		VectorMA(mins, i, enemy_dir, checkspot);
		checkspot[2] -= 3;
		if(!(gi.pointcontents(checkspot) & CONTENTS_SOLID))
		{//not solid underneath
			checkspot[2] -= 16;
			if(!(gi.pointcontents(checkspot) & CONTENTS_SOLID))
			{
				return false;//not a step down
			}
		}
	}
	return true;
}

qboolean MG_CheckClearPathToSpot(edict_t *self, vec3_t spot)
{
	trace_t	trace;
	vec3_t	mins, checkspot, enemy_dir;
	float	dist, i;
	qboolean	close_ok = false;
	
	VectorCopy(self->mins, mins);
	mins[2] += 18;
	gi.trace(self->s.origin, mins, self->maxs, spot, self, MASK_SOLID,&trace);

	if(trace.ent)
	{
		if(trace.ent == self->enemy)
			return true;//bumped into our enemy!
	}

	if(trace.allsolid||trace.startsolid)//trace is through a wall next to me?
		return false;

	if(trace.fraction<1.0)
	{//couldn't get to enemy
		VectorSubtract(spot, trace.endpos, enemy_dir);
		dist = VectorLength(enemy_dir);
		if(dist>48 || !visible_pos(self, spot))
			return false;//couldn't even get close to a visible enemy
	}

	if(!self->groundentity || (self->flags & FL_INWATER && gi.pointcontents(spot)&MASK_WATER))
		return true;

	if(self->flags & FL_FLY || self->movetype == PHYSICSTYPE_FLY || !self->gravity || self->classID == CID_GORGON)
		return true;

	//Now lets see if there is a solid ground or steps path to the enemy

	//FIXME: what about jumping monsters?  Call a jump message?

	VectorSubtract(spot, self->s.origin, enemy_dir);
	dist = VectorNormalize(enemy_dir);

	VectorCopy(self->s.origin, mins);
	mins[2] += self->mins[2];

	for(i = 0; i < dist; i += 8)
	{//check to see if ground is there all the way to enemy
		VectorMA(mins, i, enemy_dir, checkspot);
		checkspot[2] -= 3;
		if(!(gi.pointcontents(checkspot) & CONTENTS_SOLID))
		{//not solid underneath
			checkspot[2] -= 16;
			if(!(gi.pointcontents(checkspot) & CONTENTS_SOLID))
				return false;//not a step down
		}
	}
	return true;
}

qboolean MG_OK_ToShoot(edict_t *self, edict_t *target)
{
	if(target == self->enemy || 
			(target->takedamage && 
				(target->classID == CID_RAT || 
					(!(target->svflags&SVF_MONSTER) && target->health<50)
				)
			)
		)
		return true;
	return false;
}

qboolean MG_CheckClearShotToEnemy(edict_t *self)
{
	trace_t		trace;
	vec3_t		startpos, endpos;
	vec3_t		zerovec = {0, 0, 0};
	
	VectorCopy(self->s.origin, startpos);
	startpos[2]+=self->viewheight;

	VectorCopy(self->enemy->s.origin, endpos);

	gi.trace(startpos, zerovec, zerovec, endpos, self, MASK_MONSTERSOLID,&trace);
//	trace = gi.trace(startpos, vec3_origin, vec3_origin, endpos, self, MASK_MONSTERSOLID);

	if(MG_OK_ToShoot(self, trace.ent))
		return true;

	return false;
}

void MG_MonsterFirePathTarget(edict_t *self, char *ptarg)
{
	edict_t	*ent;

	ent = NULL;

	while ((ent = G_Find(ent, FOFS(pathtargetname), ptarg)) != NULL)
	{
		if (ent->use)
			ent->use(ent, self, self);
	}
}

qboolean MG_MonsterAttemptTeleport(edict_t *self, vec3_t destination, qboolean ignoreLOS)
{
	qboolean no_teleport = false;
	trace_t trace;
	vec3_t	top, bottom, mins, maxs;
	edict_t	*ent = NULL;
	int		i;

	if(self->svflags & SVF_BOSS || self->classID == CID_OGLE)
		return false;

	if(self->classID != CID_ASSASSIN)
	{
		//if still SEE monsters cheat, re-enable the following 2 lines
		//if(skill->value < 2)
		//	return false;//only cheat on hard

		if(!ignoreLOS)
		{//check line of sight with all players
			ent = g_edicts;

			for(i = 0; i <= game.maxclients; i++)
			{
				ent = &g_edicts[i];

				if(ent->client)
				{
					edict_t	*temp;

					temp = G_Spawn();

					VectorSet(temp->s.origin,
						ent->client->playerinfo.pcmd.camera_vieworigin[0] * 0.125,
						ent->client->playerinfo.pcmd.camera_vieworigin[1] * 0.125,
						ent->client->playerinfo.pcmd.camera_vieworigin[2] * 0.125);

					if(gi.inPVS(temp->s.origin, destination))
					{
						no_teleport = true;
						G_FreeEdict(temp);
						break;
					}

					if(gi.inPVS(temp->s.origin, self->s.origin))
					{
						no_teleport = true;
						G_FreeEdict(temp);
						break;
					}

					G_FreeEdict(temp);
				}
			}
		}
	}

	if(!no_teleport)
	{//do traces
		VectorCopy(destination, bottom);
		bottom[2] -= self->size[2];
		
		VectorCopy(self->mins, mins);
		VectorCopy(self->maxs, maxs);
		mins[2] = 0;
		maxs[2] = 1;

		gi.trace(destination, mins, maxs, bottom, self, MASK_MONSTERSOLID, &trace);//self->clipmask

		if(trace.fraction<1.0)
		{
			VectorCopy(trace.endpos, bottom);
			VectorCopy(bottom, top);
			top[2] += self->size[2] - 1;
			gi.trace(bottom, mins, maxs, top, self, MASK_MONSTERSOLID, &trace);

			if(trace.allsolid || trace.startsolid)
				return false;

			if(trace.fraction == 1.0)
			{
				bottom[2] -= self->mins[2];

				if(self->classID == CID_ASSASSIN)
					assassinPrepareTeleportDest(self, bottom);
				else
				{
					VectorCopy(bottom, self->s.origin);
					gi.linkentity(self);
				}

				MG_RemoveBuoyEffects(self);
				self->lastbuoy = -1;
				return true;
			}
		}
		else if(!trace.allsolid && !trace.startsolid)
		{
			bottom[2] -= self->mins[2];
			if(self->classID == CID_ASSASSIN)
				assassinPrepareTeleportDest(self, bottom);
			else
			{
				VectorCopy(bottom, self->s.origin);
				gi.linkentity(self);
			}
			MG_RemoveBuoyEffects(self);
			self->lastbuoy = -1;
			return true;
		}
	}
	return false;
}

void MG_AddBuoyEffect(edict_t *self, qboolean endbuoy)
{
	if(BUOY_DEBUG)
	{//turn on sparkly effects on my start and end buoys
		if(!endbuoy)
		{//marking our next buoy
			if(self->nextbuoy[0])
			{//check a 10 second debouce timer
				if(!self->nextbuoy[0]->count)
				{
#ifdef _DEVEL
					gi.dprintf("Adding green effect to buoy %s\n", self->nextbuoy[0]->targetname);
#endif
					gi.CreateEffect(&self->nextbuoy[0]->s,
						FX_M_EFFECTS,
						CEF_OWNERS_ORIGIN|CEF_FLAG6,//green
						self->nextbuoy[0]->s.origin,
						"bv",
						FX_BUOY,
						vec3_origin);
				}
				self->nextbuoy[0]->s.frame = self->nextbuoy[0]->count++;
			}
		}
		else
		{//marking our end buoy (enemy's closest buoy)
			if(self->nextbuoy[1])
			{
				if(!self->nextbuoy[1]->s.frame)
				{
#ifdef _DEVEL
					gi.dprintf("Adding red effect to buoy %s\n", self->nextbuoy[1]->targetname);
#endif
					gi.CreateEffect(&self->nextbuoy[1]->s,
						FX_M_EFFECTS,
						CEF_OWNERS_ORIGIN,//red
						self->nextbuoy[1]->s.origin,
						"bv",
						FX_BUOY,
						vec3_origin);
				}
				self->nextbuoy[1]->s.frame = self->nextbuoy[1]->count++;
			}
		}
	}
}

void MG_RemoveBuoyEffects(edict_t *self)
{//fixme- because this is really using only one effect with flags, it's turning off both red and green when it turns either off...
	if(BUOY_DEBUG)
	{//turn off sparkly effects on my start and end buoys
		if(self->nextbuoy[0])
		{//reset a 10 second debouce timer
			if(self->nextbuoy[0]->count>1)
				self->nextbuoy[0]->count--;
			else
			{
				self->nextbuoy[0]->count = 0;
			}
			self->nextbuoy[0]->s.frame = self->nextbuoy[0]->count;
		}

		if(self->nextbuoy[1])
		{
			if(self->nextbuoy[1]->count>1)
				self->nextbuoy[1]->count--;
			else
			{
				self->nextbuoy[1]->count = 0;
			}
			self->nextbuoy[1]->s.frame = self->nextbuoy[1]->count;
		}
	}

}

//FIXME:  If a monster CAN see player but can't get to him for a short while and does not have a clear path to him, use the buoys anyway!
void MG_Pathfind(edict_t *self, qboolean check_clear_path)
{
	buoy_t		*current_buoy = NULL;
	buoy_t		*last_buoy = NULL;
	buoy_t		*jump_buoy = NULL;
	qboolean	clear_path = false;

	if(self->spawnflags & MSF_FIXED)
		return;

	if(!(self->monsterinfo.aiflags & AI_USING_BUOYS))
	{
		self->ai_mood = AI_MOOD_PURSUE;
		return;
	}

	if(DEACTIVATE_BUOYS)
	{
		self->monsterinfo.searchType = SEARCH_COMMON;
		self->ai_mood = AI_MOOD_PURSUE;
		return;
	}

	if (self->monsterinfo.searchType == SEARCH_COMMON)
	{
		if(!self->enemy)
			return;

		if(check_clear_path)
			clear_path = MG_CheckClearPathToEnemy(self);
		else
			clear_path = false;

		if(!clear_path)
		{//this sucks- why should I do this every time pathfind is called- I need to know if the monster can get to the player directly.. if so, no Makeconnection attempt, less traces
			if(!MG_MakeConnection(self, NULL, false))//if(!MG_MakeConnection(self, true, false))
			{
			}
		}
	}
	else if (self->monsterinfo.searchType == SEARCH_BUOY)
	{
		current_buoy = &level.buoy_list[self->buoy_index];
		last_buoy = &level.buoy_list[self->lastbuoy];

		if(self->ai_mood != AI_MOOD_FLEE && self->ai_mood != AI_MOOD_WANDER)
			self->ai_mood = AI_MOOD_NAVIGATE;

		if (self->ai_mood == AI_MOOD_DELAY)
			self->ai_mood = AI_MOOD_NAVIGATE;

		if (self->ai_mood == AI_MOOD_JUMP && self->groundentity)
			self->ai_mood = AI_MOOD_NAVIGATE;

		if (MG_ReachedBuoy(self, NULL))
		{
			MG_RemoveBuoyEffects(self);
#ifdef _DEVEL
			if (BUOY_DEBUG)
			{
				gi.dprintf("Reached goal %s\n", current_buoy->targetname);
			}
#endif
			//Check the possibility of activating something
			if ((current_buoy->modflags & BUOY_ACTIVATE) && (self->ai_mood != AI_MOOD_DELAY))
			{
#ifdef _DEVEL
				if (BUOY_DEBUG)
					gi.dprintf("Activating target %s\n", current_buoy->pathtarget);
#endif
				if (self->wait < level.time)
				{
					self->wait = level.time + current_buoy->wait;
					MG_MonsterFirePathTarget(self, current_buoy->pathtarget);
					if (current_buoy->delay)
					{
						QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
						self->ai_mood = AI_MOOD_DELAY;
						self->mood_nextthink = level.time + current_buoy->delay;
						return;
					}
				}
			}


			//if in AI_MOOD_FORCED_BUOY mode and this buoy is my forced_buoy,
			//take off that ai_mood flag and clear forced_buoy
			//also, if AI_MOOD_IGNORE_ENEMY flag, remove it
			if(self->ai_mood_flags&AI_MOOD_FLAG_FORCED_BUOY && self->forced_buoy == current_buoy->id)
			{
				self->forced_buoy = -1;
				self->ai_mood_flags &= ~AI_MOOD_FLAG_FORCED_BUOY;

				if(self->ai_mood_flags&AI_MOOD_FLAG_GOTO_STAND)
				{
					self->ai_mood_flags &= ~AI_MOOD_FLAG_GOTO_STAND;
					self->enemy = NULL;
					self->ai_mood = AI_MOOD_STAND;
					QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, "");
					return;
				}

				if(self->ai_mood_flags&AI_MOOD_FLAG_GOTO_WANDER)
				{
					self->ai_mood_flags &= ~AI_MOOD_FLAG_GOTO_WANDER;
					self->enemy = NULL;
					self->ai_mood = AI_MOOD_WANDER;
					return;
				}

				if(self->ai_mood_flags&AI_MOOD_FLAG_GOTO_FIXED)
				{
					self->ai_mood_flags &= ~AI_MOOD_FLAG_GOTO_FIXED;
					self->spawnflags |= MSF_FIXED;
					if(self->enemy)
						self->ai_mood = AI_MOOD_PURSUE;
					else
						self->ai_mood = AI_MOOD_STAND;
				}
		
				if(self->ai_mood != AI_MOOD_FLEE)
					self->ai_mood_flags &= ~AI_MOOD_FLAG_IGNORE_ENEMY;
				else
				{//reached buoy was fleeing to now what?
					if(MG_GoToRandomBuoy(self))
					{
						self->monsterinfo.searchType = SEARCH_BUOY;
						return;
					}
					else
					{//couldn't flee using buoys, use dumb fleeing
						//FIXME: cowering if can't flee using buoys?
						self->ai_mood_flags |= AI_MOOD_FLAG_DUMB_FLEE;
						return;
					}
				}

				if(!M_ValidTarget(self, self->enemy))
				{//got to where I was going, no enemy, so chill, baby.
					if (self->monsterinfo.pausetime == -1)
					{
						self->spawnflags |= MSF_WANDER;
						self->ai_mood = AI_MOOD_WANDER;
					}
					else if (level.time > self->monsterinfo.pausetime)
						self->ai_mood = AI_MOOD_WALK;
					else
					{
						self->ai_mood = AI_MOOD_STAND;
						QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, "");
					}
					return;
				}
			}

			if ((current_buoy->modflags & BUOY_JUMP) && (self->ai_mood != AI_MOOD_JUMP))
			{
				if(MG_MakeConnection(self, current_buoy, true))//make a regular connection, allowing skipping of jump_buoys
				{
					jump_buoy = current_buoy;
					current_buoy = &level.buoy_list[self->buoy_index];
					if(jump_buoy == current_buoy)
					{//Shit, found same buoy, shouldn't happen with dont_use_last = true! unless switching enemies
#ifdef _DEVEL
						if (BUOY_DEBUG)
							gi.dprintf("Warning: %s found same next buoy as last buoy at jump buoy\n",self->classname);
#endif
					}
					else if(current_buoy->id == jump_buoy->jump_target_id)
					{//go ahead and jump
						if(self->groundentity)
						{
							vec3_t	jumpangles, jumpfwd, jump_spot;

#ifdef _DEVEL
							if (BUOY_DEBUG)
								gi.dprintf("Jumping after buoy %s at angle %4.2f with height %4.2f and speed %4.2f\n", 
										current_buoy->targetname, jump_buoy->jump_yaw, jump_buoy->jump_uspeed, jump_buoy->jump_fspeed);
#endif
							VectorSet(jumpangles, 0, jump_buoy->jump_yaw, 0);
							AngleVectors(jumpangles, jumpfwd, NULL, NULL);
							
							//since we may not be right on the buoy, find out where they want us to go by extrapolating and finding MY dir to there
							VectorMA(jump_buoy->origin, jump_buoy->jump_fspeed, jumpfwd, jump_spot);
							VectorSubtract(jump_spot, self->s.origin, jumpfwd);
							jumpfwd[2] = 0;
							VectorNormalize(jumpfwd);

							VectorScale(jumpfwd, jump_buoy->jump_fspeed, self->movedir);
							self->movedir[2] = jump_buoy->jump_uspeed;
							self->ai_mood = AI_MOOD_JUMP;//don't technically need this line
							self->mood_nextthink = level.time + 0.5;
							//as an alternative, call self->forced_jump(self);
							QPostMessage(self, MSG_CHECK_MOOD, PRI_DIRECTIVE, "i", AI_MOOD_JUMP);
							return;
						}
					}
					else
					{//follow the new path
#ifdef _DEVEL
						if (BUOY_DEBUG)
						{
							current_buoy = &level.buoy_list[self->buoy_index];
							gi.dprintf("Heading to new goal %s\n", current_buoy->targetname);
						}
#endif
						return;
//WAS: oops, not the right one, set it back and search down below again
//						current_buoy = jump_buoy;
//						MG_AssignMonsterNextBuoy(self, current_buoy, NULL);
					}
				}
				else
				{
					return;//?
				}
			}

			if (!MG_MakeConnection(self, current_buoy, false))
			{
				return;//?
			}
			else
			{
#ifdef _DEVEL
				if (BUOY_DEBUG)
				{
					gi.dprintf("Heading to new goal %s\n", current_buoy->targetname);
				}
#endif
			}
		}
		
		if(self->last_buoy_time > 0 && self->last_buoy_time + BUOY_SEARCH_TIME < level.time)
		{
#ifdef _DEVEL
			if (BUOY_DEBUG)
			{
				gi.dprintf("Buoy search timed out trying to get to %s\n", current_buoy->targetname);
			}
#endif
			
			if(self->classID == CID_ASSASSIN)
			{
				if(MG_MonsterAttemptTeleport(self, current_buoy->origin, true))
				{
					self->monsterinfo.aiflags |= AI_OVERRIDE_GUIDE;
#ifdef _DEVEL
					if(BUOY_DEBUG)
						gi.dprintf("%s teleported to buoy %s (ignoring player LOS)\n", 
								self->classname, current_buoy->targetname);
#endif
					return;
				}
			}
			else if(CHEATING_MONSTERS)
			{
				if(CHEATING_MONSTERS<2)
				{
#ifdef _DEVEL
					if(MG_MonsterAttemptTeleport(self, current_buoy->origin, false))
						if(BUOY_DEBUG)
							gi.dprintf("%s cheated and teleported to buoy %s\n", 
									self->classname, current_buoy->targetname);
#else
					MG_MonsterAttemptTeleport(self, current_buoy->origin, false);
#endif
				}
				else
				{
#ifdef _DEVEL
					if(MG_MonsterAttemptTeleport(self, current_buoy->origin, true))
						if(BUOY_DEBUG)
							gi.dprintf("%s cheated and teleported to buoy %s (ignoring player LOS)\n", 
									self->classname, current_buoy->targetname);
#else
					MG_MonsterAttemptTeleport(self, current_buoy->origin, true);
#endif
				}
			}
			if (!MG_MakeConnection(self, NULL, false))
			{
			}
		}
		else if(!irand(0, 4) && !clear_visible_pos(self, current_buoy->origin))
		{//DAMN!  Lost sight of buoy, let's re-aquire
#ifdef _DEVEL
			if (BUOY_DEBUG)
				gi.dprintf("%s Lost sight of buoy %s looking for another...\n", 
						self->classname, current_buoy->targetname);
#endif			
			if (!MG_MakeConnection(self, NULL, false))
			{
			}
		}
	}
}

/* 
 *
 *
 *
 *
 *
 *
 *	Guide functions
 *
 *
 *
 *
 *
 *
 */

void MG_BuoyNavigate(edict_t *self)
{//Only handles buoy selection, some mood changing
//called from ai_run
	qboolean	valid_enemy = false;
	int			i;
	buoy_t		*found_buoy = NULL;
	qboolean	found;

	//See if my enemy is still valid
	valid_enemy = M_ValidTarget(self, self->enemy);

	if(self->spawnflags & MSF_FIXED)
		return;

	if(!(self->monsterinfo.aiflags & AI_USING_BUOYS))
	{
		self->ai_mood = AI_MOOD_PURSUE;//ai_mood_normal?
		return;
	}
	
//STEP 1: See if should be running away or wandering

	if(self->monsterinfo.flee_finished < level.time)
		self->monsterinfo.aiflags &= ~AI_FLEE;//clear the flee flag now

	if(self->monsterinfo.aiflags & AI_COWARD || 
		(self->monsterinfo.aiflags&AI_FLEE && self->monsterinfo.flee_finished >= level.time))
		self->ai_mood = AI_MOOD_FLEE;

	if(!valid_enemy)
	{//no enemy, now what?
		self->enemy = NULL;
		if(self->spawnflags & MSF_WANDER || self->monsterinfo.pausetime == -1)
		{
			self->spawnflags |= MSF_WANDER;
			self->ai_mood = AI_MOOD_WANDER;
		}
		else if (level.time > self->monsterinfo.pausetime)
			self->ai_mood = AI_MOOD_WALK;
		else
			self->ai_mood = AI_MOOD_STAND;
	}
	else
	{
		if(self->ai_mood == AI_MOOD_WANDER)
			self->ai_mood = AI_MOOD_PURSUE;
	}

	if(self->ai_mood == AI_MOOD_FLEE || self->ai_mood == AI_MOOD_WANDER)
	{//go off in a random buoy path

		if(!(self->ai_mood_flags&AI_MOOD_FLAG_FORCED_BUOY))
		{//first time, find closest buoy, alert other enemies
			if(self->ai_mood == AI_MOOD_FLEE)
			{//wake up enemies for next 10 seconds
				level.sight_entity = self;
				level.sight_entity_framenum = level.framenum + 100;
				level.sight_entity->light_level = 128;
			}

			if(MG_GoToRandomBuoy(self))
			{
				self->monsterinfo.searchType = SEARCH_BUOY;
				return;
			}
			else if(self->ai_mood == AI_MOOD_FLEE)
			{//couldn't flee using buoys, use dumb fleeing
				//FIXME: cowering if can't flee using buoys?
				self->ai_mood_flags |= AI_MOOD_FLAG_DUMB_FLEE;
				return;

			}
				//otherwise, want to wander, but can't, continue down the possibilities
		}
		else
		{
			self->monsterinfo.searchType = SEARCH_BUOY;
			MG_Pathfind(self, false);
			return;//already wandering normal buoy navigation
		}
	}
//STEP 2: Not running away or wandering, see what should be doing

	if(!valid_enemy)
	{//No enemy, Not wandering or can't, see if I have a homebuoy
		if(self->homebuoy)
		{//have a home base, let's get back there if no enemy
			for(i = 0; i <= level.active_buoys; i++)
			{
				found_buoy = &level.buoy_list[i];
				if(found_buoy->targetname && !stricmp(found_buoy->targetname, self->homebuoy))
				{
					found = true;
					break;
				}
			}

			if(!found)
			{
#ifdef _DEVEL
				if(BUOY_DEBUG)
					gi.dprintf("ERROR: %s can't find it's homebuoy %s\n", self->classname, self->homebuoy);
#endif
				return;
			}

			if(!MG_ReachedBuoy(self, found_buoy->origin))
			{
#ifdef _DEVEL
				if(BUOY_DEBUG)
					gi.dprintf("%s heading for homebuoy %s\n", self->classname, self->homebuoy);
#endif
				self->ai_mood_flags|=AI_MOOD_FLAG_FORCED_BUOY;
				self->forced_buoy = found_buoy->id;

				if(MG_MakeConnection(self, NULL, false))
				{
					self->ai_mood = AI_MOOD_NAVIGATE;

					QPostMessage(self, MSG_WALK,PRI_DIRECTIVE, NULL);

					MG_RemoveBuoyEffects(self);
				}
				else
				{
					self->ai_mood_flags &= ~AI_MOOD_FLAG_FORCED_BUOY;
					self->forced_buoy = -1;
				}
			}
		}
//No enemy Not wandering, not going to homebuoy (or can't do these for some reason), so just stand around
		//do we really need to clear the enemy?  mAybe we shouldn't...
		if(self->enemy)
		{
			if(self->enemy->client)
				self->oldenemy = self->enemy;//remember last player enemy
		}
		self->enemy = NULL;

		self->mood_nextthink = level.time + 1;
		//fixme: check for a self->target also?
		if (self->monsterinfo.pausetime == -1)
		{
			self->spawnflags |= MSF_WANDER;
			self->ai_mood = AI_MOOD_WANDER;
		}
		else if (level.time > self->monsterinfo.pausetime)
			self->ai_mood = AI_MOOD_WALK;
		else
			self->ai_mood = AI_MOOD_STAND;
		return;
	}
	else if(self->ai_mood_flags&AI_MOOD_FLAG_IGNORE_ENEMY)
	{//have an enemy, but being forced to use buoys, and ignore enemy until get to forced_buoy
		self->ai_mood = AI_MOOD_NAVIGATE;
		MG_Pathfind(self, false);
		return;
	}

//Actually have a valid enemy. let's try to get him
	self->ai_mood = AI_MOOD_PURSUE;

	MG_Pathfind(self, true);
	if(self->ai_mood == AI_MOOD_PURSUE)
		self->goalentity = self->enemy;
}

void Cvar_SetValue (char *var_name, float value);
void MG_GenericMoodSet(edict_t *self)
{
	vec3_t		v, forward, pursue_vel;
	float		enemydist;
	qboolean	coward = false;
	qboolean	can_attack_ranged = false;
	qboolean	can_attack_close = false;
	qboolean	clear_shot = false;
	qboolean	enemyvis = false;
	qboolean	enemyinfront = false;
	qboolean	found = false;
	qboolean	melee_go = false;
	int			i;
	buoy_t		*found_buoy;
	qboolean	valid_enemy = false;

	if(!level.active_buoys)
	{
		if(!DEACTIVATE_BUOYS)
		{
			gi.dprintf("WARNING: no buoys on this map!!!\n");
			if(!level.active_buoys)
			{//1st buoy, initialize a couple arrays
				for(i = 0; i < MAX_CLIENTS; i++)
				{
					level.player_buoy[i] = NULL_BUOY;				//stores current bestbuoy for a player enemy (if any)
					level.player_last_buoy[i] = NULL_BUOY;		//when player_buoy is invalid, saves it here so monsters can check it first instead of having to do a whole search
				}
			}
			Cvar_SetValue("deactivate_buoys", 1);
			DEACTIVATE_BUOYS = true;
		}
	}

	if(self->mood_nextthink > level.time || self->mood_nextthink <= 0.0f)
		return;

	//See if my enemy is still valid
	valid_enemy = M_ValidTarget(self, self->enemy);

	if(!(self->monsterinfo.aiflags & AI_USING_BUOYS))
	{//skip buoy stuff
		self->ai_mood = AI_MOOD_PURSUE;
	}
	else
	{//use buoys
//STEP 1: See if should be running away or wandering
		if(self->monsterinfo.flee_finished < level.time)
			self->monsterinfo.aiflags &= ~AI_FLEE;//clear the flee flag now

		if(self->monsterinfo.aiflags & AI_COWARD || 
			(self->monsterinfo.aiflags&AI_FLEE && self->monsterinfo.flee_finished >= level.time))
			self->ai_mood = AI_MOOD_FLEE;

		if(!valid_enemy)
		{//no enemy, now what?
			self->enemy = NULL;
			
			if(self->spawnflags & MSF_FIXED)
				return;

			if(self->spawnflags & MSF_WANDER || self->monsterinfo.pausetime == -1)
			{
				self->spawnflags |= MSF_WANDER;
				self->ai_mood = AI_MOOD_WANDER;
			}
			else if (level.time > self->monsterinfo.pausetime)
				self->ai_mood = AI_MOOD_WALK;
			else
				self->ai_mood = AI_MOOD_STAND;
		}
		else
		{
			if(self->spawnflags & MSF_FIXED)
				goto checkattacks;

			if(self->ai_mood == AI_MOOD_WANDER)
				self->ai_mood = AI_MOOD_PURSUE;
		}

		if(self->ai_mood == AI_MOOD_FLEE || self->ai_mood == AI_MOOD_WANDER)
		{//go off in a random buoy path
			if(!(self->ai_mood_flags&AI_MOOD_FLAG_FORCED_BUOY))
			{//first time, find closest buoy, alert other enemies
				if(self->ai_mood == AI_MOOD_FLEE)
				{//wake up enemies for next 10 seconds
					level.sight_entity = self;
					level.sight_entity_framenum = level.framenum + 100;
					level.sight_entity->light_level = 128;
				}

				if(MG_GoToRandomBuoy(self))
				{
					self->monsterinfo.searchType = SEARCH_BUOY;
					return;
				}
				else if(self->ai_mood == AI_MOOD_FLEE)
				{//couldn't flee using buoys, use dumb fleeing
					//FIXME: cowering if can't flee using buoys?
					self->ai_mood_flags |= AI_MOOD_FLAG_DUMB_FLEE;
					return;
				}
				//otherwise, want to wander, but can't, continue down the possibilities
			}
			else
			{
				self->monsterinfo.searchType = SEARCH_BUOY;
				MG_Pathfind(self, false);
				return;//already wandering normal buoy navigation
			}
		}
//STEP 2: Not running away or wandering, see what should be doing

		if(!valid_enemy)
		{//No enemy, Not wandering or can't, see if I have a homebuoy
			if(self->homebuoy)
			{//have a home base, let's get back there if no enemy
				for(i = 0; i <= level.active_buoys; i++)
				{
					found_buoy = &level.buoy_list[i];
					if(found_buoy->targetname && !stricmp(found_buoy->targetname, self->homebuoy))
					{
						found = true;
						break;
					}
				}

////////////////////////////  BUGBUGBUGBUG 7 lines above, targetname can be NULL if you just happen to die.

				if(!found)
				{
#ifdef _DEVEL
					if(BUOY_DEBUG)
						gi.dprintf("ERROR: %s can't find it's homebuoy %s\n", self->classname, self->homebuoy);
#endif
					return;
				}

				if(!MG_ReachedBuoy(self, found_buoy->origin))
				{
#ifdef _DEVEL
					if(BUOY_DEBUG)
						gi.dprintf("%s heading for homebuoy %s\n", self->classname, self->homebuoy);
#endif

					self->ai_mood_flags|=AI_MOOD_FLAG_FORCED_BUOY;
					self->forced_buoy = found_buoy->id;

					if(MG_MakeConnection(self, NULL, false))
					{
						self->ai_mood = AI_MOOD_NAVIGATE;

						QPostMessage(self, MSG_WALK,PRI_DIRECTIVE, NULL);

						MG_RemoveBuoyEffects(self);
					}
					else
					{
						self->ai_mood_flags &= ~AI_MOOD_FLAG_FORCED_BUOY;
						self->forced_buoy = -1;
					}
				}
			}
//No enemy Not wandering, not going to homebuoy (or can't do these for some reason), so just stand around
			//do we really need to clear the enemy?  mAybe we shouldn't...
			if(self->enemy)
			{
				if(self->enemy->client)
					self->oldenemy = self->enemy;//remember last player enemy
			}
			self->enemy = NULL;

			self->mood_nextthink = level.time + 1;
			//fixme: check for a self->target also?
			if (self->monsterinfo.pausetime == -1)
			{
				self->spawnflags |= MSF_WANDER;
				self->ai_mood = AI_MOOD_WANDER;
			}
			else if (level.time > self->monsterinfo.pausetime)
				self->ai_mood = AI_MOOD_WALK;
			else
				self->ai_mood = AI_MOOD_STAND;
			return;
		}
		else if(self->ai_mood_flags&AI_MOOD_FLAG_IGNORE_ENEMY)
		{//have an enemy, but being forced to use buoys, and ignore enemy until get to forced_buoy
			self->ai_mood = AI_MOOD_NAVIGATE;
			MG_Pathfind(self, false);
			return;
		}
	}

	if(!valid_enemy || !self->enemy)
	{
		if (self->monsterinfo.aiflags & AI_EATING)
			self->ai_mood = AI_MOOD_EAT;
		return;
	}
	
//STEP 3: OK, have a valid enemy, let's go get him!
checkattacks:
	self->ai_mood = AI_MOOD_PURSUE;

	//get distance to target, ignore Z diff if close
	VectorSubtract (self->s.origin, self->enemy->s.origin, v);
	if (v[2] <= 40)
		v[2] = 0;
	enemydist = VectorLength(v) - self->enemy->maxs[0];

	if ((self->monsterinfo.aiflags & AI_EATING) && (enemydist > self->wakeup_distance) && !self->monsterinfo.awake)
	{
		self->monsterinfo.last_successful_enemy_tracking_time = level.time;
		self->ai_mood = AI_MOOD_EAT;
		return;
	}

	if(self->monsterinfo.aiflags & AI_NO_MISSILE)
		self->spawnflags &= ~MSF_FIXED;//don't stand around if can't fire

	if(self->attack_debounce_time > level.time || self->monsterinfo.attack_finished > level.time)
	{//can't attack yet
		can_attack_ranged = false;
		clear_shot = false;
		can_attack_close = false;
	}
	else
	{
		if(classStatics[self->classID].msgReceivers[MSG_MISSILE] && !(self->monsterinfo.aiflags&AI_NO_MISSILE))
		{
			can_attack_ranged = true;
			clear_shot = MG_CheckClearShotToEnemy(self);
		}
		else
			clear_shot = false;
		
		if(classStatics[self->classID].msgReceivers[MSG_MELEE] && !(self->monsterinfo.aiflags&AI_NO_MELEE))
			can_attack_close = true;
	}

	if(enemyvis = visible(self, self->enemy))
	{
		self->ai_mood_flags &= ~AIMF_CANT_FIND_ENEMY;
		self->ai_mood_flags &= ~AIMF_SEARCHING;
		self->monsterinfo.last_successful_enemy_tracking_time = level.time;
		
		if(self->ai_mood_flags&AI_MOOD_FLAG_BACKSTAB)
		{//only approach and attack the enemy's back- be sure to take this off if hurt?
			if(enemydist < 128)
			{
				self->ai_mood_flags &= ~AI_MOOD_FLAG_BACKSTAB;
			}
			else if(infront(self->enemy, self))
			{
				self->ai_mood = AI_MOOD_DELAY;
				return;
			}
		}
	}
	enemyinfront = infront(self, self->enemy);

//HEY! What if too close- backpedal or flee for a bit?
//Also, need a chance of closing in anyway- a bypass_missile_chance?
	if(enemyvis && enemyinfront &&
		can_attack_ranged &&
		clear_shot &&
		enemydist <= self->missile_range)
	{//are they far enough away?
		if(irand(0, 100)>self->bypass_missile_chance)
		{
			if(enemydist >= self->min_missile_range)
			{//ranged attack!
				self->ai_mood = AI_MOOD_ATTACK;
				self->ai_mood_flags &= ~AI_MOOD_FLAG_MELEE;
				self->ai_mood_flags |= AI_MOOD_FLAG_MISSILE;
				self->attack_debounce_time = level.time + (3 - skill->value)/2;
				return;
			}
			else if(!can_attack_close)
			{//too close and can't melee!
				goto enemy_too_close;
			}
		}
	}

//otherwise, close in
	if (!MG_CheckClearPathToEnemy(self)&&self->classID!=CID_TBEAST)
	{//can't directly approach enemy
//		if(enemyinfront)//ie, failed to shoot for some other reason
//		{//can't shoot enemy, use buoys to get there
			MG_Pathfind(self, false);//false means don't do a mg_checkclearpath
			if(self->ai_mood == AI_MOOD_PURSUE)
				self->goalentity = self->enemy;
			if(self->cant_attack_think)
				self->cant_attack_think(self, enemydist, enemyvis, enemyinfront);
			return;
//		}
	}

//use dummy AI
	self->monsterinfo.searchType = SEARCH_COMMON;
	self->movetarget = self->goalentity = self->enemy;

//can directly approach player

	if(self->melee_range < 0)
	{//keep a distance
		if(enemydist <= -self->melee_range)
		{//hang back and wait until CAN fire a shot off
			goto enemy_too_close;
		}//else close in
	}

//check for melee range attack

	if(can_attack_close)
	{
		if(!enemyvis || !enemyinfront || enemydist > self->melee_range || enemydist < self->min_melee_range)
		{
			AngleVectors(self->s.angles, forward, NULL, NULL);
			VectorSubtract(self->s.origin, self->s.old_origin, pursue_vel);
			melee_go  = M_PredictTargetEvasion(self, self->enemy, pursue_vel, self->enemy->velocity, self->melee_range, 5);//predict for next half second
		}
		else
			melee_go = true;
	}

	if(melee_go)
	{//Close enough to melee
		self->ai_mood = AI_MOOD_ATTACK;
		self->ai_mood_flags |= AI_MOOD_FLAG_MELEE;
		self->ai_mood_flags &= ~AI_MOOD_FLAG_MISSILE;
		self->attack_debounce_time = level.time + (3 - skill->value)/2;
		//OR: ok to missile too?
		return;
	}
	else if(enemydist < self->min_melee_range)
	{
		goto enemy_too_close;
		return;
	}

//Can't melee, so just run blindly
	self->ai_mood = AI_MOOD_PURSUE;
	if(self->cant_attack_think)
		self->cant_attack_think(self, enemydist, enemyvis, enemyinfront);
	return;

enemy_too_close:
	if(classStatics[self->classID].msgReceivers[MSG_FALLBACK])
	{//what if I hit a wall?  Go into attack anyway?
		self->ai_mood = AI_MOOD_BACKUP;//walk back while firing
	}
	else//maybe turn and run for a bit?
	{
#ifdef _DEVEL
		if(MGAI_DEBUG)
			gi.dprintf("%s running away to get some distance from %s\n", self->classname, self->enemy->classname);
#endif
		self->monsterinfo.aiflags |= AI_FLEE;
		self->monsterinfo.flee_finished = level.time + flrand(3, 6);
		//self->ai_mood = AI_MOOD_DELAY;//fixme: this is not good!
	}
	return;
}

void MG_InitMoods(edict_t *self)
{
	self->monsterinfo.searchType = SEARCH_COMMON;

	if(!self->mintel)
		self->mintel = MaxBuoysForClass[self->classID];

	self->mood_think = MG_GenericMoodSet;//we'll re-specialize these soon
	self->mood_nextthink = level.time + 0.1;
	if(self->mood_nextthink <= 0.0f)
		self->mood_nextthink = 0.1;

	//setup attack ranges for the mood functions to use
	//these can be set by the designer if desired and can be
	//affected later by the loss of a weapon or limb...
	if(!self->min_melee_range)
		self->min_melee_range = 0;//rendundant, I know, but clearer to see it here with other stuff

	if(!self->melee_range)
		self->melee_range = AttackRangesForClass[self->classID * 4 + 0];

	if(!self->missile_range)
		self->missile_range = AttackRangesForClass[self->classID * 4 + 1];
	
	if(!self->min_missile_range)
		self->min_missile_range = AttackRangesForClass[self->classID * 4 + 2];

	if(!self->bypass_missile_chance)
		self->bypass_missile_chance = AttackRangesForClass[self->classID * 4 + 3];

	if(!self->jump_chance)
		self->jump_chance = JumpChanceForClass[self->classID];

	if(!self->wakeup_distance)
		self->wakeup_distance = MAX_SIGHT_PLAYER_DIST;

	//so ai_run knows to call MG_BuoyNavigate...
	if(self->mintel > 0)
		self->monsterinfo.aiflags |= AI_USING_BUOYS;

	if(!skill->value)//no skill = 1/2 health monsters
		self->max_health = self->health = self->health * 0.5;

	self->lastbuoy = NULL_BUOY;
}


