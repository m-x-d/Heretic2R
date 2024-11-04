//==============================================================================
//
// m_rat_anim.c
//
// Heretic II
// Copyright 1998 Raven Software
//
//==============================================================================

#include "g_local.h"

#pragma hdrstop("g_local.pch")
// PRECOMPILED HEADER ABOVE
// WARNING:  DO NOT CHANGE THE ABOVE HEADERS OR THE PRECOMPILED STUFF WILL BREAK!  
// ADD ANY ADDITIONAL FILES BELOW

#include "m_rat_anim.h"
#include "m_rat.h"

#include "g_monster.h"

void rat_ai_run (edict_t *self, float dist);

/*----------------------------------------------------------------------
  Rat Death 2 - the little death, flipping over
-----------------------------------------------------------------------*/
animframe_t rat_frames_death2 [] =
{
	FRAME_DeathB1,	NULL, 0, 0, 0,  NULL, 0, ratdeathsqueal,
	FRAME_DeathB2,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_DeathB3,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_DeathB4,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_DeathB5,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_DeathB6,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_DeathB7,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_DeathB9,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_DeathB9,	NULL, 0, 0, 0,  NULL, 0, NULL
};

animmove_t rat_move_death2 = {9, rat_frames_death2, M_EndDeath};


/*----------------------------------------------------------------------
  Rat Death 1 - the big death, flying backwards and flipping over
-----------------------------------------------------------------------*/
animframe_t rat_frames_death1 [] =
{
	FRAME_deathA1,	NULL, 0, 0, 0,  ai_move, 0, ratdeathsqueal,
	FRAME_deathA2,	NULL, 0, 0, 0,  ai_move, 0, NULL,
	FRAME_deathA3,	NULL, 0, 0, 0,  ai_move, 0, NULL,
	FRAME_deathA4,	NULL, 0, 0, 0,  ai_move, 0, NULL,
	FRAME_deathA5,	NULL, 0, 0, 0,  ai_move, 0, NULL,
	FRAME_deathA6,	NULL, 0, 0, 0,  ai_move, 0, NULL,
	FRAME_deathA7,	NULL, 0, 0, 0,  ai_move, 0, NULL,
	FRAME_deathA8,	NULL, 0, 0, 0,  ai_move, 0, NULL,
	FRAME_deathA9,	NULL, 0, 0, 0,  ai_move, 0, NULL,
	FRAME_deathA10,	NULL, 0, 0, 0,  ai_move, 0, NULL,
	FRAME_deathA11,	NULL, 0, 0, 0,  ai_move, 0, NULL,
	FRAME_deathA12,	NULL, 0, 0, 0,  ai_move, 0, NULL,
	FRAME_deathA13,	NULL, 0, 0, 0,  ai_move, 0, NULL,
	FRAME_deathA14,	NULL, 0, 0, 0,  ai_move, 0, NULL,
};

animmove_t rat_move_death1 = {14, rat_frames_death1, M_EndDeath};


/*----------------------------------------------------------------------
  Rat Pain - backup and run away
-----------------------------------------------------------------------*/
animframe_t rat_frames_pain1 [] =
{
	FRAME_backup1,	NULL, 0, 0, 0,  ai_move, -4, ratsqueal,
	FRAME_backup2,	NULL, 0, 0, 0,  ai_move, -4, NULL,
	FRAME_backup3,	NULL, 0, 0, 0,  ai_move, -4, NULL,
	FRAME_backup4,	NULL, 0, 0, 0,  ai_move, -4, NULL,
	FRAME_backup5,	NULL, 0, 0, 0,  ai_move, -4, NULL,
	FRAME_backup6,	NULL, 0, 0, 0,  ai_move, -4, NULL
};

animmove_t rat_move_pain1 = {6, rat_frames_pain1, rat_pause};

/*----------------------------------------------------------------------
  Rat Melee - rat attacking at feet
-----------------------------------------------------------------------*/
animframe_t rat_frames_melee1 [] =
{
	FRAME_eat1,	NULL, 0, 0, 0,  ai_charge, 0, NULL,
	FRAME_eat2,	NULL, 0, 0, 0,  ai_charge, 0, NULL,
	FRAME_eat3,	NULL, 0, 0, 0,  ai_charge, 0, NULL,
	FRAME_eat4,	NULL, 0, 0, 0,  ai_charge, 0, NULL,
	FRAME_eat5,	NULL, 0, 0, 0,  ai_charge, 0, ratbite
};

animmove_t rat_move_melee1 = {5, rat_frames_melee1, rat_pause};


/*----------------------------------------------------------------------
  Rat Melee - rat attacking jumping in the air
-----------------------------------------------------------------------*/
animframe_t rat_frames_melee2 [] =
{
	FRAME_attack1,	NULL, 0, 0, 0,  ai_move, 0, ratjump,
	FRAME_attack2,	NULL, 0, 0, 0,  ai_move, 0, NULL,
	FRAME_attack3,	NULL, 0, 0, 0,  ai_move, 0, NULL,
	FRAME_attack4,	NULL, 0, 0, 0,  ai_move, 0, NULL,
	FRAME_attack5,	NULL, 0, 0, 0,  ai_move, 0, ratbite,
	FRAME_attack6,	NULL, 0, 0, 0,  ai_move, 0, NULL,
	FRAME_attack7,	NULL, 0, 0, 0,  ai_move, 0, NULL,
	FRAME_attack8,	NULL, 0, 0, 0,  ai_move, 0, NULL,
	FRAME_attack9,	NULL, 0, 0, 0,  ai_move, 0, NULL,
	FRAME_attack10,	NULL, 0, 0, 0,  ai_move, 0, NULL,
	FRAME_attack11,	NULL, 0, 0, 0,  ai_move, 0, NULL,
};

animmove_t rat_move_melee2 = {11, rat_frames_melee2, rat_pause};

/*----------------------------------------------------------------------
  Rat Melee - rat attacking 2
-----------------------------------------------------------------------*/
animframe_t rat_frames_melee3 [] =
{
	FRAME_eat12,	NULL, 0, 0, 0,  ai_move, 0, NULL,
	FRAME_eat13,	NULL, 0, 0, 0,  ai_move, 0, NULL,
	FRAME_eat14,	NULL, 0, 0, 0,  ai_move, 0, NULL,
	FRAME_eat15,	NULL, 0, 0, 0,  ai_move, 0, NULL,
	FRAME_eat16,	NULL, 0, 0, 0,  ai_move, 0, NULL,
	FRAME_eat17,	NULL, 0, 0, 0,  ai_move, 0, NULL,
	FRAME_eat18,	NULL, 0, 0, 0,  ai_move, 0, ratbite,
	FRAME_eat19,	NULL, 0, 0, 0,  ai_move, 0, NULL,
	FRAME_eat20,	NULL, 0, 0, 0,  ai_move, 0, NULL,
	FRAME_eat21,	NULL, 0, 0, 0,  ai_move, 0, NULL,
	FRAME_eat22,	NULL, 0, 0, 0,  ai_move, 0, NULL,
	FRAME_eat23,	NULL, 0, 0, 0,  ai_move, 0, NULL,
	FRAME_eat24,	NULL, 0, 0, 0,  ai_move, 0, NULL
};

animmove_t rat_move_melee3 = {13, rat_frames_melee3, rat_pause};


/*----------------------------------------------------------------------
  Rat Running - rat running to the right
-----------------------------------------------------------------------*/
animframe_t rat_frames_run3 [] =
{
	FRAME_run_rt1,	NULL, 0, 0, 0,  rat_ai_run, 10, ratchatter,
	FRAME_run_rt2,	NULL, 0, 0, 0,  rat_ai_run, 10, NULL,
	FRAME_run_rt3,	NULL, 0, 0, 0,  rat_ai_run, 10, NULL,
	FRAME_run_rt4,	NULL, 0, 0, 0,  rat_ai_run, 10, rat_runorder
};

animmove_t rat_move_run3 = {4, rat_frames_run3, NULL};

/*----------------------------------------------------------------------
  Rat Running - rat running to the left
-----------------------------------------------------------------------*/
animframe_t rat_frames_run2 [] =
{
	FRAME_run_lft1,	NULL, 0, 0, 0,  rat_ai_run, 10, ratchatter,
	FRAME_run_lft2,	NULL, 0, 0, 0,  rat_ai_run, 10, NULL,
	FRAME_run_lft3,	NULL, 0, 0, 0,  rat_ai_run, 10, NULL,
	FRAME_run_lft4,	NULL, 0, 0, 0,  rat_ai_run, 10, NULL
};

animmove_t rat_move_run2 = {4, rat_frames_run2, rat_runorder};

/*----------------------------------------------------------------------
  Rat Running - rat running
-----------------------------------------------------------------------*/
animframe_t rat_frames_run1 [] =
{
	FRAME_run1,	NULL, 0, 0, 0,  rat_ai_run, 10, ratchatter,
	FRAME_run2,	NULL, 0, 0, 0,  rat_ai_run, 20, NULL,
	FRAME_run3,	NULL, 0, 0, 0,  rat_ai_run, 10, NULL,
	FRAME_run4,	NULL, 0, 0, 0,  rat_ai_run, 10, NULL,
	FRAME_run5,	NULL, 0, 0, 0,  rat_ai_run, 10, NULL
};

animmove_t rat_move_run1 = {5, rat_frames_run1, rat_runorder};

/*----------------------------------------------------------------------
  Rat Walking - rat walking
-----------------------------------------------------------------------*/
animframe_t rat_frames_walk1 [] =
{
	FRAME_walk1,	NULL, 0, 0, 0,  ai_walk, 4, ratchatter,
	FRAME_walk2,	NULL, 0, 0, 0,  ai_walk, 4, NULL,
	FRAME_walk3,	NULL, 0, 0, 0,  ai_walk, 4, NULL,
	FRAME_walk4,	NULL, 0, 0, 0,  ai_walk, 4, NULL,
	FRAME_walk5,	NULL, 0, 0, 0,  ai_walk, 4, NULL,
	FRAME_walk6,	NULL, 0, 0, 0,  ai_walk, 4, NULL
};

animmove_t rat_move_walk1 = {6, rat_frames_walk1, NULL};

/*----------------------------------------------------------------------
  Rat Stand8 - from haunches, dropping down to ground
-----------------------------------------------------------------------*/

animframe_t rat_frames_stand8 [] =
{
//	FRAME_haunch43,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_haunch44,	NULL, 0, 0, 0,  rat_ai_stand, 0, NULL,
	FRAME_haunch45,	NULL, 0, 0, 0,  NULL/*ai_stand*/, 0, NULL,
	FRAME_haunch46,	NULL, 0, 0, 0,  NULL/*ai_stand*/, 0, NULL,
	FRAME_haunch47,	NULL, 0, 0, 0,  rat_ai_stand, 0, NULL,
	FRAME_haunch48,	NULL, 0, 0, 0,  NULL/*ai_stand*/, 0, NULL,
	FRAME_haunch49,	NULL, 0, 0, 0,  NULL/*ai_stand*/, 0, NULL
};

animmove_t rat_move_stand8 = {5, rat_frames_stand8, rat_standorder};

/*----------------------------------------------------------------------
  Rat Stand7 - sitting on haunches, scratch right
-----------------------------------------------------------------------*/
animframe_t rat_frames_stand7 [] =
{
	FRAME_haunch38,	NULL, 0, 0, 0,  rat_ai_stand, 0, NULL,
	FRAME_haunch39,	NULL, 0, 0, 0,  NULL/*ai_stand*/, 0, NULL,
	FRAME_haunch40,	NULL, 0, 0, 0,  NULL/*ai_stand*/, 0, ratscratch,
	FRAME_haunch41,	NULL, 0, 0, 0,  rat_ai_stand, 0, NULL,
	FRAME_haunch42,	NULL, 0, 0, 0,  NULL/*ai_stand*/, 0, NULL
};

animmove_t rat_move_stand7 = {5, rat_frames_stand7, rat_standorder};

/*----------------------------------------------------------------------
  Rat Stand6 - sitting on haunches, scratch left
-----------------------------------------------------------------------*/
animframe_t rat_frames_stand6 [] =
{
//	FRAME_haunch31,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_haunch32,	NULL, 0, 0, 0,  rat_ai_stand, 0, NULL,
	FRAME_haunch33,	NULL, 0, 0, 0,  NULL/*ai_stand*/, 0, NULL,
	FRAME_haunch34,	NULL, 0, 0, 0,  NULL/*ai_stand*/, 0, ratscratch,
	FRAME_haunch35,	NULL, 0, 0, 0,  rat_ai_stand, 0, NULL,
	FRAME_haunch36,	NULL, 0, 0, 0,  NULL/*ai_stand*/, 0, NULL,
//	FRAME_haunch37,	NULL, 0, 0, 0,  ai_stand, 0, NULL
};

animmove_t rat_move_stand6 = {5, rat_frames_stand6, rat_standorder};

/*----------------------------------------------------------------------
  Rat Stand5 - sitting on haunches, look right
-----------------------------------------------------------------------*/
animframe_t rat_frames_stand5 [] =
{
	FRAME_haunch26,	NULL, 0, 0, 0,  rat_ai_stand, 0, NULL,
	FRAME_haunch27,	NULL, 0, 0, 0,  NULL/*ai_stand*/, 0, NULL,
	FRAME_haunch28,	NULL, 0, 0, 0,  NULL/*ai_stand*/, 0, NULL,
	FRAME_haunch29,	NULL, 0, 0, 0,  rat_ai_stand, 0, NULL,
	FRAME_haunch30,	NULL, 0, 0, 0,  NULL/*ai_stand*/, 0, NULL
};

animmove_t rat_move_stand5 = {5, rat_frames_stand5, rat_standorder};

/*----------------------------------------------------------------------
  Rat Stand4 - sitting on haunches, looking left
-----------------------------------------------------------------------*/
animframe_t rat_frames_stand4 [] =
{
	FRAME_haunch20,	NULL, 0, 0, 0,  rat_ai_stand, 0, NULL,
	FRAME_haunch21,	NULL, 0, 0, 0,  NULL/*ai_stand*/, 0, NULL,
	FRAME_haunch22,	NULL, 0, 0, 0,  NULL/*ai_stand*/, 0, NULL,
	FRAME_haunch23,	NULL, 0, 0, 0,  rat_ai_stand, 0, NULL,
	FRAME_haunch24,	NULL, 0, 0, 0,  NULL/*ai_stand*/, 0, NULL
//	FRAME_haunch25,	NULL, 0, 0, 0,  ai_stand, 0, NULL
};

animmove_t rat_move_stand4 = {5, rat_frames_stand4, rat_standorder};

/*----------------------------------------------------------------------
  Rat Stand3 - sitting on haunches
-----------------------------------------------------------------------*/
animframe_t rat_frames_stand3 [] =
{
	FRAME_haunch12,	NULL, 0, 0, 0,  rat_ai_stand, 0, ratchatter,
	FRAME_haunch13,	NULL, 0, 0, 0,  NULL/*ai_stand*/, 0, NULL,
	FRAME_haunch14,	NULL, 0, 0, 0,  NULL/*ai_stand*/, 0, NULL,
	FRAME_haunch15,	NULL, 0, 0, 0,  NULL/*ai_stand*/, 0, NULL,
	FRAME_haunch16,	NULL, 0, 0, 0,  rat_ai_stand, 0, NULL,
	FRAME_haunch17,	NULL, 0, 0, 0,  NULL/*ai_stand*/, 0, NULL,
	FRAME_haunch18,	NULL, 0, 0, 0,  NULL/*ai_stand*/, 0, NULL
//RAME_haunch19,	NULL, 0, 0, 0,  ai_stand, 0, NULL
};

animmove_t rat_move_stand3 = {7, rat_frames_stand3, rat_standorder};

/*----------------------------------------------------------------------
  Rat Stand2 - rising up to sit on haunches
-----------------------------------------------------------------------*/
animframe_t rat_frames_stand2 [] =
{
	FRAME_haunch1,	NULL, 0, 0, 0,  rat_ai_stand, 0, NULL,
	FRAME_haunch2,	NULL, 0, 0, 0,  NULL/*ai_stand*/, 0, NULL,
	FRAME_haunch3,	NULL, 0, 0, 0,  NULL/*ai_stand*/, 0, NULL,
	FRAME_haunch4,	NULL, 0, 0, 0,  rat_ai_stand, 0, NULL,
	FRAME_haunch5,	NULL, 0, 0, 0,  NULL/*ai_stand*/, 0, NULL,
	FRAME_haunch6,	NULL, 0, 0, 0,  NULL/*ai_stand*/, 0, NULL,
	FRAME_haunch7,	NULL, 0, 0, 0,  rat_ai_stand, 0, NULL,
	FRAME_haunch8,	NULL, 0, 0, 0,  NULL/*ai_stand*/, 0, NULL,
	FRAME_haunch9,	NULL, 0, 0, 0,  NULL/*ai_stand*/, 0, NULL,
	FRAME_haunch10,	NULL, 0, 0, 0,  NULL/*ai_stand*/, 0, NULL
//	FRAME_haunch11,	NULL, 0, 0, 0,  ai_stand, 0, NULL
};

animmove_t rat_move_stand2 = {10, rat_frames_stand2, rat_standorder};

/*----------------------------------------------------------------------
  Rat Standing - 
-----------------------------------------------------------------------*/
animframe_t rat_frames_stand1 [] =
{
	FRAME_idle1,	NULL, 0, 0, 0,  rat_ai_stand, 0, ratchatter,
	FRAME_idle2,	NULL, 0, 0, 0,  NULL/*ai_stand*/, 0, NULL,
	FRAME_idle3,	NULL, 0, 0, 0,  NULL/*ai_stand*/, 0, NULL,
	FRAME_idle4,	NULL, 0, 0, 0,  NULL/*ai_stand*/, 0, NULL,
	FRAME_idle5,	NULL, 0, 0, 0,  rat_ai_stand, 0, NULL,
	FRAME_idle6,	NULL, 0, 0, 0,  NULL/*ai_stand*/, 0, NULL,
	FRAME_idle7,	NULL, 0, 0, 0,  NULL/*ai_stand*/, 0, NULL,
	FRAME_idle8,	NULL, 0, 0, 0,  NULL/*ai_stand*/, 0, NULL
};

animmove_t rat_move_stand1 = {8, rat_frames_stand1, rat_standorder};


/*----------------------------------------------------------------------
  Rat Watch2 - Stand up and hiss then go back to all fours
-----------------------------------------------------------------------*/
animframe_t rat_frames_watch2 [] =
{
	FRAME_hiss1,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_hiss2,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_hiss3,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_hiss4,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_hiss5,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_hiss6,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_hiss7,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_hiss8,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_hiss9,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_hiss10,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_hiss11,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_hiss12,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_hiss13,	NULL, 0, 0, 0, NULL, 0, NULL,
};
animmove_t rat_move_watch2 = {13, rat_frames_watch2, rat_eatorder};

/*----------------------------------------------------------------------
  Rat Hiss 1 - Hiss while on all fours
-----------------------------------------------------------------------*/
animframe_t rat_frames_watch1 [] =
{
	FRAME_hiss14,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_hiss15,	NULL, 0, 0, 0, NULL, 0, rathiss,
	FRAME_hiss16,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_hiss17,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_hiss18,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_hiss19,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_hiss20,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_hiss21,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_hiss22,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_hiss23,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_hiss24,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_hiss25,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_hiss26,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_hiss27,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_hiss28,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_hiss29,	NULL, 0, 0, 0, NULL, 0, NULL
};

animmove_t rat_move_watch1 = {16, rat_frames_watch1, rat_eatorder};

/*----------------------------------------------------------------------
  Rat Eat3 - Bite and pull back a little
-----------------------------------------------------------------------*/
animframe_t rat_frames_eat3 [] =
{
	FRAME_eat9,		NULL, 0, 0, 0,  rat_ai_eat, 0, NULL,
	FRAME_eat10,	NULL, 0, 0, 0,  NULL/*rat_ai_eat*/, 0, NULL,
	FRAME_eat11,	NULL, 0, 0, 0,  NULL/*rat_ai_eat*/, 0, ratchew,
	FRAME_eat12,	NULL, 0, 0, 0,  NULL/*rat_ai_eat*/, 0, NULL
};

animmove_t rat_move_eat3 = {4, rat_frames_eat3, rat_eatorder};

/*----------------------------------------------------------------------
  Rat Eat2 - Bite low and tear up
-----------------------------------------------------------------------*/
animframe_t rat_frames_eat2 [] =
{
	FRAME_eat12,	NULL, 0, 0, 0,  rat_ai_eat, 0, NULL,
	FRAME_eat13,	NULL, 0, 0, 0,  NULL/*rat_ai_eat*/, 0, NULL,
	FRAME_eat14,	NULL, 0, 0, 0,  NULL/*rat_ai_eat*/, 0, NULL,
	FRAME_eat15,	NULL, 0, 0, 0,  NULL/*rat_ai_eat*/, 0, NULL,
	FRAME_eat16,	NULL, 0, 0, 0,  NULL/*rat_ai_eat*/, 0, NULL,
	FRAME_eat17,	NULL, 0, 0, 0,  NULL/*rat_ai_eat*/, 0, ratchew,
	FRAME_eat18,	NULL, 0, 0, 0,  rat_ai_eat, 0, NULL,
	FRAME_eat19,	NULL, 0, 0, 0,  NULL/*rat_ai_eat*/, 0, NULL,
	FRAME_eat20,	NULL, 0, 0, 0,  NULL/*rat_ai_eat*/, 0, ratswallow,
	FRAME_eat21,	NULL, 0, 0, 0,  NULL/*rat_ai_eat*/, 0, NULL,
	FRAME_eat22,	NULL, 0, 0, 0,  NULL/*rat_ai_eat*/, 0, NULL,
	FRAME_eat23,	NULL, 0, 0, 0,  NULL/*rat_ai_eat*/, 0, NULL,
	FRAME_eat24,	NULL, 0, 0, 0,  NULL/*rat_ai_eat*/, 0, NULL
};

animmove_t rat_move_eat2 = {13, rat_frames_eat2, rat_eatorder};

/*----------------------------------------------------------------------
  Rat Eat1 - Bite down low
-----------------------------------------------------------------------*/
animframe_t rat_frames_eat1 [] =
{
	FRAME_eat1,	NULL, 0, 0, 0,  rat_ai_eat, 0, NULL,
	FRAME_eat2,	NULL, 0, 0, 0,  NULL/*rat_ai_eat*/, 0, NULL,
	FRAME_eat3,	NULL, 0, 0, 0,  NULL/*rat_ai_eat*/, 0, NULL,
	FRAME_eat4,	NULL, 0, 0, 0,  NULL/*rat_ai_eat*/, 0, NULL,
	FRAME_eat5,	NULL, 0, 0, 0,  NULL/*rat_ai_eat*/, 0, ratchew,
	FRAME_eat6,	NULL, 0, 0, 0,  rat_ai_eat, 0, NULL,
	FRAME_eat7,	NULL, 0, 0, 0,  NULL/*rat_ai_eat*/, 0, NULL,
	FRAME_eat8,	NULL, 0, 0, 0,  NULL/*rat_ai_eat*/, 0, ratchew,
	FRAME_eat9,	NULL, 0, 0, 0,  NULL/*rat_ai_eat*/, 0, NULL
};

animmove_t rat_move_eat1 = {9, rat_frames_eat1, rat_eatorder};
