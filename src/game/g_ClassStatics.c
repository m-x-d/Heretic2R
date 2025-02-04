//
// g_ClassStatics.c
//
// Copyright 1998 Raven Software
//

#include "g_ClassStatics.h"
#include "m_assassin.h" //mxd
#include "m_beast.h" //mxd
#include "m_bee.h" //mxd
#include "m_chicken.h" //mxd
#include "m_elflord.h" //mxd

G_ClassStatics_t classStatics[NUM_CLASSIDS];
int Cid_init[NUM_CLASSIDS];

void RatStaticsInit();
void GorgonStaticsInit();
void PlagueElfStaticsInit();
void GkrokonStaticsInit();
void FishStaticsInit();
void ObjectStaticsInit();
void LightStaticsInit();
void TriggerStaticsInit();
void HarpyStaticsInit();
void SpreaderStaticsInit();
void BBrushStaticsInit();
void FuncRotateStaticsInit();
void FuncDoorStaticsInit();
void SsithraStaticsInit();
void MssithraStaticsInit();
void OgleStaticsInit();
void SeraphOverlordStaticsInit();
void SeraphGuardStaticsInit();
void TeleporterStaticsInit();
void HighPriestessStaticsInit();
void TcheckrikStaticsInit();
void ButtonStaticsInit();
void LeverStaticsInit();
void FlameThrowerStaticsInit();
void CorvusStaticsInit();
void morcalavinStaticsInit();
void ImpStaticsInit();
void MotherStaticsInit();
void victimSsithraStaticsInit();
void SsithrascoutStaticsInit();
void DranorStaticsInit();
void TrigDamageStaticsInit();
void TrigPushStaticsInit();
void ElflordCinStaticsInit();
void PriestessCStaticsInit();
void Priestess2CStaticsInit();
void Siernan1CinStaticsInit();
void Siernan2CinStaticsInit();
void TomeStaticsInit();
void MorcalavinCinStaticsInit();
void Corvus2StaticsInit();
void Corvus3StaticsInit();
void Corvus4StaticsInit();
void Corvus5StaticsInit();
void Corvus6StaticsInit();
void Corvus7StaticsInit();
void Corvus8StaticsInit();
void Corvus9StaticsInit();

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
	CorvusStaticsInit,
	morcalavinStaticsInit,
	TBeastStaticsInit,
	ImpStaticsInit,
	LeverStaticsInit,
	FlameThrowerStaticsInit,

	MotherStaticsInit,
	victimSsithraStaticsInit,
	SsithrascoutStaticsInit,
	DranorStaticsInit,
	TrigDamageStaticsInit,
	TrigPushStaticsInit,
	ElflordCinStaticsInit,
	Siernan1CinStaticsInit,
	Siernan2CinStaticsInit,
	PriestessCStaticsInit,
	Priestess2CStaticsInit,
	TomeStaticsInit,
	MorcalavinCinStaticsInit,
	Corvus2StaticsInit,
	Corvus3StaticsInit,
	Corvus4StaticsInit,
	Corvus5StaticsInit,
	Corvus6StaticsInit,
	Corvus7StaticsInit,
	Corvus8StaticsInit,
	Corvus9StaticsInit,
};