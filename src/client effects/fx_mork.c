//
// fx_mork.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "fx_mork.h" //mxd
#include "fx_flamethrow.h" //mxd
#include "fx_HighPriestessProjectiles.h" //mxd
#include "Particle.h"
#include "Random.h"
#include "Vector.h"
#include "Utilities.h"
#include "ce_DLight.h"
#include "q_Sprite.h"
#include "g_playstats.h"

//mxd. Mirrored in m_stats.h
enum
{
	// Offensive
	FX_M_BEAM,

	// Impacts
	FX_M_MISC_EXPLODE,

	// Other
	FX_IMP_FIRE,
	FX_IMP_FBEXPL,
	FX_CW_STARS, //TODO: unused?
	FX_BUOY,
	FX_BUOY_PATH,
	FX_M_MOBLUR, //TODO: unused?
	FX_ASS_DAGGER,
	FX_UNDER_WATER_WAKE,

	// jweier
	FX_QUAKE_RING,
	FX_GROUND_ATTACK,
	FX_MORK_BEAM, //TODO: unused?
	FX_MORK_MISSILE,
	FX_MORK_MISSILE_HIT,
	FX_MORK_TRACKING_MISSILE,

	FX_MSSITHRA_EXPLODE,
	FX_MSSITHRA_ARROW,
	FX_MSSITHRA_ARROW_CHARGE,
};

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

static qboolean FXMorkTrailThink_old(struct client_entity_s* self, centity_t* owner)
{
	if (self->alpha <= 0.1f || self->r.scale <= 0.0f)
		return false;

	self->r.scale -= 0.1f;
	return true;
}

static qboolean FXCWTrailThink(struct client_entity_s* self, const centity_t* owner)
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

static qboolean FXMorkTrailThink2(struct client_entity_s* self, centity_t* owner)
{
	if (self->alpha <= 0.1f || self->r.scale <= 0.0f)
		return false;

	self->r.scale -= 0.15f;
	return true;
}

//mxd. Added to reduce code duplication.
static void AddGenericExplosion(const centity_t* owner, vec3_t dir, struct model_s* model)
{
	Vec3ScaleAssign(32.0f, dir);

	const int count = GetScaledCount(irand(12, 16), 0.8f);

	for (int i = 0; i < count; i++)
	{
		const int next_think_time = (i == count - 1 ? 500 : 1000); //mxd
		client_entity_t* spark = ClientEntity_new(FX_M_EFFECTS, 0, owner->origin, NULL, next_think_time);

		spark->r.model = &model; // Blue spark sprite.
		spark->r.scale = flrand(0.5f, 1.0f);
		spark->d_scale = -2.0f;
		spark->r.flags = RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		spark->d_alpha = -0.4f;
		spark->radius = 20.0f;

		VectorRandomCopy(dir, spark->velocity, flrand(16.0f, 64.0f));

		spark->acceleration[0] = flrand(-400.0f, 400.0f);
		spark->acceleration[1] = flrand(-400.0f, 400.0f);
		spark->acceleration[2] = flrand(-60.0f, -40.0f);

		AddEffect(NULL, spark);
	}
}

static void FXMorkMissileExplode(const centity_t* owner, vec3_t dir)
{
	AddGenericExplosion(owner, dir, mork_projectile_models[1]); //mxd. Blue spark sprite.
}

static client_entity_t* MorkMakeLightningPiece(const vec3_t start, const vec3_t end, const float radius, const int lifetime)
{
#define M_LIGHTNING_WIDTH	6.0f
#define M_LIGHTNING_WIDTH2	8.0f

	vec3_t dir;
	VectorSubtract(end, start, dir);
	const float dist = VectorNormalize(dir);
	const float tile_num = dist / 32.0f;

	// Blue lightning.
	client_entity_t* lightning_b = ClientEntity_new(-1, CEF_DONT_LINK, start, NULL, lifetime);

	lightning_b->r.model = &mork_lightning_models[0]; // Blue lightning sprite.
	lightning_b->r.flags = RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	lightning_b->r.scale = M_LIGHTNING_WIDTH;
	lightning_b->r.tile = tile_num;
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
	lightning_r->r.flags = RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
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

static qboolean FXMorkBeamCircle(struct client_entity_s* self, const centity_t* owner)
{
	self->LifeTime += 54;

	vec3_t angles;
	VectorSet(angles, self->r.angles[PITCH], self->r.angles[YAW], anglemod((float)self->LifeTime));

	vec3_t up;
	AngleVectors(angles, NULL, NULL, up);
	VectorMA(owner->current.origin, 12.0f, up, self->r.origin);

	MorkMakeLightningPiece(self->startpos, self->r.origin, 2000.0f, 1000);
	VectorCopy(self->r.origin, self->startpos);

	return true;
}

static qboolean FXMorkBeamUpdate(struct client_entity_s* self, const centity_t* owner)
{
	static int particle_types[] =
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

	beam_inner->r.flags = RF_TRANSLUCENT | RF_TRANS_ADD_ALPHA;
	beam_inner->r.model = &mork_projectile_models[2];
	beam_inner->r.spriteType = SPRITE_LINE;
	beam_inner->r.tile = 1.0f;
	beam_inner->alpha = 2.0f;

	VectorCopy(self->startpos, beam_inner->r.startpos);
	VectorCopy(owner->origin, beam_inner->r.endpos);

	beam_inner->d_alpha = -1.0f;
	beam_inner->d_scale = -0.1f;
	beam_inner->Update = FXMorkTrailThink_old;

	AddEffect(NULL, beam_inner);

	// Make outer beam.
	client_entity_t* beam_outer = ClientEntity_new(FX_M_EFFECTS, CEF_DONT_LINK, owner->origin, NULL, 17);

	beam_outer->radius = 2000.0f;
	VectorCopy(owner->origin, beam_outer->origin);

	beam_outer->r.flags = RF_TRANSLUCENT | RF_TRANS_ADD_ALPHA;
	beam_outer->r.model = &mork_projectile_models[2];
	beam_outer->r.spriteType = SPRITE_LINE;
	beam_outer->r.tile = 1.0f;
	beam_outer->r.scale = 16.0f;
	beam_outer->r.scale = 4.0f;
	COLOUR_SET(beam_outer->r.color, 100, 75, 250); //mxd. Use macro.

	VectorCopy(self->startpos, beam_outer->r.startpos);
	VectorCopy(owner->origin, beam_outer->r.endpos);

	beam_outer->d_alpha = -0.6f;
	beam_outer->d_scale = -0.5f;
	beam_outer->Update = FXMorkTrailThink_old;

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

static void FXMorkBeam(centity_t* owner, const int type, const vec3_t origin, const vec3_t vel) //mxd. Separated from FXMEffects().
{
	const paletteRGBA_t light_color = { .r = 0, .g = 0, .b = 255, .a = 255 };

	client_entity_t* beam = ClientEntity_new(type, CEF_NO_DRAW | CEF_OWNERS_ORIGIN | CEF_DONT_LINK, origin, NULL, 20);

	beam->radius = 500.0f;
	beam->r.flags = RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	beam->flags |= CEF_NO_DRAW;
	VectorCopy(owner->current.origin, beam->r.origin);
	beam->dlight = CE_DLight_new(light_color, 150.0f, 0.0f);

	beam->AddToView = LinkedEntityUpdatePlacement;
	beam->Update = FXMorkBeamUpdate;

	VectorCopy(owner->origin, beam->startpos);

	AddEffect(owner, beam);
	FXMorkBeamUpdate(beam, owner);

	for (int i = 0; i < 3; i++)
	{
		client_entity_t* trail = ClientEntity_new(type, 0, origin, NULL, 20);

		trail->radius = 500.0f;
		trail->r.model = &mork_projectile_models[3]; //TODO: segment_trail_wt is invisible!
		trail->r.flags = RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		trail->r.scale = 0.5f;
		trail->LifeTime = i * 120;
		trail->Update = FXMorkBeamCircle;

		VectorCopy(owner->current.origin, trail->r.origin);
		VectorCopy(owner->origin, trail->startpos);
		VectorCopy(vel, trail->r.angles);

		AddEffect(owner, trail);
		FXMorkBeamCircle(trail, owner);
	}
}

static void FXImpFireBallExplode(const centity_t* owner, vec3_t dir)
{
	AddGenericExplosion(owner, dir, imp_models[1]); //mxd. Fire spark sprite.
}

static qboolean FXImpFireballUpdate(struct client_entity_s* self, const centity_t* owner)
{
	vec3_t angles;
	VectorScale(self->r.angles, RAD_TO_ANGLE, angles);

	vec3_t fwd;
	vec3_t right;
	AngleVectors(angles, fwd, right, NULL);

	const paletteRGBA_t light_color = { .c = 0xe5007fff };
	const int num_parts = irand(3, 7);

	for (int i = 0; i < num_parts; i++)
	{
		client_particle_t* p = ClientParticle_new(irand(PART_32x32_FIRE0, PART_32x32_FIRE2), light_color, 1000);

		VectorRandomSet(p->origin, 4.0f);
		VectorAdd(self->r.origin, p->origin, p->origin);
		p->scale = flrand(0.1f, 0.5f);
		p->type |= PFL_ADDITIVE;

		VectorSet(p->velocity, flrand(-10.0f, 10.0f), flrand(-10.0f, 10.0f), flrand(-1.0f, 1.0f));

		// Make the fire shoot out the back and to the side.
		VectorMA(p->velocity, flrand(-40.0f, -10.0f), fwd, p->velocity);

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

	trail->r.flags = RF_TRANSLUCENT | RF_TRANS_ADD_ALPHA | RF_TRANS_ADD;
	trail->r.model = &mork_projectile_models[2];
	COLOUR_SET(trail->r.color, 180, 60, 0); //mxd. Use macro.
	trail->r.spriteType = SPRITE_LINE;
	trail->r.tile = 1.0f;
	trail->r.scale = 3.0f;

	VectorCopy(self->startpos, trail->r.startpos);
	VectorCopy(owner->origin, trail->r.endpos);

	trail->d_alpha = -4.0f;
	trail->Update = FXMorkTrailThink2;

	AddEffect(NULL, trail);
	VectorCopy(owner->origin, self->startpos);

	return true;
}

static void FXImpFireball(centity_t* owner, const vec3_t origin, const vec3_t vel) //mxd. Separated from FXMEffects().
{
	const int flags = CEF_OWNERS_ORIGIN | CEF_DONT_LINK | CEF_ADDITIVE_PARTS | CEF_ABSOLUTE_PARTS;
	client_entity_t* fx = ClientEntity_new(FX_SPARKS, flags, origin, NULL, 20);

	fx->radius = 64.0f;
	fx->r.model = &imp_models[0];
	fx->r.flags = RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	fx->r.frame = 2;
	fx->r.scale = 0.5f;
	fx->r.color.c = 0xe5007fff;

	vectoangles(vel, fx->r.angles);
	VectorCopy(owner->origin, fx->startpos);

	fx->Update = FXImpFireballUpdate;
	fx->AddToView = LinkedEntityUpdatePlacement;

	if (r_detail->value > DETAIL_NORMAL)
	{
		const paletteRGBA_t light_color = { .c = 0xff3333ff };
		fx->dlight = CE_DLight_new(light_color, 150.0f, 0.0f);
	}

	AddEffect(owner, fx);
}

static qboolean FXCWUpdate(struct client_entity_s* self, centity_t* owner)
{
	static int star_particles[3] = { PART_16x16_STAR, PART_16x16_SPARK_C, PART_16x16_SPARK_B }; //mxd. Made local static.

	client_entity_t* spawner = ClientEntity_new(FX_M_EFFECTS, CEF_NO_DRAW | CEF_ABSOLUTE_PARTS, self->r.origin, NULL, 500);
	AddEffect(NULL, spawner);

	vec3_t angles;
	VectorScale(self->r.angles, RAD_TO_ANGLE, angles);

	vec3_t fwd;
	vec3_t right;
	AngleVectors(angles, fwd, right, NULL);

	const int num_particles = irand(3, 7);

	for (int i = 0; i < num_particles; i++)
	{
		client_particle_t* p = ClientParticle_new(star_particles[irand(0, 2)], color_white, 2000);

		VectorRandomSet(p->origin, 4.0f); //mxd
		VectorAdd(self->r.origin, p->origin, p->origin);
		p->scale = flrand(2.5f, 3.0f);

		VectorSet(p->velocity, flrand(-10.0f, 10.0f), flrand(-10.0f, 10.0f), flrand(-1.0f, 1.0f));
		VectorMA(p->velocity, flrand(-40.0f, -10.0f), fwd, p->velocity);

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
	trail_head->r.flags = RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	trail_head->r.model = &cw_model; // Patball sprite.
	trail_head->r.spriteType = SPRITE_LINE;
	trail_head->r.tile = 1.0f;
	trail_head->r.scale = 3.0f;

	VectorCopy(self->startpos, trail_head->r.startpos);
	VectorCopy(owner->current.origin, trail_head->r.endpos);

	trail_head->d_alpha = -2.0f;
	trail_head->Update = FXMorkTrailThink2;

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

	trail->r.flags = RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	trail->r.color.c = 0xffffaacc;
	trail->alpha = flrand(0.75f, 1.0f);
	trail->d_alpha = -1.0f;
	trail->d_scale = -0.1f; // Outer part does not scale down.

	// Angle.
	const vec3_t dir = { flrand(0.0f, 359.0f), flrand(0.0f, 359.0f), flrand(0.0f, 359.0f) };
	VectorCopy(dir, trail->direction);
	AngleVectors(dir, fwd, NULL, NULL);

	// Length.
	trail->SpawnInfo = irand(20, 70);
	VectorCopy(owner->current.origin, trail->r.startpos);
	VectorMA(owner->current.origin, (float)trail->SpawnInfo, fwd, trail->r.endpos);

	// Angular velocity.
	VectorRandomSet(trail->up, 10.0f);

	// Speed.
	VectorCopy(self->direction, trail->velocity);
	VectorCopy(self->direction, trail->velocity2);

	trail->Update = FXCWTrailThink;

	AddEffect(owner, trail);

	// Update self.
	self->r.scale = flrand(0.65f, 0.95f);
	VectorCopy(owner->current.origin, self->startpos);

	return true;
}

void FXCWStars (centity_t *owner,int type, vec3_t vel)
{
	client_entity_t	*fx;

	fx = ClientEntity_new( type, CEF_OWNERS_ORIGIN | CEF_DONT_LINK, owner->origin, NULL, 20);

	fx->Update=FXCWUpdate;
	fx->radius = 500;
	fx->r.model = &mork_projectile_core_model;
	VectorCopy(vel, fx->direction);
	fx->r.color.r = 10;
	fx->r.color.g = 50;
	fx->r.color.b = 255;
	fx->r.flags |= RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	fx->r.scale = 0.8;
	fx->AddToView = LinkedEntityUpdatePlacement;

	VectorCopy(owner->origin, fx->startpos);

	AddEffect(owner,fx);
}

#define BUOY_FX_END			PART_4x4_RED
#define BUOY_FX_START		PART_4x4_GREEN
#define BUOY_FX_JUMP_FROM	PART_4x4_CYAN
#define BUOY_FX_JUMP_TO		PART_4x4_BLUE
#define BUOY_FX_ACTIVATE	PART_4x4_MAGENTA
#define BUOY_FX_ONEWAY		PART_4x4_WHITE

int FXBuoyUpdate (struct client_entity_s *self, centity_t *owner)
{
	client_particle_t	*p;
	int					num_parts, i;
	paletteRGBA_t		LightColor={255,255,255,255};
	vec3_t				offset, forward, angles;
	int					type = (int)(self->acceleration2[2]);

	if(type == BUOY_FX_START || type == BUOY_FX_END)
	{//these effects time out
		if(self->LifeTime < fxi.cl->time)
			return (false);
	}

	if(owner)
	{
		if(!owner->current.frame)
			return (false);

		if(owner->current.frame > 5)
			num_parts = 5;
		else
			num_parts = owner->current.frame;
	}
	else
		num_parts = irand(1, 3);

	for(i = 0; i < num_parts; i++)
	{
		p = ClientParticle_new(type, LightColor, 1000);

		switch(type)
		{
		case BUOY_FX_END://red
			if(irand(0,1))
				offset[0] = flrand(4, 12);
			else
				offset[0] = flrand(-12, -4);
			if(irand(0,1))
				offset[1] = flrand(4, 12);
			else
				offset[1] = flrand(-12, -4);
			offset[2] = 0;
			VectorSet(p->origin, offset[0], offset[1], 0);
			VectorSet(p->velocity, offset[0], offset[1], 0);
			p->acceleration[2] = 0;
			break;

		case BUOY_FX_START://green
			VectorSet(p->origin, flrand(-2, 2), flrand(-2, 2), flrand(8, 16));
			VectorSet(p->velocity, 0, 0, flrand(3.0, 7.0));
			p->acceleration[2] = flrand(0.05, 2);
			break;

		case BUOY_FX_JUMP_FROM://cyan
			if(irand(0,1))
				offset[0] = flrand(4, 12);
			else
				offset[0] = flrand(-12, -4);
			if(irand(0,1))
				offset[1] = flrand(4, 12);
			else
				offset[1] = flrand(-12, -4);
			offset[2] = 0;
			VectorSet(p->origin, offset[0], offset[1], 0);
			VectorSet(p->velocity, offset[0], offset[1], 1);
			p->acceleration[2] = 2;
			break;

		case BUOY_FX_JUMP_TO://blue
			if(irand(0, 1))
				offset[0] = 8;
			else
				offset[0] = -8;
			if(irand(0, 1))
				offset[1] = 8;
			else
				offset[1] = -8;
			offset[2] = -2;
			
			VectorSet(p->origin, offset[0], offset[1], offset[2]);
			VectorSet(p->velocity, offset[0], offset[1], offset[2]);
			p->acceleration[2] = -2;
			break;

		case BUOY_FX_ACTIVATE://magenta
			VectorSet(angles, 0, self->yaw++, 0);
			AngleVectors(angles, forward, NULL, NULL);
			
			VectorScale(forward, 8, p->origin);
			p->origin[2] = 8;
			VectorCopy(p->origin, p->velocity);
			p->acceleration[2] = 0;
			break;

		case BUOY_FX_ONEWAY://white
			VectorSet(p->origin, 0, 0, flrand(8, 16));
			VectorSet(p->velocity, 0, 0, 7);
			p->acceleration[2] = flrand(0.05, 2);
			break;

		default:
			assert(0);
			break;
		}
		
		p->scale = flrand(0.5, 1.0);
		p->d_alpha = flrand(-200.0, -160.0);
		p->duration = (255.0 * 1000.0) / -p->d_alpha;		// time taken to reach zero alpha

		AddParticleToList(self, p);
	}
	
	return true;
}

void FXBuoy (centity_t *owner, int flags, vec3_t org, float white)
{
	client_entity_t	*fx;

	if(owner)
		fx = ClientEntity_new(FX_BUOY, CEF_OWNERS_ORIGIN, owner->current.origin, NULL, 50);
	else
		fx = ClientEntity_new(FX_BUOY, 0, org, NULL, 50);

	if(white)
		fx->acceleration2[2] = BUOY_FX_ONEWAY;//white
	else if(flags&CEF_FLAG6)
		fx->acceleration2[2] = BUOY_FX_START;//green
	else if(flags&CEF_FLAG7)
		fx->acceleration2[2] = BUOY_FX_JUMP_FROM;//cyan
	else if(flags&CEF_FLAG8)
		fx->acceleration2[2] = BUOY_FX_JUMP_TO;//blue - maybe 3 - yellow?
	else if(flags&CEF_DONT_LINK)
		fx->acceleration2[2] = BUOY_FX_ACTIVATE;//magenta
	else
		fx->acceleration2[2] = BUOY_FX_END;//red
//otherwise red
	fx->flags |= CEF_NO_DRAW;
	fx->Update=FXBuoyUpdate;
	fx->LifeTime = fxi.cl->time + 10000;

	if(owner)
	{
		AddEffect(owner, fx);
	}
	else
	{
		VectorCopy(org, fx->startpos);
		AddEffect(NULL, fx);
	}
}

qboolean FXPermanentUpdate (struct client_entity_s *self, centity_t *owner)
{
	self->updateTime = 16384;
	return true;
}

qboolean FXRemoveUpdate(struct client_entity_s *self,centity_t *owner) //TODO: remove
{
	return false;
}

qboolean FXBuoyPathDelayedStart (struct client_entity_s *self, centity_t *owner)
{
	client_entity_t	*TrailEnt;
	vec3_t	v;
	float dist;

	TrailEnt=ClientEntity_new(FX_BUOY,
							  CEF_DONT_LINK,
							  self->origin,
							  NULL,
							  16384);
	
	TrailEnt->Update = FXPermanentUpdate;
	TrailEnt->updateTime = 16384;
	TrailEnt->radius = 500;

	TrailEnt->r.flags |= RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	TrailEnt->r.model = &buoy_model;

	TrailEnt->r.spriteType = SPRITE_LINE;
	TrailEnt->alpha = 1.0;
	TrailEnt->r.scale = 7.0;

	VectorSubtract(self->startpos, self->endpos, v);
	dist = VectorLength(v);
	if(VectorLength(v)<64)
		TrailEnt->r.tile = 1;
	else
		TrailEnt->r.tile = 3;

	VectorCopy(self->startpos, TrailEnt->r.startpos);
	VectorCopy(self->endpos, TrailEnt->r.endpos);

	AddEffect(NULL, TrailEnt);

	self->Update = FXRemoveUpdate;
	self->updateTime = 100;

	return true;
}

void FXBuoyPath (vec3_t org, vec3_t vel)
{
	client_entity_t	*fx;
	vec3_t			origin;

	VectorAdd(org, vel, origin);
	Vec3ScaleAssign(0.5, origin);

	fx = ClientEntity_new(FX_BUOY, CEF_DONT_LINK | CEF_NO_DRAW, origin, NULL, 100);
	
	fx->flags |= CEF_NO_DRAW;
	fx->Update=FXBuoyPathDelayedStart;
	fx->radius = 100;

	VectorCopy(org, fx->startpos);
	VectorCopy(vel, fx->endpos);

	AddEffect(NULL,fx);
}


qboolean FXMMoBlurUpdate(struct client_entity_s *self, centity_t *owner)
{
	if (self->alpha <= 0.05f)
		return false;

	return true;
}

void FXMMoBlur(centity_t *owner, vec3_t org, vec3_t angles, qboolean dagger)
{//r_detail 2 only?
	client_entity_t	*blur;

	if(dagger)
	{
		blur = ClientEntity_new(FX_M_EFFECTS, 0, org, NULL, 20);//CEF_DONT_LINK
		VectorCopy(angles, blur->r.angles);
		blur->r.model = &assassin_dagger_model;
		blur->alpha = 0.75;
		blur->r.scale = 0.9;

		blur->d_alpha = -3.0;
		blur->d_scale = -0.3;
	}
	else
	{
		blur = ClientEntity_new(FX_M_EFFECTS, 0, owner->current.origin, NULL, 20);//CEF_DONT_LINK
		VectorSet(blur->r.angles,
			angles[PITCH] * -1 * ANGLE_TO_RAD,
			angles[YAW] * ANGLE_TO_RAD,
			angles[ROLL] * ANGLE_TO_RAD);
		blur->r.model = &mork_model;
		blur->r.frame = owner->current.frame;
		blur->d_alpha = -1.0;
		blur->d_scale = -0.1;
		blur->alpha = 1.0;
		blur->r.scale = 1.0;
	}
	blur->r.flags = RF_TRANSLUCENT | RF_TRANS_ADD_ALPHA | RF_GLOW;
	blur->r.scale = 1.0;
	blur->Update = FXMMoBlurUpdate;
	blur->updateTime = 20;
	AddEffect(NULL,blur);
}

qboolean FXAssDaggerUpdate (struct client_entity_s *self, centity_t *owner)
{
	if(++self->LifeTime == 4)
	{
		fxi.S_StartSound(self->r.origin, -1, CHAN_AUTO, fxi.S_RegisterSound(va("monsters/assassin/throw%c.wav", irand('1', '2'))), 0.5, ATTN_IDLE, 0);
		self->LifeTime = 0;
	}
	
	FXMMoBlur(NULL, self->r.origin, self->r.angles, true);
	self->r.angles[PITCH] += self->velocity2[0];
	return true;
}

void FXAssDagger(centity_t *owner, vec3_t vel, float avel)
{
	client_entity_t	*dagger;

	dagger = ClientEntity_new(FX_M_EFFECTS, CEF_DONT_LINK, owner->current.origin, NULL, 20);

//	vectoangles(vel, dagger->r.angles);
//	VectorScale(dagger->r.angles, ANGLE_TO_RAD, dagger->r.angles);
	VectorScale(owner->current.angles, ANGLE_TO_RAD, dagger->r.angles);
	dagger->r.model = &assassin_dagger_model;
	dagger->r.flags |= RF_FULLBRIGHT;
	dagger->Update = FXAssDaggerUpdate;
	VectorCopy(vel, dagger->velocity);
	dagger->velocity2[0] = (avel*ANGLE_TO_RAD);

	AddEffect(owner, dagger);
}


int water_particle [6] =
{
	PART_4x4_WHITE,
	PART_8x8_BUBBLE,
	PART_16x16_WATERDROP,
	PART_32x32_WFALL,
	PART_32x32_STEAM,
	PART_32x32_BUBBLE
};

qboolean FXUnderWaterWakeUpdate (struct client_entity_s *self, centity_t *owner)
{
	client_particle_t	*p;
	vec3_t				right;
	int					num_parts, i;
	paletteRGBA_t		LightColor={200, 255, 255, 140};//RGBA

	VectorCopy(owner->lerp_origin, self->r.origin);
	AngleVectors(owner->lerp_angles, NULL, right, NULL);

	num_parts = irand(3, 7);
	for(i = 0; i < num_parts; i++)
	{
		if(r_detail->value > DETAIL_LOW)
			p = ClientParticle_new(water_particle[irand(0, 5)], LightColor, irand(1000, 1500));
		else
			p = ClientParticle_new(water_particle[irand(0, 5)]|PFL_SOFT_MASK, LightColor, irand(1000, 1500));

		VectorSet(p->origin, flrand(-8, 8), flrand(-8, 8), flrand(-4, 4));
		VectorAdd(self->r.origin, p->origin, p->origin);
		
		p->scale = flrand(0.75, 1.5);
		p->color.a = irand(100, 200);

		VectorSet(p->velocity, flrand(-2, 2), flrand(-2, 2), flrand(-2.0, 2.0));

		if (irand(0, 1))
			VectorMA(p->velocity, flrand(-10, -2), right, p->velocity);
		else
			VectorMA(p->velocity, flrand(10, 2), right, p->velocity);

		p->acceleration[2] = 2;
		p->d_alpha = flrand(-300, -200);
		p->d_scale = flrand(-0.15, -0.10);

		AddParticleToList(self, p);
	}

	self->LifeTime--;
	if(self->LifeTime<=0)
		return (false);
	
	return (true);
}

void FXUnderWaterWake (centity_t *owner)
{
	client_entity_t	*fx;

	fx = ClientEntity_new(FX_M_EFFECTS, CEF_OWNERS_ORIGIN|CEF_NO_DRAW|CEF_ABSOLUTE_PARTS, owner->current.origin, NULL, 20);

	fx->Update=FXUnderWaterWakeUpdate;
	fx->radius = 30;
	fx->LifeTime = 77;

	AddEffect(owner, fx);
}

#define NUM_RIPPER_PUFFS	12
#define RIPPER_PUFF_ANGLE	((360.0*ANGLE_TO_RAD)/(float)NUM_RIPPER_PUFFS)
#define MACEBALL_RING_VEL	256.0
#define NUM_RINGS			3

void FXQuakeRing ( vec3_t origin )
{
	client_entity_t		*ring;
	paletteRGBA_t		color;
	int					i, j;
	vec3_t				norm = {0,0,1};
	vec3_t				up, right, lastvel;
	float				curyaw;
	float				ring_vel = MACEBALL_RING_VEL;

	color.c = 0xffffffff;

	// Take the normal and find two "axis" vectors that are in the plane the normal defines
	PerpendicularVector(up, norm);
	CrossProduct(up, norm, right);

	VectorScale(norm, 8.0, norm);
	color.c = 0xffffffff;

	// Draw a circle of expanding lines.
	for(j = 0; j < NUM_RINGS; j++)
	{
		curyaw = 0;
		VectorScale(right, ring_vel, lastvel);

		for(i = 0; i < NUM_RIPPER_PUFFS; i++)
		{
			curyaw += RIPPER_PUFF_ANGLE;

			ring = ClientEntity_new(FX_M_EFFECTS, CEF_USE_VELOCITY2 | CEF_AUTO_ORIGIN | CEF_ABSOLUTE_PARTS | CEF_ADDITIVE_PARTS, 
										origin, NULL, 3000);

			ring->r.model = morc_models;
			ring->r.frame = 0;
			ring->r.spriteType = SPRITE_LINE;
			ring->r.frame = 1;
			ring->r.flags |= RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
			ring->radius = 256.0;
			ring->r.tile = 1;
			
			// The startpos and startvel comes from the last velocity.
			VectorCopy(lastvel, ring->velocity);
			VectorScale(ring->velocity, 1.0, ring->acceleration);
			VectorMA(origin, .01, ring->velocity, ring->r.startpos);	// Move the line out a bit to avoid a zero-length line.

			VectorScale(up, ring_vel*sin(curyaw), ring->velocity2);
			VectorMA(ring->velocity2, ring_vel*cos(curyaw), right, ring->velocity2);

			VectorScale(ring->velocity2, 1.0, ring->acceleration2);
			VectorMA(origin, .01, ring->velocity2, ring->r.endpos);	// Move the line out a bit to avoid a zero-length line.

			// Finally, copy the last velocity we used.
			VectorCopy(ring->velocity2, lastvel);

			// NOW apply the extra directional velocity to force it slightly away from the surface.
			VectorAdd(ring->velocity, norm, ring->velocity);
			VectorAdd(ring->velocity2, norm, ring->velocity2);

			ring->r.scale = 8.0;
			ring->d_scale = 32.0;
			ring->alpha = 0.75;
			ring->d_alpha = -1.0;

			AddEffect(NULL, ring);
		}

		ring_vel /= 2;
	}

	fxi.Activate_Screen_Shake(12, 1000, fxi.cl->time, SHAKE_ALL_DIR);
	fxi.S_StartSound(origin, -1, CHAN_AUTO, fxi.S_RegisterSound("world/quakeshort.wav"), 1, ATTN_NONE, 0);
}

void FXGroundAttack( vec3_t origin )
{
	client_entity_t	*glow;
	vec3_t			dir = {0,0,1};

	origin[2] -= 16;
	
	// create the dummy entity, so particles can be attached
	glow = ClientEntity_new(FX_M_EFFECTS, CEF_NO_DRAW | CEF_ADDITIVE_PARTS, origin, 0, 17);
	
	VectorScale(dir, 50, glow->direction);
	
	glow->radius = 100;
	glow->LifeTime = fxi.cl->time + 1000;
	
	fxi.S_StartSound(origin, -1, CHAN_AUTO, fxi.S_RegisterSound("misc/flamethrow.wav"), 1, ATTN_NORM, 0);
	glow->Update = FXFlamethrower_trail;
	
	AddEffect(NULL, glow);
}

static qboolean beam_update(struct client_entity_s *self, centity_t *owner) //TODO: remove
{
	return true;
}

static qboolean beam_add_to_view(struct client_entity_s *self, centity_t *owner)
{
	LinkedEntityUpdatePlacement(self, owner);
	VectorCopy(self->r.origin, self->r.endpos);

	return true;
}

void FXMorkBeam2 ( centity_t *owner, vec3_t	startpos )
{
	client_entity_t	*fx;
	paletteRGBA_t	LightColor={128,128,255,255};
	//vec3_t			vel;

	fx = ClientEntity_new( FX_M_EFFECTS, CEF_OWNERS_ORIGIN | CEF_DONT_LINK, owner->origin, NULL, 17);

	fx->r.spriteType = SPRITE_LINE;
	
	fx->radius = 1024;
	fx->r.model = mork_projectile_models + 2;
	fx->r.scale = 8;
	fx->alpha = 1.0;
	fx->r.color.c = 0xFFFFFFFF;

	VectorCopy(startpos, fx->r.startpos);
	VectorCopy(owner->origin, fx->r.endpos);

	fx->Update = beam_update;
	fx->AddToView = beam_add_to_view;

	AddEffect(owner, fx);
}

static qboolean missile_add_to_view(struct client_entity_s *self, centity_t *owner)
{
	LinkedEntityUpdatePlacement(self, owner);
	VectorCopy(self->r.origin, self->r.startpos);

	self->direction[0] += flrand(-1.0, 1.0);
	self->direction[1] += flrand(-1.0, 1.0);
	self->direction[2] += flrand(-1.0, 1.0);

	VectorNormalize(self->direction);
	VectorMA(self->r.startpos, irand(self->LifeTime/4, self->LifeTime), self->direction, self->r.endpos);

	self->r.scale = flrand(1.0, 2.0);

	return true;
}

static qboolean MorkMissileThink1(struct client_entity_s *self, centity_t *owner)
{
	if (self->LifeTime < 24)
	{
		self->LifeTime += 1;
	}

	return true;
}

static qboolean MorkMissileThink2(struct client_entity_s *self, centity_t *owner)
{
	if (self->alpha < 0.25)
	{
		self->alpha += 0.1;
	}

	if (self->r.scale < 3.0)
	{
		self->r.scale += 0.1;
	}

	if (self->dlight->intensity <= 200.0f)
	{
		self->dlight->intensity += 10.0f;
	}

	return true;
}

static qboolean MorkMissileThink3(struct client_entity_s *self, centity_t *owner)
{
	if (self->alpha < 0.5)
	{
		self->alpha += 0.1;
	}

	if (self->r.scale < 1.0)
	{
		self->r.scale += 0.1;
	}

	if (self->SpawnInfo > irand(15, 20))
	{
		fxi.S_StartSound(self->r.origin, -1, CHAN_AUTO, fxi.S_RegisterSound("monsters/elflord/weld.wav"), 0.5, ATTN_IDLE, 0);
		self->SpawnInfo=0;
	}
	
	self->SpawnInfo++;
	return true;
}

void FXMorkMissile ( centity_t *owner, vec3_t startpos )
{
	client_entity_t	*fx;
	paletteRGBA_t	LightColor={128,128,255,255};
	int				i;
	
	i = GetScaledCount(8, 0.85);

	while (i--)
	{
		fx = ClientEntity_new( FX_M_EFFECTS, CEF_OWNERS_ORIGIN, startpos, NULL, 17);
	
		fx->r.spriteType = SPRITE_LINE;
		fx->r.flags |= RF_TRANS_ADD | RF_TRANSLUCENT | RF_FULLBRIGHT;
		
		fx->radius = 1024;
		fx->r.model = morc_models + 1;
		fx->r.scale = irand(0.1, 1);
		fx->r.scale2 = 0.1;
		fx->alpha = 1.0;
		fx->r.color.c = 0xFFFFFFFF;

		VectorCopy(startpos, fx->r.startpos);
		
		fx->direction[0] = flrand(-1.0, 1.0);
		fx->direction[1] = flrand(-1.0, 1.0);
		fx->direction[2] = flrand(-1.0, 1.0);

		VectorMA(startpos, irand(4, 16), fx->direction, fx->r.endpos);

		fx->Update = MorkMissileThink1;
		fx->AddToView = missile_add_to_view;

		AddEffect(owner, fx);
	}

	//Light blue halo
	fx = ClientEntity_new( FX_M_EFFECTS, CEF_OWNERS_ORIGIN | CEF_DONT_LINK, startpos, NULL, 100);
	fx->dlight = CE_DLight_new(LightColor,10.0f,0.0f);
	fx->r.model = morc_models + 2;
	fx->r.flags |= RF_TRANS_ADD | RF_TRANSLUCENT | RF_FULLBRIGHT;
	fx->alpha = 0.1;
	fx->r.scale = 0.1;

	fx->Update = MorkMissileThink2;
	fx->AddToView = LinkedEntityUpdatePlacement;
	
	AddEffect(owner, fx);

	//The white core
	fx = ClientEntity_new( FX_M_EFFECTS, CEF_OWNERS_ORIGIN | CEF_DONT_LINK, startpos, NULL, 100);
	fx->r.model = morc_models + 3;
	fx->r.flags |= RF_TRANS_ADD | RF_TRANSLUCENT | RF_FULLBRIGHT;
	fx->alpha = 0.1;
	fx->r.scale = 0.1;

	fx->Update = MorkMissileThink3;
	fx->AddToView = LinkedEntityUpdatePlacement;
	
	AddEffect(owner, fx);

}

void FXMorkMissileHit ( vec3_t origin, vec3_t dir )
{
	client_entity_t	*fx;
	paletteRGBA_t	LightColor={128,128,255,255};
	
	//The white core
	fx = ClientEntity_new( FX_M_EFFECTS, CEF_OWNERS_ORIGIN | CEF_DONT_LINK, origin, NULL, 2000);
	fx->r.model = morc_models + 3;
	fx->r.flags |= RF_TRANS_ADD | RF_TRANS_ADD_ALPHA | RF_TRANSLUCENT | RF_FULLBRIGHT | RF_NODEPTHTEST;
	fx->r.scale = 1;
	fx->alpha = 0.5;
	fx->d_alpha = -1.0;
	fx->d_scale = 16.0;

	AddEffect(NULL, fx);

	fxi.S_StartSound(origin, -1, CHAN_AUTO, fxi.S_RegisterSound("monsters/mork/ppexplode.wav"), 1.0, ATTN_IDLE, 0);
}

static qboolean FXMTrailThink(struct client_entity_s *self,centity_t *Owner)
{
	if (self->alpha <= 0.1 || self->r.scale <= 0.0)
		return false;

	self->r.scale -= 0.1;
	self->r.scale2 -= 0.1;
	
	return true;
}

static qboolean FXMMissileTrailThink(struct client_entity_s *self,centity_t *Owner)
{
	client_entity_t	*TrailEnt;

	self->r.scale = flrand(0.35, 0.65);

	TrailEnt=ClientEntity_new(FX_M_EFFECTS,
							  CEF_DONT_LINK,
							  Owner->lerp_origin,
							  NULL,
							  17);

	TrailEnt->radius = 500;

	VectorCopy( Owner->lerp_origin, TrailEnt->r.origin );

	TrailEnt->r.flags |= RF_TRANSLUCENT | RF_TRANS_ADD_ALPHA;
	TrailEnt->r.model = morc_models + 5;

 	TrailEnt->r.spriteType = SPRITE_LINE;
	TrailEnt->r.tile = 1;
	TrailEnt->r.scale = 2.0;
	TrailEnt->r.scale2 = 2.0;
	TrailEnt->alpha = 0.5;

	VectorCopy( self->startpos, TrailEnt->r.startpos );
	VectorCopy( Owner->lerp_origin, TrailEnt->r.endpos );

	TrailEnt->d_alpha = -2.5;
	TrailEnt->d_scale = 0.0;
	TrailEnt->Update = FXMTrailThink;
	
	AddEffect(NULL,TrailEnt);

	VectorCopy(Owner->lerp_origin, self->startpos);
	
	return true;
}

void FXMorkTrackingMissile ( centity_t *owner, vec3_t origin, vec3_t velocity )
{
	client_entity_t	*Trail;
	paletteRGBA_t	LightColor={0,0,255,255};

	FXHPMissileCreateWarp(FX_M_EFFECTS, origin);

	Trail = ClientEntity_new( FX_M_EFFECTS, CEF_OWNERS_ORIGIN | CEF_DONT_LINK, origin, NULL, 20);

	Trail->Update=FXMMissileTrailThink;
	Trail->dlight=CE_DLight_new(LightColor,150.0f,0.0f);
	Trail->radius = 500;
	Trail->r.model = morc_models + 4;
	Trail->r.color.c = 0xFFFFFFFF;
	Trail->r.flags |= RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	Trail->r.scale = 1.0;
	Trail->AddToView = LinkedEntityUpdatePlacement;

	VectorCopy(origin, Trail->startpos);

	AddEffect(owner,Trail);

	FXMMissileTrailThink(Trail,owner);
}

qboolean rubble_spin (client_entity_t *self, centity_t *owner)
{
	if (self->LifeTime < fxi.cl->time)
		return false;
	
	self->r.angles[YAW] += 0.1;
	self->r.angles[PITCH] += 0.1;
	self->r.angles[ROLL] += 0.1;

	return true;
}

qboolean mssithra_explosion_think (client_entity_t *self, centity_t *owner)
{
	client_entity_t	*explosion, *TrailEnt;
	paletteRGBA_t	color = {255,255,255,255};
	vec3_t			dir;
	int				i;	
	int				white;

	if (self->LifeTime < fxi.cl->time)
		return false;

	//Spawn a white core
	explosion = ClientEntity_new( FX_M_EFFECTS, 0, self->origin, NULL, 1000);
	explosion->r.model = mssithra_models + 5;
	
	explosion->r.flags |= RF_FULLBRIGHT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	explosion->r.scale = 0.1;
	explosion->radius = 500;
	explosion->r.color.c = 0xFFFFFFFF;
	explosion->alpha = 0.75;
	explosion->d_scale = 4.0;
	explosion->d_alpha = -2.5;
	
	explosion->r.origin[0] += irand(-8,8);
	explosion->r.origin[1] += irand(-8,8);
	explosion->r.origin[2] += irand(-8,8);
	
	AddEffect(NULL, explosion);

	i = GetScaledCount(3, 0.85);

	//Spawn a small explosion sphere
	while (i--)
	{
		explosion = ClientEntity_new( FX_M_EFFECTS, 0, self->origin, NULL, 1000);
		explosion->r.model = mssithra_models + irand(0, 1);
		
		explosion->r.flags |= RF_FULLBRIGHT;
		explosion->r.scale = 0.1;
		explosion->radius = 500;
		explosion->r.color.c = 0xFFFFFFFF;
		explosion->alpha = 0.75;
		explosion->d_scale = 2.0;
		explosion->d_alpha = -2.5;
		
		explosion->r.origin[0] += irand(-16,16);
		explosion->r.origin[1] += irand(-16,16);
		explosion->r.origin[2] += irand(-16,16);

		AddEffect(NULL, explosion);
	}

	VectorCopy(self->direction, dir);

	if (irand(0,1))
	{
		if (r_detail->value > 1)
		{
			//Spawn an explosion of lines
			i = GetScaledCount(2, 0.85);

			while (i--)
			{
				TrailEnt=ClientEntity_new(FX_M_EFFECTS, 0, self->r.origin, 0, 17);

				TrailEnt->r.model = mssithra_models + irand(3,4);
				
				TrailEnt->r.flags |= RF_FULLBRIGHT;
				TrailEnt->r.scale = flrand(0.5, 1.5);
				TrailEnt->alpha = 1.0;

				VectorRandomCopy(dir, TrailEnt->velocity, 1.25);
				
				VectorScale(TrailEnt->velocity, irand(50,100), TrailEnt->velocity);
				TrailEnt->acceleration[2] -= 256;

				TrailEnt->Update = rubble_spin;
				TrailEnt->LifeTime = fxi.cl->time + 2000;

				AddEffect(NULL, TrailEnt);
			}
		}
	}
	else
	{
		//Spawn an explosion of lines
		i = GetScaledCount(2, 0.85);

		while (i--)
		{
			TrailEnt=ClientEntity_new(FX_M_EFFECTS, 0, self->r.origin, 0, 500);

			TrailEnt->r.model = mssithra_models + 2;
			
			TrailEnt->r.spriteType = SPRITE_LINE;

			TrailEnt->r.flags |= RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
			TrailEnt->r.color.c = 0xFFFFFFFF;
			TrailEnt->r.scale = flrand(1.0, 2.5);
			TrailEnt->alpha = 1.0;
			TrailEnt->d_alpha = -1.0;
			TrailEnt->d_scale = -1.0;

			white = irand(128, 255);

			TrailEnt->r.color.r = white;
			TrailEnt->r.color.g = white;
			TrailEnt->r.color.b = 128 + irand(108, 127);
			TrailEnt->r.color.a = 64 + irand(16, 128);

			VectorRandomCopy(dir, TrailEnt->velocity, 1.25);
			
			VectorCopy(self->r.origin, TrailEnt->r.endpos);
			VectorMA(TrailEnt->r.endpos, irand(16, 32), TrailEnt->velocity, TrailEnt->r.startpos);

			VectorScale(TrailEnt->velocity, irand(50,150), TrailEnt->velocity);

			AddEffect(NULL, TrailEnt);
		}
	}

	return true;
}

void FXMSsithraExplode( vec3_t origin, vec3_t dir )
{
	client_entity_t	*spawner;

	//Create an explosion spawner
	spawner = ClientEntity_new( FX_M_EFFECTS, CEF_NO_DRAW, origin, NULL, 20);
	spawner->Update = mssithra_explosion_think;
	spawner->color.c = 0xff00ffff;
	spawner->dlight = CE_DLight_new(spawner->color, 100.0f,-50.0f);
	spawner->LifeTime = fxi.cl->time + 250;
	VectorCopy(dir, spawner->direction);

	AddEffect(NULL, spawner);

	fxi.S_StartSound(origin, -1, CHAN_AUTO, fxi.S_RegisterSound("monsters/mssithra/hit.wav"), 0.5, ATTN_NORM, 0);
}

void FXMSsithraExplodeSmall( vec3_t origin, vec3_t dir )
{
	//Play correct sound here
	FireSparks(NULL, FX_SPARKS, 0, origin, vec3_up);
	fxi.S_StartSound(origin, -1, CHAN_AUTO, fxi.S_RegisterSound("monsters/mssithra/hit.wav"), 0.5, ATTN_NORM, 0);
}

qboolean ArrowCheckFuse (client_entity_t *self, centity_t *owner)
{
	if ( (owner->current.effects & EF_ALTCLIENTFX) || (owner->current.effects & EF_MARCUS_FLAG1) )
	{//We've stopped moving and have imbedded ourself in a wall
		if(!(self->flags & CEF_NO_DRAW))
		{
			FireSparks(NULL, FX_SPARKS, 0, self->r.origin, vec3_up);
			self->flags |= CEF_NO_DRAW;
		}

		if(irand(0, 1))
			FXDarkSmoke(self->r.origin, flrand(0.2, 0.5), flrand(30, 50));
	}

	return true;
}

qboolean ArrowDrawTrail (client_entity_t *self, centity_t *owner)
{
	LinkedEntityUpdatePlacement(self, owner);
	
	VectorCopy(self->r.origin, self->r.startpos);
	VectorMA(self->r.startpos, self->SpawnInfo, self->direction, self->r.endpos);
	VectorMA(self->r.startpos, 8, self->direction, self->r.startpos);

	if (self->flags & CEF_FLAG6)
	{
		if (self->r.scale > 8.0)
			self->r.scale = self->r.scale2 = flrand(8.0, 12.0);

		if (self->SpawnInfo > -64)
			self->SpawnInfo-=4;

		if (self->LifeTime > 10)
		{
			self->LifeTime = 0;
			fxi.S_StartSound(self->r.origin, -1, CHAN_AUTO, fxi.S_RegisterSound("monsters/pssithra/guntravel.wav"), 0.5, ATTN_NORM, 0);
		}
		else
			self->LifeTime++;
	}
	else
	{
		if (self->r.scale > 4.0)
			self->r.scale = self->r.scale2 = flrand(4.0, 6.0);

		//Let the trail slowly extend
		if (self->SpawnInfo > -64)
			self->SpawnInfo-=2;
	}

	self->alpha = flrand(0.5, 1.0);

	return true;
}

void FXMSsithraArrow( centity_t *owner, vec3_t velocity, qboolean super )
{
	client_entity_t	*spawner;

	//Create an explosion spawner
	spawner = ClientEntity_new( FX_M_EFFECTS, CEF_OWNERS_ORIGIN, owner->current.origin, NULL, 20);
	spawner->r.model = mssithra_models + 2;
	spawner->r.spriteType = SPRITE_LINE;
	spawner->r.flags |= RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	spawner->r.color.c = 0xFFFFFFFF;
	spawner->r.scale = 1.0;
	spawner->alpha = 1.0;
	spawner->LifeTime = 0;

	if (super)
	{
		spawner->flags |= CEF_FLAG6;
		spawner->d_scale = spawner->d_scale2 = 16.0;
	}
	else
	{
		spawner->d_scale = spawner->d_scale2 = 8.0;
	}

	VectorCopy(spawner->r.origin, spawner->r.startpos);
	VectorNormalize2(velocity, spawner->direction);
	VectorMA(spawner->r.startpos, -64, spawner->direction, spawner->r.endpos);
	
	spawner->Update = ArrowCheckFuse;
	spawner->AddToView = ArrowDrawTrail;
	spawner->SpawnInfo = 0;

	AddEffect(owner, spawner);
}

void FXMSsithraArrowCharge( vec3_t startpos )
{
	client_entity_t	*TrailEnt;
	paletteRGBA_t	color = {255,128,255,255};
	vec3_t			dir;
	int				length;
	int				i;
	int				white;

	i = GetScaledCount(6, 0.85);

	while (i--)
	{
		TrailEnt=ClientEntity_new(FX_M_EFFECTS, 0, startpos, 0, 500);

		TrailEnt->r.model = mssithra_models + 2;
		
		TrailEnt->r.spriteType = SPRITE_LINE;

		TrailEnt->r.flags |= RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		TrailEnt->flags |= CEF_USE_VELOCITY2;
		TrailEnt->r.color.c = 0xFFFFFFFF;
		TrailEnt->r.scale = flrand(4.0, 6.0);
		TrailEnt->alpha = 0.1;
		TrailEnt->d_alpha = 0.25;
		TrailEnt->d_scale = 0.0;

		white = irand(128, 255);

		TrailEnt->r.color.r = white;
		TrailEnt->r.color.g = white;
		TrailEnt->r.color.b = 128 + irand(108, 127);
		TrailEnt->r.color.a = 64 + irand(16, 128);

		VectorSet(dir, 0, 0, 1);
		VectorRandomCopy(dir, TrailEnt->velocity2, 1.5);
		
		VectorCopy(startpos, TrailEnt->r.startpos);
		length = irand(24, 32);
		VectorMA(TrailEnt->r.startpos, length, TrailEnt->velocity2, TrailEnt->r.endpos);

		VectorScale(TrailEnt->velocity2, -(length*2), TrailEnt->velocity2);
		VectorClear(TrailEnt->velocity);

		AddEffect(NULL, TrailEnt);	
	}

	TrailEnt=ClientEntity_new(FX_M_EFFECTS, 0, startpos, 0, 500);

	white = irand(128, 255);

	TrailEnt->r.color.r = white;
	TrailEnt->r.color.g = white;
	TrailEnt->r.color.b = 128 + irand(108, 127);
	TrailEnt->r.color.a = 64 + irand(16, 128);

	TrailEnt->dlight = CE_DLight_new(TrailEnt->r.color, 200, -25);
}

void FXMEffects(centity_t *owner,int type,int flags, vec3_t org)
{
	vec3_t			vel;
	byte			fx_index;
	
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_M_EFFECTS].formatString, &fx_index, &vel);//fixme- make this 1 dir and 1 float

	switch (fx_index)
	{
		case FX_M_MISC_EXPLODE:
			FXMorkMissileExplode(owner, vel);
			break;

		case FX_M_BEAM:
			FXMorkBeam(owner, type, org, vel); //mxd
			break;

		case FX_IMP_FIRE:
			FXImpFireball(owner, org, vel); //mxd
			break;
		
		case FX_IMP_FBEXPL:
			FXImpFireBallExplode(owner, vel);
			break;

		case FX_CW_STARS:
			FXCWStars(owner, type, org);
			break;

		case FX_BUOY:
			FXBuoy(owner, flags, org, vel[0]);
			break;

		case FX_BUOY_PATH:
			FXBuoyPath(org, vel);
			break;

		case FX_M_MOBLUR:
			FXMMoBlur(owner, org, vel, false);
			break;

		case FX_ASS_DAGGER:
			FXAssDagger(owner, vel, org[0]);
			break;

		case FX_UNDER_WATER_WAKE:
			FXUnderWaterWake(owner);
			break;

		case FX_QUAKE_RING:
			FXQuakeRing(vel);
			break;

		case FX_GROUND_ATTACK:
			FXGroundAttack(vel);
			break;

		case FX_MORK_BEAM:
			FXMorkBeam2(owner, vel);
			break;

		case FX_MORK_MISSILE:
			FXMorkMissile(owner, vel);
			break;

		case FX_MORK_MISSILE_HIT:
			FXMorkMissileHit(org, vel);
			break;

		case FX_MORK_TRACKING_MISSILE:
			FXMorkTrackingMissile(owner, org, vel);
			break;

		case FX_MSSITHRA_EXPLODE:
			if (flags & CEF_FLAG6)
				FXMSsithraExplodeSmall(org, vel);
			else
				FXMSsithraExplode(org, vel);

			break;

		case FX_MSSITHRA_ARROW:
			FXMSsithraArrow(owner, vel, (flags & CEF_FLAG6));
			break;

		case FX_MSSITHRA_ARROW_CHARGE:
			FXMSsithraArrowCharge(vel);
			break;

		default:
			assert(0); //mxd
			break;
	}
}