//
// m_plagueElf.c
//
// Copyright 1998 Raven Software
//

#include "m_plagueElf.h"
#include "m_plagueElf_shared.h"
#include "m_plagueElf_anim.h"
#include "m_plagueElf_moves.h"
#include "c_ai.h"
#include "mg_ai.h" //mxd
#include "mg_guide.h" //mxd
#include "g_debris.h" //mxd
#include "g_DefaultMessageHandler.h"
#include "g_monster.h"
#include "m_stats.h"
#include "Random.h"
#include "Utilities.h"
#include "Vector.h"
#include "g_local.h"

#define FIRST_SIGHT_ALONE	(VOICE_FIRST_ALONE + 1)
#define LAST_SIGHT_ALONE	(VOICE_LAST_GROUP - 1)

#define FIRST_SIGHT_GROUP	(VOICE_FIRST_GROUP + 1)
#define LAST_SIGHT_GROUP	(VOICE_LAST_GROUP - 1)

#define SE_ALONE	0
#define SE_PAIR		1
#define SE_GROUP	2

#define PLAGUEELF_SUPPORT_RADIUS	200
#define PLAGUEELF_PHASE_INTERVAL	60 //mxd

#define PALACE_ELF_SKIN	4

#pragma region ========================== Plague Elf Base Info ==========================

static const animmove_t* animations[NUM_ANIMS] =
{
	&plagueElf_move_stand1,
	&plagueElf_move_walk1,
	&plagueElf_move_walk2,
	&plagueElf_move_run1,
	&plagueElf_move_runatk1,
	&plagueElf_move_fjump,
	&plagueElf_move_inair,
	&plagueElf_move_land,
	&plagueElf_move_melee1,
	&plagueElf_move_melee2,
	&plagueElf_move_death1,
	&plagueElf_move_death2,
	&plagueElf_move_death3,
	&plagueElf_move_death4,
	&plagueElf_fist1,
	&plagueElf_lean1,
	&plagueElf_shake1,
	&plagueElf_move_pain1,
	&plagueElf_delay,
	&plagueElf_move_missile,

	&plagueElf_move_kdeath_go,
	&plagueElf_move_kdeath_loop,
	&plagueElf_move_kdeath_end,

	&plagueElf_crazy_A,
	&plagueElf_crazy_B,
	&plagueElf_cursing,
	&plagueElf_point,
	&plagueElf_scared,

	// Cinematics.
	&plagueElf_move_c_idle1,
	&plagueElf_move_c_idle2,
	&plagueElf_move_c_idle3,
	&plagueElf_move_c_walk,
	&plagueElf_move_c_walk2,
	&plagueElf_move_c_run,
	&plagueElf_move_c_attack1,
	&plagueElf_move_c_attack2,
	&plagueElf_move_c_attack3,
	&plagueElf_move_c_attack4,
	&plagueElf_move_c_pain1,
	&plagueElf_move_c_death1,
	&plagueElf_move_c_death2,
	&plagueElf_move_c_death3,
	&plagueElf_move_c_death4,
	&plagueElf_move_run1,
};

static int sounds[NUM_SOUNDS];

static const float plague_elf_voice_times[] = //mxd. Named 'pelf_VoiceTimes' in original logic.
{
	0.0f, // FIRST_SIGHT_GROUP
	1.0f, // VOICE_SIGHT_AFTER_HIM1
	0.6f, // VOICE_SIGHT_AFTER_HIM2
	1.3f, // VOICE_SIGHT_CUT_HIM2
	1.0f, // VOICE_SIGHT_CUT_HIM1
	1.2f, // VOICE_SIGHT_EAT_FLESH1
	1.2f, // VOICE_SIGHT_EAT_FLESH2
	0.9f, // VOICE_SIGHT_GET_HIM1
	0.9f, // VOICE_SIGHT_GET_HIM2
	0.9f, // VOICE_SIGHT_GET_HIM3
	1.0f, // VOICE_SIGHT_KILL_HIM1
	0.9f, // VOICE_SIGHT_KILL_HIM2
	1.3f, // VOICE_SIGHT_KILL_HIM3
	1.2f, // VOICE_SIGHT_OVER_THERE
	1.2f, // VOICE_SIGHT_THERES_ONE
	0.7f, // VOICE_SUPPORT_FOLLOW_ME
	1.5f, // VOICE_SUPPORT_CURE
	1.6f, // VOICE_SUPPORT_LIVER
	2.0f, // VOICE_SUPPORT_SLASH
	1.1f, // VOICE_SUPPORT_SURROUND_HIM
	1.8f, // VOICE_SUPPORT_UNAFFECTED1
	2.0f, // VOICE_SUPPORT_UNAFFECTED2
	1.3f, // VOICE_SUPPORT_YEAH_GET_HIM1
	1.0f, // VOICE_SUPPORT_YEAH_GET_HIM2
	0.0f, // VOICE_FIRST_ALONE
	1.1f, // VOICE_MISC_DIE
	1.3f, // VOICE_MISC_FLESH
	1.1f, // VOICE_SUPPORT_GONNA_DIE1
	1.5f, // VOICE_SUPPORT_GONNA_DIE2
	1.5f, // VOICE_SUPPORT_GONNA_DIE3
	1.2f, // VOICE_SUPPORT_GONNA_DIE4
	2.0f, // VOICE_MISC_MUST_KILL
	1.1f, // VOICE_SUPPORT_MUST_DIE
	1.1f, // VOICE_SUPPORT_YES
	0.0f, // VOICE_LAST_GROUP
	1.6f, // VOICE_MISC_GET_AWAY1
	0.9f, // VOICE_MISC_GET_AWAY2
	1.2f, // VOICE_MISC_GO_AWAY
	0.6f, // VOICE_MISC_HELP_ME1
	2.3f, // VOICE_MISC_HELP_ME2
	2.2f, // VOICE_MISC_HELP_ME3
	1.5f, // VOICE_MISC_LEAVE_ME1
	1.0f, // VOICE_MISC_LEAVE_ME2
	0.6f, // VOICE_MISC_NO
	0.9f, // VOICE_MISC_TRAPPED
	0.8f, // VOICE_MISC_COME_BACK1
	1.0f, // VOICE_MISC_COME_BACK2
	0.9f, // VOICE_MISC_DONT_HURT
};

#pragma endregion

#pragma region ========================== Utility functions =========================

void PlagueElfDyingSound(edict_t* self, const int type) //mxd. Named 'dying_elf_sounds' in original logic. Originally defined in g_obj.c.
{
#define PELF_NUM_PAIN_VOICES	4
#define PELF_NUM_IDLE_VOICES	4
#define PELF_NUM_TOUCH_VOICES	7

	static const char* dying_pelf_touch_voices[PELF_NUM_TOUCH_VOICES] =
	{
		"voices/helpe.wav",
		"voices/helpk.wav",
		"voices/helpm.wav",
		"voices/getawayb.wav",
		"voices/leavemeb.wav",
		"voices/mercyp.wav",
		"voices/nomrj.wav",
	};

	static const char* dying_pelf_pain_voices[PELF_NUM_PAIN_VOICES] =
	{
		"voices/getawayb.wav",
		"voices/leavemeb.wav",
		"voices/mercyp.wav",
		"voices/nomrj.wav",
	};

	static const char* dying_pelf_idle_voices[PELF_NUM_IDLE_VOICES] =
	{
		"pelfgasp.wav",
		"pelfpant.wav",
		"pelfshiv.wav",
		"pelfsigh.wav",
	};

	char sound_string[1024];
	strcpy_s(sound_string, sizeof(sound_string), "monsters/plagueElf/"); //mxd. strcpy -> strcpy_s

	switch (type)
	{
		case DYING_ELF_PAIN_VOICE:
			strcat_s(sound_string, sizeof(sound_string), dying_pelf_pain_voices[irand(0, PELF_NUM_PAIN_VOICES - 1)]); //mxd. strcat -> strcat_s
			gi.sound(self, CHAN_VOICE, gi.soundindex(sound_string), 1.0f, ATTN_NORM, 0.0f);
			break;

		case DYING_ELF_IDLE_VOICE:
			strcat_s(sound_string, sizeof(sound_string), dying_pelf_idle_voices[irand(0, PELF_NUM_IDLE_VOICES - 1)]); //mxd. strcat -> strcat_s
			gi.sound(self, CHAN_VOICE, gi.soundindex(sound_string), 1.0f, ATTN_IDLE, 0.0f);
			break;

		case DYING_ELF_TOUCH_VOICE:
			strcat_s(sound_string, sizeof(sound_string), dying_pelf_touch_voices[irand(0, PELF_NUM_TOUCH_VOICES - 1)]); //mxd. strcat -> strcat_s
			gi.sound(self, CHAN_VOICE, gi.soundindex(sound_string), 1.0f, ATTN_NORM, 0.0f);
			break;
	}
}

static void PlagueElfSpellInit(edict_t* spell) //mxd. Named 'create_pe_spell' in original logic.
{
	spell->movetype = MOVETYPE_FLYMISSILE;
	spell->solid = SOLID_BBOX;
	spell->classname = "plagueElf_Spell";
	spell->enemy = NULL;
	spell->clipmask = (MASK_MONSTERSOLID | MASK_PLAYERSOLID | MASK_SHOT);
	spell->s.scale = 0.5f;
	spell->s.effects |= (EF_MARCUS_FLAG1 | EF_CAMERA_NO_CLIP);
	spell->svflags |= SVF_ALWAYS_SEND;
	spell->touch = PlagueElfSpellTouch;
}

// The spell needs to bounce.
static void PlagueElfReflectSpellInit(edict_t* self, edict_t* spell) //mxd. Named 'make_pe_spell_reflect' in original logic.
{
	PlagueElfSpellInit(spell);

	spell->s.modelindex = self->s.modelindex;
	spell->owner = self->owner;
	spell->enemy = self->enemy;
	spell->touch = self->touch;
	spell->plagueelf_spell_fx_type = self->plagueelf_spell_fx_type;

	Create_rand_relect_vect(self->velocity, spell->velocity);
	VectorCopy(self->s.origin, spell->s.origin);

	vectoangles(spell->velocity, spell->s.angles);
	spell->s.angles[YAW] += 90.0f;

	Vec3ScaleAssign(500.0f, spell->velocity);

	spell->think = G_FreeEdict;
	spell->nextthink = self->nextthink;

	G_LinkMissile(spell);
}

static void PlagueElfDismemberSound(edict_t* self) //mxd. Named 'pelf_dismember_sound' in original logic.
{
	if (self->health > 0)
		gi.sound(self, CHAN_BODY, sounds[irand(SND_DISMEMBER1, SND_DISMEMBER2)], 1.0f, ATTN_NORM, 0.0f);
}

static qboolean PlagueElfCanThrowNode(edict_t* self, const int node_id, int* throw_nodes) //mxd. Named 'canthrownode_pe' in original logic.
{
	static const int bit_for_mesh_node[NUM_MESH_NODES] = //mxd. Made local static.
	{
		BIT_BASE,
		BIT_HANDLE,
		BIT_HOE,
		BIT_GAFF,
		BIT_HAMMER,
		BIT_BODY,
		BIT_L_LEG,
		BIT_R_LEG,
		BIT_R_ARM,
		BIT_L_ARM,
		BIT_HEAD
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

static void PlagueElfTryFlee(edict_t* self, const int coward_chance, const int flee_chance, const float flee_time) //mxd. Named 'plagueElf_chicken' in original logic.
{
	const int chance = irand(0, 20 + SKILL * 10); //mxd. flrand() in original logic.

	if (chance < coward_chance)
	{
		self->monsterinfo.aiflags |= AI_COWARD;
	}
	else if (chance < flee_chance)
	{
		self->monsterinfo.aiflags |= AI_FLEE;
		self->monsterinfo.flee_finished = level.time + flee_time;
	}
}

// Throws weapon, turns off those nodes, sets that weapon as gone.
static void PlagueElfDropWeapon(edict_t* self) //mxd. Named 'plagueElf_dropweapon' in original logic. Removed unused 'damage' arg.
{
	// NO PART FLY FRAME!
	if (self->s.fmnodeinfo[MESH__HANDLE].flags & FMNI_NO_DRAW)
		return;

	vec3_t forward;
	vec3_t right;
	vec3_t up;
	AngleVectors(self->s.angles, forward, right, up);

	vec3_t hand_spot = { 0 };
	VectorMA(hand_spot, 5.0f, forward, hand_spot);
	VectorMA(hand_spot, 8.0f, right, hand_spot);
	VectorMA(hand_spot, -6.0f, up, hand_spot);

	const int chance = ((self->dead_state == DEAD_DEAD || (self->s.fmnodeinfo[MESH__R_ARM].flags & FMNI_NO_DRAW)) ? 0 : 3);

	if (!(self->s.fmnodeinfo[MESH__HOE].flags & FMNI_NO_DRAW))
	{
		if (irand(0, 10) < chance)
		{
			// Just take off top.
			ThrowWeapon(self, &hand_spot, BIT_HOE, 0.0f, FRAME_torsooff);
			self->s.fmnodeinfo[MESH__HOE].flags |= FMNI_NO_DRAW;
			PlagueElfTryFlee(self, 2, 5, flrand(2.0f, 7.0f));
		}
		else
		{
			ThrowWeapon(self, &hand_spot, BIT_HANDLE | BIT_HOE, 0.0f, FRAME_partfly);
			self->s.fmnodeinfo[MESH__HOE].flags |= FMNI_NO_DRAW;
			self->s.fmnodeinfo[MESH__HANDLE].flags |= FMNI_NO_DRAW;
			PlagueElfTryFlee(self, 4, 8, flrand(3.0f, 8.0f));
		}

		return;
	}

	if (!(self->s.fmnodeinfo[MESH__GAFF].flags & FMNI_NO_DRAW))
	{
		if (irand(0, 10) < chance)
		{
			// Just take off top.
			ThrowWeapon(self, &hand_spot, BIT_GAFF, 0.0f, FRAME_partfly);
			self->s.fmnodeinfo[MESH__GAFF].flags |= FMNI_NO_DRAW;
			PlagueElfTryFlee(self, 2, 6, flrand(2.0f, 7.0f));
		}
		else
		{
			ThrowWeapon(self, &hand_spot, BIT_HANDLE | BIT_GAFF, 0.0f, FRAME_partfly);
			self->s.fmnodeinfo[MESH__GAFF].flags |= FMNI_NO_DRAW;
			self->s.fmnodeinfo[MESH__HANDLE].flags |= FMNI_NO_DRAW;
			PlagueElfTryFlee(self, 4, 8, flrand(3.0f, 8.0f));
		}

		return;
	}

	if (!(self->s.fmnodeinfo[MESH__HAMMER].flags & FMNI_NO_DRAW))
	{
		if (irand(0, 10) < chance)
		{
			// Just take off top.
			ThrowWeapon(self, &hand_spot, BIT_HAMMER, 0.0f, FRAME_partfly);
			self->s.fmnodeinfo[MESH__HAMMER].flags |= FMNI_NO_DRAW;
			PlagueElfTryFlee(self, 2, 6, flrand(2.0f, 7.0f));
		}
		else
		{
			ThrowWeapon(self, &hand_spot, BIT_HANDLE | BIT_HAMMER, 0.0f, FRAME_partfly);
			self->s.fmnodeinfo[MESH__HAMMER].flags |= FMNI_NO_DRAW;
			self->s.fmnodeinfo[MESH__HANDLE].flags |= FMNI_NO_DRAW;
			PlagueElfTryFlee(self, 4, 8, flrand(3.0f, 8.0f));
		}

		return;
	}

	ThrowWeapon(self, &hand_spot, BIT_HANDLE, 0.0f, FRAME_partfly);
	self->s.fmnodeinfo[MESH__HANDLE].flags |= FMNI_NO_DRAW;

	if (self->dead_state != DEAD_DEAD)
		PlagueElfTryFlee(self, 6, 8, flrand(5.0f, 10.0f));
}

static int PlagueElfChooseSightSound(const int event) //mxd. Named 'pelf_ChooseSightSound' in original logic. Removed unused 'self' arg.
{
	switch (event)
	{
		case SE_ALONE:
			return irand(FIRST_SIGHT_ALONE, LAST_SIGHT_ALONE);

		case SE_PAIR:
		case SE_GROUP:
			return irand(FIRST_SIGHT_GROUP, LAST_SIGHT_GROUP);

		default:
			return FIRST_SIGHT_ALONE; //mxd. Should never happen.
	}
}

//TODO: more varied responses based on sound_id.
static int PlagueElfChooseResponseSound(const int event, int sound_id) //mxd. Named 'pelf_ChooseResponseSound' in original logic. Removed unused 'self' arg.
{
	switch (event)
	{
		case SE_PAIR:
		case SE_GROUP:
			return irand(FIRST_SIGHT_GROUP, LAST_SIGHT_GROUP);

		default:
			return FIRST_SIGHT_GROUP; //mxd. Should never happen.
	}
}

// Plague elf has said something and is looking for a response.
static void PlagueElfPollResponse(const edict_t* self, const int sound_event, const int sound_id, const float time) //mxd. Named 'pelf_PollResponse' in original logic.
{
	edict_t* ent = NULL;
	edict_t* last_valid = NULL;

	while ((ent = FindInRadius(ent, self->s.origin, PLAGUEELF_SUPPORT_RADIUS)) != NULL)
	{
		// Not us.
		if (ent == self || ent->classID != self->classID || ent->health <= 0)
			continue;

		// Not already talking.
		if (ent->monsterinfo.sound_pending > 0 || ent->monsterinfo.sound_finished > level.time)
			continue;

		// Not going after different goals.
		if (ent->enemy != NULL && ent->enemy != self->enemy)
			continue;

		if (!AI_OkToWake(ent, false, false))
			continue;

		// Save this as valid!
		last_valid = ent;

		// Random chance to continue on.
		if (irand(0, 1) == 1)
			break;
	}

	// Make the last valid one talk.
	if (last_valid != NULL)
	{
		if (last_valid->enemy == NULL)
		{
			last_valid->enemy = self->enemy;
			AI_FoundTarget(last_valid, false);
		}

		// This is the elf to respond, so post the message.
		G_PostMessage(last_valid, MSG_VOICE_POLL, PRI_DIRECTIVE, "bbf", sound_event, sound_id, time);
	}
}

static void PlagueElfPhaseOutInit(edict_t* self) //mxd. Named 'pelf_init_phase_out' in original logic.
{
	if (Q_stricmp(self->classname, "monster_palace_plague_guard_invisible") == 0) //mxd. stricmp -> Q_stricmp.
	{
		self->pre_think = PlagueElfPhaseOutPreThink;
		self->next_pre_think = level.time + FRAMETIME;
		self->svflags |= SVF_NO_AUTOTARGET;
	}
}

static void PlagueElfPhaseInInit(edict_t* self) //mxd. Named 'pelf_init_phase_in' in original logic.
{
	if (Q_stricmp(self->classname, "monster_palace_plague_guard_invisible") == 0) //mxd. stricmp -> Q_stricmp.
	{
		self->pre_think = PlagueElfPhaseInPreThink;
		self->next_pre_think = level.time + FRAMETIME;
	}
}

#pragma endregion

#pragma region ========================== Message handlers ==========================

static void PlagueElfCinematicActionMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'plagueElf_c_anims' in original logic.
{
	int curr_anim;

	ReadCinematicMessage(self, msg);
	self->monsterinfo.c_anim_flag = 0;

	switch (msg->ID)
	{
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

		case MSG_C_ATTACK4:
			self->monsterinfo.c_anim_flag = C_ANIM_REPEAT;
			curr_anim = ANIM_C_ATTACK4;
			break;

		case MSG_C_DEATH1:
			self->monsterinfo.c_anim_flag = C_ANIM_DONE;
			curr_anim = ANIM_C_DEATH1;
			break;

		case MSG_C_DEATH2:
			self->monsterinfo.c_anim_flag = C_ANIM_DONE;
			curr_anim = ANIM_C_DEATH2;
			break;

		case MSG_C_DEATH3:
			self->monsterinfo.c_anim_flag = C_ANIM_DONE;
			curr_anim = ANIM_C_DEATH3;
			break;

		case MSG_C_DEATH4:
			self->monsterinfo.c_anim_flag = C_ANIM_DONE;
			curr_anim = ANIM_C_DEATH4;
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

		case MSG_C_PAIN1:
			self->monsterinfo.c_anim_flag = C_ANIM_REPEAT;
			curr_anim = ANIM_C_PAIN1;
			break;

		case MSG_C_RUN1:
			self->monsterinfo.c_anim_flag = C_ANIM_MOVE;
			curr_anim = ANIM_C_RUN1;
			break;

		case MSG_C_THINKAGAIN: // Think for yourself, elf.
			self->enemy = self->monsterinfo.c_ent;
			AI_FoundTarget(self, true);
			curr_anim = ANIM_C_THINKAGAIN;
			break;

		case MSG_C_WALK1:
			self->monsterinfo.c_anim_flag = C_ANIM_MOVE;
			curr_anim = ANIM_C_WALK1;
			break;

		case MSG_C_WALK2:
			self->monsterinfo.c_anim_flag = C_ANIM_MOVE;
			curr_anim = ANIM_C_WALK2;
			break;

		default:
			return; //mxd. 'break' in original logic. Let's avoid using uninitialized curr_anim var.
	}

	SetAnim(self, curr_anim);
}

static void PlagueElfDeathMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'plagueElf_death' in original logic.
{
	PlagueElfPhaseInInit(self);

	edict_t* target;
	edict_t* inflictor;
	edict_t* attacker;
	float damage;
	G_ParseMsgParms(msg, "eeei", &target, &inflictor, &attacker, &damage);

	// Big enough death to be thrown back.
	if (self->monsterinfo.aiflags & AI_DONT_THINK)
	{
		SetAnim(self, irand(ANIM_DIE1, ANIM_DIE2));
		return;
	}

	self->msgHandler = DeadMsgHandler;

	// Dead but still being hit.
	if (self->dead_state == DEAD_DEAD)
		return;

	self->dead_state = DEAD_DEAD;
	PlagueElfDropWeapon(self);

	if (self->health <= -80) // Gib death.
	{
		gi.sound(self, CHAN_BODY, sounds[SND_GIB], 1.0f, ATTN_NORM, 0.0f);
		BecomeDebris(self);

		return;
	}

	if (self->health < -10)
	{
		self->svflags |= SVF_DEADMONSTER;
		SetAnim(self, ANIM_KDEATH_GO);

		vec3_t dir;
		VectorNormalize2(target->velocity, dir);

		vec3_t yaw_dir;
		VectorScale(dir, -1.0f, yaw_dir);

		self->ideal_yaw = VectorYaw(yaw_dir);
		self->yaw_speed = 48.0f;

		VectorScale(dir, 250.0f, self->velocity);
		self->velocity[2] = flrand(150.0f, 200.0f); //mxd. irand() in original logic.

		return;
	}

	// Regular death.
	if (self->count == 0) //TODO: self->count doesn't seem to be set anywhere.
		SetAnim(self, irand(ANIM_DIE1, ANIM_DIE4));
	else if (self->count == 1)
		SetAnim(self, ANIM_DIE1);
	else if (self->count == 2)
		SetAnim(self, ANIM_DIE2);
	else if (self->count == 3)
		SetAnim(self, ANIM_DIE3);
	else
		SetAnim(self, ANIM_DIE4);
}

static void PlagueElfMissileMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'plagueElf_missile' in original logic.
{
	PlagueElfPhaseInInit(self);
	SetAnim(self, ANIM_MISSILE);
}

static void PlagueElfMeleeMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'plagueElf_melee' in original logic.
{
	if (!M_ValidTarget(self, self->enemy))
	{
		// If our enemy is dead, we need to stand.
		G_PostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
		return;
	}

	// A monster in melee will continue too long if the player backs away, this prevents it.
	if ((self->spawnflags & MSF_FIXED) || (self->monsterinfo.aiflags & AI_NO_MELEE))
	{
		SetAnim(self, ANIM_MISSILE);
		return;
	}

	vec3_t forward;
	AngleVectors(self->s.angles, forward, NULL, NULL);
	Vec3ScaleAssign(2.0f, forward);

	if (M_PredictTargetEvasion(self, self->enemy, forward, self->enemy->velocity, self->melee_range, 5.0f))
		SetAnim(self, irand(ANIM_MELEE1, ANIM_MELEE2));
	else if (self->spawnflags & MSF_FIXED)
		SetAnim(self, ANIM_DELAY);
	else
		SetAnim(self, ANIM_RUNATK1);
}

static void PlagueElfDeadPainMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'plagueElf_dead_pain' in original logic.
{
	if (msg != NULL && !(self->svflags & SVF_PARTS_GIBBED))
		DismemberMsgHandler(self, msg);
}

static void PlagueElfPainMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'plagueElf_pain' in original logic.
{
	int temp;
	int damage;
	qboolean force_pain;
	G_ParseMsgParms(msg, "eeiii", &temp, &temp, &force_pain, &damage, &temp);

	PlagueElfPhaseInInit(self);

	if (!force_pain && !(self->monsterinfo.aiflags & AI_COWARD) && !(self->monsterinfo.aiflags & AI_FLEE) && irand(0, self->health) > damage) //mxd. flrand() in original logic.
		return;

	if (self->fire_damage_time > level.time)
		self->monsterinfo.aiflags |= AI_COWARD;

	if (self->pain_debounce_time < level.time)
	{
		self->pain_debounce_time = level.time + 1.0f;
		SetAnim(self, ANIM_PAIN1);
	}
}

static void PlagueElfJumpMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'pelf_jump' in original logic.
{
	SetAnim(self, ((self->spawnflags & MSF_FIXED) ? ANIM_DELAY : ANIM_FJUMP));
}

static void PlagueElfCheckMoodMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'pelf_check_mood' in original logic.
{
	G_ParseMsgParms(msg, "i", &self->ai_mood);
	plagueelf_pause(self);
}

static void PlagueElfRunMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'plagueElf_run' in original logic.
{
	if (self->curAnimID == ANIM_CURSING || self->curAnimID == ANIM_POINT)
		return;

	if (M_ValidTarget(self, self->enemy))
		SetAnim(self, ((self->spawnflags & MSF_FIXED) ? ANIM_DELAY : ANIM_RUN1));
	else // If our enemy is dead, we need to stand.
		G_PostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
}

static void PlagueElfStandMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'plagueElf_stand' in original logic.
{
	SetAnim(self, ((self->ai_mood == AI_MOOD_DELAY) ? ANIM_DELAY : ANIM_SHAKE1));
}

static void PlagueElfWalkMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'plagueElf_walk' in original logic.
{
	if (self->curAnimID == ANIM_CURSING || self->curAnimID == ANIM_POINT)
		return;

	if (self->spawnflags & MSF_FIXED)
		SetAnim(self, ANIM_DELAY);
	else
		SetAnim(self, irand(ANIM_WALK1, ANIM_WALK2));
}

// Plague elf has seen first target (usually player).
static void PlagueElfVoiceSightMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'pelf_SightSound' in original logic.
{
	if (self->targetname != NULL || self->monsterinfo.c_mode)
		return; // Cinematic waiting to be activated, don't do this.

	// Have we already said something?
	if (self->monsterinfo.supporters != -1)
		return;

	edict_t* enemy = NULL;
	byte sight_type;
	G_ParseMsgParms(msg, "be", &sight_type, &enemy);

	// Find out how many elves are around (save this if we want it later).
	self->monsterinfo.supporters = M_FindSupport(self, PLAGUEELF_SUPPORT_RADIUS);

	// See if we are the first to see the player.
	if (M_CheckAlert(self, PLAGUEELF_SUPPORT_RADIUS))
	{
		if (self->monsterinfo.supporters < 1) // Loner.
		{
			const int sound_id = PlagueElfChooseSightSound(SE_ALONE);
			gi.sound(self, CHAN_VOICE, sounds[sound_id], 1.0f, ATTN_NORM, 0.0f);
		}
		else // Paired or grouped.
		{
			const int event_type = (self->monsterinfo.supporters < 2 ? SE_PAIR : SE_GROUP); //mxd
			const int sound_id = PlagueElfChooseSightSound(event_type);

			self->monsterinfo.sound_finished = level.time + plague_elf_voice_times[sound_id];
			gi.sound(self, CHAN_VOICE, sounds[sound_id], 1, ATTN_NORM, 0);

			PlagueElfPollResponse(self, event_type, sound_id, self->monsterinfo.sound_finished - flrand(0.5f, 0.25f));
		}

		if (self->monsterinfo.supporters > 0)
			SetAnim(self, irand(ANIM_CURSING, ANIM_POINT)); //FIXME: make sure enemy is far enough away to anim!
	}
}

// A plague elf has been polled for a response, now choose a reply and echo it back.
static void PlagueElfVoicePollMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'pelf_EchoResponse' in original logic.
{
	if (self->targetname != NULL || self->monsterinfo.c_mode)
		return; // Cinematic waiting to be activated, don't do this.

	int sound_event;
	int sound_id;
	float time;
	G_ParseMsgParms(msg, "bbf", &sound_event, &sound_id, &time);

	self->monsterinfo.sound_start = time;
	self->monsterinfo.sound_finished = level.time + plague_elf_voice_times[self->monsterinfo.sound_pending];

	switch (sound_event)
	{
		case SE_PAIR:
			self->monsterinfo.sound_pending = PlagueElfChooseResponseSound(SE_PAIR, sound_id);

			if (irand(0, 4) > 0)
				SetAnim(self, irand(ANIM_CURSING, ANIM_POINT)); //FIXME: make sure enemy is far enough away to anim!
			break;

		case SE_GROUP:
			self->monsterinfo.sound_pending = PlagueElfChooseResponseSound(SE_GROUP, sound_id);

			if (irand(0, 2) > 0)
				PlagueElfPollResponse(self, SE_GROUP, self->monsterinfo.sound_pending, self->monsterinfo.sound_finished);

			if (irand(0, 2) > 0)
				SetAnim(self, irand(ANIM_CURSING, ANIM_POINT)); //FIXME: make sure enemy is far enough away to anim!
			break;

		default:
			self->monsterinfo.sound_pending = PlagueElfChooseResponseSound(SE_GROUP, sound_id);

			if (irand(0, 2))
				PlagueElfPollResponse(self, SE_GROUP, self->monsterinfo.sound_pending, self->monsterinfo.sound_finished);
			break;
	}
}

// Play a sound from a trigger or a pending sound event.
static void PlagueElfVoicePuppetMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'pelf_EchoSound' in original logic.
{
	int sound;
	G_ParseMsgParms(msg, "i", &sound);

	gi.sound(self, CHAN_VOICE, sounds[sound], 1.0f, ATTN_NORM, 0.0f);
}

#pragma endregion

#pragma region ========================== Edict callbacks ===========================

void PlagueElfSpellTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surface) //mxd. Named 'plagueElfSpellTouch' in original logic.
{
	if (surface != NULL && (surface->flags & SURF_SKY))
	{
		SkyFly(self);
		return;
	}

	if (EntReflecting(other, true, true))
	{
		edict_t* spell = G_Spawn();

		PlagueElfReflectSpellInit(self, spell);
		gi.CreateEffect(&spell->s, FX_PE_SPELL, CEF_OWNERS_ORIGIN, NULL, "bv", self->plagueelf_spell_fx_type, spell->velocity);
		G_SetToFree(self);

		return;
	}

	VectorNormalize(self->velocity);

	switch (self->plagueelf_spell_fx_type)
	{
		case FX_PE_MAKE_SPELL:
			gi.CreateEffect(NULL, FX_PE_SPELL, 0, self->s.origin, "bv", FX_PE_EXPLODE_SPELL, self->velocity);
			self->dmg = irand(PLAGUEELF_DMG_SPELL_MIN, PLAGUEELF_DMG_SPELL_MAX) + SKILL;
			break;

		case FX_PE_MAKE_SPELL2:
			gi.CreateEffect(NULL, FX_PE_SPELL, 0, self->s.origin, "bv", FX_PE_EXPLODE_SPELL2, self->velocity);
			self->dmg = irand(PLAGUEELF_GUARD_DMG_SPELL_MIN, PLAGUEELF_GUARD_DMG_SPELL_MAX);
			break;

		case FX_PE_MAKE_SPELL3:
			gi.CreateEffect(NULL, FX_PE_SPELL, 0, self->s.origin, "bv", FX_PE_EXPLODE_SPELL3, self->velocity);
			self->dmg = irand(PLAGUEELF_GUARD_DMG_SPELL_MIN, PLAGUEELF_GUARD_DMG_SPELL_MAX) + 2;
			self->dmg_radius = 40.0f;
			break;
	}

	if (other->takedamage != DAMAGE_NO)
	{
		vec3_t normal;

		if (plane != NULL)
			VectorCopy(plane->normal, normal);
		else
			VectorCopy(vec3_up, normal);

		T_Damage(other, self, self->owner, self->movedir, self->s.origin, normal, self->dmg, 0, DAMAGE_SPELL, MOD_DIED);
	}

	if (self->dmg_radius > 0.0f)
		T_DamageRadius(self, self->owner, self->owner, self->dmg_radius, (float)self->dmg, 0, DAMAGE_SPELL, MOD_DIED);

	VectorClear(self->velocity);
	G_FreeEdict(self);
}

static qboolean PlagueElfThrowHead(edict_t* self, float damage, const qboolean dismember_ok, int* throw_nodes) //mxd. Added to simplify logic.
{
	if (self->s.fmnodeinfo[MESH__HEAD].flags & FMNI_NO_DRAW)
		return false;

	if (self->s.fmnodeinfo[MESH__HEAD].flags & FMNI_USE_SKIN)
		damage *= 1.5f; // Greater chance to cut off if previously damaged.

	if (flrand(0, (float)self->health) < damage * 0.25f)
		PlagueElfDropWeapon(self);

	if (dismember_ok && flrand(0, (float)self->health) < damage * 0.3f)
	{
		PlagueElfCanThrowNode(self, MESH__HEAD, throw_nodes);
		PlagueElfDismemberSound(self);

		vec3_t gore_spot = { 0.0f, 0.0f, 18.0f };
		ThrowBodyPart(self, &gore_spot, *throw_nodes, damage, 0);

		Vec3AddAssign(self->s.origin, gore_spot);
		SprayDebris(self, gore_spot, 8, damage);

		if (self->health > 0)
		{
			self->health = 1;
			T_Damage(self, self, self, vec3_origin, vec3_origin, vec3_origin, 10, 20, 0, MOD_DIED);
		}

		return true;
	}

	self->s.fmnodeinfo[MESH__HEAD].flags |= FMNI_USE_SKIN;
	self->s.fmnodeinfo[MESH__HEAD].skin = self->s.skinnum + 1;

	return false;
}

static qboolean PlagueElfThrowTorso(edict_t* self, float damage, const qboolean dismember_ok, int* throw_nodes) //mxd. Added to simplify logic.
{
	if (self->s.fmnodeinfo[MESH__BODY].flags & FMNI_NO_DRAW)
		return false;

	if (self->s.fmnodeinfo[MESH__BODY].flags & FMNI_USE_SKIN)
		damage *= 1.5f; // Greater chance to cut off if previously damaged.

	if (dismember_ok && self->health > 0 && flrand(0, (float)self->health) < damage * 0.3f)
	{
		PlagueElfCanThrowNode(self, MESH__BODY, throw_nodes);
		PlagueElfCanThrowNode(self, MESH__L_ARM, throw_nodes);
		PlagueElfCanThrowNode(self, MESH__R_ARM, throw_nodes);
		PlagueElfCanThrowNode(self, MESH__HEAD, throw_nodes);

		PlagueElfDropWeapon(self);
		PlagueElfDismemberSound(self);

		vec3_t gore_spot = { 0.0f, 0.0f, 12.0f };
		ThrowBodyPart(self, &gore_spot, *throw_nodes, damage, FRAME_torsooff);

		Vec3AddAssign(self->s.origin, gore_spot);
		SprayDebris(self, gore_spot, 12, damage);

		if (self->health > 0)
		{
			self->health = 1;
			T_Damage(self, self, self, vec3_origin, vec3_origin, vec3_origin, 10, 20, 0, MOD_DIED);
		}

		return true;
	}

	if (flrand(0, (float)self->health) < damage * 0.5f)
		PlagueElfDropWeapon(self);

	self->s.fmnodeinfo[MESH__BODY].flags |= FMNI_USE_SKIN;
	self->s.fmnodeinfo[MESH__BODY].skin = self->s.skinnum + 1;

	return false;
}

static void PlagueElfThrowLeftArm(edict_t* self, float damage, const qboolean dismember_ok, int* throw_nodes) //mxd. Added to simplify logic.
{
	if (self->s.fmnodeinfo[MESH__L_ARM].flags & FMNI_NO_DRAW)
		return;

	if (self->s.fmnodeinfo[MESH__L_ARM].flags & FMNI_USE_SKIN)
		damage *= 1.5f; // Greater chance to cut off if previously damaged.

	if (flrand(0, (float)self->health) < damage * 0.4f)
		PlagueElfDropWeapon(self);

	if (dismember_ok && flrand(0, (float)self->health) < damage * 0.75f)
	{
		if (PlagueElfCanThrowNode(self, MESH__L_ARM, throw_nodes))
		{
			PlagueElfTryFlee(self, 6, 8, flrand(7.0f, 15.0f));
			PlagueElfDismemberSound(self);

			vec3_t right;
			AngleVectors(self->s.angles, NULL, right, NULL);

			vec3_t gore_spot = { 0.0f, 0.0f, self->maxs[2] * 0.3f };
			VectorMA(gore_spot, -10.0f, right, gore_spot);

			ThrowBodyPart(self, &gore_spot, *throw_nodes, damage, 0);
		}
	}
	else
	{
		self->s.fmnodeinfo[MESH__L_ARM].flags |= FMNI_USE_SKIN;
		self->s.fmnodeinfo[MESH__L_ARM].skin = self->s.skinnum + 1;
	}
}

static void PlagueElfThrowRightArm(edict_t* self, float damage, const qboolean dismember_ok, int* throw_nodes) //mxd. Added to simplify logic.
{
	if (self->s.fmnodeinfo[MESH__R_ARM].flags & FMNI_NO_DRAW)
		return;

	if (self->s.fmnodeinfo[MESH__R_ARM].flags & FMNI_USE_SKIN)
		damage *= 1.5f; // Greater chance to cut off if previously damaged.

	if (dismember_ok && flrand(0, (float)self->health) < damage * 0.75f)
	{
		if (PlagueElfCanThrowNode(self, MESH__R_ARM, throw_nodes))
		{
			PlagueElfDropWeapon(self);
			PlagueElfDismemberSound(self);

			vec3_t right;
			AngleVectors(self->s.angles, NULL, right, NULL);

			vec3_t gore_spot = { 0.0f, 0.0f, self->maxs[2] * 0.3f };
			VectorMA(gore_spot, 10.0f, right, gore_spot);

			ThrowBodyPart(self, &gore_spot, *throw_nodes, damage, 0);
		}
	}
	else
	{
		if (flrand(0, (float)self->health) < damage * 0.75f)
			PlagueElfDropWeapon(self);

		self->s.fmnodeinfo[MESH__R_ARM].flags |= FMNI_USE_SKIN;
		self->s.fmnodeinfo[MESH__R_ARM].skin = self->s.skinnum + 1;
	}
}

static void PlagueElfThrowLeg(edict_t* self, const float damage, const int mesh_part, int* throw_nodes) //mxd. Added to simplify logic.
{
	// Still alive?
	if (self->health > 0)
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

		if (PlagueElfCanThrowNode(self, mesh_part, throw_nodes))
		{
			vec3_t right;
			AngleVectors(self->s.angles, NULL, right, NULL);

			vec3_t gore_spot = { 0.0f, 0.0f, self->maxs[2] * 0.3f };
			const float side = (mesh_part == MESH__L_LEG ? -1.0f : 1.0f); //BUGFIX: original logic scales both legs by -10.
			VectorMA(gore_spot, 10.0f * side, right, gore_spot);

			ThrowBodyPart(self, &gore_spot, *throw_nodes, damage, 0);
		}
	}
}

void PlagueElfDismember(edict_t* self, int damage, HitLocation_t hl) //mxd. Named 'plagueElf_dismember' in original logic.
{
	//FIXME: throw current weapon.
	//FIXME: make part fly dir the vector from hit loc to sever loc.
	qboolean dismember_ok = false;

	if (hl & hl_MeleeHit)
	{
		dismember_ok = true;
		hl &= ~hl_MeleeHit;
	}

	if (hl <= hl_NoneSpecific || hl >= hl_Max) //mxd. '> hl_Max' in original logic.
		return;

	// Hit chest during melee, may have hit arms.
	if ((self->curAnimID == ANIM_MELEE1 || self->curAnimID == ANIM_MELEE2) && hl == hl_TorsoFront && irand(0, 10) < 4) //BUGFIX: mxd. Original logic checks ANIM_MELEE1 two times.
		hl = ((irand(0, 10) < 7) ? hl_ArmLowerRight : hl_ArmLowerLeft);

	if ((hl == hl_ArmUpperLeft && self->s.fmnodeinfo[MESH__L_ARM].flags & FMNI_NO_DRAW) ||
		(hl == hl_ArmUpperRight && self->s.fmnodeinfo[MESH__R_ARM].flags & FMNI_NO_DRAW) ||
		((hl == hl_TorsoFront || hl == hl_TorsoBack) && self->s.fmnodeinfo[MESH__R_ARM].flags & FMNI_NO_DRAW && self->s.fmnodeinfo[MESH__L_ARM].flags & FMNI_NO_DRAW && irand(0, 10) < 4))
		hl = hl_Head; // Decapitate.

	int throw_nodes = 0;

	switch (hl)
	{
		case hl_Head:
			if (PlagueElfThrowHead(self, (float)damage, dismember_ok, &throw_nodes))
				return;
			break;

		case hl_TorsoFront: // Split in half?
		case hl_TorsoBack:
			if (PlagueElfThrowTorso(self, (float)damage, dismember_ok, &throw_nodes))
				return;
			break;

		case hl_ArmUpperLeft: // Left arm.
		case hl_ArmLowerLeft:
			PlagueElfThrowLeftArm(self, (float)damage, dismember_ok, &throw_nodes);
			break;

		case hl_ArmUpperRight: // Right arm.
		case hl_ArmLowerRight:
			PlagueElfThrowRightArm(self, (float)damage, dismember_ok, &throw_nodes);
			break;

		case hl_LegUpperLeft: // Left leg.
		case hl_LegLowerLeft:
			PlagueElfThrowLeg(self, (float)damage, MESH__L_LEG, &throw_nodes);
			break;

		case hl_LegUpperRight: // Right leg.
		case hl_LegLowerRight:
			PlagueElfThrowLeg(self, (float)damage, MESH__R_LEG, &throw_nodes);
			break;

		default:
			if (flrand(0, (float)self->health) < (float)damage * 0.25f)
				PlagueElfDropWeapon(self);
			break;
	}

	if (throw_nodes > 0)
		self->pain_debounce_time = 0.0f;

	if (self->s.fmnodeinfo[MESH__R_ARM].flags & FMNI_NO_DRAW)
	{
		self->monsterinfo.aiflags |= AI_NO_MELEE;
		self->monsterinfo.aiflags |= AI_NO_MISSILE;
	}
	else if (self->s.fmnodeinfo[MESH__HANDLE].flags & FMNI_NO_DRAW)
	{
		self->monsterinfo.aiflags |= AI_NO_MELEE;

		if (self->missile_range > 0.0f)
		{
			if (!(self->s.fmnodeinfo[MESH__R_ARM].flags & FMNI_NO_DRAW))
			{
				self->monsterinfo.aiflags &= ~AI_NO_MISSILE;
				self->melee_range = 0.0f;
			}
			else
			{
				self->monsterinfo.aiflags |= AI_NO_MISSILE;
			}
		}
	}

	if ((self->monsterinfo.aiflags & AI_NO_MISSILE) && (self->monsterinfo.aiflags & AI_NO_MELEE))
		self->monsterinfo.aiflags |= AI_COWARD;
}

void PlagueElfPhaseOutPreThink(edict_t* self) //mxd. Named 'pelf_phase_out' in original logic.
{
	if (self->s.color.a > PLAGUEELF_PHASE_INTERVAL)
	{
		self->s.color.a -= (byte)irand(PLAGUEELF_PHASE_INTERVAL / 2, PLAGUEELF_PHASE_INTERVAL);
		self->pre_think = PlagueElfPhaseOutPreThink;
		self->next_pre_think = level.time + 0.05f;
	}
	else
	{
		self->s.color.a = (SKILL == SKILL_EASY ? 50 : 0);
		self->pre_think = NULL;
		self->next_pre_think = -1.0f;
	}
}

void PlagueElfPhaseInPreThink(edict_t* self) //mxd. Named 'pelf_phase_in' in original logic.
{
	if (self->s.color.a < 255 - PLAGUEELF_PHASE_INTERVAL)
	{
		self->s.color.a += (byte)irand(PLAGUEELF_PHASE_INTERVAL / 2, PLAGUEELF_PHASE_INTERVAL);
		self->pre_think = PlagueElfPhaseInPreThink;
		self->next_pre_think = level.time + 0.05f;
	}
	else
	{
		self->svflags &= ~SVF_NO_AUTOTARGET;
		self->s.color.c = 0xffffffff;

		if (self->health <= 0)
		{
			self->pre_think = NULL;
			self->next_pre_think = -1.0f;
		}
		else
		{
			PlagueElfPhaseOutInit(self);
		}
	}
}

#pragma endregion

#pragma region ========================== Action functions ==========================

void plagueelf_knockback_death_loop(edict_t* self) //mxd. Named 'plagueelf_death_loop' in original logic.
{
	SetAnim(self, ANIM_KDEATH_LOOP);
}

void plagueelf_knockback_death_check_land(edict_t* self) //mxd. Named 'plagueelf_check_land' in original logic.
{
	if (self->s.frame == FRAME_death7)
		MG_SetNoBlocking(self);

	M_ChangeYaw(self);

	vec3_t end_pos;
	VectorCopy(self->s.origin, end_pos);
	end_pos[2] -= 48.0f;

	trace_t trace;
	gi.trace(self->s.origin, self->mins, self->maxs, end_pos, self, MASK_MONSTERSOLID, &trace);

	if ((trace.fraction < 1.0f || trace.allsolid || trace.startsolid) && self->curAnimID != ANIM_KDEATH_END && self->curAnimID != ANIM_KDEATH_GO)
	{
		self->elasticity = 1.25f;
		self->friction = 0.5f;

		SetAnim(self, ANIM_KDEATH_END);
	}
}

void plagueelf_strike(edict_t* self) //mxd. Named 'plagueElf_strike' in original logic.
{
	//FIXME: Account for weapon being knocked out of hand?
	if (self->s.fmnodeinfo[MESH__HANDLE].flags & FMNI_NO_DRAW)
		return;

	vec3_t start_offset;
	vec3_t end_offset;

	switch (self->curAnimID)
	{
		case ANIM_MELEE1:
			VectorSet(start_offset, -8.0f, 0.0f, 32.0f);
			VectorSet(end_offset, 36.0f, 8.0f, 16.0f);
			break;

		case ANIM_MELEE2:
			VectorSet(start_offset, -8.0f, -16.0f, 32.0f);
			VectorSet(end_offset, 36.0f, 0.0f, 0.0f);
			break;

		case ANIM_RUNATK1:
			VectorSet(start_offset, 2.0f, -4.0f, 24.0f);
			VectorSet(end_offset, 50.0f, 4.0f, 4.0f);
			break;
	}

	const vec3_t mins = { -4.0f, -4.0f, -4.0f };
	const vec3_t maxs = {  4.0f,  4.0f,  4.0f };

	trace_t trace;
	vec3_t direction;
	edict_t* victim = M_CheckMeleeLineHit(self, start_offset, end_offset, mins, maxs, &trace, direction);

	if (victim == NULL)
	{
		// Play a swish sound.
		gi.sound(self, CHAN_WEAPON, sounds[SND_ATTACKMISS1], 1.0f, ATTN_NORM, 0.0f);
		return;
	}

	vec3_t blood_dir;
	VectorSubtract(start_offset, end_offset, blood_dir);
	VectorNormalize(blood_dir);

	if (victim == self)
	{
		// Create a spark effect.
		gi.CreateEffect(NULL, FX_SPARKS, 0, trace.endpos, "d", blood_dir);
		return;
	}

	if (!(self->s.fmnodeinfo[MESH__GAFF].flags & FMNI_NO_DRAW) || !(self->s.fmnodeinfo[MESH__HOE].flags & FMNI_NO_DRAW))
		gi.sound(self, CHAN_WEAPON, sounds[SND_ATTACKHIT1], 1.0f, ATTN_NORM, 0.0f); // It's the hoe or the hook.
	else
		gi.sound(self, CHAN_WEAPON, sounds[SND_ATTACKHIT2], 1.0f, ATTN_NORM, 0.0f); // It's the hammer or handle.

	int damage = irand(PLAGUEELF_DMG_MIN, PLAGUEELF_DMG_MAX);

	if (!(self->s.fmnodeinfo[MESH__HOE].flags & FMNI_NO_DRAW))
		damage += PLAGUEELF_DMG_HOE;
	else if (!(self->s.fmnodeinfo[MESH__GAFF].flags & FMNI_NO_DRAW))
		damage += PLAGUEELF_DMG_GAFF;
	else if (self->s.fmnodeinfo[MESH__HAMMER].flags & FMNI_NO_DRAW)
		damage += PLAGUEELF_DMG_HAMMER;

	// Hurt whatever we were whacking away at.
	T_Damage(victim, self, self, direction, trace.endpos, blood_dir, damage, damage * 2, DAMAGE_DISMEMBER, MOD_DIED);
}

void plagueelf_death_squeal(edict_t* self) //mxd. Named 'plagueElfdeathsqueal' in original logic.
{
	gi.sound(self, CHAN_VOICE, sounds[irand(SND_DIE1, SND_DIE3)], 1.0f, ATTN_NORM, 0.0f);
}

void plagueelf_growl(edict_t* self) //mxd. Named 'plagueElfgrowl' in original logic.
{
	if (irand(0, 10) > 2)
		return;

	const int chance = irand(0, 12);

	if (chance < 3)
		gi.sound(self, CHAN_VOICE, sounds[SND_PANT], 1.0f, ATTN_IDLE, 0.0f);
	else if (chance < 6)
		gi.sound(self, CHAN_VOICE, sounds[SND_MOAN1], 1.0f, ATTN_IDLE, 0.0f);
	else if (chance < 9)
		gi.sound(self, CHAN_VOICE, sounds[SND_MOAN2], 1.0f, ATTN_IDLE, 0.0f);
}

void plagueelf_attack(edict_t* self) //mxd. Named 'plagueElfattack' in original logic.
{
	if (irand(0, 10) < 5)
		gi.sound(self, CHAN_VOICE, sounds[irand(SND_ATTACK1, SND_ATTACK2)], 1.0f, ATTN_IDLE, 0.0f);
}

void plagueelf_spell(edict_t* self) //mxd. Named 'plagueElf_spell' in original logic.
{
	//FIXME: adjust for up/down.
	if (!M_ValidTarget(self, self->enemy))
	{
		// If our enemy is dead, we need to stand.
		G_PostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
		return;
	}

	if (self->s.fmnodeinfo[MESH__R_ARM].flags & FMNI_NO_DRAW)
		return;

	self->monsterinfo.attack_finished = level.time + 0.4f;

	edict_t* spell = G_Spawn();

	PlagueElfSpellInit(spell);

	spell->touch = PlagueElfSpellTouch;
	spell->owner = self;
	spell->enemy = self->enemy;

	vec3_t forward;
	vec3_t right;
	AngleVectors(self->s.angles, forward, right, NULL);

	VectorCopy(self->s.origin, spell->s.origin);
	VectorMA(spell->s.origin, 4.0f, forward, spell->s.origin);
	VectorMA(spell->s.origin, 8.0f, right, spell->s.origin);
	spell->s.origin[2] += 12.0f;

	VectorCopy(self->movedir, spell->movedir);
	vectoangles(forward, spell->s.angles);

	vec3_t fire_dir;
	VectorSubtract(self->enemy->s.origin, spell->s.origin, fire_dir);
	VectorNormalize(fire_dir);

	forward[2] = fire_dir[2];
	VectorNormalize(forward);
	VectorScale(forward, 400.0f, spell->velocity);

	VectorCopy(spell->velocity, spell->movedir);
	VectorNormalize(spell->movedir);
	vectoangles(spell->movedir, spell->s.angles);

	spell->s.angles[PITCH] = anglemod(-spell->s.angles[PITCH]);

	if (Q_stricmp(self->classname, "monster_plagueElf") != 0) // One of the special dudes. //mxd. stricmp -> Q_stricmp
	{
		if (irand(0, 3) > 0 || SKILL < SKILL_HARD || Q_stricmp(self->classname, "monster_palace_plague_guard_invisible") == 0) //mxd. stricmp -> Q_stricmp
			spell->plagueelf_spell_fx_type = FX_PE_MAKE_SPELL2;
		else
			spell->plagueelf_spell_fx_type = FX_PE_MAKE_SPELL3;
	}
	else
	{
		spell->plagueelf_spell_fx_type = FX_PE_MAKE_SPELL;
	}

	gi.CreateEffect(&spell->s, FX_PE_SPELL, CEF_OWNERS_ORIGIN, NULL, "bv", spell->plagueelf_spell_fx_type, spell->velocity);

	G_LinkMissile(spell);

	spell->think = G_FreeEdict;
	spell->nextthink = level.time + 3.0f;
}

void plagueelf_cinematic_spell(edict_t* self) //mxd. Named 'plagueElf_c_spell' in original logic.
{
	//FIXME: adjust for up/down.
	if (self->s.fmnodeinfo[MESH__R_ARM].flags & FMNI_NO_DRAW) // Was his arm lopped off?
		return;

	self->monsterinfo.attack_finished = level.time + 0.4f;

	edict_t* spell = G_Spawn();

	PlagueElfSpellInit(spell);

	spell->touch = PlagueElfSpellTouch;
	spell->owner = self;

	vec3_t forward;
	vec3_t right;
	AngleVectors(self->s.angles, forward, right, NULL);

	VectorCopy(self->s.origin, spell->s.origin);
	VectorMA(spell->s.origin, 4.0f, forward, spell->s.origin);
	VectorMA(spell->s.origin, 8.0f, right, spell->s.origin);
	spell->s.origin[2] += 12.0f;

	VectorCopy(self->movedir, spell->movedir);
	vectoangles(forward, spell->s.angles);

	vec3_t hold_pos;
	VectorCopy(self->s.origin, hold_pos);
	VectorMA(spell->s.origin, 40.0f, forward, spell->s.origin);

	vec3_t fire_dir;
	VectorSubtract(hold_pos, spell->s.origin, fire_dir);
	VectorNormalize(fire_dir);

	forward[2] = fire_dir[2];
	VectorNormalize(forward);
	VectorScale(forward, 500.0f, spell->velocity);

	VectorCopy(spell->velocity, spell->movedir);
	VectorNormalize(spell->movedir);
	vectoangles(spell->movedir, spell->s.angles);

	spell->s.angles[PITCH] = anglemod(-spell->s.angles[PITCH]);

	if (Q_stricmp(self->classname, "monster_plagueElf") != 0) // One of the special dudes.
		spell->plagueelf_spell_fx_type = ((irand(0, 3) > 0) ? FX_PE_MAKE_SPELL2 : FX_PE_MAKE_SPELL3);
	else
		spell->plagueelf_spell_fx_type = FX_PE_MAKE_SPELL;

	gi.CreateEffect(&spell->s, FX_PE_SPELL, CEF_OWNERS_ORIGIN, NULL, "bv", spell->plagueelf_spell_fx_type, spell->velocity);

	G_LinkMissile(spell);

	spell->think = G_FreeEdict;
	spell->nextthink = level.time + 3.0f;
}

void plagueelf_apply_jump(edict_t* self) //mxd. Named 'plagueElfApplyJump' in original logic.
{
	self->jump_time = level.time + 0.5f;
	VectorCopy(self->movedir, self->velocity);
	VectorNormalize(self->movedir);
}

void plagueelf_pause(edict_t* self) //mxd. Named 'plagueElf_pause' in original logic.
{
	self->monsterinfo.misc_debounce_time = 0.0f;

	if (self->ai_mood == AI_MOOD_FLEE)
	{
		if (self->s.color.a != 255 && self->pre_think != PlagueElfPhaseInPreThink)
			PlagueElfPhaseInInit(self);
	}
	else if (self->pre_think != PlagueElfPhaseOutPreThink)
	{
		const byte min_alpha = (SKILL == SKILL_EASY ? 50 : 0); //mxd
		if (self->s.color.a > min_alpha)
			PlagueElfPhaseOutInit(self);
	}

	if ((self->spawnflags & MSF_FIXED) && self->curAnimID == ANIM_DELAY && self->enemy != NULL)
	{
		self->monsterinfo.searchType = SEARCH_COMMON;
		MG_FaceGoal(self, true);
	}

	self->mood_think(self);

	switch (self->ai_mood)
	{
		case AI_MOOD_ATTACK:
			if (self->ai_mood_flags & AI_MOOD_FLAG_MISSILE)
				G_PostMessage(self, MSG_MISSILE, PRI_DIRECTIVE, NULL);
			else
				G_PostMessage(self, MSG_MELEE, PRI_DIRECTIVE, NULL);
			break;

		case AI_MOOD_PURSUE:
			if (self->ai_mood_flags & AI_MOOD_FLAG_DUMB_FLEE)
				SetAnim(self, ANIM_SCARED);
			else
				G_PostMessage(self, MSG_RUN, PRI_DIRECTIVE, NULL);
			break;

		case AI_MOOD_NAVIGATE:
			G_PostMessage(self, MSG_RUN, PRI_DIRECTIVE, NULL);
			break;

		case AI_MOOD_WALK:
			G_PostMessage(self, MSG_WALK, PRI_DIRECTIVE, NULL);
			break;

		case AI_MOOD_FLEE:
			if ((self->ai_mood_flags & AI_MOOD_FLAG_DUMB_FLEE) || (!(self->ai_mood_flags & AI_MOOD_FLAG_FORCED_BUOY) && irand(0, 20) == 0))
			{
				if (self->enemy != NULL && M_DistanceToTarget(self, self->enemy) < 100.0f)
				{
					if (self->curAnimID == ANIM_SCARED)
						PlagueElfDyingSound(self, DYING_ELF_PAIN_VOICE);

					SetAnim(self, irand(ANIM_CRAZY_A, ANIM_CRAZY_B));
				}
				else
				{
					SetAnim(self, ANIM_SCARED);
				}
			}
			else
			{
				SetAnim(self, irand(ANIM_CRAZY_A, ANIM_CRAZY_B));
			}
			break;

		case AI_MOOD_STAND:
			G_PostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
			break;

		case AI_MOOD_DELAY:
			SetAnim(self, ANIM_DELAY);
			break;

		case AI_MOOD_WANDER:
			SetAnim(self, ((self->spawnflags & MSF_FIXED) ? ANIM_DELAY : ANIM_WALK1));
			break;

		case AI_MOOD_BACKUP:
			G_PostMessage(self, MSG_FALLBACK, PRI_DIRECTIVE, NULL);
			break;

		case AI_MOOD_JUMP:
			SetAnim(self, ((self->spawnflags & MSF_FIXED) ? ANIM_DELAY : ANIM_FJUMP));
			break;

		default:
			break;
	}
}

void plagueelf_land(edict_t* self) //mxd. Named 'pelf_land' in original logic.
{
	gi.sound(self, CHAN_BODY, gi.soundindex("misc/land.wav"), 1.0f, ATTN_NORM, 0.0f); //TODO: precache sound?
	gi.CreateEffect(&self->s, FX_DUST_PUFF, CEF_OWNERS_ORIGIN, self->s.origin, NULL);
}

void plagueelf_inair_go(edict_t* self) //mxd. Named 'pelf_go_inair' in original logic.
{
	SetAnim(self, ANIM_INAIR);
}

void plagueelf_squeal(edict_t* self) //mxd. Named 'plagueElfsqueal' in original logic.
{
	if (self->monsterinfo.aiflags & (AI_COWARD | AI_FLEE))
		PlagueElfDyingSound(self, DYING_ELF_PAIN_VOICE);
	else
		gi.sound(self, CHAN_VOICE, sounds[irand(SND_PAIN1, SND_PAIN3)], 1.0f, ATTN_NORM, 0.0f);
}

void plagueelf_run_go(edict_t* self) //mxd. Named 'plagueElf_go_run' in original logic.
{
	SetAnim(self, ((self->spawnflags & MSF_FIXED) ? ANIM_DELAY : ANIM_RUN1));
}

void plagueelf_check_distance(edict_t* self) //mxd. Named 'pelf_check_too_close' in original logic.
{
	if (self->enemy != NULL && M_DistanceToTarget(self, self->enemy) < flrand(0.0f, 100.0f))
		SetAnim(self, irand(ANIM_CRAZY_A, ANIM_CRAZY_B));
}

#pragma endregion

void PlagueElfStaticsInit(void)
{
	static ClassResourceInfo_t res_info; //mxd. Made local static.

	classStatics[CID_PLAGUEELF].msgReceivers[MSG_STAND] = PlagueElfStandMsgHandler;
	classStatics[CID_PLAGUEELF].msgReceivers[MSG_WALK] = PlagueElfWalkMsgHandler;
	classStatics[CID_PLAGUEELF].msgReceivers[MSG_RUN] = PlagueElfRunMsgHandler;
	classStatics[CID_PLAGUEELF].msgReceivers[MSG_MELEE] = PlagueElfMeleeMsgHandler;
	classStatics[CID_PLAGUEELF].msgReceivers[MSG_MISSILE] = PlagueElfMissileMsgHandler;
	classStatics[CID_PLAGUEELF].msgReceivers[MSG_PAIN] = PlagueElfPainMsgHandler;
	classStatics[CID_PLAGUEELF].msgReceivers[MSG_DEATH] = PlagueElfDeathMsgHandler;
	classStatics[CID_PLAGUEELF].msgReceivers[MSG_DISMEMBER] = DismemberMsgHandler;
	classStatics[CID_PLAGUEELF].msgReceivers[MSG_JUMP] = PlagueElfJumpMsgHandler;
	classStatics[CID_PLAGUEELF].msgReceivers[MSG_DEATH_PAIN] = PlagueElfDeadPainMsgHandler;
	classStatics[CID_PLAGUEELF].msgReceivers[MSG_FALLBACK] = PlagueElfRunMsgHandler;
	classStatics[CID_PLAGUEELF].msgReceivers[MSG_CHECK_MOOD] = PlagueElfCheckMoodMsgHandler;

	// Sound support.
	classStatics[CID_PLAGUEELF].msgReceivers[MSG_VOICE_SIGHT] = PlagueElfVoiceSightMsgHandler;
	classStatics[CID_PLAGUEELF].msgReceivers[MSG_VOICE_POLL] = PlagueElfVoicePollMsgHandler;
	classStatics[CID_PLAGUEELF].msgReceivers[MSG_VOICE_PUPPET] = PlagueElfVoicePuppetMsgHandler;

	// Cinematics.
	classStatics[CID_PLAGUEELF].msgReceivers[MSG_C_IDLE1] = PlagueElfCinematicActionMsgHandler;
	classStatics[CID_PLAGUEELF].msgReceivers[MSG_C_IDLE2] = PlagueElfCinematicActionMsgHandler;
	classStatics[CID_PLAGUEELF].msgReceivers[MSG_C_IDLE3] = PlagueElfCinematicActionMsgHandler;
	classStatics[CID_PLAGUEELF].msgReceivers[MSG_C_WALK1] = PlagueElfCinematicActionMsgHandler;
	classStatics[CID_PLAGUEELF].msgReceivers[MSG_C_WALK2] = PlagueElfCinematicActionMsgHandler;
	classStatics[CID_PLAGUEELF].msgReceivers[MSG_C_RUN1] = PlagueElfCinematicActionMsgHandler;
	classStatics[CID_PLAGUEELF].msgReceivers[MSG_C_ATTACK1] = PlagueElfCinematicActionMsgHandler;
	classStatics[CID_PLAGUEELF].msgReceivers[MSG_C_ATTACK2] = PlagueElfCinematicActionMsgHandler;
	classStatics[CID_PLAGUEELF].msgReceivers[MSG_C_ATTACK3] = PlagueElfCinematicActionMsgHandler;
	classStatics[CID_PLAGUEELF].msgReceivers[MSG_C_ATTACK4] = PlagueElfCinematicActionMsgHandler;
	classStatics[CID_PLAGUEELF].msgReceivers[MSG_C_DEATH1] = PlagueElfCinematicActionMsgHandler;
	classStatics[CID_PLAGUEELF].msgReceivers[MSG_C_DEATH2] = PlagueElfCinematicActionMsgHandler;
	classStatics[CID_PLAGUEELF].msgReceivers[MSG_C_DEATH3] = PlagueElfCinematicActionMsgHandler;
	classStatics[CID_PLAGUEELF].msgReceivers[MSG_C_DEATH4] = PlagueElfCinematicActionMsgHandler;
	classStatics[CID_PLAGUEELF].msgReceivers[MSG_C_THINKAGAIN] = PlagueElfCinematicActionMsgHandler;
	classStatics[CID_PLAGUEELF].msgReceivers[MSG_C_PAIN1] = PlagueElfCinematicActionMsgHandler;
	classStatics[CID_PLAGUEELF].msgReceivers[MSG_C_GIB1] = CinematicGibMsgHandler;

	res_info.numAnims = NUM_ANIMS;
	res_info.animations = animations;
	res_info.modelIndex = gi.modelindex("models/monsters/plaguelf/tris.fm"); // Note that the name is different in the path.

	sounds[SND_PAIN1] = gi.soundindex("monsters/plagueElf/pain1.wav");
	sounds[SND_PAIN2] = gi.soundindex("monsters/plagueElf/pain2.wav");
	sounds[SND_PAIN3] = gi.soundindex("monsters/plagueElf/pain3.wav");
	sounds[SND_DIE1] = gi.soundindex("monsters/plagueElf/death1.wav");
	sounds[SND_DIE2] = gi.soundindex("monsters/plagueElf/death2.wav");
	sounds[SND_DIE3] = gi.soundindex("monsters/plagueElf/death3.wav");
	sounds[SND_GIB] = gi.soundindex("monsters/plagueElf/gib2.wav");
	sounds[SND_ATTACKHIT1] = gi.soundindex("monsters/plagueElf/hookhit.wav");
	sounds[SND_ATTACKHIT2] = gi.soundindex("monsters/plagueElf/hamhit.wav");
	sounds[SND_ATTACKMISS1] = gi.soundindex("weapons/staffswing.wav");
	sounds[SND_MOAN1] = gi.soundindex("monsters/plagueElf/pelfgrn1.wav");
	sounds[SND_MOAN2] = gi.soundindex("monsters/plagueElf/pelfgron.wav");
	//sounds[SND_SHIVER] = gi.soundindex("monsters/plagueElf/pelfshiv.wav");
	sounds[SND_PANT] = gi.soundindex("monsters/plagueElf/pelfpant.wav");
	//sounds[SND_GASP] = gi.soundindex("monsters/plagueElf/pelfgasp.wav");
	//sounds[SND_SIGH] = gi.soundindex("monsters/plagueElf/pelfsigh.wav");
	sounds[SND_ATTACK1] = gi.soundindex("monsters/plagueElf/attack1.wav");
	sounds[SND_ATTACK2] = gi.soundindex("monsters/plagueElf/attack2.wav");
	sounds[SND_DISMEMBER1] = gi.soundindex("monsters/plagueElf/loselimb1.wav");
	sounds[SND_DISMEMBER2] = gi.soundindex("monsters/plagueElf/loselimb2.wav");

	// Plague elf voices. //TODO: there are A LOT of unused sounds in monsters/plagueElf/voices.
	sounds[VOICE_SIGHT_EAT_FLESH1] = gi.soundindex("monsters/plagueElf/voices/eatfleshb.wav");
	sounds[VOICE_SIGHT_GET_HIM1] = gi.soundindex("monsters/plagueElf/voices/gethimb.wav");
	sounds[VOICE_SIGHT_GET_HIM2] = gi.soundindex("monsters/plagueElf/voices/gethimk.wav");
	sounds[VOICE_SIGHT_GET_HIM3] = gi.soundindex("monsters/plagueElf/voices/gethimm.wav");
	sounds[VOICE_SIGHT_THERES_ONE] = gi.soundindex("monsters/plagueElf/voices/theresonep.wav");

	sounds[VOICE_SUPPORT_GONNA_DIE1] = gi.soundindex("monsters/plagueElf/voices/gonnadieb.wav");
	sounds[VOICE_SUPPORT_LIVER] = gi.soundindex("monsters/plagueElf/voices/liverm.wav");
	sounds[VOICE_SUPPORT_YES] = gi.soundindex("monsters/plagueElf/voices/yesb.wav");

	//sounds[VOICE_MISC_LEAVE_ME1] = gi.soundindex("monsters/plagueElf/voices/leavemeb.wav");
	//sounds[VOICE_MISC_NO] = gi.soundindex("monsters/plagueElf/voices/nomrj.wav");

	res_info.numSounds = NUM_SOUNDS;
	res_info.sounds = sounds;

	classStatics[CID_PLAGUEELF].resInfo = &res_info;
}

// QUAKED monster_plagueElf (1 .5 0) (-17 -25 -1) (22 12 63) AMBUSH ASLEEP WALKING CINEMATIC MISSILE 32 64 FIXED WANDER MELEE_LEAD STALK COWARD EXTRA1 EXTRA2 EXTRA3 EXTRA4
// The Plague Elf.

// Spawnflags:
// MISSILE		- Can fire a ranged attack.
// AMBUSH		- Will not be woken up by other monsters or shots from player.
// ASLEEP		- Will not appear until triggered.
// WALKING		- Use WANDER instead.
// WANDER		- Monster will wander around aimlessly (but follows buoys).
// MELEE_LEAD	- Monster will try to cut you off when you're running and fighting him, works well if there are a few monsters in a group, half doing this, half not.
// STALK		- Monster will only approach and attack from behind. If you're facing the monster it will just stand there.
//				  Once the monster takes pain, however, it will stop this behaviour and attack normally.
// COWARD		- Monster starts off in flee mode (runs away from you when woken up).

// Variables:
// homebuoy					- Monsters will head to this buoy if they don't have an enemy ("homebuoy" should be targetname of the buoy you want them to go to).
// wakeup_target			- Monsters will fire this target the first time it wakes up (only once).
// pain_target				- Monsters will fire this target the first time it gets hurt (only once).
// mintel					- Monster intelligence - this basically tells a monster how many buoys away an enemy has to be for it to give up (default 16).
// melee_range				- How close the player has to be for the monster to go into melee. If this is zero, the monster will never melee.
//							  If it is negative, the monster will try to keep this distance from the player.
//							  If the monster has a backup, he'll use it if too close, otherwise, a negative value here means the monster will just stop
//							  running at the player at this distance (default 0).
//							 Examples:
//								melee_range = 60 - monster will start swinging it player is closer than 60.
//								melee_range = 0 - monster will never do a melee attack.
//								melee_range = -100 - monster will never do a melee attack and will back away (if it has that ability) when player gets too close.
// missile_range			- Maximum distance the player can be from the monster to be allowed to use it's ranged attack (default 512).
// min_missile_range		- Minimum distance the player can be from the monster to be allowed to use it's ranged attack (default 0).
// bypass_missile_chance	- Chance that a monster will NOT fire it's ranged attack, even when it has a clear shot. This, in effect, will make the monster
//							  come in more often than hang back and fire. A percentage (0 = always fire/never close in, 100 = never fire/always close in) - must be whole number (default 0 (melee P.E.) / 60 (missile P.E.)).
// jump_chance				- Every time the monster has the opportunity to jump, what is the chance (out of 100) that he will... (100 = jump every time) - must be whole number (default 50).
// wakeup_distance			- How far (max) the player can be away from the monster before it wakes up. This means that if the monster can see the player,
//							  at what distance should the monster actually notice him and go for him (default 1024).
// NOTE: A value of zero will result in defaults, if you actually want zero as the value, use -1.

void SP_monster_plagueElf(edict_t* self)
{
	if (self->spawnflags & MSF_WALKING)
	{
		self->spawnflags |= MSF_WANDER;
		self->spawnflags &= ~MSF_WALKING;
	}

	if (!M_WalkmonsterStart(self)) // Failed initialization.
		return;

	self->msgHandler = DefaultMsgHandler;
	self->monsterinfo.dismember = PlagueElfDismember;

	if (self->health == 0)
		self->health = PLAGUEELF_HEALTH;

	// Apply to the end result (whether designer set or not).
	self->health = MonsterHealth(self->health);
	self->max_health = self->health;

	self->mass = PLAGUEELF_MASS;
	self->yaw_speed = 20.0f;

	self->movetype = PHYSICSTYPE_STEP;
	self->solid = SOLID_BBOX;
	self->materialtype = MAT_FLESH;

	VectorClear(self->knockbackvel);

	if (irand(0, 1) == 1)
		self->ai_mood_flags |= AI_MOOD_FLAG_PREDICT;

	self->s.origin[2] += 32.0f; //FIXME: Hack to account for new origin with old QuakEd header.

	VectorCopy(STDMinsForClass[self->classID], self->mins);
	VectorCopy(STDMaxsForClass[self->classID], self->maxs);
	self->viewheight = (int)(self->maxs[2] * 0.8f);

	self->s.modelindex = (byte)classStatics[CID_PLAGUEELF].resInfo->modelIndex;

	// All skins are even numbers, pain skins are skin + 1.
	if (self->s.skinnum == 0 && irand(0, 1) == 0)
		self->s.skinnum = 2; // If the skin hasn't been touched, set it.

	if (self->s.scale == 0.0f)
	{
		self->s.scale = MODEL_SCALE;
		self->monsterinfo.scale = self->s.scale;
	}

	//FIXME: otherenemy should be more than just *one* kind.
	self->monsterinfo.otherenemyname = "monster_box"; //TODO: 'monster_box' is not defined anywhere.

	if (self->spawnflags & MSF_WANDER)
	{
		G_PostMessage(self, MSG_WALK, PRI_DIRECTIVE, NULL);
	}
	else if (self->spawnflags & MSF_PELF_CINEMATIC)
	{
		self->svflags |= SVF_FLOAT;
		self->monsterinfo.c_mode = true;
		G_PostMessage(self, MSG_C_IDLE1, PRI_DIRECTIVE, "iiige", 0, 0, 0, NULL, NULL);
	}
	else
	{
		G_PostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
	}

	// Setup missile plague elf.
	if (self->spawnflags & (MSF_FIXED | MSF_PELF_MISSILE))
	{
		self->melee_range = 0.0f;

		if (self->missile_range == 0.0f)
			self->missile_range = 512.0f;

		self->min_missile_range = 0.0f;

		if (self->bypass_missile_chance == 0)
			self->bypass_missile_chance = 60;

		self->s.fmnodeinfo[MESH__HOE].flags |= FMNI_NO_DRAW;
		self->s.fmnodeinfo[MESH__GAFF].flags |= FMNI_NO_DRAW;
		self->s.fmnodeinfo[MESH__HAMMER].flags |= FMNI_NO_DRAW;
		self->s.fmnodeinfo[MESH__HANDLE].flags |= FMNI_NO_DRAW;

		self->monsterinfo.aiflags |= AI_NO_MELEE;
	}
	else // Setup melee plague elf.
	{
		const int chance = irand(0, 3);

		// Turn on/off the weapons that aren't used.
		if (chance < 1)
		{
			// Show the hammer.
			self->s.fmnodeinfo[MESH__HOE].flags |= FMNI_NO_DRAW;
			self->s.fmnodeinfo[MESH__GAFF].flags |= FMNI_NO_DRAW;
		}
		else if (chance < 2)
		{
			// Show the hoe.
			self->s.fmnodeinfo[MESH__HAMMER].flags |= FMNI_NO_DRAW;
			self->s.fmnodeinfo[MESH__GAFF].flags |= FMNI_NO_DRAW;
		}
		else
		{
			// Show the gaff (that hook thingie).
			self->s.fmnodeinfo[MESH__HAMMER].flags |= FMNI_NO_DRAW;
			self->s.fmnodeinfo[MESH__HOE].flags |= FMNI_NO_DRAW;
		}

		self->monsterinfo.aiflags |= AI_NO_MISSILE;
	}

	self->monsterinfo.supporters = -1;

	// Setup my mood function.
	MG_InitMoods(self);
	self->svflags |= SVF_WAIT_NOTSOLID;
}

// QUAKED monster_palace_plague_guard (1 .5 0) (-17 -25 -1) (22 12 63) AMBUSH ASLEEP WALKING CINEMATIC MISSILE 32 64 FIXED WANDER MELEE_LEAD STALK COWARD EXTRA1 EXTRA2 EXTRA3 EXTRA4
// Palace Guard Plague Elf. Can fire 2 ranged attacks, has a new skin, tougher, has armor?
// Same spawnflags/variables as monster_plagueElf. 
void SP_monster_palace_plague_guard(edict_t* self)
{
	if (self->health == 0)
		self->health = PLAGUEELF_HEALTH * 2;

	self->spawnflags |= MSF_PELF_MISSILE;

	if (self->s.scale == 0.0f)
	{
		self->s.scale = flrand(1.0f, 1.3f);
		self->monsterinfo.scale = self->s.scale;
	}

	SP_monster_plagueElf(self);

	self->s.skinnum = PALACE_ELF_SKIN;
}

// QUAKED monster_palace_plague_guard_invisible (1 .5 0) (-17 -25 -1) (22 12 63) AMBUSH ASLEEP WALKING CINEMATIC MISSILE 32 64 FIXED WANDER MELEE_LEAD STALK COWARD EXTRA1 EXTRA2 EXTRA3 START_VISIBLE
// Invisible Palace Guard Plague Elf. Can fire 2 ranged attacks, has a new skin, tougher, has armor? Is invisible unless firing or hit.
// Same spawnflags/variables as monster_plagueElf, except:
// START_VISIBLE - When not set, start invisible.
void SP_monster_palace_plague_guard_invisible(edict_t* self)
{
	if (self->health == 0)
		self->health = PLAGUEELF_HEALTH * 2;

	self->spawnflags |= MSF_PELF_MISSILE;

	SP_monster_plagueElf(self);

	self->s.skinnum = PALACE_ELF_SKIN;
	self->s.color.c = 0xffffffff;

	self->melee_range = -64.0f;
	self->min_missile_range = 30.0f;
	self->bypass_missile_chance = 80;

	if (!(self->spawnflags & MSF_EXTRA4)) // These guys start visible.
		PlagueElfPhaseOutInit(self);
}