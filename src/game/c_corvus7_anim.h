//
// c_corvus7_anim.h
//
// Copyright 1998 Raven Software
//

#pragma once

#pragma region ========================== Model definitions ==========================

#define FRAME_Breath1         	0
#define FRAME_Breath2         	1
#define FRAME_Breath3         	2
#define FRAME_Breath4         	3
#define FRAME_Breath5         	4
#define FRAME_Breath6         	5
#define FRAME_Breath7         	6
#define FRAME_Breath8         	7
#define FRAME_Breath9         	8
#define FRAME_Breath10        	9
#define FRAME_Breath11        	10
#define FRAME_Breath12        	11
#define FRAME_Breath13        	12
#define FRAME_Breath14        	13
#define FRAME_Breath15        	14
#define FRAME_Breath16        	15
#define FRAME_Breath17        	16
#define FRAME_Breath18        	17
#define FRAME_Breath19        	18
#define FRAME_Breath20        	19
#define FRAME_Breath21        	20
#define FRAME_Breath22        	21
#define FRAME_Breath23        	22
#define FRAME_cinewalk1       	23
#define FRAME_cinewalk2       	24
#define FRAME_cinewalk3       	25
#define FRAME_cinewalk4       	26
#define FRAME_cinewalk5       	27
#define FRAME_cinewalk6       	28
#define FRAME_cinewalk7       	29
#define FRAME_cinewalk8       	30
#define FRAME_cinewalk9       	31
#define FRAME_cinewalk10      	32
#define FRAME_cinewalk11      	33
#define FRAME_cinewalk12      	34
#define FRAME_cinewalk13      	35
#define FRAME_cinewalk14      	36
#define FRAME_cinewalk15      	37
#define FRAME_cinewalk16      	38
#define FRAME_cluster         	39
#define FRAME_gorun1          	40
#define FRAME_gorun2          	41
#define FRAME_gorun3          	42
#define FRAME_jog1            	43
#define FRAME_jog2            	44
#define FRAME_jog3            	45
#define FRAME_jog4            	46
#define FRAME_jog5            	47
#define FRAME_jog6            	48
#define FRAME_jog7            	49
#define FRAME_jog8            	50
#define FRAME_Lpivot1         	51
#define FRAME_Lpivot2         	52
#define FRAME_Lpivot3         	53
#define FRAME_Lpivot4         	54
#define FRAME_MCinaa1         	55
#define FRAME_MCinaa2         	56
#define FRAME_MCinaa3         	57
#define FRAME_MCinaa4         	58
#define FRAME_MCinaa5         	59
#define FRAME_MCinaa6         	60
#define FRAME_MCinaa7         	61
#define FRAME_MCinaa8         	62
#define FRAME_MCinaa9         	63
#define FRAME_MCinaa10        	64
#define FRAME_MCinaa11        	65
#define FRAME_MCinaa12        	66
#define FRAME_MCinaa13        	67
#define FRAME_MCinaa14        	68
#define FRAME_MCinaa15        	69
#define FRAME_MCinaa16        	70
#define FRAME_MCinaa17        	71
#define FRAME_MCinaa18        	72
#define FRAME_MCinaa19        	73
#define FRAME_MCinaa20        	74
#define FRAME_MCinaa21        	75
#define FRAME_MCinaa22        	76
#define FRAME_MCinaa23        	77
#define FRAME_MCinaa24        	78
#define FRAME_MCinaa25        	79
#define FRAME_MCinaa26        	80
#define FRAME_MCinaa27        	81
#define FRAME_MCinaa28        	82
#define FRAME_MCinaa29        	83
#define FRAME_MCinaa30        	84
#define FRAME_MCinaa31        	85
#define FRAME_MCinaa32        	86
#define FRAME_MCinaa33        	87
#define FRAME_MCinaa34        	88
#define FRAME_MCinaa35        	89
#define FRAME_MCinaa36        	90
#define FRAME_MCinaa37        	91
#define FRAME_MCinaa38        	92
#define FRAME_MCinaa39        	93
#define FRAME_MCinaa40        	94
#define FRAME_MCinab1         	95
#define FRAME_MCinab2         	96
#define FRAME_MCinab3         	97
#define FRAME_MCinab4         	98
#define FRAME_MCinab5         	99
#define FRAME_MCinab6         	100
#define FRAME_MCinab7         	101
#define FRAME_MCinab8         	102
#define FRAME_MCinab9         	103
#define FRAME_MCinab10        	104
#define FRAME_MCinab11        	105
#define FRAME_MCinab12        	106
#define FRAME_MCinab13        	107
#define FRAME_MCinab14        	108
#define FRAME_MCinab15        	109
#define FRAME_MCinab16        	110
#define FRAME_MCinac1         	111
#define FRAME_MCinac2         	112
#define FRAME_MCinac3         	113
#define FRAME_MCinac4         	114
#define FRAME_MCinac5         	115
#define FRAME_MCinac6         	116
#define FRAME_MCinac7         	117
#define FRAME_MCinac8         	118
#define FRAME_MCinac9         	119
#define FRAME_MCinac10        	120
#define FRAME_MCinac11        	121
#define FRAME_MCinac12        	122
#define FRAME_MCinac13        	123
#define FRAME_MCinac14        	124
#define FRAME_idleA1          	125
#define FRAME_idleA2          	126
#define FRAME_idleA3          	127
#define FRAME_idleA4          	128
#define FRAME_idleA5          	129
#define FRAME_idleA6          	130
#define FRAME_idleA7          	131
#define FRAME_idleA8          	132
#define FRAME_idleA9          	133
#define FRAME_idleA10         	134
#define FRAME_idleA11         	135
#define FRAME_idleA12         	136
#define FRAME_idleB1          	137
#define FRAME_idleB2          	138
#define FRAME_idleB3          	139
#define FRAME_idleB4          	140
#define FRAME_idleB5          	141
#define FRAME_idleB6          	142
#define FRAME_idleB7          	143
#define FRAME_idleB8          	144
#define FRAME_idleB9          	145
#define FRAME_idleB10         	146
#define FRAME_idleB11         	147
#define FRAME_idleB12         	148

#define MODEL_SCALE			1.0f

#define NUM_MESH_NODES		16

#define MESH_BASE2			0
#define MESH__BACK			1
#define MESH__STOFF			2
#define MESH__BOFF			3
#define MESH__ARMOR			4
#define MESH__RARM			5
#define MESH__RHANDHI		6
#define MESH__STAFACTV		7
#define MESH__BLADSTF		8
#define MESH__HELSTF		9
#define MESH__LARM			10
#define MESH__LHANDHI		11
#define MESH__BOWACTV		12
#define MESH__RLEG			13
#define MESH__LLEG			14
#define MESH__HEAD			15

#pragma endregion

typedef enum AnimID_e
{
	ANIM_C_ACTION1,
	ANIM_C_ACTION2,
	ANIM_C_ACTION3,
	ANIM_C_IDLE1,
	ANIM_C_IDLE2,
	ANIM_C_IDLE3,
	ANIM_C_WALKSTART,
	ANIM_C_WALK1,
	ANIM_C_WALK2,
	ANIM_C_WALKSTOP1,
	ANIM_C_WALKSTOP2,
	ANIM_C_PIVOTLEFTGO,
	ANIM_C_PIVOTLEFT,
	ANIM_C_PIVOTLEFTSTOP,
	ANIM_C_PIVOTRIGHTGO,
	ANIM_C_PIVOTRIGHT,
	ANIM_C_PIVOTRIGHTSTOP,

	NUM_ANIMS
} AnimID_t;
