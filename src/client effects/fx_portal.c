//
// Heretic II
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Client Entities.h"
#include "Particle.h"
#include "ResourceManager.h"
#include "FX.h"
#include "Random.h"
#include "Vector.h"
#include "string.h"
#include "Utilities.h"
#include "q_Sprite.h"
#include "g_playstats.h"


#define SINESCALE (256.0 / (2 * M_PI))
#define NUM_PORTAL_PARTS	20
#define PICTURE_RADIUS		55.0
#define PORTAL_RADIUS		50.0
#define	SCALE		0.01F
#define DELTA_SCALE	1.0F
#define MIN_PART_SCALE	1.0
#define MAX_PART_SCALE	3.0

#define	NUM_PORTAL_MODELS	3

static struct model_s *portal_models[NUM_PORTAL_MODELS];

void PreCachePortal()
{
	portal_models[0] = fxi.RegisterModel("sprites/fx/ripple_add.sp2");
	portal_models[1] = fxi.RegisterModel("sprites/fx/portal1.sp2");
	portal_models[2] = fxi.RegisterModel("sprites/spells/patball.sp2");
}

int RandomSparkTypes [9] =
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

qboolean FXMagicPortalThink(client_entity_t *self, centity_t *owner)
{
	client_particle_t	*ce;
	paletteRGBA_t		color;
	vec3_t				vel, right, up;
	int					i, part_type;
	int					count;
	client_entity_t		*ripple;
	client_entity_t		*line;

	if (self->LifeTime == 1)
	{	// already disabled.
		if (fxi.cl->time > self->lastThinkTime)
		{	// Remove it.
			return(false);
		}
		else
		{
			return(true);
		}
	}

	if(owner->current.effects&EF_DISABLE_EXTRA_FX || (self->SpawnDelay && self->SpawnDelay < fxi.cl->time))
	{	// Start disabling it, but give it a couple of seconds to fitz out.
		self->LifeTime = 1;
		self->lastThinkTime = fxi.cl->time + 2000;

		return(true);
	}

	AngleVectors(self->r.angles, NULL, right, up);
	count = GetScaledCount(NUM_PORTAL_PARTS, 0.7);
	for(i = 0; i < count; i++)
	{
		VectorScale(right, flrand(-1, 1), vel);
		VectorMA(vel, flrand(-1, 1), up, vel);
		if(Vec3IsZero(vel))
			vel[2] = 1.0;			// Safety in case flrand gens all zeros (VERY unlikely)
		VectorNormalize(vel);
		VectorScale(vel, PORTAL_RADIUS, vel);

		color.c = 0xffffffff;

		part_type = irand(0, 8);
		ce = ClientParticle_new(RandomSparkTypes[part_type], color, 3000);
		VectorCopy(vel, ce->origin);

		VectorScale(vel, flrand(0.0625, 0.125f), ce->velocity);
		ce->acceleration[2] = flrand(2, 5);
		ce->scale = flrand(MIN_PART_SCALE, MAX_PART_SCALE);
		ce->d_scale = -0.2F;
		
		if(!(self->flags & CEF_NO_DRAW))
		{
			ce->color.r = irand(100, 200);
			ce->color.g = irand(0, 50);
			ce->color.b = irand(100, 200);
		}
		ce->type |= PFL_ADDITIVE;

//		ce->color.a = irand(128, 255);
		ce->color.a = irand(2, 128);

//		ce->d_alpha = flrand(ce->color.a * -0.8f, -ce->color.a);
		ce->d_alpha = irand(320, 480);
		AddParticleToList(self, ce);
	}

	if(self->SpawnInfo == 2)
	{
		ripple = ClientEntity_new(FX_WATER_ENTRYSPLASH,
									 0,
									 owner->current.origin,
									 self->direction,
									 4000);

		ripple->r.model = portal_models;
		//ripple->r.flags |= RF_TRANS_ADD_ALPHA | RF_TRANS_ADD | RF_TRANSLUCENT | RF_FIXED | RF_TRANS_GHOST;
		ripple->r.flags |= RF_TRANS_ADD_ALPHA | RF_TRANS_ADD | RF_FIXED | RF_TRANS_GHOST;
		ripple->r.scale = 0.1f;
		ripple->d_scale = 1.0f;
		if(!(self->flags & CEF_NO_DRAW))
		{
			ripple->r.color.r = irand(10, 100);
			ripple->r.color.g = irand(0, 50);
			ripple->r.color.b = irand(10, 100);
			ripple->alpha = 0.40f;
			ripple->d_alpha = -0.20f;
		}
		else
		{
			ripple->alpha = 0.6f;
			ripple->d_alpha = -0.30f;
		}
		//FIXME: calculate correct duration

		AddEffect(owner, ripple); 

		self->SpawnInfo = 0;
	}
	else
		self->SpawnInfo++;

	if(!(self->flags & CEF_NO_DRAW))
	{
		self->alpha += flrand(-0.02, 0.02);

		if(1)
		{
			self->r.verts[0][0] += flrand(-1, 1);
			self->r.verts[0][1] += flrand(-1, 1);

			self->r.verts[1][0] += flrand(-1, 1);
			self->r.verts[1][1] += flrand(-1, 1);

			self->r.verts[2][0] += flrand(-1, 1);
			self->r.verts[2][1] += flrand(-1, 1);

			self->r.verts[3][0] += flrand(-1, 1);
			self->r.verts[3][1] += flrand(-1, 1);
		}

		if(1)
		{
			//FIXME: scroll based on viewangle to camera viewport - but what about alpha channel???
			self->r.verts[0][2] += flrand(-0.001, 0.001);
			self->r.verts[0][3] += flrand(-0.001, 0.001);

			self->r.verts[1][2] += flrand(-0.001, 0.001);
			self->r.verts[1][3] += flrand(-0.001, 0.001);

			self->r.verts[2][2] += flrand(-0.001, 0.001);
			self->r.verts[2][3] += flrand(-0.001, 0.001);

			self->r.verts[3][2] += flrand(-0.001, 0.001);
			self->r.verts[3][3] += flrand(-0.001, 0.001);
		}
	}

	//===================================================

	if(r_detail->value > DETAIL_HIGH)
	{//Spawn a hit explosion of lines
		i = GetScaledCount(16, 0.85);

		while (i--)
		{
			line=ClientEntity_new(FX_WEAPON_STAFF_STRIKE, 0, owner->current.origin, 0, 600);

			line->r.model = portal_models + 2;
			
			line->r.spriteType = SPRITE_LINE;

			line->r.flags |= RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
			line->r.color.c = 0xFFFFFFFF;
			line->r.scale = flrand(1.0, 2.5);
			line->alpha = flrand(1.0, 0.75);
			line->d_alpha = -2.0;
			line->d_scale = -1.0;

			line->r.color.r = irand(128, 255);
			line->r.color.g = irand(128, 255);
			line->r.color.b = 128 + irand(108, 127);
			line->r.color.a = 64 + irand(16, 128);

			VectorRandomCopy(self->direction, line->velocity, 1.25);

			VectorCopy(owner->current.origin, line->r.startpos);
			VectorMA(line->r.startpos, irand(16,48), line->velocity, line->r.endpos);

			VectorScale(line->velocity, irand(25,100), line->velocity);
			VectorSet(line->acceleration, line->velocity[0] * 0.1, line->velocity[1] * 0.1, 0);

			AddEffect(NULL, line);
		}
	}

	return(true);
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
