//
// g_trigger.h
//
// Copyright 2025 mxd
//

#pragma once

#include "q_shared.h"

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

//mxd. Required by save system...
extern void ChooseCDTrackTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf);
extern void ChooseCDTrackUse(edict_t* self, edict_t* other, edict_t* activator);
extern void KillSoundThink(edict_t* self);
extern void TriggerActivateActivated(edict_t* self, edict_t* activator);
extern void TriggerCounterUse(edict_t* self, edict_t* other, edict_t* activator);
extern void TriggerDeactivateActivated(edict_t* self, edict_t* activator);
extern void TriggerElevatorInitThink(edict_t* self);
extern void TriggerElevatorUse(edict_t* self, edict_t* other, edict_t* activator);
extern void TriggerEnable(edict_t* self, edict_t* other, edict_t* activator);
extern void TriggerEndgameThink(edict_t* self);
extern void TriggerEndgameTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf);
extern void TriggerEndgameUse(edict_t* self, edict_t* other, edict_t* activator);
extern void TriggerFarclipTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf);
extern void TriggerLightningActivated(edict_t* self, edict_t* other);
extern void TriggerLightningUse(edict_t* self, edict_t* other, edict_t* activator);
extern void TriggerMappercentageUse(edict_t* self, edict_t* other);
extern void TriggerMissionGiveUse(edict_t* self, edict_t* other);
extern void TriggerMissionTakeUse(edict_t* self, edict_t* other);
extern void TriggerMultipleTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf);
extern void TriggerMultipleWaitThink(edict_t* self);
extern void TriggerPlayerPushButtonTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surface);
extern void TriggerPlayerPushLeverActivated(edict_t* self, edict_t* other);
extern void TriggerPlayerUsePuzzleActivated(edict_t* self, edict_t* activator);
extern void TriggerPuzzleUse(edict_t* self, edict_t* other, edict_t* activator);
extern void TriggerQuakeActivated(edict_t* self, edict_t* other);
extern void TriggerQuitToMenuTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf);
extern void TriggerQuitToMenuUse(edict_t* self, edict_t* other, edict_t* activator);
extern void TriggerRelayUse(edict_t* self, edict_t* other, edict_t* activator);