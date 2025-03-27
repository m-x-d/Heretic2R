//
// g_ai.c
//
// Copyright 1998 Raven Software
//

#include "g_ai.h" //mxd
#include "g_ai_local.h" //mxd
#include "mg_ai.h" //mxd
#include "mg_guide.h"
#include "m_stats.h"
#include "p_anims.h"
#include "Random.h"
#include "Vector.h"

void SV_NewChaseDir(edict_t* actor, edict_t* enemy, float dist); //TODO: add to m_move.h
void ssithraCheckJump(edict_t* self); //TODO: add to m_plagueSsithra.h

// AI targeting globals.
static int enemy_range; // Range from enemy RANGE_MELEE, RANGE_NEAR, RANGE_MID, RANGE_FAR.
static float enemy_yaw; // Ideal yaw to face enemy.

// Called once each frame to set level.sight_client to the player to be checked for in findtarget.
// If all clients are either dead or in notarget, sight_client will be null.
// In coop games, sight_client will cycle between the clients.
void AI_SetSightClient(void)
{
	const int start = ((level.sight_client == NULL) ? 1 : level.sight_client - g_edicts);
	int check = start;

	while (true)
	{
		if (++check > game.maxclients)
			check = 1;

		edict_t* ent = &g_edicts[check];

		if (ent->inuse && ent->health > 0 && !(ent->flags & FL_NOTARGET))
		{
			level.sight_client = ent;
			return; // Got one.
		}

		if (check == start)
		{
			level.sight_client = NULL;
			return; // Nobody to see.
		}
	}
}

// Monster will eat until harm is done to him or the player moves within range.
void ai_eat(edict_t* self, float dist) //TODO: rename to AI_Eat?
{
	self->enemy = NULL;
	FindTarget(self);
}

// Move the specified distance at current facing.
void ai_move(edict_t* self, const float dist)
{
	MG_WalkMove(self, self->s.angles[YAW], dist);
}

// Used for standing around and looking for players. Distance is for slight position adjustments needed by the animations.
void ai_stand(edict_t* self, const float dist) //mxd. 'dist' is always 0.
{
	if (dist != 0.0f)
		M_walkmove(self, self->s.angles[YAW], dist);

	if (self->enemy != NULL) //TODO: move below AI_STAND_GROUND check?
		return;

	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
	{
		if (self->enemy != NULL) //TODO: never executed: already returned if have enemy!
		{
			vec3_t diff;
			VectorSubtract(self->enemy->s.origin, self->s.origin, diff);
			self->ideal_yaw = VectorYaw(diff);

			if (self->s.angles[YAW] != self->ideal_yaw && (self->monsterinfo.aiflags & AI_TEMP_STAND_GROUND))
			{
				self->monsterinfo.aiflags &= ~(AI_STAND_GROUND | AI_TEMP_STAND_GROUND);
				QPostMessage(self, MSG_RUN, PRI_DIRECTIVE, NULL);
			}

			M_ChangeYaw(self);
			ai_checkattack(self, 0.0f);
		}
		else
		{
			FindTarget(self);
		}

		return;
	}

	if (FindTarget(self))
		return;

	//FIXME: 'walking a beat' monsters, but that may not be working, need to test! Check for target also?
	if (self->monsterinfo.pausetime == -1.0f)
	{
		self->spawnflags |= MSF_WANDER;
		self->ai_mood = AI_MOOD_WANDER;
		QPostMessage(self, MSG_CHECK_MOOD, PRI_DIRECTIVE, "i", AI_MOOD_WANDER);

		return;
	}

	if (level.time > self->monsterinfo.pausetime)
	{
		self->ai_mood = AI_MOOD_WALK;
		QPostMessage(self, MSG_WALK, PRI_DIRECTIVE, NULL);

		return;
	}

	if (!(self->spawnflags & MSF_AMBUSH) && self->monsterinfo.idle != NULL && level.time > self->monsterinfo.idle_time)
	{
		if (self->monsterinfo.idle_time > 0.0f)
		{
			self->monsterinfo.idle(self);
			self->monsterinfo.idle_time = level.time + flrand(15.0f, 30.0f);
		}
		else
		{
			self->monsterinfo.idle_time = level.time + flrand(0.0f, 15.0f);
		}
	}
}

// The monster is walking it's beat.
void ai_walk(edict_t* self, const float dist)
{
	if (FindTarget(self))
	{
		QPostMessage(self, MSG_RUN, PRI_DIRECTIVE, NULL);
		return;
	}

	if (self->monsterinfo.searchType == SEARCH_BUOY)
	{
		if (!(self->monsterinfo.aiflags & AI_STRAIGHT_TO_ENEMY))
		{
			MG_BuoyNavigate(self); // Only called if already wandering.
			MG_Pathfind(self, false);
		}
	}
	else if (self->enemy == NULL && self->pathtarget == NULL)
	{
		if (self->movetarget != NULL)
		{
			if (self->movetarget->classname != NULL && strcmp(self->movetarget->classname, "path_corner") != 0 && MG_ReachedBuoy(self, self->movetarget->s.origin))
			{
				self->movetarget = NULL;
				QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
			}
		}
		else if (irand(0, 30) == 0 || (Vec3NotZero(self->monsterinfo.nav_goal) && MG_ReachedBuoy(self, self->monsterinfo.nav_goal)))
		{
			self->monsterinfo.pausetime = 0.0f;
			QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
		}
	}

	MG_MoveToGoal(self, dist);
}

/*
=============
ai_charge

Turns towards enemy and advances
Use this call with a distnace of 0 to replace ai_face
==============
*/
void ai_charge (edict_t *self, float dist)
{
	vec3_t	v;

	if(!self->enemy)
	{
		if(MGAI_DEBUG)
			gi.dprintf("ERROR: AI_CHARGE at a NULL enemy!\n");
		MG_FaceGoal(self, true);//get ideal yaw and turn
		return;//send stand MSG?
	}

	VectorSubtract (self->enemy->s.origin, self->s.origin, v);

	self->ideal_yaw = VectorYaw(v);

	MG_ChangeYaw (self);

	if(self->spawnflags&MSF_FIXED)
		return;

	if (dist)
		MG_WalkMove (self, self->s.angles[YAW], dist);
}

/*
=============
ai_moveright

simple sliding right (or left if dist <0)
==============
*/
void ai_moveright (edict_t *self, float dist)
{
	vec3_t	right;
	float	movedir;

	if(self->spawnflags&MSF_FIXED)
		return;

	if (!dist)
		return;

	AngleVectors (self->s.angles, NULL, right, NULL);

	movedir = VectorYaw(right);

	MG_WalkMove (self, movedir, dist);
}

/*
=============
ai_goal_charge

Turns towards target and advances
Use this call with a distnace of 0 to replace ai_face
==============
*/
void ai_goal_charge (edict_t *self, float dist)
{
	MG_FaceGoal(self, true);

	if(self->spawnflags&MSF_FIXED)
		return;

	if (dist)
		MG_WalkMove (self, self->s.angles[YAW], dist);
}

/*
=============
ai_charge2

Turns towards target and advances
Use this call with a distnace of 0 to replace ai_face
==============
*/
void ai_charge2 (edict_t *self, float dist)
{
	vec3_t	v;

	if(!self->enemy)
		return;

	VectorSubtract (self->enemy->s.origin, self->s.origin, v);
	self->ideal_yaw = VectorYaw(v);
	MG_ChangeYaw (self);

	if(self->spawnflags&MSF_FIXED)
		return;

	if (dist)
		MG_WalkMove (self, self->s.angles[YAW], dist);
}

/*
=============
ai_turn

don't move, but turn towards ideal_yaw
Distance is for slight position adjustments needed by the animations
=============
*/
void ai_turn (edict_t *self, float dist)
{
	if(!(self->spawnflags&MSF_FIXED))
		if (dist)
			M_walkmove (self, self->s.angles[YAW], dist);

	if (FindTarget (self))
		return;
	
	M_ChangeYaw (self);
}


/*

.enemy
Will be world if not currently angry at anyone.

.movetarget
The next path spot to walk toward.  If .enemy, ignore .movetarget.
When an enemy is killed, the monster will try to return to it's path.

.hunt_time
Set to time + something when the player is in sight, but movement straight for
him is blocked.  This causes the monster to use wall following code for
movement direction instead of sighting on the player.

.ideal_yaw
A yaw angle of the intended direction, which will be turned towards at up
to 45 deg / state.  If the enemy is in view and hunt_time is not active,
this will be the exact line towards the enemy.

.pausetime
A monster will leave it's stand state and head towards it's .movetarget when
time > .pausetime.

walkmove(angle, speed) primitive is all or nothing
*/

int categorize_range (edict_t *self, edict_t *other, float len)
{
	float	dist;

	if (self->monsterinfo.aiflags & AI_EATING) // Eating
	{
		if (len < MELEE_DISTANCE)	// Melee distance
		{
			if ( len < (self->maxs[0] + other->maxs[0] + 15))
				return RANGE_MELEE;
		}

		if (len < 175)				// Attacking distance
			return RANGE_NEAR;
		if (len < 350)				// Hissing distance
			return RANGE_MID;
		return RANGE_FAR;
	}
	else				// Not eating
	{
		if (len < MELEE_DISTANCE)
		{
			if ( len < (self->maxs[0] + other->maxs[0] + 25))
				return RANGE_MELEE;
		}
		if (len < 500)
			return RANGE_NEAR;

		if(self->wakeup_distance)
			dist = self->wakeup_distance;
		else
			dist = MAX_SIGHT_PLAYER_DIST;

		if (len < dist)
			return RANGE_MID;
		return RANGE_FAR;
	}
}
/*
=============
range

returns the range catagorization of an entity reletive to self
0	melee range, will become hostile even if back is turned
1	visibility and infront, or visibility and show hostile
2	infront and show hostile
3	only triggered by damage
=============
*/
int range (edict_t *self, edict_t *other)
{
	vec3_t	v;
	float	len;

	VectorSubtract (self->s.origin, other->s.origin, v);
	len = VectorLength (v);
	
	return categorize_range(self, other, len);
}

/*
=============
visible

returns 1 if the entity is visible to self, even if not infront ()
=============
*/
qboolean visible (edict_t *self, edict_t *other)
{
	vec3_t	spot1;
	vec3_t	spot2;
	trace_t	trace;

	if (!other)
		return false;

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
	VectorCopy (other->s.origin, spot2);
	spot2[2] += other->viewheight;
	gi.trace (spot1, vec3_origin, vec3_origin, spot2, self, MASK_OPAQUE,&trace);
	
	if (trace.fraction == 1.0)
		return true;
	return false;
}

/*
=============
clear_visible

returns 1 if the entity is visible, but not through transparencies
=============
*/
qboolean clear_visible (edict_t *self, edict_t *other)
{
	vec3_t	spot1;
	vec3_t	spot2;
	trace_t	trace;

	if (!other)
		return false;

	if (!self)
		return false;

	VectorCopy (self->s.origin, spot1);
	spot1[2] += self->viewheight;
	VectorCopy (other->s.origin, spot2);
	spot2[2] += other->viewheight;
	gi.trace (spot1, vec3_origin, vec3_origin, spot2, self, MASK_SOLID,&trace);
	
	if (trace.fraction == 1.0)
		return true;
	return false;
}

/*
=============
infront

returns 1 if the entity is in front (in sight) of self
=============
*/
qboolean infront (edict_t *self, edict_t *other)
{
	vec3_t	vec;
	float	dot;
	vec3_t	forward, check_angles;
	
	if(Vec3NotZero(self->v_angle_ofs))
		VectorAdd(self->v_angle_ofs,self->s.angles,check_angles);
	else
		VectorCopy(self->s.angles,check_angles);
	
	AngleVectors (check_angles, forward, NULL, NULL);
	VectorSubtract (other->s.origin, self->s.origin, vec);
	VectorNormalize (vec);
	dot = DotProduct (vec, forward);
	
	if (dot > 0.3)
		return true;
	return false;
}


//============================================================================

/*
Alerted

Checks and see if an alert entity is capable of waking up a monster
*/
void alert_timed_out(alertent_t *self);
qboolean Alerted (edict_t *self)
{//This alert entity wakes up monsters, let's see what's up...
	edict_t		*enemy = NULL;
	alertent_t	*alerter;
	vec3_t		e_spot, viewspot, dir;
	qboolean	saw_it = false;
	float		dist;
	
	if(self->monsterinfo.aiflags&AI_NO_ALERT)
		return false;

	//start the search from the most recent alert to the oldest
	alerter = level.last_alert;//OOPS, SKIPS LAST
	
alertloop:

	if(self->enemy)
		goto loopagain;
	
	//alerter is gone
	if(!alerter)
		goto loopagain;

	if(!alerter->inuse)//loading a saved game invalidates all alerts
		goto loopagain;

	if(alerter->lifetime < level.time)
	{//alert timed out, remove from list
		alert_timed_out(alerter);
		goto loopagain;
	}

	if(self->last_alert)//don't be woken up by the same alert twice
	{
		if(self->last_alert->inuse)
		{
			if(alerter == self->last_alert && self->last_alert->inuse)
				goto loopagain;
		}
		else
			self->last_alert = NULL;
	}

	//alerter's enemy is gone
	if(!alerter->enemy)
		goto loopagain;

	if(alerter->enemy->client)
	{//no alerts for notarget players
		if(alerter->enemy->flags & FL_NOTARGET)
			goto loopagain;
	}

	//NEVER alert ambush monsters?
	//if(!(self->spawnflags & MSF_AMBUSH))
	//	goto loopagain;
	 
	 if(!(self->svflags&SVF_MONSTER))
		goto loopagain;
	
	if(self->health<=0)
		goto loopagain;

	//eating or in a cinematic or not awake, leave them alone
	if(!ok_to_wake(self, false, true))
		goto loopagain;
		
	//should we keep track of owner in case they move to move the alert with them?  Only for monsters
	//if(alerter->owner)
	//	VectorCopy(alerter->owner->s.origin, alerter->s.origin);

	VectorSubtract(self->s.origin, alerter->origin, dir);
	dist = VectorLength(dir);

	//if monster's wakeup_distance is shorter than dist to alerter, leave it alone
	if(dist > self->wakeup_distance)
		goto loopagain;

	//closer means better chance to alert
	//problem - different alerts might be more likely to be heard/seen...
	if(dist > flrand(100, self->wakeup_distance))//if within 100 always wake up?
		goto loopagain;
	
	//if not a player, a player's missile or a monster, goto loopagain?
	//get center of alert enemy
	enemy = alerter->enemy;

	VectorAdd(enemy->s.origin, enemy->mins, e_spot);
	VectorMA(e_spot, 0.5, enemy->size, e_spot);

	//get my view spot
	VectorCopy(self->s.origin, viewspot);
	viewspot[2] += self->viewheight;
	
	//if being alerted by a monster and not waiting to ambush
	if(alerter->alert_svflags&SVF_MONSTER && !(self->spawnflags&MSF_AMBUSH))
	{//can "see" the owner of the alert even around a corner
		saw_it = gi.inPVS(e_spot, viewspot);
	}
	else
	{//alerter not a monster, a projectile or player
		//can I see (even through translucent brushes) the owner of the alert?
		//if so, ok even if I'm an ambush monster
		saw_it = MG_IsVisiblePos(self, e_spot);

		if(!saw_it&&!(self->spawnflags&MSF_AMBUSH))
		{//no line of sight and not an ambush monster
			if(gi.inPVS(viewspot, alerter->origin))
			{//25% chance will see impact(alerter) and detect alert owner anyway
				if(!irand(0,3))
					saw_it = true;//go ahead and go for it
			}
		}
	}
	
	if(!saw_it)
		goto loopagain;

	if(!self->monsterinfo.alert)
		goto loopagain;
	
	self->last_alert = alerter;
	return self->monsterinfo.alert(self, alerter, enemy);

loopagain:
	if(alerter)
	{
		alerter = alerter->prev_alert;
		if(alerter)
			goto alertloop;
	}
	
	return false;
}

/*
===========
HuntTarget - a target has been found, it is visible, so do we attack it, watch it, or stand there?

============
*/
void HuntTarget (edict_t *self)
{
	vec3_t	vec;
	int r;

	self->goalentity = self->enemy;
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
	{
		QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
	}
	else
	{
		r = range(self,self->enemy);
		if ((self->monsterinfo.aiflags & AI_EATING) && (r == RANGE_MID))
		{
			QPostMessage(self, MSG_WATCH, PRI_DIRECTIVE, NULL);
		}
		else
		{
			QPostMessage(self, MSG_RUN, PRI_DIRECTIVE, NULL);
		}
	}
	VectorSubtract (self->enemy->s.origin, self->s.origin, vec);
	self->ideal_yaw = VectorYaw(vec);
	// wait a while before first attack
	if (!(self->monsterinfo.aiflags & AI_STAND_GROUND))
		AttackFinished (self, 1);
}

/*
===========
FoundTarget - a target has been found, let other monsters know this. Then hunt it
============
*/
void FoundTarget (edict_t *self, qboolean setsightent)
{
	char	*o_target;
	// let other monsters see this monster for a while
	if (!self->enemy)
		return;

	if(self->classID == CID_OGLE)
		self->spawnflags = 0;

	self->monsterinfo.awake = true;
	self->spawnflags &= ~MSF_AMBUSH;
	self->targetname = NULL;
	self->monsterinfo.pausetime = -1;//was 0;

	if(self->wakeup_target)
	{
		o_target = self->target;
		self->target = self->wakeup_target;
		G_UseTargets(self, self->enemy);
		self->target = o_target;
		self->wakeup_target = NULL;
	}

	if (self->enemy->client && setsightent)
	{
		level.sight_entity = self;
		level.sight_entity_framenum = level.framenum;
		level.sight_entity->light_level = 128;
	}

	self->show_hostile = level.time + 1;		// wake up other monsters

	VectorCopy(self->enemy->s.origin, self->monsterinfo.last_sighting);

	if (!self->combattarget )	// Not going for a combat point?
	{
		// dont want to do this if we are a fish
		if (self->classID != CID_FISH)
			HuntTarget (self);

		if (!self->oldenemy)
		{
			if (!(self->monsterinfo.aiflags & AI_SOUND_TARGET) )
				QPostMessage(self, MSG_VOICE_SIGHT, PRI_DIRECTIVE, "be", SIGHT_SOUND_TARGET, self->enemy);
			else
				QPostMessage(self, MSG_VOICE_SIGHT, PRI_DIRECTIVE, "be", SIGHT_VISIBLE_TARGET, self->enemy);
		}

		self->spawnflags &= ~MSF_AMBUSH;
		return;
	}

	self->goalentity = self->movetarget = G_PickTarget(self->combattarget);
	
	if (!self->movetarget)
	{
		self->goalentity = self->movetarget = self->enemy;
		// dont want to do this if we are a fish
		if (self->classID != CID_FISH)
			HuntTarget (self);
		
		if (!self->oldenemy)
		{
			if (!(self->monsterinfo.aiflags & AI_SOUND_TARGET) )
				QPostMessage(self, MSG_VOICE_SIGHT, PRI_DIRECTIVE, "be", SIGHT_SOUND_TARGET, self->enemy);
			else
				QPostMessage(self, MSG_VOICE_SIGHT, PRI_DIRECTIVE, "be", SIGHT_VISIBLE_TARGET, self->enemy);
		}

//		gi.dprintf("%s at %s, combattarget %s not found\n", self->classname, vtos(self->s.origin), self->combattarget);
		self->spawnflags &= ~MSF_AMBUSH;
		return;
	}

	// clear out our combattarget, these are a one shot deal
	self->combattarget = NULL;
	self->monsterinfo.aiflags |= AI_COMBAT_POINT;

	// clear the targetname, that point is ours!
	self->movetarget->targetname = NULL;

	// run for it , assuming we aren't a fish
	if (self->classID != CID_FISH)
		QPostMessage(self, MSG_RUN, PRI_DIRECTIVE, NULL);

	//Make a sight sound
	if (!self->oldenemy)
	{
		if (!(self->monsterinfo.aiflags & AI_SOUND_TARGET) )
			QPostMessage(self, MSG_VOICE_SIGHT, PRI_DIRECTIVE, "be", SIGHT_SOUND_TARGET, self->enemy);
		else
			QPostMessage(self, MSG_VOICE_SIGHT, PRI_DIRECTIVE, "be", SIGHT_VISIBLE_TARGET, self->enemy);
	}
	
	self->spawnflags &= ~MSF_AMBUSH;
}

/*
qboolean ok_to_wake (edict_t *monster, qboolean gorgon_roar)

Can this monster be woken up by something other than direct line of sight to player
*/

qboolean ok_to_wake (edict_t *monster, qboolean gorgon_roar, qboolean ignore_ambush)
{
	if(gorgon_roar)
	{
		if(monster->monsterinfo.c_mode)
			return false;
	}
	else if(monster->monsterinfo.aiflags & AI_EATING ||//eating or perching
		monster->targetname ||//a monster that's supposed to be triggered - problem, one a monster is used and woken up, won't respond to alerts like others...?
		monster->monsterinfo.c_mode ||//cinematic
		monster->spawnflags & MSF_ASLEEP ||//shouldn't happen, but just in case
		(monster->spawnflags & MSF_AMBUSH && !ignore_ambush))
		return false;

	return true;
}

qboolean PlayerCreeping(playerinfo_t *playerinfo)
{
	if(playerinfo->upperseq == ASEQ_CREEPF ||
		playerinfo->upperseq == ASEQ_STAND ||
		playerinfo->upperseq == ASEQ_CREEPB ||
		playerinfo->upperseq == ASEQ_CREEPB_END ||
		playerinfo->upperseq == ASEQ_CROUCH_GO ||
		playerinfo->upperseq == ASEQ_CROUCH ||
		playerinfo->upperseq == ASEQ_CROUCH_END ||
		playerinfo->upperseq == ASEQ_CROUCH_WALK_F ||
		playerinfo->upperseq == ASEQ_CROUCH_WALK_B ||
		playerinfo->upperseq == ASEQ_CROUCH_WALK_L ||
		playerinfo->upperseq == ASEQ_CROUCH_WALK_R ||
		playerinfo->lowerseq == ASEQ_CREEPF ||
		playerinfo->lowerseq == ASEQ_STAND ||
		playerinfo->lowerseq == ASEQ_CREEPB ||
		playerinfo->lowerseq == ASEQ_CREEPB_END ||
		playerinfo->lowerseq == ASEQ_CROUCH_GO ||
		playerinfo->lowerseq == ASEQ_CROUCH ||
		playerinfo->lowerseq == ASEQ_CROUCH_END||
		playerinfo->lowerseq == ASEQ_CROUCH_WALK_F ||
		playerinfo->lowerseq == ASEQ_CROUCH_WALK_B ||
		playerinfo->lowerseq == ASEQ_CROUCH_WALK_L ||
		playerinfo->lowerseq == ASEQ_CROUCH_WALK_R)
		return (true);

	return (false);
}
/*
===========
FindTarget

Self is currently not attacking anything, so try to find a target

Returns TRUE if an enemy was sighted

When a player fires a missile or does other things to make noise, the 
point of impact becomes an alertent so that monsters that see the
impact will respond as if they had seen the player.

Since FindTarget is not called every frame for monsters (average
about once every 3 frames per monster), this does two potential checks.

First it checks against the current sight_client which cycles through
the players.

If that check fails, it will check for all the secondary alerts and
enemies.  if it can't find any, it will check for another player if
it can find one other than the first one it checked.
============
*/
qboolean ogle_findtarget (edict_t *self);
qboolean FindTarget (edict_t *self)
{
	edict_t		*client, *firstclient;
	qboolean	heardit = false;
	int			r;
	edict_t		*ent;
	int			flag;
	qboolean	clientonly = true;
	qboolean	e_infront = false;
	vec3_t		v;
	float		dist;

//FIXME: wakeup_distance -1 never look?
	if(self->classID == CID_OGLE)
		return ogle_findtarget(self);

	if (self->monsterinfo.aiflags & AI_GOOD_GUY)
	{
		if (self->goalentity)
		{
			if (strcmp(self->goalentity->classname, "target_actor") == 0)
				return false;
		}

		//FIXME look for monsters?
		return false;
	}

	// if we're going to a combat point, just proceed
	if (self->monsterinfo.aiflags & AI_COMBAT_POINT)
		return false;

	if(self->ai_mood_flags&AI_MOOD_FLAG_IGNORE_ENEMY)
	{//being forced to use buoys, and ignore enemy until get to forced_buoy
		return false;
	}

// if the first spawnflag bit is set, the monster will only wake up on
// really seeing the player, not another monster getting angry or hearing
// something

// revised behavior so they will wake up if they "see" a player make a noise
// but not weapon impact/explosion noises

startcheck:
	flag = 1;
	if(clientonly)
	{//look oly at the level.sight_client
		firstclient = client = level.sight_client;
	}
	else
	{
		if(ANARCHY)
		{//crazy monsters mode
			int	checkcnt = 0;
			client = self;
			while((!client || !client->inuse || !(client->svflags & SVF_MONSTER)||client->health<=0||client == self) && checkcnt < globals.num_edicts)
			{
				client = &g_edicts[irand(0, globals.num_edicts)];
				checkcnt++;
			}
		}
		else
		{
			if(level.sight_entity == self)
			{
				level.sight_entity = NULL;
				return false;
			}

			if (level.sight_entity && (level.sight_entity_framenum >= (level.framenum - 1)) && !(self->spawnflags & MSF_AMBUSH) )
			{//go after the enemy another monster saw saw, but only if not in ambush
				client = level.sight_entity;
				if (client->enemy == self->enemy)
				{
					return false;
				}
			}
			else if (Alerted(self))
			{//picked up an enemy from an alert
				return true;
			}
			else
			{
				client = NULL;
				// Looking for secondary enemies
				if ((self->monsterinfo.otherenemyname) && (self->monsterinfo.chase_finished < level.time))
				{
					ent = NULL;
					while((ent=FindInRadius(ent,self->s.origin,175)) != NULL)
					{
						if (!strcmp(ent->classname,self->monsterinfo.otherenemyname)&&ent!=self)
						{
							flag = 0;
							client = ent;
							break;
						}
					}
				}

				//  Look at the sight client
				if (!client)
				{//found no non-clients, cycle to next client and check it for second check
					AI_SetSightClient();
					if(firstclient == level.sight_client)
						return false;//same as first check, and that failed if we're here, so return.
					client = level.sight_client;
				}
			}
		}
	}
	if (!client)
		goto nextcheck;	// no clients to get mad at

	// if the entity went away, forget it
	if (!client->inuse)
		goto nextcheck;

	if (client == self)
		goto nextcheck;	//????

	if (client == self->enemy)
		return true;	// JDC false;

	if (self->monsterinfo.otherenemyname)
	{
		if (!strcmp(client->classname,self->monsterinfo.otherenemyname))
			client->light_level = 128; // Let it be seen
	}

	// if we are a fish - is the target in the water - have to be at least waist deep?
	if (self->classID == CID_FISH && client->waterlevel < 2)
		goto nextcheck;	//????

	if (client->client)
	{
		if (client->flags & FL_NOTARGET)
			goto nextcheck;
	}
	else if (client->svflags & SVF_MONSTER)
	{
		if(flag)
		{//not a secondary enemy
			if(!ANARCHY)
			{
				if (ok_to_wake(self, false, true))
				{//eating or in a cinematic or not awake or targeted, leave them alone
					goto nextcheck;
				}
			}

			if (!client->enemy)
			{
				if(!ANARCHY)
					goto nextcheck;
			}
			else
			{
				if (client->enemy->health<0 && !ANARCHY)
					goto nextcheck;

				if (client->enemy->flags & FL_NOTARGET)
					goto nextcheck;  
			}

			if(!visible(self, client))
				goto nextcheck;

			if(!ANARCHY)
				self->enemy = client->enemy;
			else
				self->enemy = client;

			if(client->ai_mood == AI_FLEE)
				FoundTarget(self, false);//let them stay the sight entity
			else
				FoundTarget(self, true);//make me the sight entity

			/*if (!(self->monsterinfo.aiflags & AI_SOUND_TARGET))
				QPostMessage(self, MSG_VOICE_SIGHT, PRI_DIRECTIVE, "e", self->enemy);*/
				//self->monsterinfo.sight (self, self->enemy);

			return true;
		}
	}
	else if (heardit)
	{
		if (client->owner->flags & FL_NOTARGET)
			goto nextcheck;
	}
	else
		goto nextcheck;

	if (!heardit)
	{
		if(self->classID == CID_ASSASSIN)
			e_infront = true;
		else
			e_infront = infront(self, client);

		if(!e_infront && client->client)
		{
			if(PlayerCreeping(&client->client->playerinfo))
				goto nextcheck;
		}

		VectorSubtract(client->s.origin, self->s.origin, v);
		dist = VectorLength(v);

		if(dist > self->wakeup_distance)
			goto nextcheck;

		r = categorize_range (self, client, dist);

		if (r == RANGE_FAR)
			goto nextcheck;

		if ((self->monsterinfo.aiflags & AI_EATING) && (r > RANGE_MID))  
		{
			self->enemy = client;
			goto nextcheck;
		}

// this is where we would check invisibility

		// is client in an spot too dark to be seen?
		if (client->light_level <= 5)
			goto nextcheck;

		if(self->svflags&SVF_MONSTER && client->client)
		{
			if(skill->value < 2.0 && !(self->monsterinfo.aiflags & AI_NIGHTVISION))
			{
				if(client->light_level < flrand(6, 77))
				{
					goto nextcheck;
				}
			}
		}

		if (r == RANGE_NEAR)
		{
			if (client->show_hostile < level.time && !e_infront)
			{
				goto nextcheck;
			}
		}
		else if (r == RANGE_MID)
		{
			if (!e_infront)
			{
				goto nextcheck;
			}
		}

		//sfs--this check is much less trivial than infront: prolly wasn't a noticeable deal,
		//		since RANGE_FAR was first rejection check, but it's still better to try the
		//		dotproduct before the traceline
		if (!visible (self, client))
		{
			goto nextcheck;
		}

		self->enemy = client;

		flag=1;
		if (self->monsterinfo.otherenemyname)
		{
			if (strcmp(self->enemy->classname, self->monsterinfo.otherenemyname) == 0)	// This is a secondary enemy
			{
				self->monsterinfo.chase_finished = level.time + 15;
				flag=0;
			}
		}	

		if (flag)	// This is not a secondary enemy
		{
			self->monsterinfo.aiflags &= ~AI_SOUND_TARGET;

			if (!self->enemy->client)
			{
				self->enemy = self->enemy->enemy;
				if (!self->enemy->client)
				{
					self->enemy = NULL;
					goto nextcheck;
				}
			}
		}
	}
	else	// heardit
	{
		goto nextcheck;

	}

//
// got one
//
	FoundTarget (self, true);

	return true;

nextcheck:
	if(clientonly)
	{
		clientonly = false;
		goto startcheck;
	}
	else
		return false;
}


//=============================================================================

/*
============
FacingIdeal

============
*/
qboolean FacingIdeal(edict_t *self)
{
	float	delta;

	delta = anglemod(self->s.angles[YAW] - self->ideal_yaw);
	if (delta > 45 && delta < 315)
		return false;
	return true;
}


//=============================================================================

qboolean M_CheckAttack (edict_t *self)
{
	vec3_t	spot1, spot2;
	float	chance;
	trace_t	tr;

	if (self->enemy->health > 0)
	{
	// see if any entities are in the way of the shot
		VectorCopy (self->s.origin, spot1);
		spot1[2] += self->viewheight;
		VectorCopy (self->enemy->s.origin, spot2);
		spot2[2] += self->enemy->viewheight;

		gi.trace (spot1, vec3_origin, vec3_origin, spot2, self, CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_SLIME|CONTENTS_LAVA,&tr);

		// do we have a clear shot?
		if (tr.ent != self->enemy)
			return false;
	}
	
	// melee attack
	
	if (enemy_range == RANGE_MELEE)
	{
		// don't always melee in easy mode
		if (skill->value == 0 && irand(0, 3) )
			return false;

		if (classStatics[self->classID].msgReceivers[MSG_MELEE])
		{
			self->monsterinfo.attack_state = AS_MELEE;
		}
		else
		{
			self->monsterinfo.attack_state = AS_MISSILE;
		}
		return true;
	}
	
	// missile attack

	if (!classStatics[self->classID].msgReceivers[MSG_MISSILE])
		return false;
		
	if (level.time < self->monsterinfo.attack_finished)
		return false;
		
	if (enemy_range == RANGE_FAR)
		return false;

	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
	{
		chance = 0.4;
	}
	else if (enemy_range == RANGE_MELEE)
	{
		chance = 0.2;
	}
	else if (enemy_range == RANGE_NEAR)
	{
		chance = 0.1;
	}
	else if (enemy_range == RANGE_MID)
	{
		chance = 0.02;
	}
	else
	{
		return false;
	}

	if (skill->value == 0)
		chance *= 0.5;
	else if (skill->value >= 2)
		chance *= 2;

	if (flrand(0.0, 1.0) < chance)
	{
		self->monsterinfo.attack_state = AS_MISSILE;
		self->monsterinfo.attack_finished = level.time + flrand(0.0, 2.0);
		return true;
	}

	if (self->flags & FL_FLY)
	{
		if (!irand(0, 2))
			self->monsterinfo.attack_state = AS_SLIDING;
		else
			self->monsterinfo.attack_state = AS_STRAIGHT;
	}

	return false;
}


/*
=============
ai_run_melee

Turn and close until within an angle to launch a melee attack
=============
*/
void ai_run_melee(edict_t *self)
{
	self->ideal_yaw = enemy_yaw;
	M_ChangeYaw (self);

	if (FacingIdeal(self))
	{
		QPostMessage(self, MSG_MELEE, PRI_DIRECTIVE, NULL);
		self->monsterinfo.attack_state = AS_STRAIGHT;
	}
}


/*
=============
ai_run_missile

Turn in place until within an angle to launch a missile attack
=============
*/
void ai_run_missile(edict_t *self)
{
	self->ideal_yaw = enemy_yaw;
	M_ChangeYaw (self);

	if (FacingIdeal(self))
	{
		QPostMessage(self, MSG_MISSILE, PRI_DIRECTIVE, NULL);
		self->monsterinfo.attack_state = AS_STRAIGHT;
	}
};


/*
=============
ai_run_slide

Strafe sideways, but stay at aproximately the same range
=============
*/
void ai_run_slide(edict_t *self, float distance)
{
	float	ofs;
	
	self->ideal_yaw = enemy_yaw;
	M_ChangeYaw (self);

	if (self->monsterinfo.lefty)
		ofs = 90;
	else
		ofs = -90;
	
	if (M_walkmove (self, self->ideal_yaw + ofs, distance))
		return;
		
	self->monsterinfo.lefty = 1 - self->monsterinfo.lefty;
	M_walkmove (self, self->ideal_yaw - ofs, distance);
}


/*
=============
ai_checkattack

Decides if we're going to attack or do something else
used by ai_run, and ai_stand
=============
*/
qboolean ai_checkattack (edict_t *self, float dist)
{
	vec3_t		temp;
	qboolean	hesDeadJim;

	if ((self->monsterinfo.aiflags & AI_FLEE)||(self->monsterinfo.aiflags & AI_COWARD))	// He's running away, not attacking
	{
		return false;
	}

	// see if the enemy is dead
	hesDeadJim = false;
	if ((!self->enemy) || (!self->enemy->inuse))
	{
		hesDeadJim = true;
	}
	else
	{
		if (self->monsterinfo.aiflags & AI_BRUTAL)
		{
			if (self->enemy->health <= -80)
				hesDeadJim = true;
		}
		else
		{
			if (self->enemy->health <= 0)
				hesDeadJim = true;
		}
	}

	if (hesDeadJim)
	{
		self->enemy = NULL;
	// FIXME: look all around for other targets
		if (self->oldenemy && self->oldenemy->health > 0)
		{
			self->enemy = self->oldenemy;
			self->oldenemy = NULL;
			HuntTarget (self);
		}
		else
		{
			if (self->movetarget)
			{
				self->goalentity = self->movetarget;
				QPostMessage(self, MSG_WALK, PRI_DIRECTIVE, NULL);
			}
			else
			{
				// we need the pausetime otherwise the stand code
				// will just revert to walking with no target and
				// the monsters will wonder around aimlessly trying
				// to hunt the world entity
				self->monsterinfo.pausetime = level.time + 100000000;
				QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
			}
			return true;
		}
	}

	self->show_hostile = level.time + 1;		// wake up other monsters

// check knowledge of enemy
	qboolean enemy_vis = clear_visible(self, self->enemy);
	if (enemy_vis)
	{
		self->monsterinfo.search_time = level.time + 5;
		VectorCopy (self->enemy->s.origin, self->monsterinfo.last_sighting);
	}

// look for other coop players here
//	if (coop && self->monsterinfo.search_time < level.time)
//	{
//		if (FindTarget (self))
//			return true;
//	}

	enemy_range = range(self, self->enemy);
	VectorSubtract (self->enemy->s.origin, self->s.origin, temp);
	enemy_yaw = VectorYaw(temp);


	// JDC self->ideal_yaw = enemy_yaw;
	
	if (self->monsterinfo.attack_state == AS_MISSILE)
	{
		ai_run_missile (self);
		return true;
	}
	if (self->monsterinfo.attack_state == AS_MELEE)
	{
		ai_run_melee (self);
		return true;
	}

	// if enemy is not currently visible, we will never attack
	if (!enemy_vis)
		return false;

	return self->monsterinfo.checkattack (self);
}

float ai_face_goal (edict_t *self)
{
	vec3_t vec;

	if (self->monsterinfo.searchType == SEARCH_BUOY)
		VectorSubtract(self->monsterinfo.nav_goal, self->s.origin, vec);
	else if(self->goalentity)
		VectorSubtract(self->goalentity->s.origin, self->s.origin, vec);
	else if(self->enemy)
		VectorSubtract(self->enemy->s.origin, self->s.origin, vec);
	else
		return false;

	if (self->monsterinfo.idle_time < level.time)
	{
		self->ideal_yaw = VectorYaw(vec);
		return M_ChangeYaw(self);
	}
	else
		return false;
}

/*
=============
ai_flee

The monster has an enemy it is trying to get away from
=============
*/
void ai_flee (edict_t *self, float dist)
{
	vec3_t	vec;

	if (self->enemy)
	{
		VectorSubtract (self->enemy->s.origin, self->s.origin, vec);
		self->ideal_yaw = VectorYaw(vec);
		self->ideal_yaw = anglemod(self->ideal_yaw + self->best_move_yaw);
		M_ChangeYaw(self);
		if(!M_walkmove(self, self->s.angles[YAW], dist) && AnglesEqual(self->s.angles[YAW], self->ideal_yaw, 5))
			self->best_move_yaw = flrand(60, 300);
		else
			self->best_move_yaw = 180;

		/*
		VectorSubtract(self->s.origin, self->enemy->s.origin, vec);
		self->ideal_yaw = vectoyaw(vec);
		M_ChangeYaw(self);
		M_MoveAwayFromGoal (self, dist);*/
	}
}

/*
=============================================================
void extrapolateFiredir (edict_t *self,vec3_t p1,float pspeed,edict_t *targ,float accept,vec3_t vec2) 

MG

Estimates where the "targ" will be by the time a projectile
travelling at "pspeed" leaving "org" arrives at "targ"'s origin.
It then calculates a new spot to shoot at so that the
projectile will arrive at such spot at the same time as
"targ".  Will return '0 0 0' (FALSE) if there is not a clear
line of fire to the spot or if the new vector is out of the
acceptable range (based on dot product of original vec and
the new vec).

PROPOSAL: Factor in skill->value? 0 = no leading, 4 = perfect leading, 1-3 innaccuracy levels for leading
=============================================================
*/
void extrapolateFiredir (edict_t *self,vec3_t p1,float pspeed,edict_t *targ,float accept,vec3_t vec2) 
{
	float dist1, dist2, tspeed, dot, eta1, eta2, eta_delta, tdist;
	qboolean failed = false;
	vec3_t p2, p3, targ_dir, vec1, tempv;
	trace_t trace;
	float offset;
	
	if(!targ)
	{
		VectorClear(vec2);
		return;
	}

	tdist = vhlen(targ->s.origin, self->s.origin);
	if(!skill->value)
	{//poor shot, take forward and screw it up
		AngleVectors(self->s.angles, tempv, NULL, NULL);
		VectorMA(p1, pspeed, tempv, p2);
		
		if(tdist < 128)
			offset = 48 * tdist/128;
		else
			offset = 48;

		p2[0] += flrand(-offset, offset);
		p2[1] += flrand(-offset, offset);
		p2[2] += flrand(-offset/2, offset * 0.666);
		VectorSubtract(p2, p1, vec2);
		VectorNormalize(vec2);
		return;
	}

	offset = 2 - skill->value;//skill >= 2 = perfect aim, skill 1 is very poor
	
	if(tdist < 128)
		offset *= tdist/128;

	if(offset<0)
		offset = 0;

	if(skill->value < 2.0 && !(self->monsterinfo.aiflags & AI_NIGHTVISION))
	{
		if(targ->client)
		{
			offset += targ->light_level/32;
		}
	}

	VectorCopy(targ->s.origin, p2);
	if(offset)
	{
		p2[0] += flrand(-offset*12, offset*12);
		p2[1] += flrand(-offset*12, offset*12);
		p2[2] += flrand(-offset*8, offset*8);
	}

	VectorSubtract(p2, p1, vec1);

	dist1 = VectorNormalize(vec1);
	
	VectorCopy(targ->velocity, targ_dir);
	
	tspeed = VectorNormalize(targ_dir);
	eta1 = dist1/pspeed;					//Estimated time of arrival of projectile to p2
	
	VectorMA(p2, tspeed*eta1, targ_dir, p3);
	VectorSubtract(p3, p1, tempv);
	
	dist2 = VectorNormalize(tempv);
	eta2 = dist2/pspeed;					//ETA of projectile to p3
	eta_delta = eta2-eta1;				//change in ETA's
	
	VectorMA(p3, tspeed*eta_delta*flrand(0, 1), targ_dir, p3);
//careful,  above version does not modify targ_dir
	
	gi.trace(p1, vec3_origin, vec3_origin, p3, self, MASK_SOLID,&trace);
	if(trace.fraction<1.0)
		failed = true;

	VectorSubtract(p3, p1, vec2);
	VectorNormalize(vec2);

	dot  =  DotProduct(vec1, vec2);

	if(dot<accept)						//Change in dir too great
	{
		failed = true;
	}
	if(failed)
	{
		VectorClear(vec2);
	}
}

/*
ClearLastAlerts

takes last alerts away from any monster who's last alert was the "self" alert
*/
void ClearLastAlerts(alertent_t *self)
{
	int	i;

	for(i = 0; i < game.maxentities; i++)
	{
		if(g_edicts[i].svflags & SVF_MONSTER)
		{
			if(g_edicts[i].last_alert == self)
			{
				g_edicts[i].last_alert = NULL;
			}
		}
	}
}

/*
alert_timed_out

clears out all the fields for an alert, removes it from the linked list and sets it's slot to being empty to make room for insertion of others later
*/
void alert_timed_out(alertent_t *self)
{
	if (self==NULL)
		return;

	//take self out of alert chain
	if(self->prev_alert)
	{
		if(self->next_alert)
			self->prev_alert->next_alert = self->next_alert;
		else
		{//I'm the last one!
			level.last_alert = self->prev_alert;
			self->prev_alert->next_alert = NULL;
		}
	}
	else
	{//I'm the first one!
		if(self->next_alert)
			level.alert_entity = self->next_alert;
		else
			level.last_alert = level.alert_entity = NULL;
	}

	//Clear out all fields
	self->next_alert = NULL;
	self->prev_alert = NULL;
	self->enemy = NULL;
	VectorClear(self->origin);
	self->alert_svflags = 0;
	self->lifetime = 0;
	self->inuse = false;
	ClearLastAlerts(self);

	level.num_alert_ents--;
}

/*
GetFirstEmptyAlertInList

returns the first alert in the level.alertents list that isn;t in use
*/
alertent_t *GetFirstEmptyAlertInList(void)
{
	int	i;
	//have max number of alerts, remove the first one
	if(level.num_alert_ents >= MAX_ALERT_ENTS)
		alert_timed_out(level.alert_entity);
	
	for(i = 0; i < MAX_ALERT_ENTS; i++)
	{
		if(!level.alertents[i].inuse)
		{
			return &level.alertents[i];
		}
	}
	return NULL;
}

/*
AlertMonsters

allots an alertent monsters will check during FindTarget to see if they should be alerted by it

self = used for sv_flags info and positioning of the alert entity
enemy = entity to make the monsters mad at if they're alerted
(float)lifetime = how many seconds the alert exists for
(qboolean)ignore_shadows = this alert gives away enemy's position, even if he is in shadows (I use this for staff hits on the floor and any other effect the player makes at his own location(like shooting a weapon), not for projectiles impacting)
*/
void AlertMonsters (edict_t *self, edict_t *enemy, float lifetime, qboolean ignore_shadows)
{//FIXME: if  and stick new one at the end
	alertent_t	*alerter = level.alert_entity;
	alertent_t	*last_alert = NULL;

	if (deathmatch->value)		// Don't need this if no monsters...
		return;

	if(!lifetime)
		lifetime = 1.0;//stick around for 1 second

	//stick into the level's alerter chain
	if(alerter)
	{//go down the list and find an empty slot
		//fixme: just store the entnum?
		while(alerter->next_alert)
		{
			last_alert = alerter;
			alerter = alerter->next_alert;
		}
		level.last_alert = alerter = GetFirstEmptyAlertInList();
	}
	else//we're the first one!
		level.alert_entity = level.last_alert = alerter = GetFirstEmptyAlertInList();

	if(!alerter)
	{
#ifdef _DEVEL
		gi.dprintf("Error: out of alerts and can't find any empty slots!\n");
#endif
		return;
	}

	//I'm active, don't let my slot be used until I'm freed
	alerter->inuse = true;
	//point to the previous alerter, if any
	alerter->prev_alert = last_alert;
	//make the previous alerter point to me as the next one
	if(alerter->prev_alert)
		alerter->prev_alert->next_alert = alerter;
	//put me in the "self"'s spot
	VectorCopy(self->s.origin, alerter->origin);
	alerter->enemy = enemy;
	//should we keep track of owner in case they move to move the alert with them?  Only for monsters
	//alerter->owner = self;
	alerter->alert_svflags = self->svflags;
	if(ignore_shadows)//whatever happened would give away enemy's position, even in shadows
		alerter->alert_svflags |= SVF_ALERT_NO_SHADE;

	//stick around until after this point in time
	alerter->lifetime = level.time + lifetime;

	level.num_alert_ents++;
}

void ai_spin (edict_t *self, float amount)
{
	self->s.angles[YAW] += amount;
}

qboolean ai_have_enemy (edict_t *self)
{
	qboolean enemy_gone = false;

	if(!self->enemy)
		enemy_gone = true;
	else if(self->enemy->health <= 0)
		enemy_gone = true;

	if(!enemy_gone)
		return true;
	else 
	{
		if(self->oldenemy)
		{
			if(self->oldenemy->health>0)
			{
				self->enemy=self->oldenemy;
				self->oldenemy = NULL;
//				gi.dprintf("Going for old enemy\n");
				return true;
			}
		}
	}
	
	QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
//	gi.dprintf("Lost enemies\n");
	return false;
}

qboolean movable (edict_t *ent)
{
	if(	ent->movetype!=PHYSICSTYPE_NONE &&
		ent->movetype!=PHYSICSTYPE_PUSH)
		return true;

	return false;
}
