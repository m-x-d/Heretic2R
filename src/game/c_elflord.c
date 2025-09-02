//
// c_elflord.c
//
// Copyright 1998 Raven Software
//

#include "c_elflord.h"
#include "c_elflord_anim.h"
#include "c_ai.h"
#include "g_debris.h" //mxd
#include "FX.h"
#include "Matrix.h"
#include "Vector.h"
#include "Utilities.h"

// Elf lord cinematic actions.
static const animmove_t* animations[NUM_ANIMS] =
{
	&elflord_move_c_action1,
	&elflord_move_c_action2,
	&elflord_move_c_death1,
	&elflord_move_c_death2,
	&elflord_move_c_idle1,
	&elflord_move_c_idle2,
	NULL
};

#define MIST_ADD	35.0f
static float mist_yaw;

static void ElfLordCinematicGibMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'elflord_c_gib' in original logic.
{
	gi.CreateEffect(&self->s, FX_WEAPON_SPHEREEXPLODE, CEF_OWNERS_ORIGIN, NULL, "db", self->movedir, (byte)(self->s.scale * 7.5f));
	gi.sound(self, CHAN_WEAPON, gi.soundindex("weapons/SphereImpact.wav"), 2.0f, ATTN_NORM, 0.0f); //TODO: why volume is 2.0?

	self->think = BecomeDebris;
	self->nextthink = level.time + FRAMETIME; //mxd. Use define.
}

void elflord_c_boom(edict_t* self)
{
	gi.CreateEffect(NULL, FX_WEAPON_FLYINGFISTEXPLODE, 0, self->s.origin, "d", self->movedir);
}

void elflord_c_throwhead(edict_t* self)
{
	mist_yaw = 0;

	vec3_t gore_spot = { 0 };
	ThrowBodyPart(self, &gore_spot, 8, 15, FRAME_idle1);
	gi.sound(self, CHAN_BODY, gi.soundindex("monsters/elflord/death1.wav"), 2, ATTN_NORM, 0);
}

void elflord_c_mist(edict_t* self, float x, float y, float z) //mxd. Named 'elflord_mist' in original version.
{
	if (self->monsterinfo.aiflags & AI_NO_MELEE)
		return; //fixme: actually prevent these anims.

	mist_yaw += MIST_ADD;

	// Converts degrees to radians for use with trig and matrix functions.
	const float yaw = anglemod(mist_yaw) * ANGLE_TO_RAD;

	// Creates a rotation matrix to rotate the point about the z axis.
	matrix3_t yaw_matrix;
	CreateYawMatrix(yaw_matrix, yaw);

	// Rotates point about local z axis.
	vec3_t yaw_offset;
	Matrix3MultByVec3(yaw_matrix, vec3_origin, yaw_offset);

	// Get normalized offset.
	vec3_t normalized;
	VectorCopy(yaw_offset, normalized);
	normalized[2] = 0.0f;
	VectorNormalize(normalized);

	// Add offset to owners origin.
	Vec3AddAssign(self->s.origin, yaw_offset);

	// Get direction vector scaled by speed.
	const vec3_t velocity = { cosf(yaw) * 200.0f, sinf(yaw) * 200.0f, -100.0f };
	gi.CreateEffect(NULL, FX_PLAGUEMIST, 0, yaw_offset, "vb", velocity, 2050 / 35);
}

static void ElfLordCinematicActionMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'elflord_c_anims' in original logic.
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

		case MSG_C_DEATH1:
			self->s.fmnodeinfo[MESH__LIGHT25].flags |= FMNI_NO_DRAW;
			self->s.fmnodeinfo[MESH__HEAD25].flags |= FMNI_NO_DRAW;
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_DEATH1;
			break;

		case MSG_C_DEATH2:
			self->s.fmnodeinfo[MESH__LIGHT25].flags |= FMNI_NO_DRAW;
			self->s.fmnodeinfo[MESH__HEAD25].flags |= FMNI_NO_DRAW;
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_DEATH2;
			break;

		case MSG_C_IDLE1:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT | C_ANIM_IDLE;
			curr_anim = ANIM_C_IDLE1;
			break;

		case MSG_C_IDLE2:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT | C_ANIM_IDLE;
			curr_anim = ANIM_C_IDLE2;
			break;

		default:
			assert(0); //mxd
			return; //mxd. 'break' in original version.
	}

	SetAnim(self, curr_anim);
}

void ElflordCinStaticsInit(void)
{
	static ClassResourceInfo_t res_info; //mxd. Made local static.

	classStatics[CID_C_ELFLORD].msgReceivers[MSG_C_ACTION1] = ElfLordCinematicActionMsgHandler;
	classStatics[CID_C_ELFLORD].msgReceivers[MSG_C_ACTION2] = ElfLordCinematicActionMsgHandler;
	classStatics[CID_C_ELFLORD].msgReceivers[MSG_C_DEATH1] = ElfLordCinematicActionMsgHandler;
	classStatics[CID_C_ELFLORD].msgReceivers[MSG_C_DEATH2] = ElfLordCinematicActionMsgHandler;
	classStatics[CID_C_ELFLORD].msgReceivers[MSG_C_IDLE1] = ElfLordCinematicActionMsgHandler;
	classStatics[CID_C_ELFLORD].msgReceivers[MSG_C_IDLE2] = ElfLordCinematicActionMsgHandler;
	classStatics[CID_C_ELFLORD].msgReceivers[MSG_C_GIB1] = ElfLordCinematicGibMsgHandler;

	res_info.numAnims = NUM_ANIMS;
	res_info.animations = animations;
	res_info.modelIndex = gi.modelindex("models/monsters/elflord/cinematic/tris.fm");

	classStatics[CID_C_ELFLORD].resInfo = &res_info;
}

// QUAKED character_elflord (1 .5 0) (-24 -24 -78) (24 24 16) INVISIBLE
// The Celestial Watcher who whispers when he talks.
void SP_character_elflord(edict_t* self)
{
	VectorSet(self->mins, -24.0f, -24.0f, -78.0f);
	VectorSet(self->maxs,  24.0f,  24.0f,  16.0f);

	CinematicCharacterInit(self, CID_C_ELFLORD);

	self->s.scale = 2.0f;
	self->monsterinfo.scale = 2.0f;
}