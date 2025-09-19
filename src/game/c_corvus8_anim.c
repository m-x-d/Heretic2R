//
// c_corvus8_anim.c
//
// Copyright 1998 Raven Software
//

#include "c_corvus8_anim.h"
#include "c_ai.h"

// Corvus action 1.
static const animframe_t corvus8_frames_c_action1[] =
{
	{ FRAME_kneel1, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kneel2, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kneel3, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kneel4, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kneel5, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kneel6, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kneel7, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kneel8, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus8_move_c_action1 = ANIMMOVE(corvus8_frames_c_action1, ai_c_cycleend);

// Corvus action 2.
static const animframe_t corvus8_frames_c_action2[] =
{
	{ FRAME_knltrn1,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_knltrn2,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_knltrn3,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_knltrn4,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_knltrn5,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_knltrn6,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_knltrn7,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_knltrn8,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_knltrn9,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_knltrn10, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_knltrn11, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_knltrn12, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_knltrn13, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus8_move_c_action2 = ANIMMOVE(corvus8_frames_c_action2, ai_c_cycleend);

// Corvus action 3.
static const animframe_t corvus8_frames_c_action3[] =
{
	{ FRAME_knlcyc1,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_knlcyc2,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_knlcyc3,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_knlcyc4,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_knlcyc5,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_knlcyc6,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_knlcyc7,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_knlcyc8,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_knlcyc9,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_knlcyc10, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_knlcyc11, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_knlcyc12, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus8_move_c_action3 = ANIMMOVE(corvus8_frames_c_action3, ai_c_cycleend);

// Corvus action 4.
static const animframe_t corvus8_frames_c_action4[] =
{
	{ FRAME_Ido1, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Ido2, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Ido3, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Ido4, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Ido5, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Ido6, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus8_move_c_action4 = ANIMMOVE(corvus8_frames_c_action4, ai_c_cycleend);

// Corvus action 5.
static const animframe_t corvus8_frames_c_action5[] =
{
	{ FRAME_stand8, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_stand7, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_stand6, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_stand5, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_stand4, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_stand3, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_stand2, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_stand1, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus8_move_c_action5 = ANIMMOVE(corvus8_frames_c_action5, ai_c_cycleend);

// Corvus action 6.
static const animframe_t corvus8_frames_c_action6[] =
{
	{ FRAME_butits1,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_butits2,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_butits3,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_butits4,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_butits5,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_butits6,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_butits7,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_butits8,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_butits9,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_butits10, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_butits11, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_butits12, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_butits13, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_butits14, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_butits15, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_butits16, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_butits17, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_butits18, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_butits19, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_butits20, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_butits21, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_butits22, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_butits23, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_butits24, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus8_move_c_action6 = ANIMMOVE(corvus8_frames_c_action6, ai_c_cycleend);

// Corvus action 7.
static const animframe_t corvus8_frames_c_action7[] =
{
	{ FRAME_bytome1,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_bytome2,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_bytome3,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_bytome4,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_bytome5,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_bytome6,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_bytome7,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_bytome8,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_bytome9,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_bytome10, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_bytome11, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_bytome12, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_bytome13, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_bytome14, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_bytome15, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_bytome16, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_bytome17, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_bytome18, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus8_move_c_action7 = ANIMMOVE(corvus8_frames_c_action7, ai_c_cycleend);

// Corvus action 8.
static const animframe_t corvus8_frames_c_action8[] =
{
	{ FRAME_iam1,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_iam2,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_iam3,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_iam4,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_iam5,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_iam6,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_iam7,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_iam8,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_iam9,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_iam10, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_iam11, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus8_move_c_action8 = ANIMMOVE(corvus8_frames_c_action8, ai_c_cycleend);

// Corvus action 9.
static const animframe_t corvus8_frames_c_action9[] =
{
	{ FRAME_noharm1,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_noharm2,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_noharm3,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_noharm4,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_noharm5,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_noharm6,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_noharm7,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_noharm8,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_noharm9,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_noharm10, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_noharm11, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_noharm12, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_noharm13, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_noharm14, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_noharm15, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_noharm16, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_noharm17, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_noharm18, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_noharm19, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_noharm20, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_noharm21, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_noharm22, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_noharm23, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_noharm24, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_noharm25, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_noharm26, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_noharm27, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_noharm28, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_noharm29, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_noharm30, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_noharm31, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus8_move_c_action9 = ANIMMOVE(corvus8_frames_c_action9, ai_c_cycleend);

// Corvus action 10.
static const animframe_t corvus8_frames_c_action10[] =
{
	{ FRAME_willu1,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_willu2,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_willu3,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_willu4,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_willu5,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_willu6,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_willu7,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_willu8,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_willu9,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_willu10, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus8_move_c_action10 = ANIMMOVE(corvus8_frames_c_action10, ai_c_cycleend);

// Corvus action 11.
static const animframe_t corvus8_frames_c_action11[] =
{
	{ FRAME_bytome1, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_bytome2, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_bytome3, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_bytome4, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_bytome5, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_bytome6, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_bytome7, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_bytome8, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_bytome9, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus8_move_c_action11 = ANIMMOVE(corvus8_frames_c_action11, ai_c_cycleend);

// Corvus action 12.
static const animframe_t corvus8_frames_c_action12[] =
{
	{ FRAME_bytome10, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_bytome11, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_bytome12, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_bytome13, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_bytome14, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_bytome15, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_bytome16, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_bytome17, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_bytome18, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus8_move_c_action12 = ANIMMOVE(corvus8_frames_c_action12, ai_c_cycleend);

// Corvus action 13.
static const animframe_t corvus8_frames_c_action13[] =
{
	{ FRAME_arethe1,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_arethe2,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_arethe3,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_arethe4,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_arethe5,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_arethe6,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_arethe7,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_arethe8,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_arethe9,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_arethe10, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_arethe11, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_arethe12, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_arethe13, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_arethe14, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_arethe15, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_arethe16, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_arethe17, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_arethe18, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_arethe19, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_arethe20, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_arethe21, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus8_move_c_action13 = ANIMMOVE(corvus8_frames_c_action13, ai_c_cycleend);

// Corvus action 14.
static const animframe_t corvus8_frames_c_action14[] =
{
	{ FRAME_canit1,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_canit2,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_canit3,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_canit4,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_canit5,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_canit6,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_canit7,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_canit8,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_canit9,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_canit10, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_canit11, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_canit12, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_canit13, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_canit14, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_canit15, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_canit16, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_canit17, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_canit18, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_canit19, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_canit20, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_canit21, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus8_move_c_action14 = ANIMMOVE(corvus8_frames_c_action14, ai_c_cycleend);

// Corvus action 15.
static const animframe_t corvus8_frames_c_action15[] =
{
	{ FRAME_grtful1,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_grtful2,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_grtful3,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_grtful4,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_grtful5,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_grtful6,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_grtful7,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_grtful8,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_grtful9,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_grtful10, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_grtful11, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_grtful12, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_grtful13, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_grtful14, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_grtful15, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_grtful16, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_grtful17, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_grtful18, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_grtful19, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_grtful20, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_grtful21, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_grtful22, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_grtful23, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_grtful24, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_grtful25, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_grtful26, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_grtful27, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_grtful28, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_grtful29, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_grtful30, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_grtful31, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_grtful32, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_grtful33, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_grtful34, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_grtful35, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_grtful36, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_grtful37, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_grtful38, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_grtful39, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_grtful40, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_grtful41, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_grtful42, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_grtful43, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_grtful44, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_grtful45, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_grtful46, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_grtful47, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_grtful48, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_grtful49, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_grtful50, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_grtful51, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_grtful52, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_grtful53, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_grtful54, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_grtful55, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_grtful56, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_grtful57, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_grtful58, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus8_move_c_action15 = ANIMMOVE(corvus8_frames_c_action15, ai_c_cycleend);

// Corvus action 16.
static const animframe_t corvus8_frames_c_action16[] =
{
	{ FRAME_kelmnt1,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kelmnt2,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kelmnt3,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kelmnt4,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kelmnt5,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kelmnt6,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kelmnt7,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kelmnt8,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kelmnt9,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kelmnt10, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kelmnt11, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kelmnt12, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kelmnt13, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kelmnt14, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kelmnt15, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kelmnt16, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kelmnt17, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kelmnt18, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kelmnt19, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kelmnt20, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kelmnt21, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kelmnt22, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kelmnt23, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kelmnt24, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kelmnt25, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kelmnt26, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kelmnt27, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kelmnt28, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kelmnt29, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kelmnt30, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kelmnt31, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kelmnt32, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kelmnt33, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kelmnt34, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kelmnt35, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kelmnt36, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kelmnt37, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kelmnt38, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kelmnt39, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kelmnt40, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kelmnt41, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kelmnt42, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kelmnt43, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kelmnt44, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kelmnt45, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kelmnt46, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kelmnt47, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kelmnt48, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kelmnt49, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kelmnt50, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kelmnt51, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kelmnt52, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kelmnt53, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kelmnt54, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kelmnt55, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kelmnt56, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kelmnt57, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kelmnt58, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kelmnt59, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kelmnt60, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kelmnt61, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kelmnt62, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kelmnt63, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kelmnt64, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kelmnt65, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kelmnt66, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kelmnt67, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kelmnt68, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kelmnt69, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kelmnt70, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kelmnt71, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kelmnt72, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kelmnt73, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kelmnt74, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kelmnt75, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kelmnt76, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_kelmnt77, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus8_move_c_action16 = ANIMMOVE(corvus8_frames_c_action16, ai_c_cycleend);

// Corvus action 17.
static const animframe_t corvus8_frames_c_action17[] =
{
	{ FRAME_potion1,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_potion2,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_potion3,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_potion4,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_potion5,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_potion6,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_potion7,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_potion8,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_potion9,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_potion10, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_potion11, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_potion12, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_potion13, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_potion14, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_potion15, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_potion16, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_potion17, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_potion18, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_potion19, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_potion20, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_potion21, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_potion22, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_potion23, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_potion24, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_potion25, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_potion26, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_potion27, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_potion28, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_potion29, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_potion30, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_potion31, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_potion32, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_potion33, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_potion34, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_potion35, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_potion36, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_potion37, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_potion38, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_potion39, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_potion40, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_potion41, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_potion42, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_potion43, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_potion44, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_potion45, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_potion46, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_potion47, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_potion48, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_potion49, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_potion50, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_potion51, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus8_move_c_action17 = ANIMMOVE(corvus8_frames_c_action17, ai_c_cycleend);

// Corvus action 18.
static const animframe_t corvus8_frames_c_action18[] =
{
	{ FRAME_pstop1, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_pstop2, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_pstop3, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_pstop4, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_pstop5, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus8_move_c_action18 = ANIMMOVE(corvus8_frames_c_action18, ai_c_cycleend);

// Corvus action 19.
static const animframe_t corvus8_frames_c_action19[] =
{
	{ FRAME_drink1,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_drink2,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_drink3,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_drink4,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_drink5,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_drink6,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_drink7,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_drink8,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_drink9,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_drink10, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_drink11, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_drink12, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_drink13, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_drink14, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_drink15, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_drink16, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_drink17, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_drink18, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_drink19, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_drink20, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_drink21, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus8_move_c_action19 = ANIMMOVE(corvus8_frames_c_action19, ai_c_cycleend);

// Corvus action 20.
static const animframe_t corvus8_frames_c_action20[] =
{
	{ FRAME_pour1,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_pour2,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_pour3,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_pour4,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_pour5,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_pour6,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_pour7,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_pour8,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_pour9,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_pour10, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_pour11, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_pour12, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_pour13, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_pour14, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_pour15, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_pour16, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_pour17, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_pour18, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_pour19, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus8_move_c_action20 = ANIMMOVE(corvus8_frames_c_action20, ai_c_cycleend);

// Corvus idle 1.
static const animframe_t corvus8_frames_c_idle1[] =
{
	{ FRAME_Breath1,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Breath2,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Breath3,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Breath4,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Breath5,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Breath6,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Breath7,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Breath8,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Breath9,  ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Breath10, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Breath11, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Breath12, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Breath13, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Breath14, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Breath15, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Breath16, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Breath17, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Breath18, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Breath19, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Breath20, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Breath21, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Breath22, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Breath23, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus8_move_c_idle1 = ANIMMOVE(corvus8_frames_c_idle1, ai_c_cycleend);

// Corvus idle 2.
static const animframe_t corvus8_frames_c_idle2[] =
{
	{ FRAME_pour19, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_pour19, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus8_move_c_idle2 = ANIMMOVE(corvus8_frames_c_idle2, ai_c_cycleend);

// Corvus - starting his pivot to the left.
static const animframe_t corvus8_frames_c_pivotleftgo[] =
{
	{ FRAME_Lpivot1, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Lpivot2, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus8_move_c_pivotleftgo = ANIMMOVE(corvus8_frames_c_pivotleftgo, ai_c_cycleend);

static const animframe_t corvus8_frames_c_pivotleft[] =
{
	{ FRAME_Lpivot3, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Lpivot4, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus8_move_c_pivotleft = ANIMMOVE(corvus8_frames_c_pivotleft, ai_c_cycleend);

static const animframe_t corvus8_frames_c_pivotleftstop[] =
{
	{ FRAME_Lpivot4, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus8_move_c_pivotleftstop = ANIMMOVE(corvus8_frames_c_pivotleftstop, ai_c_cycleend);

// Corvus - starting his pivot to the right.
static const animframe_t corvus8_frames_c_pivotrightgo[] =
{
	{ FRAME_Lpivot4, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Lpivot3, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus8_move_c_pivotrightgo = ANIMMOVE(corvus8_frames_c_pivotrightgo, ai_c_cycleend);

static const animframe_t corvus8_frames_c_pivotright[] =
{
	{ FRAME_Lpivot2, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Lpivot1, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus8_move_c_pivotright = ANIMMOVE(corvus8_frames_c_pivotright, ai_c_cycleend);

static const animframe_t corvus8_frames_c_pivotrightstop[] =
{
	{ FRAME_Lpivot1, NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus8_move_c_pivotrightstop = ANIMMOVE(corvus8_frames_c_pivotrightstop, ai_c_cycleend);

// Corvus walk 1.
static const animframe_t corvus8_frames_c_walk1[] =
{
	{ FRAME_jog1, ai_c_move, 10, 0, 0, NULL, 0, NULL },
	{ FRAME_jog2, ai_c_move, 16, 0, 0, NULL, 0, NULL },
	{ FRAME_jog3, ai_c_move, 15, 0, 0, NULL, 0, NULL },
	{ FRAME_jog4, ai_c_move, 16, 0, 0, NULL, 0, NULL },
	{ FRAME_jog5, ai_c_move, 17, 0, 0, NULL, 0, NULL },
	{ FRAME_jog6, ai_c_move, 16, 0, 0, NULL, 0, NULL },
	{ FRAME_jog7, ai_c_move, 15, 0, 0, NULL, 0, NULL },
	{ FRAME_jog8, ai_c_move, 16, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus8_move_c_walk1 = ANIMMOVE(corvus8_frames_c_walk1, ai_c_cycleend);

// Corvus walk 2.
static const animframe_t corvus8_frames_c_walk2[] =
{
	{ FRAME_cinewalk1,  ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_cinewalk2,  ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_cinewalk3,  ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_cinewalk4,  ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_cinewalk5,  ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_cinewalk6,  ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_cinewalk7,  ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_cinewalk8,  ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_cinewalk9,  ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_cinewalk10, ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_cinewalk11, ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_cinewalk12, ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_cinewalk13, ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_cinewalk14, ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_cinewalk15, ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_cinewalk16, ai_c_move, 4, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus8_move_c_walk2 = ANIMMOVE(corvus8_frames_c_walk2, ai_c_cycleend);

// Corvus walk 3.
static const animframe_t corvus8_frames_c_walk3[] =
{
	{ FRAME_pwalk1,  ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_pwalk2,  ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_pwalk3,  ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_pwalk4,  ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_pwalk5,  ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_pwalk6,  ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_pwalk7,  ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_pwalk8,  ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_pwalk9,  ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_pwalk10, ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_pwalk11, ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_pwalk12, ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_pwalk13, ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_pwalk14, ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_pwalk15, ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_pwalk16, ai_c_move, 4, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus8_move_c_walk3 = ANIMMOVE(corvus8_frames_c_walk3, ai_c_cycleend);

// Corvus - starting his walk anims.
static const animframe_t corvus8_frames_c_walkstart[] =
{
	{ FRAME_gorun2, ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_gorun3, ai_c_move, 4, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus8_move_c_walkstart = ANIMMOVE(corvus8_frames_c_walkstart, ai_c_cycleend);

// Corvus - stopping his walk with right foot in front.
static const animframe_t corvus8_frames_c_walkstop1[] =
{
	{ FRAME_jog1, ai_c_move, 2, 0, 0, NULL, 0, NULL },
	{ FRAME_jog2, ai_c_move, 2, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus8_move_c_walkstop1 = ANIMMOVE(corvus8_frames_c_walkstop1, ai_c_cycleend);

// Corvus - stopping his walk with left foot in front.
static const animframe_t corvus8_frames_c_walkstop2[] =
{
	{ FRAME_jog5, ai_c_move, 2, 0, 0, NULL, 0, NULL },
	{ FRAME_jog6, ai_c_move, 2, 0, 0, NULL, 0, NULL },
};
const animmove_t corvus8_move_c_walkstop2 = ANIMMOVE(corvus8_frames_c_walkstop2, ai_c_cycleend);