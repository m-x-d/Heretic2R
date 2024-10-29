//
// fx_animate.c
//
// Heretic II
// Copyright 1998 Raven Software
//

#include "ce_DefaultMessageHandler.h"
#include "Client Effects.h"
#include "Client Entities.h"
#include "Particle.h"
#include "ResourceManager.h"
#include "FX.h"
#include "Vector.h"
#include "Utilities.h"
#include "Angles.h"
#include "random.h"

typedef struct FXAnimModel
{
	char	*ModelName;
	struct model_s *model;
	float	radius;
	float	alpha;
	int		numframes;
	int		defaultframe;
} FXAnimModel_t;

// ************************************************************************************************
// Animating chunks
// ************************************************************************************************

FXAnimModel_t FXAnimModel[NUM_FX_ANIM]=
{
	"models/objects/banner/wall/tris.fm",		NULL, 220.0,		1.0,	15,		1,	// FX_ANIM_BANNER
	"models/objects/lights/candelabrum/tris.fm",NULL, 100.0,		1.0,	7,		1,	// FX_ANIM_CANDELABRUM
	"models/objects/chandelier/chan2/tris.fm",	NULL, 100.0,		1.0,	7,		7,	// FX_ANIM_CHANDELIER2
	NULL,									NULL, 100.0,		0.5,	1,		0,	// FX_ANIM_FLAME - NEVER USED
	NULL,									NULL, 100.0,		0.5,	1,		0, 	// FX_ANIM_FIRE - NEVER USED
	"models/objects/banner/onpole/tris.fm",		NULL, 100.0,		1.0,	21,		0,	// FX_ANIM_BANNERONPOLE
	"models/objects/flags/onpole/tris.fm",		NULL, 100.0,		1.0,	80,		0,	// FX_ANIM_FLAGONPOLE
	"models/objects/eggs/cocoon/tris.fm",		NULL, 100.0,		1.0,	20,		0,	// FX_ANIM_COCOON
	"models/objects/labs/container1/tris.fm",	NULL, 100.0,		1.0,	30,		0,	// FX_ANIM_LABPARTSCONTAINER
	"models/objects/labs/tray/tris.fm",			NULL, 100.0,		1.0,	15,		0,	// FX_ANIM_LABTRAY
	"models/objects/labs/container2/tris.fm",	NULL, 100.0,		1.0,	80,		0,	// FX_ANIM_EYEBALLJAR
	"models/objects/torture/ogle/tris.fm",		NULL, 100.0,		1.0,	100,	0,	// FX_ANIM_HANGING_OGLE
};

void PreCacheFXAnimate(void)
{
	int i;

	for(i = 0; i < NUM_FX_ANIM; ++i)
	{
		if (FXAnimModel[i].ModelName)
			FXAnimModel[i].model = fxi.RegisterModel(FXAnimModel[i].ModelName);
	}
}

// -------------------------------------------------

static qboolean FXAnimateGo(struct client_entity_s *self, centity_t *owner)
{

	self->r.frame++;
	if(self->r.frame >= self->NoOfAnimFrames)
	{
		self->r.frame = 0;
	}
	return(true);
}

static qboolean FXAnimateRandomGo(struct client_entity_s *self, centity_t *owner)
{
   	self->updateTime = 100;
	self->r.frame++;

	if(self->r.frame == self->NoOfAnimFrames)
	{
		self->r.frame = 0;
		self->updateTime = irand(500, 5000);
	}
	return(true);
}

void FXAnimate(centity_t *owner, int type, int flags, vec3_t origin)
{
	client_entity_t	*self;
	byte			atype, scale, skinnum, anim;

	self = ClientEntity_new(type, flags, origin, 0, 100);

	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_ANIMATE].formatString, &anim, &scale, &skinnum, self->r.angles);
	VectorDegreesToRadians(self->r.angles, self->r.angles);
	atype = anim & 0x7f;

	self->r.model = &FXAnimModel[atype].model;
	self->r.scale = scale * 0.02;
	self->r.skinnum = skinnum;

	self->alpha = FXAnimModel[atype].alpha;
	self->radius = FXAnimModel[atype].radius;
	self->NoOfAnimFrames = FXAnimModel[atype].numframes;
	self->r.frame = FXAnimModel[atype].defaultframe;

	if(anim & 0x80)
	{
		// Animate (special animate for cocoon)
		if(atype == FX_ANIM_COCOON)
		{
			self->Update = FXAnimateRandomGo;
		}
		else
		{
			self->Update = FXAnimateGo;
		}

		self->nextThinkTime=fxi.cl->time + irand(40, 1600);	// So they don't all start on frame 0 at the same time

	}
	else
	{
		// Don`t animate and think less
		self->Update = KeepSelfAI;
		self->updateTime = 1000;
	}
	AddEffect(owner, self);
}
// end