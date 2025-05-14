//
// m_tcheckrik.c
//
// Copyright 1998 Raven Software
//

#include "m_tcheckrik.h"
#include "m_tcheckrik_shared.h"
#include "m_tcheckrik_anim.h"
#include "m_tcheckrik_spells.h" //mxd
#include "g_DefaultMessageHandler.h"
#include "g_debris.h" //mxd
#include "c_ai.h"
#include "mg_ai.h" //mxd
#include "mg_guide.h" //mxd
#include "m_stats.h"
#include "Random.h"
#include "Utilities.h"
#include "Vector.h"
#include "g_monster.h"

#pragma region ========================== Tcheckrik Base Info ==========================

static const animmove_t* animations[NUM_ANIMS] =
{
	&insect_move_back,
	&insect_move_deathfr,
	&insect_move_idle,
	&insect_move_land,
	&insect_move_inair,
	&insect_move_forcedjump,
	&insect_move_finair,
	&insect_move_fjump,
	&insect_move_paina,
	&insect_move_painc,
	&insect_move_run,
	&insect_move_spear,
	&insect_move_sword,
	&insect_move_spell,
	&insect_move_spell2,
	&insect_move_walk,
	&insect_delay,
	&insect_move_knock1_go,
	&insect_move_knock1_loop,
	&insect_move_knock1_end,
	&insect_move_twitch,

	// Cinematic animations.
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

static int sounds[NUM_SOUNDS];

#pragma endregion

#pragma region ========================== Utility functions =========================

static qboolean TcheckrikCanThrowNode(edict_t* self, const int node_id, int* throw_nodes) //mxd. Named 'canthrownode_tc' in original logic.
{
	static const int bit_for_mesh_node[NUM_MESH_NODES] = //mxd. Made local static.
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

	// See if it's on, if so, add it to throw_nodes. Turn it off on thrower.
	if (!(self->s.fmnodeinfo[node_id].flags & FMNI_NO_DRAW))
	{
		*throw_nodes |= bit_for_mesh_node[node_id];
		self->s.fmnodeinfo[node_id].flags |= FMNI_NO_DRAW;

		return true;
	}

	return false;
}

// Throws weapon, turns off those nodes, sets that weapon as gone.
static void TcheckrikDropWeapon(edict_t* self, const int weapon_id) //mxd. Named 'insect_dropweapon' in original logic.
{
	vec3_t forward;
	vec3_t right;
	vec3_t up;
	AngleVectors(self->s.angles, forward, right, up);

	if ((weapon_id == 0 || (weapon_id & BIT_STAFF)) && !(self->s.fmnodeinfo[MESH__STAFF].flags & FMNI_NO_DRAW))
	{
		vec3_t hand_spot = { 0 };
		VectorMA(hand_spot, 8.0f, forward, hand_spot);
		VectorMA(hand_spot, 5.0f, right, hand_spot);
		VectorMA(hand_spot, 12.0f, up, hand_spot);

		ThrowWeapon(self, &hand_spot, BIT_STAFF, 0, FRAME_partfly);

		self->s.fmnodeinfo[MESH__STAFF].flags |= FMNI_NO_DRAW;
		self->s.fmnodeinfo[MESH__GEM].flags |= FMNI_NO_DRAW;

		return;
	}

	if ((weapon_id == 0 || (weapon_id & BIT_SPEAR)) && !(self->s.fmnodeinfo[MESH__SPEAR].flags & FMNI_NO_DRAW))
	{
		vec3_t hand_spot = { 0 };
		VectorMA(hand_spot, 6.0f, forward, hand_spot);
		VectorMA(hand_spot, 4.0f, right, hand_spot);

		ThrowWeapon(self, &hand_spot, BIT_SPEAR, 0, FRAME_partfly);

		self->s.fmnodeinfo[MESH__SPEAR].flags |= FMNI_NO_DRAW;

		return;
	}

	if ((weapon_id == 0 || (weapon_id & BIT_SWORD)) && !(self->s.fmnodeinfo[MESH__MALEHAND].flags & FMNI_NO_DRAW))
	{
		vec3_t hand_spot = { 0 };
		VectorMA(hand_spot, 6.0f, forward, hand_spot);
		VectorMA(hand_spot, -6.0f, right, hand_spot);
		VectorMA(hand_spot, -6.0f, up, hand_spot);

		ThrowWeapon(self, &hand_spot, BIT_SWORD, 0, FRAME_partfly);

		self->s.fmnodeinfo[MESH__MALEHAND].flags |= FMNI_NO_DRAW;
	}
}

#pragma endregion

#pragma region ========================== Message handlers ==========================

static void TcheckrikDeathPain(edict_t* self, G_Message_t* msg) //mxd. Named 'insect_dead_pain' in original logic.
{
	if (msg != NULL && !(self->svflags & SVF_PARTS_GIBBED))
		DismemberMsgHandler(self, msg);

	if (self->curAnimID != ANIM_TWITCH && self->dead_state != DEAD_DEAD)
		return; // Still dying.

	if (self->s.frame == FRAME_knock15)
	{
		SetAnim(self, ANIM_TWITCH);

		self->think = M_MoveFrame;
		self->nextthink = level.time + FRAMETIME; //mxd. Use define.
	}
}

static void TcheckrikDeathMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'insect_death' in original logic.
{
	if (self->monsterinfo.aiflags & AI_DONT_THINK)
	{
		SetAnim(self, ANIM_DEATHFR);
		return;
	}

	self->msgHandler = DeadMsgHandler;

	if (self->dead_state == DEAD_DEAD) // Dead but still being hit.
		return;

	gi.RemoveEffects(&self->s, FX_I_EFFECTS);

	self->s.effects |= EF_DISABLE_EXTRA_FX;
	self->dead_state = DEAD_DEAD;

	TcheckrikDropWeapon(self, BIT_SPEAR);
	TcheckrikDropWeapon(self, BIT_STAFF);

	if (self->health <= -80) // Gib death.
	{
		gi.sound(self, CHAN_BODY, sounds[SND_GIB], 1.0f, ATTN_NORM, 0.0f);

		self->think = BecomeDebris;
		self->nextthink = level.time + FRAMETIME; //mxd. Use define.

		return;
	}

	const int snd_id = ((self->mass == MASS_TC_MALE) ? SND_DIEM : SND_DIEF); //mxd. Inline insect_random_death_sound().
	gi.sound(self, CHAN_VOICE, sounds[snd_id], 1.0f, ATTN_NORM, 0.0f);

	if (self->health < -20)
	{
		SetAnim(self, ANIM_KNOCK1_GO);
		VectorClear(self->knockbackvel);

		vec3_t dir;
		VectorNormalize2(self->velocity, dir);

		vec3_t yaw_dir;
		VectorScale(dir, -1.0f, yaw_dir);

		self->ideal_yaw = VectorYaw(yaw_dir);
		self->yaw_speed = 16.0f;

		VectorScale(dir, 300.0f, self->velocity);
		self->velocity[2] = flrand(150.0f, 250.0f); //mxd. irand() in original logic.
	}
	else
	{
		SetAnim(self, ANIM_DEATHFR);
	}
}

static void TcheckrikFallbackMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'insect_backpedal' in original logic.
{
	if (M_ValidTarget(self, self->enemy))
		SetAnim(self, (self->spawnflags & MSF_FIXED) ? ANIM_DELAY : ANIM_BACK); // Not male?
	else
		QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
}

static void TcheckrikMeleeMsgHandler(edict_t* self, G_Message_t* msg)//mxd. Named 'insect_melee' in original logic.
{
	if (!M_ValidTarget(self, self->enemy))
	{
		QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
		return;
	}

	if (self->mass == MASS_TC_MALE) // Male
	{
		// Run away from Trial Beast.
		if (self->enemy->classID == CID_TBEAST && self->enemy->enemy == self && M_DistanceToTarget(self, self->enemy) < 250.0f)
		{
			self->monsterinfo.aiflags |= AI_FLEE;
			self->monsterinfo.flee_finished = level.time + 3.0f;
			QPostMessage(self, MSG_RUN, PRI_DIRECTIVE, NULL);

			return;
		}

		SetAnim(self, ANIM_SWORD);
	}
	else // Too close for female, back away (maybe attack anyway?).
	{
		if ((irand(0, 5) == 0 && !(self->monsterinfo.aiflags & AI_NO_MELEE)) || (self->spawnflags & MSF_FIXED)) //mxd. Group irand() / aiflags checks.
			SetAnim(self, ANIM_SPELL);
		else
			SetAnim(self, ANIM_BACK);
	}
}

static void TcheckrikMissileMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'insect_missile' in original logic.
{
	if (!M_ValidTarget(self, self->enemy))
	{
		QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
		return;
	}

	if (self->mass == MASS_TC_MALE) // Male.
	{
		// Run away from Trial Beast.
		if (self->enemy->classID == CID_TBEAST && self->enemy->enemy == self && M_DistanceToTarget(self, self->enemy) < 250.0f)
		{
			self->monsterinfo.aiflags |= AI_FLEE;
			self->monsterinfo.flee_finished = level.time + 3.0f;
			QPostMessage(self, MSG_RUN, PRI_DIRECTIVE, NULL);

			return;
		}

		SetAnim(self, ANIM_SPEAR);
	}
	else // Female.
	{
		SetAnim(self, ((self->spawnflags & MSF_INSECT_ALTERNATE) ? ANIM_SPELL2 : ANIM_SPELL));
	}
}

static void TcheckrikPainMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'insect_pain' in original logic.
{
	int temp;
	int damage;
	int force_damage;
	ParseMsgParms(msg, "eeiii", &temp, &temp, &force_damage, &damage, &temp);

	if (!force_damage && irand(0, self->health) > damage) //mxd. flrand() in original logic.
		return;

	gi.RemoveEffects(&self->s, FX_I_EFFECTS);
	self->s.effects |= EF_DISABLE_EXTRA_FX;

	// Sound. 
	const int snd_id = (self->mass == MASS_TC_MALE ? SND_PAINM : SND_PAINF); //mxd. Inline insect_random_pain_sound().
	gi.sound(self, CHAN_VOICE, sounds[snd_id], 1.0f, ATTN_NORM, 0.0f);

	// Remove spell effects.
	self->monsterinfo.aiflags &= ~AI_OVERRIDE_GUIDE;

	if (force_damage || self->pain_debounce_time < level.time)
	{
		self->pain_debounce_time = level.time + 1.0f;
		SetAnim(self, (irand(0, 1) == 1 ? ANIM_PAINA : ANIM_PAINC));
	}
}

static void TcheckrikCheckMoodMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'insect_check_mood' in original logic.
{
	ParseMsgParms(msg, "i", &self->ai_mood);
	tcheckrik_pause(self);
}

static void TcheckrikRunMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'insect_run' in original logic.
{
	if (!M_ValidTarget(self, self->enemy))
	{
		QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
		return;
	}

	if (self->enemy->classID == CID_TBEAST && self->enemy->enemy != self && M_DistanceToTarget(self, self->enemy) < 250.0f)
	{
		self->monsterinfo.aiflags |= AI_FLEE;
		self->monsterinfo.flee_finished = level.time + 3.0f;
	}

	SetAnim(self, ((self->spawnflags & MSF_FIXED) ? ANIM_DELAY : ANIM_RUN));
}

static void TcheckrikStandMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'insect_stand' in original logic.
{
	SetAnim(self, ((self->ai_mood == AI_MOOD_DELAY) ? ANIM_DELAY : ANIM_IDLE));
}

static void TcheckrikWalkMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'insect_walk' in original logic.
{
	SetAnim(self, ((self->spawnflags & MSF_FIXED) ? ANIM_DELAY : ANIM_WALK));
}

static void TcheckrikJumpMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'insect_jump' in original logic.
{
	if (self->spawnflags & MSF_FIXED)
	{
		SetAnim(self, ANIM_DELAY);
	}
	else
	{
		SetAnim(self, ANIM_FORCED_JUMP);
		self->monsterinfo.aiflags |= AI_OVERRIDE_GUIDE;
	}
}

static void TcheckrikCinematicActionMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'insect_c_anims' in original logic.
{
	int curr_anim;

	ReadCinematicMessage(self, msg);
	self->monsterinfo.c_anim_flag = 0;

	switch (msg->ID)
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
			assert(0); //mxd
			return; //mxd. 'break' in original version.
	}

	SetAnim(self, curr_anim);
}

#pragma endregion

#pragma region ========================== Edict callbacks ===========================

static qboolean TcheckrikThrowHead(edict_t* self, float damage, const qboolean dismember_ok, int* throw_nodes) //mxd. Added to simplify logic.
{
	if (self->s.fmnodeinfo[MESH__HEAD].flags & FMNI_NO_DRAW)
		return false;

	if (self->s.fmnodeinfo[MESH__HEAD].flags & FMNI_USE_SKIN)
		damage *= 1.5f; // Greater chance to cut off if previously damaged.

	if (flrand(0.0f, (float)self->health) < damage * 0.25f)
		TcheckrikDropWeapon(self, 0);

	if (dismember_ok && flrand(0, (float)self->health) < damage * 0.3f)
	{
		TcheckrikCanThrowNode(self, MESH__HEAD, throw_nodes);
		TcheckrikCanThrowNode(self, MESH__CROWN, throw_nodes);
		TcheckrikCanThrowNode(self, MESH__LMANDIBLE, throw_nodes);
		TcheckrikCanThrowNode(self, MESH__RMANDIBLE, throw_nodes);

		vec3_t gore_spot = { 0.0f, 0.0f, 24.0f };
		ThrowBodyPart(self, &gore_spot, *throw_nodes, damage, FRAME_partfly);

		Vec3AddAssign(self->s.origin, gore_spot);
		SprayDebris(self, gore_spot, 8, damage);

		if (self->health > 0)
		{
			self->health = 1;
			T_Damage(self, self, self, vec3_origin, vec3_origin, vec3_origin, 10, 20, 0, MOD_DIED);
		}

		return true;
	}

	int part_id = MESH__HEAD; //mxd

	if (irand(0, 1) == 1 || (self->s.fmnodeinfo[part_id].flags & FMNI_USE_SKIN))
	{
		if (self->mass == MASS_TC_MALE)
			part_id = ((irand(0, 1) == 1 || self->s.fmnodeinfo[MESH__RMANDIBLE].flags & FMNI_USE_SKIN) ? MESH__LMANDIBLE : MESH__RMANDIBLE);
		else
			part_id = MESH__CROWN;
	}

	self->s.fmnodeinfo[part_id].flags |= FMNI_USE_SKIN;
	self->s.fmnodeinfo[part_id].skin = self->s.skinnum + 1;

	return false;
}

static void TcheckrikThrowTorso(edict_t* self, const float damage) //mxd. Added to simplify logic.
{
	if (self->s.fmnodeinfo[MESH_MASTER].flags & FMNI_USE_SKIN)
		return;

	if (flrand(0.0f, (float)self->health) < damage * 0.5f)
		TcheckrikDropWeapon(self, 0);

	self->s.fmnodeinfo[MESH_MASTER].flags |= FMNI_USE_SKIN;
	self->s.fmnodeinfo[MESH_MASTER].skin = self->s.skinnum + 1;
}

static void TcheckrikThrowArmUpperLeft(edict_t* self, float damage) //mxd. Added to simplify logic.
{
	if (self->mass == MASS_TC_MALE) // Male - sword arm.
	{
		if (self->s.fmnodeinfo[MESH__L2NDARM].flags & FMNI_USE_SKIN)
			damage *= 1.5f; // Greater chance to cut off if previously damaged.

		if (flrand(0, (float)self->health) < damage * 0.4f)
			TcheckrikDropWeapon(self, BIT_SWORD);

		self->s.fmnodeinfo[MESH__L2NDARM].flags |= FMNI_USE_SKIN;
		self->s.fmnodeinfo[MESH__L2NDARM].skin = self->s.skinnum + 1;
	}
	else // Female.
	{
		if (self->s.fmnodeinfo[MESH_MASTER].flags & FMNI_USE_SKIN)
			return;

		self->s.fmnodeinfo[MESH_MASTER].flags |= FMNI_USE_SKIN;
		self->s.fmnodeinfo[MESH_MASTER].skin = self->s.skinnum + 1;
	}
}

static qboolean TcheckrikThrowArmLowerLeft(edict_t* self, float damage, const qboolean dismember_ok, int* throw_nodes) //mxd. Added to simplify logic.
{
	if (self->mass == MASS_TC_MALE) // Male - left spear arm.
	{
		// Male - left spear arm.
		if (self->s.fmnodeinfo[MESH__L2NDARM].flags & FMNI_NO_DRAW)
			return false;

		if (self->s.fmnodeinfo[MESH__L2NDARM].flags & FMNI_USE_SKIN)
			damage *= 1.5f; // Greater chance to cut off if previously damaged.

		if (dismember_ok && flrand(0, (float)self->health) < damage * 0.75f)
		{
			if (TcheckrikCanThrowNode(self, MESH__L2NDARM, throw_nodes))
			{
				vec3_t right;
				AngleVectors(self->s.angles, NULL, right, NULL);

				vec3_t gore_spot = { 0.0f, 0.0f, self->maxs[2] * 0.3f };
				VectorMA(gore_spot, -10.0f, right, gore_spot);

				ThrowBodyPart(self, &gore_spot, *throw_nodes, damage, FRAME_partfly);

				if ((self->s.fmnodeinfo[MESH__R2NDARM].flags & FMNI_NO_DRAW) && !(self->s.fmnodeinfo[MESH__SPEAR].flags & FMNI_NO_DRAW))
					TcheckrikDropWeapon(self, BIT_SPEAR);
			}
		}
		else
		{
			if (flrand(0, (float)self->health) < damage * 0.4f)
				TcheckrikDropWeapon(self, BIT_SPEAR);

			self->s.fmnodeinfo[MESH__L2NDARM].flags |= FMNI_USE_SKIN;
			self->s.fmnodeinfo[MESH__L2NDARM].skin = self->s.skinnum + 1;
		}

		return false;
	}

	// Female.
	if (!(self->s.fmnodeinfo[MESH_MASTER].flags & FMNI_USE_SKIN))
	{
		self->s.fmnodeinfo[MESH_MASTER].flags |= FMNI_USE_SKIN;
		self->s.fmnodeinfo[MESH_MASTER].skin = self->s.skinnum + 1;
	}

	return true;
}

static void TcheckrikThrowArmUpperRight(edict_t* self, float damage) //mxd. Added to simplify logic.
{
	if (self->mass == MASS_TC_MALE) // Male - right upper arm - nothing in it.
	{
		if (self->s.fmnodeinfo[MESH_MASTER].flags & FMNI_USE_SKIN)
			return;

		self->s.fmnodeinfo[MESH_MASTER].flags |= FMNI_USE_SKIN;
		self->s.fmnodeinfo[MESH_MASTER].skin = self->s.skinnum + 1;
	}
	else // Female.
	{
		if (self->s.fmnodeinfo[MESH_MASTER].flags & FMNI_USE_SKIN)
		{
			damage *= 1.5f; // Greater chance to cut off if previously damaged.
		}
		else
		{
			self->s.fmnodeinfo[MESH_MASTER].flags |= FMNI_USE_SKIN;
			self->s.fmnodeinfo[MESH_MASTER].skin = self->s.skinnum + 1;
		}

		if (flrand(0, (float)self->health) < damage * 0.4f)
			TcheckrikDropWeapon(self, BIT_STAFF);
	}
}

static void TcheckrikThrowArmLowerRight(edict_t* self, float damage, const qboolean dismember_ok, int* throw_nodes) //mxd. Added to simplify logic.
{
	if (self->mass == MASS_TC_MALE) // Male - right spear arm.
	{
		if (self->s.fmnodeinfo[MESH__R2NDARM].flags & FMNI_NO_DRAW)
			return;

		if (self->s.fmnodeinfo[MESH__R2NDARM].flags & FMNI_USE_SKIN)
			damage *= 1.5f; // Greater chance to cut off if previously damaged.

		if (dismember_ok && flrand(0.0f, (float)self->health) < damage * 0.75f)
		{
			if (TcheckrikCanThrowNode(self, MESH__R2NDARM, throw_nodes))
			{
				vec3_t right;
				AngleVectors(self->s.angles, NULL, right, NULL);

				vec3_t gore_spot = { 0.0f, 0.0f, self->maxs[2] * 0.3f };
				VectorMA(gore_spot, 10.0f, right, gore_spot); //mxd. -10 (same as left arm) in original logic.

				ThrowBodyPart(self, &gore_spot, *throw_nodes, damage, FRAME_partfly);

				if ((self->s.fmnodeinfo[MESH__L2NDARM].flags & FMNI_NO_DRAW) && !(self->s.fmnodeinfo[MESH__SPEAR].flags & FMNI_NO_DRAW))
					TcheckrikDropWeapon(self, BIT_SPEAR);
			}
		}
		else
		{
			if (flrand(0.0f, (float)self->health) < damage * 0.4f)
				TcheckrikDropWeapon(self, BIT_SPEAR);

			self->s.fmnodeinfo[MESH__R2NDARM].flags |= FMNI_USE_SKIN;
			self->s.fmnodeinfo[MESH__R2NDARM].skin = self->s.skinnum + 1;
		}
	}
	else // Female.
	{
		if (self->s.fmnodeinfo[MESH_MASTER].flags & FMNI_USE_SKIN)
		{
			damage *= 1.5f; // Greater chance to cut off if previously damaged.
		}
		else
		{
			self->s.fmnodeinfo[MESH_MASTER].flags |= FMNI_USE_SKIN;
			self->s.fmnodeinfo[MESH_MASTER].skin = self->s.skinnum + 1;
		}

		if (flrand(0, (float)self->health) < damage * 0.4f)
			TcheckrikDropWeapon(self, BIT_STAFF);
	}
}

static void TcheckrikThrowLeg(edict_t* self, const float damage, const int mesh_part, int* throw_nodes) //mxd. Added to simplify logic.
{
	if (self->health > 0) // Still alive.
	{
		if (self->s.fmnodeinfo[mesh_part].flags & FMNI_USE_SKIN)
			return;

		self->s.fmnodeinfo[mesh_part].flags |= FMNI_USE_SKIN;
		self->s.fmnodeinfo[mesh_part].skin = self->s.skinnum + 1;
	}
	else
	{
		if (self->s.fmnodeinfo[mesh_part].flags & FMNI_NO_DRAW)
			return;

		if (TcheckrikCanThrowNode(self, mesh_part, throw_nodes))
		{
			vec3_t right;
			AngleVectors(self->s.angles, NULL, right, NULL);

			vec3_t gore_spot = { 0.0f, 0.0f, self->maxs[2] * 0.3f };
			const float side = (mesh_part == MESH__LLEG ? -1.0f : 1.0f); //mxd. Original logic throws both legs in the same direction.
			VectorMA(gore_spot, 10.0f * side, right, gore_spot);
			ThrowBodyPart(self, &gore_spot, *throw_nodes, damage, FRAME_partfly);
		}
	}
}

static void TcheckrikDismember(edict_t* self, int damage, HitLocation_t hl) //mxd. Named 'insect_dismember' in original logic.
{
	int throw_nodes = 0;
	qboolean dismember_ok = false;

	if (hl & hl_MeleeHit)
	{
		dismember_ok = true;
		hl &= ~hl_MeleeHit;
	}

	if (hl <= hl_NoneSpecific || hl >= hl_Max) //mxd. '> hl_Max' in original logic.
		return;

	self->monsterinfo.aiflags &= ~AI_OVERRIDE_GUIDE;

	switch (hl)
	{
		case hl_Head:
			if (TcheckrikThrowHead(self, (float)damage, dismember_ok, &throw_nodes)) //mxd
				return;
			break;

		case hl_TorsoFront: // Split in half?
		case hl_TorsoBack:
			TcheckrikThrowTorso(self, (float)damage); //mxd
			break;

		case hl_ArmUpperLeft:
			TcheckrikThrowArmUpperLeft(self, (float)damage); //mxd
			break;

		case hl_ArmLowerLeft: // Left arm.
			if (TcheckrikThrowArmLowerLeft(self, (float)damage, dismember_ok, &throw_nodes)) //mxd
				return;
			break;

		case hl_ArmUpperRight:
			TcheckrikThrowArmUpperRight(self, (float)damage); //mxd
			break;

		case hl_ArmLowerRight:// Right arm.
			TcheckrikThrowArmLowerRight(self, (float)damage, dismember_ok, &throw_nodes); //mxd
			break;

		case hl_LegUpperLeft: // Left leg.
		case hl_LegLowerLeft:
			TcheckrikThrowLeg(self, (float)damage, MESH__LLEG, &throw_nodes); //mxd
			break;

		case hl_LegUpperRight: // Right leg.
		case hl_LegLowerRight:
			TcheckrikThrowLeg(self, (float)damage, MESH__RLEG, &throw_nodes); //mxd
			break;

		default:
			if (flrand(0.0f, (float)self->health) < (float)damage * 0.25f)
				TcheckrikDropWeapon(self, 0);
			break;
	}

	if (throw_nodes != 0)
		self->pain_debounce_time = 0.0f;

	if (self->mass == MASS_TC_MALE)
	{
		if (self->s.fmnodeinfo[MESH__MALEHAND].flags & FMNI_NO_DRAW)
			self->monsterinfo.aiflags |= AI_NO_MELEE;

		if (self->s.fmnodeinfo[MESH__SPEAR].flags & FMNI_NO_DRAW)
			self->monsterinfo.aiflags |= AI_NO_MISSILE;
	}
}

#pragma endregion

#pragma region ========================== Action functions ==========================

void tcheckrik_c_dead(edict_t* self) //mxd. Named 'insect_c_reallydead' in original logic.
{
	self->nextthink = level.time; //TODO: should be -1?..
	self->think = NULL;
}

void tcheckrik_attack(edict_t* self, float attack_type) //mxd. Named 'insectCut' in original logic.
{
	static const vec3_t weapon_mins = { -2.0f, -2.0f, -1.0f };
	static const vec3_t weapon_maxs = {  2.0f,  2.0f,  1.0f };

	int damage; //mxd. float in original logic.

	vec3_t forward;
	vec3_t right;
	AngleVectors(self->s.angles, forward, right, NULL);

	vec3_t hit_start;
	VectorCopy(self->s.origin, hit_start);

	vec3_t hit_end;
	VectorMA(hit_start, 72.0f, forward, hit_end);

	switch ((int)attack_type)
	{
		case TC_ATK_STAB:
			if (self->s.fmnodeinfo[MESH__SPEAR].flags & FMNI_NO_DRAW)
				return;
			VectorMA(hit_end, -16.0f, right, hit_end);
			damage = irand(TC_DMG_STAB_MIN, TC_DMG_STAB_MAX);
			break;

		case TC_ATK_HACK:
			VectorMA(hit_start, 12.0f, forward, hit_start);
			if (self->s.fmnodeinfo[MESH__MALEHAND].flags & FMNI_NO_DRAW)
			{
				hit_start[2] += 28.0f;
				VectorMA(hit_end, 24.0f, right, hit_end);
				damage = irand(TC_MALE_DMG_HACK_MIN, TC_MALE_DMG_HACK_MAX);
			}
			else
			{
				hit_start[2] += 18.0f;
				VectorMA(hit_end, 12.0f, right, hit_end);
				damage = irand(TC_FEMALE_DMG_HACK_MIN, TC_FEMALE_DMG_HACK_MAX);
			}
			break;

		default:
			return;
	}

	trace_t trace;
	gi.trace(hit_start, weapon_mins, weapon_maxs, hit_end, self, MASK_MONSTERSOLID | MASK_SHOT, &trace);

	// Do this check before the allsolid check, because trace is screwy -- fraction should be valid in all cases, so shouldn't be a problem.
	if (trace.fraction == 1.0f)
		return; // Missed totally.

	if (trace.allsolid || trace.startsolid || trace.ent->takedamage == DAMAGE_NO)
	{
		// Ping!
		vec3_t hit_angles;
		vectoangles(trace.plane.normal, hit_angles);

		gi.CreateEffect(NULL, FX_SPARKS, 0, trace.endpos, "d", hit_angles);
		gi.sound(self, CHAN_AUTO, sounds[SND_SWIPEHITW], 1.0f, ATTN_NORM, 0.0f);

		return;
	}

	// Hit someone, cut em!
	vec3_t hit_dir;
	VectorSubtract(hit_end, hit_start, hit_dir);

	gi.sound(self, CHAN_AUTO, sounds[SND_SWIPEHITF], 1.0f, ATTN_NORM, 0.0f);
	T_Damage(trace.ent, self, self, hit_dir, trace.endpos, vec3_origin, damage, damage * 2, DAMAGE_DISMEMBER, MOD_DIED);
}

void tcheckrik_dead(edict_t* self) //mxd. Named 'insect_dead' in original logic.
{
	self->s.effects |= EF_DISABLE_EXTRA_FX;
	self->msgHandler = DeadMsgHandler;
	self->dead_state = DEAD_DEAD;

	M_EndDeath(self);
}

void tcheckrik_idle_sound(edict_t* self) //mxd. Named 'insect_random_idle_sound' in original logic.
{
	if (self->mass == MASS_TC_MALE)
		gi.sound(self, CHAN_VOICE, sounds[irand(SND_GROWLM1, SND_GROWLM2)], 1.0f, ATTN_NORM, 0.0f);
	else
		gi.sound(self, CHAN_VOICE, sounds[irand(SND_GROWLF1, SND_GROWLF2)], 1.0f, ATTN_NORM, 0.0f);
}

void tcheckrik_wait_twitch(edict_t* self) //mxd. Named 'insect_wait_twitch' in original logic.
{
	if (self->curAnimID != ANIM_TWITCH && irand(0, 1) == 0)
	{
		SetAnim(self, ANIM_TWITCH);
		self->nextthink = level.time + FRAMETIME; //mxd. Use define.
	}
	else if (irand(0, 5) != 0)
	{
		SetAnim(self, ANIM_TWITCH);
		self->nextthink = level.time + flrand(0.2f, 10.0f);
	}
	else
	{
		self->s.effects |= EF_DISABLE_EXTRA_FX;
		self->msgHandler = DeadMsgHandler;
		self->dead_state = DEAD_DEAD;

		M_EndDeath(self);
		self->think = NULL;
	}
}

void tcheckrik_flyback_loop(edict_t* self) //mxd. Named 'insect_flyback_loop' in original logic.
{
	SetAnim(self, ANIM_KNOCK1_LOOP);
}

void tcheckrik_flyback_move(edict_t* self) //mxd. Named 'insect_flyback_move' in original logic.
{
	M_ChangeYaw(self);

	vec3_t end_pos;
	VectorCopy(self->s.origin, end_pos);
	end_pos[2] -= 48.0f;

	trace_t trace;
	gi.trace(self->s.origin, self->mins, self->maxs, end_pos, self, MASK_MONSTERSOLID, &trace);

	if ((trace.fraction < 1.0f || trace.startsolid || trace.allsolid) && self->curAnimID != ANIM_KNOCK1_END && self->curAnimID != ANIM_KNOCK1_GO)
	{
		self->elasticity = 1.1f;
		self->friction = 0.5f;
		SetAnim(self, ANIM_KNOCK1_END);
	}
}

void tcheckrik_growl(edict_t* self) //mxd. Named 'insectgrowl' in original logic.
{
	if (irand(0, 10) > 2)
		return;

	if (self->mass == MASS_TC_MALE)
		gi.sound(self, CHAN_WEAPON, sounds[irand(SND_GROWLM1, SND_GROWLM2)], 1.0f, ATTN_IDLE, 0.0f);
	else
		gi.sound(self, CHAN_WEAPON, sounds[irand(SND_GROWLF1, SND_GROWLF2)], 1.0f, ATTN_IDLE, 0.0f);
}

void tcheckrik_sound(edict_t* self, float channel, float sound_num, float attenuation) //mxd. Named 'insect_sound' in original logic.
{
	gi.sound(self, (int)channel, sounds[(int)sound_num], 1.0f, attenuation, 0.0f);
}

void tcheckrik_staff_attack(edict_t* self) //mxd. Named 'insectStaff' in original logic.
{
	vec3_t forward;
	vec3_t right;
	AngleVectors(self->s.angles, forward, right, NULL);

	vec3_t org;
	VectorCopy(self->s.origin, org);
	VectorMA(org, 12.0f, forward, org);
	VectorMA(org, 4.0f, right, org);

	if (self->spawnflags & MSF_INSECT_YELLOWJACKET)
	{
		if (self->s.frame == FRAME_SpearB4)
		{
			if (SKILL == SKILL_EASY)
			{
				SpellCastInsectSpear(self, org, self->s.angles, irand(1, 3));
			}
			else if (SKILL == SKILL_MEDIUM)
			{
				SpellCastInsectSpear(self, org, self->s.angles, 1);
				SpellCastInsectSpear(self, org, self->s.angles, 2);
			}
			else // HARD, HARD+.
			{
				SpellCastInsectSpear(self, org, self->s.angles, 1);
				SpellCastInsectSpear(self, org, self->s.angles, 2);
				SpellCastInsectSpear(self, org, self->s.angles, 3);
			}

			gi.sound(self, CHAN_WEAPON, sounds[SND_SPELLM2], 1.0f, ATTN_NORM, 0.0f);
		}
	}
	else
	{
		SpellCastInsectSpear(self, org, self->s.angles, 0);
		gi.sound(self, CHAN_WEAPON, sounds[SND_SPELLM], 1.0f, ATTN_NORM, 0.0f);
	}
}

void tcheckrik_spell_attack(edict_t* self, float spell_type) //mxd. Named 'insectSpell' in original logic.
{
	if (!(self->spawnflags & MSF_INSECT_CINEMATIC))
		ai_charge(self, 0.0f);

	vec3_t forward;
	vec3_t right;
	AngleVectors(self->s.angles, forward, right, NULL);

	vec3_t org;
	VectorCopy(self->s.origin, org);

	switch ((int)spell_type)
	{
		case TC_SPL_GLOW:
			if (self->s.fmnodeinfo[MESH__STAFF].flags & FMNI_NO_DRAW) // No staff, weaker spell.
			{
				VectorMA(org, 8.0f, forward, org);
				VectorMA(org, 2.0f, right, org);
				org[2] += 6.0f;

				SpellCastInsectStaff(self, org, self->s.angles, forward, false);
				gi.sound(self, CHAN_WEAPON, sounds[SND_SPELLF], 1.0f, ATTN_NORM, 0.0f);
			}
			else // Staff, bigger spell.
			{
				VectorMA(org, 16.0f, forward, org);
				VectorMA(org, 4.0f, right, org);
				org[2] += 8.0f;

				self->s.effects &= ~(EF_DISABLE_EXTRA_FX | EF_MARCUS_FLAG1);
				self->tcheckrik_globe_spell_released = false;

				SpellCastGlobeOfOuchiness(self, org, self->s.angles, forward);
				gi.sound(self, CHAN_ITEM, sounds[SND_SPLPWRUPF], 1.0f, ATTN_NORM, 0.0f);
			}
			break;

		case TC_SPL_FIRE:
			if (self->s.fmnodeinfo[MESH__STAFF].flags & FMNI_NO_DRAW) // No staff, weaker spell. //TODO: identical if/else clauses. Shoot powered spell when have staff?..
			{
				VectorMA(org, 16.0f, forward, org);
				org[2] += 12.0f;

				SpellCastInsectStaff(self, org, self->s.angles, forward, false);
				gi.sound(self, CHAN_AUTO, sounds[SND_SPELLF], 1.0f, ATTN_NORM, 0.0f);
			}
			else // Staff, bigger spell.
			{
				VectorMA(org, 16.0f, forward, org);
				org[2] += 12.0f;

				SpellCastInsectStaff(self, org, self->s.angles, forward, false);
				gi.sound(self, CHAN_AUTO, sounds[SND_SPELLF], 1.0f, ATTN_NORM, 0.0f);
			}
			break;

		case TC_SPL_FIRE2:
			if (!(self->s.fmnodeinfo[MESH__STAFF].flags & FMNI_NO_DRAW)) // No staff, weaker spell.
			{
				VectorMA(org, 16.0f, forward, org);
				org[2] += 12.0f;

				SpellCastInsectStaff(self, org, self->s.angles, forward, true); //TODO: should be the other way around (unpowered here, powered below)?
				gi.sound(self, CHAN_AUTO, gi.soundindex("monsters/imp/fireball.wav"), 1.0f, ATTN_NORM, 0.0f);
			}
			else // Staff, bigger spell.
			{
				VectorMA(org, 16.0f, forward, org);
				org[2] += 12.0f;

				SpellCastInsectStaff(self, org, self->s.angles, forward, false);
				gi.sound(self, CHAN_AUTO, sounds[SND_SPELLF], 1.0f, ATTN_NORM, 0.0f);
			}
			break;

		default:
			break;
	}
}

void tcheckrik_release_spell(edict_t* self) //mxd. Named 'insectReleaseSpell' in original logic.
{
	gi.RemoveEffects(&self->s, FX_I_EFFECTS);

	self->s.effects |= (EF_DISABLE_EXTRA_FX | EF_MARCUS_FLAG1);
	self->tcheckrik_globe_spell_released = true;
}

void tcheckrik_pause(edict_t* self) //mxd. Named 'insect_pause' in original logic.
{
	if (self->spawnflags & MSF_INSECT_BEAST_FODDER)
	{
		edict_t* tbeast = NULL;

		if ((tbeast = G_Find(tbeast, FOFS(classname), "monster_trial_beast")) != NULL && tbeast->health > 0)
		{
			self->enemy = tbeast;
			tbeast->oldenemy = tbeast->enemy;
			tbeast->enemy = self;

			self->spawnflags &= ~MSF_INSECT_BEAST_FODDER;

			self->monsterinfo.aiflags &= ~AI_NO_MISSILE;
			self->missile_range = 3000.0f;
			self->min_missile_range = 250.0f;
			self->bypass_missile_chance = 0;

			self->monsterinfo.aiflags &= ~AI_NO_MELEE;
			self->melee_range = 48.0f;

			self->s.fmnodeinfo[MESH__MALEHAND].flags &= ~FMNI_NO_DRAW; //TODO: why is this done here and not in spawn function?..
		}
	}

	if (self->monsterinfo.aiflags & AI_OVERRIDE_GUIDE)
	{
		if (self->groundentity != NULL)
			self->monsterinfo.aiflags &= ~AI_OVERRIDE_GUIDE;
		else
			return;
	}

	if ((self->spawnflags & MSF_FIXED) && self->curAnimID == ANIM_DELAY && self->enemy != NULL)
	{
		self->monsterinfo.searchType = SEARCH_COMMON;
		MG_FaceGoal(self, true);
	}

	self->mood_think(self);

	if (self->ai_mood == AI_MOOD_NORMAL)
	{
		FindTarget(self);

		if (self->enemy != NULL)
		{
			vec3_t diff;
			VectorSubtract(self->s.origin, self->enemy->s.origin, diff);

			if (VectorLength(diff) > 80.0f || (self->monsterinfo.aiflags & AI_FLEE)) // Far enough to run after.
				QPostMessage(self, MSG_RUN, PRI_DIRECTIVE, NULL);
			else // Close enough to attack.
				QPostMessage(self, MSG_MELEE, PRI_DIRECTIVE, NULL);
		}

		return;
	}

	if (self->enemy != NULL && self->enemy->classID == CID_TBEAST && AI_IsVisible(self, self->enemy) && M_DistanceToTarget(self, self->enemy) > 250.0f)
	{
		if (AI_IsInfrontOf(self, self->enemy))
		{
			self->ai_mood = AI_MOOD_ATTACK;
			self->ai_mood_flags |= AI_MOOD_FLAG_MISSILE;
		}
		else
		{
			self->ai_mood = AI_MOOD_PURSUE;
			self->ai_mood_flags &= ~AI_MOOD_FLAG_DUMB_FLEE;
			self->monsterinfo.aiflags &= ~AI_FLEE;
		}

		self->monsterinfo.flee_finished = -1.0f;
	}

	switch (self->ai_mood)
	{
		case AI_MOOD_ATTACK:
			QPostMessage(self, ((self->ai_mood_flags & AI_MOOD_FLAG_MISSILE) ? MSG_MISSILE : MSG_MELEE), PRI_DIRECTIVE, NULL);
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
		case AI_MOOD_WALK:
			QPostMessage(self, MSG_WALK, PRI_DIRECTIVE, NULL);
			break;

		case AI_MOOD_BACKUP:
			QPostMessage(self, MSG_FALLBACK, PRI_DIRECTIVE, NULL);
			break;

		case AI_MOOD_JUMP:
			SetAnim(self, ((self->spawnflags & MSF_FIXED) ? ANIM_DELAY : ANIM_FJUMP));
			break;

		default:
			break;
	}
}

void tcheckrik_inair_go(edict_t* self) //mxd. Named 'insect_go_inair' in original logic.
{
	SetAnim(self, ANIM_INAIR);
}

void tcheckrik_forced_inair_go(edict_t* self) //mxd. Named 'insect_go_finair' in original logic.
{
	SetAnim(self, ANIM_FINAIR);
}

void tcheckrik_check_loop(edict_t* self, float frame) //mxd. Named 'insectCheckLoop' in original logic.
{
#define TCHECKRIK_MELEE_RANGE	64.0f
#define TCHECKRIK_MISSILE_RANGE	384.0f

	// See if should fire again.
	if (self->enemy == NULL || self->enemy->health <= 0 || !AI_IsVisible(self, self->enemy) || !AI_IsInfrontOf(self, self->enemy))
		return;

	ai_charge2(self, 0.0f);

	if (irand(0, 100) < self->bypass_missile_chance || self->monsterinfo.attack_finished > level.time)
		return;

	if (AI_IsInfrontOf(self, self->enemy))
	{
		vec3_t diff;
		VectorSubtract(self->s.origin, self->enemy->s.origin, diff);
		const float len = VectorLength(diff);
		const float min_seperation = self->maxs[0] + self->enemy->maxs[0];

		// Don't loop if enemy close enough.
		if (len < min_seperation + TCHECKRIK_MELEE_RANGE)
			return;

		if (len < min_seperation + TCHECKRIK_MISSILE_RANGE && irand(0, 10) < 3)
			return;
	}

	self->monsterinfo.currframeindex = (int)frame;
}

#pragma endregion

void TcheckrikStaticsInit(void)
{
	static ClassResourceInfo_t res_info; //mxd. Made local static.

	classStatics[CID_TCHECKRIK].msgReceivers[MSG_STAND] = TcheckrikStandMsgHandler;
	classStatics[CID_TCHECKRIK].msgReceivers[MSG_WALK] = TcheckrikWalkMsgHandler;
	classStatics[CID_TCHECKRIK].msgReceivers[MSG_RUN] = TcheckrikRunMsgHandler;
	classStatics[CID_TCHECKRIK].msgReceivers[MSG_MELEE] = TcheckrikMeleeMsgHandler;
	classStatics[CID_TCHECKRIK].msgReceivers[MSG_MISSILE] = TcheckrikMissileMsgHandler;
	classStatics[CID_TCHECKRIK].msgReceivers[MSG_PAIN] = TcheckrikPainMsgHandler;
	classStatics[CID_TCHECKRIK].msgReceivers[MSG_DEATH] = TcheckrikDeathMsgHandler;
	classStatics[CID_TCHECKRIK].msgReceivers[MSG_FALLBACK] = TcheckrikFallbackMsgHandler;
	classStatics[CID_TCHECKRIK].msgReceivers[MSG_DISMEMBER] = DismemberMsgHandler;
	classStatics[CID_TCHECKRIK].msgReceivers[MSG_JUMP] = TcheckrikJumpMsgHandler;
	classStatics[CID_TCHECKRIK].msgReceivers[MSG_DEATH_PAIN] = TcheckrikDeathPain;
	classStatics[CID_TCHECKRIK].msgReceivers[MSG_CHECK_MOOD] = TcheckrikCheckMoodMsgHandler;

	// Cinematics.
	classStatics[CID_TCHECKRIK].msgReceivers[MSG_C_ACTION1] = TcheckrikCinematicActionMsgHandler;
	classStatics[CID_TCHECKRIK].msgReceivers[MSG_C_ACTION2] = TcheckrikCinematicActionMsgHandler;
	classStatics[CID_TCHECKRIK].msgReceivers[MSG_C_ACTION3] = TcheckrikCinematicActionMsgHandler;
	classStatics[CID_TCHECKRIK].msgReceivers[MSG_C_ACTION4] = TcheckrikCinematicActionMsgHandler;
	classStatics[CID_TCHECKRIK].msgReceivers[MSG_C_ATTACK1] = TcheckrikCinematicActionMsgHandler;
	classStatics[CID_TCHECKRIK].msgReceivers[MSG_C_ATTACK2] = TcheckrikCinematicActionMsgHandler;
	classStatics[CID_TCHECKRIK].msgReceivers[MSG_C_ATTACK3] = TcheckrikCinematicActionMsgHandler;
	classStatics[CID_TCHECKRIK].msgReceivers[MSG_C_BACKPEDAL1] = TcheckrikCinematicActionMsgHandler;
	classStatics[CID_TCHECKRIK].msgReceivers[MSG_C_DEATH1] = TcheckrikCinematicActionMsgHandler;
	classStatics[CID_TCHECKRIK].msgReceivers[MSG_C_IDLE1] = TcheckrikCinematicActionMsgHandler;
	classStatics[CID_TCHECKRIK].msgReceivers[MSG_C_IDLE2] = TcheckrikCinematicActionMsgHandler;
	classStatics[CID_TCHECKRIK].msgReceivers[MSG_C_IDLE3] = TcheckrikCinematicActionMsgHandler;
	classStatics[CID_TCHECKRIK].msgReceivers[MSG_C_PAIN1] = TcheckrikCinematicActionMsgHandler;
	classStatics[CID_TCHECKRIK].msgReceivers[MSG_C_RUN1] = TcheckrikCinematicActionMsgHandler;
	classStatics[CID_TCHECKRIK].msgReceivers[MSG_C_WALK1] = TcheckrikCinematicActionMsgHandler;

	res_info.numAnims = NUM_ANIMS;
	res_info.animations = animations;

	// Note that the name is different in the path.
	res_info.modelIndex = gi.modelindex("models/monsters/tcheckrik/male/tris.fm");

	sounds[SND_PAINM] = gi.soundindex("monsters/insect/painm.wav");
	sounds[SND_PAINF] = gi.soundindex("monsters/insect/painf.wav");
	sounds[SND_DIEM] = gi.soundindex("monsters/insect/deathm.wav");
	sounds[SND_DIEF] = gi.soundindex("monsters/insect/deathf.wav");
	sounds[SND_GIB] = gi.soundindex("monsters/insect/gib.wav");
	sounds[SND_SWIPE] = gi.soundindex("monsters/insect/swipe.wav");
	sounds[SND_SWIPEHITF] = gi.soundindex("monsters/plagueelf/hookhit.wav");
	sounds[SND_SWIPEHITW] = gi.soundindex("monsters/insect/swipehitw.wav");
	sounds[SND_SPELLM] = gi.soundindex("monsters/insect/spellm.wav");
	sounds[SND_SPELLM2] = gi.soundindex("monsters/insect/spellm2.wav");
	sounds[SND_SPLPWRUPF] = gi.soundindex("monsters/insect/splpwrupf.wav");
	sounds[SND_SPELLF] = gi.soundindex("monsters/insect/spellf.wav");
	sounds[SND_GROWLM1] = gi.soundindex("monsters/insect/growlm1.wav");
	sounds[SND_GROWLM2] = gi.soundindex("monsters/insect/growlm2.wav");
	sounds[SND_GROWLF1] = gi.soundindex("monsters/insect/growlf1.wav");
	sounds[SND_GROWLF2] = gi.soundindex("monsters/insect/growlf2.wav");
	sounds[SND_THUD] = gi.soundindex("monsters/insect/thud.wav");

	res_info.numSounds = NUM_SOUNDS;
	res_info.sounds = sounds;

	classStatics[CID_TCHECKRIK].resInfo = &res_info;
}

// QUAKED monster_tcheckrik_male (1 .5 0) (-16 -16 -32) (16 16 32) AMBUSH ASLEEP WALKING CINEMATIC BEAST_FODDER YELLOWJACKET 64 FIXED WANDER LEAD STALK COWARD EXTRA1 EXTRA2 EXTRA3 EXTRA4
// The insect.

// Spawnflags:
// AMBUSH		- Will not be woken up by other monsters or shots from player.
// ASLEEP		- Will not appear until triggered.
// WALKING		- Use WANDER instead.
// BEAST_FODDER	- Will run away if a Trial Beast is present.
// YELLOWJACKET	- Uses black and yellow skin, shoots a spread of three yellow stinger projectiles.
// WANDER		- Monster will wander around aimlessly (but follows buoys).
// LEAD			- Monster will try to cut you off when you're running and fighting him, works well if there are a few monsters in a group, half doing this, half not.
// STALK		- Monster will only approach and attack from behind. If you're facing the monster it will just stand there.
//				  Once the monster takes pain, however, it will stop this behaviour and attack normally.
// COWARD		- Monster starts off in flee mode (runs away from you when woken up).

// Variables:
// homebuoy					- Monsters will head to this buoy if they don't have an enemy ("homebuoy" should be targetname of the buoy you want them to go to).
// wakeup_target			- Monsters will fire this target the first time it wakes up (only once).
// pain_target				- Monsters will fire this target the first time it gets hurt (only once).
// mintel					- Monster intelligence - this basically tells a monster how many buoys away an enemy has to be for it to give up (default 32).
// melee_range				- How close the player has to be for the monster to go into melee. If this is zero, the monster will never melee.
//							  If it is negative, the monster will try to keep this distance from the player.
//							  If the monster has a backup, he'll use it if too close, otherwise, a negative value here means the monster will just stop
//							  running at the player at this distance (default 30).
//							 Examples:
//								melee_range = 60 - monster will start swinging it player is closer than 60.
//								melee_range = 0 - monster will never do a melee attack.
//								melee_range = -100 - monster will never do a melee attack and will back away (if it has that ability) when player gets too close.
// missile_range			- Maximum distance the player can be from the monster to be allowed to use it's ranged attack (default 512).
// min_missile_range		- Minimum distance the player can be from the monster to be allowed to use it's ranged attack (default 48).
// bypass_missile_chance	- Chance that a monster will NOT fire it's ranged attack, even when it has a clear shot. This, in effect, will make the monster
//							  come in more often than hang back and fire. A percentage (0 = always fire/never close in, 100 = never fire/always close in) - must be whole number (default 30).
// jump_chance				- Every time the monster has the opportunity to jump, what is the chance (out of 100) that he will... (100 = jump every time) - must be whole number (default 40).
// wakeup_distance			- How far (max) the player can be away from the monster before it wakes up. This means that if the monster can see the player,
//							  at what distance should the monster actually notice him and go for him (default 1024).
// NOTE: A value of zero will result in defaults, if you actually want zero as the value, use -1.
void SP_monster_tcheckrik_male(edict_t* self)
{
	if (irand(0, 1) == 1) // Made random again.
		self->spawnflags |= MSF_INSECT_YELLOWJACKET;

	if (self->spawnflags & MSF_WALKING)
	{
		self->spawnflags |= MSF_WANDER;
		self->spawnflags &= ~MSF_WALKING;
	}

	if (!M_WalkmonsterStart(self)) //mxd. M_Start -> M_WalkmonsterStart.
		return; // Failed initialization.

	self->msgHandler = DefaultMsgHandler;
	self->monsterinfo.dismember = TcheckrikDismember;
	self->touch = M_Touch;

	if (self->health == 0)
		self->health = TC_HEALTH_MALE;

	self->health = MonsterHealth(self->health);
	self->max_health = self->health;

	self->mass = MASS_TC_MALE;
	self->yaw_speed = 20.0f;

	self->solid = SOLID_BBOX;
	self->movetype = PHYSICSTYPE_STEP;
	VectorClear(self->knockbackvel);

	VectorCopy(STDMinsForClass[self->classID], self->mins);
	VectorCopy(STDMaxsForClass[self->classID], self->maxs);

	self->viewheight = (int)(self->maxs[2] * 0.8f);
	self->s.modelindex = (byte)gi.modelindex("models/monsters/tcheckrik/male/tris.fm");
	self->materialtype = MAT_INSECT;

	// All skins are even numbers, pain skins are skin + 1.
	if (self->spawnflags & MSF_INSECT_YELLOWJACKET)
		self->s.skinnum = 2;
	else
		self->s.skinnum = 0;

	if (self->s.scale == 0.0f)
	{
		self->s.scale = MODEL_SCALE;
		self->monsterinfo.scale = self->s.scale;
	}

	// Turn on/off the weapons that aren't used.
	self->s.fmnodeinfo[MESH__CROWN].flags |= FMNI_NO_DRAW;
	self->s.fmnodeinfo[MESH__RWINGS].flags |= FMNI_NO_DRAW;
	self->s.fmnodeinfo[MESH__LWINGS].flags |= FMNI_NO_DRAW;
	self->s.fmnodeinfo[MESH__STAFF].flags |= FMNI_NO_DRAW;
	self->s.fmnodeinfo[MESH__GEM].flags |= FMNI_NO_DRAW;
	self->s.fmnodeinfo[MESH__FEMHAND].flags |= FMNI_NO_DRAW;
	self->s.fmnodeinfo[MESH__CROWN].flags |= FMNI_NO_DRAW;

	if (self->spawnflags & MSF_INSECT_BEAST_FODDER)
		self->s.fmnodeinfo[MESH__MALEHAND].flags |= FMNI_NO_DRAW; //?

	//FIXME (somewhere: otherenemy should be more than just *one* kind.
	self->monsterinfo.otherenemyname = "monster_rat";

	// Override female's settings in m_stats.c.
	if (self->bypass_missile_chance == 0)
		self->bypass_missile_chance = 30;

	if (self->melee_range == 0.0f)
		self->melee_range = 48.0f;

	// Set up my mood function.
	MG_InitMoods(self);

	if (irand(0, 2) == 0)
		self->ai_mood_flags |= AI_MOOD_FLAG_PREDICT;

	if (self->spawnflags & MSF_WANDER)
	{
		QPostMessage(self, MSG_WALK, PRI_DIRECTIVE, NULL);
	}
	else if (self->spawnflags & MSF_INSECT_CINEMATIC)
	{
		self->monsterinfo.c_mode = true;
		QPostMessage(self, MSG_C_IDLE1, PRI_DIRECTIVE, "iiige", 0, 0, 0, NULL, NULL);
	}
	else
	{
		QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
	}

	gi.CreateEffect(&self->s, FX_I_EFFECTS, CEF_OWNERS_ORIGIN, vec3_origin, "bv", FX_I_RREFS, vec3_origin);
}

// QUAKED monster_tcheckrik_female (1 .5 0) (-16 -16 -32) (16 16 32) AMBUSH ASLEEP WALKING CINEMATIC 16 ALTERNATE 64 FIXED WANDER MELEE_LEAD STALK COWARD EXTRA1 EXTRA2 EXTRA3 EXTRA4
// The insect.

// Spawnflags:
// AMBUSH		- Will not be woken up by other monsters or shots from player.
// ASLEEP		- Will not appear until triggered.
// WALKING		- Use WANDER instead.
// WANDER		- Monster will wander around aimlessly (but follows buoys).
// ALTERNATE	- Fires alternate projectile, more damage, not multiple though.
// MELEE_LEAD	- Monster will try to cut you off when you're running and fighting him, works well if there are a few monsters in a group, half doing this, half not.
// STALK		- Monster will only approach and attack from behind. If you're facing the monster it will just stand there.
//				  Once the monster takes pain, however, it will stop this behaviour and attack normally.
// COWARD		- Monster starts off in flee mode (runs away from you when woken up).

// Variables:
// homebuoy					- Monsters will head to this buoy if they don't have an enemy ("homebuoy" should be targetname of the buoy you want them to go to).
// wakeup_target			- Monsters will fire this target the first time it wakes up (only once).
// pain_target				- Monsters will fire this target the first time it gets hurt (only once).
// mintel					- Monster intelligence - this basically tells a monster how many buoys away an enemy has to be for it to give up (default 32).
// melee_range				- How close the player has to be for the monster to go into melee. If this is zero, the monster will never melee.
//							  If it is negative, the monster will try to keep this distance from the player.
//							  If the monster has a backup, he'll use it if too close, otherwise, a negative value here means the monster will just stop
//							  running at the player at this distance (default -72).
//							 Examples:
//								melee_range = 60 - monster will start swinging it player is closer than 60.
//								melee_range = 0 - monster will never do a melee attack.
//								melee_range = -100 - monster will never do a melee attack and will back away (if it has that ability) when player gets too close.
// missile_range			- Maximum distance the player can be from the monster to be allowed to use it's ranged attack (default 512).
// min_missile_range		- Minimum distance the player can be from the monster to be allowed to use it's ranged attack (default 48).
// bypass_missile_chance	- Chance that a monster will NOT fire it's ranged attack, even when it has a clear shot. This, in effect, will make the monster
//							  come in more often than hang back and fire. A percentage (0 = always fire/never close in, 100 = never fire/always close in) - must be whole number (default 0).
// jump_chance				- Every time the monster has the opportunity to jump, what is the chance (out of 100) that he will... (100 = jump every time) - must be whole number (default 40).
// wakeup_distance			- How far (max) the player can be away from the monster before it wakes up. This means that if the monster can see the player,
//							  at what distance should the monster actually notice him and go for him (default 1024).
// NOTE: A value of zero will result in defaults, if you actually want zero as the value, use -1.
void SP_monster_tcheckrik_female(edict_t* self)
{
	if (self->spawnflags & MSF_WALKING)
	{
		self->spawnflags |= MSF_WANDER;
		self->spawnflags &= ~MSF_WALKING;
	}

	if (!M_WalkmonsterStart(self)) //mxd. M_Start -> M_WalkmonsterStart.
		return; // Failed initialization.

	self->msgHandler = DefaultMsgHandler;
	self->monsterinfo.dismember = TcheckrikDismember;
	self->touch = M_Touch;

	if (self->health == 0)
		self->health = TC_HEALTH_FEMALE;

	self->health = MonsterHealth(self->health);
	self->max_health = self->health;

	self->mass = MASS_TC_FEMALE;
	self->yaw_speed = 20.0f;

	self->solid = SOLID_BBOX;
	self->movetype = PHYSICSTYPE_STEP;
	VectorClear(self->knockbackvel);

	VectorCopy(STDMinsForClass[self->classID], self->mins);
	VectorCopy(STDMaxsForClass[self->classID], self->maxs);

	self->viewheight = (int)(self->maxs[2] * 0.4f);
	self->s.modelindex = (byte)gi.modelindex("models/monsters/tcheckrik/female/tris.fm");
	self->materialtype = MAT_INSECT;

	// All skins are even numbers, pain skins are skin + 1.
	self->s.skinnum = 0;

	if (self->s.scale == 0.0f)
	{
		self->s.scale = MODEL_SCALE;
		self->monsterinfo.scale = self->s.scale;
	}

	// Turn on/off the weapons that aren't used.
	self->s.fmnodeinfo[MESH__LMANDIBLE].flags |= FMNI_NO_DRAW;
	self->s.fmnodeinfo[MESH__RMANDIBLE].flags |= FMNI_NO_DRAW;
	self->s.fmnodeinfo[MESH__L2NDARM].flags |= FMNI_NO_DRAW;
	self->s.fmnodeinfo[MESH__R2NDARM].flags |= FMNI_NO_DRAW;
	self->s.fmnodeinfo[MESH__MALEHAND].flags |= FMNI_NO_DRAW; //?
	self->s.fmnodeinfo[MESH__SPEAR].flags |= FMNI_NO_DRAW; //?

	//FIXME (somewhere: otherenemy should be more than just *one* kind.
	self->monsterinfo.otherenemyname = "monster_rat";

	// Set up my mood function
	MG_InitMoods(self);

	if (irand(0, 2) == 0)
		self->ai_mood_flags |= AI_MOOD_FLAG_PREDICT;

	QPostMessage(self, ((self->spawnflags & MSF_WANDER) ? MSG_WALK : MSG_STAND), PRI_DIRECTIVE, NULL);
	self->monsterinfo.aiflags |= AI_NO_MELEE;
	self->svflags |= SVF_WAIT_NOTSOLID;

	gi.CreateEffect(&self->s, FX_I_EFFECTS, CEF_OWNERS_ORIGIN, vec3_origin, "bv", FX_I_RREFS, vec3_origin);

	if (self->spawnflags & MSF_INSECT_ALTERNATE)
		COLOUR_SET(self->s.color, 250, 150, 100);
}