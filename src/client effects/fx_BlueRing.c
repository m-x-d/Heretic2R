//
// Heretic II
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Client Entities.h"
#include "ce_DefaultMessageHandler.h"
#include "Particle.h"
#include "ResourceManager.h"
#include "FX.h"
#include "Vector.h"
#include "Utilities.h"

#define	NUM_FLAME_ITEMS		20
#define FLAME_ABSVEL		450
#define FLAME_INIT_SCALE	0.1F
#define FLAME_DELTA_SCALE	6.0F
#define FLAME_DURATION		500
#define FLAME_DELTA_ALPHA	(1000/FLAME_DURATION)

#define	NUM_RING_MODELS	1
static struct model_s *blue_models[NUM_RING_MODELS];
void PreCacheBluering()
{
	blue_models[0] = fxi.RegisterModel("sprites/spells/bluering.sp2");
}

void FXBlueRing(centity_t *Owner, int Type, int Flags, vec3_t Origin)
{
	client_entity_t		*flameitem;
	float				curAng;
	int					count;

	count = GetScaledCount(NUM_FLAME_ITEMS, 0.95);
	// Bound this between 8 and 16 sprites.
	if (count > 20)
		count=20;
	else if (count < 8)
		count=8;
	for(curAng = 0.0F; curAng < (M_PI * 2.0F); curAng += (M_PI * 2.0F) / count)
	{
		flameitem = ClientEntity_new(Type, Flags & ~CEF_OWNERS_ORIGIN, Origin, NULL, FLAME_DURATION);

		flameitem->r.model = blue_models;
		flameitem->r.flags = RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		flameitem->r.frame = 0;
		flameitem->r.scale = FLAME_INIT_SCALE;
		flameitem->d_scale = FLAME_DELTA_SCALE;

		VectorSet(flameitem->velocity, FLAME_ABSVEL * cos(curAng), FLAME_ABSVEL * sin(curAng), 0);
		flameitem->radius = 20.0F;
		flameitem->d_alpha = -FLAME_DELTA_ALPHA;

		AddEffect(NULL, flameitem);
	}
	fxi.S_StartSound(Origin, -1, CHAN_AUTO, fxi.S_RegisterSound("weapons/Spell Blue Ring.wav"), 1, ATTN_NORM, 0);
}
// end
