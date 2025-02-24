//
// g_ClassStatics.c
//
// Copyright 1998 Raven Software
//

#include "g_ClassStatics.h"
#include "c_actors.h" //mxd
#include "m_monsters.h" //mxd

G_ClassStatics_t classStatics[NUM_CLASSIDS];
int Cid_init[NUM_CLASSIDS];

void ObjectStaticsInit();
void LightStaticsInit();
void TriggerStaticsInit();
void BBrushStaticsInit();
void FuncRotateStaticsInit();
void FuncDoorStaticsInit();
void TeleporterStaticsInit();
void ButtonStaticsInit();
void LeverStaticsInit();
void FlameThrowerStaticsInit();
void TrigDamageStaticsInit();
void TrigPushStaticsInit();

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
	TeleporterStaticsInit,
	HighPriestessStaticsInit,
	TcheckrikStaticsInit,
	ButtonStaticsInit,
	BeeStaticsInit,
	Corvus1CinStaticsInit,
	MorcalavinStaticsInit,
	TBeastStaticsInit,
	ImpStaticsInit,
	LeverStaticsInit,
	FlameThrowerStaticsInit,

	MotherStaticsInit,
	VictimSsithraCinStaticsInit,
	SsithraScoutCinStaticsInit,
	DranorCinStaticsInit,
	TrigDamageStaticsInit,
	TrigPushStaticsInit,
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