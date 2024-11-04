//
// c_ai.h
//
// Heretic II
// Copyright 1998 Raven Software
//

#ifndef	C_AI_H
#define C_AI_H

void ai_c_cycleend (edict_t *self);
void ai_c_gib(edict_t *self, G_Message_t *msg);
void ai_c_move (edict_t *self,float x,float y,float z);
void ai_c_readmessage(edict_t *self, G_Message_t *msg);
void ai_c_stand (edict_t *self,float forward,float right,float up);
void ai_c_wav (edict_t *self,float wav_num);

void c_corvus_init(edict_t *self,int classId);
void c_character_init(edict_t *self,int classId);


#endif
