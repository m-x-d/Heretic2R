//
// c_ai.c
//
// Copyright 1998 Raven Software
//

#include "c_ai.h" //mxd
#include "c_corvus1_anim.h"
#include "g_debris.h" //mxd
#include "g_DefaultMessageHandler.h"
#include "g_obj.h" //mxd
#include "g_monster.h"
#include "m_move.h" //mxd
#include "mg_ai.h" //mxd
#include "mg_guide.h" //mxd
#include "FX.h"
#include "Vector.h"
#include "Player.h" //mxd
#include "g_local.h"

#define SF_C_INVISIBLE	1 //mxd. Named 'ENT_INVISIBLE' in original logic.

void ReadCinematicMessage(edict_t* self, G_Message_t* msg) //mxd. Named 'ai_c_readmessage' in original logic.
{
	int turning;
	int repeat;
	G_ParseMsgParms(msg, "iiige", &self->monsterinfo.c_dist, &turning, &repeat, &self->monsterinfo.c_callback, &self->monsterinfo.c_ent);

	self->monsterinfo.c_repeat = repeat;
	self->ideal_yaw = anglemod(self->s.angles[YAW] + (float)turning);
}

// This is called at the end of each anim cycle.
void ai_c_cycleend(edict_t* self)
{
	// A movement action that still has a distance to walk.
	if ((self->monsterinfo.c_anim_flag & C_ANIM_MOVE) && (self->monsterinfo.c_dist > 0 || self->s.angles[YAW] != self->ideal_yaw))
		return;

	// A repeating action that still has to repeat.
	if ((self->monsterinfo.c_anim_flag & C_ANIM_REPEAT) && self->monsterinfo.c_repeat > 0)
	{
		self->monsterinfo.c_repeat--;
		if (self->monsterinfo.c_repeat > 0)
			return;
	}

	if (self->monsterinfo.c_anim_flag & C_ANIM_DONE)
	{
		self->nextthink = THINK_NEVER;
		self->think = NULL;
	}

	// This anim is all done.
	if (self->monsterinfo.c_callback != NULL) // Was a callback specified?
	{
		self->monsterinfo.c_callback(self);
		return;
	}

	// Well then just sit there if you aren't already.
	if (!(self->monsterinfo.c_anim_flag & C_ANIM_IDLE))
		G_PostMessage(self, MSG_C_IDLE1, PRI_DIRECTIVE, "iiige", 0, 0, 0, NULL, NULL);
}

void ai_c_move(edict_t* self, const float forward, float right, float up)
{
	M_ChangeYaw(self);

	if (forward == 0.0f) // Just standing there.
		return;

	// Is the distance desired to move in the next frame past what it should be?
	float dist = forward;
	if (fabsf(dist) > fabsf((float)self->monsterinfo.c_dist))
		dist = (float)self->monsterinfo.c_dist;

	const float yaw = self->s.angles[YAW] * ANGLE_180 * 2.0f / 360.0f;
	vec3_t move = { cosf(yaw) * dist, sinf(yaw) * dist, 0.0f };
	MG_MoveStep(self, move, true);

	self->monsterinfo.c_dist -= (int)dist;

	// If this cycle gets stopped by finishing a distance, then kill it.
	if (self->monsterinfo.c_dist == 0 && (self->monsterinfo.c_anim_flag & C_ANIM_MOVE))
		ai_c_cycleend(self);
}

void CinematicSwapPlayer(const edict_t* self, edict_t* cinematic) //mxd. Named 'c_swapplayer' in original logic.
{
	if (self->client == NULL || cinematic == NULL) // What are you trying to do? Exchange a non-player for Corvus?
		return;

	P_PlayerUpdateModelAttributes(&self->client->playerinfo);

	// Add in plague level for the skin, since the cinematic players use six stock skins.
	cinematic->s.skinnum = self->client->playerinfo.skinnum + (self->client->playerinfo.plaguelevel * DAMAGE_NUM_LEVELS);

	for (int i = 0; i < NUM_MESH_NODES; i++)
	{
		cinematic->s.fmnodeinfo[i].flags = self->s.fmnodeinfo[i].flags;
		cinematic->s.fmnodeinfo[i].skin = self->s.fmnodeinfo[i].skin;
	}

	// Open up hands.
	if (cinematic->s.fmnodeinfo[MESH__RHANDHI].flags & FMNI_NO_DRAW)
		cinematic->s.fmnodeinfo[MESH__RHANDHI].flags &= ~FMNI_NO_DRAW;

	if (cinematic->s.fmnodeinfo[MESH__LHANDHI].flags & FMNI_NO_DRAW)
		cinematic->s.fmnodeinfo[MESH__LHANDHI].flags &= ~FMNI_NO_DRAW;

	// If bow is active, put it on his back.
	if (!(cinematic->s.fmnodeinfo[MESH__BOWACTV].flags & FMNI_NO_DRAW))
		cinematic->s.fmnodeinfo[MESH__BOFF].flags &= ~FMNI_NO_DRAW;

	// If staff is active, put it on his hip.
	if (!(cinematic->s.fmnodeinfo[MESH__STAFACTV].flags & FMNI_NO_DRAW))
		cinematic->s.fmnodeinfo[MESH__STOFF].flags &= ~FMNI_NO_DRAW;

	// Get rid of all weapons in the hands.
	cinematic->s.fmnodeinfo[MESH__BLADSTF].flags |= FMNI_NO_DRAW;
	cinematic->s.fmnodeinfo[MESH__HELSTF].flags |= FMNI_NO_DRAW;
	cinematic->s.fmnodeinfo[MESH__BOWACTV].flags |= FMNI_NO_DRAW;
	cinematic->s.fmnodeinfo[MESH__STAFACTV].flags |= FMNI_NO_DRAW;
}

void CinematicCorvusInit(edict_t* self, const int class_id) //mxd. Named 'c_corvus_init' in original logic.
{
	static vec3_t c_mins = { -16.0f, -16.0f, -34.0f };
	static vec3_t c_maxs = {  16.0f,  16.0f,  25.0f };

	self->classID = class_id;
	self->s.modelindex = (byte)classStatics[class_id].resInfo->modelIndex;

	if (!M_Start(self)) // Failed initialization.
		return;

	self->msgHandler = DefaultMsgHandler;
	self->think = M_WalkmonsterStartGo;

	if (self->health == 0)
		self->health = 30;

	self->mass = 300;
	self->yaw_speed = 20;
	VectorClear(self->knockbackvel);

	VectorCopy(c_mins, self->mins);
	VectorCopy(c_maxs, self->maxs);
	VectorCopy(c_mins, self->intentMins);
	VectorCopy(c_maxs, self->intentMaxs);
	self->viewheight = (int)(self->maxs[2] * 0.8f);

	if (self->monsterinfo.scale == 0.0f)
	{
		self->s.scale = 1.0f;
		self->monsterinfo.scale = 1.0f;
	}

	self->materialtype = MAT_FLESH;
	self->clipmask = MASK_MONSTERSOLID;
	self->count = self->s.modelindex;
	self->takedamage = DAMAGE_NO;

	if (self->spawnflags & SF_C_INVISIBLE)
	{
		self->s.modelindex = 0;
		self->solid = SOLID_NOT;
		self->movetype = PHYSICSTYPE_NONE;
	}
	else
	{
		self->solid = SOLID_BBOX;
		self->movetype = PHYSICSTYPE_STEP;
	}

	// Setup my mood function.
	MG_InitMoods(self);

	self->monsterinfo.c_mode = true;
}

void CinematicCharacterInit(edict_t* self, const int class_id) //mxd. Named 'c_character_init' in original logic.
{
	self->classID = class_id;
	self->s.modelindex = (byte)classStatics[class_id].resInfo->modelIndex;

	if (!M_Start(self)) // Failed initialization.
		return;

	self->msgHandler = DefaultMsgHandler;
	self->think = M_WalkmonsterStartGo;

	self->viewheight = (int)(self->maxs[2] * 0.8f);

	if (self->health == 0)
		self->health = 30;

	self->mass = 300;
	self->yaw_speed = 20;
	VectorClear(self->knockbackvel);

	if (self->monsterinfo.scale == 0.0f)
	{
		self->s.scale = 1.0f;
		self->monsterinfo.scale = 1.0f;
	}

	self->count = self->s.modelindex;
	self->clipmask = MASK_MONSTERSOLID;
	self->materialtype = MAT_FLESH;
	self->takedamage = DAMAGE_NO;

	if (self->spawnflags & SF_C_INVISIBLE)
	{
		self->s.modelindex = 0;
		self->solid = SOLID_NOT;
		self->movetype = PHYSICSTYPE_NONE;
	}
	else
	{
		self->solid = SOLID_BBOX;
		self->movetype = PHYSICSTYPE_STEP;
	}

	BboxYawAndScale(self);

	// Setup my mood function.
	MG_InitMoods(self);

	self->monsterinfo.c_mode = true;
	G_PostMessage(self, MSG_C_IDLE1, PRI_DIRECTIVE, "iiige", 0, 0, 0, NULL, NULL);
}

void CinematicGibMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'ai_c_gib' in original logic.
{
	gi.sound(self, CHAN_BODY, gi.soundindex("monsters/plagueElf/gib2.wav"), 1.0f, ATTN_NORM, 0.0f);
	self->think = BecomeDebris;
	self->nextthink = level.time + FRAMETIME; //mxd. Use define.
}