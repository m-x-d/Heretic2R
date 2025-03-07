//
// g_ClassStatics.c
//
// Copyright 1998 Raven Software
//

#include "g_ClassStatics.h"
#include "c_actors.h" //mxd
#include "g_breakable.h" //mxd
#include "g_field.h" //mxd
#include "g_flamethrower.h" //mxd
#include "g_misc.h" //mxd
#include "m_monsters.h" //mxd

G_ClassStatics_t classStatics[NUM_CLASSIDS];
int Cid_init[NUM_CLASSIDS];

void ObjectStaticsInit();
void LightStaticsInit();
void TriggerStaticsInit();
void FuncRotateStaticsInit();
void FuncDoorStaticsInit();
void ButtonStaticsInit();
void LeverStaticsInit();

void (*classStaticsInits[NUM_CLASSIDS])(void) = 
{
	NULL,
	RatStaticsInit,
	GorgonStaticsInit,
	PlagueElfStaticsInit,
	GkrokonStaticsInit,
	FishStaticsInit,
	ObjectStaticsInit,
	LightStaticsInit,
	TriggerStaticsInit,
	HarpyStaticsInit,
	SpreaderStaticsInit,
	ElflordStaticsInit,
	BBrushStaticsInit,
	FuncRotateStaticsInit,
	FuncDoorStaticsInit,
	ChickenStaticsInit,
	SsithraStaticsInit,
	NULL,
	MssithraStaticsInit,
	OgleStaticsInit,
	SeraphOverlordStaticsInit,
	SeraphGuardStaticsInit,
	AssassinStaticsInit,
	MiscTeleporterStaticsInit,
	HighPriestessStaticsInit,
	TcheckrikStaticsInit,
	ButtonStaticsInit,
	BeeStaticsInit,
	Corvus1CinStaticsInit,
	MorcalavinStaticsInit,
	TBeastStaticsInit,
	ImpStaticsInit,
	LeverStaticsInit,
	FlamethrowerStaticsInit,

	MotherStaticsInit,
	VictimSsithraCinStaticsInit,
	SsithraScoutCinStaticsInit,
	DranorCinStaticsInit,
	TriggerDamageStaticsInit,
	TriggerPushStaticsInit,
	ElflordCinStaticsInit,
	Siernan1CinStaticsInit,
	Siernan2CinStaticsInit,
	PriestessCinStaticsInit,
	Priestess2CinStaticsInit,
	TomeCinStaticsInit,
	MorcalavinCinStaticsInit,
	Corvus2CinStaticsInit,
	Corvus3CinStaticsInit,
	Corvus4CinStaticsInit,
	Corvus5CinStaticsInit,
	Corvus6CinStaticsInit,
	Corvus7CinStaticsInit,
	Corvus8CinStaticsInit,
	Corvus9CinStaticsInit,
};