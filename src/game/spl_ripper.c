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
#include "m_beast.h"

#define RIPPER_RADIUS				12.0
#define RIPPER_EXPLODE_BALL_RADIUS	8.0
#define RIPPER_MAX_DISTANCE			2000


void RipperThink(edict_t *Self);

extern void AlertMonsters (edict_t *self, edict_t *enemy, float lifetime, qboolean ignore_shadows);



//---------------------
//---------------------
//---------------------
// RIPPER
//---------------------
//---------------------
//---------------------


// ****************************************************************************
// RipperExplodeBallThink
// ****************************************************************************

static void RipperExplodeBallThink(edict_t *self)
{
	vec3_t	vel;
	vec3_t	startpos, endpos, vect, forward;
	edict_t	*tracebuddy;
	trace_t trace;
	int		numHit = 0;	// can hit no more than 8 guys...

   	VectorCopy(self->s.origin, startpos);
   	VectorCopy(self->last_org, endpos);
   	tracebuddy = self;
   	do
   	{
   		gi.trace(startpos, self->mins, self->maxs, endpos, tracebuddy, MASK_SHOT,&trace);
   		// if we hit anything that won't take damage, kill the beam
   		if (!trace.ent->takedamage)
   			break;
   		if(trace.fraction < .99 )
   		{
   			// did we hit something that reflects ?
			if(!EntReflecting(trace.ent, true, true))
			{
   				T_Damage(trace.ent, self, self->owner, self->movedir, trace.endpos, vec3_origin, 
   						self->dmg, 0, DAMAGE_SPELL | DAMAGE_EXTRA_BLOOD,MOD_IRONDOOM); 

				if (trace.ent->svflags & SVF_MONSTER)
				{
   					// What the hell, let's spawn a big gush of blood in the travelling direction.
   					VectorScale(self->movedir, RIPPER_EXPLODE_SPEED*0.25, vel);
					if(trace.ent->materialtype == MAT_INSECT)
	   					gi.CreateEffect(NULL, FX_BLOOD, CEF_FLAG8, self->last_org, "ub", vel, (byte)20);
   					else
						gi.CreateEffect(NULL, FX_BLOOD, 0, self->last_org, "ub", vel, (byte)20);

   					gi.sound(self, CHAN_WEAPON, gi.soundindex("weapons/RipperDamage.wav"), 1, ATTN_NORM, 0);
				}
			}
   			// this seems to alleviate the problem of a trace hitting the same ent multiple times...
   			VectorCopy(trace.endpos, startpos);
   			VectorSubtract(endpos, startpos, vect);
   			if(VectorLength(vect) > 16.0)
   			{
				VectorSubtract(startpos,endpos,forward);
				VectorNormalize(forward);
   				VectorMA(startpos, 16.0, forward, startpos);
   			}
   			tracebuddy = trace.ent;
   			numHit++;
   		}
   	} while((trace.fraction < .99) && !(trace.contents & MASK_SOLID) && (numHit < 6) );

	// Prevent any further transmission of this entity to clients.
	self->svflags |= SVF_NOCLIENT;

	VectorCopy(self->s.origin, self->last_org);
	self->nextthink = level.time + 0.1;
}


static void RipperExplodeBallTouch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surface)
{
	G_SetToFree(self);
}


// ****************************************************************************
// RipperImpact
// ****************************************************************************
// This is like a touch function, except since the ripper is instant now...
static void RipperImpact(edict_t *caster, edict_t *other, vec3_t startpos, vec3_t endpos, vec3_t angles)
{
	edict_t	*ripper;
	byte	makeScorch = 0;
	int		i;
	int		dmg;
	vec3_t	hitpos, fwd;
	short	ballarray[RIPPER_BALLS];
	byte	byaw;
	float	curyaw;

	// Get the forward vector for various calculations
	AngleVectors(angles, fwd, NULL, NULL);

	// did we hit someone where reflection is functional ?
	if(EntReflecting(other, true, true))
	{
		// Erg...  Do nothing right now.
	}

	AlertMonsters (caster, caster, 2, false);
	if(other && other->takedamage)
	{
		dmg = irand(RIPPER_DAMAGE_MIN, RIPPER_DAMAGE_MAX);
		VectorCopy(endpos, hitpos);
		T_Damage(other, caster, caster, fwd, endpos, fwd, dmg, dmg*2, DAMAGE_SPELL,MOD_IRONDOOM);
	}
	else
	{
		// Back off the origin for the damage a bit. We are a point and this will
		// help fix hitting base of a stair and not hurting a guy on next step up.
		VectorMA(endpos, -8.0, fwd, hitpos);
	}

	// Shoot out ripper balls
	curyaw = (angles[YAW]*ANGLE_TO_RAD) + (RIPPER_BALL_ANGLE*0.5);		
														// Add half an increment so that the balls don't come right back.
														// STORE THE CURRENT YAW IN RADIANS FOR SIN/COS OPERATIONS

	// Reduce precision to a 0-255 byte.
	byaw = (byte)(curyaw*(256.0/(M_PI*2.0)));			// The byte yaw loses lots of precision here
	curyaw = ((float)byaw)*((M_PI*2.0)/256.0);			// ...and pass this imprecision back to the float yaw.
	for (i=0; i<RIPPER_BALLS; i++)
	{
		ripper = G_Spawn();

		// Set up the angle vectors
		ripper->movedir[0] = cos(curyaw);
		ripper->movedir[1] = sin(curyaw);
		ripper->movedir[2] = 0.0;

		// Place the ball		
		VectorCopy(hitpos, ripper->s.origin);
		VectorScale(ripper->movedir, RIPPER_EXPLODE_SPEED, ripper->velocity);

		// Set up the net transmission attributes
		ripper->s.effects |= EF_ALWAYS_ADD_EFFECTS;
		ripper->svflags |= SVF_ALWAYS_SEND;
		ripper->movetype = MOVETYPE_FLYMISSILE;

		// Set up the dimentsions
		VectorSet(ripper->mins, -RIPPER_EXPLODE_BALL_RADIUS, -RIPPER_EXPLODE_BALL_RADIUS, -RIPPER_EXPLODE_BALL_RADIUS);
		VectorSet(ripper->maxs, RIPPER_EXPLODE_BALL_RADIUS, RIPPER_EXPLODE_BALL_RADIUS, RIPPER_EXPLODE_BALL_RADIUS);

		// Set up physics attributes
		ripper->solid = SOLID_BBOX;
		ripper->clipmask = MASK_SOLID;
		ripper->touch = RipperExplodeBallTouch;
		ripper->think = RipperExplodeBallThink;
		ripper->dmg = RIPPER_EXPLODE_DAMAGE;
		ripper->classname = "Spell_Ripper";
		ripper->nextthink = level.time + 0.1;

		// last_org is used for damaging things
		VectorCopy(ripper->s.origin, ripper->last_org);

		ripper->owner = caster;

		// Store the entity numbers for sending with the effect.
		ballarray[i] = ripper->s.number;

		curyaw += RIPPER_BALL_ANGLE;
	}

	gi.sound(ripper, CHAN_WEAPON, gi.soundindex("weapons/RipperImpact.wav"), 1, ATTN_NORM, 0);

	// Okay (whew) now we send the big ol' mutherlovin' effect.  There's a lot here, but believe me, this is WAY more
	//		efficient than sending out eight (well, nine with the impact) seperate effects...

	// Send it attached to the last ball...
	assert(ripper);
	assert(RIPPER_BALLS == 8);

	// So we send this with:
	//	--The position of the caster (for the launch trail effect)
	//	--The byte angle to shoot things at.
	//	--All eight entity numbers to attach things to.  
	//		Okay, so I only need seven (the eighth comes with the effect), but for consistency I'm going to send all eight.
	//	(NOTE: Further optimization would maybe pass a pitch and distance, and make the yaw precise, but that would save
	//		little and make things more confusing than they are...)
	//	Currently this sends 29 bytes plus the entity it's attached to.
	//		This is down from (12 plus the entity) times eight, plus another effect for impact, plus another for the trail.
	gi.CreateEffect(&ripper->s, FX_WEAPON_RIPPEREXPLODE, CEF_OWNERS_ORIGIN, NULL, "vbssssssss", 
			startpos, 
			byaw, 
			ballarray[0],
			ballarray[1],
			ballarray[2],
			ballarray[3],
			ballarray[4],
			ballarray[5],
			ballarray[6],
			ballarray[7]);
}


// ****************************************************************************
// SpellCastRipper
// ****************************************************************************


void SpellCastRipper(edict_t *caster, vec3_t StartPos, vec3_t AimAngles, vec3_t unused)
{
	trace_t trace;
	vec3_t	endpos, forward;
	vec3_t	mins={-RIPPER_RADIUS, -RIPPER_RADIUS, -RIPPER_RADIUS}, maxs={RIPPER_RADIUS, RIPPER_RADIUS, RIPPER_RADIUS};

	gi.sound(caster, CHAN_WEAPON, gi.soundindex("weapons/RipperFire.wav"), 1, ATTN_NORM, 0);

	// Make sure we don't spawn in a wall.
	gi.trace(caster->s.origin, mins, maxs, StartPos, caster, MASK_PLAYERSOLID,&trace);
	if (trace.startsolid || trace.fraction<.99)
	{
		RipperImpact(caster, trace.ent, caster->s.origin, trace.endpos, AimAngles);
		return;
	}
	
	// Get the forward angle
	AngleVectors(AimAngles, forward, NULL, NULL);

	// Now trace from the starting point to the final destination.
	VectorMA(StartPos, RIPPER_MAX_DISTANCE, forward, endpos);
	gi.trace(StartPos, mins, maxs, endpos, caster, MASK_SHOT,&trace);
	if(level.fighting_beast)
	{
		edict_t *ent;
		
		if(ent = check_hit_beast(StartPos, trace.endpos))
			trace.ent = ent;
	}

	RipperImpact(caster, trace.ent, StartPos, trace.endpos, AimAngles);
}


// end

