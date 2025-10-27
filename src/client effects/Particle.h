//
// Particle.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "q_Typedef.h"
#include "ParticleFlags.h" //mxd

typedef enum ParticleTypes_e
{
	// Small 4x4 particles in the upper left (0, 0).
	PART_4x4_WHITE,
	PART_4x4_BLUE,
	PART_4x4_RED,
	PART_4x4_GREEN,

	PART_4x4_CYAN,
	PART_4x4_YELLOW,
	PART_4x4_MAGENTA,
	PART_4x4_ORANGE,

	PART_4x4_BLUE2,
	PART_4x4_BLUE3,
	PART_4x4_UNUSED1,
	PART_4x4_UNUSED2,

	PART_4x4_BLOOD1,
	PART_4x4_BLOOD2,
	PART_4x4_GREENBLOOD1,
	PART_4x4_GREENBLOOD2,

	// Unused 16x16 section, to right of above 16x16 section (1, 0).
	PART_8x8_BUBBLE,
	PART_8x8_BLOOD,
	PART_8x8_GLOBBIT1,
	PART_8x8_GLOBBIT2,

	// Two sections, directly below the above two sections (0, 1) and (1, 1).
	PART_16x16_MIST,
	PART_16x16_GLOB,

	// Next 32x32 section, to the left of the above 32x32 section (2, 0), (3, 0), (2, 1) and (3, 1).
	PART_16x16_STAR,
	PART_16x16_WATERDROP,
	PART_16x16_LIGHTNING,
	PART_16x16_BLOOD,

	// Two 32x32 sections, below the above two 32x32 sections (0, 2) and (2, 2).
	PART_32x32_STEAM,
	PART_32x32_WFALL,

	// Three 32x32 sections, in the upper right 1/4 of the page (4, 0) (6, 0) (4, 2).
	PART_32x32_FIRE0,
	PART_32x32_FIRE1,
	PART_32x32_FIRE2,

	// The single remaining 32x32 section, in the lower right of the above one (6, 2).
	PART_16x16_SPARK_B,
	PART_16x16_SPARK_R,
	PART_16x16_SPARK_G,
	PART_16x16_SPARK_Y,

	// The third 32x32 section down, on the left (0, 4).
	PART_32x32_FIREBALL,
	PART_32x32_BLACKSMOKE, // The same spot, the fireball is additive, the smoke is not.

	// The next 32x32 section to the right of the above cell (2, 4), (3, 4), (2, 5) and (3, 5).
	PART_16x16_SPARK_C,

	// These are little 8x8 critters.
	PART_8x8_RED_X,
	PART_8x8_RED_CIRCLE,
	PART_8x8_GREEN_X,
	PART_8x8_GREEN_CIRCLE,

	PART_8x8_BLUE_X,
	PART_8x8_BLUE_CIRCLE,
	PART_8x8_CYAN_X,
	PART_8x8_CYAN_CIRCLE,

	PART_8x8_RED_DIAMOND,
	PART_8x8_GREEN_DIAMOND,
	PART_8x8_BLUE_DIAMOND,
	PART_8x8_CYAN_DIAMOND,

	// Bottom two 32x32 sections, lower left (0, 6) and (2, 6).
	PART_32x32_BUBBLE,
	PART_32x32_ALPHA_GLOBE,

	// Upper left of lower right quadrant (4, 4).
	PART_16x16_SPARK_I,
	PART_16x16_BLUE_PUFF,
	PART_16x16_UNUSED,
	PART_16x16_ORANGE_PUFF,

	// Two 32x32's in lower right quadrant (6, 4) and (4, 6).
	PART_32x32_BLOOD,
	PART_32x32_GREENBLOOD,

	// Remaining four 16x16's in lower right quadrant (6, 6), (6, 7), (7, 6) and (7, 7).
	PART_16x16_FIRE1,
	PART_16x16_FIRE2,
	PART_16x16_FIRE3,
	PART_16x16_GREENBLOOD,

	PART_MAX // Total = 62.
} ParticleTypes_t;

typedef struct client_particle_s
{
	struct client_particle_s* next;	// Next client particle, if any.

	vec3_t origin;
	vec3_t velocity;
	vec3_t acceleration;

	paletteRGBA_t color;
	float d_alpha;

	float scale;
	float d_scale;

	int type;

	int startTime;	// Milliseconds.
	int duration;	// Milliseconds.
} client_particle_t;

#define PARTICLE_GRAVITY			80
#define PARTICLE_TRAIL_THINK_TIME	100	// Spawns a particle puff 10 times per sec.

// These are used in the vectors for origin, velocity and acceleration in particles that use Cylindrical coordinates.
#define CYL_RADIUS		0
#define CYL_YAW			1
#define CYL_Z			2

// These are used in the vectors for origin, velocity and acceleration in particles that use Spherical coordinates.
#define SPH_RADIUS		0
#define SPH_YAW			1
#define SPH_PITCH		2

extern int ParticleUpdateTime; //mxd

extern void InitParticleMngr(void);
extern void ReleaseParticleMngr(void);

extern void AddParticleToList(struct client_entity_s* ce, client_particle_t* fx);
extern void RemoveParticleList(client_particle_t** root);
extern int AddParticlesToView(struct client_entity_s* ce);
extern int UpdateParticles(struct client_entity_s* ce);
extern void FreeParticles(struct client_entity_s* ce);
extern client_particle_t* ClientParticle_new(int type, paletteRGBA_t color, int duration);