//
// fx_sphereofannihilation.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Matrix.h"
#include "Particle.h"
#include "Random.h"
#include "Reference.h"
#include "Utilities.h"
#include "Vector.h"
#include "ce_DLight.h"
#include "q_Sprite.h"
#include "g_playstats.h"

#define FX_SPHERE_FLY_SPEED					600.0f
#define FX_SPHERE_AURA_SCALE				1.2f
#define FX_SPHERE_EXPLOSION_BASE_RADIUS		89.0f
#define FX_SPHERE_EXPLOSION_SMOKE_SPEED		140.0f
#define FX_SPHERE_EXPLOSION_PITCH_INCREMENT	(ANGLE_180 / 32.0f) //mxd
#define FX_SPHERE_EXPLOSION_YAW_INCREMENT	(ANGLE_180 / 27.0f) //mxd

static struct model_s* sphere_models[8];

void PreCacheSphere(void)
{
	sphere_models[0] = fxi.RegisterModel("sprites/spells/shboom.sp2");
	sphere_models[1] = fxi.RegisterModel("sprites/spells/bluball.sp2");
	sphere_models[2] = fxi.RegisterModel("sprites/spells/glowball.sp2");
	sphere_models[3] = fxi.RegisterModel("models/spells/sphere/tris.fm");
	sphere_models[4] = fxi.RegisterModel("sprites/fx/halo.sp2");
	sphere_models[5] = fxi.RegisterModel("sprites/spells/glowbeam.sp2");
	sphere_models[6] = fxi.RegisterModel("sprites/fx/haloblue.sp2");
	sphere_models[7] = fxi.RegisterModel("sprites/spells/spark_blue.sp2");
}

static qboolean SphereOfAnnihilationSphereUpdate(client_entity_t* self, centity_t* owner) //mxd. Named 'FXSphereOfAnnihilationSphereThink' in original logic.
{
	float detail_scale;
	if (R_DETAIL == DETAIL_LOW)
		detail_scale = 0.7f;
	else if (R_DETAIL == DETAIL_NORMAL)
		detail_scale = 0.85f;
	else
		detail_scale = 1.0f; //TODO: separate case for DETAIL_UBERHIGH. Why is sphere scaled by detail level?..

	self->r.scale = owner->current.scale * detail_scale;

	return true;
}

static qboolean SphereOfAnnihilationAuraUpdate(client_entity_t* self, centity_t* owner) //mxd. Named 'FXSphereOfAnnihilationAuraThink' in original logic.
{
	// No aura trail on low level.
	if (R_DETAIL == DETAIL_LOW)
		return true;

	vec3_t trail_start = VEC3_INIT(owner->origin);

	vec3_t trail_offset;
	VectorSubtract(owner->lerp_origin, owner->origin, trail_offset);

	// Copy the real origin to the entity origin, so the light will follow us.
	VectorCopy(self->r.origin, self->origin);

	float trail_length = VectorNormalize(trail_offset);
	if (trail_length < 0.001f)
		trail_length += 2.0f;

	Vec3ScaleAssign(FX_SPHERE_FLY_SPEED, trail_offset);

	const int duration = ((R_DETAIL > DETAIL_NORMAL) ? 500 : 400);
	const int flags = (int)(self->flags & ~(CEF_OWNERS_ORIGIN | CEF_NO_DRAW)); //mxd

	for (int i = 0; i < 40 && trail_length > 0.0f; i++)
	{
		client_entity_t* trail = ClientEntity_new(FX_WEAPON_SPHERE, flags, trail_start, NULL, duration);

		trail->radius = 70.0f;
		trail->r.model = &sphere_models[0]; // shboom sprite.
		trail->r.flags = (RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA);
		trail->Scale = FX_SPHERE_AURA_SCALE + flrand(0.0f, 0.1f);
		COLOUR_SET(trail->color, irand(128, 180), irand(128, 180), irand(64, 80)); //mxd. Use macro.
		trail->alpha = 0.7f;
		trail->d_alpha = -1.0f;
		trail->d_scale = -0.5f;

		AddEffect(NULL, trail);

		trail_length -= FX_SPHERE_FLY_SPEED;
		Vec3AddAssign(trail_offset, trail_start);
	}

	return true;
}

void FXSphereOfAnnihilation(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	static const paletteRGBA_t light_color = { .r = 0, .g = 0, .b = 255, .a = 255 };

	short caster_entnum;
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_WEAPON_SPHERE].formatString, &caster_entnum);

	// Create a fiery blue aura around the sphere.
	const int caster_update = ((R_DETAIL < DETAIL_NORMAL) ? 125 : 100); //mxd
	client_entity_t* aura_thinker = ClientEntity_new(type, flags, origin, NULL, caster_update);

	aura_thinker->flags |= CEF_NO_DRAW;
	if (R_DETAIL > DETAIL_LOW)
		aura_thinker->dlight = CE_DLight_new(light_color, 150.0f, 0.0f);

	aura_thinker->extra = (void*)(&fxi.server_entities[caster_entnum]); // The caster's centity_t.
	aura_thinker->AddToView = LinkedEntityUpdatePlacement;
	aura_thinker->Update = SphereOfAnnihilationAuraUpdate;

	AddEffect(owner, aura_thinker);
	SphereOfAnnihilationAuraUpdate(aura_thinker, owner);

	// Create the sphere of annihilation itself.
	client_entity_t* sphere = ClientEntity_new(type, flags, origin, NULL, 100);

	sphere->radius = 70.0f;
	sphere->r.model = &sphere_models[1]; // bluball sprite.
	sphere->r.flags = RF_TRANSLUCENT;
	sphere->r.scale = owner->current.scale;
	sphere->AddToView = LinkedEntityUpdatePlacement;
	sphere->Update = SphereOfAnnihilationSphereUpdate;

	AddEffect(owner, sphere);
}

static qboolean SphereOfAnnihilationGlowballUpdate(client_entity_t* self, centity_t* owner) //mxd. Named 'FXSphereOfAnnihilationGlowballThink' in original logic.
{
	if (owner->current.effects & EF_MARCUS_FLAG1)
		self->color.r++;

	int duration;
	if (R_DETAIL == DETAIL_LOW)
		duration = 300;
	else if (R_DETAIL == DETAIL_NORMAL)
		duration = 400;
	else
		duration = 500; //TODO: separate case for DETAIL_UBERHIGH.

	if (self->color.r > 3)
	{
		// Create a trailing spark.
		client_entity_t* spark = ClientEntity_new(FX_WEAPON_SPHERE, self->flags & ~(CEF_OWNERS_ORIGIN), self->r.origin, NULL, duration);

		spark->radius = 20.0f;
		spark->r.model = &sphere_models[2]; // glowball sprite.
		spark->r.flags = (RF_TRANSLUCENT | RF_TRANS_ADD);
		COLOUR_SET(spark->r.color, irand(128, 180), irand(128, 180), irand(180, 255)); //mxd. Use macro.
		spark->Scale = flrand(0.8f, 1.0f);
		spark->d_scale = -1.5f;

		AddEffect(NULL, spark);
	}

	if (self->color.r < 16)
	{
		for (int i = 0; i < 3; i++)
		{
			self->velocity[i] *= 3.0f;
			self->velocity[i] += 6.0f * (owner->origin[i] - self->r.origin[i]);
			self->velocity[i] *= 0.265f;
		}

		return true;
	}

	return false;
}

static qboolean SphereOfAnnihilationGlowballSpawnerUpdate(client_entity_t* self, centity_t* owner) //mxd. Named 'FXSphereOfAnnihilationGlowballSpawnerThink' in original logic.
{
	// 'Self->extra' refers to the caster's centity_t.
	const centity_t* controller = (centity_t*)self->extra;

	// This tells if we are wasting our time, because the reference points are culled.
	if (controller != NULL && !RefPointsValid(controller)) // Only if we were SUPPOSED to have refpoints.
		return true;

	if ((owner->current.effects & EF_MARCUS_FLAG1) == 0)
		return true;

	// If the spell is still building, create some swirling blue Glowballs.
	vec3_t ball_origin;
	if (controller != NULL)
		VectorCopy(controller->origin, ball_origin);
	else
		VectorCopy(self->r.origin, ball_origin);

	client_entity_t* glowball = ClientEntity_new(FX_WEAPON_SPHEREGLOWBALLS, (int)(self->flags & ~(CEF_NO_DRAW | CEF_OWNERS_ORIGIN)), ball_origin, NULL, 50);

	glowball->flags |= CEF_DONT_LINK;

	// Make me spawn from my caster's left / right hands (alternating). Assuming we aren't a reflection type glowball.
	if (controller != NULL)
	{
		matrix3_t rotation;
		Matrix3FromAngles(controller->lerp_angles, rotation);

		if (self->SpawnInfo != 0)
		{
			const int ref_point = ((self->color.g & 1) ? CORVUS_RIGHTHAND : CORVUS_LEFTHAND); //mxd
			Matrix3MultByVec3(rotation, controller->referenceInfo->references[ref_point].placement.origin, glowball->r.origin);
		}
		else
		{
			vec3_t angles = VEC3_INIT(controller->current.angles);
			Vec3ScaleAssign(RAD_TO_ANGLE, angles);

			vec3_t forward;
			AngleVectors(angles, forward, NULL, NULL);

			Vec3ScaleAssign(16.0f, forward); // Hard-coded for Celestial Watcher (monster_elflord). 
			Matrix3MultByVec3(rotation, forward, glowball->r.origin);
		}

		Vec3AddAssign(controller->origin, glowball->r.origin);
	}
	else
	{
		for (int i = 0; i < 3; i++)
			glowball->r.origin[i] = self->r.origin[i] + flrand(-10.0f, 10.0f);
	}

	vec3_t owner_angles;
	VectorScale(owner->current.angles, RAD_TO_ANGLE, owner_angles);

	vec3_t owner_forward;
	AngleVectors(owner_angles, owner_forward, NULL, NULL);

	// Set my velocity and acceleration.
	glowball->velocity[0] = owner_forward[0] * 175.0f + flrand(-25.0f, 25.0f);
	glowball->velocity[1] = owner_forward[1] * 175.0f + flrand(-25.0f, 25.0f);
	glowball->velocity[2] = flrand(-200.0f, 100.0f);

	const int axis = ((self->color.g & 1) ? 0 : 1); //mxd
	glowball->velocity[axis] *= -1.0f;

	VectorClear(glowball->acceleration);

	// Fill in the rest of my info.
	glowball->radius = 20.0f;
	glowball->r.model = &sphere_models[2]; // glowball sprite.
	glowball->r.flags = (RF_TRANSLUCENT | RF_TRANS_ADD);
	COLOUR_SET(glowball->r.color, irand(128, 180), irand(128, 180), irand(180, 255)); //mxd. Use macro.
	glowball->color.r = 1;
	glowball->extra = (void*)owner;
	glowball->Update = SphereOfAnnihilationGlowballUpdate;

	AddEffect(owner, glowball);
	SphereOfAnnihilationGlowballUpdate(glowball, owner);

	self->color.g++;

	return true;
}

void FXSphereOfAnnihilationGlowballs(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	// Get the caster's centity_t.
	short caster_entnum;
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_WEAPON_SPHEREGLOWBALLS].formatString, &caster_entnum);

	// Create a spawner that will create the glowballs.
	const int caster_update = ((R_DETAIL >= DETAIL_NORMAL) ? 100 : 250); //mxd
	client_entity_t* glowball_spawner = ClientEntity_new(type, flags | CEF_VIEWSTATUSCHANGED, origin, NULL, caster_update);

	glowball_spawner->flags |= CEF_NO_DRAW;
	glowball_spawner->color.g = 0;
	glowball_spawner->SpawnInfo = !(flags & CEF_FLAG6);

	if (caster_entnum > -1)
		glowball_spawner->extra = (void*)&fxi.server_entities[caster_entnum];

	glowball_spawner->AddToView = LinkedEntityUpdatePlacement;
	glowball_spawner->Update = SphereOfAnnihilationGlowballSpawnerUpdate;

	AddEffect(owner, glowball_spawner);
}

static qboolean SphereOfAnnihilationSphereExplosionUpdate(client_entity_t* self, centity_t* owner) //mxd. Named 'FXSphereOfAnnihilationSphereExplodeThink' in original logic.
{
	float frac = (float)(fx_time - self->startTime) / 100.0f;
	if (self->AnimSpeed > 0.0f)
		frac *= self->AnimSpeed;

	if (floorf(frac) >= (float)(self->NoOfAnimFrames - 1))
		return false;

	// Spin the ball of blue fire whilst it expands and fades.
	self->r.angles[0] += FX_SPHERE_EXPLOSION_PITCH_INCREMENT;
	self->r.angles[1] += FX_SPHERE_EXPLOSION_YAW_INCREMENT;

	self->radius = FX_SPHERE_EXPLOSION_BASE_RADIUS * self->r.scale;
	self->dlight->intensity = self->radius / 0.7f;

	const float multiplier = 1.0f - frac / (float)(self->NoOfAnimFrames - 1);
	const byte c = (byte)(multiplier * 255);
	COLOUR_SET(self->dlight->color, c, c, c); //mxd. Use macro.
	self->alpha = multiplier;

	return true;
}

void FXSphereOfAnnihilationExplode(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	vec3_t dir;
	byte size;
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_WEAPON_SPHEREEXPLODE].formatString, dir, &size);

	if (flags & CEF_FLAG6)
		FXClientScorchmark(origin, dir);

	// Create an expanding ball of blue fire.
	client_entity_t* explosion = ClientEntity_new(type, flags | CEF_ADDITIVE_PARTS, origin, NULL, 50);

	explosion->radius = FX_SPHERE_EXPLOSION_BASE_RADIUS * explosion->r.scale;
	explosion->r.model = &sphere_models[3]; // Sphere model.
	explosion->r.flags = (RF_FULLBRIGHT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA);
	explosion->r.scale = 0.01f;
	explosion->d_scale = 2.5f;
	explosion->NoOfAnimFrames = (int)size;
	explosion->AnimSpeed = 1.0f;
	explosion->dlight = CE_DLight_new(color_white, explosion->radius / 0.7f, 0.0f);
	explosion->Update = SphereOfAnnihilationSphereExplosionUpdate;

	AddEffect(NULL, explosion);
	SphereOfAnnihilationSphereExplosionUpdate(explosion, NULL);

	// Add some glowing blast particles.
	Vec3ScaleAssign(FX_SPHERE_EXPLOSION_SMOKE_SPEED, dir);
	const int count = GetScaledCount(40, 0.3f);

	for (int i = 0; i < count; i++)
	{
		client_particle_t* ce = ClientParticle_new(PART_16x16_SPARK_B, color_white, 600);

		VectorCopy(dir, ce->velocity);
		ce->scale = flrand(16.0f, 32.0f);

		for (int c = 0; c < 3; c++)
			ce->velocity[c] += flrand(-FX_SPHERE_EXPLOSION_SMOKE_SPEED, FX_SPHERE_EXPLOSION_SMOKE_SPEED);

		AddParticleToList(explosion, ce);
	}
}

void FXSphereOfAnnihilationPower(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	vec3_t dir;
	byte b_size;
	byte b_len;
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_WEAPON_SPHEREPOWER].formatString, dir, &b_size, &b_len);

	// If there is a cheaper way to get ACCURATE right and up, I'd be happy to see it...
	vec3_t angles;
	vectoangles(dir, angles);
	angles[PITCH] *= -1.0f;// something's broken with angle signs somewhere ;(

	vec3_t forward;
	vec3_t right;
	vec3_t up;
	AngleVectors(angles, forward, right, up);

	// Only one beam.
	const vec3_t beam_start = VEC3_INIT(origin);

	// When CEF_FLAG8 is set, move to the left. Otherwise to the right.
	const float vel_scaler = SPHERE_LASER_SPEED * ((flags & CEF_FLAG8) ? -0.4f : 0.4f); //mxd
	const float acc_scaler = SPHERE_LASER_SPEED * ((flags & CEF_FLAG8) ? 1.0f : -1.0f); //mxd

	// Make the flares at the start.
	client_entity_t* flare_start = ClientEntity_new(type, flags | CEF_ADDITIVE_PARTS, beam_start, NULL, 500);

	flare_start->radius = 128.0f;
	flare_start->r.model = &sphere_models[6]; // Blue halo sprite.
	flare_start->r.flags = (RF_TRANS_ADD | RF_TRANS_ADD_ALPHA | RF_TRANSLUCENT);
	flare_start->d_alpha = -4.0f;
	flare_start->r.scale = 0.25f;
	flare_start->d_scale = -0.5f;

	VectorScale(right, vel_scaler, flare_start->velocity);
	VectorScale(right, acc_scaler, flare_start->acceleration);

	AddEffect(NULL, flare_start);

	const float len = (float)b_len * 8.0f; // Shrunk down so range can be up to 2048.
	const float size = b_size; //mxd

	vec3_t beam_end;
	VectorMA(beam_start, len, forward, beam_end);

	// Make the line beam down the side.
	client_entity_t* beam = ClientEntity_new(-1, CEF_DONT_LINK, beam_start, NULL, 200);

	beam->radius = 256.0f;
	beam->r.model = &sphere_models[5]; // Glowbeam sprite.
	beam->r.spriteType = SPRITE_LINE;
	beam->r.flags = (RF_TRANS_ADD | RF_TRANS_ADD_ALPHA);
	beam->r.scale = (size - 3.0f) * 6.0f;
	beam->alpha = 0.95f;
	beam->d_alpha = -5.0f;
	VectorCopy(beam_start, beam->r.startpos);
	VectorCopy(beam_end, beam->r.endpos);

	VectorScale(right, vel_scaler, beam->velocity);
	VectorScale(right, acc_scaler, beam->acceleration);

	AddEffect(NULL, beam);

	const int count = GetScaledCount((int)(25.0f + size * 2.5f), 0.3f);

	// Make the particles.
	for (int i = 0; i < count; i++)
	{
		client_particle_t* ce = ClientParticle_new(PART_16x16_SPARK_B, color_white, 666);

		ce->scale = flrand(8.0f, 24.0f) + size * 2.0f;
		ce->scale *= 0.4f;
		ce->acceleration[2] = 0.0f;
		ce->d_alpha = -768.0f;
		VectorMA(ce->origin, flrand(0.0f, len), forward, ce->origin);
		VectorMA(ce->velocity, flrand(-15.0f, 15.0f), right, ce->velocity);
		VectorMA(ce->velocity, flrand(-15.0f, 15.0f), up, ce->velocity);
		VectorMA(ce->origin, flrand(-size * 0.4f, size * 0.4f), right, ce->origin);
		VectorMA(ce->origin, flrand(-size * 0.4f, size * 0.4f), up, ce->origin);

		AddParticleToList(flare_start, ce);
	}

	if (flags & CEF_FLAG6)
	{
		// Make the flare at the end of the line.
		client_entity_t* flare_end = ClientEntity_new(type, flags | CEF_ADDITIVE_PARTS, beam_end, NULL, 500);

		flare_end->radius = 128.0f;
		flare_end->r.model = &sphere_models[6]; // Blue halo sprite.
		flare_end->r.flags = (RF_TRANS_ADD | RF_TRANS_ADD_ALPHA | RF_TRANSLUCENT);
		flare_end->d_scale = 1.0f;
		flare_end->d_alpha = -2.0f;

		AddEffect(NULL, flare_end);
	}
}

static qboolean SpherePlayerExplosionUpdate(client_entity_t* self, centity_t* owner) //mxd. Named 'FXSpherePlayerExplodeThink' in original logic.
{
	if (fx_time > self->nextEventTime)
	{
		self->d_alpha = -5.0f;
		self->dlight->d_intensity = -self->radius * 2.0f;

		if (fx_time > self->nextEventTime + 1000)
			return false;
	}
	else
	{
		self->dlight->intensity = FX_SPHERE_EXPLOSION_BASE_RADIUS * self->r.scale * 1.7f;
	}

	return true;
}

static qboolean SpherePlayerExplosionAddToView(client_entity_t* self, centity_t* owner) //mxd. Named 'FXSpherePlayerExplodeAddToView' in original logic.
{
	self->r.angles[0] += FX_SPHERE_EXPLOSION_PITCH_INCREMENT * (float)(fx_time - self->lastThinkTime) / 50.0f;
	self->r.angles[1] += FX_SPHERE_EXPLOSION_YAW_INCREMENT *   (float)(fx_time - self->lastThinkTime) / 50.0f;

	self->lastThinkTime = fx_time;

	return true;
}

static qboolean SpherePlayerExplosionGlowballAddToView(client_entity_t* glowball, centity_t* owner) //mxd. Named 'FXSpherePlayerExplodeGlowballThink' in original logic.
{
	// Update the angle of the spark.
	VectorMA(glowball->direction, (float)(fx_time - glowball->lastThinkTime) / 1000.0f, glowball->velocity2, glowball->direction);

	glowball->radius = (SPHERE_RADIUS_MAX - SPHERE_RADIUS_MIN) * ((float)(fx_time - glowball->SpawnDelay) / 100.0f) / (SPHERE_MAX_CHARGES + 2);

	// Update the position of the spark.
	vec3_t forward;
	AngleVectors(glowball->direction, forward, NULL, NULL);
	VectorMA(glowball->origin, glowball->radius, forward, glowball->r.origin);

	glowball->lastThinkTime = fx_time;

	return true;
}

static qboolean SpherePlayerExplosionGlowballUpdate(client_entity_t* glowball, centity_t* owner) //mxd. Named 'FXSpherePlayerExplodeGlowballTerminate' in original logic.
{
	// Don't instantly delete yourself. Don't accept any more updates and die out within a second.
	glowball->d_alpha = -5.0f; // Fade out.
	glowball->updateTime = 1000; // Die in one second.
	glowball->Update = RemoveSelfAI;

	return true;
}

void FXSpherePlayerExplode(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	vec3_t dir;
	byte size;
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_WEAPON_SPHEREPLAYEREXPLODE].formatString, dir, &size);

	// Create an expanding ball of blue fire.
	client_entity_t* explosion = ClientEntity_new(type, flags | CEF_ADDITIVE_PARTS, origin, NULL, 50);

	explosion->radius = SPHERE_RADIUS_MIN + ((SPHERE_RADIUS_MAX - SPHERE_RADIUS_MIN) / SPHERE_MAX_CHARGES * (float)explosion->SpawnInfo);
	explosion->r.model = &sphere_models[3]; // Sphere model.
	explosion->r.flags = (RF_FULLBRIGHT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA);
	explosion->r.scale = 0.01f;
	explosion->color = color_white;
	explosion->d_scale = 1.5f;
	explosion->SpawnInfo = (int)size;
	explosion->updateTime = (size + 1) * 100;
	explosion->nextEventTime = fx_time + explosion->updateTime;
	explosion->lastThinkTime = fx_time;
	explosion->dlight = CE_DLight_new(color_white, explosion->radius / 0.7f, 0);

	explosion->AddToView = SpherePlayerExplosionAddToView;
	explosion->Update = SpherePlayerExplosionUpdate;

	AddEffect(NULL, explosion);
	SpherePlayerExplosionUpdate(explosion, NULL);

	// Add some glowing blast particles.
	Vec3ScaleAssign(FX_SPHERE_EXPLOSION_SMOKE_SPEED, dir);

	const int count = GetScaledCount(40, 0.3f);

	for (int i = 0; i < count; i++)
	{
		client_entity_t* glowball = ClientEntity_new(type, flags & ~CEF_OWNERS_ORIGIN, origin, NULL, 5000);

		glowball->r.model = &sphere_models[7]; // spark_blue sprite.
		glowball->r.flags = (RF_FULLBRIGHT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA);
		glowball->d_scale = 3.0f;
		glowball->lastThinkTime = fx_time;
		glowball->SpawnDelay = fx_time;
		glowball->nextThinkTime = fx_time + (explosion->SpawnInfo + 1) * 100;

		VectorClear(glowball->direction);

		// This is a velocity around the sphere.
		for (int c = 0; c < 2; c++)
		{
			glowball->direction[c] = flrand(0.0f, 360.0f); // This angle is kept at a constant distance from org.
			glowball->velocity2[c] = flrand(-90.0f, 90.0f);
			glowball->velocity2[c] += 90.0f * Q_signf(glowball->velocity2[c]); // Assure that the sparks are moving around at a pretty good clip.
		}

		glowball->AddToView = SpherePlayerExplosionGlowballAddToView;
		glowball->Update = SpherePlayerExplosionGlowballUpdate;

		AddEffect(NULL, glowball);
	}

	// Now make a big flash.
	client_entity_t* flash = ClientEntity_new(type, flags, origin, NULL, 250);

	flash->radius = 128.0f;
	flash->r.model = &sphere_models[4]; // halo sprite.
	flash->r.flags = (RF_TRANS_ADD | RF_TRANS_ADD_ALPHA | RF_TRANSLUCENT);
	flash->r.frame = 1;
	flash->d_alpha = -4.0f;
	flash->d_scale = -4.0f;

	AddEffect(NULL, flash);
}