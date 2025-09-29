//
// p_main.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "Player.h" //mxd
#include "p_types.h"

#define PLAYER_FLAG_NONE				0x00000000
#define PLAYER_FLAG_FALLING				0x00000001
#define PLAYER_FLAG_IDLE				0x00000002 //TODO: set, but never used?
#define PLAYER_FLAG_FALLBREAK			0x00000004 //TODO: set, but never used?
#define PLAYER_FLAG_BOWDRAWN			0x00000008 //TODO: set, but never used?
#define PLAYER_FLAG_TURNLOCK			0x00000010
#define PLAYER_FLAG_TURNDAMP			0x00000020 //TODO: unused.
#define PLAYER_FLAG_SURFSWIM			0x00000040
#define PLAYER_FLAG_UNDERWATER			0x00000080
#define PLAYER_FLAG_RESIZED				0x00000100
#define PLAYER_FLAG_ONROPE				0x00000200
#define PLAYER_FLAG_STAND				0x00000400
#define PLAYER_FLAG_TURN180				0x00000800 //TODO: set, but never used?
#define PLAYER_FLAG_LEAVELOWER			0x00001000
#define PLAYER_FLAG_WATER				(PLAYER_FLAG_UNDERWATER | PLAYER_FLAG_SURFSWIM)

// We MASK out the low two bytes every time an animation is set.
#define PLAYER_FLAG_ANIMMASK			0x0000FFFF //TODO: unused.
#define PLAYER_FLAG_PERSMASK			0xFFFF0000

// The two high bytes are persistent.
#define PLAYER_FLAG_TELEPORT			0x00010000
#define PLAYER_FLAG_MORPHING			0x00020000
#define PLAYER_FLAG_LOCKMOVE_WAS_SET	0x00040000
#define PLAYER_FLAG_USE_ENT_POS			0x00080000
#define PLAYER_FLAG_BLEED				0x00100000	// Player is bleeding.
#define PLAYER_FLAG_NO_LARM				0x00200000	// Player lost left arm.
#define PLAYER_FLAG_NO_RARM				0x00400000	// Player lost right arm.
#define PLAYER_FLAG_ALTFIRE				0x00800000	// This alternates every time a weapon fires, so they don't cut out.
#define PLAYER_FLAG_COLLISION			0x01000000
#define PLAYER_FLAG_DIVE				0x02000000
#define PLAYER_FLAG_SLIDE				0x04000000
#define PLAYER_FLAG_KNOCKDOWN			0x08000000
#define PLAYER_FLAG_RELEASEROPE			0x10000000

extern PLAYER_API void PlayerInit(playerinfo_t *info, int complete_reset); //TODO: change complete_reset type to qboolean?
extern PLAYER_API void PlayerClearEffects(const playerinfo_t *info);
extern PLAYER_API void PlayerUpdate(playerinfo_t *info);
extern PLAYER_API void PlayerUpdateCmdFlags(playerinfo_t *info);
extern PLAYER_API void PlayerUpdateModelAttributes(playerinfo_t *info);
extern void PlayerSetHandFX(playerinfo_t *info, int handfxtype, int lifetime);