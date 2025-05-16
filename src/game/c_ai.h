//
// c_ai.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "g_Message.h" //mxd
#include "q_Typedef.h" //mxd

extern void ai_c_cycleend(edict_t* self);
extern void ai_c_move(edict_t* self, float forward, float right, float up);

extern void CinematicCorvusInit(edict_t* self, int class_id);
extern void CinematicCharacterInit(edict_t* self, int class_id);

extern void ReadCinematicMessage(edict_t* self, G_Message_t* msg);
extern void CinematicGibMsgHandler(edict_t* self, G_Message_t* msg);

#ifdef __cplusplus
	extern "C" void CinematicSwapPlayer(const edict_t* self, edict_t* cinematic); //mxd. Used by dc.cpp only.
#endif