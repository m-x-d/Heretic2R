//
// fx_pespell.c
//
// Heretic II
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Client Entities.h"
#include "Particle.h"
#include "ResourceManager.h"
#include "FX.h"
#include "Vector.h"
#include "ce_DLight.h"
#include "random.h"
#include "Utilities.h"
#include "fx_debris.h"
#include "g_playstats.h"

#define SPELL_DELTA_FORWARD	8.0
#define SPELL_DELTA_THETA	0.12
#define	SPELL_SPIRAL_RAD	0.75
#define	SPELL_SCALE			0.25

#define	NUM_SPELL_MODELS	5

static struct model_s *spell_models[NUM_SPELL_MODELS];

void PrecachePESpell()
{
	spell_models[0] = fxi.RegisterModel("Sprites/Spells/flyingfist.sp2");
	spell_models[1] = fxi.RegisterModel("Sprites/Spells/spellhands_red.sp2");
	spell_models[2] = fxi.RegisterModel("Sprites/Spells/halo_ind.sp2");
	spell_models[3] = fxi.RegisterModel("Sprites/Spells/spark_ind.sp2");
	spell_models[4] = fxi.RegisterModel("Sprites/fx/core_b.sp2");
}

enum
{
	FX_PE_MAKE_SPELL,
	FX_PE_EXPLODE_SPELL,
	FX_PE_MAKE_SPELL2,
	FX_PE_EXPLODE_SPELL2,
	FX_PE_MAKE_SPELL3,
	FX_PE_EXPLODE_SPELL3,
};
// ************************************************************************************************
// FXPESpellTrailThink
// ************************************************************************************************

static qboolean FXPESpellTrailThink(struct client_entity_s *self, centity_t *owner)
{
	client_entity_t	*TrailEnt;
	vec3_t			accel_dir;
	int				i;

	self->updateTime = 20;

	if(self->SpawnInfo > 9)
		self->SpawnInfo--;

	i = GetScaledCount( irand(self->SpawnInfo >> 3, self->SpawnInfo >> 2), 0.8 );
	while(i--)
	{
		TrailEnt = ClientEntity_new(FX_PE_SPELL, 0, self->r.origin, NULL, 1000);
		TrailEnt->r.flags |= RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	
		VectorCopy(self->velocity, accel_dir);
		VectorNormalize(accel_dir);

		TrailEnt->r.model = spell_models;
		TrailEnt->r.scale = SPELL_SCALE + flrand(0.0, 0.05);

		TrailEnt->r.color.r = irand(40, 60);
		TrailEnt->r.color.g = irand(245, 255);
		TrailEnt->r.color.b = irand(95, 105);

		VectorRandomCopy(self->r.origin, TrailEnt->r.origin, flrand(-5.0, 5.0));
		VectorScale(accel_dir, flrand(-50.0, -400.0), TrailEnt->velocity);

		TrailEnt->d_alpha = flrand(-1.5, -2.0);
		TrailEnt->d_scale = flrand(-1.0, -1.25);
		TrailEnt->updateTime = (TrailEnt->alpha * 1000.0) / -TrailEnt->d_scale;
		TrailEnt->radius = 20.0;
		
		AddEffect(NULL,TrailEnt);
	}

	return(true);
}

// ************************************************************************************************
// FXPESpell
// ************************************************************************************************

////////////////////////////////////
// From CreateEffect FX_WEAPON_PESPELL
////////////////////////////////////
void FXPESpellGo(centity_t *owner, int type, int flags, vec3_t origin, vec3_t vel)
{
	vec3_t			dir;
	client_entity_t	*missile;	
	paletteRGBA_t	LightColor;
	float			lightsize;

	missile = ClientEntity_new(type, flags | CEF_DONT_LINK, origin, NULL, 100);

	missile->flags |= CEF_NO_DRAW;
	LightColor.c = 0xff20a0ff;		// Orange light
	lightsize = 120.0;
	
	VectorCopy(vel, missile->velocity);
	VectorNormalize2(vel, dir);
	AnglesFromDir(dir, missile->r.angles);

	missile->radius = 128;
	if(r_detail->value > DETAIL_NORMAL)
		missile->dlight = CE_DLight_new(LightColor, lightsize, 0.0f);
	missile->Update = FXPESpellTrailThink;

	missile->SpawnInfo = 32;

	AddEffect(owner, missile);

	fxi.S_StartSound(missile->r.origin, -1, CHAN_WEAPON, fxi.S_RegisterSound("monsters/plagueelf/spell.wav"), 
			1, ATTN_NORM, 0);
}



// ************************************************************************************************
// FXPESpellExplode
// ************************************************************************************************

///////////////////////////////////////
// From CreateEffect FX_WEAPON_PESPELLEXPLODE
///////////////////////////////////////
void FXPESpellExplode(centity_t *owner,int type,int flags,vec3_t origin, vec3_t dir)
{
	client_entity_t	*SmokePuff;
	int				i;
	paletteRGBA_t	LightColor;
	byte			powerup = 0;
	float			lightrad;
	
	if(flags & CEF_FLAG6)
	{
		FXClientScorchmark(origin, dir);
	}

	Vec3ScaleAssign(32.0, dir);

	i = GetScaledCount(irand(8, 12), 0.8);
	LightColor.c = 0xff20a0ff;
	lightrad = 150;

	while(i--)
	{
		if (!i)
			SmokePuff=ClientEntity_new(type,flags,origin,NULL,500);
		else
			SmokePuff=ClientEntity_new(type,flags,origin,NULL,1000);

		SmokePuff->r.model = spell_models + 1;
		SmokePuff->r.scale=flrand(0.8,1.6);
		SmokePuff->d_scale=-2.0;

		VectorRandomCopy(dir, SmokePuff->velocity, 64.0);
		SmokePuff->acceleration[0] = flrand(-200, 200);
		SmokePuff->acceleration[1] = flrand(-200, 200);
		SmokePuff->acceleration[2] = flrand(-40, -60);

		SmokePuff->r.flags |=RF_FULLBRIGHT|RF_TRANSLUCENT|RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		SmokePuff->r.color.r = irand(40, 60);
		SmokePuff->r.color.g = irand(245, 255);
		SmokePuff->r.color.b = irand(95, 105);

		SmokePuff->r.frame=0;

		SmokePuff->d_alpha= -0.4;
			
		SmokePuff->radius=20.0;

		if(!i)
		{//precache this?
			fxi.S_StartSound(SmokePuff->r.origin, -1, CHAN_WEAPON, fxi.S_RegisterSound("monsters/plagueelf/spellhit.wav"), 
					1, ATTN_NORM, 0);
			SmokePuff->dlight=CE_DLight_new(LightColor,lightrad,0.0f);
			VectorClear(SmokePuff->velocity);
		}	

		AddEffect(NULL,SmokePuff);
	}
}

//=================================================================================

// ************************************************************************************************
// FXPESpell2TrailThink
// ************************************************************************************************

static qboolean FXPESpell2TrailThink(struct client_entity_s *self, centity_t *owner)
{
	client_entity_t	*TrailEnt;
	vec3_t			accel_dir;
	int				i;

	self->updateTime = 20;

	if(self->SpawnInfo > 9)
		self->SpawnInfo--;

	i = GetScaledCount( irand(self->SpawnInfo >> 3, self->SpawnInfo >> 2), 0.8 );
	while(i--)
	{
		TrailEnt = ClientEntity_new(FX_PE_SPELL, 0, self->r.origin, NULL, 1000);
		TrailEnt->r.flags |= RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	
		VectorCopy(self->velocity, accel_dir);
		VectorNormalize(accel_dir);

		TrailEnt->r.model = spell_models + 2;
		TrailEnt->r.frame = irand(0,1);

		TrailEnt->r.scale = SPELL_SCALE + flrand(0.0, 0.05);

		/*TrailEnt->r.color.g = irand(40, 60);
		TrailEnt->r.color.b = irand(245, 255);
		TrailEnt->r.color.r = irand(95, 105);*/

		VectorRandomCopy(self->r.origin, TrailEnt->r.origin, flrand(-5.0, 5.0));
		VectorScale(accel_dir, flrand(-50.0, -400.0), TrailEnt->velocity);

		TrailEnt->d_alpha = flrand(-1.5, -2.0);
		TrailEnt->d_scale = flrand(-1.0, -1.25);
		TrailEnt->updateTime = (TrailEnt->alpha * 1000.0) / -TrailEnt->d_scale;
		TrailEnt->radius = 20.0;
		
		AddEffect(NULL,TrailEnt);
	}

	return(true);
}

// ************************************************************************************************
// FXPESpell2
// ************************************************************************************************

////////////////////////////////////
// From CreateEffect FX_WEAPON_PESPELL
////////////////////////////////////
void FXPESpell2Go(centity_t *owner, int type, int flags, vec3_t origin, vec3_t vel)
{
	vec3_t			dir;
	client_entity_t	*missile;	
	paletteRGBA_t	LightColor;
	float			lightsize;

	missile = ClientEntity_new(type, flags | CEF_DONT_LINK, origin, NULL, 100);

	missile->flags |= CEF_NO_DRAW;
	LightColor.c = 0xffff0077;		// purple
	lightsize = 120.0;
	
	VectorCopy(vel, missile->velocity);
	VectorNormalize2(vel, dir);
	AnglesFromDir(dir, missile->r.angles);

	missile->radius = 128;
	if(r_detail->value > DETAIL_NORMAL)
		missile->dlight = CE_DLight_new(LightColor, lightsize, 0.0f);
	missile->Update = FXPESpell2TrailThink;

	missile->SpawnInfo = 32;

	AddEffect(owner, missile);

	fxi.S_StartSound(missile->r.origin, -1, CHAN_WEAPON, fxi.S_RegisterSound("monsters/plagueelf/spell2.wav"), 
			1, ATTN_NORM, 0);
}



// ************************************************************************************************
// FXPESpell2Explode
// ************************************************************************************************

///////////////////////////////////////
// From CreateEffect FX_WEAPON_PESPELLEXPLODE
///////////////////////////////////////
void FXPESpell2Explode(centity_t *owner,int type,int flags,vec3_t origin, vec3_t dir)
{
	client_entity_t	*SmokePuff;
	int				i;
	paletteRGBA_t	LightColor;
	byte			powerup = 0;
	float			lightrad;
	
	if(flags & CEF_FLAG6)
	{
		FXClientScorchmark(origin, dir);
	}

	Vec3ScaleAssign(32.0, dir);

	i = GetScaledCount(irand(8, 12), 0.8);
	LightColor.c = 0xffff0077;		// purple
	lightrad = 150;

	while(i--)
	{
		if (!i)
			SmokePuff=ClientEntity_new(type,flags,origin,NULL,500);
		else
			SmokePuff=ClientEntity_new(type,flags,origin,NULL,1000);

		SmokePuff->r.model = spell_models + 3;
		SmokePuff->r.scale=flrand(0.8,1.6);
		SmokePuff->d_scale=-2.0;

		VectorRandomCopy(dir, SmokePuff->velocity, 64.0);
		SmokePuff->acceleration[0] = flrand(-200, 200);
		SmokePuff->acceleration[1] = flrand(-200, 200);
		SmokePuff->acceleration[2] = flrand(-40, -60);

		SmokePuff->r.flags |=RF_FULLBRIGHT|RF_TRANSLUCENT|RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		/*
		SmokePuff->r.color.g = irand(40, 60);
		SmokePuff->r.color.b = irand(245, 255);
		SmokePuff->r.color.r = irand(95, 105);*/

		SmokePuff->r.frame=0;

		SmokePuff->d_alpha= -0.4;
			
		SmokePuff->radius=20.0;

		if(!i)
		{//precache this?
			fxi.S_StartSound(SmokePuff->r.origin, -1, CHAN_WEAPON, fxi.S_RegisterSound("monsters/plagueelf/spell2hit.wav"), 
					1, ATTN_NORM, 0);
			SmokePuff->dlight=CE_DLight_new(LightColor,lightrad,0.0f);
			VectorClear(SmokePuff->velocity);
		}	

		AddEffect(NULL,SmokePuff);
	}
}

//=====================================================================================

// ************************************************************************************************
// FXPESpell3TrailThink
// ************************************************************************************************

static qboolean FXPESpell3TrailThink(struct client_entity_s *self, centity_t *owner)
{
	client_entity_t	*TrailEnt;
	vec3_t			accel_dir;
	int				i;

	self->updateTime = 20;

	if(self->SpawnInfo > 9)
		self->SpawnInfo--;

	i = GetScaledCount( irand(self->SpawnInfo >> 3, self->SpawnInfo >> 2), 0.8 );
	while(i--)
	{
		TrailEnt = ClientEntity_new(FX_PE_SPELL, 0, self->r.origin, NULL, 1000);
		TrailEnt->r.flags |= RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	
		VectorCopy(self->velocity, accel_dir);
		VectorNormalize(accel_dir);

		TrailEnt->r.model = spell_models;
		TrailEnt->r.scale = SPELL_SCALE + flrand(0.0, 0.05);

		/*TrailEnt->r.color.g = irand(40, 60);
		TrailEnt->r.color.r = irand(245, 255);
		TrailEnt->r.color.b = irand(95, 105);*/

		VectorRandomCopy(self->r.origin, TrailEnt->r.origin, flrand(-5.0, 5.0));
		VectorScale(accel_dir, flrand(-50.0, -400.0), TrailEnt->velocity);

		TrailEnt->d_alpha = flrand(-1.5, -2.0);
		TrailEnt->d_scale = flrand(-1.0, -1.25);
		TrailEnt->updateTime = (TrailEnt->alpha * 1000.0) / -TrailEnt->d_scale;
		TrailEnt->radius = 20.0;
		
		AddEffect(NULL,TrailEnt);
	}

	return(true);
}

// ************************************************************************************************
// FXPESpell3
// ************************************************************************************************

////////////////////////////////////
// From CreateEffect FX_WEAPON_PESPELL
////////////////////////////////////
void FXPESpell3Go(centity_t *owner, int type, int flags, vec3_t origin, vec3_t vel)
{
	vec3_t			dir;
	client_entity_t	*missile;	
	paletteRGBA_t	LightColor;
	float			lightsize;

	missile = ClientEntity_new(type, flags | CEF_DONT_LINK, origin, NULL, 100);

	missile->flags |= CEF_NO_DRAW;
	LightColor.c = 0xffff6611;		// cyan
	lightsize = 120.0;
	
	VectorCopy(vel, missile->velocity);
	VectorNormalize2(vel, dir);
	AnglesFromDir(dir, missile->r.angles);

	missile->radius = 128;
	if(r_detail->value > DETAIL_NORMAL)
		missile->dlight = CE_DLight_new(LightColor, lightsize, 0.0f);
	missile->Update = FXPESpell3TrailThink;

	missile->SpawnInfo = 32;

	AddEffect(owner, missile);

	fxi.S_StartSound(missile->r.origin, -1, CHAN_WEAPON, fxi.S_RegisterSound("monsters/plagueelf/spell3.wav"), 
			1, ATTN_NORM, 0);
}



// ************************************************************************************************
// FXPESpell3Explode
// ************************************************************************************************

///////////////////////////////////////
// From CreateEffect FX_WEAPON_PESPELLEXPLODE
///////////////////////////////////////
void FXPESpell3Explode(centity_t *owner,int type,int flags,vec3_t origin, vec3_t dir)
{
	client_entity_t	*SmokePuff;
	int				i;
	paletteRGBA_t	LightColor;
	byte			powerup = 0;
	float			lightrad;
	
	if(flags & CEF_FLAG6)
	{
		FXClientScorchmark(origin, dir);
	}

	Vec3ScaleAssign(32.0, dir);

	i = GetScaledCount(irand(8, 12), 0.8);
	LightColor.c = 0xffff6611;		// cyan
	lightrad = 150;

	while(i--)
	{
		if (!i)
			SmokePuff=ClientEntity_new(type,flags,origin,NULL,500);
		else
			SmokePuff=ClientEntity_new(type,flags,origin,NULL,1000);

		SmokePuff->r.model = spell_models + 4;
		SmokePuff->r.scale=flrand(1.0, 1.8);
		SmokePuff->d_scale=-2.0;

		VectorRandomCopy(dir, SmokePuff->velocity, 64.0);
		SmokePuff->acceleration[0] = flrand(-200, 200);
		SmokePuff->acceleration[1] = flrand(-200, 200);
		SmokePuff->acceleration[2] = flrand(-40, -60);

		SmokePuff->r.flags |=RF_FULLBRIGHT|RF_TRANSLUCENT|RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		/*
		SmokePuff->r.color.g = irand(40, 60);
		SmokePuff->r.color.r = irand(245, 255);
		SmokePuff->r.color.b = irand(95, 105);
		*/

		SmokePuff->r.frame=0;

		SmokePuff->d_alpha= -0.4;
			
		SmokePuff->radius=20.0;

		if(!i)
		{//precache this?
			fxi.S_StartSound(SmokePuff->r.origin, -1, CHAN_WEAPON, fxi.S_RegisterSound("monsters/plagueelf/spell3hit.wav"), 
					1, ATTN_NORM, 0);
			SmokePuff->dlight=CE_DLight_new(LightColor,lightrad,0.0f);
			VectorClear(SmokePuff->velocity);
		}	

		AddEffect(NULL,SmokePuff);
	}
}

//====================================================================
//	FX_PE_SPELL effect handler
//====================================================================

void FXCWStars (centity_t *owner,int type,int flags, vec3_t vel);
void FXPESpell(centity_t *owner, int type, int flags, vec3_t origin)
{
	byte			whicheffect = 0;
	vec3_t			vel;

	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_PE_SPELL].formatString, &whicheffect, vel);

	switch(whicheffect)
	{
		case FX_PE_MAKE_SPELL:
			FXPESpellGo(owner, type, flags, origin, vel);
			break;

		case FX_PE_EXPLODE_SPELL:
			FXPESpellExplode(owner, type, flags, origin, vel);
			break;

		case FX_PE_MAKE_SPELL2:
			FXPESpell2Go(owner, type, flags, origin, vel);
			break;

		case FX_PE_EXPLODE_SPELL2:
			FXPESpell2Explode(owner, type, flags, origin, vel);
			break;
		
		case FX_PE_MAKE_SPELL3:
			FXCWStars(owner, type, flags, origin);
			//FXPESpell3Go(owner, type, flags, origin, vel);
			break;

		case FX_PE_EXPLODE_SPELL3:
			FXPESpell3Explode(owner, type, flags, origin, vel);
			break;

		default:
			Com_Printf("Unknown effect type (%d) for FXSsithraArrow\n", whicheffect);
			break;
	}
}