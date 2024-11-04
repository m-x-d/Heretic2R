//
// Heretic II
// Copyright 1998 Raven Software
//

#include "g_local.h"
#include "fx.h"
#include "vector.h"
#include "angles.h"
#include "matrix.h"
#include "g_volume_effect.h"
#include "Utilities.h"
#include "g_ClassStatics.h"
#include "g_Physics.h"
#include "g_playstats.h"
#include "decals.h"
#include "random.h"
#include "p_item.h"
#include "g_teleport.h"

#define MACEBALL_UPSPEED	200.0F
#define MACEBALL_SPEED		250.0F
#define MACEBALL_DOWNSPEED	150.0F
#define MACEBALL_RADIUS		16.0
#define MACEBALL_SCALE		0.17
#define MACEBALL_SCALEINC	0.03
#define MACEBALL_ROT		(360*0.02)

#define MACEBALL_SEARCHRAD	500.0

extern player_export_t	playerExport;	// interface to player DLL.

// ****************************************************************************
// Maceball think
// ****************************************************************************

void MaceballThink(edict_t *self)
{	
	vec3_t movevect;
	qboolean killme=false;

//	self->s.angles[YAW] += MACEBALL_ROT*0.5;
//	self->s.angles[PITCH] += MACEBALL_ROT*2.0;

	if (self->s.scale < MACEBALL_SCALE)
	{
		self->s.scale += MACEBALL_SCALEINC;
		if (self->s.scale > MACEBALL_SCALE)
			self->s.scale = MACEBALL_SCALE;
	}

	// Check the NOTARGET flag to see if the mace should readjust to a new target.
	if (self->flags & FL_NOTARGET)
	{	// Head towards its enemy
		self->flags &= ~FL_NOTARGET;
		if (self->enemy)
		{
			VectorScale(self->movedir, MACEBALL_UPSPEED, self->velocity);
			VectorSubtract(self->enemy->s.origin, self->s.origin, movevect);
			VectorNormalize(movevect);
			VectorMA(self->velocity, MACEBALL_SPEED, movevect, self->velocity);
		}

		if (self->velocity[2] > 0)
		{	// Adjust anyway so that the ball has a consistent bounce height.
			self->velocity[2] = MACEBALL_UPSPEED;
		}
	}

	VectorCopy(self->s.origin, self->last_org);

	self->nextthink = level.time + 0.1;

	// Now check if we should die soon.
	if (killme || self->deadflag == DEAD_DYING || self->touch_debounce_time + MACEBALL_EXTRALIFE <= level.time)
	{
		gi.sound(self, CHAN_WEAPON, gi.soundindex("weapons/MaceBallDeath.wav"), 2, ATTN_NORM, 0);
		gi.CreateEffect(NULL,
						FX_WEAPON_MACEBALLEXPLODE,
						0,
						self->s.origin,
						"d",
						self->velocity);
		G_SetToFree(self);
	}
}

// ****************************************************************************
// Maceball bounce
// ****************************************************************************

static vec3_t BoxNormals[6] = 
{
	{ 0.0, 0.0, 1.0 },		// up
	{ 0.0, 0.0, -1.0 },		// down
	{ 0.0, 1.0, 0.0 },		// left
	{ 0.0, -1.0, 0.0 },		// right
	{ 1.0, 0.0, 0.0 },		// front
	{ -1.0, 0.0, 0.0 },		// back
};

void GetCollisionPoint(vec3_t velocity, vec3_t origin, vec_t size, vec3_t point)
{
	int		maxn = 1;
	int		i;
	float	theta;
	float	max = -1.0;

	for(i = 0; i < 6; i++)
	{
		theta = DotProduct(velocity, BoxNormals[i]);
		if(theta > max)
		{
			max = theta;
			maxn = i;
		}
	}
	VectorMA(origin, size, BoxNormals[maxn], point);
}

void MaceballBounce(edict_t *self, trace_t *trace)
{
	vec3_t		point;
	qboolean	done = false;
	qboolean	targetowner = false;

	vec3_t movevect;

	// did we hit something we can destroy ?
	if (trace->ent && trace->ent->takedamage && trace->ent->health>0)
	{
		VectorNormalize2(self->velocity, movevect);
		if(EntReflecting(trace->ent, true, true))
		{
			// do nothing except bounce if we hit someone who's reflecting
			// and make whoever it bounced off the owner of the ball now
			self->enemy = self->owner;
			self->owner = trace->ent;
		}
		else
		if (trace->ent->svflags & SVF_BOSS)
		{
			T_Damage(trace->ent, self, self->owner, movevect, trace->endpos, movevect, 
					MACEBALL_BOSS_DAMAGE, MACEBALL_BOSS_DAMAGE, 0,MOD_P_IRONDOOM);
			self->deadflag = DEAD_DYING;
		}
		else
		{
   			int no_teleport = 1;
   			gitem_t	*Defence,
   						*ManaItem;
   			int		ManaIndex;
   			int		Quantity;

   			// can we teleport the player out of danger ?
   			// firstly, are we a player ?
   			if (trace->ent->client)
   			{
   				Defence = trace->ent->client->playerinfo.pers.defence;
   				Quantity = playerExport.p_itemlist[12].quantity;
   				if(Defence->ammo && Quantity)
   				{
   					// do we have enough mana to teleport ?
   					ManaItem = P_FindItem(Defence->ammo);
   					ManaIndex = ITEM_INDEX(ManaItem);
   					if (trace->ent->client->playerinfo.pers.inventory.Items[ManaIndex]/Quantity > 0)
   					{
   						// yes, do we actually have a teleport ?
						if (trace->ent->client->playerinfo.pers.inventory.Items[13])
						{
   							SpellCastTeleport(trace->ent, trace->ent->s.origin, NULL, NULL, 0.0F);
   							if (!deathmatch->value || (deathmatch->value && !((int)dmflags->value & DF_INFINITE_MANA)))
   								trace->ent->client->playerinfo.pers.inventory.Items[ManaIndex] -= Quantity;
   							no_teleport = 0;
						}
   					}
   				}
   				
   			}
   			if (no_teleport)
   			{
   				T_Damage(trace->ent, self, self->owner, movevect, trace->endpos, movevect, 
   	 				self->dmg, self->dmg, 0,MOD_P_IRONDOOM);
				// if we hit a player or a monster, kill this maceball
				if (trace->ent->client || (trace->ent->svflags & SVF_MONSTER))
					self->deadflag = DEAD_DYING;
   			}
   		}
	}

	// If it's time is up, then kill it.
	if (self->touch_debounce_time <= level.time)
	{
		self->deadflag = DEAD_DYING;
		return;
	}

	if (self->health == 1)
		targetowner = true;

	gi.sound(self, CHAN_WEAPON, gi.soundindex("weapons/MaceBallBounce.wav"), 1, ATTN_NORM, 0);

	// Do a spiffy hit effect
	GetCollisionPoint(self->velocity, self->s.origin, self->maxs[0], point);
	gi.CreateEffect(NULL, FX_WEAPON_MACEBALLBOUNCE, 0, point, "d", trace->plane.normal);

	if (trace->plane.normal[2] < .5)
	{	// Hit a vertical surface
		return;		// Don't track a target.
	}

	self->flags |= FL_NOTARGET;		// This indicates to the thinker to revise the trajectory.

	// It should track its target
	if (self->enemy==NULL || self->enemy->health<=0)
	{	// Find new enemy
		self->enemy = FindSpellTargetInRadius(self, MACEBALL_SEARCHRAD, self->s.origin,
												self->mins, self->maxs);

		if (self->enemy == NULL)	// no target, don't head for a target.
		{
			self->health = 1;
			return;
		}
	}
	// Since we have an enemy, set the flag to readjust next think.
	VectorCopy(trace->plane.normal, self->movedir);
}



// ****************************************************************************
// SpellCastMaceballs
// ****************************************************************************

void SpellCastMaceball(edict_t *caster, vec3_t startpos, vec3_t aimangles, vec3_t unused, float value)
{
	edict_t			*ball;
	trace_t			trace;
	vec3_t			orig;

	ball = G_Spawn();

	VectorCopy(startpos, ball->s.origin);

	VectorSet(ball->mins, -MACEBALL_RADIUS, -MACEBALL_RADIUS, -MACEBALL_RADIUS);
	VectorSet(ball->maxs, MACEBALL_RADIUS, MACEBALL_RADIUS, MACEBALL_RADIUS);

	VectorCopy(startpos, orig);
	GetAimVelocity(caster->enemy, orig, MACEBALL_SPEED, aimangles, ball->velocity);

	// Throw the ball down.
	ball->velocity[2] = -MACEBALL_DOWNSPEED;
	VectorAdd(ball->velocity, caster->velocity, ball->velocity);
	// If the caster has an enemy, then aim it at the enemy
	ball->enemy = caster->enemy;

	// Spin it forward
	ball->mass = 2500;
	ball->elasticity = ELASTICITY_MACEBALL;
	ball->friction = 0;
	ball->gravity = MACEBALL_GRAVITY;
	ball->svflags = SVF_DO_NO_IMPACT_DMG;

	ball->movetype = PHYSICSTYPE_STEP;

	ball->solid = SOLID_BBOX;
	ball->clipmask = MASK_MONSTERSOLID;
	ball->owner = caster;
	ball->think = MaceballThink;
	ball->nextthink = level.time + 0.1;
	ball->bounced = MaceballBounce;
	ball->classname = "Spell_Maceball";
	ball->touch_debounce_time = level.time + MACEBALL_LIFE;		// The ball will expire the next bounce after this one.
	ball->s.modelindex = gi.modelindex("models/spells/maceball/tris.fm");
	ball->s.scale = MACEBALL_SCALEINC;
	ball->dmg = MACEBALL_DAMAGE;
	ball->health = 2;
	ball->s.effects |= EF_MACE_ROTATE;
	VectorCopy(ball->s.origin, ball->last_org);

	gi.linkentity(ball);

	gi.trace(caster->s.origin, ball->mins, ball->maxs, startpos, caster, MASK_PLAYERSOLID,&trace);
	if (trace.startsolid)
	{	// Spawning in something, give up now, and kill the thang.
		VectorClear(ball->velocity);

		gi.sound(ball, CHAN_WEAPON, gi.soundindex("weapons/MaceBallDeath.wav"), 2, ATTN_NORM, 0);
		gi.CreateEffect(NULL,
						FX_WEAPON_MACEBALLEXPLODE,
						0,
						ball->s.origin,
						"d",
						ball->velocity);
		G_SetToFree(ball);
		return;
	}
	else if (trace.fraction < 0.99)
	{	// Hit something along the way from the center of the player to here.
		VectorCopy(trace.endpos, ball->s.origin);
		VectorCopy(trace.endpos, ball->last_org);
	}
	gi.sound(caster, CHAN_WEAPON, gi.soundindex("weapons/MaceBallCast.wav"), 1, ATTN_NORM, 0);
}


// end

