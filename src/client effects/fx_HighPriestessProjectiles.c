//
// fx_HighPriestessProjectiles.c
//
// Copyright 1998 Raven Software
//

#include "fx_HighPriestessProjectiles.h" //mxd
#include "Client Effects.h"
#include "Particle.h"
#include "Vector.h"
#include "Random.h"
#include "Utilities.h"
#include "ce_DLight.h"
#include "q_Sprite.h"
#include "g_playstats.h"

#define PRIESTESS_TELEPORT_LINEHEIGHT 764

enum
{
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

static struct model_s* hpproj_models[11];

void PreCacheHPMissile(void)
{
	// Projectile head sprites.
	hpproj_models[0] = fxi.RegisterModel("sprites/fx/hpproj1_1.sp2");
	hpproj_models[1] = fxi.RegisterModel("sprites/fx/hpproj1_2.sp2"); //TODO: unused?

	// Trail segments.
	hpproj_models[2] = fxi.RegisterModel("sprites/fx/segment_trail.sp2");
	hpproj_models[6] = fxi.RegisterModel("sprites/fx/segment_trail_y.sp2");
	hpproj_models[7] = fxi.RegisterModel("sprites/fx/segment_trail_wt.sp2");

	// Halos.
	hpproj_models[3] = fxi.RegisterModel("sprites/fx/hp_halo.sp2");
	hpproj_models[4] = fxi.RegisterModel("sprites/lens/halo1.sp2");

	// Light Bug model.
	hpproj_models[5] = fxi.RegisterModel("models/objects/lights/bug/tris.fm");

	// Rocks.
	hpproj_models[8] = fxi.RegisterModel("models/debris/stone/schunk1/tris.fm");
	hpproj_models[9] = fxi.RegisterModel("models/debris/stone/schunk2/tris.fm");

	// Lightning bolts.
	hpproj_models[10] = fxi.RegisterModel("sprites/fx/hp_lightning.sp2");
}

static qboolean FXHPTeleportLineThink(struct client_entity_s* self, centity_t* owner)
{
	if (self->alpha <= 0.0f)
		return false;

	self->r.endpos[2] += 32.0f;

	if (self->r.endpos[2] - self->r.startpos[2] > PRIESTESS_TELEPORT_LINEHEIGHT)
	{
		if (self->SpawnInfo == 0)
			fxi.Activate_Screen_Flash((int)color_white.c);

		self->SpawnInfo = 1;
		self->d_alpha = -1.0f;
		self->r.scale += 16.0f;

		return true;
	}

	self->r.scale += 1.5f;

	// Spawn some particles and some rock chunks.
	const int num_particles = GetScaledCount(8, 0.7f);

	for (int i = 0; i < num_particles; i++)
	{
		client_particle_t* p = ClientParticle_new(PART_4x4_WHITE, color_white, 2500);

		// Chance to make them brown.
		if (irand(0, 1))
		{
			const byte col = (byte)irand(0, 64);
			COLOUR_SET(p->color, col, col, 0); //mxd
		}
		else
		{
			const byte col = (byte)irand(32, 128);
			COLOUR_SET(p->color, col, col, col); //mxd
		}

		p->origin[0] += flrand(-24.0f, 24.0f); //mxd. irand() in original version. Why?
		p->origin[1] += flrand(-24.0f, 24.0f); //mxd. irand() in original version. Why?
		p->origin[2] -= 36;

		p->scale = flrand(0.25f, 1.0f);

		p->acceleration[0] = flrand(-75.0f, 75.0f); //mxd. irand() in original version. Why?
		p->acceleration[1] = flrand(-75.0f, 75.0f); //mxd. irand() in original version. Why?
		p->acceleration[2] = flrand(400.0f, 500.0f); //mxd. irand() in original version. Why?

		p->d_alpha = -2.0f;

		AddParticleToList(self, p);
	}

	const int num_rocks = GetScaledCount(5, 0.7f);

	for (int i = 0; i < num_rocks; i++)
	{
		client_entity_t* rock = ClientEntity_new(FX_HP_MISSILE, CEF_DONT_LINK, self->origin, NULL, 1000);

		rock->radius = 500.0f;
		rock->r.model = &hpproj_models[irand(8, 9)];
		rock->r.scale = flrand(0.1f, 0.5f);

		VectorCopy(self->r.origin, rock->r.origin);
		rock->r.origin[0] += flrand(-32.0f, 32.0f); //mxd. irand() in original version. Why?
		rock->r.origin[1] += flrand(-32.0f, 32.0f); //mxd. irand() in original version. Why?
		rock->r.origin[2] -= 36.0f;

		rock->velocity[2] = 1.0f;

		rock->acceleration[0] = flrand(-150.0f, 150.0f); //mxd. irand() in original version. Why?
		rock->acceleration[1] = flrand(-150.0f, 150.0f); //mxd. irand() in original version. Why?
		rock->acceleration[2] = flrand(300.0f, 600.0f - (self->r.scale * 100.0f)); //mxd. irand() in original version. Why?

		rock->d_alpha = -0.25f;

		AddEffect(NULL, rock);
	}

	return true;
}

static qboolean FXHPTeleportLineThink2(struct client_entity_s* self, centity_t* owner)
{
	if (self->alpha > 0.0f && self->r.scale > 0.0f)
	{
		self->r.scale -= 4.0f;
		return true;
	}

	return false;
}

static qboolean FXHPMissileSpawnerThink(const struct client_entity_s* self, centity_t* owner)
{
	if (self->LifeTime < fxi.cl->time)
		return false;

	client_entity_t* trail = ClientEntity_new(FX_HP_MISSILE, CEF_DONT_LINK, self->origin, NULL, 1000);

	trail->radius = 500.0f;
	trail->r.flags = RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	trail->r.model = &hpproj_models[3]; // Halo sprite.
	trail->r.scale = 0.1f;

	trail->d_alpha = -2.5f;
	trail->d_scale = 4.0f;

	VectorCopy(self->origin, trail->origin);
	VectorRandomSet(trail->velocity, 16.0f); //mxd. Set using irand() in original version. Why?

	AddEffect(NULL, trail);

	return true;
}

static qboolean FXHPMissileSpawnerThink2(const struct client_entity_s* self, centity_t* owner)
{
	if (self->LifeTime < fxi.cl->time)
		return false;

	client_entity_t* trail = ClientEntity_new(FX_HP_MISSILE, CEF_DONT_LINK, self->origin, NULL, 1000);

	trail->radius = 500.0f;
	trail->r.flags = RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	trail->r.model = &hpproj_models[4];
	COLOUR_SET(trail->r.color, 229, 250, 88);

	trail->alpha = 0.5f;
	trail->r.scale = 0.25f;

	trail->d_alpha = -2.5f;
	trail->d_scale = 2.0f;

	VectorCopy(self->origin, trail->origin);
	VectorRandomSet(trail->velocity, 16.0f); //mxd. Set using irand() in original version. Why?

	AddEffect(NULL, trail);

	return true;
}

static qboolean FXHPHaloDie(const struct client_entity_s* self, centity_t* owner)
{
	return (self->r.scale > 0.0f && self->alpha > 0.0f);
}

static qboolean FXHPMissileSpawnerThink3(struct client_entity_s* self, centity_t* owner)
{
	if (self->LifeTime < fxi.cl->time)
	{
		self->Update = FXHPHaloDie;
		self->d_scale = -2.0f;
		self->d_alpha = -1.0f;

		return true;
	}

	if (self->d_scale == 0.0f)
	{
		self->r.scale = flrand(1.75f, 2.25f);
		return true;
	}

	if (self->r.scale >= 2.0f)
		self->d_scale = 0.0f;

	if (self->alpha > 0.5f)
	{
		self->d_alpha = 0.0f;
		self->alpha = 0.5f;
	}

	return true;
}

static qboolean FXHPTrailThink(struct client_entity_s* self, centity_t* owner)
{
	if (self->alpha > 0.1f && self->r.scale > 0.0f)
	{
		self->r.scale -= 0.1f;
		return true;
	}

	return false;
}

static qboolean FXHPTrailThink2(struct client_entity_s* self, centity_t* owner)
{
	if (self->alpha > 0.1f && self->r.scale > 0.0f)
	{
		self->r.scale -= 0.15f;
		return true;
	}

	return false;
}

static qboolean FXHPTrailThink3(struct client_entity_s* self, centity_t* owner)
{
	if (self->alpha > 0.1f && self->r.scale > 0.0f)
	{
		self->r.scale -= 0.25f;
		return true;
	}

	return false;
}

static qboolean FXHPBugThink(struct client_entity_s* self, centity_t* owner)
{
	self->r.scale = flrand(0.2f, 0.4f);
	self->alpha = flrand(0.3f, 0.5f);

	return true;
}

static qboolean FXHPMissileTrailThink(struct client_entity_s* self, const centity_t* owner)
{
	self->r.scale = flrand(0.35f, 0.65f);

	client_entity_t* trail = ClientEntity_new(FX_HP_MISSILE, CEF_DONT_LINK, owner->origin, NULL, 17);

	trail->radius = 500.0f;

	VectorCopy(owner->origin, trail->origin);

	trail->r.flags = RF_TRANSLUCENT | RF_TRANS_ADD_ALPHA;
	trail->r.model = &hpproj_models[2]; // Segment trail sprite.

	trail->r.spriteType = SPRITE_LINE;
	trail->r.tile = 1.0f;
	trail->r.scale = 2.5f;

	VectorCopy(self->startpos, trail->r.startpos);
	VectorCopy(owner->origin, trail->r.endpos);

	trail->d_alpha = -2.5f;
	trail->Update = FXHPTrailThink;

	AddEffect(NULL, trail);

	VectorCopy(owner->origin, self->startpos);

	return true;
}

static qboolean FXHPMissileTrailThink2(struct client_entity_s* self, const centity_t* owner)
{
	self->r.scale = flrand(0.35f, 0.55f);

	client_entity_t* trail = ClientEntity_new(FX_HP_MISSILE, CEF_DONT_LINK, owner->origin, NULL, 17);

	trail->radius = 500.0f;

	VectorCopy(owner->origin, trail->origin);

	trail->r.flags = RF_TRANSLUCENT | RF_TRANS_ADD_ALPHA;
	trail->r.model = &hpproj_models[2]; // Segment trail sprite.

	trail->r.spriteType = SPRITE_LINE;
	trail->r.tile = 1.0f;
	trail->r.scale = 2.0f; //mxd. BUGFIX: set to 2.0, then set to 1.0 below in original version.

	VectorCopy(self->startpos, trail->r.startpos);
	VectorCopy(owner->origin, trail->r.endpos);

	trail->d_alpha = -4.0f;
	trail->d_scale = 0.0f;
	trail->Update = FXHPTrailThink2;

	AddEffect(NULL, trail);

	VectorCopy(owner->origin, self->startpos);

	return true;
}

static qboolean FXHPMissileTrailThink3(struct client_entity_s* self, const centity_t* owner)
{
	client_entity_t* trail = ClientEntity_new(FX_HP_MISSILE, CEF_DONT_LINK, owner->origin, NULL, 17);

	trail->radius = 500.0f;

	VectorCopy(owner->origin, trail->origin);

	trail->r.flags = RF_TRANSLUCENT | RF_TRANS_ADD_ALPHA;
	trail->r.model = &hpproj_models[6]; // Yellow segment trail sprite.

	trail->r.spriteType = SPRITE_LINE;
	trail->r.tile = 1.0f;
	trail->r.scale = 2.0f; //mxd. BUGFIX: set to 2.0, then set to 1.0 below in original version.

	VectorCopy(self->startpos, trail->r.startpos);
	VectorCopy(owner->origin, trail->r.endpos);

	trail->d_alpha = -2.0f;
	trail->d_scale = 0.0f;
	trail->Update = FXHPTrailThink3;

	AddEffect(NULL, trail);

	VectorCopy(owner->origin, self->startpos);

	return true;
}

static void FXHPMissileExplode(struct client_entity_s* self, const centity_t* owner)
{
	const vec3_t dir = { 1.0f, 1.0f, 1.0f };
	const int count = GetScaledCount(irand(6, 8), 0.8f);

	for (int i = 0; i < count; i++)
	{
		const int next_think_time = (i == count - 1 ? 500 : 1000); //mxd
		client_entity_t* smoke_puff = ClientEntity_new(FX_HP_MISSILE, 0, owner->origin, NULL, next_think_time);

		smoke_puff->r.model = &hpproj_models[3]; // Halo sprite.
		smoke_puff->r.scale = flrand(0.5f, 1.0f);
		smoke_puff->d_scale = flrand(-1.0f, -1.5f);

		smoke_puff->r.flags = RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;

		VectorRandomCopy(dir, smoke_puff->velocity, flrand(64.0f, 128.0f));
		VectorSet(smoke_puff->acceleration, flrand(-100.0f, -50.0f), flrand(-100.0f, -50.0f), flrand(-250.0f, -100.0f));

		smoke_puff->d_alpha = -0.2f;
		smoke_puff->radius = 20.0f;

		AddEffect(NULL, smoke_puff);
	}
}

static void FXHPBugExplode(struct client_entity_s* self, const centity_t* owner)
{
	const vec3_t dir = { 1.0f, 1.0f, 1.0f };
	const int count = GetScaledCount(irand(12, 16), 0.8f);

	for (int i = 0; i < count; i++)
	{
		const int next_think_time = (i == count - 1 ? 500 : 1000); //mxd
		client_entity_t* smoke_puff = ClientEntity_new(FX_HP_MISSILE, 0, owner->origin, NULL, next_think_time);

		smoke_puff->r.model = &hpproj_models[0]; // Projectile head sprite.
		smoke_puff->r.scale = flrand(0.5f, 1.0f);
		smoke_puff->d_scale = -2.0f;

		smoke_puff->r.flags = RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;

		VectorRandomCopy(dir, smoke_puff->velocity, flrand(16.0f, 64.0f));
		VectorSet(smoke_puff->acceleration, flrand(-200.0f, 200.0f), flrand(-200.0f, 200.0f), flrand(-100.0f, -60.0f));

		smoke_puff->d_alpha = -0.4f;
		smoke_puff->radius = 20.0f;

		AddEffect(NULL, smoke_puff);
	}
}

void FXHPMissileCreateWarp(const int type, const vec3_t origin)
{
	client_entity_t* halo = ClientEntity_new(type, CEF_DONT_LINK, origin, NULL, 2000);

	halo->radius = 500.0f;
	halo->r.model = &hpproj_models[3]; // Halo sprite.
	halo->r.color.c = 0xffff5555;
	halo->r.flags = RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	halo->r.scale = 0.1f;
	halo->d_scale = 2.0f;
	halo->d_alpha = -2.0f;

	AddEffect(NULL, halo);
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

		FXHPMissileCreateWarp(Type, Origin);

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

		FXHPMissileCreateWarp(Type, Origin);

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