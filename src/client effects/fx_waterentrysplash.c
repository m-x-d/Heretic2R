//
// fx_WaterEntrySplash.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Particle.h"
#include "Vector.h"
#include "Random.h"
#include "g_playstats.h"

#define WATER_SPLASH_SCALE	0.3f
#define WATER_SPLASH_RADIUS	20.0f

static struct model_s* water_models[2];

void PreCacheWaterSplash(void)
{
	water_models[0] = fxi.RegisterModel("sprites/fx/waterentryripple.sp2");
	water_models[1] = fxi.RegisterModel("sprites/fx/wfall.sp2");
}

static qboolean WaterEntrySplashThinkerThink(struct client_entity_s* self, centity_t* owner)
{
	// Have enough ripples been created yet? If not, create one, else free myself.
	if (self->NoOfAnimFrames < 1)
		return false;

	// Create a water entry ripple.
	const int flags = (int)(self->flags & ~(CEF_OWNERS_ORIGIN | CEF_NO_DRAW)); //mxd
	client_entity_t* ripple = ClientEntity_new(FX_WATER_ENTRYSPLASH, flags, self->origin, self->direction, 1200);

	ripple->r.model = &water_models[0]; // waterentryripple sprite.
	ripple->r.flags = RF_FIXED | RF_TRANSLUCENT | RF_ALPHA_TEXTURE;
	ripple->alpha = 0.6f;
	ripple->d_alpha = self->d_alpha;

	if (self->flags & CEF_FLAG7)
	{
		// Random ripples.
		ripple->r.scale = 0.1f;
		ripple->d_scale = 0.7f;
		ripple->r.origin[0] += flrand(-6.0f, 6.0f);
		ripple->r.origin[1] += flrand(-6.0f, 6.0f);
	}
	else
	{
		ripple->r.scale = WATER_SPLASH_SCALE;
		ripple->d_scale = 1.0f;
	}

	AddEffect(NULL, ripple);
	self->NoOfAnimFrames--;

	return true;
}

// ************************************************************************************************
// FXWaterEntrySplash
// ------------------
// ************************************************************************************************

void DoWaterEntrySplash(int Type,int Flags,vec3_t Origin, byte SplashSize, vec3_t Dir)
{
	byte			NoOfRipples;
	client_entity_t	*EntrySplashThinker,
					*splash;
	float				Theta,DeltaTheta;

	if (SplashSize >= 128)
		NoOfRipples = 4;
	else
	if (SplashSize >= 96)
		NoOfRipples = 3;
	else
	if (SplashSize >= 64)
		NoOfRipples = 2;
	else
		NoOfRipples = 1;


	if(Flags&CEF_FLAG6)
	{//they want a sound too
		fxi.S_StartSound(Origin, -1, CHAN_AUTO,
				fxi.S_RegisterSound(va("misc/splish%c.wav", irand('2', '3'))), 1, ATTN_STATIC, 0);
	}

	// Create a water entry ripple THINKER that will create the actual water entry ripples.

	EntrySplashThinker=ClientEntity_new(Type,Flags,Origin,Dir,200);
	
	EntrySplashThinker->flags|=CEF_NO_DRAW;
	EntrySplashThinker->NoOfAnimFrames=(int)NoOfRipples;
	EntrySplashThinker->Update=WaterEntrySplashThinkerThink;
	EntrySplashThinker->d_alpha=-0.8-(0.2-(0.2*((1.0/127.0)*(SplashSize&127))));

	AddEffect(NULL,EntrySplashThinker);	

	// Create water entry particle splash if required.

	if (SplashSize&128)
	{
		DeltaTheta=(2*M_PI)/12;
		for(Theta=0.0;Theta<(2*M_PI);Theta+=DeltaTheta)
		{
			splash = ClientEntity_new(Type,Flags,Origin,Dir,500);
			splash->r.model = water_models + 1;
			splash->r.scale = flrand(0.15, 0.25);
			splash->r.flags = RF_TRANSLUCENT;
			splash->r.frame = 1;
			splash->radius = 2.0;
			splash->d_alpha=-2.0;		
			splash->d_scale=1.0;

			splash->origin[0]=WATER_SPLASH_RADIUS*cos(Theta);
			splash->origin[1]=WATER_SPLASH_RADIUS*sin(Theta);
			splash->origin[2]=0.0;

			splash->velocity[0]=flrand(4.0,6.0)*splash->origin[0];
			splash->velocity[1]=flrand(4.0,6.0)*splash->origin[1];
			splash->velocity[2]=flrand(30.0,40.0);

			splash->acceleration[0]=0.0;
			splash->acceleration[1]=0.0;
			splash->acceleration[2]=-250;

			AddEffect(NULL, splash);
		}

		DeltaTheta=(2*M_PI)/6;
		for(Theta=0.0;Theta<(2*M_PI);Theta+=DeltaTheta)
		{
			splash = ClientEntity_new(Type,Flags,Origin,Dir,800);
			splash->r.model = water_models + 1;
			splash->r.scale = flrand(0.1, 0.2);
			splash->r.flags = RF_TRANSLUCENT;
			splash->r.frame = 1;
			splash->radius = 2.0;
			splash->d_alpha=-1.0;		
			splash->d_scale=-0.5;

			splash->origin[0]=WATER_SPLASH_RADIUS*cos(Theta);
			splash->origin[1]=WATER_SPLASH_RADIUS*sin(Theta);
			splash->origin[2]=0.0;

			splash->velocity[0]=flrand(1.0,2.0)*splash->origin[0];
			splash->velocity[1]=flrand(1.0,2.0)*splash->origin[1];
			splash->velocity[2]=flrand(100.0,150.0);

			splash->acceleration[0]=0.0;
			splash->acceleration[1]=0.0;
			splash->acceleration[2]=-525;

			AddEffect(NULL, splash);
		}

		// Create a water entry ripple.

		splash=ClientEntity_new(FX_WATER_ENTRYSPLASH, Flags&(~CEF_OWNERS_ORIGIN), Origin, Dir, 1200);

		splash->r.model = water_models;
		splash->r.scale=WATER_SPLASH_SCALE*2.0;
		splash->r.frame=0;
		splash->r.flags |= RF_FIXED|RF_TRANSLUCENT|RF_ALPHA_TEXTURE;
		splash->alpha=0.6;
		splash->d_scale=2.0;
		splash->d_alpha=EntrySplashThinker->d_alpha;
		
		AddEffect(NULL,splash);	
	}
	else if (Flags & CEF_FLAG7 && r_detail->value >= DETAIL_HIGH && Dir[2] >= 0.99)	// Horizontal splash
	{	// Add little splooshies
		int parts, i;
		client_particle_t *part;
		paletteRGBA_t color={0xff,0xff,0xff,0xff};
		
		// Create an extra, centered water entry ripple.
		splash=ClientEntity_new(FX_WATER_ENTRYSPLASH, Flags&(~CEF_OWNERS_ORIGIN), Origin, Dir, 1200);

		splash->r.model = water_models;
		splash->r.scale=WATER_SPLASH_SCALE;
		splash->r.frame=0;
		splash->r.flags |= RF_FIXED|RF_TRANSLUCENT|RF_ALPHA_TEXTURE;
		splash->alpha=0.6;
		splash->d_scale=1.0;
		splash->d_alpha=EntrySplashThinker->d_alpha;
		
		AddEffect(NULL,splash);	

		parts = SplashSize / 12;

		for (i=0; i<parts; i++)
		{
			part = ClientParticle_new(PART_32x32_WFALL, color, 750);
			VectorSet(part->origin, flrand(-2.0,2.0), flrand(-2.0,2.0), 0);
			VectorScale(part->origin, 8.0, part->velocity);
			part->velocity[2] = flrand(50.0, 100.0);
			part->acceleration[2] = -320;

			part->scale = 4.0;
			part->d_scale = -5.33;
			part->d_alpha = -1.3;

			AddParticleToList(splash, part);
		}
	}
	else
	{
		EntrySplashThinker->flags &= ~CEF_FLAG7;		// Remove that splooshy flag
	}
}


void FXWaterEntrySplash(centity_t *Owner,int Type,int Flags,vec3_t Origin)
{
	byte			SplashSize;
	vec3_t			Dir;

	fxi.GetEffect(Owner,Flags,clientEffectSpawners[FX_WATER_ENTRYSPLASH].formatString,&SplashSize,Dir);

	DoWaterEntrySplash(Type, Flags, Origin, SplashSize, Dir);
}