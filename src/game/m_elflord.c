//==============================================================================
//
// m_elflord.c
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
#include "g_playstats.h"
#include "m_elflord.h"
#include "m_elflord_anims.h"
#include "g_monster.h"
#include "m_stats.h"
#include "g_HitLocation.h"

/*QUAKED monster_elflord (1 .5 0) (-16 -16 -0) (16 16 32)
*/

//Mirrored on client side
enum
{
	CW_STAR,
	CW_STAR_HIT,
	CW_BEAM,
	CW_METEOR,
} cwatcher_effect_id_t;

void SpellCastSphereOfAnnihilation(edict_t *Caster,vec3_t StartPos,vec3_t AimAngles,vec3_t AimDir,
								  float Value,qboolean *ReleaseFlagsPtr);

/*----------------------------------------------------------------------
  Elf Lord Base Info
-----------------------------------------------------------------------*/

static animmove_t *animations[] =
{
	&elflord_move_idle,
	&elflord_move_run,
	&elflord_move_charge,
	&elflord_move_charge_trans,
	&elflord_move_floatback,
	&elflord_move_dodgeright,
	&elflord_move_dodgeleft,
	&elflord_move_soa_begin,
	&elflord_move_soa_loop,
	&elflord_move_soa_end,
	&elflord_move_ls,
	&elflord_move_pain,
	&elflord_move_death_btrans,
	&elflord_move_death_loop,
	&elflord_move_shield,
	&elflord_move_attack,
	&elflord_move_move,
	&elflord_move_wait,
	&elflord_move_come_to_life,
};

static int sounds[NUM_SOUNDS];

static ClassResourceInfo_t resInfo;

/*-----------------------------------------------
	elflord_projectile_blocked
-----------------------------------------------*/

void elflord_projectile_blocked( edict_t *self, trace_t *trace )
{
	vec3_t vel;

	if ( (!stricmp(trace->ent->classname, "elflord_projectile")) || (trace->ent == self->owner) )
		return;
	
	VectorNormalize2(self->velocity, vel);

	if (trace->ent && trace->ent->takedamage)
	{
		T_Damage(trace->ent, self->owner, self->owner, vel, trace->endpos, trace->plane.normal, irand(ELFLORD_STAR_MIN_DAMAGE, ELFLORD_STAR_MAX_DAMAGE), 0, DAMAGE_NORMAL, MOD_DIED);
	}

	//Create the star explosion
	gi.CreateEffect(	NULL, 
						FX_CWATCHER, 
						0,
						trace->endpos,
						"bv",
						CW_STAR_HIT,
						trace->plane.normal);
	
	self->think = G_FreeEdict;
	self->nextthink = level.time + 0.1;
}

/*-----------------------------------------------
	elford_Attack
-----------------------------------------------*/

void elford_Attack( edict_t *self )
{
	edict_t *projectile;
	vec3_t	aim, vf, vr, ang, org;
	float	len, yaw_ofs = -20;
	int		i;

	if (!M_ValidTarget(self, self->enemy))
		return;

	for(i=0;i<3;i++)
	{
		projectile = G_Spawn();

		projectile->classname = "elflord_projectile";
		projectile->solid = SOLID_BBOX;
		projectile->movetype = PHYSICSTYPE_FLY;
		projectile->clipmask = MASK_SHOT;
		
		AngleVectors(self->s.angles, vf, vr, NULL);
		VectorCopy(self->s.origin, projectile->s.origin);
		VectorMA(projectile->s.origin, 48, vf, projectile->s.origin);
		VectorMA(projectile->s.origin, 16, vr, projectile->s.origin);
		projectile->s.origin[2] += 8;

		VectorSet(projectile->mins, -2, -2, -2);
		VectorSet(projectile->maxs,  2,  2,  2);
		
		projectile->owner = self;
		projectile->svflags |= SVF_ALWAYS_SEND;
		
		VectorCopy(self->enemy->s.origin, org);
		
		M_PredictTargetPosition(self->enemy, self->enemy->velocity, skill->value * 2, org);

		org[2] += self->enemy->viewheight;

		VectorSubtract(org, projectile->s.origin, aim);
		len = VectorNormalize(aim);
		vectoangles(aim, ang);
		ang[YAW] += yaw_ofs;
		ang[PITCH] *= -1;
		yaw_ofs += 20;
		AngleVectors(ang, aim, NULL, NULL);

		VectorScale(aim, (600 + (skill->value * 100)), projectile->velocity);

		projectile->gravity = 0;

		gi.linkentity(projectile);

		gi.CreateEffect(	&projectile->s, 
							FX_CWATCHER, 
							CEF_OWNERS_ORIGIN,
							projectile->s.origin,
							"bv",
							CW_STAR,
							self->s.origin);
		
		projectile->isBlocking = projectile->bounced = projectile->isBlocked = elflord_projectile_blocked;
	}

	gi.sound(self, CHAN_WEAPON, sounds[SND_PROJ1], 1, ATTN_NORM, 0);
}

/*-----------------------------------------------
	elflord_StartBeam
-----------------------------------------------*/

void elflord_StartBeam(edict_t *self)
{
	edict_t *beam;
	trace_t	trace;
	vec3_t	endpos, ang, vr;
	vec3_t  mins = {-2, -2, -2};
	vec3_t  maxs = { 2,  2,  2};

	if (!M_ValidTarget(self, self->enemy))
		return;

	beam = G_Spawn();

	VectorCopy(self->s.angles, ang);
	ang[PITCH] *= -1;
	AngleVectors(ang, self->pos1, vr, NULL);

	VectorMA(self->s.origin, 32, self->pos1, self->pos2);
	self->pos2[2] -= 32;

	beam->classname = "elflord_Beam";
	beam->solid = SOLID_NOT;
	beam->movetype = PHYSICSTYPE_NONE;
	beam->owner = self;
	beam->svflags |= SVF_ALWAYS_SEND;
	
	VectorMA(self->s.origin, 640, self->pos1, endpos);
	gi.trace(self->s.origin, mins, maxs, endpos, self, MASK_SHOT, &trace);

	VectorCopy(trace.endpos, beam->s.origin);

	beam->pain_debounce_time = level.time + 5;

	gi.linkentity(beam);

	gi.CreateEffect(	&beam->s, 
						FX_CWATCHER, 
						CEF_OWNERS_ORIGIN,
						beam->s.origin,
						"bv",
						CW_BEAM,
						self->pos2);
	
	gi.sound(self, CHAN_VOICE, sounds[SND_BEAM], 0.5, ATTN_NONE, 0);

	self->targetEnt = beam;
}

/*-----------------------------------------------
	elflord_EndBeam
-----------------------------------------------*/

void elflord_EndBeam(edict_t *self)
{
	self->targetEnt->think = G_FreeEdict;
	self->targetEnt->nextthink = level.time + 0.1;
}

/*-----------------------------------------------
	elflord_decell
-----------------------------------------------*/

void elflord_decell(edict_t *self, float value)
{
	if (self->velocity[0] != 0.0 || self->velocity[1] != 0.0 || self->velocity[2] != 0.0)
	{
		self->velocity[0] *= value;
		self->velocity[1] *= value;
		self->velocity[2] *= value;

		if (abs(self->velocity[0]) < 1.0) 
			self->velocity[0] = 0.0;
		if (abs(self->velocity[1]) < 1.0) 
			self->velocity[1] = 0.0;
		if (abs(self->velocity[2]) < 1.0) 
			self->velocity[2] = 0.0;
	}
}

/*-----------------------------------------------
	elflord_ai_stand
-----------------------------------------------*/

void elflord_ai_stand (edict_t *self, float dist)
{
	ai_stand(self, 0);
	if (M_ValidTarget(self, self->enemy))
	{
		return;
	}
}


/**************************************/

/*-----------------------------------------------
	elflord_finish_death
-----------------------------------------------*/

void elflord_finish_death(edict_t *self)
{
	SetAnim(self, ANIM_DIE_LOOP);
}

/*-----------------------------------------------
	elfLordGoCharge
-----------------------------------------------*/

void elfLordGoCharge(edict_t *self)
{
	SetAnim(self, ANIM_CHARGE);
}

/*-----------------------------------------------
	elflord_soa_loop
-----------------------------------------------*/

void elflord_soa_loop(edict_t *self)
{
	SetAnim(self, ANIM_ATTACK_SOA_LOOP);
}

/*-----------------------------------------------
	elflord_soa_end
-----------------------------------------------*/

void elflord_soa_end(edict_t *self)
{
	self->show_hostile = false;
	gi.sound(self, CHAN_WEAPON, sounds[SND_SAFIRE], 1, ATTN_NORM, 0);
	SetAnim(self, ANIM_ATTACK_SOA_END);
}

/*-----------------------------------------------
	elflord_stand
-----------------------------------------------*/

void elflord_stand(edict_t *self, G_Message_t *msg)
{
	SetAnim(self, ANIM_HOVER);
}

/*-----------------------------------------------
	elflord_flymove
-----------------------------------------------*/

void elflord_flymove (edict_t *self, float dist)
{
	vec3_t forward;

	if (!M_ValidTarget(self, self->enemy))
		return;

	VectorSubtract(self->enemy->s.origin, self->s.origin, forward);
	
	self->ideal_yaw = vectoyaw(forward);
	
	M_ChangeYaw(self);
	
	AngleVectors(self->s.angles, forward, NULL, NULL);
	
	VectorMA(self->velocity, dist, forward, self->velocity);
	
	self->velocity[2] = self->enemy->s.origin[2] + 100 - self->absmin[2];

	if(!elfLordCheckAttack(self))
		MG_CheckEvade(self);
}

/*-----------------------------------------------
	elflordRandomRushSound
-----------------------------------------------*/

void elflordRandomRushSound(edict_t *self)
{
}

/*-----------------------------------------------
	elflord_run
-----------------------------------------------*/

void elflord_run(edict_t *self, G_Message_t *msg)
{
	SetAnim(self, ANIM_FLOAT_FORWARD);
}

/*-----------------------------------------------
	elflord_soa_start
-----------------------------------------------*/

void elflord_soa_start(edict_t *self, G_Message_t *msg)
{
	vec3_t	forward, startpos;

	if (!M_ValidTarget(self, self->enemy))
		return;

	gi.sound(self, CHAN_VOICE, sounds[SND_SACHARGE], 1, ATTN_NORM, 0);
	self->show_hostile = true;

	AngleVectors(self->s.angles, forward, NULL, NULL);
	VectorCopy(self->s.origin, startpos);
	SpellCastSphereOfAnnihilation(self,
								 startpos,
								 self->s.angles,		//v_angle,
								 forward,
								 0.0,
								 &self->show_hostile);
	SetAnim(self, ANIM_ATTACK_SOA_BTRANS);
}

/*-----------------------------------------------
	elflord_soa_charge
-----------------------------------------------*/

void elflord_soa_charge(edict_t *self)
{
	gi.sound(self, CHAN_VOICE, sounds[SND_SACHARGE], 1, ATTN_NORM, 0);
}

/*-----------------------------------------------
	elflord_soa_go
-----------------------------------------------*/

void elflord_soa_go(edict_t *self)
{
	vec3_t forward;
	
	gi.sound(self, CHAN_VOICE, sounds[SND_SAFIRE], 1, ATTN_NORM, 0);
	self->show_hostile = false;

	AngleVectors(self->s.angles, forward, NULL, NULL);
	SpellCastSphereOfAnnihilation(self,
								 self->s.origin,
								 self->s.angles,		//v_angle,
								 forward,
								 0.0,
								 &self->show_hostile);
}

/*-----------------------------------------------
	elflord_death_start
-----------------------------------------------*/

void elflord_death_start(edict_t *self, G_Message_t *msg)
{
	//Turn off a beam if it's on
	if (self->targetEnt)
		G_FreeEdict(self->targetEnt);

	self->health = 0;
	self->max_health = 0;
	M_ShowLifeMeter( self, 0, 0);

	self->think = G_FreeEdict;
	self->nextthink = level.time + 0.1;
}

/*-----------------------------------------------
	elflord_pain
-----------------------------------------------*/

void elflord_pain (edict_t *self, G_Message_t *msg)
{
	if (irand(0,9))
		return;

	if(!irand(0, 1))
		gi.sound(self, CHAN_VOICE, sounds[SND_PAIN1], 1, ATTN_NORM, 0);
	else
		gi.sound(self, CHAN_VOICE, sounds[SND_PAIN2], 1, ATTN_NORM, 0);
}

/*-----------------------------------------------
	elflordSound
-----------------------------------------------*/

void elflordSound(edict_t *self, float channel, float sndindex, float atten)
{
	gi.sound(self, channel, sounds[(int)(sndindex)], 1, atten, 0);
}

/*-----------------------------------------------
	elflord_FindMoveTarget
-----------------------------------------------*/

void elflord_FindMoveTarget (edict_t *self)
{
	edict_t *movetarg = NULL, *lastvalid = NULL;
	vec3_t	vel, target;
	float	len;

	while((movetarg = oldfindradius(movetarg, self->s.origin, 640)) != NULL)
	{
		//Must be a path_corner
		if (strcmp(movetarg->classname, "path_corner"))
			continue;

		//Must be a specified path_corner too
		if (movetarg->targetname && strcmp(movetarg->targetname, "elflord"))
			continue;

		if (vhlen(movetarg->s.origin, self->s.origin) < 64)
			continue;

		lastvalid = movetarg;

		if (irand(0,1))
			continue;
		
		//TODO: Determine a velocity to get us here
		VectorCopy(movetarg->s.origin, target);
		target[2] = self->s.origin[2];

		VectorSubtract(target, self->s.origin, vel);
		len = VectorNormalize(vel);

		len = ( ((len / 10) / FRAMETIME) * 2 );

		VectorScale(vel, len, self->velocity);

		return;
	}

	//We randomly skipped the last possible spot, so just use that
	if (lastvalid)
	{
		VectorCopy(lastvalid->s.origin, target);
		target[2] = self->s.origin[2];

		VectorSubtract(target, self->s.origin, vel);
		len = VectorNormalize(vel);

		len = ( ((len / 10) / FRAMETIME) * 2 );

		VectorScale(vel, len, self->velocity);
	}
}

/*-----------------------------------------------
	elflord_track
-----------------------------------------------*/

void elflord_track(edict_t *self)
{
	trace_t	trace;
	vec3_t	dir, newdir, endpos;
	vec3_t  mins = {-2, -2, -2};
	vec3_t  maxs = { 2,  2,  2};

	if (!M_ValidTarget(self, self->enemy))
	{
		//Remove the beam
		self->targetEnt->think = G_FreeEdict;
		self->targetEnt->nextthink = level.time + 0.1;
		
		//Don't finish what we were doing
		SetAnim(self, ANIM_HOVER);
		return;
	}

	VectorSubtract(self->enemy->s.origin, self->pos2, dir);
	VectorNormalize(dir);

	VectorScale(self->pos1, 3 - (skill->value * 0.5), newdir);
	VectorAdd(newdir, dir, newdir);
	VectorScale(newdir, 1 / ((3 - (skill->value * 0.5)) + 1), newdir);

	VectorNormalize(newdir);

	VectorMA(self->s.origin, 640, newdir, endpos);

	gi.trace(self->s.origin, mins, maxs, endpos, self, MASK_SHOT, &trace);

	if (trace.ent && trace.ent->takedamage)
	{ 
		T_Damage(trace.ent, self, self, newdir, trace.endpos, trace.plane.normal, irand(ELFLORD_BEAM_MIN_DAMAGE, ELFLORD_BEAM_MAX_DAMAGE), 0, DAMAGE_NORMAL, MOD_DIED);
	}

	VectorCopy(trace.endpos, self->targetEnt->s.origin);
	
	vectoangles(newdir, self->s.angles);

	ai_charge2(self, 0);

	VectorCopy(newdir, self->pos1);
}

/*-----------------------------------------------
	elflord_FixAngles
-----------------------------------------------*/

void elflord_FixAngles(edict_t *self)
{
	self->s.angles[PITCH] = 0;
}

/*-----------------------------------------------
	elflord_MoveToFinalPosition
-----------------------------------------------*/

void elflord_MoveToFinalPosition( edict_t *self )
{
	edict_t *movetarg = NULL;
	vec3_t	vel, target;
	float	len;

	while((movetarg = oldfindradius(movetarg, self->s.origin, 640)) != NULL)
	{
		//Must be a path_corner
		if (strcmp(movetarg->classname, "path_corner"))
			continue;

		//Must be a specified path_corner too
		if (movetarg->targetname && strcmp(movetarg->targetname, "elflord_final"))
			continue;

		VectorCopy(movetarg->s.origin, target);
		target[2] = self->s.origin[2];

		VectorSubtract(target, self->s.origin, vel);
		len = VectorNormalize(vel);

		len = ( ((len / 10) / FRAMETIME) * 2 );

		VectorScale(vel, len, self->velocity);

		return;
	}
}

/*-----------------------------------------------
	elfLordCheckAttack
-----------------------------------------------*/

qboolean elfLordCheckAttack (edict_t *self)
{
	int		chance, 
			p_chance = 0, 
			soa_chance = 0,  
			beam_chance = 0, 
			move_chance = 0;

	if (!M_ValidTarget(self, self->enemy))
	{
		SetAnim(self, ANIM_HOVER);
		return false;
	}

	elflord_decell(self, 0.8);

	if (self->count < self->max_health)
	{
		VectorClear(self->velocity);
		SetAnim(self, ANIM_COME_TO_LIFE);
		return false;
	}

	if (self->health < self->max_health / 3)
	{//Last stage
		if (!self->dmg)
		{
			elflord_MoveToFinalPosition(self);
			SetAnim(self, ANIM_MOVE);
			self->dmg = 1;
			return false;
		}

		if (coop->value)
		{
			p_chance	= 50;
			soa_chance	= 50;
			beam_chance = 0;
		}
		else
		{
			p_chance	= 5;
			soa_chance	= 5;
			beam_chance = 90;
		}
	}
	else if (self->health < self->max_health / 1.5)
	{//Second stage
		p_chance	= 25;
		soa_chance	= 75;
		beam_chance = 0;
	}
	else
	{//First stage
		p_chance	= 90;
		soa_chance	= 0;
		beam_chance = 0;
	}

	chance = irand(0,100);

	if(irand(0,100) < p_chance)
	{
		SetAnim(self, ANIM_ATTACK);
		return false;
	}
	else if(irand(0,100) < beam_chance)
	{
		SetAnim(self, ANIM_ATTACK_LS);
		return false;
	}
	else if(irand(0,100) < soa_chance)
	{
		SetAnim(self, ANIM_ATTACK_SOA_BTRANS);
		return false;
	}

	if (!self->dmg)
	{
		elflord_FindMoveTarget(self);
		SetAnim(self, ANIM_MOVE);
		return false;
	}

	return false;
}

/*-----------------------------------------------
	elfLordPause
-----------------------------------------------*/

void elfLordPause(edict_t *self)
{
	elfLordCheckAttack(self);
}

/*-----------------------------------------------
	elfLordWakeUp
-----------------------------------------------*/

void elfLordWakeUp (edict_t *self, G_Message_t *msg)
{
	SetAnim(self, ANIM_COME_TO_LIFE);
}

/*-----------------------------------------------
	elflord_face
-----------------------------------------------*/

void elflord_face(edict_t *self)
{
	if (!M_ValidTarget(self, self->enemy))
		return;

	ai_charge2(self, 0);
}

static ClassResourceInfo_t resInfo;

/*-----------------------------------------------
	elflord_SlideMeter
-----------------------------------------------*/

void elflord_SlideMeter( edict_t *self )
{
	self->velocity[2] = 32;

	if (self->count < self->max_health)
	{
		M_ShowLifeMeter( self, self->count, self->count);
		self->count += self->max_health / 20;
	}
}

/*-----------------------------------------------
	elflord_PreThink
-----------------------------------------------*/

void elflord_PreThink( edict_t *self )
{
	if (self->enemy && self->count >= self->max_health)
	{
		M_ShowLifeMeter( self, self->health, self->max_health);
	}

	self->next_pre_think = level.time + 0.1;
}

/*-----------------------------------------------
	ElflordStaticsInit
-----------------------------------------------*/

void ElflordStaticsInit()
{
	classStatics[CID_ELFLORD].msgReceivers[MSG_STAND] = elflord_stand;
	classStatics[CID_ELFLORD].msgReceivers[MSG_RUN] = elflord_run;
	classStatics[CID_ELFLORD].msgReceivers[MSG_FLY] = elflord_run;
	classStatics[CID_ELFLORD].msgReceivers[MSG_DEATH] = elflord_death_start;
	classStatics[CID_ELFLORD].msgReceivers[MSG_MISSILE] = elflord_soa_start;
	classStatics[CID_ELFLORD].msgReceivers[MSG_PAIN] = elflord_pain;
	classStatics[CID_ELFLORD].msgReceivers[MSG_SIGHT] = elfLordWakeUp;

	resInfo.numAnims = NUM_ANIMS;
	resInfo.animations = animations;
	resInfo.modelIndex = gi.modelindex("models/monsters/elflord/tris.fm");

	sounds[SND_PAIN1] = gi.soundindex ("monsters/elflord/pain1.wav");	
	sounds[SND_PAIN2] = gi.soundindex ("monsters/elflord/pain2.wav");	
	sounds[SND_DIE] = gi.soundindex ("monsters/elflord/death1.wav");	

	//use sphere sounds
	sounds[SND_SACHARGE] = gi.soundindex ("weapons/SphereGrow.wav");
	sounds[SND_SAFIRE] = gi.soundindex ("weapons/SphereFire.wav");
	sounds[SND_SAHIT] = gi.soundindex ("weapons/SphereImpact.wav");

	sounds[SND_PROJ1] = gi.soundindex ("monsters/elflord/shoot.wav");
	sounds[SND_BEAM] = gi.soundindex ("monsters/elflord/beam.wav");

	resInfo.numSounds = NUM_SOUNDS;
	resInfo.sounds = sounds;

	classStatics[CID_ELFLORD].resInfo = &resInfo;
}

/*QUAKED SP_monster_elflord (0.5 0.5 1) (-24 -24 -64) (24 24 16)

Celestial Watcher

"wakeup_target" - monsters will fire this target the first time it wakes up (only once)

"pain_target" - monsters will fire this target the first time it gets hurt (only once)

*/
void SP_monster_elflord (edict_t *self)
{
	// Generic Monster Initialization
	if (!flymonster_start(self))		
		return;							// Failed initialization

	self->msgHandler = DefaultMsgHandler;
	self->classID = CID_ELFLORD;

	if (!self->health)
		self->health = ELFLORD_HEALTH;

	self->max_health = self->health = MonsterHealth(self->health);

	self->mass = ELFLORD_MASS;
	self->yaw_speed = 20;

	self->movetype=PHYSICSTYPE_STEP;
	self->flags |= FL_FLY;
	self->gravity = 0.0;
	self->clipmask= MASK_MONSTERSOLID;
	self->svflags |= SVF_ALWAYS_SEND|SVF_BOSS|SVF_TAKE_NO_IMPACT_DMG;
	self->materialtype = MAT_FLESH;
	self->solid=SOLID_BBOX;

	VectorSet(self->mins, -24, -24, -64);
	VectorSet(self->maxs,  24,  24, 16);

	VectorClear(self->velocity);

	self->s.modelindex = classStatics[CID_ELFLORD].resInfo->modelIndex;

	self->dmg = 0;
	self->pre_think = elflord_PreThink;
	self->s.skinnum = 0;
	self->monsterinfo.scale = 2.0;

	self->count = 1;
	self->monsterinfo.otherenemyname = "player";

	self->s.scale = 2.0;

	QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);

	self->next_pre_think = level.time + 0.1;

	self->s.renderfx |= RF_GLOW;

	gi.linkentity(self);
}
