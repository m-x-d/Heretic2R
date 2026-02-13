//
// fx_mork.c
//
// Copyright 1998 Raven Software
//

#include "fx_mork.h" //mxd
#include "fx_flamethrow.h" //mxd
#include "fx_HighPriestessProjectiles.h" //mxd
#include "fx_smoke.h" //mxd
#include "fx_sparks.h" //mxd
#include "Particle.h"
#include "Random.h"
#include "Vector.h"
#include "Utilities.h"
#include "ce_DLight.h"
#include "q_Sprite.h"
#include "g_playstats.h"

static struct model_s* mork_projectile_models[4];
static struct model_s* mork_lightning_models[3];
static struct model_s* mork_projectile_core_model;
static struct model_s* imp_models[2];
static struct model_s* cw_model;
static struct model_s* buoy_model;
static struct model_s* mork_model;
static struct model_s* assassin_dagger_model;

static struct model_s* morc_models[6];
static struct model_s* mssithra_models[6];

void PreCacheMEffects(void)
{
	mork_projectile_models[0] = fxi.RegisterModel("sprites/fx/hpproj1_1.sp2"); //TODO: this one is unused?
	mork_projectile_models[1] = fxi.RegisterModel("sprites/fx/hpproj1_2.sp2");
	mork_projectile_models[2] = fxi.RegisterModel("sprites/fx/segment_trail_wt.sp2");
	mork_projectile_models[3] = fxi.RegisterModel("sprites/lens/halo2.sp2");

	mork_lightning_models[0] = fxi.RegisterModel("sprites/fx/lightning.sp2");
	mork_lightning_models[1] = fxi.RegisterModel("sprites/fx/rlightning.sp2");
	mork_lightning_models[2] = fxi.RegisterModel("sprites/fx/neon.sp2");

	mork_projectile_core_model = fxi.RegisterModel("sprites/fx/core_b.sp2");

	imp_models[0] = fxi.RegisterModel("sprites/fx/halo.sp2");
	imp_models[1] = fxi.RegisterModel("sprites/fx/fire.sp2");

	cw_model = fxi.RegisterModel("sprites/spells/patball.sp2");
	buoy_model = fxi.RegisterModel("sprites/fx/segment_trail_buoy.sp2");
	mork_model = fxi.RegisterModel("models/monsters/morcalavin/tris.fm");
	assassin_dagger_model = fxi.RegisterModel("models/monsters/assassin/dagger/tris.fm");

	morc_models[0] = fxi.RegisterModel("sprites/fx/neon.sp2");
	morc_models[1] = fxi.RegisterModel("sprites/fx/lightning.sp2");
	morc_models[2] = fxi.RegisterModel("sprites/fx/hpproj1_2.sp2");
	morc_models[3] = fxi.RegisterModel("sprites/fx/hp_halo.sp2");
	morc_models[4] = fxi.RegisterModel("sprites/fx/morc_halo.sp2");
	morc_models[5] = fxi.RegisterModel("sprites/fx/segment_trail.sp2");

	mssithra_models[0] = fxi.RegisterModel("models/fx/explosion/inner/tris.fm");
	mssithra_models[1] = fxi.RegisterModel("models/fx/explosion/outer/tris.fm");
	mssithra_models[2] = fxi.RegisterModel("sprites/fx/firestreak.sp2");
	mssithra_models[3] = fxi.RegisterModel("models/debris/stone/schunk1/tris.fm");
	mssithra_models[4] = fxi.RegisterModel("models/debris/stone/schunk2/tris.fm");
	mssithra_models[5] = fxi.RegisterModel("sprites/lens/halo2.sp2");
}

static qboolean CWStarsTrailUpdate(client_entity_t* self, centity_t* owner) //mxd. Named 'FXCWTrailThink' in original logic.
{
	if (self->alpha <= 0.1f || self->r.scale <= 0.0f)
		return false;

	self->r.scale -= 0.15f;

	VectorCopy(owner->lerp_origin, self->r.origin);
	VectorCopy(self->r.origin, self->r.startpos);
	Vec3AddAssign(self->up, self->direction);

	vec3_t forward;
	AngleVectors(self->direction, forward, NULL, NULL);
	VectorMA(self->r.startpos, (float)self->SpawnInfo, forward, self->r.endpos);

	return true;
}

static qboolean MorkBeamTrailUpdate(client_entity_t* self, centity_t* owner) //mxd. Named 'FXMorkTrailThink' in original logic.
{
	if (self->alpha <= 0.1f || self->r.scale <= 0.0f)
		return false;

	self->r.scale -= 0.1f;
	return true;
}

static qboolean CWStarsTrailHeadUpdate(client_entity_t* self, centity_t* owner) //mxd. Named 'FXMorkTrailThink2' in original logic.
{
	if (self->alpha <= 0.1f || self->r.scale <= 0.0f)
		return false;

	self->r.scale -= 0.15f;
	return true;
}

//mxd. Added to reduce code duplication.
static void AddGenericExplosion(const centity_t* owner, vec3_t dir, struct model_s** model)
{
	Vec3ScaleAssign(32.0f, dir);

	const int count = GetScaledCount(irand(12, 16), 0.8f);

	for (int i = 0; i < count; i++)
	{
		const int next_think_time = (i == count - 1 ? 500 : 1000); //mxd
		client_entity_t* spark = ClientEntity_new(FX_M_EFFECTS, 0, owner->origin, NULL, next_think_time);

		spark->r.model = model;
		spark->r.flags = (RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA);
		spark->r.scale = flrand(0.5f, 1.0f);
		spark->d_scale = -2.0f;
		spark->d_alpha = -0.4f;
		spark->radius = 20.0f;

		VectorRandomCopy(dir, spark->velocity, flrand(16.0f, 64.0f));

		spark->acceleration[0] = flrand(-400.0f, 400.0f);
		spark->acceleration[1] = flrand(-400.0f, 400.0f);
		spark->acceleration[2] = flrand(-60.0f, -40.0f);

		AddEffect(NULL, spark);
	}
}

static void MorkMissileExplode(const centity_t* owner, vec3_t dir)
{
	AddGenericExplosion(owner, dir, &mork_projectile_models[1]); //mxd. Blue spark sprite.
}

static client_entity_t* MorkMakeLightningPiece(const vec3_t start, const vec3_t end, const float radius, const int lifetime)
{
#define M_LIGHTNING_WIDTH	6.0f
#define M_LIGHTNING_WIDTH2	8.0f

	// Blue lightning.
	client_entity_t* lightning_b = ClientEntity_new(-1, CEF_DONT_LINK, start, NULL, lifetime);

	lightning_b->r.model = &mork_lightning_models[0]; // Blue lightning sprite.
	lightning_b->r.flags = (RF_TRANS_ADD | RF_TRANS_ADD_ALPHA);
	lightning_b->r.scale = M_LIGHTNING_WIDTH;
	lightning_b->r.tile = VectorSeparation(end, start) / 32.0f;
	lightning_b->r.spriteType = SPRITE_LINE;

	lightning_b->radius = radius;
	lightning_b->alpha = 0.95f;
	lightning_b->d_alpha = -4.0f;
	VectorCopy(start, lightning_b->r.startpos);
	VectorCopy(end, lightning_b->r.endpos);

	AddEffect(NULL, lightning_b);

	// Red lightning.
	client_entity_t* lightning_r = ClientEntity_new(-1, CEF_DONT_LINK, start, NULL, lifetime * 2);

	lightning_r->r.model = &mork_lightning_models[1]; // Red lightning sprite.
	lightning_r->r.frame = irand(0, 1);
	lightning_r->r.flags = (RF_TRANS_ADD | RF_TRANS_ADD_ALPHA);
	lightning_r->r.scale = M_LIGHTNING_WIDTH2;
	lightning_r->r.tile = 1.0f;
	lightning_r->r.spriteType = SPRITE_LINE;

	lightning_r->radius = radius;
	lightning_r->alpha = 0.5f;
	lightning_r->d_alpha = -1.25f;
	VectorCopy(start, lightning_r->r.startpos);
	VectorCopy(end, lightning_r->r.endpos);

	AddEffect(NULL, lightning_r);

	return lightning_r;
}

static qboolean MorkBeamCircleUpdate(client_entity_t* self, centity_t* owner) //mxd. Named 'FXMorkBeamCircle' in original logic.
{
	self->LifeTime += 54;

	vec3_t up;
	const vec3_t angles = VEC3_SET(self->r.angles[PITCH], self->r.angles[YAW], anglemod((float)self->LifeTime));
	AngleVectors(angles, NULL, NULL, up);
	VectorMA(owner->current.origin, 12.0f, up, self->r.origin);

	MorkMakeLightningPiece(self->startpos, self->r.origin, 2000.0f, 1000);
	VectorCopy(self->r.origin, self->startpos);

	return true;
}

static qboolean MorkBeamUpdate(client_entity_t* self, centity_t* owner) //mxd. Named 'FXMorkBeam' in original logic.
{
	static const int particle_types[] =
	{
		PART_4x4_WHITE,
		PART_16x16_STAR,
		PART_32x32_BUBBLE,
		PART_16x16_SPARK_B,
		PART_8x8_BLUE_CIRCLE
	}; //mxd

	// Make inner beam.
	client_entity_t* beam_inner = ClientEntity_new(FX_M_EFFECTS, CEF_DONT_LINK, owner->origin, NULL, 17);

	beam_inner->radius = 2000.0f;
	VectorCopy(owner->origin, beam_inner->origin);

	beam_inner->r.flags = (RF_TRANSLUCENT | RF_TRANS_ADD_ALPHA);
	beam_inner->r.model = &mork_projectile_models[2];
	beam_inner->r.spriteType = SPRITE_LINE;
	beam_inner->r.tile = 1.0f;
	beam_inner->alpha = 2.0f;

	VectorCopy(self->startpos, beam_inner->r.startpos);
	VectorCopy(owner->origin, beam_inner->r.endpos);

	beam_inner->d_alpha = -1.0f;
	beam_inner->d_scale = -0.1f;
	beam_inner->Update = MorkBeamTrailUpdate;

	AddEffect(NULL, beam_inner);

	// Make outer beam.
	client_entity_t* beam_outer = ClientEntity_new(FX_M_EFFECTS, CEF_DONT_LINK, owner->origin, NULL, 17);

	beam_outer->radius = 2000.0f;
	VectorCopy(owner->origin, beam_outer->origin);

	beam_outer->r.flags = (RF_TRANSLUCENT | RF_TRANS_ADD_ALPHA);
	beam_outer->r.model = &mork_projectile_models[2];
	beam_outer->r.spriteType = SPRITE_LINE;
	beam_outer->r.tile = 1.0f;
	beam_outer->r.scale = 4.0f;
	COLOUR_SET(beam_outer->r.color, 100, 75, 250); //mxd. Use macro.

	VectorCopy(self->startpos, beam_outer->r.startpos);
	VectorCopy(owner->origin, beam_outer->r.endpos);

	beam_outer->d_alpha = -0.6f;
	beam_outer->d_scale = -0.5f;
	beam_outer->Update = MorkBeamTrailUpdate;

	AddEffect(NULL, beam_outer);

	VectorCopy(owner->origin, self->startpos);

	// Create particles.
	int num_particles = (int)(floorf((float)(irand(6, 9)) * self->r.scale));
	num_particles = min(500, num_particles);

	for (int i = 0; i < num_particles; i++)
	{
		const int particle_type = particle_types[irand(0, sizeof(particle_types) / sizeof(particle_types[0]) - 1)];
		client_particle_t* spark = ClientParticle_new(particle_type, self->r.color, 20);

		spark->scale = flrand(1.0f, 2.0f);
		spark->d_scale = flrand(-1.5f, -1.0f);
		COLOUR_SETA(spark->color, 255, 255, 255, irand(100, 200)); //mxd. Use macro.
		spark->d_alpha = flrand(-60.0f, -42.0f);
		spark->duration = irand(1500, 3000);
		spark->acceleration[2] = flrand(10.0f, 20.0f);
		VectorRandomSet(spark->origin, 10.0f);

		AddParticleToList(beam_inner, spark);
	}

	return true;
}

static void MorkBeam(centity_t* owner, const int type, const vec3_t origin, const vec3_t vel) //mxd. Separated from FXMEffects().
{
	const paletteRGBA_t light_color = { .r = 0, .g = 0, .b = 255, .a = 255 };

	client_entity_t* beam = ClientEntity_new(type, CEF_NO_DRAW | CEF_OWNERS_ORIGIN | CEF_DONT_LINK, origin, NULL, 20);

	beam->radius = 500.0f;
	beam->r.flags = (RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA);
	beam->flags |= CEF_NO_DRAW;
	VectorCopy(owner->current.origin, beam->r.origin);
	beam->dlight = CE_DLight_new(light_color, 150.0f, 0.0f);

	beam->AddToView = LinkedEntityUpdatePlacement;
	beam->Update = MorkBeamUpdate;

	VectorCopy(owner->origin, beam->startpos);

	AddEffect(owner, beam);
	MorkBeamUpdate(beam, owner);

	for (int i = 0; i < 3; i++)
	{
		client_entity_t* trail = ClientEntity_new(type, 0, origin, NULL, 20);

		trail->radius = 500.0f;
		trail->r.model = &mork_projectile_models[3]; //TODO: segment_trail_wt is invisible!
		trail->r.flags = (RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA);
		trail->r.scale = 0.5f;
		trail->LifeTime = i * 120;
		trail->Update = MorkBeamCircleUpdate;

		VectorCopy(owner->current.origin, trail->r.origin);
		VectorCopy(owner->origin, trail->startpos);
		VectorCopy(vel, trail->r.angles);

		AddEffect(owner, trail);
		MorkBeamCircleUpdate(trail, owner);
	}
}

static void ImpFireBallExplode(const centity_t* owner, vec3_t dir)
{
	AddGenericExplosion(owner, dir, &imp_models[1]); //mxd. Fire spark sprite.
}

static qboolean ImpFireballUpdate(client_entity_t* self, centity_t* owner)
{
	vec3_t angles;
	VectorScale(self->r.angles, RAD_TO_ANGLE, angles);

	vec3_t forward;
	vec3_t right;
	AngleVectors(angles, forward, right, NULL);

	const paletteRGBA_t light_color = { .c = 0xe5007fff };
	const int num_parts = irand(3, 7);

	for (int i = 0; i < num_parts; i++)
	{
		client_particle_t* p = ClientParticle_new(irand(PART_32x32_FIRE0, PART_32x32_FIRE2), light_color, 1000);

		VectorRandomSet(p->origin, 4.0f);
		Vec3AddAssign(self->r.origin, p->origin);
		p->scale = flrand(0.1f, 0.5f);
		p->type |= PFL_ADDITIVE;

		VectorSet(p->velocity, flrand(-10.0f, 10.0f), flrand(-10.0f, 10.0f), flrand(-1.0f, 1.0f));

		// Make the fire shoot out the back and to the side.
		VectorMA(p->velocity, flrand(-40.0f, -10.0f), forward, p->velocity);

		// Alternate left and right side of phoenix.
		const float sign = ((i & 1) ? -1.0f : 1.0f); //mxd
		VectorMA(p->velocity, flrand(10, 2) * sign, right, p->velocity);

		p->acceleration[2] = flrand(2.0f, 10.0f);
		p->d_scale = flrand(-15.0f, -10.0f);
		p->d_alpha = flrand(-200.0f, -160.0f);
		p->duration = (int)((255.0f * 1000.0f) / -p->d_alpha); // Time taken to reach zero alpha.

		AddParticleToList(self, p);
	}

	// Trail.
	self->r.scale = flrand(0.35f, 0.65f);

	client_entity_t* trail = ClientEntity_new(FX_M_EFFECTS, CEF_DONT_LINK, owner->origin, NULL, 17);

	trail->radius = 2000.0f;
	VectorCopy(owner->origin, trail->origin);

	trail->r.flags = (RF_TRANSLUCENT | RF_TRANS_ADD_ALPHA | RF_TRANS_ADD);
	trail->r.model = &mork_projectile_models[2];
	COLOUR_SET(trail->r.color, 180, 60, 0); //mxd. Use macro.
	trail->r.spriteType = SPRITE_LINE;
	trail->r.tile = 1.0f;
	trail->r.scale = 3.0f;

	VectorCopy(self->startpos, trail->r.startpos);
	VectorCopy(owner->origin, trail->r.endpos);

	trail->d_alpha = -4.0f;
	trail->Update = CWStarsTrailHeadUpdate;

	AddEffect(NULL, trail);
	VectorCopy(owner->origin, self->startpos);

	return true;
}

static void ImpFireball(centity_t* owner, const vec3_t origin, const vec3_t vel) //mxd. Separated from FXMEffects().
{
	const int flags = (CEF_OWNERS_ORIGIN | CEF_DONT_LINK | CEF_ADDITIVE_PARTS | CEF_ABSOLUTE_PARTS);
	client_entity_t* fx = ClientEntity_new(FX_SPARKS, flags, origin, NULL, 20);

	fx->radius = 64.0f;
	fx->r.model = &imp_models[0];
	fx->r.flags = (RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA);
	fx->r.frame = 2;
	fx->r.scale = 0.5f;
	fx->r.color.c = 0xe5007fff;

	vectoangles(vel, fx->r.angles);
	VectorCopy(owner->origin, fx->startpos);

	fx->AddToView = LinkedEntityUpdatePlacement;
	fx->Update = ImpFireballUpdate;

	if (R_DETAIL > DETAIL_NORMAL)
	{
		const paletteRGBA_t light_color = { .c = 0xff3333ff };
		fx->dlight = CE_DLight_new(light_color, 150.0f, 0.0f);
	}

	AddEffect(owner, fx);
}

static qboolean CWStarsUpdate(client_entity_t* self, centity_t* owner) //mxd. Named 'FXCWUpdate' in original logic.
{
	static int star_particles[3] = { PART_16x16_STAR, PART_16x16_SPARK_C, PART_16x16_SPARK_B }; //mxd. Made local static.

	client_entity_t* spawner = ClientEntity_new(FX_M_EFFECTS, CEF_NO_DRAW | CEF_ABSOLUTE_PARTS, self->r.origin, NULL, 500);
	AddEffect(NULL, spawner);

	vec3_t angles;
	VectorScale(self->r.angles, RAD_TO_ANGLE, angles);

	vec3_t forward;
	vec3_t right;
	AngleVectors(angles, forward, right, NULL);

	const int num_particles = irand(3, 7);

	for (int i = 0; i < num_particles; i++)
	{
		client_particle_t* p = ClientParticle_new(star_particles[irand(0, 2)], color_white, 2000);

		VectorRandomSet(p->origin, 4.0f); //mxd
		Vec3AddAssign(self->r.origin, p->origin);
		p->scale = flrand(2.5f, 3.0f);

		VectorSet(p->velocity, flrand(-10.0f, 10.0f), flrand(-10.0f, 10.0f), flrand(-1.0f, 1.0f));
		VectorMA(p->velocity, flrand(-40.0f, -10.0f), forward, p->velocity);

		const float sign = ((i & 1) ? -1.0f : 1.0f); //mxd
		VectorMA(p->velocity, flrand(10, 2) * sign, right, p->velocity);

		p->acceleration[2] = 0.0f;
		p->d_scale = flrand(-0.15f, -0.1f);
		p->duration = (int)((p->scale * 1000.0f) / -p->d_scale); // Time taken to reach zero scale.

		AddParticleToList(spawner, p);
	}

	// Trail head.
	client_entity_t* trail_head = ClientEntity_new(FX_M_EFFECTS, CEF_DONT_LINK, owner->origin, NULL, 17);

	trail_head->radius = 2000.0f;
	trail_head->r.flags = (RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA);
	trail_head->r.model = &cw_model; // Patball sprite.
	trail_head->r.spriteType = SPRITE_LINE;
	trail_head->r.tile = 1.0f;
	trail_head->r.scale = 3.0f;

	VectorCopy(self->startpos, trail_head->r.startpos);
	VectorCopy(owner->current.origin, trail_head->r.endpos);

	trail_head->d_alpha = -2.0f;
	trail_head->Update = CWStarsTrailHeadUpdate;

	AddEffect(NULL, trail_head);

	// Line trail.
	client_entity_t* trail = ClientEntity_new(FX_M_EFFECTS, CEF_OWNERS_ORIGIN | CEF_AUTO_ORIGIN | CEF_USE_VELOCITY2, owner->current.origin, NULL, 17);

	trail->radius = 2000.0f;

	if (ref_soft)
	{
		trail->r.model = &cw_model; // Patball sprite.
		trail->r.scale = flrand(1.0f, 2.5f);
	}
	else
	{
		trail->r.model = &mork_projectile_models[2]; // White trail segment sprite.
		trail->flags |= CEF_USE_SCALE2;
		trail->r.scale = 3.0f;
		trail->r.scale2 = 0.2f;
	}

	trail->r.spriteType = SPRITE_LINE;

	trail->r.flags = (RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA);
	trail->r.color.c = 0xffffaacc;
	trail->alpha = flrand(0.75f, 1.0f);
	trail->d_alpha = -1.0f;
	trail->d_scale = -0.1f; // Outer part does not scale down.

	// Angle.
	VectorSet(trail->direction, flrand(0.0f, 359.0f), flrand(0.0f, 359.0f), flrand(0.0f, 359.0f));
	AngleVectors(trail->direction, forward, NULL, NULL);

	// Length.
	trail->SpawnInfo = irand(20, 70);
	VectorCopy(owner->current.origin, trail->r.startpos);
	VectorMA(owner->current.origin, (float)trail->SpawnInfo, forward, trail->r.endpos);

	// Angular velocity.
	VectorRandomSet(trail->up, 10.0f);

	// Speed.
	VectorCopy(self->direction, trail->velocity);
	VectorCopy(self->direction, trail->velocity2);

	trail->Update = CWStarsTrailUpdate;

	AddEffect(owner, trail);

	// Update self.
	self->r.scale = flrand(0.65f, 0.95f);
	VectorCopy(owner->current.origin, self->startpos);

	return true;
}

void FXCWStars(centity_t* owner, const int type, const vec3_t vel)
{
	client_entity_t* fx = ClientEntity_new(type, CEF_OWNERS_ORIGIN | CEF_DONT_LINK, owner->origin, NULL, 20);

	fx->radius = 500.0f;
	fx->r.model = &mork_projectile_core_model;
	COLOUR_SET(fx->r.color, 10, 50, 255); //mxd. Use macro.
	fx->r.flags = (RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA);
	fx->r.scale = 0.8f;

	VectorCopy(vel, fx->direction);
	VectorCopy(owner->origin, fx->startpos);

	fx->AddToView = LinkedEntityUpdatePlacement;
	fx->Update = CWStarsUpdate;

	AddEffect(owner, fx);
}

static qboolean BuoyUpdate(client_entity_t* self, centity_t* owner)
{
#define BUOY_FX_END			PART_4x4_RED
#define BUOY_FX_START		PART_4x4_GREEN
#define BUOY_FX_JUMP_FROM	PART_4x4_CYAN
#define BUOY_FX_JUMP_TO		PART_4x4_BLUE
#define BUOY_FX_ACTIVATE	PART_4x4_MAGENTA
#define BUOY_FX_ONEWAY		PART_4x4_WHITE

	int num_parts;
	const int type = (int)(self->acceleration2[2]);

	if ((type == BUOY_FX_START || type == BUOY_FX_END) && self->LifeTime < fx_time) // These effects time out.
		return false;

	if (owner != NULL)
	{
		if (owner->current.frame == 0)
			return false;

		num_parts = min(5, owner->current.frame);
	}
	else
	{
		num_parts = irand(1, 3);
	}

	for (int i = 0; i < num_parts; i++)
	{
		client_particle_t* p = ClientParticle_new(type, color_white, 1000);

		switch (type)
		{
			case BUOY_FX_END: // Red.
			{
				const vec3_t offset =
				{
					flrand(4.0f, 12.0f) * (float)(Q_sign(irand(-1, 0))),
					flrand(4.0f, 12.0f) * (float)(Q_sign(irand(-1, 0))),
					0.0f
				};

				VectorCopy(offset, p->origin);
				VectorCopy(offset, p->velocity);
				p->acceleration[2] = 0.0f;
			} break;

			case BUOY_FX_START: // Green.
				VectorSet(p->origin, flrand(-2.0f, 2.0f), flrand(-2.0f, 2.0f), flrand(8.0f, 16.0f));
				VectorSet(p->velocity, 0.0f, 0.0f, flrand(3.0f, 7.0f));
				p->acceleration[2] = flrand(0.05f, 2.0f);
				break;

			case BUOY_FX_JUMP_FROM: // Cyan.
			{
				const vec3_t offset =
				{
					flrand(4.0f, 12.0f) * (float)(Q_sign(irand(-1, 0))),
					flrand(4.0f, 12.0f) * (float)(Q_sign(irand(-1, 0))),
					0.0f
				};

				VectorSet(p->origin, offset[0], offset[1], 0.0f);
				VectorSet(p->velocity, offset[0], offset[1], 1.0f);
				p->acceleration[2] = 2.0f;
			} break;

			case BUOY_FX_JUMP_TO: // Blue.
			{
				const vec3_t offset =
				{
					8.0f * (float)(Q_sign(irand(-1, 0))),
					8.0f * (float)(Q_sign(irand(-1, 0))),
					-2.0f
				};

				VectorCopy(offset, p->origin);
				VectorCopy(offset, p->velocity);
				p->acceleration[2] = -2.0f;
			} break;

			case BUOY_FX_ACTIVATE: // Magenta.
			{
				vec3_t forward;
				const vec3_t angles = VEC3_SET(0.0f, self->yaw++, 0.0f);
				AngleVectors(angles, forward, NULL, NULL);

				VectorScale(forward, 8.0f, p->origin);
				p->origin[2] = 8.0f;
				VectorCopy(p->origin, p->velocity);
				p->acceleration[2] = 0.0f;
			} break;

			case BUOY_FX_ONEWAY: // White.
				VectorSet(p->origin, 0.0f, 0.0f, flrand(8.0f, 16.0f));
				VectorSet(p->velocity, 0.0f, 0.0f, 7.0f);
				p->acceleration[2] = flrand(0.05f, 2.0f);
				break;

			default:
				assert(0);
				break;
		}

		p->scale = flrand(0.5f, 1.0f);
		p->d_alpha = flrand(-200.0f, -160.0f);
		p->duration = (int)((255.0f * 1000.0f) / -p->d_alpha); // Time taken to reach zero alpha.

		AddParticleToList(self, p);
	}

	return true;
}

static void Buoy(centity_t* owner, const int flags, const vec3_t origin, const qboolean white)
{
	client_entity_t* fx;

	if (owner != NULL)
		fx = ClientEntity_new(FX_BUOY, CEF_OWNERS_ORIGIN, owner->current.origin, NULL, 50);
	else
		fx = ClientEntity_new(FX_BUOY, 0, origin, NULL, 50);

	if (white)
		fx->acceleration2[2] = BUOY_FX_ONEWAY; // White.
	else if (flags & CEF_FLAG6)
		fx->acceleration2[2] = BUOY_FX_START; // Green.
	else if (flags & CEF_FLAG7)
		fx->acceleration2[2] = BUOY_FX_JUMP_FROM; // Cyan.
	else if (flags & CEF_FLAG8)
		fx->acceleration2[2] = BUOY_FX_JUMP_TO; // Blue - maybe 3 - yellow?
	else if (flags & CEF_DONT_LINK)
		fx->acceleration2[2] = BUOY_FX_ACTIVATE; // Magenta.
	else
		fx->acceleration2[2] = BUOY_FX_END; // Red.

	fx->flags |= CEF_NO_DRAW;
	fx->LifeTime = fx_time + 10000;
	fx->Update = BuoyUpdate;

	if (owner != NULL)
	{
		AddEffect(owner, fx);
	}
	else
	{
		VectorCopy(origin, fx->startpos);
		AddEffect(NULL, fx);
	}
}

static qboolean BuoyPathDelayedStart(client_entity_t* self, centity_t* owner)
{
	client_entity_t* buoy = ClientEntity_new(FX_BUOY, CEF_DONT_LINK, self->origin, NULL, 16384);

	buoy->radius = 500.0f;

	buoy->r.flags = (RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA);
	buoy->r.model = &buoy_model; //TODO: fully transparent sprite! Use segment_trail_wt.sp2 instead?
	buoy->r.spriteType = SPRITE_LINE;
	buoy->r.scale = 7.0f;
	buoy->r.tile = (VectorSeparation(self->startpos, self->endpos) < 64.0f ? 1.0f : 3.0f);

	VectorCopy(self->startpos, buoy->r.startpos);
	VectorCopy(self->endpos, buoy->r.endpos);

	buoy->updateTime = 16384;
	buoy->Update = KeepSelfAI; //mxd. PermanentUpdate() in original logic.

	AddEffect(NULL, buoy);

	self->Update = RemoveSelfAI;
	self->updateTime = 100;

	return true;
}

static void BuoyPath(const vec3_t start, const vec3_t end)
{
	vec3_t origin;
	VectorAdd(start, end, origin);
	Vec3ScaleAssign(0.5f, origin);

	client_entity_t* fx = ClientEntity_new(FX_BUOY, CEF_DONT_LINK | CEF_NO_DRAW, origin, NULL, 100);

	fx->radius = 100.0f;
	fx->Update = BuoyPathDelayedStart;

	VectorCopy(start, fx->startpos);
	VectorCopy(end, fx->endpos);

	AddEffect(NULL, fx);
}

static qboolean MMoBlurUpdate(client_entity_t* self, centity_t* owner)
{
	return self->alpha > 0.05f;
}

static void MMoBlur(const centity_t* owner, const vec3_t origin, const vec3_t angles, const qboolean dagger)
{
	client_entity_t* blur;

	if (dagger)
	{
		blur = ClientEntity_new(FX_M_EFFECTS, 0, origin, NULL, 20);

		VectorCopy(angles, blur->r.angles);
		blur->r.model = &assassin_dagger_model;
		blur->alpha = 0.75f;
		blur->r.scale = 0.9f;
		blur->d_alpha = -3.0f;
		blur->d_scale = -0.3f;
	}
	else
	{
		blur = ClientEntity_new(FX_M_EFFECTS, 0, owner->current.origin, NULL, 20);

		VectorSet(blur->r.angles, angles[0] * -ANGLE_TO_RAD, angles[1] * ANGLE_TO_RAD, angles[2] * ANGLE_TO_RAD);
		blur->r.model = &mork_model;
		blur->r.frame = owner->current.frame;
		blur->d_alpha = -1.0f;
		blur->d_scale = -0.1f;
	}

	blur->r.flags = (RF_TRANSLUCENT | RF_TRANS_ADD_ALPHA | RF_GLOW);
	blur->r.scale = 1.0f;
	blur->updateTime = 20;
	blur->Update = MMoBlurUpdate;

	AddEffect(NULL, blur);
}

static qboolean AssassinDaggerUpdate(client_entity_t* self, centity_t* owner)
{
	if (++self->LifeTime == 4)
	{
		fxi.S_StartSound(self->r.origin, -1, CHAN_AUTO, fxi.S_RegisterSound(va("monsters/assassin/throw%i.wav", irand(1, 2))), 0.5f, ATTN_IDLE, 0.0f);
		self->LifeTime = 0;
	}

	MMoBlur(owner, self->r.origin, self->r.angles, true); //mxd. Pass owner to avoid compiler warning.
	self->r.angles[PITCH] += self->velocity2[PITCH];

	return true;
}

static void AssassinDagger(centity_t* owner, const vec3_t vel, const float avel)
{
	client_entity_t* dagger = ClientEntity_new(FX_M_EFFECTS, CEF_DONT_LINK, owner->current.origin, NULL, 20);

	VectorScale(owner->current.angles, ANGLE_TO_RAD, dagger->r.angles);
	dagger->r.model = &assassin_dagger_model;
	dagger->r.flags = RF_FULLBRIGHT;
	VectorCopy(vel, dagger->velocity);
	dagger->velocity2[0] = avel * ANGLE_TO_RAD;
	dagger->Update = AssassinDaggerUpdate;

	AddEffect(owner, dagger);
}

static qboolean UnderwaterWakeUpdate(client_entity_t* self, centity_t* owner)
{
	static const int water_particles[] = //mxd. Made local static.
	{
		PART_4x4_WHITE,
		PART_8x8_BUBBLE,
		PART_16x16_WATERDROP,
		PART_32x32_WFALL,
		PART_32x32_STEAM,
		PART_32x32_BUBBLE
	};

	static const paletteRGBA_t light_color = { .r = 200, .g = 255, .b = 255, .a = 140 };

	vec3_t right;
	AngleVectors(owner->lerp_angles, NULL, right, NULL);
	VectorCopy(owner->lerp_origin, self->r.origin);

	const int num_particles = irand(3, 7);

	for (int i = 0; i < num_particles; i++)
	{
		int particle_type = water_particles[irand(0, 5)];
		if (R_DETAIL == DETAIL_LOW)
			particle_type |= PFL_SOFT_MASK;

		client_particle_t* p = ClientParticle_new(particle_type, light_color, irand(1000, 1500));

		VectorSet(p->origin, flrand(-8.0f, 8.0f), flrand(-8.0f, 8.0f), flrand(-4.0f, 4.0f));
		Vec3AddAssign(self->r.origin, p->origin);

		p->scale = flrand(0.75f, 1.5f);
		p->color.a = (byte)irand(100, 200);

		VectorRandomSet(p->velocity, 2.0f);
		VectorMA(p->velocity, flrand(2.0f, 10.0f) * (float)(Q_sign(irand(-1, 0))), right, p->velocity);

		p->acceleration[2] = 2.0f;
		p->d_alpha = flrand(-300.0f, -200.0f);
		p->d_scale = flrand(-0.15f, -0.1f);

		AddParticleToList(self, p);
	}

	self->LifeTime--;

	return (self->LifeTime > 0);
}

static void UnderwaterWake(centity_t* owner)
{
	const int flags = (CEF_OWNERS_ORIGIN | CEF_NO_DRAW | CEF_ABSOLUTE_PARTS); //mxd
	client_entity_t* fx = ClientEntity_new(FX_M_EFFECTS, flags, owner->current.origin, NULL, 20);

	fx->radius = 30.0f;
	fx->LifeTime = 77;
	fx->Update = UnderwaterWakeUpdate;

	AddEffect(owner, fx);
}

static void QuakeRing(const vec3_t origin)
{
#define NUM_RIPPER_PUFFS	12
#define RIPPER_PUFF_ANGLE	(ANGLE_360 / (float)NUM_RIPPER_PUFFS)
#define MACEBALL_RING_VEL	256.0f
#define NUM_RINGS			3
#define RING_CEF_FLAGS		(CEF_USE_VELOCITY2 | CEF_AUTO_ORIGIN | CEF_ABSOLUTE_PARTS | CEF_ADDITIVE_PARTS) //mxd

	float ring_vel = MACEBALL_RING_VEL;
	vec3_t normal = VEC3_INIT(vec3_up);

	// Take the normal and find two "axis" vectors that are in the plane the normal defines.
	vec3_t up;
	PerpendicularVector(up, normal);

	vec3_t right;
	CrossProduct(up, normal, right);

	Vec3ScaleAssign(8.0f, normal);

	// Draw a circle of expanding lines.
	for (int i = 0; i < NUM_RINGS; i++)
	{
		float curyaw = 0.0f;

		vec3_t last_vel;
		VectorScale(right, ring_vel, last_vel);

		for (int c = 0; c < NUM_RIPPER_PUFFS; c++)
		{
			curyaw += RIPPER_PUFF_ANGLE;

			client_entity_t* ring = ClientEntity_new(FX_M_EFFECTS, RING_CEF_FLAGS, origin, NULL, 3000);

			ring->radius = 256.0f;
			ring->r.model = &morc_models[0];
			ring->r.spriteType = SPRITE_LINE;
			ring->r.frame = 1;
			ring->r.flags = (RF_TRANS_ADD | RF_TRANS_ADD_ALPHA);
			ring->r.tile = 1.0f;

			// The startpos and startvel comes from the last velocity.
			VectorCopy(last_vel, ring->velocity);
			VectorCopy(ring->velocity, ring->acceleration);
			VectorMA(origin, 0.01f, ring->velocity, ring->r.startpos); // Move the line out a bit to avoid a zero-length line.

			VectorScale(up, ring_vel * sinf(curyaw), ring->velocity2);
			VectorMA(ring->velocity2, ring_vel * cosf(curyaw), right, ring->velocity2);

			VectorCopy(ring->velocity2, ring->acceleration2);
			VectorMA(origin, 0.01f, ring->velocity2, ring->r.endpos); // Move the line out a bit to avoid a zero-length line.

			// Finally, copy the last velocity we used.
			VectorCopy(ring->velocity2, last_vel);

			// NOW apply the extra directional velocity to force it slightly away from the surface.
			Vec3AddAssign(normal, ring->velocity);
			Vec3AddAssign(normal, ring->velocity2);

			ring->r.scale = 8.0f;
			ring->d_scale = 32.0f;
			ring->alpha = 0.75f;
			ring->d_alpha = -1.0f;

			AddEffect(NULL, ring);
		}

		ring_vel /= 2.0f;
	}

	fxi.Activate_Screen_Shake(12.0f, 1000.0f, (float)fxi.cl->time, SHAKE_ALL_DIR); // 'current_time' MUST be cl.time, because that's what used by Perform_Screen_Shake() to calculate effect intensity/timing... --mxd.
	fxi.S_StartSound(origin, -1, CHAN_AUTO, fxi.S_RegisterSound("world/quakeshort.wav"), 1.0f, ATTN_NONE, 0.0f);
}

static void GroundAttack(vec3_t origin)
{
	origin[2] -= 16.0f;

	// Create the dummy entity, so particles can be attached.
	client_entity_t* spawner = ClientEntity_new(FX_M_EFFECTS, CEF_NO_DRAW | CEF_ADDITIVE_PARTS, origin, NULL, MIN_UPDATE_TIME);

	spawner->radius = 100.0f;
	spawner->LifeTime = fx_time + 1000;
	VectorScale(vec3_up, 50.0f, spawner->direction);
	spawner->Update = FlamethrowerUpdate;

	fxi.S_StartSound(origin, -1, CHAN_AUTO, fxi.S_RegisterSound("misc/flamethrow.wav"), 1.0f, ATTN_NORM, 0.0f);

	AddEffect(NULL, spawner);
}

static qboolean MorkBeam2AddToView(client_entity_t* self, centity_t* owner) //mxd. Named 'beam_add_to_view' in original logic.
{
	LinkedEntityUpdatePlacement(self, owner);
	VectorCopy(self->r.origin, self->r.endpos);

	return true;
}

static void MorkBeam2(centity_t* owner, const vec3_t startpos)
{
	client_entity_t* fx = ClientEntity_new(FX_M_EFFECTS, CEF_OWNERS_ORIGIN | CEF_DONT_LINK, owner->origin, NULL, 17);

	fx->radius = 1024.0f;
	fx->r.model = &mork_projectile_models[2];
	fx->r.spriteType = SPRITE_LINE;
	fx->r.scale = 8.0f;

	VectorCopy(startpos, fx->r.startpos);
	VectorCopy(owner->origin, fx->r.endpos);

	fx->AddToView = MorkBeam2AddToView;
	fx->Update = KeepSelfAI;

	AddEffect(owner, fx);
}

static qboolean MorkMissileAddToView(client_entity_t* self, centity_t* owner)
{
	LinkedEntityUpdatePlacement(self, owner);
	VectorCopy(self->r.origin, self->r.startpos);

	for (int i = 0; i < 3; i++)
		self->direction[i] += flrand(-1.0f, 1.0f);

	VectorNormalize(self->direction);
	VectorMA(self->r.startpos, (float)(irand(self->LifeTime / 4, self->LifeTime)), self->direction, self->r.endpos);

	self->r.scale = flrand(1.0f, 2.0f);

	return true;
}

static qboolean MorkMissileUpdate(client_entity_t* self, centity_t* owner) //mxd. Named 'MorkMissileThink1' in original logic.
{
	if (self->LifeTime < 24)
		self->LifeTime += 1;

	return true;
}

static qboolean MorkMissileHaloUpdate(client_entity_t* self, centity_t* owner) //mxd. Named 'MorkMissileThink2' in original logic.
{
	if (self->alpha < 0.25f)
		self->alpha += 0.1f;

	if (self->r.scale < 3.0f)
		self->r.scale += 0.1f;

	if (self->dlight->intensity <= 200.0f)
		self->dlight->intensity += 10.0f;

	return true;
}

static qboolean MorkMissileCoreUpdate(client_entity_t* self, centity_t* owner) //mxd. Named 'MorkMissileThink3' in original logic.
{
	if (self->alpha < 0.5f)
		self->alpha += 0.1f;

	if (self->r.scale < 1.0f)
		self->r.scale += 0.1f;

	if (self->SpawnInfo > irand(15, 20))
	{
		fxi.S_StartSound(self->r.origin, -1, CHAN_AUTO, fxi.S_RegisterSound("monsters/elflord/weld.wav"), 0.5f, ATTN_IDLE, 0.0f);
		self->SpawnInfo = 0;
	}

	self->SpawnInfo++;
	return true;
}

static void MorkMissile(centity_t* owner, vec3_t startpos)
{
	static const paletteRGBA_t light_color = { .r = 128, .g = 128,.b = 255, .a = 255 };

	const int count = GetScaledCount(8, 0.85f);

	for (int i = 0; i < count; i++)
	{
		client_entity_t* fx = ClientEntity_new(FX_M_EFFECTS, CEF_OWNERS_ORIGIN, startpos, NULL, 17);

		fx->r.spriteType = SPRITE_LINE;
		fx->r.flags |= (RF_TRANS_ADD | RF_TRANSLUCENT | RF_FULLBRIGHT);

		fx->radius = 1024.0f;
		fx->r.model = &morc_models[1]; // Lightning sprite.
		fx->r.scale = flrand(0.1f, 1.0f); //mxd. Was irand().
		fx->r.scale2 = 0.1f;

		VectorCopy(startpos, fx->r.startpos);
		VectorRandomSet(fx->direction, 1.0f);
		VectorMA(startpos, flrand(4.0f, 16.0f), fx->direction, fx->r.endpos); //mxd. Was irand().

		fx->AddToView = MorkMissileAddToView;
		fx->Update = MorkMissileUpdate;

		AddEffect(owner, fx);
	}

	// Light-blue halo.
	client_entity_t* halo = ClientEntity_new(FX_M_EFFECTS, CEF_OWNERS_ORIGIN | CEF_DONT_LINK, startpos, NULL, 100);
	halo->r.model = &morc_models[2];
	halo->r.flags = (RF_TRANS_ADD | RF_TRANSLUCENT | RF_FULLBRIGHT);
	halo->alpha = 0.1f;
	halo->r.scale = 0.1f;
	halo->dlight = CE_DLight_new(light_color, 10.0f, 0.0f);

	halo->AddToView = LinkedEntityUpdatePlacement;
	halo->Update = MorkMissileHaloUpdate;

	AddEffect(owner, halo);

	// The white core.
	client_entity_t* core = ClientEntity_new(FX_M_EFFECTS, CEF_OWNERS_ORIGIN | CEF_DONT_LINK, startpos, NULL, 100);
	core->r.model = &morc_models[3];
	core->r.flags = (RF_TRANS_ADD | RF_TRANSLUCENT | RF_FULLBRIGHT);
	core->alpha = 0.1f;
	core->r.scale = 0.1f;

	core->AddToView = LinkedEntityUpdatePlacement;
	core->Update = MorkMissileCoreUpdate;

	AddEffect(owner, core);
}

static void MorkMissileHit(const vec3_t origin)
{
	// The white core.
	client_entity_t* core = ClientEntity_new(FX_M_EFFECTS, CEF_OWNERS_ORIGIN | CEF_DONT_LINK, origin, NULL, 2000);

	core->r.model = &morc_models[3];
	core->r.flags = (RF_TRANS_ADD | RF_TRANS_ADD_ALPHA | RF_TRANSLUCENT | RF_FULLBRIGHT | RF_NODEPTHTEST);
	core->alpha = 0.5f;
	core->d_alpha = -1.0f;
	core->d_scale = 16.0f;

	AddEffect(NULL, core);

	fxi.S_StartSound(origin, -1, CHAN_AUTO, fxi.S_RegisterSound("monsters/mork/ppexplode.wav"), 1.0f, ATTN_IDLE, 0.0f);
}

static qboolean MorkTrackingMissileTrailTrailUpdate(client_entity_t* self, centity_t* owner) //mxd. Named 'FXMTrailThink' in original logic.
{
	if (self->alpha > 0.1f && self->r.scale > 0.0f)
	{
		self->r.scale -= 0.1f;
		self->r.scale2 -= 0.1f;

		return true;
	}

	return false;
}

static qboolean MorkTrackingMissileTrailUpdate(client_entity_t* self, centity_t* owner) //mxd. Named 'FXMMissileTrailThink' in original logic.
{
	self->r.scale = flrand(0.35f, 0.65f);

	client_entity_t* trail = ClientEntity_new(FX_M_EFFECTS, CEF_DONT_LINK, owner->lerp_origin, NULL, 17);

	trail->radius = 500.0f;
	trail->r.flags = (RF_TRANSLUCENT | RF_TRANS_ADD_ALPHA);
	trail->r.model = &morc_models[5]; // Blue segment trail sprite.

	trail->r.spriteType = SPRITE_LINE;
	trail->r.tile = 1.0f;
	trail->r.scale = 2.0f;
	trail->r.scale2 = 2.0f;
	trail->alpha = 0.5f;
	trail->d_alpha = -2.5f;

	VectorCopy(owner->lerp_origin, trail->r.origin);
	VectorCopy(self->startpos, trail->r.startpos);
	VectorCopy(owner->lerp_origin, trail->r.endpos);

	trail->Update = MorkTrackingMissileTrailTrailUpdate;

	AddEffect(NULL, trail);

	VectorCopy(owner->lerp_origin, self->startpos);

	return true;
}

static void MorkTrackingMissile(centity_t* owner, const vec3_t origin) //mxd. Named 'FXMorkTrackingMissile' in original logic.
{
	static const paletteRGBA_t light_color = { .r = 0,.g = 0,.b = 255,.a = 255 };

	FXHPMissileCreateWarp(FX_M_EFFECTS, origin);

	client_entity_t* trail = ClientEntity_new(FX_M_EFFECTS, CEF_OWNERS_ORIGIN | CEF_DONT_LINK, origin, NULL, 20);

	trail->radius = 500.0f;
	trail->r.model = &morc_models[4]; // Morc halo sprite.
	trail->r.flags = (RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA);
	trail->dlight = CE_DLight_new(light_color, 150.0f, 0.0f);

	VectorCopy(origin, trail->startpos);

	trail->AddToView = LinkedEntityUpdatePlacement;
	trail->Update = MorkTrackingMissileTrailUpdate;

	AddEffect(owner, trail);

	MorkTrackingMissileTrailUpdate(trail, owner);
}

static qboolean MSsithraExplosionRubbleUpdate(client_entity_t* self, centity_t* owner) //mxd. Named 'rubble_spin' in original logic.
{
	if (self->LifeTime < fx_time)
		return false;

	for (int i = 0; i < 3; i++)
		self->r.angles[i] += 0.1f;

	return true;
}

static qboolean MSsithraExplosionUpdate(client_entity_t* self, centity_t* owner) //mxd. Named 'mssithra_explosion_think' in original logic.
{
	if (self->LifeTime < fx_time)
		return false;

	// Spawn a white core.
	client_entity_t* core = ClientEntity_new(FX_M_EFFECTS, 0, self->origin, NULL, 1000);

	core->radius = 500.0f;
	core->r.model = &mssithra_models[5]; // Halo sprite.
	core->r.flags = (RF_FULLBRIGHT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA);
	core->r.scale = 0.1f;
	core->alpha = 0.75f;
	core->d_scale = 4.0f;
	core->d_alpha = -2.5f;

	for (int i = 0; i < 3; i++)
		core->r.origin[i] += flrand(-8.0f, 8.0f); //mxd. irand() in original logic.

	AddEffect(NULL, core);

	int count = GetScaledCount(3, 0.85f);

	// Spawn a small explosion sphere.
	for (int i = 0; i < count; i++)
	{
		client_entity_t* explosion = ClientEntity_new(FX_M_EFFECTS, 0, self->origin, NULL, 1000);

		explosion->radius = 500.0f;
		explosion->r.model = &mssithra_models[irand(0, 1)]; // Inner or outer explosion model.
		explosion->r.flags = RF_FULLBRIGHT;
		explosion->r.scale = 0.1f;
		explosion->alpha = 0.75f;
		explosion->d_scale = 2.0f;
		explosion->d_alpha = -2.5f;

		for (int c = 0; c < 3; c++)
			explosion->r.origin[c] += flrand(-16.0f, 16.0f); //mxd. irand() in original logic.

		AddEffect(NULL, explosion);
	}

	if (irand(0, 1))
	{
		if (R_DETAIL < DETAIL_HIGH)
			return true;

		// Spawn an explosion of stone chunks.
		count = GetScaledCount(2, 0.85f);

		for (int i = 0; i < count; i++)
		{
			client_entity_t* rubble = ClientEntity_new(FX_M_EFFECTS, 0, self->r.origin, 0, 17);

			rubble->r.model = &mssithra_models[irand(3, 4)]; // Stone chunk models.
			rubble->r.flags = RF_FULLBRIGHT; //TODO: why fullbright chunks?
			rubble->r.scale = flrand(0.5f, 1.5f);

			VectorRandomCopy(self->direction, rubble->velocity, 1.25f);

			Vec3ScaleAssign(flrand(50.0f, 100.0f), rubble->velocity); //mxd. Was irand().
			rubble->acceleration[2] -= 256.0f;

			rubble->LifeTime = fx_time + 2000;
			rubble->Update = MSsithraExplosionRubbleUpdate;

			AddEffect(NULL, rubble);
		}
	}
	else
	{
		// Spawn an explosion of lines.
		count = GetScaledCount(2, 0.85f);

		for (int i = 0; i < count; i++)
		{
			client_entity_t* fire_streak = ClientEntity_new(FX_M_EFFECTS, 0, self->r.origin, 0, 500);

			fire_streak->r.model = &mssithra_models[2]; // Firestreak sprite.
			fire_streak->r.spriteType = SPRITE_LINE;

			fire_streak->r.flags = (RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA);
			fire_streak->r.scale = flrand(1.0f, 2.5f);
			fire_streak->d_alpha = -1.0f;
			fire_streak->d_scale = -1.0f;

			const int white = irand(128, 255);
			COLOUR_SETA(fire_streak->r.color, white, white, irand(236, 255), irand(80, 192)); //mxd. Use macro.

			VectorRandomCopy(self->direction, fire_streak->velocity, 1.25f);

			VectorCopy(self->r.origin, fire_streak->r.endpos);
			VectorMA(fire_streak->r.endpos, flrand(16.0f, 32.0f), fire_streak->velocity, fire_streak->r.startpos); //mxd. Was irand().

			Vec3ScaleAssign(flrand(50.0f, 150.0f), fire_streak->velocity); //mxd. Was irand().

			AddEffect(NULL, fire_streak);
		}
	}

	return true;
}

static void MSsithraExplode(vec3_t origin, vec3_t dir)
{
	// Create an explosion spawner.
	client_entity_t* spawner = ClientEntity_new(FX_M_EFFECTS, CEF_NO_DRAW, origin, NULL, 20);

	spawner->color.c = 0xff00ffff;
	spawner->dlight = CE_DLight_new(spawner->color, 100.0f, -50.0f);
	VectorCopy(dir, spawner->direction);
	spawner->LifeTime = fx_time + 250;
	spawner->Update = MSsithraExplosionUpdate;

	AddEffect(NULL, spawner);

	fxi.S_StartSound(origin, -1, CHAN_AUTO, fxi.S_RegisterSound("monsters/mssithra/hit.wav"), 0.5f, ATTN_NORM, 0.0f);
}

static void MSsithraExplodeSmall(const vec3_t origin)
{
	//TODO: play correct sound here.
	FireSparks(NULL, FX_SPARKS, 0, origin, vec3_up);
	fxi.S_StartSound(origin, -1, CHAN_AUTO, fxi.S_RegisterSound("monsters/mssithra/hit.wav"), 0.5f, ATTN_NORM, 0.0f);
}

static qboolean MSsithraArrowUpdate(client_entity_t* self, centity_t* owner) //mxd. Named 'ArrowCheckFuse' in original logic.
{
	if ((owner->current.effects & EF_ALTCLIENTFX) || (owner->current.effects & EF_MARCUS_FLAG1))
	{
		// We've stopped moving and have embedded ourself in a wall.
		if (!(self->flags & CEF_NO_DRAW))
		{
			FireSparks(NULL, FX_SPARKS, 0, self->r.origin, vec3_up);
			self->flags |= CEF_NO_DRAW;
		}

		if (irand(0, 1))
			FXDarkSmoke(self->r.origin, flrand(0.2f, 0.5f), flrand(30.0f, 50.0f));
	}

	return true;
}

static qboolean MSsithraArrowAddToView(client_entity_t* self, centity_t* owner) //mxd. Named 'ArrowDrawTrail' in original logic.
{
	LinkedEntityUpdatePlacement(self, owner);

	VectorCopy(self->r.origin, self->r.startpos);
	VectorMA(self->r.startpos, (float)self->SpawnInfo, self->direction, self->r.endpos);
	VectorMA(self->r.startpos, 8.0f, self->direction, self->r.startpos);

	if (self->flags & CEF_FLAG6)
	{
		if (self->r.scale > 8.0f)
		{
			self->r.scale = flrand(8.0f, 12.0f);
			self->r.scale2 = self->r.scale;
		}

		if (self->SpawnInfo > -64)
			self->SpawnInfo -= 4;

		if (self->LifeTime > 10)
		{
			self->LifeTime = 0;
			fxi.S_StartSound(self->r.origin, -1, CHAN_AUTO, fxi.S_RegisterSound("monsters/pssithra/guntravel.wav"), 0.5f, ATTN_NORM, 0.0f);
		}
		else
		{
			self->LifeTime++;
		}
	}
	else
	{
		if (self->r.scale > 4.0f)
		{
			self->r.scale = flrand(4.0f, 6.0f);
			self->r.scale2 = self->r.scale;
		}

		// Let the trail slowly extend.
		if (self->SpawnInfo > -64)
			self->SpawnInfo -= 2;
	}

	self->alpha = flrand(0.5f, 1.0f);

	return true;
}

static void MSsithraArrow(centity_t* owner, vec3_t velocity, const qboolean super) //mxd. Named 'FXMSsithraArrow' in original logic.
{
	// Create an explosion spawner.
	client_entity_t* spawner = ClientEntity_new(FX_M_EFFECTS, CEF_OWNERS_ORIGIN, owner->current.origin, NULL, 20);

	spawner->r.model = &mssithra_models[2]; // Firestreak sprite.
	spawner->r.spriteType = SPRITE_LINE;
	spawner->r.flags = (RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA);

	if (super)
	{
		spawner->flags |= CEF_FLAG6;
		spawner->d_scale = 16.0f;
	}
	else
	{
		spawner->d_scale = 8.0f;
	}

	spawner->d_scale2 = spawner->d_scale;

	VectorCopy(spawner->r.origin, spawner->r.startpos);
	VectorNormalize2(velocity, spawner->direction);
	VectorMA(spawner->r.startpos, -64.0f, spawner->direction, spawner->r.endpos);

	spawner->AddToView = MSsithraArrowAddToView;
	spawner->Update = MSsithraArrowUpdate;

	AddEffect(owner, spawner);
}

static void MSsithraArrowCharge(vec3_t startpos)
{
	const int count = GetScaledCount(6, 0.85f);

	for (int i = 0; i < count; i++)
	{
		client_entity_t* firestreak = ClientEntity_new(FX_M_EFFECTS, 0, startpos, NULL, 500);

		firestreak->r.model = &mssithra_models[2]; // Firestreak sprite.
		firestreak->r.spriteType = SPRITE_LINE;

		firestreak->r.flags = (RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA);
		firestreak->flags |= CEF_USE_VELOCITY2;
		firestreak->r.scale = flrand(4.0f, 6.0f);
		firestreak->alpha = 0.1f;
		firestreak->d_alpha = 0.25f;

		const int white = irand(128, 255);
		COLOUR_SETA(firestreak->r.color, white, white, irand(236, 255), irand(80, 192)); //mxd. Use macro.

		VectorRandomCopy(vec3_up, firestreak->velocity2, 1.5f);
		VectorCopy(startpos, firestreak->r.startpos);

		const float length = flrand(24.0f, 32.0f); //mxd. Was int / irand().
		VectorMA(firestreak->r.startpos, length, firestreak->velocity2, firestreak->r.endpos);

		Vec3ScaleAssign(-(length * 2.0f), firestreak->velocity2);
		VectorClear(firestreak->velocity);

		AddEffect(NULL, firestreak);
	}

	//TODO: AddEffect() is not called on this one. Is that OK or MEMORY LEAK?..
	client_entity_t* flash = ClientEntity_new(FX_M_EFFECTS, 0, startpos, NULL, 500);

	const int white = irand(128, 255);
	COLOUR_SETA(flash->r.color, white, white, irand(236, 255), irand(80, 192)); //mxd. Use macro.

	flash->dlight = CE_DLight_new(flash->r.color, 200.0f, -25.0f);
}

// Morcalavin's FX handler.
void FXMEffects(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	byte fx_index;
	vec3_t vel;
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_M_EFFECTS].formatString, &fx_index, &vel); //FIXME: make this 1 dir and 1 float.

	switch (fx_index)
	{
		case FX_M_MISC_EXPLODE:
			MorkMissileExplode(owner, vel);
			break;

		case FX_M_BEAM:
			MorkBeam(owner, type, origin, vel); //mxd
			break;

		case FX_IMP_FIRE:
			ImpFireball(owner, origin, vel); //mxd
			break;

		case FX_IMP_FBEXPL:
			ImpFireBallExplode(owner, vel);
			break;

		case FX_CW_STARS:
			FXCWStars(owner, type, origin);
			break;

		case FX_BUOY:
			Buoy(owner, flags, origin, (qboolean)vel[0]);
			break;

		case FX_BUOY_PATH:
			BuoyPath(origin, vel);
			break;

		case FX_M_MOBLUR:
			MMoBlur(owner, origin, vel, false);
			break;

		case FX_ASS_DAGGER:
			AssassinDagger(owner, vel, origin[0]);
			break;

		case FX_UNDER_WATER_WAKE:
			UnderwaterWake(owner);
			break;

		case FX_QUAKE_RING:
			QuakeRing(vel);
			break;

		case FX_GROUND_ATTACK:
			GroundAttack(vel);
			break;

		case FX_MORK_BEAM:
			MorkBeam2(owner, vel);
			break;

		case FX_MORK_MISSILE:
			MorkMissile(owner, vel);
			break;

		case FX_MORK_MISSILE_HIT:
			MorkMissileHit(origin);
			break;

		case FX_MORK_TRACKING_MISSILE:
			MorkTrackingMissile(owner, origin);
			break;

		case FX_MSSITHRA_EXPLODE:
			if (flags & CEF_FLAG6)
				MSsithraExplodeSmall(origin);
			else
				MSsithraExplode(origin, vel);
			break;

		case FX_MSSITHRA_ARROW:
			MSsithraArrow(owner, vel, (flags & CEF_FLAG6));
			break;

		case FX_MSSITHRA_ARROW_CHARGE:
			MSsithraArrowCharge(vel);
			break;

		default:
			assert(0); //mxd
			break;
	}
}