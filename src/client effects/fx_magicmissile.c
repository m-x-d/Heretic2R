//
// fx_magicmissile.c
//
// Heretic II
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Client Entities.h"
#include "Particle.h"
#include "ResourceManager.h"
#include "FX.h"
#include "Vector.h"
#include "ce_DLight.h"
#include "Angles.h"
#include "Random.h"
#include "q_Sprite.h"
#include "Utilities.h"
#include "g_playstats.h"

static qboolean FXMagicMissileTrailElementThink(struct client_entity_s *Self,centity_t *Owner);
static qboolean FXMagicMissileTrailThink(struct client_entity_s *Self,centity_t *Owner);
static qboolean FXMagicMissileModelThink1(struct client_entity_s *Self,centity_t *Owner);
static qboolean FXMagicMissileModelThink2(struct client_entity_s *Self,centity_t *Owner);
static qboolean FXMagicMissileSmokePuffThink(struct client_entity_s *Self,centity_t *Owner);

#define	NUM_MISSILE_MODELS	3

static struct model_s *array_models[NUM_MISSILE_MODELS];

void PreCacheArray()
{
	array_models[0] = fxi.RegisterModel("sprites/spells/halo_ind.sp2");
	array_models[1] = fxi.RegisterModel("Sprites/spells/spark_ind.sp2");
	array_models[2] = fxi.RegisterModel("Sprites/spells/indigostreak.sp2");
}


// ************************************************************************************************
// FXMagicMissileTrailThink
// ------------------------
// ************************************************************************************************

// These need to be converted to particles

#define ARRAY_TRAIL_SCALE	0.3
#define ARRAY_TRAIL_COUNT	3
#define TRAIL_SPEED			32.0

static qboolean FXMagicMissileTrailThink(struct client_entity_s *Self,centity_t *Owner)
{
	int				i;
	client_entity_t	*trail;
	vec3_t			NormVelocity;

	VectorCopy(Self->velocity,NormVelocity);
	VectorNormalize(NormVelocity);

//	count = GetScaledCount(ARRAY_TRAIL_COUNT, 0.8);
//	for(i = 0; i < count; i++)
	for (i=0; i<2; i++)	// Each cardinal direction
	{
		trail = ClientEntity_new(FX_WEAPON_MAGICMISSILE,
								   0,
								   Self->r.origin,
								   NULL,
    							   500);

		// Self->up holds the direction to move in.

		VectorMA(trail->r.origin, -i * 3.0 - flrand(0.0F, 3.0F), NormVelocity, trail->r.origin);
		switch(i)
		{
		case 0:	// Up/right
			VectorCopy(Self->up, trail->velocity);
			break;
		case 1:	// Down/left
			VectorScale(Self->up, -1.0, trail->velocity);
			break;
		}

		trail->r.model = array_models;
		trail->r.flags = RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		trail->r.scale = flrand(ARRAY_TRAIL_SCALE, ARRAY_TRAIL_SCALE + 0.1);
		trail->d_scale = -1.0;
		trail->d_alpha = -2.0;
		trail->radius = 20.0;
		
		AddEffect(NULL,trail);
	}

	trail = ClientEntity_new(FX_WEAPON_MAGICMISSILE, 
								CEF_AUTO_ORIGIN | CEF_USE_VELOCITY2,
								Self->r.origin, 
								NULL, 
								500);

	trail->r.model = array_models + 2;
	trail->r.spriteType = SPRITE_LINE;
	trail->r.flags = RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	VectorCopy(trail->r.origin, trail->r.startpos);
	VectorMA(trail->r.origin, -0.1, Self->velocity, trail->r.endpos);
	VectorScale(Self->velocity, 0.9, trail->velocity);
	VectorScale(Self->velocity, 0.5, trail->velocity2);
	trail->r.scale = 12.0;
	trail->d_scale = -24.0;
	trail->d_alpha = -2.0;

	AddEffect(NULL, trail);

	return(true);
}

// ************************************************************************************************
// FXMagicMissileModelThink1
// -------------------------
// ************************************************************************************************

static qboolean FXMagicMissileModelThink1(struct client_entity_s *Self,centity_t *Owner)
{
	Self->d_scale=0.0;
	Self->r.scale = 0.8;

	Self->Update=FXMagicMissileModelThink2;

	FXMagicMissileTrailThink(Self,Owner);
	return(true);
}

// ************************************************************************************************
// FXMagicMissileModelThink2
// -------------------------
// ************************************************************************************************

static qboolean FXMagicMissileModelThink2(struct client_entity_s *Self,centity_t *Owner)
{
	FXMagicMissileTrailThink(Self,Owner);
	return(true);
}

// ************************************************************************************************
// FXMagicMissile
// --------------
// ************************************************************************************************

void FXMagicMissile(centity_t *Owner,int Type,int Flags,vec3_t Origin)
{
	vec3_t			ang;
	client_entity_t	*Missile;	
	paletteRGBA_t	LightColor={128,64,96,255};
	vec3_t			fwd, up, right;
	short			shortyaw, shortpitch;

	fxi.GetEffect(Owner,Flags,clientEffectSpawners[FX_WEAPON_MAGICMISSILE].formatString, &shortyaw, &shortpitch);

	ang[YAW]=(float)shortyaw * (360.0/65536.0);
	ang[PITCH]=(float)shortpitch * (360.0/65536.0);
	ang[ROLL]=0.0;

	AngleVectors(ang, fwd, right, up);

	// Add the magic-missile model.
	Missile=ClientEntity_new(Type, Flags | CEF_DONT_LINK, Origin, NULL, 100);
	
	Missile->r.model = array_models;
	Missile->r.frame = 1;
	if (Flags & CEF_FLAG6)
		VectorScale(fwd, MAGICMISSILE_SPEED/2, Missile->velocity);
	else
		VectorScale(fwd, MAGICMISSILE_SPEED, Missile->velocity);
	VectorCopy(ang, Missile->r.angles);

	// Set up the direction we want the trail to fly from the missile.
	VectorMA(up, 2.0, right, Missile->up);
	VectorScale(Missile->up, TRAIL_SPEED, Missile->up);

	Missile->r.flags |= RF_TRANSLUCENT | RF_TRANS_ADD;
	Missile->r.scale=0.4;
	Missile->alpha=1.0;
	Missile->d_alpha=0.0;
	Missile->d_scale=4.0;
	Missile->dlight=CE_DLight_new(LightColor,150.0f,0.0f);
	Missile->Update=FXMagicMissileModelThink1;

//	FXMagicMissileTrailThink(Self, Owner);

	AddEffect(Owner, Missile);
}

// ************************************************************************************************
// FXMagicMissileExplode
// ************************************************************************************************

#define NUM_ARRAY_EXPLODE_PARTS		12
#define ARRAY_SCALE					1.0
#define ARRAY_EXPLODE_SPEED			200
void FXMagicMissileExplode(centity_t *owner, int type, int flags, vec3_t origin)
{
	vec3_t			dir;
	client_entity_t	*smokepuff;
	int				i;
	paletteRGBA_t	lightcolor = {0, 128, 128, 255};

	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_WEAPON_MAGICMISSILEEXPLODE].formatString, dir);
	if(flags & CEF_FLAG6)
	{
		FXClientScorchmark(origin, dir);
	}
	Vec3ScaleAssign(32.0, dir);

	for(i = 0; i < NUM_ARRAY_EXPLODE_PARTS; i++)
	{
		smokepuff = ClientEntity_new(type, flags, origin, 0, 500);

		smokepuff->r.model = array_models + 1;
		smokepuff->r.scale = flrand(ARRAY_SCALE * 0.75, ARRAY_SCALE * 1.5);
		smokepuff->r.flags = RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;

		VectorRandomCopy(dir, smokepuff->velocity, ARRAY_EXPLODE_SPEED);
		smokepuff->acceleration[2] = GetGravity()*0.3;

		smokepuff->d_scale = -1.0;
		smokepuff->d_alpha = -2.0;
		smokepuff->radius = 20.0;

		AddEffect(NULL, smokepuff);
	}

	// Big flash
	smokepuff = ClientEntity_new(type, flags, origin, 0, 500);

	smokepuff->r.model = array_models;
	smokepuff->r.frame = 0;

	smokepuff->r.scale = 2.0;
	smokepuff->r.flags = RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;

	VectorScale(dir, 8.0, smokepuff->velocity);

	smokepuff->d_scale = -6.0;
	smokepuff->d_alpha = -2.0;
	smokepuff->radius = 20.0;

	smokepuff->dlight = CE_DLight_new(lightcolor, 150.0, -50.0);

	AddEffect(NULL, smokepuff);
}

#define BLAST_DIFF 40.0
#define BLAST_SCALE	0.3
#define BLAST_BACKSPEED	-1.5
#define BLAST_GRAVITY -32.0

// Create Effect FX_WEAPON_BLAST
void FXBlast(centity_t *owner, int type, int flags, vec3_t origin)
{
	vec3_t endpos, curpos;
	vec3_t unit, back;
	int i, numpuffs;
	client_entity_t *puff;
	client_particle_t *spark;
	paletteRGBA_t pal;
	float length, scale;
	short slength[BLAST_NUM_SHOTS], syaw, spitch;
	int		shot;
	vec3_t	angles;

	assert(BLAST_NUM_SHOTS==5);

	// Sends over the network 7 shorts and an origin.
	// Note that this is a vast improvement over five seperate effects with a vector each (60 bytes+origins)
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_WEAPON_BLAST].formatString, &syaw, &spitch, &slength[0], &slength[1], &slength[2], &slength[3], &slength[4]);

	// Compress the angles into two shorts.
	angles[YAW] = (float)syaw*(360.0/65536.0);
	angles[PITCH] = (float)spitch*(360.0/65536.0);
	angles[ROLL] = 0.0;

	// Set up for array.
	angles[YAW] -= BLAST_ANGLE_INC * (BLAST_NUM_SHOTS-1) * 0.5;
	for (shot=0; shot<BLAST_NUM_SHOTS; shot++)
	{
		AngleVectors(angles, unit, NULL, NULL);
		length = (float)slength[shot];
		VectorMA(origin, length, unit, endpos);

		numpuffs = (int)(length/BLAST_DIFF);
		VectorCopy(origin, curpos);
		VectorScale(unit, BLAST_DIFF, unit);
		VectorScale(unit, BLAST_BACKSPEED, back);
		scale = BLAST_SCALE;
		if (numpuffs>40)
			numpuffs=40;
		for(i=0; i<=numpuffs; i++)
		{
			puff = ClientEntity_new(type, flags | CEF_ADDITIVE_PARTS, curpos, NULL, 750);
			puff->r.model = array_models + 1;
			puff->r.flags |= RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
			puff->r.scale = scale;
			puff->radius = 14.0;
			puff->alpha = 0.95;
//			puff->d_alpha = -1.0;
			puff->d_scale = -1.4*scale;
			VectorSet(puff->velocity, flrand(-8.0,8.0), flrand(-8.0,8.0), flrand(-8.0,8.0));
			VectorAdd(puff->velocity, back, puff->velocity);
			puff->acceleration[2] = BLAST_GRAVITY;
			AddEffect(NULL, puff);
			VectorAdd(curpos, unit, curpos);
			scale+=0.1;
		}

		// We added the line, now throw out the impact
		// Big flash first...
		puff = ClientEntity_new(type, CEF_ADDITIVE_PARTS, endpos, NULL, 1000);
		puff->r.model = array_models;
		puff->r.frame = 0;
		puff->r.flags |= RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		puff->r.scale = 1.0 + length * 0.001;		// Bigger when further out.
		puff->alpha = 0.95;
		puff->d_scale = -5.0;
		puff->d_alpha = -1.0;
		puff->radius = 16.0;
		VectorCopy(back, puff->velocity);

		AddEffect(NULL, puff);

		pal.c = 0xffffffff;
		for (i=0; i<9; i++)
		{
			spark = ClientParticle_new(PART_16x16_SPARK_I, pal, 750);
			VectorSet(spark->velocity, flrand(-64.0, 64.0), flrand(-64.0, 64.0), flrand(64.0, 128.0));
			spark->acceleration[2] = -PARTICLE_GRAVITY*3.0;
			spark->scale = 16.0;
			spark->d_scale = -16.0*1.4;
			AddParticleToList(puff, spark);
		}

		angles[YAW] += BLAST_ANGLE_INC;
	}
}
 