//
// c_siernan2_anim.h
//
// Copyright 1998 Raven Software
//

#pragma once

#pragma region ========================== Model definitions ==========================

#define FRAME_Minions1        	0
#define FRAME_Minions2        	1
#define FRAME_Minions3        	2
#define FRAME_Minions4        	3
#define FRAME_Minions5        	4
#define FRAME_Minions6        	5
#define FRAME_Minions7        	6
#define FRAME_Minions8        	7
#define FRAME_Minions9        	8
#define FRAME_Minions10       	9
#define FRAME_Minions11       	10
#define FRAME_Minions12       	11
#define FRAME_Minions13       	12
#define FRAME_Minions14       	13
#define FRAME_Minions15       	14
#define FRAME_Minions16       	15
#define FRAME_Minions17       	16
#define FRAME_Minions18       	17
#define FRAME_Minions19       	18
#define FRAME_Minions20       	19
#define FRAME_Minions21       	20
#define FRAME_Minions22       	21
#define FRAME_Minions23       	22
#define FRAME_Minions24       	23
#define FRAME_Minions25       	24
#define FRAME_Minions26       	25
#define FRAME_Minions27       	26
#define FRAME_Minions28       	27
#define FRAME_Minions29       	28
#define FRAME_Minions30       	29
#define FRAME_Minions31       	30
#define FRAME_Minions32       	31
#define FRAME_Minions33       	32
#define FRAME_Minions34       	33
#define FRAME_Minions35       	34
#define FRAME_Minions36       	35
#define FRAME_Minions37       	36
#define FRAME_Minions38       	37
#define FRAME_Minions39       	38
#define FRAME_Minions40       	39
#define FRAME_Minions41       	40
#define FRAME_Minions42       	41
#define FRAME_Minions43       	42
#define FRAME_Minions44       	43
#define FRAME_Minions45       	44
#define FRAME_Minions46       	45
#define FRAME_Minions47       	46
#define FRAME_Minions48       	47
#define FRAME_Minions49       	48
#define FRAME_Minions50       	49
#define FRAME_Minions51       	50
#define FRAME_Minions52       	51
#define FRAME_Minions53       	52
#define FRAME_Minions54       	53
#define FRAME_Minions55       	54
#define FRAME_Minions56       	55
#define FRAME_Minions57       	56
#define FRAME_Minions58       	57
#define FRAME_Minions59       	58
#define FRAME_Minions60       	59
#define FRAME_Minions61       	60
#define FRAME_Minions62       	61
#define FRAME_Minions63       	62
#define FRAME_Minions64       	63
#define FRAME_Minions65       	64
#define FRAME_Minions66       	65
#define FRAME_Minions67       	66
#define FRAME_Minions68       	67
#define FRAME_Minions69       	68
#define FRAME_Minions70       	69
#define FRAME_Minions71       	70
#define FRAME_Minions72       	71
#define FRAME_Minions73       	72
#define FRAME_Minions74       	73
#define FRAME_Minions75       	74
#define FRAME_Minions76       	75
#define FRAME_Minions77       	76
#define FRAME_Minions78       	77
#define FRAME_Minions79       	78
#define FRAME_Minions80       	79
#define FRAME_Minions81       	80
#define FRAME_Minions82       	81
#define FRAME_Minions83       	82
#define FRAME_Minions84       	83
#define FRAME_Minions85       	84
#define FRAME_Minions86       	85
#define FRAME_Minions87       	86
#define FRAME_Minions88       	87
#define FRAME_Minions89       	88
#define FRAME_Minions90       	89
#define FRAME_Minions91       	90
#define FRAME_Minions92       	91
#define FRAME_Minions93       	92
#define FRAME_Minions94       	93
#define FRAME_Minions95       	94
#define FRAME_Minions96       	95
#define FRAME_Minions97       	96
#define FRAME_Minions98       	97
#define FRAME_Minions99       	98
#define FRAME_Minions100      	99
#define FRAME_Minions101      	100
#define FRAME_Minions102      	101
#define FRAME_Minions103      	102
#define FRAME_Minions104      	103
#define FRAME_Minions105      	104
#define FRAME_Minions106      	105
#define FRAME_Minions107      	106
#define FRAME_Minions108      	107
#define FRAME_Minions109      	108
#define FRAME_Minions110      	109
#define FRAME_Minions111      	110
#define FRAME_Minions112      	111
#define FRAME_Minions113      	112
#define FRAME_Minions114      	113
#define FRAME_Minions115      	114
#define FRAME_Minions116      	115
#define FRAME_Minions117      	116
#define FRAME_Minions118      	117
#define FRAME_Minions119      	118
#define FRAME_Minions120      	119
#define FRAME_Minions121      	120
#define FRAME_Minions122      	121
#define FRAME_Minions123      	122
#define FRAME_Minions124      	123
#define FRAME_Minions125      	124
#define FRAME_Minions126      	125
#define FRAME_Minions127      	126
#define FRAME_Minions128      	127
#define FRAME_Minions129      	128
#define FRAME_Minions130      	129

#define NUM_MESH_NODES			6

#define MESH_MINIONS99			0
#define MESH__TORSO99			1
#define MESH__HEAD100			2
#define MESH__CENTERSPIKE100	3
#define MESH__LEFTARM99			4
#define MESH__RIGHTARM99		5

#pragma endregion

typedef enum AnimID_e
{
	ANIM_C_ACTION1,
	ANIM_C_ACTION2,
	ANIM_C_IDLE1,

	NUM_ANIMS
} AnimID_t;
