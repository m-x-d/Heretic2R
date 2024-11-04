#include "m_ogle.h"
#include "g_local.h"
#include "m_ogle_anim.h"

#include "g_monster.h"
#include "c_ai.h"

/*----------------------------------------------------
	ogle standing - temp
----------------------------------------------------*/
animframe_t ogle_frames_stand1[] =
{
	FRAME_walk1, NULL, 0, 0, 0, ai_stand, 0, NULL,
};
animmove_t ogle_move_stand1 = {1,ogle_frames_stand1, ogle_pause};

/*----------------------------------------------------
	ogle walking
----------------------------------------------------*/
animframe_t ogle_frames_walk1[] =
{
	FRAME_walk1, NULL, 0, 0, 0, ai_run, 2, NULL,
	FRAME_walk2, NULL, 0, 0, 0, ai_run, 2, NULL,
	FRAME_walk3, NULL, 0, 0, 0, ai_run, 2, NULL,
	FRAME_walk4, NULL, 0, 0, 0, ai_run, 2, NULL,
	FRAME_walk5, NULL, 0, 0, 0, ai_run, 2, NULL,
	FRAME_walk6, NULL, 0, 0, 0, ai_run, 2, NULL,
	FRAME_walk7, NULL, 0, 0, 0, ai_run, 2, NULL,
	FRAME_walk8, NULL, 0, 0, 0, ai_run, 2, NULL,
};
animmove_t ogle_move_walk1 = {8,ogle_frames_walk1, ogle_pause};

/*----------------------------------------------------
	ogle pushing anims

	push1	- pushing a cart, right hand higher than left
	push2	- pushing a cart, both hands equal
	push3	- pushing a cart, left hand higher than right
----------------------------------------------------*/

animframe_t ogle_frames_push1[] =
{
	FRAME_pusha1, NULL, 0, 0, 0, ogle_push, 4, NULL,
	FRAME_pusha2, NULL, 0, 0, 0, ogle_push, 4, NULL,
	FRAME_pusha3, NULL, 0, 0, 0, ogle_push, 4, NULL,
	FRAME_pusha4, NULL, 0, 0, 0, ogle_push, 4, NULL,
	FRAME_pusha5, NULL, 0, 0, 0, ogle_push, 4, NULL,
	FRAME_pusha6, NULL, 0, 0, 0, ogle_push, 4, NULL,
	FRAME_pusha7, NULL, 0, 0, 0, ogle_push, 4, NULL,
	FRAME_pusha8, NULL, 0, 0, 0, ogle_push, 4, NULL,
};
animmove_t ogle_move_push1 = {8,ogle_frames_push1, NULL};

animframe_t ogle_frames_push2[] =
{
	FRAME_pushb1, NULL, 0, 0, 0, ogle_push, 4, NULL,
	FRAME_pushb2, NULL, 0, 0, 0, ogle_push, 4, NULL,
	FRAME_pushb3, NULL, 0, 0, 0, ogle_push, 4, NULL,
	FRAME_pushb4, NULL, 0, 0, 0, ogle_push, 4, NULL,
	FRAME_pushb5, NULL, 0, 0, 0, ogle_push, 4, NULL,
	FRAME_pushb6, NULL, 0, 0, 0, ogle_push, 4, NULL,
	FRAME_pushb7, NULL, 0, 0, 0, ogle_push, 4, NULL,
	FRAME_pushb8, NULL, 0, 0, 0, ogle_push, 4, NULL,
};
animmove_t ogle_move_push2 = {8,ogle_frames_push2, NULL};

animframe_t ogle_frames_push3[] =
{
	FRAME_pushc1, NULL, 0, 0, 0, ogle_push, 4, NULL,
	FRAME_pushc2, NULL, 0, 0, 0, ogle_push, 4, NULL,
	FRAME_pushc3, NULL, 0, 0, 0, ogle_push, 4, NULL,
	FRAME_pushc4, NULL, 0, 0, 0, ogle_push, 4, NULL,
	FRAME_pushc5, NULL, 0, 0, 0, ogle_push, 4, NULL,
	FRAME_pushc6, NULL, 0, 0, 0, ogle_push, 4, NULL,
	FRAME_pushc7, NULL, 0, 0, 0, ogle_push, 4, NULL,
	FRAME_pushc8, NULL, 0, 0, 0, ogle_push, 4, NULL,
};
animmove_t ogle_move_push3 = {8,ogle_frames_push3, NULL};

/*----------------------------------------------------
	ogle working anims

	work1	- hammering with chisel quickly, straight
	work2	- hammering with chisel slowly, straight
	work3	- hammering with chisel, downward
	work4	- picking with axe, straight
	work5	- picking with axe, downward
----------------------------------------------------*/

animframe_t ogle_frames_work1[] =
{
	FRAME_hamupa2, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_hamupa3, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_hamupa4, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_hamupa5, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_hamupa1, NULL, 0, 0, 0, NULL, 0, ogle_pick_dust,
};
animmove_t ogle_move_work1 = {5,ogle_frames_work1, ogle_pause};

animframe_t ogle_frames_work2[] =
{
	FRAME_hamupb2, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_hamupb3, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_hamupb4, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_hamupb5, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_hamupb6, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_hamupb7, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_hamupb8, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_hamupb9, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_hamupb1, NULL, 0, 0, 0, NULL, 0, ogle_pick_dust,
};
animmove_t ogle_move_work2 = {9,ogle_frames_work2, ogle_pause};

animframe_t ogle_frames_work3[] =
{
	FRAME_hamdwn2, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_hamdwn3, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_hamdwn4, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_hamdwn5, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_hamdwn6, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_hamdwn7, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_hamdwn8, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_hamdwn9, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_hamdwn1, NULL, 0, 0, 0, NULL, 0, ogle_pick_dust,
};
animmove_t ogle_move_work3 = {9,ogle_frames_work3, ogle_pause};

animframe_t ogle_frames_work4[] =
{
	FRAME_pikxup2, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_pikxup3, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_pikxup4, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_pikxup5, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_pikxup6, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_pikxup7, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_pikxup1, NULL, 0, 0, 0, NULL, 0, ogle_pick_dust,
};
animmove_t ogle_move_work4 = {7,ogle_frames_work4, ogle_pause};

animframe_t ogle_frames_work5[] =
{
	FRAME_pikxdn2, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_pikxdn3, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_pikxdn4, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_pikxdn5, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_pikxdn6, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_pikxdn7, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_pikxdn1, NULL, 0, 0, 0, NULL, 0, ogle_pick_dust,
};
animmove_t ogle_move_work5 = {7,ogle_frames_work5, ogle_pause};

/*----------------------------------------------------
	ogle pain anims

	pain1	- guarding head
	pain2	- transition from rest1 back into work4
	pain3	- transition from rest4 back into work4
----------------------------------------------------*/

animframe_t ogle_frames_pain1[] =
{
	FRAME_paina1, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_paina2, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_paina2, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_paina2, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_paina2, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_paina2, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_paina3, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_paina4, NULL, 0, 0, 0, NULL, 0, NULL,
};
animmove_t ogle_move_pain1 = {8, ogle_frames_pain1, ogle_pause};

animframe_t ogle_frames_pain2[] =
{
	FRAME_rstapn1, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_rstapn2, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_rstapn2, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_rstapn2, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_rstapn2, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_rstapn2, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_rstapn3, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_rstapn4, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_rstapn5, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_rstapn6, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_rstapn7, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_rstapn8, NULL, 0, 0, 0, NULL, 0, NULL,
};
animmove_t ogle_move_pain2 = {12, ogle_frames_pain2, ogle_pause};

animframe_t ogle_frames_pain3[] =
{
	FRAME_brkpn1, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_brkpn2, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_brkpn3, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_brkpn4, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_brkpn5, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_brkpn6, NULL, 0, 0, 0, NULL, 0, NULL,
};
animmove_t ogle_move_pain3 = {6,ogle_frames_pain3, ogle_pause};

/*----------------------------------------------------
	ogle rest anims

	rest1_trans		- transition from work4 to leaning back on pick 
	rest1			- leaning back on pick, breating
	rest1_wipe		- wipes brow from picking, returns to rest
	rest2_wipe		- wipes brow from picking, returns to work
	rest3_wipe		- wipes brow from chisel, returns to work
	rest4_trans		- transition from work4 to resting pick on shoulder
	rest4_trans2	- transition from chisel to resting pick on shoulder
	rest4			- resting pick on shoulder, breathing
----------------------------------------------------*/

animframe_t ogle_frames_rest1_trans[] =
{
	FRAME_rsta1, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_rsta2, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_rsta3, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_rsta4, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_rsta5, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_rsta6, NULL, 0, 0, 0, NULL, 0, NULL,
};
animmove_t ogle_move_rest1_trans = {6,ogle_frames_rest1_trans, ogle_rest};

animframe_t ogle_frames_rest1[] =
{
	FRAME_rstaid1, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_rstaid2, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_rstaid3, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_rstaid4, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_rstaid5, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_rstaid6, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_rstaid7, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_rstaid8, NULL, 0, 0, 0, NULL, 0, NULL,
};
animmove_t ogle_move_rest1 = {8,ogle_frames_rest1, ogle_rest};

animframe_t ogle_frames_rest1_wipe[] =
{
	FRAME_rstawp1, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_rstawp2, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_rstawp3, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_rstawp4, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_rstawp5, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_rstawp6, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_rstawp7, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_rstawp8, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_rstawp9, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_rstawp10,NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_rstawp11,NULL, 0, 0, 0, NULL, 0, NULL,
};
animmove_t ogle_move_rest1_wipe = {11,ogle_frames_rest1_wipe, ogle_rest};

animframe_t ogle_frames_rest2_wipe[] =
{
	FRAME_pkaxwp1, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_pkaxwp2, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_pkaxwp3, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_pkaxwp4, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_pkaxwp5, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_pkaxwp6, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_pkaxwp7, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_pkaxwp8, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_pkaxwp9, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_pkaxwp10,NULL, 0, 0, 0, NULL, 0, NULL,
};
animmove_t ogle_move_rest2_wipe = {10,ogle_frames_rest2_wipe, ogle_rest};

animframe_t ogle_frames_rest3_wipe[] =
{
	FRAME_hamwp1, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_hamwp2, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_hamwp3, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_hamwp4, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_hamwp5, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_hamwp6, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_hamwp7, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_hamwp8, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_hamwp9, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_hamwp10,NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_hamwp11,NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_hamwp12,NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_hamwp13,NULL, 0, 0, 0, NULL, 0, NULL,
};
animmove_t ogle_move_rest3_wipe = {13,ogle_frames_rest3_wipe, ogle_rest};

animframe_t ogle_frames_rest4_trans[] =
{
	FRAME_brka1, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_brka2, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_brka3, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_brka4, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_brka5, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_brka6, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_brka7, NULL, 0, 0, 0, NULL, 0, NULL,
};
animmove_t ogle_move_rest4_trans = {7,ogle_frames_rest4_trans, ogle_rest};

animframe_t ogle_frames_rest4_trans2[] =
{
	FRAME_brkb1, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_brkb2, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_brkb3, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_brkb4, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_brkb5, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_brkb6, NULL, 0, 0, 0, NULL, 0, NULL,
};
animmove_t ogle_move_rest4_trans2 = {6,ogle_frames_rest4_trans2, ogle_rest};

animframe_t ogle_frames_rest4[] =
{
	FRAME_brkidl1, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_brkidl2, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_brkidl3, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_brkidl4, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_brkidl5, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_brkidl6, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_brkidl7, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_brkidl8, NULL, 0, 0, 0, NULL, 0, NULL,
};
animmove_t ogle_move_rest4 = {8, ogle_frames_rest4, ogle_rest};

/*----------------------------------------------------
	ogle celebration anims

	celebrate1	- hands in the air, back field in motion
	celebrate2	- hands in the air, jumping up and down
	celebrate3_trans - transition from celebrate2
	celebrate3	- the infamous Butthead manuever
	celebrate4_trans - transition from celebrate2
	celebrate4  - more buttheadisms
	celebrate5_trans - transition from celebrate2
	celebrate5  - butt wigglin'
----------------------------------------------------*/

animframe_t ogle_frames_celebrate1[] =
{
	FRAME_cela1, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_cela2, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_cela3, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_cela4, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_cela5, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_cela6, NULL, 0, 0, 0, NULL, 0, NULL,
};
animmove_t ogle_move_celebrate1 = {6, ogle_frames_celebrate1, ogle_celebrate};

animframe_t ogle_frames_celebrate2[] =
{
	FRAME_celb1,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_celb2,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_celb3,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_celb4,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_celb5,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_celb6,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_celb7,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_celb8,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_celb9,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_celb10, NULL, 0, 0, 0, NULL, 0, NULL,
};
animmove_t ogle_move_celebrate2 = {10, ogle_frames_celebrate2, ogle_celebrate};

animframe_t ogle_frames_celebrate3_trans[] =
{
	FRAME_celc1,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_celc2,  NULL, 0, 0, 0, NULL, 0, NULL,
};
animmove_t ogle_move_celebrate3_trans = {2, ogle_frames_celebrate3_trans, ogle_celebrate};

animframe_t ogle_frames_celebrate3[] =
{
	FRAME_celc3,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_celc4,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_celc5,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_celc6,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_celc7,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_celc8,  NULL, 0, 0, 0, NULL, 0, NULL,
};
animmove_t ogle_move_celebrate3 = {6, ogle_frames_celebrate3, ogle_celebrate};

animframe_t ogle_frames_celebrate4_trans[] =
{
	FRAME_celd1,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_celd2,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_celd3,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_celd4,  NULL, 0, 0, 0, NULL, 0, NULL,
};
animmove_t ogle_move_celebrate4_trans = {4, ogle_frames_celebrate4_trans, ogle_celebrate};

animframe_t ogle_frames_celebrate4[] =
{
	FRAME_celd5,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_celd6,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_celd7,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_celd8,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_celd9,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_celd10, NULL, 0, 0, 0, NULL, 0, NULL,
};
animmove_t ogle_move_celebrate4 = {6, ogle_frames_celebrate4, ogle_celebrate};

animframe_t ogle_frames_celebrate5_trans[] =
{
	FRAME_cele1,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_cele2,  NULL, 0, 0, 0, NULL, 0, NULL,
};
animmove_t ogle_move_celebrate5_trans = {2, ogle_frames_celebrate5_trans, ogle_celebrate};

animframe_t ogle_frames_celebrate5[] =
{
	FRAME_cele5,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_cele6,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_cele7,  NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_cele8,  NULL, 0, 0, 0, NULL, 0, NULL,
};
animmove_t ogle_move_celebrate5 = {4, ogle_frames_celebrate5, ogle_celebrate};

/*----------------------------------------------------
	ogle charge anims

	charge1	-	arm raised and ready for action
	charge2 -	both arms are down
	charge3 -	both arms in arm, raising them up and down
	charge4 -	both arms in arm, moving them in unison left and right
	charge5 -	both arms in arm, moving them in opposingly left and right
----------------------------------------------------*/

animframe_t ogle_frames_charge1[] =
{
	FRAME_charga1,  NULL, 0, 0, 0, ai_run, 10, NULL,
	FRAME_charga2,  NULL, 0, 0, 0, ai_run, 9,  NULL,
	FRAME_charga3,  NULL, 0, 0, 0, ai_run, 8,  NULL,
	FRAME_charga4,  NULL, 0, 0, 0, ai_run, 9,  NULL,
	FRAME_charga5,  NULL, 0, 0, 0, ai_run, 10, NULL,
	FRAME_charga6,  NULL, 0, 0, 0, ai_run, 9,  NULL,
	FRAME_charga7,  NULL, 0, 0, 0, ai_run, 8,  NULL,
	FRAME_charga8,  NULL, 0, 0, 0, ai_run, 10, NULL,
};
animmove_t ogle_move_charge1 = {8, ogle_frames_charge1, ogle_pause};

animframe_t ogle_frames_charge2[] =
{
	FRAME_chargb1,  NULL, 0, 0, 0, ai_run, 10, NULL,
	FRAME_chargb2,  NULL, 0, 0, 0, ai_run, 9,  NULL,
	FRAME_chargb3,  NULL, 0, 0, 0, ai_run, 8,  NULL,
	FRAME_chargb4,  NULL, 0, 0, 0, ai_run, 9,  NULL,
	FRAME_chargb5,  NULL, 0, 0, 0, ai_run, 10, NULL,
	FRAME_chargb6,  NULL, 0, 0, 0, ai_run, 9,  NULL,
	FRAME_chargb7,  NULL, 0, 0, 0, ai_run, 8,  NULL,
	FRAME_chargb8,  NULL, 0, 0, 0, ai_run, 10, NULL,
};
animmove_t ogle_move_charge2 = {8, ogle_frames_charge2, ogle_pause};

animframe_t ogle_frames_charge3[] =
{
	FRAME_chargc1,  NULL, 0, 0, 0, ai_run, 10, NULL,
	FRAME_chargc2,  NULL, 0, 0, 0, ai_run, 9,  NULL,
	FRAME_chargc3,  NULL, 0, 0, 0, ai_run, 8,  NULL,
	FRAME_chargc4,  NULL, 0, 0, 0, ai_run, 9,  NULL,
	FRAME_chargc5,  NULL, 0, 0, 0, ai_run, 10, NULL,
	FRAME_chargc6,  NULL, 0, 0, 0, ai_run, 9,  NULL,
	FRAME_chargc7,  NULL, 0, 0, 0, ai_run, 8,  NULL,
	FRAME_chargc8,  NULL, 0, 0, 0, ai_run, 10, NULL,
};
animmove_t ogle_move_charge3 = {8, ogle_frames_charge3, ogle_pause};

animframe_t ogle_frames_charge4[] =
{
	FRAME_chargd1,  NULL, 0, 0, 0, ai_run, 10, NULL,
	FRAME_chargd2,  NULL, 0, 0, 0, ai_run, 9,  NULL,
	FRAME_chargd3,  NULL, 0, 0, 0, ai_run, 8,  NULL,
	FRAME_chargd4,  NULL, 0, 0, 0, ai_run, 9,  NULL,
	FRAME_chargd5,  NULL, 0, 0, 0, ai_run, 10, NULL,
	FRAME_chargd6,  NULL, 0, 0, 0, ai_run, 9,  NULL,
	FRAME_chargd7,  NULL, 0, 0, 0, ai_run, 8,  NULL,
	FRAME_chargd8,  NULL, 0, 0, 0, ai_run, 10, NULL,
};
animmove_t ogle_move_charge4 = {8, ogle_frames_charge4, ogle_pause};

animframe_t ogle_frames_charge5[] =
{
	FRAME_charge1,  NULL, 0, 0, 0, ai_run,	10, NULL,
	FRAME_charge2,  NULL, 0, 0, 0, ai_run,	9,  NULL,
	FRAME_charge3,  NULL, 0, 0, 0, ai_run,	8,  NULL,
	FRAME_charge4,  NULL, 0, 0, 0, ai_run,	9,  NULL,
	FRAME_charge5,  NULL, 0, 0, 0, ai_run,	10, NULL,
	FRAME_charge6,  NULL, 0, 0, 0, ai_run,	9,  NULL,
	FRAME_charge7,  NULL, 0, 0, 0, ai_run,	8,  NULL,
	FRAME_charge8,  NULL, 0, 0, 0, ai_run,	10, NULL,
};
animmove_t ogle_move_charge5 = {8, ogle_frames_charge5, ogle_pause};

/*----------------------------------------------------
	ogle attack anims

	attack1	-	picking away at enemy
	attack2 -	running attack
	attack3 -	running attack 2
----------------------------------------------------*/

animframe_t ogle_frames_attack1[] =
{
	FRAME_pikxup2, NULL, 0, 0, 0, ai_charge, 0, NULL,
	FRAME_pikxup3, NULL, 0, 0, 0, ai_charge, 0, NULL,
	FRAME_pikxup4, NULL, 0, 0, 0, ai_charge, 0, NULL,
	FRAME_pikxup5, NULL, 0, 0, 0, ai_charge, 0, NULL,
	FRAME_pikxup6, NULL, 0, 0, 0, ai_charge, 0, NULL,
	FRAME_pikxup7, NULL, 0, 0, 0, ai_charge, 0, NULL,
	FRAME_pikxup1, NULL, 0, 0, 0, ai_charge, 0, ogle_strike,
};
animmove_t ogle_move_attack1 = {7,ogle_frames_attack1, ogle_pause};

animframe_t ogle_frames_attack2[] =
{
	FRAME_rnatka2, NULL, 0, 0, 0, ai_charge, 12, NULL,
	FRAME_rnatka3, NULL, 0, 0, 0, ai_charge, 11, NULL,
	FRAME_rnatka4, NULL, 0, 0, 0, ai_charge, 10, NULL,
	FRAME_rnatka5, NULL, 0, 0, 0, ai_charge, 12, NULL,
	FRAME_rnatka6, NULL, 0, 0, 0, ai_charge, 11, NULL,
	FRAME_rnatka7, NULL, 0, 0, 0, ai_charge, 10, NULL,
	FRAME_rnatka8, NULL, 0, 0, 0, ai_charge, 11, NULL,
	FRAME_rnatka1, NULL, 0, 0, 0, ai_charge, 12, ogle_strike,
};
animmove_t ogle_move_attack2 = {8,ogle_frames_attack2, ogle_pause};

animframe_t ogle_frames_attack3[] =
{
	FRAME_rnatkb2, NULL, 0, 0, 0, ai_charge, 12, NULL,
	FRAME_rnatkb3, NULL, 0, 0, 0, ai_charge, 11, NULL,
	FRAME_rnatkb4, NULL, 0, 0, 0, ai_charge, 10, NULL,
	FRAME_rnatkb5, NULL, 0, 0, 0, ai_charge, 12, NULL,
	FRAME_rnatkb6, NULL, 0, 0, 0, ai_charge, 11, NULL,
	FRAME_rnatkb7, NULL, 0, 0, 0, ai_charge, 10, NULL,
	FRAME_rnatkb8, NULL, 0, 0, 0, ai_charge, 11, NULL,
	FRAME_rnatkb1, NULL, 0, 0, 0, ai_charge, 12, ogle_strike,
};
animmove_t ogle_move_attack3 = {8,ogle_frames_attack3, ogle_pause};


animframe_t ogle_frames_death1[] =
{
	FRAME_deatha1, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_deatha2, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_deatha3, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_deatha4, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_deatha5, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_deatha6, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_deatha7, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_deatha8, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_deatha9, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_deatha10, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_deatha11, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_deatha12, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_deatha13, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_deatha14, NULL, 0, 0, 0, NULL, 0, NULL,
};
animmove_t ogle_move_death1 = {14,ogle_frames_death1, M_EndDeath};

animframe_t ogle_frames_death2[] =
{
	FRAME_deathb1, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_deathb2, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_deathb3, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_deathb4, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_deathb5, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_deathb6, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_deathb7, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_deathb8, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_deathb9, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_deathb10, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_deathb11, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_deathb12, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_deathb13, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_deathb14, NULL, 0, 0, 0, NULL, 0, NULL,
};
animmove_t ogle_move_death2 = {14,ogle_frames_death2, M_EndDeath};

/*----------------------------------------------------
	ogle actions
----------------------------------------------------*/
animframe_t ogle_c_frames_action1[] =
{
	FRAME_hamupa2, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_hamupa3, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_hamupa4, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_hamupa5, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_hamupa1, ai_c_move, 0, 0, 0, NULL, 0, ogle_pick_dust,
};
animmove_t ogle_c_move_action1 = {5,ogle_c_frames_action1, ai_c_cycleend};

animframe_t ogle_c_frames_action2[] =
{
	FRAME_hamupb2, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_hamupb3, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_hamupb4, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_hamupb5, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_hamupb6, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_hamupb7, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_hamupb8, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_hamupb9, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_hamupb1, ai_c_move, 0, 0, 0, NULL, 0, ogle_pick_dust,
};
animmove_t ogle_c_move_action2 = {9,ogle_c_frames_action2, ai_c_cycleend};

animframe_t ogle_c_frames_action3[] =
{
	FRAME_hamdwn2, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_hamdwn3, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_hamdwn4, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_hamdwn5, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_hamdwn6, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_hamdwn7, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_hamdwn8, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_hamdwn9, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_hamdwn1, ai_c_move, 0, 0, 0, NULL, 0, ogle_pick_dust,
};
animmove_t ogle_c_move_action3 = {9,ogle_c_frames_action3, ai_c_cycleend};

animframe_t ogle_c_frames_action4[] =
{
	FRAME_pikxup2, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_pikxup3, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_pikxup4, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_pikxup5, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_pikxup6, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_pikxup7, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_pikxup1, ai_c_move, 0, 0, 0, NULL, 0, ogle_pick_dust,
};
animmove_t ogle_c_move_action4 = {7,ogle_c_frames_action4, ai_c_cycleend};

animframe_t ogle_c_frames_action5[] =
{
	FRAME_pikxdn2, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_pikxdn3, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_pikxdn4, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_pikxdn5, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_pikxdn6, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_pikxdn7, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_pikxdn1, ai_c_move, 0, 0, 0, NULL, 0, ogle_pick_dust,
};
animmove_t ogle_c_move_action5 = {7,ogle_c_frames_action5, ai_c_cycleend};


animframe_t ogle_c_frames_action6[] =
{
	FRAME_cela1, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_cela2, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_cela3, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_cela4, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_cela5, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_cela6, ai_c_move, 0, 0, 0, NULL, 0, NULL,
};
animmove_t ogle_c_move_action6 = {6, ogle_c_frames_action6, ai_c_cycleend};

animframe_t ogle_c_frames_action7[] =
{
	FRAME_celb1,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_celb2,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_celb3,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_celb4,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_celb5,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_celb6,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_celb7,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_celb8,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_celb9,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_celb10, ai_c_move, 0, 0, 0, NULL, 0, NULL,
};
animmove_t ogle_c_move_action7 = {10, ogle_c_frames_action7, ai_c_cycleend};


animframe_t ogle_c_frames_action8[] =
{
	FRAME_celc3,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_celc4,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_celc5,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_celc6,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_celc7,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_celc8,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
};
animmove_t ogle_c_move_action8 = {6, ogle_c_frames_action8, ai_c_cycleend};

animframe_t ogle_c_frames_action9[] =
{
	FRAME_celd5,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_celd6,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_celd7,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_celd8,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_celd9,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_celd10, ai_c_move, 0, 0, 0, NULL, 0, NULL,
};
animmove_t ogle_c_move_action9 = {6, ogle_c_frames_action9, ai_c_cycleend};

animframe_t ogle_c_frames_action10[] =
{
	FRAME_cele5,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_cele6,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_cele7,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_cele8,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
};
animmove_t ogle_c_move_action10 = {4, ogle_c_frames_action10, ai_c_cycleend};



animframe_t ogle_c_frames_action11[] =
{
	FRAME_charga1,  ai_c_move, 10, 0, 0, NULL, 0,  NULL,
	FRAME_charga2,  ai_c_move, 9,  0, 0, NULL, 0,  NULL,
	FRAME_charga3,  ai_c_move, 8,  0, 0, NULL, 0,  NULL,
	FRAME_charga4,  ai_c_move, 9,  0, 0, NULL, 0,  NULL,
	FRAME_charga5,  ai_c_move, 10, 0, 0, NULL, 0,  NULL,
	FRAME_charga6,  ai_c_move, 9,  0, 0, NULL, 0,  NULL,
	FRAME_charga7,  ai_c_move, 8,  0, 0, NULL, 0,  NULL,
	FRAME_charga8,  ai_c_move, 10, 0, 0, NULL, 0,  NULL,
};
animmove_t ogle_c_move_action11 = {8, ogle_c_frames_action11, ai_c_cycleend};

animframe_t ogle_c_frames_action12[] =
{
	FRAME_chargb1,  ai_c_move, 10, 0, 0, NULL, 0,  NULL,
	FRAME_chargb2,  ai_c_move,  9, 0, 0, NULL, 0,  NULL,
	FRAME_chargb3,  ai_c_move,  8, 0, 0, NULL, 0,  NULL,
	FRAME_chargb4,  ai_c_move,  9, 0, 0, NULL, 0,  NULL,
	FRAME_chargb5,  ai_c_move, 10, 0, 0, NULL, 0,  NULL,
	FRAME_chargb6,  ai_c_move,  9, 0, 0, NULL, 0,  NULL,
	FRAME_chargb7,  ai_c_move,  8, 0, 0, NULL, 0,  NULL,
	FRAME_chargb8,  ai_c_move, 10, 0, 0, NULL, 0,  NULL,
};
animmove_t ogle_c_move_action12 = {8, ogle_c_frames_action12, ai_c_cycleend};

animframe_t ogle_c_frames_action13[] =
{
	FRAME_chargc1,  ai_c_move, 10, 0, 0, NULL, 0,  NULL,
	FRAME_chargc2,  ai_c_move,  9, 0, 0, NULL, 0,  NULL,
	FRAME_chargc3,  ai_c_move,  8, 0, 0, NULL, 0,  NULL,
	FRAME_chargc4,  ai_c_move,  9, 0, 0, NULL, 0,  NULL,
	FRAME_chargc5,  ai_c_move, 10, 0, 0, NULL, 0,  NULL,
	FRAME_chargc6,  ai_c_move,  8, 0, 0, NULL, 0,  NULL,
	FRAME_chargc7,  ai_c_move,  9, 0, 0, NULL, 0,  NULL,
	FRAME_chargc8,  ai_c_move, 10, 0, 0, NULL, 0,  NULL,
};
animmove_t ogle_c_move_action13 = {8, ogle_c_frames_action13, ai_c_cycleend};

animframe_t ogle_c_frames_action14[] =
{
	FRAME_chargd1,  ai_c_move, 10, 0, 0, NULL, 0,  NULL,
	FRAME_chargd2,  ai_c_move,  9, 0, 0, NULL, 0,  NULL,
	FRAME_chargd3,  ai_c_move,  8, 0, 0, NULL, 0,  NULL,
	FRAME_chargd4,  ai_c_move,  9, 0, 0, NULL, 0,  NULL,
	FRAME_chargd5,  ai_c_move, 10, 0, 0, NULL, 0,  NULL,
	FRAME_chargd6,  ai_c_move,  9, 0, 0, NULL, 0,  NULL,
	FRAME_chargd7,  ai_c_move,  8, 0, 0, NULL, 0,  NULL,
	FRAME_chargd8,  ai_c_move, 10, 0, 0, NULL, 0,  NULL,
};
animmove_t ogle_c_move_action14 = {8, ogle_c_frames_action14, ai_c_cycleend};

animframe_t ogle_c_frames_action15[] =
{
	FRAME_charge1,  ai_c_move, 10, 0, 0, NULL,	0,  NULL,
	FRAME_charge2,  ai_c_move,  9, 0, 0, NULL,	0,  NULL,
	FRAME_charge3,  ai_c_move,  8, 0, 0, NULL,	0,  NULL,
	FRAME_charge4,  ai_c_move,  9, 0, 0, NULL,	0,  NULL,
	FRAME_charge5,  ai_c_move, 10, 0, 0, NULL,	0,  NULL,
	FRAME_charge6,  ai_c_move,  9, 0, 0, NULL,	0,  NULL,
	FRAME_charge7,  ai_c_move,  8, 0, 0, NULL,	0,  NULL,
	FRAME_charge8,  ai_c_move, 10, 0, 0, NULL,	0,  NULL,
};
animmove_t ogle_c_move_action15 = {8, ogle_c_frames_action15, ai_c_cycleend};




animframe_t ogle_c_frames_attack1[] =
{
	FRAME_pikxup2, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_pikxup3, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_pikxup4, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_pikxup5, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_pikxup6, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_pikxup7, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_pikxup1, ai_c_move, 0, 0, 0, NULL, 0, ogle_strike,
};
animmove_t ogle_c_move_attack1 = {7,ogle_c_frames_attack1, ai_c_cycleend};

animframe_t ogle_c_frames_attack2[] =
{
	FRAME_rnatka2, ai_c_move, 12, 0, 0, NULL, 0, NULL,
	FRAME_rnatka3, ai_c_move, 11, 0, 0, NULL, 0, NULL,
	FRAME_rnatka4, ai_c_move, 10, 0, 0, NULL, 0, NULL,
	FRAME_rnatka5, ai_c_move, 12, 0, 0, NULL, 0, NULL,
	FRAME_rnatka6, ai_c_move, 11, 0, 0, NULL, 0, NULL,
	FRAME_rnatka7, ai_c_move, 10, 0, 0, NULL, 0, NULL,
	FRAME_rnatka8, ai_c_move, 11, 0, 0, NULL, 0, NULL,
	FRAME_rnatka1, ai_c_move, 12, 0, 0, NULL, 0, ogle_strike,
};
animmove_t ogle_c_move_attack2 = {8,ogle_c_frames_attack2, ai_c_cycleend};

animframe_t ogle_c_frames_attack3[] =
{
	FRAME_rnatkb2, ai_c_move, 12, 0, 0, NULL, 0, NULL,
	FRAME_rnatkb3, ai_c_move, 11, 0, 0, NULL, 0, NULL,
	FRAME_rnatkb4, ai_c_move, 10, 0, 0, NULL, 0, NULL,
	FRAME_rnatkb5, ai_c_move, 12, 0, 0, NULL, 0, NULL,
	FRAME_rnatkb6, ai_c_move, 11, 0, 0, NULL, 0, NULL,
	FRAME_rnatkb7, ai_c_move, 10, 0, 0, NULL, 0, NULL,
	FRAME_rnatkb8, ai_c_move, 11, 0, 0, NULL, 0, NULL,
	FRAME_rnatkb1, ai_c_move, 12, 0, 0, NULL, 0, ogle_strike,
};
animmove_t ogle_c_move_attack3 = {8,ogle_c_frames_attack3, ai_c_cycleend};

/*----------------------------------------------------
	ogle death
----------------------------------------------------*/
animframe_t ogle_c_frames_death1[] =
{
	FRAME_deatha1, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_deatha2, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_deatha3, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_deatha4, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_deatha5, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_deatha6, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_deatha7, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_deatha8, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_deatha9, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_deatha10, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_deatha11, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_deatha12, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_deatha13, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_deatha14, ai_c_move, 0, 0, 0, NULL, 0, NULL,
};
animmove_t ogle_c_move_death1 = {14,ogle_c_frames_death1, ai_c_cycleend};

animframe_t ogle_c_frames_death2[] =
{
	FRAME_deathb1, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_deathb2, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_deathb3, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_deathb4, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_deathb5, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_deathb6, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_deathb7, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_deathb8, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_deathb9, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_deathb10, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_deathb11, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_deathb12, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_deathb13, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_deathb14, ai_c_move, 0, 0, 0, NULL, 0, NULL,
};
animmove_t ogle_c_move_death2 = {14,ogle_c_frames_death2, ai_c_cycleend};

/*----------------------------------------------------
	ogle standing - temp
----------------------------------------------------*/
animframe_t ogle_c_frames_idle1[] =
{
	FRAME_walk1, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_walk1, ai_c_move, 0, 0, 0, NULL, 0, NULL,
};
animmove_t ogle_c_move_idle1 = {2,ogle_c_frames_idle1, ai_c_cycleend};


animframe_t ogle_c_frames_idle2[] =
{
	FRAME_rstaid1, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rstaid2, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rstaid3, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rstaid4, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rstaid5, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rstaid6, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rstaid7, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rstaid8, ai_c_move, 0, 0, 0, NULL, 0, NULL,
};
animmove_t ogle_c_move_idle2 = {8,ogle_c_frames_idle2, ai_c_cycleend};

animframe_t ogle_c_frames_idle3[] =
{
	FRAME_rstawp1, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rstawp2, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rstawp3, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rstawp4, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rstawp5, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rstawp6, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rstawp7, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rstawp8, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rstawp9, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rstawp10,ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rstawp11,ai_c_move, 0, 0, 0, NULL, 0, NULL,
};
animmove_t ogle_c_move_idle3 = {11,ogle_c_frames_idle3, ai_c_cycleend};

animframe_t ogle_c_frames_idle4[] =
{
	FRAME_pkaxwp1, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_pkaxwp2, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_pkaxwp3, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_pkaxwp4, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_pkaxwp5, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_pkaxwp6, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_pkaxwp7, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_pkaxwp8, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_pkaxwp9, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_pkaxwp10,ai_c_move, 0, 0, 0, NULL, 0, NULL,
};
animmove_t ogle_c_move_idle4 = {10,ogle_c_frames_idle4, ai_c_cycleend};

animframe_t ogle_c_frames_idle5[] =
{
	FRAME_hamwp1, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_hamwp2, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_hamwp3, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_hamwp4, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_hamwp5, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_hamwp6, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_hamwp7, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_hamwp8, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_hamwp9, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_hamwp10,ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_hamwp11,ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_hamwp12,ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_hamwp13,ai_c_move, 0, 0, 0, NULL, 0, NULL,
};
animmove_t ogle_c_move_idle5 = {13,ogle_c_frames_idle5, ai_c_cycleend};

animframe_t ogle_c_frames_idle6[] =
{
	FRAME_brkidl1, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_brkidl2, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_brkidl3, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_brkidl4, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_brkidl5, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_brkidl6, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_brkidl7, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_brkidl8, ai_c_move, 0, 0, 0, NULL, 0, NULL,
};
animmove_t ogle_c_move_idle6 = {8, ogle_c_frames_idle6, ai_c_cycleend};


/*----------------------------------------------------
	ogle pain
----------------------------------------------------*/
animframe_t ogle_c_frames_pain1[] =
{
	FRAME_paina1, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_paina2, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_paina3, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_paina4, ai_c_move, 0, 0, 0, NULL, 0, NULL,
};
animmove_t ogle_c_move_pain1 = {4,ogle_c_frames_pain1, ai_c_cycleend};

animframe_t ogle_c_frames_pain2[] =
{
	FRAME_rstapn1, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rstapn2, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rstapn3, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rstapn4, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rstapn5, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rstapn6, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rstapn7, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rstapn8, ai_c_move, 0, 0, 0, NULL, 0, NULL,
};
animmove_t ogle_c_move_pain2 = {8,ogle_c_frames_pain2, ai_c_cycleend};

animframe_t ogle_c_frames_pain3[] =
{
	FRAME_brkpn1, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_brkpn2, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_brkpn3, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_brkpn4, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_brkpn5, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_brkpn6, ai_c_move, 0, 0, 0, NULL, 0, NULL,
};
animmove_t ogle_c_move_pain3 = {6,ogle_c_frames_pain3, ai_c_cycleend};


/*----------------------------------------------------
	ogle transitions
----------------------------------------------------*/
animframe_t ogle_c_frames_trans1[] =
{
	FRAME_rsta1, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rsta2, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rsta3, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rsta4, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rsta5, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_rsta6, ai_c_move, 0, 0, 0, NULL, 0, NULL,
};
animmove_t ogle_c_move_trans1 = {6,ogle_c_frames_trans1, ai_c_cycleend};


animframe_t ogle_c_frames_trans2[] =
{
	FRAME_brka1, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_brka2, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_brka3, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_brka4, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_brka5, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_brka6, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_brka7, ai_c_move, 0, 0, 0, NULL, 0, NULL,
};
animmove_t ogle_c_move_trans2 = {7,ogle_c_frames_trans2, ai_c_cycleend};

animframe_t ogle_c_frames_trans3[] =
{
	FRAME_brkb1, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_brkb2, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_brkb3, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_brkb4, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_brkb5, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_brkb6, ai_c_move, 0, 0, 0, NULL, 0, NULL,
};
animmove_t ogle_c_move_trans3 = {6,ogle_c_frames_trans3, ai_c_cycleend};

animframe_t ogle_c_frames_trans4[] =
{
	FRAME_celc1,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_celc2,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
};
animmove_t ogle_c_move_trans4 = {2, ogle_c_frames_trans4, ai_c_cycleend};

animframe_t ogle_c_frames_trans5[] =
{
	FRAME_celd1,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_celd2,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_celd3,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_celd4,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
};
animmove_t ogle_c_move_trans5 = {4, ogle_c_frames_trans5, ai_c_cycleend};

animframe_t ogle_c_frames_trans6[] =
{
	FRAME_cele1,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_cele2,  ai_c_move, 0, 0, 0, NULL, 0, NULL,
};
animmove_t ogle_c_move_trans6 = {2, ogle_c_frames_trans6, ai_c_cycleend};


/*----------------------------------------------------
	ogle walking
----------------------------------------------------*/
animframe_t ogle_c_frames_walk1[] =
{
	FRAME_walk1, ai_c_move, 2, 0, 0, NULL, 0, NULL,
	FRAME_walk2, ai_c_move, 2, 0, 0, NULL, 0, NULL,
	FRAME_walk3, ai_c_move, 2, 0, 0, NULL, 0, NULL,
	FRAME_walk4, ai_c_move, 2, 0, 0, NULL, 0, NULL,
	FRAME_walk5, ai_c_move, 2, 0, 0, NULL, 0, NULL,
	FRAME_walk6, ai_c_move, 2, 0, 0, NULL, 0, NULL,
	FRAME_walk7, ai_c_move, 2, 0, 0, NULL, 0, NULL,
	FRAME_walk8, ai_c_move, 2, 0, 0, NULL, 0, NULL,
};
animmove_t ogle_c_move_walk1 = {8,ogle_c_frames_walk1, ai_c_cycleend};


animframe_t ogle_c_frames_walk2[] =
{
	FRAME_pusha1, ai_c_move, 2, 0, 0, NULL, 0, NULL,
	FRAME_pusha2, ai_c_move, 2, 0, 0, NULL, 0, NULL,
	FRAME_pusha3, ai_c_move, 2, 0, 0, NULL, 0, NULL,
	FRAME_pusha4, ai_c_move, 2, 0, 0, NULL, 0, NULL,
	FRAME_pusha5, ai_c_move, 2, 0, 0, NULL, 0, NULL,
	FRAME_pusha6, ai_c_move, 2, 0, 0, NULL, 0, NULL,
	FRAME_pusha7, ai_c_move, 2, 0, 0, NULL, 0, NULL,
	FRAME_pusha8, ai_c_move, 2, 0, 0, NULL, 0, NULL,
};
animmove_t ogle_c_move_walk2 = {8,ogle_c_frames_walk2, ai_c_cycleend};

animframe_t ogle_c_frames_walk3[] =
{
	FRAME_pushb1, ai_c_move, 2, 0, 0, NULL, 0, NULL,
	FRAME_pushb2, ai_c_move, 2, 0, 0, NULL, 0, NULL,
	FRAME_pushb3, ai_c_move, 2, 0, 0, NULL, 0, NULL,
	FRAME_pushb4, ai_c_move, 2, 0, 0, NULL, 0, NULL,
	FRAME_pushb5, ai_c_move, 2, 0, 0, NULL, 0, NULL,
	FRAME_pushb6, ai_c_move, 2, 0, 0, NULL, 0, NULL,
	FRAME_pushb7, ai_c_move, 2, 0, 0, NULL, 0, NULL,
	FRAME_pushb8, ai_c_move, 2, 0, 0, NULL, 0, NULL,
};
animmove_t ogle_c_move_walk3 = {8,ogle_c_frames_walk3, ai_c_cycleend};

animframe_t ogle_c_frames_walk4[] =
{
	FRAME_pushc1, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_pushc2, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_pushc3, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_pushc4, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_pushc5, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_pushc6, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_pushc7, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_pushc8, ai_c_move, 0, 0, 0, NULL, 0, NULL,
};
animmove_t ogle_c_move_walk4 = {8,ogle_c_frames_walk4, ai_c_cycleend};



