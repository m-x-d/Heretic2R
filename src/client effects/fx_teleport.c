//
// fx_teleport.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Particle.h"
#include "Random.h"
#include "Utilities.h"
#include "Vector.h"

#define PARTICLE_VELOCITY_IN			400.0f
#define PARTICLE_VELOCITY_OUT			70.0f
#define PARTICLE_ACCELERATION_SCALE_IN	3.1f
#define PARTICLE_ACCELERATION_SCALE_OUT	12.0f

#define NUM_TELEPORT_FX_PARTICLES		250
#define NUM_TELEPORT_PAD_PARTICLES		2

#define TELEPORT_PAD_HEIGHT				10.0f
#define TELEPORT_PAD_RADIUS				56.0f

static struct model_s* teleport_models[2];

void PreCacheTeleport(void)
{
	teleport_models[0] = fxi.RegisterModel("sprites/spells/teleport_1.sp2");
	teleport_models[1] = fxi.RegisterModel("sprites/spells/teleport_2.sp2");
}

void FXPlayerTeleportIn(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	// Create the teleport effect around the player to begin with.
	client_entity_t* teleport_fx = ClientEntity_new(type, flags, origin, NULL, 410);

	paletteRGBA_t color;
	byte* col1;
	byte* col2;

	// Determine if this teleport is a spell, or a pad. Set up our colors appropriately.
	if (!(flags & CEF_FLAG6))
	{
		col1 = &color.g;
		col2 = &color.r;
		teleport_fx->r.model = &teleport_models[0];
	}
	else
	{
		col1 = &color.r;
		col2 = &color.g;
		teleport_fx->r.model = &teleport_models[1];
	}

	teleport_fx->radius = 20.0f;
	teleport_fx->r.flags = (RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA);
	teleport_fx->r.scale = 1.8f;
	teleport_fx->d_scale = -3.0f;
	teleport_fx->AddToView = LinkedEntityUpdatePlacement;

	AddEffect(owner, teleport_fx);

	// Spawn particles.
	const int count = GetScaledCount(NUM_TELEPORT_FX_PARTICLES, 0.3f);

	for (int i = 0; i < count; i++)
	{
		const byte temp_col = (byte)irand(0, 255);
		*col1 = temp_col;
		*col2 = 255;
		color.b = temp_col;
		color.a = 255;

		// Use single point particles if we are in software.
		client_particle_t* p = ClientParticle_new(PART_4x4_WHITE | PFL_SOFT_MASK, color, 400);

		const vec3_t angles = VEC3_SET(flrand(0.0f, ANGLE_360), flrand(0.0f, ANGLE_360), flrand(0.0f, ANGLE_360));
		DirFromAngles(angles, p->velocity);
		Vec3ScaleAssign(flrand(PARTICLE_VELOCITY_IN - 30.0f, PARTICLE_VELOCITY_IN), p->velocity);
		VectorScale(p->velocity, -PARTICLE_ACCELERATION_SCALE_IN, p->acceleration);

		AddParticleToList(teleport_fx, p);
	}
}

void FXPlayerTeleportOut(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	// Create the teleport effect around the player to begin with.
	client_entity_t* teleport_fx = ClientEntity_new(type, flags, origin, NULL, 410);

	paletteRGBA_t color;
	byte* col1;
	byte* col2;

	// Determine if this teleport is a spell, or a pad. Set up our colors appropriately.
	if (!(flags & CEF_FLAG6))
	{
		col1 = &color.g;
		col2 = &color.r;
		teleport_fx->r.model = &teleport_models[0];
	}
	else
	{
		col1 = &color.r;
		col2 = &color.g;
		teleport_fx->r.model = &teleport_models[1];
	}

	teleport_fx->radius = 20.0f;
	teleport_fx->r.flags = (RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA);
	teleport_fx->r.scale = 0.5f;
	teleport_fx->d_scale = 3.0f;
	teleport_fx->AddToView = LinkedEntityUpdatePlacement;

	AddEffect(owner, teleport_fx);

	// Spawn particles.
	const int count = GetScaledCount(NUM_TELEPORT_FX_PARTICLES, 0.3f);

	for (int i = 0; i < count; i++)
	{
		const byte temp_col = (byte)irand(0, 255);
		*col1 = temp_col;
		*col2 = 255;
		color.b = temp_col;
		color.a = 1;

		// Use single point particles if we are in software.
		client_particle_t* p = ClientParticle_new(PART_4x4_WHITE | PFL_SOFT_MASK, color, 400);

		p->d_alpha = 300.0f;
		VectorSet(p->origin, 1.0f, 1.0f, 1.0f);

		const vec3_t angles = VEC3_SET(flrand(0.0f, ANGLE_360), flrand(0.0f, ANGLE_360), flrand(0.0f, ANGLE_360));
		DirFromAngles(angles, p->origin);
		Vec3ScaleAssign(flrand(PARTICLE_VELOCITY_OUT - 10.0f, PARTICLE_VELOCITY_OUT), p->origin);
		VectorScale(p->origin, -PARTICLE_ACCELERATION_SCALE_OUT, p->acceleration);

		AddParticleToList(teleport_fx, p);
	}
}

static qboolean TeleportPadUpdate(client_entity_t* self, centity_t* owner) //mxd. Named 'FXteleportPadThink' in original logic.
{
	const int count = GetScaledCount(NUM_TELEPORT_PAD_PARTICLES, 0.7f);

	for (int i = 0; i < count; i++)
	{
		// Stuff gets sucked to the center.
		client_particle_t* c1 = ClientParticle_new((int)(PART_16x16_SPARK_R | PFL_NEARCULL | PFL_MOVE_SPHERE), color_white, 1000);

		c1->color.a = 1;
		c1->d_alpha = 256.0f;
		c1->scale = 16.0f;
		c1->d_scale = -12.0f;

		c1->origin[SPH_RADIUS] = TELEPORT_PAD_RADIUS;
		c1->origin[SPH_YAW] = flrand(ANGLE_0, ANGLE_360);
		c1->origin[SPH_PITCH] = -ANGLE_90;

		c1->velocity[SPH_YAW] = ANGLE_360 * flrand(-2.0f, 2.0f);
		c1->velocity[SPH_PITCH] = ANGLE_180;

		c1->acceleration[SPH_RADIUS] = -2.0f * TELEPORT_PAD_RADIUS;

		AddParticleToList(self, c1);

		// Stuff comes up from the ground.
		client_particle_t* c2 = ClientParticle_new((int)(PART_16x16_LIGHTNING | PFL_NEARCULL | PFL_MOVE_CYL_Z), color_white, 1000);

		c2->color.a = 1;
		c2->d_alpha = 512.0f;
		c2->scale = 4.0f;
		c2->d_scale = -4.0f;

		c2->origin[CYL_RADIUS] = TELEPORT_PAD_RADIUS * 0.5f;
		c2->origin[CYL_Z] = -TELEPORT_PAD_RADIUS;
		c2->origin[CYL_YAW] = flrand(ANGLE_0, ANGLE_360);

		c2->velocity[CYL_RADIUS] = TELEPORT_PAD_RADIUS * 0.5f;

		c2->acceleration[CYL_Z] = 5.0f * TELEPORT_PAD_RADIUS;
		c2->acceleration[CYL_YAW] = 2.0f * ANGLE_360;

		AddParticleToList(self, c2);
	}

	if (irand(0, 1) == 0)
	{
		client_particle_t* ce = ClientParticle_new((int)(PART_16x16_SPARK_R | PFL_NEARCULL), color_white, 250);

		ce->d_alpha = -1024.0f;
		ce->scale = 8.0f;
		ce->d_scale = flrand(80.0f, 160.0f); //mxd. Original logic uses irand() here.
		VectorRandomSet(ce->velocity, 16.0f);

		AddParticleToList(self, ce);
	}

	return true;
}

// This is the persistent effect for the teleport pad.
void FXTeleportPad(centity_t* owner, const int type, int flags, vec3_t origin)
{
	flags |= (CEF_NO_DRAW | CEF_ADDITIVE_PARTS | CEF_PULSE_ALPHA | CEF_VIEWSTATUSCHANGED | CEF_NOMOVE);
	client_entity_t* glow = ClientEntity_new(type, flags, origin, NULL, 110);

	glow->radius = 100.0f;
	glow->r.origin[2] += TELEPORT_PAD_HEIGHT;
	glow->Update = TeleportPadUpdate;

	AddEffect(owner, glow);
}