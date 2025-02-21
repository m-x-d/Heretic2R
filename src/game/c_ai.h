//
// c_ai.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "g_Message.h" //mxd
#include "q_Typedef.h" //mxd

void ai_c_cycleend(edict_t* self);
void ai_c_gib(edict_t* self, G_Message_t* msg);
void ai_c_move(edict_t* self, float forward, float right, float up);
void ai_c_readmessage(edict_t* self, G_Message_t* msg);
void ai_c_stand(edict_t* self, float forward, float right, float up);

void c_corvus_init(edict_t* self, int class_id);
void c_character_init(edict_t* self, int class_id);