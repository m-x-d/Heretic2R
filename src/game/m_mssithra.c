//==============================================================================
//
// m_mssithra.c
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

#include "m_mssithra.h"
#include "m_mssithra_anim.h"
#include "g_misc.h"
#include "g_HitLocation.h"
#include "p_anim_branch.h"
#include "m_stats.h"
#include "g_playstats.h"
#include "p_actions.h"

void extrapolateFiredir (edict_t *self,vec3_t p1,float pspeed,edict_t *targ,float accept,vec3_t vec2);
void create_ssithra_arrow(edict_t *Arrow);

#define MSSITHRA_JUMP_VELOCITY	300.0
#define MSSITHRA_HOP_VELOCITY	128.0
#define MSSITHRA_SF_NAMOR		8
#define MSSITHRA_SF_SPIN		16

//========================================
//INITIALIZE
//========================================
static animmove_t *animations[NUM_ANIMS] =
{
	&mssithra_move_claw1,
	&mssithra_move_death1,
	&mssithra_move_idle1,
	&mssithra_move_jump1,
	&mssithra_move_fjump,
//	&mssithra_move_pain1,
	&mssithra_move_roar,
	&mssithra_move_shoota1,
	&mssithra_move_shootb1,
	&mssithra_move_walk1,
	&mssithra_move_backpedal1,
	&mssithra_move_run1,
	&mssithra_move_delay,
	&mssithra_move_shoot1_trans,
	&mssithra_move_shoot1_loop,
	&mssithra_move_shoot1_detrans
};

static int Sounds[NUM_SOUNDS];

static ClassResourceInfo_t resInfo;
/*

//========================================
//MOVEMENT
//========================================

*/

void mssithra_stand(edict_t *self, G_Message_t *msg)
{
	SetAnim(self, ANIM_IDLE1);
}

void mssithra_decide_stand(edict_t *self)
{
	if(mssithraCheckMood(self))
		return;

	SetAnim(self, ANIM_IDLE1);
}

void mssithra_pain(edict_t *self, G_Message_t *msg)
{//fixme - make part fly dir the vector from hit loc to sever loc
	int				temp, damage;
	qboolean		force_pain;

	if(self->deadflag == DEAD_DEAD) //Dead but still being hit	
		return;

	ParseMsgParms(msg, "eeiii", &temp, &temp, &force_pain, &damage, &temp);

	if(!force_pain)
	{
		if(irand(0,10)<5||!self->groundentity)
			return;

		if(self->pain_debounce_time > level.time)
			return;
	}

	if(self->curAnimID == ANIM_CLAW1)
		return;

	self->pain_debounce_time = level.time + 2;

	if(irand(0,10)<5)
		gi.sound (self, CHAN_VOICE, Sounds[SND_PAIN1], 1, ATTN_NORM, 0);
	else
		gi.sound (self, CHAN_VOICE, Sounds[SND_PAIN2], 1, ATTN_NORM, 0);

	if (self->flags&FL_INWATER)
		self->flags |= FL_SWIM;
	else 
		self->flags &= ~FL_SWIM;

	if(irand(0,10)<1)
		SetAnim(self, ANIM_ROAR1);//make him tougher? more aggressive?
}

void mssithra_pain_react (edict_t *self)
{
	if(!self->enemy)
	{
		mssithra_decide_stand(self);
		return;
	}

	if(self->enemy->health<=0||self->enemy == self||!self->enemy->takedamage)
	{
		self->enemy=NULL;
		mssithra_decide_stand(self);
		return;
	}
}

//===========================================
//DEATHS
//===========================================

void mssithra_death(edict_t *self, G_Message_t *msg)
{
	self->svflags |= SVF_DEADMONSTER;
	self->msgHandler=DyingMsgHandler;

	if(self->deadflag == DEAD_DEAD)
		return;
	
	self->deadflag = DEAD_DEAD;

	gi.sound (self, CHAN_VOICE, Sounds[SND_DIE], 1, ATTN_NORM, 0);
	if (self->flags&FL_INWATER)
		self->flags |= FL_SWIM;
	else
		self->flags &= ~FL_SWIM;
	
	SetAnim(self, ANIM_DEATH1);

	//Remove the life bar once dead
	M_ShowLifeMeter( self, 0, 0);
	self->post_think = NULL;
	self->next_post_think = -1;
}

void mssithra_dead(edict_t *self)
{//maybe allow dead bodies to be chopped?  Make BBOX small?
	self->msgHandler = DeadMsgHandler;
	self->deadState = DEAD_DEAD;

	self->think = NULL;
	self->nextthink = 0;
	self->flags |= FL_DONTANIMATE;

	gi.linkentity(self);
}

void mssithraKillSelf (edict_t *self)
{
	vec3_t gore_spot;

	self->svflags &= ~SVF_DEADMONSTER;	// now treat as a different content type
	self->msgHandler = DefaultMsgHandler;
	self->deadflag = false;
	VectorCopy(self->s.origin,gore_spot);
	gore_spot[2]+=12;
	self->health = 1;
	T_Damage (self, self, self, vec3_origin, gore_spot, vec3_origin, 10, 0,0,MOD_DIED);
	self->health = -69;
}

//===========================================
//SOUNDS
//===========================================

void mssithraSound(edict_t *self, float soundnum, float channel, float attenuation)
{
	return;
	if(!channel)
		channel = CHAN_AUTO;

	if(!attenuation)
		attenuation = ATTN_NORM;
	else if(attenuation == -1)
		attenuation = ATTN_NONE;

	gi.sound(self, (int)channel, Sounds[(int)(soundnum)], 1, (int)attenuation, 0);
}

//===========================================
//ATTACKS
//===========================================

void mssithra_melee(edict_t *self, G_Message_t *msg)
{
	vec3_t	v;
	float	len, melee_range, min_seperation, jump_range;
	
	if(M_ValidTarget(self, self->enemy))
	{
		VectorSubtract (self->s.origin, self->enemy->s.origin, v);
		len = VectorLength (v);
		melee_range = 64;
		jump_range = 128;
		min_seperation = self->maxs[0] + self->enemy->maxs[0];

		if (len < min_seperation + melee_range)	// A hit
		{
			SetAnim(self, ANIM_CLAW1);
		}
	}
	else
		QPostMessage(self, MSG_STAND,PRI_DIRECTIVE, NULL);
}

void mssithra_missile(edict_t *self, G_Message_t *msg)
{//NEWSTUFF: jump closer to claw, loop shooting anims
	vec3_t	v;
	float	len, min_seperation, jump_range;
	
	if(M_ValidTarget(self, self->enemy))
	{
		VectorSubtract (self->s.origin, self->enemy->s.origin, v);
		len = VectorLength (v);
		jump_range = 128;
		min_seperation = self->maxs[0] + self->enemy->maxs[0];
		
		if (irand(0,(skill->value+1)*2))
		{
			SetAnim(self, ANIM_SHOOT_TRANS);
		}
		else
		{
			SetAnim(self, ANIM_IDLE1);
		}
	}
	else
		QPostMessage(self, MSG_STAND,PRI_DIRECTIVE, NULL);
}

void mssithraSwipe (edict_t *self)
{
	vec3_t	v, off, dir, org, ang;
	float	len;
	
	if (self->enemy == NULL)	// If the player gets gibbed, enemy can be NULL.
		return;

	VectorSubtract (self->s.origin, self->enemy->s.origin, v);
	len = VectorLength (v);

	if (len < (self->maxs[0] + self->enemy->maxs[0] + 45)  )	// A hit
	{	
		if (infront(self, self->enemy))
		{
			gi.sound (self, CHAN_WEAPON, Sounds[SND_SWIPEHIT], 1, ATTN_NORM, 0);
			VectorSet(off, 35.0, 0.0, 32.0);
			VectorGetOffsetOrigin(off, self->s.origin, self->s.angles[YAW], org);
			VectorCopy(self->s.angles, ang);
			ang[YAW] += DEGREE_90;
			AngleVectors(ang, dir, NULL, NULL);
			T_Damage (self->enemy, self, self, dir, org, vec3_origin, 
					MSSITHRA_DMG_SWIPE, 0, DAMAGE_DISMEMBER,MOD_DIED);

			if(self->enemy->health>0)//else don't gib?
			{	
				if(!irand(0,5))
				{
					if(!stricmp(self->enemy->classname, "player"))
						P_KnockDownPlayer(&self->enemy->client->playerinfo);
				}	
			}
			return;
		}
	}
	gi.sound (self, CHAN_WEAPON, Sounds[SND_SWIPE], 1, ATTN_NORM, 0);
}

void mssithra_missile_explode(edict_t *self)
{
	int damage = irand(8, 16);

	//TODO: Spawn an explosion effect
	gi.CreateEffect( NULL,
					 FX_M_EFFECTS,
					 0,
					 self->s.origin,
					 "bv",
					 FX_MSSITHRA_EXPLODE,
					 self->movedir);

	T_DamageRadius(self, self->owner, self->owner, 128, damage, damage/2, DAMAGE_ATTACKER_IMMUNE, MOD_DIED);

	G_FreeEdict(self);
}


edict_t *MssithraAlphaArrowReflect(edict_t *self, edict_t *other, vec3_t vel)
{
	edict_t *arrow;

	arrow = G_Spawn();

	create_ssithra_arrow(arrow);
	VectorCopy(self->s.origin, arrow->s.origin);
	arrow->owner = self->owner;

	arrow->nextthink=self->nextthink;
	arrow->think=G_FreeEdict;

	VectorCopy(vel, arrow->velocity);
	VectorNormalize2(vel, arrow->movedir);
	AnglesFromDir(arrow->movedir, arrow->s.angles);
	arrow->s.angles[YAW]+=90;

	arrow->reflect_debounce_time = self->reflect_debounce_time - 1;
	arrow->reflected_time=self->reflected_time;

	G_LinkMissile(arrow);

	gi.CreateEffect(&arrow->s, 
					FX_M_EFFECTS, 
					CEF_OWNERS_ORIGIN | CEF_FLAG6, 
					NULL, 
					"bv", 
					FX_MSSITHRA_ARROW,
					arrow->velocity);

	G_SetToFree(self);

	gi.CreateEffect(&arrow->s, FX_LIGHTNING_HIT, CEF_OWNERS_ORIGIN, NULL, "t", vel);

	return(arrow);;
}


void mssithraAlphaArrowTouch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surface)
{
	// are we reflecting ?
	if (self->reflect_debounce_time)
	{
		if(EntReflecting(other, true, true))
		{
			Create_rand_relect_vect(self->velocity, self->velocity);
			Vec3ScaleAssign(MSSITHRA_ARROW_SPEED/2, self->velocity);
			MssithraAlphaArrowReflect(self, other, self->velocity);

			return;
		}
	}

	if(other->takedamage)
	{
		if (plane->normal)
			VectorCopy(plane->normal, self->movedir);

		self->dmg = irand(MSSITHRA_DMG_MIN*2, MSSITHRA_DMG_MAX*2);
		mssithra_missile_explode(self);
	}
	else
	{
		gi.sound(self, CHAN_WEAPON, Sounds[SND_INWALL], 0.5, ATTN_NORM, 0);

		self->s.effects |= EF_ALTCLIENTFX;

		VectorClear(self->velocity);
		
		if (plane->normal)
			VectorCopy(plane->normal, self->movedir);

		self->dmg = irand(MSSITHRA_DMG_MIN, MSSITHRA_DMG_MAX);
		self->think = mssithra_missile_explode;
		self->nextthink = level.time + flrand(0.5, 1.5);
	}
}

// create the guts of the ssithra arrow
void create_ssithra_arrow(edict_t *Arrow)
{
	Arrow->s.modelindex = gi.modelindex("models/objects/exarrow/tris.fm");
	Arrow->movetype = MOVETYPE_FLYMISSILE;
	Arrow->solid = SOLID_BBOX;
	Arrow->classname = "mssithra_Arrow";

	Arrow->touch = mssithraAlphaArrowTouch;

	Arrow->clipmask = MASK_SHOT;
	
	Arrow->s.effects |= EF_ALWAYS_ADD_EFFECTS | EF_CAMERA_NO_CLIP;
	Arrow->svflags |= SVF_ALWAYS_SEND;

	VectorSet(Arrow->mins,-1.0,-1.0,-1.0);	
	VectorSet(Arrow->maxs,1.0,1.0,1.0);
	Arrow->s.scale = 1.5;
}

void mssithraArrow(edict_t *self)
{//fixme; adjust for up/down
	vec3_t	Forward, targ_pos;
	vec3_t	Right, fire_spot, fire_dir;// , up;
	edict_t	*Arrow;
	float	spread;
	int	num_shots = 3;
	
	if (!self->enemy)
		return;

	if(self->enemy->health<=0)
	{
		self->enemy=NULL;
		mssithra_decide_stand(self);
		return;
	}

	if(self->monsterinfo.attack_finished>level.time)
		return;

	gi.sound(self,CHAN_WEAPON,Sounds[SND_ARROW],1,ATTN_NORM,0);
	self->monsterinfo.attack_finished = level.time + 0.4;

	VectorCopy(self->s.origin,fire_spot);	
	fire_spot[2]+=self->maxs[2]*0.5;
	AngleVectors(self->s.angles,Forward,Right,NULL);
	VectorMA(fire_spot,72,Forward,fire_spot);
	VectorMA(fire_spot,16,Right,fire_spot);
	fire_spot[2] += 24;

	VectorRandomCopy(self->enemy->s.origin, targ_pos, 16);
	VectorSubtract(targ_pos, fire_spot, Forward);
	VectorNormalize2(Forward, fire_dir);

	while(num_shots)
	{
		Arrow = G_Spawn();

		create_ssithra_arrow(Arrow);
		Arrow->reflect_debounce_time = MAX_REFLECT;
		
		VectorCopy(fire_spot,Arrow->s.origin);	
		VectorCopy(self->movedir,Arrow->movedir);

		//Increase the spread for lower levels
		switch((int)skill->value)
		{
		case 0:
			spread = 0.35;
			break;

		case 1:
			spread = 0.2;
			break;

		case 2:
		default:
			spread = 0.1;
			break;
		}

		AngleVectors(self->s.angles,NULL,Right,NULL);
		switch (num_shots)
		{
			case 3:
				VectorScale(Right, spread,Right);
			break;

			case 1:
				VectorScale(Right,-spread,Right);
			break;

			case 2:
			default:
				VectorClear(Right);
			break;
		}

		VectorAdd(fire_dir, Right, Arrow->movedir);
		VectorNormalize(Arrow->movedir);
		VectorScale(Arrow->movedir,MSSITHRA_ARROW_SPEED,Arrow->velocity);

		vectoangles(Arrow->velocity, Arrow->s.angles);
		Arrow->s.angles[YAW] += 90;

		Arrow->owner=self;
		Arrow->enemy=self->enemy;

		gi.CreateEffect(&Arrow->s, 
						FX_M_EFFECTS, 
						CEF_OWNERS_ORIGIN | CEF_FLAG6, 
						NULL, 
						"bv", 
						FX_MSSITHRA_ARROW,
						Arrow->velocity);

		G_LinkMissile(Arrow); 

		Arrow->nextthink=level.time+5;
		Arrow->think=G_FreeEdict;//mssithraArrowThink;

		num_shots--;
	}
}

void mssithraCheckLoop (edict_t *self, float frame)
{//see if should fire again
	vec3_t	v;
	float	len, melee_range, min_seperation, jump_range;

	if(!self->enemy)
		return;

	ai_charge2(self, 0);

	if(!visible(self, self->enemy))
		return;

	if(!infront(self, self->enemy))
		return;

	if(irand(0,10)<5)
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

/*-------------------------------------------------------------------------
	mssithra_pause
-------------------------------------------------------------------------*/
qboolean mssithraCheckMood (edict_t *self)
{
	if(self->monsterinfo.aiflags&AI_OVERRIDE_GUIDE)
		return false;

	self->mood_think(self);

	if (self->ai_mood == AI_MOOD_NORMAL)
		return false;
		
	switch (self->ai_mood)
	{
	case AI_MOOD_ATTACK:
		if(self->ai_mood_flags & AI_MOOD_FLAG_MISSILE)
			QPostMessage(self, MSG_MISSILE, PRI_DIRECTIVE, NULL);
		else
			QPostMessage(self, MSG_MELEE, PRI_DIRECTIVE, NULL);
		return true;
		break;
	case AI_MOOD_PURSUE:
	case AI_MOOD_NAVIGATE:
		QPostMessage(self, MSG_RUN, PRI_DIRECTIVE, NULL);
		return true;
		break;
	case AI_MOOD_WALK:
		QPostMessage(self, MSG_WALK, PRI_DIRECTIVE, NULL);
		return true;
		break;

	case AI_MOOD_STAND:
		QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
		return true;
		break;
	}

	return false;
}

void mssithra_postthink(edict_t *self)
{
	//Only display a lifemeter if we have an enemy
	if (self->enemy)
	{
		if (self->dmg < self->max_health)
		{
			M_ShowLifeMeter( self, self->dmg, self->dmg);
			self->dmg+=50;
		}
		else
		{
			M_ShowLifeMeter( self, self->health, self->max_health);
		}
	}

	self->next_post_think = level.time + 0.05;
}

void mmssithraRandomGrowlSound (edict_t *self)
{
	if(!irand(0,2))
		gi.sound(self, CHAN_VOICE, Sounds[SND_GROWL1], 1, ATTN_NORM, 0);
	else if(!irand(0,1))
		gi.sound(self, CHAN_VOICE, Sounds[SND_GROWL2], 1, ATTN_NORM, 0);
	else
		gi.sound(self, CHAN_VOICE, Sounds[SND_GROWL3], 1, ATTN_NORM, 0);
}

void mssithra_ShotLoop(edict_t *self)
{
	SetAnim(self, ANIM_SHOOT_LOOP);
}

void mssithraCheckShotLoop(edict_t *self)
{
	//TODO: Check to keep shooting or to stop shooting
	if (irand(0,(skill->value+1)*2))
	{
		//Just keep playing the animation
		return;
	}
	else
	{
		SetAnim(self, ANIM_SHOOT_DETRANS);
	}
}

void MssithraStaticsInit()
{
	classStatics[CID_MSSITHRA].msgReceivers[MSG_STAND] = mssithra_stand;
	classStatics[CID_MSSITHRA].msgReceivers[MSG_MISSILE] = mssithra_missile;
	classStatics[CID_MSSITHRA].msgReceivers[MSG_MELEE] = mssithra_melee;
	classStatics[CID_MSSITHRA].msgReceivers[MSG_DEATH] = mssithra_death;
	classStatics[CID_MSSITHRA].msgReceivers[MSG_PAIN] = mssithra_pain;
	classStatics[CID_MSSITHRA].msgReceivers[MSG_RUN] = mssithra_missile;

	resInfo.numAnims = NUM_ANIMS;
	resInfo.animations = animations;
	resInfo.modelIndex = gi.modelindex("models/monsters/mutantsithra/tris.fm");

	Sounds[SND_PAIN1]=gi.soundindex("monsters/mssithra/pain1.wav");
	Sounds[SND_PAIN2]=gi.soundindex("monsters/mssithra/pain2.wav");	
	Sounds[SND_DIE]=gi.soundindex("monsters/mssithra/death1.wav");	
	Sounds[SND_SWIPE] = gi.soundindex ("monsters/mssithra/swipe.wav");
	Sounds[SND_SWIPEHIT]=gi.soundindex("monsters/mssithra/swipehit.wav");	
	Sounds[SND_ARROW]=gi.soundindex("weapons/RedRainPowerFire.wav");	
	Sounds[SND_AEXPLODE]=gi.soundindex("weapons/FlyingFistImpact.wav");
	Sounds[SND_GROWL1]=gi.soundindex("monsters/mssithra/growl1.wav");	
	Sounds[SND_GROWL2] = gi.soundindex ("monsters/mssithra/growl2.wav");	
	Sounds[SND_GROWL3] = gi.soundindex ("monsters/mssithra/growl3.wav");	
	Sounds[SND_ROAR]=gi.soundindex("monsters/mssithra/roar.wav");	
	Sounds[SND_INWALL]=gi.soundindex("weapons/staffhitwall.wav");	

	resInfo.numSounds = NUM_SOUNDS;
	resInfo.sounds = Sounds;

	classStatics[CID_MSSITHRA].resInfo = &resInfo;
}

/*QUAKED monster_mssithra (1 .5 0) (-36 -36 0) (36 36 96) AMBUSH ASLEEP 4 8 16 32 64 FIXED WANDER MELEE_LEAD STALK COWARD EXTRA1 EXTRA2 EXTRA3 EXTRA4

The  mssithra 

AMBUSH - Will not be woken up by other monsters or shots from player

ASLEEP - will not appear until triggered

WALKING - Use WANDER instead

WANDER - Monster will wander around aimlessly (but follows buoys)

MELEE_LEAD - Monster will tryto cut you off when you're running and fighting him, works well if there are a few monsters in a group, half doing this, half not

STALK - Monster will only approach and attack from behind- if you're facing the monster it will just stand there.  Once the monster takes pain, however, it will stop this behaviour and attack normally

COWARD - Monster starts off in flee mode- runs away from you when woken up

"homebuoy" - monsters will head to this buoy if they don't have an enemy ("homebuoy" should be targetname of the buoy you want them to go to)

"wakeup_target" - monsters will fire this target the first time it wakes up (only once)

"pain_target" - monsters will fire this target the first time it gets hurt (only once)

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
melee_range				= 100
missile_range			= 400
min_missile_range		= 100
bypass_missile_chance	= 25
jump_chance				= 25
wakeup_distance			= 1024

NOTE: A value of zero will result in defaults, if you actually want zero as the value, use -1
*/
void SP_monster_mssithra (edict_t *self)
{
	// Generic Monster Initialization
	if (!walkmonster_start (self))		// failed initialization
		return;

	self->msgHandler = DefaultMsgHandler;
	self->classID = CID_MSSITHRA;
	self->materialtype = MAT_FLESH;
	self->flags |= FL_IMMUNE_SLIME;
	if(self->flags&FL_INWATER||
		gi.pointcontents(self->s.origin)&CONTENTS_WATER||
		self->waterlevel >= 3)
		self->flags|=FL_SWIM;

	if(self->health<=0)
		self->health = MSSITHRA_HEALTH;

	//Apply to the end result (whether designer set or not)
	self->max_health = self->health = MonsterHealth(self->health);

	self->mass = MSSITHRA_MASS;
	self->yaw_speed = 20;
	self->viewheight = 88;
	self->monsterinfo.aiflags |= AI_BRUTAL|AI_AGRESSIVE;

	self->movetype=PHYSICSTYPE_STEP;//MOVETYPE_STEP
	VectorClear(self->knockbackvel);

	self->solid=SOLID_BBOX;

	VectorCopy(STDMinsForClass[self->classID], self->mins);
	VectorCopy(STDMaxsForClass[self->classID], self->maxs);	

	self->s.modelindex = classStatics[CID_MSSITHRA].resInfo->modelIndex;

	self->s.skinnum = 0;
	
	if (!self->monsterinfo.scale)
	{
		self->s.scale = self->monsterinfo.scale = (MODEL_SCALE + 0.25);
	}

	self->monsterinfo.otherenemyname = "obj_barrel";

	//set up my mood function
	MG_InitMoods(self);

	QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);

	self->post_think = mssithra_postthink;
	self->next_post_think = level.time + 0.1;

	//Turn the goofy bolts off!
	self->s.fmnodeinfo[MESH__BOLTS].flags |= FMNI_NO_DRAW;
	
	self->dmg = 0;
	self->svflags|=SVF_BOSS;
}
