//
// c_dranor_anim.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "g_local.h"

#pragma region ========================== Model definitions ==========================

#define FRAME_1butyou1        	0
#define FRAME_1butyou2        	1
#define FRAME_1butyou3        	2
#define FRAME_1butyou4        	3
#define FRAME_1butyou5        	4
#define FRAME_1butyou6        	5
#define FRAME_1butyou7        	6
#define FRAME_1butyou8        	7
#define FRAME_1butyou9        	8
#define FRAME_1butyou10       	9
#define FRAME_1butyou11       	10
#define FRAME_1butyou12       	11
#define FRAME_1butyou13       	12
#define FRAME_1butyou14       	13
#define FRAME_1butyou15       	14
#define FRAME_1butyou16       	15
#define FRAME_1butyou17       	16
#define FRAME_1butyou18       	17
#define FRAME_1butyou19       	18
#define FRAME_1butyou20       	19
#define FRAME_1butyou21       	20
#define FRAME_1butyou22       	21
#define FRAME_1butyou23       	22
#define FRAME_1butyou24       	23
#define FRAME_1butyou25       	24
#define FRAME_1butyou26       	25
#define FRAME_1butyou27       	26
#define FRAME_1butyou28       	27
#define FRAME_1butyou29       	28
#define FRAME_1butyou30       	29
#define FRAME_1butyou31       	30
#define FRAME_1butyou32       	31
#define FRAME_1butyou33       	32
#define FRAME_1butyou34       	33
#define FRAME_1butyou35       	34
#define FRAME_1butyou36       	35
#define FRAME_1butyou37       	36
#define FRAME_1butyou38       	37
#define FRAME_1butyou39       	38
#define FRAME_1butyou40       	39
#define FRAME_1butyou41       	40
#define FRAME_1butyou42       	41
#define FRAME_1butyou43       	42
#define FRAME_1butyou44       	43
#define FRAME_1butyou45       	44
#define FRAME_1butyou46       	45
#define FRAME_1butyou47       	46
#define FRAME_1butyou48       	47
#define FRAME_1butyou49       	48
#define FRAME_1butyou50       	49
#define FRAME_1butyou51       	50
#define FRAME_1butyou52       	51
#define FRAME_1butyou53       	52
#define FRAME_1butyou54       	53
#define FRAME_1butyou55       	54
#define FRAME_1butyou56       	55
#define FRAME_1butyou57       	56
#define FRAME_1butyou58       	57
#define FRAME_1butyou59       	58
#define FRAME_1butyou60       	59
#define FRAME_1butyou61       	60
#define FRAME_1butyou62       	61
#define FRAME_1butyou63       	62
#define FRAME_1butyou64       	63
#define FRAME_1butyou65       	64
#define FRAME_1butyou66       	65
#define FRAME_1butyou67       	66
#define FRAME_1butyou68       	67
#define FRAME_1butyou69       	68
#define FRAME_1butyou70       	69
#define FRAME_1butyou71       	70
#define FRAME_1idle1          	71
#define FRAME_1idle2          	72
#define FRAME_1idle3          	73
#define FRAME_1idle4          	74
#define FRAME_1idle5          	75
#define FRAME_1idle6          	76
#define FRAME_1idle7          	77
#define FRAME_1idle8          	78
#define FRAME_1idle9          	79
#define FRAME_1idle10         	80
#define FRAME_1insert1        	81
#define FRAME_1insert2        	82
#define FRAME_1insert3        	83
#define FRAME_1insert4        	84
#define FRAME_1insert5        	85
#define FRAME_1insert6        	86
#define FRAME_1insert7        	87
#define FRAME_1insert8        	88
#define FRAME_1iwas1          	89
#define FRAME_1iwas2          	90
#define FRAME_1iwas3          	91
#define FRAME_1iwas4          	92
#define FRAME_1iwas5          	93
#define FRAME_1iwas6          	94
#define FRAME_1iwas7          	95
#define FRAME_1iwas8          	96
#define FRAME_2butyou1        	97
#define FRAME_2butyou2        	98
#define FRAME_2butyou3        	99
#define FRAME_2butyou4        	100
#define FRAME_2butyou5        	101
#define FRAME_2butyou6        	102
#define FRAME_2butyou7        	103
#define FRAME_2butyou8        	104
#define FRAME_2butyou9        	105
#define FRAME_2butyou10       	106
#define FRAME_2butyou11       	107
#define FRAME_2butyou12       	108
#define FRAME_2butyou13       	109
#define FRAME_2butyou14       	110
#define FRAME_2butyou15       	111
#define FRAME_2butyou16       	112
#define FRAME_2butyou17       	113
#define FRAME_2butyou18       	114
#define FRAME_2butyou19       	115
#define FRAME_2butyou20       	116
#define FRAME_2butyou21       	117
#define FRAME_2butyou22       	118
#define FRAME_2butyou23       	119
#define FRAME_2butyou24       	120
#define FRAME_2butyou25       	121
#define FRAME_2butyou26       	122
#define FRAME_2butyou27       	123
#define FRAME_2butyou28       	124
#define FRAME_2butyou29       	125
#define FRAME_2butyou30       	126
#define FRAME_2butyou31       	127
#define FRAME_2butyou32       	128
#define FRAME_2butyou33       	129
#define FRAME_2idle1          	130
#define FRAME_2idle2          	131
#define FRAME_2idle3          	132
#define FRAME_2idle4          	133
#define FRAME_2idle5          	134
#define FRAME_2idle6          	135
#define FRAME_2idle7          	136
#define FRAME_2idle8          	137
#define FRAME_2idle9          	138
#define FRAME_2idle10         	139
#define FRAME_2idle11         	140
#define FRAME_2idle12         	141
#define FRAME_2idle13         	142
#define FRAME_2idle14         	143
#define FRAME_2idle15         	144
#define FRAME_2idle16         	145
#define FRAME_2idle17         	146
#define FRAME_2idle18         	147
#define FRAME_2idle19         	148
#define FRAME_2idle20         	149
#define FRAME_2insert1        	150
#define FRAME_2insert2        	151
#define FRAME_2insert3        	152
#define FRAME_2insert4        	153
#define FRAME_2insert5        	154
#define FRAME_2insert6        	155
#define FRAME_2insert7        	156
#define FRAME_2insert8        	157
#define FRAME_2insert9        	158
#define FRAME_2insert10       	159
#define FRAME_2insert11       	160
#define FRAME_2insert12       	161
#define FRAME_2insert13       	162
#define FRAME_2insert14       	163
#define FRAME_2insert15       	164
#define FRAME_2insert16       	165
#define FRAME_2insert17       	166
#define FRAME_2insert18       	167
#define FRAME_2insert19       	168
#define FRAME_2insert20       	169
#define FRAME_2insert21       	170
#define FRAME_2insert22       	171
#define FRAME_2insert23       	172
#define FRAME_2insert24       	173
#define FRAME_2insert25       	174
#define FRAME_2insert26       	175
#define FRAME_2insert27       	176
#define FRAME_2insert28       	177
#define FRAME_2insert29       	178
#define FRAME_2insert30       	179
#define FRAME_2insert31       	180
#define FRAME_2insert32       	181
#define FRAME_2insert33       	182
#define FRAME_2iwas1          	183
#define FRAME_2iwas2          	184
#define FRAME_2iwas3          	185
#define FRAME_2iwas4          	186
#define FRAME_2iwas5          	187
#define FRAME_2iwas6          	188
#define FRAME_2iwas7          	189
#define FRAME_2iwas8          	190
#define FRAME_2iwas9          	191
#define FRAME_2iwas10         	192
#define FRAME_2iwas11         	193
#define FRAME_2iwas12         	194
#define FRAME_2iwas13         	195
#define FRAME_2iwas14         	196
#define FRAME_2iwas15         	197
#define FRAME_2iwas16         	198
#define FRAME_2iwas17         	199
#define FRAME_2iwas18         	200
#define FRAME_2iwas19         	201
#define FRAME_2iwas20         	202
#define FRAME_2iwas21         	203
#define FRAME_2iwas22         	204
#define FRAME_2iwas23         	205
#define FRAME_2iwas24         	206
#define FRAME_2iwas25         	207
#define FRAME_2iwas26         	208
#define FRAME_2iwas27         	209
#define FRAME_2iwas28         	210
#define FRAME_2iwas29         	211
#define FRAME_2iwas30         	212
#define FRAME_2iwas31         	213
#define FRAME_2iwas32         	214
#define FRAME_2iwas33         	215
#define FRAME_2iwas34         	216
#define FRAME_2iwas35         	217
#define FRAME_2iwas36         	218
#define FRAME_2iwas37         	219
#define FRAME_2iwas38         	220
#define FRAME_2iwas39         	221
#define FRAME_2iwas40         	222
#define FRAME_2iwas41         	223
#define FRAME_2iwas42         	224
#define FRAME_2iwas43         	225
#define FRAME_2iwas44         	226
#define FRAME_2iwas45         	227
#define FRAME_2iwas46         	228
#define FRAME_2iwas47         	229
#define FRAME_2iwas48         	230
#define FRAME_2iwas49         	231
#define FRAME_2iwas50         	232
#define FRAME_2iwas51         	233
#define FRAME_2iwas52         	234
#define FRAME_2iwas53         	235
#define FRAME_2iwas54         	236
#define FRAME_2iwas55         	237
#define FRAME_2iwas56         	238
#define FRAME_2iwas57         	239
#define FRAME_2iwas58         	240
#define FRAME_2iwas59         	241
#define FRAME_2iwas60         	242
#define FRAME_2iwas61         	243
#define FRAME_2iwas62         	244
#define FRAME_2iwas63         	245
#define FRAME_2iwas64         	246
#define FRAME_2iwas65         	247
#define FRAME_2iwas66         	248
#define FRAME_2iwas67         	249
#define FRAME_2iwas68         	250
#define FRAME_2iwas69         	251
#define FRAME_2iwas70         	252
#define FRAME_2iwas71         	253
#define FRAME_2iwas72         	254
#define FRAME_2iwas73         	255
#define FRAME_2iwas74         	256
#define FRAME_death1          	257
#define FRAME_death2          	258
#define FRAME_death3          	259
#define FRAME_death4          	260
#define FRAME_death5          	261
#define FRAME_death6          	262
#define FRAME_death7          	263
#define FRAME_death8          	264
#define FRAME_death9          	265
#define FRAME_death10         	266
#define FRAME_death11         	267
#define FRAME_death12         	268
#define FRAME_death13         	269
#define FRAME_death14         	270
#define FRAME_death15         	271
#define FRAME_death16         	272
#define FRAME_death17         	273
#define FRAME_death18         	274
#define FRAME_death19         	275
#define FRAME_death20         	276
#define FRAME_go1             	277
#define FRAME_go2             	278
#define FRAME_go3             	279
#define FRAME_go4             	280
#define FRAME_go5             	281
#define FRAME_go6             	282
#define FRAME_go7             	283
#define FRAME_go8             	284
#define FRAME_go9             	285
#define FRAME_go10            	286
#define FRAME_go11            	287
#define FRAME_go12            	288
#define FRAME_go13            	289
#define FRAME_go14            	290
#define FRAME_go15            	291
#define FRAME_go16            	292
#define FRAME_go17            	293
#define FRAME_go18            	294
#define FRAME_go19            	295
#define FRAME_go20            	296
#define FRAME_go21            	297
#define FRAME_go22            	298
#define FRAME_go23            	299
#define FRAME_go24            	300
#define FRAME_go25            	301
#define FRAME_go26            	302
#define FRAME_go27            	303
#define FRAME_go28            	304
#define FRAME_go29            	305
#define FRAME_go30            	306
#define FRAME_go31            	307
#define FRAME_go32            	308
#define FRAME_go33            	309
#define FRAME_go34            	310
#define FRAME_go35            	311
#define FRAME_go36            	312
#define FRAME_go37            	313
#define FRAME_go38            	314
#define FRAME_go39            	315
#define FRAME_go40            	316
#define FRAME_go41            	317
#define FRAME_go42            	318
#define FRAME_go43            	319
#define FRAME_go44            	320
#define FRAME_go45            	321
#define FRAME_go46            	322
#define FRAME_go47            	323
#define FRAME_go48            	324
#define FRAME_go49            	325
#define FRAME_go50            	326
#define FRAME_go51            	327
#define FRAME_go52            	328
#define FRAME_go53            	329
#define FRAME_go54            	330
#define FRAME_go55            	331
#define FRAME_go56            	332
#define FRAME_go57            	333
#define FRAME_go58            	334
#define FRAME_go59            	335
#define FRAME_go60            	336
#define FRAME_go61            	337
#define FRAME_go62            	338
#define FRAME_go63            	339
#define FRAME_go64            	340
#define FRAME_go65            	341
#define FRAME_go66            	342
#define FRAME_go67            	343
#define FRAME_go68            	344
#define FRAME_go69            	345
#define FRAME_go70            	346
#define FRAME_go71            	347
#define FRAME_go72            	348
#define FRAME_go73            	349
#define FRAME_go74            	350
#define FRAME_go75            	351
#define FRAME_go76            	352
#define FRAME_go77            	353
#define FRAME_go78            	354
#define FRAME_go79            	355
#define FRAME_go80            	356
#define FRAME_go81            	357
#define FRAME_go82            	358
#define FRAME_go83            	359
#define FRAME_go84            	360
#define FRAME_go85            	361
#define FRAME_go86            	362
#define FRAME_go87            	363
#define FRAME_go88            	364
#define FRAME_go89            	365
#define FRAME_go90            	366
#define FRAME_go91            	367
#define FRAME_go92            	368
#define FRAME_go93            	369
#define FRAME_go94            	370
#define FRAME_go95            	371
#define FRAME_go96            	372
#define FRAME_go97            	373
#define FRAME_go98            	374
#define FRAME_go99            	375
#define FRAME_go100           	376
#define FRAME_go101           	377
#define FRAME_go102           	378
#define FRAME_go103           	379
#define FRAME_go104           	380
#define FRAME_go105           	381
#define FRAME_go106           	382
#define FRAME_go107           	383
#define FRAME_go108           	384
#define FRAME_go109           	385
#define FRAME_go110           	386
#define FRAME_go111           	387
#define FRAME_go112           	388
#define FRAME_go113           	389
#define FRAME_go114           	390
#define FRAME_relax1          	391
#define FRAME_relax2          	392
#define FRAME_relax3          	393
#define FRAME_relax4          	394
#define FRAME_relax5          	395
#define FRAME_relax6          	396
#define FRAME_relax7          	397
#define FRAME_relax8          	398
#define FRAME_relax9          	399
#define FRAME_relax10         	400
#define FRAME_relax11         	401
#define FRAME_slayer1         	402
#define FRAME_slayer2         	403
#define FRAME_slayer3         	404
#define FRAME_slayer4         	405
#define FRAME_slayer5         	406
#define FRAME_slayer6         	407
#define FRAME_slayer7         	408
#define FRAME_slayer8         	409
#define FRAME_slayer9         	410
#define FRAME_slayer10        	411
#define FRAME_slayer11        	412
#define FRAME_slayer12        	413
#define FRAME_slayer13        	414
#define FRAME_slayer14        	415
#define FRAME_slayer15        	416
#define FRAME_slayer16        	417
#define FRAME_slayer17        	418
#define FRAME_slayer18        	419
#define FRAME_slayer19        	420
#define FRAME_slayer20        	421
#define FRAME_slayer21        	422
#define FRAME_slayer22        	423
#define FRAME_slayer23        	424
#define FRAME_slayer24        	425
#define FRAME_slayer25        	426
#define FRAME_slayer26        	427
#define FRAME_slayer27        	428
#define FRAME_slayer28        	429
#define FRAME_slayer29        	430
#define FRAME_slayer30        	431
#define FRAME_slayer31        	432
#define FRAME_slayer32        	433
#define FRAME_slayer33        	434
#define FRAME_slayer34        	435
#define FRAME_slayer35        	436
#define FRAME_slayer36        	437
#define FRAME_slayer37        	438
#define FRAME_slayer38        	439
#define FRAME_slayer39        	440
#define FRAME_slayer40        	441
#define FRAME_slayer41        	442
#define FRAME_slayer42        	443
#define FRAME_slayer43        	444
#define FRAME_slayer44        	445
#define FRAME_slayer45        	446
#define FRAME_slayer46        	447
#define FRAME_slayer47        	448
#define FRAME_slayer48        	449
#define FRAME_slayer49        	450
#define FRAME_slayer50        	451
#define FRAME_slayer51        	452
#define FRAME_slayer52        	453

#define MODEL_SCALE			1.0f

#define NUM_MESH_NODES		11

#define MESH_BASE			0
#define MESH__HANDLE		1
#define MESH__HOE			2
#define MESH__GAFF			3
#define MESH__HAMMER		4
#define MESH__BODY			5
#define MESH__L_LEG			6
#define MESH__R_LEG			7
#define MESH__R_ARM			8
#define MESH__L_ARM			9
#define MESH__HEAD			10

#pragma endregion

typedef enum AnimID_e
{
	ANIM_C_ACTION1,
	ANIM_C_ACTION2,
	ANIM_C_ACTION3,
	ANIM_C_ACTION4,
	ANIM_C_ACTION5,
	ANIM_C_ACTION6,
	ANIM_C_ACTION7,
	ANIM_C_ACTION8,
	ANIM_C_ACTION9,
	ANIM_C_ACTION10,
	ANIM_C_ACTION11,
	ANIM_C_ACTION12,
	ANIM_C_DEATH1,
	ANIM_C_IDLE1,
	ANIM_C_IDLE2,
	ANIM_C_IDLE3,
	NUM_ANIMS
} AnimID_t;

extern const animmove_t dranor_move_c_action1;
extern const animmove_t dranor_move_c_action2;
extern const animmove_t dranor_move_c_action3;
extern const animmove_t dranor_move_c_action4;
extern const animmove_t dranor_move_c_action5;
extern const animmove_t dranor_move_c_action6;
extern const animmove_t dranor_move_c_action7;
extern const animmove_t dranor_move_c_action8;
extern const animmove_t dranor_move_c_action9;
extern const animmove_t dranor_move_c_action10;
extern const animmove_t dranor_move_c_action11;
extern const animmove_t dranor_move_c_action12;
extern const animmove_t dranor_move_c_death1;
extern const animmove_t dranor_move_c_idle1;
extern const animmove_t dranor_move_c_idle2;
extern const animmove_t dranor_move_c_idle3;