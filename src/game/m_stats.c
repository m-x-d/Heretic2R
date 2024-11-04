#include "m_stats.h"

/*
int AttackRangesForClass [NUM_ATTACKRANGES] =

melee_range = How close an enemy must be to use melee attack.
	If negative, how much distance the monster tries to keep 
	between itself and an enemy.
missile_range = Maximum distance a monster will try to attack
	from using it's ranged attack.
min_missile_range = min range at which a monster will not use
	it's missile attack (too close)
bypass_missile_chance = 0 - 100 chance of a monster not taking
	a shot even when it can, otherwise it will close in.
*/

int AttackRangesForClass [NUM_ATTACKRANGES] =
{
//melee_rng,missile_rng,min_msl_rng,bypass_missile_chance
	0,		0,			0,			0,//	CID_NONE, -- NA - 0
	0,		0,			0,			0,//	CID_RAT, -- JW
	72,		150,		50,			20,//	CID_GORGON, -- MG
	50,		0,			0,			0,//	CID_PLAGUEELF, -- MG
	0,		256,		48,			0,//	CID_GKROKON, -- JW
	0,		0,			0,			0,//	CID_FISH, -- ? - 5
	0,		0,			0,			0,//	CID_OBJECT, -- NA
	0,		0,			0,			0,//	CID_LIGHT, -- NA
	0,		0,			0,			0,//	CID_TRIGGER, -- NA
	0,		0,			0,			0,//	CID_HARPY, -- JW
	100,	512,		200,		50,//	CID_SPREADER, -- JW - 10
	0,		0,			0,			0,//	CID_ELFLORD, -- JW
	0,		0,			0,			0,//	CID_BBRUSH, -- NA
	0,		0,			0,			0,//	CID_FUNC_ROTATE, -- NA
	0,		0,			0,			0,//	CID_FUNC_DOOR, -- NA
	0,		0,			0,			0,//	CID_CHICKEN,
	64,		512,		64,			50,//	CID_SSITHRA, -- MG
	0,		0,			0,			0,//	CID_SPELL, -- NA - 15
	100,	800,		100,		25,//	CID_MSSITHRA, -- MG
	48,		0,			0,			0,//	CID_OGLE, -- JW
	100,	0,			0,			0,//	CID_SERAPH_OVERLORD, -- JW
	100,	1024,		64,			85,//	CID_SERAPH_GUARD, -- JW
	48,		1024,		64,			10,//	CID_ASSASSIN, -- MG - 20
	0,		0,			0,			0,//	CID_TELEPORTER, -- NA
	0,		0,			0,			0,//	CID_HIGHPRIESTESS,
	-72,	512,		48,			0,//	CID_TCHECKRIK, -- MG
	0,		0,			0,			0,//	CID_BUTTON, -- NA
	0,		0,			0,			0,//	CID_BEE, -- ? - 25
	0,		0,			0,			0,//	CID_CORVUS,
	64,		1024,		64,			25,//	CID_MORK,
	400,	1500,		100,		77,//	CID_TBEAST,//30
	-64,	1024,		32,			20,//	CID_IMP, 

	0,		0,			0,			0,//	CID_SSITHRA_VICTIM,	//32
	0,		0,			0,			0,//	CID_SSITHRA_SCOUT,	//33
	0,		0,			0,			0,//	CID_DRANOR,			//34

	0,		0,			0,			0,//	CID_TRIG_DAMAGE,	//35
	0,		0,			0,			0,//	CID_TRIG_PUSH,		//36
	0,		0,			0,			0,//	CID_C_ELFLORD,		//37
	0,		0,			0,			0,//	CID_C_SIERNAN1,		//38
	0,		0,			0,			0,//	CID_C_SIERNAN2,		//39
	0,		0,			0,			0,//	CID_C_HIGHPRIESTESS,//40
	0,		0,			0,			0,//	CID_C_HIGHPRIESTESS2,//41
	0,		0,			0,			0,//	CID_C_TOME,			//42
	0,		0,			0,			0,//	CID_C_MORCALAVIN,	//43
	0,		0,			0,			0,//	CID_CORVUS2,		//44
	0,		0,			0,			0,//	CID_CORVUS3,		//45
	0,		0,			0,			0,//	CID_CORVUS4,		//46
	0,		0,			0,			0,//	CID_CORVUS5,		//47
	0,		0,			0,			0,//	CID_CORVUS6,		//48
	0,		0,			0,			0,//	CID_CORVUS7,		//49
	0,		0,			0,			0,//	CID_CORVUS8,		//50
	0,		0,			0,			0,//	CID_CORVUS9,		//51
};

/*
byte *MaxBuoysForClass[NUM_CLASSIDS]

  MG

  This table determines how many buoys a monster will search
  through in a line before it goes back and tries a different
  buoy path.  Helps cut down the hit of searching through
  buoys and helps find the shorter path if there is one.
*/
byte MaxBuoysForClass[NUM_CLASSIDS] =
{
	0,//	CID_NONE,
	8,//	CID_RAT,
	20,//	CID_GORGON,
	16,//	CID_PLAGUEELF,
	12,//	CID_GKROKON,
	6,//	CID_FISH,
	0,//	CID_OBJECT,
	0,//	CID_LIGHT,
	0,//	CID_TRIGGER,
	14,//	CID_HARPY,
	24,//	CID_SPREADER,
	0,//	CID_ELFLORD,
	0,//	CID_BBRUSH,
	0,//	CID_FUNC_ROTATE, -- NA
	0,//	CID_FUNC_DOOR, -- NA
	12,//	CID_CHICKEN,
	28,//	CID_SSITHRA,
	0,//	CID_SPELL,
	16,//	CID_MSSITHRA,
	8,//	CID_OGLE,
	24,//	CID_SERAPH_OVERLORD,
	20,//	CID_SERAPH_GUARD,
	64,//	CID_ASSASSIN,
	0,//	CID_TELEPORTER,
	16,//	CID_HIGHPRIESTESS,
	32,//	CID_TCHECKRIK,
	0,//	CID_BUTTON,
	6,//	CID_BEE,
	0,//	CID_CORVUS,
	40,//	CID_MORK,
	100,//	CID_BEAST,
	14,//	CID_IMP,

	0,//	CID_SSITHRA_VICTIM,		//32
	0,//	CID_SSITHRA_SCOUT,		//33
	0,//	CID_DRANOR,				//34

	0,//	CID_TRIG_DAMAGE,	//35
	0,//	CID_TRIG_PUSH,		//36
	0,//	CID_C_ELFLORD,		//37
	0,//	CID_C_SIERNAN1,		//38
	0,//	CID_C_SIERNAN2,		//39
	0,//	CID_C_HIGHPRIESTESS,//40
	0,//	CID_C_HIGHPRIESTESS2,//41
	0,//	CID_C_TOME,			//42
	0,//	CID_C_MORCALAVIN,	//43
	0,//	CID_CORVUS2,		//44
	0,//	CID_CORVUS3,		//45
	0,//	CID_CORVUS4,		//46
	0,//	CID_CORVUS5,		//47
	0,//	CID_CORVUS6,		//48
	0,//	CID_CORVUS7,		//49
	0,//	CID_CORVUS8,		//50
	0,//	CID_CORVUS9,		//51
};

/*
int JumpChanceForClass[NUM_CLASSIDS] =

  MG

  0 - 100 chance that a monster will
  check and see if it can jump when it
  hits a ledge
*/
int JumpChanceForClass[NUM_CLASSIDS] =
{
	0,//	CID_NONE,
	100,//	CID_RAT,
	80,//	CID_GORGON,
	50,//	CID_PLAGUEELF,
	100,//	CID_GKROKON,
	0,//	CID_FISH,
	0,//	CID_OBJECT,
	0,//	CID_LIGHT,
	0,//	CID_TRIGGER,
	0,//	CID_HARPY,
	30,//	CID_SPREADER,
	0,//	CID_ELFLORD,
	0,//	CID_BBRUSH,
	0,//	CID_FUNC_ROTATE, -- NA
	0,//	CID_FUNC_DOOR, -- NA
	100,//	CID_CHICKEN,
	100,//	CID_SSITHRA,
	0,//	CID_SPELL,
	25,//	CID_MSSITHRA,
	10,//	CID_OGLE,
	30,//	CID_SERAPH_OVERLORD,
	20,//	CID_SERAPH_GUARD,
	100,//	CID_ASSASSIN,
	0,//	CID_TELEPORTER,
	0,//	CID_HIGHPRIESTESS,
	40,//	CID_TCHECKRIK,
	0,//	CID_BUTTON,
	0,//	CID_BEE,
	0,//	CID_CORVUS,
	0,//	CID_MORK,
	100,//	CID_TBEAST,
	0,//	CID_IMP,

	0,//	CID_SSITHRA_VICTIM,		//32
	0,//	CID_SSITHRA_SCOUT,		//33
	0,//	CID_DRANOR,				//34

	0,//	CID_TRIG_DAMAGE,	//35
	0,//	CID_TRIG_PUSH,		//36
	0,//	CID_C_ELFLORD,		//37
	0,//	CID_C_SIERNAN1,		//38
	0,//	CID_C_SIERNAN2,		//39
	0,//	CID_C_HIGHPRIESTESS,//40
	0,//	CID_C_HIGHPRIESTESS2,//41
	0,//	CID_C_TOME,			//42
	0,//	CID_C_MORCALAVIN,	//43
	0,//	CID_CORVUS2,		//44
	0,//	CID_CORVUS3,		//45
	0,//	CID_CORVUS4,		//46
	0,//	CID_CORVUS5,		//47
	0,//	CID_CORVUS6,		//48
	0,//	CID_CORVUS7,		//49
	0,//	CID_CORVUS8,		//50
	0,//	CID_CORVUS9,		//51
};

struct MonsterShadow_s	G_MonsterShadow[NUM_CLASSIDS] =
{
//useShadow	//scale
	0,		0,		//	CID_NONE
	1,		0.75,	//	CID_RAT
	1,		1.0,	//	CID_GORGON
	1,		1.5,	//	CID_PLAGUEELF
	1,		1.0,	//	CID_GKROKON
	0,		0.0,	//	CID_FISH
	0,		0.0,	//	CID_OBJECT
	0,		0.0,	//	CID_LIGHT
	0,		0.0,	//	CID_TRIGGER
	0,		0.0,	//	CID_HARPY
	1,		1.0,	//	CID_SPREADER
	0,		0.0,	//	CID_ELFLORD
	0,		0.0,	//	CID_BBRUSH
	0,		0.0,	//	CID_FUNC_ROTATE, -- NA
	0,		0.0,	//	CID_FUNC_DOOR, -- NA
	1,		1.0,	//	CID_CHICKEN
	1,		1.0,	//	CID_SSITHRA
	0,		0.0,	//	CID_SPELL
	1,		1.0,	//	CID_MSSITHRA
	1,		1.0,	//	CID_OGLE
	1,		1.0,	//	CID_SERAPH_OVERLORD
	1,		1.0,	//	CID_SERAPH_GUARD
	1,		1.0,	//	CID_ASSASSIN
	0,		0.0,	//	CID_TELEPORTER
	1,		1.0,	//	CID_HIGHPRIESTESS
	1,		1.0,	//	CID_TCHECKRIK
	0,		0.0,	//	CID_BUTTON
	0,		0.0,	//	CID_BEE,
	0,		0.0,	//	CID_CORVUS,
	1,		1.0,	//	CID_MORK,
	1,		1.0,	//	CID_TBEAST,
	0,		0.0,	//	CID_IMP

	0,		0.0,	//	CID_SSITHRA_VICTIM,		//32
	0,		0.0,	//	CID_SSITHRA_SCOUT,		//33
	0,		0.0,	//	CID_DRANOR,				//34

	0,		0.0,	//	CID_TRIG_DAMAGE,	//35
	0,		0.0,	//	CID_TRIG_PUSH,		//36
	0,		0.0,	//	CID_C_ELFLORD,		//37
	0,		0.0,	//	CID_C_SIERNAN1,		//38
	0,		0.0,	//	CID_C_SIERNAN2,		//39
	0,		0.0,	//	CID_C_HIGHPRIESTESS,//40
	0,		0.0,	//	CID_C_HIGHPRIESTESS2,//41
	0,		0.0,	//	CID_C_TOME,			//42
	0,		0.0,	//	CID_C_MORCALAVIN,	//43
	0,		0.0,	//	CID_CORVUS2,		//44
	0,		0.0,	//	CID_CORVUS3,		//45
	0,		0.0,	//	CID_CORVUS4,		//46
	0,		0.0,	//	CID_CORVUS5,		//47
	0,		0.0,	//	CID_CORVUS6,		//48
	0,		0.0,	//	CID_CORVUS7,		//49
	0,		0.0,	//	CID_CORVUS8,		//50
	0,		0.0,	//	CID_CORVUS9,		//51
};


vec3_t STDMinsForClass [NUM_CLASSIDS] =
{
//melee_rng,missile_rng,min_msl_rng,bypass_missile_chance
	0,		0,			0,//	CID_NONE, -- NA - 0
	-8,		-8,			0,//	CID_RAT, -- JW
	-16,	-16,		-16,//	CID_GORGON, -- MG
	-16,	-16,		-32,//	CID_PLAGUEELF, -- MG
	-20.0,	-20.0,		0.0,//	CID_GKROKON, -- JW
	0,		0,			0,//	CID_FISH, -- ? - 5
	0,		0,			0,//	CID_OBJECT, -- NA
	0,		0,			0,//	CID_LIGHT, -- NA
	0,		0,			0,//	CID_TRIGGER, -- NA
	-16,	-16,		-12,//	CID_HARPY, -- JW
	-16,	-16,		-40,//	CID_SPREADER, -- JW - 10
	0,		0,			0,//	CID_ELFLORD, -- JW
	0,		0,			0,//	CID_BBRUSH, -- NA
	0,		0,			0,//	CID_FUNC_ROTATE, -- NA
	0,		0,			0,//	CID_FUNC_DOOR, -- NA
	0,		0,			0,//	CID_CHICKEN,
	-16,	-16,		-26,//	CID_SSITHRA, -- MG
	0,		0,			0,//	CID_SPELL, -- NA - 15
	-36,	-36,		0,//	CID_MSSITHRA, -- MG
	-16,	-16,		-24,//	CID_OGLE, -- JW
	-24,	-24,		-34,//	CID_SERAPH_OVERLORD, -- JW
	-24,	-24,		-34,//	CID_SERAPH_GUARD, -- JW
	-16,	-16,		-32,//	CID_ASSASSIN, -- MG - 20
	0,		0,			0,//	CID_TELEPORTER, -- NA
	0,		0,			0,//	CID_HIGHPRIESTESS,
	-16,	-16,		-32,//	CID_TCHECKRIK, -- MG
	0,		0,			0,//	CID_BUTTON, -- NA
	0,		0,			0,//	CID_BEE, -- ? - 25
	0,		0,			0,//	CID_CORVUS,
	0,		0,			0,//	CID_MORK,
	0,		0,			0,//	CID_TBEAST,//30
	-16,	-16,		0,//	CID_IMP, 

	0,		0,			0,//	CID_SSITHRA_VICTIM,	//32
	0,		0,			0,//	CID_SSITHRA_SCOUT,	//33
	0,		0,			0,//	CID_DRANOR,			//34

	0,		0,			0,			//	CID_TRIG_DAMAGE,	//35
	0,		0,			0,			//	CID_TRIG_PUSH,		//36
	0,		0,			0,			//	CID_C_ELFLORD,		//37
	0,		0,			0,			//	CID_C_SIERNAN1,		//38
	0,		0,			0,			//	CID_C_SIERNAN2,		//39
	0,		0,			0,			//	CID_C_HIGHPRIESTESS,//40
	0,		0,			0,			//	CID_C_HIGHPRIESTESS2,//41
	0,		0,			0,			//	CID_C_TOME,			//42
	0,		0,			0,			//	CID_C_MORCALAVIN,	//43
	0,		0,			0,			//	CID_CORVUS2,		//44
	0,		0,			0,			//	CID_CORVUS3,		//45
	0,		0,			0,			//	CID_CORVUS4,		//46
	0,		0,			0,			//	CID_CORVUS5,		//47
	0,		0,			0,			//	CID_CORVUS6,		//48
	0,		0,			0,			//	CID_CORVUS7,		//49
	0,		0,			0,			//	CID_CORVUS8,		//50
	0,		0,			0,			//	CID_CORVUS9,		//51
};

vec3_t STDMaxsForClass [NUM_CLASSIDS] =
{
//melee_rng,missile_rng,min_msl_rng,bypass_missile_chance
	0,		0,			0,//	CID_NONE, -- NA - 0
	8,		8,			16,//	CID_RAT, -- JW
	16,		16,			16,//	CID_GORGON, -- MG
	16,		16,			27,//	CID_PLAGUEELF, -- MG
	20,		20,			32,//	CID_GKROKON, -- JW
	0,		0,			0,//	CID_FISH, -- ? - 5
	0,		0,			0,//	CID_OBJECT, -- NA
	0,		0,			0,//	CID_LIGHT, -- NA
	0,		0,			0,//	CID_TRIGGER, -- NA
	16,		16,			12,//	CID_HARPY, -- JW
	16,		16,			24,//	CID_SPREADER, -- JW - 10
	0,		0,			0,//	CID_ELFLORD, -- JW
	0,		0,			0,//	CID_BBRUSH, -- NA
	0,		0,			0,//	CID_FUNC_ROTATE, -- NA
	0,		0,			0,//	CID_FUNC_DOOR, -- NA
	0,		0,			0,//	CID_CHICKEN,
	16,		16,			26,//	CID_SSITHRA, -- MG
	0,		0,			0,//	CID_SPELL, -- NA - 15
	36,		36,			96,//	CID_MSSITHRA, -- MG
	16,		16,			16,//	CID_OGLE, -- JW
	24,		24,			34,//	CID_SERAPH_OVERLORD, -- JW
	24,		24,			34,//	CID_SERAPH_GUARD, -- JW
	16,		16,			48,//	CID_ASSASSIN, -- MG - 20
	0,		0,			0,//	CID_TELEPORTER, -- NA
	0,		0,			0,//	CID_HIGHPRIESTESS,
	16,		16,			32,//	CID_TCHECKRIK, -- MG
	0,		0,			0,//	CID_BUTTON, -- NA
	0,		0,			0,//	CID_BEE, -- ? - 25
	0,		0,			0,//	CID_CORVUS,
	0,		0,			0,//	CID_MORK,
	0,		0,			0,//	CID_TBEAST,//30
	16,		16,			32,//	CID_IMP, 

	0,		0,			0,//	CID_SSITHRA_VICTIM,	//32
	0,		0,			0,//	CID_SSITHRA_SCOUT,	//33
	0,		0,			0,//	CID_DRANOR,			//34

	0,		0,			0,			//	CID_TRIG_DAMAGE,	//35
	0,		0,			0,			//	CID_TRIG_PUSH,		//36
	0,		0,			0,			//	CID_C_ELFLORD,		//37
	0,		0,			0,			//	CID_C_SIERNAN1,		//38
	0,		0,			0,			//	CID_C_SIERNAN2,		//39
	0,		0,			0,			//	CID_C_HIGHPRIESTESS,//40
	0,		0,			0,			//	CID_C_HIGHPRIESTESS2,//41
	0,		0,			0,			//	CID_C_TOME,			//42
	0,		0,			0,			//	CID_C_MORCALAVIN,	//43
	0,		0,			0,			//	CID_CORVUS2,		//44
	0,		0,			0,			//	CID_CORVUS3,		//45
	0,		0,			0,			//	CID_CORVUS4,		//46
	0,		0,			0,			//	CID_CORVUS5,		//47
	0,		0,			0,			//	CID_CORVUS6,		//48
	0,		0,			0,			//	CID_CORVUS7,		//49
	0,		0,			0,			//	CID_CORVUS8,		//50
	0,		0,			0,			//	CID_CORVUS9,		//51
};

//MISC GLOBALS

edict_t	*give_head_to_harpy = NULL;
edict_t *take_head_from = NULL;
