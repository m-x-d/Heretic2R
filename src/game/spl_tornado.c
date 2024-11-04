//
// spl_tornado.c
//
// Heretic II
// Copyright 1998 Raven Software
//
// By Jake Simpson

#include "g_local.h"
#include "fx.h"
#include "vector.h"
#include "random.h"
#include "decals.h"
#include "Utilities.h"
#include "g_playstats.h"
#include "g_Physics.h"

#define FIST_RADIUS				2.0
#define TORN_RADIUS				10.0
#define TORN_EFFECT_RADIUS		100.0		
#define TORN_KNOCKBACK_SCALE	200.0
#define TORN_KNOCKBACK_BASE		250.0
#define TORN_MASS_FACTOR		200.0
extern void AlertMonsters (edict_t *self, edict_t *enemy, float lifetime, qboolean ignore_shadows);
static void tornboltTouch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surface);
extern edict_t *findringradius (edict_t *from, vec3_t org, float rad, edict_t *ringent);
void SpellCasttornbolt(edict_t *caster, vec3_t startpos, vec3_t aimangles, vec3_t aimdir, float value);

// do the think for the tornado ring
static void TornadoThink(edict_t *self)
{
	edict_t	*ent = NULL;
	vec3_t	vel, hitloc;
	float	scale;
	int		damage = TORN_DAMAGE;
	vec3_t	endpos, angles;

	// stolen wholesale from the ring of repulsion. Cheers Pat :)
	while(ent = findringradius(ent, self->s.origin, TORN_EFFECT_RADIUS, self))
	{
		if (ent->mass && ent!=self->owner)
   		{
   			VectorSubtract(ent->s.origin, self->s.origin, vel);
   			scale = (TORN_EFFECT_RADIUS - VectorLength(vel)) 
   						* (TORN_KNOCKBACK_SCALE/TORN_EFFECT_RADIUS) 
   						* sqrt(TORN_MASS_FACTOR / ent->mass)
   						+ TORN_KNOCKBACK_BASE;
			scale *= 20; // just for yucks
			VectorNormalize(vel);
			Vec3ScaleAssign(scale, vel);
			vel[2] += 200;
   			// Vel is just passing the direction of the knockback.
   			QPostMessage(ent, MSG_REPULSE, PRI_DIRECTIVE, "fff", vel[0], vel[1], vel[2] );
			damage = TORN_DAMAGE;
			// double the damage if this tornado is powered up
   			if (ent->takedamage)
   			{
				// Do a nasty looking blast at the impact point
				gi.CreateEffect(&ent->s, FX_LIGHTNING_HIT, CEF_OWNERS_ORIGIN, NULL, "t", ent->velocity);
				VectorClear(ent->velocity);

				// no damage if reflection is on.
				if(EntReflecting(ent, true, true))
					damage = 0;

   				VectorMA(ent->s.origin, -ent->maxs[0], vel, hitloc);
   				if (ent->movetype != PHYSICSTYPE_NONE)
   					T_Damage (ent, ent, self->targetEnt, vel, hitloc, vec3_origin, damage, 600, DAMAGE_RADIUS | DAMAGE_SPELL,MOD_TORN);
   				else
   					T_Damage (ent, ent, self->targetEnt, vel, hitloc, vec3_origin, damage, 600, DAMAGE_RADIUS | DAMAGE_SPELL,MOD_TORN);
   			}
   		}
	}

	if (self->jump_time < level.time)
	{
		VectorSet(angles, flrand(0.0, 6.28), flrand(0.0, 6.28), 0);
		DirFromAngles(angles, endpos);
		Vec3ScaleAssign((flrand(0,110.0)), endpos);
		endpos[2] = 100.0;
		VectorAdd(endpos, self->s.origin, endpos);
   		gi.CreateEffect(NULL, FX_LIGHTNING, 0, 
   			self->s.origin, "vbb", endpos, (byte)RED_RAIN_LIGHTNING_WIDTH, (byte)0);
   		gi.sound(self,CHAN_WEAPON,gi.soundindex("weapons/Lightning.wav"),1,ATTN_NORM,0);
		self->jump_time = level.time + flrand(0.2, 1.0);
	}

	self->nextthink = level.time + 0.1;
	if (self->count < level.time)
	{
		self->nextthink = level.time + 1.0;
		self->think = G_SetToFree;
		self->s.effects &= ~EF_SPEED_ACTIVE;
		self->s.effects &= ~EF_HIGH_MAX;
	}
}

// times up, create the tornado effect
void create_tornado(edict_t *tornado)
{
	int	flags = 0;

	tornado->classname = "Spell_Tornado";
	tornado->think = TornadoThink;
	tornado->nextthink = level.time + 0.1;
	tornado->timestamp = level.time;
	tornado->count = level.time + SPIN_DUR;
	tornado->gravity = 0;
	tornado->alert_time = level.time + flrand(0.6, 1.2);
	tornado->s.sound = gi.soundindex("weapons/tornadospell.wav");
	tornado->s.sound_data = (255 & ENT_VOL_MASK) | ATTN_NORM;
	tornado->s.effects |= EF_SPEED_ACTIVE;
	tornado->jump_time = level.time + flrand(0.2, 1.0);
   
	tornado->PersistantCFX = gi.CreatePersistantEffect(&tornado->s, FX_TORNADO, CEF_BROADCAST|CEF_OWNERS_ORIGIN | flags , NULL, "");

}	
		
// we just cast/dropped the tornado, set up a timer so it doesn't erupt immediately and hit the caster
void SpellCastDropTornado(edict_t *caster, vec3_t startpos, vec3_t aimangles, vec3_t aimdir, float value)
{
	edict_t		*tornado;
	trace_t		trace;
	vec3_t		end;
	edict_t		*spot = NULL;
	int			flags = 0;
	float		length;
	vec3_t		diff;
	int			g_type = 0;
	char		*spawn_check[3] =
	{{"info_player_start"},
	 {"info_player_deathmatch"},
	 {"info_player_coop"}};

	tornado = G_Spawn();
	tornado->movetype = PHYSICSTYPE_NONE;
	tornado->classname = "Spell_Tornado_time";
	tornado->think = create_tornado;
	tornado->nextthink = level.time + TORN_DUR;
	tornado->takedamage = DAMAGE_NO;

	tornado->owner = caster;

	// use the speed active ef_flag to tell the client effect when the effect is over
	tornado->s.effects |= EF_ALWAYS_ADD_EFFECTS ;
	tornado->svflags |= SVF_ALWAYS_SEND;
	tornado->solid = SOLID_NOT;
	tornado->clipmask = MASK_SOLID;
	tornado->targetEnt = caster;

	VectorCopy(startpos, tornado->s.origin);
	tornado->s.origin[2] += 1.0;
	VectorCopy (tornado->s.origin, end);
	end[2] -= 256;
	
	gi.linkentity (tornado);

	gi.trace (tornado->s.origin, NULL, NULL, end, tornado, MASK_SOLID,&trace);

	tornado->s.origin[2] += 3.0;
	VectorCopy(trace.endpos, tornado->s.origin);

	// check to see if we are over a spawn point - this won't catch specific teleport arrival points, but will get some of them
	if (coop->value)
		g_type = 2;
	else
	if (deathmatch->value)
		g_type = 1;

	// go search out an spots there are.
	while ((spot = G_Find (spot, FOFS(classname), spawn_check[g_type])) != NULL)
	{
		// if we are over a spawn spot, explode the tornado immediately.
		VectorSubtract(spot->s.origin, tornado->s.origin, diff);
		length = VectorLength(diff);
		if (length < 80)
		{
			tornado->think = G_SetToFree;
			tornado->nextthink = level.time + 0.1;
			gi.CreateEffect(NULL, FX_TORNADO_BALL_EXPLODE, 0 , tornado->s.origin, "");
			return;
		}
	}

	gi.CreateEffect(&tornado->s, FX_TORNADO_BALL, CEF_OWNERS_ORIGIN | flags , NULL, "");
}			


// TORNADO BOLT FUNCTIONS


// ****************************************
// Creation functions
// ****************************************
void tornboltInitThink(edict_t *self)
{
	self->svflags |= SVF_NOCLIENT;
	self->think = NULL;
}

void Createtornbolt(edict_t *tornbolt)
{
	tornbolt->s.effects |= EF_ALWAYS_ADD_EFFECTS;
	tornbolt->svflags |= SVF_ALWAYS_SEND;
	tornbolt->movetype = MOVETYPE_FLYMISSILE;
	tornbolt->s.scale = 1.0;

	tornbolt->touch = tornboltTouch;
	tornbolt->think = tornboltInitThink;
	tornbolt->classname = "Spell_tornbolt";
	tornbolt->nextthink = level.time + 0.1;
	VectorSet(tornbolt->mins, -FIST_RADIUS, -FIST_RADIUS, -FIST_RADIUS);
	VectorSet(tornbolt->maxs, FIST_RADIUS, FIST_RADIUS, FIST_RADIUS);

	tornbolt->solid = SOLID_BBOX;
	tornbolt->clipmask = MASK_SHOT;
}

edict_t *tornboltReflect(edict_t *self, edict_t *other, vec3_t vel)
{
	edict_t	*tornbolt;

	// create a new missile to replace the old one - this is necessary cos physics will do nasty things
	// with the existing one,since we hit something. Hence, we create a new one totally.
	tornbolt = G_Spawn();

	// copy everything across
	VectorCopy(self->s.origin, tornbolt->s.origin);
	Createtornbolt(tornbolt);
	VectorCopy(vel, tornbolt->velocity);
	VectorNormalize2(vel, tornbolt->movedir);
	AnglesFromDir(tornbolt->movedir, tornbolt->s.angles);
	tornbolt->owner = other;
	tornbolt->enemy = self->owner;
	tornbolt->flags |= (self->flags & FL_NO_KNOCKBACK);
	tornbolt->reflect_debounce_time = self->reflect_debounce_time -1; //so it doesn't infinitely reflect in one frame somehow
	tornbolt->reflected_time=self->reflected_time;
	G_LinkMissile(tornbolt); 

	// create new trails for the new missile
	gi.CreateEffect(&tornbolt->s, FX_WEAPON_FLYINGFIST, CEF_OWNERS_ORIGIN | CEF_FLAG6, NULL,
					"t", tornbolt->velocity);

	// kill the existing missile, since its a pain in the ass to modify it so the physics won't screw it. 
	G_SetToFree(self);

	// Do a nasty looking blast at the impact point
	gi.CreateEffect(&tornbolt->s, FX_LIGHTNING_HIT, CEF_OWNERS_ORIGIN, NULL, "t", tornbolt->velocity);

	return(tornbolt);
}



// ************************************************************************************************
// tornboltTouch
// ************************************************************************************************

static void tornboltTouch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surface)
{
	int			damage;
	vec3_t		planedir;
	int			flags = 0;

	if(other == self->owner)
	{
		return;
	}
	if(surface && (surface->flags & SURF_SKY))
	{
		SkyFly(self);
		return;
	}

	// has the target got reflection turned on ?
	if (self->reflect_debounce_time)
	{
		if(EntReflecting(other, true, true))
		{
			Create_rand_relect_vect(self->velocity, self->velocity);
			// scale speed down
			Vec3ScaleAssign(FLYING_FIST_SPEED/2, self->velocity);
			tornboltReflect(self, other, self->velocity);
			return;
		}
	}

	AlertMonsters (self, self->owner, 1, false);
	if(other->takedamage)
	{
		if(deathmatch->value)
			damage = irand(FIREBALL_DAMAGE_MIN/2, FIREBALL_DAMAGE_MAX/2);
		else
			damage = irand(FIREBALL_DAMAGE_MIN, FIREBALL_DAMAGE_MAX);
		T_Damage(other, self, self->owner, self->movedir, self->s.origin, plane->normal, damage, damage, DAMAGE_SPELL,MOD_FIREBALL);
	}
	else
	{
		VectorMA(self->s.origin, -8.0, self->movedir, self->s.origin);
	}

	// Attempt to apply a scorchmark decal to the thing I hit.
	if(IsDecalApplicable(self, other, self->s.origin, surface, plane, planedir))
	{
		flags |= CEF_FLAG6;
	}

	gi.CreateEffect(NULL, FX_WEAPON_FLYINGFISTEXPLODE, flags, self->s.origin, "d", self->movedir);

	G_SetToFree(self);
}


// ************************************************************************************************
// SpellCasttornbolt
// ************************************************************************************************


void SpellCasttornbolt(edict_t *caster, vec3_t startpos, vec3_t aimangles, vec3_t aimdir, float value)
{
	edict_t		*tornbolt;
	trace_t		trace;
	vec3_t		forward, endpos;

	// Spawn the tornado blot / flying-fist (fireball)
	tornbolt = G_Spawn();

	gi.sound(caster, CHAN_WEAPON, gi.soundindex("weapons/tornboltCast.wav"), 1.0, ATTN_NORM, 0);

	Createtornbolt(tornbolt);
	tornbolt->reflect_debounce_time = MAX_REFLECT;
	VectorCopy(startpos, tornbolt->s.origin);	

	//Check ahead first to see if it's going to hit anything at this angle
	AngleVectors(aimangles, forward, NULL, NULL);
	VectorMA(tornbolt->s.origin, FLYING_FIST_SPEED, forward, endpos);
	gi.trace(startpos, vec3_origin, vec3_origin, endpos, caster, MASK_MONSTERSOLID,&trace);
	if(trace.ent && ok_to_autotarget(caster, trace.ent))
	{//already going to hit a valid target at this angle- so don't autotarget
		VectorScale(forward, FLYING_FIST_SPEED, tornbolt->velocity);
	}
	else
	{//autotarget current enemy
		GetAimVelocity(caster->enemy, tornbolt->s.origin, FLYING_FIST_SPEED / 2, aimangles, tornbolt->velocity);
	}
	tornbolt->owner = caster;
	tornbolt->enemy = caster->enemy;
	// Remember velocity in case we have to reverse it
	VectorNormalize2(tornbolt->velocity, tornbolt->movedir);

	G_LinkMissile(tornbolt); 

	// Make sure we don`t start in a solid
	gi.trace(caster->s.origin, vec3_origin, vec3_origin, tornbolt->s.origin, caster, MASK_PLAYERSOLID,&trace);
	if (trace.startsolid || trace.fraction < 1.0)
	{
		VectorCopy(trace.endpos, tornbolt->s.origin);
		tornboltTouch(tornbolt, trace.ent, &trace.plane, trace.surface);
		return;
	}
	// Spawn effect after it has been determined it has not started in wall
	// This is so it won`t try to remove it before it exists
	gi.CreateEffect(&tornbolt->s, FX_WEAPON_FLYINGFIST, CEF_OWNERS_ORIGIN | CEF_FLAG6, NULL,
					"t", tornbolt->velocity);
}

