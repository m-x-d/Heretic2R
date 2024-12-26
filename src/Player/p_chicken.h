//
// p_chicken.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "p_types.h"

// Dummy anim to catch sequence leaks.
extern panimmove_t chickenp_move_dummy;

extern panimmove_t chickenp_move_stand;
extern panimmove_t chickenp_move_stand1;
extern panimmove_t chickenp_move_stand2;
extern panimmove_t chickenp_move_walk;
extern panimmove_t chickenp_move_run;
extern panimmove_t chickenp_move_back;
extern panimmove_t chickenp_move_runb;
extern panimmove_t chickenp_move_bite; //TODO: unused
extern panimmove_t chickenp_move_strafel;
extern panimmove_t chickenp_move_strafer;
extern panimmove_t chickenp_move_jump;
extern panimmove_t chickenp_move_wjump;
extern panimmove_t chickenp_move_wjumpb; //TODO: unused
extern panimmove_t chickenp_move_rjump;
extern panimmove_t chickenp_move_rjumpb;
extern panimmove_t chickenp_move_jump_loop;
extern panimmove_t chickenp_move_attack;
extern panimmove_t chickenp_move_jump_flap;
extern panimmove_t chickenp_move_runattack;
extern panimmove_t chickenp_move_swim_idle;
extern panimmove_t chickenp_move_swim;

void PlayerChickenBite(playerinfo_t* info);
void PlayerChickenSqueal(const playerinfo_t* info);
int PlayerChickenJump(playerinfo_t* info);

void PlayerChickenFlap(playerinfo_t* info);
void PlayerChickenCheckFlap(playerinfo_t* info);

void ChickenAssert(playerinfo_t* info); //TODO: rename to PlayerChickenAssert?
void PlayerChickenCluck(const playerinfo_t* info, float force);

void ChickenStepSound(const playerinfo_t* info, float value); //TODO: rename to PlayerChickenStepSound?