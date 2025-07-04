//
// p_anim_branch.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "Player.h"

// Chicken Branch.
extern int ChickenBranchLwrStanding(playerinfo_t *info);
extern void ChickenBranchIdle(playerinfo_t *info);

// Player Lower Branch.
extern int BranchLwrStanding(playerinfo_t *info);
extern int BranchLwrStandingRun(playerinfo_t *info);
extern int BranchLwrRunning(playerinfo_t *info);
extern int BranchLwrWalking(playerinfo_t *info);
extern int BranchLwrShortstep(playerinfo_t *info);
extern int BranchLwrBackspring(playerinfo_t *info);
extern int BranchLwrCrouching(playerinfo_t *info);
extern int BranchLwrJumping(playerinfo_t *info);
extern int BranchLwrSurfaceSwim(playerinfo_t *info);
extern int BranchLwrUnderwaterSwim(playerinfo_t *info);
extern int BranchLwrClimbing(playerinfo_t *info);
extern int BranchLwrKnockDown(playerinfo_t *info);
extern int BranchLwrRunningStrafe(playerinfo_t *info);

// Player Upper Branch.
extern int BranchIdle(const playerinfo_t *info);
extern int BranchUprReady(playerinfo_t *info);
extern int BranchCheckBowAmmo(playerinfo_t *info);
extern int BranchCheckHellAmmo(playerinfo_t *info);
extern int BranchCheckMana(playerinfo_t *info);

//mxd. Utility
extern qboolean CheckFall(const playerinfo_t* info);
extern qboolean CheckUncrouch(const playerinfo_t* info);

extern PLAYER_API qboolean BranchCheckDismemberAction(const playerinfo_t *info, int weapon);