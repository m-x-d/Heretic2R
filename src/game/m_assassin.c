//==============================================================================
//
// m_assassin.c
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
#include "g_playstats.h"
#include "m_assassin.h"
#include "m_assassin_anim.h"
#include "g_HitLocation.h"
#include "g_misc.h"
#include "angles.h"
#include "c_ai.h"
#include "p_anim_branch.h"
#include "p_anims.h"
#include "m_stats.h"
#include "p_actions.h"

qboolean clear_visible (edict_t *self, edict_t *other);
void extrapolateFiredir (edict_t *self,vec3_t p1,float pspeed,edict_t *targ,float accept,vec3_t vec2);
void create_assassin_dagger(edict_t *Arrow);
void assassinInitDeCloak (edict_t *self);

/*----------------------------------------------------------------------
  assassin Base Info
-----------------------------------------------------------------------*/
static animmove_t *animations[ NUM_ANIMS] =
{
	&assassin_move_daggerl,// = {14, assassin_frames_daggerl, assassin_pause},
	&assassin_move_daggerr,//= {15, assassin_frames_daggerr, assassin_pause},
	&assassin_move_daggerb,// = {15, assassin_frames_daggerb, assassin_pause},
	&assassin_move_daggerc,// = {11, assassin_frames_daggerc, assassin_pause},
	&assassin_move_newdagger,
	&assassin_move_newdaggerb,
	&assassin_move_backflip,// = {16, assassin_frames_backflip, assassin_pause},
	&assassin_move_frontflip,// = {16, assassin_frames_frontflip, assassin_pause},
	&assassin_move_dodge_right,// = {10, assassin_frames_dodge_right, assassin_pause},
	&assassin_move_dodge_left,// = {10, assassin_frames_dodge_left, assassin_pause},
	&assassin_move_deatha,// = {14, assassin_frames_deatha, assassin_dead},
	&assassin_move_deathb,// = {14, assassin_frames_deathb, assassin_dead},
	&assassin_move_jump,// = {17, assassin_frames_jump, assassin_pause},
	&assassin_move_run,// = {10, assassin_frames_run, assassin_pause},
	&assassin_move_pain1,// = {5, assassin_frames_pain1, assassin_pause},
	&assassin_move_pain2,// = {4, assassin_frames_pain2, assassin_pause},
	&assassin_move_delay,// = {1, assassin_frames_delay, assassin_pause},
	&assassin_move_stand,
	&assassin_move_crouch,
	&assassin_move_uncrouch,
	&assassin_move_evade_jump,
	&assassin_move_evade_backflip,
	&assassin_move_evade_frontflip,
	&assassin_move_inair,
	&assassin_move_land,
	&assassin_move_forcedjump,
	&assassin_move_fjump,
	&assassin_move_bfinair,
	&assassin_move_bfland,
	&assassin_move_ffinair,
	&assassin_move_ffland,
	&assassin_move_evinair,
	&assassin_move_teleport,
	&assassin_move_cloak,
	&assassin_move_walk,
	&assassin_move_walk_loop,
	&assassin_move_backspring,
//crouches
	&assassin_move_crouch_trans,
	&assassin_move_crouch_idle,
	&assassin_move_crouch_look_right,
	&assassin_move_crouch_look_right_idle,
	&assassin_move_crouch_look_l2r,
	&assassin_move_crouch_look_left,
	&assassin_move_crouch_look_left_idle,
	&assassin_move_crouch_look_r2l,
	&assassin_move_crouch_look_r2c,
	&assassin_move_crouch_look_l2c,
	&assassin_move_crouch_poke,
	&assassin_move_crouch_end,

	// Cinematic Anims
	&assassin_move_c_idle1,
	&assassin_move_c_run1,
	&assassin_move_c_attack1,
	&assassin_move_c_attack2,
};

static int Sounds[NUM_SOUNDS];
static ClassResourceInfo_t resInfo;


/*-------------------------------------------------------------------------
	assassin_c_anims
-------------------------------------------------------------------------*/
void assassin_c_anims(edict_t *self, G_Message_t *msg)
{
	int int_msg;
	int curr_anim;

	ai_c_readmessage(self, msg);
	int_msg = (int) msg->ID;

	self->monsterinfo.c_anim_flag = 0; 

	switch(int_msg)
	{
		case MSG_C_ATTACK1:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_ATTACK1;
			break;
		case MSG_C_ATTACK2:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_ATTACK2;
			break;
		case MSG_C_IDLE1:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_IDLE1;
			break;
		case MSG_C_RUN1:
			self->monsterinfo.c_anim_flag |= C_ANIM_MOVE;
			curr_anim = ANIM_C_RUN1;
			break;
		default:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_IDLE1;
			break;
	} 

	SetAnim(self, curr_anim);
}



/*----------------------------------------------------------------------
  Action Functions for the monster
-----------------------------------------------------------------------*/

void assassinApplyJump (edict_t *self)
{
	self->jump_time = level.time + 2;
	VectorCopy(self->movedir, self->velocity);
	VectorNormalize(self->movedir);
//	gi.dprintf("Jump velocity will be: %4.2f %4.2f %4.2f\n", self->velocity[0], self->velocity[1], self->velocity[2]);
	self->monsterinfo.aiflags &= ~AI_OVERRIDE_GUIDE;
}

void assassin_jump(edict_t *self, G_Message_t *msg)
{
//	gi.dprintf("Assassin Jumping from AI RUN!\n");
	SetAnim(self, ANIM_FORCED_JUMP);
	self->monsterinfo.aiflags |= AI_OVERRIDE_GUIDE;
}

edict_t *AssassinArrowReflect(edict_t *self, edict_t *other, vec3_t vel)
{
	edict_t	*Arrow;

	Arrow = G_Spawn();

	create_assassin_dagger(Arrow);
	Arrow->s.modelindex = self->s.modelindex;
	VectorCopy(self->s.origin, Arrow->s.origin);
	Arrow->owner = other;
	Arrow->enemy = self->owner;
	Arrow->nextthink=self->nextthink;
	VectorScale(self->avelocity, -0.5, Arrow->avelocity);
	VectorCopy(vel, Arrow->velocity);
	VectorNormalize2(vel, Arrow->movedir);
	AnglesFromDir(Arrow->movedir, Arrow->s.angles);
	Arrow->reflect_debounce_time = self->reflect_debounce_time -1;
	Arrow->reflected_time=self->reflected_time;

	gi.CreateEffect(&Arrow->s,
				FX_M_EFFECTS,
				0,
				Arrow->avelocity,
				"bv",
				FX_ASS_DAGGER,
				Arrow->velocity);

	G_LinkMissile(Arrow); 

	G_SetToFree(self);

	gi.CreateEffect(&Arrow->s, FX_LIGHTNING_HIT, CEF_OWNERS_ORIGIN, NULL, "t", vel);

	return(Arrow);
}


// The blocked function isn't currently used.
/*
void assassinDaggerBlocked (edict_t *self, trace_t *trace)
{
	float damage;
	vec3_t	hitangles;
	edict_t *Other;
	Other = trace->ent;

	if(Other==self->owner||Other->owner == self->owner)
	{
		return;
	}

	// are we reflecting ?
	if(EntReflecting(trace->ent, true, true))
	{
		Create_rand_relect_vect(self->velocity, self->velocity);
		Vec3ScaleAssign(ASSASSIN_DAGGER_SPEED / 2, self->velocity);
		AssassinArrowReflect(self, Other, self->velocity);

		return;
	}

	if(trace->surface&&(trace->surface->flags&SURF_SKY))
	{
		SkyFly(self);
		return;
	}

//take into account if angle is within 45 of 0?
	if(Other->takedamage)
	{
		if(Other->materialtype == MAT_FLESH||Other->client)
			gi.sound(self,CHAN_AUTO,Sounds[SND_DAGHITF],1,ATTN_NORM,0);
		else
			gi.sound(self,CHAN_AUTO,Sounds[SND_DAGHITW],1,ATTN_NORM,0);
		if(skill->value < 2)
			damage = flrand(ASSASSIN_MIN_DAMAGE * 0.5, ASSASSIN_MAX_DAMAGE * 0.5);
		else
		{
			damage = flrand(ASSASSIN_MIN_DAMAGE,ASSASSIN_MAX_DAMAGE);
			if(Q_fabs(self->s.angles[PITCH])<45)//up to extra 10 pts damage if pointed correctly
				damage += 45/(45 - Q_fabs(self->s.angles[PITCH])) * 10;
		}
		T_Damage(Other,self,self->owner,self->movedir,self->s.origin,trace->plane.normal, damage, 0, 0,MOD_DIED);
	}
	else//spark
	{
		if(Vec3NotZero(trace->plane.normal))
			vectoangles(trace->plane.normal, hitangles);
		else
			VectorSet(hitangles, 0, 0, 90);
		gi.CreateEffect(NULL, FX_SPARKS, 0, self->s.origin, "d", hitangles);
		gi.sound(self,CHAN_AUTO,Sounds[SND_DAGHITW],1,ATTN_NORM,0);
	}

	G_FreeEdict(self);
}
*/

void assassinDaggerTouch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surface)
{
	float damage;
	vec3_t	hitangles, normal;

	if(other==self->owner||other->owner == self->owner)
	{
		return;
	}

	// are we reflecting ?
	if(EntReflecting(other, true, true) && self->reflect_debounce_time)
	{
		Create_rand_relect_vect(self->velocity, self->velocity);
		Vec3ScaleAssign(ASSASSIN_DAGGER_SPEED / 2, self->velocity);
		AssassinArrowReflect(self, other, self->velocity);

		return;
	}

	if(surface&&(surface->flags&SURF_SKY))
	{
		SkyFly(self);
		return;
	}

//take into account if angle is within 45 of 0?
	VectorSet(normal, 0, 0, 1);
	if(plane)
	{
		if(plane->normal)
		{
			VectorCopy(plane->normal, normal);
		}
	}

	if(other->takedamage)
	{
		if(other->materialtype == MAT_FLESH||other->client)
			gi.sound(self,CHAN_AUTO,Sounds[SND_DAGHITF],1,ATTN_NORM,0);
		else
			gi.sound(self,CHAN_AUTO,Sounds[SND_DAGHITW],1,ATTN_NORM,0);
		damage = flrand(ASSASSIN_MIN_DAMAGE,ASSASSIN_MAX_DAMAGE);
		if(skill->value >= 2 && Q_fabs(self->s.angles[PITCH])<45)//up to extra 10 pts damage if pointed correctly AND on hard skill
			damage += 45/(45 - Q_fabs(self->s.angles[PITCH])) * 10;

		T_Damage(other,self,self->owner,self->movedir,self->s.origin,normal, damage, 0, 0,MOD_DIED);
	}
	else//spark
	{
		if(Vec3NotZero(normal))
			vectoangles(normal, hitangles);
		else
			VectorSet(hitangles, 0, 0, 90);
		gi.CreateEffect(NULL, FX_SPARKS, 0, self->s.origin, "d", hitangles);
		gi.sound(self,CHAN_AUTO,Sounds[SND_DAGHITW],1,ATTN_NORM,0);
	}

	G_FreeEdict(self);
}

// create the guts of the dagger
void create_assassin_dagger(edict_t *Arrow)
{
	Arrow->movetype=MOVETYPE_FLYMISSILE;
	Arrow->solid=SOLID_BBOX;
	Arrow->classname="Assassin_Dagger";
	Arrow->touch=assassinDaggerTouch;
	Arrow->gravity = 0.0f;
	Arrow->clipmask=MASK_SHOT;
	Arrow->s.effects |= EF_CAMERA_NO_CLIP;
	Arrow->svflags |= SVF_ALWAYS_SEND;
	Arrow->s.scale = 0.5;
	Arrow->think=G_FreeEdict;//ssithraArrowThink;

	VectorSet(Arrow->mins, -1.0, -1.0, -1.0);	
	VectorSet(Arrow->maxs, 1.0, 1.0, 1.0);
}

void assassinThrowDagger(edict_t *self, float right_ofs)
{//fixme; adjust for up/down
	vec3_t	Forward,check_lead, right, enemy_pos, enemy_dir;//, up;
	edict_t	*Arrow;
	float	   enemy_dist, eta;//, spoo_arc;
	

//	if(self->s.fmnodeinfo[MESH__RIGHTARM].flags&FMNI_NO_DRAW)
//		return;


//	gi.sound(self,CHAN_WEAPON,Sounds[SND_ARROW1],1,ATTN_NORM,0);
	self->monsterinfo.attack_finished = level.time + 0.4;
	Arrow = G_Spawn();

	create_assassin_dagger(Arrow);

	Arrow->reflect_debounce_time = MAX_REFLECT;
	Arrow->nextthink=level.time+3;
	Arrow->enemy=self->enemy;
	Arrow->owner=self;
	VectorCopy(self->enemy->s.origin, enemy_pos);
	enemy_pos[2] += self->enemy->viewheight;

	AngleVectors (self->s.angles, Forward, right, NULL);
	VectorCopy (self->s.origin, Arrow->s.origin);
	Arrow->s.origin[2] += 8;
	VectorMA (Arrow->s.origin, 8, Forward, Arrow->s.origin);
	VectorMA (Arrow->s.origin, right_ofs, right, Arrow->s.origin);
	VectorCopy (self->movedir, Arrow->movedir);
	vectoangles (Forward, Arrow->s.angles);

	extrapolateFiredir (self, Arrow->s.origin, ASSASSIN_DAGGER_SPEED, self->enemy, 0.3, check_lead);

	VectorSubtract(enemy_pos, Arrow->s.origin, enemy_dir);
	enemy_dist = VectorNormalize(enemy_dir);
	if(Vec3IsZero(check_lead))
	{
		if(DotProduct(enemy_dir, Forward)>0.3)
			VectorScale(enemy_dir, ASSASSIN_DAGGER_SPEED, Arrow->velocity);
		else
			VectorScale(Forward, ASSASSIN_DAGGER_SPEED, Arrow->velocity);
	}
	else
	{
		VectorScale(check_lead, ASSASSIN_DAGGER_SPEED, Arrow->velocity);
	}

	VectorCopy(Arrow->velocity, Arrow->movedir);
	VectorNormalize(Arrow->movedir);
	vectoangles(Arrow->movedir, Arrow->s.angles);
	Arrow->s.angles[PITCH] = -90;

	eta = enemy_dist / ASSASSIN_DAGGER_SPEED;//eta

//	gi.dprintf("ETA: %f\n", eta);
	//ideally, spin @1110 degrees in 1 sec
	Arrow->avelocity[PITCH] = -1/eta * (360*3 +30 + flrand(-10,10));
//	gi.dprintf("avel: %f\n", Arrow->avelocity[PITCH]);
//	gi.dprintf("final rotation: %f\n", Arrow->avelocity[PITCH]*eta);
//	gi.dprintf("final angle: %f\n", anglemod(Arrow->s.angles[PITCH]+Arrow->avelocity[PITCH]*eta));

/*
//doesn't make desired effect
	if(right_ofs>0)
		Arrow->s.angles[ROLL] = flrand(0, 35);
	else if(right_ofs<0)
		Arrow->s.angles[ROLL] = flrand(-35, 0);
*/

	gi.CreateEffect(&Arrow->s,
				FX_M_EFFECTS,
				0,
				Arrow->avelocity,
				"bv",
				FX_ASS_DAGGER,
				Arrow->velocity);

	G_LinkMissile(Arrow); 
}

/*-------------------------------------------------------------------------
	assassin dagger
-------------------------------------------------------------------------*/
void assassindagger (edict_t *self, float right_ofs)
{
	vec3_t	v, off, dir, org, ang;
	float	len;
	int		damage;
	int		thrownum = 0;
	
	if(!self->enemy)
	{
		QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
		return;
	}

	if(self->enemy->health<0)
	{
		QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
		return;
	}

	VectorSubtract (self->s.origin, self->enemy->s.origin, v);
	len = VectorLength (v);

	if (len <= (self->maxs[0] + self->enemy->maxs[0] + 56)  )	// A hit
	{	
		if (infront(self, self->enemy))
		{
			if (1)//one or two hands?
				gi.sound (self, CHAN_WEAPON, Sounds[SND_SLASH1], 1, ATTN_NORM, 0);
			else 
				gi.sound (self, CHAN_WEAPON, Sounds[SND_SLASH2], 1, ATTN_NORM, 0);	

			VectorSet(off, 35.0, 0.0, 32.0);
			VectorGetOffsetOrigin(off, self->s.origin, self->s.angles[YAW], org);
			VectorCopy(self->s.angles, ang);
			ang[YAW] += DEGREE_90;
			AngleVectors(ang, dir, NULL, NULL);
			//4 to 8
			damage = irand(ASSASSIN_MIN_DAMAGE, ASSASSIN_MAX_DAMAGE);

			if(skill->value >= 2)
			{
				if(!infront(self->enemy, self))
				{//backstab!
					damage = self->enemy->health + irand(-20, 10);
					if(damage<ASSASSIN_MAX_DAMAGE)
						damage = ASSASSIN_MAX_DAMAGE;
				}
			}

			/*			if(self->s.fmnodeinfo[MESH__HANDLE].flags & FMNI_NO_DRAW)
				damage-=5;
			else if(self->s.fmnodeinfo[MESH__HOE].flags & FMNI_NO_DRAW)
				damage+=5;
			else if(self->s.fmnodeinfo[MESH__GAFF].flags & FMNI_NO_DRAW)
				damage+=7;
			else if(self->s.fmnodeinfo[MESH__HAMMER].flags & FMNI_NO_DRAW)
				damage+=10;*/

			T_Damage (self->enemy, self, self, dir, org, vec3_origin, damage, 0, 0,MOD_DIED);
		}
	}
	else			// A misssss
	{
		if((int)right_ofs & BIT_RKNIFE)
		{//turn off dagger
			thrownum++;
			if(!(self->s.fmnodeinfo[MESH__RKNIFE].flags & FMNI_NO_DRAW))
			{
				self->s.fmnodeinfo[MESH__RKNIFE].flags |= FMNI_NO_DRAW;
				assassinThrowDagger(self, 12);
			}
		}
		
		if((int)right_ofs & BIT_LKNIFE)
		{
			thrownum++;
			if(!(self->s.fmnodeinfo[MESH__LKNIFE].flags & FMNI_NO_DRAW))
			{
				self->s.fmnodeinfo[MESH__LKNIFE].flags |= FMNI_NO_DRAW;
				assassinThrowDagger(self, -12);
			}
		}

		if(thrownum>1)
			gi.sound (self, CHAN_WEAPON, Sounds[SND_THROW2], 1, ATTN_NORM, 0);
		else if(thrownum>0)
			gi.sound (self, CHAN_WEAPON, Sounds[SND_THROW1], 1, ATTN_NORM, 0);
	}
}


void assassin_Touch(edict_t *self, trace_t *trace)
{
	vec3_t	dir;
	float	strength;
	edict_t *other;

	other = trace->ent;

	if(self->health <= 0)
		return;

	if(!trace)
		return;

	if((!self->groundentity||self->groundentity==other) && Vec3NotZero(self->velocity))
	{
		strength = VectorLength(self->velocity);

		if(strength > 50)
		{
			if(movable(other) && (other->svflags&SVF_MONSTER || other->client))
			{

				VectorCopy(self->velocity, dir);
				if(dir[2] < 0)
					dir[2] = 0;
				VectorNormalize(dir);

	//			VectorAdd(other->velocity, dir, other->knockbackvel);

				if(self->s.origin[2]+self->mins[2] > other->s.origin[2] + other->maxs[2] * 0.8)
				{
					if(other->client)
						P_KnockDownPlayer(&other->client->playerinfo);

					gi.sound(self, CHAN_BODY, Sounds[SND_LANDF], 1, ATTN_NORM, 0);
	/*
					VectorMA(other->velocity, strength, dir, other->velocity);
					other->groundentity = NULL;
					other->velocity[2] = 101;
	*/

					if(other->takedamage)
					{
						if(strength>5)
							strength = 5;
						T_Damage(other, self, self, dir, trace->endpos, dir, strength, strength*4, 0,MOD_DIED);
					}

					SetAnim(self, ANIM_EVFRONTFLIP);
				}
			}
		}
		/*
		//backflip off walls!  Too late to implement
		else if(trace->plane)
		{
			if(Vec3NotZero(trace->plane.normal))
			{
				if(trace->plane.normal[2] < 0.75)
				{
					VectorCopy(self->velocity, dir);
					VectorNormalize(dir);
					if(DotProduct(dir, trace->plane.normal) < -0.3)
					{
						SetAnim(self, ANIM_EVBACKFLIP);
					}
				}
			}
		}
		*/
	}
}


/*-------------------------------------------------------------------------
	assassin_dead
-------------------------------------------------------------------------*/
void assassin_dead(edict_t *self)
{
	self->msgHandler = DeadMsgHandler;
	self->deadState = DEAD_DEAD;
	M_EndDeath(self);
}

/*-------------------------------------------------------------------------
	assassin_death
-------------------------------------------------------------------------*/
void assassin_random_death_sound (edict_t *self)
{
	gi.sound(self, CHAN_VOICE, Sounds[SND_DIE1], 1, ATTN_NORM, 0);
}

void assassin_death(edict_t *self, G_Message_t *msg)
{
	int chance;
	
	if(self->monsterinfo.aiflags&AI_DONT_THINK)
	{
		if (irand(0,10) < 5)  // Big enough death to be thrown back
			SetAnim(self, ANIM_DEATHB);//gib?
		else 
			SetAnim(self, ANIM_DEATHA);
		return;
	}

//	gi.dprintf("Dead\n");
	self->msgHandler = DeadMsgHandler;

	if(self->deadflag == DEAD_DEAD) //Dead but still being hit	
		return;

	self->isBlocked = self->bounced = NULL;

	self->deadflag = DEAD_DEAD;

	assassin_dropweapon (self, BIT_LKNIFE|BIT_RKNIFE);

	if(self->health <= -80) //gib death
	{
		gi.sound(self, CHAN_BODY, Sounds[SND_GIB], 1, ATTN_NORM, 0);
		if(irand(0,10)<5)
		{
			self->s.fmnodeinfo[MESH__TORSOFT].flags |= FMNI_NO_DRAW;
			self->s.fmnodeinfo[MESH__TORSOBK].flags |= FMNI_NO_DRAW;
			self->s.fmnodeinfo[MESH__HEAD].flags |= FMNI_NO_DRAW;
			self->s.fmnodeinfo[MESH__R4ARM].flags |= FMNI_NO_DRAW;
			self->s.fmnodeinfo[MESH__L4ARM].flags |= FMNI_NO_DRAW;
			self->s.fmnodeinfo[MESH__KNIFES].flags |= FMNI_NO_DRAW;
			self->s.fmnodeinfo[MESH__LUPARM].flags |= FMNI_NO_DRAW;
			self->s.fmnodeinfo[MESH__RUPARM].flags |= FMNI_NO_DRAW;
			self->s.fmnodeinfo[MESH__LKNIFE].flags |= FMNI_NO_DRAW;
			self->s.fmnodeinfo[MESH__RKNIFE].flags |= FMNI_NO_DRAW;

			SprayDebris(self, self->s.origin, 12, 100);
		}
		else
		{
			self->think = BecomeDebris;
			self->nextthink = level.time + FRAMETIME;
			return;
		}
	}
	else
	{
		assassin_random_death_sound(self);
		self->msgHandler = DyingMsgHandler;
	}
	

	chance = irand(0,10);
	if(chance < 5 && self->health <= -80)
		SetAnim(self, ANIM_DEATHA);
	else
		SetAnim(self, ANIM_DEATHB);

	self->pre_think = NULL;
	self->next_pre_think = -1;

	if(self->s.renderfx & RF_ALPHA_TEXTURE)
		if(self->pre_think != assassinDeCloak)
			assassinInitDeCloak(self);
}


/*-------------------------------------------------------------------------
	assassingrowl
-------------------------------------------------------------------------*/
void assassingrowl (edict_t *self)
{
	int chance;	

	if (!irand(0, 20))
	{
		chance = irand(0, 9);
		if (chance < 3)
			gi.sound (self, CHAN_AUTO, Sounds[SND_GROWL1], 1, ATTN_IDLE, 0);
		else if (chance < 6)
			gi.sound(self, CHAN_AUTO, Sounds[SND_GROWL2], 1, ATTN_IDLE, 0);
		else 
			gi.sound(self, CHAN_AUTO, Sounds[SND_GROWL3], 1, ATTN_IDLE, 0);
	}
}

void assassin_random_attack(edict_t *self)
{
	int chance;

	chance = irand(0,3);


	if((chance < 1&&!(self->s.fmnodeinfo[MESH__L4ARM].flags&FMNI_NO_DRAW)) ||
		(self->s.fmnodeinfo[MESH__R4ARM].flags&FMNI_NO_DRAW&&!(self->s.fmnodeinfo[MESH__L4ARM].flags&FMNI_NO_DRAW)) )
	{
		SetAnim(self, ANIM_DAGGERL);		
	}
	else if((chance < 2&&!(self->s.fmnodeinfo[MESH__R4ARM].flags&FMNI_NO_DRAW)) ||
		(!(self->s.fmnodeinfo[MESH__R4ARM].flags&FMNI_NO_DRAW)&&self->s.fmnodeinfo[MESH__L4ARM].flags&FMNI_NO_DRAW) )
	{
		if(irand(0, 1))
			SetAnim(self, ANIM_DAGGERR);		
		else
			SetAnim(self, ANIM_NEWDAGGER);		
	}
	else if(!(self->s.fmnodeinfo[MESH__R4ARM].flags&FMNI_NO_DRAW)&&
		!(self->s.fmnodeinfo[MESH__L4ARM].flags&FMNI_NO_DRAW))
	{
		if(irand(0, 1))
			SetAnim(self, ANIM_DAGGERB);
		else
			SetAnim(self, ANIM_NEWDAGGERB);		
	}
	else
	{
//		gi.dprintf("Can't attack!  Run away!\n");
		self->monsterinfo.aiflags |= AI_COWARD;
		SetAnim(self, ANIM_RUN);
	}
}

/*-------------------------------------------------------------------------
	assassin_melee
-------------------------------------------------------------------------*/
void assassin_melee(edict_t *self, G_Message_t *msg)
{
	if (M_ValidTarget(self, self->enemy))
	{
		if(!irand(0,7))
		{
			/*if(self->s.renderfx & RF_ALPHA_TEXTURE)
			{
				if(self->pre_think != assassinDeCloak)
				{
					gi.sound(self,CHAN_AUTO,Sounds[SND_DECLOAK],1,ATTN_NORM,0);
					self->pre_think = assassinDeCloak;
					self->next_pre_think = level.time + FRAMETIME;
					assassin_pause(self);
					return;
				}
			}*/
			
			if(assassinCheckTeleport(self, ASS_TP_OFF))
			{
//				gi.dprintf("melee->teleport\n");
				return;//try to get away
			}
		}
		assassin_random_attack(self);
	}
	else
		QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
}

void assassin_missile(edict_t *self, G_Message_t *msg)
{
	if (M_ValidTarget(self, self->enemy))
	{
		if(!irand(0, 10))
		{//10% chance try special action
			if(!irand(0, 2))
			{//25% chance teleport
				if(assassinCheckTeleport(self, ASS_TP_OFF))
				{
//					gi.dprintf("missile->teleport\n");
					return;
				}
			}
			else if(!(self->s.renderfx & RF_ALPHA_TEXTURE))
			{//75% cloak	
				if(!(self->spawnflags&MSF_ASS_NOSHADOW))
				{	
					SetAnim(self, ANIM_CLOAK);
					return;
				}
			}//else uncloak - unncloak when die
		}
		//need to check for if behind player- diff behaviour- get close and backstab?
		assassin_random_attack(self);
	}
	else
		QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
}
/*-------------------------------------------------------------------------
	assassin_pain
-------------------------------------------------------------------------*/
void assassin_post_pain (edict_t *self)
{
	/*if(self->s.renderfx & RF_ALPHA_TEXTURE)
	{
		if(!irand(0,4))
		{
			if(self->pre_think != assassinDeCloak)
			{
				gi.sound(self,CHAN_AUTO,Sounds[SND_DECLOAK],1,ATTN_NORM,0);
				self->pre_think = assassinDeCloak;
				self->next_pre_think = level.time + FRAMETIME;
				assassin_pause(self);
				return;
			}
		}
	}*/
	if(self->fire_damage_time < level.time)//don't teleport if burning
	{
		if(assassinCheckTeleport(self, ASS_TP_ANY))
		{
//			gi.dprintf("pain->teleport\n");
			return;
		}
	}

	assassin_pause(self);
}

int Bit_for_MeshNode_as [16] =
{
	BIT_DADDYNULL,	
	BIT_TORSOFT,	
	BIT_TORSOBK,	
	BIT_HEAD,		
	BIT_LKNIFE,	
	BIT_RKNIFE,	
	BIT_R4ARM,		
	BIT_L4ARM,		
	BIT_HIPS,		
	BIT_LCALF,		
	BIT_RCALF,	
	BIT_RTHIGH,	
	BIT_LTHIGH,	
	BIT_KNIFES,	
	BIT_LUPARM,	
	BIT_RUPARM	
};

qboolean canthrownode_as (edict_t *self, int BP, int *throw_nodes)
{//see if it's on, if so, add it to throw_nodes
	//turn it off on thrower
	if(!(self->s.fmnodeinfo[BP].flags & FMNI_NO_DRAW))
	{
		*throw_nodes |= Bit_for_MeshNode_as[BP];
		self->s.fmnodeinfo[BP].flags |= FMNI_NO_DRAW;
		return true;
	}
	return false;
}

//THROWS weapon, turns off those nodes, sets that weapon as gone
void assassin_dropweapon (edict_t *self, int whichknives)
{//NO PART FLY FRAME!
	vec3_t handspot, right;

	AngleVectors(self->s.angles,NULL,right,NULL);

	if(!(self->s.fmnodeinfo[MESH__LKNIFE].flags & FMNI_NO_DRAW)&&whichknives & BIT_LKNIFE)
	{
		VectorClear(handspot);
		VectorMA(handspot, -12, right, handspot);
		ThrowWeapon(self, &handspot, BIT_LKNIFE, 0, FRAME_prtfly);//FRAME_atakc3);
		self->s.fmnodeinfo[MESH__LKNIFE].flags |= FMNI_NO_DRAW;
	}
	if(!(self->s.fmnodeinfo[MESH__RKNIFE].flags & FMNI_NO_DRAW)&&whichknives & BIT_RKNIFE)
	{
		VectorClear(handspot);
		VectorMA(handspot, 12, right, handspot);
		ThrowWeapon(self, &handspot, BIT_RKNIFE, 0, FRAME_prtfly);//FRAME_atakc3);
		self->s.fmnodeinfo[MESH__RKNIFE].flags |= FMNI_NO_DRAW;
	}
}

int assassin_convert_hitloc_dead(int hl)
{
	switch(hl)
	{
		case hl_Head:
			return hl_TorsoFront;
			break;
		
		case hl_TorsoFront://split in half?
			if(!irand(0,1))
				return hl_LegUpperRight;
			else
				return hl_LegUpperLeft;
			break;
		
		case hl_TorsoBack://split in half?
			return hl_Head;
			break;
		
		case hl_ArmUpperLeft:
			return hl_ArmLowerLeft;
			break;
		
		case hl_ArmLowerLeft://left arm
			return hl_ArmUpperLeft;
			break;
		
		case hl_ArmUpperRight:
			return hl_ArmLowerRight;
			break;
		
		case hl_ArmLowerRight://right arm
			return hl_ArmUpperRight;
			break;

		case hl_LegUpperLeft:
			return hl_LegLowerLeft;
			break;
		
		case hl_LegLowerLeft://left leg
			return hl_LegUpperLeft;
			break;
		
		case hl_LegUpperRight:
			return hl_LegLowerRight;
			break;
		
		case hl_LegLowerRight://right leg
			return hl_LegUpperRight;
			break;

		default:
			return irand(hl_Head, hl_LegLowerRight);
			break;
	}

}

void assassin_dismember(edict_t *self, int damage, int HitLocation)
{
	int			throw_nodes = 0;
	vec3_t		gore_spot, right;
	qboolean	dismember_ok = false;

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

	if(self->health>0)
	{
		if(self->curAnimID==ANIM_DAGGERL||(self->curAnimID==ANIM_DAGGERB&&irand(0,2)<1))
		{//Hit chest during melee, may have hit arms
			if(HitLocation == hl_TorsoFront&&irand(0,10)<4)
			{
				if(irand(0,10)<7)
					HitLocation = hl_ArmLowerRight;
				else
					HitLocation = hl_ArmLowerLeft;
			}
		}

		if(self->curAnimID==ANIM_DAGGERR||self->curAnimID==ANIM_DAGGERC||(self->curAnimID==ANIM_DAGGERB&&irand(0,2)<1))
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
			(HitLocation == hl_ArmUpperLeft&& self->s.fmnodeinfo[MESH__LUPARM].flags & FMNI_NO_DRAW) ||
			(HitLocation == hl_ArmUpperRight&& self->s.fmnodeinfo[MESH__RUPARM].flags & FMNI_NO_DRAW)||
			(
				(HitLocation == hl_TorsoFront|| HitLocation == hl_TorsoBack) &&
				self->s.fmnodeinfo[MESH__LUPARM].flags & FMNI_NO_DRAW &&
				self->s.fmnodeinfo[MESH__RUPARM].flags & FMNI_NO_DRAW &&
				irand(0,10)<4)
			)
			HitLocation = hl_Head;//Decap
	}
	else
		HitLocation = assassin_convert_hitloc_dead(HitLocation);

	VectorClear(gore_spot);
	switch(HitLocation)
	{
		case hl_Head:
			if(self->s.fmnodeinfo[MESH__HEAD].flags & FMNI_NO_DRAW)
				break;
			if(self->s.fmnodeinfo[MESH__HEAD].flags & FMNI_USE_SKIN)
				damage*=1.5;//greater chance to cut off if previously damaged
			if(flrand(0,self->health)<damage*0.3&&dismember_ok)
			{
				canthrownode_as(self, MESH__HEAD,&throw_nodes);

				gore_spot[2]+=18;
				ThrowBodyPart(self, &gore_spot, throw_nodes, damage, FRAME_prtfly);

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
			if(self->s.fmnodeinfo[MESH__TORSOFT].flags & FMNI_NO_DRAW)
				break;
			if(self->s.fmnodeinfo[MESH__TORSOFT].flags & FMNI_USE_SKIN)
				damage*=1.5;//greater chance to cut off if previously damaged
			if(flrand(0,self->health)<damage*0.3&&dismember_ok)
			{
				gore_spot[2]+=12;

				canthrownode_as(self, MESH__TORSOFT,&throw_nodes);
				canthrownode_as(self, MESH__TORSOBK,&throw_nodes);
				canthrownode_as(self, MESH__HEAD,&throw_nodes);
				canthrownode_as(self, MESH__R4ARM,&throw_nodes);
				canthrownode_as(self, MESH__L4ARM,&throw_nodes);
				canthrownode_as(self, MESH__KNIFES,&throw_nodes);
				canthrownode_as(self, MESH__LUPARM,&throw_nodes);
				canthrownode_as(self, MESH__RUPARM,&throw_nodes);

				assassin_dropweapon (self, BIT_LKNIFE|BIT_RKNIFE);
				ThrowBodyPart(self, &gore_spot, throw_nodes, damage, FRAME_torsofly);
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
//				if(flrand(0,self->health)<damage*0.5)
//					assassin_dropweapon (self, (int)damage);
				self->s.fmnodeinfo[MESH__TORSOFT].flags |= FMNI_USE_SKIN;			
				self->s.fmnodeinfo[MESH__TORSOFT].skin = self->s.skinnum+1;
			}
			break;
		case hl_TorsoBack://split in half?
			if(self->s.fmnodeinfo[MESH__TORSOBK].flags & FMNI_NO_DRAW)
				break;
			if(self->s.fmnodeinfo[MESH__TORSOBK].flags & FMNI_USE_SKIN)
				damage*=1.5;//greater chance to cut off if previously damaged
			if(flrand(0,self->health)<damage*0.3&&dismember_ok)
			{
				gore_spot[2]+=12;

				canthrownode_as(self, MESH__TORSOFT,&throw_nodes);
				canthrownode_as(self, MESH__TORSOBK,&throw_nodes);
				canthrownode_as(self, MESH__HEAD,&throw_nodes);
				canthrownode_as(self, MESH__R4ARM,&throw_nodes);
				canthrownode_as(self, MESH__L4ARM,&throw_nodes);
				canthrownode_as(self, MESH__KNIFES,&throw_nodes);
				canthrownode_as(self, MESH__LUPARM,&throw_nodes);
				canthrownode_as(self, MESH__RUPARM,&throw_nodes);

				assassin_dropweapon (self, BIT_LKNIFE|BIT_RKNIFE);
				ThrowBodyPart(self, &gore_spot, throw_nodes, damage, FRAME_torsofly);
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
//				if(flrand(0,self->health)<damage*0.5)
//					assassin_dropweapon (self, (int)damage);
				self->s.fmnodeinfo[MESH__TORSOBK].flags |= FMNI_USE_SKIN;			
				self->s.fmnodeinfo[MESH__TORSOBK].skin = self->s.skinnum+1;
			}
			break;

		case hl_ArmUpperLeft:
			if(self->s.fmnodeinfo[MESH__LUPARM].flags & FMNI_NO_DRAW)
				break;
			if(self->s.fmnodeinfo[MESH__LUPARM].flags & FMNI_USE_SKIN)
				damage*=1.5;//greater chance to cut off if previously damaged
//			if(flrand(0,self->health)<damage*0.4)
//				assassin_dropweapon (self, (int)damage);
			if(flrand(0,self->health)<damage*0.75&&dismember_ok)
			{
				canthrownode_as(self, MESH__L4ARM, &throw_nodes);
				if(canthrownode_as(self, MESH__LUPARM, &throw_nodes))
				{
					AngleVectors(self->s.angles,NULL,right,NULL);
					gore_spot[2]+=self->maxs[2]*0.3;
					VectorMA(gore_spot,-10,right,gore_spot);
					assassin_dropweapon (self, BIT_LKNIFE);
					ThrowBodyPart(self, &gore_spot, throw_nodes, damage, FRAME_prtfly);
				}
			}
			else
			{
				self->s.fmnodeinfo[MESH__LUPARM].flags |= FMNI_USE_SKIN;			
				self->s.fmnodeinfo[MESH__LUPARM].skin = self->s.skinnum+1;
			}
			break;
		case hl_ArmLowerLeft://left arm
			if(self->s.fmnodeinfo[MESH__L4ARM].flags & FMNI_NO_DRAW)
				break;
			if(self->s.fmnodeinfo[MESH__L4ARM].flags & FMNI_USE_SKIN)
				damage*=1.5;//greater chance to cut off if previously damaged
			if(flrand(0,self->health)<damage*0.75&&dismember_ok)
			{
				if(canthrownode_as(self, MESH__L4ARM, &throw_nodes))
				{
					AngleVectors(self->s.angles,NULL,right,NULL);
					gore_spot[2]+=self->maxs[2]*0.3;
					VectorMA(gore_spot,-10,right,gore_spot);
					assassin_dropweapon (self, BIT_LKNIFE);
					ThrowBodyPart(self, &gore_spot, throw_nodes, damage, FRAME_prtfly);
				}
			}
			else
			{
				self->s.fmnodeinfo[MESH__L4ARM].flags |= FMNI_USE_SKIN;			
				self->s.fmnodeinfo[MESH__L4ARM].skin = self->s.skinnum+1;
			}
			break;
		case hl_ArmUpperRight:
			if(self->s.fmnodeinfo[MESH__RUPARM].flags & FMNI_NO_DRAW)
				break;
			if(flrand(0,self->health)<damage*0.75&&dismember_ok)
			{
				canthrownode_as(self, MESH__R4ARM, &throw_nodes);
				if(canthrownode_as(self, MESH__RUPARM, &throw_nodes))
				{
					AngleVectors(self->s.angles,NULL,right,NULL);
					gore_spot[2]+=self->maxs[2]*0.3;
					VectorMA(gore_spot,10,right,gore_spot);
					assassin_dropweapon (self, BIT_RKNIFE);
					ThrowBodyPart(self, &gore_spot, throw_nodes, damage, FRAME_prtfly);
				}
			}
			else
			{
				self->s.fmnodeinfo[MESH__RUPARM].flags |= FMNI_USE_SKIN;			
				self->s.fmnodeinfo[MESH__RUPARM].skin = self->s.skinnum+1;
			}
			break;
		case hl_ArmLowerRight://right arm
			if(self->s.fmnodeinfo[MESH__R4ARM].flags & FMNI_NO_DRAW)
				break;
			if(flrand(0,self->health)<damage*0.75&&dismember_ok)
			{
				if(canthrownode_as(self, MESH__R4ARM, &throw_nodes))
				{
					AngleVectors(self->s.angles,NULL,right,NULL);
					gore_spot[2]+=self->maxs[2]*0.3;
					VectorMA(gore_spot,10,right,gore_spot);
					assassin_dropweapon (self, BIT_RKNIFE);
					ThrowBodyPart(self, &gore_spot, throw_nodes, damage, FRAME_prtfly);
				}
			}
			else
			{
				self->s.fmnodeinfo[MESH__R4ARM].flags |= FMNI_USE_SKIN;			
				self->s.fmnodeinfo[MESH__R4ARM].skin = self->s.skinnum+1;
			}
			break;

		case hl_LegUpperLeft:
			if(self->health>0)
			{
				if(self->s.fmnodeinfo[MESH__LTHIGH].flags & FMNI_USE_SKIN)
					break;
				self->s.fmnodeinfo[MESH__LTHIGH].flags |= FMNI_USE_SKIN;			
				self->s.fmnodeinfo[MESH__LTHIGH].skin = self->s.skinnum+1;
				break;
			}
			else
			{
				if(self->s.fmnodeinfo[MESH__LTHIGH].flags & FMNI_NO_DRAW)
					break;
				canthrownode_as(self, MESH__LCALF, &throw_nodes);
				if(canthrownode_as(self, MESH__LTHIGH, &throw_nodes))
				{
					AngleVectors(self->s.angles,NULL,right,NULL);
					gore_spot[2]+=self->maxs[2]*0.3;
					VectorMA(gore_spot,-10,right,gore_spot);
					ThrowBodyPart(self, &gore_spot, throw_nodes, damage, 0);
				}
				break;
			}
		case hl_LegLowerLeft://left leg
			if(self->health>0)
			{
				if(self->s.fmnodeinfo[MESH__LCALF].flags & FMNI_USE_SKIN)
					break;
				self->s.fmnodeinfo[MESH__LCALF].flags |= FMNI_USE_SKIN;			
				self->s.fmnodeinfo[MESH__LCALF].skin = self->s.skinnum+1;
				break;
			}
			else
			{
				if(self->s.fmnodeinfo[MESH__LCALF].flags & FMNI_NO_DRAW)
					break;
				if(canthrownode_as(self, MESH__LCALF, &throw_nodes))
				{
					AngleVectors(self->s.angles,NULL,right,NULL);
					gore_spot[2]+=self->maxs[2]*0.3;
					VectorMA(gore_spot,-10,right,gore_spot);
					ThrowBodyPart(self, &gore_spot, throw_nodes, damage, 0);
				}
				break;
			}

		case hl_LegUpperRight:
			if(self->health>0)
			{
				if(self->s.fmnodeinfo[MESH__RTHIGH].flags & FMNI_USE_SKIN)
					break;
				self->s.fmnodeinfo[MESH__RTHIGH].flags |= FMNI_USE_SKIN;
				self->s.fmnodeinfo[MESH__RTHIGH].skin = self->s.skinnum+1;
				break;
			}
			else
			{
				if(self->s.fmnodeinfo[MESH__RTHIGH].flags & FMNI_NO_DRAW)
					break;
				canthrownode_as(self, MESH__RCALF, &throw_nodes);
				if(canthrownode_as(self, MESH__RTHIGH, &throw_nodes))
				{
					AngleVectors(self->s.angles,NULL,right,NULL);
					gore_spot[2]+=self->maxs[2]*0.3;
					VectorMA(gore_spot,-10,right,gore_spot);
					ThrowBodyPart(self, &gore_spot, throw_nodes, damage, 0);
				}
				break;
			}
		case hl_LegLowerRight://right leg
			if(self->health>0)
			{
				if(self->s.fmnodeinfo[MESH__RCALF].flags & FMNI_USE_SKIN)
					break;
				self->s.fmnodeinfo[MESH__RCALF].flags |= FMNI_USE_SKIN;
				self->s.fmnodeinfo[MESH__RCALF].skin = self->s.skinnum+1;
				break;
			}
			else
			{
				if(self->s.fmnodeinfo[MESH__RCALF].flags & FMNI_NO_DRAW)
					break;
				if(canthrownode_as(self, MESH__RCALF, &throw_nodes))
				{
					AngleVectors(self->s.angles,NULL,right,NULL);
					gore_spot[2]+=self->maxs[2]*0.3;
					VectorMA(gore_spot,-10,right,gore_spot);
					ThrowBodyPart(self, &gore_spot, throw_nodes, damage, 0);
				}
				break;
			}

		default:
			if(flrand(0,self->health)<damage*0.25)
				assassin_dropweapon (self, (int)damage);
			break;
	}

	if(self->s.fmnodeinfo[MESH__L4ARM].flags&FMNI_NO_DRAW&&
		self->s.fmnodeinfo[MESH__R4ARM].flags&FMNI_NO_DRAW)			
	{
		self->monsterinfo.aiflags |= AI_COWARD;
		self->monsterinfo.aiflags |= AI_NO_MELEE;
		self->monsterinfo.aiflags |= AI_NO_MISSILE;
	}
//	gi.dprintf(" done\n");
}

void assassin_dismember_msg(edict_t *self, G_Message_t *msg)
{//fixme: throw current weapon
//fixme - make part fly dir the vector from hit loc to sever loc
//remember- turn on caps!
	int				damage;
	HitLocation_t	HitLocation;

	ParseMsgParms(msg, "ii", &damage, &HitLocation);
	assassin_dismember(self, damage, HitLocation);
}

void assassin_dead_pain (edict_t *self, G_Message_t *msg)
{
	if(msg)
		if(!(self->svflags & SVF_PARTS_GIBBED))
			assassin_dismember_msg(self, msg);
}

void assassin_random_pain_sound (edict_t *self)
{
	if(irand(0,1))
		gi.sound(self, CHAN_VOICE, Sounds[SND_PAIN1], 1, ATTN_NORM, 0);
	else
		gi.sound(self, CHAN_VOICE, Sounds[SND_PAIN2], 1, ATTN_NORM, 0);
}

void assassin_pain(edict_t *self, G_Message_t *msg)
{
	edict_t			*attacker, *inflictor;
	int				damage, temp;
	qboolean		force_pain;
	
	if(self->curAnimID == ANIM_TELEPORT)
		return;

	ParseMsgParms(msg, "eeiii", &inflictor, &attacker, &force_pain, &damage, &temp);

	if(inflictor == attacker || !stricmp(inflictor->classname, "Spell_RedRain")||!stricmp(inflictor->classname, "Spell_Hellbolt"))
	{//melee hit or contant effect, don't stick around!
		if(!(self->spawnflags&MSF_ASS_NOTELEPORT)&&
			!(self->spawnflags&MSF_FIXED)&&
			self->groundentity)
		{
			if(assassinChooseTeleportDestination(self, ASS_TP_ANY, true, false))
				return;
		}
	}
				
	if(!force_pain)
		if(flrand(0,self->health)>damage)
			return;

	self->monsterinfo.aiflags &= ~AI_OVERRIDE_GUIDE;

	if(!self->maxs[2])
		assassinUndoCrouched (self);

	assassin_random_pain_sound(self);

	if (self->pain_debounce_time < level.time||force_pain)
	{
		self->pain_debounce_time = level.time + skill->value + 2;
		SetAnim(self, ANIM_PAIN2);
		if(irand(0, 10) > skill->value)
		{
			self->monsterinfo.misc_debounce_time = level.time + 3;//3 seconds before can re-cloak
			assassinInitDeCloak(self);
		}
	}
}

void assassinSkipFrameSkillCheck (edict_t *self)
{
	if(irand(0, 3) < skill->value)
		self->s.frame++;
}

/*-------------------------------------------------------------------------
	assassin_pause
-------------------------------------------------------------------------*/
void assassin_pause (edict_t *self)
{
	vec3_t	v;
	float	len;

//this gets stuck on, sometimes
	self->s.fmnodeinfo[MESH__LKNIFE].flags |= FMNI_NO_DRAW;
	self->s.fmnodeinfo[MESH__RKNIFE].flags |= FMNI_NO_DRAW;

	if(self->monsterinfo.aiflags&AI_OVERRIDE_GUIDE)
		return;

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
			QPostMessage(self, MSG_RUN, PRI_DIRECTIVE, NULL);
			break;
		case AI_MOOD_STAND:
			QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
			break;

		case AI_MOOD_WALK:
			QPostMessage(self, MSG_WALK, PRI_DIRECTIVE, NULL);
			break;

		case AI_MOOD_DELAY:
			SetAnim(self, ANIM_DELAY);
			break;

		case AI_MOOD_WANDER:
			if(self->spawnflags&MSF_FIXED)
			{
				SetAnim(self, ANIM_DELAY);
				return;
			}
			QPostMessage(self, MSG_WALK, PRI_DIRECTIVE, NULL);
			break;

		case AI_MOOD_JUMP:
			if(self->spawnflags&MSF_FIXED)
				SetAnim(self, ANIM_DELAY);
			else
				SetAnim(self, ANIM_FJUMP);//fjump will apply the movedir when he's ready
			break;

		default :
#ifdef _DEVEL
			gi.dprintf("assassin: Unusable mood %d!\n", self->ai_mood);
#endif
			break;
		}
	}
}

void assassinChooseJumpAmbush(edict_t *self)
{
	float	dot;
	vec3_t	forward, enemy_dir;

	if(!self->enemy)
	{
		if(irand(0, 10)<4)
			SetAnim(self, ANIM_JUMP);
		else
			SetAnim(self, ANIM_FRONTFLIP);
		return;
	}

	AngleVectors(self->s.angles, forward, NULL, NULL);
	VectorSubtract(self->enemy->s.origin, self->s.origin, enemy_dir);
	VectorNormalize(enemy_dir);
	dot = DotProduct(forward, enemy_dir);
	if(dot<0)
	{//behind
		SetAnim(self, ANIM_BACKFLIP);
		return;
	}

	if(!irand(0, 3))
		SetAnim(self, ANIM_JUMP);
	else
		SetAnim(self, ANIM_FRONTFLIP);
	return;
}

qboolean assassinChooseSideJumpAmbush(edict_t *self)
{//OR: turn and jump?
	float	dot;
	vec3_t	right, enemy_dir;

	if(!self->enemy)
		return false;

	AngleVectors(self->s.angles, NULL, right, NULL);
	VectorSubtract(self->enemy->s.origin, self->s.origin, enemy_dir);
	VectorNormalize(enemy_dir);

	dot = DotProduct(right, enemy_dir);
	if(dot>0)
		VectorScale(right, 300, self->movedir);
	else
		VectorScale(right, -300, self->movedir);

	self->movedir[2] = 200;
	SetAnim(self, ANIM_FJUMP);
	return true;
}
/*-------------------------------------------------------------------------
	assassin_run
-------------------------------------------------------------------------*/
void assassin_run(edict_t *self, G_Message_t *msg)
{
	if(!self->enemy&&self->spawnflags&MSF_WANDER)
	{
		SetAnim(self, ANIM_RUN);
		return;
	}

	if(self->spawnflags&MSF_ASS_STARTSHADOW)//decloak
	{//FIXME: should I wait until infront of enemy and visible to him?
		self->spawnflags &= ~MSF_ASS_STARTSHADOW;
		assassinInitCloak(self);
	}

	if(!(self->spawnflags&MSF_FIXED))
	{
		if(self->spawnflags&MSF_ASS_JUMPAMBUSH)//jump out
		{
			self->spawnflags &= ~MSF_ASS_JUMPAMBUSH;
			assassinChooseJumpAmbush(self);
			return;
		}

		if(self->spawnflags&MSF_ASS_SIDEJUMPAMBUSH)//side-jump out
		{
			self->spawnflags &= ~MSF_ASS_SIDEJUMPAMBUSH;
			if(assassinChooseSideJumpAmbush(self))
				return;
		}
	}

	if(self->curAnimID >= ANIM_CROUCH_IDLE && self->curAnimID < ANIM_CROUCH_END)
	{
		SetAnim(self, ANIM_CROUCH_END);
		return;
	}

	if (M_ValidTarget(self, self->enemy))
	{
		if(!irand(0, 7))
		{
			if(assassinCheckTeleport(self, ASS_TP_OFF))
			{
//				gi.dprintf("run->teleport\n");
				return;
			}
			else if(!irand(0, 3))
			{
				if(!(self->s.renderfx & RF_ALPHA_TEXTURE))
				{	
					if(!(self->spawnflags&MSF_ASS_NOSHADOW))
					{	
						SetAnim(self, ANIM_CLOAK);
						return;
					}
				}//else uncloak - unncloak when die
			}
		}
		if(!(self->spawnflags&MSF_FIXED))
			SetAnim(self, ANIM_RUN);
		else
			SetAnim(self, ANIM_DELAY);
	}
	else
		QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
}


void assassin_go_run(edict_t *self, float dist)
{
	if(!self->maxs[2])
		assassinUndoCrouched (self);

	if(self->enemy)
		ai_run(self, dist);
	else
		ai_walk(self, dist);
}
/*----------------------------------------------------------------------
  assassin runorder - order the assassin to choose an run animation
-----------------------------------------------------------------------*/
void assassin_runorder(edict_t *self)
{
	QPostMessage(self, MSG_RUN, PRI_DIRECTIVE, NULL);
}


/*-------------------------------------------------------------------------
	assassinsqueal
-------------------------------------------------------------------------*/
void assassinsqueal (edict_t *self)
{
/*
	if(irand(0, 1))
		gi.sound(self, CHAN_WEAPON, Sounds[SND_PAIN1], 1, ATTN_NORM, 0);
	else
		gi.sound(self, CHAN_WEAPON, Sounds[SND_PAIN2], 1, ATTN_NORM, 0);
*/
}

/*-------------------------------------------------------------------------
	assassin_stand
-------------------------------------------------------------------------*/
void assassin_stand(edict_t *self, G_Message_t *msg)
{	
	if (self->ai_mood == AI_MOOD_DELAY)
		SetAnim(self, ANIM_DELAY);
	else
	{
		if(self->s.renderfx & RF_ALPHA_TEXTURE)
		{
			if(self->pre_think != assassinDeCloak)
				assassinInitDeCloak(self);
		}
		SetAnim(self, ANIM_STAND);
	}

	return;
}


/*-------------------------------------------------------------------------
	assassin_walk
-------------------------------------------------------------------------*/
void assassin_crouch_idle_decision (edict_t *self)
{//FIXME: need to uncrouch
	int	chance;

	chance = irand(0, 100);
	switch(self->curAnimID)
	{
	case ANIM_CROUCH_IDLE:
		if(chance < 55)
			SetAnim(self, ANIM_CROUCH_IDLE);
		else if(chance < 75)
			SetAnim(self, ANIM_CROUCH_POKE);
		else if(chance < 85)
			SetAnim(self, ANIM_CROUCH_LOOK_RIGHT);
		else if(chance < 95)
			SetAnim(self, ANIM_CROUCH_LOOK_LEFT);
		else
			SetAnim(self, ANIM_CROUCH_END);
		break;
	
	case ANIM_CROUCH_LOOK_RIGHT:
	case ANIM_CROUCH_LOOK_RIGHT_IDLE:
	case ANIM_CROUCH_LOOK_L2R:
		if(chance < 60)
			SetAnim(self, ANIM_CROUCH_LOOK_RIGHT_IDLE);
		else if(chance < 85)
			SetAnim(self, ANIM_CROUCH_LOOK_R2C);
		else
			SetAnim(self, ANIM_CROUCH_LOOK_R2L);
		break;
	
	case ANIM_CROUCH_LOOK_LEFT:
	case ANIM_CROUCH_LOOK_LEFT_IDLE:
	case ANIM_CROUCH_LOOK_R2L:
		if(chance < 60)
			SetAnim(self, ANIM_CROUCH_LOOK_LEFT_IDLE);
		else if(chance < 85)
			SetAnim(self, ANIM_CROUCH_LOOK_L2C);
		else
			SetAnim(self, ANIM_CROUCH_LOOK_L2R);
		break;
	
	case ANIM_CROUCH_TRANS:
		self->monsterinfo.pausetime = 99999999;
		SetAnim(self, ANIM_CROUCH_IDLE);
		break;

	case ANIM_CROUCH_LOOK_R2C:
	case ANIM_CROUCH_LOOK_L2C:
	case ANIM_CROUCH_POKE:
		SetAnim(self, ANIM_CROUCH_IDLE);
		break;

	case ANIM_CROUCH_END:
		self->damage_debounce_time = level.time + 10;
		self->monsterinfo.pausetime = -1;
		SetAnim(self, ANIM_STAND);
		break;
	
	default:
		SetAnim(self, ANIM_CROUCH_END);
		break;
	}
}

void assassin_ai_walk (edict_t *self, float dist)
{
	if(self->damage_debounce_time < level.time)
	{
		if(self->enemy)
		{
			if(vhlen(self->s.origin, self->enemy->s.origin) < 48 && infront(self, self->enemy))
			{
				assassinNodeOn (self, MESH__LKNIFE);
				SetAnim(self, ANIM_CROUCH_TRANS);
				return;
			}
		}
		else if(self->oldenemy)
		{
			if(vhlen(self->s.origin, self->oldenemy->s.origin) < 48 && infront(self, self->oldenemy))
			{
				assassinNodeOn (self, MESH__LKNIFE);
				SetAnim(self, ANIM_CROUCH_TRANS);
				return;
			}
		}
	}
	ai_walk(self, dist);
}

void assassin_walk(edict_t *self, G_Message_t *msg)
{
	if(self->spawnflags&MSF_FIXED)
	{
		SetAnim(self, ANIM_DELAY);
		return;
	}
	if(self->curAnimID == ANIM_WALK_LOOP)
		SetAnim(self, ANIM_WALK_LOOP);
	else
		SetAnim(self, ANIM_WALK);
	return;	
}

void assasin_walk_loop_go (edict_t *self)
{
	SetAnim(self, ANIM_WALK_LOOP);
}
//=============================================================

// EVASION

//=============================================================

void assassinDodgeLeft (edict_t *self)
{
	SetAnim(self, ANIM_DODGE_LEFT);
}

void assassinDodgeRight (edict_t *self)
{
	SetAnim(self, ANIM_DODGE_RIGHT);
}

void assassinFrontFlip (edict_t *self)
{
	SetAnim(self, ANIM_EVFRONTFLIP);
}

void assassinBackFlip (edict_t *self)
{
	SetAnim(self, ANIM_EVBACKFLIP);
}

void assassinBackSprings (edict_t *self)
{
	SetAnim(self, ANIM_BACKSPRING);
}

void assassinJump (edict_t *self)
{
	SetAnim(self, ANIM_EVJUMP);
}

void assassinCrouch (edict_t *self)
{
	SetAnim(self, ANIM_CROUCH);
}

void assassinCrouchedAttack (edict_t *self)
{
	SetAnim(self, ANIM_DAGGERC);
}

void assassin_evade (edict_t *self, G_Message_t *msg)
{
	edict_t			*projectile;
	HitLocation_t	HitLocation;
	int duck_chance, dodgeleft_chance, dodgeright_chance, jump_chance, backflip_chance, frontflip_chance;
	int chance;
	float eta;

	if(!self->groundentity)
		return;

	ParseMsgParms(msg, "eif", &projectile, &HitLocation, &eta);
	
	if(eta < 2)
		self->evade_debounce_time = level.time + eta;
	else
		self->evade_debounce_time = level.time + 2;

	if(skill->value || self->spawnflags & MSF_ASS_TELEPORTDODGE)
	{//Pussies were complaining about assassins teleporting away from certain death, so don't do that unless in hard
		if(!stricmp(projectile->classname, "Spell_PhoenixArrow") ||
			!stricmp(projectile->classname, "Spell_FireWall") ||
			!stricmp(projectile->classname, "Spell_SphereOfAnnihilation") ||
			!stricmp(projectile->classname, "Spell_Maceball"))
		{
			if(assassinChooseTeleportDestination(self, ASS_TP_OFF, true, true))
				return;
		}
	}

	switch(HitLocation)
	{
		case hl_Head:
			duck_chance = 95;
			dodgeleft_chance = 50;
			dodgeright_chance = 50;
			jump_chance = 0;
			backflip_chance = 20;
			frontflip_chance = 20;
		break;
		case hl_TorsoFront://split in half?
			duck_chance = 85;
			dodgeleft_chance = 40;
			dodgeright_chance = 40;
			jump_chance = 0;
			backflip_chance = 60;
			frontflip_chance = 0;
		break;
		case hl_TorsoBack://split in half?
			duck_chance = 80;
			dodgeleft_chance = 40;
			dodgeright_chance = 40;
			jump_chance = 0;
			backflip_chance = 0;
			frontflip_chance = 60;
		break;
		case hl_ArmUpperLeft:
			duck_chance = 75;
			dodgeleft_chance = 0;
			dodgeright_chance = 90;
			jump_chance = 0;
			backflip_chance = 20;
			frontflip_chance = 20;
		break;
		case hl_ArmLowerLeft://left arm
			duck_chance = 75;
			dodgeleft_chance = 0;
			dodgeright_chance = 80;
			jump_chance = 30;
			backflip_chance = 20;
			frontflip_chance = 20;
		break;
		case hl_ArmUpperRight:
			duck_chance = 60;
			dodgeleft_chance = 90;
			dodgeright_chance = 0;
			jump_chance = 0;
			backflip_chance = 20;
			frontflip_chance = 20;
		break;
		case hl_ArmLowerRight://right arm
			duck_chance = 20;
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
			jump_chance = 80;
			backflip_chance = 40;
			frontflip_chance = 40;
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
			jump_chance = 80;
			backflip_chance = 40;
			frontflip_chance = 40;
		break;
		default:
			duck_chance = 20;
			dodgeleft_chance = 10;
			dodgeright_chance = 10;
			jump_chance = 10;
			backflip_chance = 10;
			frontflip_chance = 10;
		break;
	}

	if(irand(0, 100) < skill->value * 10)
	{
		if(self->pre_think != assassinCloak)
			assassinInitCloak(self);
	}

	chance = irand(0, 10);
	if(skill->value || self->spawnflags & MSF_ASS_TELEPORTDODGE)
	{//Pussies were complaining about assassins teleporting away from certain death, so don't do that unless in hard
		if(chance > 8 && !(self->spawnflags&MSF_ASS_NOTELEPORT))
			if(assassinChooseTeleportDestination(self, ASS_TP_DEF, false, false))
			{
//				gi.dprintf("Assassin teleport evade\n");
				return;
			}
	}

	chance = irand(0, 100);
	if(chance < frontflip_chance)
	{
//		gi.dprintf("Assassin fflip evade\n");
		assassinFrontFlip(self);
		return;
	}

	chance = irand(0, 100);
	if(chance < backflip_chance)
	{
		if(self->curAnimID == ANIM_RUN && irand(0, 3))//running, do the front flip
		{
//			gi.dprintf("Assassin fflip evade\n");
			assassinFrontFlip(self);
		}
		else
		{
			if(irand(0, 1))
			{
//				gi.dprintf("Assassin bspring evade\n");
				assassinBackSprings(self);
			}
			else
			{
//				gi.dprintf("Assassin bflip evade\n");
				assassinBackFlip(self);
			}
		}
		return;
	}

	chance = irand(0, 100);
	if(chance < duck_chance)
	{
		self->evade_debounce_time = level.time + eta + 2 - skill->value;
//		gi.dprintf("Assassin crouch evade\n");
		assassinCrouch(self);
		return;
	}

	chance = irand(0, 100);
	if(chance < dodgeleft_chance)
	{
//		gi.dprintf("Assassin dleft evade\n");
		assassinDodgeLeft(self);
		return;
	}
	
	chance = irand(0, 100);
	if(chance < dodgeright_chance)
	{
//		gi.dprintf("Assassin dright evade\n");
		assassinDodgeRight(self);
		return;
	}
	
	chance = irand(0, 100);
	if(chance < jump_chance)
	{
		if(self->curAnimID == ANIM_RUN && irand(0, 4))//running, do the front flip
		{
//			gi.dprintf("Assassin fflip evade\n");
			assassinFrontFlip(self);
		}
		else
		{
//			gi.dprintf("Assassin jump evade\n");
			assassinJump(self);
		}
		return;
	}

	if(skill->value || self->spawnflags & MSF_ASS_TELEPORTDODGE)
	{//Pussies were complaining about assassins teleporting away from certain death, so don't do that unless in hard
		if(!(self->spawnflags&MSF_ASS_NOTELEPORT))
			if(assassinChooseTeleportDestination(self, ASS_TP_DEF, false, false))
			{
//				gi.dprintf("Assassin tport(desperate) evade\n");
				return;
			}
	}

	self->evade_debounce_time = 0;
//	gi.dprintf("Assassin failed to evade\n");
}

void assassinCrouchedCheckAttack (edict_t *self, float attack)
{
	if(!clear_visible(self, self->enemy))
		return;

	if(!infront(self, self->enemy))
		return;

	if(irand(0,10)<5)
		return;

	if(attack == true)
		assassindagger(self, BIT_RKNIFE);
	else if(attack == 2)//start crouched attack anim
		SetAnim(self, ANIM_DAGGERC);
	else//loop back inside that anim
		self->monsterinfo.currframeindex = 2;
}

void assassinNodeOn (edict_t *self, float node)
{
	self->s.fmnodeinfo[(int)node].flags &= ~FMNI_NO_DRAW;
}

void assassinStop (edict_t *self)
{
	if(self->evade_debounce_time - level.time> 0.1f)
		self->nextthink = level.time + (self->evade_debounce_time - level.time);
}

void assassinSetCrouched (edict_t *self)
{
	VectorSet (self->maxs, 16, 16, 0);	
	self->viewheight = 0;
}

void assassinUndoCrouched (edict_t *self)
{
	VectorSet (self->maxs, 16, 16, 48);	
	self->viewheight = 40;
}

void assassin_sound(edict_t *self, float channel, float soundnum, float attn)
{
	gi.sound (self, (int)(channel), Sounds[(int)(soundnum)], 1, (int)(attn), 0);
}

void assassinGoJump (edict_t *self, float fwdspd,float upspd,float rtspd)
{//fixme: do checks and traces first
	vec3_t up, forward, right;

	self->monsterinfo.aiflags &= ~AI_OVERRIDE_GUIDE;
	assassin_sound(self, CHAN_VOICE, SND_JUMP, ATTN_NORM);
	AngleVectors(self->s.angles, forward, right, up);
	
	VectorMA(self->velocity, upspd, up, self->velocity);
	VectorMA(self->velocity, fwdspd, forward, self->velocity);
	VectorMA(self->velocity, rtspd, right, self->velocity);
}

void assassin_go_inair(edict_t *self)
{
	SetAnim(self, ANIM_INAIR);
}

void assassin_go_evinair(edict_t *self)
{
	SetAnim(self, ANIM_EVINAIR);
}

void assassin_go_ffinair(edict_t *self)
{
	SetAnim(self, ANIM_FFINAIR);
}

void assassin_go_bfinair(edict_t *self)
{
	SetAnim(self, ANIM_BFINAIR);
}

void assassinCheckLoop (edict_t *self, float frame)
{//see if should fire again
	vec3_t	v;
	float	len, melee_range, min_seperation, jump_range;

	if(!self->enemy)
		return;

	ai_charge2(self, 0);

	if(!clear_visible(self, self->enemy))
		return;

	if(!infront(self, self->enemy))
		return;

	if(irand(0, 100) < self->bypass_missile_chance)
	{
		self->monsterinfo.attack_finished = level.time + 3 - skill->value;
		return;
	}

	if(self->ai_mood_flags&AI_MOOD_FLAG_BACKSTAB)
		return;

	VectorSubtract (self->s.origin, self->enemy->s.origin, v);
	len = VectorLength (v);
	melee_range = 64;
	jump_range = 128;
	min_seperation = self->maxs[0] + self->enemy->maxs[0];

	if (infront(self, self->enemy))
	{//don't loop if enemy close enough
		if (len < min_seperation + melee_range)
			return;
		else if (len < min_seperation + jump_range && irand(0,10)<3)
			return;
	}

	self->monsterinfo.currframeindex = (int)frame;
}

void assassinSmoke(edict_t *self)
{
	vec3_t pos;
	VectorCopy(self->s.origin, pos);
	pos[2]+=self->mins[2];
	gi.CreateEffect(NULL, FX_TPORTSMOKE, 0, pos, "");//, "db", hitangles, 5);
	//gi.CreateEffect(&self->s, FX_DUST_PUFF, CEF_OWNERS_ORIGIN, self->s.origin, NULL);
}

void assassinGone(edict_t *self)
{
	vec3_t enemy_dir;

	if(self->placeholder)
		G_FreeEdict(self->placeholder);

	VectorCopy(self->pos2, self->s.origin);
	
	if(self->enemy)
	{//face enemy
		VectorSubtract(self->enemy->s.origin, self->s.origin, enemy_dir);
		self->s.angles[YAW] = anglemod(vectoyaw(enemy_dir));
	}

	assassinSmoke(self);

	VectorCopy(self->pos2, enemy_dir);//reuse
	enemy_dir[2] += 100;
	if(gi.pointcontents(enemy_dir) == CONTENTS_EMPTY&&!irand(0,3))
		assassinFrontFlip(self);
	else
		SetAnim(self, ANIM_UNCROUCH);

	self->monsterinfo.aiflags &= ~AI_OVERRIDE_GUIDE;
	self->svflags &= ~SVF_NO_AUTOTARGET;

	//dumbed down
	self->touch_debounce_time = level.time + (10 - skill->value*3);

	//Should we clear velocity too?
	//VectorClear(self->velocity);
	gi.linkentity(self);
}

void assassinPrepareTeleportDest(edict_t *self, vec3_t spot, qboolean instant)
{
	if(self->s.renderfx & RF_ALPHA_TEXTURE)
	{
		if(self->pre_think != assassinDeCloak)
		{
			assassinInitDeCloak(self);
			self->monsterinfo.misc_debounce_time = level.time + 3;
		}
	}

	VectorCopy(spot, self->pos2);

	self->placeholder = G_Spawn();
	VectorCopy(self->pos2, self->placeholder->s.origin);
	self->placeholder->solid = SOLID_BBOX;
	VectorCopy(self->mins, self->placeholder->mins);
	VectorCopy(self->maxs, self->placeholder->maxs);
	self->placeholder->think = G_FreeEdict;
	self->placeholder->nextthink = level.time + 2;//just in case

	//dumbed down
	if(instant && skill->value > 1)
	{
		assassinReadyTeleport(self);
		assassinGone(self);
	}
	else
	{
		SetAnim(self, ANIM_TELEPORT);
	}
}

qboolean assassinChooseTeleportDestination(edict_t *self, int type, qboolean imperative, qboolean instant)
{//FIXME: don't teleport into area with red rain or ripper balls!
	vec3_t	teleport_angles, forward, endpos, startpos;
	trace_t trace;
	int	chance, num_tries, i;
	edict_t	*noblockent;
	float	tracedist;

	//Instead of chance, do around self if evade, around other if ambush

	if(!self->enemy)//fixme- choose my spot?
		return false;

	if(self->spawnflags&MSF_FIXED)
		return false;

	if(imperative)
		num_tries = (skill->value + 1) * 10;
	else
		num_tries = 1;

	for(i = 0; i < num_tries; i++)
	{
		switch(type)
		{
		case ASS_TP_OFF:
			chance = irand(0, 66);
			break;
		case ASS_TP_ANY:
			chance = irand(0, 100);
			break;
		case ASS_TP_DEF:
			chance = irand(33, 100);
			break;
		}

		if(chance<33)
		{//ANY, OFF to behind enemy
			VectorSet(teleport_angles, 0, anglemod(self->enemy->s.angles[YAW] + flrand(-90, 90)), 0);
			AngleVectors(teleport_angles, forward, NULL, NULL);
			VectorCopy(self->enemy->s.origin, startpos);
			startpos[2]+=self->enemy->mins[2];
			startpos[2]-=self->mins[2];
			tracedist = irand(self->min_missile_range, self->missile_range);
			VectorMA(startpos, -tracedist, forward, endpos);
			noblockent = self->enemy;
		}
		else if(chance<66)
		{//ANY to anywhere around enemy
			VectorSet(teleport_angles, 0, anglemod(flrand(0, 360)), 0);
			AngleVectors(teleport_angles, forward, NULL, NULL);
			VectorCopy(self->enemy->s.origin, startpos);
			startpos[2]+=self->enemy->mins[2];
			startpos[2]-=self->mins[2];
			tracedist = irand(self->min_missile_range, self->missile_range);
			VectorMA(startpos, -tracedist, forward, endpos);
			noblockent = self->enemy;
		}
		else
		{//ANY, DEF to anywhere around me
			VectorSet(teleport_angles, 0, anglemod(flrand(0, 360)), 0);
			AngleVectors(teleport_angles, forward, NULL, NULL);
			VectorCopy(self->s.origin, startpos);
			tracedist = irand(self->min_missile_range, self->missile_range/2);
			VectorMA(startpos, -tracedist, forward, endpos);
			noblockent = self;
		}
		
		gi.trace(startpos, self->mins, self->maxs, endpos, noblockent, MASK_MONSTERSOLID,&trace);
		
		if(trace.fraction*tracedist < 100)//min origin lerp dist
			continue;

		if(trace.allsolid || trace.startsolid)
			continue;
		
		if(vhlen(trace.endpos, self->enemy->s.origin)>=self->min_missile_range)
		{
			VectorCopy(trace.endpos, startpos);
			VectorCopy(trace.endpos, endpos);
			endpos[2] -=64;
			gi.trace(startpos, self->mins, self->maxs, endpos, noblockent, MASK_MONSTERSOLID,&trace);
			if(trace.fraction<1.0 && !trace.allsolid && !trace.startsolid)//the last two should be false if trace.fraction is < 1.0 but doesn't hurt to check
			{
				assassinPrepareTeleportDest(self, trace.endpos, instant);
				return true;
			}
		}
	}
	return false;
}

void assassinReadyTeleport (edict_t *self)
{
	assassinSmoke(self);
	self->svflags |= SVF_NO_AUTOTARGET;
}

qboolean assassinCheckTeleport (edict_t *self, int type)
{
	if(self->spawnflags&MSF_ASS_NOTELEPORT)
		return false;

	if(self->spawnflags&MSF_FIXED)
		return false;

	if(!self->groundentity)
		return false;

	if(!M_ValidTarget(self, self->enemy))
		return false;

/*	if(!infront(self->enemy, self))
		return false;

	if(!visible(self->enemy, self))
		return false;*/

	return assassinChooseTeleportDestination(self, type, false, false);
}

void assassinUnCrouch (edict_t *self)
{
	SetAnim(self, ANIM_UNCROUCH);
}

qboolean assassinCheckCloak (edict_t *self)
{
	int	chance = 0;

	if(!self->monsterinfo.awake)
		return (false);

	if(self->monsterinfo.misc_debounce_time > level.time)//cloak debounce time
		return (false);

	if(self->spawnflags & MSF_ASS_NOSHADOW)
		return (false);

	if(self->ai_mood == AI_MOOD_FLEE)
		return (true);

	if(!self->enemy)
		return (false);

	if(infront(self->enemy, self))
		chance = -3;

	if(irand(0, 10 - skill->value + chance) <= 0)
		return (true);

	return (false);
}

qboolean assassinCheckDeCloak (edict_t *self)
{
	float	dist;
	int		chance = 0;

	if(!self->monsterinfo.awake)
		return (false);

	if(self->monsterinfo.misc_debounce_time > level.time)//cloak debounce time
		return (false);

	if(!self->enemy)
	{
		if(!(self->spawnflags & MSF_ASS_STARTSHADOW))
			return (true);
		return (false);
	}

	dist = M_DistanceToTarget(self, self->enemy);
	if(dist<ASSASSIN_MIN_CLOAK_RANGE)
		return (true);

	if(!infront(self->enemy, self))
		chance = -3;

	if(irand(0, 10 + skill->value * 2 + chance) <= 0)
		return (true);

	return (false);
}

void assassinCloakThink (edict_t *self)
{
	edict_t *found = NULL;
	int		lowerseq, upperseq;
	vec3_t	mins, maxs, startpos, endpos, tport_dest;
	trace_t	trace;

	self->pre_think = assassinCloakThink;
	self->next_pre_think = level.time + FRAMETIME;
//check cloak or decloak
	if(!(self->s.renderfx & RF_ALPHA_TEXTURE))
	{//not cloaked
		if(assassinCheckCloak(self))
		{
			self->monsterinfo.misc_debounce_time = level.time + 7;//10 seconds before will willingly uncloak
			assassinInitCloak(self);
		}
	}
	else
	{//cloaked
		if(assassinCheckDeCloak(self))
			assassinInitDeCloak(self);
	}
//check to teleport

	//dumbed down
	if(!skill->value)//was < 2 
	{
		if(self->touch_debounce_time > level.time)
		{
			return;
		}
	}

	if(self->waterlevel == 3 && self->air_finished <= level.time)//going to drown!
	{//pick either last buoy or my startspot
		VectorCopy(self->pos1, tport_dest);
		if(self->lastbuoy>NULL_BUOY)
		{
			if(!(gi.pointcontents(level.buoy_list[self->lastbuoy].origin) & MASK_WATER))
				VectorCopy(level.buoy_list[self->lastbuoy].origin, tport_dest);
		}

		VectorCopy(tport_dest, startpos);
		VectorCopy(self->mins, mins);
		mins[2] = 0;
		VectorCopy(self->maxs, maxs);
		maxs[2] = 1;
		startpos[2] -= self->size[2];

		gi.trace(startpos, mins, maxs, endpos, self, MASK_MONSTERSOLID,&trace);
		if(!trace.allsolid && !trace.startsolid)
		{
			VectorCopy(trace.endpos, startpos);
			VectorCopy(trace.endpos, endpos);
			startpos[2] +=self->size[2];
			gi.trace(startpos, self->mins, self->maxs, endpos, self, MASK_MONSTERSOLID,&trace);
			if(trace.fraction == 1.0 && !trace.allsolid && !trace.startsolid)
			{
				assassinPrepareTeleportDest(self, trace.endpos, false);
				return;
			}
		}
	}
	
	if(skill->value || self->spawnflags & MSF_ASS_TELEPORTDODGE)
	{//Pussies were complaining about assassins teleporting away from certain death, so don't do that unless in hard
		if(!(self->spawnflags & MSF_ASS_NOTELEPORT) && !(self->spawnflags&MSF_FIXED) && self->groundentity)
		{
			if(irand(0, 4 - skill->value) <= 0)
			{//easy is 40% chance per second, hard is 60% chance to check per second
				while(found = findradius(found, self->s.origin, 200 + skill->value * 50))
				{
					if(!stricmp(found->classname, "Spell_Maceball"))
					{
						if(!self->enemy)
						{
							if(found->owner)
							{
								self->enemy = found->owner;
								FoundTarget(self, false);
							}
						}
						if(assassinChooseTeleportDestination(self, ASS_TP_OFF, true, true))
							return;
					}

					if(!stricmp(found->classname, "Spell_RedRain") ||
						!stricmp(found->classname, "Spell_PhoenixArrow") ||
						!stricmp(found->classname, "Spell_FireWall") ||
						!stricmp(found->classname, "Spell_SphereOfAnnihilation"))
					{
						if(!self->enemy)
						{
							if(found->owner)
							{
								self->enemy = found->owner;
								FoundTarget(self, false);
							}
						}
						if(assassinChooseTeleportDestination(self, ASS_TP_ANY, true, false))
							return;
					}

					if(found==self->enemy && found->client)
					{
						if(M_DistanceToTarget(self, self->enemy) < 128)
						{
							if(infront(self->enemy, self))
							{
								//is he using his staff or jumping into me?
								lowerseq = found->client->playerinfo.lowerseq;
								switch(lowerseq)
								{
								case ASEQ_WSWORD_SPIN:
								case ASEQ_WSWORD_SPIN2:
								case ASEQ_WSWORD_STEP2:
								case ASEQ_WSWORD_STEP:
								case ASEQ_POLEVAULT2:
								case ASEQ_POLEVAULT1_W:
								case ASEQ_POLEVAULT1_R:
									if(assassinChooseTeleportDestination(self, ASS_TP_ANY, true, true))
										return;
									break;
								default:
									break;
								}

								upperseq = found->client->playerinfo.upperseq;
								switch(upperseq)
								{
								case ASEQ_WSWORD_SPIN:
								case ASEQ_WSWORD_SPIN2:
								case ASEQ_WSWORD_STEP2:
								case ASEQ_WSWORD_STEP:
								case ASEQ_POLEVAULT2:
								case ASEQ_POLEVAULT1_W:
								case ASEQ_POLEVAULT1_R:
									if(assassinChooseTeleportDestination(self, ASS_TP_ANY, true, true))
										return;
									break;
								default:
									break;
								}
							}
					
							if(found->client->playerinfo.shield_timer > level.time)
							{
								if(assassinChooseTeleportDestination(self, ASS_TP_OFF, true, true))
									return;
							}
						}

					}
				}
			}
		}
	}
	
	if(self->evade_debounce_time < level.time)
		MG_CheckEvade(self);
}

void assassinCloak (edict_t *self)
{
	int	interval = 15;
	
	if(self->s.color.r > 50)
		self->s.color.r += irand(-interval*3, 0);
	if(self->s.color.g > 50)
		self->s.color.g += irand(-interval*3, 0);
	if(self->s.color.b > 50)
		self->s.color.b += irand(-interval*3, 0);

	if(self->s.color.a > 150)
		self->s.color.a += irand(-interval, 0);

	if(self->s.color.r > 50||
		self->s.color.g > 50||
		self->s.color.b > 50||
		self->s.color.a > 150)
	{
		self->pre_think = assassinCloak;
		self->next_pre_think = level.time + FRAMETIME;
	}
	else
	{
		self->pre_think = assassinCloakThink;
		self->next_pre_think = level.time + FRAMETIME;
	}

	if(self->evade_debounce_time < level.time)
		MG_CheckEvade(self);
}

void assassinDeCloak (edict_t *self)
{
	if(!(self->s.renderfx & RF_ALPHA_TEXTURE))
		return;

	if(self->s.color.r<255 - 10)
		self->s.color.r += 10;
	else
		self->s.color.r = 255;

	if(self->s.color.g<255 - 10)
		self->s.color.g += 10;
	else
		self->s.color.g = 255;
	
	if(self->s.color.b<255 - 10)
		self->s.color.b += 10;
	else
		self->s.color.b = 255;
	
	if(self->s.color.a<255 - 5)
		self->s.color.a += 5;
	else
		self->s.color.a = 255;

	if(self->s.color.r == 255&&
		self->s.color.g == 255&&
		self->s.color.b == 255&&
		self->s.color.a == 255)
	{
		self->svflags &= ~SVF_NO_AUTOTARGET;
		self->s.renderfx &= ~RF_ALPHA_TEXTURE;
		if(self->health > 0)
		{
			self->pre_think = assassinCloakThink;
			self->next_pre_think = level.time + FRAMETIME;
		}
		else
		{
			self->pre_think = NULL;
			self->next_pre_think = -1;
		}
	}
	else
	{
		self->pre_think = assassinDeCloak;
		self->next_pre_think = level.time + FRAMETIME;
	}

	if(self->evade_debounce_time < level.time)
		MG_CheckEvade(self);
}

void assassinInitDeCloak (edict_t *self)
{
	gi.sound(self,CHAN_AUTO,Sounds[SND_DECLOAK],1,ATTN_NORM,0);
	self->pre_think = assassinDeCloak;
	self->next_pre_think = level.time + FRAMETIME;
}

void assassinInitCloak (edict_t *self)
{
	self->s.renderfx |= RF_ALPHA_TEXTURE;
	self->svflags |= SVF_NO_AUTOTARGET;
	gi.sound(self,CHAN_AUTO,Sounds[SND_CLOAK],1,ATTN_NORM,0);
	self->s.color.r = 255;
	self->s.color.g = 255;
	self->s.color.b = 255;
	self->s.color.a = 255;
	self->pre_think = assassinCloak;
	self->next_pre_think = level.time + FRAMETIME;
}

void assassin_check_mood (edict_t *self, G_Message_t *msg)
{
	ParseMsgParms(msg, "i", &self->ai_mood);

	assassin_pause(self);
}
//=============================================================

/*-------------------------------------------------------------------------
	AssassinStaticsInit
-------------------------------------------------------------------------*/
void AssassinStaticsInit()
{
	classStatics[CID_ASSASSIN].msgReceivers[MSG_STAND] = assassin_stand;
	classStatics[CID_ASSASSIN].msgReceivers[MSG_WALK] = assassin_walk;
	classStatics[CID_ASSASSIN].msgReceivers[MSG_RUN] = assassin_run;
	classStatics[CID_ASSASSIN].msgReceivers[MSG_MELEE] = assassin_melee;
	classStatics[CID_ASSASSIN].msgReceivers[MSG_MISSILE] = assassin_missile;
	classStatics[CID_ASSASSIN].msgReceivers[MSG_PAIN] = assassin_pain;
	classStatics[CID_ASSASSIN].msgReceivers[MSG_DEATH] = assassin_death;
	classStatics[CID_ASSASSIN].msgReceivers[MSG_DISMEMBER] = MG_parse_dismember_msg;
	classStatics[CID_ASSASSIN].msgReceivers[MSG_JUMP] = assassin_jump;
	classStatics[CID_ASSASSIN].msgReceivers[MSG_EVADE] = assassin_evade;
	classStatics[CID_ASSASSIN].msgReceivers[MSG_DEATH_PAIN] = assassin_dead_pain;
	classStatics[CID_ASSASSIN].msgReceivers[MSG_CHECK_MOOD] = assassin_check_mood;

	classStatics[CID_ASSASSIN].msgReceivers[MSG_C_IDLE1] = assassin_c_anims;
	classStatics[CID_ASSASSIN].msgReceivers[MSG_C_RUN1] = assassin_c_anims;
	classStatics[CID_ASSASSIN].msgReceivers[MSG_C_ATTACK1] = assassin_c_anims;
	classStatics[CID_ASSASSIN].msgReceivers[MSG_C_ATTACK2] = assassin_c_anims;

	resInfo.numAnims = NUM_ANIMS;
	resInfo.animations = animations;
	
	//note that the name is different in the path
	resInfo.modelIndex = gi.modelindex("models/monsters/assassin/tris.fm");

	Sounds[SND_PAIN1]=gi.soundindex("monsters/assassin/pain1.wav");
	Sounds[SND_PAIN2]=gi.soundindex("monsters/assassin/pain2.wav");
	Sounds[SND_DIE1]=gi.soundindex("monsters/assassin/death1.wav");
	Sounds[SND_GIB]=gi.soundindex("monsters/assassin/gib.wav");
	Sounds[SND_THROW1]=gi.soundindex("monsters/assassin/throw1.wav");
	Sounds[SND_THROW2]=gi.soundindex("monsters/assassin/throw2.wav");
	Sounds[SND_DAGHITF]=gi.soundindex("monsters/assassin/daghitf.wav");
	Sounds[SND_DAGHITW]=gi.soundindex("monsters/assassin/daghitw.wav");
	Sounds[SND_JUMP]=gi.soundindex("monsters/assassin/jump.wav");
	Sounds[SND_FLIP]=gi.soundindex("monsters/assassin/flip.wav");
	Sounds[SND_LAND]=gi.soundindex("monsters/assassin/land.wav");
	Sounds[SND_LANDF]=gi.soundindex("monsters/assassin/landf.wav");
	Sounds[SND_SLIDE]=gi.soundindex("monsters/assassin/slide.wav");
	Sounds[SND_SLASH1]=gi.soundindex("monsters/assassin/slash1.wav");
	Sounds[SND_SLASH2]=gi.soundindex("monsters/assassin/slash2.wav");
	Sounds[SND_GROWL1] = gi.soundindex ("monsters/assassin/growl1.wav");
	Sounds[SND_GROWL2]=gi.soundindex("monsters/assassin/growl2.wav");
	Sounds[SND_GROWL3] = gi.soundindex ("monsters/assassin/growl3.wav");
	Sounds[SND_CLOAK]=gi.soundindex("monsters/assassin/cloak.wav");
	Sounds[SND_DECLOAK] = gi.soundindex ("monsters/assassin/decloak.wav");

	resInfo.numSounds = NUM_SOUNDS;
	resInfo.sounds = Sounds;

	classStatics[CID_ASSASSIN].resInfo = &resInfo;
}

void assassinCheckDefense(edict_t *self, float enemydist, qboolean enemyvis, qboolean enemyinfront)
{
	if(!enemyinfront&&enemyvis&&enemydist<self->melee_range)
	{
#ifdef _DEVEL
		if(assassinCheckTeleport(self, ASS_TP_DEF))
			gi.dprintf("defense->teleport\n");
#else
		assassinCheckTeleport(self, ASS_TP_DEF);
#endif
	}
	else if(!enemyvis && self->monsterinfo.last_successful_enemy_tracking_time + 6 - skill->value < level.time)
	{
		if(irand(0, 10) > 10 - (3 * (skill->value + 1)))//hard = 90%, med is 40%, easy is 30%
		{
#ifdef _DEVEL
			gi.dprintf("Assassin trying to teleport to %s since can't find them...\n", self->classname, self->enemy->classname);
#endif
			assassinCheckTeleport(self, ASS_TP_OFF);
		}
	}
}

/*QUAKED monster_assassin (1 .5 0) (-16 -16 -32) (16 16 48) AMBUSH ASLEEP WALKING FwdJumpAmbush NoCloak NoTeleport CINEMATIC FIXED WANDER MELEE_LEAD STALK COWARD TeleportAmbush CloakAmbush SideJumpAmbush TeleportDodge

The assassin 

SPAWNFLAGS:

AMBUSH - Will not be woken up by other monsters or shots from player

ASLEEP - will not appear until triggered

WALKING - use WANDER instead

FwdJumpAmbush - will jump out front or back when triggered (depending on whether player is in front or behind him)

NoCloak - can't turn into a shadow

NoTeleport - can't use smoke grenades to trick player and teleport

CINEMATIC - puts monster into cinematic mode for scripting

FIXED - Will stand in place and attack from afar.  Never moves.

WANDER - Monster will wander around aimlessly (but follows buoys)

MELEE_LEAD - Monster will tryto cut you off when you're running and fighting him, works well if there are a few monsters in a group, half doing this, half not

STALK - Monster will only approach and attack from behind- if you're facing the monster it will just stand there.  Once the monster takes pain, however, it will stop this behaviour and attack normally

COWARD - Monster starts off in flee mode- runs away from you when woken up

TeleportAmbush - Will teleport into his origin when triggered (before triggered, is not anywhere at all, like "ASLEEP")

CloakAmbush - Start as a shadow and decloak when wakes up

SideJumpAmbush - Will jump out to left or right (depending on which side of the assassin the player is)

TeleportDodge - Can use teleporting to dodge attacks

FIELDS:

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
mintel					= 64
melee_range				= 48
missile_range			= 1024
min_missile_range		= 64
bypass_missile_chance	= 10
jump_chance				= 100
wakeup_distance			= 1024

NOTE: A value of zero will result in defaults, if you actually want zero as the value, use -1
*/
/*-------------------------------------------------------------------------
	SP_monster_assassin
-------------------------------------------------------------------------*/
void SP_monster_assassin (edict_t *self)
{
	if(self->spawnflags & MSF_WALKING)
	{
		self->spawnflags |= MSF_WANDER;
		self->spawnflags &= ~MSF_WALKING;
	}

	if(self->spawnflags&MSF_ASS_JUMPAMBUSH||
		self->spawnflags&MSF_ASS_SIDEJUMPAMBUSH||
		self->spawnflags&MSF_ASS_STARTSHADOW)
		self->spawnflags |= MSF_AMBUSH;

	if(self->spawnflags & MSF_ASS_TPORTAMBUSH)
		self->spawnflags|=MSF_ASLEEP;

	if (!walkmonster_start(self))	// Unsuccessful initialization.
		return;
		
	self->msgHandler = DefaultMsgHandler;
	self->classID = CID_ASSASSIN;
	self->monsterinfo.dismember = assassin_dismember;

	if(!self->health)
		self->health = ASSASSIN_HEALTH * (skill->value + 1)/3;

	self->mass = ASSASSIN_MASS;
	self->yaw_speed = 20;

	self->movetype = PHYSICSTYPE_STEP;
	VectorClear(self->knockbackvel);
	
	self->solid=SOLID_BBOX;

	VectorCopy(STDMinsForClass[self->classID], self->mins);
	VectorCopy(STDMaxsForClass[self->classID], self->maxs);	
	self->viewheight = 40;
	
	self->isBlocked = self->bounced = assassin_Touch;

	self->s.modelindex = classStatics[CID_ASSASSIN].resInfo->modelIndex;

	self->materialtype = MAT_FLESH;

	//FIXME (somewhere: otherenemy should be more than just *one* kind
	self->monsterinfo.otherenemyname = "monster_rat";

	//set up my mood function
	MG_InitMoods(self);
	if(!irand(0,2))
		self->ai_mood_flags |= AI_MOOD_FLAG_PREDICT;
	self->cant_attack_think = assassinCheckDefense;

	self->monsterinfo.aiflags |= AI_NIGHTVISION;

	if(self->spawnflags & MSF_WANDER)
	{
		QPostMessage(self, MSG_WALK, PRI_DIRECTIVE, NULL);
	}
	else if(self->spawnflags & MSF_ASS_CINEMATIC)
	{
		self->monsterinfo.c_mode = 1;
		QPostMessage(self, MSG_C_IDLE1, PRI_DIRECTIVE, "iiige",0,0,0,NULL,NULL);
	}
	else
	{
		if(self->spawnflags&MSF_ASS_STARTSHADOW)
			assassinInitCloak (self);

		self->pre_think = assassinCloakThink;
		self->next_pre_think = level.time + FRAMETIME;

		QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
	}

	self->svflags |= SVF_WAIT_NOTSOLID;

	self->s.fmnodeinfo[MESH__KNIFES].flags |= FMNI_NO_DRAW;

	VectorCopy(self->s.origin, self->pos1);
}

