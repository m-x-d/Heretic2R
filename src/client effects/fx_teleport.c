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

void PlayerTeleportIn(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	paletteRGBA_t color;
	byte* col1;
	byte* col2;

	// Create the teleport effect around the player to begin with.
	client_entity_t* teleport_fx = ClientEntity_new(type, flags, origin, NULL, 410);

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
	teleport_fx->r.flags = RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
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

		const vec3_t angles = { flrand(0.0f, ANGLE_360), flrand(0.0f, ANGLE_360), flrand(0.0f, ANGLE_360) };
		DirFromAngles(angles, p->velocity);
		Vec3ScaleAssign(flrand(PARTICLE_VELOCITY_IN - 30.0f, PARTICLE_VELOCITY_IN), p->velocity);
		VectorScale(p->velocity, -PARTICLE_ACCELERATION_SCALE_IN, p->acceleration);

		AddParticleToList(teleport_fx, p);
	}
}

void PlayerTeleportOut(centity_t *owner, int type, int flags, vec3_t origin)
{
	client_entity_t  	*teleport_fx;
	int 					i, temp_col;
	client_particle_t	*p;
	paletteRGBA_t		color;
	int 					particle_type;
	vec3_t				angles;
	byte					*col1;
	byte					*col2;
	byte					*col3;
	int					count;

	// create the teleport effect around the player to begin with
 	teleport_fx = ClientEntity_new(type, flags, origin, NULL, 410);
	teleport_fx->radius = 20.0F;
 	teleport_fx->AddToView = LinkedEntityUpdatePlacement;

	// determine if this teleport is a spell, or a pad
	// set up our colors appropriately
	if (!(flags & CEF_FLAG6))
	{
		col1 = &color.b;
		col2 = &color.g;
		col3 = &color.r;
		teleport_fx->r.model = &teleport_models[0];
	}
	else
	{
		col1 = &color.b;
		col2 = &color.r;
		col3 = &color.g;
		teleport_fx->r.model = &teleport_models[1];
	}

	teleport_fx->r.flags = RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA; 
	teleport_fx->r.frame = 0;
	teleport_fx->r.scale = 0.5;
	teleport_fx->d_scale = 3.0;

  	AddEffect(owner, teleport_fx);

	// Use single point particles if we are in software
	particle_type = PART_4x4_WHITE | PFL_SOFT_MASK;

	// spawn particles
	count = GetScaledCount(NUM_TELEPORT_FX_PARTICLES, 0.3);
	for(i = 0; i < count; i++)
	{
		temp_col = irand(0, 255); 
		*col1 = *col2 = temp_col;
		*col3 = 255;
		color.a = 1;
		p = ClientParticle_new(particle_type, color, 400);

		p->d_alpha = 300;
		VectorSet(p->origin, 1.0,1.0,1.0);
		VectorSet(angles, flrand(0, 6.28), flrand(0, 6.28), flrand(0, 6.28));
		DirFromAngles(angles,p->origin);
		Vec3ScaleAssign(flrand(PARTICLE_VELOCITY_OUT-10, PARTICLE_VELOCITY_OUT), p->origin);
		VectorScale(p->origin, -PARTICLE_ACCELERATION_SCALE_OUT, p->acceleration);

		AddParticleToList(teleport_fx, p);
	}
}


static qboolean FXteleportPadThink(struct client_entity_s *self, centity_t *owner)
{
	client_particle_t	*ce;
	paletteRGBA_t		color;
//	vec3_t				vel;
	int					i;
	int					count;

	count = GetScaledCount(NUM_TELEPORT_PAD_PARTICLES, 0.7);
	for(i = 0; i < count; i++)
	{
		// Stuff gets sucked to the center
		color.c = 0xffffffff;
  		ce = ClientParticle_new(PART_16x16_SPARK_R | PFL_NEARCULL | PFL_MOVE_SPHERE, color, 1000);

		ce->color.a = 1;
		ce->d_alpha = 256;
		ce->scale = 16.0F;
		ce->d_scale = -12.0F;

		ce->origin[SPH_RADIUS] = TELEPORT_PAD_RADIUS;
		ce->origin[SPH_YAW] = flrand(0, ANGLE_360);
//		ce->origin[SPH_PITCH] = flrand(-ANGLE_90, 0);
		ce->origin[SPH_PITCH] = -ANGLE_90;

		ce->velocity[SPH_YAW] = flrand(-2.0*ANGLE_360, 2.0*ANGLE_360);
		ce->velocity[SPH_PITCH] = ANGLE_180;

		ce->acceleration[SPH_RADIUS] = -2.0*TELEPORT_PAD_RADIUS;

		AddParticleToList(self, ce);

		// Stuff comes up from the ground
		color.c = 0xffffffff;
  		ce = ClientParticle_new(PART_16x16_LIGHTNING | PFL_NEARCULL | PFL_MOVE_CYL_Z, color, 1000);

		ce->color.a = 1;
		ce->d_alpha = 512;
		ce->scale = 4.0F;
		ce->d_scale = -4.0F;

		ce->origin[CYL_RADIUS] = TELEPORT_PAD_RADIUS*0.5;
		ce->origin[CYL_Z] = -TELEPORT_PAD_RADIUS;
		ce->origin[CYL_YAW] = flrand(0, ANGLE_360);

		ce->velocity[CYL_RADIUS] = TELEPORT_PAD_RADIUS*0.5;

		ce->acceleration[CYL_Z] = 5.0*TELEPORT_PAD_RADIUS;
		ce->acceleration[CYL_YAW] = 2.0*ANGLE_360;

		AddParticleToList(self, ce);
	}

	if (irand(0,1) == 0)
	{
		color.c = 0xffffffff;
  		ce = ClientParticle_new(PART_16x16_SPARK_R | PFL_NEARCULL, color, 250);
		VectorSet(ce->velocity, flrand(-16.0, 16.0), flrand(-16.0, 16.0), flrand(-16.0, 16.0));

		ce->color.a = 255;
		ce->d_alpha = -1024;
		ce->scale = 8.0F;
		ce->d_scale = irand(80.0, 160.0);
		AddParticleToList(self, ce);
	}


	return(true);
}

// This is the persistant effect for the teleport pad
void FXTeleportPad(centity_t *owner, int type, int flags, vec3_t origin)
{
	client_entity_t		*glow;

	flags |= CEF_NO_DRAW | CEF_ADDITIVE_PARTS | CEF_PULSE_ALPHA | CEF_VIEWSTATUSCHANGED | CEF_NOMOVE;
	glow = ClientEntity_new(type,  flags, origin, 0, 110);
	glow->Update = FXteleportPadThink;
	glow->radius = 100;
	glow->r.origin[2] += TELEPORT_PAD_HEIGHT;
	AddEffect(owner, glow);
}
