//
// fx_cwatcher.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Vector.h"
#include "Random.h"
#include "Utilities.h"
#include "q_Sprite.h"
#include "ce_DLight.h"
#include "g_playstats.h"

enum cwatcher_model_index_e
{
	CWM_BEAM,
	CWM_BEAM_HALO,
	CWM_BEAM_LINE,
	CWM_STAR_HALO,
	CWM_STAR_TRAIL,

	NUM_CW_MODELS
};

enum cwatcher_effect_id_e
{
	CW_STAR,
	CW_STAR_HIT,
	CW_BEAM,
	CW_BEAM_START
};

static struct model_s* cwmodels[NUM_CW_MODELS];

void PreCacheCWModels(void)
{
	cwmodels[CWM_BEAM] = fxi.RegisterModel("sprites/fx/segment_trail_wt.sp2");
	cwmodels[CWM_BEAM_HALO] = fxi.RegisterModel("sprites/fx/halo.sp2");
	cwmodels[CWM_BEAM_LINE] = fxi.RegisterModel("sprites/fx/bluestreak.sp2");
	cwmodels[CWM_STAR_HALO] = fxi.RegisterModel("sprites/Spells/halo_ind.sp2");
	cwmodels[CWM_STAR_TRAIL] = fxi.RegisterModel("sprites/Spells/indigostreak.sp2");
}

static qboolean FXCWBeamUpdate(struct client_entity_s* self, const centity_t* owner)
{
	const vec3_t vel = { 0.0f, 0.0f, 1.0f };

	LinkedEntityUpdatePlacement(self, owner);
	VectorCopy(self->r.origin, self->r.endpos);

	// Spawn a bunch of lines and sparks.
	const int count = GetScaledCount(2, 0.85f);

	for (int i = 0; i < count; i++)
	{
		client_entity_t* beam = ClientEntity_new(FX_CWATCHER, CEF_DONT_LINK, self->r.origin, NULL, 1000);
		beam->r.model = &cwmodels[CWM_BEAM_LINE];
		beam->radius = 400.0f;

		beam->r.flags = RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		beam->flags |= CEF_USE_VELOCITY2;

		beam->r.spriteType = SPRITE_LINE;

		beam->r.scale = flrand(1.5f, 2.5f);
		beam->alpha = 0.75f;
		beam->d_alpha = -2.5f;
		beam->d_scale = -2.5f;

		beam->r.origin[0] += (float)(irand(-2, 2)); //TODO: why not flrand?
		beam->r.origin[1] += (float)(irand(-2, 2)); //TODO: why not flrand?

		VectorRandomCopy(vel, beam->velocity2, 1.0f);
		VectorCopy(beam->origin, beam->r.startpos);
		VectorMA(beam->r.startpos, flrand(16.0f, 32.0f), beam->velocity2, beam->r.endpos);
		VectorScale(beam->velocity2, flrand(50.0f, 100.0f), beam->velocity2);

		AddEffect(NULL, beam);
	}

	if (++self->LifeTime == 8)
	{
		fxi.S_StartSound(self->r.origin, -1, CHAN_AUTO, fxi.S_RegisterSound("Monsters/elflord/weld.wav"), 0.5f, ATTN_IDLE, 0);
		self->LifeTime = 0;
	}

	return true;
}

static qboolean FXCWBeamThink(struct client_entity_s* self, centity_t* owner)
{
	if (self->LifeTime >= fxi.cl->time)
	{
		self->r.scale = flrand(14.0f, 16.0f);
		return true;
	}

	return false;
}

static qboolean FXCWBeamThink2(struct client_entity_s* self, centity_t* owner)
{
	if (self->LifeTime >= fxi.cl->time)
	{
		self->r.scale = flrand(1.5f, 2.0f);
		return true;
	}

	return false;
}

static qboolean FXCWStarThink(struct client_entity_s* self, centity_t* owner)
{
	self->r.scale = flrand(0.3f, 0.5f);
	return true;
}

void FXCWatcherEffects(centity_t *owner, int type, int flags, vec3_t origin)
{
	client_entity_t		*spawner;
	paletteRGBA_t		light = { 160, 70, 240, 255 },
						white_light = { 255, 255, 255, 255 };

	vec3_t				vel;
	byte				fxID;
	int					i;

	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_CWATCHER].formatString, &fxID, &vel);

	switch (fxID)
	{
	case CW_STAR:
		
		//Halo around the spark
		spawner = ClientEntity_new(type, flags | CEF_DONT_LINK, origin, NULL, 17);
		spawner->r.model = cwmodels + CWM_STAR_HALO;
		spawner->radius = 400;
		
		spawner->r.frame = 0;
		spawner->r.flags |= RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		spawner->r.scale=0.8;
		spawner->alpha=0.5;
		spawner->d_alpha=0.0;
		
		spawner->Update = KeepSelfAI;
 		spawner->AddToView = LinkedEntityUpdatePlacement;

		AddEffect(owner, spawner);

		//A bright star halo
		spawner = ClientEntity_new(type, flags | CEF_DONT_LINK, origin, NULL, 17);
		spawner->r.model = cwmodels + CWM_STAR_HALO;
		spawner->radius = 400;
		
		spawner->r.frame = 1;
		spawner->r.flags |= RF_TRANSLUCENT | RF_TRANS_ADD;
		spawner->r.scale=0.5;
		spawner->alpha=0.75;
		spawner->d_alpha=0.0;
		
		spawner->dlight=CE_DLight_new(light,100.0f,0.0f);

		spawner->Update = FXCWStarThink;
 		spawner->AddToView = LinkedEntityUpdatePlacement;

		AddEffect(owner, spawner);

		break;
		
	case CW_STAR_HIT:

		//A bright explosion

		fxi.S_StartSound(origin, -1, CHAN_AUTO, fxi.S_RegisterSound("monsters/elflord/impact.wav"), 0.5, ATTN_IDLE, 0);

		i = GetScaledCount(4, 0.8);

		while (i--)
		{
			spawner = ClientEntity_new(type, flags | CEF_DONT_LINK, origin, NULL, 500);
			spawner->r.model = cwmodels + CWM_STAR_HALO;
			spawner->radius = 400;
			
			spawner->r.frame = 1;
			spawner->r.flags |= RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
			spawner->r.scale=flrand(0.5, 0.25);
			spawner->alpha=0.75;
			spawner->d_alpha=-2.0;
			spawner->d_scale=-0.5;

			VectorRandomCopy(vel, spawner->velocity, 1.25);
			VectorScale(spawner->velocity, flrand(100,200), spawner->velocity);
			spawner->acceleration[2] = -128;
			
			AddEffect(NULL, spawner);
		}

		i = GetScaledCount(2, 0.8);

		while (i--)
		{
			spawner = ClientEntity_new(type, flags | CEF_DONT_LINK, origin, NULL, 1000);
			spawner->r.model = cwmodels + CWM_STAR_TRAIL;
			spawner->radius = 400;
			
			spawner->r.flags |= RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
			spawner->flags |= CEF_USE_VELOCITY2;

			spawner->r.spriteType = SPRITE_LINE;

			spawner->r.scale=flrand(2.0, 1.5);
			spawner->alpha=0.75;
			spawner->d_alpha=-2.5;
			spawner->d_scale=-4.0;

			VectorRandomCopy(vel, spawner->velocity2, 1.0);
			VectorCopy(spawner->origin, spawner->r.startpos);
			VectorMA(spawner->r.startpos, flrand(16,32), spawner->velocity2, spawner->r.endpos);
			
			VectorScale(spawner->velocity2, flrand(50,100), spawner->velocity2);
			VectorClear(spawner->velocity);
	
			if (!i)
				spawner->dlight=CE_DLight_new(light,200.0f,-400.0f);

			AddEffect(NULL, spawner);
		}

		break;

	case CW_BEAM:
		spawner = ClientEntity_new(type, flags, origin, NULL, 17);
		spawner->r.model = cwmodels + CWM_BEAM;
		spawner->radius = 400;
		
		spawner->r.flags = RF_TRANS_ADD | RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD_ALPHA;
		spawner->r.scale = 16;
		spawner->r.color.c = 0xA0FFFFFF;
		spawner->r.spriteType = SPRITE_LINE;
		VectorCopy(vel, spawner->r.startpos);
		VectorCopy(origin, spawner->r.endpos);
		
		spawner->LifeTime = fxi.cl->time + 3100;

		spawner->Update = FXCWBeamThink;
 		spawner->AddToView = FXCWBeamUpdate;

		AddEffect(owner, spawner);

		//Spawn a halo to cover the flat end
		spawner = ClientEntity_new(type, flags, origin, NULL, 17);
		spawner->r.model = cwmodels + CWM_BEAM_HALO;
		spawner->radius = 400;
		
		spawner->r.flags = RF_TRANS_ADD | RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD_ALPHA | RF_NODEPTHTEST;
		spawner->r.scale = 2.5;
		spawner->r.color.c = 0xFFFFFFFF;

		spawner->LifeTime = fxi.cl->time + 3100;

		spawner->dlight=CE_DLight_new(white_light,200.0f,0.0f);

 		spawner->Update = FXCWBeamThink2;
		spawner->AddToView = LinkedEntityUpdatePlacement;

		fxi.Activate_Screen_Shake(8, 4000, fxi.cl->time, SHAKE_ALL_DIR);

		AddEffect(owner, spawner);

		break;

	case CW_BEAM_START:

		//Spawn a halo to cover the flat end
		spawner = ClientEntity_new(type, flags, origin, NULL, 4000);
		spawner->r.model = cwmodels + CWM_BEAM_HALO;
		spawner->radius = 400;
		
		spawner->r.flags = RF_TRANS_ADD | RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD_ALPHA | RF_NODEPTHTEST;
		spawner->r.scale = 2.5;
		spawner->r.color.c = 0xFFFFFFFF;

		spawner->dlight=CE_DLight_new(white_light,200.0f,0.0f);

		spawner->AddToView = LinkedEntityUpdatePlacement;

		spawner->LifeTime = fxi.cl->time + 3100;

		fxi.Activate_Screen_Shake(8, 3000, fxi.cl->time, SHAKE_ALL_DIR);

		AddEffect(owner, spawner);

		break;
		
	default:
		assert(0);
		break;
	}
}

