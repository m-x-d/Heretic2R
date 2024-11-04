#ifndef _G_TELEPORT_H_
#define _G_TELEPORT_H_

#include "g_local.h"

// TELEPORT defines

#define TELE_TIME		4					// number of server frames we take to do the fades
#define TELE_FADE		255/TELE_TIME		// amount to fade the player by each fade

#define TELE_TIME_OUT	5					// number of server frames we take to do the fades
#define TELE_FADE_OUT	255/TELE_TIME_OUT	// amount to fade the player by each fade

#define MORPH_TELE_TIME 5					// number of server frames to do the fade
#define MORPH_TELE_FADE 255/MORPH_TELE_TIME

#define DEATHMATCH_RANDOM	2

void teleporter_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);			 
void SpellCastTeleport(edict_t *caster,vec3_t StartPos,vec3_t AimAngles,vec3_t AimDir,float Value);
void CleanUpTeleport(edict_t *self);
void Perform_Teleport(edict_t *self);
void Perform_Morph(edict_t *self);
void CleanUpMorph(edict_t *self);
void reset_morph_to_elf(edict_t *ent);
extern void	SP_misc_teleport(edict_t *ent);

#endif // _G_TELEPORT_H_