//==============================================================================
//
// m_tcheckrik.c
//
// Heretic II
// Copyright 1998 Raven Software
//
//
//	AI :
//
//	STAND1		: Looking straight ahead
//	
//	WALK1       : a normal straight line
//  WALK2       : another normal straight line
//
//	MELEE1      : Attack 
//	MELEE2      : Attack 
//
//  RUNATTACK   : Running and swinging
//	RUN1        : chasing an enemy straight ahead
//  SHAKE		: stand and spaz
//	DIE1		: Fall back dead
//  LEAN1       : lean agains the wall
//  FIST1       : Beat against the wall in rage and desperation
//
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

#include "m_tcheckrik.h"
#include "m_tcheckrik_anim.h"
#include "g_HitLocation.h"
#include "g_misc.h"
#include "angles.h"
#include "g_HitLocation.h"
#include "c_ai.h"

#include "m_stats.h"

void insect_dismember(edict_t *self, int damage, int HitLocation);

/*----------------------------------------------------------------------
  insect Base Info
-----------------------------------------------------------------------*/
static animmove_t *animations[ NUM_ANIMS] =
{
	&insect_move_back,// = {10, insect_frames_back, insect_pause},
	&insect_move_deathfr,// = {23, insect_frames_deathfr, insect_dead},
	&insect_move_idle,// = { 50, insect_frames_idle, NULL},
	&insect_move_land,// = {6, insect_frames_land, insect_pause},
	&insect_move_inair,// = {1, insect_frames_inair, NULL},
	&insect_move_forcedjump,// = {9, insect_frames_forcedjump, insect_go_inair},
	&insect_move_finair,// = {1, insect_frames_inair, NULL},
	&insect_move_fjump,// = {9, insect_frames_forcedjump, insect_go_inair},
	&insect_move_paina,// = {7, insect_frames_paina, insect_pause},
	&insect_move_painc,// = {6, insect_frames_painc, insect_pause},
	&insect_move_run,// = {10, insect_frames_run, insect_pause},
	&insect_move_spear,// = {11, insect_frames_spear, insect_pause},
	&insect_move_sword,// = {9, insect_frames_sword, insect_pause},
	&insect_move_spell,// = {8, insect_frames_spell, insect_pause},
	&insect_move_spell2,// = {8, insect_frames_spell, insect_pause},
	&insect_move_walk,// = {20, insect_frames_walk, insect_pause},
	&insect_delay,// = {50 , insect_frames_delay, insect_pause},
	&insect_move_knock1_go,
	&insect_move_knock1_loop,
	&insect_move_knock1_end,
	&insect_move_twitch,

	// Cinematic 
	&insect_move_c_action1,
	&insect_move_c_action2,
	&insect_move_c_action3,
	&insect_move_c_action4,
	&insect_move_c_attack1, 
	&insect_move_c_attack2, 
	&insect_move_c_attack3, 
	&insect_move_c_backpedal, 
	&insect_move_c_death1, 
	&insect_move_c_idle1,
	&insect_move_c_idle2,
	&insect_move_c_idle3,
	&insect_move_c_pain1, 
	&insect_move_c_run, 
	&insect_move_c_walk, 
};

static int Sounds[NUM_SOUNDS];
static ClassResourceInfo_t resInfo;

/*----------------------------------------------------------------------
  Cinematic Functions for the monster
-----------------------------------------------------------------------*/


/*-------------------------------------------------------------------------
	plagueElf_c_anims
-------------------------------------------------------------------------*/
void insect_c_anims(edict_t *self, G_Message_t *msg)
{
	int int_msg;
	int curr_anim;

	ai_c_readmessage(self, msg);
	int_msg = (int) msg->ID;

	self->monsterinfo.c_anim_flag = 0; 

	switch(int_msg)
	{
		case MSG_C_ACTION1:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_ACTION1;
			break;
		case MSG_C_ACTION2:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_ACTION2;
			break;
		case MSG_C_ACTION3:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_ACTION3;
			break;
		case MSG_C_ACTION4:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_ACTION4;
			break;
		case MSG_C_ATTACK1:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_ATTACK1;
			break;
		case MSG_C_ATTACK2:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_ATTACK2;
			break;
		case MSG_C_ATTACK3:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_ATTACK3;
			break;
		case MSG_C_BACKPEDAL1:
			self->monsterinfo.c_anim_flag |= C_ANIM_MOVE;
			curr_anim = ANIM_C_BACKPEDAL;
			break;
		case MSG_C_DEATH1:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_DEATH1;
			break;
		case MSG_C_IDLE1:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT | C_ANIM_IDLE;
			curr_anim = ANIM_C_IDLE1;
			break;
		case MSG_C_IDLE2:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_IDLE2;
			break;
		case MSG_C_IDLE3:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_IDLE3;
			break;
		case MSG_C_PAIN1:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_PAIN1;
			break;
		case MSG_C_RUN1:
			self->monsterinfo.c_anim_flag |= C_ANIM_MOVE;
			curr_anim = ANIM_C_RUN1;
			break;
		case MSG_C_WALK1:
			self->monsterinfo.c_anim_flag |= C_ANIM_MOVE;
			curr_anim = ANIM_C_WALK1;
			break;
		default:
			self = self;
			break;
	} 

	SetAnim(self, curr_anim);
}



/*-------------------------------------------------------------------------
	insect_c_pause
-------------------------------------------------------------------------*/
void insect_c_reallydead (edict_t *self)
{
	self->nextthink = level.time;
	self->think = NULL;
}






/*----------------------------------------------------------------------
  Action Functions for the monster
-----------------------------------------------------------------------*/

/*-------------------------------------------------------------------------
	insectCut
-------------------------------------------------------------------------*/
vec3_t TC_WPN_MINS = {-2.0, -2.0, -1.0};
vec3_t TC_WPN_MAXS = {2.0, 2.0, 1.0};
void insectCut (edict_t *self, float attacktype)
{
	trace_t trace;
	vec3_t pos1, pos2, dir, forward, right, hitangles;
	float damage;

	AngleVectors(self->s.angles, forward, right, NULL);
	VectorCopy(self->s.origin, pos1);
	VectorMA (pos1, 72, forward, pos2);
	switch((int)attacktype)
	{
	case TC_ATK_STAB:
		if(self->s.fmnodeinfo[MESH__SPEAR].flags & FMNI_NO_DRAW)
			return;
		VectorMA(pos2, -16, right, pos2);
		damage = irand(TC_DMG_STAB_MIN, TC_DMG_STAB_MAX);
		break;
	case TC_ATK_HACK:
//		if(self->s.fmnodeinfo[MESH__SWORD].flags & FMNI_NO_DRAW)
		if(self->s.fmnodeinfo[MESH__MALEHAND].flags & FMNI_NO_DRAW)
		{
			VectorMA (pos1, 12, forward, pos1);
			pos1[2]+= 28;
			VectorMA(pos2, 24, right, pos2);
			damage = irand(TC_MALE_DMG_HACK_MIN, TC_MALE_DMG_HACK_MAX);
		}
		else
		{
			VectorMA (pos1, 12, forward, pos1);
			pos1[2]+= 18;
			VectorMA(pos2, 12, right, pos2);
			damage = irand(TC_FEMALE_DMG_HACK_MIN, TC_FEMALE_DMG_HACK_MAX);
		}
		break;
	default:
		return;
		break;
	}

	gi.trace(pos1, TC_WPN_MINS, TC_WPN_MAXS, pos2, self, MASK_MONSTERSOLID|MASK_SHOT,&trace);

	//sfs--do this check before the allsolid check, because trace is screwy--fraction should be valid in all cases, so shouldn't be a problem
	if(trace.fraction == 1.0)
	{//missed totally
		return;
	}

	if(trace.allsolid || trace.startsolid || !trace.ent->takedamage)
	{//ping!
		vectoangles(trace.plane.normal, hitangles);
		gi.CreateEffect(NULL, FX_SPARKS, 0, trace.endpos, "d", hitangles);
		gi.sound (self, CHAN_AUTO, Sounds[SND_SWIPEHITW], 1, ATTN_NORM, 0);
//		gi.sound(self, CHAN_WEAPON, gi.soundindex("weapons/staffhitwall.wav"), 1, ATTN_NORM, 0);
		return;		
	}

	//hit someone, cut em!
	
	VectorSubtract(pos2, pos1, dir);
	gi.sound (self, CHAN_AUTO, Sounds[SND_SWIPEHITF], 1, ATTN_NORM, 0);
//	gi.sound(self, CHAN_WEAPON, gi.soundindex("weapons/staffhit.wav"), 1, ATTN_NORM, 0);

	T_Damage (trace.ent, self, self, dir, trace.endpos, vec3_origin, damage, damage*2, DAMAGE_DISMEMBER,MOD_DIED);
	
/*	gi.CreateEffect(&self->s,
		FX_I_EFFECTS,
		0,
		vec3_origin,
		"bv",
		FX_I_SWORD,
		vec3_origin);*/
}


/*-------------------------------------------------------------------------
	insect_dead
-------------------------------------------------------------------------*/
void insect_dead(edict_t *self)
{
	self->s.effects |= EF_DISABLE_EXTRA_FX;
	self->msgHandler = DeadMsgHandler;
	self->deadState = DEAD_DEAD;
	M_EndDeath(self);
}

/*-------------------------------------------------------------------------
	insect_death
-------------------------------------------------------------------------*/
void insect_random_death_sound (edict_t *self)
{
	if(self->mass==MASS_TC_MALE)//irand(0,1))
		gi.sound(self, CHAN_VOICE, Sounds[SND_DIEM], 1, ATTN_NORM, 0);
	else
		gi.sound(self, CHAN_VOICE, Sounds[SND_DIEF], 1, ATTN_NORM, 0);
}

void insect_random_idle_sound (edict_t *self)
{
	if(self->mass==MASS_TC_MALE)//
	{
		if(irand(0,1))
			gi.sound(self, CHAN_VOICE, Sounds[SND_GROWLM1], 1, ATTN_NORM, 0);
		else
			gi.sound(self, CHAN_VOICE, Sounds[SND_GROWLM2], 1, ATTN_NORM, 0);
	}
	else
	{
		if(irand(0,1))
			gi.sound(self, CHAN_VOICE, Sounds[SND_GROWLF1], 1, ATTN_NORM, 0);
		else
			gi.sound(self, CHAN_VOICE, Sounds[SND_GROWLF2], 1, ATTN_NORM, 0);
	}
}

void M_MoveFrame (edict_t *self);
void insect_dead_pain (edict_t *self, G_Message_t *msg)
{
	if(msg)
		if(!(self->svflags & SVF_PARTS_GIBBED))
			MG_parse_dismember_msg(self, msg);

	if(self->curAnimID!=ANIM_TWITCH&&self->deadState!=DEAD_DEAD)
		return;//still dying

	if(self->s.frame==FRAME_knock15)
	{
		self->think = M_MoveFrame;
		SetAnim(self, ANIM_TWITCH);
		self->nextthink = level.time + 0.1;
	}
}

void insect_wait_twitch (edict_t *self)
{
	if(self->curAnimID!=ANIM_TWITCH&&!irand(0,1))
	{
		SetAnim(self, ANIM_TWITCH);
		self->nextthink = level.time + 0.1;
	}
	else if(irand(0, 5))
	{
		SetAnim(self, ANIM_TWITCH);
		self->nextthink = level.time + flrand(0.2, 10);
	}
	else
	{
		self->s.effects |= EF_DISABLE_EXTRA_FX;
		self->msgHandler = DeadMsgHandler;
		self->deadState = DEAD_DEAD;
		M_EndDeath(self);
		self->think = NULL;
	}
}

void insect_flyback_loop(edict_t *self)
{
	SetAnim(self, ANIM_KNOCK1_LOOP);
}

void insect_flyback_move(edict_t *self)
{
	vec3_t	endpos;
	trace_t	trace;

	M_ChangeYaw(self);

	VectorCopy(self->s.origin, endpos);
	endpos[2] -= 48;

	gi.trace(self->s.origin, self->mins, self->maxs, endpos, self, MASK_MONSTERSOLID,&trace);

	if ( (	trace.fraction < 1 || trace.startsolid || trace.allsolid ) && 
			self->curAnimID != ANIM_KNOCK1_END && 
			self->curAnimID != ANIM_KNOCK1_GO)
	{
		self->elasticity = 1.1;
		self->friction = 0.5;
		SetAnim(self, ANIM_KNOCK1_END);
	}
}

void insect_death(edict_t *self, G_Message_t *msg)
{
	if(self->monsterinfo.aiflags&AI_DONT_THINK)
	{
		SetAnim(self, ANIM_DEATHFR);
		return;
	}

	self->msgHandler = DeadMsgHandler;

	if(self->deadflag == DEAD_DEAD) //Dead but still being hit	
		return;

	gi.RemoveEffects(&self->s, FX_I_EFFECTS);
	self->s.effects |= EF_DISABLE_EXTRA_FX;

	self->deadflag = DEAD_DEAD;

	insect_dropweapon (self, BIT_SPEAR);
	insect_dropweapon (self, BIT_STAFF);
//	insect_dropweapon (self, BIT_SWORD);

	if(self->health <= -80) //gib death
	{
		gi.sound(self, CHAN_BODY, Sounds[SND_GIB], 1, ATTN_NORM, 0);
		self->think = BecomeDebris;
		self->nextthink = level.time + 0.1;
		return;
	}
	else
	{
		insect_random_death_sound(self);
		if(self->health<-20)
		{
			vec3_t vf, yf, dVel;

			SetAnim(self, ANIM_KNOCK1_GO);

			VectorSet(self->knockbackvel, 0, 0, 0);

			VectorCopy(self->velocity, vf);
			VectorNormalize(vf);

			VectorScale(vf, -1, yf);

			self->ideal_yaw = vectoyaw( yf );
			self->yaw_speed = 16;

			VectorScale(vf, 300, dVel);
			dVel[2] = irand(150,250);

			VectorCopy(dVel, self->velocity);
			return;
		}

		SetAnim(self, ANIM_DEATHFR);
		return;
	}
}


/*-------------------------------------------------------------------------
	insectdeathsqueal
-------------------------------------------------------------------------*/
void insectdeathsqueal (edict_t *self)
{
//	gi.sound(self, CHAN_WEAPON, Sounds[SND_DIE1], 1, ATTN_IDLE, 0);
	return;
}

/*-------------------------------------------------------------------------
	insectgrowl
-------------------------------------------------------------------------*/
void insectgrowl (edict_t *self)
{
	int chance;	

	chance = irand(0, 10);

	if (chance <= 2 )
	{
		if(irand(0, 1))
		{
			if(self->mass == MASS_TC_MALE)
				gi.sound (self, CHAN_WEAPON, Sounds[SND_GROWLM1], 1, ATTN_IDLE, 0);
			else 
				gi.sound (self, CHAN_WEAPON, Sounds[SND_GROWLF1], 1, ATTN_IDLE, 0);
		}
		else 
		{
			if(self->mass == MASS_TC_MALE)
				gi.sound (self, CHAN_WEAPON, Sounds[SND_GROWLM2], 1, ATTN_IDLE, 0);
			else 
				gi.sound (self, CHAN_WEAPON, Sounds[SND_GROWLF2], 1, ATTN_IDLE, 0);
		}
	}
}

/*-------------------------------------------------------------------------
	insect_melee
-------------------------------------------------------------------------*/
/*
void insect_melee(edict_t *self, G_Message_t *msg)
{
	if (!self->enemy)
		return;

	if (self->enemy->health <= 0)
	{
		if (!FindTarget(self))
		{
			QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
			return;
		}
	}

	if (anglemod(self->s.angles[YAW] - self->ideal_yaw) > self->yaw_speed)
	{
		M_ChangeYaw(self);
		SetAnim(self, ANIM_RUN);
		return;
	}

	if(self->mass == MASS_TC_MALE)
	{//male
		if(irand(0, 1)&&self->s.fmnodeinfo[MESH__SPEAR].flags & FMNI_NO_DRAW)
			SetAnim(self, ANIM_SPEAR);
		else// if(self->s.fmnodeinfo[MESH__SWORD].flags & FMNI_NO_DRAW)
			SetAnim(self, ANIM_SWORD);	
	}
	else//if no staff, diff spell?
		SetAnim(self, ANIM_SPELL);
}
*/

void insect_backpedal(edict_t *self, G_Message_t *msg)
{
	if (M_ValidTarget(self, self->enemy))
	{
		if(self->spawnflags&MSF_FIXED)
			SetAnim(self, ANIM_DELAY);
		else
			SetAnim(self, ANIM_BACK);//not male?
	}
	else
		QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
}

void insect_sound(edict_t *self, float channel, float soundnum, float attn)
{
	gi.sound (self, (int)(channel), Sounds[(int)(soundnum)], 1, (int)(attn), 0);
}

void insect_melee(edict_t *self, G_Message_t *msg)
{
	if (M_ValidTarget(self, self->enemy))
	{
		if(self->mass == MASS_TC_MALE)
		{//male
			if(self->enemy->classID == CID_TBEAST && self->enemy->enemy == self)
			{
				if(M_DistanceToTarget(self, self->enemy) < 250)
				{
					self->monsterinfo.aiflags |= AI_FLEE;
					self->monsterinfo.flee_finished = level.time + 3;
					QPostMessage(self, MSG_RUN, PRI_DIRECTIVE, NULL);
					return;
				}
			}
			SetAnim(self, ANIM_SWORD);	
		}
		else//too close for female, back away(maybe attack anyway?)
		{
			if(!irand(0,5)&&!(self->monsterinfo.aiflags&AI_NO_MELEE)||(self->spawnflags&MSF_FIXED))
				SetAnim(self, ANIM_SPELL);
			else
				SetAnim(self, ANIM_BACK);
		}
	}
	else
		QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
}

void insect_missile(edict_t *self, G_Message_t *msg)
{
	if (M_ValidTarget(self, self->enemy))
	{
		if(self->mass == MASS_TC_MALE)
		{//male
			if(self->enemy->classID == CID_TBEAST && self->enemy->enemy == self)
			{
				if(M_DistanceToTarget(self, self->enemy) < 250)
				{
					if(self->enemy->enemy != self)
					{
						self->enemy->oldenemy = self->enemy->enemy;
						self->enemy->enemy = self;
					}
					self->monsterinfo.aiflags |= AI_FLEE;
					self->monsterinfo.flee_finished = level.time + 3;
					QPostMessage(self, MSG_RUN, PRI_DIRECTIVE, NULL);
					return;
				}
			}
			SetAnim(self, ANIM_SPEAR);	
		}
		else
		{
			if(self->spawnflags & MSF_INSECT_ALTERNATE)
				SetAnim(self, ANIM_SPELL2);
			else
				SetAnim(self, ANIM_SPELL);
		}
	}
	else
		QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
}

extern void SpellCastInsectSpear(edict_t *Caster,vec3_t StartPos,vec3_t AimAngles,int offset);
//extern void SpellCastFlyingFist(edict_t *Caster,vec3_t StartPos,vec3_t AimAngles,vec3_t AimDir,float Value);
void insectStaff(edict_t *self)
{
	vec3_t	org, forward, right;

	AngleVectors(self->s.angles, forward, right, NULL);
	VectorCopy(self->s.origin, org);
	VectorMA(org, 12, forward, org);
	VectorMA(org, 4, right, org);
	if(self->spawnflags & MSF_INSECT_YELLOWJACKET)
	{
		if(self->s.frame == FRAME_SpearB4)
		{
			if(!skill->value)
				SpellCastInsectSpear(self, org, self->s.angles, irand(1, 3));
			else if(skill->value == 1)
			{
				SpellCastInsectSpear(self, org, self->s.angles, 1);
				SpellCastInsectSpear(self, org, self->s.angles, 2);
			}
			else
			{
				SpellCastInsectSpear(self, org, self->s.angles, 1);
				SpellCastInsectSpear(self, org, self->s.angles, 2);
				SpellCastInsectSpear(self, org, self->s.angles, 3);
			}
			gi.sound (self, CHAN_WEAPON, Sounds[SND_SPELLM2], 1, ATTN_NORM, 0);
		}
	}
	else
	{
		SpellCastInsectSpear(self, org, self->s.angles, false);
		gi.sound (self, CHAN_WEAPON, Sounds[SND_SPELLM], 1, ATTN_NORM, 0);
	}
}

void imp_fireball (edict_t *self);
void insectSpell(edict_t *self, float whichspell)
{
	vec3_t	org, forward, right;

	if (!(self->spawnflags & MSF_INSECT_CINEMATIC))
		ai_charge(self, 0);
	
	AngleVectors(self->s.angles, forward, right, NULL);
	switch((int)whichspell)
	{
	case TC_SPL_GLOW:
		if(self->s.fmnodeinfo[MESH__STAFF].flags & FMNI_NO_DRAW)
		{//no staff, weaker spell
			VectorCopy(self->s.origin, org);
			VectorMA(org, 8, forward, org);
			VectorMA(org, 2, right, org);
			org[2] += 6;
			SpellCastInsectStaff(self, org, self->s.angles, forward, 0);
			gi.sound (self, CHAN_WEAPON, Sounds[SND_SPELLF], 1, ATTN_NORM, 0);
			//SpellCastFlyingFist(self, org, self->s.angles, forward, 0);
		}
		else
		{//staff, bigger spell
			VectorCopy(self->s.origin, org);
			VectorMA(org, 16, forward, org);
			VectorMA(org, 4, right, org);
			org[2] += 8;

			self->s.effects &= ~(EF_DISABLE_EXTRA_FX | EF_MARCUS_FLAG1);
			self->damage_debounce_time = false;
			SpellCastGlobeOfOuchiness(self, org, self->s.angles, forward);
			gi.sound (self, CHAN_ITEM, Sounds[SND_SPLPWRUPF], 1, ATTN_NORM, 0);
		}
		break;
	case TC_SPL_FIRE:
		if(self->s.fmnodeinfo[MESH__STAFF].flags & FMNI_NO_DRAW)
		{//no staff
			VectorCopy(self->s.origin, org);	// We need to initialize org, don't we?
			VectorMA(org, 16, forward, org);
			org[2] += 12;
			SpellCastInsectStaff(self, org, self->s.angles, forward, 0);
			gi.sound (self, CHAN_AUTO, Sounds[SND_SPELLF], 1, ATTN_NORM, 0);
			return;
		}
		else
		{
			VectorCopy(self->s.origin, org);
			VectorMA(org, 16, forward, org);
			org[2] += 12;
			SpellCastInsectStaff(self, org, self->s.angles, forward, 0);
			gi.sound (self, CHAN_AUTO, Sounds[SND_SPELLF], 1, ATTN_NORM, 0);
		}
		break;
	case TC_SPL_FIRE2:
		if(!(self->s.fmnodeinfo[MESH__STAFF].flags & FMNI_NO_DRAW))
		{//no staff
			VectorCopy(self->s.origin, org);
			VectorMA(org, 16, forward, org);
			org[2] += 12;
			SpellCastInsectStaff(self, org, self->s.angles, forward, 1);
			gi.sound(self, CHAN_AUTO, gi.soundindex("monsters/imp/fireball.wav"), 1, ATTN_NORM, 0);
		}
		else
		{
			VectorCopy(self->s.origin, org);
			VectorMA(org, 16, forward, org);
			org[2] += 12;
			SpellCastInsectStaff(self, org, self->s.angles, forward, 0);
			gi.sound (self, CHAN_AUTO, Sounds[SND_SPELLF], 1, ATTN_NORM, 0);
		}
		break;
	}

}

void insectReleaseSpell (edict_t *self)
{
	gi.RemoveEffects(&self->s, FX_I_EFFECTS);
	self->s.effects |= EF_DISABLE_EXTRA_FX | EF_MARCUS_FLAG1;
	self->damage_debounce_time = true;
}

/*-------------------------------------------------------------------------
	insect_pain
-------------------------------------------------------------------------*/
int Bit_for_MeshNode_tc [16] =
{
	BIT_MASTER,
	BIT_LLEG,			
	BIT_HEAD,			
	BIT_LMANDIBLE,		
	BIT_RMANDIBLE,		
	BIT_CROWN,			
	BIT_L2NDARM,	
	BIT_SPEAR,		
	BIT_FEMHAND,	
	BIT_SWORD,	
	BIT_STAFF,		
	BIT_GEM,	
	BIT_R2NDARM,
	BIT_RWINGS,	
	BIT_LWINGS,	
	BIT_RLEG		
};

qboolean canthrownode_tc (edict_t *self, int BP, int *throw_nodes)
{//see if it's on, if so, add it to throw_nodes
	//turn it off on thrower
	if(!(self->s.fmnodeinfo[BP].flags & FMNI_NO_DRAW))
	{
		*throw_nodes |= Bit_for_MeshNode_tc[BP];
		self->s.fmnodeinfo[BP].flags |= FMNI_NO_DRAW;
		return true;
	}
	return false;
}

void insect_chicken (edict_t *self, int coward, int flee, float fleetime)
{
	float chance;

	chance = flrand(0,10);
	if(chance<coward)
	{
		self->monsterinfo.aiflags |= AI_COWARD;
		return;
	}

	if(chance<flee)
	{
		self->monsterinfo.aiflags |= AI_FLEE;
		self->monsterinfo.flee_finished = level.time + fleetime;
	}
}

//THROWS weapon, turns off those nodes, sets that weapon as gone
void insect_dropweapon (edict_t *self, int weapon)
{//NO PART FLY FRAME!
	vec3_t handspot, forward, right, up;
//	float chance;

	VectorClear(handspot);
	AngleVectors(self->s.angles,forward,right,up);

//	if(self->deadflag == DEAD_DEAD||(self->s.fmnodeinfo[MESH__R_ARM].flags & FMNI_NO_DRAW))
//		chance = 0;
//	else
//		chance = 3;
	if((!weapon || weapon&BIT_STAFF) &&
		!(self->s.fmnodeinfo[MESH__STAFF].flags & FMNI_NO_DRAW))
	{
		VectorMA(handspot,8,forward,handspot);
		VectorMA(handspot,5,right,handspot);
		VectorMA(handspot,12,up,handspot);
		ThrowWeapon(self, &handspot, BIT_STAFF, 0, FRAME_partfly);
		self->s.fmnodeinfo[MESH__STAFF].flags |= FMNI_NO_DRAW;
		self->s.fmnodeinfo[MESH__GEM].flags |= FMNI_NO_DRAW;
//		insect_chicken(self,2,4,flrand(3,8));
		return;
	}
	if((!weapon || weapon&BIT_SPEAR)&&
		!(self->s.fmnodeinfo[MESH__SPEAR].flags & FMNI_NO_DRAW))
	{
		VectorMA(handspot,6,forward,handspot);
		VectorMA(handspot,4,right,handspot);
		ThrowWeapon(self, &handspot, BIT_SPEAR, 0, FRAME_partfly);
		self->s.fmnodeinfo[MESH__SPEAR].flags |= FMNI_NO_DRAW;
//		insect_chicken(self,2,4,flrand(3,8));
		return;
	}
	if((!weapon || weapon&BIT_SWORD)&&
		!(self->s.fmnodeinfo[MESH__MALEHAND].flags & FMNI_NO_DRAW))
//		!(self->s.fmnodeinfo[MESH__SWORD].flags & FMNI_NO_DRAW))
	{
		VectorMA(handspot,6,forward,handspot);
		VectorMA(handspot,-6,right,handspot);
		VectorMA(handspot,-6,up,handspot);
		ThrowWeapon(self, &handspot, BIT_SWORD, 0, FRAME_partfly);
		self->s.fmnodeinfo[MESH__MALEHAND].flags |= FMNI_NO_DRAW;
//		self->s.fmnodeinfo[MESH__SWORD].flags |= FMNI_NO_DRAW;
//		insect_chicken(self,2,4,flrand(3,8));
		return;
	}
}

void insect_dismember(edict_t *self, int damage, int HitLocation)
{//fixme: throw current weapon
//fixme - make part fly dir the vector from hit loc to sever loc
//remember- turn on caps!
	int				throw_nodes = 0;
	vec3_t			gore_spot, right;
	qboolean dismember_ok = false;

	if(HitLocation & hl_MeleeHit)
	{
		dismember_ok = true;
		HitLocation &= ~hl_MeleeHit;
	}

	if(HitLocation<1)
		return;

	if(HitLocation>hl_Max)
		return;
//	gi.dprintf("HL: %d",HitLocation);

/*	if(self->curAnimID==ANIM_MELEE1||self->curAnimID==ANIM_MELEE1)
	{//Hit chest during melee, may have hit arms
		if(HitLocation == hl_TorsoFront&&irand(0,10)<4)
		{
			if(irand(0,10)<7)
				HitLocation = hl_ArmLowerRight;
			else
				HitLocation = hl_ArmLowerLeft;
		}
	}

	if(
		(HitLocation == hl_ArmUpperLeft&& self->s.fmnodeinfo[MESH__L_ARM].flags & FMNI_NO_DRAW) ||
		(HitLocation == hl_ArmUpperRight&& self->s.fmnodeinfo[MESH__R_ARM].flags & FMNI_NO_DRAW)||
		(
			(HitLocation == hl_TorsoFront|| HitLocation == hl_TorsoBack) &&
			self->s.fmnodeinfo[MESH__R_ARM].flags & FMNI_NO_DRAW &&
			self->s.fmnodeinfo[MESH__L_ARM].flags & FMNI_NO_DRAW &&
			irand(0,10)<4)
		)
		HitLocation = hl_Head;//Decap
*/
	self->monsterinfo.aiflags &= ~AI_OVERRIDE_GUIDE;
	VectorClear(gore_spot);
	switch(HitLocation)
	{
		case hl_Head:
			if(self->s.fmnodeinfo[MESH__HEAD].flags & FMNI_NO_DRAW)
				break;
			if(self->s.fmnodeinfo[MESH__HEAD].flags & FMNI_USE_SKIN)
				damage*=1.5;//greater chance to cut off if previously damaged
			if(flrand(0,self->health)<damage*0.25)
				insect_dropweapon (self, 0);
			if(flrand(0,self->health)<damage*0.3&&dismember_ok)
			{
				canthrownode_tc(self, MESH__HEAD,&throw_nodes);
				canthrownode_tc(self, MESH__CROWN,&throw_nodes);
				canthrownode_tc(self, MESH__LMANDIBLE,&throw_nodes);
				canthrownode_tc(self, MESH__RMANDIBLE,&throw_nodes);

				gore_spot[2]+=24;
				ThrowBodyPart(self, &gore_spot, throw_nodes, damage, FRAME_partfly);

				VectorAdd(self->s.origin, gore_spot, gore_spot);
				SprayDebris(self,gore_spot,8,damage);

				if(self->health>0)
				{
					self->health = 1;
					T_Damage (self, self, self, vec3_origin, vec3_origin, vec3_origin, 10, 20,0,MOD_DIED);
				}
				return;
			}
			else
			{
				if(irand(0,1)||self->s.fmnodeinfo[MESH__HEAD].flags & FMNI_USE_SKIN)
				{
					if(self->mass == MASS_TC_MALE)
					{
						if(irand(0,1)||self->s.fmnodeinfo[MESH__RMANDIBLE].flags & FMNI_USE_SKIN)
						{
							self->s.fmnodeinfo[MESH__LMANDIBLE].flags |= FMNI_USE_SKIN;
							self->s.fmnodeinfo[MESH__LMANDIBLE].skin = self->s.skinnum+1;
						}
						else
						{
							self->s.fmnodeinfo[MESH__RMANDIBLE].flags |= FMNI_USE_SKIN;
							self->s.fmnodeinfo[MESH__RMANDIBLE].skin = self->s.skinnum+1;
						}
					}
					else
					{
						self->s.fmnodeinfo[MESH__CROWN].flags |= FMNI_USE_SKIN;
						self->s.fmnodeinfo[MESH__CROWN].skin = self->s.skinnum+1;
					}
				}
				else
				{
					self->s.fmnodeinfo[MESH__HEAD].flags |= FMNI_USE_SKIN;
					self->s.fmnodeinfo[MESH__HEAD].skin = self->s.skinnum+1;
				}
			}
			break;
		case hl_TorsoFront://split in half?
		case hl_TorsoBack://split in half?
			if(self->s.fmnodeinfo[MESH_MASTER].flags & FMNI_USE_SKIN)
				break;
			if(flrand(0,self->health)<damage*0.5)
				insect_dropweapon (self, 0);
			self->s.fmnodeinfo[MESH_MASTER].flags |= FMNI_USE_SKIN;			
			self->s.fmnodeinfo[MESH_MASTER].skin = self->s.skinnum+1;
			break;
		case hl_ArmUpperLeft:
			if(self->mass == MASS_TC_FEMALE)
			{//female
				if(self->s.fmnodeinfo[MESH_MASTER].flags & FMNI_USE_SKIN)
					return;
				self->s.fmnodeinfo[MESH_MASTER].flags |= FMNI_USE_SKIN;			
				self->s.fmnodeinfo[MESH_MASTER].skin = self->s.skinnum+1;
				return;
			}
			else
			{//male - sword arm
				if(self->s.fmnodeinfo[MESH__L2NDARM].flags & FMNI_USE_SKIN)
					damage*=1.5;//greater chance to cut off if previously damaged
				if(flrand(0,self->health)<damage*0.4)
					insect_dropweapon (self, BIT_SWORD);
				self->s.fmnodeinfo[MESH__L2NDARM].flags |= FMNI_USE_SKIN;			
				self->s.fmnodeinfo[MESH__L2NDARM].skin = self->s.skinnum+1;
			}
			break;
		case hl_ArmLowerLeft://left arm
			if(self->mass == MASS_TC_FEMALE)
			{//female
				if(self->s.fmnodeinfo[MESH_MASTER].flags & FMNI_USE_SKIN)
					return;
				self->s.fmnodeinfo[MESH_MASTER].flags |= FMNI_USE_SKIN;			
				self->s.fmnodeinfo[MESH_MASTER].skin = self->s.skinnum+1;
				return;
			}
			else
			{//male - left spear arm
				if(self->s.fmnodeinfo[MESH__L2NDARM].flags & FMNI_NO_DRAW)
					break;
				if(self->s.fmnodeinfo[MESH__L2NDARM].flags & FMNI_USE_SKIN)
					damage*=1.5;//greater chance to cut off if previously damaged
				if(flrand(0,self->health)<damage*0.75&&dismember_ok)
				{
					if(canthrownode_tc(self, MESH__L2NDARM, &throw_nodes))
					{
						AngleVectors(self->s.angles,NULL,right,NULL);
						gore_spot[2]+=self->maxs[2]*0.3;
						VectorMA(gore_spot,-10,right,gore_spot);
//						insect_chicken(self,6,8,flrand(7,15));
						ThrowBodyPart(self, &gore_spot, throw_nodes, damage, FRAME_partfly);
						if(self->s.fmnodeinfo[MESH__R2NDARM].flags & FMNI_NO_DRAW&&
							!(self->s.fmnodeinfo[MESH__SPEAR].flags & FMNI_NO_DRAW))
							insect_dropweapon (self, BIT_SPEAR);
					}
				}
				else
				{
					if(flrand(0,self->health)<damage*0.4)
						insect_dropweapon (self, BIT_SPEAR);
					self->s.fmnodeinfo[MESH__L2NDARM].flags |= FMNI_USE_SKIN;			
					self->s.fmnodeinfo[MESH__L2NDARM].skin = self->s.skinnum+1;
				}
			}
			break;
		case hl_ArmUpperRight:
			if(self->mass == MASS_TC_FEMALE)
			{//female
				if(self->s.fmnodeinfo[MESH_MASTER].flags & FMNI_USE_SKIN)
					damage*=1.5;//greater chance to cut off if previously damaged
				else
				{
					self->s.fmnodeinfo[MESH_MASTER].flags |= FMNI_USE_SKIN;			
					self->s.fmnodeinfo[MESH_MASTER].skin = self->s.skinnum+1;
				}
				if(flrand(0,self->health)<damage*0.4)
					insect_dropweapon (self, BIT_STAFF);
			}
			else
			{
				if(self->s.fmnodeinfo[MESH_MASTER].flags & FMNI_USE_SKIN)
					return;
				self->s.fmnodeinfo[MESH_MASTER].flags |= FMNI_USE_SKIN;			
				self->s.fmnodeinfo[MESH_MASTER].skin = self->s.skinnum+1;
				return;
			}
			//male - right upper arm- nothing in it
			break;
		case hl_ArmLowerRight://right arm
			if(self->mass == MASS_TC_FEMALE)
			{//female
				if(self->s.fmnodeinfo[MESH_MASTER].flags & FMNI_USE_SKIN)
					damage*=1.5;//greater chance to cut off if previously damaged
				else
				{
					self->s.fmnodeinfo[MESH_MASTER].flags |= FMNI_USE_SKIN;			
					self->s.fmnodeinfo[MESH_MASTER].skin = self->s.skinnum+1;
				}
				if(flrand(0,self->health)<damage*0.4)
					insect_dropweapon (self, BIT_STAFF);
			}
			else
			{//male - right spear arm
				if(self->s.fmnodeinfo[MESH__R2NDARM].flags & FMNI_NO_DRAW)
					break;
				if(self->s.fmnodeinfo[MESH__R2NDARM].flags & FMNI_USE_SKIN)
					damage*=1.5;//greater chance to cut off if previously damaged
				if(flrand(0,self->health)<damage*0.75&&dismember_ok)
				{
					if(canthrownode_tc(self, MESH__R2NDARM, &throw_nodes))
					{
						AngleVectors(self->s.angles,NULL,right,NULL);
						gore_spot[2]+=self->maxs[2]*0.3;
						VectorMA(gore_spot,-10,right,gore_spot);
//						insect_chicken(self,6,8,flrand(7,15));
						ThrowBodyPart(self, &gore_spot, throw_nodes, damage, FRAME_partfly);
						if(self->s.fmnodeinfo[MESH__L2NDARM].flags & FMNI_NO_DRAW&&
							!(self->s.fmnodeinfo[MESH__SPEAR].flags & FMNI_NO_DRAW))
							insect_dropweapon (self, BIT_SPEAR);
					}
				}
				else
				{
					if(flrand(0,self->health)<damage*0.4)
						insect_dropweapon (self, BIT_SPEAR);
					self->s.fmnodeinfo[MESH__R2NDARM].flags |= FMNI_USE_SKIN;			
					self->s.fmnodeinfo[MESH__R2NDARM].skin = self->s.skinnum+1;
				}
			}
			break;

		case hl_LegUpperLeft:
		case hl_LegLowerLeft://left leg
			if(self->health>0)
			{//still alive
				if(self->s.fmnodeinfo[MESH__LLEG].flags & FMNI_USE_SKIN)
					break;
				self->s.fmnodeinfo[MESH__LLEG].flags |= FMNI_USE_SKIN;			
				self->s.fmnodeinfo[MESH__LLEG].skin = self->s.skinnum+1;
				break;
			}
			else
			{
				if(self->s.fmnodeinfo[MESH__LLEG].flags & FMNI_NO_DRAW)
					break;
				if(canthrownode_tc(self, MESH__LLEG, &throw_nodes))
				{
					AngleVectors(self->s.angles,NULL,right,NULL);
					gore_spot[2]+=self->maxs[2]*0.3;
					VectorMA(gore_spot,-10,right,gore_spot);
					ThrowBodyPart(self, &gore_spot, throw_nodes, damage, FRAME_partfly);
				}
			}
			break;
		case hl_LegUpperRight:
		case hl_LegLowerRight://right leg
			if(self->health>0)
			{//still alive
				if(self->s.fmnodeinfo[MESH__RLEG].flags & FMNI_USE_SKIN)
					break;
				self->s.fmnodeinfo[MESH__RLEG].flags |= FMNI_USE_SKIN;
				self->s.fmnodeinfo[MESH__RLEG].skin = self->s.skinnum+1;
				break;
			}
			else
			{
				if(self->s.fmnodeinfo[MESH__RLEG].flags & FMNI_NO_DRAW)
					break;
				if(canthrownode_tc(self, MESH__RLEG, &throw_nodes))
				{
					AngleVectors(self->s.angles,NULL,right,NULL);
					gore_spot[2]+=self->maxs[2]*0.3;
					VectorMA(gore_spot,-10,right,gore_spot);
					ThrowBodyPart(self, &gore_spot, throw_nodes, damage, FRAME_partfly);
				}
			}
			break;

		default:
			if(flrand(0,self->health)<damage*0.25)
				insect_dropweapon (self, 0);
			break;
	}
	if(throw_nodes)
		self->pain_debounce_time = 0;

	/*if(self->mass == MASS_TC_FEMALE)
	{
		if(self->s.fmnodeinfo[MESH__STAFF].flags & FMNI_NO_DRAW)
			self->monsterinfo.aiflags |= AI_NO_MISSILE;
	}
	else*/
	if(self->mass == MASS_TC_MALE)
	{
//		if(self->s.fmnodeinfo[MESH__SWORD].flags & FMNI_NO_DRAW)
		if(self->s.fmnodeinfo[MESH__MALEHAND].flags & FMNI_NO_DRAW)
			self->monsterinfo.aiflags |= AI_NO_MELEE;

		if(self->s.fmnodeinfo[MESH__SPEAR].flags & FMNI_NO_DRAW)
			self->monsterinfo.aiflags |= AI_NO_MISSILE;
	}
}

void insect_random_pain_sound (edict_t *self)
{
	if(self->mass==MASS_TC_MALE)//irand(0,1))
		gi.sound(self, CHAN_VOICE, Sounds[SND_PAINM], 1, ATTN_NORM, 0);
	else
		gi.sound(self, CHAN_VOICE, Sounds[SND_PAINF], 1, ATTN_NORM, 0);
}

void insect_pain(edict_t *self, G_Message_t *msg)
{
	int				temp, damage;
	int				force_damage;
	
	ParseMsgParms(msg, "eeiii", &temp, &temp, &force_damage, &damage, &temp);

	if(!force_damage&&flrand(0,self->health)>damage)
		return;

	gi.RemoveEffects(&self->s, FX_I_EFFECTS);
	self->s.effects |= EF_DISABLE_EXTRA_FX;
	//sound
	insect_random_pain_sound(self);
	//remove spell effects
	self->monsterinfo.aiflags &= ~AI_OVERRIDE_GUIDE;
	if (force_damage || self->pain_debounce_time < level.time)
	{
		self->pain_debounce_time = level.time + 1;
		if(irand(0,1))
			SetAnim(self, ANIM_PAINA);
		else
			SetAnim(self, ANIM_PAINC);
	}
}

/*-------------------------------------------------------------------------
	insect_pause
-------------------------------------------------------------------------*/
void insect_pause (edict_t *self)
{
	vec3_t	v;
	float	len;

	if(self->spawnflags & MSF_INSECT_BEAST_FODDER)
	{
		edict_t	*found = NULL;

		if(found = G_Find(found, FOFS(classname), "monster_trial_beast"))
		{
			if(found->health > 0)
			{
				self->enemy = found;
				found->oldenemy = found->enemy;
				found->enemy = self;

				self->spawnflags &= ~MSF_INSECT_BEAST_FODDER;
				self->monsterinfo.aiflags &= ~AI_NO_MISSILE;
				self->missile_range = 3000;
				self->min_missile_range = 250;

				//self->monsterinfo.aiflags |= AI_NO_MELEE;
				//self->melee_range = -250;
				self->monsterinfo.aiflags &= ~AI_NO_MELEE;
				self->melee_range = 48;

				self->bypass_missile_chance = 0;
				self->s.fmnodeinfo[MESH__MALEHAND].flags &= ~FMNI_NO_DRAW;
			}
		}
	}

	if(self->monsterinfo.aiflags&AI_OVERRIDE_GUIDE)
	{
		if(self->groundentity)
			self->monsterinfo.aiflags &= ~AI_OVERRIDE_GUIDE;
		else
			return;
	}

	if(self->spawnflags & MSF_FIXED && self->curAnimID == ANIM_DELAY && self->enemy)
	{
		self->monsterinfo.searchType = SEARCH_COMMON;
		MG_FaceGoal(self, true);
	}

	self->mood_think(self);

	if (self->ai_mood == AI_MOOD_NORMAL)
	{
		FindTarget(self);

		if(self->enemy)
		{
			VectorSubtract (self->s.origin, self->enemy->s.origin, v);
			len = VectorLength (v);

			if ((len > 80) || (self->monsterinfo.aiflags & AI_FLEE))  // Far enough to run after
			{
				QPostMessage(self, MSG_RUN,PRI_DIRECTIVE, NULL);
			}
			else	// Close enough to Attack 
			{
				QPostMessage(self, MSG_MELEE, PRI_DIRECTIVE, NULL);
			}
		}
	}
	else
	{
		//gi.dprintf("plageElf_pause:	Using High Level Functionality\n");
		
		
		if(self->enemy)
		{
			if(self->enemy->classID == CID_TBEAST)
			{
				if(visible(self, self->enemy))
				{
					if(M_DistanceToTarget(self, self->enemy) > 250)
					{
						if(infront(self, self->enemy))
						{
							self->ai_mood = AI_MOOD_ATTACK;
							self->monsterinfo.flee_finished = -1;
							self->ai_mood_flags |= AI_MOOD_FLAG_MISSILE;
						}
						else
						{
							self->ai_mood = AI_MOOD_PURSUE;
							self->ai_mood_flags &= ~AI_MOOD_FLAG_DUMB_FLEE;
							self->monsterinfo.aiflags &= ~AI_FLEE;
							self->monsterinfo.flee_finished = -1;
						}
					}
				}
			}
		}

		switch (self->ai_mood)
		{
		case AI_MOOD_ATTACK:
			if(self->ai_mood_flags & AI_MOOD_FLAG_MISSILE)
				QPostMessage(self, MSG_MISSILE, PRI_DIRECTIVE, NULL);
			else
				QPostMessage(self, MSG_MELEE, PRI_DIRECTIVE, NULL);
			break;

		case AI_MOOD_PURSUE:
		case AI_MOOD_NAVIGATE:
		case AI_MOOD_FLEE:
			QPostMessage(self, MSG_RUN, PRI_DIRECTIVE, NULL);
			break;

		case AI_MOOD_STAND:
			QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
			break;

		case AI_MOOD_DELAY:
			SetAnim(self, ANIM_DELAY);
			break;

		
		case AI_MOOD_WANDER:
			QPostMessage(self, MSG_WALK, PRI_DIRECTIVE, NULL);
			break;
		
		case AI_MOOD_WALK:
			QPostMessage(self, MSG_WALK, PRI_DIRECTIVE, NULL);
			break;

		case AI_MOOD_BACKUP:
			QPostMessage(self, MSG_FALLBACK, PRI_DIRECTIVE, NULL);
			break;

		case AI_MOOD_JUMP:
			if(self->spawnflags&MSF_FIXED)
				SetAnim(self, ANIM_DELAY);
			else
				SetAnim(self, ANIM_FJUMP);
			break;

		default :
#ifdef _DEVEL
			gi.dprintf("insect: Unusable mood %d!\n", self->ai_mood);
#endif
			break;
		}
	}
}

void insect_check_mood (edict_t *self, G_Message_t *msg)
{
	ParseMsgParms(msg, "i", &self->ai_mood);

	insect_pause(self);
}

/*-------------------------------------------------------------------------
	insect_run
-------------------------------------------------------------------------*/
void insect_run(edict_t *self, G_Message_t *msg)
{
	//turn on lerpflags

	if (M_ValidTarget(self, self->enemy))
	{
		if(self->enemy->classID == CID_TBEAST && self->enemy->enemy != self)
		{
			if(M_DistanceToTarget(self, self->enemy) < 250)
			{
				self->monsterinfo.aiflags |= AI_FLEE;
				self->monsterinfo.flee_finished = level.time + 3;
			}
		}

		if(self->spawnflags&MSF_FIXED)
			SetAnim(self, ANIM_DELAY);
		else
			SetAnim(self, ANIM_RUN);
	}
	else
	{
		QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
	}
}



/*----------------------------------------------------------------------
  insect runorder - order the insect to choose an run animation
-----------------------------------------------------------------------*/
void insect_runorder(edict_t *self)
{
	QPostMessage(self, MSG_RUN, PRI_DIRECTIVE, NULL);
}


/*-------------------------------------------------------------------------
	insectsqueal
-------------------------------------------------------------------------*/
void insectsqueal (edict_t *self)
{
/*	if(irand(0, 1))
		gi.sound(self, CHAN_WEAPON, Sounds[SND_PAIN1], 1, ATTN_NORM, 0);
	else
		gi.sound(self, CHAN_WEAPON, Sounds[SND_PAIN2], 1, ATTN_NORM, 0);*/
}

/*-------------------------------------------------------------------------
	insect_stand
-------------------------------------------------------------------------*/
void insect_stand(edict_t *self, G_Message_t *msg)
{	
	if (self->ai_mood == AI_MOOD_DELAY)
		SetAnim(self, ANIM_DELAY);
	else
		SetAnim(self, ANIM_IDLE);	

	return;
}

/*-------------------------------------------------------------------------
	insect_walk
-------------------------------------------------------------------------*/
void insect_walk(edict_t *self, G_Message_t *msg)
{
	if(self->spawnflags&MSF_FIXED)
		SetAnim(self, ANIM_DELAY);
	else
		SetAnim(self, ANIM_WALK);	
}


/*void Monster_touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t	pos1, pos2;
	float	zdiff;

	if ((other->svflags & SVF_MONSTER) || (!stricmp(other->classname, "player")))
	{
		VectorCopy(other->s.origin, pos1);
		pos1[2] += other->mins[2];

		VectorCopy(self->s.origin, pos2);
		pos2[2] += self->maxs[2];

		zdiff = pos1[2] - pos2[2];

		if (zdiff >= 0 ) // On top
		{
			other->velocity[0] = flrand(75.0, 100.0);			
			other->velocity[1] = flrand(75.0, 100.0);
			other->velocity[2] += 101;
	
			if (irand(0,1))
				other->velocity[0] *= -1;

			if (irand(0,1))
				other->velocity[1] *= -1;

			self->velocity[0] = -(other->velocity[0]);
			self->velocity[1] = -(other->velocity[1]);
			self->velocity[2] += 100;

			other->groundentity = NULL;
			self->groundentity = NULL;
		}
	}
}*/

void insect_go_inair(edict_t *self)
{
	SetAnim(self, ANIM_INAIR);
}

void insect_go_finair(edict_t *self)
{
	SetAnim(self, ANIM_FINAIR);
}

void insectCheckLoop (edict_t *self, float frame)
{//see if should fire again
	vec3_t	v;
	float	len, melee_range, min_seperation, missile_range;

	if(!self->enemy)
		return;

	if(self->enemy->health <= 0)
		return;

	if(!visible(self, self->enemy))
		return;

	if(!infront(self, self->enemy))
		return;

	ai_charge2(self, 0);

	if(irand(0,100)<self->bypass_missile_chance)
		return;

	if(self->monsterinfo.attack_finished > level.time)
		return;

	VectorSubtract (self->s.origin, self->enemy->s.origin, v);
	len = VectorLength (v);
	melee_range = 64;
	missile_range = 384;
	min_seperation = self->maxs[0] + self->enemy->maxs[0];

	if (infront(self, self->enemy))
	{//don't loop if enemy close enough
		if (len < min_seperation + melee_range)
			return;
		else if (len < min_seperation + missile_range && irand(0,10)<3)
			return;
	}

	self->monsterinfo.currframeindex = (int)frame;
}

void insect_jump (edict_t *self, G_Message_t *msg)
{
	if(self->spawnflags&MSF_FIXED)
	{
		SetAnim(self, ANIM_DELAY);
		return;
	}
	SetAnim(self, ANIM_FORCED_JUMP);
	self->monsterinfo.aiflags |= AI_OVERRIDE_GUIDE;
}

/*-------------------------------------------------------------------------
	insectStaticsInit
-------------------------------------------------------------------------*/
void TcheckrikStaticsInit()
{
	classStatics[CID_TCHECKRIK].msgReceivers[MSG_STAND] = insect_stand;
	classStatics[CID_TCHECKRIK].msgReceivers[MSG_WALK] = insect_walk;
	classStatics[CID_TCHECKRIK].msgReceivers[MSG_RUN] = insect_run;
	classStatics[CID_TCHECKRIK].msgReceivers[MSG_MELEE] = insect_melee;
	classStatics[CID_TCHECKRIK].msgReceivers[MSG_MISSILE] = insect_missile;
	classStatics[CID_TCHECKRIK].msgReceivers[MSG_PAIN] = insect_pain;
	classStatics[CID_TCHECKRIK].msgReceivers[MSG_DEATH] = insect_death;
	classStatics[CID_TCHECKRIK].msgReceivers[MSG_FALLBACK] = insect_backpedal;
	classStatics[CID_TCHECKRIK].msgReceivers[MSG_DISMEMBER] = MG_parse_dismember_msg;
	classStatics[CID_TCHECKRIK].msgReceivers[MSG_JUMP] = insect_jump;
	classStatics[CID_TCHECKRIK].msgReceivers[MSG_DEATH_PAIN] = insect_dead_pain;
	classStatics[CID_TCHECKRIK].msgReceivers[MSG_CHECK_MOOD] = insect_check_mood;

	// Cinematics
	classStatics[CID_TCHECKRIK].msgReceivers[MSG_C_ACTION1] = insect_c_anims;
	classStatics[CID_TCHECKRIK].msgReceivers[MSG_C_ACTION2] = insect_c_anims;
	classStatics[CID_TCHECKRIK].msgReceivers[MSG_C_ACTION3] = insect_c_anims;
	classStatics[CID_TCHECKRIK].msgReceivers[MSG_C_ACTION4] = insect_c_anims;
	classStatics[CID_TCHECKRIK].msgReceivers[MSG_C_ATTACK1] = insect_c_anims;
	classStatics[CID_TCHECKRIK].msgReceivers[MSG_C_ATTACK2] = insect_c_anims;
	classStatics[CID_TCHECKRIK].msgReceivers[MSG_C_ATTACK3] = insect_c_anims;
	classStatics[CID_TCHECKRIK].msgReceivers[MSG_C_BACKPEDAL1] = insect_c_anims;
	classStatics[CID_TCHECKRIK].msgReceivers[MSG_C_DEATH1] = insect_c_anims;
	classStatics[CID_TCHECKRIK].msgReceivers[MSG_C_IDLE1] = insect_c_anims;
	classStatics[CID_TCHECKRIK].msgReceivers[MSG_C_IDLE2] = insect_c_anims;
	classStatics[CID_TCHECKRIK].msgReceivers[MSG_C_IDLE3] = insect_c_anims;
	classStatics[CID_TCHECKRIK].msgReceivers[MSG_C_PAIN1] = insect_c_anims;
	classStatics[CID_TCHECKRIK].msgReceivers[MSG_C_RUN1] = insect_c_anims;
	classStatics[CID_TCHECKRIK].msgReceivers[MSG_C_WALK1] = insect_c_anims;


	resInfo.numAnims = NUM_ANIMS;
	resInfo.animations = animations;
	
	//note that the name is different in the path
	resInfo.modelIndex = gi.modelindex("models/monsters/tcheckrik/male/tris.fm");

	Sounds[SND_PAINM]=gi.soundindex("monsters/insect/painm.wav");
	Sounds[SND_PAINF]=gi.soundindex("monsters/insect/painf.wav");	
	Sounds[SND_DIEM]=gi.soundindex("monsters/insect/deathm.wav");	
	Sounds[SND_DIEF]=gi.soundindex("monsters/insect/deathf.wav");	
	Sounds[SND_GIB]=gi.soundindex("monsters/insect/gib.wav");
	Sounds[SND_SWIPE]=gi.soundindex("monsters/insect/swipe.wav");	
	Sounds[SND_SWIPEHITF]=gi.soundindex("monsters/plagueelf/hookhit.wav");	
	Sounds[SND_SWIPEHITW]=gi.soundindex("monsters/insect/swipehitw.wav");	
	Sounds[SND_SPELLM]=gi.soundindex("monsters/insect/spellm.wav");	
	Sounds[SND_SPELLM2]=gi.soundindex("monsters/insect/spellm2.wav");	
	Sounds[SND_SPLPWRUPF]=gi.soundindex("monsters/insect/splpwrupf.wav");	
	Sounds[SND_SPELLF]=gi.soundindex("monsters/insect/spellf.wav");	
	Sounds[SND_GROWLM1]=gi.soundindex("monsters/insect/growlm1.wav");	
	Sounds[SND_GROWLM2] = gi.soundindex ("monsters/insect/growlm2.wav");	
	Sounds[SND_GROWLF1]=gi.soundindex("monsters/insect/growlf1.wav");	
	Sounds[SND_GROWLF2] = gi.soundindex ("monsters/insect/growlf2.wav");	
	Sounds[SND_THUD] = gi.soundindex ("monsters/insect/thud.wav");	

	resInfo.numSounds = NUM_SOUNDS;
	resInfo.sounds = Sounds;

	classStatics[CID_TCHECKRIK].resInfo = &resInfo;
}

/*QUAKED monster_tcheckrik_male (1 .5 0) (-16 -16 -32) (16 16 32) AMBUSH ASLEEP WALKING CINEMATIC BEAST_FODDER YELLOWJACKET 64 FIXED WANDER LEAD STALK COWARD EXTRA1 EXTRA2 EXTRA3 EXTRA4

The insect 

AMBUSH - Will not be woken up by other monsters or shots from player

ASLEEP - will not appear until triggered

WALKING - use WANDER instead

BEAST_FODDER - Will run away if a Trial Beast is present

YELLOWJACKET - uses black and yellow skin, shoots a spread of three yellow stinger projectiles

WANDER - Monster will wander around aimlessly (but follows buoys)

LEAD - Monster will tryto cut you off when you're running and fighting him, works well if there are a few monsters in a group, half doing this, half not

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
		melee_range = -10self->classID == CID_MOTHER0 - monster will never do a melee attack and will back away (if it has that ability) when player gets too close

missile_range - Maximum distance the player can be from the monster to be allowed to use it's ranged attack.

min_missile_range - Minimum distance the player can be from the monster to be allowed to use it's ranged attack.

bypass_missile_chance - Chance that a monster will NOT fire it's ranged attack, even when it has a clear shot.  This, in effect, will make the monster come in more often than hang back and fire.  A percentage (0 = always fire/never close in, 100 = never fire/always close in).- must be whole number

jump_chance - every time the monster has the opportunity to jump, what is the chance (out of 100) that he will... (100 = jump every time)- must be whole number

wakeup_distance - How far (max) the player can be away from the monster before it wakes up.  This just means that if the monster can see the player, at what distance should the monster actually notice him and go for him.

DEFAULTS:
mintel					= 32
melee_range				= 30
missile_range			= 512
min_missile_range		= 48
bypass_missile_chance	= 30
jump_chance				= 40
wakeup_distance			= 1024
*/
/*-------------------------------------------------------------------------
	SP_monster_insect
-------------------------------------------------------------------------*/
void SP_monster_tcheckrik_male (edict_t *self)
{
	if(irand(0, 1))//made random again
		self->spawnflags |= MSF_INSECT_YELLOWJACKET;

	if(self->spawnflags & MSF_WALKING)
	{
		self->spawnflags |= MSF_WANDER;
		self->spawnflags &= ~MSF_WALKING;
	}

	if (!monster_start(self))
		return;					// Failed initialization
		
	self->msgHandler = DefaultMsgHandler;
	self->classID = CID_TCHECKRIK;
	self->think = walkmonster_start_go;
	self->monsterinfo.dismember = insect_dismember;

	if (!self->health)
	{
		self->health = TC_HEALTH_MALE;
	}
	self->max_health = self->health = MonsterHealth(self->health);

	self->mass = MASS_TC_MALE;
	self->yaw_speed = 20;

	self->movetype = PHYSICSTYPE_STEP;
	VectorClear(self->knockbackvel);
	
	self->solid=SOLID_BBOX;

	VectorCopy(STDMinsForClass[self->classID], self->mins);
	VectorCopy(STDMaxsForClass[self->classID], self->maxs);	
//	VectorSet (self->mins, -16, -16, -32);
//	VectorSet (self->maxs, 16, 16, 32);	
	self->viewheight = self->maxs[2]*0.8;
	
	self->touch = M_Touch;

	self->s.modelindex = gi.modelindex("models/monsters/tcheckrik/male/tris.fm");

	// All skins are even numbers, pain skins are skin+1.
	if(self->spawnflags & MSF_INSECT_YELLOWJACKET)
		self->s.skinnum = 2;
	else
		self->s.skinnum = 0;

	if (!self->s.scale)
	{
		self->s.scale = self->monsterinfo.scale = MODEL_SCALE;
	}

	self->materialtype = MAT_INSECT;

	//turn on/off the weapons that aren't used
	self->s.fmnodeinfo[MESH__CROWN].flags |= FMNI_NO_DRAW;
	self->s.fmnodeinfo[MESH__RWINGS].flags |= FMNI_NO_DRAW;
	self->s.fmnodeinfo[MESH__LWINGS].flags |= FMNI_NO_DRAW;
	self->s.fmnodeinfo[MESH__STAFF].flags |= FMNI_NO_DRAW;
	self->s.fmnodeinfo[MESH__GEM].flags |= FMNI_NO_DRAW;
	self->s.fmnodeinfo[MESH__FEMHAND].flags |= FMNI_NO_DRAW;
	self->s.fmnodeinfo[MESH__CROWN].flags |= FMNI_NO_DRAW;

	if(self->spawnflags & MSF_INSECT_BEAST_FODDER)
		self->s.fmnodeinfo[MESH__MALEHAND].flags |= FMNI_NO_DRAW;//?
	
	//FIXME (somewhere: otherenemy should be more than just *one* kind
	self->monsterinfo.otherenemyname = "monster_rat";


	//override female's settings in m_stats.c
	if(!self->bypass_missile_chance)
		self->bypass_missile_chance = 30;

	if(!self->melee_range)
		self->melee_range = 48;
	//set up my mood function
	MG_InitMoods(self);
	if(!irand(0,2))
		self->ai_mood_flags |= AI_MOOD_FLAG_PREDICT;

	if(self->spawnflags & MSF_WANDER)
	{
		QPostMessage(self, MSG_WALK, PRI_DIRECTIVE, NULL);
	}
	else if(self->spawnflags & MSF_INSECT_CINEMATIC)
	{
		self->monsterinfo.c_mode = 1;
		QPostMessage(self, MSG_C_IDLE1, PRI_DIRECTIVE, "iiige",0,0,0,NULL,NULL);
	}
	else
	{
		QPostMessage(self,MSG_STAND,PRI_DIRECTIVE, NULL);
	}

	gi.CreateEffect(&self->s,
					FX_I_EFFECTS,
					CEF_OWNERS_ORIGIN,
					vec3_origin,
					"bv",
					FX_I_RREFS,
					vec3_origin);
}

/*QUAKED monster_tcheckrik_female (1 .5 0) (-16 -16 -32) (16 16 32) AMBUSH ASLEEP WALKING CINEMATIC 16 ALTERNATE 64 FIXED WANDER MELEE_LEAD STALK COWARD EXTRA1 EXTRA2 EXTRA3 EXTRA4

The insect 

AMBUSH - Will not be woken up by other monsters or shots from player

ASLEEP - will not appear until triggered

WALKING - use WANDER instead

WANDER - Monster will wander around aimlessly (but follows buoys)

ALTERNATE - fires alternate projectile, more damage, not multiple though

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
mintel					= 32
melee_range				= -72
missile_range			= 512
min_missile_range		= 48
bypass_missile_chance	= 0
jump_chance				= 40
wakeup_distance			= 1024

NOTE: A value of zero will result in defaults, if you actually want zero as the value, use -1
*/
/*-------------------------------------------------------------------------
	SP_monster_insect
-------------------------------------------------------------------------*/
void SP_monster_tcheckrik_female (edict_t *self)
{
	if(self->spawnflags & MSF_WALKING)
	{
		self->spawnflags |= MSF_WANDER;
		self->spawnflags &= ~MSF_WALKING;
	}

	if (!monster_start(self))
		return;					// Failed initialization
		
	self->msgHandler = DefaultMsgHandler;
	self->classID = CID_TCHECKRIK;
	self->think = walkmonster_start_go;
	self->monsterinfo.dismember = insect_dismember;

	if (!self->health)
	{
		self->health = TC_HEALTH_FEMALE;
	}
	self->max_health = self->health = MonsterHealth(self->health);

	self->mass = MASS_TC_FEMALE;
	self->yaw_speed = 20;

	self->movetype = PHYSICSTYPE_STEP;
	VectorClear(self->knockbackvel);
	
	self->solid=SOLID_BBOX;

	VectorCopy(STDMinsForClass[self->classID], self->mins);
	VectorCopy(STDMaxsForClass[self->classID], self->maxs);	
	self->viewheight = self->maxs[2]*0.4;
	
	self->touch = M_Touch;

	self->s.modelindex = gi.modelindex("models/monsters/tcheckrik/female/tris.fm");

	// All skins are even numbers, pain skins are skin+1.
	self->s.skinnum = 0;	
//	self->s.skinnum = 2;

	if (!self->s.scale)
	{
		self->s.scale = self->monsterinfo.scale = MODEL_SCALE;
	}

	self->materialtype = MAT_INSECT;

	//turn on/off the weapons that aren't used
	self->s.fmnodeinfo[MESH__LMANDIBLE].flags |= FMNI_NO_DRAW;
	self->s.fmnodeinfo[MESH__RMANDIBLE].flags |= FMNI_NO_DRAW;
	self->s.fmnodeinfo[MESH__L2NDARM].flags |= FMNI_NO_DRAW;
	self->s.fmnodeinfo[MESH__R2NDARM].flags |= FMNI_NO_DRAW;
	self->s.fmnodeinfo[MESH__MALEHAND].flags |= FMNI_NO_DRAW;//?
//	self->s.fmnodeinfo[MESH__SWORD].flags |= FMNI_NO_DRAW;//?
	self->s.fmnodeinfo[MESH__SPEAR].flags |= FMNI_NO_DRAW;//?
	
//	self->s.fmnodeinfo[MESH__GEM].flags |= FMNI_NO_DRAW;
	
	//FIXME (somewhere: otherenemy should be more than just *one* kind
	self->monsterinfo.otherenemyname = "monster_rat";

	//set up my mood function
	MG_InitMoods(self);
	if(!irand(0,2))
		self->ai_mood_flags |= AI_MOOD_FLAG_PREDICT;

	if(self->spawnflags & MSF_WANDER)
	{
		QPostMessage(self, MSG_WALK, PRI_DIRECTIVE, NULL);
	}
	else
	{
		QPostMessage(self,MSG_STAND,PRI_DIRECTIVE, NULL);
	}

	self->monsterinfo.aiflags |= AI_NO_MELEE;

	gi.CreateEffect(&self->s,
					FX_I_EFFECTS,
					CEF_OWNERS_ORIGIN,
					vec3_origin,
					"bv",
					FX_I_RREFS,
					vec3_origin);
	
	self->svflags |= SVF_WAIT_NOTSOLID;

	if(self->spawnflags & MSF_INSECT_ALTERNATE)
	{
		self->s.color.a = 255;
		self->s.color.r = 250;
		self->s.color.g = 150;
		self->s.color.b = 100;
	}
}
