//==============================================================================
//
// m_plagueSsitra.c
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

#include "m_plaguessithra.h"
#include "m_plaguessithra_anim.h"
#include "g_misc.h"
#include "g_HitLocation.h"
#include "m_stats.h"


void extrapolateFiredir (edict_t *self,vec3_t p1,float pspeed,edict_t *targ,float accept,vec3_t vec2);
void ssithra_blocked (edict_t *self, trace_t *trace);
qboolean visible_pos (edict_t *self, vec3_t spot2);
void create_ssith_arrow(edict_t *Arrow);
qboolean ssithraCheckInWater (edict_t *self);

enum 
{
	FX_SS_MAKE_ARROW,
	FX_SS_MAKE_ARROW2,
	FX_SS_EXPLODE_ARROW,
	FX_SS_EXPLODE_ARROW2
};

int Bit_for_MeshNode [16] =
{
	BIT_POLY,
	BIT_LOWERTORSO,
	BIT_CAPLOWERTORSO,
	BIT_LEFTLEG,
	BIT_RIGHTLEG,
	BIT_UPPERTORSO,
	BIT_CAPTOPUPPERTORSO,
	BIT_CAPBOTTOMUPPERTORSO,
	BIT_LEFTARM,
	BIT_RIGHTARM,
	BIT_HEAD,
	BIT_CENTERSPIKE,
	BIT_LEFT1SPIKE,
	BIT_RIGHT1SPIKE,
	BIT_RIGHT2SPIKE,
	BIT_CAPHEAD
};

//========================================
//INITIALIZE
//========================================
static animmove_t *animations[NUM_ANIMS] =
{
	&ssithra_move_idle1,
	&ssithra_move_walk1,
	&ssithra_move_backpedal1,
	&ssithra_move_bound1,
	&ssithra_move_death_a1,
	&ssithra_move_death_b1,
	&ssithra_move_dive1,
	&ssithra_move_duckshoot1,
	&ssithra_move_duck1,
	&ssithra_move_gallop1,
	&ssithra_move_fjump,
	&ssithra_move_idlebasic1,
	&ssithra_move_idleright1,
	&ssithra_move_melee1,
	&ssithra_move_meleest,
	&ssithra_move_namor1,
	&ssithra_move_pain_a1,
	&ssithra_move_shoot1,
	&ssithra_move_startle1,
	&ssithra_move_swimforward1,
	&ssithra_move_swimwander,
	&ssithra_move_water_death1,//19
	&ssithra_move_water_idle1,
	&ssithra_move_water_pain_a1,
	&ssithra_move_water_pain_b1,
	&ssithra_move_water_shoot1,
	&ssithra_move_run1,
	&ssithra_move_spinright,
	&ssithra_move_spinright_go,
	&ssithra_move_spinleft,
	&ssithra_move_spinleft_go,
	&ssithra_move_faceandnamor,
	&ssithra_move_dead_a,
	&ssithra_move_lookright,
	&ssithra_move_lookleft,
	&ssithra_move_transup,
	&ssithra_move_transdown,
	&ssithra_move_headless,
	&ssithra_move_headlessloop,
	&ssithra_move_death_c,
	&ssithra_move_dead_b,
	&ssithra_move_dead_water,
	&ssithra_move_sliced,
	&ssithra_move_delay,
	&ssithra_move_duckloop,
	&ssithra_move_unduck,
	&ssithra_move_lunge
};

static int Sounds[NUM_SOUNDS];

static ClassResourceInfo_t resInfo;


void ssithra_blocked (edict_t *self, trace_t *trace)
{
	vec3_t hitdir;
	float strength;

	if(trace->ent == NULL)
		return;

	if(trace->ent->movetype==PHYSICSTYPE_NONE||
		trace->ent->movetype==PHYSICSTYPE_PUSH)
		return;

	strength = VectorLength(self->velocity);

	if(strength<50)
		return;

//	gi.dprintf("ssithra shove!\n");
	VectorCopy(self->velocity, hitdir);

//	if(!stricmp(trace->ent->classname, "player"))
//		KnockDownPlayer(&trace->ent->client->playerinfo);
	if(hitdir[2] < 0)
		hitdir[2] = 0;
	VectorNormalize(hitdir);
	VectorScale(hitdir, strength, hitdir);
	VectorAdd(trace->ent->velocity, hitdir, trace->ent->knockbackvel);

	if(!(self->spawnflags & MSF_FIXED))
		ssithraJump(self, 150, 200, 0);
}

//========================================
//MOVEMENT
//========================================

void ssithra_stand(edict_t *self, G_Message_t *msg)
{
	int inwater;

	if (self->ai_mood == AI_MOOD_DELAY)
	{
		SetAnim(self, ANIM_DELAY);
		return;
	}

	inwater = ssithraCheckInWater(self);

	if(inwater)
	{
		SetAnim(self, ANIM_WATER_IDLE);
	}
	else
	{
		if(self->curAnimID == ANIM_STAND1)
		{
			if(flrand(0,10)<8)//9
				SetAnim(self, ANIM_STAND1);
			else
				SetAnim(self, ANIM_IDLEBASIC);
		}
		else if(self->curAnimID == ANIM_IDLEBASIC)
		{
			SetAnim(self, ANIM_STAND1);
		}
		else if(self->curAnimID == ANIM_IDLERIGHT)
		{
			if(flrand(0,10)<6)//7
				SetAnim(self, ANIM_STAND1);
			else
				SetAnim(self, ANIM_IDLEBASIC);
		}
		else
			SetAnim(self, ANIM_STAND1);
	}
}

void ssithra_walk(edict_t *self, G_Message_t *msg)
{
	if(self->spawnflags&MSF_FIXED)
		SetAnim(self, ANIM_DELAY);
	else 
		SetAnim(self, ANIM_WALK1);
}

void ssithra_spinright_go(edict_t *self)
{//ANIM_STARTLE?
//	gi.dprintf("GO: Spinning right\n");
	SetAnim(self,ANIM_SPINRIGHT_GO);
}

void ssithra_spinleft_go(edict_t *self)
{//ANIM_STARTLE?
//	gi.dprintf("GO: Spinning right\n");
	SetAnim(self,ANIM_SPINLEFT_GO);
}

void ssithraStartle (edict_t *self)
{
//	gi.dprintf("Startled!\n");
	SetAnim(self,ANIM_STARTLE);
}

void ssithraLookRight (edict_t *self)
{
//	gi.dprintf("Startled to right!\n");
	SetAnim(self,ANIM_IDLERIGHT);
}

void ssithra_gallop(edict_t *self, G_Message_t *msg)
{
	if(self->curAnimID == ANIM_SPINRIGHT)
	{
		SetAnim(self, ANIM_SPINRIGHT_GO);
		return;
	}

	if(self->curAnimID == ANIM_SPINLEFT)
	{
		SetAnim(self, ANIM_SPINLEFT_GO);
		return;
	}

	if(!self->enemy)
	{
		SetAnim(self,ANIM_STAND1);
		return;
	}

	if(self->enemy->health<=0)
	{
		SetAnim(self,ANIM_STAND1);
		return;
	}

	if(self->spawnflags&MSF_FIXED)
	{
		SetAnim(self, ANIM_DELAY);
		return;
	}

	if(self->spawnflags&MSF_SSITHRA_NAMOR)//Namor
	{
		self->spawnflags &= ~MSF_SSITHRA_NAMOR;
		SetAnim(self,ANIM_NAMOR);
	}
	else if(self->spawnflags&MSF_SSITHRA_SPIN)//Spin
	{
		self->spawnflags &= ~MSF_SSITHRA_SPIN;
		if(irand(0,1))
		{
//			gi.dprintf("Spinning right\n");
			SetAnim(self,ANIM_SPINRIGHT);
		}
		else
		{
//			gi.dprintf("Spinning leftt\n");
			SetAnim(self,ANIM_SPINLEFT);
		}
	}
	else
		SetAnim(self, ANIM_RUN1);
}

void ssithra_idlebasic(edict_t *self, G_Message_t *msg)
{
	SetAnim(self, ANIM_IDLEBASIC);
}

void ssithra_decide_stand(edict_t *self)
{
	int inwater;

	inwater = ssithraCheckInWater(self);

	if(inwater)
	{
		SetAnim(self, ANIM_WATER_IDLE);
	}
	else
	{
		if((self->curAnimID == ANIM_STAND1)||(self->curAnimID == ANIM_IDLEBASIC))
		{
			if(flrand(0,10)<7)//9
				SetAnim(self, ANIM_STAND1);
			else if(flrand(0,10)<7)//9
				SetAnim(self, ANIM_IDLERIGHT);
			else if(flrand(0,10)<5)//9
				SetAnim(self, ANIM_LOOKRIGHT);
			else
				SetAnim(self, ANIM_LOOKLEFT);
		}
		else if(self->curAnimID == ANIM_IDLERIGHT||
			self->curAnimID == ANIM_LOOKLEFT||
			self->curAnimID == ANIM_LOOKRIGHT)
		{
			if(flrand(0,10)<6)//7
				SetAnim(self, ANIM_STAND1);
			else
				SetAnim(self, ANIM_IDLEBASIC);
		}
		else
			SetAnim(self, ANIM_STAND1);
	}
}

void ssithra_decide_gallop(edict_t *self)
{
	if(self->spawnflags & MSF_FIXED)
	{
		SetAnim(self, ANIM_DELAY);
		return;
	}

	VectorClear(self->velocity);
	self->count = false;

	if(ssithraCheckInWater(self))
	{
		SetAnim(self, ANIM_SWIMFORWARD);
	}
	else
	{
		SetAnim(self, ANIM_RUN1);
	}

	SsithraCheckMood(self);
}

void ssithra_decide_swimforward(edict_t *self)
{//fixme: climb out of water check!
	self->count = false;
	VectorClear(self->velocity);

	if(!ssithraCheckInWater(self))
	{//Not actually in water!
		SetAnim(self, ANIM_RUN1);
	}
	else if(self->curAnimID == ANIM_WATER_SHOOT)
		SetAnim(self, ANIM_TRANSDOWN);

	SsithraCheckMood(self);
}

void ssithra_decide_backpedal(edict_t *self)
{
	SsithraCheckMood(self);
}

void ssithraCheckRipple (edict_t *self)
{
	vec3_t	top, bottom;
	vec3_t	dir;
	trace_t trace;
	byte		angle_byte;

	// no ripples while in cinematics
	if (sv_cinematicfreeze->value)
		return;

	VectorCopy(self->s.origin, top);
	VectorCopy(top, bottom);
	top[2] += self->maxs[2] * 0.75;
	bottom[2] += self->mins[2];

	gi.trace(top, vec3_origin, vec3_origin, bottom, self, MASK_WATER,&trace);

	if(trace.fraction >= 1.0)
		return;

	AngleVectors(self->s.angles,dir,NULL,NULL);
	VectorScale(dir,200,dir);
	angle_byte = Q_ftol(((self->s.angles[YAW] + DEGREE_180)/360.0) * 255.0);

	gi.CreateEffect(NULL, FX_WATER_WAKE, 0,	trace.endpos, "sbv", self->s.number,
		angle_byte, dir);
}

//========================================
//SUPPORT CODE
//========================================

qboolean ssithraCheckInWater (edict_t *self)
{
	qboolean inwater = false;
	
	if(self->flags & FL_INWATER)
	{
		if(!(self->flags & FL_INLAVA))
		{
			if(!(self->flags & FL_INSLIME))
			{
				if(self->waterlevel > 2 || !self->groundentity)//???
				{
					inwater = true;
				}
			}
		}
	}

	if(inwater)
	{
		self->monsterinfo.aiflags |= AI_NO_MELEE;
		return true;
	}
	else
	{
		if(!(self->s.fmnodeinfo[MESH__LEFTARM].flags&FMNI_NO_DRAW))
			self->monsterinfo.aiflags &= ~AI_NO_MELEE;
		return false;
	}
}

void ssithraNamorTriggered (edict_t *self, edict_t *other, edict_t *activator)
{//FIXME: might work anyway?
	SetAnim(self, ANIM_NAMOR);
	monster_use(self, other, activator);
}

void ssithraVOfs(edict_t *self, float pofs, float yofs, float rofs)
{
	self->v_angle_ofs[PITCH] = pofs;
	self->v_angle_ofs[YAW] = yofs;
	self->v_angle_ofs[ROLL] = rofs;
}


//========================================
//JUMPS
//make a find_water_top function... pass a vector
//========================================

qboolean ssithraWaterLedgeNearEnemy (edict_t *self)
{
	vec3_t enemy_dir, endpos, targ_org;
	trace_t trace;

	if(self->spawnflags & MSF_FIXED)
		return false;

	if(!MG_GetTargOrg(self, targ_org))
		return false;

	VectorSubtract(targ_org,self->s.origin,enemy_dir);

	VectorNormalize(enemy_dir);
	VectorMA(self->s.origin,128,enemy_dir,endpos);
	gi.trace(self->s.origin,self->mins,self->maxs,endpos,self,MASK_SOLID,&trace);
	if(trace.fraction<1.0)
		return true;
	//no ledge to jump up on
	return false;
}

void ssithra_check_namor(edict_t *self)
{//fixme: climb out of water check!
	qboolean enemy_inwater = false;
	vec3_t	targ_org;

	if(self->spawnflags & MSF_FIXED)
		return;

	if(!MG_GetTargOrg(self, targ_org))
		return;
	
	enemy_inwater = (gi.pointcontents(targ_org)&CONTENTS_WATER);

	if(!enemy_inwater)
	{
		if(visible_pos(self, targ_org))
		{
			if(ssithraWaterLedgeNearEnemy(self))
			{
#ifdef _DEVEL
				if(MGAI_DEBUG)
					gi.dprintf("Ssithra facing and namor\n");
#endif
				SetAnim(self,ANIM_FACEANDNAMOR);
			}
		}
	}
}

void ssithraWhichJump(edict_t *self)
{
	vec3_t	Forward, movedir;
	vec3_t	targ_org;

	if(self->spawnflags & MSF_FIXED)
		return;

	if(!MG_GetTargOrg(self, targ_org))
		return;

	if(ssithraCheckInWater(self)&&!(gi.pointcontents(targ_org)&CONTENTS_WATER))
	{
		if(MGAI_DEBUG)
			gi.dprintf("Ssithra_whichjump->namor\n");
		SetAnim(self,ANIM_NAMOR);
	}
	else
	{
		int inwater;

		inwater = ssithraCheckInWater(self);

		if(inwater)
			return;

//		gi.dprintf("Bound\n");
		SetAnim(self,ANIM_BOUND);
		VectorCopy(self->s.angles, movedir);
		//VectorSet(movedir, 0, yaw, 0);
		//movedir[YAW] = yaw;
		AngleVectors(movedir,Forward,NULL,NULL);
		VectorScale(Forward,SSITHRA_HOP_VELOCITY,self->velocity);
		self->velocity[2]=SSITHRA_HOP_VELOCITY + 32;
//		self->movetype=MOVETYPE_TOSS;
	}
}

void ssithraMsgJump(edict_t *self, G_Message_t *msg)
{
	if(self->spawnflags&MSF_FIXED)
		SetAnim(self, ANIM_DELAY);
	else 
		ssithraWhichJump(self);
}

void ssithraBoundCheck (edict_t *self)
{//fixme: do checks and traces first
	vec3_t forward, up, startpos, endpos, mins, maxs;
	trace_t trace;
	int inwater;

	if(self->spawnflags & MSF_FIXED)
		return;

	inwater = ssithraCheckInWater(self);

	if(inwater)
	{
		if(self->curAnimID!=ANIM_SWIMFORWARD)
			SetAnim(self,ANIM_SWIMFORWARD);
		return;
	}

	AngleVectors(self->s.angles,forward,NULL,up);
	VectorCopy(self->s.origin,startpos);
	
	VectorMA(startpos,48,forward,endpos);//forward
	gi.trace(startpos,self->mins,self->maxs,endpos,self,MASK_SOLID,&trace);

	VectorCopy(trace.endpos,startpos);
	VectorMA(startpos,-128,up,endpos);//down
	gi.trace(startpos,self->mins,self->maxs,endpos,self,MASK_SOLID|MASK_WATER,&trace);
	
	//If it's a step down or less, no jumpie
	if(Q_fabs(trace.endpos[2] - self->s.origin[2]) <= 18)
		return;

	if(trace.fraction == 1.0 || trace.allsolid || trace.startsolid)
		return;//too far to jump down, or in solid

	if(trace.contents&CONTENTS_WATER||trace.contents&CONTENTS_SLIME)
	{
		VectorCopy(trace.endpos,startpos);
		VectorMA(startpos,-64,up,endpos);//down from water surf
		VectorCopy(self->mins, mins);
		VectorCopy(self->maxs, maxs);
		mins[2] = 0;
		maxs[2] = 1;
		
		gi.trace(startpos, mins, maxs, endpos, self, MASK_SOLID,&trace);
		if(trace.fraction<1.0 || trace.allsolid || trace.startsolid)
			ssithraWhichJump(self);
		else	
			SetAnim(self,ANIM_DIVE);
	}
	else
		SetAnim(self,ANIM_GALLOP);
		//ssithraJump(self,100,50,0);
}

void ssithraDiveCheck (edict_t *self)
{//fixme: do checks and traces first
	vec3_t forward, up, startpos, endpos, mins, maxs;
	trace_t trace;
	int inwater;
	vec3_t	targ_org, targ_mins;

	if(self->spawnflags & MSF_FIXED)
		return;

	if (self->monsterinfo.searchType == SEARCH_BUOY)
	{
		if(self->buoy_index < 0 || self->buoy_index > level.active_buoys)
			return;

		VectorCopy(level.buoy_list[self->buoy_index].origin, targ_org);

		VectorClear(targ_mins);
	}
	else
	{
		if(!self->goalentity)
			return;
		
		VectorCopy(self->goalentity->s.origin, targ_org);
		VectorCopy(self->goalentity->mins, targ_mins);
	}

	inwater = ssithraCheckInWater(self);

	if(inwater)
	{
		SetAnim(self,ANIM_SWIMFORWARD);
		return;
	}

	if(!infront_pos(self, targ_org))
		return;

	//make sure the enemy isn't right here and accessible before diving in
	if(vhlen(targ_org,self->s.origin)<96)//close
		if(Q_fabs((targ_org[2]+targ_mins[2]) - (self->s.origin[2] + self->mins[2]))<18)//relatively same stephieght
			if(!(gi.pointcontents(targ_org)&CONTENTS_WATER))
				if(!(self->monsterinfo.aiflags & AI_FLEE))
					return;

	AngleVectors(self->s.angles,forward,NULL,up);
	VectorCopy(self->s.origin,startpos);
	
	VectorMA(startpos,48,forward,endpos);//forward
	gi.trace(startpos,self->mins,self->maxs,endpos,self,MASK_SOLID,&trace);

	VectorCopy(trace.endpos,startpos);
	VectorMA(startpos,-128,up,endpos);//down
	gi.trace(startpos,self->mins,self->maxs,endpos,self,MASK_SOLID|MASK_WATER,&trace);
	
	if(trace.fraction == 1||trace.allsolid||trace.startsolid)
		return;//too far to jump down, or in solid

	if(trace.contents&CONTENTS_WATER||trace.contents&CONTENTS_SLIME)
	{
		VectorCopy(trace.endpos, startpos);
		VectorMA(startpos, -64, up, endpos);//down from water surf
		VectorCopy(self->mins, mins);
		VectorCopy(self->maxs, maxs);
		mins[2] = 0;
		maxs[2] = 1;
		
		gi.trace(startpos, mins, maxs, endpos, self, MASK_SOLID,&trace);

		if(trace.fraction<1.0 || trace.allsolid || trace.startsolid)
			ssithraWhichJump(self);
		else	
			SetAnim(self, ANIM_DIVE);
	}
}

void ssithraApplyJump (edict_t *self)
{
	if(self->spawnflags & MSF_FIXED)
		return;

	self->jump_time = level.time + 1;
	VectorCopy(self->movedir, self->velocity);
	VectorNormalize(self->movedir);
}

void ssithraJump (edict_t *self, float upspd,float fwdspd,float rtspd)
{//fixme: do checks and traces first
	vec3_t up, forward, right;

	if(self->spawnflags & MSF_FIXED)
		return;

	if((self->s.fmnodeinfo[MESH__LEFTLEG].flags & FMNI_NO_DRAW)||
		(self->s.fmnodeinfo[MESH__RIGHTLEG].flags & FMNI_NO_DRAW))
	{
		upspd*=2;
		fwdspd/=2;
	}

	AngleVectors(self->s.angles,forward,right,up);
	VectorMA(self->velocity,upspd,up,self->velocity);
	VectorMA(self->velocity,fwdspd,forward,self->velocity);
	VectorMA(self->velocity,rtspd,right,self->velocity);
}

void ssithraNamorJump (edict_t *self)
{
	trace_t trace;
	vec3_t top;
	float watersurfdist, enemyzdiff;
	vec3_t targ_org;

	if(self->spawnflags & MSF_FIXED)
		return;

	if(!MG_GetTargOrg(self, targ_org))
		return;

	if(MGAI_DEBUG)
		gi.dprintf("Namor Jump\n");

	//FIXME: jumps too high sometimes?
	self->count = false;
	VectorCopy(self->s.origin,top);
	top[2]+=512;
	gi.trace(self->s.origin, vec3_origin, vec3_origin, top, self, MASK_SOLID, &trace);
	VectorCopy(trace.endpos,top);
	gi.trace(top, vec3_origin, vec3_origin, self->s.origin, self, MASK_SOLID|MASK_WATER, &trace);

	//How far above my feet is waterlevel?
	VectorSubtract(trace.endpos, self->s.origin, top);
	watersurfdist = VectorLength(top) - self->mins[2];//adjust for my feet

	//how high above water level is player?
	enemyzdiff = targ_org[2] - trace.endpos[2];

	//FIXME: aim a little to side if enemy close so don't 
	//land on top of him?  Or hit him if land on top?
	ssithraJump(self,watersurfdist*2 + enemyzdiff*2 + 200, 100, 0);
}



void ssithraCheckJump (edict_t *self)
{
	vec3_t		vf, source, source2;
	vec3_t		maxs, mins, save_org;
	trace_t		trace;
	float		hgt_diff, jump_fdist;
	qboolean	jump_up_check = false;
	qboolean	check_down = false;
	qboolean	can_move = false;
	vec3_t	targ_org, targ_mins;

	if(self->spawnflags & MSF_FIXED)
		return;

	if (self->monsterinfo.searchType == SEARCH_BUOY)
	{
		if(self->buoy_index < 0 || self->buoy_index > level.active_buoys)
			return;

		VectorCopy(level.buoy_list[self->buoy_index].origin, targ_org);

		VectorClear(targ_mins);
	}
	else
	{
		if(!self->goalentity)
			return;
		
		VectorCopy(self->goalentity->s.origin, targ_org);
		VectorCopy(self->goalentity->mins, targ_mins);
	}

	if (!(infront_pos(self, targ_org)))
		return;

	if (targ_org[2] < self->s.origin[2] - 28)
	{
		check_down = true;
	}
	else
	{
		check_down = false;
	}

	if (check_down)
	{//jumping down
		//Setup the trace
		int inwater;

		inwater = ssithraCheckInWater(self);

		if(MGAI_DEBUG)
			gi.dprintf("checking jump down: ");

		if(inwater)
		{
			if(MGAI_DEBUG)
				gi.dprintf("checkdown allsolid\n");
			return;
		}

		VectorCopy(self->maxs, maxs);
		VectorCopy(self->s.origin, source);
		AngleVectors(self->s.angles, vf, NULL, NULL);

		if(self->monsterinfo.aiflags&AI_FLEE||self->monsterinfo.aiflags&AI_COWARD)
			jump_fdist = 128;
		else
			jump_fdist = vhlen(targ_org, self->s.origin);
		
		if(jump_fdist > 128)
			jump_fdist = 128;

		VectorMA(source, 128, vf, source);

		maxs[2] += 32;
		gi.trace (self->s.origin, self->mins, maxs, source, self, MASK_MONSTERSOLID,&trace);

		if (trace.fraction == 1)
		{//clear ahead and above
			VectorCopy(source, source2);

			source2[2] -= 1024;
			//trace down
			gi.trace (source, self->mins, self->maxs, source2, self, MASK_ALL,&trace);
			
			if (trace.allsolid || trace.startsolid)
			{
#ifdef _DEVEL
				if(MGAI_DEBUG)
					gi.dprintf("checkdown allsolid\n");
#endif
				return;
			}

			if (trace.fraction == 1)
			{
#ifdef _DEVEL
				if(MGAI_DEBUG)
					gi.dprintf("checkdown- too far\n");
#endif
				return;
			}
			else
			{
//				if (trace.ent == (struct edict_s *)-1)
//					return;

				if (trace.contents != CONTENTS_SOLID)
				{
#ifdef _DEVEL
					if(MGAI_DEBUG)
						gi.dprintf("checkjump trying to jump into water\n");
#endif
					if ((trace.contents&CONTENTS_WATER||trace.contents&CONTENTS_SLIME)||trace.ent == self->enemy)
					{//jumping into water
						VectorSubtract(trace.endpos, self->s.origin, source2);
						VectorNormalize(source2);
						self->ideal_yaw = vectoyaw(source2);

						if (self->monsterinfo.jump_time < level.time)
						{//check depth
							VectorCopy(trace.endpos, source);
							VectorCopy(source, source2);
							source2[2] -= 64;
							VectorCopy(self->mins, mins);
							VectorCopy(self->maxs, maxs);
							mins[2] = 0;
							maxs[2] = 1;

							gi.trace (source, mins, maxs, source2, self, MASK_SOLID,&trace);
							if(trace.fraction < 1.0 || trace.startsolid || trace.allsolid)
								ssithraWhichJump(self);
							else
								SetAnim(self,ANIM_DIVE);
							self->monsterinfo.jump_time = level.time + 1;
						}
					}
				}
				else
				{
#ifdef _DEVEL
					if(MGAI_DEBUG)
						gi.dprintf("checkjump down->whichjump\n");
#endif
					VectorSubtract(trace.endpos, self->s.origin, source2);
					VectorNormalize(source2);
					self->ideal_yaw = vectoyaw(source2);
					
					if (self->monsterinfo.jump_time < level.time)
					{
						ssithraWhichJump(self);
						self->monsterinfo.jump_time = level.time + 1;
					}
				}
			}
		}
#ifdef _DEVEL
		else if(MGAI_DEBUG)
			gi.dprintf("checkdown: not clear infront\n");
#endif
	}
	else
	{
		if(vhlen(self->s.origin, targ_org)<200)
			jump_up_check = true;
		else
		{
			VectorCopy(self->s.origin,source);
			source[2]-=10;
			if(gi.pointcontents(source)&CONTENTS_WATER)
			{//FIXME: swimming can bring origin out of water!  in water
				AngleVectors(self->s.angles, vf, NULL, NULL);
				VectorMA(self->s.origin, 72, vf, source);
				gi.trace (self->s.origin, self->mins, self->maxs, source, self, MASK_SOLID,&trace);
				if(trace.fraction<1.0)
					jump_up_check = true;
				//shore is within 72 units of me
			}
			else//enemy far away, in front, and water in front of me
			{//check if water in front
				AngleVectors(self->s.angles, vf, NULL, NULL);
				VectorMA(self->s.origin, 48, vf, source);
				gi.trace (self->s.origin, self->mins, self->maxs, source, self, MASK_SOLID,&trace);
				VectorCopy(trace.endpos,source);
				source[2]-=128;
				gi.trace (trace.endpos, self->mins, self->maxs, source, self, MASK_SOLID|MASK_WATER,&trace);
				if(trace.fraction<1.0&&trace.contents&CONTENTS_WATER)
				{
					VectorCopy(trace.endpos, source);
					VectorCopy(source, source2);
					source[2] -= 64;
					VectorCopy(self->mins, mins);
					VectorCopy(self->maxs, maxs);
					mins[2] = 0;
					maxs[2] = 1;
					
					gi.trace(source, mins, maxs, source2, self, MASK_SOLID,&trace);
					if(trace.fraction<1.0 || trace.allsolid || trace.startsolid)
						ssithraWhichJump(self);
					else	
						SetAnim(self,ANIM_DIVE);

					return;
				}
			}
		}

		if(jump_up_check)
		{
			if(targ_org[2] > self->s.origin[2]+28||
				!(self->monsterinfo.aiflags & AI_FLEE))//||override_dist)
			{//jumping up
				//Setup the trace
//				gi.dprintf("checking for jump up or into water\n");
				VectorCopy(self->maxs, maxs);
				VectorCopy(self->s.origin, source);

				//um,. what about if runing away?
				hgt_diff = (targ_org[2] + targ_mins[2]) - (self->s.origin[2] + self->mins[2]) + 32;
				source[2] += hgt_diff;
				gi.trace (self->s.origin, self->mins, self->maxs, source, self, MASK_ALL,&trace);

				if (trace.fraction == 1)
				{//clear above
					VectorCopy(source, source2);

					AngleVectors(self->s.angles, vf, NULL, NULL);
					VectorMA(source, 64, vf, source2);
					source2[2] -= 24;
					//trace forward and down a little
					gi.trace (source, self->mins, self->maxs, source2, self, MASK_ALL,&trace);
					
					if (trace.allsolid || trace.startsolid)
						return;

					if (trace.fraction < 0.1)
					{
//						gi.dprintf("Can't jump up, no ledge\n");
						return;
					}
		//			{
		//				if (stricmp(trace.ent->classname, "worldspawn"))
		//					return;
		//			}
					else
					{
						if (trace.ent == (struct edict_s *)-1)
							return;

		//				if (trace.contents != CONTENTS_SOLID)
		//					return;
		//				else
						{
							VectorSubtract(trace.endpos, self->s.origin, source2);
							VectorNormalize(source2);
							self->ideal_yaw = vectoyaw(source2);
							
							if (self->monsterinfo.jump_time < level.time)
							{
//								gi.dprintf("checkjump up ->whichjump\n");
								ssithraWhichJump(self);
								self->monsterinfo.jump_time = level.time + 1;
							}
						}
					}
				}
			}
			else
			{//check to jump over something
				VectorCopy(self->s.origin, save_org);
				can_move = M_walkmove (self, self->s.angles[YAW], 64);
				VectorCopy(save_org, self->s.origin);
				
				if(can_move)
					return;

				AngleVectors(self->s.angles, vf, NULL, NULL);
				VectorCopy(self->s.origin, source);
				VectorMA(source, 128, vf, source2);
				VectorCopy(self->mins, mins);
				mins[2]+=24;//can clear it
				gi.trace(source, mins, self->maxs, source2, self, MASK_SOLID,&trace);
				
				if(trace.allsolid||trace.startsolid)
					return;
				
				if(trace.fraction<1 && trace.ent != self->enemy)
					return;

				//Go for it!
				ssithraJump(self, 128, 200*trace.fraction, 0);
				SetAnim(self, ANIM_BOUND);
			}

		}
	}
}

void ssithraForward (edict_t *self, float forwarddist)
{//simple addition of velocity, if on ground or not
vec3_t forward;

	ssithraCheckInWater(self);

	if(self->groundentity)//on ground
	{
		VectorClear(self->velocity);
		return;
	}

	AngleVectors(self->s.angles,forward,NULL,NULL);
	VectorScale(forward,forwarddist,forward);
	forward[2] = self->velocity[2];
	VectorCopy(forward,self->velocity);
}

void ssithraCheckLeaveWaterSplash (edict_t *self)
{
	vec3_t dir, endpos;
	trace_t trace;

	if(self->count)
		return;

	VectorCopy(self->s.origin,endpos);
	endpos[2]+=12;
	if(!ssithraCheckInWater(self))
	{
		VectorCopy(self->velocity,dir);
		VectorNormalize(dir);
		VectorMA(self->s.origin,-256,dir,endpos);
		gi.trace(self->s.origin,vec3_origin,vec3_origin,endpos,self,MASK_WATER,&trace);
		if(trace.fraction>=1.0)
			return;//?!
//		gi.dprintf("Out water splash\n");
		gi.sound(self,CHAN_BODY,Sounds[SND_NAMOR],1,ATTN_NORM,0);
		dir[0]=dir[1]=0;
		dir[2]=300;
		// FIXME: Size propn. to exit velocity.
		gi.CreateEffect(NULL, FX_WATER_ENTRYSPLASH, 0,
			trace.endpos,	"bd", 128|96, dir);
		self->count= true;
	}
}

void ssithraCheckHitWaterSplash (edict_t *self)
{
	vec3_t dir, endpos;
	trace_t trace;

	if(self->count)
		return;

	if(Q_fabs(self->velocity[0])+Q_fabs(self->velocity[1])<200)
	{
		VectorCopy(self->s.origin, endpos);
		endpos[2] -= 128;
		gi.trace(self->s.origin,self->mins,self->maxs,endpos,self,MASK_ALL,&trace);
		if(trace.fraction<1&&
			!trace.allsolid&&
			!trace.startsolid&&
			!(trace.contents & CONTENTS_WATER)&&
			!(trace.contents & CONTENTS_SLIME))
		{
			//not going to hit water!
			SetAnim(self, ANIM_BOUND);
			return;
		}
	}

	if(self->flags&FL_INWATER)
	{
		VectorCopy(self->velocity,dir);
		VectorNormalize(dir);
		VectorMA(self->s.origin,-256,dir,endpos);
		gi.trace(self->s.origin,vec3_origin,vec3_origin,endpos,self,MASK_WATER,&trace);
		gi.trace(trace.endpos,vec3_origin,vec3_origin,self->s.origin,self,MASK_WATER,&trace);
		if(trace.fraction>=1.0)
			return;//?!
//		gi.dprintf("In water splash\n");
		gi.sound(self,CHAN_BODY,Sounds[SND_INWATER],1,ATTN_NORM,0);
		gi.sound (self, CHAN_BODY, gi.soundindex("player/Water Enter.wav"), 1, ATTN_NORM, 0);
		VectorCopy(self->velocity,dir);
		dir[0]=dir[1]=0;
		VectorNormalize(dir);
		// FIXME: Size propn. to entry velocity.
		gi.CreateEffect(NULL, FX_WATER_ENTRYSPLASH, CEF_FLAG7,
			trace.endpos,	"bd", 128|96, dir);
		self->count= true;
	}
}

void ssithraCheckFacedNamor (edict_t *self)
{
	if(self->spawnflags & MSF_FIXED)
		return;

	if(Q_fabs(self->ideal_yaw - self->s.angles[YAW])<self->yaw_speed)
		SetAnim(self,ANIM_NAMOR);
}

//========================================
//PAINS
//========================================

void ssithraSlideFall (edict_t *self)
{
	if(self->mins[2]<0)
	{
//		if(Vec3IsZero(self->velocity))
//			self->groundentity = NULL;
		if(self->mins[2]<=-6)
			self->mins[2]+=6;
		else
			self->mins[2]=0;
//		gi.dprintf("ssithra top's mins: %f\n",self->mins[2]);
		self->think = ssithraSlideFall;
		self->nextthink = level.time + FRAMETIME;
	}
	else
	{
		self->friction = 1;
		self->owner->msgHandler=DefaultMsgHandler;
		self->owner->nextthink = level.time;
		SetAnim(self->owner,ANIM_SLICED);
		self->owner->msgHandler=DyingMsgHandler;
		self->think = NULL;
		self->nextthink = -1;
	}
}

void ssithraSlideOff (edict_t *self)
{
	vec3_t right;

	AngleVectors(self->s.angles,NULL,right,NULL);
	VectorScale(right,100,self->velocity);
	self->think = ssithraSlideFall;
	self->nextthink = level.time + FRAMETIME;
}

void ssithraSplit (edict_t *self, int BodyPart)
{//blood stripe
	vec3_t	p1, p2, dir, up, right;
	edict_t	*tophalf;
	int	whichnode, node_num;

	AngleVectors(self->s.angles, NULL, right, up);
	VectorClear(p1);
	VectorMA(p1,6,up,p1);
	VectorMA(p1,10,right,p1);
	VectorClear(p2);
	VectorMA(p2,-6,up,p2);
	VectorMA(p2,-10,right,p2);
	VectorSubtract(p2,p1,dir);
	VectorNormalize(dir);
	VectorScale(dir, 40.0, dir);

	//Why doesn't this work?
	gi.CreateEffect(&self->s, FX_BLOOD, 0, p1, "ub", dir, 20);
	VectorAdd(self->s.origin,p2,p2);
	SprayDebris(self,p2, 6,200);

//	gi.dprintf("sliding in half- making top\n");
	
	tophalf = G_Spawn();

	tophalf->nextthink = level.time + FRAMETIME*10;
	tophalf->think = ssithraSlideOff;
	tophalf->svflags |= SVF_MONSTER;
	tophalf->s.renderfx |= RF_FRAMELERP;
	tophalf->takedamage = DAMAGE_AIM;
	tophalf->max_health = tophalf->health = 25;
	tophalf->clipmask = MASK_MONSTERSOLID;
	tophalf->s.effects |= EF_CAMERA_NO_CLIP;

	tophalf->s.skinnum = self->s.skinnum;
	tophalf->deadflag = DEAD_DEAD;
	tophalf->deadState = DEAD_DEAD;
	tophalf->svflags |= SVF_DEADMONSTER;
	tophalf->monsterinfo.thinkinc = MONSTER_THINK_INC;//FRAMETIME;
	tophalf->monsterinfo.nextframeindex = -1;
	tophalf->friction = 0.1;

	VectorCopy (self->s.origin, tophalf->s.origin);
	VectorCopy (tophalf->s.origin, tophalf->s.old_origin);
	VectorCopy (self->s.angles, tophalf->s.angles);

	if(!self->s.frame)//?!?!
		tophalf->s.frame = FRAME_startle32;
	else
		tophalf->s.frame = self->s.frame;

	tophalf->materialtype = MAT_FLESH;
	tophalf->mass = self->mass = 300;

	tophalf->movetype = PHYSICSTYPE_STEP;

	tophalf->solid=SOLID_BBOX;
	tophalf->owner = self;

	VectorSet(tophalf->mins,-16,-16,self->mins[2]);
   	VectorSet(tophalf->maxs,16,16,16);
	VectorSet(self->maxs,16,16,0);
	tophalf->s.origin[2] += 10;

//Fixm: sometimes top half appears too low and forward?
	VectorClear(self->knockbackvel);
	VectorClear(self->velocity);

	tophalf->s.modelindex = self->s.modelindex;

	tophalf->s.scale = tophalf->s.scale;

	tophalf->monsterinfo.otherenemyname = "obj_barrel";

	for(whichnode = 1, node_num = 0; whichnode<=16384; whichnode*=2)//bitwise
	{
		node_num++;
		if(!((int)(BodyPart)&(int)(whichnode)))
		{//turn off this node on top
			tophalf->s.fmnodeinfo[node_num].flags |= FMNI_NO_DRAW;
		}
		else
		{//turn on this node on top and keep them
			tophalf->s.fmnodeinfo[node_num] = self->s.fmnodeinfo[node_num];//copy skins and flags and colors
			tophalf->s.fmnodeinfo[node_num].flags &= ~FMNI_NO_DRAW;
			self->s.fmnodeinfo[node_num].flags |= FMNI_NO_DRAW;
		}
	}
	tophalf->s.fmnodeinfo[MESH__CAPBOTTOMUPPERTORSO].flags &= ~FMNI_NO_DRAW;
	self->s.fmnodeinfo[MESH__CAPLOWERTORSO].flags &= ~FMNI_NO_DRAW;
	self->s.fmnodeinfo[MESH__RIGHT2SPIKE].flags |= FMNI_NO_DRAW;

	self->nextthink = 9999999999999999;
}

/*
-1	hl_Null				= -1,
0	hl_NoneSpecific		= 0,
1	hl_Head,
2	hl_TorsoFront,
3	hl_TorsoBack,
4	hl_ArmUpperLeft,
5	hl_ArmLowerLeft,
6	hl_ArmUpperRight,
7	hl_ArmLowerRight,
8	hl_LegUpperLeft,
9	hl_LegLowerLeft,
10	hl_LegUpperRight,
11	hl_LegLowerRight,
12	hl_Max,
*/
qboolean canthrownode (edict_t *self, int BP, int *throw_nodes)
{//see if it's on, if so, add it to throw_nodes
	//turn it off on thrower
	if(!(self->s.fmnodeinfo[BP].flags & FMNI_NO_DRAW))
	{
		*throw_nodes |= Bit_for_MeshNode[BP];
		self->s.fmnodeinfo[BP].flags |= FMNI_NO_DRAW;
		return true;
	}
	return false;
}

int ssithra_convert_hitloc_dead(edict_t *self, int hl)
{
	qboolean	fellback = false;

	if(self->curAnimID == ANIM_DEATH_A)
		fellback = true;

	switch(hl)
	{
		case hl_Head:
			if(fellback)
				return hl_TorsoFront;
			else
				return hl_TorsoBack;
			break;
		
		case hl_TorsoFront://split in half?
			if(fellback)
			{
				if(!irand(0,1))
					return hl_LegUpperRight;
				else
					return hl_LegUpperLeft;
			}
			else
				return hl_Head;
			break;
		
		case hl_TorsoBack://split in half?
			if(fellback)
				return hl_Head;
			else
			{
				if(!irand(0,1))
					return hl_LegUpperRight;
				else
					return hl_LegUpperLeft;
			}
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

void ssithra_dismember(edict_t *self, int damage, int HitLocation)
{//fixme - make part fly dir the vector from hit loc to sever loc
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

	if(self->health>0)
	{
		switch (self->curAnimID)
		{//Hit front chest during shoot or melee, may have hit arms
			case ANIM_DUCKSHOOT:
			case ANIM_SHOOT:
			case ANIM_WATER_SHOOT:
			case ANIM_HEADLESS:
			case ANIM_HEADLESSLOOP:
				if(HitLocation == hl_TorsoFront&&irand(0,10)<4)
					HitLocation = hl_ArmLowerRight;
				break;

			case ANIM_MELEE:
			case ANIM_MELEE_STAND:
				if(HitLocation == hl_TorsoFront&&irand(0,10)<4)
					HitLocation = hl_ArmLowerLeft;
				break;

			default:
				break;
		}

		if((HitLocation == hl_ArmUpperLeft&& self->s.fmnodeinfo[MESH__LEFTARM].flags & FMNI_NO_DRAW) ||
			(HitLocation == hl_ArmUpperRight&& self->s.fmnodeinfo[MESH__RIGHTARM].flags & FMNI_NO_DRAW)||
			((HitLocation == hl_TorsoFront|| HitLocation == hl_TorsoBack) &&
			self->s.fmnodeinfo[MESH__RIGHTARM].flags & FMNI_NO_DRAW &&
			self->s.fmnodeinfo[MESH__LEFTARM].flags & FMNI_NO_DRAW)&&
			irand(0,10)<4)
			HitLocation = hl_Head;//Decap
	}
	else
		HitLocation = ssithra_convert_hitloc_dead(self, HitLocation);

	VectorClear(gore_spot);
	switch(HitLocation)
	{
		case hl_Head:
			if(self->s.fmnodeinfo[MESH__HEAD].flags & FMNI_NO_DRAW)
				break;
			// Is the pain skin engaged?
			if(self->s.fmnodeinfo[MESH__HEAD].flags & FMNI_USE_SKIN)
				damage*=1.5;//greater chance to cut off if previously damaged
			if(flrand(0,self->health)<damage*0.3&&dismember_ok)
			{
				canthrownode(self, MESH__HEAD,&throw_nodes);
				canthrownode(self, MESH__CENTERSPIKE,&throw_nodes);
				canthrownode(self, MESH__LEFT1SPIKE,&throw_nodes);
				canthrownode(self, MESH__RIGHT1SPIKE,&throw_nodes);
				canthrownode(self, MESH__RIGHT2SPIKE,&throw_nodes);

				self->s.fmnodeinfo[MESH__CAPTOPUPPERTORSO].flags &= ~FMNI_NO_DRAW;

				gore_spot[2]+=18;
				ThrowBodyPart(self, &gore_spot, throw_nodes, damage, 0);

				VectorAdd(self->s.origin, gore_spot, gore_spot);
				SprayDebris(self,gore_spot,8,damage);

				if(self->health > 0 && irand(0,10)<3&&!(self->s.fmnodeinfo[MESH__RIGHTARM].flags & FMNI_NO_DRAW))
				{//shooting blind, headless, FIX: make it so can still chop off arms or legs here
					SetAnim(self,ANIM_HEADLESS);
					self->msgHandler=DyingMsgHandler;
				}
				else
				{
					self->health = 1;
					T_Damage (self, self, self, vec3_origin, gore_spot, vec3_origin, 10, 20,0,MOD_DIED);
				}
				return;
			}
			else
			{
				// Set the pain skin
				self->s.fmnodeinfo[MESH__HEAD].flags |= FMNI_USE_SKIN;
				self->s.fmnodeinfo[MESH__HEAD].skin = self->s.skinnum+1;

				if(flrand(0,self->health/4)<damage)
				{//no red spray with these, particles?
					gore_spot[2]+=18;
					if(irand(0,10)<3)
					{
						if(canthrownode(self, MESH__CENTERSPIKE, &throw_nodes))
							ThrowBodyPart(self, &gore_spot, throw_nodes, damage, 0);
					}
					if(irand(0,10)<3)
					{
						if(canthrownode(self, MESH__RIGHT1SPIKE, &throw_nodes))
							ThrowBodyPart(self, &gore_spot, throw_nodes, damage, 0);
					}
					if(irand(0,10)<3)
					{
						if(canthrownode(self, MESH__RIGHT2SPIKE, &throw_nodes))
							ThrowBodyPart(self, &gore_spot, throw_nodes, damage, 0);
					}
					if(irand(0,10)<3)
					{
						if(canthrownode(self, MESH__LEFT1SPIKE, &throw_nodes))
							ThrowBodyPart(self, &gore_spot, throw_nodes, damage, 0);
					}
				}
			}
			break;
		case hl_TorsoFront://split in half?
		case hl_TorsoBack://split in half?
			if(self->s.fmnodeinfo[MESH__UPPERTORSO].flags & FMNI_NO_DRAW)
				break;
			if(self->s.fmnodeinfo[MESH__UPPERTORSO].flags & FMNI_USE_SKIN)
				damage*=1.5;//greater chance to cut off if previously damaged
			if(flrand(0,self->health)<damage*0.3&&dismember_ok)
			{
				gore_spot[2]+=12;
				//seal up the caps left by this split
				self->s.fmnodeinfo[MESH__CAPBOTTOMUPPERTORSO].flags &= ~FMNI_NO_DRAW;
				self->s.fmnodeinfo[MESH__CAPLOWERTORSO].flags &= ~FMNI_NO_DRAW;

				canthrownode(self, MESH__UPPERTORSO,&throw_nodes);
				canthrownode(self, MESH__CAPBOTTOMUPPERTORSO,&throw_nodes);
				canthrownode(self, MESH__CAPTOPUPPERTORSO,&throw_nodes);
				canthrownode(self, MESH__LEFTARM,&throw_nodes);
				canthrownode(self, MESH__RIGHTARM,&throw_nodes);
				canthrownode(self, MESH__HEAD,&throw_nodes);
				canthrownode(self, MESH__CENTERSPIKE,&throw_nodes);
				canthrownode(self, MESH__LEFT1SPIKE,&throw_nodes);
				canthrownode(self, MESH__RIGHT1SPIKE,&throw_nodes);
				canthrownode(self, MESH__RIGHT2SPIKE,&throw_nodes);

				if(self->health > 0 && irand(0,10)<3)//Slide off
					ssithraSplit(self, throw_nodes);
				else
				{
					ThrowBodyPart(self, &gore_spot, throw_nodes, damage, FRAME_partrest1);

					VectorAdd(self->s.origin, gore_spot, gore_spot);
					SprayDebris(self,gore_spot,12,damage);
					SetAnim(self,ANIM_SLICED);
				}
				self->msgHandler=DyingMsgHandler;
			}
			else
			{
				// Set the pain skin
				self->s.fmnodeinfo[MESH__UPPERTORSO].flags |= FMNI_USE_SKIN;
				self->s.fmnodeinfo[MESH__UPPERTORSO].skin = self->s.skinnum+1;
			}
			break;
		case hl_ArmUpperLeft:
		case hl_ArmLowerLeft://left arm
			if(self->s.fmnodeinfo[MESH__LEFTARM].flags & FMNI_NO_DRAW)
				break;
			if(self->s.fmnodeinfo[MESH__LEFTARM].flags & FMNI_USE_SKIN)
				damage*=1.5;//greater chance to cut off if previously damaged
			if(flrand(0,self->health)<damage*0.75&&dismember_ok)
			{
				if(canthrownode(self, MESH__LEFTARM, &throw_nodes))
				{
					AngleVectors(self->s.angles,NULL,right,NULL);
					gore_spot[2]+=self->maxs[2]*0.3;
					VectorMA(gore_spot,-10,right,gore_spot);
					ThrowBodyPart(self, &gore_spot, throw_nodes, damage, 0);
				}
			}
			else
			{
				// Set the pain skin
				self->s.fmnodeinfo[MESH__LEFTARM].flags |= FMNI_USE_SKIN;
				self->s.fmnodeinfo[MESH__LEFTARM].skin = self->s.skinnum+1;
			}
			break;
		case hl_ArmUpperRight:
		case hl_ArmLowerRight://right arm
			if(self->s.fmnodeinfo[MESH__RIGHTARM].flags & FMNI_NO_DRAW)
				break;
			if(self->s.fmnodeinfo[MESH__RIGHTARM].flags & FMNI_USE_SKIN)
				damage*=1.5;//greater chance to cut off if previously damaged
			if(flrand(0,self->health)<damage*0.75&&dismember_ok)
			{
				if(canthrownode(self, MESH__RIGHTARM, &throw_nodes))
				{
					AngleVectors(self->s.angles,NULL,right,NULL);
					gore_spot[2]+=self->maxs[2]*0.3;
					VectorMA(gore_spot,10,right,gore_spot);
					ThrowBodyPart(self, &gore_spot, throw_nodes, damage, 0);
				}
			}
			else
			{
				// Set the pain skin
				self->s.fmnodeinfo[MESH__RIGHTARM].flags |= FMNI_USE_SKIN;
				self->s.fmnodeinfo[MESH__RIGHTARM].skin = self->s.skinnum+1;
			}
			break;
		
		case hl_LegUpperLeft:
		case hl_LegLowerLeft://left leg
			if(self->health>0)
			{//still alive
				if(self->s.fmnodeinfo[MESH__LEFTLEG].flags & FMNI_USE_SKIN)
					break;
				self->s.fmnodeinfo[MESH__LEFTLEG].flags |= FMNI_USE_SKIN;			
				self->s.fmnodeinfo[MESH__LEFTLEG].skin = self->s.skinnum+1;
				break;
			}
			else
			{
				if(self->s.fmnodeinfo[MESH__LEFTLEG].flags & FMNI_NO_DRAW)
					break;
				if(canthrownode(self, MESH__LEFTLEG, &throw_nodes))
				{
					AngleVectors(self->s.angles,NULL,right,NULL);
					gore_spot[2]+=self->maxs[2]*0.3;
					VectorMA(gore_spot,-10,right,gore_spot);
					ThrowBodyPart(self, &gore_spot, throw_nodes, damage, 0);
				}
				break;
			}
		case hl_LegUpperRight:
		case hl_LegLowerRight://right leg
			if(self->health>0)
			{//still alive
				if(self->s.fmnodeinfo[MESH__RIGHTLEG].flags & FMNI_USE_SKIN)
					break;
				self->s.fmnodeinfo[MESH__RIGHTLEG].flags |= FMNI_USE_SKIN;
				self->s.fmnodeinfo[MESH__RIGHTLEG].skin = self->s.skinnum+1;
				break;
			}
			else
			{
				if(self->s.fmnodeinfo[MESH__RIGHTLEG].flags & FMNI_NO_DRAW)
					break;
				if(canthrownode(self, MESH__RIGHTLEG, &throw_nodes))
				{
					AngleVectors(self->s.angles,NULL,right,NULL);
					gore_spot[2]+=self->maxs[2]*0.3;
					VectorMA(gore_spot,-10,right,gore_spot);
					ThrowBodyPart(self, &gore_spot, throw_nodes, damage, 0);
				}
				break;
			}
		default:
			break;
	}

	if(throw_nodes)
		self->pain_debounce_time = 0;

	if(self->s.fmnodeinfo[MESH__LEFTARM].flags&FMNI_NO_DRAW&&
		self->s.fmnodeinfo[MESH__RIGHTARM].flags&FMNI_NO_DRAW)			
	{
		self->monsterinfo.aiflags |= AI_COWARD;
		self->ai_mood_flags &= ~AI_MOOD_FLAG_BACKSTAB;
		self->spawnflags &= ~MSF_FIXED;
	}
	else
	{
		if(self->s.fmnodeinfo[MESH__LEFTARM].flags&FMNI_NO_DRAW)
		{
			self->monsterinfo.aiflags |= AI_NO_MELEE;
			self->ai_mood_flags &= ~AI_MOOD_FLAG_BACKSTAB;
		}
		if(self->s.fmnodeinfo[MESH__RIGHTARM].flags&FMNI_NO_DRAW)
		{
			self->monsterinfo.aiflags |= AI_NO_MISSILE;
			self->ai_mood_flags &= ~AI_MOOD_FLAG_BACKSTAB;
			self->spawnflags &= ~MSF_FIXED;
		}
	}
}

void ssithra_dead_pain (edict_t *self, G_Message_t *msg)
{
	if(msg)
		if(!(self->svflags & SVF_PARTS_GIBBED))
			MG_parse_dismember_msg(self, msg);
}

void ssithra_pain(edict_t *self, G_Message_t *msg)
{//fixme - make part fly dir the vector from hit loc to sever loc
	int inwater;
	int				temp, damage;
	qboolean		force_pain;
	

	if(self->deadflag == DEAD_DEAD) //Dead but still being hit	
		return;

	ParseMsgParms(msg, "eeiii", &temp, &temp, &force_pain, &damage, &temp);

	if(!force_pain)
	{
		if(self->pain_debounce_time)
			if(irand(0,10)<5||!self->groundentity)
				return;

		if(self->pain_debounce_time > level.time)
			return;
	}

	ssithraUnCrouch(self);

	self->pain_debounce_time = level.time + 2;

	if(irand(0,10)<5)
		gi.sound (self, CHAN_VOICE, Sounds[SND_PAIN1], 1, ATTN_NORM, 0);
	else
		gi.sound (self, CHAN_VOICE, Sounds[SND_PAIN2], 1, ATTN_NORM, 0);


	inwater = ssithraCheckInWater(self);

	if(inwater)
	{//underwater pain sound?
		if(self->curAnimID!=ANIM_SWIMFORWARD)
			SetAnim(self, ANIM_WATER_PAIN_A);
		else//swimming
			SetAnim(self, ANIM_WATER_PAIN_B);
	}
	else 
	{
		SetAnim(self, ANIM_PAIN_A);
	}
}

void ssithra_pain_react (edict_t *self)
{
	if(!self->enemy)
	{
#ifdef _DEVEL
		if(MGAI_DEBUG)
			gi.dprintf("No react to pain\n");
#endif
		ssithra_decide_stand(self);
		return;
	}

	if(self->enemy->health<=0||self->enemy == self||!self->enemy->takedamage)
	{
#ifdef _DEVEL
		if(MGAI_DEBUG)
			gi.dprintf("No react to pain\n");
#endif
		self->enemy=NULL;
		ssithra_decide_stand(self);
		return;
	}
	//go get him!
#ifdef _DEVEL
	if(MGAI_DEBUG)
		gi.dprintf("pain_react -> run\n");
#endif
	ssithra_decide_gallop(self);
}

//===========================================
//DEATHS
//===========================================

void ssithra_death(edict_t *self, G_Message_t *msg)
{//FIXME: still cut off limbs as dying?
	int inwater;

	if(self->monsterinfo.aiflags&AI_DONT_THINK)
	{
		gi.sound(self,CHAN_BODY,Sounds[SND_DIE],1,ATTN_NORM,0);
		if (irand(0,10) < 5)
			SetAnim(self, ANIM_DEATH_B);
		else
			SetAnim(self, ANIM_DEATH_A);
		return;
	}
	self->msgHandler=DyingMsgHandler;

	if(self->deadflag == DEAD_DEAD) //Dead but still being hit	
	{
//		gi.dprintf("already dead!\n");
		return;
	}
	
	self->deadflag = DEAD_DEAD;

	if(self->health <= -80) //gib death
	{
		int	i, num_limbs;

		num_limbs = irand(1, 3);
		for(i = 0; i < num_limbs; i++)
			ssithra_dismember(self, flrand(80, 160), irand(hl_Head, hl_LegLowerRight) | hl_MeleeHit);

		gi.sound(self,CHAN_BODY,Sounds[SND_GIB],1,ATTN_NORM,0);
		self->think = BecomeDebris;
		self->nextthink = level.time + 0.1;
		return;
	}

	ssithraUnCrouch(self);
	inwater = ssithraCheckInWater(self);

	if(inwater)
	{
		SetAnim(self, ANIM_WATER_DEATH);
	}
	else
	{
		if (self->health == -69)
		{//maybe allow dead bodies to be chopped?  Make BBOX small?
			self->deadState = DEAD_DEAD;

			gi.linkentity(self);

			self->flags |= FL_DONTANIMATE;

			self->msgHandler = DeadMsgHandler;

			self->svflags |= SVF_DEADMONSTER;	// now treat as a different content type

			SetAnim(self, ANIM_DEAD_B);
		}
		else if (self->health == -33)
			SetAnim(self, ANIM_DEATH_C);
		else if (irand(0,10) < 4 || self->health > -10)//barely dead
			SetAnim(self, ANIM_DEATH_B);
		else
			SetAnim(self, ANIM_DEATH_A);
	}
}

void ssithra_dead(edict_t *self)
{//maybe allow dead bodies to be chopped?  Make BBOX small?
	self->msgHandler = DeadMsgHandler;
	self->svflags |= SVF_DEADMONSTER;	// now treat as a different content type
	self->deadState = DEAD_DEAD;

	self->flags |= FL_DONTANIMATE;

	M_EndDeath(self);
}

float hold_z;
void fish_deadfloat(edict_t *self);
void ssithraWaterDead(edict_t *self)
{
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;
			
	self->think = fish_deadfloat;
	self->nextthink = level.time + 0.1;

	gi.linkentity (self);
}

void ssithraCollapse (edict_t *self)
{
	vec3_t gore_spot;

	if(irand(0,10)<5)
	{
		self->msgHandler = DefaultMsgHandler;
		SetAnim(self,ANIM_HEADLESSLOOP);
		self->msgHandler = DyingMsgHandler;
		return;
	}
	else
	{
		self->svflags &= ~SVF_DEADMONSTER;	// now treat as a different content type
		self->msgHandler = DefaultMsgHandler;
		VectorCopy(self->s.origin,gore_spot);
		gore_spot[2]+=self->maxs[2]*0.75;
		self->health = 1;
		T_Damage (self, self, self, vec3_origin, gore_spot, vec3_origin, 10, 20,0,MOD_DIED);
		self->health = -33;
	}
}

void ssithraKillSelf (edict_t *self)
{
	vec3_t gore_spot;

	self->svflags &= ~SVF_DEADMONSTER;	// now treat as a different content type
	self->msgHandler = DefaultMsgHandler;
	self->deadflag = false;
	VectorCopy(self->s.origin,gore_spot);
	gore_spot[2]+=12;
	self->health = 1;
	T_Damage (self, self, self, vec3_origin, gore_spot, vec3_origin, 10, 20,0,MOD_DIED);
	self->health = -69;
}

//===========================================
//SOUNDS
//===========================================

void ssithraSound(edict_t *self, float soundnum, float channel, float attenuation)
{
	if(!channel)
		channel = CHAN_AUTO;

	if(!attenuation)
		attenuation = ATTN_NORM;
	else if(attenuation == -1)
		attenuation = ATTN_NONE;

	if(soundnum == SND_SWIM)
		if(irand(0,10)<5)
			soundnum = SND_SWIM2;

	gi.sound(self,channel,Sounds[(int)(soundnum)],1,attenuation,0);
}

void ssithraGrowlSound(edict_t *self)
{
	if(!irand(0, 3))
		gi.sound(self,CHAN_VOICE,Sounds[irand(SND_GROWL1, SND_GROWL3)],1,ATTN_IDLE,0);
}
//===========================================
//ATTACKS
 //===========================================

void ssithra_melee(edict_t *self, G_Message_t *msg)
{
	if (M_ValidTarget(self, self->enemy))
	{
		if(ssithraCheckInWater(self))
		{
			QPostMessage(self, MSG_MISSILE, PRI_DIRECTIVE, NULL);
			return;
		}

		if(!(self->monsterinfo.aiflags & AI_NO_MISSILE) && !(self->spawnflags&MSF_FIXED))
		{
			if(vhlen(self->enemy->s.origin, self->s.origin) - 16 < flrand(0, self->melee_range))
			{
				SetAnim(self, ANIM_BACKPEDAL);
				return;
			}
		}

		if(M_DistanceToTarget(self, self->enemy) > self->melee_range*2 &&!(self->spawnflags&MSF_FIXED))
			SetAnim(self, ANIM_MELEE);
		else
			SetAnim(self, ANIM_MELEE_STAND);
	}
	else
		QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
}

void ssithra_missile(edict_t *self, G_Message_t *msg)
{
	int	chance;

	if (M_ValidTarget(self, self->enemy))
	{
		if (ssithraCheckInWater(self))
		{
			if(M_DistanceToTarget(self, self->enemy) < self->melee_range)
			{
				if(self->curAnimID == ANIM_SWIMFORWARD)
					SetAnim(self, ANIM_TRANSUP);
				else
					SetAnim(self, ANIM_WATER_SHOOT);
			}
			else
				ssithraArrow(self);
		}
		else
		{
			if(self->spawnflags & MSF_SSITHRA_CLOTHED)
				chance = 20;
			else
				chance = 80;

			if (irand(0, skill->value * 100) > chance)
				SetAnim(self, ANIM_DUCKSHOOT);
			else
				SetAnim(self, ANIM_SHOOT);
		}
	}
	else
	{
		QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
	}
}

void ssithra_backup(edict_t *self, G_Message_t *msg)
{
	if (M_ValidTarget(self, self->enemy))
	{
		if(self->spawnflags&MSF_FIXED)
		{
			SetAnim(self, ANIM_DELAY);
			return;
		}
		
		if (ssithraCheckInWater(self))
		{
			SetAnim(self, ANIM_WATER_SHOOT);
		}
		else
		{
			SetAnim(self, ANIM_BACKPEDAL);
		}
	}
	else
	{
		QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
	}
}

void ssithraSwipe (edict_t *self)
{//use melee swipe functoin in g_monster
	trace_t	trace;
	edict_t *victim;
	vec3_t	soff, eoff, mins, maxs, bloodDir, direction;

	VectorSet(soff, 16, -16, 24);
	VectorSet(eoff, 50, 0, -8);
	
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
		}
		else
		{
			//Hurt whatever we were whacking away at
			gi.sound (self, CHAN_WEAPON, Sounds[SND_SWIPEHIT], 1, ATTN_NORM, 0);
			if(self->spawnflags&MSF_SSITHRA_ALPHA)
				T_Damage(victim, self, self, direction, trace.endpos, bloodDir, irand(SSITHRA_DMG_MIN*1.2, SSITHRA_DMG_MAX*1.2), 10, 0,MOD_DIED);
			else
				T_Damage(victim, self, self, direction, trace.endpos, bloodDir, irand(SSITHRA_DMG_MIN, SSITHRA_DMG_MAX), 0, 0,MOD_DIED);
		}
	}
	else
	{
		//Play swoosh sound?
	}
}

// the arrow needs to bounce
void make_arrow_reflect(edict_t *self, edict_t *Arrow)
{
	create_ssith_arrow(Arrow);
	Arrow->s.modelindex = self->s.modelindex;
	VectorCopy(self->s.origin, Arrow->s.origin);
	Arrow->owner = self->owner;
	Arrow->enemy = self->enemy;

	Arrow->touch=self->touch;
	Arrow->nextthink=self->nextthink;
	Arrow->think=G_FreeEdict;
	Arrow->health = self->health;

	Create_rand_relect_vect(self->velocity, Arrow->velocity);

	vectoangles(Arrow->velocity, Arrow->s.angles);
	Arrow->s.angles[YAW]+=90;

	Vec3ScaleAssign(SSITHRA_SPOO_SPEED/2,Arrow->velocity);

	G_LinkMissile(Arrow);
}

void ssithraAlphaArrowTouch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surface)
{
	float damage;
	vec3_t	normal;
	edict_t	*Arrow;
	
	// are we reflecting ?
	if(EntReflecting(other, true, true))
	{
		Arrow = G_Spawn();

		make_arrow_reflect(self,Arrow);

		gi.CreateEffect(&Arrow->s,
			FX_SSITHRA_ARROW,
			CEF_OWNERS_ORIGIN,
			NULL,
			"bv",
			FX_SS_MAKE_ARROW2,
			Arrow->velocity);

		G_SetToFree(self);

		return;
	}


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
		damage = flrand(SSITHRA_DMG_MIN,SSITHRA_DMG_MAX);
		T_Damage(other,self,self->owner,self->movedir,self->s.origin,normal,damage,0,0,MOD_DIED);
	}
	else
		damage = 0;

	T_DamageRadius(self, self->owner, self->owner, SSITHRA_DMG_ARROW_RADIUS, 
			(20 - damage*2), (30 - damage), DAMAGE_ATTACKER_IMMUNE,MOD_DIED);

	VectorNormalize(self->velocity);

	gi.CreateEffect(NULL,
		FX_SSITHRA_ARROW,
		0,
		self->s.origin, 
		"bv",
		FX_SS_EXPLODE_ARROW2,
		self->velocity);

	VectorClear(self->velocity);

	G_FreeEdict(self);
}

void ssithraArrowTouch (edict_t *self,edict_t *Other,cplane_t *Plane,csurface_t *Surface)
{
	float damage;
	vec3_t	normal;
	edict_t	*Arrow;

	if(Surface&&(Surface->flags&SURF_SKY))
	{
		SkyFly(self);
		return;
	}

	if(EntReflecting(Other, true, true))
	{
		Arrow = G_Spawn();

		make_arrow_reflect(self,Arrow);

		gi.CreateEffect( NULL,
					 FX_M_EFFECTS,
					 CEF_FLAG6,
					 self->s.origin,
					 "bv",
					 FX_MSSITHRA_EXPLODE,
					 self->movedir);
		/*
		gi.CreateEffect(&Arrow->s,
			FX_SSITHRA_ARROW,
			CEF_OWNERS_ORIGIN,
			NULL,
			"bv",
			FX_SS_MAKE_ARROW,
			Arrow->velocity);
		*/

		G_SetToFree(self);

		return;
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
		damage = flrand(SSITHRA_DMG_MIN,SSITHRA_DMG_MAX);
		T_Damage(Other,self,self->owner,self->movedir,self->s.origin,normal,damage,0,0,MOD_DIED);
	}

	VectorNormalize(self->velocity);

	gi.CreateEffect( NULL,
				 FX_M_EFFECTS,
				 CEF_FLAG6,
				 self->s.origin,
				 "bv",
				 FX_MSSITHRA_EXPLODE,
				 self->movedir);

	/*
	gi.CreateEffect(NULL,
		FX_SSITHRA_ARROW,
		0,
		self->s.origin, 
		"bv",
		FX_SS_EXPLODE_ARROW,
		self->velocity);
	*/

	VectorClear(self->velocity);

	G_FreeEdict(self);
}

void ssithraArrowExplode(edict_t *self)
{
	int damage = irand(SSITHRA_BIGARROW_DMG_MIN, SSITHRA_BIGARROW_DMG_MAX);

	//TODO: Spawn an explosion effect
	gi.CreateEffect( NULL,
					 FX_M_EFFECTS,
					 0,
					 self->s.origin,
					 "bv",
					 FX_MSSITHRA_EXPLODE,
					 self->movedir);

	T_DamageRadius(self, self->owner, self->owner, 64, damage, damage/2, DAMAGE_ATTACKER_IMMUNE, MOD_DIED);

	G_FreeEdict(self);
}

void ssithraDuckArrowTouch (edict_t *self,edict_t *other,cplane_t *plane,csurface_t *surface)
{
	if(surface&&(surface->flags&SURF_SKY))
	{
		SkyFly(self);
		return;
	}

	//NOTENOTE: NO REFLECTION FOR THIS MISSILE!
	
	if(other->takedamage)
	{
		if (plane->normal)
			VectorCopy(plane->normal, self->movedir);

		self->dmg = irand(SSITHRA_DMG_MIN*2, SSITHRA_DMG_MAX*2);
		ssithraArrowExplode(self);
	}
	else
	{
		VectorClear(self->velocity);
		
		self->s.effects |= EF_MARCUS_FLAG1;

		if (plane->normal)
			VectorCopy(plane->normal, self->movedir);

		self->dmg = irand(SSITHRA_DMG_MIN, SSITHRA_DMG_MAX);

		self->think = ssithraArrowExplode;
		self->nextthink = level.time + flrand(0.5, 1.5);
	}
}

void create_ssith_arrow(edict_t *Arrow)
{
	Arrow->movetype = MOVETYPE_FLYMISSILE;
	Arrow->solid = SOLID_BBOX;
	Arrow->classname = "Ssithra_Arrow";
	Arrow->touch = ssithraArrowTouch;
	Arrow->enemy = NULL;
	Arrow->clipmask = MASK_SHOT;
	Arrow->s.scale = 0.75;
	Arrow->s.effects |= EF_CAMERA_NO_CLIP;
	Arrow->svflags |= SVF_ALWAYS_SEND;
	Arrow->s.modelindex = gi.modelindex("models/objects/exarrow/tris.fm");
}

void ssithraDoArrow(edict_t *self, float z_offs)
{
	vec3_t	Forward,check_lead, right, enemy_dir;
	edict_t	*Arrow;

	if(self->s.fmnodeinfo[MESH__RIGHTARM].flags&FMNI_NO_DRAW)
		return;

	gi.sound(self,CHAN_WEAPON,Sounds[SND_ARROW1],1,ATTN_NORM,0);
	self->monsterinfo.attack_finished = level.time + 0.4;
	Arrow = G_Spawn();

	create_ssith_arrow(Arrow);

	if(self->spawnflags & MSF_SSITHRA_ALPHA)
		Arrow->touch=ssithraArrowTouch;
	else
		Arrow->touch=ssithraArrowTouch;

	Arrow->owner=self;
	Arrow->enemy=self->enemy;
	
	Arrow->health = 0; // tell the touch function what kind of arrow we are;

	AngleVectors(self->s.angles, Forward, right, NULL);

	VectorCopy(self->s.origin,Arrow->s.origin);	
	VectorMA(Arrow->s.origin, 16, Forward, Arrow->s.origin);
	VectorMA(Arrow->s.origin, -4, right, Arrow->s.origin);
	Arrow->s.origin[2] += 16;

	VectorCopy(self->movedir,Arrow->movedir);
	vectoangles(Forward,Arrow->s.angles);

	VectorClear(check_lead);
	if(skill->value > 1)
	{
		extrapolateFiredir(self, Arrow->s.origin,
			SSITHRA_SPOO_SPEED, self->enemy, 0.3, check_lead);
	}
	else
	{
		VectorSubtract(self->enemy->s.origin, Arrow->s.origin, enemy_dir);
		VectorNormalize(enemy_dir);
		if(DotProduct(enemy_dir, Forward) >= 0.3)
		{
			Forward[2] = enemy_dir[2];
		}
	}

	if(Vec3IsZero(check_lead))
	{
		VectorScale(Forward,SSITHRA_SPOO_SPEED,Arrow->velocity);
	}
	else
	{
		VectorScale(check_lead,SSITHRA_SPOO_SPEED,Arrow->velocity);
	}

	VectorCopy(Arrow->velocity, Arrow->movedir);
	VectorNormalize(Arrow->movedir);
	vectoangles(Arrow->movedir, Arrow->s.angles);
	Arrow->s.angles[PITCH] = anglemod(Arrow->s.angles[PITCH] * -1);
	Arrow->s.angles[YAW] += 90;

	gi.CreateEffect(&Arrow->s,
		FX_M_EFFECTS,
		CEF_OWNERS_ORIGIN,
		Arrow->s.origin,
		"bv",
		FX_MSSITHRA_ARROW,
		Arrow->velocity);
	
	G_LinkMissile(Arrow); 

	Arrow->nextthink=level.time+3;
	Arrow->think=G_FreeEdict;//ssithraArrowThink;
}

void ssithraDoDuckArrow(edict_t *self, float z_offs)
{
	vec3_t	Forward,check_lead, right, enemy_dir;
	edict_t	*Arrow;

	if(self->s.fmnodeinfo[MESH__RIGHTARM].flags&FMNI_NO_DRAW)
		return;

	gi.sound(self, CHAN_WEAPON, Sounds[SND_ARROW_FIRE] , 1, ATTN_NORM, 0);

	self->monsterinfo.attack_finished = level.time + 0.4;
	Arrow = G_Spawn();

	create_ssith_arrow(Arrow);

	Arrow->touch=ssithraDuckArrowTouch;

	Arrow->owner=self;
	Arrow->enemy=self->enemy;
	
	Arrow->health = 0; // tell the touch function what kind of arrow we are;

	AngleVectors(self->s.angles, Forward, right, NULL);

	VectorCopy(self->s.origin,Arrow->s.origin);	
	VectorMA(Arrow->s.origin, 12*self->s.scale, Forward, Arrow->s.origin);
	VectorMA(Arrow->s.origin, 4*self->s.scale, right, Arrow->s.origin);
	Arrow->s.origin[2] += z_offs;

	Arrow->s.scale = 1.5;

	VectorCopy(self->movedir,Arrow->movedir);
	vectoangles(Forward,Arrow->s.angles);

	VectorClear(check_lead);
	if(skill->value > 1)
	{
		extrapolateFiredir(self, Arrow->s.origin,
			SSITHRA_SPOO_SPEED, self->enemy, 0.3, check_lead);
	}
	else
	{
		VectorSubtract(self->enemy->s.origin, Arrow->s.origin, enemy_dir);
		VectorNormalize(enemy_dir);
		if(DotProduct(enemy_dir, Forward) >= 0.3)
		{
			Forward[2] = enemy_dir[2];
		}
	}

	if(Vec3IsZero(check_lead))
	{
		VectorScale(Forward,SSITHRA_SPOO_SPEED*1.5,Arrow->velocity);
	}
	else
	{
		VectorScale(check_lead,SSITHRA_SPOO_SPEED*1.5,Arrow->velocity);
	}

	VectorCopy(Arrow->velocity, Arrow->movedir);
	VectorNormalize(Arrow->movedir);
	vectoangles(Arrow->movedir, Arrow->s.angles);
	Arrow->s.angles[PITCH] = anglemod(Arrow->s.angles[PITCH] * -1);
	Arrow->s.angles[YAW] += 90;

	gi.CreateEffect(&Arrow->s,
					FX_M_EFFECTS,
					CEF_OWNERS_ORIGIN | CEF_FLAG6,
					Arrow->s.origin,
					"bv",
					FX_MSSITHRA_ARROW,
					Arrow->velocity);
	
	G_LinkMissile(Arrow); 

	Arrow->nextthink=level.time+5;
	Arrow->think=ssithraArrowExplode;
}

void ssithraStartDuckArrow(edict_t *self)
{
	vec3_t	startpos, vf, vr;

	AngleVectors(self->s.angles, vf, vr, NULL);
	VectorMA(self->s.origin, 18*self->s.scale, vf, startpos);
	VectorMA(startpos, 4*self->s.scale, vr, startpos);

	gi.sound(self, CHAN_WEAPON, Sounds[SND_ARROW_CHARGE] , 1, ATTN_NORM, 0);
	
	gi.CreateEffect(NULL,
					FX_M_EFFECTS,
					0,
					self->s.origin,
					"bv",
					FX_MSSITHRA_ARROW_CHARGE,
					startpos);
}

void ssithraArrow(edict_t *self)
{//fixme; adjust for up/down
	if(!self->enemy)
	{
		ssithra_decide_stand(self);
		return;
	}

	if(self->enemy->health<=0)
	{
		self->enemy=NULL;
		ssithra_decide_stand(self);
		return;
	}

	if(self->monsterinfo.attack_finished>level.time)
		return;

	if(self->spawnflags & MSF_SSITHRA_ALPHA)
		ssithraDoDuckArrow(self, self->maxs[2] * 0.8);
	else
		ssithraDoArrow(self, 8);
}

void ssithraPanicArrow(edict_t *self)
{//fixme; adjust for up/down
	vec3_t	Forward,firedir;//, up;
	edict_t	*Arrow;

	if(self->s.fmnodeinfo[MESH__RIGHTARM].flags&FMNI_NO_DRAW)
	{
		if(self->curAnimID == ANIM_HEADLESS || self->curAnimID == ANIM_HEADLESSLOOP)
			ssithraKillSelf(self);
		return;
	}

//	gi.dprintf("Ssithra fire panic arrow\n");
	gi.sound(self,CHAN_WEAPON,Sounds[SND_ARROW2],1,ATTN_NORM,0);
	self->monsterinfo.attack_finished = level.time + 0.4;
	Arrow = G_Spawn();

//	Arrow->s.modelindex=gi.modelindex("models/objects/projectiles/sitharrow/tris.fm");

	create_ssith_arrow(Arrow);
	
	Arrow->owner=self;

	Arrow->health = 1; // tell the touch function what kind of arrow we are;

	VectorAdd(self->s.angles,self->v_angle_ofs,firedir);
	AngleVectors(firedir,Forward,NULL,NULL);
	VectorCopy(self->s.origin,Arrow->s.origin);	
	VectorMA(Arrow->s.origin,12,Forward,Arrow->s.origin);
	VectorCopy(self->movedir,Arrow->movedir);
	vectoangles(Forward,Arrow->s.angles);
	
	VectorScale(Forward,SSITHRA_SPOO_SPEED,Arrow->velocity);

	vectoangles(Arrow->velocity, Arrow->s.angles);
	Arrow->s.angles[YAW]+=90;
//fixme: redo these- make them look like squid ink?
	gi.CreateEffect(&Arrow->s,
		FX_SSITHRA_ARROW,
		CEF_OWNERS_ORIGIN,
		NULL,
		"bv",
		FX_SS_MAKE_ARROW,
		Arrow->velocity);

	G_LinkMissile(Arrow); 

	Arrow->nextthink=level.time+3;
	Arrow->think=G_FreeEdict;//ssithraArrowThink;
}

void ssithra_water_shoot (edict_t *self)
{
	SetAnim(self,ANIM_WATER_SHOOT);
}

void ssithraCheckLoop (edict_t *self)
{//see if should fire again
	vec3_t	v;
	float	len, melee_range, min_seperation, jump_range;

	if(!self->enemy)
		return;

	if(!visible(self, self->enemy))
		return;

	if(!infront(self, self->enemy))
		return;

	if(irand(0, 100) < self->bypass_missile_chance)
		return;

	VectorSubtract (self->enemy->s.origin, self->s.origin, v);
	len = VectorLength (v);
	melee_range = 64;
	jump_range = 128;
	min_seperation = self->maxs[0] + self->enemy->maxs[0];

	if (infront(self, self->enemy))
	{//don't loop if enemy close enough
		if (len < min_seperation + melee_range)
		{
			QPostMessage(self, MSG_MELEE, PRI_DIRECTIVE, NULL);
			return;
		}
		else if (len < min_seperation + jump_range && irand(0,10)<3)
		{
			VectorScale(v, 3, self->movedir);
			self->movedir[2] += 150;
			SetAnim(self, ANIM_LUNGE);
			return;
		}
	}

	self->monsterinfo.currframeindex = self->monsterinfo.currframeindex - 2;
}

//========================================
//EVASION
//========================================
void ssithraCheckDuckArrow (edict_t *self)
{
	if(M_ValidTarget(self, self->enemy))
	{
		if(ahead(self, self->enemy))
		{
//			if(M_DistanceToTarget(self, self->enemy)<self->missile_range)
//			{
//				if(clear_visible(self, self->enemy))
//				{
					ssithraDoDuckArrow(self, -18);
//				}
//			}
		}
	}
}

void ssithraCheckUnDuck (edict_t *self)
{
	if(self->evade_debounce_time < level.time)
		SetAnim(self, ANIM_UNDUCK);
	else
		SetAnim(self, ANIM_DUCKLOOP);
}

void ssithraJumpEvade (edict_t *self)
{
	vec3_t	forward;

	AngleVectors(self->s.angles, forward, NULL, NULL);
	VectorSet(self->movedir, 0, 0, 300);
	VectorMA(self->movedir, 200, forward, self->movedir);
	SetAnim(self, ANIM_FJUMP);
}

void ssithraCrouch (edict_t *self)
{
	self->maxs[2] = 0;
	self->viewheight = -6;
	gi.linkentity(self);
	SetAnim(self, ANIM_DUCKSHOOT);
}

void ssithraUnCrouch(edict_t *self)
{
	self->maxs[2] = STDMaxsForClass[self->classID][2] * self->s.scale;
	gi.linkentity(self);
	self->viewheight = self->maxs[2]*0.8;
}

void ssithra_evade (edict_t *self, G_Message_t *msg)
{
	edict_t			*projectile;		
	HitLocation_t	HitLocation;
	int duck_chance, jump_chance;
	int chance;
	float eta;

	ParseMsgParms(msg, "eif", &projectile, &HitLocation, &eta);
	
	switch(HitLocation)
	{
		case hl_Head:
			duck_chance = 90;
			jump_chance = 0;
		break;
		case hl_TorsoFront://split in half?
			duck_chance = 70;
			jump_chance = 0;
		break;
		case hl_TorsoBack://split in half?
			duck_chance = 70;
			jump_chance = 0;
		break;
		case hl_ArmUpperLeft:
			duck_chance = 60;
			jump_chance = 0;
		break;
		case hl_ArmLowerLeft://left arm
			duck_chance = 20;
			jump_chance = 30;
		break;
		case hl_ArmUpperRight:
			duck_chance = 60;
			jump_chance = 0;
		break;
		case hl_ArmLowerRight://right arm
			duck_chance = 20;
			jump_chance = 30;
		break;
		case hl_LegUpperLeft:
			duck_chance = 0;
			jump_chance = 50;
		break;
		case hl_LegLowerLeft://left leg
			duck_chance = 0;
			jump_chance = 90;
		break;
		case hl_LegUpperRight:
			duck_chance = 0;
			jump_chance = 50;
		break;
		case hl_LegLowerRight://right leg
			duck_chance = 0;
			jump_chance = 90;
		break;
		default:
			duck_chance = 20;
			jump_chance = 10;
		break;
	}

	if(!(self->spawnflags&MSF_FIXED))
	{
		chance = irand(0, 100);
		if(chance < jump_chance)
		{
			ssithraJumpEvade(self);
			return;
		}
	}

	chance = irand(0, 100);
	if(chance < duck_chance)
	{
		self->evade_debounce_time = level.time + eta;
		ssithraCrouch(self);
		return;
	}
}


//========================================
//MOODS
//========================================
qboolean SsithraCheckMood (edict_t *self)
{
	if(self->spawnflags & MSF_FIXED && self->curAnimID == ANIM_DELAY && self->enemy)
	{
		self->monsterinfo.searchType = SEARCH_COMMON;
		MG_FaceGoal(self, true);
	}

	self->mood_think(self);

	if(self->ai_mood == AI_MOOD_NORMAL)
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
			//gi.dprintf("Pursue- Nav\n");
			QPostMessage(self, MSG_RUN, PRI_DIRECTIVE, NULL);
			return true;
			break;
		case AI_MOOD_WALK:
			QPostMessage(self, MSG_WALK, PRI_DIRECTIVE, NULL);
			return true;
			break;
		case AI_MOOD_STAND:
			ssithra_decide_stand(self);
			return true;
			break;
			
		case AI_MOOD_DELAY:
#ifdef _DEVEL
			if(MGAI_DEBUG)
				gi.dprintf("Delay on frame %d\n", self->monsterinfo.currframeindex);
#endif
			SetAnim(self, ANIM_DELAY);
			return true;
			break;
			
		case AI_MOOD_WANDER:
			if(ssithraCheckInWater(self))
				SetAnim(self, ANIM_SWIMWANDER);
			else
				SetAnim(self, ANIM_WALK1);
			return true;
			break;

		case AI_MOOD_BACKUP:
			QPostMessage(self, MSG_FALLBACK, PRI_DIRECTIVE, NULL);
			return true;
			break;

		case AI_MOOD_JUMP:
			if(self->spawnflags&MSF_FIXED)
				SetAnim(self, ANIM_DELAY);
			else
				SetAnim(self, ANIM_FJUMP);
			return true;
			break;

		default :
#ifdef _DEVEL
//			if(MGAI_DEBUG)
				gi.dprintf("ssithra: Unusable mood %d!\n", self->ai_mood);
#endif
			break;
	}
	return false;
}

void ssithra_check_mood (edict_t *self, G_Message_t *msg)
{
	ParseMsgParms(msg, "i", &self->ai_mood);

	SsithraCheckMood(self);
}

/*-----------------------------------------------
	ssithra_sight
-----------------------------------------------*/
#define SSITHRA_SUPPORT_RADIUS 200

void ssithra_sight (edict_t *self, G_Message_t *msg)
{
	edict_t	*enemy = NULL;
	byte	sight_type;
	int		sound;

	if(self->targetname || self->monsterinfo.c_mode)
		return;//cinematic waiting to be activated, don't do this

	//Have we already said something?
	if (self->monsterinfo.supporters != -1)
		return;
	
	ParseMsgParms(msg, "be", &sight_type, &enemy);

	//See if we are the first to see the player
	if(M_CheckAlert(self, SSITHRA_SUPPORT_RADIUS))
	{
		sound = irand(SND_SIGHT1, SND_SIGHT6);
		gi.sound(self, CHAN_BODY, Sounds[sound], 1, ATTN_NORM, 0);
	}
}

qboolean ssithraAlerted (edict_t *self, alertent_t *alerter, edict_t *enemy)
{
	alertent_t	*checkent = NULL;
	vec3_t	saveangles;

	if(self->alert_time < level.time)
	{//not looking around
		if(!(alerter->alert_svflags&SVF_ALERT_NO_SHADE) && skill->value < 3.0 && !(self->monsterinfo.aiflags & AI_NIGHTVISION))
		{//not a sound-alert, 
			if(enemy->light_level < flrand(6, 77))
			{
				return false;
			}
		}
	}

	//the alert action happened in front of me, but the enemy is behind or the alert is behind me
	if(!infront_pos(self, alerter->origin))
	{
		if(irand(0, 1)&&self->curAnimID!=ANIM_IDLEBASIC)
		{//50% chance of startling them up if not already in startle anim
			//startle me, but don't wake up just yet
			if(alerter->lifetime < level.time + 4)
				self->alert_time = level.time + 4;//be ready for 4 seconds to wake up if alerted again
			else
				self->alert_time = alerter->lifetime;//be alert as long as the alert sticks around
			VectorCopy(self->v_angle_ofs, saveangles);
			VectorClear(self->v_angle_ofs);
			self->v_angle_ofs[YAW]=-90;

			if(infront_pos(self, alerter->origin))//fancy way of seeing if explosion was to right
			{
				VectorCopy(saveangles,self->v_angle_ofs);
				ssithraLookRight(self);//fixme: if already looking right, see you
			}
			else
			{
				VectorCopy(saveangles,self->v_angle_ofs);
				ssithraStartle(self);
			}
			return false;
		}
		else//spin around and wake up!
			self->spawnflags |= MSF_SSITHRA_SPIN;
	}
	else if(!infront(self,enemy))
	{
		if(irand(0, 1)&&self->curAnimID!=ANIM_IDLEBASIC)
		{//50% chance of startling them up if not already in startle anim
			//startle me, but don't wake up just yet
			self->alert_time = level.time + 4;//be ready to wake up for next 4 seconds
			VectorCopy(self->v_angle_ofs, saveangles);
			VectorClear(self->v_angle_ofs);
			self->v_angle_ofs[YAW]=-90;

			if(infront(self, enemy))//fancy way of seeing if explosion was to right
			{
				VectorCopy(saveangles,self->v_angle_ofs);
				ssithraLookRight(self);//fixme: if already looking right, see you
			}
			else
			{
				VectorCopy(saveangles,self->v_angle_ofs);
				ssithraStartle(self);
			}
			return false;
		}
		else//spin around and wake up!
			self->spawnflags |= MSF_SSITHRA_SPIN;
	}
	
	if(checkent)
	{//enemy of alert is behind me
	}

	if(enemy->svflags&SVF_MONSTER)
		self->enemy = alerter->enemy;
	else
		self->enemy = enemy;

	FoundTarget(self, true);

	return true;
}
//================================================================================


void SsithraStaticsInit()
{
	classStatics[CID_SSITHRA].msgReceivers[MSG_STAND] = ssithra_stand;
	classStatics[CID_SSITHRA].msgReceivers[MSG_WALK] = ssithra_walk;
	classStatics[CID_SSITHRA].msgReceivers[MSG_RUN] = ssithra_gallop;
	classStatics[CID_SSITHRA].msgReceivers[MSG_MELEE] = ssithra_melee;
	classStatics[CID_SSITHRA].msgReceivers[MSG_MISSILE] = ssithra_missile;
	classStatics[CID_SSITHRA].msgReceivers[MSG_WATCH] = ssithra_idlebasic;
	classStatics[CID_SSITHRA].msgReceivers[MSG_PAIN] = ssithra_pain;
	classStatics[CID_SSITHRA].msgReceivers[MSG_DEATH] = ssithra_death;
	classStatics[CID_SSITHRA].msgReceivers[MSG_DISMEMBER] = MG_parse_dismember_msg;
	classStatics[CID_SSITHRA].msgReceivers[MSG_JUMP] = ssithraMsgJump;
	classStatics[CID_SSITHRA].msgReceivers[MSG_FALLBACK] = ssithra_backup;
	classStatics[CID_SSITHRA].msgReceivers[MSG_DEATH_PAIN] = ssithra_dead_pain;
	classStatics[CID_SSITHRA].msgReceivers[MSG_EVADE] = ssithra_evade;
	classStatics[CID_SSITHRA].msgReceivers[MSG_CHECK_MOOD] = ssithra_check_mood;
	classStatics[CID_SSITHRA].msgReceivers[MSG_VOICE_SIGHT] = ssithra_sight;
	

	resInfo.numAnims = NUM_ANIMS;
	resInfo.animations = animations;
	resInfo.modelIndex = gi.modelindex("models/monsters/ssithra/tris.fm");

	Sounds[SND_PAIN1]=gi.soundindex("monsters/pssithra/pain1.wav");
	Sounds[SND_PAIN2]=gi.soundindex("monsters/pssithra/pain2.wav");	
	Sounds[SND_DIE]=gi.soundindex("monsters/pssithra/die.wav");	
	Sounds[SND_GIB]=gi.soundindex("monsters/pssithra/gib.wav");
	Sounds[SND_SWIPEHIT]=gi.soundindex("monsters/pssithra/swipehit.wav");	
	Sounds[SND_ARROW1]=gi.soundindex("monsters/pssithra/arrow1.wav");	
	Sounds[SND_ARROW2]=gi.soundindex("monsters/pssithra/arrow2.wav");	
	Sounds[SND_GROWL1]=gi.soundindex("monsters/pssithra/growl1.wav");	
	Sounds[SND_GROWL2] = gi.soundindex ("monsters/pssithra/growl2.wav");	
	Sounds[SND_GROWL3] = gi.soundindex ("monsters/pssithra/growl3.wav");	
	Sounds[SND_INWATER] = gi.soundindex ("monsters/pssithra/inwater.wav");	
	Sounds[SND_NAMOR] = gi.soundindex ("monsters/pssithra/namor.wav");	
	Sounds[SND_LAND] = gi.soundindex ("monsters/pssithra/land.wav");	
	Sounds[SND_SWIPE] = gi.soundindex ("monsters/pssithra/swipe.wav");
	Sounds[SND_SWIM] = gi.soundindex ("monsters/pssithra/swim.wav");
	Sounds[SND_SWIM2] = gi.soundindex ("monsters/pssithra/swim2.wav");

	Sounds[SND_SIGHT1] = gi.soundindex ("monsters/pssithra/ssithvoice1.wav");
	Sounds[SND_SIGHT2] = gi.soundindex ("monsters/pssithra/ssithvoice2.wav");
	Sounds[SND_SIGHT3] = gi.soundindex ("monsters/pssithra/ssithvoice3.wav");
	Sounds[SND_SIGHT4] = gi.soundindex ("monsters/pssithra/ssithvoice4.wav");
	Sounds[SND_SIGHT5] = gi.soundindex ("monsters/pssithra/ssithvoice5.wav");
	Sounds[SND_SIGHT6] = gi.soundindex ("monsters/pssithra/ssithvoice6.wav");

	Sounds[SND_ARROW_CHARGE] = gi.soundindex ("monsters/pssithra/guncharge.wav");
	Sounds[SND_ARROW_FIRE] = gi.soundindex ("monsters/pssithra/gunfire.wav");

	resInfo.numSounds = NUM_SOUNDS;
	resInfo.sounds = Sounds;

	classStatics[CID_SSITHRA].resInfo = &resInfo;
}

/*QUAKED monster_ssithra (1 .5 0) (-16 -16 -32) (16 16 26) AMBUSH ASLEEP 4 Namor Spin ToughGuy Clothed FIXED WANDER MELEE_LEAD STALK COWARD EXTRA1 EXTRA2 EXTRA3 EXTRA4

The plague ssithra 

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
mintel					= 28
melee_range				= 48
missile_range			= 512
min_missile_range		= 48
bypass_missile_chance	= 25
jump_chance				= 100
wakeup_distance			= 1024

NOTE: A value of zero will result in defaults, if you actually want zero as the value, use -1
*/
void SP_monster_plague_ssithra (edict_t *self)
{
	qboolean alpha = true;

	if(!skill->value)
	{
		if(!irand(0, 2))//30% - but won't fire explosives
			self->spawnflags |= MSF_SSITHRA_CLOTHED;
	}
	else if (skill->value == 1)
	{
		if(!irand(0, 3))//25%
			self->spawnflags |= MSF_SSITHRA_CLOTHED;
	}
	else
	{
		if(irand(0, 1))//50%
			self->spawnflags |= MSF_SSITHRA_CLOTHED;
	}

	if(self->spawnflags&MSF_SSITHRA_NAMOR)
		self->spawnflags |= MSF_AMBUSH;

	// Generic Monster Initialization
	if (!monster_start(self))
		return;					// Failed initialization

	self->msgHandler = DefaultMsgHandler;
	self->monsterinfo.alert = ssithraAlerted;
	self->classID = CID_SSITHRA;
	self->think = walkmonster_start_go;
	self->monsterinfo.dismember = ssithra_dismember;

	self->materialtype = MAT_FLESH;
//	self->monsterinfo.aiflags |= AI_SWIM_OK;
	self->flags |= FL_IMMUNE_SLIME;
	
	ssithraCheckInWater(self);

	self->isBlocked = ssithra_blocked;

	if(self->health<=0)
		self->health = SSITHRA_HEALTH;

	//Apply to the end result (whether designer set or not)
	self->max_health = self->health = MonsterHealth(self->health);

	self->mass = SSITHRA_MASS;
	self->yaw_speed = 20;

	self->movetype = PHYSICSTYPE_STEP;
	VectorClear(self->knockbackvel);

	self->solid=SOLID_BBOX;

	self->monsterinfo.supporters = -1;
	VectorCopy(STDMinsForClass[self->classID], self->mins);
	VectorCopy(STDMaxsForClass[self->classID], self->maxs);	
	self->viewheight = self->maxs[2]*0.8;

	self->s.modelindex = classStatics[CID_SSITHRA].resInfo->modelIndex;

	self->touch = M_Touch;
	
	if(self->spawnflags & MSF_SSITHRA_CLOTHED)
		self->s.skinnum = 2;
	else
		self->s.skinnum = 0;

	//scaling them up in code like this is bad for designers
	self->s.scale = self->monsterinfo.scale = (MODEL_SCALE)+0.1;// + flrand(0.1, 0.3));
	// Note that at less than 110% of the size 

	//Turn off dismemberment caps, can't see them, so save some polys
	self->s.fmnodeinfo[MESH__CAPLOWERTORSO].flags |= FMNI_NO_DRAW;
	self->s.fmnodeinfo[MESH__CAPTOPUPPERTORSO].flags |= FMNI_NO_DRAW;
	self->s.fmnodeinfo[MESH__CAPBOTTOMUPPERTORSO].flags |= FMNI_NO_DRAW;
	self->s.fmnodeinfo[MESH__CAPHEAD].flags |= FMNI_NO_DRAW;
	if(!(self->spawnflags&MSF_SSITHRA_ALPHA))
	{
		if(irand(0,10)<6)
		{
			if(irand(0,10)<5)
			{
				self->s.fmnodeinfo[MESH__CENTERSPIKE].flags |= FMNI_USE_SKIN;
				self->s.fmnodeinfo[MESH__CENTERSPIKE].skin = self->s.skinnum+1;
			}
			else
				self->s.fmnodeinfo[MESH__CENTERSPIKE].flags |= FMNI_NO_DRAW;
			alpha = false;
		}
		if(irand(0,10)<6)
		{
			if(irand(0,10)<5)
			{
				self->s.fmnodeinfo[MESH__LEFT1SPIKE].flags |= FMNI_USE_SKIN;
				self->s.fmnodeinfo[MESH__LEFT1SPIKE].skin = self->s.skinnum+1;
			}
			else
				self->s.fmnodeinfo[MESH__LEFT1SPIKE].flags |= FMNI_NO_DRAW;
			alpha = false;
		}
		if(irand(0,10)<6)
		{
			if(irand(0,10)<5)
			{
				self->s.fmnodeinfo[MESH__RIGHT1SPIKE].flags |= FMNI_USE_SKIN;
				self->s.fmnodeinfo[MESH__RIGHT1SPIKE].skin = self->s.skinnum+1;
			}
			else
				self->s.fmnodeinfo[MESH__RIGHT1SPIKE].flags |= FMNI_NO_DRAW;
			alpha = false;
		}
		if(irand(0,10)<6)
		{
			if(irand(0,10)<5)
			{
				self->s.fmnodeinfo[MESH__RIGHT2SPIKE].flags |= FMNI_USE_SKIN;
				self->s.fmnodeinfo[MESH__RIGHT2SPIKE].skin = self->s.skinnum+1;
			}
			else
				self->s.fmnodeinfo[MESH__RIGHT2SPIKE].flags |= FMNI_NO_DRAW;
			alpha = false;
		}
	}

	self->s.color.a = 255;
	if(alpha)//tough guy!
	{//TODO: other ssithras won't attack this guy and will follow him
		self->health += 75;
		self->s.scale = self->monsterinfo.scale = MODEL_SCALE + 0.5;
		self->spawnflags|=MSF_SSITHRA_ALPHA;
		self->s.color.r = 255;
		self->s.color.g = 255;
		self->s.color.b = 128;
	}
	else
	{
		self->s.color.r = 200 + irand(-50, 50);
		self->s.color.g = 200 + irand(-50, 50);
		self->s.color.b = 200 + irand(-50, 50);
	}

	self->monsterinfo.otherenemyname = "obj_barrel";

	//set up my mood function
	MG_InitMoods(self);
	if(!irand(0,2))
		self->ai_mood_flags |= AI_MOOD_FLAG_PREDICT;

	QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
	self->svflags |= SVF_WAIT_NOTSOLID;
	self->flags |= FL_AMPHIBIAN;

//	if(self->spawnflags & MSF_SSITHRA_NAMOR)
//		self->use = ssithraNamorTriggered;
}


/*QUAKED obj_corpse_ssithra (1 .5 0) (-30 -12 -2) (30 12 2) INVULNERABLE ANIMATE EXPLODING NOPUSH
A dead plague ssithra
---------- KEYS -----------------  
style - skin of ssithra (default 0)
0 - damage skin
1 - some bad bad skin. (not used)
2 - normal skin
-------  FIELDS  ------------------
INVULNERABLE - it can't be hurt
ANIMATE - N/A
EXPLODING - N/A
NOPUSH - N/A (corpse can't be pushed)
-----------------------------------
*/
void SP_obj_corpse_ssithra(edict_t *self)
{
	self->s.origin[2] += 26.0;

	VectorSet(self->mins,-30,-12,-2);
	VectorSet(self->maxs,30,12,2);

	self->s.modelindex = gi.modelindex("models/monsters/ssithra/tris.fm");

	self->s.frame = FRAME_death_a12;	//Ths is the reason the function can't be put in g_obj.c

	// Setting the skinnum correctly
	if (!self->style)
		self->s.skinnum = 1;
	else
		self->s.skinnum = 0;

	self->spawnflags |= OBJ_NOPUSH;	// Can't be pushed
	self->svflags |= SVF_DEADMONSTER;//doesn't block walking

	ObjectInit(self,120,80,MAT_FLESH,SOLID_BBOX);
}

