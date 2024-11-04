/*-------------------------------------------------------------------
m_harpy.c

Heretic II
Copyright 1998 Raven Software

  AI:

  HOVER1	:hovering in midair
  FLY1		:flying forwards
  FLYBACK	:flying backwards

-------------------------------------------------------------------*/

#include "g_local.h"
#include "m_harpy.h"
#include "m_harpy_anim.h"
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
#include "p_anims.h"
#include "p_actions.h"

#define HARPY_CHECK_DIST		128
#define	HARPY_COLLISION_DIST	148
#define HARPY_MIN_SSWOOP_DIST	128
#define HARPY_MIN_HOVER_DIST	128
#define HARPY_MAX_HOVER_DIST	512
#define HARPY_MIN_SWOOP_DIST	108

#define HARPY_DRIFT_AMOUNT_X	128
#define HARPY_DRIFT_AMOUNT_Y	128
#define HARPY_DRIFT_AMOUNT_Z	64

#define HARPY_SWOOP_INCR		2
#define	HARPY_SWOOP_SPEED_MAX	512

#define HARPY_PROJECTILE_RADIUS	1024

void harpy_blocked (edict_t *self, struct trace_s *trace);

/*-----------------------------------------------------------------
	harpy base info
-----------------------------------------------------------------*/

int BPN_for_hitloc [hl_harpy_max] = {
	0,
	BPN_BACKSPIKES,//hl_backspikes	
	BPN_HEAD|BPN_HORNS|BPN_HORN|BPN_NECKSPIKES,//hl_head
	BPN_STINGER,//hl_stinger			
	BPN_LWING,//hl_lwing			
	BPN_LHAND,//hl_lefthand		
	BPN_RWING,//hl_rwing			
	BPN_RHAND,//hl_righthand	
	BPN_LUARM|BPN_LLARM|BPN_LHAND,//hl_leftupperleg	
	BPN_LLARM|BPN_LHAND,//hl_leftlowerleg	
	BPN_RUARM|BPN_RLARM|BPN_RHAND,//hl_rightupperleg
	BPN_RLARM|BPN_RHAND//hl_rightlowerleg
};

int MESH_for_hitloc [hl_harpy_max] =
{
	0,
	MESH_BACKSPIKES,//hl_backspikes	
	MESH_HEAD,//hl_head
	MESH_STINGER,//hl_stinger			
	MESH_LWING,//hl_lwing			
	MESH_LHAND,//hl_lefthand		
	MESH_RWING,//hl_rwing			
	MESH_RHAND,//hl_righthand	
	MESH_LUARM,//hl_leftupperleg	
	MESH_LLARM,//hl_leftlowerleg	
	MESH_RUARM,//hl_rightupperleg
	MESH_RLARM//hl_rightlowerleg
};

static animmove_t *animations[NUM_ANIMS] =
{
	&harpy_move_die1, 
	&harpy_move_fly1,
	&harpy_move_flyback1,
	&harpy_move_hover1,
	&harpy_move_hoverscream,	
	&harpy_move_dive_go,
	&harpy_move_dive_loop,
	&harpy_move_dive_end,
	&harpy_move_pain1,
	&harpy_move_glide,
	&harpy_move_dive_trans,
	&harpy_move_dive_hit_loop,
	&harpy_move_tumble,	
	&harpy_move_pirch1_idle,
	&harpy_move_pirch2_idle,
	&harpy_move_pirch3_idle,
	&harpy_move_pirch4_idle,
	&harpy_move_pirch5_idle,
	&harpy_move_pirch6_idle,
	&harpy_move_pirch7_idle,
	&harpy_move_pirch8_idle,
	&harpy_move_pirch9_idle,
	&harpy_move_takeoff,
	&harpy_move_circle,
	&harpy_move_circle_flap
};

static int sounds[NUM_SOUNDS];
static ClassResourceInfo_t resInfo;


/*===============================================================

	Harpy Helper Functions

===============================================================*/

void harpy_throw_wings(edict_t *self)
{
	int				throw_nodes = 0;
	vec3_t			gore_spot;

	if (!(self->s.fmnodeinfo[MESH_RWING].flags & FMNI_NO_DRAW))
	{
		throw_nodes |= BPN_for_hitloc[hl_rwing];
	}

	if (!(self->s.fmnodeinfo[MESH_LWING].flags & FMNI_NO_DRAW))
	{
		throw_nodes |= BPN_for_hitloc[hl_lwing];
	}

	VectorCopy(vec3_origin, gore_spot);
	gore_spot[2]+=10;

	ThrowBodyPart(self, &gore_spot, throw_nodes, 50, FRAME_partfly1);
}

int head_die(edict_t *self, edict_t *inflictor, edict_t *attacker,int damage,vec3_t point)
{
	BecomeDebris(self);
	return true;
}

void harpy_head_think (edict_t *self)
{
	vec3_t	down;

	if(!self->owner || self->owner->health <= 0)
	{
		self->movetype = PHYSICSTYPE_STEP;
		self->elasticity = 0.8;
		self->gravity = 1.0;
		self->solid = SOLID_BBOX;
		self->takedamage = DAMAGE_YES;
		self->clipmask = MASK_MONSTERSOLID;
		self->nextthink = -1;
		self->svflags |= SVF_DEADMONSTER;
		self->health = 25;
		self->die = head_die;
		AngleVectors(self->s.angles, down, NULL, NULL);
		VectorScale(down, 100, self->velocity);
		VectorSet(self->mins, -4, -4, -4);
		VectorSet(self->maxs, 4, 4, 4);
		gi.linkentity(self);
		return;
	}

	VectorCopy(self->owner->s.angles, self->s.angles);
	VectorCopy(self->owner->s.origin, self->s.origin);

	AngleVectors(self->s.angles, NULL, NULL, down);
	Vec3ScaleAssign(-1, down);

	VectorMA(self->s.origin, self->count, down, self->s.origin);

	self->nextthink = level.time + 0.1;
}

void harpy_take_head(edict_t *self, edict_t *victim, int BodyPart, int frame, int flags)
{
	edict_t *head;
	vec3_t	forward, down;

	head = G_Spawn();

	head->s.effects |= EF_CAMERA_NO_CLIP;
	head->svflags |= SVF_ALWAYS_SEND;
	head->solid = SOLID_NOT;
	head->movetype = PHYSICSTYPE_NOCLIP;
	head->gravity = 0;
	head->clipmask = 0;
	head->materialtype = victim->materialtype;

	head->owner = self;
	head->activator = victim;

	VectorCopy(self->s.angles, head->s.angles);
	VectorCopy(self->s.origin, head->s.origin);

	AngleVectors(head->s.angles, forward, NULL, down);
	Vec3ScaleAssign(-1, down);

	head->count = 8;
	VectorMA(head->s.origin, head->count, down, head->s.origin);
	
	head->s.origin[2] += 100;
	
	gi.CreateEffect(&head->s,//owner
					FX_BODYPART,//type
					flags,//can't mess with this, sends only 1st byte and effects message
					head->s.origin,//spot,
					"ssbbb",//int int float byte
					(short)(frame),//only 1 frame, sorry no anim
					(short)(BodyPart),//bitwise - node(s) to leave on
					0,//speed
					victim->s.modelindex,//my modelindex
					victim->s.number);//my number

	head->think = harpy_head_think;
	head->nextthink = level.time + 0.1;

	gi.linkentity(head);

	give_head_to_harpy = NULL;
	take_head_from = NULL;

	VectorScale(forward, 200, self->velocity);
	self->velocity[2] = 20;
	//fix angles?
	self->enemy = NULL;

	QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
	//go into a circle?
}

void player_decap (edict_t *self, edict_t *other);
void harpy_blocked (edict_t *self, struct trace_s *trace)
{
	vec3_t	vf, dir;
	float dot;
	int		damage;

	if(!self->enemy && self->spawnflags & MSF_SPECIAL1)
	{
		SetAnim(self, ANIM_CIRCLING_FLAP);
		return;
	}

	if (self->health <= 0)
		return;

	if (!trace->ent)
		return;

	AngleVectors(self->s.angles, vf, NULL, NULL);
	VectorCopy(self->velocity, dir);
	VectorNormalize(dir);
	dot = DotProduct(dir, vf);
	if (trace->ent->takedamage && (self->curAnimID == ANIM_DIVE_GO || self->curAnimID == ANIM_DIVE_LOOP || self->curAnimID == ANIM_DIVE_END || self->curAnimID == ANIM_HIT_LOOP))
	{
		if(trace->ent->client || classStatics[trace->ent->classID].msgReceivers[MSG_DISMEMBER])
		{
			if(trace->ent->health < HARPY_DMG_MAX && trace->ent->s.origin[2] < self->s.origin[2])
			{//also make this skill dependant
				give_head_to_harpy = self;
				take_head_from = trace->ent;
				if(trace->ent->client)
				{
					trace->ent->health = 1;
					player_decap(trace->ent, self);
				}
				else
					QPostMessage(trace->ent, MSG_DISMEMBER, PRI_DIRECTIVE, "ii", 9999999, hl_Head|hl_MeleeHit);
				return;
			}
		}

		damage = irand(HARPY_DMG_MIN, HARPY_DMG_MAX);
		T_Damage (trace->ent, self, self, dir, trace->ent->s.origin, trace->plane.normal, damage, damage*2, 0,MOD_DIED);
		if(trace->ent->health>0)
		{
			if(trace->ent->client)
			{
				if(!irand(0, 5))
				{
					if(trace->ent->client->playerinfo.lowerseq != ASEQ_KNOCKDOWN)
						P_KnockDownPlayer(&trace->ent->client->playerinfo);
				}
			}
		}
		SetAnim(self, ANIM_FLYBACK1);
		return;
	}
	else if(self->damage_debounce_time < level.time || dot>0)
	{//only back up from a block once every 2 seconds
		self->damage_debounce_time = level.time + 2;
		SetAnim(self, ANIM_FLYBACK1);
		return;
	}
	SetAnim(self, ANIM_FLY1);
}

//Various sound functions
void harpy_flap_noise(edict_t *self)
{
	gi.sound(self,CHAN_BODY,sounds[SND_FLAP],1,ATTN_NORM,0);
}

void harpy_flap_fast_noise(edict_t *self)
{
	gi.sound(self,CHAN_BODY,sounds[SND_FLAP_FAST],1,ATTN_NORM,0);
}

void harpy_death_noise(edict_t *self)
{
	gi.sound(self,CHAN_BODY,sounds[SND_DEATH],1,ATTN_NORM,0);
}

void harpy_pain1_noise(edict_t *self)
{
	gi.sound(self,CHAN_BODY,sounds[SND_PAIN1],1,ATTN_NORM,0);
}

void harpy_pain2_noise(edict_t *self)
{
	gi.sound(self,CHAN_BODY,sounds[SND_PAIN2],1,ATTN_NORM,0);
}

void harpy_attack_noise(edict_t *self)
{
	gi.sound(self,CHAN_BODY,sounds[SND_ATTACK],1,ATTN_NORM,0);
}

void harpy_dive_noise(edict_t *self)
{
	gi.sound(self, CHAN_BODY, sounds[SND_DIVE], 1, ATTN_NORM, 0);
}

int harpy_check_move(edict_t *self, float dist)
{
	trace_t	trace;
	vec3_t	vec, vf;

	VectorCopy(self->s.origin, vec);
	
	AngleVectors(self->s.angles, vf, NULL, NULL);
	VectorMA(vec, dist, vf, vec);

	gi.trace(self->s.origin, self->mins, self->maxs, vec, self, MASK_SHOT|MASK_WATER,&trace);

	if (trace.fraction < 1 || trace.allsolid || trace.startsolid)
	{
		if (trace.ent == self->enemy)
			return true;

		return false;
	}

	return true;
}

/*-----------------------------------------------
	harpy_ai_circle
-----------------------------------------------*/

#define	HARPY_CIRCLE_AMOUNT	4
#define HARPY_CIRCLE_SPEED  64

void harpy_ai_circle (edict_t *self, float fd, float rd, float ud)
{
	vec3_t	vf;

	self->s.angles[ROLL] += flrand(-1.25, 1);
	
	if(self->s.angles[ROLL] < -45)
		self->s.angles[ROLL] = -45;

	if(self->s.angles[ROLL] > 0)
		self->s.angles[ROLL] = 0;
	
	self->s.angles[YAW] = anglemod(self->s.angles[YAW] - (HARPY_CIRCLE_AMOUNT + (fd - 32)/4));

	AngleVectors(self->s.angles, vf, NULL, NULL);
	VectorMA(self->velocity, HARPY_CIRCLE_SPEED + fd, vf, self->velocity);
	Vec3ScaleAssign(0.5, self->velocity);

	if(!irand(0, 150))
		gi.sound(self, CHAN_VOICE, sounds[SND_SCREAM], 1, ATTN_NORM, 0);		
}

//replaces ai_walk and ai_run for harpy
void harpy_ai_glide (edict_t *self, float fd, float rd, float ud)
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

void harpy_ai_fly (edict_t *self, float fd, float rd, float ud)
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

	if (!harpy_check_move(self, fd/10))
	{
		SetAnim(self, ANIM_HOVER1);
		return;
	}

	//Add in the movements relative to the creature's facing
	AngleVectors(self->s.angles, vf, vr, vu);
	
	VectorMA(self->velocity, fd, vf, self->velocity);
	VectorMA(self->velocity, rd, vr, self->velocity);
	VectorMA(self->velocity, ud, vu, self->velocity);

	if (self->groundentity)
		self->velocity[2] += 32;
}

//replaces ai_stand for harpy
void harpy_ai_hover(edict_t *self, float dist)
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

	harpy_ai_glide(self,0,0,0);
}

//receiver for MSG_FLYBACK
void harpy_flyback(edict_t *self)
{
	SetAnim(self, ANIM_FLYBACK1);
	return;
}


void harpy_ai_pirch(edict_t *self)
{
	monsterinfo_t *monsterinfo = &self->monsterinfo;
	vec3_t	vec, vf, vr;
	float	dot, rdot, len;

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

	if (irand(0,100) < 10 && self->monsterinfo.attack_finished < level.time)
	{
		self->monsterinfo.attack_finished = level.time + 5;

		if (irand(0,1))
			gi.sound (self, CHAN_WEAPON, sounds[SND_IDLE1], 1, ATTN_NORM, 0);
		else
			gi.sound (self, CHAN_WEAPON, sounds[SND_IDLE2], 1, ATTN_NORM, 0);
	}

	AngleVectors(self->s.angles, vf, vr, NULL);

	dot = DotProduct(vec, vf);

	if (dot < 0)
	{
		SetAnim(self, ANIM_TAKEOFF);
		return;
	}

	rdot = DotProduct(vec, vr);

	//Left
	if (rdot < 0)
	{
		if (rdot < -0.8)
		{
			SetAnim(self, ANIM_PIRCH9);
		}
		else if (rdot < -0.6)
		{
			SetAnim(self, ANIM_PIRCH8);
		}
		else if (rdot < -0.4)
		{
			SetAnim(self, ANIM_PIRCH7);
		}
		else if (rdot < -0.2)
		{
			SetAnim(self, ANIM_PIRCH6);
		}
		else
		{
			SetAnim(self, ANIM_PIRCH5);
		}
	}
	else
	{
		if (rdot > 0.8)
		{
			SetAnim(self, ANIM_PIRCH1);
		}
		else if (rdot > 0.6)
		{
			SetAnim(self, ANIM_PIRCH2);
		}
		else if (rdot > 0.4)
		{
			SetAnim(self, ANIM_PIRCH3);
		}
		else if (rdot > 0.2)
		{
			SetAnim(self, ANIM_PIRCH4);
		}
		else
		{
			SetAnim(self, ANIM_PIRCH5);
		}
	}
}

void move_harpy_tumble(edict_t *self)
{
	trace_t	trace;
	vec3_t	endpos;

	self->movetype = PHYSICSTYPE_STEP;
	self->gravity = 1;

	VectorSet(self->mins, -16, -16, 0);
	VectorSet(self->maxs, 16, 16, 12);

	/*
	if (!self->avelocity[PITCH] && !self->avelocity[YAW] && !self->avelocity[ROLL])
	{
		self->avelocity[PITCH] = flrand(128.0F, 256.0F);
		self->avelocity[YAW] = flrand(64.0F, 512.0F);
		self->avelocity[ROLL] = flrand(64.0F, 512.0F);
	}
	*/

	VectorCopy(self->s.origin, endpos);
	endpos[2] -= 32;

	gi.trace(self->s.origin, self->mins, self->maxs, endpos, self, MASK_MONSTERSOLID,&trace);

	if ( (self->groundentity != NULL) || (trace.fraction != 1) || (trace.startsolid) || (trace.allsolid) || (self->monsterinfo.jump_time < level.time) )
	{
		gi.CreateEffect(&self->s, FX_DUST_PUFF, CEF_OWNERS_ORIGIN, self->s.origin, NULL);

		VectorCopy(self->s.angles, self->movedir);
		harpy_death_noise(self);

		SetAnim(self, ANIM_DIE);
	}
}

void harpy_fix_angles(edict_t *self)
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

	Harpy Message Functions

===============================================================*/

//receiver for MSG_DEATH 
void harpy_dead_pain(edict_t *self, G_Message_t *msg)
{
	if(self->health <= -40) //gib death
	{
		//harpy_throw_wings(self);
		BecomeDebris(self);
		self->think = NULL;
		self->nextthink = 0;	
		gi.linkentity (self);
		return;
	}
	else if(msg)
		MG_parse_dismember_msg(self, msg);

}

void harpy_die(edict_t *self, G_Message_t *msg)
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
	VectorSet(self->maxs, 16, 16, 12);
	
	if(self->health <= -40) //gib death
	{
		//harpy_throw_wings(self);
		gi.sound(self, CHAN_BODY, sounds[SND_GIB], 1, ATTN_NORM, 0);
		
		BecomeDebris(self);
		gi.linkentity (self);
		return;
	}

	self->msgHandler = DeadMsgHandler;

	if(irand(0, 1))
		self->svflags &= ~SVF_TAKE_NO_IMPACT_DMG;

	SetAnim(self, ANIM_DIE);
	return;
}

void harpy_dismember(edict_t *self, int damage, int HitLocation)
{
	int				throw_nodes, MeshLoc;
	vec3_t			gore_spot;
	qboolean dismember_ok = false;

	if(HitLocation & hl_MeleeHit)
	{
		dismember_ok = true;
		HitLocation &= ~hl_MeleeHit;
	}

	if(HitLocation>hl_WingedPoints)
		return;

	if (HitLocation==hl_backspikes)
		HitLocation = irand(hl_lwing, hl_rwing);

	switch (HitLocation)
	{
		case hl_head:
			
			if (self->s.fmnodeinfo[MESH_HEAD].flags & FMNI_NO_DRAW)
				dismember_ok = false;
			
			if (irand(0,10) > 2)
				dismember_ok = false;
			
			break;
			
		case hl_rightlowerleg: 
		case hl_rightupperleg:
			
			if (self->s.fmnodeinfo[MESH_RUARM].flags & FMNI_NO_DRAW)
				dismember_ok = false;
			
			if (irand(0,10) > 4)
				dismember_ok = false;
			
			break;
			
		case hl_leftlowerleg: 
		case hl_leftupperleg:	
			
			if (self->s.fmnodeinfo[MESH_LUARM].flags & FMNI_NO_DRAW)
				dismember_ok = false;
			
			if (irand(0,10) > 4)
				dismember_ok = false;
			
			break;
			
		case hl_rwing:	
			
			if (self->s.fmnodeinfo[MESH_RWING].flags & FMNI_NO_DRAW)
				dismember_ok = false;
			
			if (irand(0,10) > 6)
				dismember_ok = false;
			
			break;
			
		case hl_lwing:	
			
			if (self->s.fmnodeinfo[MESH_LWING].flags & FMNI_NO_DRAW)
				dismember_ok = false;
			
			if (irand(0,10) > 6)
				dismember_ok = false;
			
			break;
			
		default	:	
			
			dismember_ok = false;
			
			break;
	}

	if (dismember_ok)
	{
		VectorCopy(vec3_origin, gore_spot);
		gore_spot[2]+=10;
		
		throw_nodes = BPN_for_hitloc[HitLocation];

		MeshLoc = MESH_for_hitloc[HitLocation];
	
		ThrowBodyPart(self, &gore_spot, throw_nodes, damage, FRAME_partfly1);
		
		switch(MeshLoc)
		{
			case hl_head :			
				
				self->s.fmnodeinfo[MESH_HEAD].flags |= FMNI_NO_DRAW;
				self->s.fmnodeinfo[MESH_HORN].flags |= FMNI_NO_DRAW;
				self->s.fmnodeinfo[MESH_HORNS].flags |= FMNI_NO_DRAW;
				break;
				
			case hl_leftlowerleg : 	
			case hl_leftupperleg :	
				
				self->s.fmnodeinfo[MESH_LUARM].flags |= FMNI_NO_DRAW;
				self->s.fmnodeinfo[MESH_LLARM].flags |= FMNI_NO_DRAW;
				self->s.fmnodeinfo[MESH_LHAND].flags |= FMNI_NO_DRAW;
				break;
				
			case hl_rightlowerleg :	
			case hl_rightupperleg :	
				
				self->s.fmnodeinfo[MESH_RUARM].flags |= FMNI_NO_DRAW;
				self->s.fmnodeinfo[MESH_RLARM].flags |= FMNI_NO_DRAW;
				self->s.fmnodeinfo[MESH_RHAND].flags |= FMNI_NO_DRAW;
				break;
				
			default	:				
				
				self->s.fmnodeinfo[MeshLoc].flags |= FMNI_NO_DRAW;
				break;
		}

		if (HitLocation == hl_rwing || HitLocation == hl_lwing || HitLocation == hl_head) 
		{
			self->monsterinfo.jump_time = level.time + 2;
			
			if (self->health > 0)
			{
				self->health = -1;
				harpy_death_noise(self);
				SetAnim(self, ANIM_TUMBLE);
				self->msgHandler = DeadMsgHandler;
			}
		}
	}
	else
	{
		if (irand(0,10) < 1)
		{
			MeshLoc = MESH_for_hitloc[HitLocation];

			if (!(self->s.fmnodeinfo[MeshLoc].flags & FMNI_USE_SKIN))
			{
				self->s.fmnodeinfo[MeshLoc].flags |= FMNI_USE_SKIN;
				self->s.fmnodeinfo[MeshLoc].skin = self->s.skinnum+1;
			}
		}
	}
}

//receiver for MSG_PAIN
void harpy_pain(edict_t *self, G_Message_t *msg)
{
	int				temp, damage;
	qboolean		force_pain;
	
	ParseMsgParms(msg, "eeiii", &temp, &temp, &force_pain, &damage, &temp);

	if (self->curAnimID >= ANIM_PIRCH1 && self->curAnimID <= ANIM_PIRCH9)
	{
		SetAnim(self, ANIM_TAKEOFF);
		return;
	}
	if (force_pain||((irand(0,10) < 2) && (self->pain_debounce_time < level.time)))
	{
		if (irand(0,1))
			harpy_pain1_noise(self);
		else
			harpy_pain2_noise(self);

		self->pain_debounce_time = level.time + 2;
		SetAnim(self, ANIM_PAIN1);
	}
}

//receiver for MSG_STAND, MSG_HOVER 
void harpy_hover(edict_t *self, G_Message_t *msg)
{
	//NOTENOTE: We use the animation IDs to make sure we don't reject a stand message after leaving these animations
	//if (self->curAnimID >= ANIM_PIRCH1 && self->curAnimID <= ANIM_PIRCH9 && self->curAnimID != ANIM_CIRCLING)
	//	return;

	if ( (self->spawnflags & MSF_PERCHING) || (self->spawnflags & MSF_SPECIAL1) )
		return;

	if(irand(1, 10) > 3)
	{
		SetAnim(self, ANIM_HOVER1);
	}
	else
	{
		gi.sound(self, CHAN_BODY, sounds[SND_SCREAM], 1, ATTN_NORM, 0);		
		SetAnim(self, ANIM_HOVERSCREAM);
	}
}

void harpy_stand(edict_t *self, G_Message_t *msg)
{
	//NOTENOTE: We use the animation IDs to make sure we don't reject a stand message after leaving these animations
	//if (self->curAnimID >= ANIM_PIRCH1 && self->curAnimID <= ANIM_PIRCH9 && self->curAnimID != ANIM_CIRCLING)
	//	return;

	if ( (self->spawnflags & MSF_PERCHING) || (self->spawnflags & MSF_SPECIAL1) )
		return;

	if(irand(1, 10) > 3)
	{
		SetAnim(self, ANIM_HOVER1);
	}
	else
	{
		gi.sound(self, CHAN_BODY, sounds[SND_SCREAM], 1, ATTN_NORM, 0);		
		SetAnim(self, ANIM_HOVERSCREAM);
	}
}

void harpy_fly(edict_t *self, G_Message_t *msg)
{
	SetAnim(self, ANIM_FLY1);
}

void harpy_evade(edict_t *self, G_Message_t *msg)
{
	if (self->curAnimID > ANIM_PIRCH1 && self->curAnimID < ANIM_PIRCH9)
	{
		self->mins[2] -= 4;
		SetAnim(self, ANIM_TAKEOFF);
	}
}

void harpy_perch(edict_t *self, G_Message_t *msg)
{
	SetAnim(self, ANIM_PIRCH5);
}

void harpy_hit(edict_t *self)
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
		if (victim == self)
		{
			SetAnim(self, ANIM_FLYBACK1);
		}
		else
		{
			damage = irand(HARPY_DMG_MIN, HARPY_DMG_MAX);
			T_Damage (victim, self, self, vf, self->enemy->s.origin, trace.plane.normal, damage, damage*2, 0,MOD_DIED);
		}
	}
}

void harpy_pause (edict_t *self)
{
	if (M_ValidTarget(self, self->enemy))
		QPostMessage(self, MSG_RUN, PRI_DIRECTIVE, NULL);
	else if(self->curAnimID == ANIM_CIRCLING)
	{
		if(!irand(0, 6))
			SetAnim(self, ANIM_CIRCLING_FLAP);
	}
	else if(self->curAnimID == ANIM_CIRCLING_FLAP && irand(0, 1))
		SetAnim(self, ANIM_CIRCLING);
}


//end of anim func for death anim
void harpy_dead(edict_t *self)
{
	VectorSet(self->mins, -16, -16, 0);
	VectorSet(self->maxs, 16, 16, 12);

	M_EndDeath(self);
}

qboolean harpy_check_directions(edict_t *self, vec3_t goal, vec3_t vf, vec3_t vr, vec3_t vu, float checkdist, vec3_t ret)
{
	trace_t	trace;
	vec3_t	goalpos;

	//Don't always check one direction first (looks mechanical)
	if (irand(0,1))
		VectorScale(vr, -1, vr);

	VectorMA(self->s.origin, checkdist, vr, goalpos);

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

qboolean harpy_check_swoop(edict_t *self, vec3_t goal)
{
	trace_t	trace;
	vec3_t	checkpos;
	float	zd;

	//Find the difference in the target's height and the creature's height
	zd = Q_fabs(self->enemy->s.origin[2] - self->s.origin[2]);
	
	if (zd < HARPY_MIN_SSWOOP_DIST)
		return false;

	zd -= zd/4;

	VectorCopy(self->s.origin, checkpos);
	checkpos[2] -= zd;

	//Trace down about that far and about one forth the distance to the target
	gi.trace(self->s.origin, self->mins, self->maxs, checkpos, self, MASK_SHOT|MASK_WATER,&trace);

	if (trace.fraction < 1 || trace.startsolid || trace.allsolid)
	{
		//gi.dprintf("harpy_check_swoop: failed down check\n");
		return false;
	}

	//Trace straight to the target

	gi.trace(checkpos, self->mins, self->maxs, goal, self, MASK_SHOT|MASK_WATER,&trace);

	if (trace.ent != self->enemy)
	{
		//gi.dprintf("harpy_check_swoop: failed out check\n");
		return false;
	}

	//There's a clear path
	return true;
}

void move_harpy_dive(edict_t *self)
{
	vec3_t	vec, vf, enemy_pos;
	float	dist, zd, hd, forward;

	VectorSet(enemy_pos, self->enemy->s.origin[0], self->enemy->s.origin[1], self->enemy->s.origin[2] + flrand(self->maxs[2], self->enemy->maxs[2]));
	//Find out the Z and Horizontal deltas to target
	zd = Q_fabs(self->s.origin[2] - enemy_pos[2]);
	
	AngleVectors(self->s.angles, vf, NULL, NULL);

	VectorCopy(self->s.origin, vec);
	vec[2] = enemy_pos[2];

	VectorSubtract(enemy_pos, vec, vec);
	hd = VectorLength(vec);

	if ((self->groundentity != NULL) || (!harpy_check_move(self, 64)))
	{
		if (self->groundentity == self->enemy)
			SetAnim(self, ANIM_DIVE_END);

		SetAnim(self, ANIM_FLYBACK1);
		return;
	}

	dist = Q_fabs(self->s.origin[2] - enemy_pos[2]);

	forward = (256 - (dist*0.85));

	if (forward > 256)
		forward = 256;
	else if (forward < 0)
		forward = 0;

	if (dist > HARPY_MIN_SWOOP_DIST)
	{
		VectorMA(vf, forward, vf, self->velocity);
		self->velocity[2] = -dist*2.25;
		if (self->velocity[2] < -300)
			self->velocity[2] = -300;
	}
	else
	{
		SetAnim(self, ANIM_DIVE_TRANS);
		return;
	}

	harpy_ai_glide(self, 0, 0, 0);
}

void move_harpy_dive_end(edict_t *self)
{
	vec3_t	vec, vf, vr, vu, nvec, enemy_pos;
	float	dist, hd, fd, dot;
	
	VectorSet(enemy_pos, self->enemy->s.origin[0], self->enemy->s.origin[1], self->enemy->s.origin[2] + flrand(self->maxs[2], self->enemy->maxs[2]));

	VectorCopy(self->s.origin, vec);
	vec[2] = enemy_pos[2];

	VectorSubtract(enemy_pos, vec, vec);
	hd = VectorLength(vec);
	self->ideal_yaw = vectoyaw(vec);

	M_ChangeYaw(self);

	AngleVectors(self->s.angles, vf, vr, vu);
	
	self->velocity[2] *= 0.75;

	self->monsterinfo.jump_time *= HARPY_SWOOP_INCR;

	fd = self->monsterinfo.jump_time;

	if (fd > HARPY_SWOOP_SPEED_MAX)
		fd = HARPY_SWOOP_SPEED_MAX;

	if ((self->groundentity != NULL) || (!harpy_check_move(self, 128)))
	{
		if (self->groundentity == self->enemy)
			SetAnim(self, ANIM_DIVE_END);

		SetAnim(self, ANIM_FLYBACK1);
		return;
	}

	VectorSubtract(enemy_pos, self->s.origin, vec);
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
	VectorSubtract(enemy_pos, self->s.origin, vec);
	dist = VectorLength(vec);

	if (dist < HARPY_COLLISION_DIST)
	{
		SetAnim(self, ANIM_DIVE_END);
		return;
	}	

	harpy_ai_glide(self, 0, 0, 0);
}

void harpy_dive_loop(edict_t *self)
{
	SetAnim(self, ANIM_DIVE_LOOP);
}

void harpy_hit_loop(edict_t *self)
{
	SetAnim(self, ANIM_HIT_LOOP);
}

void harpy_check_dodge(edict_t *self)
{
	qboolean	dodge = false;
	trace_t		trace;
	edict_t		*ent = NULL;
	vec3_t		vec, vr, projvec, dodgedir, goalpos;
	float		dodgedot;

	if (!self->enemy)
		return;

	VectorSubtract(self->enemy->s.origin, self->s.origin, vec);
	VectorNormalize(vec);

	while ((ent = findradius(ent, self->s.origin, HARPY_PROJECTILE_RADIUS)) != NULL)
	{
		//We're only interested in his projectiles
		if (ent->owner != self->enemy)
			continue;
		
		//VectorCopy(ent->velocity, projvec);
		VectorNormalize2(ent->velocity, projvec);

		dodgedot = DotProduct(projvec, vec);

		//gi.dprintf("Found projectile with dot %f\n", dodgedot);

		if (dodgedot < -0.85 && irand(0,1))
		{
			//gi.dprintf("Dodge it!\n");

			dodge = true;
			AngleVectors(self->s.angles, NULL, vr, NULL);

			if (irand(0,1))
				VectorScale(vr, -1, vr);

			VectorMA(self->s.origin, 100, vr, goalpos);

			gi.trace(self->s.origin, self->mins, self->maxs, goalpos, self, MASK_SHOT|MASK_WATER,&trace);

			if (trace.fraction < 1 || trace.startsolid || trace.allsolid)
				VectorScale(vr, -1, dodgedir);
			else
				VectorCopy(vr, dodgedir);
		}
	}

	if (dodge)
	{
		//If he is, dodge!
		if (self->monsterinfo.misc_debounce_time < level.time)
		{
			Vec3ScaleAssign(flrand(300,500), dodgedir);
			VectorAdd(dodgedir, self->velocity, self->velocity);
			self->monsterinfo.misc_debounce_time = level.time + irand(2,4);
		}
	}	
	
	harpy_ai_glide(self, 0, 0, 0);
}

void move_harpy_hover(edict_t *self)
{
	qboolean	canmove = false, dodge = false;
	trace_t		trace;
	edict_t		*ent = NULL;
	vec3_t		goal, dodgedir, mins, maxs, vf, vr, vu, vec, projvec, goalpos;
	float		dist, zd, dodgedot;
	
	//gi.dprintf("move_harpy_hover: entered function\n");

	if (!self->enemy)
	{
		if (!FindTarget(self))
		{
			//gi.dprintf("move_harpy_hover: Enemy lost\n");
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
	if (dist > HARPY_MIN_HOVER_DIST && dist < HARPY_MAX_HOVER_DIST)
	{
		//gi.dprintf("move_harpy_hover: valid player distance\n");

		//Make sure we've got line of sight
		VectorSet(mins, -1, -1, -1);
		VectorSet(maxs, 1, 1, 1);

		gi.trace(self->s.origin, mins, maxs, self->enemy->s.origin, self, MASK_SHOT|MASK_WATER,&trace);

		//If not, try looking from a bit to the side in all six directions
		if (trace.ent != self->enemy)
		{
			//gi.dprintf("move_harpy_hover: lost line of sight to player\n");
			
			//Setup the directions
			AngleVectors(self->s.angles, vf, vr, vu);

			canmove = harpy_check_directions(self, self->enemy->s.origin, vf, vr, vu, HARPY_CHECK_DIST, goal);
			
			//If we can see him from one of these, go there
			if (canmove)
			{
				//gi.dprintf("move_harpy_hover: new position found, moving...\n");
				VectorMA(self->velocity, flrand(300.0F, 400.0F), goal, self->velocity);
				return;
			}

			//gi.dprintf("move_harpy_hover: no new direction found, bumping about\n");
			
			//Otherwise just flap around and wait, perhaps lower yourself a bit if high up
			self->velocity[0] = flrand(-HARPY_DRIFT_AMOUNT_X, HARPY_DRIFT_AMOUNT_X);
			self->velocity[1] = flrand(-HARPY_DRIFT_AMOUNT_Y, HARPY_DRIFT_AMOUNT_Y);
			self->velocity[2] = flrand(-HARPY_DRIFT_AMOUNT_Z, HARPY_DRIFT_AMOUNT_Z);

			return;
		}
		else
		{
			//Check to make sure the player isn't shooting anything

			//This won't change over the calculations
			VectorSubtract(self->enemy->s.origin, self->s.origin, vec);
			VectorNormalize(vec);

			while ((ent = findradius(ent, self->s.origin, HARPY_PROJECTILE_RADIUS)) != NULL)
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

					if (trace.fraction < 1 || trace.startsolid || trace.allsolid)
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

			//If nothing is happening, check to swoop
			canmove = harpy_check_swoop(self, self->enemy->s.origin);

			//If you can--nail um
			if (canmove)
			{
				//gi.dprintf("move_harpy_hover: valid swoop\n");
				self->monsterinfo.jump_time = 2;
				SetAnim(self, ANIM_DIVE_GO);

				return;
			}

			//If not, check to see if there's somewhere that you can get to that will allow it
			//FIXME: Too many checks.. just try something simple

			//If all else fails, then just pick a random direction to nudge yourself to
			else
			{
				//gi.dprintf("move_harpy_hover: swoop worthless\n");
				
				//Find the difference in the target's height and the creature's height
				zd = Q_fabs(self->enemy->s.origin[2] - self->s.origin[2]);
		
				//We can't swoop because we're too low, so fly upwards if possible
				if (zd < HARPY_MIN_SSWOOP_DIST)
				{
					if (!harpy_check_move(self, -64))
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
					self->velocity[0] = flrand(-HARPY_DRIFT_AMOUNT_X, HARPY_DRIFT_AMOUNT_X);
					self->velocity[1] = flrand(-HARPY_DRIFT_AMOUNT_Y, HARPY_DRIFT_AMOUNT_Y);
					self->velocity[2] = flrand(-HARPY_DRIFT_AMOUNT_Z, HARPY_DRIFT_AMOUNT_Z);

					AngleVectors(self->s.angles, vec, NULL, NULL);
					VectorMA(self->velocity, irand(200,300), vec, self->velocity);
				}

				return;
			}

		}

		//If he's too far away trace a line (expanded) to see if you can move at him
	}
	else if (dist < HARPY_MIN_HOVER_DIST)
	{
		//gi.dprintf("move_harpy_hover: backing away\n");
		if (!harpy_check_move(self, -64))
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
		//gi.dprintf("move_harpy_hover: covering ground\n");
		if (!harpy_check_move(self, 64))
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

//New physics call that modifies the harpy's velocity and angles based on aerodynamics
void harpy_flight_model(edict_t *self)
{
}

void move_harpy_fly(edict_t *self)
{	
	edict_t *dummy;
	dummy = self;
	
	return;
}

void move_harpy_die(edict_t *self)
{
	//fall to the floor
	return;
}

void harpy_hover_anim(edict_t *self)
{
	SetAnim(self, ANIM_HOVER1);
}

/*===============================================================

	Harpy Spawn Functions

===============================================================*/
void harpy_dead_pain(edict_t *self, G_Message_t *msg);

void HarpyStaticsInit()
{
	classStatics[CID_HARPY].msgReceivers[MSG_DEATH] = harpy_die;
	classStatics[CID_HARPY].msgReceivers[MSG_FLY] = harpy_hover;
	classStatics[CID_HARPY].msgReceivers[MSG_STAND] = harpy_stand;
	classStatics[CID_HARPY].msgReceivers[MSG_RUN] = harpy_hover;
	classStatics[CID_HARPY].msgReceivers[MSG_PAIN] = harpy_pain;
	classStatics[CID_HARPY].msgReceivers[MSG_DISMEMBER] = MG_parse_dismember_msg;
	classStatics[CID_HARPY].msgReceivers[MSG_WATCH] = harpy_perch;
	classStatics[CID_HARPY].msgReceivers[MSG_DEATH_PAIN] = harpy_dead_pain;
	classStatics[CID_HARPY].msgReceivers[MSG_EVADE] = harpy_evade;

	resInfo.numAnims = NUM_ANIMS;
	resInfo.animations = animations;
	resInfo.modelIndex = gi.modelindex("models/monsters/harpy/tris.fm");
	resInfo.numSounds = NUM_SOUNDS;
	resInfo.sounds = sounds;

	sounds[SND_GIB]=gi.soundindex("misc/fleshbreak.wav");	
	sounds[SND_FLAP]=gi.soundindex("monsters/harpy/flap.wav");	
	sounds[SND_SCREAM]=gi.soundindex("monsters/harpy/scream.wav");	
	sounds[SND_FLAP_FAST]=gi.soundindex("monsters/harpy/flap_quick.wav");	
	sounds[SND_DIVE]=gi.soundindex("monsters/harpy/dive.wav");	
	sounds[SND_DEATH]=gi.soundindex("monsters/harpy/death.wav");	
	sounds[SND_PAIN1]=gi.soundindex("monsters/harpy/pain1.wav");	
	sounds[SND_PAIN2]=gi.soundindex("monsters/harpy/pain2.wav");	
	sounds[SND_ATTACK]=gi.soundindex("monsters/harpy/attack.wav");	
	
	sounds[SND_IDLE1]=gi.soundindex("monsters/harpy/pain1.wav");	
	sounds[SND_IDLE2]=gi.soundindex("monsters/harpy/pain2.wav");	

	classStatics[CID_HARPY].resInfo = &resInfo;
}

/*QUAKED monster_harpy(1 .5 0) (-16 -16 -12) (16 16 12) AMBUSH ASLEEP PERCHING CIRCLING

The harpy

AMBUSH - Will not be woken up by other monsters or shots from player

ASLEEP - will not appear until triggered

PERCHING - Will watch player until get too close or get behind the harpy

CIRCLING - harpy circles around in the air

"wakeup_target" - monsters will fire this target the first time it wakes up (only once)

"pain_target" - monsters will fire this target the first time it gets hurt (only once)

*/
void SP_monster_harpy(edict_t *self)
{
	if (!flymonster_start(self))
		return;				// Failed initialization

	self->msgHandler = DefaultMsgHandler;
	self->classID = CID_HARPY;
	self->monsterinfo.dismember = harpy_dismember;

	if (!self->health)
		self->health = HARPY_HEALTH;

	self->max_health = self->health = MonsterHealth(self->health);

	self->mass = HARPY_MASS;
	self->yaw_speed = 14;

	self->movetype = PHYSICSTYPE_FLY;
	self->gravity = 0;
	self->flags |= FL_FLY;
	self->solid = SOLID_BBOX;
	self->clipmask = MASK_MONSTERSOLID;

	VectorCopy(STDMinsForClass[self->classID], self->mins);
	VectorCopy(STDMaxsForClass[self->classID], self->maxs);	

	self->svflags |= SVF_TAKE_NO_IMPACT_DMG;
	self->svflags |= SVF_DO_NO_IMPACT_DMG;

	self->materialtype = MAT_FLESH;

	self->s.modelindex = classStatics[CID_HARPY].resInfo->modelIndex;
	self->s.skinnum = 0;

	self->isBlocked = harpy_blocked;
	self->bounced = harpy_blocked;

	if (!self->s.scale)
	{
		self->monsterinfo.scale = self->s.scale = flrand(1.25, 1.75);
	}

	self->monsterinfo.otherenemyname = "monster_rat";	

	self->monsterinfo.aiflags |= AI_NO_ALERT;//pay no attention to alert ents

	if (self->spawnflags & MSF_PERCHING)
	{
		
		self->s.origin[2] += 4;
		SetAnim(self, ANIM_PIRCH5);
	}
	else if (self->spawnflags & MSF_SPECIAL1)
	{
		SetAnim(self, ANIM_CIRCLING);
	}
	else
	{
		QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
	}

	if(irand(0,1))
		self->s.fmnodeinfo[MESH_HORNS].flags |= FMNI_NO_DRAW;

	if(irand(0,1))
		self->s.fmnodeinfo[MESH_HORN].flags |= FMNI_NO_DRAW;

	if(irand(0,1))
		self->s.fmnodeinfo[MESH_BACKSPIKES].flags |= FMNI_NO_DRAW;

	if(irand(0,4))
		self->s.fmnodeinfo[MESH_NECKSPIKES].flags |= FMNI_NO_DRAW;

	if(irand(0,2))
		self->s.fmnodeinfo[MESH_TAILSPIKES].flags |= FMNI_NO_DRAW;

	gi.linkentity(self);
}
