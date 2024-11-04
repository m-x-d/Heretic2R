#include "g_local.h"
#include "m_seraph_guard.h"
#include "m_seraph_guard_anim.h"
#include "m_stats.h"

animframe_t seraph_guard_frames_stand[] =
{
	FRAME_idle1,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle2,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle3,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle4,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle5,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle6,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle7,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle8,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle9,  NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle10, NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle11, NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle12, NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle13, NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle14, NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle15, NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle16, NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle17, NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle18, NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle19, NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle20, NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle21, NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle22, NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle23, NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle24, NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle25, NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle26, NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle27, NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle28, NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_idle29, NULL, 0, 0, 0, ai_stand, 0, NULL,
};
animmove_t seraph_guard_move_stand = {29, seraph_guard_frames_stand, seraph_guard_pause};

animframe_t seraph_guard_frames_melee[] =
{
	FRAME_axea1,  NULL, 0, 0, 0, ai_charge, 0, NULL,
	FRAME_axea2,  NULL, 0, 0, 0, ai_charge, 0, NULL,
	FRAME_axea3,  NULL, 0, 0, 0, ai_charge, 0, NULL,
	FRAME_axea3,  NULL, 0, 0, 0, ai_charge, 0, NULL,
	FRAME_axea4,  NULL, 0, 0, 0, ai_charge, 0, NULL,
	FRAME_axea5,  seraph_guard_strike, SGUARD_DMG_AXE, 0, 0, ai_charge, 0, NULL,
	FRAME_axea5,  NULL, 0, 0, 0, ai_charge, 0, NULL,
	FRAME_axea5,  NULL, 0, 0, 0, ai_charge, 0, NULL,
	FRAME_axea5,  NULL, 0, 0, 0, ai_charge, 0, NULL,
	FRAME_axea6,  NULL, 0, 0, 0, ai_charge, 0, seraph_guard_checkpoke,
	FRAME_axea7,  NULL, 0, 0, 0, ai_charge, 0, NULL,
	FRAME_axea8,  NULL, 0, 0, 0, ai_charge, 0, NULL,
	FRAME_axea9,  NULL, 0, 0, 0, ai_charge, 0, NULL,
	FRAME_axea9,  NULL, 0, 0, 0, ai_charge, 0, NULL,
	FRAME_axea10, NULL, 0, 0, 0, ai_charge, 0, NULL,
	FRAME_axea11, seraph_guard_strike, SGUARD_DMG_AXE, 0, 0, ai_charge, 0, NULL,
	FRAME_axea11, NULL, 0, 0, 0, ai_charge, 0, NULL,
	FRAME_axea11, NULL, 0, 0, 0, ai_charge, 0, NULL,
	FRAME_axea11, NULL, 0, 0, 0, ai_charge, 0, NULL,
	FRAME_axea11, NULL, 0, 0, 0, ai_charge, 0, NULL,
	FRAME_axea12, NULL, 0, 0, 0, ai_charge, 0, NULL,
	FRAME_axea13, NULL, 0, 0, 0, ai_charge, 0, NULL,
};
animmove_t seraph_guard_move_melee = {22,seraph_guard_frames_melee, seraph_guard_pause};

animframe_t seraph_guard_frames_pain[] =
{
	FRAME_pain1, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_pain2, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_pain3, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_pain4, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_pain5, NULL, 0, 0, 0, NULL, 0, NULL,
};
animmove_t seraph_guard_move_pain = {5,seraph_guard_frames_pain, seraph_guard_pause};

animframe_t seraph_guard_frames_runmelee[] =
{
	FRAME_runaxe4, NULL, 0, 0, 0, ai_charge, 20, NULL,
	FRAME_runaxe5, NULL, 0, 0, 0, ai_charge, 24, NULL,
	FRAME_runaxe6, NULL, 0, 0, 0, ai_charge, 20, NULL,
	FRAME_runaxe7, seraph_guard_strike, SGUARD_DMG_AXE, 0, 0, ai_charge, 20, NULL,
	FRAME_runaxe8, NULL, 0, 0, 0, ai_charge, 20, NULL,
	FRAME_runaxe1, NULL, 0, 0, 0, ai_charge, 24, NULL,
	FRAME_runaxe2, NULL, 0, 0, 0, ai_charge, 20, NULL,
	FRAME_runaxe3, NULL, 0, 0, 0, ai_charge, 20, NULL,
};
animmove_t seraph_guard_move_runmelee = {8,seraph_guard_frames_runmelee, seraph_guard_pause};

animframe_t seraph_guard_frames_run[] =
{
	FRAME_run1, NULL, 0, 0, 0, ai_run, 24, seraph_guard_pause,
	FRAME_run2, NULL, 0, 0, 0, ai_run, 20, seraph_guard_pause,
	FRAME_run3, NULL, 0, 0, 0, ai_run, 20, seraph_guard_pause,
	FRAME_run4, NULL, 0, 0, 0, ai_run, 20, seraph_guard_pause,
	FRAME_run5, NULL, 0, 0, 0, ai_run, 24, seraph_guard_pause,
	FRAME_run6, NULL, 0, 0, 0, ai_run, 20, seraph_guard_pause,
	FRAME_run7, NULL, 0, 0, 0, ai_run, 20, seraph_guard_pause,
	FRAME_run8, NULL, 0, 0, 0, ai_run, 20, seraph_guard_pause,
};
animmove_t seraph_guard_move_run = {8,seraph_guard_frames_run, seraph_guard_pause};

animframe_t seraph_guard_frames_fjump[] =
{
	FRAME_run1, NULL, 0, 0, 0, NULL, 0, seraphGuardApplyJump,
	FRAME_run2, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_run3, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_run4, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_run5, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_run6, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_run7, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_run8, NULL, 0, 0, 0, NULL, 0, NULL,
};
animmove_t seraph_guard_move_fjump = {8,seraph_guard_frames_fjump, seraph_guard_pause};

animframe_t seraph_guard_frames_walk[] =
{
	FRAME_walk1,  NULL, 0, 0, 0, ai_walk, 6, NULL,
	FRAME_walk2,  NULL, 0, 0, 0, ai_walk, 6, NULL,
	FRAME_walk3,  NULL, 0, 0, 0, ai_walk, 6, NULL,
	FRAME_walk4,  NULL, 0, 0, 0, ai_walk, 5, NULL,
	FRAME_walk5,  NULL, 0, 0, 0, ai_walk, 4, NULL,
	FRAME_walk6,  NULL, 0, 0, 0, ai_walk, 4, NULL,
	FRAME_walk7,  NULL, 0, 0, 0, ai_walk, 5, NULL,
	FRAME_walk8,  NULL, 0, 0, 0, ai_walk, 6, NULL,
	FRAME_walk9,  NULL, 0, 0, 0, ai_walk, 6, NULL,
	FRAME_walk10, NULL, 0, 0, 0, ai_walk, 6, NULL,
	FRAME_walk11, NULL, 0, 0, 0, ai_walk, 6, NULL,
	FRAME_walk12, NULL, 0, 0, 0, ai_walk, 5, NULL,
	FRAME_walk13, NULL, 0, 0, 0, ai_walk, 4, NULL,
	FRAME_walk14, NULL, 0, 0, 0, ai_walk, 4, NULL,
	FRAME_walk15, NULL, 0, 0, 0, ai_walk, 5, NULL,
	FRAME_walk16, NULL, 0, 0, 0, ai_walk, 6, NULL,
};
animmove_t seraph_guard_move_walk = {16,seraph_guard_frames_walk, seraph_guard_pause};

animframe_t seraph_guard_frames_melee2[] =
{
	FRAME_axeb1, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_axeb2, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_axeb2, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_axeb3, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_axeb4, seraph_guard_strike, SGUARD_DMG_AXE_SPIN, 0, 0, NULL, 0, NULL,
	FRAME_axeb4, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_axeb4, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_axeb4, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_axeb4, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_axeb4, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_axeb5, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_axeb6, NULL, 0, 0, 0, NULL, 0, NULL,
};
animmove_t seraph_guard_move_melee2 = {12, seraph_guard_frames_melee2, seraph_guard_pause};

animframe_t seraph_guard_frames_melee3[] =
{
	FRAME_axec1,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_axec2,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_axec2,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_axec3,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_axec4,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_axec5,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_axec6,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_axec7,  seraph_guard_strike, SGUARD_DMG_AXE, 0, 0, NULL, 0, NULL,
	FRAME_axec7,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_axec7,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_axec7,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_axec7,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_axec7,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_axec8,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_axec9,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_axec10, NULL, 0, 0, 0, NULL, 0, NULL,
};
animmove_t seraph_guard_move_melee3 = {16, seraph_guard_frames_melee3, seraph_guard_pause};

animframe_t seraph_guard_frames_death1[] =
{
	FRAME_deatha1,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_deatha2,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_deatha3,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_deatha4,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_deatha5,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_deatha6,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_deatha7,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_deatha8,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_deatha9,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_deatha10, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_deatha11, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_deatha12, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_deatha13, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_deatha14, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_deatha15, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_deatha16, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_deatha17, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_deatha18, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_deatha19, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_deatha20, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_deatha21, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_deatha22, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_deatha23, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_deatha24, NULL, 0, 0, 0, NULL, 0, seraph_guard_dead,
};
animmove_t seraph_guard_move_death1 = {24, seraph_guard_frames_death1, NULL};

animframe_t seraph_guard_frames_death2_go[] =
{
	FRAME_deathb1,   NULL, 0, 0, 0, NULL, 0, seraph_guard_check_land,
	FRAME_deathb2,   NULL, 0, 0, 0, NULL, 0, seraph_guard_check_land,
	FRAME_deathb3,   NULL, 0, 0, 0, NULL, 0, seraph_guard_check_land,
	FRAME_deathb4,   NULL, 0, 0, 0, NULL, 0, seraph_guard_death_loop,
};
animmove_t seraph_guard_move_death2_go = {4, seraph_guard_frames_death2_go, NULL};

animframe_t seraph_guard_frames_death2_loop[] =
{
	FRAME_deathb5,   NULL, 0, 0, 0, NULL, 0, seraph_guard_check_land,
};
animmove_t seraph_guard_move_death2_loop = {1, seraph_guard_frames_death2_loop, NULL};

animframe_t seraph_guard_frames_death2_end[] =
{
	FRAME_deathb6,  NULL, 0, 0, 0, NULL, 0, seraph_guard_check_land,
	FRAME_deathb7,  NULL, 0, 0, 0, NULL, 0, seraph_guard_check_land,
	FRAME_deathb8,  NULL, 0, 0, 0, NULL, 0, seraph_guard_check_land,
	FRAME_deathb9,  NULL, 0, 0, 0, NULL, 0, seraph_guard_check_land,
	FRAME_deathb10, NULL, 0, 0, 0, NULL, 0, seraph_guard_check_land,
	FRAME_deathb11, NULL, 0, 0, 0, NULL, 0, seraph_guard_check_land,
	FRAME_deathb12, NULL, 0, 0, 0, NULL, 0, seraph_guard_check_land,
	FRAME_deathb13, NULL, 0, 0, 0, NULL, 0, seraph_guard_check_land,
	FRAME_deathb14, NULL, 0, 0, 0, NULL, 0, seraph_guard_check_land,
	FRAME_deathb15, NULL, 0, 0, 0, NULL, 0, seraph_guard_dead,
};
animmove_t seraph_guard_move_death2_end = {10, seraph_guard_frames_death2_end, NULL};

animframe_t seraph_guard_frames_backup[] =
{
	FRAME_backup1, NULL, 0, 0, 0, seraph_guard_back, 24, NULL,
	FRAME_backup2, NULL, 0, 0, 0, seraph_guard_back, 20, NULL,
	FRAME_backup3, NULL, 0, 0, 0, seraph_guard_back, 20, NULL,
	FRAME_backup4, NULL, 0, 0, 0, seraph_guard_back, 20, NULL,
	FRAME_backup5, NULL, 0, 0, 0, seraph_guard_back, 24, NULL,
	FRAME_backup6, NULL, 0, 0, 0, seraph_guard_back, 20, NULL,
	FRAME_backup7, NULL, 0, 0, 0, seraph_guard_back, 20, NULL,
	FRAME_backup8, NULL, 0, 0, 0, seraph_guard_back, 20, NULL,
	FRAME_backup9, NULL, 0, 0, 0, seraph_guard_back, 20, NULL,
	FRAME_backup10, NULL, 0, 0, 0, seraph_guard_back, 20, NULL,
	FRAME_backup11, NULL, 0, 0, 0, seraph_guard_back, 24, NULL,
	FRAME_backup12, NULL, 0, 0, 0, seraph_guard_back, 20, NULL,
	FRAME_backup13, NULL, 0, 0, 0, seraph_guard_back, 20, NULL,
	FRAME_backup14, NULL, 0, 0, 0, seraph_guard_back, 20, NULL,
	FRAME_backup15, NULL, 0, 0, 0, seraph_guard_back, 24, NULL,
	FRAME_backup16, NULL, 0, 0, 0, seraph_guard_back, 20, NULL,
};
animmove_t seraph_guard_move_backup = {16, seraph_guard_frames_backup, seraph_guard_pause};

animframe_t seraph_guard_frames_missile[] =
{
	FRAME_newshot1,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_newshot2,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_newshot3,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_newshot4,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_newshot5,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_newshot6,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_newshot7,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_newshot8,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_newshot9,  NULL, 0, 0, 0, NULL, 0, seraph_guard_fire,
	FRAME_newshot10, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_newshot11, NULL, 0, 0, 0, NULL, 0, NULL,
};
animmove_t seraph_guard_move_missile = {11, seraph_guard_frames_missile, seraph_guard_pause};

animframe_t seraph_guard_frames_delay[] =
{
	FRAME_idle1,  NULL, 0, 0, 0, NULL, 0, seraph_guard_pause,
	FRAME_idle2,  NULL, 0, 0, 0, NULL, 0, seraph_guard_pause,
	FRAME_idle3,  NULL, 0, 0, 0, NULL, 0, seraph_guard_pause,
	FRAME_idle4,  NULL, 0, 0, 0, NULL, 0, seraph_guard_pause,
	FRAME_idle5,  NULL, 0, 0, 0, NULL, 0, seraph_guard_pause,
	FRAME_idle6,  NULL, 0, 0, 0, NULL, 0, seraph_guard_pause,
	FRAME_idle7,  NULL, 0, 0, 0, NULL, 0, seraph_guard_pause,
	FRAME_idle8,  NULL, 0, 0, 0, NULL, 0, seraph_guard_pause,
	FRAME_idle9,  NULL, 0, 0, 0, NULL, 0, seraph_guard_pause,
	FRAME_idle10, NULL, 0, 0, 0, NULL, 0, seraph_guard_pause,
	FRAME_idle11, NULL, 0, 0, 0, NULL, 0, seraph_guard_pause,
	FRAME_idle12, NULL, 0, 0, 0, NULL, 0, seraph_guard_pause,
	FRAME_idle13, NULL, 0, 0, 0, NULL, 0, seraph_guard_pause,
	FRAME_idle14, NULL, 0, 0, 0, NULL, 0, seraph_guard_pause,
	FRAME_idle15, NULL, 0, 0, 0, NULL, 0, seraph_guard_pause,
	FRAME_idle16, NULL, 0, 0, 0, NULL, 0, seraph_guard_pause,
	FRAME_idle17, NULL, 0, 0, 0, NULL, 0, seraph_guard_pause,
	FRAME_idle18, NULL, 0, 0, 0, NULL, 0, seraph_guard_pause,
	FRAME_idle19, NULL, 0, 0, 0, NULL, 0, seraph_guard_pause,
	FRAME_idle20, NULL, 0, 0, 0, NULL, 0, seraph_guard_pause,
	FRAME_idle21, NULL, 0, 0, 0, NULL, 0, seraph_guard_pause,
	FRAME_idle22, NULL, 0, 0, 0, NULL, 0, seraph_guard_pause,
	FRAME_idle23, NULL, 0, 0, 0, NULL, 0, seraph_guard_pause,
	FRAME_idle24, NULL, 0, 0, 0, NULL, 0, seraph_guard_pause,
	FRAME_idle25, NULL, 0, 0, 0, NULL, 0, seraph_guard_pause,
	FRAME_idle26, NULL, 0, 0, 0, NULL, 0, seraph_guard_pause,
	FRAME_idle27, NULL, 0, 0, 0, NULL, 0, seraph_guard_pause,
	FRAME_idle28, NULL, 0, 0, 0, NULL, 0, seraph_guard_pause,
	FRAME_idle29, NULL, 0, 0, 0, NULL, 0, seraph_guard_pause,
};
animmove_t seraph_guard_move_delay = {29, seraph_guard_frames_delay, seraph_guard_pause};
