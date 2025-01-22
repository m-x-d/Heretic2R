//
// fx_Mork.c
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
#include "fx_flamethrow.h" //mxd
#include "fx_HighPriestessProjectiles.h" //mxd
#include "random.h"
#include "q_Sprite.h"
#include "Utilities.h"
#include "Reference.h"
#include "Matrix.h"
#include "g_playstats.h"

#define	NUM_M_MISSILE_MODELS	4
#define NUM_M_SHIELD_MODELS 3
#define NUM_M_LIGHTNING_MODELS 3
#define	NUM_M_PP_MODELS	6
#define	NUM_IMP_MODELS	3
#define NUM_CW_MODELS 2
#define NUM_BUOY_MODELS 1

static struct model_s *Morkproj_models[NUM_M_MISSILE_MODELS];
static struct model_s *Morkshield_models[NUM_M_SHIELD_MODELS];
static struct model_s *Morklightning_models[NUM_M_LIGHTNING_MODELS];
static struct model_s *Morkpp_models[NUM_M_PP_MODELS];
static struct model_s *Imp_models[NUM_IMP_MODELS];
static struct model_s *CW_models[NUM_CW_MODELS];
static struct model_s *buoy_models[NUM_BUOY_MODELS];
static struct model_s *mork_model[1];
static struct model_s *ass_dagger_model[1];

static struct model_s *morc_models[6];
static struct model_s *mssithra_models[6];

void PreCacheMEffects()
{
	Morkproj_models[0] = fxi.RegisterModel("sprites/fx/hpproj1_1.sp2");
	Morkproj_models[1] = fxi.RegisterModel("sprites/fx/hpproj1_2.sp2");
	Morkproj_models[2] = fxi.RegisterModel("sprites/fx/segment_trail_wt.sp2");
	Morkproj_models[3] = fxi.RegisterModel("sprites/lens/halo2.sp2");

	Morklightning_models[0] = fxi.RegisterModel("sprites/fx/lightning.sp2");
	Morklightning_models[1] = fxi.RegisterModel("sprites/fx/rlightning.sp2");
	Morklightning_models[2] = fxi.RegisterModel("sprites/fx/neon.sp2");

	Morkpp_models[0] = fxi.RegisterModel("sprites/fx/steam.sp2");
	Morkpp_models[1] = fxi.RegisterModel("models/spells/phoenixarrow/tris.fm");
	Morkpp_models[2] = fxi.RegisterModel("sprites/fx/halo.sp2");
	Morkpp_models[3] = fxi.RegisterModel("sprites/fx/core_b.sp2");//spells/phoenix.sp2");
	Morkpp_models[4] = fxi.RegisterModel("models/fx/explosion/inner/tris.fm");
	Morkpp_models[5] = fxi.RegisterModel("models/fx/explosion/outer/tris.fm");

	Imp_models[0] = fxi.RegisterModel("sprites/fx/halo.sp2");
	Imp_models[1] = fxi.RegisterModel("sprites/fx/fire.sp2");
	Imp_models[2] = fxi.RegisterModel("sprites/fx/halo.sp2");

	CW_models[0] = fxi.RegisterModel("sprites/spells/patball.sp2");
	CW_models[1] = fxi.RegisterModel("sprites/fx/waterentryripple.sp2");

	buoy_models[0] = fxi.RegisterModel("sprites/fx/segment_trail_buoy.sp2");

	mork_model[0] = fxi.RegisterModel("models/monsters/morcalavin/tris.fm");
	ass_dagger_model[0] = fxi.RegisterModel("models/monsters/assassin/dagger/tris.fm");

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

enum {
//offensive
	FX_M_BEAM,
//impacts
	FX_M_MISC_EXPLODE,
//other
	FX_IMP_FIRE,
	FX_IMP_FBEXPL,
	FX_CW_STARS,
	FX_BUOY,
	FX_BUOY_PATH,
	FX_M_MOBLUR,
	FX_ASS_DAGGER,
	FX_UNDER_WATER_WAKE,

//jweier
	FX_QUAKE_RING,
	FX_GROUND_ATTACK,
	FX_MORK_BEAM,
	FX_MORK_MISSILE,
	FX_MORK_MISSILE_HIT,
	FX_MORK_TRACKING_MISSILE,

	FX_MSSITHRA_EXPLODE,
	FX_MSSITHRA_ARROW,
	FX_MSSITHRA_ARROW_CHARGE,
};

static qboolean FXMorkTrailThink_old(struct client_entity_s *self,centity_t *owner)
{
	if (self->alpha <= 0.1 || self->r.scale <= 0.0)
		return false;

	self->r.scale -= 0.1;
	
	return true;
}

static qboolean FXCWTrailThink(struct client_entity_s *self,centity_t *owner)
{
	vec3_t forward;
	if (self->alpha <= 0.1 || self->r.scale <= 0.0)
		return false;

	self->r.scale -= 0.15;
	
	VectorCopy(owner->lerp_origin, self->r.origin);

	VectorCopy(self->r.origin, self->r.startpos);

	Vec3AddAssign(self->up, self->direction);

	AngleVectors(self->direction, forward, NULL, NULL);

	VectorMA(self->r.startpos, self->SpawnInfo, forward, self->r.endpos);
	return true;
}


static qboolean FXMorkTrailThink2(struct client_entity_s *self,centity_t *owner)
{
	if (self->alpha <= 0.1 || self->r.scale <= 0.0)
		return false;

	self->r.scale -= 0.15;
	
	return true;
}

void FXMorkMissileExplode(struct client_entity_s *self,centity_t *owner, vec3_t dir)
{
	client_entity_t	*SmokePuff;
	int				i;
	paletteRGBA_t	LightColor={255,64,32,255};
	byte			powerup = 0;
	
	Vec3ScaleAssign(32.0, dir);

	i = GetScaledCount(irand(12,16), 0.8);
	
	while(i--)
	{
		if (!i)
			SmokePuff=ClientEntity_new(FX_M_EFFECTS,0,owner->origin,NULL,500);
		else
			SmokePuff=ClientEntity_new(FX_M_EFFECTS,0,owner->origin,NULL,1500);
		
		SmokePuff->r.model = Morkproj_models + 1;
		SmokePuff->r.scale=flrand(0.5,1.0);
		SmokePuff->d_scale=-2.0;

		SmokePuff->r.flags |=RF_FULLBRIGHT|RF_TRANSLUCENT|RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		SmokePuff->r.frame = 0;

		VectorRandomCopy(dir, SmokePuff->velocity, flrand(16.0, 64.0));

		SmokePuff->acceleration[0] = flrand(-400, 400);
		SmokePuff->acceleration[1] = flrand(-400, 400);
		SmokePuff->acceleration[2] = flrand(-40, -60);

		SmokePuff->d_alpha= -0.4;
			
		SmokePuff->radius=20.0;

		AddEffect(NULL,SmokePuff);
	}
}

#define M_LIGHTNING_WIDTH	6.0
#define M_LIGHTNING_WIDTH2	8.0

client_entity_t *MorkMakeLightningPiece(vec3_t start, vec3_t end, float radius, int lifetime, qboolean plasma)
{
	client_entity_t *lightning;
	vec3_t	vec;
	float	dist, tile_num;

	VectorSubtract(end, start, vec);
	dist = VectorNormalize(vec);
	tile_num = dist/32;

	lightning = ClientEntity_new(-1, CEF_DONT_LINK, start, NULL, lifetime);
	if(plasma)
	{
		lightning->r.model = Morklightning_models + 2;
		lightning->r.frame = 2;
		lightning->alpha = 2.55;
	}
	else
	{
		lightning->r.model = Morklightning_models;
		lightning->alpha = 0.95;
	}
	lightning->r.flags |= RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	lightning->r.scale = M_LIGHTNING_WIDTH;
	lightning->r.tile = tile_num;
	lightning->radius = radius;
	lightning->d_alpha = -4.0;
	VectorCopy(start, lightning->r.startpos);
	VectorCopy(end, lightning->r.endpos);
	lightning->r.spriteType = SPRITE_LINE;
	AddEffect(NULL, lightning); 
	if(plasma)
		return(lightning);

	lightning = ClientEntity_new(-1, CEF_DONT_LINK, start, NULL, lifetime * 2);
	lightning->r.model = Morklightning_models + 1;
	lightning->r.frame = irand(0, 1);
	lightning->r.flags |= RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	lightning->r.scale = M_LIGHTNING_WIDTH2;
	lightning->radius = radius;
	lightning->r.tile = 1;
	lightning->alpha = 0.5;
	lightning->d_alpha = -1.250;
	VectorCopy(start, lightning->r.startpos);
	VectorCopy(end, lightning->r.endpos);
	lightning->r.spriteType = SPRITE_LINE;
	AddEffect(NULL, lightning); 

	return(lightning);
}

static qboolean FXMorkBeamCircle (struct client_entity_s *self,centity_t *owner)
{
	vec3_t	angles, up;

	self->LifeTime+=54;

	VectorSet(angles, self->r.angles[PITCH], self->r.angles[YAW], anglemod(self->LifeTime));
	AngleVectors(angles, NULL, NULL, up);
	VectorMA(owner->current.origin, 12, up, self->r.origin);
	
	MorkMakeLightningPiece(self->startpos, self->r.origin, 2000, 1000, false);

	VectorCopy(self->r.origin, self->startpos);

	return true;
}

static qboolean FXMorkBeam (struct client_entity_s *self,centity_t *owner)
{
	client_entity_t		*TrailEnt;
	int					numparts, parttype;
	client_particle_t	*spark;
	int					i;
	//VectorCopy(owner->origin, self->origin);
//Make inner beam
	TrailEnt=ClientEntity_new(FX_M_EFFECTS,
							  CEF_DONT_LINK,
							  owner->origin,
							  NULL,
							  17);

	TrailEnt->radius = 2000;

	VectorCopy( owner->origin, TrailEnt->origin );

	TrailEnt->r.flags |= RF_TRANSLUCENT | RF_TRANS_ADD_ALPHA;
	TrailEnt->r.model = Morkproj_models + 2;

	TrailEnt->r.spriteType = SPRITE_LINE;
	TrailEnt->r.tile = 1;
	TrailEnt->alpha = 2.0;
	TrailEnt->r.scale = 1.0;

	VectorCopy( self->startpos, TrailEnt->r.startpos );
	VectorCopy( owner->origin , TrailEnt->r.endpos );

	TrailEnt->d_alpha = -1.0;
	TrailEnt->d_scale = -0.1;
	TrailEnt->Update = FXMorkTrailThink_old;
	
	AddEffect(NULL,TrailEnt);

//make outer beam
	TrailEnt=ClientEntity_new(FX_M_EFFECTS,
							  CEF_DONT_LINK,
							  owner->origin,
							  NULL,
							  17);

	TrailEnt->radius = 2000;

	VectorCopy( owner->origin, TrailEnt->origin );

	TrailEnt->r.flags |= RF_TRANSLUCENT | RF_TRANS_ADD_ALPHA;
	TrailEnt->r.model = Morkproj_models + 2;

	TrailEnt->r.spriteType = SPRITE_LINE;
	TrailEnt->r.tile = 1;
	TrailEnt->r.scale = 16;
	TrailEnt->alpha = 1.0;
	TrailEnt->r.scale = 4.0;
	TrailEnt->r.color.r = 100;
	TrailEnt->r.color.g = 75;
	TrailEnt->r.color.b = 250;

	VectorCopy( self->startpos, TrailEnt->r.startpos );
	VectorCopy( owner->origin , TrailEnt->r.endpos );

	TrailEnt->d_alpha = -0.6;
	TrailEnt->d_scale = -0.5;

	TrailEnt->Update = FXMorkTrailThink_old;
	
	AddEffect(NULL,TrailEnt);

	VectorCopy(owner->origin, self->startpos);

	numparts = floor(irand(6, 9)*self->r.scale);
	if (numparts>500)
		numparts=500;
	for(i = 0; i < numparts; i++)
	{
		parttype = irand(0, 4);
		switch(parttype)
		{
			case 0:
				parttype = PART_4x4_WHITE;
				break;
			case 1:
				parttype = PART_16x16_STAR;
				break;
			case 2:
				parttype = PART_32x32_BUBBLE;
				break;
			case 3:
				parttype = PART_16x16_SPARK_B;
				break;
			case 4:
				parttype = PART_8x8_BLUE_CIRCLE;
				break;
			default:
				parttype = PART_4x4_WHITE;
				break;
		}

		spark = ClientParticle_new(parttype, self->r.color, 20);
		spark->scale = flrand(1, 2);
		spark->d_scale = flrand(-1, -1.5);
		spark->color.r = 255;
		spark->color.g = 255;
		spark->color.b = 255;
		spark->color.a = irand(100, 200.0);
		spark->d_alpha = flrand(-60.0, -42.0);
		spark->duration = flrand(1500, 3000);
		spark->acceleration[2] = flrand(10, 20);
//		spark->extraUpdate = ParticleFadeToBlue;

		VectorSet(spark->origin, flrand(-10,10),
									flrand(-10,10),
									flrand(-10, 10));

		AddParticleToList(TrailEnt, spark);
	}

	return true;
}

void ImpFireBallExplode(struct client_entity_s *self,centity_t *owner, vec3_t dir)
{
	client_entity_t	*SmokePuff;
	int				i;
	paletteRGBA_t	LightColor={255,64,32,255};
	byte			powerup = 0;
	
	Vec3ScaleAssign(32.0, dir);

	i = GetScaledCount(irand(12,16), 0.8);
	
	while(i--)
	{
		if (!i)
			SmokePuff=ClientEntity_new(FX_M_EFFECTS,0,owner->origin,NULL,500);
		else
			SmokePuff=ClientEntity_new(FX_M_EFFECTS,0,owner->origin,NULL,1500);
		
		SmokePuff->r.model = Imp_models + 1;
		SmokePuff->r.scale=flrand(0.5,1.0);
		SmokePuff->d_scale=-2.0;

		SmokePuff->r.flags |=RF_FULLBRIGHT|RF_TRANSLUCENT|RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		SmokePuff->r.frame = 0;

		VectorRandomCopy(dir, SmokePuff->velocity, flrand(16.0, 64.0));

		SmokePuff->acceleration[0] = flrand(-400, 400);
		SmokePuff->acceleration[1] = flrand(-400, 400);
		SmokePuff->acceleration[2] = flrand(-40, -60);

		SmokePuff->d_alpha= -0.4;
			
		SmokePuff->radius=20.0;

		AddEffect(NULL,SmokePuff);
	}
}

int ImpFireBallUpdate (struct client_entity_s *self, centity_t *owner)
{
	client_particle_t	*p;
	client_entity_t	*TrailEnt;
	vec3_t				angles, fwd, right;
	int					num_parts, i;
	paletteRGBA_t		LightColor;

	VectorScale(self->r.angles, 180.0/M_PI, angles);
	AngleVectors(angles, fwd, right, NULL);

	LightColor.c = 0xe5007fff;
	num_parts = irand(3, 7);
	for(i = 0; i < num_parts; i++)
	{
		p = ClientParticle_new(irand(PART_32x32_FIRE0, PART_32x32_FIRE2), LightColor, 1000);
		VectorSet(p->origin, flrand(-4, 4), flrand(-4, 4), flrand(-4, 4));
		VectorAdd(self->r.origin, p->origin, p->origin);
		p->scale = flrand(0.1, 0.5);
		p->type |= PFL_ADDITIVE;

		VectorSet(p->velocity, flrand(-10, 10), flrand(-10, 10), flrand(-1.0, 1.0));
		// Make the fire shoot out the back and to the side
		VectorMA(p->velocity, flrand(-40, -10), fwd, p->velocity);
		// Alternate left and right side of phoenix
		if (i&0x01)
			VectorMA(p->velocity, flrand(-10, -2), right, p->velocity);
		else
			VectorMA(p->velocity, flrand(10, 2), right, p->velocity);
		p->acceleration[2] = flrand(2, 10);
		p->d_scale = flrand(-15.0, -10.0);
		p->d_alpha = flrand(-200.0, -160.0);
		p->duration = (255.0 * 1000.0) / -p->d_alpha;		// time taken to reach zero alpha


		AddParticleToList(self, p);
	}
	
//trail	

	self->r.scale = flrand(0.35, 0.65);

	TrailEnt=ClientEntity_new(FX_M_EFFECTS,
							  CEF_DONT_LINK,
							  owner->origin,
							  NULL,
							  17);

	TrailEnt->radius = 2000;

	VectorCopy( owner->origin, TrailEnt->origin );

	TrailEnt->r.flags |= RF_TRANSLUCENT | RF_TRANS_ADD_ALPHA | RF_TRANS_ADD;
	TrailEnt->r.model = Morkproj_models + 2;

	TrailEnt->r.color.r = 180;
	TrailEnt->r.color.g = 60;
	TrailEnt->r.color.b = 0;
	TrailEnt->r.color.a = 255;
	TrailEnt->r.spriteType = SPRITE_LINE;
	TrailEnt->r.tile = 1;
	TrailEnt->r.scale = 3.0;

	VectorCopy( self->startpos, TrailEnt->r.startpos );
	VectorCopy( owner->origin , TrailEnt->r.endpos );

	TrailEnt->d_alpha = -4.0;
	TrailEnt->d_scale = 0.0;
	TrailEnt->Update = FXMorkTrailThink2;
	
	AddEffect(NULL,TrailEnt);

	VectorCopy(owner->origin, self->startpos);

	return true;
}

int star_particle [3] =
{
	PART_16x16_STAR,
	PART_16x16_SPARK_C,
	PART_16x16_SPARK_B,
};

int FXCWUpdate (struct client_entity_s *self, centity_t *owner)
{
	client_particle_t	*p;
	client_entity_t	*TrailEnt;
	vec3_t				angles, fwd, right, vec;
	int					num_parts, i;
	paletteRGBA_t	LightColor={255,255,255,255};

	client_entity_t	*placeholder;
	placeholder = ClientEntity_new(FX_M_EFFECTS, CEF_NO_DRAW|CEF_ABSOLUTE_PARTS, self->r.origin, NULL, 500);
	AddEffect(NULL, placeholder);

	VectorScale(self->r.angles, 180.0/M_PI, angles);
	AngleVectors(angles, fwd, right, NULL);

	num_parts = irand(3, 7);
	for(i = 0; i < num_parts; i++)
	{
		p = ClientParticle_new(star_particle[irand(0, 2)], LightColor, 2000);
		VectorSet(p->origin, flrand(-4, 4), flrand(-4, 4), flrand(-4, 4));
		VectorAdd(self->r.origin, p->origin, p->origin);
		p->scale = flrand(2.5, 3.0);

		VectorSet(p->velocity, flrand(-10, 10), flrand(-10, 10), flrand(-1.0, 1.0));
		VectorMA(p->velocity, flrand(-40, -10), fwd, p->velocity);

		if (i&0x01)
			VectorMA(p->velocity, flrand(-10, -2), right, p->velocity);
		else
			VectorMA(p->velocity, flrand(10, 2), right, p->velocity);

		p->acceleration[2] = 0;
		p->d_scale = flrand(-0.15, -0.10);
		p->duration = (p->scale * 1000.0) / -p->d_scale;		// time taken to reach zero scale

		AddParticleToList(placeholder, p);
	}
	
//trail	

	TrailEnt=ClientEntity_new(FX_M_EFFECTS,
							  CEF_DONT_LINK,
							  owner->origin,
							  NULL,
							  17);

	TrailEnt->radius = 2000;

	TrailEnt->r.flags |= RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	TrailEnt->r.model = CW_models;

	TrailEnt->r.spriteType = SPRITE_LINE;
	TrailEnt->r.tile = 1;
	TrailEnt->alpha = 1.0;
	TrailEnt->r.scale = 3.0;

	VectorCopy( self->startpos, TrailEnt->r.startpos );
	VectorCopy( owner->current.origin , TrailEnt->r.endpos );

	TrailEnt->d_alpha = -2.0;
	TrailEnt->d_scale = 0.0;
	TrailEnt->Update = FXMorkTrailThink2;

	AddEffect(NULL,TrailEnt);

//===============================================	

	TrailEnt=ClientEntity_new(FX_M_EFFECTS,
							  CEF_OWNERS_ORIGIN|CEF_AUTO_ORIGIN|CEF_USE_VELOCITY2,
							  owner->current.origin,
							  NULL,
							  17);

	TrailEnt->radius = 2000;

	if(ref_soft)
	{
		TrailEnt->r.model = CW_models;
		TrailEnt->r.scale = flrand(1.0, 2.5);
	}
	else
	{
		TrailEnt->r.model = Morkproj_models + 2;
		TrailEnt->flags |= CEF_USE_SCALE2;
		TrailEnt->r.scale = 3.0;
		TrailEnt->r.scale2 = 0.2;
	}

	TrailEnt->r.spriteType = SPRITE_LINE;

	TrailEnt->r.flags |= RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	TrailEnt->r.color.c = 0xFFFFaacc;
	TrailEnt->alpha = flrand(1.0, 0.75);
	TrailEnt->d_alpha = -1.0;
	TrailEnt->d_scale = -0.1;//outer part does not scale down

	//angle
	VectorSet(vec, flrand(0, 359), flrand(0, 359), flrand(0, 359));
	VectorCopy(vec, TrailEnt->direction);
	AngleVectors(vec, fwd, NULL, NULL);
	
	//length
	TrailEnt->SpawnInfo = flrand(20, 70);
	VectorCopy(owner->current.origin, TrailEnt->r.startpos);
	VectorMA(owner->current.origin, TrailEnt->SpawnInfo, fwd, TrailEnt->r.endpos);

	//avelocity
	VectorSet(TrailEnt->up, flrand(-10, 10), flrand(-10, 10), flrand(-10, 10));

	//speed
	VectorCopy(self->direction, TrailEnt->velocity);
	VectorCopy(self->direction, TrailEnt->velocity2);

	TrailEnt->Update = FXCWTrailThink;
	
	AddEffect(owner, TrailEnt);

//==============================================

	self->r.scale = flrand(0.65, 0.95);

	VectorCopy(owner->current.origin, self->startpos);

	return (true);
}

void FXCWStars (centity_t *owner,int type,int flags, vec3_t vel)
{
	client_entity_t	*fx;

	fx = ClientEntity_new( type, CEF_OWNERS_ORIGIN | CEF_DONT_LINK, owner->origin, NULL, 20);

	fx->Update=FXCWUpdate;
	fx->radius = 500;
	fx->r.model = Morkpp_models + 3;
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
	TrailEnt->r.model = buoy_models;

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
		blur->r.model = ass_dagger_model;
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
		blur->r.model = mork_model;
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
	dagger->r.model = ass_dagger_model;
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
	fx->r.model = Morkproj_models + 2;
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
	client_entity_t	*fx;
	paletteRGBA_t	LightColor={0,0,255,255};
	vec3_t			vel;
	byte			fx_index;
	int				i;
	
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_M_EFFECTS].formatString, &fx_index, &vel);//fixme- make this 1 dir and 1 float

	switch (fx_index)
	{
		case FX_M_MISC_EXPLODE:
			FXMorkMissileExplode(NULL, owner, vel);
			break;

		case FX_M_BEAM:
			fx = ClientEntity_new( type, CEF_NO_DRAW | CEF_OWNERS_ORIGIN | CEF_DONT_LINK, org, NULL, 20);

			fx->flags |= CEF_NO_DRAW;
			VectorCopy(owner->current.origin, fx->r.origin);
			fx->Update=FXMorkBeam;
			fx->dlight=CE_DLight_new(LightColor,150.0f,0.0f);
			fx->radius = 500;
			fx->r.flags |= RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
			fx->AddToView = LinkedEntityUpdatePlacement;

			VectorCopy(owner->origin, fx->startpos);

			AddEffect(owner,fx);

			FXMorkBeam(fx,owner);

			for(i = 0; i<3; i++)
			{
				fx = ClientEntity_new( type, 0, org, NULL, 20);

				VectorCopy(owner->current.origin, fx->r.origin);
				fx->Update=FXMorkBeamCircle;
				fx->radius = 500;
				fx->r.model = Morkproj_models + 3;
				fx->r.flags |= RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
				fx->r.scale = 0.5;
				fx->LifeTime = i * 120;

				VectorCopy(owner->origin, fx->startpos);
				VectorCopy(vel, fx->r.angles);

				AddEffect(owner,fx);

				FXMorkBeamCircle(fx,owner);
			}
			break;

		case FX_IMP_FIRE:
			fx = ClientEntity_new(FX_SPARKS, CEF_OWNERS_ORIGIN | CEF_DONT_LINK|CEF_ADDITIVE_PARTS | CEF_ABSOLUTE_PARTS, org, NULL, 20);

			fx->r.flags |= RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
			fx->r.model = Imp_models + 2;

			vectoangles(vel, fx->r.angles);
			
			fx->r.frame = 2;
			fx->radius = 64;
			fx->r.scale = 0.5;
			fx->d_alpha = 0.0f;
			fx->d_scale = 0.0f;
			fx->r.color.c = 0xe5007fff;

			fx->Update = ImpFireBallUpdate;
			fx->AddToView = LinkedEntityUpdatePlacement;

			if(r_detail->value > DETAIL_NORMAL)
			{
				LightColor.c = 0xff3333ff;
				fx->dlight = CE_DLight_new(LightColor,150.0f,0.0f);
			}

			VectorCopy(owner->origin, fx->startpos);

			AddEffect(owner,fx);
			break;
		
		case FX_IMP_FBEXPL:
			ImpFireBallExplode(NULL, owner, vel);
			break;

		case FX_CW_STARS:
			FXCWStars(owner, type, flags, org);
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