//
// fx_StaffHit.c -- mxd. Part of fx_Staff.c in original version.
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Random.h"
#include "Utilities.h"
#include "Vector.h"
#include "ce_DLight.h"
#include "g_playstats.h"
#include "q_Sprite.h"

static struct model_s* staffhit_models[4];

void PreCacheStaffHit(void)
{
	staffhit_models[0] = fxi.RegisterModel("sprites/spells/patball.sp2");
	staffhit_models[1] = fxi.RegisterModel("sprites/fx/halo.sp2");
	staffhit_models[2] = fxi.RegisterModel("sprites/fx/firestreak.sp2");
	staffhit_models[3] = fxi.RegisterModel("sprites/fx/steam.sp2");
}

//mxd. Separated from FXStaffStrike().
static void StaffStrikeLevel2(const int flags, const vec3_t origin, const vec3_t direction)
{
	if (R_DETAIL >= DETAIL_NORMAL)
		fxi.Activate_Screen_Flash(0x30FFFFFF);

	// Spawn a bright flash at the core of the explosion.
	client_entity_t* flash = ClientEntity_new(FX_WEAPON_STAFF_STRIKE, (int)(flags & ~CEF_NO_DRAW), origin, NULL, 1000);

	flash->r.model = &staffhit_models[1]; // halo sprite.
	flash->r.flags = RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	flash->r.scale = flrand(0.75f, 1.0f);
	flash->alpha = 0.75f;
	flash->d_alpha = -2.0f;
	flash->d_scale = -2.0f;
	flash->r.frame = 1;
	flash->r.color.c = 0xFF888888;
	flash->dlight = CE_DLight_new(flash->r.color, 150.0f, -100.0f);

	AddEffect(NULL, flash);

	// Spawn a hit explosion of lines.
	const int count = GetScaledCount(64, 0.85f);

	for (int i = 0; i < count; i++)
	{
		client_entity_t* streak = ClientEntity_new(FX_WEAPON_STAFF_STRIKE, (int)(flags & ~CEF_NO_DRAW), origin, NULL, 500);

		streak->r.model = &staffhit_models[0]; // patball sprite.
		streak->r.spriteType = SPRITE_LINE;
		streak->r.flags = RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		streak->r.scale = flrand(1.0f, 2.5f);
		streak->alpha = flrand(0.75f, 1.0f);
		streak->d_alpha = -2.0f;
		streak->d_scale = -1.0f;

		const int c = irand(128, 255);
		COLOUR_SETA(streak->r.color, c, c, irand(236, 255), irand(80, 192)); //mxd. Use macro.

		VectorRandomCopy(direction, streak->velocity, 1.25f);

		VectorCopy(origin, streak->r.startpos);
		VectorMA(streak->r.startpos, flrand(16.0f, 48.0f), streak->velocity, streak->r.endpos); //mxd. Original logic uses irand() here.

		VectorScale(streak->velocity, flrand(200.0f, 300.0f), streak->velocity);
		VectorSet(streak->acceleration, streak->velocity[0] * 0.1f, streak->velocity[1] * 0.1f, 0); //mxd. Original logic uses irand() here.

		AddEffect(NULL, streak);
	}
}

//mxd. Separated from FXStaffStrike().
static void StaffStrikeLevel3(const int flags, const vec3_t origin, const vec3_t direction)
{
	// Spawn a bright flash at the core of the explosion.
	client_entity_t* flash = ClientEntity_new(FX_WEAPON_STAFF_STRIKE, (int)(flags & ~CEF_NO_DRAW), origin, NULL, 1000);

	flash->r.model = &staffhit_models[1]; // Halo sprite.
	flash->r.flags = RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	flash->r.scale = flrand(0.75f, 1.0f);
	flash->alpha = 0.75f;
	flash->d_alpha = -2.0f;
	flash->d_scale = -2.0f;
	flash->r.frame = 1;

	const int white = irand(8, 16);
	COLOUR_SETA(flash->r.color, irand(236, 255), 64 + white, 16 + white, irand(80, 192)); //mxd. Use macro.

	flash->dlight = CE_DLight_new(flash->r.color, 150.0f, -100.0f);

	AddEffect(NULL, flash);

	// Spawn an explosion of lines.
	const int streak_count = GetScaledCount(16, 0.85f);

	for (int i = 0; i < streak_count; i++)
	{
		client_entity_t* streak = ClientEntity_new(FX_WEAPON_STAFF_STRIKE, (int)(flags & ~CEF_NO_DRAW), origin, NULL, 500);

		streak->r.model = &staffhit_models[2]; // firestreak sprite.
		streak->r.spriteType = SPRITE_LINE;
		streak->r.flags = RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		streak->r.scale = flrand(1.0f, 2.5f);
		streak->d_alpha = -1.0f;
		streak->d_scale = -1.0f;

		const int c = irand(128, 255);
		COLOUR_SETA(streak->r.color, c, c, irand(236, 255), irand(80, 192)); //mxd. Use macro.

		VectorRandomCopy(direction, streak->velocity, 1.25f);

		VectorCopy(origin, streak->r.endpos);
		VectorMA(streak->r.endpos, flrand(8.0f, 16.0f), streak->velocity, streak->r.startpos); //mxd. Original logic uses irand() here.

		VectorScale(streak->velocity, flrand(100.0f, 200.0f), streak->velocity); //mxd. Original logic uses irand() here.

		AddEffect(NULL, streak);
	}

	// Spawn smoke.
	const int smoke_count = GetScaledCount(4, 0.85f);

	for (int i = 0; i < smoke_count; i++)
	{
		client_entity_t* steam = ClientEntity_new(FX_WEAPON_STAFF_STRIKE, (int)(flags & ~CEF_NO_DRAW), origin, NULL, 1000);

		steam->r.model = &staffhit_models[3]; // steam sprite.
		steam->r.flags = RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		steam->r.scale = flrand(0.25f, 0.5f);
		steam->alpha = 0.9f;
		steam->d_alpha = -2.0f;
		steam->d_scale = 2.0f;

		const int c = irand(32, 64);
		COLOUR_SETA(steam->r.color, c, c, c, 128); //mxd. Use macro.

		VectorRandomCopy(direction, steam->velocity, 1.25f);

		VectorCopy(origin, steam->r.endpos);
		VectorMA(steam->r.endpos, flrand(16.0f, 48.0f), steam->velocity, steam->r.startpos); //mxd. Original logic uses irand() here.

		VectorScale(steam->velocity, flrand(10.0f, 50.0f), steam->velocity); //mxd. Original logic uses irand() here.
		steam->velocity[2] += 64.0f;

		AddEffect(NULL, steam);
	}
}

void FXStaffStrike(centity_t* owner, int type, const int flags, vec3_t origin)
{
	vec3_t dir;
	byte power_level;
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_WEAPON_STAFF_STRIKE].formatString, &dir, &power_level);

	switch (power_level)
	{
		case 2: StaffStrikeLevel2(flags, origin, dir); break; //mxd
		case 3: StaffStrikeLevel3(flags, origin, dir); break; //mxd
		default: break;
	}
}