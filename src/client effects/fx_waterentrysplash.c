//
// fx_waterentrysplash.c
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
#include "Random.h"
#include "g_playstats.h"

#define	NUM_SPLASH_MODELS	2
static struct model_s *water_models[NUM_SPLASH_MODELS];
void PreCacheWaterSplash()
{
	water_models[0] = fxi.RegisterModel("sprites/fx/waterentryripple.sp2");
	water_models[1] = fxi.RegisterModel("sprites/fx/wfall.sp2");
}

// -----------------------------------------------------------------

#define	SCALE 0.3
#define SPLASH_RADIUS 20

static qboolean FXWaterEntrySplashThinkerThink(struct client_entity_s *Self,centity_t *Owner);
static void WaterEntryParticles(struct client_entity_s *Owner, float Radius, int NumParticles);

// ************************************************************************************************
// FXWaterEntryRippleThinkerThink
// ------------------------------
// ************************************************************************************************

static qboolean FXWaterEntrySplashThinkerThink(struct client_entity_s *Self,centity_t *Owner)
{
	client_entity_t	*EntryRipple;

	// Have enough ripples been created yet? If not, create one, else free myself.

	if(Self->NoOfAnimFrames>0)
	{ 
		// Create a water entry ripple.

		EntryRipple=ClientEntity_new(FX_WATER_ENTRYSPLASH,
									 Self->flags&~(CEF_OWNERS_ORIGIN|CEF_NO_DRAW),
									 Self->origin,
									 Self->direction,
									 1200);

		EntryRipple->r.model = water_models;
		EntryRipple->r.frame=0;
		EntryRipple->r.flags=RF_FIXED | RF_TRANSLUCENT | RF_ALPHA_TEXTURE;
		EntryRipple->alpha=0.6;
		EntryRipple->d_alpha=Self->d_alpha;

		if (Self->flags & CEF_FLAG7)
		{	// Random ripples.
			EntryRipple->r.scale=0.1;
			EntryRipple->d_scale=0.7;
			EntryRipple->r.origin[0] += flrand(-6.0,6.0);
			EntryRipple->r.origin[1] += flrand(-6.0,6.0);
		}
		else
		{
			EntryRipple->r.scale=SCALE;
			EntryRipple->d_scale=1.0;
		}
		
		AddEffect(NULL,EntryRipple);	
		
		Self->NoOfAnimFrames--;
		return(true);
	}
	else
		return(false);
}

// ************************************************************************************************
// WaterEntryParticles
// -------------------
// ************************************************************************************************

static void WaterEntryParticles(struct client_entity_s *Owner,float Radius,int NumParticles)
{
	float				Theta,DeltaTheta;
	client_particle_t	*Particle;
	paletteRGBA_t		color;

	if (NumParticles<1||NumParticles>500)
		return;
	DeltaTheta=(2*M_PI)/NumParticles;

	for(Theta=0.0;Theta<(2*M_PI);Theta+=DeltaTheta)
	{		
		color.c=0xffffffff;
		color.a = irand(128, 200);

		Particle=ClientParticle_new(PART_4x4_WHITE,color,1000);

		Particle->origin[0]=Radius*cos(Theta);
		Particle->origin[1]=Radius*sin(Theta);
		Particle->origin[2]=0.0;

		Particle->velocity[0]=flrand(0.0,50.0)*Particle->origin[0]*0.05;
		Particle->velocity[1]=flrand(0.0,50.0)*Particle->origin[1]*0.05;
		Particle->velocity[2]=flrand(50.0,120.0);

		Particle->acceleration[0]=0.0;
		Particle->acceleration[1]=0.0;
		Particle->acceleration[2]=-PARTICLE_GRAVITY*3.5;

		Particle->scale=flrand(0.5, 0.7);
		Particle->d_alpha=-255.0;		
		Particle->d_scale=-0.25;

		AddParticleToList(Owner,Particle);
	}
}

// ************************************************************************************************
// FXWaterEntrySplash
// ------------------
// ************************************************************************************************

void FXDoWaterEntrySplash(centity_t *Owner,int Type,int Flags,vec3_t Origin, byte SplashSize, vec3_t Dir)
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
	EntrySplashThinker->Update=FXWaterEntrySplashThinkerThink;
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

			splash->origin[0]=SPLASH_RADIUS*cos(Theta);
			splash->origin[1]=SPLASH_RADIUS*sin(Theta);
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

			splash->origin[0]=SPLASH_RADIUS*cos(Theta);
			splash->origin[1]=SPLASH_RADIUS*sin(Theta);
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
		splash->r.scale=SCALE*2.0;
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
		splash->r.scale=SCALE;
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

	FXDoWaterEntrySplash(Owner, Type, Flags, Origin, SplashSize, Dir);
}