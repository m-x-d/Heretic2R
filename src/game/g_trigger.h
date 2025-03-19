//
// g_trigger.h
//
// Copyright 2025 mxd
//

#pragma once

#include "q_Typedef.h"

extern void TriggerStaticsInit(void);
extern void TriggerMultipleUse(edict_t* self, edict_t* other, edict_t* activator);
extern void TriggerInit(edict_t* self);

extern void SP_trigger_Multiple(edict_t* self);
extern void SP_trigger_Once(edict_t* self);
extern void SP_trigger_Relay(edict_t* self);
extern void SP_trigger_puzzle(edict_t* self);
extern void SP_trigger_Counter(edict_t* self);
extern void SP_trigger_Always(edict_t* self);
extern void SP_trigger_PlayerUsePuzzle(edict_t* self);
extern void SP_trigger_PlayerPushButton(edict_t* self);
extern void SP_trigger_Elevator(edict_t* self);
extern void SP_trigger_Deactivate(edict_t* self);
extern void SP_trigger_Activate(edict_t* self);
extern void SP_choose_CDTrack(edict_t* self);
extern void SP_trigger_quit_to_menu(edict_t* self);
extern void SP_trigger_mappercentage(edict_t* self);
extern void SP_trigger_lightning(edict_t* self);
extern void SP_trigger_quake(edict_t* self);
extern void SP_trigger_mission_give(edict_t* self);
extern void SP_trigger_mission_take(edict_t* self);
extern void SP_trigger_farclip(edict_t* self);
extern void SP_trigger_endgame(edict_t* self);
extern void SP_trigger_PlayerPushLever(edict_t* self);