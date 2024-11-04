/*
==============================================================================

// m_gorgon.c
//
// Heretic II
// Copyright 1998 Raven Software


GORGON

	AI :

	STANDING 1 : Looking straight ahead
	STANDING 2 : Looking to left
	STANDING 3 : Looking to the right

	WALK       : a normal straight line
	WALK RIGHT : if turning to the right while walking
	WALK LEFT  : if turning to the left while walking

	RUN1       : Running
	RUN2       : Turning left while running
	RUN3       : Turning right while running

	MELEE1     : Attack Left
	MELEE2     : Attack Right
	MELEE3     : Attack Up
	MELEE4     : Attack Pullback
	MELEE5     : Running attack
	MELEE6     : Hop left
	MELEE7     : Hop right
	MELEE8     : Hop forward
	MELEE9     : Hop backward

	PAIN1      : step back and bow head down
	PAIN2      : bow head to the left
	PAIN3      : bow head to the right

	DIE1       : take a few steps backwards and keel over
	DIE2       : fly backwards and slide to a stop

==============================================================================
*/

#include "g_local.h"
#include "Utilities.h"
#include "g_DefaultMessageHandler.h"
#include "g_monster.h"
#include "fx.h"
#include "random.h"
#include "buoy.h"
#include "vector.h"

#include "g_misc.h"
#include "m_gorgon.h"
#include "m_gorgon_anim.h"
#include "m_stats.h"
#include "p_anim_branch.h"
#include "p_anims.h"
#include "g_HitLocation.h"
#include "p_actions.h"


// *************************************
// Definitions
// *************************************

void BecomeDebris(edict_t *self);
qboolean ai_have_enemy (edict_t *self);
qboolean clear_visible (edict_t *self, edict_t *other);
qboolean EqualAngle(float angle1, float angle2, float leniency);
qboolean ok_to_wake (edict_t *monster, qboolean gorgon_roar, qboolean ignore_ambush);
float MG_ChangeWhichYaw (edict_t *self, qboolean ideal_yaw);
qboolean gorgon_check_jump (edict_t *self);

#define TRYSTEP_OK			0
#define TRYSTEP_ALLSOLID	1
#define TRYSTEP_STARTSOLID	2
#define TRYSTEP_OFFEDGE		3
#define TRYSTEP_NOSUPPORT	4
#define TRYSTEP_INWATER		5
#define GORGON_STD_MELEE_RNG	48
#define GORGON_STD_MAXHOP_RNG	200

static animmove_t *animations[NUM_ANIMS] =
{
	&gorgon_move_stand1,
	&gorgon_move_stand2,
	&gorgon_move_stand3,
	&gorgon_move_stand4,
	&gorgon_move_walk,
	&gorgon_move_walk2,
	&gorgon_move_walk3,
	&gorgon_move_melee1,
	&gorgon_move_melee2,
	&gorgon_move_melee3,
	&gorgon_move_melee4,
	&gorgon_move_melee5,
	&gorgon_move_melee6,
	&gorgon_move_melee7,
	&gorgon_move_melee8,
	&gorgon_move_melee9,
	&gorgon_move_melee10,
	&gorgon_move_fjump,
	&gorgon_move_run1,
	&gorgon_move_run2,
	&gorgon_move_run3,
	&gorgon_move_pain1,
	&gorgon_move_pain2,
	&gorgon_move_pain3,
	&gorgon_move_die1,
	&gorgon_move_die2,
	&gorgon_move_snatch,
	&gorgon_move_catch,
	&gorgon_move_miss,
	&gorgon_move_readycatch,
	&gorgon_move_snatchhi,
	&gorgon_move_snatchlow,
	&gorgon_move_slip,
	&gorgon_move_slip_pain,
	&gorgon_move_delay,
	&gorgon_move_roar,
	&gorgon_move_roar2,
	&gorgon_move_land2,
	&gorgon_move_land,
	&gorgon_move_inair,
	&gorgon_move_to_swim,
	&gorgon_move_swim,
	&gorgon_move_swim_bite_a,
	&gorgon_move_swim_bite_a,
	&gorgon_move_outwater,
	&gorgon_move_eat_down,
	&gorgon_move_eat_up,
	&gorgon_move_eat_loop,
	&gorgon_move_eat_tear,
	&gorgon_move_eat_pullback,
	&gorgon_move_look_around,
	&gorgon_move_eat_left,
	&gorgon_move_eat_right,
	&gorgon_move_eat_snap,
	&gorgon_move_eat_react
};

static int sounds[NUM_SOUNDS];

static ClassResourceInfo_t resInfo;

void gorgon_watch(edict_t *self, G_Message_t *msg);
void gorgon_death_pain(edict_t *self, G_Message_t *msg);

void gorgon_blocked (edict_t *self, trace_t *trace)
{
	vec3_t dir;

	if(self->velocity[2]>=0)
		return;

	if(trace->ent->s.origin[2] + trace->ent->maxs[2] > self->s.origin[2] + self->mins[2])
		return;

	if(!trace->ent->takedamage)
		return;

	VectorCopy(self->velocity,dir);
	VectorNormalize(dir);
	T_Damage (trace->ent, self, self, self->velocity, trace->ent->s.origin, dir, flrand(5, 15), 50, DAMAGE_EXTRA_KNOCKBACK,MOD_DIED);
	if(trace->ent->health>0)
	{
		if(trace->ent->client)
		{
			if(!irand(0, 2))
			{
				if(trace->ent->client->playerinfo.lowerseq != ASEQ_KNOCKDOWN)
				{
					if(infront(trace->ent, self))
					{
						P_KnockDownPlayer(&trace->ent->client->playerinfo);
					}
				}
			}
		}
	}
}

void gorgon_roar_sound (edict_t *self)
{
	int chance;

	chance = irand(0, 100);
	if (chance < 20)
		gi.sound(self, CHAN_VOICE, sounds[SND_PAIN1], 1, ATTN_NORM, 0);
	else if (chance < 40)
		gi.sound(self, CHAN_VOICE, sounds[SND_PAIN2], 1, ATTN_NORM, 0);
	else if (chance < 60)
		gi.sound(self, CHAN_VOICE, sounds[SND_DIE], 1, ATTN_NORM, 0);
	else
		gorgon_growl(self);
}


void gorgon_roar_response_go (edict_t *self)
{
	self->pre_think = NULL;
	self->next_pre_think = -1;

	if(self->ai_mood == AI_MOOD_EAT)
		self->ai_mood = AI_MOOD_PURSUE;

	SetAnim(self, ANIM_ROAR2);

	self->nextthink = level.time + 0.1;
}

void gorgon_roar_response (edict_t *self, G_Message_t *msg)
{//respond to call
	if(!irand(0, 3))
		return;//25% don't roar back

	self->pre_think = gorgon_roar_response_go;
	self->next_pre_think = level.time + flrand (0.5, 2);
	self->nextthink = level.time + 10;
}

void gorgonRoar (edict_t *self)
{//finds gorgons in immediate vicinity and wakes them up
	edict_t *found = NULL;

	if(!self->enemy)
		return;

	while(found = findradius(found, self->s.origin, GORGON_ALERT_DIST))
	{
		if(found->health>0)
		{
			if(!found->enemy)
			{
				if(found->svflags & SVF_MONSTER)
				{
					if(found->classID == CID_GORGON)
					{
						if(!found->monsterinfo.roared)
						{
							if(gi.inPHS(self->s.origin, found->s.origin))
							{//make sure they can hear me
								if(ok_to_wake(found, true, true))
								{
									found->monsterinfo.roared = true;
									found->enemy = self->enemy;
									FoundTarget(found, false);
									QPostMessage(found, MSG_VOICE_POLL, PRI_DIRECTIVE, "");
								}
							}
						}
					}
				}
			}
		}
	}
//Not just an alert monsters since we want to wake up gorgons only
	//	AlertMonsters(self, self->enemy, 5, 1);
}

qboolean gorgonFindAsleepGorgons (edict_t *self)
{//sees if there are any gorgons in range that aren't awake
	edict_t *found = NULL;

	while(found = findradius(found, self->s.origin, GORGON_ALERT_DIST))
	{
		if(found!=self)
		{
			if(found->health>0)
			{
				if(!found->enemy)
				{
					if(found->svflags & SVF_MONSTER)
					{
						if(found->classID == CID_GORGON)
						{
							if(!found->monsterinfo.roared)
							{
								return true;
							}
						}
					}
				}
			}
		}
	}
	return false;
}
//----------------------------------------------------------------------
//  Gorgon Eat - decide which eating animations to use
//----------------------------------------------------------------------
void gorgon_eat(edict_t *self, G_Message_t *msg)
{
	int chance;

	chance = irand(0, 100);
	if (chance < 90)	// Eating
	{
		if (chance < 80)
			SetAnim(self, ANIM_EAT_LOOP);
		else
			SetAnim(self, ANIM_EAT_PULLBACK);
	}
	else 
	{
		SetAnim(self, ANIM_EAT_TEAR);
	} 

	self->monsterinfo.misc_debounce_time = level.time + 5;
}

/*----------------------------------------------------------------------
  Gorgon Watch  -decide which standing animations to use
-----------------------------------------------------------------------*/
void gorgon_watch(edict_t *self, G_Message_t *msg)
{
	int	chance;
	
	chance = irand(0, 100);
	if (chance < 10)
		SetAnim(self, ANIM_STAND2);
	else if (chance < 20)
		SetAnim(self, ANIM_STAND3);
	else 
		SetAnim(self, ANIM_STAND1);
}

qboolean gorgon_check_attack(edict_t *self)
{
	vec3_t	v, vr, vf;
	float	dot, dot2, len;
	int		chance;

	if(!M_ValidTarget(self, self->enemy))
		return false;

	if (!clear_visible(self, self->enemy))
		return false;
	
	VectorSubtract(self->enemy->s.origin, self->s.origin, v);
	len = VectorNormalize(v);

	if (len < 200)
	{
		self->show_hostile = level.time + 1;		// wake up other monsters		
		QPostMessage(self, MSG_MELEE, PRI_DIRECTIVE, NULL);

		return true;
	}
	else if (len < 400)
	{
		if (!irand(0, 3))
		{
			gorgon_growl(self);
			return true;
		}
	}
	
	if (!irand(0, 3))
	{
		AngleVectors(self->s.angles, vf, vr, NULL);
		dot  = DotProduct(vf, v);
		dot2 = DotProduct(vr, v);

		if (dot > -0.85 && dot < -0.25) //Behind and aesthetically correct
		{
			gorgon_growl(self);

			if (dot2 > 0) //to the right
				SetAnim(self, ANIM_STAND3);
			else  //to the left
				SetAnim(self, ANIM_STAND2);

			return true;
		}
		return false;
	}
	else
	{
		if (self->monsterinfo.aiflags & AI_EATING)
		{
			chance = irand(0, 100);
			if (chance < 10)
				SetAnim(self, ANIM_EAT_TEAR);
			else if (chance < 20)
				SetAnim(self, ANIM_EAT_PULLBACK);
			else 
				SetAnim(self, ANIM_EAT_LOOP);
			return true;
		}
		
		return false;
	}
}
/*----------------------------------------------------------------------
  Gorgon Stand  -decide which standing animations to use
-----------------------------------------------------------------------*/
void gorgon_stand(edict_t *self, G_Message_t *msg)
{
	int		chance;

	if (self->ai_mood == AI_MOOD_DELAY)
	{
		SetAnim(self, ANIM_DELAY);
		return;
	}

	if(gorgon_check_attack(self))
		return;

	if (self->monsterinfo.aiflags & AI_EATING)
	{
		chance = irand(0, 100);
		if (chance < 10)
			SetAnim(self, ANIM_EAT_TEAR);
		else if (chance < 20)
			SetAnim(self, ANIM_EAT_PULLBACK);
		else 
			SetAnim(self, ANIM_EAT_LOOP);
	}
	else
	{
		chance = irand(0, 100);
		if (chance < 10)
			SetAnim(self, ANIM_STAND2);
		else if (chance < 20)
			SetAnim(self, ANIM_STAND3);
		else 
			SetAnim(self, ANIM_STAND1);
	}

	self->monsterinfo.misc_debounce_time = level.time + 5;
}


/*----------------------------------------------------------------------
  Gorgon Walk -decide which walk animations to use
-----------------------------------------------------------------------*/
void gorgon_walk(edict_t *self, G_Message_t *msg)
{
	vec3_t	v;
	float	len;
	float	delta;
	vec3_t targ_org;

	if(!MG_GetTargOrg(self, targ_org))
		return;


	if (!self->enemy)//?goal?
	{
		SetAnim(self, ANIM_WALK1);
		return;
	}

	if (self->spawnflags & MSF_COWARD)
	{
		VectorSubtract (self->s.origin, targ_org, v);
		len = VectorLength (v);
		self->ideal_yaw = vectoyaw(v);
		M_ChangeYaw(self);

		if (len < 200)
		{
			self->monsterinfo.aiflags |= AI_FLEE;
			self->monsterinfo.flee_finished = level.time + flrand(4.0, 7.0);
			SetAnim(self, ANIM_RUN1);
			return;
		}
	}

	if(clear_visible(self, self->enemy) && infront(self, self->enemy))
	{
		VectorSubtract (self->s.origin, targ_org, v);
		len = VectorLength (v);
		// targ_org is within range and far enough above or below to warrant a jump
		if ((len > 40) && (len < 600) && ((self->s.origin[2] < targ_org[2] - 18) || 
			(self->s.origin[2] > targ_org[2] + 18)))
		{
			if(gorgon_check_jump(self))
			{
				SetAnim(self, ANIM_FJUMP);
				return;
			}
		}
	}

	delta = anglemod(self->s.angles[YAW] - self->ideal_yaw);
	if (delta > 25 && delta <= 180)
	{
		SetAnim(self, ANIM_WALK3);
	}
	else if (delta > 180 && delta < 335)
	{
		SetAnim(self, ANIM_WALK2);
	}
	else
	{
		SetAnim(self, ANIM_WALK1);
	}
}

/*----------------------------------------------------------------------
  Gorgon Melee - decide which melee animations to use
-----------------------------------------------------------------------*/
void gorgon_melee(edict_t *self, G_Message_t *msg)
{
	trace_t	trace;
	vec3_t	v, source, melee_point, forward, up;
	float	len, seperation, melee_range, max_hop_range;
	float	chance;

	if(self->ai_mood == AI_MOOD_NAVIGATE)
		return;

	if(!ai_have_enemy(self))
		return;

	AngleVectors(self->s.angles,forward, NULL, up);
	VectorMA(self->s.origin, self->maxs[2]*0.5, up, melee_point);
	VectorMA(melee_point, self->maxs[0], forward, melee_point);

	VectorSubtract (self->enemy->s.origin, melee_point, v);
	len = VectorLength (v);

	seperation = self->enemy->maxs[0];
	melee_range = self->melee_range;

	
	if (self->monsterinfo.attack_finished > level.time)
	{//too soon to attack again
//		gi.dprintf("Waiting for next attack\n");
		chance = flrand(0, 1);
		if (chance < 0.6)
			SetAnim(self, ANIM_STAND4);
		else if (chance < 0.7)
			SetAnim(self, ANIM_MELEE6);		// Hop left
		else if (chance < 0.8)
			SetAnim(self, ANIM_MELEE7);		// Hop right
		else
			SetAnim(self, ANIM_MELEE9);		// Hop backward

		return;
	}

	//ok to attack
	if(len - seperation < melee_range)
	{//melee
//		gi.dprintf("Biting: ");
		chance = flrand(0, 1);
		
		if(!stricmp(self->enemy->classname,"monster_rat"))
		{
			if(self->enemy->s.origin[2] > self->s.origin[2])
			{
//				gi.dprintf(" snatch high\n");
				SetAnim(self,ANIM_SNATCHHI);
			}
			else
			{
//				gi.dprintf(" snatch low\n");
				SetAnim(self,ANIM_SNATCHLOW);
			}
		}
		else if (chance < 0.25)
		{
//			gi.dprintf(" melee1\n");
			SetAnim(self, ANIM_MELEE1);		// Attack left
		}
		else if (chance < 0.5)
		{
//			gi.dprintf(" melee2\n");
			SetAnim(self, ANIM_MELEE2);		// Attack right
		}
		else if (chance < 0.75)
		{
//			gi.dprintf(" melee3\n");
			SetAnim(self, ANIM_MELEE3);		// Attack Up
		}
		else
		{
//			gi.dprintf(" melee4\n");
			SetAnim(self, ANIM_MELEE4);		// Pull back
		}
		
		self->monsterinfo.attack_finished = level.time + flrand(0, 3 - skill->value);
		return;
	}

	//Out of melee range
	if (len < 150)// && enemy_vis)
	{
		SetAnim(self, ANIM_MELEE5);
		return;
	}
	max_hop_range = self->s.scale * GORGON_STD_MAXHOP_RNG;
	if (len < max_hop_range)  // Hop forward
	{//cheacks ahead to see if can hop at it
//		gi.dprintf(" too far\n");
		//Setup the trace
//		gi.dprintf("Hopping forward\n");
		VectorCopy(self->s.origin, source);
		VectorMA(source, 64 * self->s.scale, forward, source);
		
		gi.trace (self->s.origin, self->mins, self->maxs, source, self, MASK_SHOT,&trace);
		
		if (trace.ent == self->enemy || trace.fraction == 1)
			SetAnim(self, ANIM_MELEE8);
		else
		{
			VectorCopy(self->s.origin, source);
			VectorMA(source, 32 * self->s.scale, forward, source);
			
			gi.trace (self->s.origin, self->mins, self->maxs, source, self, MASK_SHOT,&trace);

			if (trace.fraction == 1)
				SetAnim(self, ANIM_MELEE7);
			else
				SetAnim(self, ANIM_MELEE6);
		}
	}
}


/*----------------------------------------------------------------------
  Gorgon Run -decide which run animations to use
-----------------------------------------------------------------------*/
void gorgon_run(edict_t *self, G_Message_t *msg)
{
	vec3_t	v;
	float	len;
	float	delta;
	qboolean enemy_vis;
	vec3_t targ_org;

	if(!ai_have_enemy(self))
		return;

	if(!MG_GetTargOrg(self, targ_org))
		return;

	if(self->flags & FL_INWATER)
	{
		gorgonGoSwim(self);
		return;
	}

	VectorSubtract (self->s.origin, targ_org, v);
	len = VectorLength (v);
	if(self->ai_mood == AI_MOOD_PURSUE)
	{
//		gi.dprintf("Running gorgon after player...\n");
		enemy_vis = clear_visible(self, self->enemy);
	}
	else
		enemy_vis = clear_visible_pos(self, self->monsterinfo.nav_goal);

	if(enemy_vis)
	{//JUMP
		if(self->enemy && irand(0, 4) && self->damage_debounce_time < level.time && !self->monsterinfo.roared)
		{//should we do this the first time we see player?
			if(infront(self, self->enemy))
			{
				if(gorgonFindAsleepGorgons(self))
				{
					self->damage_debounce_time = level.time + 10;
					SetAnim(self, ANIM_ROAR);//threaten, brings other monsters
					return;
				}
				else if (!self->dmg_radius)
				{//make a wakeup roar
					self->dmg_radius = true;
					SetAnim(self, ANIM_ROAR2);
				}
			}
		}
		// Enemy is within range and far enough above or below to warrant a jump
		if(infront_pos(self, targ_org))
		{
			if ((len > 40) && (len < 600) && ((self->s.origin[2] < targ_org[2] - 24) || 
				(self->s.origin[2] > targ_org[2] + 24)))
			{
				if (abs(self->s.origin[2] - targ_org[2] - 24) < 200) // Can't jump more than 200 high
				{
					if (!irand(0, 2))
					{
						if(self->ai_mood == AI_MOOD_PURSUE||!irand(0, 4))
						{//20% chance to jump at a buoy
							if(gorgon_check_jump(self))
							{
								SetAnim(self, ANIM_FJUMP);
								return;
							}
						}
					}
				}
			}
		}
	}

	delta = anglemod(self->s.angles[YAW] - self->ideal_yaw);
	if (delta > 45 && delta <= 180)
	{
		SetAnim(self, ANIM_RUN3);		// Turn right
	}
	else if (delta > 180 && delta < 315)
	{
		SetAnim(self, ANIM_RUN2);		// Turn left
	}
	else
	{
		SetAnim(self, ANIM_RUN1);		// Run on 
	}
}


/*----------------------------------------------------------------------
  Gorgon Pain - make the decision between pains 1, 2, or 3 or slip
-----------------------------------------------------------------------*/
qboolean gorgonCheckSlipGo (edict_t *self, qboolean frompain);
void gorgon_pain(edict_t *self, G_Message_t *msg)
{
	edict_t		*tempent;
	int			chance, temp, damage;
	qboolean	force_pain;
	
	ParseMsgParms(msg, "eeiii", &tempent, &tempent, &force_pain, &damage, &temp);

	if(!force_pain)
	{
		if(!irand(0, 2)||!self->groundentity)
			return;

		if(self->pain_debounce_time > level.time)
			return;
	}

	self->pain_debounce_time = level.time + 0.5;

	chance = irand(0, 100);
	if (chance < 50)
		gi.sound(self, CHAN_VOICE, sounds[SND_PAIN1], 1, ATTN_NORM, 0);
	else
		gi.sound(self, CHAN_VOICE, sounds[SND_PAIN2], 1, ATTN_NORM, 0);

	if(!irand(0, 4))
		self->s.skinnum = GORGON_PAIN_SKIN;

	if(skill->value > 1.0 || !gorgonCheckSlipGo(self, true))
	{
		if (chance < 33)
			SetAnim(self, ANIM_PAIN1);
		else if (chance < 66)
			SetAnim(self, ANIM_PAIN2);
		else
			SetAnim(self, ANIM_PAIN3);
	}
}


/*----------------------------------------------------------------------
  Gorgon Die - choose death 
-----------------------------------------------------------------------*/
void gorgon_death_pain(edict_t *self, G_Message_t *msg)
{
	if (self->health <= -80)
	{
		gi.sound(self, CHAN_BODY, sounds[SND_GIB], 1, ATTN_NORM, 0);
		self->deadflag = DEAD_DEAD;
		BecomeDebris(self);
		return;
	}
}

void gorgon_death(edict_t *self, G_Message_t *msg)
{
	if(self->monsterinfo.aiflags&AI_DONT_THINK)
	{
		gi.sound(self, CHAN_VOICE, sounds[SND_DIE], 1, ATTN_NORM, 0);
		if (irand(0,10) < 5)  // Big enough death to be thrown back
			SetAnim(self, ANIM_DIE2);
		else 
			SetAnim(self, ANIM_DIE1);
		return;
	}
	self->msgHandler = DeadMsgHandler;

	// check for gib
	if (self->health <= -80)
	{
		gi.sound(self, CHAN_BODY, sounds[SND_GIB], 1, ATTN_NORM, 0);
		self->deadflag = DEAD_DEAD;
		BecomeDebris(self);
		return;
	}

	if (self->deadflag == DEAD_DEAD)  // Dead but still being hit
		return;

	self->pre_think = NULL;
	self->next_pre_think = -1;

	self->gravity = 1.0f;
	self->svflags |= SVF_TAKE_NO_IMPACT_DMG | SVF_DO_NO_IMPACT_DMG;
	// regular death
	self->s.skinnum = GORGON_PAIN_SKIN;
	gi.sound(self, CHAN_VOICE, sounds[SND_DIE], 1, ATTN_NORM, 0);
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;

	if (self->health <= -10)  // Big enough death to be thrown back
		SetAnim(self, ANIM_DIE2);
	else 
		SetAnim(self, ANIM_DIE1);
}



/*----------------------------------------------------------------------

  ACTION FUNCTIONS FOR THE MONSTER

-----------------------------------------------------------------------*/

void gorgon_footstep (edict_t *self)
{
	int chance;

	chance = irand(0, 100);
	if (chance < 25)
		gi.sound(self, CHAN_BODY, sounds[SND_STEP1], 1, ATTN_NORM, 0);
	else if (chance < 50)
		gi.sound(self, CHAN_BODY, sounds[SND_STEP2], 1, ATTN_NORM, 0);
	else if (chance < 75)
		gi.sound(self, CHAN_BODY, sounds[SND_STEP3], 1, ATTN_NORM, 0);
	else
		gi.sound(self, CHAN_BODY, sounds[SND_STEP4], 1, ATTN_NORM, 0);
}

void gorgon_growl (edict_t *self)
{
	int chance;

	chance = irand(0, 100);
	if (chance < 10)
		gi.sound(self, CHAN_WEAPON, sounds[SND_GROWL1], 1, ATTN_NORM, 0);
	else if (chance < 20)
		gi.sound(self, CHAN_WEAPON, sounds[SND_GROWL2], 1, ATTN_NORM, 0);
	else if (chance < 30)
		gi.sound(self, CHAN_WEAPON, sounds[SND_GROWL3], 1, ATTN_NORM, 0);
}

void gorgon_prethink (edict_t *self);
qboolean gorgonCheckMood(edict_t *self)
{
	self->pre_think = gorgon_prethink;
	self->next_pre_think = level.time + 0.1;

	self->mood_think(self);

	if(self->ai_mood == AI_MOOD_NORMAL)
		return false;

	switch (self->ai_mood)
	{
		case AI_MOOD_ATTACK://melee and missile the same
			QPostMessage(self, MSG_MELEE, PRI_DIRECTIVE, NULL);
			break;
		case AI_MOOD_PURSUE:
			QPostMessage(self, MSG_RUN, PRI_DIRECTIVE, NULL);
			break;
		case AI_MOOD_WALK:
			QPostMessage(self, MSG_WALK, PRI_DIRECTIVE, NULL);
			break;
		case AI_MOOD_NAVIGATE:
			if(self->flags & FL_INWATER)
			{
				gorgonGoSwim(self);
				return true;
			}
			if(self->curAnimID == ANIM_RUN1 ||
				self->curAnimID == ANIM_RUN2||
				self->curAnimID == ANIM_RUN3)
				return true;
			else
				SetAnim(self, ANIM_RUN1);
			break;
		case AI_MOOD_STAND:
			QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
			break;
		case AI_MOOD_DELAY:
			SetAnim(self, ANIM_DELAY);
			break;
		case AI_MOOD_WANDER:
		case AI_MOOD_FLEE:
			if(self->flags & FL_INWATER)
			{
				gorgonGoSwim(self);
				return true;
			}
			if(self->curAnimID == ANIM_RUN1 ||
				self->curAnimID == ANIM_RUN2||
				self->curAnimID == ANIM_RUN3)
				return true;
			else
				SetAnim(self, ANIM_RUN1);
			break;
		case AI_MOOD_JUMP:
			if(self->jump_chance < irand(0, 100))
				SetAnim(self, ANIM_DELAY);
			else
				SetAnim(self, ANIM_FJUMP);
			break;
		case AI_MOOD_EAT://FIXME: this is not neccessary?
			gorgon_ai_eat(self, 0);
			//QPostMessage(self, MSG_EAT, PRI_DIRECTIVE, NULL);
			break;
		default :
#ifdef _DEVEL
			gi.dprintf("gorgon: Unusable mood %d!\n", self->ai_mood);
#endif
			break;
	}

	return true;
}

void gorgon_check_mood (edict_t *self, G_Message_t *msg)
{
	ParseMsgParms(msg, "i", &self->ai_mood);

	gorgonCheckMood(self);
}

void gorgonbite (edict_t *self)
{
	vec3_t	v;
	float	damage;
	float	melee_range;
	vec3_t	temp, forward, up, melee_point, bite_endpos;
	trace_t trace;

	if(self->ai_mood == AI_MOOD_NAVIGATE)
		return;

	//fixme: do a checkenemy that checks oldenemy & posts messages
	if(!ai_have_enemy(self))
		return;

	AngleVectors(self->s.angles,forward, NULL, up);
	VectorMA(self->s.origin, self->maxs[2]*0.5, up, melee_point);
	VectorMA(melee_point, self->maxs[0], forward, melee_point);

	melee_range = self->s.scale * GORGON_STD_MELEE_RNG * 1.25;//give axtra range
	VectorMA(melee_point, melee_range, forward, bite_endpos);

	//let's do this the right way
	gi.trace(melee_point, vec3_origin, vec3_origin, bite_endpos, self, MASK_SHOT,&trace);
	if (trace.fraction < 1 && !trace.startsolid && !trace.allsolid && trace.ent->takedamage)// A hit
	{
		if (irand(0, 1))
			gi.sound(self, CHAN_WEAPON, sounds[SND_MELEEHIT1], 1, ATTN_NORM, 0);
		else
			gi.sound(self, CHAN_WEAPON, sounds[SND_MELEEHIT2], 1, ATTN_NORM, 0);

		VectorCopy (self->enemy->s.origin, temp);
		temp[2] += 5;

		VectorSubtract(self->enemy->s.origin, self->s.origin, v);
		VectorNormalize(v);

		damage = irand(GORGON_DMG_MIN, GORGON_DMG_MAX) * self->monsterinfo.scale;
		T_Damage (self->enemy, self, self, forward, trace.endpos, v, damage, damage/2, DAMAGE_EXTRA_BLOOD,MOD_DIED);
	}
	else			// A misssss
	{
		if (irand(0, 1))
			gi.sound(self, CHAN_WEAPON, sounds[SND_MELEEMISS1], 1, ATTN_NORM, 0);
		else
			gi.sound(self, CHAN_WEAPON, sounds[SND_MELEEMISS2], 1, ATTN_NORM, 0);
	}
}

void gorgon_dead(edict_t *self)
{
	self->pre_think = NULL;
	self->next_pre_think = -1;
	self->deadState = DEAD_DEAD;
	self->svflags |= SVF_DEADMONSTER;
	M_EndDeath(self);
}

void gorgon_land(edict_t *self)
{
	gi.sound(self, CHAN_WEAPON, sounds[SND_LAND], 1, ATTN_NORM, 0);
}

void gorgon_eatorder (edict_t *self)
{
	if(gorgon_check_attack(self))
		return;

	QPostMessage(self, MSG_EAT, PRI_DIRECTIVE, NULL);
}

#define JUMP_SCALE 1.5

void gorgon_hop (edict_t *self)
{
	vec3_t	forward,right;
	vec3_t	temp;

	if (self->s.frame == FRAME_hop8)
	{
		self->movetype = PHYSICSTYPE_STEP;
		self->velocity[2] = -10;
	}
	else 
	{
		if (self->monsterinfo.currentmove == &gorgon_move_melee6)	// hop left 
		{
			VectorCopy (self->s.angles, temp);
			temp[YAW] -= 15;
			AngleVectors (temp, NULL, right, NULL);
			VectorScale (right, -40*JUMP_SCALE, self->velocity);
		}
		else if (self->monsterinfo.currentmove == &gorgon_move_melee7)  // hop right
		{
			VectorCopy (self->s.angles, temp);
			temp[YAW] += 15;
			AngleVectors (temp, NULL, right, NULL);
			VectorScale (right, 40*JUMP_SCALE, self->velocity);
		}
		else if (self->monsterinfo.currentmove == &gorgon_move_melee8)  // hop forward
		{
			VectorCopy (self->s.angles, temp);
			AngleVectors (temp, forward, NULL, NULL);

			VectorScale (forward, 50*JUMP_SCALE, self->velocity);
		}
		else if (self->monsterinfo.currentmove == &gorgon_move_melee9)  // hop backward
		{
			VectorCopy (self->s.angles, temp);
			AngleVectors (temp, forward, NULL, NULL);
			VectorScale (forward, -50*JUMP_SCALE, self->velocity);
		}
		
		//self->movetype = PHYSICSTYPE_TOSS;
		//self->groundentity = self;
		self->velocity[2] += 175;
	}
}

void gorgonApplyJump (edict_t *self)
{
	if(Vec3IsZero(self->movedir))
	{
		vec3_t	forward;

//		gi.dprintf("Gorgon in FJump with no movedir!!!!\n");
		AngleVectors(self->s.angles, forward, NULL, NULL);
		VectorMA(self->velocity, flrand(200, 400), forward, self->velocity);
		self->velocity[2] = flrand(100, 200);
	}
	else
	{
		self->jump_time = level.time + 0.5;
		VectorCopy(self->movedir, self->velocity);
		VectorClear(self->movedir);
	}
}

void gorgonJumpOutWater (edict_t *self)
{
	vec3_t	endpos, forward;

	if(!self->enemy)
		VectorCopy(self->enemy->s.origin, endpos);
	else
	{
		AngleVectors(self->s.angles, forward, NULL, NULL);
		VectorMA(self->s.origin, 100, forward, endpos);
	}

	endpos[2] += 30;
	VectorSubtract(endpos, self->s.origin, self->velocity);
	self->velocity[2] += 200;
}

void gorgonForward (edict_t *self, float dist)
{
	vec3_t	forward, fdir;
	float	dot;

	AngleVectors(self->s.angles, forward, NULL, NULL);

	VectorCopy(self->velocity, fdir);
	VectorNormalize(fdir);

	dot = DotProduct(fdir, forward);

	dist *= (1 - dot);

	if(dist)
		VectorMA(self->velocity, dist, forward, self->velocity);
}

void gorgonFixPitch (edict_t *self)
{
	MG_ChangePitch(self, 0, 10);
}

void gorgonZeroPitch (edict_t *self)
{
	self->s.angles[PITCH] = 0;
}

void gorgonGoSwim (edict_t *self)
{
	SetAnim(self, ANIM_SWIM);
}

void gorgonCheckInWater (edict_t *self)
{
	trace_t	trace;
	vec3_t	endpos;

	VectorCopy(self->s.origin, endpos);
	endpos[2] -= 32;

	gi.trace(self->s.origin, self->mins, self->maxs, endpos, self, MASK_MONSTERSOLID,&trace);
	if(trace.fraction < 1.0)
	{
		if(trace.contents & CONTENTS_SOLID || trace.contents & CONTENTS_MONSTER)
			SetAnim(self, ANIM_INAIR);
	}
}

void gorgon_check_landed (edict_t *self)
{
	vec3_t pos;
	float save_yspeed;
	int	contents;
	
	save_yspeed = self->yaw_speed;
	self->yaw_speed *= 0.33;
	self->best_move_yaw = vectoyaw(self->velocity);
	MG_ChangeWhichYaw(self, false);//turn toward best_move_yaw, 1/3 as fast as if on ground
	self->yaw_speed = save_yspeed;
	
	if(self->groundentity)
	{
		if(irand(0, 1))
			SetAnim(self, ANIM_LAND2);
		else
			SetAnim(self, ANIM_LAND);
	}
	else if(self->velocity[2]<0)
	{
		VectorCopy(self->s.origin, pos);
		pos[2] += self->mins[2];
		VectorMA(pos, 0.5, self->velocity, pos);
		contents = gi.pointcontents(pos);
		if(contents&CONTENTS_SOLID)
		{
			if(irand(0, 1))
				SetAnim(self, ANIM_LAND2);
			else
				SetAnim(self, ANIM_LAND);
		}
		else if(contents&CONTENTS_WATER)
			SetAnim(self, ANIM_TO_SWIM);
	}
}

void gorgon_go_inair (edict_t *self)
{
	SetAnim(self, ANIM_INAIR);
}

qboolean gorgon_check_jump (edict_t *self)
{
	vec3_t	forward, right, up;
	vec3_t	landing_spot,arc_spot, test_spot;
	vec3_t  angles , v, landing_spot_angles;
	float	hold;
	float	len;
	trace_t	trace;

	if(self->jump_chance < irand(0, 100))
		return false;

	if(!MG_GetTargOrg(self, landing_spot))
	{
		return false;
	}

	if(!infront_pos(self, landing_spot))
		return false;

	VectorSubtract(self->s.origin, landing_spot, v);
	len = VectorLength(v);

	if(len > 400)
	{
		return false;
	}

	if(self->enemy)
		VectorSet(angles, 0, anglemod(-self->enemy->s.angles[YAW]), 0);
	else
		VectorCopy(self->s.angles, angles);

	//incorporate scale?

	// JUMPING
	//   Calculate landing spot behind enemy to jump to
	//   Caclulate arc spot to jump at which will arc the monster to the landing spot
	//   Calculate velocity to make monster jump to hit arc spot

	// choose landing spot behind enemy
	AngleVectors (angles, forward, right, up);

	VectorMA (landing_spot, 60, forward, landing_spot);

	VectorCopy(landing_spot, test_spot);
	test_spot[2] -= 1024;

	gi.trace(landing_spot, self->mins, self->maxs, test_spot, self, MASK_MONSTERSOLID|MASK_WATER,&trace);

	if (trace.fraction == 1.0)
	{
		return false;
	}
	else
	{
		if (!(trace.contents & CONTENTS_SOLID) && !(trace.contents & CONTENTS_WATER))
		{
			return false;
		}
	}

/*	if (trace.startsolid || trace.allsolid)
	{
		if(trace.ent != self->enemy)
			return;
	}*/

	self->jump_time = level.time + 0.5;

	// calculate arc spot (the top of his jump arc) which will land monster at landing spot 
	VectorSubtract (self->s.origin, landing_spot,  v);
	landing_spot_angles[PITCH] = 0;
	landing_spot_angles[ROLL] = 0;
	landing_spot_angles[YAW] = vectoyaw(v);

	AngleVectors (landing_spot_angles, forward, right, up);

	VectorMA (landing_spot, 20, forward, arc_spot);
	VectorMA (landing_spot, 180, up, arc_spot);

	AngleVectors (self->s.angles, forward, right, up);

	// Calculate velocity to make monster jump to hit arc spot
	VectorSubtract (arc_spot,self->s.origin, v);  // Face monster to arc spot
	vectoangles(v,angles);
	self->best_move_yaw = angles[YAW];
		
	len = VectorLength (v);

	hold = len / 200;

	AngleVectors (angles, forward, right, up);
	VectorScale (forward, 300 * hold, self->movedir);

	self->movedir[2] = 200 * hold;

	self->monsterinfo.jump_time = level.time + 3;
	return true;
}

void gorgon_jump (edict_t *self)
{
	vec3_t	forward, right, up;
	vec3_t	landing_spot,arc_spot, test_spot;
	vec3_t  angles , v, landing_spot_angles;
	float	hold;
	float	len;
	trace_t	trace;

	if(!MG_GetTargOrg(self, landing_spot))
	{
		if(!irand(0,3))
			SetAnim(self, ANIM_ROAR2);
		else
			SetAnim(self, ANIM_RUN1);
		return;
	}

	VectorSubtract(self->s.origin, landing_spot, v);
	len = VectorLength(v);

	if(len > 400)
	{
		if(!irand(0,3))
			SetAnim(self, ANIM_ROAR2);
		else
			SetAnim(self, ANIM_RUN1);
		return;
	}

	if(self->enemy)
		VectorSet(angles, 0, anglemod(-self->enemy->s.angles[YAW]), 0);
	else
		VectorCopy(self->s.angles, angles);

	//incorporate scale?

	// JUMPING
	//   Calculate landing spot behind enemy to jump to
	//   Caclulate arc spot to jump at which will arc the monster to the landing spot
	//   Calculate velocity to make monster jump to hit arc spot

	// choose landing spot behind enemy
	AngleVectors (angles, forward, right, up);

	VectorMA (landing_spot, 60, forward, landing_spot);

	VectorCopy(landing_spot, test_spot);
	test_spot[2] -= 1024;

	gi.trace(landing_spot, self->mins, self->maxs, test_spot, self, MASK_MONSTERSOLID|MASK_WATER,&trace);

	if (trace.fraction == 1.0)
	{
		return;
	}
	else
	{
		if (!(trace.contents & CONTENTS_SOLID) && !(trace.contents & CONTENTS_WATER))
		{
			return;
		}
	}

/*	if (trace.startsolid || trace.allsolid)
	{
		if(trace.ent != self->enemy)
			return;
	}*/

	self->jump_time = level.time + 0.5;

	// calculate arc spot (the top of his jump arc) which will land monster at landing spot 
	VectorSubtract (self->s.origin, landing_spot,  v);
	landing_spot_angles[PITCH] = 0;
	landing_spot_angles[ROLL] = 0;
	landing_spot_angles[YAW] = vectoyaw(v);

	AngleVectors (landing_spot_angles, forward, right, up);

	VectorMA (landing_spot, 20, forward, arc_spot);
	VectorMA (landing_spot, 180, up, arc_spot);

	AngleVectors (self->s.angles, forward, right, up);

	// Calculate velocity to make monster jump to hit arc spot
	VectorSubtract (arc_spot,self->s.origin, v);  // Face monster to arc spot
	vectoangles(v,angles);
	self->best_move_yaw = angles[YAW];
		
	len = VectorLength (v);

	hold = len / 200;

	AngleVectors (angles, forward, right, up);
	VectorScale (forward, 300 * hold, self->velocity);

	self->velocity[2] = 200 * hold;

	self->monsterinfo.jump_time = level.time + 3;
}

//Gorgon Evasion!

void gorgon_evade (edict_t *self, G_Message_t *msg)
{
	edict_t			*projectile;
	HitLocation_t	HitLocation;
	int duck_chance, dodgeleft_chance, dodgeright_chance, jump_chance, backflip_chance, frontflip_chance;
	int chance;
	float eta;

	ParseMsgParms(msg, "eif", &projectile, &HitLocation, &eta);
	
	if(eta<0.3)
		return;//needs a .3 seconds to respond, minimum

	switch(HitLocation)
	{
		case hl_Head:
			duck_chance = 30;
			dodgeleft_chance = 50;
			dodgeright_chance = 50;
			jump_chance = 0;
			backflip_chance = 20;
			frontflip_chance = 20;
		break;
		case hl_TorsoFront://split in half?
			duck_chance = 20;
			dodgeleft_chance = 40;
			dodgeright_chance = 40;
			jump_chance = 0;
			backflip_chance = 80;
			frontflip_chance = 0;
		break;
		case hl_TorsoBack://split in half?
			duck_chance = 20;
			dodgeleft_chance = 40;
			dodgeright_chance = 40;
			jump_chance = 0;
			backflip_chance = 0;
			frontflip_chance = 80;
		break;
		case hl_ArmUpperLeft:
			duck_chance = 10;
			dodgeleft_chance = 0;
			dodgeright_chance = 90;
			jump_chance = 0;
			backflip_chance = 20;
			frontflip_chance = 20;
		break;
		case hl_ArmLowerLeft://left arm
			duck_chance = 0;
			dodgeleft_chance = 0;
			dodgeright_chance = 80;
			jump_chance = 30;
			backflip_chance = 20;
			frontflip_chance = 20;
		break;
		case hl_ArmUpperRight:
			duck_chance = 20;
			dodgeleft_chance = 90;
			dodgeright_chance = 0;
			jump_chance = 0;
			backflip_chance = 20;
			frontflip_chance = 20;
		break;
		case hl_ArmLowerRight://right arm
			duck_chance = 0;
			dodgeleft_chance = 80;
			dodgeright_chance = 0;
			jump_chance = 30;
			backflip_chance = 20;
			frontflip_chance = 20;
		break;
		case hl_LegUpperLeft:
			duck_chance = 0;
			dodgeleft_chance = 0;
			dodgeright_chance = 60;
			jump_chance = 50;
			backflip_chance = 30;
			frontflip_chance = 30;
		break;
		case hl_LegLowerLeft://left leg
			duck_chance = 0;
			dodgeleft_chance = 0;
			dodgeright_chance = 30;
			jump_chance = 90;
			backflip_chance = 60;
			frontflip_chance = 60;
		break;
		case hl_LegUpperRight:
			duck_chance = 0;
			dodgeleft_chance = 60;
			dodgeright_chance = 0;
			jump_chance = 50;
			backflip_chance = 30;
			frontflip_chance = 30;
		break;
		case hl_LegLowerRight://right leg
			duck_chance = 0;
			dodgeleft_chance = 30;
			dodgeright_chance = 0;
			jump_chance = 90;
			backflip_chance = 30;
			frontflip_chance = 30;
		break;
		default:
			duck_chance = 5;
			dodgeleft_chance = 10;
			dodgeright_chance = 10;
			jump_chance = 10;
			backflip_chance = 10;
			frontflip_chance = 10;
		break;
	}
	if(self->jump_chance < 0)
		jump_chance = -1;

	chance = irand(0, 100);
	if(chance < frontflip_chance)
	{
		SetAnim(self, ANIM_MELEE8);//hop forward
		return;
	}

	chance = irand(0, 100);
	if(chance < backflip_chance)
	{
		if(self->curAnimID == ANIM_RUN1&&irand(0,10)<8)//running, do the front jump
		{
			SetAnim(self, ANIM_MELEE10);//jump forward
		}
		else
		{
			SetAnim(self, ANIM_MELEE9);//hop forward
		}
		return;
	}

	chance = irand(0, 100);
	if(chance < dodgeleft_chance)
	{
		SetAnim(self, ANIM_MELEE6);//hop left
		return;
	}
	
	chance = irand(0, 100);
	if(chance < dodgeright_chance)
	{
		SetAnim(self, ANIM_MELEE7);//hop left
		return;
	}
	
	chance = irand(0, 100);
	if(chance < jump_chance)
	{
		SetAnim(self, ANIM_MELEE10);//jump forward
		return;
	}

	chance = irand(0, 100);
	if(chance < duck_chance)
	{
		SetAnim(self, ANIM_PAIN1);//jump forward
		return;
	}
}

/*
========================================
GORGON PICK UP AND GORE SOMETHING
========================================
*/
void gorgon_ready_catch (edict_t *self)
{
	float enemy_zdist, ok_zdist;

	if(!ai_have_enemy(self))
	{
		SetAnim(self,ANIM_CATCH);
		return;
	}
	
	ok_zdist = 128 * (self->s.scale*0.5/2.5);
	if(ok_zdist<48)
		ok_zdist = 48;

	enemy_zdist = self->enemy->absmin[2] - self->absmax[2];

	if(enemy_zdist <= 0 || (enemy_zdist <= ok_zdist && self->enemy->velocity[2] <= -60))
		SetAnim(self,ANIM_CATCH);
	else
		SetAnim(self,ANIM_READY_CATCH);
}

void gorgon_throw_toy(edict_t *self)
{
	if(!self->enemy)
		return;

	self->enemy->flags &= ~FL_FLY;
	self->enemy->velocity[0] = self->enemy->velocity[1] = 0;
	self->enemy->velocity[2] = 500;
	if(self->enemy->movetype>NUM_PHYSICSTYPES)
		self->enemy->movetype = PHYSICSTYPE_STEP;
	VectorRandomCopy(vec3_origin,self->enemy->avelocity,300);
	
	if(stricmp(self->enemy->classname,"player"))
		QPostMessage(self->enemy, MSG_DEATH, PRI_DIRECTIVE, NULL);

	//FIXME: What if I miss?  Set the monster's touch to
	//	something that restores it's angles and normal thinking (AI_FLEE)
/*	if(!stricmp(self->enemy->classname,"player"))
	{
		PlayerAnimSetUpperSeq(self->enemy, 92);
		PlayerAnimSetLowerSeq(&self->enemy->client->playerinfo, 92);
	}*/
}

void gorgon_toy_ofs(edict_t *self, float ofsf, float ofsr, float ofsu)
{
	vec3_t enemy_ofs, forward, right, up, blooddir, enemy_face;

	if(!self->enemy)
		return;

	//adjust for scale
	ofsf *= self->s.scale*0.5/2.5;
	ofsr *= self->s.scale*0.5/2.5;
	ofsu += self->mins[2];//because origin moved up since those were calced
	ofsu *= self->s.scale*0.25/2.5;

	AngleVectors(self->s.angles, forward, right, up);
	VectorMA(self->s.origin, ofsf, forward, enemy_ofs);
	VectorMA(enemy_ofs, ofsr, right, enemy_ofs);
	VectorMA(enemy_ofs, ofsu, up, self->enemy->s.origin);
	VectorSubtract(self->enemy->s.origin, self->s.origin, blooddir);
	
	VectorScale(blooddir, -1, enemy_face);
	enemy_face[2]/=10;
	vectoangles(enemy_face, self->enemy->s.angles);

	switch(self->enemy->count)
	{
		case 1:
			self->enemy->s.angles[PITCH]=anglemod(self->enemy->s.angles[PITCH]+90);//can't do roll?
			break;
		case 2:
			self->enemy->s.angles[PITCH]=anglemod(self->enemy->s.angles[PITCH]-90);//can't do roll?
			break;
		case 3:
			self->enemy->s.angles[ROLL]=anglemod(self->enemy->s.angles[ROLL]+90);//can't do roll?
			break;
		case 4:
			self->enemy->s.angles[ROLL]=anglemod(self->enemy->s.angles[ROLL]-90);//can't do roll?
			break;
		default:
			break;
	}


	VectorClear(self->enemy->velocity);
	VectorClear(self->enemy->avelocity);

	if(flrand(0,1)<0.5)
	{
		if(self->enemy->materialtype == MAT_INSECT)
			gi.CreateEffect(&self->enemy->s, FX_BLOOD, CEF_FLAG8, self->enemy->s.origin, "ub", blooddir, 200);
		else
			gi.CreateEffect(&self->enemy->s, FX_BLOOD, 0, self->enemy->s.origin, "ub", blooddir, 200);
	}
}

void gorgon_check_snatch(edict_t *self, float ofsf, float ofsr, float ofsu)
{
	float enemy_dist, ok_zdist;
	vec3_t forward, right, up, startpos, endpos;
	trace_t trace;

	//adjust for scale
	ok_zdist = 64 * (self->s.scale*0.5/2.5);
	if(ok_zdist<24)
		ok_zdist = 24;

	AngleVectors(self->s.angles,forward,right,up);
	VectorMA(self->s.origin, self->maxs[0], forward, startpos);
	VectorMA(startpos, self->maxs[2]*0.5, up, startpos);
	VectorCopy(startpos, endpos);
	VectorMA(endpos, ofsf, forward, endpos);
	VectorMA(endpos, ofsr, right, endpos);
	VectorMA(endpos, ofsu, up, endpos);
	gi.trace(startpos,vec3_origin,vec3_origin,endpos, self, MASK_SHOT,&trace);
	VectorCopy(trace.endpos,endpos);
	
	VectorSubtract(self->enemy->s.origin, endpos, endpos);
	enemy_dist = VectorLength(endpos);
	if(enemy_dist>ok_zdist)
	{
//		gi.dprintf("Snatch missed (%4.2f)\n", enemy_dist);
		self->msgHandler = DefaultMsgHandler;
		if(!stricmp(self->enemy->classname,"player"))
			if(self->oldenemy)
				if(self->oldenemy->health>0)
				{
					self->oldenemy = NULL;
					self->enemy = self->oldenemy;
				}

		if(self->curAnimID == ANIM_SNATCHLOW)
			SetAnim(self,ANIM_MISS);
		else
			SetAnim(self,ANIM_MELEE2);//?
		return;
	}
//	gi.dprintf("SNAGGED!\n");
	//FIXME: if health is low, just chomp it now
	self->enemy->flags |= FL_FLY;
	if(self->enemy->movetype>NUM_PHYSICSTYPES)
		self->enemy->movetype = PHYSICSTYPE_FLY;

	if(stricmp(self->enemy->classname,"player"))
	{
		self->enemy->monsterinfo.aiflags |= AI_DONT_THINK;
		self->enemy->count = irand(1,5);
	}
	else
		self->enemy->nextthink = level.time + 10;//stuck for 10 seconds.

	VectorClear(self->enemy->velocity);
	VectorClear(self->enemy->avelocity);
}

void gorgon_gore_toy(edict_t *self, float jumpht)
{
	float enemy_zdist, ok_zdist;
	byte num_chunks;
	vec3_t dir, forward;

	if(jumpht!=-1)
	{//not getting here
		self->velocity[2] += jumpht;
		if(self->groundentity)
		{
			AngleVectors(self->s.angles, forward, NULL, NULL);
			VectorMA(self->velocity, -100, forward, self->velocity);
		}
	}
	else
		self->count = 0;

	if(!self->enemy)
		return;

	if(self->enemy->health<0)
		return;

	if(self->count)
		return;

	ok_zdist = 56 * (self->s.scale*0.5/2.5);
	if(ok_zdist<36)
		ok_zdist = 36;
	enemy_zdist = self->enemy->s.origin[2] - self->s.origin[2];
	if(enemy_zdist <= self->maxs[2] + ok_zdist || jumpht == -1)
	{
		gi.sound(self, CHAN_WEAPON, sounds[SND_MELEEMISS2], 1, ATTN_NORM, 0);
		if(jumpht!=-1)
			self->count = 1;
		VectorCopy(self->velocity,dir);
		VectorNormalize(dir);
		if(self->enemy->materialtype != MAT_FLESH)//foo
			self->enemy->materialtype = MAT_FLESH;
		num_chunks = (byte)(self->enemy->health/4);
		if(num_chunks>15)
			num_chunks = 15;
		SprayDebris(self->enemy, self->enemy->s.origin, num_chunks, self->enemy->health*4);//self->enemy is thingtype wood?!

		if(stricmp(self->enemy->classname,"player"))
		{
			gi.sound(self->enemy, CHAN_BODY, sounds[SND_GIB], 1, ATTN_NORM, 0);
			BecomeDebris(self->enemy);
		}
		else
		{
			self->enemy->nextthink = level.time;
			T_Damage (self->enemy, self, self, self->velocity, self->enemy->s.origin, dir, 2000, 300, 0,MOD_DIED);
		}
	}
}

void gorgon_miss_sound (edict_t *self)
{
	if (irand(0, 1))
		gi.sound(self, CHAN_WEAPON, sounds[SND_MELEEMISS1], 1, ATTN_NORM, 0);
	else
		gi.sound(self, CHAN_WEAPON, sounds[SND_MELEEMISS2], 1, ATTN_NORM, 0);
}

void gorgon_anger_sound (edict_t *self)
{
	byte chance;
	vec3_t spot;

	chance = irand(0,100);
	if (chance < 10)
		gi.sound(self, CHAN_VOICE, sounds[SND_PAIN1], 1, ATTN_NORM, 0);
	else if (chance < 20)
		gi.sound(self, CHAN_VOICE, sounds[SND_PAIN2], 1, ATTN_NORM, 0);
	else if (chance < 30)
		gi.sound(self, CHAN_WEAPON, sounds[SND_MELEEHIT1], 1, ATTN_NORM, 0);
	else if (chance < 40)
		gi.sound(self, CHAN_WEAPON, sounds[SND_MELEEHIT2], 1, ATTN_NORM, 0);
	else if (chance < 50)
		gi.sound(self, CHAN_VOICE, sounds[SND_DIE], 1, ATTN_NORM, 0);
	else if (chance < 60)
		gorgon_growl(self);

	if(self->enemy)
	{
		chance = (byte)irand(1,3);
		VectorCopy(self->enemy->s.origin, spot);

		QPostMessage(self->enemy,MSG_DISMEMBER,PRI_DIRECTIVE,"ii", self->enemy->health*0.5, irand(1,13));//do I need last three if not sending them?
		QPostMessage(self->enemy,MSG_PAIN,PRI_DIRECTIVE,"ii", self->enemy, self);//do I need last three if not sending them?
	}
}

void gorgon_go_snatch (edict_t *self)
{
	SetAnim(self,ANIM_SNATCH);
}

void gorgon_done_gore (edict_t *self)
{
	self->msgHandler = DefaultMsgHandler;
	self->count = 0;
	if(self->oldenemy)
	{
		if(self->oldenemy->health>0)
		{
			self->enemy = self->oldenemy;
			QPostMessage(self, MSG_RUN, PRI_DIRECTIVE, NULL);
			return;
		}
	}
	SetAnim(self,ANIM_EAT_LOOP);
}

/* ========================================
Trip and fall if making too tight a turn, wont work until origin is in center
========================================== */

void gorgonRoll (edict_t *self, float rollangle)
{//FIXME: cannot interrupt!!!
	self->s.angles[ROLL] = anglemod(rollangle);
}

void gorgonLerpOff (edict_t *self)
{
	self->s.renderfx &= ~RF_FRAMELERP;
}

void gorgonLerpOn (edict_t *self)
{
	self->s.renderfx |= RF_FRAMELERP;
}

qboolean gorgonCheckSlipGo (edict_t *self, qboolean frompain)
{
	vec3_t	v, right;

	if(!self->enemy)
		return false;

	VectorSubtract (self->enemy->s.origin, self->s.origin, v);
	VectorNormalize(v);
	AngleVectors(self->s.angles, NULL, right, NULL);
	if(DotProduct(right, v) > 0.3 && irand(0, 1))
	{//fall down, go boom
		if(frompain)
		{
			SetAnim(self, ANIM_SLIP_PAIN);
			return true;
		}
		else if(self->monsterinfo.misc_debounce_time < level.time && !irand(0, 4))
		{
			self->monsterinfo.misc_debounce_time = level.time + 7;
			SetAnim(self, ANIM_SLIP);
			return true;
		}
	}
	
	return false;
}

void gorgonCheckSlip (edict_t *self)
{
	if(!(self->spawnflags & MSF_GORGON_SPEEDY) && self->s.scale > 0.75)
	{
		gorgonCheckMood(self);
		return;
	}

	if(!gorgonCheckSlipGo (self, false))
		gorgonCheckMood(self);
}

void gorgonSlide (edict_t *self, float force)
{
	vec3_t right;

	if(!self->groundentity)
		return;//already in air

	if(force == 0)
	{
		self->friction = 1.0;
		return;
	}

	AngleVectors(self->s.angles, NULL, right, NULL);
	VectorMA(self->velocity, force, right, self->velocity);
	self->velocity[2] = 50;
	self->friction = 0.2;
}

void gorgonChooseDeath (edict_t *self)
{
	if(irand(0, 1))
		gorgon_death2twitch(self);
}

void gorgon_ai_swim (edict_t *self, float dist)
{
	vec3_t	dir = {0, 0, 0};	
	vec3_t	vec, angles;

	gorgon_prethink(self);
	self->pre_think = gorgon_prethink;
	self->next_pre_think = level.time + 0.1;

	self->mood_think(self);
//	MG_Pathfind(self, false);

	MG_SetNormalizeVelToGoal(self, dir);

	if(Vec3IsZero(dir))
	{
//		gi.dprintf("swimming gorgon couldn't find a target\n");
		Vec3ScaleAssign(0.8, self->velocity);
		return;
	}

	self->ideal_yaw = vectoyaw(dir);
	MG_ChangeYaw(self);

	if(dist == -1)
		Vec3ScaleAssign(150, dir);
	else
		Vec3ScaleAssign(dist * 3, dir);

	VectorAdd(self->velocity, dir, self->velocity);
	VectorNormalize(self->velocity);
	Vec3ScaleAssign(200, self->velocity);

	if(!self->enemy)
		return;

	VectorSubtract(self->enemy->s.origin, self->s.origin, vec);
	vectoangles(vec, angles);
	MG_ChangePitch(self, angles[PITCH], 10);

	//MG_ChangePitchForZVel(self, 10, dist * 3, 60);

	if(dist != -1)
	{//-1 = charge
		if(self->monsterinfo.attack_finished < level.time)
		{
			if(M_ValidTarget(self, self->enemy))
			{
				if(clear_visible(self, self->enemy))
				{
					if(infront(self, self->enemy))
					{
						VectorSubtract(self->enemy->s.origin, self->s.origin, dir);
						dist = VectorLength(dir);

						if(dist < self->melee_range + VectorLength(self->velocity) * 0.1)
						{
							if(irand(0, 1))
								SetAnim(self, ANIM_SWIM_BITE_A);
							else
								SetAnim(self, ANIM_SWIM_BITE_B);
							self->monsterinfo.attack_finished = level.time + flrand(0, 3 - skill->value);
						}
						else if(self->monsterinfo.jump_time < level.time)
						{
							if(!(self->enemy->flags & FL_INWATER))
							{
								if(dist < GORGON_STD_MAXHOP_RNG * 2)
									SetAnim(self, ANIM_OUT_WATER);
							}
						}
					}
				}
			}
		}
	}
}

void gorgon_prethink (edict_t *self)
{//also make wake on surface of water?
	if(self->flags & FL_INWATER)
	{
		self->gravity = 0.0f;
		self->svflags |= SVF_TAKE_NO_IMPACT_DMG | SVF_DO_NO_IMPACT_DMG;
	
		if(!self->wait)
		{
			gi.CreateEffect(NULL, FX_WATER_ENTRYSPLASH, CEF_FLAG7,
				self->s.origin,	"bd", 128|96, vec3_up);
			self->wait = true;
		}

		if(self->curAnimID == ANIM_INAIR)
			SetAnim(self, ANIM_TO_SWIM);
	}
	else
	{
		gi.RemoveEffects (&self->s, FX_M_EFFECTS);
		self->gravity = 1.0f;
		self->svflags &= ~SVF_TAKE_NO_IMPACT_DMG;
		if(self->s.scale>0.5)
			self->svflags &= ~SVF_DO_NO_IMPACT_DMG;

		if(self->wait)
		{
			gi.CreateEffect(NULL, FX_WATER_ENTRYSPLASH, 0,
				self->s.origin,	"bd", 128|96, vec3_up);
			self->wait = false;
		}

		if(self->curAnimID == ANIM_SWIM ||
			self->curAnimID == ANIM_SWIM_BITE_A ||
			self->curAnimID == ANIM_SWIM_BITE_B)
			SetAnim(self, ANIM_RUN1);

		gorgonFixPitch(self);
	}
	self->next_pre_think = level.time + 0.1;
}

void gorgon_ai_eat(edict_t *self, float crap)
{//fixme: crap will be a yaw mod for view_ofs looking around
	vec3_t	forward, right, v;
	float	edist, fdot, rdot;

	if(self->enemy)
	{
		VectorSubtract(self->enemy->s.origin, self->s.origin, v);
		edist = VectorNormalize(v);
		if(edist<self->wakeup_distance)
		{
			if(visible(self, self->enemy))
			{
				self->spawnflags &= ~MSF_EATING;
				self->monsterinfo.aiflags &= ~AI_EATING;
				FoundTarget(self, true);
				return;
			}
		}
		else if(self->curAnimID == ANIM_EAT_LOOP && !irand(0, 5))
		{
			if(visible(self, self->enemy))
			{
				AngleVectors(self->s.angles, forward, right, NULL);
				fdot = DotProduct(v, forward);
				rdot = DotProduct(v, right);
				if(fdot < 0)
				{
					if(rdot > 0.3)
						SetAnim(self, ANIM_EAT_RIGHT);
					if(rdot < -0.3)
						SetAnim(self, ANIM_EAT_LEFT);
					else
						SetAnim(self, ANIM_EAT_UP);
					return;
				}
			}
		}
	}
	else
		FindTarget(self);


	if(crap != -1)
		return;

	switch(self->curAnimID)
	{
	case ANIM_EAT_DOWN:
		SetAnim(self, ANIM_EAT_LOOP);
		break;

	case ANIM_EAT_UP:
		SetAnim(self, ANIM_LOOK_AROUND);
		break;
	
	case ANIM_EAT_LOOP:
		if(irand(0, 1))
			SetAnim(self, ANIM_EAT_LOOP);
		else if(irand(0, 1))
			SetAnim(self, ANIM_EAT_PULLBACK);
		else if(irand(0, 1))
			SetAnim(self, ANIM_EAT_TEAR);
		else if(irand(0, 1))//fixme- check gorgon to right
			SetAnim(self, ANIM_EAT_SNAP);
		else if(irand(0, 1))//fixme- check if gorgon to left snapped
			SetAnim(self, ANIM_EAT_REACT);
		else if(irand(0, 1))//fixme- check enemy
			SetAnim(self, ANIM_EAT_LEFT);
		else//fixme- check enemy
			SetAnim(self, ANIM_EAT_RIGHT);
		break;
	
	case ANIM_EAT_TEAR:
		SetAnim(self, ANIM_EAT_LOOP);
		break;
	
	case ANIM_EAT_PULLBACK:
		SetAnim(self, ANIM_EAT_LOOP);
		break;
	
	case ANIM_LOOK_AROUND:
		SetAnim(self, ANIM_EAT_DOWN);
		break;
	
	case ANIM_EAT_LEFT:
		SetAnim(self, ANIM_EAT_LOOP);
		break;
	
	case ANIM_EAT_RIGHT:
		SetAnim(self, ANIM_EAT_LOOP);
		break;
	
	case ANIM_EAT_SNAP:
		SetAnim(self, ANIM_EAT_LOOP);
		break;
	
	case ANIM_EAT_REACT:
		SetAnim(self, ANIM_EAT_LOOP);
		break;
	}
}

void gorgon_jump_msg (edict_t *self, G_Message_t *msg)
{
	if(self->jump_chance < irand(0, 100))
		return;
	SetAnim(self, ANIM_FJUMP);
}

void GorgonStaticsInit()
{
	classStatics[CID_GORGON].msgReceivers[MSG_STAND] = gorgon_stand;
	classStatics[CID_GORGON].msgReceivers[MSG_WALK] = gorgon_walk;
	classStatics[CID_GORGON].msgReceivers[MSG_RUN] = gorgon_run;
	classStatics[CID_GORGON].msgReceivers[MSG_EAT] = gorgon_eat;
	classStatics[CID_GORGON].msgReceivers[MSG_MELEE] = gorgon_melee;
	classStatics[CID_GORGON].msgReceivers[MSG_MISSILE] = gorgon_melee;
	classStatics[CID_GORGON].msgReceivers[MSG_WATCH] = gorgon_walk;
	classStatics[CID_GORGON].msgReceivers[MSG_PAIN] = gorgon_pain;
	classStatics[CID_GORGON].msgReceivers[MSG_DEATH] = gorgon_death;
	classStatics[CID_GORGON].msgReceivers[MSG_JUMP]=gorgon_jump_msg;
	classStatics[CID_GORGON].msgReceivers[MSG_DEATH_PAIN] = gorgon_death_pain;
	classStatics[CID_GORGON].msgReceivers[MSG_CHECK_MOOD] = gorgon_check_mood;
	classStatics[CID_GORGON].msgReceivers[MSG_VOICE_POLL] = gorgon_roar_response;
	classStatics[CID_GORGON].msgReceivers[MSG_EVADE] = gorgon_evade;

	resInfo.numAnims = NUM_ANIMS;
	resInfo.animations = animations;
	resInfo.modelIndex = gi.modelindex("models/monsters/gorgon/tris.fm");

	sounds[SND_PAIN1]= gi.soundindex ("monsters/gorgon/pain1.wav");	
	sounds[SND_PAIN2]= gi.soundindex ("monsters/gorgon/pain2.wav");	
	sounds[SND_DIE]= gi.soundindex ("monsters/gorgon/death1.wav");	
	sounds[SND_GURGLE]= gi.soundindex ("monsters/gorgon/gurgle.wav");	//what is this for?
	sounds[SND_GIB]= gi.soundindex ("monsters/gorgon/gib.wav");

	sounds[SND_MELEEHIT1]= gi.soundindex ("monsters/gorgon/meleehit1.wav");	
	sounds[SND_MELEEHIT2] = gi.soundindex ("monsters/gorgon/meleehit2.wav");	
	sounds[SND_MELEEMISS1] = gi.soundindex ("monsters/gorgon/meleemiss1.wav");	
	sounds[SND_MELEEMISS2] = gi.soundindex ("monsters/gorgon/meleemiss2.wav");	

	sounds[SND_STEP1]  = gi.soundindex ("monsters/gorgon/footstep1.wav");	
	sounds[SND_STEP2] = gi.soundindex ("monsters/gorgon/footstep2.wav");	
	sounds[SND_STEP3]  = gi.soundindex ("monsters/gorgon/footstep3.wav");	
	sounds[SND_STEP4]  = gi.soundindex ("monsters/gorgon/footstep4.wav");	

	sounds[SND_GROWL1]     = gi.soundindex ("monsters/gorgon/growl1.wav");	
	sounds[SND_GROWL2]     = gi.soundindex ("monsters/gorgon/growl2.wav");	
	sounds[SND_GROWL3]     = gi.soundindex ("monsters/gorgon/growl3.wav");	

	sounds[SND_LAND]     = gi.soundindex ("monsters/gorgon/land.wav");	

	resInfo.numSounds = NUM_SOUNDS;
	resInfo.sounds = sounds;

	classStatics[CID_GORGON].resInfo = &resInfo;
}

/*QUAKED monster_gorgon_leader (1 .5 0) (-16 -16 -0) (16 16 32) AMBUSH ASLEEP EATING 8 16 32 64 128

The gorgon leader

*/
void SP_monster_gorgon_leader (edict_t *self)
{
	float scale;

	G_SetToFree(self);
	return;
	// Generic Monster Initialization
	if (!monster_start(self))
		return;					// Failed initialization

	self->msgHandler = DefaultMsgHandler;
	self->classID = CID_GORGON;
	self->think = walkmonster_start_go;
	self->monsterinfo.aiflags |= AI_BRUTAL|AI_AGRESSIVE|AI_SHOVE;

	if (!self->health)
		self->health = GORGON_LEADER_HEALTH;

	self->max_health = self->health = MonsterHealth(self->health);

	self->mass = GORGON_LEADER_MASS;
	self->yaw_speed = 10;
	self->isBlocked = gorgon_blocked;

	self->movetype=PHYSICSTYPE_STEP;
	VectorClear(self->knockbackvel);

	self->solid=SOLID_BBOX;
	self->materialtype = MAT_FLESH;

	VectorSet(self->mins,-42,-42,0);//-48,-48,0
	VectorSet(self->maxs,42,42,56);//48,48,64
	self->viewheight = self->maxs[2]*0.8;

	self->s.modelindex = classStatics[CID_GORGON].resInfo->modelIndex;

	//Big guy can be stood on top of perhaps?
	//self->touch = M_Touch;

	self->s.skinnum = GORGON_SKIN;
	
	scale = 2;//flrand(0.9, 1.4);
	
	if (!self->s.scale)
	{
		self->monsterinfo.scale = self->s.scale = scale;
	}
	
	self->monsterinfo.aiflags |= AI_NIGHTVISION;

	VectorScale(self->mins, scale, self->mins);
	VectorScale(self->maxs, scale, self->maxs);
	QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
	
	MG_InitMoods(self);
	self->svflags |= SVF_WAIT_NOTSOLID;
}


/*QUAKED monster_gorgon (1 .5 0) (-16 -16 0) (16 16 32) AMBUSH ASLEEP EATING SPEEDY 16 32 64 128 WANDER MELEE_LEAD STALK COWARD EXTRA1 EXTRA2 EXTRA3 EXTRA4

The gorgon

AMBUSH - Will not be woken up by other monsters or shots from player

ASLEEP - will not appear until triggered

EATING - Chomp chomp... chewie chomp (wakeup_distance will default to 300)

SPEEDY - generally faster gorgon

WANDER - Monster will wander around aimlessly (but follows buoys)

MELEE_LEAD - Monster will tryto cut you off when you're running and fighting him, works well if there are a few monsters in a group, half doing this, half not

STALK - Monster will only approach and attack from behind- if you're facing the monster it will just stand there.  Once the monster takes pain, however, it will stop this behaviour and attack normally

COWARD - Monster starts off in flee mode- runs away from you when woken up

"homebuoy" - monsters will head to this buoy if they don't have an enemy ("homebuoy" should be targetname of the buoy you want them to go to)

"wakeup_target" - monsters will fire this target the first time it wakes up (only once)

"pain_target" - monsters will fire this target the first time it gets hurt (only once)

mintel - monster intelligence- this basically tells a monster how many buoys away an enemy has to be for it to give up.

melee_range - How close the player has to be, maximum, for the monster to go into melee.  If this is zero, the monster will never melee.  If it is negative, the monster will try to keep this distance from the player.  If the monster has a backup, he'll use it if too clode, otherwise, a negative value here means the monster will just stop running at the player at this distance.
	Examples:
		melee_range = 60 - monster will start swinging it player is closer than 60
		melee_range = 0 - monster will never do a mele attack
		melee_range = -100 - monster will never do a melee attack and will back away (if it has that ability) when player gets too close

missile_range - Maximum distance the player can be from the monster to be allowed to use it's ranged attack.

min_missile_range - Minimum distance the player can be from the monster to be allowed to use it's ranged attack.

bypass_missile_chance - Chance that a monster will NOT fire it's ranged attack, even when it has a clear shot.  This, in effect, will make the monster come in more often than hang back and fire.  A percentage (0 = always fire/never close in, 100 = never fire/always close in).- must be whole number

jump_chance - every time the monster has the opportunity to jump, what is the chance (out of 100) that he will... (100 = jump every time)- must be whole number

wakeup_distance - How far (max) the player can be away from the monster before it wakes up.  This just means that if the monster can see the player, at what distance should the monster actually notice him and go for him.

DEFAULTS:
mintel					= 20
melee_range				= 48
missile_range			= 0
min_missile_range		= 0
bypass_missile_chance	= 0
jump_chance				= 80
wakeup_distance			= 1024

NOTE: A value of zero will result in defaults, if you actually want zero as the value, use -1
*/
void SP_monster_gorgon (edict_t *self)
{
	//fix some spawnflags
	if (self->spawnflags & MSF_GORGON_COWARD)
	{
		self->spawnflags &= ~MSF_GORGON_COWARD;
		self->spawnflags |= MSF_COWARD;
	}

	// Generic Monster Initialization
	if (!walkmonster_start(self))		// Failed initialization
		return;

	self->msgHandler = DefaultMsgHandler;
	self->classID = CID_GORGON;
	self->materialtype = MAT_FLESH;
	self->touch = M_Touch;

	self->mass = GORGON_MASS;
	if(self->spawnflags & MSF_GORGON_SPEEDY)
		self->yaw_speed = 30;
	else
		self->yaw_speed = 15;
	self->dmg = 0;//used for slight turn during run

	self->movetype=PHYSICSTYPE_STEP;
	VectorClear(self->knockbackvel);
	self->solid=SOLID_BBOX;

	if(irand(0, 1))
		self->ai_mood_flags |= AI_MOOD_FLAG_PREDICT;

	VectorCopy(STDMinsForClass[self->classID], self->mins);
	VectorCopy(STDMaxsForClass[self->classID], self->maxs);	

	self->s.modelindex = classStatics[CID_GORGON].resInfo->modelIndex;

	self->s.skinnum = GORGON_SKIN;

	self->monsterinfo.otherenemyname = "monster_rat";


	if (self->spawnflags & MSF_COWARD)
	{
		if(!self->health)
			self->health = GORGON_HEALTH/2;

		self->monsterinfo.aiflags |= AI_COWARD;
		self->monsterinfo.scale = self->s.scale = 0.5;
	}
	else
	{
		if (!self->health)
			self->health = GORGON_HEALTH;

		if (!self->s.scale)
			self->s.scale = flrand(GORGON_SCALE_MIN, GORGON_SCALE_MAX);
		self->monsterinfo.scale = self->s.scale;
	}
	
	self->max_health = self->health = MonsterHealth(self->health);

	if (self->spawnflags & MSF_EATING)
	{
		self->monsterinfo.aiflags |= AI_EATING;
		QPostMessage(self, MSG_EAT, PRI_DIRECTIVE, NULL);
		if(!self->wakeup_distance)
			self->wakeup_distance = 300;
	}
	else
	{
		QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
	}

	MG_InitMoods(self);

	self->svflags |= SVF_WAIT_NOTSOLID;
	self->flags |= FL_AMPHIBIAN;
	self->monsterinfo.aiflags |= AI_SWIM_OK;

	self->monsterinfo.roared = false;

	if(!irand(0, 2))//33% chance of not making a wakeup roar
		self->dmg_radius = true;
	else
		self->dmg_radius = false;

	self->pre_think = gorgon_prethink;
	self->next_pre_think = level.time + 0.1;
}
