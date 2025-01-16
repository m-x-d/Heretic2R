//
// fx_hpproj.c
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
#include "random.h"
#include "q_Sprite.h"
#include "Utilities.h"
#include "reference.h"
#include "Matrix.h"
#include "g_playstats.h"

#define	NUM_MISSILE_MODELS	11
#define	NUM_STAFF_MODELS	1

static struct model_s *hpproj_models[NUM_MISSILE_MODELS];
static struct model_s *hpstaff_models[NUM_STAFF_MODELS];

enum {
	HPMISSILE1,
	HPMISSILE2,
	HPMISSILE3,
	HPMISSILE4,
	HPMISSILE5,
	HPMISSILE1_EXPLODE,
	HPMISSILE2_EXPLODE,
	HPMISSILE3_EXPLODE,
	HPMISSILE4_EXPLODE,
	HPMISSILE5_EXPLODE,
	HPMISSILE1_LIGHT,
	HPMISSILE2_LIGHT,
	HPMISSILE3_LIGHT,
	HPMISSILE4_LIGHT,
	HPMISSILE5_LIGHT,
	HPTELEPORT_START,
	HPTELEPORT_END,
	HPLIGHTNING_BOLT,
};

/*
	Precaching Functions

*/

void PreCacheHPStaff()
{
	//Staff Trail
	hpstaff_models[0] = fxi.RegisterModel("sprites/fx/hpproj1_2.sp2");
}

void PreCacheHPMissile()
{
	//Projectile head sprites
	hpproj_models[0] = fxi.RegisterModel("sprites/fx/hpproj1_1.sp2");
	hpproj_models[1] = fxi.RegisterModel("sprites/fx/hpproj1_2.sp2");
	
	//Trail segments
	hpproj_models[2] = fxi.RegisterModel("sprites/fx/segment_trail.sp2");
	hpproj_models[6] = fxi.RegisterModel("sprites/fx/segment_trail_y.sp2");
	hpproj_models[7] = fxi.RegisterModel("sprites/fx/segment_trail_wt.sp2");

	//Halos
	hpproj_models[3] = fxi.RegisterModel("sprites/fx/hp_halo.sp2");
	hpproj_models[4] = fxi.RegisterModel("sprites/lens/halo1.sp2");
	
	//Light Bug model
	hpproj_models[5] = fxi.RegisterModel("models/objects/lights/bug/tris.fm");

	//Rocks
	hpproj_models[8] = fxi.RegisterModel("models/debris/stone/schunk1/tris.fm");
	hpproj_models[9] = fxi.RegisterModel("models/debris/stone/schunk2/tris.fm");

	//Lightning bolts
	hpproj_models[10] = fxi.RegisterModel("sprites/fx/hp_lightning.sp2");
}

/*
	High Priestess Projectile Functions

*/

#define PRIESTESS_TELEPORT_LINEHEIGHT 764

static qboolean FXHPTeleportLineThink(struct client_entity_s *self,centity_t *Owner)
{
	client_entity_t		*effect;
	client_particle_t	*p;
	int					i, color_ofs;
	paletteRGBA_t		color = {255,255,255,255};

	if (self->alpha <= 0.0)
		return false;

	self->r.endpos[2] += 32;
	
	if ((self->r.endpos[2] - self->r.startpos[2]) > PRIESTESS_TELEPORT_LINEHEIGHT)
	{
		if (self->SpawnInfo == 0.0)
			fxi.Activate_Screen_Flash(0xffffffff);

		self->SpawnInfo = 1;
		self->d_alpha = -1.0;
		self->r.scale += 16;
	}
	else
	{
		self->r.scale += 1.5;

		//Spawn some particles and some rock chunks

		i =  GetScaledCount(8, 0.7);//irand(8,12);

		while(i--)
		{
			p = ClientParticle_new(PART_4x4_WHITE, color, 2500);

			//Chance to make them brown
			if (irand(0,1))
			{
				color_ofs = irand(0, 64);
				p->color.r = color_ofs;
				p->color.g = color_ofs;
				p->color.b = 0;
				p->color.a = 255;
			}
			else
			{
				color_ofs = irand(32, 128);
				p->color.r = p->color.g = p->color.b = color_ofs;
				p->color.a = 255;
			}

			p->origin[0] += irand(-24,24);
			p->origin[1] += irand(-24,24);
			p->origin[2] -= 36;
			
			p->scale = flrand(0.25, 1.0);
			
			p->acceleration[0] = irand(-75,75);
			p->acceleration[1] = irand(-75,75);
			p->acceleration[2] = irand(400,500);

			p->d_alpha = -2.0;
			AddParticleToList(self, p);
		}

		i = GetScaledCount(5, 0.7);//irand(4,6);

		while(i--)
		{
			effect=ClientEntity_new(  FX_HP_MISSILE,
									  CEF_DONT_LINK,
									  self->origin,
									  NULL,
									  1000);

			effect->r.model = hpproj_models + irand(8,9);

			effect->r.scale = flrand(0.1, 0.5);

			VectorCopy(self->r.origin, effect->r.origin);
			effect->r.origin[0] += irand(-32,32);
			effect->r.origin[1] += irand(-32,32);
			effect->r.origin[2] -= 36;

			effect->velocity[2] = 1;
			
			effect->acceleration[0] = irand(-150, 150);
			effect->acceleration[1] = irand(-150, 150);
			effect->acceleration[2] = irand( 300, 600 - ( self->r.scale * 100) );

			effect->d_alpha = -0.25;

			effect->radius = 500;

			AddEffect(NULL, effect);
		}
	}

	return true;
}

static qboolean FXHPTeleportLineThink2(struct client_entity_s *self,centity_t *Owner)
{
	if (self->alpha <= 0.0f)
		return false;

	if (self->r.scale <= 0.0f)
		return false;

	self->r.scale -= 4;

	return true;
}

/*-----------------------------------------------
	FXHPMissileSpawnerThink
-----------------------------------------------*/

static qboolean FXHPMissileSpawnerThink(struct client_entity_s *self,centity_t *Owner)
{
	client_entity_t	*TrailEnt;

	if (self->LifeTime < fxi.cl->time)
		return false;

	TrailEnt=ClientEntity_new(FX_HP_MISSILE,
							  CEF_DONT_LINK,
							  self->origin,
							  NULL,
							  1000);

	TrailEnt->radius = 500;

	TrailEnt->r.flags |= RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	TrailEnt->r.model = hpproj_models + 3;

	TrailEnt->r.color.c = 0xFFFFFFFF;
	TrailEnt->alpha = 1.0;
	TrailEnt->r.scale = 0.1;

	TrailEnt->d_alpha = -2.5;
	TrailEnt->d_scale = 4.0;
	
	VectorCopy(self->origin, TrailEnt->origin);
	
	TrailEnt->velocity[0] = irand(-16, 16);
	TrailEnt->velocity[1] = irand(-16, 16);
	TrailEnt->velocity[2] = irand(-16, 16);

	AddEffect(NULL,TrailEnt);

	return true;	
}

/*-----------------------------------------------
	FXHPMissileSpawnerThink2
-----------------------------------------------*/

static qboolean FXHPMissileSpawnerThink2(struct client_entity_s *self,centity_t *Owner)
{
	client_entity_t	*TrailEnt;

	if (self->LifeTime < fxi.cl->time)
		return false;

	TrailEnt=ClientEntity_new(FX_HP_MISSILE,
							  CEF_DONT_LINK,
							  self->origin,
							  NULL,
							  1000);

	TrailEnt->radius = 500;

	TrailEnt->r.flags |= RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	TrailEnt->r.model = hpproj_models + 4;

	TrailEnt->r.color.r = 229;
	TrailEnt->r.color.g = 250;
	TrailEnt->r.color.b = 88;
	TrailEnt->r.color.a = 255;

	TrailEnt->alpha = 0.5;
	TrailEnt->r.scale = 0.25;

	TrailEnt->d_alpha = -2.5;
	TrailEnt->d_scale = 2.0;
	
	VectorCopy(self->origin, TrailEnt->origin);
	
	TrailEnt->velocity[0] = irand(-16, 16);
	TrailEnt->velocity[1] = irand(-16, 16);
	TrailEnt->velocity[2] = irand(-16, 16);
	
	AddEffect(NULL,TrailEnt);

	return true;	
}

/*-----------------------------------------------
	FXHPHaloDie
-----------------------------------------------*/

static qboolean FXHPHaloDie(struct client_entity_s *self,centity_t *Owner)
{
	if (self->r.scale <= 0.0f)
		return false;

	if (self->alpha <= 0.0f)
		return false;

	return true;
}
/*-----------------------------------------------
	FXHPMissileSpawnerThink3
-----------------------------------------------*/

static qboolean FXHPMissileSpawnerThink3(struct client_entity_s *self,centity_t *Owner)
{
	if (self->LifeTime < fxi.cl->time)
	{
		self->Update = FXHPHaloDie;
		self->d_scale = -2.0;
		self->d_alpha = -1.0;
		return true;
	}
	
	if (self->d_scale == 0.0)
	{
		self->r.scale = flrand(1.75, 2.25);
		return true;
	}

	if (self->r.scale >= 2)
		self->d_scale = 0.0;
	
	if (self->alpha > 0.5)
	{
		self->d_alpha = 0.0;
		self->alpha = 0.5;
	}

	return true;	
}

/*-----------------------------------------------
	FXHPTrailThink
-----------------------------------------------*/

static qboolean FXHPTrailThink(struct client_entity_s *self,centity_t *Owner)
{
	if (self->alpha <= 0.1 || self->r.scale <= 0.0)
		return false;

	self->r.scale -= 0.1;
	
	return true;
}

/*-----------------------------------------------
	FXHPTrailThink2
-----------------------------------------------*/

static qboolean FXHPTrailThink2(struct client_entity_s *self,centity_t *Owner)
{
	if (self->alpha <= 0.1 || self->r.scale <= 0.0)
		return false;

	self->r.scale -= 0.15;

	return true;
}

/*-----------------------------------------------
	FXHPTrailThink3
-----------------------------------------------*/

static qboolean FXHPTrailThink3(struct client_entity_s *self,centity_t *Owner)
{
	if (self->alpha <= 0.1 || self->r.scale <= 0.0)
		return false;

	self->r.scale -= 0.25;

	return true;
}

/*-----------------------------------------------
	FXHPBugThink
-----------------------------------------------*/

static qboolean FXHPBugThink(struct client_entity_s *self,centity_t *Owner)
{
	self->r.scale = flrand(0.2, 0.4);
	self->alpha = flrand(0.3, 0.5);

	return true;
}

/*-----------------------------------------------
	FXHPMissileTrailThink
-----------------------------------------------*/

static qboolean FXHPMissileTrailThink(struct client_entity_s *self,centity_t *Owner)
{
	client_entity_t	*TrailEnt;

	self->r.scale = flrand(0.35, 0.65);

	TrailEnt=ClientEntity_new(FX_HP_MISSILE,
							  CEF_DONT_LINK,
							  Owner->origin,
							  NULL,
							  17);

	TrailEnt->radius = 500;

	VectorCopy( Owner->origin, TrailEnt->origin );

	TrailEnt->r.flags |= RF_TRANSLUCENT | RF_TRANS_ADD_ALPHA;
	TrailEnt->r.model = hpproj_models + 2;

 	TrailEnt->r.spriteType = SPRITE_LINE;
	TrailEnt->r.tile = 1;
	TrailEnt->r.scale = 2.5;
	TrailEnt->alpha = 1.0;
	TrailEnt->r.scale = 1.0;

	VectorCopy( self->startpos, TrailEnt->r.startpos );
	VectorCopy( Owner->origin , TrailEnt->r.endpos );

	TrailEnt->d_alpha = -2.5;
	TrailEnt->d_scale = 0.0;
	TrailEnt->Update = FXHPTrailThink;
	
	AddEffect(NULL,TrailEnt);

	VectorCopy(Owner->origin, self->startpos);
	
	return true;
}

/*-----------------------------------------------
	FXHPMissileTrailThink2
-----------------------------------------------*/

static qboolean FXHPMissileTrailThink2(struct client_entity_s *self,centity_t *Owner)
{
	client_entity_t	*TrailEnt;

	self->r.scale = flrand(0.35, 0.55);

	TrailEnt=ClientEntity_new(FX_HP_MISSILE,
							  CEF_DONT_LINK,
							  Owner->origin,
							  NULL,
							  17);

	TrailEnt->radius = 500;

	VectorCopy( Owner->origin, TrailEnt->origin );

	TrailEnt->r.flags |= RF_TRANSLUCENT | RF_TRANS_ADD_ALPHA;
	TrailEnt->r.model = hpproj_models + 2;

	TrailEnt->r.spriteType = SPRITE_LINE;
	TrailEnt->r.tile = 1;
	TrailEnt->r.scale = 2;
	TrailEnt->alpha = 1.0;
	TrailEnt->r.scale = 1.0;

	VectorCopy( self->startpos, TrailEnt->r.startpos );
	VectorCopy( Owner->origin , TrailEnt->r.endpos );

	TrailEnt->d_alpha = -4.0;
	TrailEnt->d_scale = 0.0;
	TrailEnt->Update = FXHPTrailThink2;
	
	AddEffect(NULL,TrailEnt);

	VectorCopy(Owner->origin, self->startpos);

	return true;
}

/*-----------------------------------------------
	FXHPMissileTrailThink3
-----------------------------------------------*/

static qboolean FXHPMissileTrailThink3(struct client_entity_s *self,centity_t *Owner)
{
	client_entity_t	*TrailEnt;

	TrailEnt=ClientEntity_new(FX_HP_MISSILE,
							  CEF_DONT_LINK,
							  Owner->origin,
							  NULL,
							  17);

	TrailEnt->radius = 500;

	VectorCopy( Owner->origin, TrailEnt->origin );

	TrailEnt->r.flags |= RF_TRANSLUCENT | RF_TRANS_ADD_ALPHA;
	TrailEnt->r.model = hpproj_models + 6;

	TrailEnt->r.spriteType = SPRITE_LINE;
	TrailEnt->r.tile = 1;
	TrailEnt->r.scale = 2.0;
	TrailEnt->alpha = 1.0;
	TrailEnt->r.scale = 1.0;

	VectorCopy( self->startpos, TrailEnt->r.startpos );
	VectorCopy( Owner->origin , TrailEnt->r.endpos );

	TrailEnt->d_alpha = -2.0;
	TrailEnt->d_scale = 0.0;
	TrailEnt->Update = FXHPTrailThink3;
	
	AddEffect(NULL,TrailEnt);

	VectorCopy(Owner->origin, self->startpos);

	return true;
}

/*-----------------------------------------------
	FXHPMissileExplode
-----------------------------------------------*/

void FXHPMissileExplode(struct client_entity_s *self,centity_t *Owner)
{
	vec3_t			dir;
	client_entity_t	*SmokePuff;
	int				i;
	paletteRGBA_t	LightColor={255,64,32,255};
	byte			powerup = 0;
	
	VectorSet(dir, 1, 1, 1);
	//Vec3ScaleAssign(32.0, dir);

	i = GetScaledCount(irand(6,8), 0.8);
	
	while(i--)
	{
		if (!i)
			SmokePuff=ClientEntity_new(FX_HP_MISSILE,0,Owner->origin,NULL,500);
		else
			SmokePuff=ClientEntity_new(FX_HP_MISSILE,0,Owner->origin,NULL,1500);
		
		SmokePuff->r.model = hpproj_models + 3;
		SmokePuff->r.scale=flrand(0.5,1.0);
		SmokePuff->d_scale = flrand(-1.0, -1.5);

		SmokePuff->r.flags |=RF_FULLBRIGHT|RF_TRANSLUCENT|RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		SmokePuff->r.frame = 0;

		VectorRandomCopy(dir, SmokePuff->velocity, flrand(64.0, 128.0));

		SmokePuff->acceleration[0] = flrand(-100, -50);
		SmokePuff->acceleration[1] = flrand(-100, -50);
		SmokePuff->acceleration[2] = flrand(-100, -250);

		SmokePuff->d_alpha= -0.2;
			
		SmokePuff->radius=20.0;

		AddEffect(NULL,SmokePuff);
	}
}

/*-----------------------------------------------
	FXHPBugExplode
-----------------------------------------------*/

void FXHPBugExplode(struct client_entity_s *self,centity_t *Owner)
{
	vec3_t			dir;
	client_entity_t	*SmokePuff;
	int				i;
	paletteRGBA_t	LightColor={255,64,32,255};
	byte			powerup = 0;
	
	//Vec3ScaleAssign(32.0, dir);
	VectorSet(dir, 1, 1, 1);

	i = GetScaledCount(irand(12,16), 0.8);
	
	while(i--)
	{
		if (!i)
			SmokePuff=ClientEntity_new(FX_HP_MISSILE,0,Owner->origin,NULL,500);
		else
			SmokePuff=ClientEntity_new(FX_HP_MISSILE,0,Owner->origin,NULL,1500);
		
		SmokePuff->r.model = hpproj_models;
		SmokePuff->r.scale=flrand(0.5,1.0);
		SmokePuff->d_scale=-2.0;

		SmokePuff->r.flags |=RF_FULLBRIGHT|RF_TRANSLUCENT|RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		SmokePuff->r.frame = 0;

		VectorRandomCopy(dir, SmokePuff->velocity, flrand(16.0, 64.0));

		SmokePuff->acceleration[0] = flrand(-200, 200);
		SmokePuff->acceleration[1] = flrand(-200, 200);
		SmokePuff->acceleration[2] = flrand(-60, -100);

		SmokePuff->d_alpha= -0.4;
			
		SmokePuff->radius=20.0;

		AddEffect(NULL,SmokePuff);
	}
}

/*-----------------------------------------------
	FXHPMissileCreateWarp
-----------------------------------------------*/

void FXHPMissileCreateWarp(centity_t *Owner,int Type,int Flags,vec3_t Origin)
{
	client_entity_t	*Trail;

	Trail = ClientEntity_new( Type, CEF_DONT_LINK, Origin, NULL, 2000);

	Trail->radius = 500;
	Trail->r.model = hpproj_models + 3;
	Trail->r.color.c = 0xffff5555;
	Trail->r.flags |= RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	Trail->r.scale = 0.1;
	Trail->d_scale = 2.0;
	Trail->d_alpha = -2.0;

	AddEffect(NULL,Trail);
}

static qboolean PriestessLinkedEntityUpdatePlacement(struct client_entity_s *self, centity_t *owner)
{
	LinkedEntityUpdatePlacement(self, owner);
	VectorCopy(self->r.origin, self->r.startpos);

	return true;
}

/*-----------------------------------------------
	FXHPMissile
-----------------------------------------------*/

void FXHPMissile(centity_t *Owner,int Type,int Flags,vec3_t Origin)
{
	client_entity_t	*Trail;
	paletteRGBA_t	LightColor={0,0,255,255};
	paletteRGBA_t	BugColor={229,250,88,255};
	paletteRGBA_t	BrightColor={255,255,255,255};
	vec3_t			vel, boltDir, boltAng, boltDest, oldPos, ang, huntdir;
	float			boltDist, boltStep, width, alpha;
	byte			effectType;
	int				bends, i, bolts, j;

	
	fxi.GetEffect(Owner, Flags, clientEffectSpawners[FX_HP_MISSILE].formatString, &vel, &effectType);

	switch ( effectType )
	{
	
	//Blue swirling, homing missiles
	case HPMISSILE1:

		FXHPMissileCreateWarp(Owner, Type, Flags, Origin);

		Trail = ClientEntity_new( Type, CEF_OWNERS_ORIGIN | CEF_DONT_LINK, Origin, NULL, 20);

		VectorCopy(vel, Trail->up);
		Trail->Update=FXHPMissileTrailThink;
		Trail->dlight=CE_DLight_new(LightColor,150.0f,0.0f);
		Trail->radius = 500;
		Trail->r.model = hpproj_models + 3;
		Trail->r.color.c = 0x00999999;
		Trail->r.flags |= RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		Trail->r.scale = 0.5;
		Trail->AddToView = PriestessLinkedEntityUpdatePlacement;

		VectorCopy(Origin, Trail->startpos);

		AddEffect(Owner,Trail);

		FXHPMissileTrailThink(Trail,Owner);

		break;

	//Normal trails off projectiles
	case HPMISSILE2:

		FXHPMissileCreateWarp(Owner, Type, Flags, Origin);

		Trail = ClientEntity_new( Type, CEF_OWNERS_ORIGIN | CEF_DONT_LINK, Origin, NULL, 20);

		VectorCopy(vel, Trail->up);
		Trail->Update=FXHPMissileTrailThink2;
		Trail->dlight=CE_DLight_new(LightColor,150.0f,0.0f);
		Trail->radius = 500;
		Trail->r.model = hpproj_models + 3;
		Trail->r.color.c = 0x00999999;
		Trail->r.flags |= RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		Trail->r.scale = 0.45;
		Trail->AddToView = PriestessLinkedEntityUpdatePlacement;

		VectorCopy(Owner->origin, Trail->startpos);

		AddEffect(Owner,Trail);

		FXHPMissileTrailThink2(Trail,Owner);

		break;

	//Light bugs/mines
	case HPMISSILE3:

		Trail = ClientEntity_new( Type, CEF_OWNERS_ORIGIN | CEF_DONT_LINK, Origin, NULL, 20);

		Trail->Update=FXHPMissileTrailThink3;
				
		Trail->dlight=CE_DLight_new(BugColor, 50.0f, 0.0f);
		Trail->radius = 500;
		Trail->r.model = hpproj_models + 5;
		
		VectorNormalize(vel);
		
		AnglesFromDir(vel, Trail->r.angles);
		Trail->r.angles[PITCH] -= ANGLE_90;
		
		Trail->r.color.c = 0xFF999999;
		Trail->r.scale = flrand(0.3, 0.4);
		Trail->AddToView = LinkedEntityUpdatePlacement;

		VectorCopy(Owner->origin, Trail->startpos);

		AddEffect(Owner,Trail);

		FXHPMissileTrailThink3(Trail,Owner);

		//Create the halo to follow the bug
		Trail = ClientEntity_new( Type, CEF_OWNERS_ORIGIN | CEF_DONT_LINK, Origin, NULL, 20);

		Trail->Update=FXHPBugThink;
				
		Trail->radius = 500;
		Trail->r.model = hpproj_models + 4;
		
		Trail->r.color.r = 229;
		Trail->r.color.g = 250;
		Trail->r.color.b = 88;
		Trail->r.color.a = 255;
		
		Trail->alpha = 0.5;

		Trail->r.flags |= RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		
		Trail->r.scale = flrand(0.3, 0.4);
		
		Trail->AddToView = LinkedEntityUpdatePlacement;

		VectorCopy(Origin, Trail->startpos);

		AddEffect(Owner,Trail);

		FXHPBugThink(Trail,Owner);

		break;

	//Light shafts coming from the priestess' staff
	case HPMISSILE4:

		i = irand(5,8);

		while (i--)
		{
			Trail = ClientEntity_new( Type, CEF_DONT_LINK, Origin, NULL, 2000);

			Trail->radius = 500;
			Trail->r.flags |= RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
			Trail->r.model = hpproj_models + 7;

			Trail->r.spriteType = SPRITE_LINE;
			Trail->r.tile = 1;
			Trail->r.scale = irand(1.0, 2.0);
			Trail->alpha = 1.0;
			Trail->r.scale = 1.0;

			ang[PITCH] = irand( 0, 720 );
			ang[YAW]   = irand( 0, 720 );
			ang[ROLL]  = irand( 0, 720 );

			AngleVectors( ang, vel, NULL, NULL );
			VectorMA( Origin, 512, vel, Trail->r.endpos );

			VectorCopy( Origin, Trail->r.startpos );
			VectorCopy( Origin, Trail->r.origin );
			
			Trail->d_alpha = -2.0;
			Trail->d_scale = 0.0;

			AddEffect(NULL,Trail);
		}

		break;

	//The power bolts she shoots from her staff 
	case HPMISSILE5:

		bolts = irand(2,3);

		for (j=0;j<bolts;j++)
		{
			//Find the number of bends in the bolt
			bends = irand(3,6);

			//Find the distance for each segment (totalDist/bends)
			VectorSubtract(vel, Origin, boltDir);
			boltDist = VectorNormalize(boltDir);

			//Get an offset angle from the goal direction
			vectoangles(boltDir, boltAng);

			boltAng[PITCH]  += irand(-35, 35);
			boltAng[YAW]	+= irand(-35, 35);

			VectorCopy(boltDir, boltDest);
			AngleVectors(boltAng, boltDir, NULL, NULL);

			VectorCopy(Origin, oldPos);

			width = irand(2.0, 6.0);
			alpha = irand(-4.0, -8.0);

			for (i = 1; i < bends + 1; i++)
			{
				Trail = ClientEntity_new( Type, CEF_DONT_LINK, Origin, NULL, 2000);

				if (!i)
				{
					VectorCopy(Origin, Trail->r.startpos);
					VectorCopy(Origin, Trail->r.endpos);
				}

				Trail->radius = 500;
				Trail->r.flags |= RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
				Trail->r.model = hpproj_models + 10;

				Trail->r.spriteType = SPRITE_LINE;
				Trail->r.scale = width;
				Trail->alpha = 1.0;
				Trail->r.scale = 1.0;

				VectorCopy(vel, Trail->r.origin);

				if (i == bends)
				{
					VectorCopy(oldPos, Trail->r.startpos);					
					VectorCopy(vel, Trail->r.endpos);
					
					Trail->r.endpos[2] += irand(-4, 4);
				}
				else
				{
					VectorSubtract(vel, oldPos, huntdir);
					boltStep = VectorNormalize(huntdir);
					boltStep /= ((bends+1) - i);

					VectorScale(boltDir, (2.5/bends), boltDir);
					VectorAdd(boltDir, huntdir, boltDir);

					VectorNormalize(boltDir);
								
					VectorCopy(oldPos, Trail->r.startpos);
					
					Trail->r.tile = ceil(boltStep / 128);

					VectorMA(Trail->r.startpos, boltStep, boltDir, Trail->r.endpos);
					VectorCopy(Trail->r.endpos, oldPos);
				}

				Trail->d_alpha = alpha;
				Trail->d_scale = 0.0;

				AddEffect(NULL,Trail);
			}
		}

		break;

	//Normal explosions
	case HPMISSILE1_EXPLODE:
	case HPMISSILE2_EXPLODE:
		Trail = ClientEntity_new( Type, CEF_NO_DRAW | CEF_DONT_LINK, Origin, NULL, 2000);	
		AddEffect(NULL, Trail);
		FXHPMissileExplode(Trail,Owner);
		break;

	//Light bug explosion
	case HPMISSILE3_EXPLODE:
		Trail = ClientEntity_new( Type, CEF_NO_DRAW | CEF_DONT_LINK, Origin, NULL, 2000);	
		AddEffect(NULL, Trail);
		FXHPBugExplode(Trail,Owner);
		break;

	//Light the eminates from her staff when casting effects
	case HPMISSILE1_LIGHT:
	case HPMISSILE2_LIGHT:

		Trail = ClientEntity_new( Type, CEF_NO_DRAW | CEF_DONT_LINK, Origin, NULL, 20);

		VectorCopy( Origin, Trail->origin );
		Trail->Update=FXHPMissileSpawnerThink;
		Trail->radius = 500;
		Trail->r.model = hpproj_models + 3;
		Trail->r.color.c = 0xFFFFFFFF;
		Trail->r.flags |= RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		Trail->dlight=CE_DLight_new(LightColor,200.0f,0.0f);
		Trail->LifeTime = fxi.cl->time + 4000;

		AddEffect(NULL,Trail);

		break;

	//Light the eminates from her staff when casting bugs
	case HPMISSILE3_LIGHT:

		Trail = ClientEntity_new( Type, CEF_NO_DRAW | CEF_DONT_LINK, Origin, NULL, 20);

		VectorCopy( Origin, Trail->origin );
		Trail->Update=FXHPMissileSpawnerThink2;
		
		Trail->radius = 500;
		Trail->dlight=CE_DLight_new(BugColor,10.0f,0.0f);
		Trail->LifeTime = fxi.cl->time + 2000;

		AddEffect(NULL,Trail);

		break;
	
	//Light for the massive light attack
	case HPMISSILE4_LIGHT:

		fxi.Activate_Screen_Flash(0xffffffff);

		//Create the large halo that scales up and fades out
		Trail = ClientEntity_new( Type, CEF_DONT_LINK, Origin, NULL, 4000);

		VectorCopy( Origin, Trail->origin );

		Trail->r.flags |= RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		
		Trail->r.model = hpproj_models + 4;
		Trail->r.scale = 0.1;
		Trail->alpha = 1.0;
		Trail->d_alpha = -1.0;
		Trail->d_scale = 4.0;
		Trail->radius = 500;
		Trail->LifeTime = fxi.cl->time + 4000;

		AddEffect(NULL,Trail);

		//Create the sparkling center
		Trail = ClientEntity_new( Type, CEF_DONT_LINK, Origin, NULL, 20);

		VectorCopy( Origin, Trail->origin );

		Trail->r.flags |= RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		Trail->Update=FXHPMissileSpawnerThink3;
		
		Trail->r.model = hpproj_models + 4;
		Trail->r.scale = 0.1;
		Trail->alpha = 0.1;
		Trail->d_alpha = 0.5;
		Trail->d_scale = 2.0;
		Trail->radius = 500;
		Trail->dlight=CE_DLight_new(BrightColor,250.0f,0.0f);
		Trail->LifeTime = fxi.cl->time + 4000;

		AddEffect(NULL,Trail);

		//Shake the screen
		fxi.Activate_Screen_Shake(4, 5500, fxi.cl->time, SHAKE_ALL_DIR);

		break;

		//Empty
	case HPMISSILE5_LIGHT:

		break;

		//Starting effect for her teleport
	case HPTELEPORT_START:

		Trail = ClientEntity_new( Type, CEF_DONT_LINK, Origin, NULL, 20);

		VectorCopy( Origin, Trail->origin );

		Trail->r.flags |= RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		Trail->Update=FXHPTeleportLineThink;

		Trail->r.model = hpproj_models + 7;			
		Trail->r.spriteType = SPRITE_LINE;
		Trail->r.tile = 1.0;
		Trail->r.scale = 2;
		Trail->alpha = 1.0;
		Trail->r.scale = 1.0;

		VectorCopy( Origin, Trail->r.startpos );
		Trail->r.startpos[2] -= 128;
		
		VectorCopy( Origin, Trail->r.endpos );
		Trail->r.endpos[2] += 32;
		
		Trail->dlight=CE_DLight_new(BrightColor,250.0f,0.0f);
		Trail->d_alpha = 0.0;
		Trail->d_scale = 0.0;

		AddEffect(NULL,Trail);

		break;

		//Ending effect for her teleport
		case HPTELEPORT_END:
		
		Trail = ClientEntity_new( Type, CEF_DONT_LINK, Origin, NULL, 20);

		VectorCopy( Origin, Trail->origin );

		Trail->r.flags |= RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		Trail->Update=FXHPTeleportLineThink2;

		Trail->r.model = hpproj_models + 7;			
		Trail->r.spriteType = SPRITE_LINE;
		Trail->r.tile = 1.0;
		Trail->r.scale = 64;
		Trail->alpha = 1.0;
		Trail->r.scale = 1.0;

		VectorCopy( Origin, Trail->r.startpos );
		Trail->r.startpos[2] -= 128;
		
		VectorCopy( Origin, Trail->r.endpos );
		Trail->r.endpos[2] += 512;
		
		Trail->dlight=CE_DLight_new(BrightColor,250.0f,0.0f);
		Trail->d_alpha = -1.0;
		Trail->d_scale = 0.0;

		AddEffect(NULL,Trail);

		break;

	default:

		Com_DPrintf("ERROR FXHPMissile: No available effect processor! (EFFECT ID %d)\n", effectType);
		break;
	}
}





















/*

	High Priestess Staff Effects

*/

enum 
{
	HP_STAFF_INIT,
	HP_STAFF_TRAIL,
} HighPriestessStaff_e;

/*-----------------------------------------------
	FXHPStaff
-----------------------------------------------*/

qboolean HPStaffTrailThink(struct client_entity_s *self, centity_t *owner)
{
	client_entity_t	*Trail;
	matrix3_t		RotationMatrix;

	// This tells if we are wasting our time, because the reference points are culled.
	if (!RefPointsValid((centity_t *)(self->extra)))
		return true;

	Trail = ClientEntity_new( FX_HP_STAFF, CEF_DONT_LINK, self->r.origin, NULL, 2000);

	Trail->r.flags |= RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	
	Trail->r.model = hpstaff_models;
	Trail->r.scale = 0.75;
	Trail->alpha = 0.5;
	Trail->d_alpha = -2.0;
	Trail->d_scale = -2.0;
	Trail->radius = 500;
	
	Matrix3FromAngles(((centity_t *)(self->extra))->lerp_angles,RotationMatrix);

	Matrix3MultByVec3(	RotationMatrix,
						((centity_t *)(self->extra))->referenceInfo->references[PRIESTESS_STAFF].placement.origin,
						Trail->r.origin);

	VectorAdd(((centity_t *)(self->extra))->origin, Trail->r.origin, Trail->r.origin);
	Trail->r.origin[2] -= 36;

	AddEffect(NULL,Trail);

	return true;
}

/*-----------------------------------------------
	PriestessEffectStayAlive
-----------------------------------------------*/

qboolean PriestessEffectStayAlive(struct client_entity_s *self, centity_t *owner)
{
	return true;
}

/*-----------------------------------------------
	PriestessFirstSeenInit
-----------------------------------------------*/

qboolean PriestessFirstSeenInit(struct client_entity_s *self, centity_t *owner)
{
	self->refMask |= PRIESTESS_MASK;

	EnableRefPoints(owner->referenceInfo, self->refMask);

	self->AddToView = NULL;
	self->Update = HPStaffTrailThink;

	HPStaffTrailThink(self, owner);

	return true;
}

/*-----------------------------------------------
	FXHPStaff
-----------------------------------------------*/

void FXHPStaff(centity_t *Owner,int Type,int Flags,vec3_t Origin)
{
	client_entity_t		*self;
	short				entID;
	byte				type;

	fxi.GetEffect(Owner, Flags, clientEffectSpawners[FX_HP_STAFF].formatString, &type, &entID);

	switch (type)
	{
	case HP_STAFF_INIT:
		
		self = ClientEntity_new(Type, Flags | CEF_NO_DRAW | CEF_ABSOLUTE_PARTS,  Origin, NULL, 17);

		self->Update = NULL;
		self->AddToView = PriestessFirstSeenInit;
		self->Update = PriestessEffectStayAlive;
		self->extra=(void *)(&fxi.server_entities[entID]);

		AddEffect(Owner, self);
		
		PriestessEffectStayAlive(self, Owner);
		break;

	case HP_STAFF_TRAIL:

		//Add trailing code here
		break;

	default:

		//Effect was passed with invalid effect modifier type
		assert(0);
		break;
	}
}
