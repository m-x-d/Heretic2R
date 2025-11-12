//
// c_corvus1_anim.h
//
// Copyright 1998 Raven Software
//

#pragma once

#pragma region ========================== Model definitions ==========================

#define FRAME_breath1         	0
#define FRAME_breath2         	1
#define FRAME_breath3         	2
#define FRAME_breath4         	3
#define FRAME_breath5         	4
#define FRAME_breath6         	5
#define FRAME_breath7         	6
#define FRAME_breath8         	7
#define FRAME_breath9         	8
#define FRAME_breath10        	9
#define FRAME_breath11        	10
#define FRAME_breath12        	11
#define FRAME_breath13        	12
#define FRAME_breath14        	13
#define FRAME_breath15        	14
#define FRAME_breath16        	15
#define FRAME_breath17        	16
#define FRAME_breath18        	17
#define FRAME_breath19        	18
#define FRAME_breath20        	19
#define FRAME_breath21        	20
#define FRAME_breath22        	21
#define FRAME_breath23        	22
#define FRAME_jog1            	23
#define FRAME_jog2            	24
#define FRAME_jog3            	25
#define FRAME_jog4            	26
#define FRAME_jog5            	27
#define FRAME_jog6            	28
#define FRAME_jog7            	29
#define FRAME_jog8            	30
#define FRAME_c_idleA1        	31
#define FRAME_c_idleA2        	32
#define FRAME_c_idleA3        	33
#define FRAME_c_idleA4        	34
#define FRAME_c_idleA5        	35
#define FRAME_c_idleA6        	36
#define FRAME_c_idleA7        	37
#define FRAME_c_idleA8        	38
#define FRAME_c_idleA9        	39
#define FRAME_c_idleA10       	40
#define FRAME_c_idleB1        	41
#define FRAME_c_idleB2        	42
#define FRAME_c_idleB3        	43
#define FRAME_c_idleB4        	44
#define FRAME_c_idleB5        	45
#define FRAME_c_idleB6        	46
#define FRAME_c_idleB7        	47
#define FRAME_c_idleB8        	48
#define FRAME_c_idleB9        	49
#define FRAME_c_idleB10       	50
#define FRAME_c_idleB11       	51
#define FRAME_c_idleB12       	52
#define FRAME_c_idleB13       	53
#define FRAME_c_idleB14       	54
#define FRAME_c_idleB15       	55
#define FRAME_c_idleB16       	56
#define FRAME_c_idleB17       	57
#define FRAME_c_idleB18       	58
#define FRAME_c_idleB19       	59
#define FRAME_c_idleB20       	60
#define FRAME_c_wheelA1       	61
#define FRAME_c_wheelA2       	62
#define FRAME_c_wheelA3       	63
#define FRAME_c_wheelA4       	64
#define FRAME_c_wheelB1       	65
#define FRAME_c_wheelB2       	66
#define FRAME_c_wheelB3       	67
#define FRAME_c_wheelB4       	68
#define FRAME_c_wheelB5       	69
#define FRAME_c_wheelB6       	70
#define FRAME_c_wheelB7       	71
#define FRAME_c_wheelB8       	72
#define FRAME_c_wheelB9       	73
#define FRAME_c_wheelB10      	74
#define FRAME_c_wheelB11      	75
#define FRAME_c_wheelB12      	76
#define FRAME_c_wheelB13      	77
#define FRAME_c_wheelB14      	78
#define FRAME_c_wheelB15      	79
#define FRAME_c_wheelB16      	80
#define FRAME_c_wheelB17      	81
#define FRAME_c_wheelB18      	82
#define FRAME_c_wheelB19      	83
#define FRAME_c_wheelB20      	84
#define FRAME_c_wheelB21      	85
#define FRAME_c_wheelB22      	86
#define FRAME_Lstep1          	87
#define FRAME_Lstep2          	88
#define FRAME_Lstep3          	89
#define FRAME_Lstep4          	90
#define FRAME_Lstep5          	91
#define FRAME_Rstep1          	92
#define FRAME_Rstep2          	93
#define FRAME_Rstep3          	94
#define FRAME_Rstep4          	95
#define FRAME_Rstep5          	96
#define FRAME_cinewalk1       	97
#define FRAME_cinewalk2       	98
#define FRAME_cinewalk3       	99
#define FRAME_cinewalk4       	100
#define FRAME_cinewalk5       	101
#define FRAME_cinewalk6       	102
#define FRAME_cinewalk7       	103
#define FRAME_cinewalk8       	104
#define FRAME_cinewalk9       	105
#define FRAME_cinewalk10      	106
#define FRAME_cinewalk11      	107
#define FRAME_cinewalk12      	108
#define FRAME_cinewalk13      	109
#define FRAME_cinewalk14      	110
#define FRAME_cinewalk15      	111
#define FRAME_cinewalk16      	112
#define FRAME_gorun2          	113
#define FRAME_gorun3          	114
#define FRAME_Lpivot1         	115
#define FRAME_Lpivot2         	116
#define FRAME_Lpivot3         	117
#define FRAME_Lpivot4         	118
#define FRAME_stance1         	119
#define FRAME_stance2         	120
#define FRAME_stance3         	121
#define FRAME_stance4         	122
#define FRAME_stance5         	123
#define FRAME_stance6         	124
#define FRAME_stance7         	125
#define FRAME_stance8         	126
#define FRAME_stance9         	127
#define FRAME_stance10        	128
#define FRAME_stance11        	129
#define FRAME_stance12        	130
#define FRAME_stance13        	131
#define FRAME_stance14        	132
#define FRAME_stance15        	133
#define FRAME_stance16        	134
#define FRAME_stance17        	135
#define FRAME_stance18        	136
#define FRAME_stance19        	137
#define FRAME_stance20        	138
#define FRAME_stance21        	139
#define FRAME_stance22        	140
#define FRAME_stance23        	141
#define FRAME_stance24        	142
#define FRAME_plagued1        	143
#define FRAME_plagued2        	144
#define FRAME_plagued3        	145
#define FRAME_plagued4        	146
#define FRAME_plagued5        	147
#define FRAME_plagued6        	148
#define FRAME_plagued7        	149
#define FRAME_plagued8        	150
#define FRAME_plagued9        	151
#define FRAME_plagued10       	152
#define FRAME_plagued11       	153
#define FRAME_plagued12       	154
#define FRAME_plagued13       	155
#define FRAME_plagued14       	156
#define FRAME_plagued15       	157
#define FRAME_plagued16       	158
#define FRAME_plagued17       	159
#define FRAME_plagued18       	160
#define FRAME_plagued19       	161
#define FRAME_plagued20       	162
#define FRAME_plagued21       	163
#define FRAME_plagued22       	164
#define FRAME_plagued23       	165
#define FRAME_plagued24       	166
#define FRAME_plagued25       	167
#define FRAME_plagued26       	168
#define FRAME_plagued27       	169
#define FRAME_plagued28       	170
#define FRAME_plagued29       	171
#define FRAME_plagued30       	172
#define FRAME_plagued31       	173
#define FRAME_plagued32       	174
#define FRAME_plagued33       	175
#define FRAME_plagued34       	176
#define FRAME_plagued35       	177
#define FRAME_plagued36       	178
#define FRAME_plagued37       	179
#define FRAME_plagued38       	180
#define FRAME_plagued39       	181
#define FRAME_plagued40       	182

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
	ANIM_C_ACTION4,
	ANIM_C_IDLE1,
	ANIM_C_IDLE2,
	ANIM_C_STRAFELEFT,
	ANIM_C_STRAFERIGHT,
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
