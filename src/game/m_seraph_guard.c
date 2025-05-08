//
// m_seraph_guard.c
//
// Copyright 1998 Raven Software
//

#include "m_seraph_guard.h"
#include "m_seraph_guard_shared.h"
#include "m_seraph_guard_anim.h"
#include "g_debris.h" //mxd
#include "g_DefaultMessageHandler.h"
#include "g_playstats.h"
#include "mg_ai.h" //mxd
#include "mg_guide.h" //mxd
#include "m_stats.h"
#include "p_anims.h"
#include "Random.h"
#include "Utilities.h"
#include "Vector.h"
#include "g_monster.h"

static void SeraphGuardProjectileBlocked(edict_t* self, trace_t* trace); //TODO: remove.
static void seraph_guard_dropweapon(edict_t* self); //TODO: remove.

#define SGUARD_NUM_PREDICTED_FRAMES	5.0f //mxd. Named 'NUM_PREDFRAMES' in original logic.
#define SF_SGUARD_GOLEM				4 //mxd. Named 'SERAPH_FLAG_GOLEM' in original logic.

#pragma region ========================== Seraph Guard Base Info ==========================

static const animmove_t* animations[NUM_ANIMS] =
{
	&seraph_guard_move_stand,
	&seraph_guard_move_run,
	&seraph_guard_move_fjump,
	&seraph_guard_move_runmelee,
	&seraph_guard_move_walk,
	&seraph_guard_move_pain,
	&seraph_guard_move_melee,
	&seraph_guard_move_melee2,
	&seraph_guard_move_melee3,
	&seraph_guard_move_death1,
	&seraph_guard_move_death2_go,
	&seraph_guard_move_death2_loop,
	&seraph_guard_move_death2_end,
	&seraph_guard_move_backup,
	&seraph_guard_move_missile,
	&seraph_guard_move_delay,
};

static int sounds[NUM_SOUNDS];

#pragma endregion

// Create the guts of seraph guard's projectile.
static void SeraphGuardProjectileInit(const edict_t* self, edict_t* proj) //mxd. Named 'create_guard_proj' in original logic.
{
	proj->svflags |= SVF_ALWAYS_SEND;
	proj->movetype = PHYSICSTYPE_FLY;
	proj->gravity = 0.0f;
	proj->solid = SOLID_BBOX;
	proj->classname = "Guard_Missile";
	proj->dmg = irand(SGUARD_DMG_SPELL_MIN, SGUARD_DMG_SPELL_MAX); //mxd. flrand() in original logic.
	proj->s.scale = 1.0f;
	proj->clipmask = MASK_SHOT;
	proj->nextthink = level.time + FRAMETIME; //mxd. Use define.

	proj->isBlocked = SeraphGuardProjectileBlocked;
	proj->isBlocking = SeraphGuardProjectileBlocked;
	proj->bounced = SeraphGuardProjectileBlocked;

	proj->s.effects = (EF_MARCUS_FLAG1 | EF_CAMERA_NO_CLIP);
	proj->enemy = self->enemy;

	VectorSet(proj->mins, -4.0f, -4.0f, -4.0f);
	VectorSet(proj->maxs,  4.0f,  4.0f,  4.0f);
	VectorCopy(self->s.origin, proj->s.origin);
}

static void SeraphGuardProjectileBlocked(edict_t* self, trace_t* trace) //mxd. Named 'guard_beam_blocked' in original logic.
{
	//TODO: re-implement projectile reflecting logic?

	if (trace->ent->takedamage != DAMAGE_NO)
	{
		vec3_t hit_dir;
		VectorNormalize2(self->velocity, hit_dir);

		T_Damage(trace->ent, self, self->owner, hit_dir, self->s.origin, trace->plane.normal, self->dmg, 0, DAMAGE_SPELL | DAMAGE_NO_KNOCKBACK, MOD_DIED);
	}

	gi.sound(self, CHAN_WEAPON, sounds[SND_MISSHIT], 1.0f, ATTN_NORM, 0.0f); //mxd. Use precached sound index.
	gi.CreateEffect(&self->s, FX_M_EFFECTS, CEF_OWNERS_ORIGIN, self->s.origin, "bv", FX_M_MISC_EXPLODE, vec3_origin);

	G_SetToFree(self);
}

static void SeraphGuardProjectileThink(edict_t* self) //mxd. Named 'guard_beam_think' in original logic. //TODO: is this needed?..
{
	self->think = NULL;
	self->nextthink = -1.0f;
}

static void SeraphGuardProjectile(edict_t* self) //mxd. Named 'guard_beam' in original logic.
{
	// Spawn the projectile.
	edict_t* proj = G_Spawn();

	SeraphGuardProjectileInit(self, proj);

	proj->reflect_debounce_time = MAX_REFLECT;
	proj->owner = self;

	vec3_t forward;
	vec3_t right;
	AngleVectors(self->s.angles, forward, right, NULL);

	VectorMA(proj->s.origin, 24.0f, forward, proj->s.origin);
	VectorMA(proj->s.origin, 16.0f, right, proj->s.origin);
	proj->s.origin[2] += 10.0f;

	vec3_t dir;

	if (MG_IsAheadOf(self, self->enemy))
	{
		VectorSubtract(self->enemy->s.origin, self->s.origin, dir);
		VectorNormalize(dir);
	}
	else
	{
		VectorCopy(forward, dir);
	}

	VectorScale(dir, 500.0f, proj->velocity);
	vectoangles(proj->velocity, proj->s.angles);

	proj->think = SeraphGuardProjectileThink;
	proj->nextthink = level.time + 1.0f;

	gi.sound(self, CHAN_WEAPON, sounds[SND_MISSILE], 1.0f, ATTN_NORM, 0.0f); //mxd. Use precached sound index.
	gi.CreateEffect(&proj->s, FX_M_EFFECTS, CEF_OWNERS_ORIGIN, vec3_origin, "bv", FX_M_BEAM, proj->s.angles);
	gi.linkentity(proj);
}

void seraph_guard_checkpoke(edict_t* self) //TODO: rename to seraph_guard_check_poke.
{
	// Really, this is a given, but it could fail...
	if (!M_ValidTarget(self, self->enemy))
	{
		QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
		return;
	}

	if (M_DistanceToTarget(self, self->enemy) >= 120.0f)
		return;

	// Set this for any uses below.
	vec3_t forward;
	AngleVectors(self->s.angles, forward, NULL, NULL);

	vec3_t attack_vel;
	VectorScale(forward, 2.0f, attack_vel);

	if (M_PredictTargetEvasion(self, self->enemy, attack_vel, self->enemy->velocity, 150.0f, SGUARD_NUM_PREDICTED_FRAMES))
	{
		const int chance = irand(0, 100);

		if (chance < 40)
			SetAnim(self, ANIM_MELEE3);
		else if (chance < 60)
			SetAnim(self, ANIM_MELEE2);
	}
	else
	{
		SetAnim(self, ((irand(0, 1) == 1) ? ANIM_RUN_MELEE : ANIM_MELEE2));
	}
}

void seraph_guard_death_loop(edict_t* self)
{
	SetAnim(self, ANIM_DEATH2_LOOP);
}

void seraph_guard_check_land(edict_t* self)
{
	M_ChangeYaw(self);

	vec3_t end_pos;
	VectorCopy(self->s.origin, end_pos);
	end_pos[2] -= 48.0f;

	trace_t trace;
	gi.trace(self->s.origin, self->mins, self->maxs, end_pos, self, MASK_MONSTERSOLID, &trace);

	if ((trace.fraction < 1.0f || trace.startsolid || trace.allsolid) && self->curAnimID != ANIM_DEATH2_END && self->curAnimID != ANIM_DEATH2_GO)
	{
		self->elasticity = 1.25f;
		self->friction = 0.5f;
		SetAnim(self, ANIM_DEATH2_END);
	}
}

void seraph_guard_dead(edict_t* self)
{
	M_EndDeath(self);
}

void seraph_guard_strike(edict_t* self, float damage, float var2, float var3)
{
	if (self->monsterinfo.aiflags & AI_NO_MELEE)
		return;

	damage *= self->s.scale;
	self->monsterinfo.attack_finished = level.time + (3.0f - skill->value) * 2.0f + flrand(0.0f, 1.0f);

	qboolean knockback = true;
	vec3_t start_offset;
	vec3_t end_offset;

	switch (self->curAnimID)
	{
		case ANIM_MELEE2:
			VectorSet(start_offset, 16.0f, -16.0f, 24.0f);
			VectorSet(end_offset, 124.0f, -16.0f, 16.0f);
			break;

		case ANIM_MELEE3:
			VectorSet(start_offset, 32.0f, -48.0f, 34.0f);
			VectorSet(end_offset, 64.0f, 64.0f, -8.0f);
			break;

		default:
			knockback = false;
			VectorSet(start_offset, 32.0f, -16.0f, 64.0f);
			VectorSet(end_offset, 72.0f, 16.0f, -8.0f);
			break;
	}

	Vec3ScaleAssign(self->s.scale, start_offset);
	Vec3ScaleAssign(self->s.scale, end_offset);

	const vec3_t mins = { -4.0f, -4.0f, -4.0f };
	const vec3_t maxs = {  4.0f,  4.0f,  4.0f };

	trace_t trace;
	vec3_t direction;
	edict_t* victim = M_CheckMeleeLineHit(self, start_offset, end_offset, mins, maxs, &trace, direction);

	if (victim == NULL) // Missed.
	{
		// Play swoosh sound.
		gi.sound(self, CHAN_WEAPON, sounds[SND_ATTACK_MISS], 1.0f, ATTN_NORM, 0.0f);
		return;
	}

	if (victim == self) // Hit wall.
	{
		// Create a spark effect.
		gi.CreateEffect(NULL, FX_SPARKS, CEF_FLAG6, trace.endpos, "d", direction);
		gi.sound(self, CHAN_WEAPON, sounds[SND_HIT_WALL], 1, ATTN_NORM, 0);

		return;
	}

	// Hurt whatever we were whacking away at.
	if (self->curAnimID == ANIM_MELEE3)
	{
		gi.CreateEffect(NULL, FX_WEAPON_STAFF_STRIKE, 0, trace.endpos, "db", trace.plane.normal, 2);
		gi.sound(self, CHAN_WEAPON, gi.soundindex("weapons/staffhit_2.wav"), 1.0f, ATTN_NORM, 0.0f);
	}
	else
	{
		gi.sound(self, CHAN_WEAPON, sounds[SND_ATTACK], 1.0f, ATTN_NORM, 0.0f);
	}

	damage *= (skill->value + 1.0f) / 3.0f * flrand(0.85f, 1.15f); // Add some variance to the hit, since it passes a constant (skill 0 = 1/3, skill 3 = 1 1/3).

	vec3_t blood_dir;
	VectorSubtract(end_offset, start_offset, blood_dir);
	VectorNormalize(blood_dir);

	if (knockback)
		T_Damage(victim, self, self, direction, trace.endpos, blood_dir, (int)damage, (int)damage * 20, DAMAGE_EXTRA_KNOCKBACK | DAMAGE_DISMEMBER | DAMAGE_DOUBLE_DISMEMBER | DAMAGE_EXTRA_BLOOD, MOD_DIED);
	else
		T_Damage(victim, self, self, direction, trace.endpos, blood_dir, (int)damage, 0, DAMAGE_NO_KNOCKBACK | DAMAGE_DISMEMBER | DAMAGE_DOUBLE_DISMEMBER | DAMAGE_EXTRA_BLOOD, MOD_DIED);

	// Knockdown player?
	if (self->curAnimID == ANIM_MELEE3 && victim->client != NULL && victim->health > 0)
		if (victim->client->playerinfo.lowerseq != ASEQ_KNOCKDOWN && AI_IsInfrontOf(self, victim))
			P_KnockDownPlayer(&victim->client->playerinfo);
}

void seraphGuardApplyJump(edict_t* self) //TODO: rename to seraph_guard_jump.
{
	self->jump_time = level.time + 0.75f;
	VectorCopy(self->movedir, self->velocity);
	VectorNormalize(self->movedir);
}

void seraph_guard_pause( edict_t *self )
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
		if(self->ai_mood_flags & AI_MOOD_FLAG_MISSILE)
			QPostMessage(self, MSG_MISSILE, PRI_DIRECTIVE, NULL);
		else
			QPostMessage(self, MSG_MELEE, PRI_DIRECTIVE, NULL);
		break;
	case AI_MOOD_PURSUE:
	case AI_MOOD_NAVIGATE:
	case AI_MOOD_WALK:
	case AI_MOOD_FLEE:
		QPostMessage(self, MSG_RUN, PRI_DIRECTIVE, NULL);
		break;
	case AI_MOOD_STAND:
		if (!self->enemy)
			QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
		break;

	case AI_MOOD_DELAY:
		SetAnim(self, ANIM_DELAY);
		break;

	case AI_MOOD_WANDER:
		SetAnim(self, ANIM_WALK);
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

void seraph_guard_check_mood (edict_t *self, G_Message_t *msg)
{
	ParseMsgParms(msg, "i", &self->ai_mood);

	seraph_guard_pause(self);
}

/*
==========================================================

	Seraph Guard Message functions

==========================================================
*/

/*--------------------------------------
		void seraph_guard_pain
----------------------------------------*/

void seraph_guard_pain(edict_t *self, G_Message_t *msg)
{
	int				temp, damage;
	int				force_damage, soundID;
	
	ParseMsgParms(msg, "eeiii", &temp, &temp, &force_damage, &damage, &temp);
	//Weighted random based on health compared to the maximum it was at
	if (force_damage || ((irand(0, self->max_health+50) > self->health) && irand(0,2)))
	{
		soundID = irand(SND_PAIN1, SND_PAIN4);
		gi.sound (self, CHAN_WEAPON, sounds[soundID], 1, ATTN_NORM, 0);

		SetAnim(self, ANIM_PAIN);
	}
}

/*--------------------------------------
		void seraph_guard_stand
----------------------------------------*/

void seraph_guard_stand(edict_t *self, G_Message_t *msg)
{
	SetAnim(self, ANIM_STAND);
}

/*--------------------------------------
		void seraph_guard_melee
----------------------------------------*/

void seraph_guard_melee(edict_t *self, G_Message_t *msg)
{
	vec3_t	attackVel, vf;
	float	dist;
	int		ret;

	//Don't interrupt a current animation with another melee call inside of it
	if (self->curAnimID == ANIM_MELEE1 || self->curAnimID == ANIM_MELEE2)
		return;

	if (M_ValidTarget(self, self->enemy))
	{
		if(self->ai_mood == AI_MOOD_FLEE)
		{
			SetAnim(self, ANIM_BACKUP);
			return;
		}
		//Set this for any uses below
		AngleVectors(self->s.angles, vf, NULL, NULL);

		dist = M_DistanceToTarget(self, self->enemy);	

		if (dist < 120)
		{
			if(self->s.fmnodeinfo[MESH__AXE].flags & FMNI_NO_DRAW)
			{
				SetAnim(self, ANIM_MISSILE);
				return;
			}

			VectorMA(vf, 0, vf, attackVel);
			ret  = M_PredictTargetEvasion( self, self->enemy, attackVel, self->enemy->velocity, self->melee_range, SGUARD_NUM_PREDICTED_FRAMES );

						
			if (ret)
			{
				if(dist < 88 && !irand(0, 3))
					SetAnim(self, ANIM_MISSILE);//punch
				else if(irand(0, 4))
				{
					if(irand(0, 10))
						SetAnim(self, ANIM_MELEE1);	
					else
						SetAnim(self, ANIM_MISSILE);
				}
				else
					SetAnim(self, ANIM_MELEE2);	
			}
			else
				SetAnim(self, ANIM_RUN_MELEE);
		}
		else
			SetAnim(self, ANIM_RUN);

		return;
	}

	//If our enemy is dead, we need to stand
	QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
}

void morcalavin_beam( edict_t *self);
void seraph_guard_fire (edict_t *self)
{
	if(!self->enemy)
		return;

	if(M_DistanceToTarget(self, self->enemy) < self->min_missile_range)
	{//punch
		trace_t	trace;
		edict_t *victim;
		vec3_t	soff, eoff, mins, maxs, bloodDir, direction;
		float	damage;

		damage = 15 * self->s.scale;

		VectorSet(soff, 12, -18, 24);
		VectorSet(eoff, 88, -4, -16);

		Vec3ScaleAssign(self->s.scale, soff);
		Vec3ScaleAssign(self->s.scale, eoff);
		
		VectorSet(mins, -4, -4, -4);
		VectorSet(maxs,  4,  4,  4);

		VectorSubtract(eoff, soff, bloodDir);
		VectorNormalize(bloodDir);

		victim = M_CheckMeleeLineHit(self, soff, eoff, mins, maxs, &trace, direction);	

		//Did something get hit?
		if (victim)
		{
			if (victim == self)
			{
				gi.sound (self, CHAN_WEAPON, sounds[SND_FIST_HIT_WALL], 1, ATTN_NORM, 0);
			}
			else
			{
				//Hurt whatever we were whacking away at
				damage *= (skill->value + 1)/3;//skill 0 = 1/3, skill 3 = 1 1/3
				T_Damage(victim, self, self, direction, trace.endpos, bloodDir, 
						damage, damage*20, DAMAGE_EXTRA_KNOCKBACK, MOD_DIED);
				gi.sound (self, CHAN_WEAPON, sounds[SND_HIT_WALL], 1, ATTN_NORM, 0);
				if(victim->client)
				{
					if(victim->health > 0)
					{
						if(victim->client->playerinfo.lowerseq != ASEQ_KNOCKDOWN && AI_IsInfrontOf(self, victim))
						{
							P_KnockDownPlayer(&victim->client->playerinfo);
						}
					}
				}
			}
		}
		else
		{
			//Play swoosh sound
			gi.sound (self, CHAN_WEAPON, sounds[SND_ATTACK_MISS], 1, ATTN_NORM, 0);
		}
	}
	else
		SeraphGuardProjectile (self);
}

void seraph_guard_missile(edict_t *self, G_Message_t *msg)
{
	vec3_t	attackVel, vf;
	float	dist;
	int		ret;

	if (M_ValidTarget(self, self->enemy))
	{
		if(self->ai_mood == AI_MOOD_FLEE)
		{
			SetAnim(self, ANIM_BACKUP);
			return;
		}
		//Set this for any uses below
		AngleVectors(self->s.angles, vf, NULL, NULL);

		dist = M_DistanceToTarget(self, self->enemy);	

		if (dist < self->min_missile_range)
		{
			VectorMA(vf, 0, vf, attackVel);
			ret  = M_PredictTargetEvasion( self, self->enemy, attackVel, self->enemy->velocity, self->melee_range, SGUARD_NUM_PREDICTED_FRAMES );

			if(irand(0, 4))
			{
				if(irand(0, 10))
					SetAnim(self, ANIM_MELEE1);	
				else
					SetAnim(self, ANIM_MISSILE);
			}
			else
				SetAnim(self, ANIM_MELEE2);	
		}
		else if(MG_IsAheadOf(self, self->enemy))
			SetAnim(self, ANIM_MISSILE);

		return;
	}

	//If our enemy is dead, we need to stand
	QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
}

/*--------------------------------------
		void seraph_guard_death
----------------------------------------*/

void seraph_guard_death_pain(edict_t *self, G_Message_t *msg)
{
	if (self->health < -80)
	{
		BecomeDebris(self);
		return;
	}
}

void seraph_guard_death(edict_t *self, G_Message_t *msg)
{
	edict_t	*targ, *inflictor, *attacker;
	vec3_t	dVel, vf, yf;
	float	damage;
	int		soundID;

	ParseMsgParms(msg, "eeei", &targ, &inflictor, &attacker, &damage);

	M_StartDeath(self, ANIM_DEATH1);

	if (self->health < -80)
	{
		return;
	}
	else if (self->health < -10)
	{
		seraph_guard_dropweapon(self);

		SetAnim(self, ANIM_DEATH2_GO);

		VectorCopy(targ->velocity, vf);
		VectorNormalize(vf);

		VectorScale(vf, -1, yf);

		self->ideal_yaw = VectorYaw( yf );
		self->yaw_speed = 24;

		VectorScale(vf, 300, dVel);
		dVel[2] = irand(150,200);

		VectorCopy(dVel, self->velocity);
//		self->groundentity = NULL;
	}
	else
	{
		SetAnim(self, ANIM_DEATH1);
	}

	soundID = irand(SND_DEATH1, SND_DEATH4);
	gi.sound (self, CHAN_BODY, sounds[soundID], 1, ATTN_NORM, 0);
}

/*--------------------------------------
		void seraph_guard_run
----------------------------------------*/

void seraph_guard_run(edict_t *self, G_Message_t *msg)
{
	trace_t	trace;
	vec3_t	attackVel, vf;
	float	dist;
	int		ret;

	if (M_ValidTarget(self, self->enemy))
	{
		if(self->ai_mood == AI_MOOD_FLEE)
		{
			SetAnim(self, ANIM_RUN);
			return;
		}

		if(self->monsterinfo.attack_finished > level.time || self->monsterinfo.aiflags & AI_NO_MELEE)
		{
			SetAnim(self, ANIM_RUN);
			return;
		}

		//Set this for any uses below
		AngleVectors(self->s.angles, vf, NULL, NULL);
		
		dist = M_DistanceToTarget(self, self->enemy);	

		if (dist < 100)
		{
			VectorMA(vf, 0, vf, attackVel);
			ret  = M_PredictTargetEvasion( self, self->enemy, attackVel, self->enemy->velocity, self->melee_range, SGUARD_NUM_PREDICTED_FRAMES );

			//See what the predicted outcome is
			if (ret && (M_CheckMeleeHit( self, self->melee_range, &trace) == self->enemy))
				SetAnim(self, ANIM_MELEE1);		
			else
				SetAnim(self, ANIM_RUN_MELEE);
		}
		else if (dist < 200)
		{
			VectorMA(vf, 150, vf, attackVel);
			ret  = M_PredictTargetEvasion( self, self->enemy, attackVel, self->enemy->velocity, 150, SGUARD_NUM_PREDICTED_FRAMES );

			//See what the predicted outcome is
			if (ret && (M_CheckMeleeHit( self, 150, &trace) == self->enemy))
				SetAnim(self, ANIM_RUN_MELEE);
			else
				SetAnim(self, ANIM_RUN);
		}
		else
		{
			SetAnim(self, ANIM_RUN);
		}

		return;
	}

	//If our enemy is dead, we need to stand
	QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
}


int Bit_for_MeshNode_sg [NUM_MESH_NODES] =
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

void seraph_guard_back (edict_t *self, float dist)
{
	if(!MG_TryWalkMove(self, self->s.angles[YAW] + 180, dist, true))
	{
		if(!irand(0, 1000))
		{
			self->monsterinfo.aiflags |= AI_COWARD;
			SetAnim(self, ANIM_RUN);
		}
	}
}

qboolean canthrownode_sg (edict_t *self, int BP, int *throw_nodes)
{//see if it's on, if so, add it to throw_nodes
	//turn it off on thrower
	if(!(self->s.fmnodeinfo[BP].flags & FMNI_NO_DRAW))
	{
		*throw_nodes |= Bit_for_MeshNode_sg[BP];
		self->s.fmnodeinfo[BP].flags |= FMNI_NO_DRAW;
		return true;
	}
	return false;
}

//THROWS weapon, turns off those nodes, sets that weapon as gone
static void seraph_guard_dropweapon (edict_t *self)
{//NO PART FLY FRAME!
	vec3_t handspot, forward, right, up;

	if(!(self->s.fmnodeinfo[MESH__AXE].flags & FMNI_NO_DRAW))
	{
		VectorClear(handspot);
		AngleVectors(self->s.angles,forward,right,up);

		VectorMA(handspot,8,forward,handspot);
		VectorMA(handspot,5,right,handspot);
		VectorMA(handspot,12,up,handspot);
		ThrowWeapon(self, &handspot, BIT_AXE, 0, FRAME_partfly);
		self->s.fmnodeinfo[MESH__AXE].flags |= FMNI_NO_DRAW;
		return;
	}
}

void seraph_guard_dismember(edict_t *self, int damage, int HitLocation)
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


	if(HitLocation == hl_TorsoFront && dismember_ok)
	{//melee swing at chest during my melee swing
		if(self->curAnimID == ANIM_MELEE1 ||
			self->curAnimID == ANIM_MELEE2 ||
			self->curAnimID == ANIM_MELEE3||
			self->curAnimID == ANIM_RUN_MELEE)
		{
			if(!irand(0, 2))
				HitLocation = hl_ArmLowerLeft;
			else
				HitLocation = hl_ArmLowerRight;
		}
	}

	VectorClear(gore_spot);
	switch(HitLocation)
	{
		case hl_Head:
			if(self->s.fmnodeinfo[MESH__GUARDHEAD].flags & FMNI_NO_DRAW)
				break;
			if(self->s.fmnodeinfo[MESH__GUARDHEAD].flags & FMNI_USE_SKIN)
				damage*=1.5;//greater chance to cut off if previously damaged
			if(flrand(0,self->health)<damage*0.3&&dismember_ok)
			{
				seraph_guard_dropweapon(self);
				canthrownode_sg(self, MESH__GUARDHEAD,&throw_nodes);

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
				self->s.fmnodeinfo[MESH__GUARDHEAD].flags |= FMNI_USE_SKIN;
				self->s.fmnodeinfo[MESH__GUARDHEAD].skin = self->s.skinnum+1;
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
			if(flrand(0,self->health)<damage*0.75&&dismember_ok)
			{
				if(canthrownode_sg(self, MESH__LHANDGRD, &throw_nodes))
				{
					canthrownode_sg(self, MESH__ARMSPIKES, &throw_nodes);
					AngleVectors(self->s.angles,NULL,right,NULL);
					gore_spot[2]+=self->maxs[2]*0.3;
					VectorMA(gore_spot,-10,right,gore_spot);
					seraph_guard_dropweapon (self);
					ThrowBodyPart(self, &gore_spot, throw_nodes, damage, FRAME_partfly);
				}
			}
			break;
		case hl_ArmLowerLeft://left arm
			if(self->s.fmnodeinfo[MESH__LHANDGRD].flags & FMNI_NO_DRAW)
				break;
			if(self->s.fmnodeinfo[MESH__LHANDGRD].flags & FMNI_USE_SKIN)
				damage*=1.5;//greater chance to cut off if previously damaged
			if(flrand(0,self->health)<damage&&dismember_ok)
			{
				if(canthrownode_sg(self, MESH__LHANDGRD, &throw_nodes))
				{
					canthrownode_sg(self, MESH__ARMSPIKES, &throw_nodes);
					AngleVectors(self->s.angles,NULL,right,NULL);
					gore_spot[2]+=self->maxs[2]*0.3;
					VectorMA(gore_spot,-10,right,gore_spot);
					seraph_guard_dropweapon (self);
					ThrowBodyPart(self, &gore_spot, throw_nodes, damage, FRAME_partfly);
				}
			}
			else
			{
				self->s.fmnodeinfo[MESH__LHANDGRD].flags |= FMNI_USE_SKIN;			
				self->s.fmnodeinfo[MESH__LHANDGRD].skin = self->s.skinnum+1;
			}
			break;
		case hl_ArmUpperRight:
			if(flrand(0,self->health)<damage)
			{
				self->s.fmnodeinfo[MESH__RTARM].flags |= FMNI_USE_SKIN;			
				self->s.fmnodeinfo[MESH__RTARM].skin = self->s.skinnum+1;
			}
			if(flrand(0,self->health)<damage*0.75&&dismember_ok)
			{
				if(canthrownode_sg(self, MESH__RHAND, &throw_nodes))
				{
					AngleVectors(self->s.angles,NULL,right,NULL);
					gore_spot[2]+=self->maxs[2]*0.3;
					VectorMA(gore_spot,10,right,gore_spot);
					seraph_guard_dropweapon (self);
					ThrowBodyPart(self, &gore_spot, throw_nodes, damage, FRAME_partfly);
				}
			}
			break;
		case hl_ArmLowerRight://right arm
			if(self->s.fmnodeinfo[MESH__RHAND].flags & FMNI_NO_DRAW)
				break;
			if(flrand(0,self->health)<damage&&dismember_ok)
			{
				if(canthrownode_sg(self, MESH__RHAND, &throw_nodes))
				{
					AngleVectors(self->s.angles,NULL,right,NULL);
					gore_spot[2]+=self->maxs[2]*0.3;
					VectorMA(gore_spot,10,right,gore_spot);
					seraph_guard_dropweapon (self);
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

//FIXME: when get missile anim
	if(self->s.fmnodeinfo[MESH__RHAND].flags & FMNI_NO_DRAW)
	{
		self->monsterinfo.aiflags |= AI_NO_MISSILE;
		
		if(self->s.fmnodeinfo[MESH__AXE].flags & FMNI_NO_DRAW)
			self->monsterinfo.aiflags |= AI_NO_MELEE;
	}

	if(self->monsterinfo.aiflags & AI_NO_MELEE && self->monsterinfo.aiflags & AI_NO_MISSILE)
	{
		self->monsterinfo.aiflags |= AI_COWARD;
		SetAnim(self, ANIM_BACKUP);
	}
}

void ser_grd_SightSound(edict_t *self, G_Message_t *Msg)
{
	gi.sound(self, CHAN_VOICE, sounds[irand(SND_SIGHT1, SND_SIGHT4)], 1, ATTN_NORM, 0);
}

/*
==========================================================
	
	Seraph Guard Spawn functions

==========================================================
*/

void SeraphGuardStaticsInit(void)
{
	static ClassResourceInfo_t resInfo;

	classStatics[CID_SERAPH_GUARD].msgReceivers[MSG_STAND]	= seraph_guard_stand;
	classStatics[CID_SERAPH_GUARD].msgReceivers[MSG_RUN]	= seraph_guard_run;
	classStatics[CID_SERAPH_GUARD].msgReceivers[MSG_MELEE]	= seraph_guard_melee;
	classStatics[CID_SERAPH_GUARD].msgReceivers[MSG_MISSILE] = seraph_guard_missile;
	classStatics[CID_SERAPH_GUARD].msgReceivers[MSG_PAIN]	= seraph_guard_pain;
	classStatics[CID_SERAPH_GUARD].msgReceivers[MSG_DEATH]	= seraph_guard_death;
	classStatics[CID_SERAPH_GUARD].msgReceivers[MSG_DEATH_PAIN]	= seraph_guard_death_pain;
	classStatics[CID_SERAPH_GUARD].msgReceivers[MSG_CHECK_MOOD] = seraph_guard_check_mood;
	classStatics[CID_SERAPH_GUARD].msgReceivers[MSG_DISMEMBER] = DismemberMsgHandler;
	classStatics[CID_SERAPH_GUARD].msgReceivers[MSG_VOICE_SIGHT] = ser_grd_SightSound;

	resInfo.numAnims = NUM_ANIMS;
	resInfo.animations = animations;
	resInfo.modelIndex = gi.modelindex("models/monsters/guard/tris.fm");
	resInfo.numSounds = NUM_SOUNDS;
	resInfo.sounds = sounds;

	sounds[SND_ATTACK]		=	gi.soundindex("monsters/seraph/guard/attack.wav");	
	sounds[SND_ATTACK_MISS]	=	gi.soundindex("monsters/seraph/guard/attack_miss.wav");	
	sounds[SND_HIT_WALL]	=	gi.soundindex("weapons/staffhitwall.wav");	
	sounds[SND_MISSILE]		=	gi.soundindex("monsters/seraph/guard/spell.wav");	
	sounds[SND_MISSHIT]		=	gi.soundindex("monsters/seraph/guard/spellhit.wav");
	sounds[SND_FIST_HIT_WALL]	=	gi.soundindex("objects/bam1.wav");	

	sounds[SND_DEATH1]	=	gi.soundindex("monsters/seraph/death1.wav");	
	sounds[SND_DEATH2]	=	gi.soundindex("monsters/seraph/death2.wav");	
	sounds[SND_DEATH3]	=	gi.soundindex("monsters/seraph/wimpdeath1.wav");	
	sounds[SND_DEATH4]	=	gi.soundindex("monsters/seraph/wimpdeath2.wav");	
	
	sounds[SND_PAIN1]	=	gi.soundindex("monsters/seraph/pain1.wav");	
	sounds[SND_PAIN2]	=	gi.soundindex("monsters/seraph/pain2.wav");	
	sounds[SND_PAIN3]	=	gi.soundindex("monsters/seraph/pain3.wav");	
	sounds[SND_PAIN4]	=	gi.soundindex("monsters/seraph/pain4.wav");	

	sounds[SND_SIGHT1]	=	gi.soundindex("monsters/seraph/guard/sight1.wav");	
	sounds[SND_SIGHT2]	=	gi.soundindex("monsters/seraph/guard/sight2.wav");	
	sounds[SND_SIGHT3]	=	gi.soundindex("monsters/seraph/guard/sight3.wav");	
	sounds[SND_SIGHT4]	=	gi.soundindex("monsters/seraph/guard/sight4.wav");	

	classStatics[CID_SERAPH_GUARD].resInfo = &resInfo;
}

void golem_awaken (edict_t *self, edict_t *other, edict_t *activator)
{
	if (!M_WalkmonsterStart(self)) //mxd. M_Start -> M_WalkmonsterStart.
		return; // Initialization failed.

	self->msgHandler = DefaultMsgHandler;
	self->ai_mood_flags |= AI_MOOD_FLAG_PREDICT;
	self->monsterinfo.dismember = seraph_guard_dismember;
	MG_InitMoods(self);
	QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
	self->melee_range *= self->s.scale;

	self->enemy = activator;
	AI_FoundTarget(self, false);
}


/*QUAKED monster_seraph_guard(1 .5 0) (-24 -24 -34) (24 24 34) AMBUSH ASLEEP GOLEM 8 16 32 64 FIXED WANDER MELEE_LEAD STALK COWARD EXTRA1 EXTRA2 EXTRA3 EXTRA4
The big, ugly, brutal Guard..

AMBUSH - Will not be woken up by other monsters or shots from player

ASLEEP - will not appear until triggered

WALKING- Use WANDER instead

WANDER - Monster will wander around aimlessly (but follows buoys)

MELEE_LEAD - Monster will tryto cut you off when you're running and fighting him, works well if there are a few monsters in a group, half doing this, half not

STALK - Monster will only approach and attack from behind- if you're facing the monster it will just stand there.  Once the monster takes pain, however, it will stop this behaviour and attack normally

COWARD - Monster starts off in flee mode- runs away from you when woken up

"homebuoy" - monsters will head to this buoy if they don't have an enemy ("homebuoy" should be targetname of the buoy you want them to go to)

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
mintel					= 20
melee_range				= 100
missile_range			= 1024
min_missile_range		= 64
bypass_missile_chance	= 30
jump_chance				= 20
wakeup_distance			= 1024

NOTE: A value of zero will result in defaults, if you actually want zero as the value, use -1
*/
void SP_monster_seraph_guard(edict_t *self)
{
	if (self->spawnflags & SF_SGUARD_GOLEM)
	{
		self->clipmask = MASK_MONSTERSOLID;

		if (!self->health)
			self->health = SGUARD_HEALTH * 2;

		self->max_health = self->health = MonsterHealth(self->health);

		self->mass = SGUARD_MASS  * 10;
		self->yaw_speed = 12;

		self->movetype = PHYSICSTYPE_STEP;
		self->solid=SOLID_BBOX;

		if (!self->s.scale)
		{
			self->s.scale = self->monsterinfo.scale = 2.5;
		}

		self->s.skinnum = 3;

		VectorCopy(STDMinsForClass[self->classID], self->mins);
		VectorCopy(STDMaxsForClass[self->classID], self->maxs);	

		self->materialtype = MAT_STONE;

		self->s.modelindex = classStatics[CID_SERAPH_GUARD].resInfo->modelIndex;
		self->s.skinnum = 0;

		self->monsterinfo.otherenemyname = "monster_rat";	

		//Turn off the Overlord pieces
		self->s.fmnodeinfo[MESH__WHIP].flags |= FMNI_NO_DRAW;
		self->s.fmnodeinfo[MESH__LHANDBOSS].flags |= FMNI_NO_DRAW;
		self->s.fmnodeinfo[MESH__PITHEAD].flags |= FMNI_NO_DRAW;

		self->s.frame = FRAME_idle;

		//enable this and use guards to make them wake up on command
		//self->use = golem_awaken;
	}
	else
	{
		if (!M_WalkmonsterStart(self)) //mxd. M_Start -> M_WalkmonsterStart.
			return; // Failed initialization

		self->msgHandler = DefaultMsgHandler;

		if (!self->health)
		{
			self->health = SGUARD_HEALTH;
		}

		//Apply to the end result (whether designer set or not)
		self->max_health = self->health = MonsterHealth(self->health);

		self->mass = SGUARD_MASS;
		self->yaw_speed = 24;

		if(irand(0, 2))
			self->ai_mood_flags |= AI_MOOD_FLAG_PREDICT;
		self->movetype = PHYSICSTYPE_STEP;
		self->solid=SOLID_BBOX;
		self->monsterinfo.dismember = seraph_guard_dismember;

		if (!self->s.scale)
		{
			self->s.scale = self->monsterinfo.scale = MODEL_SCALE;
		}

		VectorCopy(STDMinsForClass[self->classID], self->mins);
		VectorCopy(STDMaxsForClass[self->classID], self->maxs);	

		self->materialtype = MAT_FLESH;

		self->s.modelindex = classStatics[CID_SERAPH_GUARD].resInfo->modelIndex;
		self->s.skinnum = 0;

		self->monsterinfo.otherenemyname = "monster_rat";	

		//Turn off the Overlord pieces
		self->s.fmnodeinfo[MESH__WHIP].flags |= FMNI_NO_DRAW;
		self->s.fmnodeinfo[MESH__LHANDBOSS].flags |= FMNI_NO_DRAW;
		self->s.fmnodeinfo[MESH__PITHEAD].flags |= FMNI_NO_DRAW;

		MG_InitMoods(self);

		QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);

		self->melee_range *= self->s.scale;
	}
}
