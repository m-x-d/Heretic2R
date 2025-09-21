//
// fx_MorphOvum.c -- mxd. Named fx_Morph.c in original version.
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Particle.h"
#include "Random.h"
#include "Vector.h"
#include "Utilities.h"
#include "ce_Dlight.h"
#include "g_playstats.h"

#define MORPH_PARTICLE_VEL		8
#define MORPH_COLOR				0xff00ff00
#define MORPH_ARROW_SPEED		400.0f
#define MORPH_GLOW_INTENSITY	255
#define MORPH_ANGLE_INC			(ANGLE_360 / NUM_OF_OVUMS)

#define FEATHER_VEL				50.0f
#define FEATHER_FLOAT_TIME		15
#define FEATHER_FLOAT_SLOW		(ANGLE_180 / (float)FEATHER_FLOAT_TIME)

static struct model_s* morph_models[4];

void PreCacheMorph(void)
{
	morph_models[0] = fxi.RegisterModel("sprites/lens/halo1.sp2");
	morph_models[1] = fxi.RegisterModel("models/objects/eggs/chickenegg/tris.fm");
	morph_models[2] = fxi.RegisterModel("models/objects/feathers/feather1/tris.fm");
	morph_models[3] = fxi.RegisterModel("models/objects/feathers/feather2/tris.fm");
}

static qboolean MorphMissileThink(client_entity_t* missile, centity_t* owner)
{
	// Create a new entity for these particles to attach to.
	const int flags = (int)(missile->flags | CEF_NO_DRAW | CEF_ADDITIVE_PARTS); //mxd
	client_entity_t* trail = ClientEntity_new(FX_SPELL_MORPHMISSILE, flags, missile->r.origin, NULL, 1000);

	// And give it no owner, so its not deleted when the missile is.
	AddEffect(NULL, trail);

	// Create small particles.
	const int count = GetScaledCount(7, 0.5f);

	vec3_t diff;
	VectorSubtract(missile->r.origin, missile->origin, diff);
	Vec3ScaleAssign(1.0f / (float)count, diff);

	vec3_t cur_pos;
	VectorClear(cur_pos);

	int duration;
	if (R_DETAIL >= DETAIL_HIGH) //TODO: 800 on DETAIL_UBERHIGH.
		duration = 700;
	else if (R_DETAIL == DETAIL_NORMAL)
		duration = 600;
	else
		duration = 500;

	for (int i = 0; i < count; i++)
	{
		client_particle_t* ce = ClientParticle_new(PART_16x16_SPARK_G, color_white, duration);
		ce->acceleration[2] = 0.0f;

		// Figure out our random velocity.
		VectorRandomSet(ce->origin, MORPH_PARTICLE_VEL);

		// Scale it and make it the origin.
		VectorScale(ce->origin, -1.0f, ce->velocity);
		VectorAdd(cur_pos, ce->origin, ce->origin);

		// Add a fraction of the missile velocity to this particle velocity.
		vec3_t scaled_vel;
		VectorScale(missile->velocity, 0.1f, scaled_vel);
		VectorAdd(scaled_vel, ce->velocity, ce->velocity);

		ce->scale = flrand(3.0f, 6.0f);
		AddParticleToList(trail, ce);

		Vec3SubtractAssign(diff, cur_pos);
	}

	// Remember for even spread of particles.
	VectorCopy(missile->r.origin, missile->origin);

	// Rotate the missile.
	missile->r.angles[0] -= 0.7f;

	return true;
}

// We reflected, create a new missile.
void FXMorphMissile(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	byte yaw;
	byte pitch;

	// Get the initial yaw.
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_SPELL_MORPHMISSILE].formatString, &yaw, &pitch);

	// Create the client effect with the light on it.
	client_entity_t* missile = ClientEntity_new(type, flags | CEF_DONT_LINK, origin, NULL, 100);

	missile->radius = 32.0f;
	missile->r.angles[YAW] = (float)yaw * BYTEANGLE_TO_RAD; //mxd. Use macro.
	missile->r.angles[PITCH] = (float)pitch * BYTEANGLE_TO_RAD; //mxd. Use macro.

	// Figure out where we are going.
	DirFromAngles(missile->r.angles, missile->velocity);

	const float arrow_speed = MORPH_ARROW_SPEED * ((flags & CEF_FLAG6) ? 0.5f : 1.0f); //mxd
	Vec3ScaleAssign(arrow_speed, missile->velocity);

	missile->r.model = &morph_models[1]; // Egg model.
	missile->r.scale = 3.0f;
	missile->r.angles[PITCH] = -ANGLE_90; // Set the pitch AGAIN.
	missile->color.c = MORPH_COLOR;
	missile->dlight = CE_DLight_new(missile->color, 150.0f, 0.0f);
	missile->Update = MorphMissileThink;

	AddEffect(owner, missile);

	fxi.S_StartSound(missile->r.origin, -1, CHAN_WEAPON, fxi.S_RegisterSound("weapons/OvumFire.wav"), 1.0f, ATTN_NORM, 0.0f);
}

// Initial entry from server - create first object. This has the light on it, but no trail yet.
void FXMorphMissileInitial(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	// Get the initial yaw.
	byte yaw;
	short morph_array[NUM_OF_OVUMS];
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_SPELL_MORPHMISSILE_INITIAL].formatString, &yaw,
		&morph_array[0], &morph_array[1], &morph_array[2], &morph_array[3], &morph_array[4], &morph_array[5]);

	float yaw_rad = (float)yaw * BYTEANGLE_TO_RAD; //mxd. Use macro.

	for (int i = 0; i < NUM_OF_OVUMS; i++)
	{
		// Create the client effect with the light on it.
		client_entity_t* missile = ClientEntity_new(type, flags | CEF_DONT_LINK, origin, NULL, 100);
		missile->r.angles[YAW] = yaw_rad;

		// Figure out where we are going.
		DirFromAngles(missile->r.angles, missile->velocity);
		Vec3ScaleAssign(MORPH_ARROW_SPEED, missile->velocity);

		missile->radius = 32.0f;
		missile->r.model = &morph_models[1]; // Egg model.
		missile->r.scale = 3.0f;
		missile->r.angles[PITCH] = -ANGLE_90;

		missile->color.c = MORPH_COLOR;
		missile->dlight = CE_DLight_new(missile->color, 110.0f, 0.0f);
		missile->Update = MorphMissileThink;

		AddEffect(&fxi.server_entities[morph_array[i]], missile);

		yaw_rad += MORPH_ANGLE_INC;

		if (i == 0)
			fxi.S_StartSound(missile->r.origin, -1, CHAN_WEAPON, fxi.S_RegisterSound("weapons/OvumFire.wav"), 1.0f, ATTN_NORM, 0.0f);
	}

	if (R_DETAIL >= DETAIL_HIGH)
	{
		client_entity_t* glow = ClientEntity_new(type, flags, origin, NULL, 800);

		glow->r.model = &morph_models[0]; // Halo1 sprite.
		glow->r.flags = RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;

		glow->color.c = MORPH_COLOR;
		glow->r.color.c = MORPH_COLOR;
		glow->d_scale = 1.8f;
		glow->r.scale = 0.5f;
		glow->d_alpha = -1.0f;
		glow->dlight = CE_DLight_new(glow->color, MORPH_GLOW_INTENSITY, -MORPH_GLOW_INTENSITY);

		AddEffect(NULL, glow);
	}
}

// We hit a wall or an object.
void FXMorphExplode(centity_t* owner, int type, const int flags, vec3_t origin)
{
#define SMOKE_SPEED 160.0f

	int duration;
	float max_scale;

	vec3_t dir;
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_SPELL_MORPHEXPLODE].formatString, dir);

	// Make a bunch of small particles explode out the wall / object.
	VectorScale(dir, SMOKE_SPEED, dir);
	const int count = GetScaledCount(40, 0.3f);

	if (R_DETAIL >= DETAIL_HIGH) //TODO: separate DETAIL_UBERHIGH.
	{
		max_scale = 10.0f;
		duration = 600;
	}
	else if (R_DETAIL == DETAIL_NORMAL)
	{
		max_scale = 8.0f;
		duration = 500;
	}
	else
	{
		max_scale = 7.0f;
		duration = 400;
	}

	// Create a light at the point of explosion.
	const int dlight_flags = CEF_NO_DRAW | CEF_NOMOVE | CEF_ADDITIVE_PARTS; //mxd
	client_entity_t* dlight = ClientEntity_new(-1, dlight_flags, origin, NULL, duration);

	paletteRGBA_t color = { .c = MORPH_COLOR };
	dlight->dlight = CE_DLight_new(color, 110.0f, 100.0f); //TODO: make it fade-out?

	AddEffect(NULL, dlight);

	for (int i = 0; i < count; i++)
	{
		COLOUR_SET(color, irand(25, 50), irand(200, 255), color.r); //mxd. Use macro.
		client_particle_t* ce = ClientParticle_new(PART_16x16_SPARK_G, color, duration);

		VectorCopy(dir, ce->velocity);

		for (int c = 0; c < 3; c++)
			ce->velocity[c] += flrand(-SMOKE_SPEED, SMOKE_SPEED);

		ce->scale = flrand(3.0f, max_scale);

		AddParticleToList(dlight, ce);
	}
}

// Make the feather float down.
static qboolean FeatherThink(client_entity_t* self, centity_t* owner)
{
	if (--self->SpawnInfo == 0)
		return false;

	if (self->SpawnInfo < 10)
		self->alpha -= 0.1f;

	// Is the feather on the way down?
	if (self->velocity[2] < 2.0f)
	{
		// Yes, set gravity much lower.
		self->acceleration[2] = -1.0f;
		self->velocity[2] = -13.0f;

		// Make the x and z motion much less each time.
		self->velocity[0] -= self->velocity[0] * 0.1f;
		self->velocity[1] -= self->velocity[1] * 0.1f;

		// Has the feather already hit the horizontal?
		if (self->r.angles[PITCH] == 1.2f)
		{
			// If its time, reverse the direction of the swing.
			if (--self->LifeTime == 0)
			{
				self->LifeTime = FEATHER_FLOAT_TIME;
				self->xscale = -self->xscale;
				self->yscale = -self->yscale;
			}

			// Add in the feather swing to the origin.
			const float scale = sinf((float)self->LifeTime * FEATHER_FLOAT_SLOW);

			self->r.origin[0] += self->xscale * scale;
			self->r.origin[1] += self->yscale * scale;
			self->r.origin[2] += scale * 1.4f - 0.7f;

			return true;
		}

		// Wait till the feather hits the horizontal by itself.
		if (self->r.angles[PITCH] < 1.3f && self->r.angles[PITCH] > 1.1f)
		{
			self->r.angles[PITCH] = 1.2f;
			self->yscale = flrand(-2.5f, 2.5f);
			self->xscale = flrand(-2.5f, 2.5f);
			self->LifeTime = FEATHER_FLOAT_TIME;

			return true;
		}
	}

	// Still on the way up or not hit the horizontal yet, so keep it spinning.
	self->r.angles[PITCH] += self->yscale;
	self->r.angles[YAW] += self->xscale;

	// This is bogus, but has to be done if the above pitch check is going to work.
	if (self->r.angles[PITCH] < 0.0f)
		self->r.angles[PITCH] += ANGLE_360;
	else if (self->r.angles[PITCH] > ANGLE_360)
		self->r.angles[PITCH] -= ANGLE_360;

	return true;
}

// Make the feathers zip out of the carcass and float down.
void FXChickenExplode(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	//NOTE: CEF_FLAG6 denotes we just want to spawn a couple feathers
	if (flags & CEF_FLAG6)
	{
		const int count = irand(1, 2);

		for (int i = 0; i < count; i++)
		{
			client_entity_t* feather = ClientEntity_new(type, flags & ~CEF_OWNERS_ORIGIN, origin, NULL, 40);

			feather->radius = 5.0f;
			feather->r.model = &morph_models[irand(2, 3)]; // Feather models.
			feather->r.flags = RF_TRANSLUCENT;

			for (int c = 0; c < 3; c++)
				feather->origin[c] += flrand(-8.0f, 8.0f);

			feather->acceleration[2] = flrand(-120.0f, -85.0f); //mxd. Was irand().
			VectorSet(feather->velocity, flrand(-100.0f, 100.0f), flrand(-100.0f, 100.0f), flrand(50.0f, 150.0f));

			feather->r.scale = flrand(0.5f, 1.5f);
			feather->yscale = flrand(0.05f, 0.2f);
			feather->xscale = flrand(-0.2f, 0.2f);
			feather->SpawnInfo = 170;
			feather->Update = FeatherThink;

			AddEffect(NULL, feather);
		}
	}
	else
	{
		for (int i = 0; i < 20; i++)
		{
			client_entity_t* feather = ClientEntity_new(type, flags & ~CEF_OWNERS_ORIGIN, origin, NULL, 40);

			feather->radius = 5.0f;
			feather->r.model = &morph_models[irand(2, 3)]; // Feather models.
			feather->r.flags = RF_TRANSLUCENT;

			feather->acceleration[2] = -85.0f;
			VectorSet(feather->velocity, flrand(-FEATHER_VEL, FEATHER_VEL), flrand(-FEATHER_VEL, FEATHER_VEL), flrand(80.0f, 140.0f));
			feather->r.scale = flrand(0.5f, 2.5f);

			feather->yscale = flrand(0.1f, 0.3f);
			feather->xscale = flrand(-0.3f, 0.3f);
			feather->SpawnInfo = 170;
			feather->Update = FeatherThink;

			AddEffect(NULL, feather);
		}
	}
}