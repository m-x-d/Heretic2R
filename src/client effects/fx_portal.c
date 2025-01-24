//
// fx_portal.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Particle.h"
#include "Random.h"
#include "Vector.h"
#include "Utilities.h"
#include "q_Sprite.h"
#include "g_playstats.h"

#define NUM_PORTAL_PARTS	20
#define PICTURE_RADIUS		55.0f
#define PORTAL_RADIUS		50.0f
#define MIN_PARTICLE_SCALE	1.0f
#define MAX_PARTICLE_SCALE	3.0f

static struct model_s* portal_models[3];

void PreCachePortal(void)
{
	portal_models[0] = fxi.RegisterModel("sprites/fx/ripple_add.sp2");
	portal_models[1] = fxi.RegisterModel("sprites/fx/portal1.sp2");
	portal_models[2] = fxi.RegisterModel("sprites/spells/patball.sp2");
}

#define NUM_SPARK_TYPES		(sizeof(random_spark_types) / sizeof(random_spark_types[0]) - 1) //mxd

static int random_spark_types[9] =
{
	PART_4x4_WHITE,
	PART_4x4_BLUE,
	PART_4x4_BLUE2,
	PART_4x4_BLUE3,
	PART_8x8_BLUE_X,
	PART_8x8_BLUE_CIRCLE,
	PART_8x8_BLUE_DIAMOND,
	PART_16x16_STAR,
	PART_16x16_SPARK_B,
};

static qboolean FXMagicPortalThink(client_entity_t* self, centity_t* owner)
{
	// Already disabled?
	if (self->LifeTime == 1)
		return fxi.cl->time <= self->lastThinkTime;

	if ((owner->current.effects & EF_DISABLE_EXTRA_FX) || (self->SpawnDelay > 0 && self->SpawnDelay < fxi.cl->time))
	{
		// Start disabling it, but give it a couple of seconds to fitz out.
		self->LifeTime = 1;
		self->lastThinkTime = fxi.cl->time + 2000;

		return true;
	}

	vec3_t right;
	vec3_t up;
	AngleVectors(self->r.angles, NULL, right, up);

	const int count = GetScaledCount(NUM_PORTAL_PARTS, 0.7f);

	for (int i = 0; i < count; i++)
	{
		vec3_t vel;
		VectorScale(right, flrand(-1.0f, 1.0f), vel);
		VectorMA(vel, flrand(-1.0f, 1.0f), up, vel);

		if (Vec3IsZero(vel))
			vel[2] = 1.0f; // Safety in case flrand gens all zeros (VERY unlikely).

		VectorNormalize(vel);
		VectorScale(vel, PORTAL_RADIUS, vel);

		client_particle_t* ce = ClientParticle_new(random_spark_types[irand(0, NUM_SPARK_TYPES)], color_white, 3000);

		ce->type |= PFL_ADDITIVE;
		VectorCopy(vel, ce->origin);
		VectorScale(vel, flrand(0.0625f, 0.125f), ce->velocity);
		ce->acceleration[2] = flrand(2.0f, 5.0f);
		ce->scale = flrand(MIN_PARTICLE_SCALE, MAX_PARTICLE_SCALE);
		ce->d_scale = -0.2f;
		ce->d_alpha = flrand(320.0f, 480.0f); //mxd. Was irand() in original version.

		if (!(self->flags & CEF_NO_DRAW))
			COLOUR_SET(ce->color, irand(100, 200), irand(0, 50), irand(100, 200)); //mxd. Use macro.

		ce->color.a = (byte)irand(2, 128);

		AddParticleToList(self, ce);
	}

	if (self->SpawnInfo == 2)
	{
		self->SpawnInfo = 0;

		client_entity_t* ripple = ClientEntity_new(FX_WATER_ENTRYSPLASH, 0, owner->current.origin, self->direction, 4000);

		ripple->r.model = &portal_models[0]; // ripple_add sprite.
		ripple->r.flags = RF_TRANS_ADD_ALPHA | RF_TRANS_ADD | RF_FIXED | RF_TRANS_GHOST;
		ripple->r.scale = 0.1f;
		ripple->d_scale = 1.0f;

		if (!(self->flags & CEF_NO_DRAW))
		{
			ripple->alpha = 0.4f;
			ripple->d_alpha = -0.2f;
			COLOUR_SET(ripple->r.color, irand(10, 100), irand(0, 50), irand(10, 100)); //mxd. Use macro.
		}
		else
		{
			ripple->alpha = 0.6f;
			ripple->d_alpha = -0.3f;
		}
		//FIXME: calculate correct duration

		AddEffect(owner, ripple);
	}
	else
	{
		self->SpawnInfo++;
	}

	if (!(self->flags & CEF_NO_DRAW))
	{
		self->alpha += flrand(-0.02f, 0.02f);

		for (int i = 0; i < 4; i++)
		{
			self->r.verts[i][0] += flrand(-1.0f, 1.0f);
			self->r.verts[i][1] += flrand(-1.0f, 1.0f);

			//FIXME: scroll based on viewangle to camera viewport - but what about alpha channel???
			self->r.verts[i][2] += flrand(-0.001f, 0.001f);
			self->r.verts[i][3] += flrand(-0.001f, 0.001f);
		}
	}

	// Spawn a hit explosion of lines.
	if ((int)r_detail->value >= DETAIL_HIGH) //mxd. DETAIL_UBERHIGH only in original version.
	{
		const int num_lines = GetScaledCount(16, 0.85f);

		for (int i = 0; i < num_lines; i++)
		{
			client_entity_t* line = ClientEntity_new(FX_WEAPON_STAFF_STRIKE, 0, owner->current.origin, NULL, 600);

			line->r.model = &portal_models[2]; // patball sprite.
			line->r.spriteType = SPRITE_LINE;

			line->r.flags = RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
			line->r.scale = flrand(1.0f, 2.5f);
			line->alpha = flrand(0.75f, 1.0f);
			line->d_alpha = -2.0f;
			line->d_scale = -1.0f;
			COLOUR_SETA(line->r.color, irand(128, 255), irand(128, 255), irand(236, 255), irand(80, 192)); //mxd. Use macro.

			VectorRandomCopy(self->direction, line->velocity, 1.25f);

			VectorCopy(owner->current.origin, line->r.startpos);
			VectorMA(line->r.startpos, flrand(16.0f, 48.0f), line->velocity, line->r.endpos); //mxd. Used irand() in original logic.

			VectorScale(line->velocity, flrand(25.0f, 100.0f), line->velocity); //mxd. Used irand() in original logic.
			VectorSet(line->acceleration, line->velocity[0] * 0.1f, line->velocity[1] * 0.1f, 0.0f);

			AddEffect(NULL, line);
		}
	}

	return true;
}

// This is the persistant effect for the teleport pad
void FXMagicPortal(centity_t *owner, int type, int flags, vec3_t origin)
{
	client_entity_t		*portal;
	vec3_t				dir, forward;
	byte				color, duration;
	qboolean			special = false;

	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_MAGIC_PORTAL].formatString, dir, &color, &duration);

	AngleVectors(dir, forward, NULL, NULL);
	portal = ClientEntity_new(type, (flags) & ~CEF_NOMOVE , origin, forward, 110);
	VectorScale(forward, -1, portal->direction);
	portal->radius = 1000;

	if(special)
	{
		portal->flags |= CEF_ADDITIVE_PARTS | CEF_VIEWSTATUSCHANGED;
		portal->r.spriteType = SPRITE_DYNAMIC;
		portal->r.flags |= RF_FIXED|RF_GLOW|RF_TRANSLUCENT|RF_TRANS_ADD_ALPHA|RF_TRANS_ADD;
		portal->alpha = 0.75;
		
		if(1)
		{
			//top left
			portal->r.verts[0][0] = -PICTURE_RADIUS;// + flrand(-2, 2);//x
			portal->r.verts[0][1] = -PICTURE_RADIUS;// + flrand(-2, 2);//y
			//top right
			portal->r.verts[1][0] = PICTURE_RADIUS;// + flrand(-2, 2);//x
			portal->r.verts[1][1] = -PICTURE_RADIUS;// + flrand(-2, 2);//y
			//bottom left
			portal->r.verts[2][0] = PICTURE_RADIUS;// + flrand(-2, 2);//x
			portal->r.verts[2][1] = PICTURE_RADIUS;//+ flrand(-2, 2);//y
			//bottom right
			portal->r.verts[3][0] = -PICTURE_RADIUS;// + flrand(-2, 2);//x
			portal->r.verts[3][1] = PICTURE_RADIUS;// + flrand(-2, 2);//y
		}

		if(1)
		{
			//top left
			portal->r.verts[0][2] = 0;//PORTALPIC_SIZE;// + flrand(-2, 2);//u
			portal->r.verts[0][3] = 1;//-PORTALPIC_SIZE;// + flrand(-2, 2);//v
			//top right
			portal->r.verts[1][2] = 1;//PORTALPIC_SIZE;// + flrand(-2, 2);//u
			portal->r.verts[1][3] = 1;//PORTALPIC_SIZE;// + flrand(-2, 2);//v
			//bottom left
			portal->r.verts[2][2] = 1;//-PORTALPIC_SIZE;// + flrand(-2, 2);//u
			portal->r.verts[2][3] = 0;//PORTALPIC_SIZE;// + flrand(-2, 2);//v
			//bottom right
			portal->r.verts[3][2] = 0;//-PORTALPIC_SIZE;// + flrand(-2, 2);//u
			portal->r.verts[3][3] = 0;//-PORTALPIC_SIZE;// + flrand(-2, 2);//v
		}

		portal->r.model = portal_models + 1;
	}
	else
		portal->flags |= CEF_NO_DRAW;

	VectorCopy(dir, portal->r.angles);
	portal->SpawnInfo = 2;
	portal->Update = FXMagicPortalThink;

	portal->flags |= CEF_PULSE_ALPHA;

	if (duration==0)
	{	// Portal stays indefinitely
		portal->SpawnDelay = 0;
	}
	else
	{	// Portal disappears after so many seconds
		portal->SpawnDelay = fxi.cl->time + ((int)duration * 1000);
	}
	AddEffect(owner, portal);
}
