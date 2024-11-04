//==============================================================================
//
// m_seraph.c
//
// Heretic II
// Copyright 1998 Raven Software
//
// jweier
//==============================================================================

#include "g_local.h"
#include "m_seraph.h"
#include "m_seraph_anim.h"
#include "Utilities.h"
#include "g_DefaultMessageHandler.h"
#include "g_monster.h"
#include "Random.h"
#include "vector.h"
#include "fx.h"
#include "g_HitLocation.h"
#include "g_misc.h"

#include "m_stats.h"

void MG_InitMoods(edict_t *self);
//Seraphs need knowledge of the ogle's animations
//Any changes in m_ogle.h must be mirrored here
enum
{
	OGLE_ANIM_WALK1,
	OGLE_ANIM_PUSH1,
	OGLE_ANIM_PUSH2,
	OGLE_ANIM_PUSH3,
	OGLE_ANIM_STAND1,
	OGLE_ANIM_WORK1,
	OGLE_ANIM_WORK2,
	OGLE_ANIM_WORK3,
	OGLE_ANIM_WORK4,
	OGLE_ANIM_WORK5,
	OGLE_ANIM_PAIN1,
	OGLE_ANIM_PAIN2,
	OGLE_ANIM_PAIN3,
	OGLE_ANIM_REST1_TRANS,
	OGLE_ANIM_REST1_WIPE,
	OGLE_ANIM_REST1,
	OGLE_ANIM_REST2_WIPE,
	OGLE_ANIM_REST3_WIPE,
	OGLE_ANIM_REST4_TRANS,
	OGLE_ANIM_REST4_TRANS2,
	OGLE_ANIM_REST4,
	OGLE_ANIM_CELEBRATE1,
	OGLE_ANIM_CELEBRATE2,
	OGLE_ANIM_CELEBRATE3_TRANS,
	OGLE_ANIM_CELEBRATE3,
	OGLE_ANIM_CELEBRATE4_TRANS,
	OGLE_ANIM_CELEBRATE4,
	OGLE_ANIM_CELEBRATE5_TRANS,
	OGLE_ANIM_CELEBRATE5,
	OGLE_ANIM_CHARGE1,
	OGLE_ANIM_CHARGE2,
	OGLE_ANIM_CHARGE3,
	OGLE_ANIM_CHARGE4,
	OGLE_ANIM_CHARGE5,
	OGLE_ANIM_ATTACK1,
	OGLE_ANIM_ATTACK2,
	OGLE_ANIM_ATTACK3,
	OGLE_ANIM_DEATH1,
	OGLE_ANIM_DEATH2,
	NUM_OGLE_ANIMS
};

static animmove_t *animations[NUM_ANIMS] =
{
	&seraph_move_walk1,
	&seraph_move_walk2,
	&seraph_move_whip1,
	&seraph_move_whip1_loop,
	&seraph_move_whip1_end,
	&seraph_move_stand1,
	&seraph_move_stand1_tr,
	&seraph_move_stand1_r,
	&seraph_move_stand1_trc,
	&seraph_move_stand1_tl,
	&seraph_move_stand1_l,
	&seraph_move_stand1_tlc,
	&seraph_move_point1,
	&seraph_move_run1,
	&seraph_move_fjump,
	&seraph_move_run1_whip,
	&seraph_move_pain,
	&seraph_move_swipe,
	&seraph_move_get2work,
	&seraph_move_get2work2,
	&seraph_move_startle,
	&seraph_move_ready2idle,
	&seraph_move_backup,
	&seraph_move_death1,
	&seraph_move_death2_go,
	&seraph_move_death2_loop,
	&seraph_move_death2_end,
	&seraph_move_backup2,
};

static int sounds[NUM_SOUNDS];
static ClassResourceInfo_t resInfo;

#define OVERLORD_RADIUS	1000	//FIXME: Tweak out

/*
==========================================================

	Seraph Helper functions

==========================================================
*/

void seraphApplyJump (edict_t *self)
{
	self->jump_time = level.time + 0.75;
	VectorCopy(self->movedir, self->velocity);
	VectorNormalize(self->movedir);
}

void seraph_dead ( edict_t *self )
{
	self->health = 0;
	self->solid = SOLID_NOT;

	M_EndDeath ( self );
}

void seraph_death_loop ( edict_t *self )
{
	SetAnim(self, ANIM_DEATH2_LOOP);
}

void seraph_check_land ( edict_t *self )
{
	vec3_t	endpos;
	trace_t	trace;

	M_ChangeYaw(self);

	VectorCopy(self->s.origin, endpos);
	endpos[2] -= 48;

	gi.trace(self->s.origin, self->mins, self->maxs, endpos, self, MASK_MONSTERSOLID,&trace);

	if ( (trace.fraction < 1 || trace.allsolid || trace.startsolid ) && self->curAnimID != ANIM_DEATH2_END && self->curAnimID != ANIM_DEATH2_GO)
	{
		self->elasticity = 1.25;
		self->friction = 0.5;
		SetAnim(self, ANIM_DEATH2_END);
	}
}

void seraph_sound_startle(edict_t *self)
{
	gi.sound (self, CHAN_VOICE, sounds[SND_STARTLE], 1, ATTN_NORM, 0);
}

void seraph_sound_slap(edict_t *self)
{
	gi.sound (self, CHAN_WEAPON, sounds[SND_SLAP], 1, ATTN_NORM, 0);
}

void seraph_sound_scold(edict_t *self)
{//no talking!
	gi.sound (self, CHAN_VOICE, sounds[SND_SCOLD3], 1, ATTN_NORM, 0);
}

void seraph_sound_scold2(edict_t *self)
{
	if(irand(0, 1))//back to work!
		gi.sound(self, CHAN_VOICE, sounds[SND_SCOLD1], 1, ATTN_NORM, 0);
	else
		gi.sound(self, CHAN_VOICE, sounds[SND_SCOLD2], 1, ATTN_NORM, 0);
}

void seraph_sound_yell(edict_t *self)
{
	gi.sound (self, CHAN_VOICE, sounds[SND_SCARE], 1, ATTN_NORM, 0);
}

void seraph_sound_whip(edict_t *self)
{
	gi.sound (self, CHAN_WEAPON, sounds[SND_ATTACK], 1, ATTN_NORM, 0);
}

//Become startled and look around
void seraph_startle(edict_t *self)
{
	SetAnim(self, ANIM_STARTLE);
}

//Seraph has finished his startle, either track down the enemy, or go back to normal
void seraph_done_startle(edict_t *self)
{
	if (!FindTarget(self))
		QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
}

//Seraph has finished ANIM_GET2WORK, and must reset its enemy
void seraph_done_get2work(edict_t *self)
{
	if(self->enemy)
		self->enemy->targeted = 0;
	self->enemy = NULL;
	self->ai_mood = AI_MOOD_STAND;
	self->ai_mood_flags &= ~AI_MOOD_FLAG_IGNORE;
	seraph_pause(self);
}

//Special walk code for going to yell at an ogle
void seraph_ai_walk(edict_t *self, float dist)
{
	qboolean MG_MoveToGoal (edict_t *self, float dist);

	if (self->enemy && M_DistanceToTarget(self, self->enemy) < 72)
	{
		self->ai_mood = AI_MOOD_STAND;
		if (irand(0,1))
			SetAnim(self, ANIM_GET2WORK);
		else
			SetAnim(self, ANIM_GET2WORK2);
	}
	else
	{
		MG_MoveToGoal(self, dist);
	}
}

void seraph2idle (edict_t *self)
{
	SetAnim(self, ANIM_STAND1);
}

//Upper level AI interfacing
void seraph_pause(edict_t *self)
{
	self->mood_think(self);

	if(self->enemy)
		if ((self->ai_mood_flags & AI_MOOD_FLAG_IGNORE)&& self->enemy->classID == CID_OGLE)
			return;

	switch (self->ai_mood)
	{
	case AI_MOOD_ATTACK:
		QPostMessage(self, MSG_MELEE, PRI_DIRECTIVE, NULL);
		break;
	case AI_MOOD_PURSUE:
	case AI_MOOD_NAVIGATE:
	case AI_MOOD_WALK:
		QPostMessage(self, MSG_RUN, PRI_DIRECTIVE, NULL);
		break;
	case AI_MOOD_STAND:
		if (!self->enemy)
			QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
		break;//else what?

	case AI_MOOD_DELAY:
		break;

	case AI_MOOD_WANDER:
		SetAnim(self, ANIM_WALK1);
		break;

	case AI_MOOD_POINT_NAVIGATE:
		SetAnim(self, ANIM_WALK2);
		break;

	case AI_MOOD_JUMP:
		SetAnim(self, ANIM_FJUMP);
		break;

	default :
#ifdef _DEVEL
		gi.dprintf("seraph guard: Unusable mood %d!\n", self->ai_mood);
#endif
		break;
	}
}

void seraph_check_mood (edict_t *self, G_Message_t *msg)
{
	ParseMsgParms(msg, "i", &self->ai_mood);

	seraph_pause(self);
}

//Targets a specific ogle and puts it back to work
void seraph_enforce_ogle(edict_t *self)
{
	if(!self->enemy)
		return;

	self->enemy->use(self->enemy, self, self);

	if(irand(0, 1))//back to work!
		gi.sound(self, CHAN_VOICE, sounds[SND_SCOLD1], 1, ATTN_NORM, 0);
	else
		gi.sound(self, CHAN_VOICE, sounds[SND_SCOLD2], 1, ATTN_NORM, 0);
}

//Targets all idle ogles and puts them back to work
void seraph_enforce(edict_t *self)
{
	edict_t *ogle;

	ogle = NULL;
	
	while((ogle = findradius(ogle, self->s.origin, OVERLORD_RADIUS)) != NULL)
	{
		if (ogle->classID != CID_OGLE)
			continue;

		if (ogle->ai_mood != AI_MOOD_REST)
			continue;

		if (ogle->targetEnt != self)
			continue;
					
		//Setup within the ogle code
		ogle->use(ogle, self, self);
	}

}

//Check to see if you can make it to an idle ogle and scare them
qboolean seraph_checkscare(edict_t *self, edict_t *ogle)
{//FIX?  refers to goalentity
	trace_t	trace;
	vec3_t	sf, of, mins;
	float	dot;

	AngleVectors(self->s.angles, sf, NULL, NULL);
	AngleVectors(ogle->s.angles, of, NULL, NULL);

	dot = DotProduct(sf, of);

	//Only do it if you're in back of them, and they're facing away
	if (dot)
	{
		VectorCopy(self->mins, mins);
		mins[2] += 18;	//Account for step ability
		gi.trace(self->s.origin, mins, self->maxs, ogle->s.origin, self, MASK_MONSTERSOLID,&trace);

		if (trace.ent == ogle)
		{
			//Necessary info for the AI_MOOD_POINT_NAVIGATE stuff
			VectorCopy(self->s.origin, self->monsterinfo.nav_goal);
			self->old_yaw = self->s.angles[YAW];
			self->ai_mood = AI_MOOD_POINT_NAVIGATE;
			self->movetarget = self->goalentity = self->enemy = ogle;
			
			//Tells the other Seraphs not to try and get this one too
			ogle->targeted = 1;
			
			self->ai_mood_flags |= AI_MOOD_FLAG_IGNORE;
		
			return true;
		}
	}
	return false;
}

//Check the ogles and make sure their noses are to the grind stone
void seraph_oversee(edict_t *self)
{
	edict_t	*ogle;
	ogle=NULL;

	while((ogle = findradius(ogle, self->s.origin, OVERLORD_RADIUS)) != NULL)
	{
		if (ogle->ai_mood != AI_MOOD_REST)
			continue;

		if (ogle->classID != CID_OGLE)
			continue;
					
		if (ogle->targeted)
			return;

		if (ogle->targetEnt != self)
			return;

		//See if we can scare this one
		if (!seraph_checkscare(self, ogle))
		{
			self->ai_mood = AI_MOOD_ATTACK;
			self->ai_mood_flags |= AI_MOOD_FLAG_WHIP;
		}
		else //If we can scare it, stop and do it
			return;
	}
}

//Cycles through the various idle animations
void seraph_idle(edict_t *self)
{
	int chance = irand(0,100);

	seraph_oversee(self);

	//Check to see if we were supposed to point at an ogle
	if ( (self->ai_mood == AI_MOOD_ATTACK) && (self->ai_mood_flags & AI_MOOD_FLAG_WHIP) && (self->curAnimID != ANIM_POINT1) )
	{
		SetAnim(self, ANIM_POINT1);
		self->ai_mood = AI_MOOD_STAND;
		self->ai_mood_flags &= ~AI_MOOD_FLAG_WHIP;
		return;
	}

	//See if we're going to go scare an ogle
	if ( (self->ai_mood == AI_MOOD_POINT_NAVIGATE) )
	{
		SetAnim(self, ANIM_WALK2);
		return;
	}

	switch( self->curAnimID )
	{
		case ANIM_STAND1:
			if (chance < 20)
				SetAnim(self, ANIM_STAND1_TR);
			else if (chance < 40)
				SetAnim(self, ANIM_STAND1_TL);
			break;

		//Right
		case ANIM_POINT1:
		case ANIM_STAND1_TR:
			SetAnim(self, ANIM_STAND1_R);
			break;

		case ANIM_STAND1_R:
			if (chance < 50)
				SetAnim(self, ANIM_STAND1_TRC);
			break;

		case ANIM_STAND1_TLC:
		case ANIM_STAND1_TRC:
			SetAnim(self, ANIM_STAND1);
			break;

		//Left
		case ANIM_STAND1_TL:
			SetAnim(self, ANIM_STAND1_L);
			break;

		case ANIM_STAND1_L:
			if (chance < 50)
				SetAnim(self, ANIM_STAND1_TLC);
			break;
	}
}

//Check to do damage with a whip
void seraph_strike(edict_t *self, float damage, float a, float b)
{
	trace_t	trace;
	edict_t *victim;
	vec3_t	soff, eoff, mins, maxs, bloodDir, direction;

	VectorSet(soff, 16, 16, 32);
	VectorSet(eoff, 80, 16, 8);
	
	VectorSet(mins, -2, -2, -2);
	VectorSet(maxs,  2,  2,  2);

	VectorSubtract(soff, eoff, bloodDir);
	VectorNormalize(bloodDir);

	victim = M_CheckMeleeLineHit(self, soff, eoff, mins, maxs, &trace, direction);	

	if (victim)
	{
		if (victim == self)
		{
			//Create a puff effect
			//gi.CreateEffect(NULL, FX_SPARKS, 0, hitPos, "db", vec3_origin, irand(1,3));
			gi.sound (self, CHAN_WEAPON, sounds[SND_ATTACK], 1, ATTN_NORM, 0);
		}
		else
		{
			//Hurt whatever we were whacking away at
			T_Damage(victim, self, self, direction, trace.endpos, bloodDir, 
					damage, damage, DAMAGE_EXTRA_BLOOD|DAMAGE_EXTRA_KNOCKBACK,MOD_DIED);
			gi.sound (self, CHAN_WEAPON, sounds[SND_ATTACK], 1, ATTN_NORM, 0);
		}
	}
	else
	{
		//Play swoosh sound?
		gi.sound (self, CHAN_WEAPON, sounds[SND_ATTACK], 1, ATTN_NORM, 0);
	}
}

/*
==========================================================

	Seraph Message functions

==========================================================
*/

//Seraph has died
void seraph_death_pain(edict_t *self, G_Message_t *msg)
{
	if (self->health < -80)
	{
		BecomeDebris(self);
		return;
	}

}

void seraph_dropweapon (edict_t *self);
void seraph_death(edict_t *self, G_Message_t *msg)
{
	edict_t	*targ, *inflictor, *attacker;
	vec3_t	dVel, vf, yf;
	float	damage;
	int		soundID;

	ParseMsgParms(msg, "eeei", &targ, &inflictor, &attacker, &damage);

	M_StartDeath(self, ANIM_DEATH1);

	if (!stricmp(attacker->classname, "monster_ogle"))
	{
		self->health = 1;
		self->takedamage = DAMAGE_NO;
		self->solid = SOLID_BBOX;
		
		SetAnim(self, ANIM_DEATH1);
		
		soundID = irand(SND_DEATH1, SND_DEATH4);
		gi.sound (self, CHAN_BODY, sounds[soundID], 1, ATTN_NORM, 0);

		return;
	}

	if (self->health < -80)
	{
		return;
	}
	else if (self->health < -10)
	{
		seraph_dropweapon (self);

		SetAnim(self, ANIM_DEATH2_GO);

		VectorCopy(targ->velocity, vf);
		VectorNormalize(vf);

		VectorScale(vf, -1, yf);

		self->ideal_yaw = vectoyaw( yf );
		self->yaw_speed = 24;

		self->elasticity = 1.2;
		self->friction = 0.8;

		VectorScale(vf, 300, dVel);
		dVel[2] = irand(150,200);

		VectorCopy(dVel, self->velocity);
	}
	else
	{
		SetAnim(self, ANIM_DEATH1);
	}

	soundID = irand(SND_DEATH1, SND_DEATH4);
	gi.sound (self, CHAN_BODY, sounds[soundID], 1, ATTN_NORM, 0);
}

//Check to see if the Seraph is already standing, if not, transition into it
void seraph_stand(edict_t *self, G_Message_t *msg)
{
	if (self->curAnimID == ANIM_READY2IDLE)
		SetAnim(self, ANIM_STAND1);
	else
		SetAnim(self, ANIM_READY2IDLE);
}

//Classic run-attack function
void seraph_run(edict_t *self, G_Message_t *msg)
{
	if (M_ValidTarget(self, self->enemy))
	{
		SetAnim(self, ANIM_RUN1);
		return;
	}

	//If our enemy is dead, we need to stand
	QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
}

//Classic melee attack function
void seraph_melee(edict_t *self, G_Message_t *msg)
{
	vec3_t	attackVel, vf;
	float	dist;
	int		ret;

	//Don't interrupt a current animation with another melee call inside of it
	if (self->curAnimID == ANIM_ATTACK1_LOOP)
		return;

	if(self->enemy)
		if ( (self->ai_mood_flags & AI_MOOD_FLAG_IGNORE) && (!stricmp(self->enemy->classname, "monster_ogle")) )
			return;

	if (M_ValidTarget(self, self->enemy))
	{
		if(self->ai_mood == AI_MOOD_FLEE)
		{
			SetAnim(self, ANIM_BACKUP2);
			return;
		}
		//Set this for any uses below
		AngleVectors(self->s.angles, vf, NULL, NULL);

		dist = M_DistanceToTarget(self, self->enemy);	

		if (dist < 100)
		{
			VectorMA(vf, 0, vf, attackVel);
			ret  = M_PredictTargetEvasion( self, self->enemy, attackVel, self->enemy->velocity, self->melee_range, 5 );

			if (ret)
				SetAnim(self, ANIM_ATTACK1_LOOP);		
			else
				SetAnim(self, ANIM_RUN1_WHIP);
		}
		else
			SetAnim(self, ANIM_RUN1);

		return;
	}

	//If our enemy is dead, we need to stand
	QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
}

//Take pain
void seraph_pain(edict_t *self, G_Message_t *msg)
{
	edict_t	*attacker;
	int	temp, damage, soundID;
	int	force_pain;

	ParseMsgParms(msg, "eeiii", &temp, &attacker, &force_pain, &damage, &temp);

	if(self->monsterinfo.awake)
		self->ai_mood_flags &= ~AI_MOOD_FLAG_IGNORE;

	//Weighted random based on health compared to the maximum it was at
	if (force_pain || ((irand(0, self->max_health+50) > self->health) && !irand(0,2)))
	{
		soundID = irand(SND_PAIN1, SND_PAIN4);

		gi.sound (self, CHAN_BODY, sounds[soundID], 1, ATTN_NORM, 0);
		
		SetAnim(self, ANIM_PAIN);
	}

	if(attacker)
	{
		if(attacker!=self->enemy)
		{
			if(!self->enemy)
				self->enemy = attacker;
			else if(M_DistanceToTarget(self, self->enemy) > self->melee_range)
			{
				if(self->enemy->client)
					self->oldenemy = self->enemy;
				self->enemy = attacker;
			}
		}
	}
}


qboolean seraphAlerted (edict_t *self, alertent_t *alerter, edict_t *enemy)
{
	if(self->alert_time < level.time)
	{//not startled already
		if(!(alerter->alert_svflags&SVF_ALERT_NO_SHADE) && skill->value < 3.0 && !(self->monsterinfo.aiflags & AI_NIGHTVISION))
		{
			if(enemy->light_level < flrand(6, 77))
			{//too dark, can't see enemy
				return false;
			}
		}

		if(!infront(self,enemy))
		{
			if(alerter->lifetime < level.time + 2)
				self->alert_time = level.time + 2;//be ready for 2 seconds to wake up if alerted again
			else
				self->alert_time = alerter->lifetime;//be alert as long as the alert sticks around
			seraph_startle(self);
			return false;
		}
	}

	if(enemy->svflags&SVF_MONSTER)
		self->enemy = alerter->enemy;
	else
		self->enemy = enemy;

	FoundTarget(self, true);

	return true;
}



int Bit_for_MeshNode_so [NUM_MESH_NODES] =
{
	BIT_BASEBIN,	
	BIT_PITHEAD,//overlord head	
	BIT_SHOULDPAD,
	BIT_GUARDHEAD,//guard head
	BIT_LHANDGRD,//left hand guard
	BIT_LHANDBOSS,//left hand overlord
	BIT_RHAND,//right hand
	BIT_FRTORSO,
	BIT_ARMSPIKES,
	BIT_LFTUPARM,
	BIT_RTLEG,
	BIT_RTARM,
	BIT_LFTLEG,
	BIT_BKTORSO,
	BIT_AXE,//axe
	BIT_WHIP//whip
};

void seraph_back (edict_t *self, float dist)
{
	if(!MG_BoolWalkMove(self, self->s.angles[YAW] + 180, dist))
	{
		if(!irand(0, 1000))
		{
			self->monsterinfo.aiflags |= AI_COWARD;
			SetAnim(self, ANIM_RUN1);
		}
	}
}

qboolean canthrownode_so (edict_t *self, int BP, int *throw_nodes)
{//see if it's on, if so, add it to throw_nodes
	//turn it off on thrower
	if(!(self->s.fmnodeinfo[BP].flags & FMNI_NO_DRAW))
	{
		*throw_nodes |= Bit_for_MeshNode_so[BP];
		self->s.fmnodeinfo[BP].flags |= FMNI_NO_DRAW;
		return true;
	}
	return false;
}

//THROWS weapon, turns off those nodes, sets that weapon as gone
void seraph_dropweapon (edict_t *self)
{
	vec3_t handspot, forward, right, up;

	if(!(self->s.fmnodeinfo[MESH__WHIP].flags & FMNI_NO_DRAW))
	{
		VectorClear(handspot);
		AngleVectors(self->s.angles,forward,right,up);

		VectorMA(handspot,8,forward,handspot);
		VectorMA(handspot,5,right,handspot);
		VectorMA(handspot,12,up,handspot);
		ThrowWeapon(self, &handspot, BIT_WHIP, 0, FRAME_partfly);
		self->s.fmnodeinfo[MESH__WHIP].flags |= FMNI_NO_DRAW;
		return;
	}
}

void seraph_dismember(edict_t *self, int damage, int HitLocation)
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


	VectorClear(gore_spot);
	switch(HitLocation)
	{
		case hl_Head:
			if(self->s.fmnodeinfo[MESH__PITHEAD].flags & FMNI_NO_DRAW)
				break;
			if(self->s.fmnodeinfo[MESH__PITHEAD].flags & FMNI_USE_SKIN)
				damage*=1.5;//greater chance to cut off if previously damaged
			if(flrand(0,self->health)<damage*0.3&&dismember_ok)
			{
				seraph_dropweapon(self);
				canthrownode_so(self, MESH__PITHEAD,&throw_nodes);

				gore_spot[2]+=18;
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
				self->s.fmnodeinfo[MESH__PITHEAD].flags |= FMNI_USE_SKIN;
				self->s.fmnodeinfo[MESH__PITHEAD].skin = self->s.skinnum+1;
			}
			break;

		case hl_TorsoFront://split in half?
			if(self->s.fmnodeinfo[MESH__FRTORSO].flags & FMNI_USE_SKIN)
				damage*=1.5;//greater chance to cut off if previously damaged
			if(flrand(0,self->health)<damage)
			{
				self->s.fmnodeinfo[MESH__FRTORSO].flags |= FMNI_USE_SKIN;			
				self->s.fmnodeinfo[MESH__FRTORSO].skin = self->s.skinnum+1;
			}
			break;
		case hl_TorsoBack://split in half?
			if(self->s.fmnodeinfo[MESH__BKTORSO].flags & FMNI_USE_SKIN)
				damage*=1.5;//greater chance to cut off if previously damaged
			if(flrand(0,self->health) < damage)
			{
				self->s.fmnodeinfo[MESH__BKTORSO].flags |= FMNI_USE_SKIN;			
				self->s.fmnodeinfo[MESH__BKTORSO].skin = self->s.skinnum+1;
			}
			break;

		case hl_ArmUpperLeft:
			if(flrand(0,self->health)<damage)
			{
				self->s.fmnodeinfo[MESH__LFTUPARM].flags |= FMNI_USE_SKIN;			
				self->s.fmnodeinfo[MESH__LFTUPARM].skin = self->s.skinnum+1;
			}
			break;
		case hl_ArmLowerLeft://left arm
			if(self->s.fmnodeinfo[MESH__LHANDBOSS].flags & FMNI_NO_DRAW)
				break;
			if(self->s.fmnodeinfo[MESH__LHANDBOSS].flags & FMNI_USE_SKIN)
				damage*=1.5;//greater chance to cut off if previously damaged
			if(flrand(0,self->health)<damage*0.75&&dismember_ok)
			{
				if(canthrownode_so(self, MESH__LHANDBOSS, &throw_nodes))
				{
					AngleVectors(self->s.angles,NULL,right,NULL);
					gore_spot[2]+=self->maxs[2]*0.3;
					VectorMA(gore_spot,-10,right,gore_spot);
					ThrowBodyPart(self, &gore_spot, throw_nodes, damage, FRAME_partfly);
				}
			}
			else
			{
				self->s.fmnodeinfo[MESH__LHANDBOSS].flags |= FMNI_USE_SKIN;			
				self->s.fmnodeinfo[MESH__LHANDBOSS].skin = self->s.skinnum+1;
			}
			break;
		case hl_ArmUpperRight:
			if(flrand(0,self->health)<damage)
			{
				self->s.fmnodeinfo[MESH__RTARM].flags |= FMNI_USE_SKIN;			
				self->s.fmnodeinfo[MESH__RTARM].skin = self->s.skinnum+1;
			}
			break;
		case hl_ArmLowerRight://right arm
			if(self->s.fmnodeinfo[MESH__RHAND].flags & FMNI_NO_DRAW)
				break;
			if(flrand(0,self->health)<damage*0.75&&dismember_ok)
			{
				if(canthrownode_so(self, MESH__RHAND, &throw_nodes))
				{
					AngleVectors(self->s.angles,NULL,right,NULL);
					gore_spot[2]+=self->maxs[2]*0.3;
					VectorMA(gore_spot,10,right,gore_spot);
					seraph_dropweapon (self);
					ThrowBodyPart(self, &gore_spot, throw_nodes, damage, FRAME_partfly);
				}
			}
			else
			{
				self->s.fmnodeinfo[MESH__RHAND].flags |= FMNI_USE_SKIN;			
				self->s.fmnodeinfo[MESH__RHAND].skin = self->s.skinnum+1;
			}
			break;

		case hl_LegUpperLeft:
		case hl_LegLowerLeft://left leg
			if(self->s.fmnodeinfo[MESH__LFTLEG].flags & FMNI_USE_SKIN)
				break;
			self->s.fmnodeinfo[MESH__LFTLEG].flags |= FMNI_USE_SKIN;			
			self->s.fmnodeinfo[MESH__LFTLEG].skin = self->s.skinnum+1;
			break;
		case hl_LegUpperRight:
		case hl_LegLowerRight://right leg
			if(self->s.fmnodeinfo[MESH__RTLEG].flags & FMNI_USE_SKIN)
				break;
			self->s.fmnodeinfo[MESH__RTLEG].flags |= FMNI_USE_SKIN;			
			self->s.fmnodeinfo[MESH__RTLEG].skin = self->s.skinnum+1;
			break;
	}

	if(self->s.fmnodeinfo[MESH__RHAND].flags & FMNI_NO_DRAW)
		self->monsterinfo.aiflags |= AI_NO_MELEE;

//FIXME: when get missile anim
//	if(self->s.fmnodeinfo[MESH__LHANDGRD].flags & FMNI_NO_DRAW)
//		self->monsterinfo.aiflags |= AI_NO_MISSILE;

	if(self->monsterinfo.aiflags & AI_NO_MELEE)//&&self->monsterinfo.aiflags & AI_NO_MISSILE)
		SetAnim(self, ANIM_BACKUP);
}

void ser_ovl_SightSound(edict_t *self, G_Message_t *Msg)
{
	gi.sound(self, CHAN_VOICE, sounds[irand(SND_SIGHT1, SND_SIGHT3)], 1, ATTN_NORM, 0);
}

/*
==========================================================

	Seraph Spawn functions

==========================================================
*/

void SeraphOverlordStaticsInit()
{
	classStatics[CID_SERAPH_OVERLORD].msgReceivers[MSG_STAND]	= seraph_stand;
	classStatics[CID_SERAPH_OVERLORD].msgReceivers[MSG_RUN]		= seraph_run;
	classStatics[CID_SERAPH_OVERLORD].msgReceivers[MSG_MELEE]	= seraph_melee;
	classStatics[CID_SERAPH_OVERLORD].msgReceivers[MSG_PAIN]	= seraph_pain;
	classStatics[CID_SERAPH_OVERLORD].msgReceivers[MSG_DEATH]	= seraph_death;
	classStatics[CID_SERAPH_OVERLORD].msgReceivers[MSG_DISMEMBER]	= MG_parse_dismember_msg;
	classStatics[CID_SERAPH_OVERLORD].msgReceivers[MSG_DEATH_PAIN]	= seraph_death_pain;
	classStatics[CID_SERAPH_OVERLORD].msgReceivers[MSG_CHECK_MOOD] = seraph_check_mood;
	classStatics[CID_SERAPH_OVERLORD].msgReceivers[MSG_DISMEMBER] = MG_parse_dismember_msg;
	classStatics[CID_SERAPH_OVERLORD].msgReceivers[MSG_VOICE_SIGHT] = ser_ovl_SightSound;

	resInfo.numAnims = NUM_ANIMS;
	resInfo.animations = animations;
	resInfo.modelIndex = gi.modelindex("models/monsters/overlord/tris.fm");
	resInfo.numSounds = NUM_SOUNDS;
	resInfo.sounds = sounds;

	sounds[SND_ATTACK]	=	gi.soundindex("monsters/seraph/overlord/attack.wav");	
	
	sounds[SND_SCOLD1]	=	gi.soundindex("monsters/seraph/overlord/scold1.wav");//	Back to work
	sounds[SND_SCOLD2]	=	gi.soundindex("monsters/seraph/overlord/scold2.wav");	// Get to work
	sounds[SND_SCOLD3]	=	gi.soundindex("monsters/seraph/overlord/scold3.wav");	// No talking
	
	sounds[SND_SCARE]	=	gi.soundindex("monsters/seraph/overlord/scare.wav");//	Hey!

	sounds[SND_STARTLE]	=	gi.soundindex("monsters/seraph/overlord/startle.wav");	
	sounds[SND_SLAP]	=	gi.soundindex("monsters/seraph/overlord/slap.wav");	

	sounds[SND_DEATH1]	=	gi.soundindex("monsters/seraph/death1.wav");	
	sounds[SND_DEATH2]	=	gi.soundindex("monsters/seraph/death2.wav");	
	sounds[SND_DEATH3]	=	gi.soundindex("monsters/seraph/wimpdeath1.wav");	
	sounds[SND_DEATH4]	=	gi.soundindex("monsters/seraph/wimpdeath2.wav");	
	
	sounds[SND_PAIN1]	=	gi.soundindex("monsters/seraph/pain1.wav");	
	sounds[SND_PAIN2]	=	gi.soundindex("monsters/seraph/pain2.wav");	
	sounds[SND_PAIN3]	=	gi.soundindex("monsters/seraph/pain3.wav");	
	sounds[SND_PAIN4]	=	gi.soundindex("monsters/seraph/pain4.wav");	

	sounds[SND_SIGHT1]	=	gi.soundindex("monsters/seraph/overlord/sight1.wav");
	sounds[SND_SIGHT2]	=	gi.soundindex("monsters/seraph/overlord/sight2.wav");
	sounds[SND_SIGHT3]	=	gi.soundindex("monsters/seraph/overlord/scare.wav");

	classStatics[CID_SERAPH_OVERLORD].resInfo = &resInfo;
}

/*QUAKED monster_seraph_overlord(1 .5 0) (-24 -24 -34) (24 24 34) AMBUSH ASLEEP 4 8 16 32 64 FIXED WANDER MELEE_LEAD STALK COWARD EXTRA1 EXTRA2 EXTRA3 EXTRA4
The big, nasty, tyranical Overlords..

AMBUSH - Will not be woken up by other monsters or shots from player

ASLEEP - will not appear until triggered

WALKING- Use WANDER instead

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
mintel					= 24
melee_range				= 100
missile_range			= 0
min_missile_range		= 0
bypass_missile_chance	= 0
jump_chance				= 30
wakeup_distance			= 1024

NOTE: A value of zero will result in defaults, if you actually want zero as the value, use -1
*/
void SP_monster_seraph_overlord(edict_t *self)
{
	self->classID = CID_SERAPH_OVERLORD;

	if (!monster_start(self))			// Failed initialization
		return;
	
	self->msgHandler = DefaultMsgHandler;
	self->monsterinfo.alert = seraphAlerted;
	self->think = walkmonster_start_go;
	self->monsterinfo.dismember = seraph_dismember;

	if (!self->health)
	{
		self->health = SERAPH_HEALTH;
	}

	//Apply to the end result (whether designer set or not)
	self->max_health = self->health = MonsterHealth(self->health);

	self->mass = SERAPH_MASS;
	self->yaw_speed = 18;

	self->movetype = PHYSICSTYPE_STEP;
	self->solid=SOLID_BBOX;

	VectorCopy(STDMinsForClass[self->classID], self->mins);
	VectorCopy(STDMaxsForClass[self->classID], self->maxs);	

	self->materialtype = MAT_FLESH;

	self->s.modelindex = classStatics[CID_SERAPH_OVERLORD].resInfo->modelIndex;
	self->s.skinnum=0;

	self->ai_mood_flags |= AI_MOOD_FLAG_PREDICT;

	self->monsterinfo.otherenemyname = "monster_rat";	

	if (!self->s.scale)
	{
		self->s.scale = self->monsterinfo.scale = MODEL_SCALE;
	}

	//Turn off the Guard pieces!
	self->s.fmnodeinfo[MESH__AXE].flags |= FMNI_NO_DRAW;
	self->s.fmnodeinfo[MESH__GUARDHEAD].flags |= FMNI_NO_DRAW;
	self->s.fmnodeinfo[MESH__LHANDGRD].flags |= FMNI_NO_DRAW;
	self->s.fmnodeinfo[MESH__ARMSPIKES].flags |= FMNI_NO_DRAW;
	self->s.fmnodeinfo[MESH__SHOULDPAD].flags |= FMNI_NO_DRAW;

	MG_InitMoods(self);

	QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);

	self->melee_range *= self->s.scale;
}
