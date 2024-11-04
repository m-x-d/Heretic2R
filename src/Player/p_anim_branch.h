//
// p_anim_branch2.h
//
// Heretic II
// Copyright 1998 Raven Software
//

#ifndef _P_ANIM_BRANCH2_H_
#define _P_ANIM_BRANCH2_H_

// Chicken Branch.

int ChickenBranchLwrStanding(playerinfo_t *playerinfo);
int ChickenBranchidle(playerinfo_t *playerinfo);

// Player Lower Branch.

int BranchLwrStanding(playerinfo_t *playerinfo);
int BranchLwrStandingRun(playerinfo_t *playerinfo);
int BranchLwrRunning(playerinfo_t *playerinfo);
int BranchLwrWalking(playerinfo_t *playerinfo);
int BranchLwrShortstep(playerinfo_t *playerinfo);
int BranchLwrBackspring(playerinfo_t *playerinfo);
int BranchLwrCrouching(playerinfo_t *playerinfo);
int BranchLwrJumping(playerinfo_t *playerinfo);
int BranchLwrSliding(playerinfo_t *playerinfo);
int BranchLwrSurfaceSwim(playerinfo_t *playerinfo);
int BranchLwrUnderwaterSwim(playerinfo_t *playerinfo);
int BranchLwrHanging(playerinfo_t *playerinfo);
int BranchLwrClimbing(playerinfo_t *playerinfo);
int BranchLwrKnockDown(playerinfo_t *playerinfo);
int BranchLwrRunningStrafe(playerinfo_t *playerinfo);

// Player Upper Branch.

int BranchIdle(playerinfo_t *playerinfo);
int BranchUprReady(playerinfo_t *playerinfo);
int BranchUprRdySpell(playerinfo_t *playerinfo);
int BranchUprRdyStaff(playerinfo_t *playerinfo);
int BranchUprRdyBow(playerinfo_t *playerinfo);
int BranchCheckAmmo(playerinfo_t *playerinfo);
int BranchCheckHellAmmo(playerinfo_t *playerinfo);
int BranchCheckMana(playerinfo_t *playerinfo);

PLAYER_API qboolean BranchCheckDismemberAction(playerinfo_t *playerinfo, int weapon);

#endif // _P_ANIM_BRANCH2_H_
