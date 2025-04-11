//
// m_gkrokon.c
//
// Copyright 1998 Raven Software
//

#include "m_gkrokon.h"
#include "m_gkrokon_shared.h"
#include "m_gkrokon_anim.h"
#include "decals.h"
#include "g_debris.h" //mxd
#include "g_DefaultMessageHandler.h"
#include "g_monster.h"
#include "g_playstats.h"
#include "m_stats.h"
#include "mg_ai.h" //mxd
#include "mg_guide.h" //mxd
#include "Random.h"
#include "Utilities.h"
#include "Vector.h"
#include "g_local.h"

#define GKROKON_SPOO_SPEED	450.0f
#define GKROKON_SPOO_ARC	150.0f

#pragma region ========================== Gkrokon base info ==========================

static const animmove_t* animations[NUM_ANIMS] =
{
	&gkrokon_move_stand1,
	&gkrokon_move_stand2,
	&gkrokon_move_stand3,
	&gkrokon_move_stand4,
	&gkrokon_move_crouch1,
	&gkrokon_move_crouch2,
	&gkrokon_move_crouch3,
	&gkrokon_move_walk1,
	&gkrokon_move_run1,
	&gkrokon_move_run2,
	&gkrokon_move_jump1,
	&gkrokon_move_forced_jump,
	&gkrokon_move_melee_attack1,
	&gkrokon_move_melee_attack2,
	&gkrokon_move_missile_attack1,
	&gkrokon_move_eat1,
	&gkrokon_move_eat2,
	&gkrokon_move_eat3,
	&gkrokon_move_pain1,
	&gkrokon_move_death1,
	&gkrokon_move_hop1,
	&gkrokon_move_run_away,
	&gkrokon_move_missile_attack2,
	&gkrokon_move_delay,
};

static int sounds[NUM_SOUNDS];

#pragma endregion

#pragma region ========================== Spoo functions ==========================

static void GkrokonSpooTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surface)
{
	if (surface != NULL && (surface->flags & SURF_SKY))
	{
		G_FreeEdict(self);
		return;
	}

	// Are we reflecting?
	if (EntReflecting(other, true, true) && self->reflect_debounce_time > 0)
	{
		Create_rand_relect_vect(self->velocity, self->velocity);
		Vec3ScaleAssign(GKROKON_SPOO_SPEED / 2.0f, self->velocity);
		GkrokonSpooReflect(self, other, self->velocity);

		return;
	}

	// Apply damage.
	const int damage = irand(GKROKON_DMG_SPOO_MIN, GKROKON_DMG_SPOO_MAX); //mxd. Don't use self->dmg; Calculate here, so this val is used by T_DamageRadius() even when other->takedamage is DAMAGE_NO.

	if (other->takedamage != DAMAGE_NO)
		T_Damage(other, self, self->owner, self->movedir, other->s.origin, plane->normal, damage, damage, 0, MOD_DIED);
	else
		VectorMA(self->s.origin, -16.0f, self->movedir, self->s.origin);

	T_DamageRadius(self, self->owner, self, 5.0f, (float)damage, (float)damage / 4.0f, DAMAGE_NO_KNOCKBACK | DAMAGE_ATTACKER_IMMUNE, MOD_DIED);

	gi.RemoveEffects(&self->s, FX_REMOVE_EFFECTS);

	// Create decal?
	vec3_t plane_dir;
	VectorCopy(vec3_up, plane_dir); //mxd. Initialize, so we pass a valid vector when creating FX_SPOO_SPLAT even when IsDecalApplicable() returns false.

	if (IsDecalApplicable(other, self->s.origin, surface, plane, plane_dir))
		gi.CreateEffect(NULL, FX_SCORCHMARK, 0, self->s.origin, "d", plane_dir);

	// Splatter some spoo over the surface.
	gi.CreateEffect(NULL, FX_SPOO_SPLAT, 0, self->s.origin, "d", plane_dir);
	gi.sound(self, CHAN_WEAPON, gi.soundindex("monsters/rat/gib.wav"), 1.0f, ATTN_NORM, 0.0f);

	G_SetToFree(self);
}

static void GkrokonSpooIsBlocked(edict_t* self, trace_t* trace) //mxd. Named 'GkrokonSpooTouch2' in original logic.
{
	GkrokonSpooTouch(self, trace->ent, &trace->plane, trace->surface);
}

static void SpooInit(edict_t* spoo) //mxd. Named 'create_gkrokon_spoo' in original logic. //TODO: rename to GkrokonSpooInit.
{
	spoo->movetype = PHYSICSTYPE_FLY;
	spoo->solid = SOLID_BBOX;
	spoo->classname = "Gkrokon_Spoo";
	spoo->clipmask = MASK_SHOT;
	spoo->s.scale = 0.5f;
	spoo->s.effects = EF_CAMERA_NO_CLIP;

	spoo->touch = GkrokonSpooTouch;
	spoo->isBlocked = GkrokonSpooIsBlocked;

	VectorSet(spoo->mins, -1.0f, -1.0f, -1.0f);
	VectorSet(spoo->maxs, 1.0f, 1.0f, 1.0f);
}

edict_t* GkrokonSpooReflect(edict_t* self, edict_t* other, vec3_t vel)
{
	edict_t* spoo = G_Spawn();

	SpooInit(spoo);

	spoo->enemy = self->owner;
	spoo->owner = other;
	spoo->reflect_debounce_time = self->reflect_debounce_time - 1;
	spoo->reflected_time = self->reflected_time;

	VectorCopy(self->s.origin, spoo->s.origin);
	VectorCopy(vel, spoo->velocity);
	VectorNormalize2(vel, spoo->movedir);
	AnglesFromDir(spoo->movedir, spoo->s.angles);

	gi.linkentity(spoo);
	gi.CreateEffect(&spoo->s, FX_SPOO, CEF_OWNERS_ORIGIN, spoo->s.origin, "");

	G_SetToFree(self);

	return spoo;
}

void GkrokonSpoo(edict_t* self) //TODO: rename to gkrokon_spoo_attack
{
	if (self->enemy == NULL)
		return;

	vec3_t forward;
	AngleVectors(self->s.angles, forward, NULL, NULL);

	vec3_t diff;
	VectorSubtract(self->enemy->s.origin, self->s.origin, diff);
	const float dist = VectorNormalize(diff);

	// Skip when enemy not within 90-deg forward cone.
	if (DotProduct(diff, forward) < 0.5f) //mxd. Was done in the middle of spoo init logic in original version, before gi.linkentity() call.
		return;

	// Spawn the spoo globbit.
	edict_t* spoo = G_Spawn();

	SpooInit(spoo);

	spoo->reflect_debounce_time = MAX_REFLECT;
	spoo->enemy = self->enemy;
	spoo->owner = self;

	VectorCopy(self->s.origin, spoo->s.origin);
	VectorMA(spoo->s.origin, -16.0f, forward, spoo->s.origin);
	spoo->s.origin[2] += 12.0f;

	VectorCopy(self->movedir, spoo->movedir);
	vectoangles(forward, spoo->s.angles);

	VectorScale(diff, GKROKON_SPOO_SPEED, spoo->velocity);
	spoo->velocity[2] += GKROKON_SPOO_ARC + (dist - 256.0f);
	vectoangles(spoo->velocity, spoo->s.angles);

	gi.linkentity(spoo);
	gi.sound(spoo, CHAN_WEAPON, sounds[SND_SPOO], 1.0f, ATTN_NORM, 0.0f);
	gi.CreateEffect(&spoo->s, FX_SPOO, CEF_OWNERS_ORIGIN, spoo->s.origin, "");

	self->monsterinfo.misc_debounce_time = level.time + flrand(0.5f, 3.0f);
}

#pragma endregion

void GkrokonBite(edict_t* self, float right_side) //TODO: rename to gkrokon_bite.
{
	vec3_t start_offset;
	vec3_t end_offset;

	// From the right side.
	if (right_side != 0.0f)
	{
		VectorSet(start_offset, 32.0f, 48.0f, 32.0f);
		VectorSet(end_offset, 32.0f, 16.0f, 24.0f);
	}
	else
	{
		VectorSet(start_offset, 32.0f, -24.0f, 32.0f);
		VectorSet(end_offset, 24.0f, 32.0f, 24.0f);
	}

	const vec3_t mins = { -2.0f, -2.0f, -2.0f };
	const vec3_t maxs = {  2.0f,  2.0f,  2.0f };

	trace_t trace;
	vec3_t direction;
	edict_t* victim = M_CheckMeleeLineHit(self, start_offset, end_offset, mins, maxs, &trace, direction);

	if (victim != NULL && victim != self)
	{
		// Hurt whatever we were whacking away at.
		vec3_t blood_dir;
		VectorSubtract(start_offset, end_offset, blood_dir);
		VectorNormalize(blood_dir);

		const int damage = irand(GKROKON_DMG_BITE_MIN, GKROKON_DMG_BITE_MAX);
		T_Damage(victim, self, self, direction, trace.endpos, blood_dir, damage, damage, DAMAGE_NORMAL, MOD_DIED);

		// Play hit sound.
		gi.sound(self, CHAN_WEAPON, sounds[irand(SND_BITEHIT1, SND_BITEHIT2)], 1.0f, ATTN_NORM, 0.0f);
	}
	else
	{
		// Play swoosh sound.
		gi.sound(self, CHAN_WEAPON, sounds[irand(SND_BITEMISS1, SND_BITEMISS2)], 1.0f, ATTN_NORM, 0.0f);
	}
}

void gkrokonSound(edict_t* self, float channel, float sound_index, float attenuation) //TODO: rename to gkrokon_sound.
{
	gi.sound(self, (int)channel, sounds[(int)sound_index], 1.0f, attenuation, 0.0f);
}

void gkrokonRandomWalkSound(edict_t* self) //TODO: rename to gkrokon_random_walk_sound.
{
	if (irand(0, 3) == 0)
		gi.sound(self, CHAN_BODY, sounds[irand(SND_WALK1, SND_WALK2)], 1.0f, ATTN_NORM, 0.0f);
}

/*-----------------------------------------------
	GkrokonDead
-----------------------------------------------*/

void GkrokonDead(edict_t *self)
{
	M_EndDeath(self);
}

/*-----------------------------------------------
	beetle_ai_stand
-----------------------------------------------*/

void beetle_ai_stand(edict_t *self, float dist)
{
	if (M_ValidTarget(self, self->enemy))
	{
		MG_FaceGoal(self, true);
	}
}

/*-----------------------------------------------
	beetle_idle_sound
-----------------------------------------------*/

void beetle_idle_sound(edict_t *self)
{
	int chance = irand(0,20);

	switch (chance)
	{
	case 0:
		gkrokonSound(self, CHAN_BODY, SND_IDLE1, ATTN_NORM);
		break;

	case 1:
		gkrokonSound(self, CHAN_BODY, SND_IDLE2, ATTN_NORM);
		break;
	
	default:
		break;
	}
}

/*

	Message Handlers

*/

static void GkrokonFallbackMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'beetle_skitter' in original logic.
{
	if (self->spawnflags & MSF_FIXED)
	{
		SetAnim(self, ANIM_DELAY);
	}
	else if (irand(0, 10) < 2 && self->monsterinfo.attack_finished < level.time) //mxd. Inline SkitterAway().
	{
		self->monsterinfo.attack_finished = level.time + 5.0f;
		SetAnim(self, ANIM_SNEEZE);
	}
	else
	{
		SetAnim(self, ANIM_RUNAWAY);
	}
}

/*-----------------------------------------------
	beetle_check_mood
-----------------------------------------------*/

void beetle_check_mood (edict_t *self, G_Message_t *msg)
{
	ParseMsgParms(msg, "i", &self->ai_mood);

	GkrokonPause(self);
}

/*-----------------------------------------------
	beetle_walk
-----------------------------------------------*/

void beetle_walk(edict_t *self,G_Message_t *Msg)
{
	if(self->spawnflags&MSF_FIXED)
		SetAnim(self, ANIM_STAND3);
	else
		SetAnim(self,ANIM_WALK1);
}

/*-----------------------------------------------
	beetle_run
-----------------------------------------------*/

void beetle_run(edict_t *self, G_Message_t *Msg)
{
	int		chance;
	float	dist;

	//Make sure we're not getting up from laying down without the proper animations
	if(self->ai_mood == AI_MOOD_FLEE)
	{
		if(irand(0,1))
			SetAnim(self, ANIM_RUN1);
		else
			SetAnim(self, ANIM_RUN2);

		return;
	}

	if (self->curAnimID == ANIM_STAND1)
	{
		SetAnim(self, ANIM_STAND2);
		return;
	}

	if (M_ValidTarget(self, self->enemy))
	{
		dist = M_DistanceToTarget(self, self->enemy);	

		chance = irand(0,100);

		if (dist < 300)
		{
			if (chance < 5)
				SetAnim(self, ANIM_STAND3);
			else if (chance < 20)
				SetAnim(self, ANIM_RUN2);
			else
				SetAnim(self, ANIM_RUN1);
		}
		else
		{
			SetAnim(self, ANIM_RUN1);
		}

		return;
	}

	//If our enemy is dead, we need to stand
	QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
}

/*-----------------------------------------------
	beetle_stand
-----------------------------------------------*/

void beetle_stand(edict_t *self,G_Message_t *Msg)
{
	if (self->spawnflags & MSF_EXTRA1)
		SetAnim(self, ANIM_STAND1);
	else
		SetAnim(self, ANIM_STAND3);
}

/*-----------------------------------------------
	beetle_melee
-----------------------------------------------*/

void beetle_missile(edict_t *self,G_Message_t *Msg)
{
	int		chance;
	float	dist;

	chance = irand(0,100);

	if(self->enemy)
	{
		if(self->monsterinfo.misc_debounce_time < level.time)
		{
			dist = M_DistanceToTarget(self, self->enemy);

			if(self->spawnflags&MSF_FIXED)
			{
				if (dist < self->missile_range)
					SetAnim(self, ANIM_MISSILE1);
				else if(self->curAnimID == ANIM_CROUCH1 && !irand(0, 2))
					SetAnim(self, ANIM_CROUCH2);//go into stand
				else if(self->curAnimID == ANIM_STAND3 && !irand(0, 10))
					SetAnim(self, ANIM_CROUCH3);//go into a crouch
				else
					SetAnim(self, ANIM_DELAY);//stand around
			}
			else if (dist < 64)
			{
				if (chance < 10)
					SetAnim(self, ANIM_MISSILE1);
				else if (chance < 30)
				{
					SetAnim(self, ANIM_RUNAWAY);
					self->monsterinfo.flee_finished = level.time + 1;
				}
				else
				{
					if (irand(0,1))
						SetAnim(self, ANIM_MELEE1);
					else
						SetAnim(self, ANIM_MELEE2);
				}
			}
			else if ((dist > 64) && (dist < 200))
			{
				if (chance < 40)
					SetAnim(self, ANIM_MISSILE1);
				else if ((self->monsterinfo.flee_finished < level.time) && (dist > 100))
					SetAnim(self, ANIM_RUN2);
				else
					SetAnim(self, ANIM_STAND3);
			}
			else if (dist < 300)
			{
				if (chance < 5)
					SetAnim(self, ANIM_STAND3);
				else if (chance < 20)
					SetAnim(self, ANIM_RUN2);
				else
					SetAnim(self, ANIM_RUN1);
			}
			else
			{
				SetAnim(self, ANIM_RUN1);
			}
		}
		else if (irand(0,10) < 6)
		{
			QPostMessage(self, MSG_RUN, PRI_DIRECTIVE, NULL);
		}
		else
		{
			GkrokonFallbackMsgHandler(self, Msg); //mxd
		}
		return;
	}
	QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
}

void beetle_melee(edict_t *self,G_Message_t *Msg)
{
	beetle_missile(self, Msg);
}

/*-----------------------------------------------
	beetle_pain
-----------------------------------------------*/

void beetle_pain(edict_t *self,G_Message_t *Msg)
{
	int	temp, damage;
	int	force_damage;
	
	ParseMsgParms(Msg, "eeiii", &temp, &temp, &force_damage, &damage, &temp);
	
	//Weighted random based on health compared to the maximum it was at
	if (force_damage||((flrand(0, self->max_health+50) > self->health) && irand(0,2)))
	{
		if(irand(0,1))
			gi.sound (self, CHAN_WEAPON, sounds[SND_PAIN1], 1, ATTN_NORM, 0);
		else
			gi.sound (self, CHAN_WEAPON, sounds[SND_PAIN2], 1, ATTN_NORM, 0);

		SetAnim(self, ANIM_PAIN1);
	}

	if (!irand(0,2))
	{
		self->monsterinfo.aiflags |= AI_FLEE;
		self->monsterinfo.flee_finished = level.time + 15;
	}
}

/*-----------------------------------------------
	beetle_eat
-----------------------------------------------*/

void beetle_eat(edict_t *self,G_Message_t *Msg)
{
	int chance = irand(0,2);

	switch (chance)
	{
	case 0:
		SetAnim(self, ANIM_EAT1);
		break;
	case 1:
		SetAnim(self, ANIM_EAT2);
		break;
	case 2:
		SetAnim(self, ANIM_EAT3);
		break;
	}
}

/*-----------------------------------------------
	beetle_death
-----------------------------------------------*/

void beetle_death(edict_t *self,G_Message_t *Msg)
{
	edict_t	*targ, *inflictor, *attacker;
	float	damage;

	ParseMsgParms(Msg, "eeei", &targ, &inflictor, &attacker, &damage);

	M_StartDeath(self, ANIM_DIE1);

	if (self->health < -80)
	{
		BecomeDebris(self);
		return;
	}
	else
	{
		if(self->health < -10)
		{
			vec3_t	forward;
			int	i, num_limbs;

			num_limbs = irand(3, 10);
			for(i = 0; i < num_limbs; i++)
			{
				if(self->svflags&SVF_MONSTER)
					self->monsterinfo.dismember(self, flrand(80, 160), irand(hl_Head, hl_LegLowerRight) | hl_MeleeHit);
			}
			AngleVectors(self->s.angles, forward, NULL, NULL);
			VectorMA(self->velocity, -400, forward, self->velocity);
			self->velocity[2] += 150;
		}
		SetAnim(self, ANIM_DIE1);
	}

	gi.sound (self, CHAN_BODY, sounds[SND_DIE], 1, ATTN_NORM, 0);
}
	
int Bit_for_MeshNode_gk [NUM_MESH_NODES] =
{
	BIT_WAIT1,
	BIT_SHELLA_P1,
	BIT_SPIKE_P1,	
	BIT_HEAD_P1,	
	BIT_RPINCHERA_P1,
	BIT_RPINCHERB_P1,
	BIT_LPINCHERA_P1,
	BIT_LPINCHERB_P1,
	BIT_LARM_P1,
	BIT_RARM_P1,
	BIT_ABDOMEN_P1,
	BIT_LTHIGH_P1,
	BIT_RTHIGH_P1,
	BIT_SHELLB_P1,
};

qboolean canthrownode_gk (edict_t *self, int BP, int *throw_nodes)
{//see if it's on, if so, add it to throw_nodes
	//turn it off on thrower
	if(!(self->s.fmnodeinfo[BP].flags & FMNI_NO_DRAW))
	{
		*throw_nodes |= Bit_for_MeshNode_gk[BP];
		self->s.fmnodeinfo[BP].flags |= FMNI_NO_DRAW;
		return true;
	}
	return false;
}

void beetle_dismember(edict_t *self, int damage, int HitLocation)
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
			if(self->s.fmnodeinfo[MESH__SHELLB_P1].flags & FMNI_NO_DRAW)
				break;
			self->s.fmnodeinfo[MESH__SHELLB_P1].flags |= FMNI_USE_SKIN;
			self->s.fmnodeinfo[MESH__SHELLB_P1].skin = self->s.skinnum + 1;
			break;

		case hl_TorsoFront://split in half?
			if(!(self->s.fmnodeinfo[MESH__HEAD_P1].flags & FMNI_USE_SKIN))
			{
				if(irand(0, 4))
				{
					self->s.fmnodeinfo[MESH__HEAD_P1].flags |= FMNI_USE_SKIN;			
					self->s.fmnodeinfo[MESH__HEAD_P1].skin = self->s.skinnum+1;
				}
			}
			else if(canthrownode_gk(self, MESH__HEAD_P1, &throw_nodes))
			{
				AngleVectors(self->s.angles,NULL,right,NULL);
				gore_spot[2]+=self->maxs[2]*0.3;
				VectorMA(gore_spot,-10,right,gore_spot);
				ThrowBodyPart(self, &gore_spot, throw_nodes, damage, 0);
				T_Damage(self, self->enemy, self->enemy, vec3_origin, vec3_origin, vec3_origin, 9999, 200, DAMAGE_NORMAL,MOD_DIED);
				return;
			}

			if(!(self->s.fmnodeinfo[MESH__SPIKE_P1].flags & FMNI_USE_SKIN))
			{
				if(irand(0, 4))
				{
					self->s.fmnodeinfo[MESH__SPIKE_P1].flags |= FMNI_USE_SKIN;			
					self->s.fmnodeinfo[MESH__SPIKE_P1].skin = self->s.skinnum+1;
				}
			}
			else if(canthrownode_gk(self, MESH__SPIKE_P1, &throw_nodes))
			{
				AngleVectors(self->s.angles,NULL,right,NULL);
				gore_spot[2]+=self->maxs[2]*0.3;
				VectorMA(gore_spot,-10,right,gore_spot);
				ThrowBodyPart(self, &gore_spot, throw_nodes, damage, 0);
			}
	
			if(!(self->s.fmnodeinfo[MESH__RPINCHERA_P1].flags & FMNI_USE_SKIN))
			{
				if(irand(0, 4))
				{
					self->s.fmnodeinfo[MESH__RPINCHERA_P1].flags |= FMNI_USE_SKIN;			
					self->s.fmnodeinfo[MESH__RPINCHERA_P1].skin = self->s.skinnum+1;
				}
			}
			else if(canthrownode_gk(self, MESH__RPINCHERA_P1, &throw_nodes))
			{
				AngleVectors(self->s.angles,NULL,right,NULL);
				gore_spot[2]+=self->maxs[2]*0.3;
				VectorMA(gore_spot,-10,right,gore_spot);
				ThrowBodyPart(self, &gore_spot, throw_nodes, damage, 0);
			}
			
			if(!(self->s.fmnodeinfo[MESH__LPINCHERA_P1].flags & FMNI_USE_SKIN))
			{
				if(irand(0, 4))
				{
					self->s.fmnodeinfo[MESH__LPINCHERA_P1].flags |= FMNI_USE_SKIN;			
					self->s.fmnodeinfo[MESH__LPINCHERA_P1].skin = self->s.skinnum+1;
				}
			}
			else if(canthrownode_gk(self, MESH__LPINCHERA_P1, &throw_nodes))
			{
				AngleVectors(self->s.angles,NULL,right,NULL);
				gore_spot[2]+=self->maxs[2]*0.3;
				VectorMA(gore_spot,-10,right,gore_spot);
				ThrowBodyPart(self, &gore_spot, throw_nodes, damage, 0);
			}
			
			if(!(self->s.fmnodeinfo[MESH__RPINCHERB_P1].flags & FMNI_USE_SKIN))
			{
				if(irand(0, 4))
				{
					self->s.fmnodeinfo[MESH__RPINCHERB_P1].flags |= FMNI_USE_SKIN;			
					self->s.fmnodeinfo[MESH__RPINCHERB_P1].skin = self->s.skinnum+1;
				}
			}
			else if(canthrownode_gk(self, MESH__RPINCHERB_P1, &throw_nodes))
			{
				AngleVectors(self->s.angles,NULL,right,NULL);
				gore_spot[2]+=self->maxs[2]*0.3;
				VectorMA(gore_spot,-10,right,gore_spot);
				ThrowBodyPart(self, &gore_spot, throw_nodes, damage, 0);
			}

			if(!(self->s.fmnodeinfo[MESH__LPINCHERB_P1].flags & FMNI_USE_SKIN))
			{
				if(irand(0, 4))
				{
					self->s.fmnodeinfo[MESH__LPINCHERB_P1].flags |= FMNI_USE_SKIN;			
					self->s.fmnodeinfo[MESH__LPINCHERB_P1].skin = self->s.skinnum+1;
				}
			}
			else if(canthrownode_gk(self, MESH__LPINCHERB_P1, &throw_nodes))
			{
				AngleVectors(self->s.angles,NULL,right,NULL);
				gore_spot[2]+=self->maxs[2]*0.3;
				VectorMA(gore_spot,-10,right,gore_spot);
				ThrowBodyPart(self, &gore_spot, throw_nodes, damage, 0);
			}
			break;
		case hl_TorsoBack://split in half?
			if(self->s.fmnodeinfo[MESH__ABDOMEN_P1].flags & FMNI_NO_DRAW)
				break;
			self->s.fmnodeinfo[MESH__ABDOMEN_P1].flags |= FMNI_USE_SKIN;			
			self->s.fmnodeinfo[MESH__ABDOMEN_P1].skin = self->s.skinnum+1;
			break;

		case hl_ArmLowerLeft://left arm
		case hl_ArmUpperLeft:
			if(self->s.fmnodeinfo[MESH__LARM_P1].flags & FMNI_NO_DRAW)
				break;
			if(self->s.fmnodeinfo[MESH__LARM_P1].flags & FMNI_USE_SKIN)
				damage*=1.5;//greater chance to cut off if previously damaged
			if(flrand(0,self->health)<damage*0.75&&dismember_ok)
			{
				if(canthrownode_gk(self, MESH__LARM_P1, &throw_nodes))
				{
					AngleVectors(self->s.angles,NULL,right,NULL);
					gore_spot[2]+=self->maxs[2]*0.3;
					VectorMA(gore_spot,-10,right,gore_spot);
					ThrowBodyPart(self, &gore_spot, throw_nodes, damage, 0);
				}
			}
			else
			{
				self->s.fmnodeinfo[MESH__LARM_P1].flags |= FMNI_USE_SKIN;			
				self->s.fmnodeinfo[MESH__LARM_P1].skin = self->s.skinnum+1;
			}
			break;

		case hl_ArmUpperRight:
		case hl_ArmLowerRight://right arm
			if(self->s.fmnodeinfo[MESH__RARM_P1].flags & FMNI_NO_DRAW)
				break;
			if(flrand(0,self->health)<damage*0.75&&dismember_ok)
			{
				canthrownode_gk(self, MESH__RARM_P1, &throw_nodes);
				if(canthrownode_gk(self, MESH__RARM_P1, &throw_nodes))
				{
					AngleVectors(self->s.angles,NULL,right,NULL);
					gore_spot[2]+=self->maxs[2]*0.3;
					VectorMA(gore_spot,10,right,gore_spot);
					ThrowBodyPart(self, &gore_spot, throw_nodes, damage, 0);
				}
			}
			else
			{
				self->s.fmnodeinfo[MESH__RARM_P1].flags |= FMNI_USE_SKIN;			
				self->s.fmnodeinfo[MESH__RARM_P1].skin = self->s.skinnum+1;
			}
			break;

		case hl_LegUpperLeft:
		case hl_LegLowerLeft://left leg
			if(self->health>0)
			{
				if(self->s.fmnodeinfo[MESH__LTHIGH_P1].flags & FMNI_USE_SKIN)
					break;
				self->s.fmnodeinfo[MESH__LTHIGH_P1].flags |= FMNI_USE_SKIN;			
				self->s.fmnodeinfo[MESH__LTHIGH_P1].skin = self->s.skinnum+1;
				break;
			}
			else
			{
				if(self->s.fmnodeinfo[MESH__LTHIGH_P1].flags & FMNI_NO_DRAW)
					break;
				if(canthrownode_gk(self, MESH__LTHIGH_P1, &throw_nodes))
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
			{
				if(self->s.fmnodeinfo[MESH__RTHIGH_P1].flags & FMNI_USE_SKIN)
					break;
				self->s.fmnodeinfo[MESH__RTHIGH_P1].flags |= FMNI_USE_SKIN;
				self->s.fmnodeinfo[MESH__RTHIGH_P1].skin = self->s.skinnum+1;
			}
			else
			{
				if(self->s.fmnodeinfo[MESH__RTHIGH_P1].flags & FMNI_NO_DRAW)
					break;
				if(canthrownode_gk(self, MESH__RTHIGH_P1, &throw_nodes))
				{
					AngleVectors(self->s.angles,NULL,right,NULL);
					gore_spot[2]+=self->maxs[2]*0.3;
					VectorMA(gore_spot,-10,right,gore_spot);
					ThrowBodyPart(self, &gore_spot, throw_nodes, damage, 0);
				}
			}
			break;
		default:
			break;
	}

	if(self->s.fmnodeinfo[MESH__SPIKE_P1].flags&FMNI_NO_DRAW)			
	{
		self->monsterinfo.aiflags |= AI_COWARD;
		self->monsterinfo.aiflags |= AI_NO_MELEE;
		self->monsterinfo.aiflags |= AI_NO_MISSILE;
	}
}

void beetle_dead_pain (edict_t *self, G_Message_t *msg)
{
	if(self->health<=-80)
	{
		// We get gibbed.
		gi.sound(self,CHAN_BODY,sounds[SND_GIB],1,ATTN_NORM,0);
		
		self->think = BecomeDebris;
		self->nextthink = level.time + FRAMETIME;

		return;
	}

	if(msg)
		if(!(self->svflags & SVF_PARTS_GIBBED))
			DismemberMsgHandler(self, msg);
}

// ****************************************************************************
// GkrokonStaticsInit -
// ****************************************************************************

void GkrokonStaticsInit(void)
{
	static ClassResourceInfo_t res_info; //mxd. Made local static.

	classStatics[CID_GKROKON].msgReceivers[MSG_STAND]=beetle_stand;
	classStatics[CID_GKROKON].msgReceivers[MSG_RUN]=beetle_run;	
	classStatics[CID_GKROKON].msgReceivers[MSG_DEATH_PAIN] = beetle_dead_pain;
	classStatics[CID_GKROKON].msgReceivers[MSG_CHECK_MOOD] = beetle_check_mood;
	classStatics[CID_GKROKON].msgReceivers[MSG_FALLBACK] = GkrokonFallbackMsgHandler;
	classStatics[CID_GKROKON].msgReceivers[MSG_MELEE]=beetle_melee;
	classStatics[CID_GKROKON].msgReceivers[MSG_MISSILE]=beetle_missile;
	classStatics[CID_GKROKON].msgReceivers[MSG_PAIN]=beetle_pain;
	classStatics[CID_GKROKON].msgReceivers[MSG_EAT]=beetle_eat;
	classStatics[CID_GKROKON].msgReceivers[MSG_DEATH]=beetle_death;
	classStatics[CID_GKROKON].msgReceivers[MSG_DEATH_PAIN] = beetle_dead_pain;
	classStatics[CID_GKROKON].msgReceivers[MSG_DISMEMBER] = DismemberMsgHandler;

	res_info.numAnims=NUM_ANIMS;
	res_info.animations=animations;
	res_info.modelIndex=gi.modelindex("models/monsters/gkrokon/tris.fm");

	sounds[SND_PAIN1]=gi.soundindex("monsters/beetle/pain1.wav");
	sounds[SND_PAIN2]=gi.soundindex("monsters/beetle/pain2.wav");	
	sounds[SND_DIE]=gi.soundindex("monsters/beetle/death.wav");	
	sounds[SND_GIB]=gi.soundindex("monsters/insect/gib.wav");
	sounds[SND_SPOO]=gi.soundindex("monsters/beetle/spoo.wav");	
	sounds[SND_IDLE1]=gi.soundindex("monsters/beetle/idle1.wav");	
	sounds[SND_IDLE2]=gi.soundindex("monsters/beetle/idle2.wav");	
	sounds[SND_SIGHT]=gi.soundindex("monsters/beetle/sight.wav");	
	sounds[SND_WALK1]=gi.soundindex("monsters/beetle/walk1.wav");	
	sounds[SND_WALK2]=gi.soundindex("monsters/beetle/walk2.wav");	
	sounds[SND_FLEE]=gi.soundindex("monsters/beetle/flee.wav");	
	sounds[SND_ANGRY]=gi.soundindex("monsters/beetle/angry.wav");	
	sounds[SND_EATING]=gi.soundindex("monsters/beetle/eating.wav");	
	sounds[SND_BITEHIT1]=gi.soundindex("monsters/beetle/meleehit1.wav");	
	sounds[SND_BITEHIT2]=gi.soundindex("monsters/beetle/meleehit2.wav");	
	sounds[SND_BITEMISS1]=gi.soundindex("monsters/beetle/meleemiss1.wav");	
	sounds[SND_BITEMISS2]=gi.soundindex("monsters/beetle/meleemiss2.wav");	

	res_info.numSounds=NUM_SOUNDS;
	res_info.sounds=sounds;

	classStatics[CID_GKROKON].resInfo=&res_info;
}

// ****************************************************************************
// SP_Monster_Gkrokon - The Gkrokon's spawn function.
// ****************************************************************************

/*QUAKED monster_gkrokon (1 .5 0) (-20 -20 -0) (20 20 32) AMBUSH ASLEEP EATING 8 16 32 64 FIXED WANDER MELEE_LEAD STALK COWARD RESTING EXTRA2 EXTRA3 EXTRA4

AMBUSH - Will not be woken up by other monsters or shots from player

ASLEEP - will not appear until triggered

EATING - Chomp chomp... chewie chomp

WANDER - Monster will wander around aimlessly (but follows buoys)

MELEE_LEAD - Monster will try to cut you off when you're running and fighting him, works well if there are a few monsters in a group, half doing this, half not

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
mintel					= 12
melee_range				= 0
missile_range			= 256
min_missile_range		= 48
bypass_missile_chance	= 0
jump_chance				= 100
wakeup_distance			= 1024

NOTE: A value of zero will result in defaults, if you actually want zero as the value, use -1
*/

void SP_Monster_Gkrokon(edict_t *self)
{
	if (!M_WalkmonsterStart(self))		// Failed initialization
		return;

	self->msgHandler=DefaultMsgHandler;

	if (!self->health)
		self->health=GKROKON_HEALTH;

	self->max_health = self->health = MonsterHealth(self->health);

	self->materialtype = MAT_INSECT;
	self->mass=GKROKON_MASS;
	self->yaw_speed=20;
	self->movetype=PHYSICSTYPE_STEP;
	VectorClear(self->knockbackvel);
	self->solid=SOLID_BBOX;
	self->monsterinfo.dismember = beetle_dismember;

	self->s.renderfx |= RF_FRAMELERP;
	
	self->touch = M_Touch;

	VectorCopy(STDMinsForClass[self->classID], self->mins);
	VectorCopy(STDMaxsForClass[self->classID], self->maxs);	

	self->monsterinfo.attack_finished = level.time;

	self->s.modelindex=classStatics[CID_GKROKON].resInfo->modelIndex;

	self->s.skinnum = (irand(0, 1)) ? 0 : 2;

	self->monsterinfo.otherenemyname = "";

	self->timestamp = 0;

	self->monsterinfo.searchType = SEARCH_COMMON;
	
	MG_InitMoods(self);

	// Check our spawnflags to see what our initial behaviour should be.
	if(self->spawnflags & MSF_EATING)
	{
		self->monsterinfo.aiflags|=AI_EATING;
		QPostMessage(self,MSG_EAT,PRI_DIRECTIVE,NULL);
	}
	else
		QPostMessage(self,MSG_STAND,PRI_DIRECTIVE,NULL);

	self->count = 0;
}


void GkrokonPause(edict_t *self)
{
	if(self->spawnflags & MSF_FIXED && self->curAnimID == ANIM_DELAY && self->enemy)
	{
		self->monsterinfo.searchType = SEARCH_COMMON;
		MG_FaceGoal(self, true);
	}

	self->mood_think(self);

	switch (self->ai_mood)
	{
	case AI_MOOD_ATTACK:
		QPostMessage(self, MSG_MISSILE, PRI_DIRECTIVE, NULL);		
		break;
		
	case AI_MOOD_PURSUE:
	case AI_MOOD_NAVIGATE:
	case AI_MOOD_FLEE:
		QPostMessage(self, MSG_RUN, PRI_DIRECTIVE, NULL);
		break;

	case AI_MOOD_WALK:
	case AI_MOOD_WANDER:
		QPostMessage(self, MSG_WALK, PRI_DIRECTIVE, NULL);
		break;

	case AI_MOOD_BACKUP:
		QPostMessage(self, MSG_FALLBACK, PRI_DIRECTIVE, NULL);
		break;

	case AI_MOOD_STAND:
		QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
		break;

	case AI_MOOD_JUMP:
		if(self->spawnflags&MSF_FIXED || self->jump_chance<=0)
			SetAnim(self, ANIM_DELAY);
		else
			SetAnim(self, ANIM_FJUMP);
		break;

	case AI_MOOD_EAT:
		QPostMessage(self, MSG_EAT, PRI_DIRECTIVE, NULL);
		break;

	default :
#ifdef _DEVEL
		gi.dprintf("Unusable mood %d for Gkrokon\n", self->ai_mood);
#endif
		QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
		break;
	}
}

void beetle_to_stand (edict_t *self)
{
	SetAnim(self, ANIM_STAND3);
	GkrokonPause(self);
}

void beetle_to_crouch (edict_t *self)
{
	SetAnim(self, ANIM_CROUCH1);
	GkrokonPause(self);
}