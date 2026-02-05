//
// m_ogle.c
//
// Copyright 1998 Raven Software
//

#include "m_ogle.h"
#include "m_ogle_shared.h"
#include "m_ogle_anim.h"
#include "m_ogle_moves.h"
#include "c_ai.h"
#include "g_debris.h" //mxd
#include "g_DefaultMessageHandler.h"
#include "g_monster.h"
#include "g_obj.h" //mxd
#include "mg_guide.h" //mxd
#include "m_stats.h"
#include "Random.h"
#include "Utilities.h"
#include "Vector.h"
#include "g_local.h"

#pragma region ========================== Ogle base info ==========================

static const animmove_t* animations[NUM_ANIMS] =
{
	&ogle_move_walk1,
	&ogle_move_push1,
	&ogle_move_push2,
	&ogle_move_push3,
	&ogle_move_stand1,
	&ogle_move_work1,
	&ogle_move_work2,
	&ogle_move_work3,
	&ogle_move_work4,
	&ogle_move_work5,
	&ogle_move_pain1,
	&ogle_move_pain2,
	&ogle_move_pain3,
	&ogle_move_rest1_trans,
	&ogle_move_rest1_wipe,
	&ogle_move_rest1,
	&ogle_move_rest2_wipe,
	&ogle_move_rest3_wipe,
	&ogle_move_rest4_trans,
	&ogle_move_rest4_trans2,
	&ogle_move_rest4,
	&ogle_move_celebrate1,
	&ogle_move_celebrate2,
	&ogle_move_celebrate3_trans,
	&ogle_move_celebrate3,
	&ogle_move_celebrate4_trans,
	&ogle_move_celebrate4,
	&ogle_move_celebrate5_trans,
	&ogle_move_celebrate5,
	&ogle_move_charge1,
	&ogle_move_charge2,
	&ogle_move_charge3,
	&ogle_move_charge4,
	&ogle_move_charge5,
	&ogle_move_attack1,
	&ogle_move_attack2,
	&ogle_move_attack3,
	&ogle_move_death1,
	&ogle_move_death2,

	&ogle_c_move_action1,
	&ogle_c_move_action2,
	&ogle_c_move_action3,
	&ogle_c_move_action4,
	&ogle_c_move_action5,
	&ogle_c_move_action6,
	&ogle_c_move_action7,
	&ogle_c_move_action8,
	&ogle_c_move_action9,
	&ogle_c_move_action10,
	&ogle_c_move_action11,
	&ogle_c_move_action12,
	&ogle_c_move_action13,
	&ogle_c_move_action14,
	&ogle_c_move_action15,
	&ogle_c_move_attack1,
	&ogle_c_move_attack2,
	&ogle_c_move_attack3,
	&ogle_c_move_death1,
	&ogle_c_move_death2,
	NULL, // ANIM_C_GIB1
	&ogle_c_move_idle1,
	&ogle_c_move_idle2,
	&ogle_c_move_idle3,
	&ogle_c_move_idle4,
	&ogle_c_move_idle5,
	&ogle_c_move_idle6,
	&ogle_c_move_pain1,
	&ogle_c_move_pain2,
	&ogle_c_move_pain3,
	&ogle_move_rest1,
	&ogle_c_move_trans1,
	&ogle_c_move_trans2,
	&ogle_c_move_trans3,
	&ogle_c_move_trans4,
	&ogle_c_move_trans5,
	&ogle_c_move_trans6,
	&ogle_c_move_walk1,
	&ogle_c_move_walk2,
	&ogle_c_move_walk3,
	&ogle_c_move_walk4,
};

static int sounds[NUM_SOUNDS];

#pragma endregion

#pragma region ========================== obj_corpse_ogle ==========================

// QUAKED obj_corpse_ogle (1 .5 0) (-30 -12 -2) (30 12 2) OF_PUSHING OF_PICK_UP OF_PICK_DOWN x x OF_HAMMER_UP OF_HAMMER_DOWN
// A dead ogle.
// Variables:
// style - Ogle skin (0 - damaged, 1 - normal. Default 0).
// Spawnflags:
// Same as monster_ogle.
void SP_obj_corpse_ogle(edict_t* self)
{
	self->s.origin[2] += 22.0f;

	VectorSet(self->mins, -30.0f, -30.0f, -2.0f);
	VectorSet(self->maxs, 30.0f, 30.0f, 8.0f);

	self->s.modelindex = (byte)gi.modelindex("models/monsters/ogle/tris.fm");
	self->s.frame = FRAME_deatha14;	// Ths is the reason the function can't be put in g_obj.c.

	self->style = ((self->style == 0) ? 1 : 0); // Set the skinnum correctly.
	self->svflags |= SVF_DEADMONSTER; // Doesn't block walking.

	if (self->monsterinfo.ogleflags & OF_PUSHING)
	{
		self->s.fmnodeinfo[MESH__NAIL].flags |= FMNI_NO_DRAW;
		self->s.fmnodeinfo[MESH__HAMMER].flags |= FMNI_NO_DRAW;
		self->s.fmnodeinfo[MESH__PICK].flags |= FMNI_NO_DRAW;
	}
	else if (self->monsterinfo.ogleflags & OF_PICK_UP)
	{
		self->s.fmnodeinfo[MESH__NAIL].flags |= FMNI_NO_DRAW;
		self->s.fmnodeinfo[MESH__HAMMER].flags |= FMNI_NO_DRAW;
	}
	else if (self->monsterinfo.ogleflags & OF_PICK_DOWN)
	{
		SetAnim(self, ANIM_WORK5);
		self->s.fmnodeinfo[MESH__NAIL].flags |= FMNI_NO_DRAW;
		self->s.fmnodeinfo[MESH__HAMMER].flags |= FMNI_NO_DRAW;
	}
	else if (self->monsterinfo.ogleflags & (OF_HAMMER_UP | OF_HAMMER_DOWN))
	{
		self->s.fmnodeinfo[MESH__NAIL].flags |= FMNI_NO_DRAW;
		self->s.fmnodeinfo[MESH__PICK].flags |= FMNI_NO_DRAW;
	}
	else // OF_CHISEL_UP, OF_CHISEL_DOWN, default.
	{
		self->s.fmnodeinfo[MESH__PICK].flags |= FMNI_NO_DRAW;
	}

	ObjectInit(self, 40, 80, MAT_FLESH, SOLID_BBOX);
}

#pragma endregion

#pragma region ========================== Utility functions =========================

// Cast off the tools which have oppressed the ogle people for centuries... or something.
static void OgleDropTools(edict_t* self) //mxd. Named 'ogle_cast_off_tools_of_oppression' in original logic.
{
	if (!(self->s.fmnodeinfo[MESH__NAIL].flags & FMNI_NO_DRAW))
	{
		// Cast off the hammer and nail.
		self->s.fmnodeinfo[MESH__NAIL].flags |= FMNI_NO_DRAW;
		ThrowWeapon(self, vec3_origin, BPN_NAIL, 0, FRAME_prtfly);

		self->s.fmnodeinfo[MESH__HAMMER].flags |= FMNI_NO_DRAW;
		self->s.fmnodeinfo[MESH__HANDLE].flags |= FMNI_NO_DRAW;

		ThrowWeapon(self, vec3_origin, BPN_HAMMER | BPN_HANDLE, 0, FRAME_prtfly);
	}
	else if (!(self->s.fmnodeinfo[MESH__PICK].flags & FMNI_NO_DRAW))
	{
		// Cast off the pick.
		self->s.fmnodeinfo[MESH__PICK].flags |= FMNI_NO_DRAW;
		self->s.fmnodeinfo[MESH__HANDLE].flags |= FMNI_NO_DRAW;

		ThrowWeapon(self, vec3_origin, BPN_PICK | BPN_HANDLE, 0, FRAME_prtfly);
	}
	else if (!(self->s.fmnodeinfo[MESH__HAMMER].flags & FMNI_NO_DRAW))
	{
		// Cast off the hammer.
		self->s.fmnodeinfo[MESH__HAMMER].flags |= FMNI_NO_DRAW;
		self->s.fmnodeinfo[MESH__HANDLE].flags |= FMNI_NO_DRAW;

		ThrowWeapon(self, vec3_origin, BPN_HAMMER | BPN_HANDLE, 0, FRAME_prtfly);
	}

	self->monsterinfo.aiflags |= AI_NO_MELEE;
}

// Ogle Singing Technology (tm) All Right Reserved.
static void OgleSing(edict_t* self) //mxd. Named 'ogle_sing' in original logic.
{
	if (self->monsterinfo.awake)
		return;

	edict_t* ogle = NULL;

	switch (self->noise_index)
	{
		case 0:
			while ((ogle = G_Find(ogle, FOFS(target), self->target)) != NULL)
			{
				if (!ogle->monsterinfo.awake)
					gi.sound(ogle, CHAN_VOICE, sounds[SND_CHORUS1], 0.25f, ATTN_NORM, 0.0f);
			}
			self->monsterinfo.ogle_sing_time = level.time + 16.0f;
			break;

		case 1:
			gi.sound(self, CHAN_VOICE, sounds[SND_SOLO1], 1.0f, ATTN_NORM, 0.0f);
			self->monsterinfo.ogle_sing_time = level.time + 4.0f;
			break;

		case 2:
			while ((ogle = G_Find(ogle, FOFS(target), self->target)) != NULL)
			{
				if (!ogle->monsterinfo.awake)
					gi.sound(ogle, CHAN_VOICE, sounds[SND_CHORUS2], 0.25f, ATTN_NORM, 0.0f);
			}
			self->monsterinfo.ogle_sing_time = level.time + 3.0f;
			break;

		case 3:
			gi.sound(self, CHAN_VOICE, sounds[SND_SOLO2], 1.0f, ATTN_NORM, 0.0f);
			self->monsterinfo.ogle_sing_time = level.time + 4.0f;
			break;

		case 4:
			while ((ogle = G_Find(ogle, FOFS(target), self->target)) != NULL)
			{
				if (!ogle->monsterinfo.awake)
					gi.sound(ogle, CHAN_VOICE, sounds[SND_CHORUS3], 0.25f, ATTN_NORM, 0.0f);
			}
			self->monsterinfo.ogle_sing_time = level.time + 4.0f;
			break;
	}

	self->noise_index++;
	self->noise_index %= 5;
}

qboolean OgleFindTarget(edict_t* self) //mxd. Named 'ogle_findtarget' in original logic.
{
	// Take down weak overlords.
	edict_t* found = NULL;
	while ((found = FindInRadius(found, self->s.origin, 1024.0f)) != NULL)
	{
		if (found->classID == CID_SERAPH_OVERLORD && found->health > 0 && (found->health < SERAPH_HEALTH / 2 || found->ai_mood == AI_MOOD_FLEE))
		{
			self->enemy = found;
			AI_FoundTarget(self, false);

			return true;
		}
	}

	// Help out other ogles.
	found = NULL;
	while ((found = FindInRadius(found, self->s.origin, 1024.0f)) != NULL)
	{
		if (found->classID == CID_OGLE && found->health > 0 && found != self && found->enemy != NULL && found->enemy->health > 0)
		{
			if (found->enemy->client != NULL)
			{
				found->enemy = NULL;
			}
			else
			{
				self->enemy = found->enemy;
				AI_FoundTarget(self, false);

				return true;
			}
		}
	}

	return false;
}

static void OgleCelebrate(edict_t* self) //mxd. Added to reduce code duplication.
{
	switch (irand(0, 4))
	{
		default:
		case 0: SetAnim(self, ANIM_CELEBRATE1); break;
		case 1: SetAnim(self, ANIM_CELEBRATE2); break;
		case 2: SetAnim(self, ANIM_CELEBRATE3); break;
		case 3: SetAnim(self, ANIM_CELEBRATE4); break;
		case 4: SetAnim(self, ANIM_CELEBRATE5); break;
	}
}

static void OgleCharge(edict_t* self) //mxd. Added to reduce code duplication.
{
	switch (irand(0, 4))
	{
		default:
		case 0: SetAnim(self, ANIM_CHARGE1); break;
		case 1: SetAnim(self, ANIM_CHARGE2); break;
		case 2: SetAnim(self, ANIM_CHARGE3); break;
		case 3: SetAnim(self, ANIM_CHARGE4); break;
		case 4: SetAnim(self, ANIM_CHARGE5); break;
	}
}

static void OgleTrySetAsSongLeader(edict_t* self) //mxd. Named 'ogle_check_leadsong' in original logic.
{
	edict_t* ogle = NULL;
	while ((ogle = G_Find(ogle, FOFS(target), self->target)) != NULL)
		if (ogle->monsterinfo.ogleflags & OF_SONG_LEADER)
			return;

	self->monsterinfo.ogleflags |= OF_SONG_LEADER;
}

#pragma endregion

#pragma region ========================== Message handlers ==========================

static void OgleCinematicActionMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'ogle_c_anims' in original logic.
{
	int curr_anim;

	ReadCinematicMessage(self, msg);
	self->monsterinfo.c_anim_flag = 0;

	switch (msg->ID)
	{
		case MSG_C_ACTION1:
			self->monsterinfo.c_anim_flag = C_ANIM_REPEAT;
			curr_anim = ANIM_C_ACTION1;
			break;

		case MSG_C_ACTION2:
			self->monsterinfo.c_anim_flag = C_ANIM_REPEAT;
			curr_anim = ANIM_C_ACTION2;
			break;

		case MSG_C_ACTION3:
			self->monsterinfo.c_anim_flag = C_ANIM_REPEAT;
			curr_anim = ANIM_C_ACTION3;
			break;

		case MSG_C_ACTION4:
			self->monsterinfo.c_anim_flag = C_ANIM_REPEAT;
			curr_anim = ANIM_C_ACTION4;
			break;

		case MSG_C_ACTION5:
			self->monsterinfo.c_anim_flag = C_ANIM_REPEAT;
			curr_anim = ANIM_C_ACTION5;
			break;

		case MSG_C_ACTION6:
			self->monsterinfo.c_anim_flag = C_ANIM_REPEAT;
			curr_anim = ANIM_C_ACTION6;
			break;

		case MSG_C_ACTION7:
			self->monsterinfo.c_anim_flag = C_ANIM_REPEAT;
			curr_anim = ANIM_C_ACTION7;
			break;

		case MSG_C_ACTION8:
			self->monsterinfo.c_anim_flag = C_ANIM_REPEAT;
			curr_anim = ANIM_C_ACTION8;
			break;

		case MSG_C_ACTION9:
			self->monsterinfo.c_anim_flag = C_ANIM_REPEAT;
			curr_anim = ANIM_C_ACTION9;
			break;

		case MSG_C_ACTION10:
			self->monsterinfo.c_anim_flag = C_ANIM_REPEAT;
			curr_anim = ANIM_C_ACTION10;
			break;

		case MSG_C_ACTION11:
			self->monsterinfo.c_anim_flag = C_ANIM_MOVE;
			curr_anim = ANIM_C_ACTION11;
			break;

		case MSG_C_ACTION12:
			self->monsterinfo.c_anim_flag = C_ANIM_MOVE;
			curr_anim = ANIM_C_ACTION12;
			break;

		case MSG_C_ACTION13:
			self->monsterinfo.c_anim_flag = C_ANIM_MOVE;
			curr_anim = ANIM_C_ACTION13;
			break;

		case MSG_C_ACTION14:
			self->monsterinfo.c_anim_flag = C_ANIM_MOVE;
			curr_anim = ANIM_C_ACTION14;
			break;

		case MSG_C_ACTION15:
			self->monsterinfo.c_anim_flag = C_ANIM_MOVE;
			curr_anim = ANIM_C_ACTION15;
			break;

		case MSG_C_ATTACK1:
			self->monsterinfo.c_anim_flag = C_ANIM_MOVE;
			curr_anim = ANIM_C_ATTACK1;
			break;

		case MSG_C_ATTACK2:
			self->monsterinfo.c_anim_flag = C_ANIM_REPEAT;
			curr_anim = ANIM_C_ATTACK2;
			break;

		case MSG_C_ATTACK3:
			self->monsterinfo.c_anim_flag = C_ANIM_REPEAT;
			curr_anim = ANIM_C_ATTACK3;
			break;

		case MSG_C_DEATH1:
			self->monsterinfo.c_anim_flag = C_ANIM_DONE;
			curr_anim = ANIM_C_DEATH1;
			break;

		case MSG_C_DEATH2:
			self->monsterinfo.c_anim_flag = C_ANIM_DONE;
			curr_anim = ANIM_C_DEATH2;
			break;

		case MSG_C_IDLE1:
			self->monsterinfo.c_anim_flag = (C_ANIM_REPEAT | C_ANIM_IDLE);
			curr_anim = ANIM_C_IDLE1;
			break;

		case MSG_C_IDLE2:
			self->monsterinfo.c_anim_flag = C_ANIM_REPEAT;
			curr_anim = ANIM_C_IDLE2;
			break;

		case MSG_C_IDLE3:
			self->monsterinfo.c_anim_flag = C_ANIM_REPEAT;
			curr_anim = ANIM_C_IDLE3;
			break;

		case MSG_C_IDLE4:
			self->monsterinfo.c_anim_flag = C_ANIM_REPEAT;
			curr_anim = ANIM_C_IDLE4;
			break;

		case MSG_C_IDLE5:
			self->monsterinfo.c_anim_flag = C_ANIM_REPEAT;
			curr_anim = ANIM_C_IDLE5;
			break;

		case MSG_C_IDLE6:
			self->monsterinfo.c_anim_flag = C_ANIM_REPEAT;
			curr_anim = ANIM_C_IDLE6;
			break;

		case MSG_C_PAIN1:
			self->monsterinfo.c_anim_flag = C_ANIM_REPEAT;
			curr_anim = ANIM_C_PAIN1;
			break;

		case MSG_C_PAIN2:
			self->monsterinfo.c_anim_flag = C_ANIM_REPEAT;
			curr_anim = ANIM_C_PAIN2;
			break;

		case MSG_C_PAIN3:
			self->monsterinfo.c_anim_flag = C_ANIM_REPEAT;
			curr_anim = ANIM_C_PAIN3;
			break;

		case MSG_C_THINKAGAIN: // Think for yourself, little ogle man.
			curr_anim = ANIM_C_THINKAGAIN;
			self->monsterinfo.c_mode = false;
			break;

		case MSG_C_TRANS1:
			self->monsterinfo.c_anim_flag = C_ANIM_REPEAT;
			curr_anim = ANIM_C_TRANS1;
			break;

		case MSG_C_TRANS2:
			self->monsterinfo.c_anim_flag = C_ANIM_REPEAT;
			curr_anim = ANIM_C_TRANS2;
			break;

		case MSG_C_TRANS3:
			self->monsterinfo.c_anim_flag = C_ANIM_REPEAT;
			curr_anim = ANIM_C_TRANS3;
			break;

		case MSG_C_TRANS4:
			self->monsterinfo.c_anim_flag = C_ANIM_REPEAT;
			curr_anim = ANIM_C_TRANS4;
			break;

		case MSG_C_TRANS5:
			self->monsterinfo.c_anim_flag = C_ANIM_REPEAT;
			curr_anim = ANIM_C_TRANS5;
			break;

		case MSG_C_TRANS6:
			self->monsterinfo.c_anim_flag = C_ANIM_REPEAT;
			curr_anim = ANIM_C_TRANS6;
			break;

		case MSG_C_WALK1:
			self->monsterinfo.c_anim_flag = C_ANIM_MOVE;
			curr_anim = ANIM_C_WALK1;
			break;

		case MSG_C_WALK2:
			self->monsterinfo.c_anim_flag = C_ANIM_MOVE;
			curr_anim = ANIM_C_WALK2;
			break;

		case MSG_C_WALK3:
			self->monsterinfo.c_anim_flag = C_ANIM_MOVE;
			curr_anim = ANIM_C_WALK3;
			break;

		case MSG_C_WALK4:
			self->monsterinfo.c_anim_flag = C_ANIM_MOVE;
			curr_anim = ANIM_C_WALK4;
			break;

		default:
			return; //mxd. 'break' in original logic. Let's avoid using uninitialized curr_anim var.
	}

	SetAnim(self, curr_anim);
}

static void OglePainMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'ogle_pain' in original logic.
{
	edict_t* target;
	edict_t* attacker;
	qboolean force_pain;
	int damage;
	int temp;
	G_ParseMsgParms(msg, "eeiii", &target, &attacker, &force_pain, &damage, &temp);

	self->mood_think = OgleMoodThink;

	if (attacker != NULL && !AI_IsInfrontOf(self, attacker))
		SetAnim(self, ANIM_PAIN3);
	else
		SetAnim(self, irand(ANIM_PAIN1, ANIM_PAIN3));

	gi.sound(self, CHAN_VOICE, sounds[irand(SND_PAIN1, SND_PAIN2)], 1.0f, ATTN_NORM, 0.0f);
}

static void OgleDeathPainMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'ogle_death_pain' in original logic.
{
	if (self->health <= -80) // Gib death.
		BecomeDebris(self);
	else if (msg != NULL)
		DismemberMsgHandler(self, msg);
}

static void OgleDeathMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'ogle_death' in original logic.
{
	edict_t* target;
	edict_t* inflictor;
	edict_t* attacker;
	float damage;
	G_ParseMsgParms(msg, "eeei", &target, &inflictor, &attacker, &damage);

	M_StartDeath(self, ANIM_DEATH1);
	OgleDropTools(self);

	if (self->health < -80) // Gib death.
		return;

	if (self->health < -10)
	{
		SetAnim(self, ANIM_DEATH2);

		self->elasticity = 1.2f;
		self->friction = 0.8f;

		vec3_t dir;
		VectorNormalize2(target->velocity, dir);

		VectorScale(dir, 300.0f, self->velocity);
		self->velocity[2] = flrand(200.0f, 250.0f); //mxd. irand() in original logic.
	}
	else
	{
		SetAnim(self, ANIM_DEATH1);
	}

	gi.sound(self, CHAN_BODY, sounds[SND_DEATH], 1.0f, ATTN_NORM, 0.0f);
}

// Classic melee function (fear the ogles).
static void OgleMeleeMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'ogle_melee' in original logic.
{
	if (self->enemy == NULL)
		return;

	// Don't attack players.
	if (self->enemy->client != NULL)
	{
		self->enemy = NULL;
		return; //BUGFIX: mxd. Original logic doesn't return here.
	}

	if (self->enemy->health <= 0 && !OgleFindTarget(self))
	{
		self->enemy = NULL;
		self->goalentity = NULL;
		self->ai_mood = AI_MOOD_PURSUE;

		OgleDropTools(self);
		OgleCelebrate(self); //mxd
	}
	else
	{
		SetAnim(self, ANIM_ATTACK1);
	}
}

// Do a little dance... make a little... oh, sorry.
static void OgleStandMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'ogle_stand1' in original logic.
{
	if (self->monsterinfo.awake)
		OgleCelebrate(self); //mxd
}

// Classic run-attack function, who thought mortal combat could be so cute?
static void OgleRunMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'ogle_run1' in original logic.
{
	if (self->enemy != NULL && self->enemy->client != NULL)
		self->enemy = NULL;

	if (self->enemy != NULL && self->enemy->health <= 0)
	{
		if (!OgleFindTarget(self))
		{
			self->enemy = NULL;
			self->goalentity = NULL;
			self->ai_mood = AI_MOOD_PURSUE;

			OgleDropTools(self);
			OgleCelebrate(self); //mxd
		}

		return;
	}

	if (self->enemy != NULL)
	{
		const float dist = M_DistanceToTarget(self, self->enemy);

		if (dist < 40.0f) // Close enough to swing, not necessarily hit.
		{
			SetAnim(self, ANIM_ATTACK1);
		}
		else if (dist < 100.0f) // Close enough to swing, not necessarily hit.
		{
			const vec3_t start = VEC3_INITA(self->s.origin, 0.0f, 0.0f, (float)self->viewheight);
			const vec3_t end = VEC3_INITA(self->enemy->s.origin, 0.0f, 0.0f, (float)self->enemy->viewheight);

			trace_t trace;
			gi.trace(start, self->mins, self->maxs, end, self, MASK_MONSTERSOLID, &trace); //mxd. Original logic uses { self->mins[0], self->mins[1], self->maxs[2] / 2.0f } instead of self->mins.

			if (trace.ent == self->enemy)
				SetAnim(self, irand(ANIM_ATTACK2, ANIM_ATTACK3));
			else
				OgleCharge(self); //mxd //TODO: skip when already charging?
		}
		else if (self->curAnimID < ANIM_CHARGE1 || self->curAnimID > ANIM_CHARGE5) // If not already charging, CHAAARGE!!!
		{
			OgleCharge(self); //mxd
		}

		return;
	}

	G_PostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
}

#pragma endregion

#pragma region ========================== Edict callbacks ===========================

void OgleMoodThink(edict_t* self) //mxd. Named 'ogle_mood_think' in original logic.
{
	if (self->enemy == NULL)
	{
		if (self->ogle_overlord != NULL && self->ogle_overlord->health > 0 && (self->ogle_overlord->health < SERAPH_HEALTH / 2 || self->ogle_overlord->ai_mood == AI_MOOD_FLEE))
		{
			gi.sound(self, CHAN_BODY, sounds[irand(SND_ENRAGE1, SND_ENRAGE2)], 1.0f, ATTN_NORM, 0.0f);
			self->enemy = self->ogle_overlord;
			self->ai_mood = AI_MOOD_PURSUE;
		}
		else if (self->ai_mood == AI_MOOD_NORMAL && irand(0, 100) > 50 && self->monsterinfo.attack_finished < level.time)
		{
			self->monsterinfo.attack_finished = level.time + 45.0f;
			self->ai_mood = AI_MOOD_REST;
		}

		return;
	}

	if (self->ai_mood == AI_MOOD_WANDER)
		self->ai_mood = AI_MOOD_PURSUE;

	if (self->monsterinfo.aiflags & AI_COWARD)
		self->ai_mood = AI_MOOD_FLEE;

	if (!MG_CheckClearPathToEnemy(self) || (self->monsterinfo.aiflags & AI_NO_MELEE) || self->ai_mood == AI_MOOD_FLEE)
	{
		MG_Pathfind(self, false);
		return;
	}

	self->monsterinfo.searchType = SEARCH_COMMON;

	vec3_t diff;
	VectorSubtract(self->s.origin, self->enemy->s.origin, diff);

	if (diff[2] <= 40.0f) //TODO: what if diff[2] is negative? Should fabsf()?
		diff[2] = 0.0f;

	const float dist = VectorLength(diff) - self->enemy->maxs[0];

	// Far enough to run after.
	if (dist > self->melee_range || (self->monsterinfo.aiflags & AI_FLEE))
	{
		self->ai_mood = AI_MOOD_PURSUE;
	}
	else // Close enough to attack.
	{
		self->ai_mood = AI_MOOD_ATTACK;
		self->ai_mood_flags |= AI_MOOD_FLAG_MELEE;
		self->ai_mood_flags &= ~AI_MOOD_FLAG_MISSILE;
	}
}

// The ogle's been yelled at or struck by the overlord, get back to work!
void OgleUse(edict_t* self, edict_t* other, edict_t* activator) //mxd. Named 'ogle_use' in original logic.
{
	switch (self->curAnimID)
	{
		case ANIM_REST1:
			SetAnim(self, ANIM_PAIN2);
			break;

		case ANIM_REST4:
			SetAnim(self, ANIM_PAIN3);
			break;

		default:
			SetAnim(self, ANIM_PAIN2);
			break;
	}

	self->ai_mood = AI_MOOD_NORMAL;
}

// When the ogle is spawned, he checks around to figure out who his tormentor is.
void OgleInitOverlordThink(edict_t* self) //mxd. Named 'ogle_init_overlord' in original logic.
{
	// Restore what we lost from monsterstart().
	self->use = OgleUse;
	self->think = M_WalkmonsterStartGo;
	self->nextthink = level.time + FRAMETIME; //mxd. Use define.

	edict_t* seraph = NULL;
	while ((seraph = G_Find(seraph, FOFS(targetname), self->target)) != NULL)
	{
		if (seraph->classID == CID_SERAPH_OVERLORD)
		{
			self->ogle_overlord = seraph;
			break;
		}
	}
}

static void OgleThrowArmUpperLeft(edict_t* self, const float damage, const qboolean dismember_ok) //mxd. Split from OgleDismember() to simplify logic.
{
	if (self->s.fmnodeinfo[MESH__LUPARM].flags & FMNI_NO_DRAW)
		return;

	if (dismember_ok)
	{
		vec3_t right;
		AngleVectors(self->s.angles, NULL, right, NULL);

		vec3_t gore_spot = { 0.0f, 0.0f,  self->maxs[2] * 0.3f };
		VectorMA(gore_spot, -8.0f, right, gore_spot);

		int throw_nodes = BPN_LUPARM;
		self->s.fmnodeinfo[MESH__LUPARM].flags |= FMNI_NO_DRAW;

		if (!(self->s.fmnodeinfo[MESH__L4ARM].flags & FMNI_NO_DRAW))
		{
			throw_nodes |= BPN_L4ARM;
			self->s.fmnodeinfo[MESH__L4ARM].flags |= FMNI_NO_DRAW;
		}

		if (!(self->s.fmnodeinfo[MESH__NAIL].flags & FMNI_NO_DRAW))
		{
			throw_nodes |= BPN_NAIL;
			self->s.fmnodeinfo[MESH__NAIL].flags |= FMNI_NO_DRAW;
		}

		ThrowBodyPart(self, gore_spot, throw_nodes, (int)damage, FRAME_prtfly);
	}
	else
	{
		self->s.fmnodeinfo[MESH__LUPARM].flags |= FMNI_USE_SKIN;
		self->s.fmnodeinfo[MESH__LUPARM].skin = self->s.skinnum + 1;
	}
}

static void OgleThrowArmLowerLeft(edict_t* self, const float damage, const qboolean dismember_ok) //mxd. Split from OgleDismember() to simplify logic.
{
	if (self->s.fmnodeinfo[MESH__L4ARM].flags & FMNI_NO_DRAW)
		return;

	if (dismember_ok)
	{
		vec3_t right;
		AngleVectors(self->s.angles, NULL, right, NULL);

		vec3_t gore_spot = { 0.0f, 0.0f,  self->maxs[2] * 0.3f };
		VectorMA(gore_spot, -8.0f, right, gore_spot);

		int throw_nodes = BPN_L4ARM;
		self->s.fmnodeinfo[MESH__L4ARM].flags |= FMNI_NO_DRAW;

		if (!(self->s.fmnodeinfo[MESH__NAIL].flags & FMNI_NO_DRAW))
		{
			throw_nodes |= BPN_NAIL;
			self->s.fmnodeinfo[MESH__NAIL].flags |= FMNI_NO_DRAW;
		}

		ThrowBodyPart(self, gore_spot, throw_nodes, (int)damage, FRAME_prtfly);
	}
	else
	{
		self->s.fmnodeinfo[MESH__L4ARM].flags |= FMNI_USE_SKIN;
		self->s.fmnodeinfo[MESH__L4ARM].skin = self->s.skinnum + 1;
	}
}

static void OgleThrowArmUpperRight(edict_t* self, const float damage, const qboolean dismember_ok) //mxd. Split from OgleDismember() to simplify logic.
{
	if (self->s.fmnodeinfo[MESH__RUPARM].flags & FMNI_NO_DRAW)
		return;

	if (dismember_ok)
	{
		vec3_t right;
		AngleVectors(self->s.angles, NULL, right, NULL);

		vec3_t gore_spot = { 0.0f, 0.0f,  self->maxs[2] * 0.3f };
		VectorMA(gore_spot, 8.0f, right, gore_spot);

		int throw_nodes = BPN_RUPARM;
		self->s.fmnodeinfo[MESH__RUPARM].flags |= FMNI_NO_DRAW;

		if (!(self->s.fmnodeinfo[MESH__R4ARM].flags & FMNI_NO_DRAW))
		{
			throw_nodes |= BPN_R4ARM;
			self->s.fmnodeinfo[MESH__R4ARM].flags |= FMNI_NO_DRAW;
		}

		if (!(self->s.fmnodeinfo[MESH__HAMMER].flags & FMNI_NO_DRAW))
		{
			throw_nodes |= BPN_HAMMER;
			self->s.fmnodeinfo[MESH__HAMMER].flags |= FMNI_NO_DRAW;
		}

		if (!(self->s.fmnodeinfo[MESH__HANDLE].flags & FMNI_NO_DRAW))
		{
			throw_nodes |= BPN_HANDLE;
			self->s.fmnodeinfo[MESH__HANDLE].flags |= FMNI_NO_DRAW;
		}

		if (!(self->s.fmnodeinfo[MESH__PICK].flags & FMNI_NO_DRAW))
		{
			throw_nodes |= BPN_PICK;
			self->s.fmnodeinfo[MESH__PICK].flags |= FMNI_NO_DRAW;
		}

		ThrowBodyPart(self, gore_spot, throw_nodes, (int)damage, FRAME_prtfly);

		self->monsterinfo.aiflags |= AI_NO_MELEE;
		self->monsterinfo.aiflags |= AI_COWARD;
	}
	else
	{
		self->s.fmnodeinfo[MESH__RUPARM].flags |= FMNI_USE_SKIN;
		self->s.fmnodeinfo[MESH__RUPARM].skin = self->s.skinnum + 1;
	}
}

static void OgleThrowArmLowerRight(edict_t* self, const float damage, const qboolean dismember_ok) //mxd. Split from OgleDismember() to simplify logic.
{
	if (self->s.fmnodeinfo[MESH__R4ARM].flags & FMNI_NO_DRAW)
		return;

	if (dismember_ok)
	{
		vec3_t right;
		AngleVectors(self->s.angles, NULL, right, NULL);

		vec3_t gore_spot = { 0.0f, 0.0f,  self->maxs[2] * 0.3f };
		VectorMA(gore_spot, 8.0f, right, gore_spot);

		int throw_nodes = BPN_R4ARM;
		self->s.fmnodeinfo[MESH__R4ARM].flags |= FMNI_NO_DRAW;

		if (!(self->s.fmnodeinfo[MESH__HAMMER].flags & FMNI_NO_DRAW))
		{
			throw_nodes |= BPN_HAMMER;
			self->s.fmnodeinfo[MESH__HAMMER].flags |= FMNI_NO_DRAW;
		}

		if (!(self->s.fmnodeinfo[MESH__HANDLE].flags & FMNI_NO_DRAW))
		{
			throw_nodes |= BPN_HANDLE;
			self->s.fmnodeinfo[MESH__HANDLE].flags |= FMNI_NO_DRAW;
		}

		if (!(self->s.fmnodeinfo[MESH__PICK].flags & FMNI_NO_DRAW))
		{
			throw_nodes |= BPN_PICK;
			self->s.fmnodeinfo[MESH__PICK].flags |= FMNI_NO_DRAW;
		}

		ThrowBodyPart(self, gore_spot, throw_nodes, (int)damage, FRAME_prtfly);

		self->monsterinfo.aiflags |= AI_NO_MELEE;
		self->monsterinfo.aiflags |= AI_COWARD;
	}
	else
	{
		self->s.fmnodeinfo[MESH__R4ARM].flags |= FMNI_USE_SKIN;
		self->s.fmnodeinfo[MESH__R4ARM].skin = self->s.skinnum + 1;
	}
}

void OgleDismember(edict_t* self, int damage, HitLocation_t hl) //mxd. Named 'ogle_dismember' in original logic.
{
	qboolean dismember_ok = false;

	if (hl & hl_MeleeHit)
	{
		dismember_ok = true;
		hl &= ~hl_MeleeHit;
	}

	if (hl <= hl_NoneSpecific || hl >= hl_Max) //mxd. 'hl > hl_Max' in original logic.
		return;

	switch (hl)
	{
		case hl_Head:
		case hl_TorsoFront:
		case hl_TorsoBack:
			self->s.fmnodeinfo[MESH__TORSO].flags |= FMNI_USE_SKIN;
			self->s.fmnodeinfo[MESH__TORSO].skin = self->s.skinnum + 1;
			break;

		case hl_ArmUpperLeft:
			OgleThrowArmUpperLeft(self, (float)damage, dismember_ok); //mxd
			break;

		case hl_ArmLowerLeft:
			OgleThrowArmLowerLeft(self, (float)damage, dismember_ok); //mxd
			break;

		case hl_ArmUpperRight:
			OgleThrowArmUpperRight(self, (float)damage, dismember_ok); //mxd
			break;

		case hl_ArmLowerRight:
			OgleThrowArmLowerRight(self, (float)damage, dismember_ok); //mxd
			break;

		case hl_LegUpperLeft:
		case hl_LegLowerLeft:
			self->s.fmnodeinfo[MESH__LLEG].flags |= FMNI_USE_SKIN;
			self->s.fmnodeinfo[MESH__LLEG].skin = self->s.skinnum + 1;
			break;

		case hl_LegUpperRight:
		case hl_LegLowerRight:
			self->s.fmnodeinfo[MESH__RLEG].flags |= FMNI_USE_SKIN;
			self->s.fmnodeinfo[MESH__RLEG].skin = self->s.skinnum + 1;
			break;

		default:
			break;
	}
}

void OgleStartPushUse(edict_t* self, edict_t* other, edict_t* activator) //mxd. Named 'ogle_start_push' in original logic.
{
	SetAnim(self, irand(ANIM_PUSH1, ANIM_PUSH3));
}

#pragma endregion

#pragma region ========================== Action functions ==========================

// High level AI interface.
void ogle_pause(edict_t* self)
{
	if (self->enemy == NULL)
	{
		if ((self->monsterinfo.ogleflags & OF_SONG_LEADER) && self->monsterinfo.ogle_sing_time < level.time)
			OgleSing(self);

		// If we're in pain, get back to work!
		if (self->curAnimID == ANIM_PAIN2 || self->curAnimID == ANIM_PAIN3)
			SetAnim(self, ANIM_WORK4);
		else if (self->curAnimID == ANIM_WORK1 || self->curAnimID == ANIM_WORK2) // Switch up the animation speeds.
			SetAnim(self, ((irand(1, 10) < 8) ? ANIM_WORK1 : ANIM_WORK2));
	}

	if (self->mood_think != NULL)
		self->mood_think(self);

	switch (self->ai_mood)
	{
		case AI_MOOD_ATTACK:
			G_PostMessage(self, MSG_MELEE, PRI_DIRECTIVE, NULL);
			break;

		case AI_MOOD_FLEE:
		case AI_MOOD_PURSUE:
		case AI_MOOD_NAVIGATE:
			if (self->enemy != NULL)
				G_PostMessage(self, MSG_RUN, PRI_DIRECTIVE, NULL);
			break;

		case AI_MOOD_WALK:
			G_PostMessage(self, MSG_WALK, PRI_DIRECTIVE, NULL);
			break;

		case AI_MOOD_STAND:
			G_PostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
			break;

		case AI_MOOD_REST:
			if (self->curAnimID != ANIM_REST1 && self->curAnimID != ANIM_REST1_WIPE &&
				self->curAnimID != ANIM_REST1_TRANS && self->curAnimID != ANIM_REST2_WIPE && self->curAnimID != ANIM_REST3_WIPE)
			{
				if (self->curAnimID == ANIM_WORK4 || self->curAnimID == ANIM_WORK5)
				{
					const int chance = irand(0, 100);

					if (chance < 30)
					{
						gi.sound(self, CHAN_BODY, sounds[SND_WIPE_BROW], 1.0f, ATTN_IDLE, 0.0f);
						SetAnim(self, ANIM_REST2_WIPE);
						self->ai_mood = AI_MOOD_NORMAL;
					}
					else if (chance < 60)
					{
						SetAnim(self, ANIM_REST1_TRANS);
					}
					else
					{
						SetAnim(self, ANIM_REST4_TRANS);
					}
				}
				else if (self->curAnimID == ANIM_WORK1 || self->curAnimID == ANIM_WORK2)
				{
					gi.sound(self, CHAN_BODY, sounds[SND_WIPE_BROW], 1.0f, ATTN_IDLE, 0.0f);
					SetAnim(self, ANIM_REST3_WIPE);
					self->ai_mood = AI_MOOD_NORMAL;
				}
			}
			break;

		default:
			break;
	}
}

// The ogle is resting, choose a few different possible things to do.
void ogle_rest(edict_t* self)
{
	ogle_pause(self);

	switch (self->curAnimID)
	{
		case ANIM_REST4_TRANS:
			SetAnim(self, ANIM_REST4);
			break;

		case ANIM_REST2_WIPE:
			SetAnim(self, ANIM_WORK4);
			break;

		case ANIM_REST3_WIPE:
			SetAnim(self, ANIM_WORK2);
			break;

		case ANIM_REST1_TRANS:
		case ANIM_REST1_WIPE:
			SetAnim(self, ANIM_REST1);
			break;

		case ANIM_REST1:
			if (irand(0, 100) < 20)
				SetAnim(self, ANIM_REST1_WIPE);
			break;

		default:
			break;
	}
}

// Check to do damage with the ogle's weapon.
void ogle_strike(edict_t* self)
{
	const vec3_t start_offset = { 0.0f, 16.0f, 8.0f };
	const vec3_t end_offset = { self->melee_range, 2.0f, 8.0f };

	// Purposely backwards.
	vec3_t blood_dir;
	VectorSubtract(start_offset, end_offset, blood_dir);
	VectorNormalize(blood_dir);

	const vec3_t mins = { -4.0f, -4.0f, -4.0f };
	const vec3_t maxs = {  4.0f,  4.0f,  4.0f };

	trace_t	trace;
	vec3_t direction;
	edict_t* victim = M_CheckMeleeLineHit(self, start_offset, end_offset, mins, maxs, &trace, direction);

	if (victim == NULL)
		return; //TODO: play swoosh sound?

	if (victim == self)
	{
		// Create a puff effect. //TODO: play swoosh sound? Also, when does this happen?
		gi.CreateEffect(NULL, FX_SPARKS, CEF_FLAG6, trace.endpos, "d", blood_dir);
	}
	else
	{
		// Hurt whatever we were whacking away at.
		const int snd_id = (!(self->s.fmnodeinfo[MESH__HAMMER].flags & FMNI_NO_DRAW) ? SND_HAMMER_FLESH : SND_PICK_FLESH); //mxd
		gi.sound(self, CHAN_WEAPON, sounds[snd_id], 1.0f, ATTN_NORM, 0.0f);

		const int damage = irand(OGLE_DMG_MIN, OGLE_DMG_MAX); //mxd. float/flrand() in original logic.
		T_Damage(victim, self, self, direction, trace.endpos, blood_dir, damage, damage * 2, 0, MOD_DIED);
	}
}

// Pick and choose from a wide assortment of happy, joyous dances.
void ogle_celebrate(edict_t* self)
{
	const int chance = irand(0, 100);

	if (irand(0, 10) == 0)
		gi.sound(self, CHAN_VOICE, sounds[irand(SND_CHEER1, SND_CHEER3)], 1.0f, ATTN_IDLE, 0.0f);

	switch (self->curAnimID)
	{
		case ANIM_CELEBRATE1:
			if (chance < 70)
				SetAnim(self, ANIM_CELEBRATE1);
			else if (chance < 80)
				SetAnim(self, ANIM_CELEBRATE5_TRANS);
			else
				SetAnim(self, ANIM_CELEBRATE2);
			break;

		case ANIM_CELEBRATE2:
			if (chance < 5)
				SetAnim(self, ANIM_CELEBRATE4_TRANS);
			else if (chance < 10)
				SetAnim(self, ANIM_CELEBRATE3_TRANS);
			else if (chance < 50)
				SetAnim(self, ANIM_CELEBRATE1);
			else if (chance < 80)
				SetAnim(self, ANIM_CELEBRATE5_TRANS);
			else
				SetAnim(self, ANIM_CELEBRATE2);
			break;

		case ANIM_CELEBRATE3_TRANS:
			SetAnim(self, ANIM_CELEBRATE3);
			break;

		case ANIM_CELEBRATE4_TRANS:
			SetAnim(self, ANIM_CELEBRATE4);
			break;

		case ANIM_CELEBRATE5_TRANS:
			SetAnim(self, ANIM_CELEBRATE5);
			break;

		case ANIM_CELEBRATE3:
			if (chance < 50)
				SetAnim(self, ANIM_CELEBRATE4);
			else if (chance < 60)
				SetAnim(self, ANIM_CELEBRATE2);
			else
				SetAnim(self, ANIM_CELEBRATE3);
			break;

		case ANIM_CELEBRATE4:
			if (chance < 50)
				SetAnim(self, ANIM_CELEBRATE3);
			else if (chance < 60)
				SetAnim(self, ANIM_CELEBRATE2);
			else
				SetAnim(self, ANIM_CELEBRATE4);
			break;

		case ANIM_CELEBRATE5:
			if (chance < 90)
				SetAnim(self, ANIM_CELEBRATE5);
			else if (chance < 95)
				SetAnim(self, ANIM_CELEBRATE1);
			else
				SetAnim(self, ANIM_CELEBRATE2);
			break;

		default:
			break;
	}
}

// Spawn the dust and debris, based on what animation the ogle is in.
void ogle_pick_dust(edict_t* self)
{
	vec3_t forward;
	vec3_t right;
	vec3_t up;
	AngleVectors(self->s.angles, forward, right, up);

	vec3_t dust_pos;
	VectorCopy(self->s.origin, dust_pos);

	byte fx_flags = 0;

	switch (self->curAnimID)
	{
		case ANIM_WORK3:
			VectorMA(dust_pos, 38.0f, forward, dust_pos);
			VectorMA(dust_pos, 6.0f, right, dust_pos);
			VectorMA(dust_pos, -4.0f, up, dust_pos);
			VectorScale(forward, -1.0f, forward);
			break;

		case ANIM_WORK5:
			VectorMA(dust_pos, 42.0f, forward, dust_pos);
			VectorMA(dust_pos, 2.0f, right, dust_pos);
			VectorMA(dust_pos, -24.0f, up, dust_pos);
			VectorCopy(up, forward);
			fx_flags = CEF_FLAG6;
			break;

		case ANIM_WORK4:
			VectorMA(dust_pos, 42.0f, forward, dust_pos);
			VectorMA(dust_pos, 4.0f, right, dust_pos);
			VectorMA(dust_pos, 8.0f, up, dust_pos);
			VectorScale(forward, -1.0f, forward);
			break;

		default:
			VectorMA(dust_pos, 32.0f, forward, dust_pos);
			VectorMA(dust_pos, 4.0f, right, dust_pos);
			VectorMA(dust_pos, 22.0f, up, dust_pos);
			VectorScale(forward, -1.0f, forward);
			break;
	}

	// Random chance to create a spark.
	if (irand(0, 20) == 0)
		gi.CreateEffect(NULL, FX_SPARKS, CEF_FLAG6, dust_pos, "d", up);

	gi.CreateEffect(NULL, FX_OGLE_HITPUFF, fx_flags, dust_pos, "v", forward);

	// Check for the chisel and hammer.
	if (!(self->s.fmnodeinfo[MESH__NAIL].flags & FMNI_NO_DRAW))
		gi.sound(self, CHAN_WEAPON, sounds[irand(SND_SPIKE1, SND_SPIKE2)], 1.0f, ATTN_IDLE, 0.0f);
	else if (!(self->s.fmnodeinfo[MESH__PICK].flags & FMNI_NO_DRAW))
		gi.sound(self, CHAN_WEAPON, sounds[irand(SND_PICK1, SND_PICK2)], 1.0f, ATTN_IDLE, 0.0f);
	else
		gi.sound(self, CHAN_WEAPON, sounds[irand(SND_HAMMER1, SND_HAMMER2)], 1.0f, ATTN_IDLE, 0.0f);
}

void ogle_push(edict_t* self, float distance)
{
	qboolean done = false;

	edict_t* found = G_Find(NULL, FOFS(targetname), self->target);
	if (found != NULL)
	{
		vec3_t forward;
		AngleVectors(self->s.angles, forward, NULL, NULL);

		vec3_t end_pos;
		VectorMA(self->s.origin, 64.0f, forward, end_pos);

		trace_t	trace;
		gi.trace(self->s.origin, vec3_origin, vec3_origin, end_pos, self, MASK_MONSTERSOLID, &trace);

		if (trace.ent != NULL && trace.ent == found)
		{
			const float yaw = self->s.angles[YAW] * ANGLE_TO_RAD;
			vec3_t move = { cosf(yaw) * distance, sinf(yaw) * distance, 0.0f };

			if (SV_movestep(found, move, true))
			{
				M_GetSlopePitchRoll(found, NULL);
				M_walkmove(self, self->s.angles[YAW], distance);

				return;
			}

			done = true;
		}
	}

	if (done)
	{
		gi.sound(self, CHAN_VOICE, sounds[SND_WIPE_BROW], 1.0f, ATTN_IDLE, 0.0f);
		SetAnim(self, ANIM_REST1_WIPE);
	}
	else
	{
		SetAnim(self, ANIM_REST4_TRANS);
	}

	self->ai_mood = AI_MOOD_REST;
	self->mood_think = OgleMoodThink;
}

#pragma endregion

void OgleStaticsInit(void)
{
	static ClassResourceInfo_t res_info; //mxd. Made local static.

	classStatics[CID_OGLE].msgReceivers[MSG_STAND] = OgleStandMsgHandler;
	classStatics[CID_OGLE].msgReceivers[MSG_RUN] = OgleRunMsgHandler;
	classStatics[CID_OGLE].msgReceivers[MSG_MELEE] = OgleMeleeMsgHandler;
	classStatics[CID_OGLE].msgReceivers[MSG_DISMEMBER] = DismemberMsgHandler;
	classStatics[CID_OGLE].msgReceivers[MSG_DEATH] = OgleDeathMsgHandler;
	classStatics[CID_OGLE].msgReceivers[MSG_PAIN] = OglePainMsgHandler;
	classStatics[CID_OGLE].msgReceivers[MSG_DEATH_PAIN] = OgleDeathPainMsgHandler;

	classStatics[CID_OGLE].msgReceivers[MSG_C_ACTION1] = OgleCinematicActionMsgHandler;
	classStatics[CID_OGLE].msgReceivers[MSG_C_ACTION2] = OgleCinematicActionMsgHandler;
	classStatics[CID_OGLE].msgReceivers[MSG_C_ACTION3] = OgleCinematicActionMsgHandler;
	classStatics[CID_OGLE].msgReceivers[MSG_C_ACTION4] = OgleCinematicActionMsgHandler;
	classStatics[CID_OGLE].msgReceivers[MSG_C_ACTION5] = OgleCinematicActionMsgHandler;
	classStatics[CID_OGLE].msgReceivers[MSG_C_ACTION6] = OgleCinematicActionMsgHandler;
	classStatics[CID_OGLE].msgReceivers[MSG_C_ACTION7] = OgleCinematicActionMsgHandler;
	classStatics[CID_OGLE].msgReceivers[MSG_C_ACTION8] = OgleCinematicActionMsgHandler;
	classStatics[CID_OGLE].msgReceivers[MSG_C_ACTION9] = OgleCinematicActionMsgHandler;
	classStatics[CID_OGLE].msgReceivers[MSG_C_ACTION10] = OgleCinematicActionMsgHandler;
	classStatics[CID_OGLE].msgReceivers[MSG_C_ACTION11] = OgleCinematicActionMsgHandler;
	classStatics[CID_OGLE].msgReceivers[MSG_C_ACTION12] = OgleCinematicActionMsgHandler;
	classStatics[CID_OGLE].msgReceivers[MSG_C_ACTION13] = OgleCinematicActionMsgHandler;
	classStatics[CID_OGLE].msgReceivers[MSG_C_ACTION14] = OgleCinematicActionMsgHandler;
	classStatics[CID_OGLE].msgReceivers[MSG_C_ACTION15] = OgleCinematicActionMsgHandler;
	classStatics[CID_OGLE].msgReceivers[MSG_C_ATTACK1] = OgleCinematicActionMsgHandler;
	classStatics[CID_OGLE].msgReceivers[MSG_C_ATTACK2] = OgleCinematicActionMsgHandler;
	classStatics[CID_OGLE].msgReceivers[MSG_C_ATTACK3] = OgleCinematicActionMsgHandler;
	classStatics[CID_OGLE].msgReceivers[MSG_C_DEATH1] = OgleCinematicActionMsgHandler;
	classStatics[CID_OGLE].msgReceivers[MSG_C_DEATH2] = OgleCinematicActionMsgHandler;
	classStatics[CID_OGLE].msgReceivers[MSG_C_GIB1] = CinematicGibMsgHandler;
	classStatics[CID_OGLE].msgReceivers[MSG_C_IDLE1] = OgleCinematicActionMsgHandler;
	classStatics[CID_OGLE].msgReceivers[MSG_C_IDLE2] = OgleCinematicActionMsgHandler;
	classStatics[CID_OGLE].msgReceivers[MSG_C_IDLE3] = OgleCinematicActionMsgHandler;
	classStatics[CID_OGLE].msgReceivers[MSG_C_IDLE4] = OgleCinematicActionMsgHandler;
	classStatics[CID_OGLE].msgReceivers[MSG_C_IDLE5] = OgleCinematicActionMsgHandler;
	classStatics[CID_OGLE].msgReceivers[MSG_C_IDLE6] = OgleCinematicActionMsgHandler;
	classStatics[CID_OGLE].msgReceivers[MSG_C_PAIN1] = OgleCinematicActionMsgHandler;
	classStatics[CID_OGLE].msgReceivers[MSG_C_PAIN2] = OgleCinematicActionMsgHandler;
	classStatics[CID_OGLE].msgReceivers[MSG_C_PAIN3] = OgleCinematicActionMsgHandler;
	classStatics[CID_OGLE].msgReceivers[MSG_C_THINKAGAIN] = OgleCinematicActionMsgHandler;
	classStatics[CID_OGLE].msgReceivers[MSG_C_TRANS1] = OgleCinematicActionMsgHandler;
	classStatics[CID_OGLE].msgReceivers[MSG_C_TRANS2] = OgleCinematicActionMsgHandler;
	classStatics[CID_OGLE].msgReceivers[MSG_C_TRANS3] = OgleCinematicActionMsgHandler;
	classStatics[CID_OGLE].msgReceivers[MSG_C_TRANS4] = OgleCinematicActionMsgHandler;
	classStatics[CID_OGLE].msgReceivers[MSG_C_TRANS5] = OgleCinematicActionMsgHandler;
	classStatics[CID_OGLE].msgReceivers[MSG_C_TRANS6] = OgleCinematicActionMsgHandler;
	classStatics[CID_OGLE].msgReceivers[MSG_C_WALK1] = OgleCinematicActionMsgHandler;
	classStatics[CID_OGLE].msgReceivers[MSG_C_WALK2] = OgleCinematicActionMsgHandler;
	classStatics[CID_OGLE].msgReceivers[MSG_C_WALK3] = OgleCinematicActionMsgHandler;
	classStatics[CID_OGLE].msgReceivers[MSG_C_WALK4] = OgleCinematicActionMsgHandler;

	res_info.numAnims = NUM_ANIMS;
	res_info.animations = animations;
	res_info.modelIndex = gi.modelindex("models/monsters/ogle/tris.fm");

	sounds[SND_PICK1] = gi.soundindex("monsters/ogle/pick1.wav");
	sounds[SND_PICK2] = gi.soundindex("monsters/ogle/pick2.wav");
	sounds[SND_SPIKE1] = gi.soundindex("monsters/ogle/spike1.wav");
	sounds[SND_SPIKE2] = gi.soundindex("monsters/ogle/spike2.wav");
	sounds[SND_HAMMER1] = gi.soundindex("monsters/ogle/hammer1.wav");
	sounds[SND_HAMMER2] = gi.soundindex("monsters/ogle/hammer2.wav");
	sounds[SND_PICK_FLESH] = gi.soundindex("monsters/ogle/pickflesh.wav");
	sounds[SND_HAMMER_FLESH] = gi.soundindex("monsters/ogle/hammerflesh.wav");
	sounds[SND_WIPE_BROW] = gi.soundindex("monsters/ogle/wipebrow.wav");
	sounds[SND_ENRAGE1] = gi.soundindex("monsters/ogle/enrage1.wav");
	sounds[SND_ENRAGE2] = gi.soundindex("monsters/ogle/enrage2.wav");
	sounds[SND_DEATH] = gi.soundindex("monsters/ogle/death.wav");
	sounds[SND_CHEER1] = gi.soundindex("monsters/ogle/cheer1.wav");
	sounds[SND_CHEER2] = gi.soundindex("monsters/ogle/cheer2.wav");
	sounds[SND_CHEER3] = gi.soundindex("monsters/ogle/cheer3.wav");

	sounds[SND_PAIN1] = gi.soundindex("monsters/ogle/oglemoan1.wav");
	sounds[SND_PAIN2] = gi.soundindex("monsters/ogle/oglemoan2.wav");

	// Singing.
	sounds[SND_CHORUS1] = gi.soundindex("monsters/ogle/chorus1.wav");
	sounds[SND_CHORUS2] = gi.soundindex("monsters/ogle/chorus3.wav");
	sounds[SND_CHORUS3] = gi.soundindex("monsters/ogle/chorus5.wav");
	sounds[SND_SOLO1] = gi.soundindex("monsters/ogle/solo2.wav");
	sounds[SND_SOLO2] = gi.soundindex("monsters/ogle/solo4.wav");

	res_info.numSounds = NUM_SOUNDS;
	res_info.sounds = sounds;

	classStatics[CID_OGLE].resInfo = &res_info;
}

// QUAKED monster_ogle(1 .5 0) (-16 -16 -24) (16 16 16) OF_PUSHING OF_PICK_UP OF_PICK_DOWN OF_CHISEL_UP OF_CHISEL_DOWN OF_HAMMER_UP OF_HAMMER_DOWN OF_SONG_LEADER OF_CINEMATIC
// The little, disgruntled Ogle.

// Variables:
// wakeup_target			- Monsters will fire this target the first time it wakes up (only once).
// pain_target				- Monsters will fire this target the first time it gets hurt (only once).
// mintel					- Monster intelligence - this basically tells a monster how many buoys away an enemy has to be for it to give up (default 16).
// melee_range				- How close the player has to be for the monster to go into melee. If this is zero, the monster will never melee.
//							  If it is negative, the monster will try to keep this distance from the player.
//							  If the monster has a backup, he'll use it if too close, otherwise, a negative value here means the monster will just stop
//							  running at the player at this distance (default 48).
//							 Examples:
//								melee_range = 60 - monster will start swinging it player is closer than 60.
//								melee_range = 0 - monster will never do a melee attack.
//								melee_range = -100 - monster will never do a melee attack and will back away (if it has that ability) when player gets too close.
// missile_range			- Maximum distance the player can be from the monster to be allowed to use it's ranged attack (default 0).
// min_missile_range		- Minimum distance the player can be from the monster to be allowed to use it's ranged attack (default 0).
// bypass_missile_chance	- Chance that a monster will NOT fire it's ranged attack, even when it has a clear shot. This, in effect, will make the monster
//							  come in more often than hang back and fire. A percentage (0 = always fire/never close in, 100 = never fire/always close in) - must be whole number (default 0).
// jump_chance				- Every time the monster has the opportunity to jump, what is the chance (out of 100) that he will... (100 = jump every time) - must be whole number (default 10).
// wakeup_distance			- How far (max) the player can be away from the monster before it wakes up. This means that if the monster can see the player,
//							  at what distance should the monster actually notice him and go for him (default 1024).
// NOTE: A value of zero will result in defaults, if you actually want zero as the value, use -1.
void SP_monster_ogle(edict_t* self)
{
	// Generic Monster Initialization.
	if (!M_WalkmonsterStart(self)) // Failed initialization. //mxd. M_Start -> M_WalkmonsterStart.
		return;

	self->msgHandler = DefaultMsgHandler;
	self->monsterinfo.alert = NULL; // Can't be woken up.
	self->monsterinfo.dismember = OgleDismember;
	self->monsterinfo.otherenemyname = "monster_rat";

	// Don't want incorrect handling due to weird ogle spawnflags!
	self->monsterinfo.ogleflags = self->spawnflags;
	self->spawnflags = 0; 

	if (self->health == 0)
		self->health = OGLE_HEALTH;

	// Apply to the end result (whether designer set or not).
	self->health = MonsterHealth(self->health);
	self->max_health = self->health;

	self->mass = OGLE_MASS;
	self->yaw_speed = 16.0f;

	self->movetype = PHYSICSTYPE_STEP;
	self->solid = SOLID_BBOX;

	VectorCopy(STDMinsForClass[self->classID], self->mins);
	VectorCopy(STDMaxsForClass[self->classID], self->maxs);

	self->materialtype = MAT_FLESH;
	self->s.modelindex = (byte)classStatics[CID_OGLE].resInfo->modelIndex;
	self->s.skinnum = 0;

	if (self->monsterinfo.scale == 0.0f) //BUGFIX: mxd. 'if (self->monsterinfo.scale)' in original logic.
	{
		self->monsterinfo.scale = MODEL_SCALE;
		self->s.scale = self->monsterinfo.scale;
	}

	MG_InitMoods(self);

	self->monsterinfo.aiflags |= AI_NO_ALERT;
	self->monsterinfo.attack_finished = level.time + flrand(10.0f, 100.0f); //mxd. irand() in original logic.
	self->mood_think = OgleMoodThink;
	self->use = OgleUse;

	if (self->monsterinfo.ogleflags == 0)
	{
		switch (irand(0, 4))
		{
			default:
			case 0:	self->monsterinfo.ogleflags |= OF_PICK_UP; break;
			case 1:	self->monsterinfo.ogleflags |= OF_PICK_DOWN; break;
			case 2:	self->monsterinfo.ogleflags |= OF_HAMMER_UP; break;
			case 3:	self->monsterinfo.ogleflags |= OF_HAMMER_DOWN; break;
			case 4:	self->monsterinfo.ogleflags |= OF_CHISEL_UP; break;
		}
	}

	qboolean skip_inits = false;

	if (self->monsterinfo.ogleflags & OF_PUSHING)
	{
		if (self->targetname != NULL && self->target != NULL)
		{
			edict_t* found = G_Find(NULL, FOFS(targetname), self->target);
			if (found != NULL)
				M_GetSlopePitchRoll(found, NULL);

			skip_inits = true;

			SetAnim(self, ANIM_REST4);
			self->mood_think = NULL;
			self->use = OgleStartPushUse;

			self->s.fmnodeinfo[MESH__NAIL].flags |= FMNI_NO_DRAW;
			self->s.fmnodeinfo[MESH__HAMMER].flags |= FMNI_NO_DRAW;
			self->s.fmnodeinfo[MESH__PICK].flags |= FMNI_NO_DRAW;
		}
		else
		{
			// We gotta do SOMETHING if there is no target, otherwise the monster will puke.
			gi.dprintf("Ogle at %s set to push with no target or targetname.\n", vtos(self->s.origin));

			SetAnim(self, ANIM_WORK3);
			self->s.fmnodeinfo[MESH__PICK].flags |= FMNI_NO_DRAW;
		}
	}
	else if (self->monsterinfo.ogleflags & OF_PICK_UP)
	{
		if (self->monsterinfo.ogleflags & OF_CINEMATIC)
			SetAnim(self, ANIM_C_IDLE1);
		else
			SetAnim(self, ANIM_WORK4);

		self->s.fmnodeinfo[MESH__NAIL].flags |= FMNI_NO_DRAW;
		self->s.fmnodeinfo[MESH__HAMMER].flags |= FMNI_NO_DRAW;
	}
	else if (self->monsterinfo.ogleflags & OF_PICK_DOWN)
	{
		if (self->monsterinfo.ogleflags & OF_CINEMATIC)
			SetAnim(self, ANIM_C_IDLE1);
		else
			SetAnim(self, ANIM_WORK5);

		self->s.fmnodeinfo[MESH__NAIL].flags |= FMNI_NO_DRAW;
		self->s.fmnodeinfo[MESH__HAMMER].flags |= FMNI_NO_DRAW;
	}
	else if (self->monsterinfo.ogleflags & OF_CHISEL_UP)
	{
		if (self->monsterinfo.ogleflags & OF_CINEMATIC)
			SetAnim(self, ANIM_C_IDLE1);
		else
			SetAnim(self, irand(ANIM_WORK1, ANIM_WORK2));

		self->s.fmnodeinfo[MESH__PICK].flags |= FMNI_NO_DRAW;
	}
	else if (self->monsterinfo.ogleflags & OF_HAMMER_UP)
	{
		if (self->monsterinfo.ogleflags & OF_CINEMATIC)
			SetAnim(self, ANIM_C_IDLE1);
		else
			SetAnim(self, ANIM_WORK4);

		self->s.fmnodeinfo[MESH__NAIL].flags |= FMNI_NO_DRAW;
		self->s.fmnodeinfo[MESH__PICK].flags |= FMNI_NO_DRAW;
	}
	else if (self->monsterinfo.ogleflags & OF_HAMMER_DOWN)
	{
		if (self->monsterinfo.ogleflags & OF_CINEMATIC)
			SetAnim(self, ANIM_C_IDLE1);
		else
			SetAnim(self, ANIM_WORK5);

		self->s.fmnodeinfo[MESH__NAIL].flags |= FMNI_NO_DRAW;
		self->s.fmnodeinfo[MESH__PICK].flags |= FMNI_NO_DRAW;
	}
	else
	{
		if (self->monsterinfo.ogleflags & OF_CINEMATIC)
			SetAnim(self, ANIM_C_IDLE1);
		else
			SetAnim(self, ANIM_WORK3);

		self->s.fmnodeinfo[MESH__PICK].flags |= FMNI_NO_DRAW;
	}

	if (self->monsterinfo.ogleflags & OF_CINEMATIC)
	{
		self->svflags |= SVF_FLOAT;
		self->monsterinfo.c_mode = true;
	}

	self->svflags |= SVF_NO_AUTOTARGET;
	self->nextthink = level.time + FRAMETIME; //mxd. Use define.

	if (!skip_inits)
	{
		// Find out who our overlord is.
		self->think = OgleInitOverlordThink;

		if ((int)singing_ogles->value && !(self->monsterinfo.ogleflags & OF_SONG_LEADER))
			OgleTrySetAsSongLeader(self);

		if (self->monsterinfo.ogleflags & OF_SONG_LEADER)
			self->noise_index = 0;
	}
}