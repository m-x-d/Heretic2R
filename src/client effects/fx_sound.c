//
// fx_sound.c
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
#include "random.h"
#define TAG_LEVEL 766 // tags get cleared at each new level

typedef struct soundthinkinfo_s
{
	int style;
	float attenuation;
	float volume;
	float wait;
} soundthinkinfo_t;

static qboolean FXSoundthink(struct client_entity_s *self, centity_t *owner)
{
	int chance;
	soundthinkinfo_t *soundinfo;
	char *soundname;

	soundinfo = (soundthinkinfo_t *) self->extra;
	// flag if we are not doing a sound
	soundname = NULL;

	// if we are in a cinematic, stop us making noises
	// these are peripheral ambient noises
   	if(!(Cvar_Get("cl_cinematicfreeze","0",0)->value))
   	{
		switch((int)(soundinfo->style))
		{
		case AS_SEAGULLS:
   			chance = irand(0,2);
   			if (chance < 1)
   				soundname = "ambient/gull1.wav";
   			else if (chance < 2 )
   				soundname = "ambient/gull2.wav";
   			else
   				soundname = "ambient/gull3.wav";
			break;
		case AS_BIRDS:
   			chance = irand(0,10);
   			if (chance < 1)
   				soundname = "ambient/bird1.wav";
   			else if (chance < 2)
   				soundname = "ambient/bird2.wav";
   			else if (chance < 3)
   				soundname = "ambient/bird3.wav";
   			else if (chance < 4)
   				soundname = "ambient/bird4.wav";
   			else if (chance < 5)
   				soundname = "ambient/bird5.wav";
   			else if (chance < 6)
   				soundname = "ambient/bird6.wav";
   			else if (chance < 7)
   				soundname = "ambient/bird7.wav";
   			else if (chance < 8)
   				soundname = "ambient/bird8.wav";
   			else if (chance < 9)
   				soundname = "ambient/bird9.wav";
   			else
   				soundname = "ambient/bird10.wav";
			break;
		case AS_CRICKETS:
   			chance = irand(0,2);
   			if (chance < 1)
   				soundname = "ambient/cricket1.wav";
   			else if (chance < 2)
   				soundname = "ambient/cricket2.wav";
   			else
   				soundname = "ambient/cricket3.wav";
			break;
		case AS_FROGS:
   			chance = irand(0,1);
   			if (chance < 1)
   				soundname = "ambient/frog.wav";
   			else
   				soundname = "ambient/frog2.wav";
			break;
		case AS_CRYING:
   			chance = irand(0,3);
   			if (chance < 1)
   				soundname = "ambient/femcry1.wav";
   			else if (chance < 2)
   				soundname = "ambient/femcry2.wav";
   			else if (chance < 3)
   				soundname = "ambient/kidcry1.wav";
   			else 
   				soundname = "ambient/kidcry2.wav"; 
			break;
		case AS_MOSQUITOES:
   			chance = irand(0,1);
   			if (chance < 1)
   				soundname = "ambient/insects1.wav";
   			else
   				soundname = "ambient/insects2.wav";
			break;
		case AS_BUBBLES:
   			soundname = "ambient/bubbles.wav";
			break;
		case AS_BELL:
   			soundname = "ambient/bell.wav";
			break;
		case AS_FOOTSTEPS:
   			chance = irand(0,3);
   			if (chance < 1)
   				soundname = "ambient/runaway1.wav";
   			else if (chance < 2)
   				soundname = "ambient/runaway2.wav";
   			else 
   				soundname = "ambient/sewerrun.wav";
			break;
		case AS_MOANS:
   			chance = irand(0,5);
   			if (chance < 1)
   				soundname = "ambient/moan1.wav";
   			else if (chance < 2)
   				soundname = "ambient/moan2.wav";
   			else if (chance < 3)
   				soundname = "ambient/scream1.wav";
   			else if (chance < 4)
   				soundname = "ambient/scream2.wav";
   			else
   				soundname = "ambient/coughing.wav";
			break;
		case AS_SEWERDRIPS:
   			chance = irand(0,3);
   			if (chance <1)
   				soundname = "ambient/sewerdrop1.wav";
   			else if (chance <2)
   				soundname = "ambient/sewerdrop2.wav";
   			else
   				soundname = "ambient/sewerdrop3.wav";
			break;
		case AS_WATERDRIPS:
   			chance = irand(0,3);
   			if (chance <1)
   				soundname = "ambient/waterdrop1.wav";
   			else if (chance <2)
   				soundname = "ambient/waterdrop2.wav";
   			else
   				soundname = "ambient/waterdrop3.wav";
			break;
		case AS_HEAVYDRIPS:
   			chance = irand(0,3);
   			if (chance <1)
   				soundname = "ambient/soliddrop1.wav";
   			else if (chance <2)
   				soundname = "ambient/soliddrop2.wav";
   			else
   				soundname = "ambient/soliddrop3.wav";
			break;
		case AS_WINDCHIME:
   			soundname = "ambient/windchimes.wav";
			break;
		case AS_BIRD1:
   			soundname = "ambient/bird5.wav";
			break;
		case AS_BIRD2:
   			soundname = "ambient/bird8.wav";
			break;
		case AS_GONG:
   			soundname = "ambient/gong.wav";
			break;
		case AS_ROCKS:
   			chance = irand(0,3);
   			if (chance <1)
   				soundname = "ambient/rocks1.wav";
   			else if (chance <2)
   				soundname = "ambient/rocks4.wav";
   			else
   				soundname = "ambient/rocks5.wav";
			break;
		case AS_CAVECREAK:
   			chance = irand(0,3);
   			if (chance <1)
   				soundname = "ambient/cavecreak1.wav";
   			else if (chance <2)
   				soundname = "ambient/cavecreak1.wav";
   			else
   				soundname = "ambient/cavecreak1.wav";
			break;
		default:
   			Com_DPrintf("ERROR:  invalid ambient sound type :%d at x:%f  y:%f  z:%f\n",soundinfo->style,
   				self->origin[0],self->origin[1],self->origin[2]);
			break;
		}
   	}

	// if we have a sound to do, lets do it.
	if (soundname)
	{
		if (Vec3IsZero(self->origin))
			fxi.S_StartSound(self->origin,owner->current.number, CHAN_WEAPON, fxi.S_RegisterSound(soundname),soundinfo->volume,0, 0);
		else
			fxi.S_StartSound(self->origin,owner->current.number, CHAN_WEAPON, fxi.S_RegisterSound(soundname),soundinfo->volume,soundinfo->attenuation, 0);
	}

	self->updateTime = soundinfo->wait * flrand(.5,1.5);
	if (self->updateTime<17)
		self->updateTime = 17;

	return(true);	// Keep everything around so we can shut them down when needed.
}

void FXSound(centity_t *owner,int type,int flags,vec3_t origin)
{
	client_entity_t	*self;
	byte 					style, wait, attenuation,volume;
	soundthinkinfo_t 	*soundinfo;

  	self = ClientEntity_new(type, flags|CEF_NO_DRAW|CEF_NOMOVE, origin, 
		NULL, 20);
	self->flags &= ~CEF_OWNERS_ORIGIN;

	self->extra=fxi.TagMalloc(sizeof(soundthinkinfo_t),TAG_LEVEL);

	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_SOUND].formatString, &style,&attenuation,&volume,&wait);

	soundinfo = (soundthinkinfo_t *) self->extra;
	soundinfo->style = style;
	soundinfo->attenuation = attenuation;
	soundinfo->volume = volume/255.0;
	soundinfo->wait = wait * 1000;
	VectorCopy(origin,self->origin);
	self->Update = FXSoundthink;

	AddEffect(owner, self);
}

