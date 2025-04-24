//
// m_ogle.c
//
// Copyright 1998 Raven Software
//

#include "m_ogle.h"
#include "m_ogle_shared.h"
#include "m_ogle_anim.h"
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

static void OgleCinematicActionMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'ogle_c_anims' in original logic.
{
	int curr_anim;

	ai_c_readmessage(self, msg);
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

static void OgleMoodThink(edict_t* self) //mxd. Named 'ogle_mood_think' in original logic.
{
	if (self->enemy == NULL)
	{
		if (self->targetEnt != NULL && self->targetEnt->health > 0 && (self->targetEnt->health < SERAPH_HEALTH / 2 || self->targetEnt->ai_mood == AI_MOOD_FLEE))
		{
			gi.sound(self, CHAN_BODY, sounds[irand(SND_ENRAGE1, SND_ENRAGE2)], 1.0f, ATTN_NORM, 0.0f);
			self->enemy = self->targetEnt;
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

// Cast off the tools which have oppressed the ogle people for centuries... or something.
static void OgleDropTools(edict_t* self) //mxd. Named 'ogle_cast_off_tools_of_oppression' in original logic.
{
	if (!(self->s.fmnodeinfo[MESH__NAIL].flags & FMNI_NO_DRAW))
	{
		// Cast off the hammer and nail.
		self->s.fmnodeinfo[MESH__NAIL].flags |= FMNI_NO_DRAW;
		ThrowWeapon(self, &vec3_origin, BPN_NAIL, 0, 0);

		self->s.fmnodeinfo[MESH__HAMMER].flags |= FMNI_NO_DRAW;
		self->s.fmnodeinfo[MESH__HANDLE].flags |= FMNI_NO_DRAW;

		ThrowWeapon(self, &vec3_origin, BPN_HAMMER | BPN_HANDLE, 0, 0);
	}
	else if (!(self->s.fmnodeinfo[MESH__PICK].flags & FMNI_NO_DRAW))
	{
		// Cast off the pick.
		self->s.fmnodeinfo[MESH__PICK].flags |= FMNI_NO_DRAW;
		self->s.fmnodeinfo[MESH__HANDLE].flags |= FMNI_NO_DRAW;

		ThrowWeapon(self, &vec3_origin, BPN_PICK | BPN_HANDLE, 0, 0);
	}
	else if (!(self->s.fmnodeinfo[MESH__HAMMER].flags & FMNI_NO_DRAW))
	{
		// Cast off the hammer.
		self->s.fmnodeinfo[MESH__HAMMER].flags |= FMNI_NO_DRAW;
		self->s.fmnodeinfo[MESH__HANDLE].flags |= FMNI_NO_DRAW;

		ThrowWeapon(self, &vec3_origin, BPN_HAMMER | BPN_HANDLE, 0, 0);
	}

	self->monsterinfo.aiflags |= AI_NO_MELEE;
}

// The ogle's been yelled at or struck by the overlord, get back to work!
static void OgleUse(edict_t* self, edict_t* other, edict_t* activator) //mxd. Named 'ogle_use' in original logic.
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
static void OgleInitOverlordThink(edict_t* self) //mxd. Named 'ogle_init_overlord' in original logic.
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
			self->targetEnt = seraph; //TODO: add ogle_overlord name?
			break;
		}
	}
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
			self->monsterinfo.jump_time = level.time + 16.0f; //TODO: add ogle_sign_time name?
			break;

		case 1:
			gi.sound(self, CHAN_VOICE, sounds[SND_SOLO1], 1.0f, ATTN_NORM, 0.0f);
			self->monsterinfo.jump_time = level.time + 4.0f;
			break;

		case 2:
			while ((ogle = G_Find(ogle, FOFS(target), self->target)) != NULL)
			{
				if (!ogle->monsterinfo.awake)
					gi.sound(ogle, CHAN_VOICE, sounds[SND_CHORUS2], 0.25f, ATTN_NORM, 0.0f);
			}
			self->monsterinfo.jump_time = level.time + 3.0f;
			break;

		case 3:
			gi.sound(self, CHAN_VOICE, sounds[SND_SOLO2], 1.0f, ATTN_NORM, 0.0f);
			self->monsterinfo.jump_time = level.time + 4.0f;
			break;

		case 4:
			while ((ogle = G_Find(ogle, FOFS(target), self->target)) != NULL)
			{
				if (!ogle->monsterinfo.awake)
					gi.sound(ogle, CHAN_VOICE, sounds[SND_CHORUS3], 0.25f, ATTN_NORM, 0.0f);
			}
			self->monsterinfo.jump_time = level.time + 4.0f;
			break;
	}

	self->noise_index++;
	self->noise_index %= 5;
}

// High level AI interface.
void ogle_pause(edict_t* self)
{
	if (self->enemy == NULL)
	{
		if ((self->monsterinfo.ogleflags & OF_SONG_LEADER) && self->monsterinfo.jump_time < level.time)
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
			QPostMessage(self, MSG_MELEE, PRI_DIRECTIVE, NULL);
			break;

		case AI_MOOD_FLEE:
		case AI_MOOD_PURSUE:
		case AI_MOOD_NAVIGATE:
			if (self->enemy != NULL)
				QPostMessage(self, MSG_RUN, PRI_DIRECTIVE, NULL);
			break;

		case AI_MOOD_WALK:
			QPostMessage(self, MSG_WALK, PRI_DIRECTIVE, NULL);
			break;

		case AI_MOOD_STAND:
			QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
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
			SetAnim(self, ANIM_REST1);
			break;

		case ANIM_REST1:
			if (irand(0, 100) < 20)
				SetAnim(self, ANIM_REST1_WIPE);
			break;

		case ANIM_REST1_WIPE:
			SetAnim(self, ANIM_REST1);
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

static void OglePainMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'ogle_pain' in original logic.
{
	edict_t* target;
	edict_t* attacker;
	qboolean force_pain;
	int damage;
	int temp;
	ParseMsgParms(msg, "eeiii", &target, &attacker, &force_pain, &damage, &temp);

	self->mood_think = OgleMoodThink;

	if (attacker != NULL && !AI_IsInfrontOf(self, attacker))
		SetAnim(self, ANIM_PAIN3);
	else
		SetAnim(self, irand(ANIM_PAIN1, ANIM_PAIN3));

	gi.sound(self, CHAN_VOICE, sounds[irand(SND_PAIN1, SND_PAIN2)], 1.0f, ATTN_NORM, 0.0f);
}

void ogle_dismember(edict_t *self, int damage, int HitLocation)
{
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
			self->s.fmnodeinfo[MESH__TORSO].flags |= FMNI_USE_SKIN;			
			self->s.fmnodeinfo[MESH__TORSO].skin = self->s.skinnum+1;
		break;

	case hl_TorsoFront:
	case hl_TorsoBack:
			self->s.fmnodeinfo[MESH__TORSO].flags |= FMNI_USE_SKIN;			
			self->s.fmnodeinfo[MESH__TORSO].skin = self->s.skinnum+1;
		break;

	case hl_ArmUpperLeft:
		if(self->s.fmnodeinfo[MESH__LUPARM].flags & FMNI_NO_DRAW)
			break;
		
		if (dismember_ok)
		{
			AngleVectors(self->s.angles,NULL,right,NULL);
			gore_spot[2]+=self->maxs[2]*0.3;
			VectorMA(gore_spot,-8,right,gore_spot);
			
			throw_nodes |= BPN_LUPARM;
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

			ThrowBodyPart(self, &gore_spot, throw_nodes, damage, 0);
		}
		else
		{
			self->s.fmnodeinfo[MESH__LUPARM].flags |= FMNI_USE_SKIN;			
			self->s.fmnodeinfo[MESH__LUPARM].skin = self->s.skinnum+1;
		}

		break;

	case hl_ArmLowerLeft:
		if(self->s.fmnodeinfo[MESH__L4ARM].flags & FMNI_NO_DRAW)
			break;
		
		if (dismember_ok)
		{
			AngleVectors(self->s.angles,NULL,right,NULL);
			gore_spot[2]+=self->maxs[2]*0.3;
			VectorMA(gore_spot,-8,right,gore_spot);
			
			throw_nodes |= BPN_L4ARM;
			self->s.fmnodeinfo[MESH__L4ARM].flags |= FMNI_NO_DRAW;

			if (!(self->s.fmnodeinfo[MESH__NAIL].flags & FMNI_NO_DRAW))
			{
				throw_nodes |= BPN_NAIL;
				self->s.fmnodeinfo[MESH__NAIL].flags |= FMNI_NO_DRAW;
			}

			ThrowBodyPart(self, &gore_spot, throw_nodes, damage, 0);
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
		
		if (dismember_ok)
		{
			AngleVectors(self->s.angles,NULL,right,NULL);
			gore_spot[2]+=self->maxs[2]*0.3;
			VectorMA(gore_spot,8,right,gore_spot);
			
			throw_nodes |= BPN_RUPARM;
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

			ThrowBodyPart(self, &gore_spot, throw_nodes, damage, 0);

			self->monsterinfo.aiflags |= AI_NO_MELEE;
			self->monsterinfo.aiflags |= AI_COWARD;
		}
		else
		{
			self->s.fmnodeinfo[MESH__RUPARM].flags |= FMNI_USE_SKIN;			
			self->s.fmnodeinfo[MESH__RUPARM].skin = self->s.skinnum+1;
		}

		break;

	case hl_ArmLowerRight:
		if(self->s.fmnodeinfo[MESH__R4ARM].flags & FMNI_NO_DRAW)
			break;
		
		if (dismember_ok)
		{
			AngleVectors(self->s.angles,NULL,right,NULL);
			gore_spot[2]+=self->maxs[2]*0.3;
			VectorMA(gore_spot,8,right,gore_spot);
			
			throw_nodes |= BPN_R4ARM;
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

			ThrowBodyPart(self, &gore_spot, throw_nodes, damage, 0);

			self->monsterinfo.aiflags |= AI_NO_MELEE;
			self->monsterinfo.aiflags |= AI_COWARD;
		}
		else
		{
			self->s.fmnodeinfo[MESH__R4ARM].flags |= FMNI_USE_SKIN;			
			self->s.fmnodeinfo[MESH__R4ARM].skin = self->s.skinnum+1;
		}

		break;

	case hl_LegUpperLeft:
	case hl_LegLowerLeft:
			self->s.fmnodeinfo[MESH__LLEG].flags |= FMNI_USE_SKIN;			
			self->s.fmnodeinfo[MESH__LLEG].skin = self->s.skinnum+1;
		break;

	case hl_LegUpperRight:
	case hl_LegLowerRight:
			self->s.fmnodeinfo[MESH__RLEG].flags |= FMNI_USE_SKIN;			
			self->s.fmnodeinfo[MESH__RLEG].skin = self->s.skinnum+1;
		break;
	}
}

void ogle_death_pain(edict_t *self, G_Message_t *msg)
{
	if(self->health <= -80) //gib death
	{
		BecomeDebris(self);
		return;
	}
	else if(msg)
		DismemberMsgHandler(self, msg);
}

void ogle_death(edict_t *self, G_Message_t *msg)
{
	edict_t	*targ, *inflictor, *attacker;
	float	damage;
	vec3_t	dVel, vf, yf;

	ParseMsgParms(msg, "eeei", &targ, &inflictor, &attacker, &damage);

	M_StartDeath(self, ANIM_DEATH1);
	
	OgleDropTools ( self );

	if (self->health < -80)
	{
		return;
	}
	else if (self->health < -10)
	{
		SetAnim(self, ANIM_DEATH2);
		
		VectorCopy(targ->velocity, vf);
		VectorNormalize(vf);

		VectorScale(vf, -1, yf);

		self->elasticity = 1.2;
		self->friction = 0.8;

		VectorScale(vf, 300, dVel);
		dVel[2] = irand(200,250);

		VectorCopy(dVel, self->velocity);
//		self->groundentity = NULL;
	}
	else
	{
		SetAnim(self, ANIM_DEATH1);
	}

	gi.sound (self, CHAN_BODY, sounds[SND_DEATH], 1, ATTN_NORM, 0);

}

//Get to work!
void ogle_work1(edict_t *self, G_Message_t *msg)
{
	SetAnim(self, ANIM_WORK1);
}

qboolean ogle_findtarget (edict_t *self)
{
	edict_t		*found = NULL;

	//take down weak overlords
	while(found = FindInRadius(found, self->s.origin, 1024))
	{
		if(found->classID == CID_SERAPH_OVERLORD && found->health > 0 && (found->health<SERAPH_HEALTH/2 || found->ai_mood == AI_MOOD_FLEE))
		{
			self->enemy = found;
			AI_FoundTarget(self, false);
			return true;
		}
	}

	/*//Used to go after other stuff and break it...
	found = NULL;
	//ok, search for utensils of their oppression
	while(found = findradius(found, self->s.origin, 512))
	{
		if(found->classID == CID_OBJECT)
		{
			if(found->takedamage && found->health > 0)
			{
				if(!strcmp(found->classname, "obj_minecart1")||
					!strcmp(found->classname, "obj_minecart2")||
					!strcmp(found->classname, "obj_minecart3")||
					!strcmp(found->classname, "obj_pick")||
					!strcmp(found->classname, "obj_gascan")||
					!strcmp(found->classname, "obj_barrel_metal")||
					!strcmp(found->classname, "obj_metalchunk1")||
					!strcmp(found->classname, "obj_metalchunk2")||
					!strcmp(found->classname, "obj_metalchunk3")||
					!strcmp(found->classname, "obj_pipe1")||
					!strcmp(found->classname, "obj_pipe2")||
					!strcmp(found->classname, "obj_pushcart")||
					!strcmp(found->classname, "obj_shovel")||
					!strcmp(found->classname, "obj_wheelbarrow")||
					!strcmp(found->classname, "obj_wheelbarrowdamaged"))
				{
					if(irand(0, 1))
					{
						if(visible(self, found))
						{
							self->enemy = found;
							FoundTarget(self, false);
							return true;
						}
					}
				}
			}
		}
	}*/
	
	found = NULL;
	//help out other ogles
	while(found = FindInRadius(found, self->s.origin, 1024))
	{
		if(found->classID == CID_OGLE && found->health > 0 && found != self)
		{
			if(found->enemy)
			{
				if(found->enemy->health > 0)
				{
					{
						if(found->enemy->client)
							found->enemy = NULL;
						else
						{
							self->enemy = found->enemy;
							AI_FoundTarget(self, false);
							return true;
						}
					}
				}
			}
		}
	}

	return false;
}
//Classic melee function (fear the ogles)
void ogle_melee( edict_t *self, G_Message_t *msg )
{
	qboolean	ret;
	int chance = irand(0,4);

	if (!self->enemy)
		return;

	if(self->enemy->client)
		self->enemy = NULL;

	if (self->enemy->health <= 0)
	{
		ret = ogle_findtarget(self);

		if (!ret)
		{
			self->enemy = self->goalentity = NULL;
			self->ai_mood = AI_MOOD_PURSUE;

			OgleDropTools( self );

			switch( chance )
			{
			case 0:	SetAnim(self, ANIM_CELEBRATE1);	break;
			case 1:	SetAnim(self, ANIM_CELEBRATE2);	break;
			case 2:	SetAnim(self, ANIM_CELEBRATE3);	break;
			case 3:	SetAnim(self, ANIM_CELEBRATE4);	break;
			case 4:	SetAnim(self, ANIM_CELEBRATE5);	break;
			}

			return;
		}
	}

	SetAnim(self, ANIM_ATTACK1);
}

//do a little dance.. make a little.. oh, sorry
void ogle_stand1(edict_t *self, G_Message_t *msg)
{
	if(self->monsterinfo.awake)
	{
		int	change = irand(0,4);

		switch( change )
		{
		case 0:	SetAnim(self, ANIM_CELEBRATE1);	break;
		case 1:	SetAnim(self, ANIM_CELEBRATE2);	break;
		case 2:	SetAnim(self, ANIM_CELEBRATE3);	break;
		case 3:	SetAnim(self, ANIM_CELEBRATE4);	break;
		case 4:	SetAnim(self, ANIM_CELEBRATE5);	break;
		}
	}
}

//Classic run-attack function, who thought mortal combat could be so cute?
void ogle_run1(edict_t *self, G_Message_t *msg)
{
	trace_t		trace;
	vec3_t		start, end, mins;
	float		len;
	int			change = irand(0,4);
	qboolean	ret;

	if(self->enemy && self->enemy->client)
		self->enemy = NULL;

	if (self->enemy && self->enemy->health <= 0)
	{
		ret = ogle_findtarget(self);

		if (!ret)
		{
			self->enemy = self->goalentity = NULL;
			self->ai_mood = AI_MOOD_PURSUE;
			
			OgleDropTools( self );
	
			switch( change )
			{
			case 0:	SetAnim(self, ANIM_CELEBRATE1);	break;
			case 1:	SetAnim(self, ANIM_CELEBRATE2);	break;
			case 2:	SetAnim(self, ANIM_CELEBRATE3);	break;
			case 3:	SetAnim(self, ANIM_CELEBRATE4);	break;
			case 4:	SetAnim(self, ANIM_CELEBRATE5);	break;
			}
		}
		
		return;
	}
	else if (self->enemy)
	{
		len = M_DistanceToTarget(self, self->enemy);

		if (len < 40)	// close enough to swing, not necessarily hit						
		{
			SetAnim(self, ANIM_ATTACK1);		
		}
		else if (len < 100)	// close enough to swing, not necessarily hit						
		{
			VectorCopy(self->s.origin, start);
			VectorCopy(self->enemy->s.origin, end);
			start[2]+=self->viewheight;
			end[2]+=self->enemy->viewheight;

			VectorCopy(self->mins, mins);
			mins[2]+=self->maxs[0]/2;//because this guys's mins are 0

			gi.trace(start, mins, self->maxs, end, self, MASK_MONSTERSOLID,&trace);
			
			if(trace.ent==self->enemy)
			{
				if (irand(0,1))
					SetAnim(self, ANIM_ATTACK2);
				else
					SetAnim(self, ANIM_ATTACK3);
			}
			else
			{
				switch (change)
				{
				case 0:	SetAnim(self, ANIM_CHARGE1);	break;
				case 1:	SetAnim(self, ANIM_CHARGE2);	break;
				case 2:	SetAnim(self, ANIM_CHARGE3);	break;
				case 3:	SetAnim(self, ANIM_CHARGE4);	break;
				case 4:	SetAnim(self, ANIM_CHARGE5);	break;
				}
			}
		}
		else		
		{
			switch (self->curAnimID)
			{
			case ANIM_CHARGE1:
			case ANIM_CHARGE2:
			case ANIM_CHARGE3:
			case ANIM_CHARGE4:
			case ANIM_CHARGE5:
				break;
			
			default:
				switch (change)
				{
				case 0:	SetAnim(self, ANIM_CHARGE1);	break;
				case 1:	SetAnim(self, ANIM_CHARGE2);	break;
				case 2:	SetAnim(self, ANIM_CHARGE3);	break;
				case 3:	SetAnim(self, ANIM_CHARGE4);	break;
				case 4:	SetAnim(self, ANIM_CHARGE5);	break;
				}
				break;
			}
		}
	
		return;
	}
	
	QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
}

void ogle_check_leadsong (edict_t *self)
{
	edict_t *ogle = NULL;

	while((ogle = G_Find(ogle, FOFS(target), self->target)) != NULL)
	{
		if(ogle->monsterinfo.ogleflags & OF_SONG_LEADER)
			return;
	}
	
	self->monsterinfo.ogleflags |= OF_SONG_LEADER;
}

void ogle_start_push (edict_t *self, edict_t *other, edict_t *activator)
{
	if(!irand(0, 2))
		SetAnim(self, ANIM_PUSH1);
	else if(irand(0, 1))
		SetAnim(self, ANIM_PUSH2);
	else
		SetAnim(self, ANIM_PUSH3);
}

void ogle_push (edict_t *self, float dist)
{
	edict_t	*found = NULL;
	float	yaw;
	vec3_t	move, forward, endpos;
	trace_t	trace;
	qboolean	done = false;

	if(found = G_Find(found, FOFS(targetname), self->target))
	{
		AngleVectors(self->s.angles, forward, NULL, NULL);
		VectorMA(self->s.origin, 64, forward, endpos);
		gi.trace(self->s.origin, vec3_origin, vec3_origin, endpos, self, MASK_MONSTERSOLID, &trace);
		if(trace.ent && trace.ent == found)
		{
			yaw = self->s.angles[YAW]*M_PI*2 / 360;
			
			move[0] = cos(yaw)*dist;
			move[1] = sin(yaw)*dist;
			move[2] = 0;
			if(SV_movestep(found, move, true))
			{
				M_GetSlopePitchRoll(found, NULL);
				M_walkmove(self, self->s.angles[YAW], dist);
				return;
			}
			done = true;
		}
	}

	if(!done)
		SetAnim(self, ANIM_REST4_TRANS);
	else
	{
		gi.sound (self, CHAN_VOICE, sounds[SND_WIPE_BROW], 1, ATTN_IDLE, 0);
		SetAnim(self, ANIM_REST1_WIPE);
	}
	self->ai_mood = AI_MOOD_REST;
	self->mood_think = OgleMoodThink;
}

/*
==========================================================

	Ogle Spawn functions

==========================================================
*/

void OgleStaticsInit(void)
{
	static ClassResourceInfo_t resInfo;

	classStatics[CID_OGLE].msgReceivers[MSG_STAND]		= ogle_stand1;
	classStatics[CID_OGLE].msgReceivers[MSG_RUN]		= ogle_run1;
	classStatics[CID_OGLE].msgReceivers[MSG_MELEE]		= ogle_melee;
	classStatics[CID_OGLE].msgReceivers[MSG_DISMEMBER]  = DismemberMsgHandler;
	classStatics[CID_OGLE].msgReceivers[MSG_DEATH]		= ogle_death;
	classStatics[CID_OGLE].msgReceivers[MSG_PAIN]		= OglePainMsgHandler;
	classStatics[CID_OGLE].msgReceivers[MSG_DEATH_PAIN]		= ogle_death_pain;

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
	classStatics[CID_OGLE].msgReceivers[MSG_C_GIB1] = ai_c_gib;
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

	resInfo.numAnims = NUM_ANIMS;
	resInfo.animations = animations;
	resInfo.modelIndex = gi.modelindex("models/monsters/ogle/tris.fm");
	resInfo.numSounds = NUM_SOUNDS;
	resInfo.sounds = sounds;

	sounds[SND_PICK1]		=	gi.soundindex("monsters/ogle/pick1.wav");	
	sounds[SND_PICK2]		=	gi.soundindex("monsters/ogle/pick2.wav");	
	sounds[SND_SPIKE1]		=	gi.soundindex("monsters/ogle/spike1.wav");	
	sounds[SND_SPIKE2]		=	gi.soundindex("monsters/ogle/spike2.wav");	
	sounds[SND_HAMMER1]		=	gi.soundindex("monsters/ogle/hammer1.wav");	
	sounds[SND_HAMMER2]		=	gi.soundindex("monsters/ogle/hammer2.wav");	
	sounds[SND_PICK_FLESH]	=	gi.soundindex("monsters/ogle/pickflesh.wav");	
	sounds[SND_HAMMER_FLESH]=	gi.soundindex("monsters/ogle/hammerflesh.wav");	
	sounds[SND_WIPE_BROW]	=	gi.soundindex("monsters/ogle/wipebrow.wav");	
	sounds[SND_ENRAGE1]		=	gi.soundindex("monsters/ogle/enrage1.wav");	
	sounds[SND_ENRAGE2]		=	gi.soundindex("monsters/ogle/enrage2.wav");	
	sounds[SND_DEATH]		=	gi.soundindex("monsters/ogle/death.wav");	
	sounds[SND_CHEER1]		=	gi.soundindex("monsters/ogle/cheer1.wav");	
	sounds[SND_CHEER2]		=	gi.soundindex("monsters/ogle/cheer2.wav");	
	sounds[SND_CHEER3]		=	gi.soundindex("monsters/ogle/cheer3.wav");	

	sounds[SND_PAIN1]		=	gi.soundindex("monsters/ogle/oglemoan1.wav");
	sounds[SND_PAIN2]		=	gi.soundindex("monsters/ogle/oglemoan2.wav");
	
	//Singing
	sounds[SND_CHORUS1]		=	gi.soundindex("monsters/ogle/chorus1.wav");	
	sounds[SND_CHORUS2]		=	gi.soundindex("monsters/ogle/chorus3.wav");	
	sounds[SND_CHORUS3]		=	gi.soundindex("monsters/ogle/chorus5.wav");	
	sounds[SND_SOLO1]		=	gi.soundindex("monsters/ogle/solo2.wav");	
	sounds[SND_SOLO2]		=	gi.soundindex("monsters/ogle/solo4.wav");	

	classStatics[CID_OGLE].resInfo = &resInfo;
}

/*QUAKED monster_ogle(1 .5 0) (-16 -16 -24) (16 16 16) pushing pick_up pick_down chisel_up chisel_down hammer_up hammer_down singing CINEMATIC

The little, disgruntled Ogle

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
melee_range				= 48
missile_range			= 0
min_missile_range		= 0
bypass_missile_chance	= 0
jump_chance				= 10
wakeup_distance			= 1024

NOTE: A value of zero will result in defaults, if you actually want zero as the value, use -1
*/
void SP_monster_ogle(edict_t *self)
{
	qboolean	skip_inits = false;
	edict_t		*found = NULL;
	int chance;

	if ((deathmatch->value == 1) && !((int)sv_cheats->value & self_spawn))
	{
		G_FreeEdict (self);
		return;
	}

	self->monsterinfo.ogleflags = self->spawnflags;
	self->spawnflags = 0;//don't want incorrect handling due to weird ogle spawnflags!

	if (!M_Start(self))
		return;				// Failed initialization

	self->msgHandler = DefaultMsgHandler;
	self->monsterinfo.alert = NULL;//can't be woken up
	self->monsterinfo.dismember = ogle_dismember;
	
	if (!self->health)
		self->health = OGLE_HEALTH;

	//Apply to the end result (whether designer set or not)
	self->max_health = self->health = MonsterHealth(self->health);

	self->mass = OGLE_MASS;
	self->yaw_speed = 16;

	self->movetype = PHYSICSTYPE_STEP;
	self->solid=SOLID_BBOX;

	VectorCopy(STDMinsForClass[self->classID], self->mins);
	VectorCopy(STDMaxsForClass[self->classID], self->maxs);	

	self->materialtype = MAT_FLESH;

	self->s.modelindex = classStatics[CID_OGLE].resInfo->modelIndex;
	self->s.skinnum=0;

	if (self->monsterinfo.scale)
	{
		self->s.scale = self->monsterinfo.scale = MODEL_SCALE;
	}

	self->monsterinfo.otherenemyname = "monster_rat";	

	MG_InitMoods(self);

	self->monsterinfo.aiflags |= AI_NO_ALERT;

	self->mood_think = OgleMoodThink;

	self->use = OgleUse;
	
	chance = irand(0,4);

	if (!self->monsterinfo.ogleflags)
	{
		switch (chance)
		{
		case 0:	self->monsterinfo.ogleflags |= OF_PICK_UP;
				break;
		case 1:	self->monsterinfo.ogleflags |= OF_PICK_DOWN;
				break;
		case 2:	self->monsterinfo.ogleflags |= OF_HAMMER_UP;
				break;
		case 3:	self->monsterinfo.ogleflags |= OF_HAMMER_DOWN;
				break;
		case 4:	self->monsterinfo.ogleflags |= OF_CHISEL_UP;
				break;
		}
	}
	
	self->monsterinfo.attack_finished = level.time + irand(10,100);	

	if (self->monsterinfo.ogleflags & OF_PUSHING)
	{
		//if (self->monsterinfo.ogleflags & OF_CINEMATIC)
		if(self->targetname && self->target)
		{
			if(found = G_Find(NULL, FOFS(targetname), self->target))
				M_GetSlopePitchRoll(found, NULL);

			skip_inits = true;

			self->mood_think = NULL;
			SetAnim(self, ANIM_REST4);
			self->use = ogle_start_push;

			self->s.fmnodeinfo[MESH__NAIL].flags |= FMNI_NO_DRAW;
			self->s.fmnodeinfo[MESH__HAMMER].flags |= FMNI_NO_DRAW;
			self->s.fmnodeinfo[MESH__PICK].flags |= FMNI_NO_DRAW;
		}
		else
		{	// We gotta do SOMETHING if there is no target, otherwise the monster will puke.
#ifdef _DEVEL
			gi.dprintf("Ogle at (%s) set to push with no target.\n",
					vtos(self->s.origin));
#endif
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
		{
			if (irand(0,1))
				SetAnim(self, ANIM_WORK1);
			else
				SetAnim(self, ANIM_WORK2);
		}

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
		self->svflags|=SVF_FLOAT;
		self->monsterinfo.c_mode = 1;
	}

	self->svflags |= SVF_NO_AUTOTARGET;

	if(!skip_inits)
	{
		//Find out who our overlord is
		self->think = OgleInitOverlordThink;
		self->nextthink = level.time + 0.1;

		if(singing_ogles->value)
		{
			if(!(self->monsterinfo.ogleflags & OF_SONG_LEADER))
				ogle_check_leadsong(self);
		}

		if (self->monsterinfo.ogleflags & OF_SONG_LEADER)
			self->noise_index = 0;
	}
	else
	{
		self->think = M_WalkmonsterStartGo;
		self->nextthink = level.time + 0.1;
	}
}
