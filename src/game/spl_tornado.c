//
// spl_tornado.c
//
// Copyright 1998 Raven Software
//

#include "spl_tornado.h" //mxd
#include "spl_BlueRing.h" //mxd
#include "g_combat.h" //mxd
#include "g_playstats.h"
#include "FX.h"
#include "Random.h"
#include "Vector.h"
#include "g_local.h"

#define TORN_EFFECT_RADIUS		100.0f	
#define TORN_KNOCKBACK_SCALE	200.0f
#define TORN_KNOCKBACK_BASE		250.0f
#define TORN_MASS_FACTOR		200.0f

// do the think for the tornado ring
static void TornadoThink(edict_t *self)
{
	edict_t	*ent = NULL;
	vec3_t	vel, hitloc;
	float	scale;
	int		damage = TORN_DAMAGE;
	vec3_t	endpos, angles;

	// stolen wholesale from the ring of repulsion. Cheers Pat :)
	while(ent = FindRingRadius(ent, self->s.origin, TORN_EFFECT_RADIUS, self))
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