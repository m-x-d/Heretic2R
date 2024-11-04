/*-------------------------------------------------------------------
m_imp.c

Heretic II
Copyright 1998 Raven Software

  AI:

  HOVER1	:hovering in midair
  FLY1		:flying forwards
  FLYBACK	:flying backwards

-------------------------------------------------------------------*/

#include "g_local.h"
#include "m_imp.h"
#include "m_imp_anim.h"
#include "Utilities.h"
#include "g_DefaultMessageHandler.h"
#include "g_monster.h"
#include "Random.h"
#include "vector.h"
#include "fx.h"
#include "g_HitLocation.h"
#include "g_misc.h"
#include "m_stats.h"
#include "p_anim_branch.h"
#include "g_playstats.h"
#include "p_actions.h"

#define IMP_CHECK_DIST		128
#define	IMP_COLLISION_DIST	148
#define IMP_MIN_SSWOOP_DIST	128
#define IMP_MIN_HOVER_DIST	128
#define IMP_MAX_HOVER_DIST	512
#define IMP_MIN_SWOOP_DIST	108

#define IMP_DRIFT_AMOUNT_X	128
#define IMP_DRIFT_AMOUNT_Y	128
#define IMP_DRIFT_AMOUNT_Z	64

#define IMP_SWOOP_INCR		2
#define	IMP_SWOOP_SPEED_MAX	512

#define IMP_PROJECTILE_RADIUS	1024

void imp_blocked (edict_t *self, struct trace_s *trace);

/*-----------------------------------------------------------------
	imp base info
-----------------------------------------------------------------*/

static animmove_t *animations[NUM_ANIMS] =
{
	&imp_move_die1, 
	&imp_move_fly1,
	&imp_move_flyback,
	&imp_move_hover1,
	&imp_move_fireball,	
	&imp_move_dive_go,
	&imp_move_dive_loop,
	&imp_move_dive_end,
	&imp_move_dive_out,
	&imp_move_pain1,
	&imp_move_tumble,	
	&imp_move_perch,
	&imp_move_takeoff,
	&imp_move_dup,
	&imp_move_ddown,
};

static int sounds[NUM_SOUNDS];
static ClassResourceInfo_t resInfo;


/*===============================================================

	Imp Helper Functions

===============================================================*/

void imp_blocked (edict_t *self, struct trace_s *trace)
{
	vec3_t	vf;
	int		damage;

	if (self->health <= 0)
		return;

	if (!trace->ent)
		return;

	if(self->curAnimID == ANIM_DIVE_GO || self->curAnimID == ANIM_DIVE_LOOP || self->curAnimID == ANIM_DIVE_END)
	{
		if(!stricmp(trace->ent->classname, "player"))
		{
			if(!irand(0,4))
				P_KnockDownPlayer(&trace->ent->client->playerinfo);
		}
		damage = irand(IMP_DMG_MIN, IMP_DMG_MAX);
		T_Damage (trace->ent, self, self, vf, trace->ent->s.origin, trace->plane.normal, damage, damage*2, 0,MOD_DIED);
		gi.sound(self, CHAN_BODY, sounds[SND_HIT], 1, ATTN_NORM, 0);
		if(self->curAnimID != ANIM_DIVE_END)
			SetAnim(self, ANIM_DIVE_END);
	}
}

//Various sound functions
void imp_flap_noise(edict_t *self)
{
	gi.sound(self,CHAN_ITEM,sounds[SND_FLAP],1,ATTN_NORM,0);
}

void imp_death_noise(edict_t *self)
{
	gi.sound(self,CHAN_VOICE,sounds[SND_DEATH],1,ATTN_NORM,0);
}

void imp_dive_noise(edict_t *self)
{
	gi.sound(self,CHAN_VOICE,sounds[SND_DIVE],1,ATTN_NORM,0);
}

int imp_check_move(edict_t *self, float dist)
{
	trace_t	trace;
	vec3_t	vec, vf;

	VectorCopy(self->s.origin, vec);
	
	AngleVectors(self->s.angles, vf, NULL, NULL);
	VectorMA(vec, dist, vf, vec);

	gi.trace(self->s.origin, self->mins, self->maxs, vec, self, MASK_SHOT|MASK_WATER,&trace);

	if (trace.fraction < 1)
	{
		if (trace.ent == self->enemy)
			return true;

		return false;
	}

	return true;
}

//replaces ai_walk and ai_run for imp
void imp_ai_glide (edict_t *self, float fd, float rd, float ud)
{
	vec3_t	vec, vf, vr, nvec;
	float	yaw_delta, roll, dot, rdot;
	
	if (!self->enemy)
		return;

	//Find our ideal yaw to the player and correct to it
	VectorSubtract(self->enemy->s.origin, self->s.origin, vec);
	VectorCopy(vec, nvec);
	VectorNormalize(nvec);

	AngleVectors(self->s.angles, vf, vr, NULL);

	dot  = DotProduct(vf, nvec);
	rdot = DotProduct(vr, nvec);
	
	self->ideal_yaw = vectoyaw(vec);

	M_ChangeYaw(self);

	yaw_delta = self->ideal_yaw - self->s.angles[YAW];

	//If enough, roll the creature to simulate gliding
	if (Q_fabs(yaw_delta) > self->yaw_speed)
	{		
		if (dot < 0)
		{
			roll = Q_fabs(yaw_delta / 4);
		}
		else
		{
			roll = yaw_delta / 4;
		}
		
		//Going right?
		if (roll > 0)
		{
			self->s.angles[ROLL] += roll;
			if (self->s.angles[ROLL] > 65)
				self->s.angles[ROLL] = 65;
		}
		else
		{
			self->s.angles[ROLL] += roll;	
			if (self->s.angles[ROLL] < -65)
				self->s.angles[ROLL] = -65;
		}
	}
	else
	{
		self->s.angles[ROLL] *= 0.75;
	}
}

void imp_ai_fly (edict_t *self, float fd, float rd, float ud)
{	
	vec3_t	vec, vf, vr, vu;
	
	if (!self->enemy)
		return;

	//Add "friction" to the movement to allow graceful flowing motion, not jittering
	self->velocity[0] *= 0.8;
	self->velocity[1] *= 0.8;
	self->velocity[2] *= 0.8;

	//Find our ideal yaw to the player and correct to it
	VectorSubtract(self->enemy->s.origin, self->s.origin, vec);
	self->ideal_yaw = vectoyaw(vec);

	M_ChangeYaw(self);

	if (!imp_check_move(self, fd/10))
	{
		SetAnim(self, ANIM_HOVER1);
		return;
	}

	if(self->spawnflags&MSF_FIXED)
		return;

	//Add in the movements relative to the creature's facing
	AngleVectors(self->s.angles, vf, vr, vu);
	
	VectorMA(self->velocity, fd, vf, self->velocity);
	VectorMA(self->velocity, rd, vr, self->velocity);
	VectorMA(self->velocity, ud, vu, self->velocity);

	if (self->groundentity)
		self->velocity[2] += 32;
}

//replaces ai_stand for imp
void imp_ai_hover(edict_t *self, float dist)
{
	vec3_t	vec;

	if (!self->enemy)
	{
		if (!FindTarget(self))
			return;
	}

	//Add "friction" to the movement to allow graceful flowing motion, not jittering
	self->velocity[0] *= 0.8;
	self->velocity[1] *= 0.8;
	self->velocity[2] *= 0.8;
	
	//Make sure we're not tilted after a turn
	self->s.angles[ROLL] *= 0.25;
	
	//Find our ideal yaw to the player and correc to it
	VectorSubtract(self->enemy->s.origin, self->s.origin, vec);
	self->ideal_yaw = vectoyaw(vec);

	M_ChangeYaw(self);

	imp_ai_glide(self,0,0,0);
}

//receiver for MSG_FLYBACK
void imp_flyback(edict_t *self)
{
	SetAnim(self, ANIM_FLYBACK1);
	return;
}


void imp_ai_pirch(edict_t *self)
{
	monsterinfo_t *monsterinfo = &self->monsterinfo;
	vec3_t	vec, vf, vr;
	float	dot, len;

	if (!M_ValidTarget(self, self->enemy))
		return;

	if(!visible(self, self->enemy))
		return;

	VectorSubtract(self->enemy->s.origin, self->s.origin, vec);
	len = VectorNormalize(vec);
	
	if (len < 150)
	{
		SetAnim(self, ANIM_TAKEOFF);
		return;
	}

/*
	if (irand(0,100) < 10 && self->monsterinfo.attack_finished < level.time)
	{
		self->monsterinfo.attack_finished = level.time + 5;

		if (irand(0,1))
			gi.sound (self, CHAN_VOICE, sounds[SND_IDLE1], 1, ATTN_NORM, 0);
		else
			gi.sound (self, CHAN_VOICE, sounds[SND_IDLE2], 1, ATTN_NORM, 0);
	}
*/
	AngleVectors(self->s.angles, vf, vr, NULL);

	dot = DotProduct(vec, vf);

	if (dot < 0)
	{
		SetAnim(self, ANIM_TAKEOFF);
		return;
	}
}

void move_imp_tumble(edict_t *self)
{
	self->movetype = PHYSICSTYPE_STEP;
	self->gravity = 1;

	VectorSet(self->mins, -16, -16, 0);
	VectorSet(self->maxs, 16, 16, 16);

	if (!self->avelocity[PITCH] && !self->avelocity[YAW] && !self->avelocity[ROLL])
	{
		self->avelocity[PITCH] = flrand(128.0F, 256.0F);
		self->avelocity[YAW] = flrand(64.0F, 512.0F);
		self->avelocity[ROLL] = flrand(64.0F, 512.0F);
	}

	if (self->groundentity != NULL || self->monsterinfo.jump_time < level.time)
	{
		gi.CreateEffect(&self->s, FX_DUST_PUFF, CEF_OWNERS_ORIGIN, self->s.origin, NULL);

		VectorCopy(self->s.angles, self->movedir);
		imp_death_noise(self);

		SetAnim(self, ANIM_DIE);
	}
}

void imp_fix_angles(edict_t *self)
{
	float pitch_delta, roll_delta;

	pitch_delta = self->movedir[PITCH];
	roll_delta = self->movedir[ROLL];

	if (pitch_delta > 0)
	{
		self->s.angles[PITCH] -= pitch_delta / 2;
	
		if (self->s.angles[PITCH] < 2)
		{
			self->s.angles[PITCH] = 0;
		}
	}
	else
	{
		self->s.angles[PITCH] += pitch_delta / 2;
	
		if (self->s.angles[PITCH] > 2)
		{
			self->s.angles[PITCH] = 0;
		}
	}

	//Roll
	if (roll_delta > 0)
	{
		self->s.angles[ROLL] -= roll_delta / 2;
	
		if (self->s.angles[ROLL] < 2)
		{
			self->s.angles[ROLL] = 0;
		}
	}
	else
	{
		self->s.angles[ROLL] += roll_delta / 15;
	
		if (self->s.angles[ROLL] > 2)
		{
			self->s.angles[ROLL] = 0;
		}
	}
}

/*===============================================================

	Imp Message Functions

===============================================================*/

void imp_death_pain (edict_t *self, G_Message_t *msg)
{
	if(self->health <= -40) //gib death
	{
		BecomeDebris(self);
		return;
	}
}

//receiver for MSG_DEATH 
void imp_die(edict_t *self, G_Message_t *msg)
{
	if(self->monsterinfo.aiflags&AI_DONT_THINK)
	{
		SetAnim(self, ANIM_DIE);
		return;
	}

	self->movetype = PHYSICSTYPE_STEP;
	self->gravity = 1;
	self->elasticity = 1.1;

	VectorSet(self->mins, -16, -16, 0);
	VectorSet(self->maxs, 16, 16, 16);
	
	if(self->health <= -40) //gib death
	{
		BecomeDebris(self);
		self->think = NULL;
		self->nextthink = 0;	
		gi.linkentity (self);
		return;
	}

	self->msgHandler = DeadMsgHandler;

	SetAnim(self, ANIM_DIE);
	return;
}


//receiver for MSG_PAIN
void imp_pain(edict_t *self, G_Message_t *msg)
{
	int				temp, damage;
	qboolean		force_pain;
	
	ParseMsgParms(msg, "eeiii", &temp, &temp, &force_pain, &damage, &temp);

	if (self->curAnimID == ANIM_PERCH)
	{
		SetAnim(self, ANIM_TAKEOFF);
		return;
	}
	if (force_pain||((irand(0,10) < 2) && (self->pain_debounce_time < level.time)))
	{
/*		if (irand(0,1))
			imp_pain1_noise(self);
		else
			imp_pain2_noise(self);*/

		self->pain_debounce_time = level.time + 2;
		if(self->curAnimID == ANIM_DIVE_GO || self->curAnimID == ANIM_DIVE_LOOP)
			SetAnim(self, ANIM_DIVE_END);
		else
			SetAnim(self, ANIM_PAIN1);
	}
}

//receiver for MSG_STAND, MSG_HOVER 
//FIXME -- is MSG_HOVER redundant?
void imp_hover(edict_t *self, G_Message_t *msg)
{
	if (self->curAnimID == ANIM_PERCH)
		return;

	SetAnim(self, ANIM_HOVER1);
}

void imp_stand(edict_t *self, G_Message_t *msg)
{
	if (self->spawnflags & MSF_PERCHING)
		return;

	SetAnim(self, ANIM_HOVER1);
}

void imp_fly(edict_t *self, G_Message_t *msg)
{
	SetAnim(self, ANIM_FLY1);
}

void imp_perch(edict_t *self, G_Message_t *msg)
{
	SetAnim(self, ANIM_PERCH);
}

void imp_hit(edict_t *self, float stop_swoop)
{
	trace_t	trace;
	edict_t	*victim;
	vec3_t	vf;//, hitPos, mins, maxs;
	float	movedist, damage;

	AngleVectors(self->s.angles, vf, NULL, NULL);
	movedist = VectorLength(self->velocity);

	victim = M_CheckMeleeHit( self, movedist, &trace);

	if (victim)
	{
		if (victim == self && stop_swoop)
		{
			SetAnim(self, ANIM_DIVE_OUT);
		}
		else
		{
			damage = irand(IMP_DMG_MIN, IMP_DMG_MAX);
			gi.sound(self, CHAN_BODY, sounds[SND_HIT], 1, ATTN_NORM, 0);
			T_Damage (victim, self, self, vf, self->enemy->s.origin, trace.plane.normal, damage, damage*2, 0,MOD_DIED);
			SetAnim(self, ANIM_DIVE_END);
		}
	}
}

void imp_pause (edict_t *self)
{
	if (M_ValidTarget(self, self->enemy))
		QPostMessage(self, MSG_RUN, PRI_DIRECTIVE, NULL);
}


//end of anim func for death anim
void imp_dead(edict_t *self)
{
	VectorSet(self->mins, -16, -16, 0);
	VectorSet(self->maxs, 16, 16, 16);

	M_EndDeath(self);
}

qboolean imp_check_directions(edict_t *self, vec3_t goal, vec3_t vf, vec3_t vr, vec3_t vu, float checkdist, vec3_t ret)
{
	trace_t	trace;
	vec3_t	goalpos;

	//Check right and left first
	VectorCopy(self->s.origin, goalpos);

	//Don't always check one direction first (looks mechanical)
	if (irand(0,1))
		VectorScale(vr, -1, vr);

	VectorMA(goalpos, checkdist, vr, goalpos);

	gi.trace(goalpos, self->mins, self->maxs, goal, self, MASK_SHOT|MASK_WATER,&trace);

	//We've found somewhere to go
	if (trace.ent == self->enemy)
	{
		VectorCopy(vr, ret);
		return true;
	}
	else  //Check the other directions
	{
		VectorScale(vr, -1, vr);
		VectorMA(goalpos, checkdist, vr, goalpos);

		gi.trace(goalpos, self->mins, self->maxs, goal, self, MASK_SHOT|MASK_WATER,&trace);

		if (trace.ent == self->enemy)
		{
			VectorCopy(vr, ret);
			return true;
		}
	}
	
	//Check up and down
	VectorCopy(self->s.origin, goalpos);

	//Don't always check one direction first (looks mechanical)
	if (irand(0,1))
		VectorScale(vu, -1, vu);

	VectorMA(goalpos, checkdist, vu, goalpos);

	gi.trace(goalpos, self->mins, self->maxs, goal, self, MASK_SHOT|MASK_WATER,&trace);

	//We've found somewhere to go
	if (trace.ent == self->enemy)
	{
		VectorCopy(vu, ret);
		return true;
	}
	else  //Check the other directions
	{
		VectorScale(vu, -1, vu);
		VectorMA(goalpos, checkdist, vu, goalpos);

		gi.trace(goalpos, self->mins, self->maxs, goal, self, MASK_SHOT|MASK_WATER,&trace);

		if (trace.ent == self->enemy)
		{
			VectorCopy(vu, ret);
			return true;
		}
	}
		
	//Check forward and back
	VectorCopy(self->s.origin, goalpos);

	//Don't always check one direction first (looks mechanical)
	if (irand(0,1))
		VectorScale(vf, -1, vf);

	VectorMA(goalpos, checkdist, vf, goalpos);

	gi.trace(goalpos, self->mins, self->maxs, goal, self, MASK_SHOT|MASK_WATER,&trace);

	//We've found somewhere to go
	if (trace.ent == self->enemy)
	{
		VectorCopy(vf, ret);
		return true;
	}
	else  //Check the other directions
	{
		VectorScale(vf, -1, vf);
		VectorMA(goalpos, checkdist, vf, goalpos);

		gi.trace(goalpos, self->mins, self->maxs, goal, self, MASK_SHOT|MASK_WATER,&trace);

		if (trace.ent == self->enemy)
		{
			VectorCopy(vf, ret);
			return true;
		}
	}

	return false;
}

qboolean imp_check_swoop(edict_t *self, vec3_t goal)
{
	trace_t	trace;
	vec3_t	checkpos;
	float	zd;

	//Find the difference in the target's height and the creature's height
	zd = Q_fabs(self->enemy->s.origin[2] - self->s.origin[2]);
	
	if (zd < IMP_MIN_SSWOOP_DIST)
		return false;

	zd -= zd/4;

	VectorCopy(self->s.origin, checkpos);
	checkpos[2] -= zd;

	//Trace down about that far and about one forth the distance to the target
	gi.trace(self->s.origin, self->mins, self->maxs, checkpos, self, MASK_SHOT|MASK_WATER,&trace);

	if (trace.fraction < 1)
	{
		//gi.dprintf("imp_check_swoop: failed down check\n");
		return false;
	}

	//Trace straight to the target

	gi.trace(checkpos, self->mins, self->maxs, goal, self, MASK_SHOT|MASK_WATER,&trace);

	if (trace.ent != self->enemy)
	{
		//gi.dprintf("imp_check_swoop: failed out check\n");
		return false;
	}

	//There's a clear path
	return true;
}

void move_imp_dive(edict_t *self)
{
	vec3_t	vec, vf;
	float	dist, zd, hd, forward;

	//Find out the Z and Horizontal deltas to target
	zd = Q_fabs(self->s.origin[2] - self->enemy->s.origin[2]);
	
	AngleVectors(self->s.angles, vf, NULL, NULL);

	VectorCopy(self->s.origin, vec);
	vec[2] = self->enemy->s.origin[2];

	VectorSubtract(self->enemy->s.origin, vec, vec);
	hd = VectorLength(vec);

	if ((self->groundentity != NULL) || (!imp_check_move(self, 64)))
	{
		if (self->groundentity == self->enemy)
			imp_hit(self, true);
		return;
	}

	dist = Q_fabs(self->s.origin[2] - self->enemy->s.origin[2]);

	forward = (256 - (dist*0.85));

	if (forward > 256)
		forward = 256;
	else if (forward < 0)
		forward = 0;

//	if (dist > IMP_MIN_SWOOP_DIST)
//	{
		VectorMA(vf, forward, vf, self->velocity);
		self->velocity[2] = -dist*2.25;
		if (self->velocity[2] < -300)
			self->velocity[2] = -300;
//	}
/*	else
	{
		SetAnim(self, ANIM_DIVE_TRANS);
		return;
	}*/

	imp_ai_glide(self, 0, 0, 0);
}

void move_imp_dive_end(edict_t *self)
{
	vec3_t	vec, vf, vr, vu, nvec;
	float	hd, fd, dot;
	
	VectorCopy(self->s.origin, vec);
	vec[2] = self->enemy->s.origin[2];

	VectorSubtract(self->enemy->s.origin, vec, vec);
	hd = VectorLength(vec);
	self->ideal_yaw = vectoyaw(vec);

	M_ChangeYaw(self);

	AngleVectors(self->s.angles, vf, vr, vu);
	
	self->velocity[2] *= 0.75;

	self->monsterinfo.jump_time *= IMP_SWOOP_INCR;

	fd = self->monsterinfo.jump_time;

	if (fd > IMP_SWOOP_SPEED_MAX)
		fd = IMP_SWOOP_SPEED_MAX;

	if ((self->groundentity != NULL) || (!imp_check_move(self, 128)))
	{
		if (self->groundentity == self->enemy)
			SetAnim(self, ANIM_DIVE_END);

		SetAnim(self, ANIM_FLYBACK1);
		return;
	}

	VectorSubtract(self->enemy->s.origin, self->s.origin, vec);
	VectorCopy(vec, nvec);
	VectorNormalize(nvec);

	AngleVectors(self->s.angles, vf, vr, NULL);

	dot  = DotProduct(vf, nvec);

	if (dot < -0.5)
	{
		SetAnim(self, ANIM_FLYBACK1);
		return;
	}

	VectorMA(self->velocity, fd, vf, self->velocity);
	
	//Are we about to hit the target?
/*	VectorSubtract(self->enemy->s.origin, self->s.origin, vec);
	dist = VectorLength(vec);

	if (dist < IMP_COLLISION_DIST)
	{
		SetAnim(self, ANIM_DIVE_END);
		return;
	}	*/

	imp_ai_glide(self, 0, 0, 0);
}

void imp_dive_loop(edict_t *self)
{
	SetAnim(self, ANIM_DIVE_LOOP);
}

void imp_check_dodge(edict_t *self)
{
	qboolean	dodge = false;
	trace_t		trace;
	edict_t		*ent = NULL;
	vec3_t		vec, vr, projvec, ddir, goalpos, vu;
	float		dodgedot;
	qboolean	vert = false;

	if (!self->enemy)
		return;

	if(self->spawnflags&MSF_FIXED)
		return;

	VectorSubtract(self->enemy->s.origin, self->s.origin, vec);
	VectorNormalize(vec);

	while ((ent = findradius(ent, self->s.origin, IMP_PROJECTILE_RADIUS)) != NULL)
	{
		//We're only interested in his projectiles
		if (ent->owner != self->enemy)
			continue;
		
		VectorCopy(ent->velocity, projvec);
		VectorNormalize(projvec);

		dodgedot = DotProduct(projvec, vec);

		//gi.dprintf("Found projectile with dot %f\n", dodgedot);

		if (dodgedot < -0.85 && irand(0,1))
		{
			//gi.dprintf("Dodge it!\n");

			dodge = true;
			AngleVectors(self->s.angles, NULL, vr, vu);

			VectorCopy(self->s.origin, goalpos);

			if (irand(0,1))
			{
				if (irand(0,1))
					VectorScale(vr, -1, ddir);
				else
					VectorScale(vr, 1, ddir);
			}
			else
			{
				vert = true;
				if (irand(0,1))
					VectorScale(vu, -1, ddir);
				else
					VectorScale(vu, 1, ddir);
			}

			VectorMA(goalpos, 100, ddir, goalpos);

			gi.trace(self->s.origin, self->mins, self->maxs, goalpos, self, MASK_SHOT|MASK_WATER,&trace);

			if (trace.fraction < 1)//bad dir, try other
				VectorScale(ddir, -1, ddir);

			if(vert)
			{//ok, better check this new opposite dir
				gi.trace(self->s.origin, self->mins, self->maxs, goalpos, self, MASK_SHOT|MASK_WATER,&trace);

				if (trace.fraction < 1)
				{//uh-oh, let's go for a side dir
					if (irand(0,1))
						VectorScale(vr, 1, ddir);
					else
						VectorScale(vr, -1, ddir);
				}

				gi.trace(self->s.origin, self->mins, self->maxs, goalpos, self, MASK_SHOT|MASK_WATER,&trace);

				if (trace.fraction < 1)//what the hell, just go the other way
					VectorScale(ddir, -1, ddir);

			}
		}
	}

	if (dodge)
	{
		//If he is, dodge!
		if (self->monsterinfo.misc_debounce_time < level.time)
		{
			if(self->curAnimID!=ANIM_FIREBALL)
			{
				if(ddir[2] > 0.1)
					SetAnim(self, ANIM_DUP);
				else if(ddir[2] < -0.1)
					SetAnim(self, ANIM_DDOWN);
			}
			VectorMA(self->velocity, irand(300, 500), ddir, self->velocity);
			self->monsterinfo.misc_debounce_time = level.time + irand(2,4);
		}
	}	
	
	imp_ai_glide(self, 0, 0, 0);
}

void move_imp_hover(edict_t *self)
{
	qboolean	canmove = false, dodge = false;
	trace_t		trace;
	edict_t		*ent = NULL;
	vec3_t		goal, dodgedir, mins, maxs, vf, vr, vu, vec, projvec, goalpos;
	float		dist, zd, dodgedot, enemy_dist;
	

	if (!self->enemy)
	{
		if (!FindTarget(self))
		{
			return;
		}
	}

	//First check to see that the player is at least 128 units away in (discounting z height)
	VectorCopy(self->enemy->s.origin, goal);
	goal[2] = self->s.origin[2];

	VectorSubtract(goal, self->s.origin, goal);
	dist = VectorLength(goal);
	
	//Face target
	self->ideal_yaw = vectoyaw(goal);
	M_ChangeYaw(self);

	//If he is...
	if (dist > IMP_MIN_HOVER_DIST && dist < IMP_MAX_HOVER_DIST)
	{

		//Make sure we've got line of sight
		VectorSet(mins, -1, -1, -1);
		VectorSet(maxs, 1, 1, 1);

		gi.trace(self->s.origin, mins, maxs, self->enemy->s.origin, self, MASK_SHOT|MASK_WATER,&trace);

		//If not, try looking from a bit to the side in all six directions
		if (trace.ent != self->enemy)
		{
			
			//Setup the directions
			AngleVectors(self->s.angles, vf, vr, vu);

			canmove = imp_check_directions(self, self->enemy->s.origin, vf, vr, vu, IMP_CHECK_DIST, goal);
			
			//If we can see him from one of these, go there
			if (canmove)
			{
				VectorMA(self->velocity, flrand(300.0F, 400.0F), goal, self->velocity);
				return;
			}

			
			//Otherwise just flap around and wait, perhaps lower yourself a bit if high up
			self->velocity[0] = flrand(-IMP_DRIFT_AMOUNT_X, IMP_DRIFT_AMOUNT_X);
			self->velocity[1] = flrand(-IMP_DRIFT_AMOUNT_Y, IMP_DRIFT_AMOUNT_Y);
			self->velocity[2] = flrand(-IMP_DRIFT_AMOUNT_Z, IMP_DRIFT_AMOUNT_Z);

			return;
		}
		else
		{
			//Check to make sure the player isn't shooting anything

			//This won't change over the calculations
			VectorSubtract(self->enemy->s.origin, self->s.origin, vec);
			enemy_dist = VectorNormalize(vec);

			while ((ent = findradius(ent, self->s.origin, IMP_PROJECTILE_RADIUS)) != NULL)
			{
				//We're only interested in his projectiles
				if (ent->owner != self->enemy)
					continue;
				
				VectorCopy(ent->velocity, projvec);
				VectorNormalize(projvec);

				dodgedot = DotProduct(projvec, vec);

				//gi.dprintf("Found projectile with dot %f\n", dodgedot);

				if (dodgedot < -0.6)
				{
					//gi.dprintf("Dodge it!\n");

					dodge = true;
					AngleVectors(self->s.angles, NULL, vr, NULL);

					if (irand(0,1))
						VectorScale(vr, -1, vr);

					VectorMA(self->s.origin, 100, vr, goalpos);

					gi.trace(self->s.origin, self->mins, self->maxs, goalpos, self, MASK_SHOT|MASK_WATER,&trace);

					if (trace.fraction < 1)
						VectorScale(vr, -1, vr);

					VectorCopy(vr, dodgedir);
				}
			}

			if (dodge)
			{
				//If he is, dodge!
				VectorMA(self->velocity, irand(300, 500), dodgedir, self->velocity);
				return;
			}

			//see if he's too close
			if(enemy_dist < Q_fabs(self->melee_range))
				SetAnim(self, ANIM_FLYBACK1);
			else if(enemy_dist < self->missile_range)
			{//see if we can and want to attack him
				if(enemy_dist > self->min_missile_range)
				{
					if(flrand(0, 100) > self->bypass_missile_chance)
					{
						SetAnim(self, ANIM_FIREBALL);
						return;
					}
				}
			}

			//If nothing is happening, check to swoop
			canmove = imp_check_swoop(self, self->enemy->s.origin);

			//If you can--nail um
			if (canmove)
			{
				self->monsterinfo.jump_time = 2;
				SetAnim(self, ANIM_DIVE_GO);

				return;
			}

			//If not, check to see if there's somewhere that you can get to that will allow it
			//FIXME: Too many checks.. just try something simple

			//If all else fails, then just pick a random direction to nudge yourself to
			else
			{
				
				//Find the difference in the target's height and the creature's height
				zd = Q_fabs(self->enemy->s.origin[2] - self->s.origin[2]);
		
				//We can't swoop because we're too low, so fly upwards if possible
				if (zd < IMP_MIN_SSWOOP_DIST)
				{
					if (!imp_check_move(self, -64))
					{
						SetAnim(self, ANIM_FLY1);
						return;
					}
					else
					{
						//gi.dprintf("Moveback ok\n");
						SetAnim(self, ANIM_FLYBACK1);
						return;
					}
				}
				else
				{
					//Otherwise just flap around and wait, perhaps lower yourself a bit if high up					
					self->velocity[0] = flrand(-IMP_DRIFT_AMOUNT_X, IMP_DRIFT_AMOUNT_X);
					self->velocity[1] = flrand(-IMP_DRIFT_AMOUNT_Y, IMP_DRIFT_AMOUNT_Y);
					self->velocity[2] = flrand(-IMP_DRIFT_AMOUNT_Z, IMP_DRIFT_AMOUNT_Z);

					AngleVectors(self->s.angles, vec, NULL, NULL);
					VectorMA(self->velocity, irand(200,300), vec, self->velocity);
				}

				return;
			}

		}

		//If he's too far away trace a line (expanded) to see if you can move at him
	}
	else if (dist < IMP_MIN_HOVER_DIST)
	{
		if (!imp_check_move(self, -64))
		{
			SetAnim(self, ANIM_FLY1);
		}
		else
		{
			SetAnim(self, ANIM_FLYBACK1);
		}
	}
	else
	{
		if (!imp_check_move(self, 64))
		{
			SetAnim(self, ANIM_FLYBACK1);
		}
		else
		{
			SetAnim(self, ANIM_FLY1);
		}
	}

	return;
} 

//New physics call that modifies the imp's velocity and angles based on aerodynamics
void imp_flight_model(edict_t *self)
{
}

void move_imp_fly(edict_t *self)
{	
	edict_t *dummy;
	dummy = self;
	
	if(!irand(0,3))
		imp_check_dodge(self);

	return;
}

void move_imp_die(edict_t *self)
{
	//fall to the floor
	return;
}

void imp_hover_anim(edict_t *self)
{
	SetAnim(self, ANIM_HOVER1);
}

//===============================

//IMP FIREBALL

//===============================


void FireFizzle (edict_t *self)
{
	vec3_t dir;
	gi.sound (self, CHAN_BODY, sounds[SND_FIZZLE], 1, ATTN_NORM, 0);
	VectorSet(dir, flrand(0, 1),flrand(0, 1), flrand(0.5, 1));
	VectorNormalize(dir);
	gi.CreateEffect(&self->s,
					FX_ENVSMOKE,
					CEF_BROADCAST,self->s.origin,
					"bdbbb",irand(1,3),dir,irand(1,2),irand(3, 4),irand(1,2));
	G_SetToFree(self);
}

void fireball_blocked( edict_t *self, trace_t *trace );
void create_imp_proj(edict_t *self,edict_t *proj)
{
	proj->svflags |= SVF_ALWAYS_SEND;
	proj->movetype = PHYSICSTYPE_FLY;
	proj->gravity = 0;
	proj->solid = SOLID_BBOX;
	proj->classname = "imp fireball";
	proj->s.scale = 1.0;
	proj->clipmask = (MASK_SHOT|CONTENTS_WATER);
	proj->s.effects=EF_MARCUS_FLAG1;
	proj->enemy = self->enemy;
	proj->reflect_debounce_time = MAX_REFLECT;

	proj->isBlocked = fireball_blocked;
	proj->isBlocking = fireball_blocked;
	proj->bounced = fireball_blocked;

	VectorSet(proj->mins, -1.0, -1.0, -1.0);
	VectorSet(proj->maxs,  1.0,  1.0,  1.0);
	VectorCopy(self->s.origin, proj->s.origin);
}


edict_t *ImpFireballReflect(edict_t *self, edict_t *other, vec3_t vel)
{
	edict_t	*fireball;

	fireball = G_Spawn();

	create_imp_proj(self, fireball);

	fireball->s.modelindex = self->s.modelindex;
	VectorCopy(self->s.origin, fireball->s.origin);
	fireball->owner = other;
	fireball->enemy = self->owner;
	fireball->nextthink=self->nextthink;
	VectorScale(self->avelocity, -0.5, fireball->avelocity);
	VectorCopy(vel, fireball->velocity);
	VectorNormalize2(vel, fireball->movedir);
	AnglesFromDir(fireball->movedir, fireball->s.angles);
	fireball->classID = self->classID;
	fireball->reflect_debounce_time = self->reflect_debounce_time -1;
	fireball->reflected_time=self->reflected_time;
	fireball->ideal_yaw = self->ideal_yaw;

	gi.CreateEffect(&fireball->s,
				FX_M_EFFECTS,
				CEF_OWNERS_ORIGIN,
				NULL,
				"bv",
				FX_IMP_FIRE,
				fireball->velocity);

	G_LinkMissile(fireball); 

	G_SetToFree(self);

	gi.CreateEffect(&fireball->s, FX_LIGHTNING_HIT, CEF_OWNERS_ORIGIN, NULL, "t", vel);

	return(fireball);
}


void fireball_blocked( edict_t *self, trace_t *trace )
{	
	if(trace->surface)
	{
		if(trace->surface->flags & SURF_SKY)
		{
			SkyFly(self);
			return;
		}
	}

	if(trace->contents&CONTENTS_WATER || trace->contents&CONTENTS_SLIME)
	{
		FireFizzle(self);
		return;
	}

	if(trace->ent)
	{
		if (EntReflecting(trace->ent, true, true) && self->reflect_debounce_time)
		{
			Create_rand_relect_vect(self->velocity, self->velocity);
			Vec3ScaleAssign(self->ideal_yaw, self->velocity);
			ImpFireballReflect(self, trace->ent, self->velocity);

			return;
		}
	}

	if (trace->ent->takedamage )
	{
		vec3_t	hitDir;
		float	damage = flrand(2,5);
	
		if(self->dmg)
			damage += self->dmg;
		VectorCopy( self->velocity, hitDir );
		VectorNormalize( hitDir );

		T_Damage(trace->ent, self, self->owner, hitDir, self->s.origin, trace->plane.normal, damage, 0, DAMAGE_SPELL | DAMAGE_NO_KNOCKBACK,MOD_DIED);
	}

	gi.sound(self, CHAN_BODY, sounds[SND_FBHIT], 1, ATTN_NORM, 0);

	gi.CreateEffect(&self->s,
				FX_M_EFFECTS,
				CEF_OWNERS_ORIGIN,
				self->s.origin,
				"bv",
				FX_IMP_FBEXPL,
				vec3_origin);

	G_SetToFree(self);
}

void imp_fireball (edict_t *self)
{
	edict_t	*proj;
	vec3_t	vf, vr, check_lead;

	// Spawn the projectile

	proj = G_Spawn();

	create_imp_proj(self,proj);
	proj->reflect_debounce_time = MAX_REFLECT;

	proj->owner = self;
	
	proj->dmg = irand(10, 20);

	AngleVectors(self->s.angles, vf, vr, NULL);

	if(self->classID == CID_IMP)
	{
		VectorMA(self->s.origin, -4*self->monsterinfo.scale, vf, proj->s.origin);
		VectorMA(proj->s.origin, 16*self->monsterinfo.scale, vr, proj->s.origin);
		proj->s.origin[2] += 32*self->monsterinfo.scale;
		gi.sound(proj,CHAN_BODY,sounds[SND_ATTACK],1,ATTN_NORM,0);
	}
	else
	{
		VectorCopy(self->s.origin, proj->s.origin);
		VectorMA(proj->s.origin, 16, vf, proj->s.origin);
		proj->s.origin[2] += 12;
		gi.sound(proj, CHAN_BODY, gi.soundindex("monsters/imp/fireball.wav"), 1, ATTN_NORM, 0);
	}

	extrapolateFiredir (self, proj->s.origin, 666, self->enemy, 0.3, check_lead);
	if(Vec3IsZero(check_lead))
	{
		VectorScale(vf, 666, proj->velocity);
	}
	else
	{
		VectorScale(check_lead, 666, proj->velocity);
	}

	VectorCopy(proj->velocity, proj->movedir);
	VectorNormalize(proj->movedir);
	vectoangles(proj->movedir, proj->s.angles);

	gi.CreateEffect(&proj->s,
				FX_M_EFFECTS,//just so I don't have to make a new FX_ id
				CEF_OWNERS_ORIGIN,
				NULL,
				"bv",
				FX_IMP_FIRE,
				proj->velocity);

	gi.linkentity(proj); 
}

/*===============================================================

	Imp Spawn Functions

===============================================================*/

void ImpStaticsInit()
{
	classStatics[CID_IMP].msgReceivers[MSG_DEATH] = imp_die;
	classStatics[CID_IMP].msgReceivers[MSG_FLY] = imp_hover;
	classStatics[CID_IMP].msgReceivers[MSG_STAND] = imp_stand;
	classStatics[CID_IMP].msgReceivers[MSG_RUN] = imp_hover;
	classStatics[CID_IMP].msgReceivers[MSG_PAIN] = imp_pain;
	classStatics[CID_IMP].msgReceivers[MSG_WATCH] = imp_perch;
	classStatics[CID_IMP].msgReceivers[MSG_DEATH_PAIN] = imp_death_pain;

	resInfo.numAnims = NUM_ANIMS;
	resInfo.animations = animations;
	resInfo.modelIndex = gi.modelindex("models/monsters/imp/tris.fm");
	resInfo.numSounds = NUM_SOUNDS;
	resInfo.sounds = sounds;

	sounds[SND_GIB]=gi.soundindex("misc/fleshbreak.wav");	
	sounds[SND_FLAP]=gi.soundindex("monsters/imp/fly.wav");	
	sounds[SND_SCREAM]=gi.soundindex("monsters/imp/up.wav");	
	sounds[SND_DIVE]=gi.soundindex("monsters/imp/swoop.wav");	
	sounds[SND_DEATH]=gi.soundindex("monsters/imp/die.wav");	
	sounds[SND_HIT]=gi.soundindex("monsters/imp/swoophit.wav");	
	sounds[SND_ATTACK]=gi.soundindex("monsters/imp/fireball.wav");	
	sounds[SND_FIZZLE]=gi.soundindex("monsters/imp/fout.wav");
	sounds[SND_FBHIT]=gi.soundindex("monsters/imp/fbfire.wav");

	classStatics[CID_IMP].resInfo = &resInfo;
}

/*QUAKED monster_imp(1 .5 0) (-16 -16 0) (16 16 32) AMBUSH ASLEEP Perching 8 16 32 64 FIXED

Our old pal, the fire imp!

AMBUSH - Will not be woken up by other monsters or shots from player

ASLEEP - will not appear until triggered

PERCHING - Will watch player until get too close or get behind the imp

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
mintel					= 14
melee_range				= -64
missile_range			= 1024
min_missile_range		= 32
bypass_missile_chance	= 20
jump_chance				= 0 (flying, no jump)
wakeup_distance			= 1024

NOTE: A value of zero will result in defaults, if you actually want zero as the value, use -1
*/
void SP_monster_imp(edict_t *self)
{
	if (!flymonster_start(self))
		return;				// Failed initialization

	self->msgHandler = DefaultMsgHandler;
	self->classID = CID_IMP;

	if (!self->health)
		self->health = IMP_HEALTH;

	self->max_health = self->health = MonsterHealth(self->health);

	self->mass = IMP_MASS;
	self->yaw_speed = 14;

	self->movetype = PHYSICSTYPE_FLY;
	self->gravity = 0;
	self->flags |= FL_FLY;
	self->solid = SOLID_BBOX;
	self->clipmask = MASK_MONSTERSOLID;

	VectorCopy(STDMinsForClass[self->classID], self->mins);
	VectorCopy(STDMaxsForClass[self->classID], self->maxs);	

	self->svflags |= SVF_TAKE_NO_IMPACT_DMG;

	self->materialtype = MAT_FLESH;

	self->s.modelindex = classStatics[CID_IMP].resInfo->modelIndex;
	self->s.skinnum = 0;

	self->isBlocked = imp_blocked;

	if (!self->s.scale)
		self->monsterinfo.scale = self->s.scale = flrand(0.7, 1.2);

	self->monsterinfo.otherenemyname = "monster_rat";	

	if (self->spawnflags & MSF_PERCHING)
	{
		SetAnim(self, ANIM_PERCH);
	}
	else
	{
		QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
	}

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
}
