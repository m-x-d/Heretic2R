//
// fx_spellchange.c
//
// Heretic II
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Client Entities.h"
#include "ResourceManager.h"
#include "FX.h"
#include "Vector.h"
#include "Matrix.h"
#include "Random.h"
#include "Utilities.h"
#include "Reference.h"
#include "ce_DLight.h"
#include "Particle.h"


#define NUM_BITS		12
#define LIGHT_LIFETIME	1000

// ************************************************************************************************
// FXSpellChangePuffThink
// ----------------------------
// ************************************************************************************************

qboolean FXSpellChangeLightThink(struct client_entity_s *self,centity_t *owner)
{
	if (fxi.cl->time - self->startTime > LIGHT_LIFETIME)
	{
		return(false);
	}

	self->dlight->intensity = 200.0 * (float)(LIGHT_LIFETIME - (fxi.cl->time - self->startTime)) / (float)LIGHT_LIFETIME;

	return(true);
}

// ************************************************************************************************
// FXSpellChange
// ---------------------
// ************************************************************************************************

void FXSpellChange(centity_t *Owner,int Type,int Flags,vec3_t Origin)
{
	vec3_t			dir;
	client_entity_t	*spellpuff;
	client_particle_t *spellbit;
	int				i;
	int				spelltype=0;
	paletteRGBA_t	color;
	int				part;

	fxi.GetEffect(Owner,Flags,clientEffectSpawners[FX_SPELL_CHANGE].formatString,dir,&spelltype);

	switch(spelltype)
	{
	case 1:		// Red/fireball
		color.c = 0xFF0000FF;
		part = PART_16x16_SPARK_R;
		break;
	case 2:		// Indigo/Array
		color.c = 0xFFFF0080;
		part = PART_16x16_SPARK_I;
		break;
	case 3:		// Blue/Sphere
		color.c = 0xFFFF0000;
		part = PART_16x16_SPARK_B;
		break;
	case 4:		// Green/Mace ball
		color.c = 0xFF00FF00	;
		part = PART_16x16_SPARK_G;
		break;
	case 5:		// Yellow/Firewall
		color.c = 0xFF0080FF;
		part = PART_16x16_SPARK_Y;
		break;
	case 6:		// Big red/red rain bow
		color.c = 0xFF0000FF;
		part = PART_16x16_SPARK_R;
		break;
	case 7:		// Big yellow/Phoenix
		color.c = 0xFF00FFFF;
		part = PART_32x32_FIRE1;
		break;
	case 0:		// Default color--white
	default:
		color.c = 0x80FFFFFF;
		part = PART_16x16_LIGHTNING;
		break;
	}

	VectorScale(dir,-32.0,dir);

	// Create the new effect
	spellpuff=ClientEntity_new(Type,Flags|CEF_OWNERS_ORIGIN|CEF_NO_DRAW|CEF_ADDITIVE_PARTS, Origin, NULL, 100);
	spellpuff->dlight=CE_DLight_new(color,150.0f,0.0f);
	spellpuff->startTime=fxi.cl->time;
	spellpuff->radius=32.0;
	spellpuff->Update=FXSpellChangeLightThink;

	color.c = 0xFFFFFFFF;
	// Attach some particles to it.
	for(i=0;i<NUM_BITS;i++)
	{
		spellbit = ClientParticle_new(part, color, 500);
		VectorSet(spellbit->velocity, flrand(-32.0,32.0), flrand(-32.0,32.0), flrand(16.0,64.0));
		VectorAdd(dir,spellbit->velocity, spellbit->velocity);
		spellbit->d_scale=-2.0;
		spellbit->scale = 6.0;
		
		AddParticleToList(spellpuff, spellbit);
	}

	AddEffect(Owner,spellpuff);
}
