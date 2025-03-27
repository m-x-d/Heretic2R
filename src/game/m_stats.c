//
// m_stats.c
//
// Copyright 1998 Raven Software
//

#include "m_stats.h"

//TODO: all these stats can be stored in a single struct...

// melee_range				- How close an enemy must be to use melee attack. If negative, how much distance the monster tries to keep between itself and an enemy.
// missile_range			- Maximum distance a monster will try to attack	from using it's ranged attack.
// min_missile_range		- Min range at which a monster will not use	it's missile attack (too close).
// bypass_missile_chance	- 0 - 100 chance of a monster not taking a shot even when it can, otherwise it will close in.
int AttackRangesForClass[NUM_ATTACK_RANGES] =
{
	// melee_rn	g,	missile_rng,	min_msl_rng,	bypass_missile_chance
	0,				0,				0,				0,	// CID_NONE
	0,				0,				0,				0,	// CID_RAT
	72,				150,			50,				20,	// CID_GORGON
	50,				0,				0,				0,	// CID_PLAGUEELF
	0,				256,			48,				0,	// CID_GKROKON
	0,				0,				0,				0,	// CID_FISH
	0,				0,				0,				0,	// CID_OBJECT
	0,				0,				0,				0,	// CID_LIGHT
	0,				0,				0,				0,	// CID_TRIGGER
	0,				0,				0,				0,	// CID_HARPY
	100,			512,			200,			50,	// CID_SPREADER
	0,				0,				0,				0,	// CID_ELFLORD
	0,				0,				0,				0,	// CID_BBRUSH
	0,				0,				0,				0,	// CID_FUNC_ROTATE
	0,				0,				0,				0,	// CID_FUNC_DOOR
	0,				0,				0,				0,	// CID_CHICKEN
	64,				512,			64,				50,	// CID_SSITHRA
	0,				0,				0,				0,	// CID_SPELL
	100,			800,			100,			25,	// CID_MSSITHRA
	48,				0,				0,				0,	// CID_OGLE
	100,			0,				0,				0,	// CID_SERAPH_OVERLORD
	100,			1024,			64,				85,	// CID_SERAPH_GUARD
	48,				1024,			64,				10,	// CID_ASSASSIN
	0,				0,				0,				0,	// CID_TELEPORTER
	0,				0,				0,				0,	// CID_HIGHPRIESTESS
	-72,			512,			48,				0,	// CID_TCHECKRIK
	0,				0,				0,				0,	// CID_BUTTON
	0,				0,				0,				0,	// CID_BEE
	0,				0,				0,				0,	// CID_CORVUS
	64,				1024,			64,				25,	// CID_MORK
	400,			1500,			100,			77,	// CID_TBEAST
	-64,			1024,			32,				20,	// CID_IMP 

	0,				0,				0,				0,	// CID_SSITHRA_VICTIM
	0,				0,				0,				0,	// CID_SSITHRA_SCOUT
	0,				0,				0,				0,	// CID_DRANOR

	0,				0,				0,				0,	// CID_TRIG_DAMAGE
	0,				0,				0,				0,	// CID_TRIG_PUSH
	0,				0,				0,				0,	// CID_C_ELFLORD
	0,				0,				0,				0,	// CID_C_SIERNAN1
	0,				0,				0,				0,	// CID_C_SIERNAN2
	0,				0,				0,				0,	// CID_C_HIGHPRIESTESS
	0,				0,				0,				0,	// CID_C_HIGHPRIESTESS2
	0,				0,				0,				0,	// CID_C_TOME
	0,				0,				0,				0,	// CID_C_MORCALAVIN
	0,				0,				0,				0,	// CID_CORVUS2
	0,				0,				0,				0,	// CID_CORVUS3
	0,				0,				0,				0,	// CID_CORVUS4
	0,				0,				0,				0,	// CID_CORVUS5
	0,				0,				0,				0,	// CID_CORVUS6
	0,				0,				0,				0,	// CID_CORVUS7
	0,				0,				0,				0,	// CID_CORVUS8
	0,				0,				0,				0,	// CID_CORVUS9
};

// This table determines how many buoys a monster will search through in a line before it goes back and tries a different buoy path.
// Helps cut down the hit of searching through buoys and helps find the shorter path if there is one.
byte MaxBuoysForClass[NUM_CLASSIDS] =
{
	0,	// CID_NONE
	8,	// CID_RAT
	20,	// CID_GORGON
	16,	// CID_PLAGUEELF
	12,	// CID_GKROKON
	6,	// CID_FISH
	0,	// CID_OBJECT
	0,	// CID_LIGHT
	0,	// CID_TRIGGER
	14,	// CID_HARPY
	24,	// CID_SPREADER
	0,	// CID_ELFLORD
	0,	// CID_BBRUSH
	0,	// CID_FUNC_ROTATE
	0,	// CID_FUNC_DOOR
	12,	// CID_CHICKEN
	28,	// CID_SSITHRA
	0,	// CID_SPELL,
	16,	// CID_MSSITHRA
	8,	// CID_OGLE
	24,	// CID_SERAPH_OVERLORD
	20,	// CID_SERAPH_GUARD
	64,	// CID_ASSASSIN
	0,	// CID_TELEPORTER
	16,	// CID_HIGHPRIESTESS
	32,	// CID_TCHECKRIK
	0,	// CID_BUTTON
	6,	// CID_BEE
	0,	// CID_CORVUS
	40,	// CID_MORK
	100,// CID_BEAST
	14,	// CID_IMP

	0,	// CID_SSITHRA_VICTIM
	0,	// CID_SSITHRA_SCOUT
	0,	// CID_DRANOR

	0,	// CID_TRIG_DAMAGE
	0,	// CID_TRIG_PUSH
	0,	// CID_C_ELFLORD
	0,	// CID_C_SIERNAN1
	0,	// CID_C_SIERNAN2
	0,	// CID_C_HIGHPRIESTESS
	0,	// CID_C_HIGHPRIESTESS2
	0,	// CID_C_TOME
	0,	// CID_C_MORCALAVIN
	0,	// CID_CORVUS2
	0,	// CID_CORVUS3
	0,	// CID_CORVUS4
	0,	// CID_CORVUS5
	0,	// CID_CORVUS6
	0,	// CID_CORVUS7
	0,	// CID_CORVUS8
	0,	// CID_CORVUS9
};

// 0 - 100 chance that a monster will check and see if it can jump when it hits a ledge.
int JumpChanceForClass[NUM_CLASSIDS] =
{
	0,	// CID_NONE
	100,// CID_RAT
	80,	// CID_GORGON
	50,	// CID_PLAGUEELF
	100,// CID_GKROKON
	0,	// CID_FISH
	0,	// CID_OBJECT
	0,	// CID_LIGHT
	0,	// CID_TRIGGER
	0,	// CID_HARPY
	30,	// CID_SPREADER
	0,	// CID_ELFLORD
	0,	// CID_BBRUSH
	0,	// CID_FUNC_ROTATE
	0,	// CID_FUNC_DOOR
	100,// CID_CHICKEN
	100,// CID_SSITHRA
	0,	// CID_SPELL
	25,	// CID_MSSITHRA
	10,	// CID_OGLE
	30,	// CID_SERAPH_OVERLORD
	20,	// CID_SERAPH_GUARD
	100,// CID_ASSASSIN
	0,	// CID_TELEPORTER
	0,	// CID_HIGHPRIESTESS
	40,	// CID_TCHECKRIK
	0,	// CID_BUTTON
	0,	// CID_BEE
	0,	// CID_CORVUS
	0,	// CID_MORK
	100,// CID_TBEAST
	0,	// CID_IMP

	0,	// CID_SSITHRA_VICTIM
	0,	// CID_SSITHRA_SCOUT
	0,	// CID_DRANOR

	0,	// CID_TRIG_DAMAGE
	0,	// CID_TRIG_PUSH
	0,	// CID_C_ELFLORD
	0,	// CID_C_SIERNAN1
	0,	// CID_C_SIERNAN2
	0,	// CID_C_HIGHPRIESTESS
	0,	// CID_C_HIGHPRIESTESS2
	0,	// CID_C_TOME
	0,	// CID_C_MORCALAVIN
	0,	// CID_CORVUS2
	0,	// CID_CORVUS3
	0,	// CID_CORVUS4
	0,	// CID_CORVUS5
	0,	// CID_CORVUS6
	0,	// CID_CORVUS7
	0,	// CID_CORVUS8
	0,	// CID_CORVUS9
};

struct MonsterShadow_s G_MonsterShadow[NUM_CLASSIDS] =
{
	// use_shadow	// scale
	{ false,		0.0f },	// CID_NONE
	{ true,			0.75f },// CID_RAT
	{ true,			1.0f },	// CID_GORGON
	{ true,			1.5f },	// CID_PLAGUEELF
	{ true,			1.0f },	// CID_GKROKON
	{ false,		0.0f },	// CID_FISH
	{ false,		0.0f },	// CID_OBJECT
	{ false,		0.0f },	// CID_LIGHT
	{ false,		0.0f },	// CID_TRIGGER
	{ false,		0.0f },	// CID_HARPY
	{ true,			1.0f },	// CID_SPREADER
	{ false,		0.0f },	// CID_ELFLORD
	{ false,		0.0f },	// CID_BBRUSH
	{ false,		0.0f },	// CID_FUNC_ROTATE
	{ false,		0.0f },	// CID_FUNC_DOOR
	{ true,			1.0f },	// CID_CHICKEN
	{ true,			1.0f },	// CID_SSITHRA
	{ false,		0.0f },	// CID_SPELL
	{ true,			1.0f },	// CID_MSSITHRA
	{ true,			1.0f },	// CID_OGLE
	{ true,			1.0f },	// CID_SERAPH_OVERLORD
	{ true,			1.0f },	// CID_SERAPH_GUARD
	{ true,			1.0f },	// CID_ASSASSIN
	{ false,		0.0f },	// CID_TELEPORTER
	{ true,			1.0f },	// CID_HIGHPRIESTESS
	{ true,			1.0f },	// CID_TCHECKRIK
	{ false,		0.0f },	// CID_BUTTON
	{ false,		0.0f },	// CID_BEE
	{ false,		0.0f },	// CID_CORVUS
	{ true,			1.0f },	// CID_MORK
	{ true,			1.0f },	// CID_TBEAST
	{ false,		0.0f },	// CID_IMP

	{ false,		0.0f },	// CID_SSITHRA_VICTIM
	{ false,		0.0f },	// CID_SSITHRA_SCOUT
	{ false,		0.0f },	// CID_DRANOR

	{ false,		0.0f },	// CID_TRIG_DAMAGE
	{ false,		0.0f },	// CID_TRIG_PUSH
	{ false,		0.0f },	// CID_C_ELFLORD
	{ false,		0.0f },	// CID_C_SIERNAN1
	{ false,		0.0f },	// CID_C_SIERNAN2
	{ false,		0.0f },	// CID_C_HIGHPRIESTESS
	{ false,		0.0f },	// CID_C_HIGHPRIESTESS2
	{ false,		0.0f },	// CID_C_TOME
	{ false,		0.0f },	// CID_C_MORCALAVIN
	{ false,		0.0f },	// CID_CORVUS2
	{ false,		0.0f },	// CID_CORVUS3
	{ false,		0.0f },	// CID_CORVUS4
	{ false,		0.0f },	// CID_CORVUS5
	{ false,		0.0f },	// CID_CORVUS6
	{ false,		0.0f },	// CID_CORVUS7
	{ false,		0.0f },	// CID_CORVUS8
	{ false,		0.0f },	// CID_CORVUS9
};

vec3_t STDMinsForClass[NUM_CLASSIDS] =
{
	{  0.0f,	 0.0f,	 0.0f },	// CID_NONE
	{ -8.0f,	-8.0f,	 0.0f },	// CID_RAT
	{ -16.0f,	-16.0f,	-16.0f },	// CID_GORGON
	{ -16.0f,	-16.0f,	-32.0f },	// CID_PLAGUEELF
	{ -20.0f,	-20.0f,	 0.0f },	// CID_GKROKON
	{  0.0f,	 0.0f,	 0.0f },	// CID_FISH
	{  0.0f,	 0.0f,	 0.0f },	// CID_OBJECT
	{  0.0f,	 0.0f,	 0.0f },	// CID_LIGHT
	{  0.0f,	 0.0f,	 0.0f },	// CID_TRIGGER
	{ -16.0f,	-16.0f,	-12.0f },	// CID_HARPY
	{ -16.0f,	-16.0f,	-40.0f },	// CID_SPREADER
	{  0.0f,	 0.0f,	 0.0f },	// CID_ELFLORD
	{  0.0f,	 0.0f,	 0.0f },	// CID_BBRUSH
	{  0.0f,	 0.0f,	 0.0f },	// CID_FUNC_ROTATE
	{  0.0f,	 0.0f,	 0.0f },	// CID_FUNC_DOOR
	{  0.0f,	 0.0f,	 0.0f },	// CID_CHICKEN
	{ -16.0f,	-16.0f,	-26.0f },	// CID_SSITHRA
	{  0.0f,	 0.0f,	 0.0f },	// CID_SPELL
	{ -36.0f,	-36.0f,	 0.0f },	// CID_MSSITHRA
	{ -16.0f,	-16.0f,	-24.0f },	// CID_OGLE
	{ -24.0f,	-24.0f,	-34.0f },	// CID_SERAPH_OVERLORD
	{ -24.0f,	-24.0f,	-34.0f },	// CID_SERAPH_GUARD
	{ -16.0f,	-16.0f,	-32.0f },	// CID_ASSASSIN
	{  0.0f,	 0.0f,	 0.0f },	// CID_TELEPORTER
	{  0.0f,	 0.0f,	 0.0f },	// CID_HIGHPRIESTESS
	{ -16.0f,	-16.0f,	-32.0f },	// CID_TCHECKRIK
	{  0.0f,	 0.0f,	 0.0f },	// CID_BUTTON
	{  0.0f,	 0.0f,	 0.0f },	// CID_BEE
	{  0.0f,	 0.0f,	 0.0f },	// CID_CORVUS
	{  0.0f,	 0.0f,	 0.0f },	// CID_MORK
	{  0.0f,	 0.0f,	 0.0f },	// CID_TBEAST
	{ -16.0f,	-16.0f,	 0.0f },	// CID_IMP

	{ 0.0f, 0.0f, 0.0f },	// CID_SSITHRA_VICTIM
	{ 0.0f, 0.0f, 0.0f },	// CID_SSITHRA_SCOUT
	{ 0.0f, 0.0f, 0.0f },	// CID_DRANOR

	{ 0.0f, 0.0f, 0.0f },	// CID_TRIG_DAMAGE
	{ 0.0f, 0.0f, 0.0f },	// CID_TRIG_PUSH
	{ 0.0f, 0.0f, 0.0f },	// CID_C_ELFLORD
	{ 0.0f, 0.0f, 0.0f },	// CID_C_SIERNAN1
	{ 0.0f, 0.0f, 0.0f },	// CID_C_SIERNAN2
	{ 0.0f, 0.0f, 0.0f },	// CID_C_HIGHPRIESTESS
	{ 0.0f, 0.0f, 0.0f },	// CID_C_HIGHPRIESTESS2
	{ 0.0f, 0.0f, 0.0f },	// CID_C_TOME
	{ 0.0f, 0.0f, 0.0f },	// CID_C_MORCALAVIN
	{ 0.0f, 0.0f, 0.0f },	// CID_CORVUS2
	{ 0.0f, 0.0f, 0.0f },	// CID_CORVUS3
	{ 0.0f, 0.0f, 0.0f },	// CID_CORVUS4
	{ 0.0f, 0.0f, 0.0f },	// CID_CORVUS5
	{ 0.0f, 0.0f, 0.0f },	// CID_CORVUS6
	{ 0.0f, 0.0f, 0.0f },	// CID_CORVUS7
	{ 0.0f, 0.0f, 0.0f },	// CID_CORVUS8
	{ 0.0f, 0.0f, 0.0f },	// CID_CORVUS9
};

vec3_t STDMaxsForClass[NUM_CLASSIDS] =
{
	{ 0.0f,		0.0f,	0.0f },	//	CID_NONE
	{ 8.0f,		8.0f,	16.0f },//	CID_RAT
	{ 16.0f,	16.0f,	16.0f },//	CID_GORGON
	{ 16.0f,	16.0f,	27.0f },//	CID_PLAGUEELF
	{ 20.0f,	20.0f,	32.0f },//	CID_GKROKON
	{ 0.0f,		0.0f,	0.0f },	//	CID_FISH
	{ 0.0f,		0.0f,	0.0f },	//	CID_OBJECT
	{ 0.0f,		0.0f,	0.0f },	//	CID_LIGHT
	{ 0.0f,		0.0f,	0.0f },	//	CID_TRIGGER
	{ 16.0f,	16.0f,	12.0f },//	CID_HARPY
	{ 16.0f,	16.0f,	24.0f },//	CID_SPREADER
	{ 0.0f,		0.0f,	0.0f },	//	CID_ELFLORD
	{ 0.0f,		0.0f,	0.0f },	//	CID_BBRUSH
	{ 0.0f,		0.0f,	0.0f },	//	CID_FUNC_ROTATE
	{ 0.0f,		0.0f,	0.0f },	//	CID_FUNC_DOOR
	{ 0.0f,		0.0f,	0.0f },	//	CID_CHICKEN
	{ 16.0f,	16.0f,	26.0f },//	CID_SSITHRA
	{ 0.0f,		0.0f,	0.0f },	//	CID_SPELL
	{ 36.0f,	36.0f,	96.0f },//	CID_MSSITHRA
	{ 16.0f,	16.0f,	16.0f },//	CID_OGLE
	{ 24.0f,	24.0f,	34.0f },//	CID_SERAPH_OVERLORD
	{ 24.0f,	24.0f,	34.0f },//	CID_SERAPH_GUARD
	{ 16.0f,	16.0f,	48.0f },//	CID_ASSASSIN
	{ 0.0f,		0.0f,	0.0f },	//	CID_TELEPORTER
	{ 0.0f,		0.0f,	0.0f },	//	CID_HIGHPRIESTESS
	{ 16.0f,	16.0f,	32.0f },//	CID_TCHECKRIK
	{ 0.0f,		0.0f,	0.0f },	//	CID_BUTTON
	{ 0.0f,		0.0f,	0.0f },	//	CID_BEE
	{ 0.0f,		0.0f,	0.0f },	//	CID_CORVUS
	{ 0.0f,		0.0f,	0.0f },	//	CID_MORK
	{ 0.0f,		0.0f,	0.0f },	//	CID_TBEAST
	{ 16.0f,	16.0f,	32.0f },//	CID_IMP 

	{ 0.0f, 0.0f, 0.0f },	// CID_SSITHRA_VICTIM
	{ 0.0f, 0.0f, 0.0f },	// CID_SSITHRA_SCOUT
	{ 0.0f, 0.0f, 0.0f },	// CID_DRANOR

	{ 0.0f, 0.0f, 0.0f },	// CID_TRIG_DAMAGE
	{ 0.0f, 0.0f, 0.0f },	// CID_TRIG_PUSH
	{ 0.0f, 0.0f, 0.0f },	// CID_C_ELFLORD
	{ 0.0f, 0.0f, 0.0f },	// CID_C_SIERNAN1
	{ 0.0f, 0.0f, 0.0f },	// CID_C_SIERNAN2
	{ 0.0f, 0.0f, 0.0f },	// CID_C_HIGHPRIESTESS
	{ 0.0f, 0.0f, 0.0f },	// CID_C_HIGHPRIESTESS2
	{ 0.0f, 0.0f, 0.0f },	// CID_C_TOME
	{ 0.0f, 0.0f, 0.0f },	// CID_C_MORCALAVIN
	{ 0.0f, 0.0f, 0.0f },	// CID_CORVUS2
	{ 0.0f, 0.0f, 0.0f },	// CID_CORVUS3
	{ 0.0f, 0.0f, 0.0f },	// CID_CORVUS4
	{ 0.0f, 0.0f, 0.0f },	// CID_CORVUS5
	{ 0.0f, 0.0f, 0.0f },	// CID_CORVUS6
	{ 0.0f, 0.0f, 0.0f },	// CID_CORVUS7
	{ 0.0f, 0.0f, 0.0f },	// CID_CORVUS8
	{ 0.0f, 0.0f, 0.0f },	// CID_CORVUS9
};