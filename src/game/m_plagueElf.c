//==============================================================================
//
// m_plagueElf.c
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

#include "m_plagueElf.h"
#include "m_plagueElf_anim.h"
#include "g_HitLocation.h"
#include "g_misc.h"
#include "angles.h"
#include "c_ai.h"
#include "m_stats.h"


/*----------------------------------------------------------------------
  plagueElf Base Info
-----------------------------------------------------------------------*/
static animmove_t *animations[ NUM_ANIMS] =
{
	&plagueElf_move_stand1,
	&plagueElf_move_walk1,
	&plagueElf_move_walk2,
	&plagueElf_move_run1,	
	&plagueElf_move_runatk1,
	&plagueElf_move_fjump,
	&plagueElf_move_inair,
	&plagueElf_move_land,
	&plagueElf_move_melee1,
	&plagueElf_move_melee2,
	&plagueElf_move_death1,
	&plagueElf_move_death2,
	&plagueElf_move_death3,
	&plagueElf_move_death4,
	&plagueElf_fist1,
	&plagueElf_lean1, 
	&plagueElf_shake1,
	&plagueElf_move_pain1,
	&plagueElf_delay,
	&plagueElf_move_missile,
//
	&plagueElf_move_kdeath_go,
	&plagueElf_move_kdeath_loop,
	&plagueElf_move_kdeath_end,

	&plagueElf_crazy_A,
	&plagueElf_crazy_B,
	&plagueElf_cursing,
	&plagueElf_point,
	&plagueElf_scared,
	
	// Cinematics
	&plagueElf_move_c_idle1,
	&plagueElf_move_c_idle2,
	&plagueElf_move_c_idle3,
	&plagueElf_move_c_walk,
	&plagueElf_move_c_walk2,
	&plagueElf_move_c_run,
	&plagueElf_move_c_attack1,
	&plagueElf_move_c_attack2,
	&plagueElf_move_c_attack3,
	&plagueElf_move_c_attack4,
	&plagueElf_move_c_pain1,
	&plagueElf_move_c_death1,
	&plagueElf_move_c_death2,
	&plagueElf_move_c_death3,
	&plagueElf_move_c_death4,
	&plagueElf_move_run1,

//	&plagueElf_move_stand1,

};

static int sounds[NUM_SOUNDS];
static ClassResourceInfo_t resInfo;

int Bit_for_MeshNode_pe [12] =
{
	BIT_BASE,
	BIT_HANDLE,
	BIT_HOE,
	BIT_GAFF,
	BIT_HAMMER,
	BIT_BODY,
	BIT_L_LEG,
	BIT_R_LEG,	
	BIT_R_ARM,
	BIT_L_ARM,
	BIT_HEAD
};

float	pelf_VoiceTimes[] =
{
	0.0,	//FIRST_SIGHT_GROUP
	1.0,	//VOICE_SIGHT_AFTER_HIM1
	0.6,	//VOICE_SIGHT_AFTER_HIM2
	1.3,	//VOICE_SIGHT_CUT_HIM2
	1.0,	//VOICE_SIGHT_CUT_HIM1
	1.2,	//VOICE_SIGHT_EAT_FLESH1
	1.2,	//VOICE_SIGHT_EAT_FLESH2
	0.9,	//VOICE_SIGHT_GET_HIM1
	0.9,	//VOICE_SIGHT_GET_HIM2
	0.9,	//VOICE_SIGHT_GET_HIM3
	1.0,	//VOICE_SIGHT_KILL_HIM1
	0.9,	//VOICE_SIGHT_KILL_HIM2
	1.3,	//VOICE_SIGHT_KILL_HIM3
	1.2,	//VOICE_SIGHT_OVER_THERE
	1.2,	//VOICE_SIGHT_THERES_ONE
	0.7,	//VOICE_SUPPORT_FOLLOW_ME
	1.5,	//VOICE_SUPPORT_CURE
	1.6,	//VOICE_SUPPORT_LIVER
	2.0,	//VOICE_SUPPORT_SLASH
	1.1,	//VOICE_SUPPORT_SURROUND_HIM
	1.8,	//VOICE_SUPPORT_UNAFFECTED1
	2.0,	//VOICE_SUPPORT_UNAFFECTED2
	1.3,	//VOICE_SUPPORT_YEAH_GET_HIM1
	1.0,	//VOICE_SUPPORT_YEAH_GET_HIM2
	0.0,	//VOICE_FIRST_ALONE
	1.1,	//VOICE_MISC_DIE
	1.3,	//VOICE_MISC_FLESH
	1.1,	//VOICE_SUPPORT_GONNA_DIE1
	1.5,	//VOICE_SUPPORT_GONNA_DIE2
	1.5,	//VOICE_SUPPORT_GONNA_DIE3
	1.2,	//VOICE_SUPPORT_GONNA_DIE4
	2.0,	//VOICE_MISC_MUST_KILL
	1.1,	//VOICE_SUPPORT_MUST_DIE
	1.1,	//VOICE_SUPPORT_YES
	0.0,	//VOICE_LAST_GROUP
	1.6,	//VOICE_MISC_GET_AWAY1
	0.9,	//VOICE_MISC_GET_AWAY2
	1.2,	//VOICE_MISC_GO_AWAY
	0.6,	//VOICE_MISC_HELP_ME1
	2.3,	//VOICE_MISC_HELP_ME2
	2.2,	//VOICE_MISC_HELP_ME3
	1.5,	//VOICE_MISC_LEAVE_ME1
	1.0,	//VOICE_MISC_LEAVE_ME2
	0.6,	//VOICE_MISC_NO
	0.9,	//VOICE_MISC_TRAPPED
	0.8,	//VOICE_MISC_COME_BACK1
	1.0,	//VOICE_MISC_COME_BACK2
	0.9,	//VOICE_MISC_DONT_HURT
};


void dying_elf_sounds (edict_t *self, int type);
/*----------------------------------------------------------------------
  Cinematic Functions for the monster
-----------------------------------------------------------------------*/

void plagueElf_c_gib(edict_t *self, G_Message_t *msg)
{
	gi.sound(self, CHAN_BODY, sounds[SND_GIB], 1, ATTN_NORM, 0);
	self->think = BecomeDebris;
	self->nextthink = level.time + 0.1;
}

/*-------------------------------------------------------------------------
	plagueElf_c_anims
-------------------------------------------------------------------------*/
void plagueElf_c_anims(edict_t *self, G_Message_t *msg)
{
	int int_msg;
	int curr_anim;

	ai_c_readmessage(self, msg);
	int_msg = (int) msg->ID;

	self->monsterinfo.c_anim_flag = 0; 

	switch(int_msg)
	{
		case MSG_C_ATTACK1:
			self->monsterinfo.c_anim_flag |= C_ANIM_MOVE;
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
		case MSG_C_ATTACK4:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_ATTACK4;
			break;
		case MSG_C_DEATH1:
			self->monsterinfo.c_anim_flag |= C_ANIM_DONE;
			curr_anim = ANIM_C_DEATH1;
			break;
		case MSG_C_DEATH2:
			self->monsterinfo.c_anim_flag |= C_ANIM_DONE;
			curr_anim = ANIM_C_DEATH2;
			break;
		case MSG_C_DEATH3:
			self->monsterinfo.c_anim_flag |= C_ANIM_DONE;
			curr_anim = ANIM_C_DEATH3;
			break;
		case MSG_C_DEATH4:
			self->monsterinfo.c_anim_flag |= C_ANIM_DONE;
			curr_anim = ANIM_C_DEATH4;
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
		case MSG_C_THINKAGAIN:			// Think for yourself, elf.
			self->monsterinfo.c_mode = 0;
			self->enemy = self->monsterinfo.c_ent;
			FoundTarget(self, true);
//			self->takedamage = DAMAGE_YES;
			curr_anim = ANIM_C_THINKAGAIN;
			break;
		case MSG_C_WALK1:
			self->monsterinfo.c_anim_flag |= C_ANIM_MOVE;
			curr_anim = ANIM_C_WALK1;
			break;
		case MSG_C_WALK2:
			self->monsterinfo.c_anim_flag |= C_ANIM_MOVE;
			curr_anim = ANIM_C_WALK2;
			break;
		default:
			break;
	} 

	SetAnim(self, curr_anim);
}




/*-------------------------------------------------------------------------
	plagueelf_death_loop 
-------------------------------------------------------------------------*/

void plagueelf_death_loop ( edict_t *self )
{
	SetAnim(self, ANIM_KDEATH_LOOP);
}

/*-------------------------------------------------------------------------
	plagueelf_check_land
-------------------------------------------------------------------------*/

void plagueelf_check_land ( edict_t *self )
{
	vec3_t	endpos;
	trace_t	trace;

	if(self->s.frame == FRAME_death7)
		MG_NoBlocking(self);

	M_ChangeYaw(self);

	VectorCopy(self->s.origin, endpos);
	endpos[2] -= 48;

	gi.trace(self->s.origin, self->mins, self->maxs, endpos, self, MASK_MONSTERSOLID,&trace);

	if ( ( trace.fraction < 1 || trace.allsolid || trace.startsolid ) && self->curAnimID != ANIM_KDEATH_END && self->curAnimID != ANIM_KDEATH_GO)
	{
		self->elasticity = 1.25;
		self->friction = 0.5;
		SetAnim(self, ANIM_KDEATH_END);
	}
}

/*-------------------------------------------------------------------------
	plagueElf_strike
-------------------------------------------------------------------------*/
void plagueElf_strike (edict_t *self)
{
	trace_t		trace;
	edict_t		*victim;
	vec3_t		soff, eoff, mins, maxs, bloodDir, direction;
	int			damage;

	//FIXME: Account for weapon being knocked out of hand?
	if(self->s.fmnodeinfo[MESH__HANDLE].flags & FMNI_NO_DRAW)
		return;

	switch ( self->curAnimID )
	{
	case ANIM_MELEE1:
		VectorSet(soff, -8, 0, 32);
		VectorSet(eoff, 36, 8, 16);
		break;

	case ANIM_MELEE2:
		VectorSet(soff, -8, -16, 32);
		VectorSet(eoff, 36, 0, 0);
		break;

	case ANIM_RUNATK1:
		VectorSet(soff, 2, -4, 24);
		VectorSet(eoff, 50, 4, 4);
		break;
	}
	
	VectorSet(mins, -4, -4, -4);
	VectorSet(maxs,  4,  4,  4);

	VectorSubtract(soff, eoff, bloodDir);
	VectorNormalize(bloodDir);

	victim = M_CheckMeleeLineHit(self, soff, eoff, mins, maxs, &trace, direction);	

	if (victim)
	{
		if (victim == self)
		{
			//Create a spark effect
			gi.CreateEffect(NULL, FX_SPARKS, 0, trace.endpos, "d", bloodDir);
		}
		else
		{
			if (!(self->s.fmnodeinfo[MESH__GAFF].flags & FMNI_NO_DRAW) || !(self->s.fmnodeinfo[MESH__HOE].flags & FMNI_NO_DRAW))
			{//it's the hoe or the hook
				gi.sound (self, CHAN_WEAPON, sounds[SND_ATTACKHIT1], 1, ATTN_NORM, 0);
			}
			else //it's the hammer or handle
				gi.sound (self, CHAN_WEAPON, sounds[SND_ATTACKHIT2], 1, ATTN_NORM, 0);	

			damage = irand(PLAGUEELF_DMG_MIN, PLAGUEELF_DMG_MAX);

			if(!(self->s.fmnodeinfo[MESH__HOE].flags & FMNI_NO_DRAW))
				damage+=PLAGUEELF_DMG_HOE;
			else if(!(self->s.fmnodeinfo[MESH__GAFF].flags & FMNI_NO_DRAW))
				damage+=PLAGUEELF_DMG_GAFF;
			else if(self->s.fmnodeinfo[MESH__HAMMER].flags & FMNI_NO_DRAW)
				damage+=PLAGUEELF_DMG_HAMMER;

			//Hurt whatever we were whacking away at
			T_Damage(victim, self, self, direction, trace.endpos, bloodDir, damage, damage*2, DAMAGE_DISMEMBER,MOD_DIED);
		}
	}
	else
	{
		//Play a swish sound
		gi.sound (self, CHAN_WEAPON, sounds[SND_ATTACKMISS1], 1, ATTN_NORM, 0);
	}
}

/*-------------------------------------------------------------------------
	plagueElf_death
-------------------------------------------------------------------------*/
void plagueElf_death(edict_t *self, G_Message_t *msg)
{
	edict_t	*targ, *inflictor, *attacker;
	float	damage;
	vec3_t	dVel, vf, yf;
	int chance;

	pelf_init_phase_in(self);

	ParseMsgParms(msg, "eeei", &targ, &inflictor, &attacker, &damage);
	
	// Big enough death to be thrown back
	if(self->monsterinfo.aiflags&AI_DONT_THINK)
	{
		if (irand(0,10) < 5)
			SetAnim(self, ANIM_DIE2);
		else 
			SetAnim(self, ANIM_DIE1);
		return;
	}

	self->msgHandler = DeadMsgHandler;

	//Dead but still being hit	
	if(self->deadflag == DEAD_DEAD) 
		return;

	self->deadflag = DEAD_DEAD;

	plagueElf_dropweapon (self, (int)self->health*-1);

	if (self->health <= -80)//gib death
	{
		gi.sound(self, CHAN_BODY, sounds[SND_GIB], 1, ATTN_NORM, 0);
		BecomeDebris(self);
		return;
	}
	else if (self->health < -10)
	{
		self->svflags |= SVF_DEADMONSTER;
		SetAnim(self, ANIM_KDEATH_GO);

		VectorCopy(targ->velocity, vf);
		VectorNormalize(vf);

		VectorScale(vf, -1, yf);

		self->ideal_yaw = vectoyaw( yf );
		self->yaw_speed = 48;

		VectorScale(vf, 250, dVel);
		dVel[2] = irand(150,200);

		VectorCopy(dVel, self->velocity);
//		self->groundentity = NULL;
		return;
	}
	
	//regular death
	if(self->count == 0)
	{
		chance = irand(0,3);
		if(chance == 0)
			SetAnim(self, ANIM_DIE1);
		else if(chance == 1)
			SetAnim(self, ANIM_DIE2);
		else if(chance == 2)
			SetAnim(self, ANIM_DIE3);
		else 
			SetAnim(self, ANIM_DIE4);
	}
	else
	{
		if(self->count == 1)
			SetAnim(self, ANIM_DIE1);
		else if(self->count == 2)
			SetAnim(self, ANIM_DIE2);
		else if(self->count == 3)
			SetAnim(self, ANIM_DIE3);
		else 
			SetAnim(self, ANIM_DIE4);
	}
}


/*-------------------------------------------------------------------------
	plagueElfdeathsqueal
-------------------------------------------------------------------------*/
void plagueElfdeathsqueal (edict_t *self)
{
	int sound;

	sound = irand(SND_DIE1, SND_DIE3);

	gi.sound(self, CHAN_VOICE, sounds[sound], 1, ATTN_NORM, 0);
	return;
}

/*-------------------------------------------------------------------------
	plagueElfgrowl
-------------------------------------------------------------------------*/
void plagueElfgrowl (edict_t *self)
{
	int chance;	

	chance = irand(0, 10);

	if (chance <= 2 )
	{
		chance = irand(0, 12);
		if (chance < 3)
			return;
			//gi.sound (self, CHAN_WEAPON, sounds[SND_GASP], 1, ATTN_IDLE, 0);
		else if ( chance < 6)
			gi.sound (self, CHAN_VOICE, sounds[SND_PANT], 1, ATTN_IDLE, 0);
		else if (chance < 9)
			gi.sound(self, CHAN_VOICE, sounds[SND_MOAN1], 1, ATTN_IDLE, 0);
		else 
			gi.sound(self, CHAN_VOICE, sounds[SND_MOAN2], 1, ATTN_IDLE, 0);
	}
}

void plagueElfattack(edict_t *self)
{
	int chance, sound;	

	chance = irand(0, 10);

	if (chance < 5)
	{
		sound =  irand(SND_ATTACK1, SND_ATTACK2);
		gi.sound (self, CHAN_VOICE, sounds[sound], 1, ATTN_IDLE, 0);
	}
}

void create_pe_spell(edict_t *Spell)
{
	Spell->movetype=MOVETYPE_FLYMISSILE;
	Spell->solid=SOLID_BBOX;
	Spell->classname="plagueElf_Spell";
	Spell->touch=plagueElfSpellTouch;
	Spell->enemy=NULL;
	Spell->clipmask=MASK_MONSTERSOLID|MASK_PLAYERSOLID|MASK_SHOT;
	Spell->s.scale = 0.5;
	Spell->s.effects |= EF_MARCUS_FLAG1|EF_CAMERA_NO_CLIP;
	Spell->svflags |= SVF_ALWAYS_SEND;
}

// the spell needs to bounce
void make_pe_spell_reflect(edict_t *self, edict_t *spell)
{
	create_pe_spell(spell);
	spell->s.modelindex = self->s.modelindex;
	VectorCopy(self->s.origin, spell->s.origin);
	spell->owner = self->owner;
	spell->enemy = self->enemy;
	spell->touch=self->touch;
	spell->nextthink=self->nextthink;
	spell->think=G_FreeEdict;
	spell->health = self->health;
	spell->red_rain_count = self->red_rain_count;

	Create_rand_relect_vect(self->velocity, spell->velocity);

	vectoangles(spell->velocity, spell->s.angles);
	spell->s.angles[YAW]+=90;

	Vec3ScaleAssign(500, spell->velocity);

	G_LinkMissile(spell);
}

void plagueElfSpellTouch (edict_t *self, edict_t *Other, cplane_t *Plane, csurface_t *Surface)
{
	vec3_t	normal;
	edict_t	*Spell;

	if(Surface&&(Surface->flags&SURF_SKY))
	{
		SkyFly(self);
		return;
	}

	if(EntReflecting(Other, true, true))
	{
		Spell = G_Spawn();

		make_pe_spell_reflect(self,Spell);

		gi.CreateEffect(&Spell->s,
			FX_PE_SPELL,
			CEF_OWNERS_ORIGIN,
			NULL, 
			"bv",
			self->red_rain_count,
			Spell->velocity);

		G_SetToFree(self);

		return;
	}

	VectorNormalize(self->velocity);
	switch(self->red_rain_count)
	{
	case FX_PE_MAKE_SPELL:
		gi.CreateEffect(NULL,
			FX_PE_SPELL,
			0,
			self->s.origin, 
			"bv",
			FX_PE_EXPLODE_SPELL,
			self->velocity);
		self->dmg = irand(PLAGUEELF_DMG_SPELL_MIN, PLAGUEELF_DMG_SPELL_MAX) + skill->value;
		break;
	case FX_PE_MAKE_SPELL2:
		gi.CreateEffect(NULL,
			FX_PE_SPELL,
			0,
			self->s.origin, 
			"bv",
			FX_PE_EXPLODE_SPELL2,
			self->velocity);
		self->dmg = irand(PLAGUEELF_GUARD_DMG_SPELL_MIN, PLAGUEELF_GUARD_DMG_SPELL_MAX);
		break;
	case FX_PE_MAKE_SPELL3:
		gi.CreateEffect(NULL,
			FX_PE_SPELL,
			0,
			self->s.origin, 
			"bv",
			FX_PE_EXPLODE_SPELL3,
			self->velocity);
		self->dmg = irand(PLAGUEELF_GUARD_DMG_SPELL_MIN+2, PLAGUEELF_GUARD_DMG_SPELL_MAX+2);
		self->dmg_radius = 40;
		break;
	}

	if(Other->takedamage)
	{
		VectorSet(normal, 0, 0, 1);
		if(Plane)
		{
			if(Plane->normal)
			{
				VectorCopy(Plane->normal, normal);
			}
		}
		T_Damage(Other,self,self->owner,self->movedir,self->s.origin,normal,self->dmg,0,DAMAGE_SPELL,MOD_DIED);
	}

	if(self->dmg_radius)
		T_DamageRadius(self, self->owner, self->owner, self->dmg_radius, self->dmg, 0, DAMAGE_SPELL,MOD_DIED);

	VectorClear(self->velocity);

	G_FreeEdict(self);
}

void plagueElf_spell(edict_t *self)
{//fixme; adjust for up/down
	vec3_t	Forward, right, firedir;
	edict_t	*Spell;
	
	if (M_ValidTarget(self, self->enemy))
	{
		if(self->s.fmnodeinfo[MESH__R_ARM].flags&FMNI_NO_DRAW)
			return;

//		gi.sound(self, CHAN_WEAPON, Sounds[SND_SPELL], 1, ATTN_NORM, 0);
		self->monsterinfo.attack_finished = level.time + 0.4;
		Spell = G_Spawn();

		create_pe_spell(Spell);

		Spell->touch=plagueElfSpellTouch;

		Spell->owner=self;
		Spell->enemy=self->enemy;
		
		Spell->health = 0; // tell the touch function what kind of Spell we are;

		AngleVectors(self->s.angles, Forward, right, NULL);
		VectorCopy(self->s.origin,Spell->s.origin);	
		VectorMA(Spell->s.origin, 4, Forward, Spell->s.origin);
		VectorMA(Spell->s.origin, 8, right, Spell->s.origin);
		Spell->s.origin[2] += 12;

		VectorCopy(self->movedir,Spell->movedir);
		vectoangles(Forward,Spell->s.angles);

		VectorSubtract(self->enemy->s.origin, Spell->s.origin, firedir);
		VectorNormalize(firedir);
		Forward[2] = firedir[2];
		VectorNormalize(Forward);
		VectorScale(Forward, 400, Spell->velocity);

		VectorCopy(Spell->velocity, Spell->movedir);
		VectorNormalize(Spell->movedir);
		vectoangles(Spell->movedir, Spell->s.angles);
		Spell->s.angles[PITCH] = anglemod(Spell->s.angles[PITCH] * -1);

		if(stricmp(self->classname, "monster_plagueElf"))//one of the special dudes
		{
			if(irand(0, 3) || skill->value < 2 || !stricmp(self->classname, "monster_palace_plague_guard_invisible"))
				Spell->red_rain_count = FX_PE_MAKE_SPELL2;
			else
				Spell->red_rain_count = FX_PE_MAKE_SPELL3;
		}
		else
			Spell->red_rain_count = FX_PE_MAKE_SPELL;

		gi.CreateEffect(&Spell->s,
			FX_PE_SPELL,
			CEF_OWNERS_ORIGIN,
			NULL, 
			"bv",
			Spell->red_rain_count,
			Spell->velocity);

		G_LinkMissile(Spell); 

		Spell->nextthink=level.time+3;
		Spell->think=G_FreeEdict;//plagueElfSpellThink;
	}
	else//If our enemy is dead, we need to stand
		QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);

}


void plagueElf_c_spell(edict_t *self)
{//fixme; adjust for up/down
	vec3_t	Forward, right, firedir, holdpos;
	edict_t	*Spell;
	
	if(self->s.fmnodeinfo[MESH__R_ARM].flags&FMNI_NO_DRAW)	// Was his arm lopped off?
		return;

	self->monsterinfo.attack_finished = level.time + 0.4;
	Spell = G_Spawn();

	create_pe_spell(Spell);

	Spell->touch=plagueElfSpellTouch;

	Spell->owner=self;
//	Spell->enemy=self->enemy;
	
	Spell->health = 0; // tell the touch function what kind of Spell we are;

	AngleVectors(self->s.angles, Forward, right, NULL);
	VectorCopy(self->s.origin,Spell->s.origin);	
	VectorMA(Spell->s.origin, 4, Forward, Spell->s.origin);
	VectorMA(Spell->s.origin, 8, right, Spell->s.origin);
	Spell->s.origin[2] += 12;

	VectorCopy(self->movedir,Spell->movedir);
	vectoangles(Forward,Spell->s.angles);


	VectorCopy(self->s.origin,holdpos);
	VectorMA(Spell->s.origin, 40, Forward, Spell->s.origin);

	VectorSubtract(holdpos, Spell->s.origin, firedir);
	VectorNormalize(firedir);
	Forward[2] = firedir[2];
	VectorNormalize(Forward);
	VectorScale(Forward, 500, Spell->velocity);

	VectorCopy(Spell->velocity, Spell->movedir);
	VectorNormalize(Spell->movedir);
	vectoangles(Spell->movedir, Spell->s.angles);
	Spell->s.angles[PITCH] = anglemod(Spell->s.angles[PITCH] * -1);

	if(stricmp(self->classname, "monster_plagueElf"))//one of the special dudes
	{
		if(irand(0, 3))
			Spell->red_rain_count = FX_PE_MAKE_SPELL2;
		else
			Spell->red_rain_count = FX_PE_MAKE_SPELL3;
	}
	else
		Spell->red_rain_count = FX_PE_MAKE_SPELL;

	gi.CreateEffect(&Spell->s,
		FX_PE_SPELL,
		CEF_OWNERS_ORIGIN,
		NULL, 
		"bv",
		Spell->red_rain_count,
		Spell->velocity);

	G_LinkMissile(Spell); 

	Spell->nextthink=level.time+3;
	Spell->think=G_FreeEdict;//plagueElfSpellThink;

}

void plagueElf_runaway (edict_t *self, G_Message_t *msg)
{
	if(self->spawnflags&MSF_FIXED)
		return;

	self->monsterinfo.aiflags |= AI_FLEE;
	self->monsterinfo.flee_finished = level.time + flrand(2, 4);
}

void plagueElf_missile(edict_t *self, G_Message_t *msg)
{
	pelf_init_phase_in(self);

	SetAnim(self, ANIM_MISSILE);
}

/*-------------------------------------------------------------------------
	plagueElf_melee
-------------------------------------------------------------------------*/
void plagueElf_melee(edict_t *self, G_Message_t *msg)
{
	vec3_t	attackVel, vf;
	int		ret;

	if (M_ValidTarget(self, self->enemy))
	{//A monster in melee will continue too long if the player backs away, this prevents it
		if(self->spawnflags&MSF_FIXED||self->monsterinfo.aiflags&AI_NO_MELEE)
		{
			SetAnim(self, ANIM_MISSILE);
			return;
		}
		AngleVectors(self->s.angles, vf, NULL, NULL);
		VectorMA(vf, 1, vf, attackVel);
		ret  = M_PredictTargetEvasion( self, self->enemy, attackVel, self->enemy->velocity, self->melee_range, 5 );

		if (ret)
		{
			if(irand(0,10)<5)
				SetAnim(self, ANIM_MELEE1);		
			else
				SetAnim(self, ANIM_MELEE2);		
		}
		else 
		{	
			if(self->spawnflags&MSF_FIXED)
				SetAnim(self, ANIM_DELAY);
			else
				SetAnim(self, ANIM_RUNATK1);
		}
	}
	else//If our enemy is dead, we need to stand
		QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
}

/*-------------------------------------------------------------------------
	plagueElf_pain
-------------------------------------------------------------------------*/

void pelf_dismember_sound(edict_t *self)
{
	if(self->health <= 0)
		return;

	if(irand(0, 1))
		gi.sound(self, CHAN_BODY, sounds[SND_DISMEMBER1], 1, ATTN_NORM, 0);
	else
		gi.sound(self, CHAN_BODY, sounds[SND_DISMEMBER2], 1, ATTN_NORM, 0);
}

qboolean canthrownode_pe (edict_t *self, int BP, int *throw_nodes)
{//see if it's on, if so, add it to throw_nodes
	//turn it off on thrower
	if(!(self->s.fmnodeinfo[BP].flags & FMNI_NO_DRAW))
	{
		*throw_nodes |= Bit_for_MeshNode_pe[BP];
		self->s.fmnodeinfo[BP].flags |= FMNI_NO_DRAW;
		return true;
	}
	return false;
}

void plagueElf_chicken (edict_t *self, int coward, int flee, float fleetime)
{
	float chance;

	chance = flrand(0, 20 + skill->value * 10);
	
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

void plagueElf_dead_pain (edict_t *self, G_Message_t *msg)
{
	if(msg)
		if(!(self->svflags & SVF_PARTS_GIBBED))
			MG_parse_dismember_msg(self, msg);
}

//THROWS weapon, turns off those nodes, sets that weapon as gone
qboolean plagueElf_dropweapon (edict_t *self, int damage)
{//NO PART FLY FRAME!
	vec3_t handspot, forward, right, up;
	float chance;

	if(self->s.fmnodeinfo[MESH__HANDLE].flags & FMNI_NO_DRAW)
		return false;

	VectorClear(handspot);
	AngleVectors(self->s.angles,forward,right,up);
	VectorMA(handspot,5,forward,handspot);
	VectorMA(handspot,8,right,handspot);
	VectorMA(handspot,-6,up,handspot);

	if(self->deadflag == DEAD_DEAD||(self->s.fmnodeinfo[MESH__R_ARM].flags & FMNI_NO_DRAW))
		chance = 0;
	else
		chance = 3;
	if(!(self->s.fmnodeinfo[MESH__HOE].flags & FMNI_NO_DRAW))
	{
		if(irand(0,10)<chance)
		{//just take off top
			ThrowWeapon(self, &handspot, BIT_HOE, 0, FRAME_torsooff);
			self->s.fmnodeinfo[MESH__HOE].flags |= FMNI_NO_DRAW;
			plagueElf_chicken(self,2,5,flrand(2,7));
		}
		else
		{
			ThrowWeapon(self, &handspot, BIT_HANDLE|BIT_HOE, 0, FRAME_partfly);
			self->s.fmnodeinfo[MESH__HOE].flags |= FMNI_NO_DRAW;
			self->s.fmnodeinfo[MESH__HANDLE].flags |= FMNI_NO_DRAW;
			plagueElf_chicken(self,4,8,flrand(3,8));
		}
		return true;
	}
	if(!(self->s.fmnodeinfo[MESH__GAFF].flags & FMNI_NO_DRAW))
	{
		if(irand(0,10)<chance)
		{//just take off top
			ThrowWeapon(self, &handspot, BIT_GAFF, 0, FRAME_partfly);
			self->s.fmnodeinfo[MESH__GAFF].flags |= FMNI_NO_DRAW;
			plagueElf_chicken(self,2,6,flrand(2,7));
		}
		else
		{
			ThrowWeapon(self, &handspot, BIT_HANDLE|BIT_GAFF, 0, FRAME_partfly);
			self->s.fmnodeinfo[MESH__GAFF].flags |= FMNI_NO_DRAW;
			self->s.fmnodeinfo[MESH__HANDLE].flags |= FMNI_NO_DRAW;
			plagueElf_chicken(self,4,8,flrand(3,8));
		}
		return true;
	}
	if(!(self->s.fmnodeinfo[MESH__HAMMER].flags & FMNI_NO_DRAW))
	{
		if(irand(0,10)<chance)
		{//just take off top
			ThrowWeapon(self, &handspot, BIT_HAMMER, 0, FRAME_partfly);
			self->s.fmnodeinfo[MESH__HAMMER].flags |= FMNI_NO_DRAW;
			plagueElf_chicken(self,2,6,flrand(2,7));
		}
		else
		{
			ThrowWeapon(self, &handspot, BIT_HANDLE|BIT_HAMMER, 0, FRAME_partfly);
			self->s.fmnodeinfo[MESH__HAMMER].flags |= FMNI_NO_DRAW;
			self->s.fmnodeinfo[MESH__HANDLE].flags |= FMNI_NO_DRAW;
			plagueElf_chicken(self,4,8,flrand(3,8));
		}
		return true;
	}
	ThrowWeapon(self, &handspot, BIT_HANDLE, 0, FRAME_partfly);
	self->s.fmnodeinfo[MESH__HANDLE].flags |= FMNI_NO_DRAW;
	if(self->deadflag != DEAD_DEAD)
		plagueElf_chicken(self,6,8,flrand(5,10));
	return true;
}

void plagueElf_dismember(edict_t *self, int	damage,	int HitLocation)
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

	if(self->curAnimID==ANIM_MELEE1||self->curAnimID==ANIM_MELEE1)
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

	VectorCopy(vec3_origin,gore_spot);
	switch(HitLocation)
	{
		case hl_Head:
			if(self->s.fmnodeinfo[MESH__HEAD].flags & FMNI_NO_DRAW)
				break;
			if(self->s.fmnodeinfo[MESH__HEAD].flags & FMNI_USE_SKIN)
				damage*=1.5;//greater chance to cut off if previously damaged
			if(flrand(0,self->health)<damage*0.25)
				plagueElf_dropweapon (self, (int)damage);
			if(flrand(0,self->health)<damage*0.3&&dismember_ok)
			{
				canthrownode_pe(self, MESH__HEAD,&throw_nodes);

				gore_spot[2]+=18;
				pelf_dismember_sound(self);
				ThrowBodyPart(self, &gore_spot, throw_nodes, damage, 0);

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
				self->s.fmnodeinfo[MESH__HEAD].flags |= FMNI_USE_SKIN;
				self->s.fmnodeinfo[MESH__HEAD].skin = self->s.skinnum+1;
			}
			break;
		case hl_TorsoFront://split in half?
		case hl_TorsoBack://split in half?
			if(self->s.fmnodeinfo[MESH__BODY].flags & FMNI_NO_DRAW)
				break;
			if(self->s.fmnodeinfo[MESH__BODY].flags & FMNI_USE_SKIN)
				damage*=1.5;//greater chance to cut off if previously damaged
			if(self->health > 0 && flrand(0,self->health)<damage*0.3 && dismember_ok)
			{
				gore_spot[2]+=12;
				canthrownode_pe(self, MESH__BODY,&throw_nodes);
				canthrownode_pe(self, MESH__L_ARM,&throw_nodes);
				canthrownode_pe(self, MESH__R_ARM,&throw_nodes);
				canthrownode_pe(self, MESH__HEAD,&throw_nodes);

				plagueElf_dropweapon (self, (int)damage);
				pelf_dismember_sound(self);
				ThrowBodyPart(self, &gore_spot, throw_nodes, damage, FRAME_torsooff);
				VectorAdd(self->s.origin, gore_spot, gore_spot);
				SprayDebris(self,gore_spot,12,damage);

				if(self->health>0)
				{
					self->health = 1;
					T_Damage (self, self, self, vec3_origin, vec3_origin, vec3_origin, 10, 20,0,MOD_DIED);
				}
				return;
			}
			else
			{
				if(flrand(0,self->health)<damage*0.5)
					plagueElf_dropweapon (self, (int)damage);
				self->s.fmnodeinfo[MESH__BODY].flags |= FMNI_USE_SKIN;			
				self->s.fmnodeinfo[MESH__BODY].skin = self->s.skinnum+1;
			}
			break;
		case hl_ArmUpperLeft:
		case hl_ArmLowerLeft://left arm
			if(self->s.fmnodeinfo[MESH__L_ARM].flags & FMNI_NO_DRAW)
				break;
			if(self->s.fmnodeinfo[MESH__L_ARM].flags & FMNI_USE_SKIN)
				damage*=1.5;//greater chance to cut off if previously damaged
			if(flrand(0,self->health)<damage*0.4)
				plagueElf_dropweapon (self, (int)damage);
			if(flrand(0,self->health)<damage*0.75&&dismember_ok)
			{
				if(canthrownode_pe(self, MESH__L_ARM, &throw_nodes))
				{
					AngleVectors(self->s.angles,NULL,right,NULL);
					gore_spot[2]+=self->maxs[2]*0.3;
					VectorMA(gore_spot,-10,right,gore_spot);
					plagueElf_chicken(self,6,8,flrand(7,15));
					pelf_dismember_sound(self);
					ThrowBodyPart(self, &gore_spot, throw_nodes, damage, 0);
				}
			}
			else
			{
				self->s.fmnodeinfo[MESH__L_ARM].flags |= FMNI_USE_SKIN;			
				self->s.fmnodeinfo[MESH__L_ARM].skin = self->s.skinnum+1;
			}
			break;
		case hl_ArmUpperRight:
		case hl_ArmLowerRight://right arm
			//Knock weapon out of hand?
			if(self->s.fmnodeinfo[MESH__R_ARM].flags & FMNI_NO_DRAW)
				break;
			if(self->s.fmnodeinfo[MESH__R_ARM].flags & FMNI_USE_SKIN)
				damage*=1.5;//greater chance to cut off if previously damaged
			if(flrand(0,self->health)<damage*0.75&&dismember_ok)
			{
				if(canthrownode_pe(self, MESH__R_ARM, &throw_nodes))
				{
					AngleVectors(self->s.angles,NULL,right,NULL);
					gore_spot[2]+=self->maxs[2]*0.3;
					VectorMA(gore_spot,10,right,gore_spot);
					plagueElf_dropweapon (self, (int)damage);
					pelf_dismember_sound(self);
					ThrowBodyPart(self, &gore_spot, throw_nodes, damage, 0);
				}
			}
			else
			{
				if(flrand(0,self->health)<damage*0.75)
					plagueElf_dropweapon (self, (int)damage);
				self->s.fmnodeinfo[MESH__R_ARM].flags |= FMNI_USE_SKIN;			
				self->s.fmnodeinfo[MESH__R_ARM].skin = self->s.skinnum+1;
			}
			break;

		case hl_LegUpperLeft:
		case hl_LegLowerLeft://left leg
			if(self->health>0)
			{//still alive
				if(self->s.fmnodeinfo[MESH__L_LEG].flags & FMNI_USE_SKIN)
					break;
				self->s.fmnodeinfo[MESH__L_LEG].flags |= FMNI_USE_SKIN;			
				self->s.fmnodeinfo[MESH__L_LEG].skin = self->s.skinnum+1;
			}
			else
			{
				if(self->s.fmnodeinfo[MESH__L_LEG].flags & FMNI_NO_DRAW)
					break;
				if(canthrownode_pe(self, MESH__L_LEG, &throw_nodes))
				{
					AngleVectors(self->s.angles,NULL,right,NULL);
					gore_spot[2]+=self->maxs[2]*0.3;
					VectorMA(gore_spot,-10,right,gore_spot);
					ThrowBodyPart(self, &gore_spot, throw_nodes, damage, 0);
				}
				break;
			}
			break;
		case hl_LegUpperRight:
		case hl_LegLowerRight://right leg
			if(self->health>0)
			{//still alive
				if(self->s.fmnodeinfo[MESH__R_LEG].flags & FMNI_USE_SKIN)
					break;
				self->s.fmnodeinfo[MESH__R_LEG].flags |= FMNI_USE_SKIN;
				self->s.fmnodeinfo[MESH__R_LEG].skin = self->s.skinnum+1;
			}
			else
			{
				if(self->s.fmnodeinfo[MESH__R_LEG].flags & FMNI_NO_DRAW)
					break;
				if(canthrownode_pe(self, MESH__R_LEG, &throw_nodes))
				{
					AngleVectors(self->s.angles,NULL,right,NULL);
					gore_spot[2]+=self->maxs[2]*0.3;
					VectorMA(gore_spot,-10,right,gore_spot);
					ThrowBodyPart(self, &gore_spot, throw_nodes, damage, 0);
				}
				break;
			}
			break;

		default:
			if(flrand(0,self->health)<damage*0.25)
				plagueElf_dropweapon (self, (int)damage);
			break;
	}
	if(throw_nodes)
		self->pain_debounce_time = 0;

	if(self->s.fmnodeinfo[MESH__R_ARM].flags&FMNI_NO_DRAW)
	{
		self->monsterinfo.aiflags |= AI_NO_MELEE;
		self->monsterinfo.aiflags |= AI_NO_MISSILE;
	}
	else if(self->s.fmnodeinfo[MESH__HANDLE].flags & FMNI_NO_DRAW)
	{
		self->monsterinfo.aiflags |= AI_NO_MELEE;
		if(self->missile_range)
		{
			if(!(self->s.fmnodeinfo[MESH__R_ARM].flags&FMNI_NO_DRAW))
			{
				self->monsterinfo.aiflags &= ~AI_NO_MISSILE;
				self->melee_range = 0;
			}
			else
				self->monsterinfo.aiflags |= AI_NO_MISSILE;
		}
	}

	if(self->monsterinfo.aiflags&AI_NO_MISSILE &&
		self->monsterinfo.aiflags&AI_NO_MELEE)
		self->monsterinfo.aiflags |= AI_COWARD;
}


void plagueElf_pain(edict_t *self, G_Message_t *msg)
{
	int				temp, damage;
	qboolean		force_pain;
	
	ParseMsgParms(msg, "eeiii", &temp, &temp, &force_pain, &damage, &temp);

	pelf_init_phase_in(self);

	if(!(self->monsterinfo.aiflags & AI_COWARD) && !(self->monsterinfo.aiflags & AI_FLEE))
		if(!force_pain)
			if(flrand(0,self->health)>damage)
				return;

	if(self->fire_damage_time > level.time)
		self->monsterinfo.aiflags |= AI_COWARD;

	if (self->pain_debounce_time < level.time)
	{
		self->pain_debounce_time = level.time + 1;
		SetAnim(self, ANIM_PAIN1);
	}
}

void plagueElfApplyJump (edict_t *self)
{
	self->jump_time = level.time + 0.5;
	VectorCopy(self->movedir, self->velocity);
	VectorNormalize(self->movedir);
}

/*-------------------------------------------------------------------------
	plagueElf_pause
-------------------------------------------------------------------------*/
void pelf_phase_out (edict_t *self);
void pelf_phase_in (edict_t *self);
void plagueElf_pause (edict_t *self)
{
	self->monsterinfo.misc_debounce_time = 0;

	if(self->ai_mood == AI_MOOD_FLEE)
	{
		if(self->s.color.a != 255 && self->pre_think!=pelf_phase_in)
			pelf_init_phase_in(self);
	}
	else
	{
		if(!skill->value)
		{
			if(self->s.color.a > 50 && self->pre_think!=pelf_phase_out)
				pelf_init_phase_out(self);
		}
		else if(self->s.color.a && self->pre_think!=pelf_phase_out)
			pelf_init_phase_out(self);
	}

	if(self->spawnflags & MSF_FIXED && self->curAnimID == ANIM_DELAY && self->enemy)
	{
		self->monsterinfo.searchType = SEARCH_COMMON;
		MG_FaceGoal(self, true);
	}

	self->mood_think(self);

	switch (self->ai_mood)
	{
	case AI_MOOD_ATTACK:
		if(self->ai_mood_flags & AI_MOOD_FLAG_MISSILE)
			QPostMessage(self, MSG_MISSILE, PRI_DIRECTIVE, NULL);
		else
			QPostMessage(self, MSG_MELEE, PRI_DIRECTIVE, NULL);
		break;

	case AI_MOOD_PURSUE:
		if(self->ai_mood_flags&AI_MOOD_FLAG_DUMB_FLEE)
			SetAnim(self, ANIM_SCARED);
		else
			QPostMessage(self, MSG_RUN, PRI_DIRECTIVE, NULL);
		break;

	case AI_MOOD_NAVIGATE:
		QPostMessage(self, MSG_RUN, PRI_DIRECTIVE, NULL);
		break;

	case AI_MOOD_WALK:
		QPostMessage(self, MSG_WALK, PRI_DIRECTIVE, NULL);
		break;

	case AI_MOOD_FLEE:
		if(self->ai_mood_flags&AI_MOOD_FLAG_DUMB_FLEE || (!(self->ai_mood_flags&AI_MOOD_FLAG_FORCED_BUOY) && !irand(0, 20)))
		{
			if(self->enemy)
			{
				if(M_DistanceToTarget(self, self->enemy) < 100)
				{
					if(self->curAnimID == ANIM_SCARED)
						dying_elf_sounds (self, DYING_ELF_PAIN_VOICE);
					if(irand(0,1))
						SetAnim(self, ANIM_CRAZY_A);
					else
						SetAnim(self, ANIM_CRAZY_B);
					break;
				}
			}
			SetAnim(self, ANIM_SCARED);
		}
		else if(irand(0,1))
			SetAnim(self, ANIM_CRAZY_A);
		else
			SetAnim(self, ANIM_CRAZY_B);
		break;

	case AI_MOOD_STAND:
		QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
		break;

	case AI_MOOD_DELAY:
		SetAnim(self, ANIM_DELAY);
		break;

	case AI_MOOD_WANDER:
		if(self->spawnflags&MSF_FIXED)
			SetAnim(self, ANIM_DELAY);
		else
			SetAnim(self, ANIM_WALK1);
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
	//	gi.dprintf("plague elf: Unusable mood %d!\n", self->ai_mood);
#endif
		break;
	}
}

void pelf_land(edict_t *self)
{
	gi.sound(self, CHAN_BODY, gi.soundindex("misc/land.wav"), 1, ATTN_NORM, 0);
	gi.CreateEffect(&self->s,
					   FX_DUST_PUFF,
					   CEF_OWNERS_ORIGIN,
					   self->s.origin,
					   NULL);
}

void pelf_go_inair(edict_t *self)
{
	SetAnim(self, ANIM_INAIR);
}

void pelf_jump (edict_t *self, G_Message_t *msg)
{
	if(self->spawnflags&MSF_FIXED)
		SetAnim(self, ANIM_DELAY);
	else
		SetAnim(self, ANIM_FJUMP);
}

void pelf_check_mood (edict_t *self, G_Message_t *msg)
{
	ParseMsgParms(msg, "i", &self->ai_mood);

	plagueElf_pause(self);
}

/*-------------------------------------------------------------------------
	plagueElf_run
-------------------------------------------------------------------------*/
void plagueElf_run(edict_t *self, G_Message_t *msg)
{
	if(self->curAnimID == ANIM_CURSING || self->curAnimID == ANIM_POINT)
		return;

	if (M_ValidTarget(self, self->enemy))
	{
		if(self->spawnflags&MSF_FIXED)
			SetAnim(self, ANIM_DELAY);
		else 
			SetAnim(self, ANIM_RUN1);
	}
	else//If our enemy is dead, we need to stand
		QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
}

/*----------------------------------------------------------------------
  plagueElf runorder - order the plagueElf to choose an run animation
-----------------------------------------------------------------------*/
void plagueElf_runorder(edict_t *self)
{
	QPostMessage(self, MSG_RUN, PRI_DIRECTIVE, NULL);
}


/*-------------------------------------------------------------------------
	plagueElfsqueal
-------------------------------------------------------------------------*/
void plagueElfsqueal (edict_t *self)
{
	if(self->monsterinfo.aiflags & AI_COWARD || self->monsterinfo.aiflags & AI_FLEE)
		dying_elf_sounds (self, DYING_ELF_PAIN_VOICE);
	else
	{
		int sound = irand(SND_PAIN1, SND_PAIN3);

		gi.sound(self, CHAN_VOICE, sounds[sound], 1, ATTN_NORM, 0);
	}
}

/*-------------------------------------------------------------------------
	plagueElf_stand
-------------------------------------------------------------------------*/
void plagueElf_stand(edict_t *self, G_Message_t *msg)
{	
	if (self->ai_mood == AI_MOOD_DELAY)
		SetAnim(self, ANIM_DELAY);
	else
		SetAnim(self, ANIM_SHAKE1);	

	return;
}

/*-------------------------------------------------------------------------
	plagueElf_walk
-------------------------------------------------------------------------*/
void plagueElf_walk(edict_t *self, G_Message_t *msg)
{
	if(self->curAnimID == ANIM_CURSING || self->curAnimID == ANIM_POINT)
		return;

	if(self->spawnflags&MSF_FIXED)
		SetAnim(self, ANIM_DELAY);
	else if(irand(0, 1))	
		SetAnim(self, ANIM_WALK1);	
	else	
		SetAnim(self, ANIM_WALK2);	
	return;	
}

void plagueElf_go_run(edict_t *self)
{
	if(self->spawnflags&MSF_FIXED)
		SetAnim(self, ANIM_DELAY);
	else
		SetAnim(self, ANIM_RUN1);
}

/*

	Plagued Elf voice support functions

*/

/*-----------------------------------------------
	pelf_ChooseSightSound 	
-----------------------------------------------*/

#define SE_ALONE	0
#define SE_PAIR		1
#define SE_GROUP	2

int pelf_ChooseSightSound ( edict_t *self, int event )
{
	int sound;

	switch (event)
	{
	case SE_ALONE:
		sound = irand( FIRST_SIGHT_ALONE, LAST_SIGHT_ALONE);
		break;

	case SE_PAIR:
	case SE_GROUP:
		sound = irand( FIRST_SIGHT_GROUP, LAST_SIGHT_GROUP );
		break;
	}

	return sound;
}

/*-----------------------------------------------
	pelf_ChooseReponseSound 
-----------------------------------------------*/

int pelf_ChooseResponseSound ( edict_t *self, int event, int sound_id )
{
	int sound;

	switch (event)
	{
	case SE_PAIR:
	case SE_GROUP:
		sound = irand( FIRST_SIGHT_GROUP, LAST_SIGHT_GROUP);
		break;
	}

	return sound;
}
//plague elf has seen first target (usually player)

/*-----------------------------------------------
	pelf_SightSound 	
-----------------------------------------------*/
#define PLAGUEELF_SUPPORT_RADIUS 200

void pelf_SightSound ( edict_t *self, G_Message_t *msg )
{
	edict_t	*enemy = NULL;
	byte	sight_type;
	int		support, sound;

	if(self->targetname || self->monsterinfo.c_mode)
		return;//cinematic waiting to be activated, don't do this

	//Have we already said something?
	if (self->monsterinfo.supporters != -1)
		return;
	
	ParseMsgParms(msg, "be", &sight_type, &enemy);

	//Find out how many elves are around (save this if we want it later)
	support = self->monsterinfo.supporters = M_FindSupport(self, PLAGUEELF_SUPPORT_RADIUS);

	//See if we are the first to see the player
	if(M_CheckAlert(self, PLAGUEELF_SUPPORT_RADIUS))
	{
		//gi.dprintf("pelf_SightSound: elf has %d supporters\n", support);

		if (support < 1)	//Loner
		{
			sound = pelf_ChooseSightSound(self, SE_ALONE);
			gi.sound(self, CHAN_VOICE, sounds[sound], 1, ATTN_NORM, 0);
		}
		else if (support < 2)	//Paired
		{
			sound = pelf_ChooseSightSound(self, SE_PAIR);
			self->monsterinfo.sound_finished = level.time + pelf_VoiceTimes[sound];
			gi.sound(self, CHAN_VOICE, sounds[sound], 1, ATTN_NORM, 0);

			pelf_PollResponse( self, SE_PAIR, sound, self->monsterinfo.sound_finished - flrand(0.5, 0.25) );
		}
		else //Grouped
		{
			sound = pelf_ChooseSightSound(self, SE_GROUP);
			self->monsterinfo.sound_finished = level.time + pelf_VoiceTimes[sound];
			gi.sound(self, CHAN_VOICE, sounds[sound], 1, ATTN_NORM, 0);
			
			pelf_PollResponse( self, SE_GROUP, sound, self->monsterinfo.sound_finished - flrand(0.5, 0.25) );
		}

		if(support)
		{//FIXME: make sure enemy is far enough away to anim!
			if(irand(0, 1))
				SetAnim(self, ANIM_CURSING);
			else
				SetAnim(self, ANIM_POINT);
		}
	}
}

//The plague elf has said something and is looking for a response
void pelf_PollResponse ( edict_t *self, int sound_event, int sound_id, float time )
{
	edict_t *ent = NULL, *last_valid = NULL;
	int		numSupport = 0;

	while((ent = findradius(ent, self->s.origin, PLAGUEELF_SUPPORT_RADIUS)) != NULL)
	{
		//Not us
		if (ent==self)
			continue;

		//Same class
		if (ent->classID != self->classID)
			continue;

		//Not already talking
		if (ent->monsterinfo.sound_pending || ent->monsterinfo.sound_finished > level.time)
			continue;

		//Not going after different goals
		if ((ent->enemy) && ent->enemy != self->enemy)
			continue;

		//Alive
		if (ent->health <= 0)
			continue;

		if(!ok_to_wake(ent, false, false))
			continue;

		//Save this as valid!
		last_valid = ent;

		//Random chance to continue on
		if (irand(0,1))
			continue;

		if(!ent->enemy)
		{
			ent->enemy = self->enemy;
			FoundTarget(ent, false);
		}

		//This is the elf to respond, so post the message
		QPostMessage(ent, MSG_VOICE_POLL, PRI_DIRECTIVE, "bbf", sound_event, sound_id, time);
		last_valid = NULL;
		break;
	}

	//We skipped the last valid elf, so just make the last valid one talk
	if (last_valid)
	{
		QPostMessage(last_valid, MSG_VOICE_POLL, PRI_DIRECTIVE, "bbf", sound_event, sound_id, time);
		last_valid = NULL;
	}
}

//A plague elf has been polled for a response, now choose a reply and echo it back
void pelf_EchoResponse  ( edict_t *self, G_Message_t *msg )
{
	float	time;
	int		sound_event, sound_id;

	if(self->targetname || self->monsterinfo.c_mode)
		return;//cinematic waiting to be activated, don't do this

	//gi.dprintf("pelf_EchoResponse: Echoing alert response\n");

	ParseMsgParms(msg, "bbf", &sound_event, &sound_id, &time);

	switch ( sound_event )
	{
	case SE_PAIR:
		self->monsterinfo.sound_pending = pelf_ChooseResponseSound( self, SE_PAIR, sound_id );
		self->monsterinfo.sound_start = time;
		self->monsterinfo.sound_finished = level.time + pelf_VoiceTimes[self->monsterinfo.sound_pending];

		if(irand(0, 4))
		{//FIXME: make sure enemy is far enough away to anim!
			if(irand(0, 1))
				SetAnim(self, ANIM_CURSING);
			else
				SetAnim(self, ANIM_POINT);
		}
		break;
	
	case SE_GROUP:
		self->monsterinfo.sound_pending = pelf_ChooseResponseSound( self, SE_GROUP, sound_id );
		self->monsterinfo.sound_start = time;
		self->monsterinfo.sound_finished = level.time + pelf_VoiceTimes[self->monsterinfo.sound_pending];

		if (irand(0,2))
			pelf_PollResponse( self, SE_GROUP, self->monsterinfo.sound_pending, self->monsterinfo.sound_finished );

		if(irand(0, 2))
		{//FIXME: make sure enemy is far enough away to anim!
			if(irand(0, 1))
				SetAnim(self, ANIM_CURSING);
			else
				SetAnim(self, ANIM_POINT);

		}
		break;

	default:
		self->monsterinfo.sound_pending = pelf_ChooseResponseSound( self, SE_GROUP, sound_id );
		self->monsterinfo.sound_start = time;
		self->monsterinfo.sound_finished = level.time + pelf_VoiceTimes[self->monsterinfo.sound_pending];

		if (irand(0,2))
			pelf_PollResponse( self, SE_GROUP, self->monsterinfo.sound_pending, self->monsterinfo.sound_finished );

		break;
	}	
}

//Play a sound from a trigger or a pending sound event
void pelf_EchoSound ( edict_t *self, G_Message_t *msg )
{
	int sound;

	ParseMsgParms(msg, "i", &sound);

	gi.sound(self, CHAN_VOICE, sounds[sound], 1, ATTN_NORM, 0);	
}

//
void pelf_check_too_close(edict_t *self)
{
	if(!self->enemy)
		return;

	if(M_DistanceToTarget(self, self->enemy) < flrand(0, 100))
	{
		if(irand(0,1))
			SetAnim(self, ANIM_CRAZY_A);
		else
			SetAnim(self, ANIM_CRAZY_B);
	}
}


void pelf_phase_out (edict_t *self)
{
	int	interval = 60;

	if(self->s.color.a > interval)
	{
		self->s.color.a -= irand(interval/2, interval);
		self->pre_think = pelf_phase_out;
		self->next_pre_think = level.time + 0.05;
	}
	else 
	{
		if(!skill->value)
			self->s.color.a = 50;
		else
			self->s.color.a = 0;
		self->pre_think = NULL;
		self->next_pre_think = -1;
	}
}

void pelf_init_phase_out (edict_t *self);
void pelf_phase_in (edict_t *self)
{
	int	interval = 60;
	
	if(self->s.color.a < 255 - interval)
	{
		self->s.color.a += irand(interval/2, interval);
		self->pre_think = pelf_phase_in;
		self->next_pre_think = level.time + 0.05;
	}
	else 
	{
		self->svflags &= ~SVF_NO_AUTOTARGET;
		self->s.color.c = 0xffffffff;
		if(self->health <= 0)
		{
			self->pre_think = NULL;
			self->next_pre_think = -1;
		}
		else
			pelf_init_phase_out(self);
	}
}

void pelf_init_phase_out (edict_t *self)
{
	if(stricmp(self->classname, "monster_palace_plague_guard_invisible"))
		return;

//	gi.dprintf("Elf phasing out\n");
	self->pre_think = pelf_phase_out;
	self->next_pre_think = level.time + FRAMETIME;
	self->svflags |= SVF_NO_AUTOTARGET;
}

void pelf_init_phase_in (edict_t *self)
{
	if(stricmp(self->classname, "monster_palace_plague_guard_invisible"))
		return;

//	gi.dprintf("Elf phasing in\n");
	self->pre_think = pelf_phase_in;
	self->next_pre_think = level.time + FRAMETIME;
}
/*-------------------------------------------------------------------------
	PlagueElfStaticsInit
-------------------------------------------------------------------------*/
void PlagueElfStaticsInit()
{
	classStatics[CID_PLAGUEELF].msgReceivers[MSG_STAND] = plagueElf_stand;
	classStatics[CID_PLAGUEELF].msgReceivers[MSG_WALK] = plagueElf_walk;
	classStatics[CID_PLAGUEELF].msgReceivers[MSG_RUN] = plagueElf_run;
	classStatics[CID_PLAGUEELF].msgReceivers[MSG_MELEE] = plagueElf_melee;
	classStatics[CID_PLAGUEELF].msgReceivers[MSG_MISSILE] = plagueElf_missile;
	classStatics[CID_PLAGUEELF].msgReceivers[MSG_PAIN] = plagueElf_pain;
	classStatics[CID_PLAGUEELF].msgReceivers[MSG_DEATH] = plagueElf_death;
//	classStatics[CID_PLAGUEELF].msgReceivers[MSG_BLOCKED] = plagueElf_blocked;
	classStatics[CID_PLAGUEELF].msgReceivers[MSG_DISMEMBER] = MG_parse_dismember_msg;
	classStatics[CID_PLAGUEELF].msgReceivers[MSG_JUMP] = pelf_jump;
	classStatics[CID_PLAGUEELF].msgReceivers[MSG_DEATH_PAIN] = plagueElf_dead_pain;
	classStatics[CID_PLAGUEELF].msgReceivers[MSG_FALLBACK] = plagueElf_run;//away
	classStatics[CID_PLAGUEELF].msgReceivers[MSG_CHECK_MOOD] = pelf_check_mood;

	//Sound support
	classStatics[CID_PLAGUEELF].msgReceivers[MSG_VOICE_SIGHT] = pelf_SightSound;
	classStatics[CID_PLAGUEELF].msgReceivers[MSG_VOICE_POLL] = pelf_EchoResponse;
	classStatics[CID_PLAGUEELF].msgReceivers[MSG_VOICE_PUPPET] = pelf_EchoSound;

	classStatics[CID_PLAGUEELF].msgReceivers[MSG_C_IDLE1] = plagueElf_c_anims;
	classStatics[CID_PLAGUEELF].msgReceivers[MSG_C_IDLE2] = plagueElf_c_anims;
	classStatics[CID_PLAGUEELF].msgReceivers[MSG_C_IDLE3] = plagueElf_c_anims;
	classStatics[CID_PLAGUEELF].msgReceivers[MSG_C_WALK1] = plagueElf_c_anims;
	classStatics[CID_PLAGUEELF].msgReceivers[MSG_C_WALK2] = plagueElf_c_anims;
	classStatics[CID_PLAGUEELF].msgReceivers[MSG_C_RUN1] = plagueElf_c_anims;
	classStatics[CID_PLAGUEELF].msgReceivers[MSG_C_ATTACK1] = plagueElf_c_anims;
	classStatics[CID_PLAGUEELF].msgReceivers[MSG_C_ATTACK2] = plagueElf_c_anims;
	classStatics[CID_PLAGUEELF].msgReceivers[MSG_C_ATTACK3] = plagueElf_c_anims;
	classStatics[CID_PLAGUEELF].msgReceivers[MSG_C_ATTACK4] = plagueElf_c_anims;
	classStatics[CID_PLAGUEELF].msgReceivers[MSG_C_DEATH1] = plagueElf_c_anims;
	classStatics[CID_PLAGUEELF].msgReceivers[MSG_C_DEATH2] = plagueElf_c_anims;
	classStatics[CID_PLAGUEELF].msgReceivers[MSG_C_DEATH3] = plagueElf_c_anims;
	classStatics[CID_PLAGUEELF].msgReceivers[MSG_C_DEATH4] = plagueElf_c_anims;
	classStatics[CID_PLAGUEELF].msgReceivers[MSG_C_THINKAGAIN] = plagueElf_c_anims;
	classStatics[CID_PLAGUEELF].msgReceivers[MSG_C_PAIN1] = plagueElf_c_anims;
	classStatics[CID_PLAGUEELF].msgReceivers[MSG_C_GIB1] = plagueElf_c_gib;

	resInfo.numAnims = NUM_ANIMS;
	resInfo.animations = animations;
	
	//note that the name is different in the path
	resInfo.modelIndex = gi.modelindex("models/monsters/plaguelf/tris.fm");

	sounds[SND_PAIN1] = gi.soundindex("monsters/plagueElf/pain1.wav");
	sounds[SND_PAIN2] = gi.soundindex("monsters/plagueElf/pain2.wav");
	sounds[SND_PAIN3] = gi.soundindex("monsters/plagueElf/pain3.wav");
	sounds[SND_DIE1] = gi.soundindex("monsters/plagueElf/death1.wav");
	sounds[SND_DIE2] = gi.soundindex("monsters/plagueElf/death2.wav");
	sounds[SND_DIE3] = gi.soundindex("monsters/plagueElf/death3.wav");
	sounds[SND_GIB] = gi.soundindex("monsters/plagueElf/gib2.wav");
	sounds[SND_ATTACKHIT1] = gi.soundindex("monsters/plagueElf/hookhit.wav");
	sounds[SND_ATTACKHIT2] = gi.soundindex("monsters/plagueElf/hamhit.wav");
	sounds[SND_ATTACKMISS1] = gi.soundindex("weapons/staffswing.wav");
	sounds[SND_MOAN1] = gi.soundindex("monsters/plagueElf/pelfgrn1.wav");
	sounds[SND_MOAN2] = gi.soundindex("monsters/plagueElf/pelfgron.wav");
	sounds[SND_SHIVER] = gi.soundindex("monsters/plagueElf/pelfshiv.wav");
	sounds[SND_PANT] = gi.soundindex("monsters/plagueElf/pelfpant.wav");
	sounds[SND_GASP] = gi.soundindex("monsters/plagueElf/pelfgasp.wav");
	sounds[SND_SIGH] = gi.soundindex("monsters/plagueElf/pelfsigh.wav");
	sounds[SND_ATTACK1] = gi.soundindex("monsters/plagueElf/attack1.wav");
	sounds[SND_ATTACK2] = gi.soundindex("monsters/plagueElf/attack2.wav");
	sounds[SND_DISMEMBER1] = gi.soundindex("monsters/plagueElf/loselimb1.wav");
	sounds[SND_DISMEMBER2] = gi.soundindex("monsters/plagueElf/loselimb2.wav");
	
	//Plague elf voices
	sounds[VOICE_SIGHT_EAT_FLESH1]	= gi.soundindex("monsters/plagueElf/voices/eatfleshb.wav");
	sounds[VOICE_SIGHT_GET_HIM1]	= gi.soundindex("monsters/plagueElf/voices/gethimb.wav");
	sounds[VOICE_SIGHT_GET_HIM2]	= gi.soundindex("monsters/plagueElf/voices/gethimk.wav");
	sounds[VOICE_SIGHT_GET_HIM3]	= gi.soundindex("monsters/plagueElf/voices/gethimm.wav");
	sounds[VOICE_SIGHT_THERES_ONE]	= gi.soundindex("monsters/plagueElf/voices/theresonep.wav");

	sounds[VOICE_SUPPORT_GONNA_DIE1] = gi.soundindex("monsters/plagueElf/voices/gonnadieb.wav");
	sounds[VOICE_SUPPORT_LIVER]		= gi.soundindex("monsters/plagueElf/voices/liverm.wav");
	sounds[VOICE_SUPPORT_YES]		= gi.soundindex("monsters/plagueElf/voices/yesb.wav");

	sounds[VOICE_MISC_LEAVE_ME1]	= gi.soundindex("monsters/plagueElf/voices/leavemeb.wav");
	sounds[VOICE_MISC_NO]			= gi.soundindex("monsters/plagueElf/voices/nomrj.wav");
	
	resInfo.numSounds = NUM_SOUNDS;
	resInfo.sounds = sounds;

	classStatics[CID_PLAGUEELF].resInfo = &resInfo;
}

/*QUAKED monster_plagueElf (1 .5 0) (-17 -25 -1) (22 12 63) AMBUSH ASLEEP WALKING CINEMATIC Missile 32 64 FIXED WANDER MELEE_LEAD STALK COWARD EXTRA1 EXTRA2 EXTRA3 EXTRA4

Missile - can fire a ranged attack

The plagueElf 

AMBUSH - Will not be woken up by other monsters or shots from player

ASLEEP - will not appear until triggered

WALKING - use WANDER instead

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

DEFAULTS(melee P.E.):
mintel					= 16
melee_range				= 0
missile_range			= 512
min_missile_range		= 0
bypass_missile_chance	= 0
jump_chance				= 50
wakeup_distance			= 1024

DEFAULTS(missile P.E.):
mintel					= 16
melee_range				= 0
missile_range			= 512
min_missile_range		= 0
bypass_missile_chance	= 60
jump_chance				= 50
wakeup_distance			= 1024

NOTE: A value of zero will result in defaults, if you actually want zero as the value, use -1
*/

#define PALACE_ELF_SKIN	4
/*-------------------------------------------------------------------------
	SP_monster_plagueElf
-------------------------------------------------------------------------*/
void SP_monster_plagueElf (edict_t *self)
{
	int chance;//, scale;

	if(self->spawnflags & MSF_WALKING)
	{
		self->spawnflags |= MSF_WANDER;
		self->spawnflags &= ~MSF_WALKING;
	}

	self->classID = CID_PLAGUEELF;

	if (!walkmonster_start(self))		// Failed initialization
		return;
		
	self->msgHandler = DefaultMsgHandler;
	self->monsterinfo.dismember = plagueElf_dismember;

	if (!self->health)
	{
		self->health = PLAGUEELF_HEALTH;
	}

	//Apply to the end result (whether designer set or not)
	self->max_health = self->health = MonsterHealth(self->health);

	self->mass = PLAGUEELF_MASS;
	self->yaw_speed = 20;

	self->movetype = PHYSICSTYPE_STEP;
	VectorClear(self->knockbackvel);
	
	self->solid=SOLID_BBOX;

	if(irand(0,1))
		self->ai_mood_flags |= AI_MOOD_FLAG_PREDICT;
	//FIXME: Hack to account for new origin with old QuakEd header
	self->s.origin[2] += 32;

	VectorCopy(STDMinsForClass[self->classID], self->mins);
	VectorCopy(STDMaxsForClass[self->classID], self->maxs);	
	self->viewheight = self->maxs[2]*0.8;
	
	self->s.modelindex = classStatics[CID_PLAGUEELF].resInfo->modelIndex;

	// All skins are even numbers, pain skins are skin+1.
	if (!self->s.skinnum)
	{	// If the skin hasn't been touched, set it.
		if (irand(0,1))
			self->s.skinnum = 0;	
		else
			self->s.skinnum = 2;
	}

	if (!self->s.scale)
	{
		self->s.scale = self->monsterinfo.scale = MODEL_SCALE;
	}

	self->materialtype = MAT_FLESH;


	//FIXME (somewhere: otherenemy should be more than just *one* kind
	self->monsterinfo.otherenemyname = "monster_box";

	if(self->spawnflags & MSF_WANDER)
	{
		QPostMessage(self, MSG_WALK, PRI_DIRECTIVE, NULL);
	}
	else if(self->spawnflags & MSF_PELF_CINEMATIC)
	{
		self->svflags|=SVF_FLOAT;
		self->monsterinfo.c_mode = 1;
//		self->takedamage = DAMAGE_NO;
		QPostMessage(self, MSG_C_IDLE1, PRI_DIRECTIVE, "iiige",0,0,0,NULL,NULL);
	}
	else
	{
		QPostMessage(self,MSG_STAND,PRI_DIRECTIVE, NULL);
	}

	if(self->spawnflags&MSF_FIXED || self->spawnflags&MSF_PELF_MISSILE)
	{
//		if(!self->melee_range)
			self->melee_range = 0;

		if(!self->missile_range)
			self->missile_range = 512;
		
//		if(!self->min_missile_range)
			self->min_missile_range = 0;

		if(!self->bypass_missile_chance)
			self->bypass_missile_chance = 60;

		self->s.fmnodeinfo[MESH__HOE].flags |= FMNI_NO_DRAW;
		self->s.fmnodeinfo[MESH__GAFF].flags |= FMNI_NO_DRAW;
		self->s.fmnodeinfo[MESH__HAMMER].flags |= FMNI_NO_DRAW;
		self->s.fmnodeinfo[MESH__HANDLE].flags |= FMNI_NO_DRAW;

		self->monsterinfo.aiflags |= AI_NO_MELEE;
	}
	else
	{
		//turn on/off the weapons that aren't used
		chance = irand(0, 3);
		if(chance < 1)
		{
			//show the hammer
			self->s.fmnodeinfo[MESH__HOE].flags |= FMNI_NO_DRAW;
			self->s.fmnodeinfo[MESH__GAFF].flags |= FMNI_NO_DRAW;
		}
		else if(chance < 2)
		{
			//show the hoe
			self->s.fmnodeinfo[MESH__HAMMER].flags |= FMNI_NO_DRAW;
			self->s.fmnodeinfo[MESH__GAFF].flags |= FMNI_NO_DRAW;
		}
		else
		{
			//show the gaff (that hook thingie)
			self->s.fmnodeinfo[MESH__HAMMER].flags |= FMNI_NO_DRAW;
			self->s.fmnodeinfo[MESH__HOE].flags |= FMNI_NO_DRAW;
		}

		self->monsterinfo.aiflags |= AI_NO_MISSILE;
	}

	self->monsterinfo.supporters = -1;

	//set up my mood function
	MG_InitMoods(self);
	self->svflags |= SVF_WAIT_NOTSOLID;

	if(!stricmp(self->classname, "monster_palace_plague_guard_invisible"))
	{
		self->melee_range = -64;
		self->min_missile_range = 30;
		self->bypass_missile_chance = 80;
	}
}

//

/*QUAKED monster_palace_plague_guard (1 .5 0) (-17 -25 -1) (22 12 63) AMBUSH ASLEEP WALKING CINEMATIC Missile 32 64 FIXED WANDER MELEE_LEAD STALK COWARD EXTRA1 EXTRA2 EXTRA3 EXTRA4

Can fire 2 ranged attacks, has a new skin, toucgher, has armor?

The plagueElf 

AMBUSH - Will not be woken up by other monsters or shots from player

ASLEEP - will not appear until triggered

WALKING - use WANDER instead

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
mintel					= 16
melee_range				= 0
missile_range			= 512
min_missile_range		= 0
bypass_missile_chance	= 60
jump_chance				= 50
wakeup_distance			= 1024

NOTE: A value of zero will result in defaults, if you actually want zero as the value, use -1
*/
/*-------------------------------------------------------------------------
	SP_monster_palace_plague_guard
-------------------------------------------------------------------------*/
void SP_monster_palace_plague_guard (edict_t *self)
{
	if(!self->health)
		self->health = PLAGUEELF_HEALTH * 2;

	self->spawnflags |= MSF_PELF_MISSILE;

	if (!self->s.scale)
		self->monsterinfo.scale = self->s.scale = flrand(1.0, 1.3);

	SP_monster_plagueElf(self);

	self->s.skinnum = PALACE_ELF_SKIN;
}

/*QUAKED monster_palace_plague_guard_invisible (1 .5 0) (-17 -25 -1) (22 12 63) AMBUSH ASLEEP WALKING CINEMATIC Missile 32 64 FIXED WANDER MELEE_LEAD STALK COWARD EXTRA1 EXTRA2 EXTRA3 StartVisible

Can fire 2 ranged attacks, has a new skin, toucgher, has armor?

Is invisible unless firing or hit

The plagueElf 

AMBUSH - Will not be woken up by other monsters or shots from player

ASLEEP - will not appear until triggered

WALKING - use WANDER instead

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
mintel					= 16
melee_range				= -64
missile_range			= 512
min_missile_range		= 30
bypass_missile_chance	= 80
jump_chance				= 50
wakeup_distance			= 1024

NOTE: A value of zero will result in defaults, if you actually want zero as the value, use -1
*/
/*-------------------------------------------------------------------------
	SP_monster_palace_plague_guard_invisible
-------------------------------------------------------------------------*/
void SP_monster_palace_plague_guard_invisible (edict_t *self)
{
	if(!self->health)
		self->health = PLAGUEELF_HEALTH * 2;

	self->spawnflags |= MSF_PELF_MISSILE;

	SP_monster_plagueElf(self);

	self->s.skinnum = PALACE_ELF_SKIN;

	self->s.color.c = 0xFFFFFFFF;

	if(!(self->spawnflags&MSF_EXTRA4))//these guys start visible
		pelf_init_phase_out(self);

}