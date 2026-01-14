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

static struct model_s* cwmodels[NUM_CW_MODELS];
static struct sfx_s* cw_weld_sound; //mxd
static struct sfx_s* cw_impact_sound; //mxd

void PreCacheCWModels(void)
{
	cwmodels[CWM_BEAM] = fxi.RegisterModel("sprites/fx/segment_trail_wt.sp2");
	cwmodels[CWM_BEAM_HALO] = fxi.RegisterModel("sprites/fx/halo.sp2");
	cwmodels[CWM_BEAM_LINE] = fxi.RegisterModel("sprites/fx/bluestreak.sp2");
	cwmodels[CWM_STAR_HALO] = fxi.RegisterModel("sprites/Spells/halo_ind.sp2");
	cwmodels[CWM_STAR_TRAIL] = fxi.RegisterModel("sprites/Spells/indigostreak.sp2");
}

void PreCacheCWSFX(void) //mxd
{
	cw_weld_sound = fxi.S_RegisterSound("Monsters/elflord/weld.wav");
	cw_impact_sound = fxi.S_RegisterSound("Monsters/elflord/impact.wav");
}

static qboolean CWBeamAddToView(client_entity_t* self, centity_t* owner) //mxd. Named 'FXCWBeamUpdate' in original logic.
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
		fxi.S_StartSound(self->r.origin, -1, CHAN_AUTO, cw_weld_sound, 0.5f, ATTN_IDLE, 0.0f);
		self->LifeTime = 0;
	}

	return true;
}

static qboolean CWBeamUpdate(client_entity_t* self, centity_t* owner) //mxd. Named 'FXCWBeamThink' in original logic.
{
	if (self->LifeTime >= fx_time)
	{
		self->r.scale = flrand(14.0f, 16.0f);
		return true;
	}

	return false;
}

static qboolean CWHaloUpdate(client_entity_t* self, centity_t* owner) //mxd. Named 'FXCWBeamThink2' in original logic.
{
	if (self->LifeTime >= fx_time)
	{
		self->r.scale = flrand(1.5f, 2.0f);
		return true;
	}

	return false;
}

static qboolean CWStarUpdate(client_entity_t* self, centity_t* owner) //mxd. Named 'FXCWStarThink' in original logic.
{
	self->r.scale = flrand(0.3f, 0.5f);
	return true;
}

void FXCWatcherEffects(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	const paletteRGBA_t light = { .r = 160, .g = 70, .b = 240, .a = 255 };
	const paletteRGBA_t white_light = { .r = 255, .g = 255, .b = 255, .a = 255 };

	byte fx_id;
	vec3_t vel;
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_CWATCHER].formatString, &fx_id, &vel);

	switch (fx_id)
	{
		case CW_STAR:
		{
			// Halo around the spark.
			client_entity_t* spark = ClientEntity_new(type, flags | CEF_DONT_LINK, origin, NULL, 17);
			spark->r.model = &cwmodels[CWM_STAR_HALO];
			spark->radius = 400.0f;

			spark->r.flags = RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
			spark->r.scale = 0.8f;
			spark->alpha = 0.5f;

			spark->Update = KeepSelfAI;
			spark->AddToView = LinkedEntityUpdatePlacement;

			AddEffect(owner, spark);

			// A bright star halo.
			client_entity_t* halo = ClientEntity_new(type, flags | CEF_DONT_LINK, origin, NULL, 17);
			halo->r.model = &cwmodels[CWM_STAR_HALO];
			halo->radius = 400.0f;

			halo->r.frame = 1;
			halo->r.flags |= RF_TRANSLUCENT | RF_TRANS_ADD;
			halo->r.scale = 0.5f;
			halo->alpha = 0.75f;
			halo->dlight = CE_DLight_new(light, 100.0f, 0.0f);

			halo->Update = CWStarUpdate;
			halo->AddToView = LinkedEntityUpdatePlacement;

			AddEffect(owner, halo);
		} break;

		case CW_STAR_HIT: // A bright explosion.
		{
			fxi.S_StartSound(origin, -1, CHAN_AUTO, cw_impact_sound, 0.5f, ATTN_IDLE, 0.0f);

			int count = GetScaledCount(4, 0.8f);
			for (int i = 0; i < count; i++)
			{
				client_entity_t* halo = ClientEntity_new(type, flags | CEF_DONT_LINK, origin, NULL, 500);
				halo->r.model = &cwmodels[CWM_STAR_HALO];
				halo->radius = 400.0f;

				halo->r.frame = 1;
				halo->r.flags = RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
				halo->r.scale = flrand(0.25f, 0.5f);
				halo->alpha = 0.75f;
				halo->d_alpha = -2.0f;
				halo->d_scale = -0.5f;

				VectorRandomCopy(vel, halo->velocity, 1.25f);
				VectorScale(halo->velocity, flrand(100.0f, 200.0f), halo->velocity);
				halo->acceleration[2] = -128.0f;

				AddEffect(NULL, halo);
			}

			count = GetScaledCount(2, 0.8f);
			for (int i = 0; i < count; i++)
			{
				client_entity_t* trail = ClientEntity_new(type, flags | CEF_DONT_LINK, origin, NULL, 1000);
				trail->r.model = &cwmodels[CWM_STAR_TRAIL];
				trail->radius = 400.0f;

				trail->r.flags = RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
				trail->flags |= CEF_USE_VELOCITY2;
				trail->r.spriteType = SPRITE_LINE;
				trail->r.scale = flrand(2.0f, 1.5f);
				trail->alpha = 0.75f;
				trail->d_alpha = -2.5f;
				trail->d_scale = -4.0f;

				VectorRandomCopy(vel, trail->velocity2, 1.0f);
				VectorCopy(trail->origin, trail->r.startpos);
				VectorMA(trail->r.startpos, flrand(16.0f, 32.0f), trail->velocity2, trail->r.endpos);
				VectorScale(trail->velocity2, flrand(50.0f, 100.0f), trail->velocity2);

				if (i == count - 1)
					trail->dlight = CE_DLight_new(light, 200.0f, -400.0f);

				AddEffect(NULL, trail);
			}
		} break;

		case CW_BEAM:
		{
			client_entity_t* beam = ClientEntity_new(type, flags, origin, NULL, 17);
			beam->r.model = &cwmodels[CWM_BEAM];
			beam->radius = 400.0f;

			beam->r.flags = RF_TRANS_ADD | RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD_ALPHA;
			beam->r.scale = 16.0f;
			beam->r.color.c = 0xa0ffffff;
			beam->r.spriteType = SPRITE_LINE;
			VectorCopy(vel, beam->r.startpos);
			VectorCopy(origin, beam->r.endpos);
			beam->LifeTime = fx_time + 3100;

			beam->Update = CWBeamUpdate;
			beam->AddToView = CWBeamAddToView;

			AddEffect(owner, beam);

			// Spawn a halo to cover the flat end.
			client_entity_t* halo = ClientEntity_new(type, flags, origin, NULL, 17);
			halo->r.model = &cwmodels[CWM_BEAM_HALO];
			halo->radius = 400.0f;

			halo->r.flags = RF_TRANS_ADD | RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD_ALPHA | RF_NODEPTHTEST;
			halo->r.scale = 2.5f;
			halo->r.color = color_white; //mxd
			halo->LifeTime = fx_time + 3100;
			halo->dlight = CE_DLight_new(white_light, 200.0f, 0.0f);

			halo->Update = CWHaloUpdate;
			halo->AddToView = LinkedEntityUpdatePlacement;

			fxi.Activate_Screen_Shake(8.0f, 4000.0f, (float)fxi.cl->time, SHAKE_ALL_DIR); // 'current_time' MUST be cl.time, because that's what used by Perform_Screen_Shake() to calculate effect intensity/timing... --mxd.

			AddEffect(owner, halo);
		} break;

		case CW_BEAM_START: // Spawn a halo to cover the flat end.
		{
			client_entity_t* beam = ClientEntity_new(type, flags, origin, NULL, 4000);
			beam->r.model = &cwmodels[CWM_BEAM_HALO];
			beam->radius = 400.0f;

			beam->r.flags = RF_TRANS_ADD | RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD_ALPHA | RF_NODEPTHTEST;
			beam->r.scale = 2.5f;
			beam->r.color = color_white; //mxd
			beam->LifeTime = fx_time + 3100;
			beam->dlight = CE_DLight_new(white_light, 200.0f, 0.0f);

			beam->AddToView = LinkedEntityUpdatePlacement;

			fxi.Activate_Screen_Shake(8.0f, 3000.0f, (float)fxi.cl->time, SHAKE_ALL_DIR); // 'current_time' MUST be cl.time, because that's what used by Perform_Screen_Shake() to calculate effect intensity/timing... --mxd.

			AddEffect(owner, beam);
		} break;

		default:
			assert(0);
			break;
	}
}