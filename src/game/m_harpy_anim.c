#include "m_harpy_anim.h"
#include "g_local.h"
#include "m_harpy.h"


/*----------------------------------------------------
harpy dying 1
----------------------------------------------------*/
animframe_t harpy_frames_die1[] =
{
	FRAME_death1, NULL, 0, 0, 0, NULL, 0, harpy_fix_angles,
	FRAME_death2, NULL, 0, 0, 0, NULL, 0, harpy_fix_angles,
	FRAME_death3, NULL, 0, 0, 0, NULL, 0, harpy_fix_angles,
	FRAME_death4, NULL, 0, 0, 0, NULL, 0, harpy_fix_angles,
	FRAME_death5, NULL, 0, 0, 0, NULL, 0, harpy_fix_angles,
	FRAME_death6, NULL, 0, 0, 0, NULL, 0, harpy_fix_angles,
	FRAME_death7, NULL, 0, 0, 0, NULL, 0, harpy_fix_angles,
	FRAME_death8, NULL, 0, 0, 0, NULL, 0, harpy_fix_angles,
	FRAME_death9, NULL, 0, 0, 0, NULL, 0, harpy_fix_angles,
	FRAME_death10, NULL, 0, 0, 0, NULL, 0, harpy_fix_angles,
	FRAME_death11, NULL, 0, 0, 0, NULL, 0, harpy_fix_angles,
	FRAME_death12, NULL, 0, 0, 0, NULL, 0, harpy_fix_angles,
	FRAME_death13, NULL, 0, 0, 0, NULL, 0, harpy_fix_angles,
	FRAME_death14, NULL, 0, 0, 0, NULL, 0, harpy_fix_angles,
	FRAME_death15, NULL, 0, 0, 0, NULL, 0, harpy_fix_angles
};
animmove_t harpy_move_die1 = {15,harpy_frames_die1, harpy_dead};

/*----------------------------------------------------
harpy flying 1
----------------------------------------------------*/
animframe_t harpy_frames_fly1[] =
{
	FRAME_fly1, harpy_ai_fly, 64, 0, 0, NULL, 15, harpy_flap_noise,
	FRAME_fly2, harpy_ai_fly, 64, 0, 0, NULL, 15, move_harpy_fly,
	FRAME_fly3, harpy_ai_fly, 72, 0, 0, NULL, 15, move_harpy_fly,
	FRAME_fly4, harpy_ai_fly, 128, 0, 0, NULL, 15, move_harpy_fly,
	FRAME_fly5, harpy_ai_fly, 114, 0, 0, NULL, 15, move_harpy_fly,
	FRAME_fly6, harpy_ai_fly, 108, 0, 0, NULL, 15, move_harpy_fly,
	FRAME_fly7, harpy_ai_fly, 84, 0, 0, NULL, 15, move_harpy_fly,
	FRAME_fly8, harpy_ai_fly, 72, 0, 0, NULL, 15, move_harpy_fly,
	FRAME_fly9, harpy_ai_fly, 64, 0, 0, NULL, 15, move_harpy_fly,
	FRAME_fly10, harpy_ai_fly, 52, 0, 0, NULL, 15, move_harpy_fly,
	FRAME_fly11, harpy_ai_fly, 48, 0, 0, NULL, 15, move_harpy_fly,
	FRAME_fly12, harpy_ai_fly, 52, 0, 0, NULL, 15, move_harpy_fly
};
animmove_t harpy_move_fly1 = {12, harpy_frames_fly1, harpy_pause};

/*------------------------------------------------------
harpy flying backwards 1
------------------------------------------------------*/
animframe_t harpy_frames_flyback1[] =
{
	FRAME_flyback1, harpy_ai_fly, -32, 0, 16, NULL, 0, harpy_flap_fast_noise, 
	FRAME_flyback2, harpy_ai_fly, -64, 0, 64, NULL, 0, move_harpy_fly, 
	FRAME_flyback3, harpy_ai_fly, -52, 0, 54, NULL, 0, move_harpy_fly, 
	FRAME_flyback4, harpy_ai_fly, -48, 0, 48, NULL, 0, move_harpy_fly, 
	FRAME_flyback5, harpy_ai_fly, -42, 0, 32, NULL, 0, move_harpy_fly, 
	FRAME_flyback6, harpy_ai_fly, -36, 0, 24, NULL, 0, move_harpy_fly
};
animmove_t harpy_move_flyback1 = { 6, harpy_frames_flyback1, harpy_pause};


/*-------------------------------------------------------
harpy hovering
-------------------------------------------------------*/
animframe_t harpy_frames_hover1[] =
{
	FRAME_hover1, NULL, 0, 0, 0, harpy_ai_hover, 2, harpy_flap_noise,
	FRAME_hover2, NULL, 0, 0, 0, harpy_ai_hover, 1, NULL,
	FRAME_hover3, NULL, 0, 0, 0, harpy_ai_hover, -1, NULL,
	FRAME_hover4, NULL, 0, 0, 0, harpy_ai_hover, -2, NULL,
	FRAME_hover5, NULL, 0, 0, 0, harpy_ai_hover, -2, harpy_check_dodge,
	FRAME_hover6, NULL, 0, 0, 0, harpy_ai_hover, -1, NULL,
	FRAME_hover7, NULL, 0, 0, 0, harpy_ai_hover, 1, NULL,
	FRAME_hover8, NULL, 0, 0, 0, harpy_ai_hover, 2, NULL
};
animmove_t harpy_move_hover1 = {8, harpy_frames_hover1, move_harpy_hover};

animframe_t harpy_frames_tumble[] =
{
	FRAME_hover1, NULL, 0, 0, 0, NULL, 0, move_harpy_tumble,
	FRAME_hover2, NULL, 0, 0, 0, NULL, 0, move_harpy_tumble,
	FRAME_hover3, NULL, 0, 0, 0, NULL, 0, move_harpy_tumble,
	FRAME_hover4, NULL, 0, 0, 0, NULL, 0, move_harpy_tumble,
	FRAME_hover5, NULL, 0, 0, 0, NULL, 0, move_harpy_tumble,
	FRAME_hover6, NULL, 0, 0, 0, NULL, 0, move_harpy_tumble,
	FRAME_hover7, NULL, 0, 0, 0, NULL, 0, move_harpy_tumble,
	FRAME_hover8, NULL, 0, 0, 0, NULL, 0, move_harpy_tumble
};
animmove_t harpy_move_tumble = {8, harpy_frames_tumble, NULL};

/*-------------------------------------------------------
harpy hovering and screaming
-------------------------------------------------------*/
animframe_t harpy_frames_hoverscream[] = 
{
	FRAME_hoverscream1, NULL, 0, 0, 0, harpy_ai_hover, 2, harpy_flap_noise,
	FRAME_hoverscream2, NULL, 0, 0, 0, harpy_ai_hover, 1, NULL,
	FRAME_hoverscream3, NULL, 0, 0, 0, harpy_ai_hover, -1, NULL,
	FRAME_hoverscream4, NULL, 0, 0, 0, harpy_ai_hover, -2, NULL,
	FRAME_hoverscream5, NULL, 0, 0, 0, harpy_ai_hover, -2, NULL,
	FRAME_hoverscream6, NULL, 0, 0, 0, harpy_ai_hover, -1, NULL,
	FRAME_hoverscream7, NULL, 0, 0, 0, harpy_ai_hover, 1, NULL,
	FRAME_hoverscream8, NULL, 0, 0, 0, harpy_ai_hover, 2, NULL,
	FRAME_hoverscream9, NULL, 0, 0, 0, harpy_ai_hover, 2, NULL,
	FRAME_hoverscream10,NULL, 0, 0, 0, harpy_ai_hover, 1, NULL,
	FRAME_hoverscream11,NULL, 0, 0, 0, harpy_ai_hover, -1, NULL,
	FRAME_hoverscream12,NULL, 0, 0, 0, harpy_ai_hover, -2, NULL,
	FRAME_hoverscream13,NULL, 0, 0, 0, harpy_ai_hover, -2, NULL,
	FRAME_hoverscream14,NULL, 0, 0, 0, harpy_ai_hover, -1, NULL,
	FRAME_hoverscream15,NULL, 0, 0, 0, harpy_ai_hover, 1, NULL,
	FRAME_hoverscream16,NULL, 0, 0, 0, harpy_ai_hover, 2, NULL
};
animmove_t harpy_move_hoverscream = {16, harpy_frames_hoverscream, move_harpy_hover};


/*-------------------------------------------------------
harpy diving and attacking
-------------------------------------------------------*/
animframe_t harpy_frames_dive_go[] = 
{
	FRAME_dive01, NULL, 0, 0, 0, NULL, 0, harpy_dive_noise,
	FRAME_dive02, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_dive03, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_dive04, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_dive05, NULL, 0, 0, 0, NULL, 0, move_harpy_dive,
	FRAME_dive06, NULL, 0, 0, 0, NULL, 0, move_harpy_dive,
}; 
animmove_t harpy_move_dive_go = {6, harpy_frames_dive_go, harpy_dive_loop};

animframe_t harpy_frames_dive_loop[] = 
{
	FRAME_dive07, NULL, 0, 0, 0, NULL, 0, move_harpy_dive,
	FRAME_dive08, NULL, 0, 0, 0, NULL, 0, move_harpy_dive,
	FRAME_dive09, NULL, 0, 0, 0, NULL, 0, move_harpy_dive,
	FRAME_dive10, NULL, 0, 0, 0, NULL, 0, move_harpy_dive,
	FRAME_dive11, NULL, 0, 0, 0, NULL, 0, move_harpy_dive,
	FRAME_dive12, NULL, 0, 0, 0, NULL, 0, move_harpy_dive,
};
animmove_t harpy_move_dive_loop = {6, harpy_frames_dive_loop, NULL};

animframe_t harpy_frames_dive_trans[] = 
{
	FRAME_dive13, NULL, 0, 0, 0, NULL, 0, move_harpy_dive_end,
	FRAME_dive14, NULL, 0, 0, 0, NULL, 0, move_harpy_dive_end,
	FRAME_dive15, NULL, 0, 0, 0, NULL, 0, move_harpy_dive_end,
	FRAME_dive16, NULL, 0, 0, 0, NULL, 0, move_harpy_dive_end,
	FRAME_dive17, NULL, 0, 0, 0, NULL, 0, move_harpy_dive_end,
};
animmove_t harpy_move_dive_trans = {5, harpy_frames_dive_trans, harpy_hit_loop};

animframe_t harpy_frames_dive_hit_loop[] = 
{
	FRAME_dive18, NULL, 0, 0, 0, NULL, 0, move_harpy_dive_end,
};
animmove_t harpy_move_dive_hit_loop = {1, harpy_frames_dive_hit_loop, NULL};

animframe_t harpy_frames_dive_end[] = 
{
	FRAME_dive19, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_dive20, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_dive21, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_dive22, NULL, 0, 0, 0, NULL, 0, harpy_hit,
	FRAME_dive23, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_dive24, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_dive25, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_dive26, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_dive27, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_dive28, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_dive29, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_dive30, NULL, 0, 0, 0, NULL, 0, NULL
};
animmove_t harpy_move_dive_end = {11, harpy_frames_dive_end, harpy_flyback};

/*-------------------------------------------------------
harpy attacking up close
-------------------------------------------------------*/
animframe_t harpy_frames_closeattack[] = 
{
    FRAME_dive17, NULL, 0, 0, 0, NULL, 6, harpy_hit,
//	FRAME_dive18, NULL, 0, 0, 0, NULL, 6, NULL,
	FRAME_dive19, NULL, 0, 0, 0, NULL, 6, NULL,
//	FRAME_dive20, NULL, 0, 0, 0, NULL, 6, NULL,
	FRAME_dive21, NULL, 0, 0, 0, NULL, 6, NULL,
//	FRAME_dive22, NULL, 0, 0, 0, NULL, 6, NULL,
	FRAME_dive23, NULL, 0, 0, 0, NULL, 6, NULL,
//	FRAME_dive24, NULL, 0, 0, 0, NULL, 6, NULL,
	FRAME_dive25, NULL, 0, 0, 0, NULL, 6, NULL,
//	FRAME_dive26, NULL, 0, 0, 0, NULL, 6, NULL,
	FRAME_dive27, NULL, 0, 0, 0, NULL, 6, NULL,
//	FRAME_dive28, NULL, 0, 0, 0, NULL, 6, NULL
};
animmove_t harpy_closeattack = {6, harpy_frames_closeattack, harpy_pause};


/*-------------------------------------------------------
harpy pain
-------------------------------------------------------*/
animframe_t harpy_frames_pain1 [] =
{
	FRAME_pain1, NULL, 0, 0, 0, NULL, 2, move_harpy_hover,
	FRAME_pain2, NULL, 0, 0, 0, NULL, 1, move_harpy_hover,
	FRAME_pain3, NULL, 0, 0, 0, NULL, -1, move_harpy_hover,
	FRAME_pain4, NULL, 0, 0, 0, NULL, -2, move_harpy_hover,
	FRAME_pain5, NULL, 0, 0, 0, NULL, -2, move_harpy_hover,
	FRAME_pain6, NULL, 0, 0, 0, NULL, -1, move_harpy_hover,
	FRAME_pain7, NULL, 0, 0, 0, NULL, 1, move_harpy_hover,
	FRAME_pain8, NULL, 0, 0, 0, NULL, 2, move_harpy_hover
};
animmove_t harpy_move_pain1 = { 8, harpy_frames_pain1, harpy_pause};

animframe_t harpy_frames_glide [] =
{
	FRAME_glide1, harpy_ai_glide, 64, 0, 0, NULL, 0, NULL,
	FRAME_glide2, harpy_ai_glide, 64, 0, 0, NULL, 0, NULL,
	FRAME_glide3, harpy_ai_glide, 64, 0, 0, NULL, 0, NULL,
	FRAME_glide4, harpy_ai_glide, 64, 0, 0, NULL, 0, NULL,
	FRAME_glide5, harpy_ai_glide, 64, 0, 0, NULL, 0, NULL,
	FRAME_glide6, harpy_ai_glide, 64, 0, 0, NULL, 0, NULL,
	FRAME_glide7, harpy_ai_glide, 64, 0, 0, NULL, 0, NULL,
	FRAME_glide8, harpy_ai_glide, 64, 0, 0, NULL, 0, NULL,
	FRAME_glide9, harpy_ai_glide, 64, 0, 0, NULL, 0, NULL,
	FRAME_glide10, harpy_ai_glide, 64, 0, 0, NULL, 0, NULL,
	FRAME_glide11, harpy_ai_glide, 64, 0, 0, NULL, 0, NULL,
	FRAME_glide12, harpy_ai_glide, 64, 0, 0, NULL, 0, NULL,
};
animmove_t harpy_move_glide = { 12, harpy_frames_glide, NULL};

//pirch 1
animframe_t harpy_frames_pirch1_idle [] =
{
	FRAME_pirch1, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch2, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch3, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch4, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch5, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch6, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch7, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch8, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch9, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch10, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
};
animmove_t harpy_move_pirch1_idle = { 10, harpy_frames_pirch1_idle, NULL};

//pirch 2
animframe_t harpy_frames_pirch2_idle [] =
{
	FRAME_pirch11, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch12, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch13, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch14, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch15, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch16, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch17, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch18, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch19, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch20, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
};
animmove_t harpy_move_pirch2_idle = { 10, harpy_frames_pirch2_idle, NULL};

//pirch 3
animframe_t harpy_frames_pirch3_idle [] =
{
	FRAME_pirch21, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch22, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch23, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch24, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch25, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch26, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch27, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch28, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch29, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch30, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
};
animmove_t harpy_move_pirch3_idle = { 10, harpy_frames_pirch3_idle, NULL};

//pirch 4
animframe_t harpy_frames_pirch4_idle [] =
{
	FRAME_pirch31, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch32, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch33, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch34, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch35, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch36, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch37, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch38, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch39, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch40, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
};
animmove_t harpy_move_pirch4_idle = { 10, harpy_frames_pirch4_idle, NULL};

//pirch 5
animframe_t harpy_frames_pirch5_idle [] =
{
	FRAME_pirch41, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch42, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch43, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch44, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch45, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch46, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch47, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch48, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch49, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch50, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
};
animmove_t harpy_move_pirch5_idle = { 10, harpy_frames_pirch5_idle, NULL};

//pirch 6
animframe_t harpy_frames_pirch6_idle [] =
{
	FRAME_pirch51, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch52, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch53, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch54, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch55, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch56, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch57, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch58, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch59, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch60, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
};
animmove_t harpy_move_pirch6_idle = { 10, harpy_frames_pirch6_idle, NULL};

//pirch 7
animframe_t harpy_frames_pirch7_idle [] =
{
	FRAME_pirch61, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch62, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch63, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch64, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch65, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch66, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch67, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch68, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch69, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch70, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
};
animmove_t harpy_move_pirch7_idle = { 10, harpy_frames_pirch7_idle, NULL};

//pirch 8
animframe_t harpy_frames_pirch8_idle [] =
{
	FRAME_pirch71, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch72, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch73, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch74, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch75, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch76, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch77, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch78, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch79, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch80, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
};
animmove_t harpy_move_pirch8_idle = { 10, harpy_frames_pirch8_idle, NULL};

//pirch 9
animframe_t harpy_frames_pirch9_idle [] =
{
	FRAME_pirch81, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch82, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch83, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch84, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch85, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch86, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch87, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch88, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch89, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
	FRAME_pirch90, NULL, 0, 0, 0, NULL, 0, harpy_ai_pirch,
};
animmove_t harpy_move_pirch9_idle = { 10, harpy_frames_pirch9_idle, NULL};

animframe_t	harpy_frames_takeoff [] = 
{
	FRAME_takeoff1, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_takeoff3, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_takeoff5, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_takeoff7, harpy_ai_fly, -32, 0, 16, NULL, 0, harpy_flap_fast_noise,
	FRAME_takeoff9, harpy_ai_fly, -48, 0, 32, NULL, 0, NULL,
	FRAME_takeoff11, harpy_ai_fly, -32, 0, 32, NULL, 0, NULL,
	FRAME_takeoff13, harpy_ai_fly, -64, 0, 32, NULL, 0, NULL,
	FRAME_takeoff15, harpy_ai_fly, -32, 0, 32, NULL, 0, NULL,
};
animmove_t harpy_move_takeoff = { 8, harpy_frames_takeoff, harpy_pause};

animframe_t	harpy_frames_circle [] = 
{
	FRAME_glide1, harpy_ai_circle, 32, 0, 0, NULL, 0, NULL,
	FRAME_glide2, harpy_ai_circle, 32, 0, 0, NULL, 0, NULL,
	FRAME_glide3, harpy_ai_circle, 32, 0, 0, NULL, 0, NULL,
	FRAME_glide4, harpy_ai_circle, 32, 0, 0, NULL, 0, NULL,
	FRAME_glide5, harpy_ai_circle, 32, 0, 0, NULL, 0, NULL,
	FRAME_glide6, harpy_ai_circle, 32, 0, 0, NULL, 0, NULL,
	FRAME_glide7, harpy_ai_circle, 32, 0, 0, NULL, 0, NULL,
	FRAME_glide8, harpy_ai_circle, 32, 0, 0, NULL, 0, NULL,
	FRAME_glide9, harpy_ai_circle, 32, 0, 0, NULL, 0, NULL,
	FRAME_glide10, harpy_ai_circle, 32, 0, 0, NULL, 0, NULL,
	FRAME_glide11, harpy_ai_circle, 32, 0, 0, NULL, 0, NULL,
	FRAME_glide12, harpy_ai_circle, 32, 0, 0, NULL, 0, NULL,
};
animmove_t harpy_move_circle = { 12, harpy_frames_circle, harpy_pause};

animframe_t	harpy_frames_circle_flap [] = 
{
	FRAME_fly1, harpy_ai_circle, 32, 0, 0, NULL, 0, harpy_flap_noise,
	FRAME_fly2, harpy_ai_circle, 48, 0, 0, NULL, 0, NULL,
	FRAME_fly3, harpy_ai_circle, 64, 0, 0, NULL, 0, NULL,
	FRAME_fly4, harpy_ai_circle, 64, 0, 0, NULL, 0, NULL,
	FRAME_fly5, harpy_ai_circle, 56, 0, 0, NULL, 0, NULL,
	FRAME_fly6, harpy_ai_circle, 56, 0, 0, NULL, 0, NULL,
	FRAME_fly7, harpy_ai_circle, 50, 0, 0, NULL, 0, NULL,
	FRAME_fly8, harpy_ai_circle, 50, 0, 0, NULL, 0, NULL,
	FRAME_fly9, harpy_ai_circle, 44, 0, 0, NULL, 0, NULL,
	FRAME_fly10, harpy_ai_circle, 40, 0, 0, NULL, 0, NULL,
	FRAME_fly11, harpy_ai_circle, 36, 0, 0, NULL, 0, NULL,
	FRAME_fly12, harpy_ai_circle, 32, 0, 0, NULL, 0, NULL,
};
animmove_t harpy_move_circle_flap = { 12, harpy_frames_circle_flap, harpy_pause};
