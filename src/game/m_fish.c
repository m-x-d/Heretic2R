//==============================================================================
//
// m_fish.c
//
// Heretic II
// Copyright 1998 Raven Software
//
//==============================================================================

#include "g_local.h"
#include "Utilities.h"
#include "g_DefaultMessageHandler.h"
#include "g_monster.h"
#include "fx.h"
#include "random.h"
#include "buoy.h"
#include "vector.h"

#include "m_fish.h"
#include "m_fish_anim.h"
#include "g_misc.h"
#include "m_stats.h"

extern void M_MoveFrame (edict_t *self);
void fish_hunt(edict_t *self);
void fish_think(edict_t *self);

#define WALK_TURN_ANGLE 40
#define RUN_TURN_ANGLE 70
#define BITE_DIST 32
#define FISH_FAST 160
#define FISH_HUNT (40 + FISH_FAST)
#define FISH_SLOW 100
#define FISH_ACTIVATE_DIST 3000.0
#define FISH_SKIN1			0
#define FISH_SKIN2			2

/*----------------------------------------------------------------------
  Fish Base Info
-----------------------------------------------------------------------*/
static animmove_t *animations[NUM_ANIMS] =
{
	&fish_move_bite,
	&fish_move_melee,
	&fish_move_run1,
	&fish_move_run2,
	&fish_move_run3,
	&fish_move_walk1,
	&fish_move_walk2,
	&fish_move_walk3,
	&fish_move_stand1,
	&fish_move_pain1,
	&fish_move_death,
};

static int sounds[NUM_SOUNDS];

static ClassResourceInfo_t resInfo;

void fish_dead_pain(edict_t *self, G_Message_t *msg);

// bring all our movedir angles up positive again
void reset_fish_movedir(edict_t *self)
{
	while (self->movedir[0] > 360)
		self->movedir[0] -= 360;
	while (self->movedir[0] < 0)
		self->movedir[0] += 360;

	while (self->movedir[1] > 360)
		self->movedir[1] -= 360;
	while (self->movedir[1] < 0)
		self->movedir[1] += 360;

	while (self->movedir[2] > 360)
		self->movedir[2] -= 360;
	while (self->movedir[2] < 0)
		self->movedir[2] += 360;
}


//----------------------------------------------------------------------
//  Fish Run - choose a run to use
//----------------------------------------------------------------------
void fish_run(edict_t *self)
{
	float	delta;

	delta = anglemod(self->s.angles[YAW] - self->movedir[YAW]);
	if (delta > 70 && delta <= 180)			// Look right
	{
		// tell the think function we are doing the turn, so don't play with the yaw
		self->ai_mood_flags = 1;
		self->best_move_yaw = -RUN_TURN_ANGLE;
		SetAnim(self, ANIM_RUN3);
		return;
	}
	else if (delta > 180 && delta < 290)  // Look left
	{
		// tell the think function we are doing the turn, so don't play with the yaw
		self->ai_mood_flags = 1;
		self->best_move_yaw = RUN_TURN_ANGLE;
		SetAnim(self, ANIM_RUN2);
		return;
	}
	else
	{
		// tell the think function we are NOT doing the turn
		self->ai_mood_flags = 0;
		SetAnim(self, ANIM_RUN1);
		return;
	}
}

//----------------------------------------------------------------------
//  Fish Walk - choose a walk to use
//----------------------------------------------------------------------
void fish_walk(edict_t *self)
{
	float	delta;

	delta = anglemod(self->s.angles[YAW] - self->movedir[YAW]);
	if (delta > 40 && delta <= 180)			// Look right
	{
		// tell the think function we are doing the turn, so don't play with the yaw
		self->ai_mood_flags = 1;
		self->best_move_yaw = -WALK_TURN_ANGLE;
		SetAnim(self, ANIM_WALK3);
		return;
	}
	else if (delta > 180 && delta < 20)  // Look left
	{
		// tell the think function we are doing the turn, so don't play with the yaw
		self->ai_mood_flags = 1;
		self->best_move_yaw = WALK_TURN_ANGLE;
		SetAnim(self, ANIM_WALK2);
		return;
	}
	else
	{
		// tell the think function we are NOT doing the turn
		self->ai_mood_flags = 0;
		SetAnim(self, ANIM_WALK1);
		return;
	}
}

// update the yaw on the first frame of a new animation -stop jittering
void fish_update_yaw(edict_t *self)
{
 	self->s.angles[YAW] += self->best_move_yaw;
 	self->best_move_yaw = 0;
}


//	generic 'decided on a new direction' reaction - make us select a new direction
void fish_new_direction(edict_t *self)
{
	if (!(irand(0,1)))
		self->movedir[0] = flrand(-30.0, 30.0);
	else
		self->movedir[0] = 0;
	self->movedir[1] += flrand(-45.0, 45.0);
	// bring all our movedir angles up positive again
	reset_fish_movedir(self);

	// if we change direction, we might hit the same poly we just last collided with
	self->shrine_type = 0;

	// decide which animation to use
	if (self->ai_mood == AI_MOOD_WANDER)
	{
		self->speed = FISH_FAST * self->old_yaw;
		fish_run(self);
	}
	else
	{
		self->speed = FISH_SLOW * self->old_yaw;
		fish_walk(self);
	}
}

//	generic 'hit something' reaction - make us bounce off in a new direction
void fish_bounce_direction(edict_t *self)
{
	// reverse our direction with some randomness in the angles too
	VectorCopy(self->s.angles, self->movedir);
	// reverse our direction
	self->movedir[YAW] += 180;
	self->movedir[PITCH] = 0-self->movedir[PITCH];
	// give us some randomness to my return
	self->movedir[YAW] += (irand(-15,15));
	self->movedir[PITCH] += (irand(-5,5));

	// bring all our movedir angles up positive again
	reset_fish_movedir(self);

	// decide which animation to use
	if (self->ai_mood == AI_MOOD_WANDER)
	{
		self->speed = FISH_FAST * self->old_yaw;
		fish_run(self);
	}
	else
	{
		self->speed = FISH_SLOW * self->old_yaw;
		fish_walk(self);
	}
}

/*
===============
M_ChangeFishYaw

===============
*/
float M_ChangeFishYaw (edict_t *ent)
{
	float	ideal;
	float	current;
	float	move;
	float	speed;
	
	current = anglemod(ent->s.angles[YAW]);
	ideal = ent->movedir[YAW];

	if (current == ideal)
		return false;

	move = ideal - current;
	speed = ent->yaw_speed;
	if (ideal > current)
	{
		if (move >= 180)
			move = move - 360;
	}
	else
	{
		if (move <= -180)
			move = move + 360;
	}
	if (move > 0)
	{
		if (move > speed)
			move = speed;
	}
	else
	{
		if (move < -speed)
			move = -speed;
	}
	
	ent->s.angles[YAW] = anglemod (current + move);
	return move;
}

/*
===============
M_ChangeFishPitch

===============
*/
float M_ChangeFishPitch (edict_t *ent)
{
	float	ideal;
	float	current;
	float	move;
	float	speed;
	
	current = anglemod(ent->s.angles[PITCH]);
	ideal = ent->movedir[PITCH];

	if (current == ideal)
		return false;

	move = ideal - current;
	speed = ent->dmg_radius;
	if (ideal > current)
	{
		if (move >= 180)
			move = move - 360;
	}
	else
	{
		if (move <= -180)
			move = move + 360;
	}
	if (move > 0)
	{
		if (move > speed)
			move = speed;
	}
	else
	{
		if (move < -speed)
			move = -speed;
	}
	
	ent->s.angles[PITCH] = anglemod (current + move);
	return move;
}

// fish check to see if we are within ACTIVATE_DIST of the player
void fish_check_distance(edict_t *self)
{
	self->nextthink = level.time + 2.0;

	// determine if we are too far from the camera to warrant animating or ai
	if (gi.CheckDistances(self->s.origin, FISH_ACTIVATE_DIST))
	{
	 	self->nextthink = level.time + FRAMETIME;
	 	self->think = fish_think;
	}

}


/*-------------------------------------------------------------------------
	monster_think
-------------------------------------------------------------------------*/
void fish_think (edict_t *self)
{
	vec3_t	angles;
	vec3_t	top, bottom;
	vec3_t	dir;
	trace_t trace;
	byte	angle_byte;

	self->nextthink = level.time + FRAMETIME;

	if(!self->enemy)
		FindTarget(self);

	if(self->enemy)
	{//let's not hunt things out of water!
		if(!self->enemy->waterlevel)
			self->enemy = NULL;
	}
	// determine if we are too far from the camera to warrant animating or ai
	if (!gi.CheckDistances(self->s.origin, FISH_ACTIVATE_DIST))
	{
	 	self->think = fish_check_distance;
		VectorClear(self->velocity);
		return;
	}

	// animate us
	M_MoveFrame (self);

	self->nextthink = level.time + FRAMETIME;

	// we are already dead or getting hit, we don't need to do anything
	if ((self->deadflag & DEAD_DEAD) || (self->deadflag & DEAD_DYING) )
		return;

	M_CatagorizePosition(self);

	// did we break the surface ?
	if (self->waterlevel < 3)
	{
		// if we break water - don't let us target anyone anymore
		self->enemy = NULL;
		self->ai_mood = AI_MOOD_WANDER;
		self->dmg_radius = 10;

		// make us go down good sir !
		self->movedir[0] = flrand(-35.0, -15.0);

		// only allow one of these every second for this fish
		if (!self->count)
		{
			// create a ripple
			VectorCopy(self->s.origin, top);
			VectorCopy(top, bottom);
			top[2] += self->maxs[2] * 0.75;
			bottom[2] += self->mins[2];

			gi.trace(top, vec3_origin, vec3_origin, bottom, self, MASK_WATER,&trace);

			if(trace.fraction <= 1.0)
			{
				AngleVectors(self->s.angles,dir,NULL,NULL);
				VectorScale(dir,200,dir);
				angle_byte = Q_ftol(((self->s.angles[YAW] + DEGREE_180)/360.0) * 255.0);

				// no ripples while in cinematics
				if (!sv_cinematicfreeze->value)
					gi.CreateEffect(NULL, FX_WATER_WAKE, 0,	trace.endpos, "sbv", self->s.number,
						angle_byte, dir);

				gi.sound (self, CHAN_WEAPON, sounds[SND_SPLASH], 1, ATTN_NORM, 0);
				self->count = 6;
			}
		}
	}
	else
	{
		self->count = 0;
		self->dmg_radius = 4;
	}

	// make sure that the movedir angles are between 0-359, or we are in trouble on the pitch and yaw routines
	// messy, but the best way to be safe
	if ((self->movedir[0] < 0 || self->movedir[0] > 360)||
		 (self->movedir[1] < 0 || self->movedir[1] > 360)||
		 (self->movedir[2] < 0 || self->movedir[2] > 360))
		reset_fish_movedir(self);

	// move us from one angle to another slowly - unless we are moving through the "turn" anims,
	// which case, the anim takes care of the YAW

	// move us in pitch if we should
	M_ChangeFishPitch(self);

	VectorDegreesToRadians(self->s.angles, angles);

	if (!self->ai_mood_flags)
	{
		// update Yaw
		M_ChangeFishYaw(self);
		// update velocity
		DirFromAngles(angles, self->velocity);
		Vec3ScaleAssign(self->speed, self->velocity);
	}
	else
	{
		// update velocity
		DirFromAngles(angles, self->velocity);
		// we aren't updating Yaw, remember ? - no updating yaw velocities
		self->velocity[0] = 0;
		self->velocity[1] = 0;
		Vec3ScaleAssign(self->speed, self->velocity);
	}

	// need this for rebounds
	VectorCopy(self->velocity, self->pos1);

	M_WorldEffects(self);
}


void fish_under_water_wake (edict_t *self)
{
	gi.CreateEffect(&self->s, FX_M_EFFECTS, CEF_OWNERS_ORIGIN, self->s.origin, "bv", FX_UNDER_WATER_WAKE, vec3_origin);
}

void fish_swim_sound (edict_t *self, float fast)
{
	if(fast)
	{
		if(irand(0, 1))
			return;

		if(irand(0,1))
			gi.sound (self, CHAN_BODY, sounds[SND_FAST_SWIM1], 0.75, ATTN_IDLE, 0);
		else
			gi.sound (self, CHAN_BODY, sounds[SND_FAST_SWIM2], 0.75, ATTN_IDLE, 0);
	}
	else
	{
		if(irand(0, 4))
			return;

		if(irand(0,1))
			gi.sound (self, CHAN_BODY, sounds[SND_SLOW_SWIM1], 0.5, ATTN_IDLE, 0);
		else
			gi.sound (self, CHAN_BODY, sounds[SND_SLOW_SWIM2], 0.5, ATTN_IDLE, 0);
	}
}
/*-------------------------------------------------------------------------
	The fish hit something
-------------------------------------------------------------------------*/
void fish_blocked(edict_t *self, struct trace_s *trace)
{
	vec3_t	v;
	float	len;

	// dead fish don't rebound off stuff.
	if (self->deadflag == DEAD_DEAD)
		return;

	// did we hit a monster or player ?
	if(trace->ent && ((trace->ent->svflags & SVF_MONSTER) || (trace->ent->client)))
	{
		// hit another fish - send us on our way
		if (trace->ent->classID == CID_FISH)
			fish_bounce_direction(self);
		//we didn't, shall we attack this entity ?
		//we would be able to bite him, then sure, otherwise, just bounce us off and set him as the enemy
		else
		{
			// first decide if this guy is dead 
			if (trace->ent->deadflag == DEAD_DEAD)
			{
			  	fish_bounce_direction(self);
				self->enemy = NULL;
			}
			// nope, so lets BITE THE BASTARD :)
			else
			{
				VectorSubtract (self->s.origin, trace->ent->s.origin, v);
				len = VectorLength (v);
				self->enemy = trace->ent;

				if ((len < (self->maxs[0] + self->enemy->maxs[0] + BITE_DIST + 50)) && (self->dmg_radius ==4))	// Within 20 of bounding box & not out of water
				{
					SetAnim(self, ANIM_BITE);
					self->ai_mood = AI_MOOD_ATTACK;
				}
				else
				{
					fish_hunt(self);
				}
			}
		}
			
	}
	// we hit a wall, or something
	// reverse our direction, and the randomise a cone of projection out from that,
	// and send us on our way
	else
	{
		// did we hit a model of some type ?
		if (trace->ent)
			fish_bounce_direction(self);
		else
		// did we hit the same wall as last time ? cos if we did, we already dealt with it
		if ((int)trace->surface != (int)self->shrine_type)
		{
			self->shrine_type = (int)trace->surface;
			fish_bounce_direction(self);
		}
	}
}

/*-------------------------------------------------------------------------
	The fish finished a swim cycle, shall we just randomly change direction
	or perhaps target a player or a bad guy ? Or maybe just idle a bit
-------------------------------------------------------------------------*/
void finished_swim(edict_t *self)
{
	int	temp;

	if (self->ai_mood == AI_MOOD_PURSUE)
	{
		fish_hunt(self);
		return;
	}

	temp = irand(0,10);
	if (temp <= 3)
	{
		// randomly, we might like to run somewhere
		if (!(irand(0,3)))
			self->ai_mood = AI_MOOD_WANDER;
		else
			self->ai_mood = AI_MOOD_STAND;
		fish_new_direction(self);
	}
	else
	if (temp <= 5)
	{
		self->speed = 20;
	  	self->ai_mood = AI_MOOD_STAND;
		SetAnim(self, ANIM_STAND1);
	}
}

/*-------------------------------------------------------------------------
	The fish finished a swim cycle, shall we just randomly change direction
	or perhaps target a player or a bad guy ? Or maybe just idle a bit
-------------------------------------------------------------------------*/
void finished_fish_pain(edict_t *self)
{
   // run the hell away
	self->ai_mood = AI_MOOD_WANDER;
	self->deadflag = DEAD_NO;

	if(self->waterlevel == 3)
		fish_hunt(self);
}

/*-------------------------------------------------------------------------
	The fish finished a run swim cycle, shall we just randomly change direction
	or perhaps target a player or a bad guy ? Or maybe just idle a bit
-------------------------------------------------------------------------*/
void finished_runswim(edict_t *self)
{
	int	temp;

	if (self->ai_mood == AI_MOOD_PURSUE)
	{
		fish_hunt(self);
		return;
	}

	temp = irand(0,10);
	if (temp <= 3)
	{
		// randomly, we might like to run somewhere
		if (!(irand(0,3)))
			self->ai_mood = AI_MOOD_STAND;
		else
			self->ai_mood = AI_MOOD_WANDER;
		fish_new_direction(self);
	}
	else
	if (temp <= 5)
	{
		self->speed = 20;
	  	self->ai_mood = AI_MOOD_STAND;
		SetAnim(self, ANIM_STAND1);
	}
}

//----------------------------------------------------------------------
//  Fish Idle - decide whether to stay idling, or go walking somewhere
//----------------------------------------------------------------------
void fish_idle(edict_t *self)
{
	if (self->ai_mood != AI_MOOD_PURSUE)
	{
		if (!(irand(0,3)))
			SetAnim(self, ANIM_STAND1);
		else
			fish_new_direction(self);
	}
	else
		fish_hunt(self);
}


//----------------------------------------------------------------------
//  Fish Death - choose a death to use
//----------------------------------------------------------------------
void fish_dead_pain(edict_t *self, G_Message_t *msg)
{
	if(self->health<-60)
		BecomeDebris(self);
}

void fish_death(edict_t *self, G_Message_t *msg)
{
	VectorClear(self->velocity);
 	self->deadflag = DEAD_DEAD;
	if(self->health<-60)
	{
		gi.sound(self, CHAN_BODY, sounds[SND_GIB], 1, ATTN_NORM, 0);
		BecomeDebris(self);
		return;
	}

	gi.sound (self, CHAN_WEAPON, sounds[SND_DIE], 1, ATTN_NORM, 0);

	if(self->s.skinnum == FISH_SKIN1 || self->s.skinnum == FISH_SKIN2)
		self->s.skinnum += 1;

	SetAnim(self, ANIM_DEATH1);
}

//----------------------------------------------------------------------
//  Fish Pain - choose a pain to use
//----------------------------------------------------------------------
void fish_pain(edict_t *self, G_Message_t *msg)
{
	int				temp, damage;
	qboolean		force_pain;
	
	ParseMsgParms(msg, "eeiii", &temp, &temp, &force_pain, &damage, &temp);

	if(!force_pain)
		if(!flrand(0,3))
			return;


	SetAnim(self, ANIM_PAIN1);
	VectorClear(self->velocity);
	self->deadflag = DEAD_DYING;

	if(!irand(0,2))
		if(self->s.skinnum == FISH_SKIN1 || self->s.skinnum == FISH_SKIN2)
			self->s.skinnum += 1;

	if (irand(0, 1))
	{
		gi.sound (self, CHAN_WEAPON, sounds[SND_PAIN1], 1, ATTN_NORM, 0);
	}
	else
	{
		gi.sound (self, CHAN_WEAPON, sounds[SND_PAIN2], 1, ATTN_NORM, 0);
	}

}


//----------------------------------------------------------------------
//  Fish Melee - choose a melee to use
//----------------------------------------------------------------------
void fish_melee(edict_t *self, G_Message_t *msg)
{

	if (self->enemy->health <= 0)
	{
		if (!FindTarget(self))
		{
			SetAnim(self, ANIM_STAND1);
			return;
		}
	}

	SetAnim(self, ANIM_MELEE);
}

/*----------------------------------------------------------------------

  ACTION FUNCTIONS FOR THE FISH

	Death stuff

-----------------------------------------------------------------------*/
//----------------------------------------------------------------------
//  Fish Deadbob - (not really sure I like the name of this one)
//----------------------------------------------------------------------
void fish_deadbob(edict_t *self)
{

	if (self->velocity[2] > 0)
	{
		if (self->s.origin[2] > self->monsterinfo.misc_debounce_time + flrand(3.0, 6.0))	// So it doesn't always go to the same height
			self->velocity[2] = flrand(-7.0, -2.0);
	}
	else
	{
		if (self->s.origin[2] < self->monsterinfo.misc_debounce_time)
			self->velocity[2] = flrand(2.0, 7.0);
	}

	self->think = fish_deadbob;
	self->nextthink = level.time + .2;
}

//----------------------------------------------------------------------
//  Fish float - make the fish float to the surface
//----------------------------------------------------------------------


void fish_deadfloat(edict_t *self)
{
	self->think = fish_deadfloat;
	self->nextthink = level.time + .1;

	M_CatagorizePosition(self);

	if(self->waterlevel == 3)
	{
		if(self->velocity[2]<10)
			self->velocity[2]+=10;
		else
			self->velocity[2] = 20;	// Just in case somethimg blocked it going up
	}
	else if(self->waterlevel < 2)
	{
		if(self->velocity[2] > -150)
			self->velocity[2] -= 50;	// Fall back in now!
		else
			self->velocity[2] = -200;
	}
	else
	{
		self->monsterinfo.misc_debounce_time = self->s.origin[2];
		self->think = fish_deadbob;
		self->nextthink = level.time + .1;
	}
}

//----------------------------------------------------------------------
//  Fish Dead - he's dead, figure how far it is to the top of the water so he can float
//----------------------------------------------------------------------
void fish_dead(edict_t *self)
{
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;

	VectorClear(self->velocity);

	self->think = fish_deadfloat;
	self->nextthink = level.time + 0.1;
			
	// stop the fish making bubbles
	gi.RemoveEffects(&self->s, FX_WATER_BUBBLE);
	if (self->PersistantCFX)
	{
		gi.RemovePersistantEffect(self->PersistantCFX, REMOVE_FISH);
		self->PersistantCFX = 0;
	}

	gi.linkentity (self);
}

// he bit the player - decide what to do
void fishbite (edict_t *self)
{
	vec3_t	v;
	float	scale;
	float	len;

	if (!self->enemy || sv_cinematicfreeze->value)
		return;

	VectorSubtract (self->s.origin, self->enemy->s.origin, v);
	len = VectorLength (v);

	if (len < (self->maxs[0] + self->enemy->maxs[0] + BITE_DIST))	// Within 20 of bounding box
	{
		if (irand(0, 1))
		{
			gi.sound (self, CHAN_WEAPON, sounds[SND_BITEHIT1], 1, ATTN_NORM, 0);
		}	
		else
		{
			gi.sound (self, CHAN_WEAPON, sounds[SND_BITEHIT2], 1, ATTN_NORM, 0);
		}

		scale = -3;
		VectorScale(v, scale, v);
		VectorAdd (self->enemy->velocity, v, self->enemy->velocity);

		T_Damage (self->enemy, self, self, vec3_origin, self->enemy->s.origin, vec3_origin, irand(FISH_DMG_BITE_MIN, FISH_DMG_BITE_MAX) , 0, DAMAGE_DISMEMBER,MOD_DIED);

	}
	else			// A misssss
	{
		if (irand(0, 1))
		{
			gi.sound (self, CHAN_WEAPON, sounds[SND_BITEMISS1], 1, ATTN_NORM, 0);
		}
		else
		{
			gi.sound (self, CHAN_WEAPON, sounds[SND_BITEMISS2], 1, ATTN_NORM, 0);
		}
	}
}

void fish_target(edict_t *self)
{
	vec3_t	dir;

	if (self->enemy)
	{
		// figure out the vector from the fish to the target
		VectorSubtract(self->enemy->s.origin, self->s.origin, dir);
		// normalise it
		Vec3Normalize(dir);
		if(Vec3IsZero(dir))
			return;
		// figure out the angles we want
		AnglesFromDir(dir, self->movedir);
		VectorRadiansToDegrees (self->movedir, self->movedir);
	}
}

// figure out where our prey is, and go get him
void fish_hunt(edict_t *self)
{

	// make sure we still have a target - bouncing off stuff tends to clear it out
	if (!self->enemy)
	{
		FindTarget(self);
		// if we can't find one, let him just swim on alone..
		if (!self->enemy)
		{
			if(self->curAnimID == ANIM_PAIN1)
			{
				self->speed = 20;
	  			self->ai_mood = AI_MOOD_STAND;
				SetAnim(self, ANIM_STAND1);
			}
			return;
		}
	}

	fish_target(self);
	// set movement type
	self->ai_mood = AI_MOOD_PURSUE;
	//	make us run after it
	self->speed = FISH_HUNT * self->old_yaw;
	fish_run(self);
}

// we are done attacking.. what do we do now ? attack again ?
void fish_pause (edict_t *self)
{
	vec3_t	v;
	float	len;

	FindTarget(self);

	// is the target either not there or already dead ?
	if (!self->enemy || self->enemy->deadflag == DEAD_DEAD)
	{
		self->enemy = NULL;
		self->ai_mood = AI_MOOD_WANDER;
		fish_bounce_direction(self);
		return;//right?  crashes if not!
	}

	VectorSubtract (self->s.origin, self->enemy->s.origin, v);
	len = VectorLength (v);

	// we are close	enough to bite
	if (len < (self->maxs[0] + self->enemy->maxs[0] + BITE_DIST))	// Within BITE_DIST of bounding box
	{
		VectorClear(self->velocity);
		// if he's low on health, eat the bastard..
		if (self->enemy->health < (self->enemy->max_health / 2))
		{
			SetAnim(self, ANIM_MELEE);
			self->ai_mood = AI_MOOD_ATTACK;
		}
		// other wise a quick bite
		else
		{
			// randomly swim off anyway
			if (!irand(0,4))
			{
				self->ai_mood = AI_MOOD_ATTACK;
				SetAnim(self, ANIM_BITE);
			}
			else
			{
				self->enemy = NULL;
				self->ai_mood = AI_MOOD_WANDER;
				fish_bounce_direction(self);
			}
		}
	}

	else
	{
		if (len < 120)  // close enough to just zoom in on
		{
			fish_hunt(self);
		}
		else	// far enough that I break off..
		{
		  	self->enemy = NULL;
			self->ai_mood = AI_MOOD_WANDER;
			fish_bounce_direction(self);
		}
	}
}



// shall we chase after someone ?
void fish_chase(edict_t *self)
{

	// shall we hunt someone ?
	if (irand(0,1))
		return;

	// find a target to chase after
	FindTarget(self);

	// if we got one..
	if (self->enemy)
		fish_hunt(self);
}

/*----------------------------------------------------------------------

  SOUND FUNCTIONS FOR THE FISH

-----------------------------------------------------------------------*/

// random growl
void fish_growl (edict_t *self)
{
	int chance;

	return;
	chance = irand(0, 200);

	if (chance > 60)
	{
	}
	else if (chance < 20 )
	{
		gi.sound (self, CHAN_WEAPON, sounds[SND_GROWL1], 1, ATTN_NORM, 0);
	}
	else if (chance < 40)
	{
		gi.sound (self, CHAN_WEAPON, sounds[SND_GROWL2], 1, ATTN_NORM, 0);
	}
	else
	{
		gi.sound (self, CHAN_WEAPON, sounds[SND_GROWL3], 1, ATTN_NORM, 0);
	}
}


void FishStaticsInit()
{
	classStatics[CID_FISH].msgReceivers[MSG_PAIN] = fish_pain;
	classStatics[CID_FISH].msgReceivers[MSG_DEATH] = fish_death;
	classStatics[CID_FISH].msgReceivers[MSG_DEATH_PAIN] = fish_dead_pain;

	resInfo.numAnims = NUM_ANIMS;
	resInfo.animations = animations;
	resInfo.modelIndex = gi.modelindex("models/monsters/fish/tris.fm");

	sounds[SND_PAIN1] = gi.soundindex ("monsters/fish/pain1.wav");	
	sounds[SND_PAIN2] = gi.soundindex ("monsters/fish/pain2.wav");	
	sounds[SND_DIE] = gi.soundindex ("monsters/fish/death1.wav");	
	sounds[SND_GIB] = gi.soundindex ("monsters/fish/gib.wav");
	sounds[SND_BITEHIT1] = gi.soundindex ("monsters/fish/meleehit1.wav");	
	sounds[SND_BITEHIT2] = gi.soundindex ("monsters/fish/meleehit2.wav");	
	sounds[SND_BITEMISS1] = gi.soundindex ("monsters/fish/meleemiss1.wav");	
	sounds[SND_BITEMISS2] = gi.soundindex ("monsters/fish/meleemiss2.wav");	
	sounds[SND_GROWL1] = gi.soundindex ("monsters/fish/growl1.wav");	
	sounds[SND_GROWL2] = gi.soundindex ("monsters/fish/growl2.wav");	
	sounds[SND_GROWL3] = gi.soundindex ("monsters/fish/growl3.wav");
	sounds[SND_SPLASH] = gi.soundindex("player/breaststroke.wav");

	sounds[SND_SLOW_SWIM1] = gi.soundindex("monsters/fish/fishmov3.wav");
	sounds[SND_SLOW_SWIM2] = gi.soundindex("monsters/fish/fishmov4.wav");
	sounds[SND_FAST_SWIM1] = gi.soundindex("monsters/fish/fishmov1.wav");
	sounds[SND_FAST_SWIM2] = gi.soundindex("monsters/fish/fishmov2.wav");

	resInfo.numSounds = NUM_SOUNDS;
	resInfo.sounds = sounds;

	classStatics[CID_FISH].resInfo = &resInfo;
}

/*QUAKED monster_fish (1 .5 0) (-25 -25 -14) (25 25 14) 

The fish

"wakeup_target" - monsters will fire this target the first time it wakes up (only once)

"pain_target" - monsters will fire this target the first time it gets hurt (only once)
*/

void SP_monster_fish (edict_t *self)
{
	level.total_monsters++;

	if ((deathmatch->value == 1) && !((int)sv_cheats->value & self_spawn))
	{
		G_FreeEdict (self);
		return;
	}

	// Generic Monster Initialization

	if (!self->health)
		self->health = FISH_HEALTH;

	//Apply to the end result (whether designer set or not)
	self->max_health = self->health = MonsterHealth(self->health);

	self->nextthink = level.time + FRAMETIME;
	self->svflags |= SVF_MONSTER | SVF_TAKE_NO_IMPACT_DMG | SVF_DO_NO_IMPACT_DMG;
	self->svflags &= ~SVF_DEADMONSTER;
	self->s.renderfx |= RF_FRAMELERP;
	self->takedamage = DAMAGE_AIM;
	self->max_health = self->health;
	self->clipmask = MASK_MONSTERSOLID;
	self->materialtype = MAT_FLESH;
	self->flags |= FL_SWIM|FL_NO_KNOCKBACK;
	
	self->s.effects|=EF_CAMERA_NO_CLIP;

	// random skin of three
	if(irand(0, 1))
		self->s.skinnum = FISH_SKIN1;
	else
		self->s.skinnum = FISH_SKIN2;

	self->deadflag = DEAD_NO;
	self->isBlocked = fish_blocked;
	self->ai_mood = AI_MOOD_STAND;
	self->ai_mood_flags = 0;
	self->gravity = self->best_move_yaw = 0;
	self->wakeup_distance = 1024;
	self->monsterinfo.aiflags |= AI_NIGHTVISION;

	self->monsterinfo.aiflags |= AI_NO_ALERT;//pay no attention to alert ents

	VectorCopy (self->s.origin, self->s.old_origin);
	VectorCopy (self->s.angles, self->movedir);

	if (!self->mass)
		self->mass = FISH_MASS;

	self->s.frame = 0;

	self->oldenemy_debounce_time = -1;
	
	self->msgHandler = DefaultMsgHandler;
	self->classID = CID_FISH;
	self->think = fish_check_distance;
	self->nextthink = level.time + FRAMETIME;

	self->yaw_speed = 11;
	self->dmg_radius = 4;
	// random(ish) speed
	self->old_yaw = flrand(0.65,1.0);

	self->movetype=PHYSICSTYPE_STEP;
	VectorClear(self->knockbackvel);

	self->solid=SOLID_BBOX;

	self->s.modelindex = classStatics[CID_FISH].resInfo->modelIndex;

	self->shrine_type = 0;
	
	if (self->s.scale == 1)
		self->s.scale = self->monsterinfo.scale = MODEL_SCALE * flrand(0.5,1.0);

	VectorSet(self->mins, -16, -16, -8);
	VectorSet(self->maxs, 16, 16, 8);

	// scale the max's and mins according to scale of model
	Vec3ScaleAssign(self->s.scale, self->mins);
	Vec3ScaleAssign(self->s.scale, self->maxs);

	// give us the bubble spawner
 	self->PersistantCFX = gi.CreatePersistantEffect(&self->s,
 												FX_WATER_BUBBLE,
 												CEF_OWNERS_ORIGIN | CEF_BROADCAST,
 												NULL,
												"");

	SetAnim(self, ANIM_STAND1);

	gi.linkentity(self); 

	M_CatagorizePosition(self);
}
