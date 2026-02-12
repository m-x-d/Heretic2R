//
// fx_ArmorHit.c -- originally part of Generic Weapon Effects.c.
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Vector.h"
#include "Random.h"
#include "Utilities.h"
#include "q_Sprite.h"

static struct model_s* armorhit_model;

void PreCacheArmorHit(void)
{
	armorhit_model = fxi.RegisterModel("sprites/fx/firestreak.sp2");
}

// We hit someone with armor - do a pretty effect.
// Ripped off unashamedly from Josh's extremely cool streak effect. One of the coolest effects I've seen in a long time Josh. Good work Dude.
void FXArmorHit(centity_t* owner, const int type, int flags, vec3_t origin) //mxd. Named 'FXCreateArmorHit' in original logic.
{
	vec3_t dir;
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_ARMOR_HIT].formatString, &dir);

	flags &= ~CEF_NO_DRAW;

	// Spawn a hit explosion of lines.
	const int count = GetScaledCount(6, 0.85f);

	for (int i = 0; i < count; i++)
	{
		client_entity_t* trail_fx = ClientEntity_new(type, flags, origin, NULL, 500);

		trail_fx->r.model = &armorhit_model;
		trail_fx->r.spriteType = SPRITE_LINE;

		trail_fx->r.flags |= RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		trail_fx->r.scale = flrand(1.0f, 2.5f);
		trail_fx->alpha = flrand(0.75f, 1.0f);
		trail_fx->d_alpha = -2.0f;
		trail_fx->d_scale = -1.0f;
		COLOUR_SETA(trail_fx->r.color, irand(128, 255), irand(64, 255), irand(64, 255), irand(16, 128) + 64); //mxd. Use macro.

		VectorRandomCopy(dir, trail_fx->velocity, 1.0f);

		VectorCopy(origin, trail_fx->r.endpos);
		VectorMA(trail_fx->r.endpos, flrand(6.0f, 8.0f), trail_fx->velocity, trail_fx->r.startpos); //mxd. Was irand().

		Vec3ScaleAssign(flrand(50.0f, 100.0f), trail_fx->velocity); //mxd. Was irand().

		AddEffect(NULL, trail_fx);
	}
}