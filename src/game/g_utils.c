// g_utils.c -- misc utility functions for game module

#include "g_local.h"
#include "FX.h"
#include "g_Skeletons.h"
#include "random.h"
#include "vector.h"
#include "g_BoundingForm.h"
#include "g_Physics.h"

void G_ProjectSource (vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result)
{
	result[0] = point[0] + forward[0] * distance[0] + right[0] * distance[1];
	result[1] = point[1] + forward[1] * distance[0] + right[1] * distance[1];
	result[2] = point[2] + forward[2] * distance[0] + right[2] * distance[1] + distance[2];
}


void G_SetToFree(edict_t *self)
{
	if(self->PersistantCFX)
	{
		gi.RemovePersistantEffect(self->PersistantCFX, REMOVE_ENTITY);
		self->PersistantCFX = 0;
	}

	self->think = G_FreeEdict;
	self->nextthink = level.time + FRAMETIME;
	self->svflags &= ~SVF_NOCLIENT;

	self->next_pre_think = -1;
	self->next_post_think = -1;

	self->takedamage = DAMAGE_NO;
	self->movetype = PHYSICSTYPE_NONE;
	self->solid = SOLID_NOT;
	self->touch = NULL;
	self->blocked = NULL;
	self->isBlocked = NULL;
	self->isBlocking = NULL;
	self->bounced = NULL;
	VectorClear(self->mins);
	VectorClear(self->maxs);

	gi.linkentity(self);
}

/*
=============
G_Find

Searches all active entities for the next one that holds
the matching string at fieldofs (use the FOFS() macro) in the structure.

Searches beginning at the edict after from, or the beginning if NULL
NULL will be returned if the end of the list is reached.

=============
*/
edict_t *G_Find (edict_t *from, int fieldofs, char *match)
{
	char	*s;

	// if we aren't trying to find anything, then exit.
	if (match == NULL)
		return NULL;

	if (!from)
		from = g_edicts;
	else
		from++;

	for ( ; from < &g_edicts[globals.num_edicts] ; from++)
	{
		if (!from->inuse)
			continue;
		s = *(char **) ((byte *)from + fieldofs);
		if (!s)
			continue;
		if (!Q_stricmp (s, match))
			return from;
	}

	return NULL;
}


//
//=================
// FindOnPath
//
// Returns damageable entities that lie along a given pathway.  This is NOT 100% guaranteed to return a given edict only once.
//
//=================

edict_t *findonpath(edict_t *startent, vec3_t startpos, vec3_t endpos, vec3_t mins, vec3_t maxs, vec3_t *resultpos)
{
	vec3_t	vect, curpos;
	trace_t trace;
	float	skipamount;
	edict_t *tracebuddy;

	VectorCopy(startpos, curpos);
	tracebuddy = startent;
	while(1)
	{
		gi.trace(curpos, mins, maxs, endpos, tracebuddy, MASK_SHOT,&trace);

		// If we started inside something.
		if (trace.startsolid || trace.allsolid)
		{
			if (trace.ent && trace.ent->takedamage)
			{	// Found an item.  Skip forward a distance and return the ent.
				skipamount = maxs[2];
				if (skipamount < 4)
					skipamount = 4;
				VectorSubtract(endpos, curpos, vect);
				if (VectorNormalize(vect) < skipamount)	// skip to the end.
					VectorCopy(endpos, *resultpos);
				else
					VectorMA(curpos, skipamount, vect, *resultpos);
				
				return(trace.ent);
			}
			else
			{	// Didn't stop on anything useful, continue to next trace.
				skipamount = maxs[2];	// Skip forward a bit.
				if (skipamount < 4)
					skipamount = 4;
				VectorSubtract(endpos, curpos, vect);
				if (VectorNormalize(vect) < skipamount)	// skip to the end.
					return(NULL);		// Didn't find anything.
				else
					VectorMA(curpos, skipamount, vect, curpos);
				if (trace.ent)
					tracebuddy = trace.ent;
				continue;	// Do another trace.
			}
		}

		// If we did not start inside something, but stopped at something.
		if (trace.fraction < .99)
		{
			if (trace.ent && trace.ent->takedamage)
			{	// Found an item.  Skip forward a distance and return the ent.
				skipamount = maxs[2];
				if (skipamount < 4)
					skipamount = 4;
				VectorSubtract(endpos, trace.endpos, vect);
				if (VectorNormalize(vect) < skipamount)	// skip to the end.
					VectorCopy(endpos, *resultpos);
				else
					VectorMA(trace.endpos, skipamount, vect, *resultpos);
				
				return(trace.ent);
			}
			else
			{	// Didn't stop on anything useful, continue to next trace.
				skipamount = maxs[2];	// Skip forward a bit.
				if (skipamount < 4)
					skipamount = 4;
				VectorSubtract(endpos, trace.endpos, vect);
				if (VectorNormalize(vect) < skipamount)	// skip to the end.
					return(NULL);		// Didn't find anything.
				else
					VectorMA(trace.endpos, skipamount, vect, curpos);
				if (trace.ent)
					tracebuddy = trace.ent;
				continue;	// Do another trace.
			}
		}

		// If we finished the whole move.
		{
			VectorCopy(endpos, *resultpos);
			return(NULL);
		}
	};

	return(NULL);	// Never gets here.
}

#define NEW_FINDS (1)

#if !NEW_FINDS
/*
=================
findradius

Returns entities that have origins within a spherical area

findradius (origin, radius)
=================
*/
edict_t *findradius (edict_t *from, vec3_t org, float rad)
{
	vec3_t	eorg;
	int		j;

	if (!from)
		from = g_edicts;
	else
		from++;
	for ( ; from < &g_edicts[globals.num_edicts]; from++)
	{
		if (!from->inuse)
			continue;
		if (from->solid == SOLID_NOT)
			continue;
		for (j=0 ; j<3 ; j++)
			eorg[j] = org[j] - (from->s.origin[j] + (from->mins[j] + from->maxs[j])*0.5);
		if (VectorLength(eorg) > rad)
			continue;
		return from;
	}

	return NULL;
}

// THis works like findradius, except it uses the bbox of an ent to indicate the area to check.
edict_t *findinblocking(edict_t *from, edict_t *checkent)
{
	vec3_t	min, max;
	int		j;
	qboolean ok;

	if (!from)
		from = g_edicts;
	else
		from++;

	VectorAdd(checkent->s.origin, checkent->mins, min);
	VectorAdd(checkent->s.origin, checkent->maxs, max);
	for ( ; from < &g_edicts[globals.num_edicts]; from++)
	{
		if (!from->inuse)
			continue;
		if (from->solid == SOLID_NOT)
			continue;
		if (from == checkent)
			continue;
		ok=true;
		for (j=0 ; j<3 ; j++)
		{
			if (	from->s.origin[j] + from->mins[j] > max[j] ||
					from->s.origin[j] + from->maxs[j] < min[j])
			{	// Automatic failure.
				ok=false;
				break;
			}
		}
		if (ok)
			return from;
	}

	return NULL;
}


/*
=================
finddistance

Returns entities that have origins within a spherical shell area

finddistance (origin, mindist, maxdist)
=================
*/
edict_t *finddistance (edict_t *from, vec3_t org, float mindist, float maxdist)
{
	vec3_t	eorg;
	int		j;
	float elen;

	if (!from)
		from = g_edicts;
	else
		from++;
	for ( ; from < &g_edicts[globals.num_edicts]; from++)
	{
		if (!from->inuse)
			continue;
		if (from->solid == SOLID_NOT)
			continue;
		for (j=0 ; j<3 ; j++)
			eorg[j] = org[j] - (from->s.origin[j] + (from->mins[j] + from->maxs[j])*0.5);
		elen = VectorLength(eorg);
		if (elen > maxdist)
			continue;
		if (elen < mindist)
			continue;
		return from;
	}

	return NULL;
}

// THis works like findradius, except it uses two absolute positions to define where to search.
edict_t *findinbounds(edict_t *from, vec3_t min, vec3_t max)
{
	int		j;
	qboolean ok;

	if (!from)
		from = g_edicts;
	else
		from++;

	for ( ; from < &g_edicts[globals.num_edicts]; from++)
	{
		if (!from->inuse)
			continue;
		if (from->solid == SOLID_NOT)
			continue;

		ok=true;
		for (j=0 ; j<3 ; j++)
		{
			if (	from->s.origin[j] + from->mins[j] > max[j] ||
					from->s.origin[j] + from->maxs[j] < min[j])
			{	// Automatic failure.
				ok=false;
				break;
			}
		}
		if (ok)
			return from;
	}

	return NULL;
}

#else

// THis works like findradius, except it uses the bbox of an ent to indicate the area to check.
edict_t *findinblocking(edict_t *from, edict_t *checkent)
{
	static vec3_t	min, max;

	if (!from)
	{
		VectorAdd(checkent->s.origin, checkent->mins, min);
		VectorAdd(checkent->s.origin, checkent->maxs, max);
	}
	while (1)
	{
		from=findinbounds(from,min,max);
		if (!from)
			return 0;
		if (!from->inuse)
			continue;
		if (from == checkent)
			continue;
		return from;
	} 
}

/*
=================
findradius

Returns entities that have origins within a spherical area

findradius (origin, radius)
=================
*/
edict_t *findradius (edict_t *from, vec3_t org, float rad)
{
	static float max2;
	static vec3_t min;
	static vec3_t max;
	vec3_t	eorg;
	int		j;
	float elen;

	if (!from)
	{
		max2=rad*rad;
		VectorCopy(org,min);
		VectorCopy(org,max);
		for (j=0 ; j<3 ; j++)
		{
			min[j]-=rad;
			max[j]+=rad;
		}
	}
	while (1)
	{
		from=findinbounds(from,min,max);
		if (!from)
			return 0;
		if (!from->inuse)
			continue;
		for (j=0 ; j<3 ; j++)
			eorg[j] = org[j] - (from->s.origin[j] + (from->mins[j] + from->maxs[j])*0.5);
		elen = DotProduct(eorg,eorg);
		if (elen > max2)
			continue;
		return from;
	} 
}

/*
=================
finddistance

Returns entities that have origins within a spherical shell area

finddistance (origin, mindist, maxdist)
=================
*/
edict_t *finddistance (edict_t *from, vec3_t org, float mindist, float maxdist)
{
	static float min2;
	static float max2;
	static vec3_t min;
	static vec3_t max;
	vec3_t	eorg;
	int		j;
	float elen;

	if (!from)
	{
		min2=mindist*mindist;
		max2=maxdist*maxdist;
		VectorCopy(org,min);
		VectorCopy(org,max);
		for (j=0 ; j<3 ; j++)
		{
			min[j]-=maxdist;
			max[j]+=maxdist;
		}
	}
	while (1)
	{
		from=findinbounds(from,min,max);
		if (!from)
			return 0;
		if (!from->inuse)
			continue;
		for (j=0 ; j<3 ; j++)
			eorg[j] = org[j] - (from->s.origin[j] + (from->mins[j] + from->maxs[j])*0.5);
		elen = DotProduct(eorg,eorg);
		if (elen > max2)
			continue;
		if (elen < min2)
			continue;
		return from;
	} 
}

edict_t *findinbounds(edict_t *from, vec3_t min, vec3_t max)
{
	static edict_t *touchlist[MAX_EDICTS];
	static int index=-1;
	static int num;

	if (!from)
	{
		num = gi.BoxEdicts(min,max, touchlist, MAX_EDICTS, AREA_SOLID);
		index=0;
	}
	else
	{
		assert(touchlist[index]==from);
		// you cannot adjust the pointers yourself...
		// this means you did not call it with the previous edict
		index++;
	}
	for (;index<num;index++)
	{
		if (!touchlist[index]->inuse)
			continue;
		return touchlist[index];
	}
	return NULL;
}
#endif

/*
=============
G_PickTarget

Searches all active entities for the next one that holds
the matching string at fieldofs (use the FOFS() macro) in the structure.

Searches beginning at the edict after from, or the beginning if NULL
NULL will be returned if the end of the list is reached.

=============
*/
#define MAXCHOICES	8

edict_t *G_PickTarget (char *targetname)
{
	edict_t	*ent = NULL;
	int		num_choices = 0;
	edict_t	*choice[MAXCHOICES];

	if (!targetname)
	{
#ifdef _DEVEL
		gi.dprintf("G_PickTarget called with NULL targetname\n");
#endif
		return NULL;
	}

	while(1)
	{
		ent = G_Find (ent, FOFS(targetname), targetname);
		if (!ent)
			break;
		choice[num_choices++] = ent;
		if (num_choices == MAXCHOICES)
			break;
	}

	if (!num_choices)
	{
#ifdef _DEVEL
		gi.dprintf("G_PickTarget: target %s not found\n", targetname);
#endif
		return NULL;
	}

	return choice[irand(0, num_choices - 1)];
}



void Think_Delay (edict_t *ent)
{
	G_UseTargets (ent, ent->activator);
	G_FreeEdict (ent);
}

/*
==============================
G_UseTargets

the global "activator" should be set to the entity that initiated the firing.

If self.delay is set, a DelayedUse entity will be created that will actually
do the SUB_UseTargets after that many seconds have passed.

Centerprints any self.message to the activator.

Search for (string)targetname in all entities that
match (string)self.target and call their .use function

==============================
*/
void G_UseTargets (edict_t *ent, edict_t *activator)
{
	edict_t		*t;

//
// check for a delay
//
	if (ent->delay)
	{
	// create a temp object to fire at a later time
		t = G_Spawn();
		t->movetype = PHYSICSTYPE_NONE;
		t->classname = "DelayedUse";
		t->nextthink = level.time + ent->delay;
		t->think = Think_Delay;
		t->activator = activator;
#ifdef _DEVEL
		if (!activator)
			gi.dprintf ("Think_Delay with no activator\n");
#endif
		t->message = ent->message;
		t->text_msg = ent->text_msg;
		t->target = ent->target;
		t->killtarget = ent->killtarget;
		return;
	}
	
	
//
// print the message
//
	if ((ent->message) && !(activator->svflags & SVF_MONSTER))
	{
		gi.levelmsg_centerprintf (activator, (short)atoi(ent->message));
		if (ent->noise_index)
		{
			gi.sound (activator, CHAN_AUTO, ent->noise_index, 1, ATTN_NORM, 0);
		}
	}

	if ((ent->text_msg) && !(activator->svflags & SVF_MONSTER))
	{
		gi.centerprintf (activator, "%s", ent->text_msg);
	}

//
// kill killtargets
//
	if (ent->killtarget)
	{
		t = NULL;
		while ((t = G_Find (t, FOFS(targetname), ent->killtarget)))
		{
			QPostMessage(t,MSG_DEATH,PRI_DIRECTIVE,"eeei",t,ent,activator,100000);

			if (!ent->inuse)
			{
#ifdef _DEVEL
				gi.dprintf("entity was removed while using killtargets\n");
#endif
				return;
			}
		}
	}

//
// fire targets
//
	if (ent->target)
	{
		t = NULL;
		while ((t = G_Find (t, FOFS(targetname), ent->target)))
		{
			// doors fire area portals in a specific way
			if (!Q_stricmp(t->classname, "func_areaportal") &&
				(!Q_stricmp(ent->classname, "func_door") || !Q_stricmp(ent->classname, "func_door_rotating")))
				continue;

			if (t == ent)
			{
#ifdef _DEVEL
				gi.dprintf ("WARNING: %s used itself.\n", t->classname);
#endif
			}
			else
			{
				if (t->use)
					t->use (t, ent, activator);
			}
			if (!ent->inuse)
			{
#ifdef _DEVEL
				gi.dprintf("entity was removed while using targets\n");
#endif
				return;
			}
		}
	}
}

qboolean PossessCorrectItem(edict_t *ent, gitem_t *item)
{
	edict_t	*t;

	if(!ent->target_ent)
	{
		return(false);
	}
	ent = ent->target_ent;
	t = NULL;
	while ((t = G_Find (t, FOFS(targetname), ent->target)))
	{
		// doors fire area portals in a specific way
		if (!Q_stricmp(t->classname, "func_areaportal") &&
			(!Q_stricmp(ent->classname, "func_door") || !Q_stricmp(ent->classname, "func_door_rotating")))
			continue;

		if(t->item == item)
		{
			return(true);
		}
	}
	return(false);
}

/*
=============
VectorToString

This is just a convenience function
for printing vectors
=============
*/
char	*vtos (vec3_t v)
{
	static	int		index;
	static	char	str[8][32];
	char	*s;

	// use an array so that multiple vtos won't collide
	s = str[index];
	index = (index + 1)&7;

	Com_sprintf (s, 32, "(%i %i %i)", (int)v[0], (int)v[1], (int)v[2]);

	return s;
}


vec3_t VEC_UP		= {0.0, -1.0, 0.0};
vec3_t MOVEDIR_UP	= {0.0, 0.0, 1.0};
vec3_t VEC_DOWN		= {0.0, -2.0, 0.0};
vec3_t MOVEDIR_DOWN	= {0.0, 0.0, -1.0};

void G_SetMovedir (vec3_t angles, vec3_t movedir)
{
	if (VectorCompare (angles, VEC_UP))
	{
		VectorCopy (MOVEDIR_UP, movedir);
	}
	else if (VectorCompare (angles, VEC_DOWN))
	{
		VectorCopy (MOVEDIR_DOWN, movedir);
	}
	else
	{
		AngleVectors (angles, movedir, NULL, NULL);
	}

	VectorClear (angles);
}


float vectoyaw (vec3_t vec)
{
	float	yaw;
	
	if (vec[YAW] == 0 && vec[PITCH] == 0)
		yaw = 0;
	else
	{
		yaw = (float) (atan2(vec[YAW], vec[PITCH]) * (180 / M_PI));
		if (yaw < 0)
			yaw += 360;
	}

	return yaw;
}

char *G_CopyString (char *in)
{
	char	*out;
	
	out = gi.TagMalloc (strlen(in)+1, TAG_LEVEL);
	strcpy (out, in);
	return out;
}


void G_InitEdict (edict_t *self)
{
	self->s.clientEffects.buf = NULL;
	self->s.clientEffects.bufSize = 0;
	self->s.clientEffects.freeBlock = 0;
	self->s.clientEffects.numEffects = 0;

	self->inuse = true;
	self->movetype = PHYSICSTYPE_NONE;
	self->classname = "noclass";
	self->gravity = 1.0F;
	self->friction = 1.0F;
	self->elasticity = ELASTICITY_SLIDE;
	self->s.number = self - g_edicts;
	self->s.scale = 1.0F;
	self->msgHandler = NULL;
	self->svflags = 0;
	self->client_sent = 0;
	self->just_deleted = 0;
	self->reflected_time = level.time;

}

/*
=================
G_Spawn

Either finds a free edict, or allocates a new one.
Try to avoid reusing an entity that was recently freed, because it
can cause the client to think the entity morphed into something else
instead of being removed and recreated, which can cause interpolated
angles and bad trails.
=================
*/
edict_t *G_Spawn (void)
{
	int			i;
	edict_t		*e;
	//static unsigned int entID = 0;

	e = &g_edicts[(int)maxclients->value+1];
	for(i=maxclients->value + 1; i < globals.num_edicts; ++i, ++e)
	{
		// the first couple seconds of server time can involve a lot of
		// freeing and allocating, so relax the replacement policy
		if(!e->inuse && e->freetime <= level.time)
		{
			G_InitEdict (e);

			++e->s.usageCount;
			return e;
		}
	}

	if (i == game.maxentities)
	{
		assert(0);
		gi.error ("ED_Alloc: Spawning more than %d edicts", game.maxentities);
	}
		
	globals.num_edicts++;
	G_InitEdict (e);
	return e;
}

/*
=================
G_FreeEdict

Marks the edict as free
=================
*/
void G_FreeEdict(edict_t *self)
{
	SinglyLinkedList_t msgs;
	char *temp;
	unsigned int	usageCount;
	int		server_seen;
	int		entnum;

	gi.unlinkentity (self);		// unlink from world

	// From Quake2 3.17 code release.

	if ((self - g_edicts) <= (maxclients->value + BODY_QUEUE_SIZE))
	{
#ifdef _DEVEL
		gi.dprintf("tried to free special edict\n");
#endif
		return;
	}

	// Start non-quake2.

	// Portals need to be marked as open even if they are freed in deathmatch, only when deliberately removed for netplay.
	if (self->classname && level.time <= 0.2)			// Just upon startup
	{
		if (Q_stricmp(self->classname, "func_areaportal") == 0)
			gi.SetAreaPortalState (self->style, true);
	}

	if(self->s.effects & EF_JOINTED)
	{
		FreeSkeleton(self->s.rootJoint);
	}

	if(self->s.clientEffects.buf)
	{
		temp = self->s.clientEffects.buf; // buffer needs to be stored to be cleared by the engine
	}
	else
	{
		temp = NULL;
	}

	msgs = self->msgQ.msgs;
	usageCount = self->s.usageCount;
	server_seen = self->client_sent;
	entnum = self->s.number;

	// End non-quake2.

	memset(self, 0, sizeof(*self));

	// Start non-quake2.

	self->s.usageCount = usageCount;
	self->msgQ.msgs = msgs;
	self->s.clientEffects.buf = temp;
	self->just_deleted = SERVER_DELETED;
	self->client_sent = server_seen;
	self->s.number = entnum;

	// End non-quake2.

	self->classname = "freed";
	self->freetime = level.time + 2.0;
	self->inuse = false;
	self->s.skeletalType = SKEL_NULL;

	self->svflags = SVF_NOCLIENT;	// so it will get removed from the client properly
}


/*
============
G_TouchTriggers

============
*/
void	G_TouchTriggers (edict_t *ent)
{
	int			i, num;
	edict_t		*touch[MAX_EDICTS], *hit;

	// dead things don't activate triggers!
	if ((ent->client || (ent->svflags & SVF_MONSTER)) && (ent->health <= 0))
		return;

	num = gi.BoxEdicts (ent->absmin, ent->absmax, touch
		, MAX_EDICTS, AREA_TRIGGERS);

	// be careful, it is possible to have an entity in this
	// list removed before we get to it (killtriggered)
	for (i=0 ; i<num ; i++)
	{
		hit = touch[i];
		if (!hit->inuse)
			continue;
		if (!hit->touch)
			continue;
		hit->touch (hit, ent, NULL, NULL);
	}
}

/*
============
G_TouchSolids

Call after linking a new trigger in during gameplay
to force all entities it covers to immediately touch it
============
*/
void	G_TouchSolids (edict_t *ent)
{
	int			i, num;
	edict_t		*touch[MAX_EDICTS], *hit;

	num = gi.BoxEdicts (ent->absmin, ent->absmax, touch
		, MAX_EDICTS, AREA_SOLID);

	// be careful, it is possible to have an entity in this
	// list removed before we get to it (killtriggered)
	for (i=0 ; i<num ; i++)
	{
		hit = touch[i];
		if (!hit->inuse)
			continue;
		if (ent->touch)
			ent->touch (hit, ent, NULL, NULL);
		if (!ent->inuse)
			break;
	}
}




/*
==============================================================================

Kill box

==============================================================================
*/

/*
=================
KillBox

Kills all entities that would touch the proposed new positioning
of ent.  Ent should be unlinked before calling this!
=================
*/
qboolean KillBox (edict_t *ent)
{
	edict_t *current=NULL;
	vec3_t	mins, maxs;

	// since we can't trust the absmin and absmax to be set correctly on entry, I'll create my own versions

	VectorAdd(ent->s.origin, ent->mins, mins);
	VectorAdd(ent->s.origin, ent->maxs, maxs);

	while (1)
	{
		current = findinbounds(current, mins, maxs);

		// don't allow us to kill the player
		if(current == ent)
			continue;

		// we've checked everything
		if(!current)
			break;

		// nail it
		if (current->takedamage)
			T_Damage (current, ent, ent, vec3_origin, ent->s.origin, vec3_origin, 100000, 0, 
					  DAMAGE_NO_PROTECTION|DAMAGE_AVOID_ARMOR|DAMAGE_HURT_FRIENDLY,MOD_TELEFRAG);
	
	}

	return true;		// all clear
}

/*
ClearBBox

returns true if there is nothing in you BBOX
*/

qboolean ClearBBox (edict_t *self)
{
	vec3_t	top, bottom, mins, maxs;
	trace_t	trace;
	VectorSet(mins, self->mins[0], self->mins[1], 0);
	VectorSet(maxs, self->maxs[0], self->maxs[1], 1);
	VectorSet(bottom, self->s.origin[0], self->s.origin[1], self->absmin[2]);
	VectorSet(top, self->s.origin[0], self->s.origin[1], self->absmax[2] - 1);

	gi.trace(top, mins, maxs, bottom, self, self->clipmask,&trace);
	if(trace.startsolid || trace.allsolid)
		return false;

	if(trace.fraction == 1.0)
		return true;

	return false;
}

/*
=================
oldfindradius

Returns entities that have origins within a spherical area

oldfindradius (origin, radius)
=================
*/
edict_t *oldfindradius (edict_t *from, vec3_t org, float rad)
{
	vec3_t	eorg;
	int		j;

	if (!from)
		from = g_edicts;
	else
		from++;
	for ( ; from < &g_edicts[globals.num_edicts]; from++)
	{
		if (!from->inuse)
			continue;
		if (from->solid == SOLID_NOT)
			continue;
		for (j=0 ; j<3 ; j++)
			eorg[j] = org[j] - (from->s.origin[j] + (from->mins[j] + from->maxs[j])*0.5);
		if (VectorLength(eorg) > rad)
			continue;
		return from;
	}

	return NULL;
}



// ========================================
// LinkMissile(edict_t *self)
//
// This is a way to kinda "cheat" the system.
// We don't want missiles to be considered for collision, 
// yet we want them to collide with other things.
// So when we link the entity (for rendering, etc) we set 
// SOLID_NOT so certain things don't happen.
// ========================================
void G_LinkMissile(edict_t *self)
{
    int oldsolid;
	
	oldsolid=self->solid;

//  self->solid=SOLID_NOT; // comment this line out for old behaviour
    gi.linkentity(self);
    self->solid=oldsolid;
}

