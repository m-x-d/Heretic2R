//
// m_spreader.c
//
// Copyright 1998 Raven Software
//

#include "m_spreader.h"
#include "m_spreader_shared.h"
#include "m_spreader_anim.h"
#include "m_spreadermist.h"
#include "g_DefaultMessageHandler.h"
#include "g_debris.h" //mxd
#include "mg_ai.h" //mxd
#include "mg_guide.h" //mxd
#include "m_stats.h"
#include "p_anims.h"
#include "Random.h"
#include "Utilities.h"
#include "Vector.h"
#include "g_monster.h"

static void spreaderTakeOff(edict_t* self); //TODO: remove.

#pragma region ========================== Spreader Base Info ==========================

static const animmove_t* animations[NUM_ANIMS] =
{
	&spreader_move_attack1,
	&spreader_move_attack2,
	&spreader_move_backup1,
	&spreader_move_backattack1,
	&spreader_move_death1_go,
	&spreader_move_death1_loop,
	&spreader_move_death1_end,
	&spreader_move_duck1,
	&spreader_move_dkatck1,
	&spreader_move_duckdown,
	&spreader_move_duckstill,
	&spreader_move_duckup,
	&spreader_move_idle1,
	&spreader_move_pain1,
	&spreader_move_pvtlt1,
	&spreader_move_pvtrt1,
	&spreader_move_rnatck1,
	&spreader_move_run1,
	&spreader_move_land,
	&spreader_move_inair,
	&spreader_move_fjump,
	&spreader_move_walk1,
	&spreader_move_walk2,
	&spreader_move_death2,
	&spreader_move_fly,
	&spreader_move_flyloop,
	&spreader_move_fdie,
	&spreader_move_dead,
	&spreader_move_delay
};

static int sounds[NUM_SOUNDS];

#pragma endregion

void spreader_showgrenade(edict_t* self) //TODO: rename to spreader_show_grenade.
{
	if (!(self->monsterinfo.aiflags & AI_NO_MISSILE)) //FIXME: actually prevent these anims.
		self->s.fmnodeinfo[MESH__BOMB].flags &= ~FMNI_NO_DRAW;
}

void spreader_pain_sound(edict_t* self)
{
	gi.sound(self, CHAN_WEAPON, sounds[SND_PAIN], 1.0f, ATTN_NORM, 0.0f);
}

void spreader_miststartsound(edict_t* self) //TODO: rename to spreader_mist_start_sound.
{
	if (!(self->monsterinfo.aiflags & AI_NO_MELEE)) //FIXME: actually prevent these anims.
		gi.sound(self, CHAN_WEAPON, sounds[SND_SPRAYSTART], 1.0f, ATTN_IDLE, 0.0f);
}

void spreader_miststopsound(edict_t* self) //TODO: remove?
{
}

void spreader_idlenoise(edict_t* self) //TODO: rename to spreader_idle_noise.
{
	static int delay_noise = 0;

	if (irand(0, 9) < 7 && delay_noise > 0)
		return;

	if (++delay_noise >= 50)
		delay_noise = 0;

	gi.sound(self, CHAN_AUTO, sounds[irand(SND_VOICE1, SND_VOICE2)], 1.0f, ATTN_IDLE, 0.0f);
}

void spreader_hidegrenade(edict_t* self) //TODO: rename to spreader_hide_grenade.
{
	self->s.fmnodeinfo[MESH__BOMB].flags |= FMNI_NO_DRAW;
	gi.sound(self, CHAN_AUTO, sounds[SND_THROW], 1.0f, ATTN_IDLE, 0.0f); //TODO: move to spreader_toss_grenade()?
}

void spreader_flyback_loop(edict_t* self)
{
	SetAnim(self, ANIM_DEATH1_LOOP);
}

void spreader_flyback_move(edict_t* self)
{
	M_ChangeYaw(self);

	vec3_t end_pos;
	VectorCopy(self->s.origin, end_pos);
	end_pos[2] -= 48.0f;

	trace_t trace;
	gi.trace(self->s.origin, self->mins, self->maxs, end_pos, self, MASK_MONSTERSOLID, &trace);

	if (trace.fraction < 1.0f || trace.startsolid || trace.allsolid)
	{
		if (trace.fraction < 1.0f)
		{
			const int fx_flags = (irand(0, 1) == 1 ? CEF_FLAG6 : 0);
			const vec3_t bottom = { trace.endpos[0] + flrand(-4.0f, 4.0f), trace.endpos[1] + flrand(-4.0f, 4.0f), trace.endpos[2] + self->mins[2] };
			gi.CreateEffect(NULL, FX_BLOOD_TRAIL, fx_flags, bottom, "d", trace.plane.normal);
		}

		if (self->curAnimID != ANIM_DEATH1_END && self->curAnimID != ANIM_DEATH1_GO)
		{
			self->elasticity = 1.1f;
			self->friction = 0.5f;
			SetAnim(self, ANIM_DEATH1_END);
		}
	}
}

void spreader_dead(edict_t* self)
{
	if (irand(0, 1) == 1 && self->curAnimID == ANIM_DEATH1_END)
	{
		const vec3_t spray_dir = { flrand(-10.0f, 10.0f), flrand(-10.0f, 10.0f), flrand(10.0f, 100.0f) };
		const vec3_t offset = { 0.0f, 0.0f, -24.0f };

		//vec3_t spray_origin;
		//VectorAdd(self->s.origin, offset, spray_origin);

		// Create the volume effect for the damage.
		edict_t* gas = CreateRadiusDamageEnt(self, self, 1, 0, 30, 1.0f, DAMAGE_NO_BLOOD | DAMAGE_ALIVE_ONLY, 4.5f, 0.2f, NULL, offset, true);

		gas->svflags |= SVF_ALWAYS_SEND;
		gas->s.effects = EF_MARCUS_FLAG1;

		gi.CreateEffect(&gas->s, FX_PLAGUEMIST, CEF_OWNERS_ORIGIN, offset, "vb", spray_dir, 100); //FIXME: pass spray_origin, not offset? //TODO: test this.
	}

	M_EndDeath(self);
}

static void SpreaderCrouch(edict_t* self) //mxd. Named 'spreader_crouch' in original logic. //TODO: inline?
{
	VectorSet(self->maxs, 16.0f, 16.0f, 0.0f);
	self->viewheight = 0;
	SetAnim(self, ANIM_DUCKDOWN);
}

static qboolean SpreaderCheckUncrouch(edict_t* self) //mxd. Named 'spreader_check_uncrouch' in original logic.
{
	//FIXME: Need to ultimately make sure this is ok!
	const vec3_t mins = { self->mins[0], self->mins[1], 0.0f };
	const vec3_t maxs = { self->maxs[0], self->maxs[1], 1.0f };
	const float desired_height = STDMaxsForClass[CID_SPREADER][2] * self->s.scale;

	vec3_t end_pos;
	VectorCopy(self->s.origin, end_pos);
	end_pos[2] += desired_height;

	trace_t trace;
	gi.trace(self->s.origin, mins, maxs, end_pos, self, MASK_MONSTERSOLID, &trace);

	if (trace.fraction < 1.0f)
		return false;

	self->maxs[2] = desired_height;
	self->viewheight = (int)(self->maxs[2] - self->s.scale * 8.0f);

	return true;
}

void spreader_duckpause(edict_t* self) //TODO: rename to spreader_duck_pause.
{
	if (M_ValidTarget(self, self->enemy))
	{
		if (M_DistanceToTarget(self, self->enemy) < 128.0f)
		{
			SetAnim(self, ANIM_DUCKATTACK);
			return;
		}

		if (self->evade_debounce_time > level.time || irand(0, 10) == 0)
		{
			SetAnim(self, ANIM_DUCKSTILL);
			return;
		}
	}

	SetAnim(self, (SpreaderCheckUncrouch(self) ? ANIM_DUCKUP : ANIM_DUCKSTILL));
}

void spreader_pause(edict_t* self)
{
	const qboolean is_fixed = (self->spawnflags & MSF_FIXED); //mxd

	if (is_fixed && self->curAnimID == ANIM_DELAY && self->enemy != NULL)
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

		case AI_MOOD_NAVIGATE:
		case AI_MOOD_PURSUE:
		case AI_MOOD_FLEE:
			QPostMessage(self, MSG_RUN, PRI_DIRECTIVE, NULL);
			break;

		case AI_MOOD_STAND:
			if (self->enemy == NULL) //TODO: else what?
				QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
			break;

		case AI_MOOD_DELAY:
			SetAnim(self, ANIM_DELAY);
			break;

		case AI_MOOD_WANDER:
		case AI_MOOD_WALK:
			QPostMessage(self, MSG_WALK, PRI_DIRECTIVE, NULL);
			break;

		case AI_MOOD_JUMP:
			SetAnim(self, (is_fixed ? ANIM_DELAY : ANIM_FJUMP));
			break;

		case AI_MOOD_BACKUP:
			QPostMessage(self, MSG_FALLBACK, PRI_DIRECTIVE, NULL);
			break;

		default:
			break;
	}
}

static void SpreaderCheckMoodMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'spreader_check_mood' in original logic.
{
	ParseMsgParms(msg, "i", &self->ai_mood);
	spreader_pause(self);
}

static void SpreaderPainMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'spreader_pain' in original logic.
{
	int temp;
	int damage;
	qboolean force_pain;
	ParseMsgParms(msg, "eeiii", &temp, &temp, &force_pain, &damage, &temp);

	// Weighted random based on health compared to the maximum it was at.
	if (force_pain || (irand(0, self->max_health + 50) > self->health && irand(0, 2) != 0)) //mxd. flrand() in original logic.
	{
		gi.sound(self, CHAN_BODY, sounds[SND_PAIN], 1.0f, ATTN_NORM, 0.0f);
		SetAnim(self, ANIM_PAIN1);
	}
}

static void SpreaderStandMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'spreader_stand' in original logic.
{
	SetAnim(self, ANIM_IDLE1);
}

static void SpreaderRunMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'spreader_run' in original logic.
{
	if (!M_ValidTarget(self, self->enemy))
	{
		// If our enemy is dead, we need to stand.
		QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
		return;
	}

	SetAnim(self, ((self->spawnflags & MSF_FIXED) ? ANIM_DELAY : ANIM_RUN1));
}

static void SpreaderWalkMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'spreader_walk' in original logic.
{
	SetAnim(self, ((self->spawnflags & MSF_FIXED) ? ANIM_DELAY : ANIM_WALK1));
}

static void SpreaderMeleeMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'spreader_melee' in original logic.
{
	if (!M_ValidTarget(self, self->enemy))
	{
		SetAnim(self, ANIM_IDLE1);
		return;
	}

	const int chance = irand(0, 100);

	if (self->spawnflags & MSF_FIXED)
	{
		SetAnim(self, ((chance < 50) ? ANIM_DUCKDOWN : ANIM_DELAY));
		return;
	}

	if (M_DistanceToTarget(self, self->enemy) < 64.0f)
	{
		// Bumped into player? Knock him down.
		if (self->curAnimID == ANIM_RUNATTACK && self->enemy->health > 0 && self->enemy->client != NULL && self->enemy->client->playerinfo.lowerseq != ASEQ_KNOCKDOWN)
			if (irand(0, 2) == 0 && AI_IsInfrontOf(self->enemy, self))
				P_KnockDownPlayer(&self->enemy->client->playerinfo);

		if (self->curAnimID == ANIM_RUNATTACK || (self->curAnimID == ANIM_RUN1 && self->s.frame > FRAME_run1))
		{
			if (chance < 40)
				SetAnim(self, ANIM_DUCKDOWN);
			else
				SetAnim(self, ANIM_ATTACK2);
		}
		else
		{
			if (irand(0, 1) == 1 && !(self->monsterinfo.aiflags & AI_NO_MELEE))
				SetAnim(self, ANIM_BACKATTACK);
			else
				SetAnim(self, ANIM_BACKUP);
		}

		return;
	}

	if (chance < 20)
		SetAnim(self, ANIM_BACKUP);
	else if (chance < 40)
		SetAnim(self, ANIM_DUCKDOWN);
	else if (chance < 60 || (self->monsterinfo.aiflags & AI_NO_MELEE))
		SetAnim(self, ANIM_RUN1);
	else
		SetAnim(self, ANIM_ATTACK2);
}

static void SpreaderMissileMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'spreader_missile' in original logic.
{
	if (!M_ValidTarget(self, self->enemy))
	{
		SetAnim(self, ANIM_IDLE1);
		return;
	}

	const float dist = M_DistanceToTarget(self, self->enemy);

	if (dist < 64.0f)
	{
		if (!(self->monsterinfo.aiflags & AI_NO_MELEE) && irand(0, 5) != 0)
			SetAnim(self, ANIM_BACKATTACK);
		else
			SetAnim(self, ANIM_BACKUP);

		return;
	}

	const int chance = irand(0, 100);

	if (dist < 128.0f)
	{
		if (chance < 20)
			SetAnim(self, ANIM_BACKUP);
		else if (chance < 40)
			SetAnim(self, ANIM_DUCKDOWN);
		else if (chance < 60 || (self->monsterinfo.aiflags & AI_NO_MELEE))
			SetAnim(self, ANIM_RUN1);
		else
			SetAnim(self, ANIM_ATTACK2);

		return;
	}

	vec3_t forward;
	AngleVectors(self->s.angles, forward, NULL, NULL);

	vec3_t attack_vel;
	VectorScale(forward, 200.0f, attack_vel);
	const qboolean in_range = M_PredictTargetEvasion(self, self->enemy, attack_vel, self->enemy->velocity, 150.0f, 5.0f);

	// See what the predicted outcome is.
	trace_t trace;
	if (!(self->monsterinfo.aiflags & AI_NO_MELEE) && in_range && chance < 25 && M_CheckMeleeHit(self, 200.0f, &trace) == self->enemy)
		SetAnim(self, ANIM_RUNATTACK);
	else if (self->monsterinfo.aiflags & AI_NO_MISSILE)
		SetAnim(self, ANIM_RUN1);
	else
		SetAnim(self, ANIM_ATTACK1);
}

static void SpreaderFallbackMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'spreader_fallback' in original logic.
{
	if (self->spawnflags & MSF_FIXED)
		SetAnim(self, ANIM_DELAY);
	else if (!(self->monsterinfo.aiflags & AI_NO_MELEE))
		SetAnim(self, ANIM_BACKATTACK);
	else
		SetAnim(self, ANIM_BACKUP);
}

static void SpreaderEvadeMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'spreader_evade' in original logic.
{
	edict_t* projectile;
	HitLocation_t hl;
	float eta;
	ParseMsgParms(msg, "eif", &projectile, &hl, &eta);

	if (irand(0, 1) == 0 || self->curAnimID == ANIM_DUCKDOWN || self->curAnimID == ANIM_DUCKUP || self->curAnimID == ANIM_DUCKSTILL)
		return;

	int duck_chance;

	switch (hl)
	{
		case hl_Head:
			duck_chance = 100;
			break;

		case hl_TorsoFront:
		case hl_TorsoBack:
		case hl_ArmUpperLeft:
		case hl_ArmUpperRight:
			duck_chance = 75;
			break;

		case hl_ArmLowerLeft:
		case hl_ArmLowerRight:
			duck_chance = 35;
			break;

		case hl_LegUpperLeft:
		case hl_LegLowerLeft:
		case hl_LegUpperRight:
		case hl_LegLowerRight:
			duck_chance = 0;
			break;

		default:
			duck_chance = 10;
			break;
	}

	if (irand(0, 100) < duck_chance)
	{
		self->evade_debounce_time = level.time + eta; //TODO: add spreader_stay_duck_time name.
		SpreaderCrouch(self);
	}
}

static void SpreaderDeathMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'spreader_death' in original logic.
{
	edict_t* target = NULL; //mxd. Pre-initialize.

	if (msg != NULL)
	{
		edict_t* inflictor;
		edict_t* attacker;
		float damage;
		ParseMsgParms(msg, "eeei", &target, &inflictor, &attacker, &damage);
	}

	self->s.fmnodeinfo[MESH__BOMB].flags |= FMNI_NO_DRAW; //mxd. Hide grenade. Original logic calls spreader_hidegrenade() here, which also plays SND_THROW.
	M_StartDeath(self, ANIM_DEATH1_GO);

	if (self->health < -80) // To be gibbed.
		return;

	if (self->health < -10)
	{
		SetAnim(self, ANIM_DEATH1_GO);
		VectorClear(self->knockbackvel);

		if (target != NULL) //BUGFIX: mxd. No NULL check in original logic.
		{
			vec3_t dir;
			VectorNormalize2(target->velocity, dir);

			vec3_t yaw_dir;
			VectorScale(dir, -1.0f, yaw_dir);

			self->ideal_yaw = VectorYaw(yaw_dir);
			self->yaw_speed = 16.0f;

			VectorScale(dir, 300.0f, self->velocity);
			self->velocity[2] = flrand(150.0f, 250.0f); //mxd. irand() in original logic.
		}
	}
	else
	{
		SetAnim(self, ANIM_DEATH2);
	}

	gi.sound(self, CHAN_BODY, sounds[SND_DEATH], 1.0f, ATTN_NORM, 0.0f);
}

static qboolean SpreaderCanThrowNode(edict_t* self, const int node_id, int* throw_nodes) //mxd. Named 'canthrownode_ps' in original logic.
{
	static const int bit_for_mesh_node[NUM_MESH_NODES] = //mxd. Made local static.
	{
		BIT_PARENT,
		BIT_CHILD,
		BIT_BODY,
		BIT_BOMB,
		BIT_RITLEG,
		BIT_LFTARM,
		BIT_LFTLEG,
		BIT_HEAD,
		BIT_RITARM,
		BIT_TANK3,
		BIT_TANK2,
		BIT_TANK1,
		BIT_HOSE
	};

	// See if it's on, if so, add it to throw_nodes. Turn it off on thrower.
	if (!(self->s.fmnodeinfo[node_id].flags & FMNI_NO_DRAW))
	{
		*throw_nodes |= bit_for_mesh_node[node_id];
		self->s.fmnodeinfo[node_id].flags |= FMNI_NO_DRAW;

		return true;
	}

	return false;
}

void spreader_dropweapon (edict_t *self)
{//NO PART FLY FRAME!
	vec3_t handspot, right;

	AngleVectors(self->s.angles,NULL,right,NULL);

	if(self->s.fmnodeinfo[BIT_BOMB].flags & FMNI_NO_DRAW)
		return;

	VectorClear(handspot);
	VectorMA(handspot, -12, right, handspot);
	ThrowWeapon(self, &handspot, BIT_BOMB, 0, 0);
	self->s.fmnodeinfo[MESH__BOMB].flags |= FMNI_NO_DRAW;
}

void spreader_dead_pain (edict_t *self, G_Message_t *msg)
{
	if(msg)
		if(!(self->svflags & SVF_PARTS_GIBBED))
			DismemberMsgHandler(self, msg);
}

void spreader_dismember(edict_t *self, int damage, int HitLocation)
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
	
	VectorCopy(vec3_origin,gore_spot);
	switch(HitLocation)
	{
		case hl_Head:
			if(self->s.fmnodeinfo[MESH__HEAD].flags & FMNI_NO_DRAW)
				break;
			if(self->s.fmnodeinfo[MESH__HEAD].flags & FMNI_USE_SKIN)
				damage*=1.5;//greater chance to cut off if previously damaged
			if(flrand(0,self->health)<damage*0.3&&dismember_ok)
			{
				SpreaderCanThrowNode(self, MESH__HEAD,&throw_nodes);

				gore_spot[2]+=18;
				ThrowBodyPart(self, &gore_spot, throw_nodes, damage, 0);

				VectorAdd(self->s.origin, gore_spot, gore_spot);
				SprayDebris(self, gore_spot,(byte)(8),damage);

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
			if(self->s.fmnodeinfo[MESH__BODY].flags & FMNI_USE_SKIN)
				damage*=1.5;//greater chance to cut off if previously damaged
			if(flrand(0, self->health) < damage*0.3 && !irand(0,9))		// One in 10 chance of the takeoff even if reqs met.
			{//fly straight up, hit cieling, head squish, otherwise go though sky
				gore_spot[2]+=12;

				SpreaderCanThrowNode(self, MESH__TANK3,&throw_nodes);
				SpreaderCanThrowNode(self, MESH__TANK2,&throw_nodes);
				SpreaderCanThrowNode(self, MESH__TANK1,&throw_nodes);
				SpreaderCanThrowNode(self, MESH__HOSE,&throw_nodes);

				ThrowWeapon(self, &gore_spot, throw_nodes, 0, FRAME_death17);

				if(self->health>0)
					spreaderTakeOff(self);
				return;
			}
			else
			{
				self->s.fmnodeinfo[MESH__BODY].flags |= FMNI_USE_SKIN;			
				self->s.fmnodeinfo[MESH__BODY].skin = self->s.skinnum+1;
			}
			break;
		case hl_TorsoBack://split in half?
			if(self->s.fmnodeinfo[MESH__BODY].flags & FMNI_USE_SKIN)
				damage*=1.5;//greater chance to cut off if previously damaged
			if(flrand(0, self->health) < damage*0.7 && !irand(0,3))		// 25% chance of the takeoff even if reqs met..
			{//fly straight up, hit cieling, head squish, otherwise go though sky
				gore_spot[2]+=12;

				SpreaderCanThrowNode(self, MESH__TANK3,&throw_nodes);
				SpreaderCanThrowNode(self, MESH__TANK2,&throw_nodes);
				SpreaderCanThrowNode(self, MESH__TANK1,&throw_nodes);
				SpreaderCanThrowNode(self, MESH__HOSE,&throw_nodes);

				ThrowWeapon(self, &gore_spot, throw_nodes, 0, FRAME_death17);

				if(self->health>0)
					spreaderTakeOff(self);
				return;
			}
			else
			{
				self->s.fmnodeinfo[MESH__BODY].flags |= FMNI_USE_SKIN;			
				self->s.fmnodeinfo[MESH__BODY].skin = self->s.skinnum+1;
			}
			break;

		case hl_ArmUpperLeft:
		case hl_ArmLowerLeft://left arm
			//what about hose?
			if(self->s.fmnodeinfo[MESH__LFTARM].flags & FMNI_NO_DRAW)
				break;
			if(self->s.fmnodeinfo[MESH__LFTARM].flags & FMNI_USE_SKIN)
				damage*=1.5;//greater chance to cut off if previously damaged
			if(flrand(0,self->health)<damage*0.75&&dismember_ok)
			{
				if(SpreaderCanThrowNode(self, MESH__LFTARM, &throw_nodes))
				{
					AngleVectors(self->s.angles,NULL,right,NULL);
					gore_spot[2]+=self->maxs[2]*0.3;
					VectorMA(gore_spot,-10,right,gore_spot);
					ThrowBodyPart(self, &gore_spot, throw_nodes, damage, 0);
				}
			}
			else
			{
				self->s.fmnodeinfo[MESH__LFTARM].flags |= FMNI_USE_SKIN;			
				self->s.fmnodeinfo[MESH__LFTARM].skin = self->s.skinnum+1;
			}
			break;
		case hl_ArmUpperRight:
		case hl_ArmLowerRight://right arm
			//what about grenade?
			if(self->s.fmnodeinfo[MESH__RITARM].flags & FMNI_NO_DRAW)
				break;
			if(flrand(0,self->health)<damage*0.75&&dismember_ok)
			{
				if(SpreaderCanThrowNode(self, MESH__RITARM, &throw_nodes))
				{
					AngleVectors(self->s.angles,NULL,right,NULL);
					gore_spot[2]+=self->maxs[2]*0.3;
					VectorMA(gore_spot,10,right,gore_spot);
					spreader_dropweapon (self);
					ThrowBodyPart(self, &gore_spot, throw_nodes, damage, 0);
				}
			}
			else
			{
				self->s.fmnodeinfo[MESH__RITARM].flags |= FMNI_USE_SKIN;			
				self->s.fmnodeinfo[MESH__RITARM].skin = self->s.skinnum+1;
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
			if(self->s.fmnodeinfo[MESH__RITLEG].flags & FMNI_USE_SKIN)
				break;
			self->s.fmnodeinfo[MESH__RITLEG].flags |= FMNI_USE_SKIN;
			self->s.fmnodeinfo[MESH__RITLEG].skin = self->s.skinnum+1;
			break;

		default:
			if(flrand(0,self->health)<damage*0.25)
				spreader_dropweapon (self);
			break;
	}

	if(self->s.fmnodeinfo[MESH__LFTARM].flags&FMNI_NO_DRAW&&
		self->s.fmnodeinfo[MESH__RITARM].flags&FMNI_NO_DRAW)			
	{
		self->monsterinfo.aiflags |= AI_COWARD;
	}
	else
	{
		if(self->s.fmnodeinfo[MESH__LFTARM].flags&FMNI_NO_DRAW)
			self->monsterinfo.aiflags |= AI_NO_MELEE;

		if(self->s.fmnodeinfo[MESH__RITARM].flags&FMNI_NO_DRAW)
			self->monsterinfo.aiflags |= AI_NO_MISSILE;
	}

//	gi.dprintf(" done\n");
}

void spreader_stop (edict_t *self, trace_t *trace)
{//aparently being on ground no longer causes you to lose avelocity so I do it manually
	VectorClear(self->avelocity);
	self->svflags &= ~SVF_TAKE_NO_IMPACT_DMG;
	self->isBlocked = NULL;
	self->bounced = NULL;
}

void spreader_isblocked (edict_t *self, trace_t *trace)
{
	edict_t *other;
	vec3_t	gore_spot;

	if(trace->surface)
	{
		if(trace->surface->flags & SURF_SKY)
		{
			self->movetype = PHYSICSTYPE_NOCLIP;
			self->solid = SOLID_NOT;
			self->isBlocked = NULL;
			self->bounced = NULL;
			return;
		}
	}

	other = trace->ent;
	
	if((other->movetype != PHYSICSTYPE_NONE) && (other->movetype != PHYSICSTYPE_PUSH))
	{
		if(other == self->enemy && self->touch_debounce_time > level.time)
			return;
		
		self->enemy = other;
		
		VectorAdd(other->velocity, self->velocity, other->velocity);
		
		if(other->takedamage)
			T_Damage (other, self, self, vec3_origin, vec3_origin, vec3_origin, 10, 20,0,MOD_DIED);
		
		self->touch_debounce_time = level.time + 0.3;
		return;
	}


	self->s.fmnodeinfo[MESH__HEAD].flags |= FMNI_NO_DRAW;
	VectorCopy(self->s.origin, gore_spot);
	gore_spot[2]+=self->maxs[2] - 8;
	SprayDebris(self, gore_spot, 8, 100);

	self->health = 1;
	T_Damage (self, self, self, vec3_origin, vec3_origin, vec3_origin, 10, 20,0,MOD_DIED);
	self->isBlocked = spreader_stop;
	self->bounced = spreader_stop;
	SpreaderDeathMsgHandler(self, NULL);
	self->avelocity[YAW] = 0;

	self->elasticity = 1.3;
	self->friction = 0.8;
	return;
}

void spreaderTakeOff (edict_t *self)
{
	vec3_t	forward;
	edict_t	*gas;

	self->msgHandler=DeadMsgHandler;
	self->isBlocked = spreader_isblocked;
	self->bounced = spreader_isblocked;
	
	AngleVectors(self->s.angles, forward, NULL, NULL);
	VectorMA(self->s.origin, -12, forward, self->pos1);
	self->pos1[2] += self->maxs[2] * 0.8;

	// create the volume effect for the damage
	gas = CreateRadiusDamageEnt(self,//owner
					self,//damage-owner
					1,//damage
					0,//d_damage
					150,//radius
					0.0,//d_radius
					DAMAGE_NO_BLOOD|DAMAGE_ALIVE_ONLY,//dflags
					4.5,//lifetime
					0.1,//thinkincr
					NULL,//origin
					self->pos1,//velocity or offset
					true);//offset from owner?

	gas->svflags |= SVF_ALWAYS_SEND;
	gas->s.effects=EF_MARCUS_FLAG1;

	gi.CreateEffect(&gas->s, FX_PLAGUEMISTEXPLODE, CEF_OWNERS_ORIGIN, self->pos1, "b", 70);	

	gi.sound(self, CHAN_VOICE, sounds[SND_PAIN], 1, ATTN_NORM, 0);
	gi.sound(self, CHAN_WEAPON, sounds[SND_SPRAYSTART], 1, ATTN_NORM, 0);
	gi.sound(self, CHAN_AUTO, sounds[SND_BOMB],	1, ATTN_NORM, 0);

	self->pain_debounce_time = level.time + flrand(0.4, 0.8);//for sound loop

	self->velocity[0] = self->velocity[1] = 0;//not realistic, but funnier
	self->velocity[2]+=150;
	self->avelocity[YAW] = flrand(-200, -600);
	if(irand(0,10)<5)
		self->avelocity[YAW]*=-1;

	self->movetype = PHYSICSTYPE_FLY;
	self->svflags |= SVF_ALWAYS_SEND;

	spreader_dropweapon (self);

	self->svflags |= SVF_TAKE_NO_IMPACT_DMG;

	SetAnim(self, ANIM_FLY);
}

void spreaderSplat (edict_t *self, trace_t *trace)//, edict_s *other, cplane_s *plane, csurface_s *surf)/*(edict_t *self, trace_t *trace)*/
{
	vec3_t dir;
	float speed;

	if(trace->ent)
	{
		if(trace->ent->takedamage)
		{
			if(Vec3IsZero(self->velocity))
				VectorSet(dir, 0, 0, 1);
			else
			{
				VectorCopy(self->velocity, dir);
				speed = VectorNormalize(dir);
			}
			
			if(speed<50)
				speed = irand(50, 200);

			T_Damage(trace->ent, self, self, dir, trace->endpos, dir, speed, 0, 0,MOD_DIED);

			if(trace->ent->health>0)//else don't gib?
				if(!stricmp(trace->ent->classname, "player"))
					P_KnockDownPlayer(&trace->ent->client->playerinfo);
		}
	}

	self->dead_state = DEAD_DEAD;
	self->health = -1000;
	self->mass = 0.01;
	self->think = BecomeDebris;
	self->nextthink = level.time + 0.01;
}

void spreader_go_deadloop (edict_t *self)
{
	SetAnim(self, ANIM_DEAD);
}

void spreaderSolidAgain (edict_t *self)
{
	vec3_t org;

	VectorCopy(self->s.origin, org);
	org[2]+=self->maxs[2];

	self->nextthink = level.time + 0.1;

	if(!gi.pointcontents(org))
	{
		if(self->movetype == PHYSICSTYPE_STEP)
			return;

		self->svflags &= ~SVF_ALWAYS_SEND;
		self->movetype = PHYSICSTYPE_STEP;
		self->solid = SOLID_BBOX;
		self->isBlocked = spreaderSplat;
		self->bounced = spreaderSplat;
		self->svflags &= ~SVF_TAKE_NO_IMPACT_DMG;
	}
	else
	{
		if(self->velocity[2]>-600)
		{
			if(VectorLength(self->movedir)>500)
				VectorCopy(self->movedir, self->velocity);
			else
				self->velocity[2] -= 50;
		}
		else
			VectorCopy(self->velocity, self->movedir);
	}
}

void spreaderDropDown (edict_t *self)
{
	self->movetype = PHYSICSTYPE_NOCLIP;
	self->solid=SOLID_NOT;
	self->velocity[2] = -200;
	self->avelocity[0] = irand(-300, 300);
	self->avelocity[1] = irand(-300, 300);
	self->avelocity[2] = irand(-300, 300);

	SetAnim(self, ANIM_FDIE);
	self->think = M_Think;
	self->nextthink = level.time + 0.1;
}

void spreaderFly (edict_t *self)
{
	vec3_t spraydir, forward;

	if(self->pain_debounce_time <= level.time)
	{
		gi.sound(self, CHAN_BODY, sounds[SND_SPRAYLOOP], 1, ATTN_NORM, 0);
		self->pain_debounce_time = level.time + flrand(0.4, 0.8);
	}

	if(self->velocity[2]<800)
		self->velocity[2] += 100;

	if(self->velocity[2] > 800)
		self->velocity[2] = 800;

	AngleVectors(self->s.angles, forward, NULL, NULL);
	VectorMA(self->s.origin, -12, forward, self->pos1);
	self->pos1[2] += self->maxs[2] * 0.8;

	spraydir[0] = flrand(-100, 100);
	spraydir[1] = flrand(-100, 100);
	spraydir[2] = -self->velocity[2];
	gi.CreateEffect(NULL, FX_PLAGUEMIST, 0, self->pos1, "vb", spraydir, 41);

	if(self->s.origin[2]>3900)
	{
		self->movetype = PHYSICSTYPE_NONE;
		VectorClear(self->velocity);
		self->think = spreaderDropDown;
		self->nextthink = level.time + flrand(1.5, 3);
	}
	else if(self->health<=0)
	{
		VectorClear(self->avelocity);
		//spreader_death(self);
	}
}

void spreaderFlyLoop (edict_t *self)
{
	SetAnim(self, ANIM_FLYLOOP);
}

void spreader_land(edict_t *self)
{
	gi.sound(self, CHAN_BODY, gi.soundindex("misc/land.wav"), 1, ATTN_NORM, 0);
	gi.CreateEffect(&self->s,
					   FX_DUST_PUFF,
					   CEF_OWNERS_ORIGIN,
					   self->s.origin,
					   NULL);
}

void spreaderApplyJump (edict_t *self)
{
	self->jump_time = level.time + 0.5;
	VectorCopy(self->movedir, self->velocity);
	VectorNormalize(self->movedir);
}

void spreader_jump (edict_t *self, G_Message_t *msg)
{
	SetAnim(self, ANIM_FJUMP);
}

void spreader_go_inair (edict_t *self)
{
	SetAnim(self, ANIM_INAIR);
}

/*------------------------------------------------------------------------
	startup stuff -- initialization for the spreader class and individual
	spreaders
-------------------------------------------------------------------------
	SpreaderStaticsInit
-------------------------------------------------------------------------*/
void SpreaderStaticsInit(void)
{
	static ClassResourceInfo_t resInfo;

	resInfo.numAnims = NUM_ANIMS;
	resInfo.animations = animations;
	resInfo.modelIndex = gi.modelindex("models/monsters/spreader/tris.fm");
	
	classStatics[CID_SPREADER].msgReceivers[MSG_STAND] = SpreaderStandMsgHandler;
	classStatics[CID_SPREADER].msgReceivers[MSG_RUN] = SpreaderRunMsgHandler;
	classStatics[CID_SPREADER].msgReceivers[MSG_WALK] = SpreaderWalkMsgHandler;
	classStatics[CID_SPREADER].msgReceivers[MSG_MELEE] = SpreaderMeleeMsgHandler;
	classStatics[CID_SPREADER].msgReceivers[MSG_MISSILE] = SpreaderMissileMsgHandler;
	classStatics[CID_SPREADER].msgReceivers[MSG_DISMEMBER] = DismemberMsgHandler;
	classStatics[CID_SPREADER].msgReceivers[MSG_JUMP] = spreader_jump;
	classStatics[CID_SPREADER].msgReceivers[MSG_EVADE] = SpreaderEvadeMsgHandler;
	classStatics[CID_SPREADER].msgReceivers[MSG_FALLBACK] = SpreaderFallbackMsgHandler;
	classStatics[CID_SPREADER].msgReceivers[MSG_DEATH] = SpreaderDeathMsgHandler;
	classStatics[CID_SPREADER].msgReceivers[MSG_PAIN] = SpreaderPainMsgHandler;
	classStatics[CID_SPREADER].msgReceivers[MSG_DEATH_PAIN] = spreader_dead_pain;
	classStatics[CID_SPREADER].msgReceivers[MSG_CHECK_MOOD] = SpreaderCheckMoodMsgHandler;

	sounds[SND_SPRAYSTART]	= gi.soundindex("monsters/spreader/spraystart.wav");
	sounds[SND_SPRAYLOOP]	= gi.soundindex("monsters/spreader/sprayloop.wav");
	sounds[SND_PAIN]		= gi.soundindex("monsters/spreader/pain1.wav");
	sounds[SND_VOICE1]		= gi.soundindex("monsters/spreader/voice1.wav");
	sounds[SND_VOICE2]		= gi.soundindex("monsters/spreader/voice2.wav");
	sounds[SND_THROW]		= gi.soundindex("monsters/spreader/throw.wav");
	sounds[SND_DEATH]		= gi.soundindex("monsters/spreader/death.wav");
	sounds[SND_BOMB]		= gi.soundindex("monsters/spreader/gasbomb.wav");	
	sounds[SND_SPRAYLOOP]	= gi.soundindex("monsters/spreader/sprayloop.wav");	
	resInfo.numSounds = NUM_SOUNDS;
	
	resInfo.sounds = sounds;

	classStatics[CID_SPREADER].resInfo = &resInfo;
}

/*QUAKED monster_spreader (1 .5 0) (-16 -16 -0) (16 16 32) AMBUSH ASLEEP WALKING 8 16 32 64 FIXED WANDER MELEE_LEAD STALK COWARD EXTRA1 EXTRA2 EXTRA3 EXTRA4

The spreader 

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
mintel					= 24
melee_range				= 100
missile_range			= 512
min_missile_range		= 200
bypass_missile_chance	= 50
jump_chance				= 30
wakeup_distance			= 1024

NOTE: A value of zero will result in defaults, if you actually want zero as the value, use -1
*/
/*-------------------------------------------------------------------------
	SP_monster_spreader
-------------------------------------------------------------------------*/
void SP_monster_spreader (edict_t *self)
{
	if(self->spawnflags & MSF_WALKING)
	{
		self->spawnflags |= MSF_WANDER;
		self->spawnflags &= ~MSF_WALKING;
	}

	if (!M_Start(self))
		return;					// Failed initialization
		
	self->msgHandler = DefaultMsgHandler;
	self->think = M_WalkmonsterStartGo;
	self->monsterinfo.dismember = spreader_dismember;

	if (!self->health)
		self->health = SPREADER_HEALTH;
	self->max_health = self->health = MonsterHealth(self->health);

	self->mass = SPREADER_MASS;
	self->materialtype = MAT_FLESH;
	self->yaw_speed = 20;

	self->s.origin[2] += 40;
	self->movetype = PHYSICSTYPE_STEP;
	
	self->solid=SOLID_BBOX;
	
	VectorCopy(STDMinsForClass[self->classID], self->mins);
	VectorCopy(STDMaxsForClass[self->classID], self->maxs);	
	self->viewheight = 36;

	self->s.modelindex = classStatics[CID_SPREADER].resInfo->modelIndex;
	self->s.fmnodeinfo[MESH__BOMB].flags |= FMNI_NO_DRAW; //hide the bomb

	self->ai_mood_flags |= AI_MOOD_FLAG_PREDICT;

	self->s.skinnum = 0;	
	self->monsterinfo.scale = MODEL_SCALE;

	self->moveinfo.sound_start = gi.soundindex("monsters/spreader/spraystart.wav");
	self->moveinfo.sound_middle = gi.soundindex("monsters/spreader/sprayloop.wav");
	self->moveinfo.sound_end = gi.soundindex("monsters/spreader/sprayloop.wav");

	self->touch = M_Touch;

	self->monsterinfo.otherenemyname = "monster_box"; //TODO: 'monster_box' is not defined anywhere.
	self->monsterinfo.aiflags = 0;
	self->monsterinfo.flee_finished = 0;
	
	MG_InitMoods(self);
	self->min_melee_range = 24;

	//FIXME what else should he spawn doing?  
	if(self->spawnflags & MSF_WANDER)
	{
		QPostMessage(self, MSG_WALK, PRI_DIRECTIVE, NULL);
	}
	else
	{
		QPostMessage(self,MSG_STAND,PRI_DIRECTIVE, NULL);
	}
}
