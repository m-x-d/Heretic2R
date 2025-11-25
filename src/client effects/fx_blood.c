//
// fx_blood.c
//
// Copyright 1998 Raven Software
//

#include "fx_blood.h" //mxd
#include "Client Effects.h"
#include "Particle.h"
#include "Vector.h"
#include "Random.h"
#include "Reference.h"
#include "Utilities.h"
#include "g_playstats.h"

static struct model_s* splat_models[2];
static struct sfx_s* splat_sounds[3]; //mxd

void PreCacheSplat(void)
{
	splat_models[0] = fxi.RegisterModel("sprites/fx/bsplat.sp2");
	splat_models[1] = fxi.RegisterModel("sprites/fx/ysplat.sp2");
}

void PreCacheSplatSFX(void) //mxd
{
	splat_sounds[0] = fxi.S_RegisterSound("ambient/waterdrop1.wav");
	splat_sounds[1] = fxi.S_RegisterSound("ambient/waterdrop2.wav");
	splat_sounds[2] = fxi.S_RegisterSound("ambient/waterdrop3.wav");
}

static int insect_blood_particles[] =
{
	//PART_4x4_GREEN, //mxd. Way too green...
	//PART_4x4_YELLOW, //mxd. Way too yellow...
	PART_8x8_GLOBBIT1,
	PART_8x8_GLOBBIT2,
	PART_16x16_MIST,
	PART_16x16_GLOB,
	PART_16x16_SPARK_G,
	PART_16x16_SPARK_Y,
	PART_32x32_GREENBLOOD,
	PART_16x16_GREENBLOOD,
	PART_4x4_GREENBLOOD1,
	PART_4x4_GREENBLOOD2
};

#define NUM_INSECT_BLOOD_PARTICLES	ARRAY_SIZE(insect_blood_particles) //mxd

client_entity_t* DoBloodSplash(vec3_t loc, int amount, const qboolean yellow_blood)
{
	client_particle_t* drop;

	client_entity_t* splash = ClientEntity_new(FX_BLOOD, CEF_NO_DRAW, loc, NULL, 1000);
	AddEffect(NULL, splash);

	const float speed = (float)amount * 4.0f + 16.0f;
	const float gravity = GetGravity() * 0.5f;
	const int extra_flags = (R_DETAIL > DETAIL_HIGH ? PFL_LM_COLOR : 0); //mxd

	amount = min(500, amount);

	for (int i = 0; i < amount; i++)
	{
		const int size = i & 3; // Size = 0-3.

		switch (size)
		{
			case 0: // Tiny particles.
			{
				for (int j = 0; j < 6; j++)
				{
					if (ref_soft)
					{
						const int part_type = (yellow_blood ? PART_4x4_GREENBLOOD1 : PART_4x4_BLOOD1);
						drop = ClientParticle_new(part_type | extra_flags | PFL_SOFT_MASK, color_red, 650); //mxd. +extra_flags.
					}
					else
					{
						const int part_type = (yellow_blood ? irand(PART_4x4_GREENBLOOD1, PART_4x4_GREENBLOOD2) : irand(PART_4x4_BLOOD1, PART_4x4_BLOOD2));
						drop = ClientParticle_new(part_type | extra_flags, color_white, 800); //mxd. +extra_flags.
					}

					VectorSet(drop->velocity, flrand(-speed, speed), flrand(-speed, speed), flrand(speed * 2.0f, speed * 4.0f));
					drop->acceleration[2] = gravity;
					drop->d_alpha = 0.0f;
					drop->d_scale = -1.0f;

					AddParticleToList(splash, drop);
				}
			} break;

			case 1: // Some larger globs.
			{
				for (int j = 0; j < 3; j++)
				{
					if (ref_soft)
					{
						const int bpart = (yellow_blood ? PART_8x8_GLOBBIT1 : PART_8x8_BLOOD);
						drop = ClientParticle_new(bpart | extra_flags | PFL_SOFT_MASK, color_red, 650); //mxd. +extra_flags.
					}
					else
					{
						const int bpart = (yellow_blood ? irand(PART_8x8_GLOBBIT1, PART_8x8_GLOBBIT2) : PART_8x8_BLOOD);
						drop = ClientParticle_new(bpart | extra_flags, color_white, 800); //mxd. +extra_flags.
					}

					VectorSet(drop->velocity, flrand(-speed, speed), flrand(-speed, speed), flrand(speed * 2.0f, speed * 4.0f));
					drop->scale = 2.0f;
					drop->acceleration[2] = gravity;
					drop->d_alpha = 0.0f;
					drop->d_scale = -2.0f;

					AddParticleToList(splash, drop);
				}
			} break;

			case 2: // Some big blobs.
			{
				for (int j = 0; j < 2; j++)
				{
					const int bpart = (yellow_blood ? PART_16x16_GREENBLOOD : PART_16x16_BLOOD);
					drop = ClientParticle_new(bpart | extra_flags, color_white, 1000); //mxd. +extra_flags.
					VectorSet(drop->velocity, flrand(-speed, speed), flrand(-speed, speed), flrand(speed, speed * 2.0f));
					drop->scale = 1.0f;
					drop->acceleration[2] = gravity * 0.5f;
					drop->d_scale = flrand(4.0f, 8.0f);
					drop->d_alpha = flrand(-512.0f, -256.0f);

					AddParticleToList(splash, drop);
				}
			} break;

			case 3: // A big splash.
			{
				const int bpart = (yellow_blood ? PART_32x32_GREENBLOOD : PART_32x32_BLOOD);
				drop = ClientParticle_new(bpart | extra_flags, color_white, 500); //mxd. +extra_flags.
				VectorSet(drop->velocity, flrand(-speed, speed), flrand(-speed, speed), flrand(0, speed));
				drop->scale = 4.0f;
				drop->acceleration[2] = 0.0f;
				drop->d_scale = flrand(48.0f, 64.0f);
				drop->d_alpha = flrand(-1024.0f, -512.0f);

				AddParticleToList(splash, drop);
			} break;
		}
	}

	return splash;
}

void DoBloodTrail(client_entity_t* spawner, int amount)
{
	client_particle_t* drop;

	const qboolean yellow_blood = ((spawner->SpawnInfo & SIF_FLAG_MASK) == MAT_INSECT); // Insect blood is yellow-green.

	const float speed = (amount == -1 ? 0.0f : (float)amount * 4.0f + 8.0f);
	const float gravity = GetGravity() * 0.5f;
	const float range = (float)amount;
	const int extra_flags = (R_DETAIL > DETAIL_HIGH ? PFL_LM_COLOR : 0); //mxd

	amount = min(500, amount);

	for (int i = 0; i < amount; i++)
	{
		const int size = i & 1;	// Size = 0-1.

		switch (size)
		{
			case 0: // Tiny particles.
			{
				for (int j = 0; j < 3; j++)
				{
					if (ref_soft)
					{
						const int bpart = (yellow_blood ? insect_blood_particles[irand(0, NUM_INSECT_BLOOD_PARTICLES - 1)] : PART_4x4_BLOOD1); //mxd
						drop = ClientParticle_new(bpart | extra_flags | PFL_SOFT_MASK, color_red, 800); //mxd. +extra_flags.
					}
					else
					{
						const int bpart = (yellow_blood ? insect_blood_particles[irand(0, NUM_INSECT_BLOOD_PARTICLES - 1)] : irand(PART_4x4_BLOOD1, PART_4x4_BLOOD2)); //mxd
						drop = ClientParticle_new(bpart | extra_flags, color_white, 800); //mxd. +extra_flags.
					}

					VectorSet(drop->velocity, flrand(-speed, speed), flrand(-speed, speed), flrand(speed * 2.0f, speed * 4.0f));
					VectorMA(drop->velocity, 0.5f, spawner->velocity, drop->velocity);
					VectorRandomSet(drop->origin, range); //mxd
					VectorAdd(drop->origin, spawner->r.origin, drop->origin);
					drop->acceleration[2] = gravity;
					drop->d_alpha = 0.0f;
					drop->d_scale = -1.0f;

					AddParticleToList(spawner, drop);
				}
			} break;

			case 1: // Some larger globs.
			{
				const int bpart = (yellow_blood ? insect_blood_particles[irand(0, NUM_INSECT_BLOOD_PARTICLES - 1)] : PART_8x8_BLOOD); //mxd
				drop = ClientParticle_new(bpart | extra_flags, color_white, 800); //mxd. +extra_flags.
				VectorSet(drop->velocity, flrand(-speed, speed), flrand(-speed, speed), flrand(speed * 2.0f, speed * 4.0f));
				VectorMA(drop->velocity, 0.5f, spawner->velocity, drop->velocity);
				VectorRandomSet(drop->origin, range); //mxd
				VectorAdd(drop->origin, spawner->r.origin, drop->origin);
				drop->acceleration[2] = gravity;
				drop->d_alpha = 0.0f;
				drop->scale = 2.0f;
				drop->d_scale = -2.0f;

				AddParticleToList(spawner, drop);
			} break;
		}
	}
}

// Floor blood splat update: spawn blood splat particles.
static qboolean BloodSplatSplashUpdate(client_entity_t* self, centity_t* owner)
{
	//mxd. Show the splash.
	if (self->flags & CEF_NO_DRAW)
	{
		self->flags &= ~CEF_NO_DRAW;
		self->alpha = 0.1f; //mxd. Add subtle fade-in effect.
		self->d_alpha = 2.5f;
	}

	client_particle_t* p = NULL;
	const qboolean dark_blood = (self->flags & CEF_FLAG7); //mxd
	const qboolean yellow_blood = (self->flags & CEF_FLAG8);
	const int extra_flags = (R_DETAIL > DETAIL_HIGH ? PFL_LM_COLOR : 0); //mxd

	paletteRGBA_t color;
	if (dark_blood)
		COLOUR_SETA(color, 90, 70, 55, 160);
	else
		COLOUR_SETA(color, 180, 140, 110, 160);

	while (self->SpawnInfo > 0)
	{
		if (ref_soft)
		{
			const int bpart = (yellow_blood ? insect_blood_particles[irand(0, NUM_INSECT_BLOOD_PARTICLES - 1)] : PART_4x4_BLOOD1);
			p = ClientParticle_new(bpart | extra_flags | PFL_SOFT_MASK, color, 800); //mxd. +extra_flags.
		}
		else
		{
			const int bpart = (yellow_blood ? insect_blood_particles[irand(0, NUM_INSECT_BLOOD_PARTICLES - 1)] : irand(PART_4x4_BLOOD1, PART_4x4_BLOOD2));
			p = ClientParticle_new(bpart | extra_flags, color, 800); //mxd. +extra_flags.
		}

		p->acceleration[2] = GetGravity() * 0.2f;

		vec3_t vel;
		VectorRandomCopy(self->direction, vel, 10.0f);
		VectorScale(vel, flrand(2.0f, 5.0f), p->velocity);

		p->d_alpha = 0.0f;
		p->scale = flrand(0.6f, 1.0f);
		AddParticleToList(self, p);

		self->SpawnInfo--;
	}

	if (p != NULL) //mxd. Added sanity check.
		fxi.S_StartSound(p->origin, -1, CHAN_AUTO, splat_sounds[irand(0, 2)], flrand(0.5f, 0.8f), ATTN_STATIC, 0.0f);

	self->r.scale += 0.01f; //mxd. Grow a bit...
	self->Update = NULL; //mxd. Await next BloodSplatDripUpdate() call...

	return true;
}

// Ceiling blood splat update: spawn blood drops particles.
static qboolean BloodSplatDripUpdate(client_entity_t* self, centity_t* owner)
{
#define BLOODDROPS_TIME	6000.0f //mxd. How long to spawn blood-drops.

	if (!AttemptRemoveSelf(self, owner))
	{
		if (self->floor_bloodsplat != NULL)
			self->floor_bloodsplat->Update = RemoveSelfAI;

		return false;
	}

	if (self->bloodsplat_cur_particles < 1)
		return true;

	const qboolean dark_blood = (self->flags & CEF_FLAG7); //mxd
	const qboolean yellow_blood = (self->flags & CEF_FLAG8);

	paletteRGBA_t color;
	if (dark_blood)
		COLOUR_SETA(color, 75, 70, 55, 160);
	else
		COLOUR_SETA(color, 150, 140, 110, 160);

	const float lifetime_scaler = 0.5f + ((float)self->bloodsplat_cur_particles / (float)self->bloodsplat_max_particles) * 0.5f; //mxd. Reduce particles count over time [1.0 -> 0.5].
	const int num_drips = (int)((float)(irand(7, 15)) * lifetime_scaler);
	const float scale = flrand(0.2f, 0.4f) * lifetime_scaler;
	const int extra_flags = (R_DETAIL > DETAIL_HIGH ? PFL_LM_COLOR : 0); //mxd

	//mxd. Randomize position a bit.
	const float p_scale = self->r.scale * 6.0f;
	const float origin_offset[2] = { flrand(-p_scale, p_scale), flrand(-p_scale, p_scale) };

	//mxd. Use the same sprite for the whole shape.
	int duration = 0;
	int bpart;

	if (ref_soft)
		bpart = ((yellow_blood ? insect_blood_particles[irand(0, NUM_INSECT_BLOOD_PARTICLES - 1)] : PART_4x4_BLOOD1) | PFL_SOFT_MASK);
	else
		bpart = (yellow_blood ? insect_blood_particles[irand(0, NUM_INSECT_BLOOD_PARTICLES - 1)] : irand(PART_4x4_BLOOD1, PART_4x4_BLOOD2));

	// Create drop-like shape out of multiple particles.
	for (int i = 0; i < num_drips; i++)
	{
		client_particle_t* p = ClientParticle_new(bpart | extra_flags, color, 0); //mxd. +extra_flags.

		const float grav_mod = 0.4f + (float)i * 0.025f;
		p->acceleration[2] = GetGravity() * grav_mod;
		p->d_alpha = 0.0f; // Disable particle fade-out.
		p->scale = scale + (float)i * 0.08f;
		p->origin[0] += origin_offset[0];
		p->origin[1] += origin_offset[1];

		p->duration = (int)(self->radius * 18.0f * grav_mod);

		if (duration == 0)
			duration = p->duration;

		AdvanceParticle(p, i * 4);
		AddParticleToList(self, p);
	}

	//mxd. Setup floor splash, don't show it yet (don't setup if previous splash hasn't happened yet).
	if (self->floor_bloodsplat != NULL && self->floor_bloodsplat->SpawnInfo == 0 && duration > 0)
	{
		self->floor_bloodsplat->SpawnInfo = num_drips; // Pass number of spawned particles.
		self->floor_bloodsplat->Update = BloodSplatSplashUpdate;
		self->floor_bloodsplat->updateTime = duration;
		self->floor_bloodsplat->nextThinkTime = fx_time + duration;
	}

	//mxd. Set next update time.
	self->bloodsplat_cur_particles--;

	const float frac = sinf(ANGLE_90 + ((float)self->bloodsplat_cur_particles / (float)self->bloodsplat_max_particles) * ANGLE_90);
	const int next_time = (int)(BLOODDROPS_TIME * frac);

	self->updateTime = next_time - self->updateTime;
	self->nextThinkTime = fx_time + self->updateTime;

	return true;
}

static client_entity_t* InitFloorSplat(const vec3_t origin, const vec3_t normal, const float scale, const qboolean dark, const qboolean yellow) //mxd
{
	client_entity_t* floor_splat = ClientEntity_new(FX_BLOOD, CEF_NOMOVE | CEF_NO_DRAW, origin, normal, 1100);

	floor_splat->r.angles[ROLL] = flrand(0.0f, ANGLE_360);
	floor_splat->r.model = &splat_models[yellow ? 1 : 0];
	floor_splat->r.frame = irand(0, 4);
	floor_splat->r.flags = (RF_FIXED | RF_ALPHA_TEXTURE); 

	if (R_DETAIL >= DETAIL_HIGH) //mxd. +RF_LM_COLOR.
		floor_splat->r.flags |= RF_LM_COLOR;

	const byte brightness = (byte)(dark ? irand(32, 72) : irand(72, 128));
	COLOUR_SET(floor_splat->r.color, brightness, brightness, brightness); //mxd. Use macro.

	floor_splat->radius = 10.0f;
	floor_splat->r.scale = scale * flrand(0.2f, 0.45f);

	if (dark)
		floor_splat->flags |= CEF_FLAG7; //mxd

	if (yellow)
		floor_splat->flags |= CEF_FLAG8;

	AddEffect(NULL, floor_splat);
	InsertInCircularList(floor_splat);

	return floor_splat;
}

void ThrowBlood(const vec3_t torigin, const vec3_t tnormal, const qboolean dark, const qboolean yellow, const qboolean trueplane) //mxd. Original logic modifies 'origin' when !trueplane.
{
#define MIN_DRIPPER_NORMAL_Z	0.4f //mxd. Original logic uses GROUND_NORMAL instead.

	vec3_t normal;
	VectorCopy(tnormal, normal);

	vec3_t origin;
	VectorCopy(torigin, origin);

	if (!trueplane)
	{
		VectorInverse(normal);

		if (!GetTruePlane(origin, normal, 16.0f, flrand(0.25f, 0.5f))) //mxd. Add offset_scale randomization (to reduce z-fighting among overlapping blood splats).
			return;
	}

	client_entity_t* bsplat = ClientEntity_new(FX_BLOOD, CEF_NOMOVE, origin, normal, 1000);

	bsplat->r.angles[ROLL] = flrand(0.0f, ANGLE_360);
	bsplat->r.model = &splat_models[yellow ? 1 : 0];
	bsplat->r.frame = irand(0, 4);
	bsplat->r.flags = (RF_FIXED | RF_ALPHA_TEXTURE);

	if (R_DETAIL >= DETAIL_HIGH) //mxd. +RF_LM_COLOR.
		bsplat->r.flags |= RF_LM_COLOR;

	bsplat->alpha = 0.1f; //mxd. Add subtle fade-in effect.
	bsplat->d_alpha = 2.5f;

	const byte brightness = (byte)(dark ? irand(32, 72) : irand(72, 128));
	COLOUR_SET(bsplat->r.color, brightness, brightness, brightness); //mxd. Use macro.

	bsplat->radius = 10.0f;
	bsplat->r.scale = flrand(0.2f, 0.45f);

	bsplat->Update = AttemptRemoveSelf;

	// When hit ceiling, init blood dripper fx.
	if (normal[2] <= -MIN_DRIPPER_NORMAL_Z && !irand(0, 2) && bsplat->r.frame != 2 && bsplat->r.frame != 4)
	{
		vec3_t end_pos;
		VectorMA(origin, 256.0f, vec3_down, end_pos); //mxd. Original logic adds scaled normal instead (which is strange).

		trace_t tr;
		fxi.Trace(origin, vec3_origin, vec3_origin, end_pos, MASK_DRIP, CEF_CLIP_TO_WORLD, &tr);

		if (tr.fraction < 1.0f && tr.fraction > 0.0625f) // Between 16 and 256.
		{
			bsplat->radius = tr.fraction * 256.0f;

			//mxd. Create matching floor splat. //TODO: different logic when FLOOR is LAVA.
			if (tr.plane.normal[2] > MIN_DRIPPER_NORMAL_Z)
			{
				vec3_t splat_pos;
				VectorMA(tr.endpos, flrand(0.25f, 0.5f), tr.plane.normal, splat_pos); // Lift off the floor a bit.

				bsplat->floor_bloodsplat = InitFloorSplat(splat_pos, tr.plane.normal, bsplat->r.scale, dark, yellow);
			}
		}
		else if (tr.fraction == 1.0f)
		{
			bsplat->radius = 256.0f;
		}

		//mxd. Setup total number of particles to spawn.
		if (bsplat->radius > 10.0f)
		{
			bsplat->bloodsplat_max_particles = irand(7, 16);
			bsplat->bloodsplat_cur_particles = bsplat->bloodsplat_max_particles;

			if (dark)
				bsplat->flags |= CEF_FLAG7; //mxd

			if (yellow)
				bsplat->flags |= CEF_FLAG8;

			bsplat->Update = BloodSplatDripUpdate;
			BloodSplatDripUpdate(bsplat, NULL); //mxd. Start dropping particles right away.
		}
	}

	AddEffect(NULL, bsplat);
	InsertInCircularList(bsplat);
}

void FXBloodTrail(centity_t* owner, int type, const int flags, vec3_t origin)
{
	vec3_t normal;
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_BLOOD_TRAIL].formatString, &normal);

	VectorMA(origin, 0.25f, normal, origin);
	ThrowBlood(origin, normal, flags & CEF_FLAG7, flags & CEF_FLAG6, true);
}

void FXBlood(centity_t* owner, int type, const int flags, vec3_t origin)
{
	byte amount;
	vec3_t velocity;

	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_BLOOD].formatString, &velocity, &amount);

	// Let's add level of detail here to the amount we pump out.
	if (R_DETAIL == DETAIL_LOW)
		amount = (byte)((float)amount * 0.5f);
	else if (R_DETAIL == DETAIL_NORMAL)
		amount = (byte)((float)amount * 0.75f);

	amount = max(1, amount);

	const qboolean yellow_blood = (flags & CEF_FLAG8);
	client_entity_t* spawner = DoBloodSplash(origin, amount, yellow_blood);
	VectorCopy(velocity, spawner->velocity);
}

static qboolean LinkedBloodThink(client_entity_t* spawner, centity_t* owner)
{
#define NUM_BLOOD_PARTS		3

	spawner->updateTime = irand(40, 60);
	spawner->LifeTime -= 50;

	if (spawner->LifeTime < 0 || !RefPointsValid(owner)) // Effect finished or reference points are culled.
		return false;

	if (spawner->LifeTime < 800) // Effect needs to stay alive until particles die.
		return true;

	const qboolean yellow_blood = (spawner->flags & CEF_FLAG8);
	const int extra_flags = (R_DETAIL > DETAIL_HIGH ? PFL_LM_COLOR : 0); //mxd

	vec3_t org;
	VectorGetOffsetOrigin(owner->referenceInfo->references[spawner->SpawnInfo].placement.origin, owner->current.origin, owner->current.angles[YAW], org);

	client_entity_t* ce = ClientEntity_new(-1, 0, org, NULL, 800);
	ce->flags |= CEF_NO_DRAW | CEF_NOMOVE;
	ce->radius = 32.0f;
	AddEffect(NULL, ce);

	vec3_t vel;
	VectorSubtract(org, spawner->origin, vel);
	Vec3ScaleAssign(5.0f, vel);

	for (int i = 0; i < NUM_BLOOD_PARTS; i++)
	{
		const int bpart = (yellow_blood ? insect_blood_particles[irand(0, NUM_INSECT_BLOOD_PARTICLES - 1)] : irand(PART_4x4_BLOOD1, PART_4x4_BLOOD2));
		client_particle_t* p = ClientParticle_new(bpart | extra_flags, spawner->color, 800); //mxd. +extra_flags.
		VectorCopy(vel, p->velocity);
		p->acceleration[2] = GetGravity();
		p->d_alpha = 0.0f;
		p->d_scale = -1.0f;

		AddParticleToList(ce, p);
	}

	// Remember current origin for calc of velocity.
	VectorCopy(org, spawner->origin);
	return true;
}

void FXLinkedBlood(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	byte life;
	byte refpointidx;
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_LINKEDBLOOD].formatString, &life, &refpointidx);

	client_entity_t* spawner = ClientEntity_new(type, flags, origin, NULL, (int)(fxi.cls->rframetime * 2000.0f));

	spawner->LifeTime = life;
	spawner->SpawnInfo = refpointidx;
	spawner->flags |= CEF_NO_DRAW;
	spawner->color = color_white; //mxd
	spawner->AddToView = OffsetLinkedEntityUpdatePlacement;
	spawner->Update = LinkedBloodThink;

	AddEffect(owner, spawner);
}