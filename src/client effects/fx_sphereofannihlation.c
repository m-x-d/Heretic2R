//
// fx_sphereofannihilation.c
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
#include "Matrix.h"
#include "ce_DLight.h"
#include "Random.h"
#include "Utilities.h"
#include "Reference.h"
#include "q_Sprite.h"
#include "g_playstats.h"

#define	NUM_SPHERE_MODELS	8

static struct model_s *sphere_models[NUM_SPHERE_MODELS];

void PreCacheSphere()
{
	sphere_models[0] = fxi.RegisterModel("sprites/spells/shboom.sp2");
	sphere_models[1] = fxi.RegisterModel("sprites/spells/bluball.sp2");
	sphere_models[2] = fxi.RegisterModel("sprites/spells/glowball.sp2");
	sphere_models[3] = fxi.RegisterModel("models/spells/sphere/tris.fm");
	sphere_models[4] = fxi.RegisterModel("sprites/fx/halo.sp2");
	sphere_models[5] = fxi.RegisterModel("sprites/spells/glowbeam.sp2");
	sphere_models[6] = fxi.RegisterModel("sprites/fx/haloblue.sp2");
	sphere_models[7] = fxi.RegisterModel("sprites/spells/spark_blue.sp2");
}

// --------------------------------------------------------------

#define FX_SPHERE_FLY_SPEED				600.0
#define	FX_SOFT_SPHERE_AURA_SCALE			0.9
#define	FX_SPHERE_AURA_SCALE			1.2
#define FX_SPHERE_EXPLOSION_BASE_RADIUS	89.0

static qboolean FXSphereOfAnnihilationSphereThink(struct client_entity_s *Self,centity_t *Owner);
static qboolean FXSphereOfAnnihilationAuraElementThink(struct client_entity_s *Self,centity_t *Owner);
static qboolean FXSphereOfAnnihilationAuraThink(struct client_entity_s *Self,centity_t *Owner);
static qboolean FXSphereOfAnnihilationGlowballSparkThink(struct client_entity_s *Self,centity_t *Owner);
static qboolean FXSphereOfAnnihilationGlowballThink(struct client_entity_s *Self,centity_t *Owner);
static qboolean FXSphereOfAnnihilationGlowballSpawnerThink(struct client_entity_s *Self,centity_t *Owner);
static qboolean FXSphereOfAnnihilationSmokePuffThink(struct client_entity_s *Self,centity_t *Owner);

extern void FXClientLensFlare(centity_t *owner,int Type,int Flags,vec3_t Origin, int lifeTime, paletteRGBA_t *tint);


// ****************************************************************************
// FXSphereOfAnnihilationSphereThink -
// ****************************************************************************

static qboolean FXSphereOfAnnihilationSphereThink(struct client_entity_s *Self,centity_t *Owner)
{
	float		detail_scale;
	if(r_detail->value == DETAIL_LOW)
		detail_scale = 0.7;
	else
	if(r_detail->value == DETAIL_NORMAL)
		detail_scale = 0.85;
	else
		detail_scale = 1.0;

	Self->r.scale=Owner->current.scale * detail_scale;

	return(true);
}

// ****************************************************************************
// FXSphereOfAnnihilationAuraThink -
// ****************************************************************************

static qboolean FXSphereOfAnnihilationAuraThink(struct client_entity_s *Self,centity_t *Owner)
{
	vec3_t			TrailStart,Trail;
	float			TrailLength,DeltaTrailLength;
	vec3_t			Right,Up;
	client_entity_t	*TrailEnt;
	int				i;
	int				dur;

	//
	// no aura trail on low level
	if(r_detail->value == DETAIL_LOW)
		return(true);

	VectorCopy(Owner->origin,TrailStart);
	VectorSubtract(Owner->lerp_origin,Owner->origin,Trail);

	// copy the real origin to the entity origin, so the light will follow us
	VectorCopy(Self->r.origin, Self->origin);

	if((TrailLength=VectorNormalize(Trail))<0.001)
	{
		TrailLength+=2.0;
	}
	
	PerpendicularVector(Right,Trail);
	CrossProduct(Trail,Right,Up);

	DeltaTrailLength=FX_SPHERE_FLY_SPEED;
	VectorScale(Trail,FX_SPHERE_FLY_SPEED,Trail);

	if(r_detail->value == DETAIL_NORMAL)
		dur = 400;
	else
		dur = 500;

	i=0;
	while(TrailLength>0.0)
	{
		i++;
		if (i>=40)
			break;
		//

		TrailEnt=ClientEntity_new(FX_WEAPON_SPHERE,
								  Self->flags&~(CEF_OWNERS_ORIGIN|CEF_NO_DRAW),
								  TrailStart,
								  NULL,
								  dur);

		TrailEnt->r.model = sphere_models;
		TrailEnt->r.flags=RF_TRANSLUCENT|RF_TRANS_ADD|RF_TRANS_ADD_ALPHA;
		if (r_detail->value < DETAIL_NORMAL)
			TrailEnt->Scale=FX_SOFT_SPHERE_AURA_SCALE+flrand(0.0, 0.1);
		else
			TrailEnt->Scale=FX_SPHERE_AURA_SCALE+flrand(0.0, 0.1);
		TrailEnt->color.r=irand(128, 180);
		TrailEnt->color.g=irand(128, 180);
		TrailEnt->color.b=irand(64, 80);
		TrailEnt->alpha = 0.7;
		TrailEnt->d_alpha = -1.0;
		if (r_detail->value < DETAIL_NORMAL)
			TrailEnt->d_scale = -1.0;
		else
			TrailEnt->d_scale = -0.5;

		TrailEnt->radius=70.0;
				
		AddEffect(NULL,TrailEnt);

		TrailLength-=DeltaTrailLength;
		
		VectorAdd(TrailStart,Trail,TrailStart);
	}

	return(true);
}

// ****************************************************************************
// FXSphereOfAnnihilation -
// ****************************************************************************

void FXSphereOfAnnihilation(centity_t *Owner,int Type,int Flags,vec3_t Origin)
{
	client_entity_t	*SphereThinker,
					*AuraThinker;
	short			CasterEntnum;
	paletteRGBA_t	LightColor={0,0,255,255};
	int				caster_update;

	fxi.GetEffect(Owner,Flags,clientEffectSpawners[FX_WEAPON_SPHERE].formatString,&CasterEntnum);

	// Create a fiery blue aura around the sphere.

	if (r_detail->value < DETAIL_NORMAL)
		caster_update = 125;
	else
		caster_update = 100;

	AuraThinker=ClientEntity_new(Type,Flags,Origin,NULL,caster_update);

	AuraThinker->flags|=CEF_NO_DRAW;
	if (r_detail->value != DETAIL_LOW)
		AuraThinker->dlight=CE_DLight_new(LightColor,150.0,0.0);
	AuraThinker->Update=FXSphereOfAnnihilationAuraThink;
	AuraThinker->extra=(void *)(&fxi.server_entities[CasterEntnum]);// The caster's centity_t.
	AuraThinker->AddToView = LinkedEntityUpdatePlacement;

	AddEffect(Owner,AuraThinker);

	FXSphereOfAnnihilationAuraThink(AuraThinker,Owner);

	// Create the sphere of annihilation itself.

	SphereThinker = ClientEntity_new(Type, Flags, Origin, NULL, 100);

	SphereThinker->r.model = sphere_models + 1;
	SphereThinker->r.flags |= RF_TRANSLUCENT;
	SphereThinker->r.scale = Owner->current.scale;
	SphereThinker->radius = 70.0;
	SphereThinker->Update = FXSphereOfAnnihilationSphereThink;
	SphereThinker->AddToView = LinkedEntityUpdatePlacement;
	
	AddEffect(Owner, SphereThinker);
}

// ****************************************************************************
// FXSphereOfAnnihilationGlowballThink -
// ****************************************************************************

static qboolean FXSphereOfAnnihilationGlowballThink(struct client_entity_s *Self,centity_t *Owner)
{
	client_entity_t	*Spark;
	int			dur;

	if((Owner->current.effects&EF_MARCUS_FLAG1))
		Self->color.r++;

	if(r_detail->value == DETAIL_LOW)
		dur = 300;
	else
	if(r_detail->value == DETAIL_NORMAL)
		dur = 400;
	else
		dur = 500;


	if(Self->color.r>3)
	{
		// Create a trailing spark.

		Spark=ClientEntity_new(FX_WEAPON_SPHERE,
							   Self->flags&~(CEF_OWNERS_ORIGIN),
							   Self->r.origin,
							   NULL,
							   dur);
		
		Spark->r.model = sphere_models + 2;
		Spark->r.flags=RF_TRANSLUCENT|RF_TRANS_ADD;
		Spark->r.color.r=irand(128, 180);
		Spark->r.color.g=irand(128, 180);
		Spark->r.color.b=irand(180, 255);
		Spark->Scale=flrand(0.8, 1.0);
		Spark->radius=20.0;
		Spark->d_scale = -1.5;
		
		AddEffect(NULL,Spark);
	}

	// 'Self->extra' refers to the sphere's centity_t.

	if(Self->color.r<16)
	{
		Self->velocity[0]*=3.0;
		Self->velocity[0]+=6.0*(Owner->origin[0]-Self->r.origin[0]);
		Self->velocity[0]*=0.265;

		Self->velocity[1]*=3.0;
		Self->velocity[1]+=6.0*(Owner->origin[1]-Self->r.origin[1]);
		Self->velocity[1]*=0.265;
		
		Self->velocity[2]*=3.0;
		Self->velocity[2]+=6.0*(Owner->origin[2]-Self->r.origin[2]);
		Self->velocity[2]*=0.265;

		return(true);
	}
	else
	{	
		return(false);
	}
}

// ****************************************************************************
// FXSphereOfAnnihilationGlowballSpawnerThink -
// ****************************************************************************

static qboolean FXSphereOfAnnihilationGlowballSpawnerThink(struct client_entity_s *Self,centity_t *Owner)
{
	client_entity_t	*Glowball;
	centity_t		*controller;
	vec3_t			Forward,Right,
					Forward2,Right2,
					Temp,
					temp_origin;
	matrix3_t		RotationMatrix;

	// This tells if we are wasting our time, because the reference points are culled.
	if (Self->extra && !RefPointsValid((centity_t *)(Self->extra)))	// Only if we were SUPPOSED to have refpoints.
		return true;

	// If the spell is still building, create some swirling blue Glowballs.

	if(Owner->current.effects&EF_MARCUS_FLAG1)
	{	
		// 'Self->extra' refers to the caster's centity_t.

		if (Self->extra)
			VectorCopy(((centity_t *)(Self->extra))->origin, temp_origin);
		else
			VectorCopy(Self->r.origin, temp_origin);

		Glowball=ClientEntity_new(FX_WEAPON_SPHEREGLOWBALLS,
								  Self->flags&~(CEF_NO_DRAW|CEF_OWNERS_ORIGIN),
								  temp_origin,
								  NULL,
								  50);

		Glowball->flags|=CEF_DONT_LINK;
		
		// Make me spawn from my caster's left / right hands (alternating).
		// assuming we aren't a reflection type glowball
		if (Self->extra)
		{
			controller = ((centity_t *)(Self->extra));
			VectorCopy(controller->current.angles,Temp);
			VectorScale(Temp,180.0/M_PI,Temp);
			AngleVectors(Temp,Forward,Right,NULL);

			Matrix3FromAngles(controller->lerp_angles,RotationMatrix);

			if(Self->SpawnInfo)
			{
				if(!(Self->color.g&1))
					Matrix3MultByVec3(RotationMatrix,
									  controller->referenceInfo->references[CORVUS_LEFTHAND].placement.origin,
									  Glowball->r.origin);
				else
					Matrix3MultByVec3(RotationMatrix,
									  controller->referenceInfo->references[CORVUS_RIGHTHAND].placement.origin,
									  Glowball->r.origin);
			}
			else
			{
				vec3_t fwd_ofs;

				VectorScale(Forward, 16, fwd_ofs);//Hard-coded for Celestial Watcher(monster_elflord), 
				Matrix3MultByVec3(RotationMatrix,
								  fwd_ofs,
								  Glowball->r.origin);
			}

			VectorAdd(controller->origin,Glowball->r.origin,Glowball->r.origin);
		}
		else
		{
			VectorCopy(Self->r.angles,Temp);
			VectorScale(Temp,180.0/M_PI,Temp);
			AngleVectors(Temp,Forward,Right,NULL);


			VectorSet(Glowball->r.origin,	flrand(-10.0, 10.0)+ Self->r.origin[0],
											flrand(-10.0, 10.0)+ Self->r.origin[1],
											flrand(-10.0, 10.0)+ Self->r.origin[2]);
		}

	   	VectorCopy(Owner->current.angles,Temp);
	   	VectorScale(Temp,180.0/M_PI,Temp);
	   	AngleVectors(Temp,Forward2,Right2,NULL);


		// Set my velocity and accelaration.

		Glowball->velocity[0]=Forward2[0]*175.0+flrand(-25.0, 25.0);

		if(Self->color.g&1)
			Glowball->velocity[0]=-Glowball->velocity[0];

		Glowball->velocity[1]=Forward2[1]*175.0+flrand(-25.0, 25.0);

		if(!(Self->color.g&1))
			Glowball->velocity[1]=-Glowball->velocity[1];
		
		Glowball->velocity[2]=flrand(-200.0, 100.0);

		VectorClear(Glowball->acceleration);

		// Fill in the rest of my info.
		
		Glowball->r.model = sphere_models + 2;
		Glowball->r.flags=RF_TRANSLUCENT|RF_TRANS_ADD;
		Glowball->r.color.r= irand(128, 180);
		Glowball->r.color.g= irand(128, 180);
		Glowball->r.color.b= irand(180, 255);
		Glowball->color.r=1;
		Glowball->radius=20.0;
		Glowball->extra=(void *)Owner;
		Glowball->Update=FXSphereOfAnnihilationGlowballThink;

		AddEffect(Owner,Glowball);

		FXSphereOfAnnihilationGlowballThink(Glowball,Owner);

		Self->color.g++;
	}

	return(true);
}

// ****************************************************************************
// FXSphereOfAnnihilationGlowballs -
// ****************************************************************************

void FXSphereOfAnnihilationGlowballs(centity_t *Owner,int Type,int Flags,vec3_t Origin)
{
	client_entity_t	*GlowballSpawner;
	short			CasterEntnum;
	int				caster_update;

	// Get the caster's centity_t.

	fxi.GetEffect(Owner,Flags,clientEffectSpawners[FX_WEAPON_SPHEREGLOWBALLS].formatString,&CasterEntnum);

	// Create a spawner that will create the glowballs.

	if ((r_detail->value >= DETAIL_NORMAL))
		caster_update = 100;
	else
		caster_update = 250;

	GlowballSpawner = ClientEntity_new(Type, Flags | CEF_VIEWSTATUSCHANGED, Origin, NULL, caster_update);

	GlowballSpawner->flags|=CEF_NO_DRAW;
	GlowballSpawner->color.g=0;
	GlowballSpawner->Update = FXSphereOfAnnihilationGlowballSpawnerThink;
	GlowballSpawner->AddToView = LinkedEntityUpdatePlacement;

	if(Flags&CEF_FLAG6)
		GlowballSpawner->SpawnInfo = false;
	else
		GlowballSpawner->SpawnInfo = true;

	if (CasterEntnum == -1)
		GlowballSpawner->extra=NULL;
	else
		GlowballSpawner->extra=(void *)(&fxi.server_entities[CasterEntnum]);

	AddEffect(Owner,GlowballSpawner);
}

// ****************************************************************************
// FXSphereOfAnnihilationSphereExplodeThink -
// ****************************************************************************

static qboolean FXSphereOfAnnihilationSphereExplodeThink(struct client_entity_s *Self,centity_t *Owner)
{
	float	Frac,
			Multiplier;
	int		FrameNo;

	Frac=(fxi.cl->time-Self->startTime)/100.0;
	if(Self->AnimSpeed>0.0)
	{
		Frac*=Self->AnimSpeed;
	}

	if((FrameNo=floor(Frac))>=(Self->NoOfAnimFrames-1))
	{
		return(false);
	}
	else
	{
		// Spin the ball of blue fire whilst it expands and fades.

		Self->r.angles[0]+=(M_PI/32.0);

		Self->r.angles[1]+=(M_PI/27.0);

		Self->radius=FX_SPHERE_EXPLOSION_BASE_RADIUS*Self->r.scale;
		
		Multiplier=1.0-Frac/(Self->NoOfAnimFrames-1);

		Self->dlight->intensity=(Self->radius/0.7);

		Self->dlight->color.r=255*Multiplier;
		Self->dlight->color.g=255*Multiplier;
		Self->dlight->color.b=255*Multiplier;

		Self->alpha=Multiplier;
		
		return(true);
	}
}

#define SMOKE_SPEED 140

// ****************************************************************************
// FXSphereOfAnnihilationExplode -
// ****************************************************************************
void FXSphereOfAnnihilationExplode(centity_t *Owner, int Type, int Flags, vec3_t Origin)
{
	vec3_t				Dir;
	byte				Size;
	client_entity_t		*Explosion;
	paletteRGBA_t		LightColor={255,255,255,255};
	int					I, count;
	client_particle_t	*ce;

	fxi.GetEffect(Owner,Flags,clientEffectSpawners[FX_WEAPON_SPHEREEXPLODE].formatString,Dir,&Size);
	if(Flags & CEF_FLAG6)
	{
		FXClientScorchmark(Origin, Dir);
	}
	// Create an expanding ball of blue fire.
	Explosion=ClientEntity_new(Type,Flags | CEF_ADDITIVE_PARTS,Origin,NULL,50);

	Explosion->r.model = sphere_models + 3;
	Explosion->r.flags=RF_FULLBRIGHT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	Explosion->r.scale=0.01;
	Explosion->color.c=0xffffffff;
	Explosion->d_scale=2.5;
	Explosion->NoOfAnimFrames=(int)Size;
	Explosion->AnimSpeed=1.0;
	Explosion->radius=FX_SPHERE_EXPLOSION_BASE_RADIUS*Explosion->r.scale;
	Explosion->dlight=CE_DLight_new(LightColor,Explosion->radius/0.7,0);	
	Explosion->Update=FXSphereOfAnnihilationSphereExplodeThink;
	
	AddEffect(NULL,Explosion);

	FXSphereOfAnnihilationSphereExplodeThink(Explosion,NULL);
	// Add some glowing blast particles.

	VectorScale(Dir,SMOKE_SPEED,Dir);

	count = GetScaledCount(40, 0.3);

	for(I=0;I<count;I++)
	{

		// No more tinting these particles.  It wasn't really worth it anyway
/*		LightColor.r=irand(25, 50);
		LightColor.g=irand(25, 50);
		LightColor.b=irand(200, 255);
		*/

		ce = ClientParticle_new(PART_16x16_SPARK_B, LightColor, 600);

		VectorCopy(Dir,ce->velocity);
		
		ce->scale=flrand(16.0, 32.0);

		ce->velocity[0]+=flrand(-SMOKE_SPEED,SMOKE_SPEED);
		ce->velocity[1]+=flrand(-SMOKE_SPEED,SMOKE_SPEED);
		ce->velocity[2]+=flrand(-SMOKE_SPEED,SMOKE_SPEED);
	 	AddParticleToList(Explosion, ce);

	}
}

void FXSphereOfAnnihilationPower(centity_t *Owner,int Type,int Flags,vec3_t Origin)
{
	vec3_t				dir;
	byte					size;
	client_entity_t		*exp1, *beam;
	paletteRGBA_t		LightColor={255,255,255,255};
	int					i;
	client_particle_t	*ce;
	vec3_t				spot1;
	vec3_t				tempSpot;
	vec3_t				fwd, right, up;
	vec3_t				ang;
	byte				len2;
	int					len;
	int					count;

	fxi.GetEffect(Owner,Flags,clientEffectSpawners[FX_WEAPON_SPHEREPOWER].formatString,dir,&size, &len2);

	len = len2*8;// shrunk down so range can be up to 2048

	// if there is a cheaper way to get ACCURATE right and up, I'd be happy to see it...
	vectoangles(dir, ang);
	ang[PITCH] *= -1;// something's broken with angle signs somewhere ;(
	AngleVectors(ang, fwd, right, up);

	// Only one beam
	VectorCopy(Origin, spot1);

	// make the flares at the start
	exp1 = ClientEntity_new(Type,Flags | CEF_ADDITIVE_PARTS, spot1, NULL, 500);
	exp1->r.model = sphere_models + 6;
	exp1->r.flags |= RF_TRANS_ADD | RF_TRANS_ADD_ALPHA | RF_TRANSLUCENT;
	exp1->r.frame = 0;
	exp1->radius=128;
	exp1->d_alpha=-4.0;
	exp1->r.scale=.25;
	exp1->d_scale = -0.5;
	if (Flags & CEF_FLAG8)
	{
		VectorScale(right, -0.4*SPHERE_LASER_SPEED, exp1->velocity);	// Move to the left
		VectorScale(right, SPHERE_LASER_SPEED, exp1->acceleration);
	}
	else
	{
		VectorScale(right, 0.4*SPHERE_LASER_SPEED, exp1->velocity);		// Move to the right
		VectorScale(right, -SPHERE_LASER_SPEED, exp1->acceleration);
	}
	AddEffect(NULL, exp1);

	VectorMA(spot1, len, fwd, tempSpot);
	//make the line beam down the side
	beam = ClientEntity_new(-1, CEF_DONT_LINK, spot1, NULL, 200);
	beam->r.model = sphere_models + 5;
 	beam->r.spriteType = SPRITE_LINE;
	beam->r.flags |= RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	beam->r.scale = (size-3) * 6;
	beam->radius = 256;
	beam->alpha = 0.95;
	beam->d_alpha = -5.0;
	VectorCopy(spot1, beam->r.startpos);
	VectorCopy(tempSpot, beam->r.endpos);
	if (Flags & CEF_FLAG8)
	{
		VectorScale(right, -0.4*SPHERE_LASER_SPEED, beam->velocity);	// Move to the left
		VectorScale(right, SPHERE_LASER_SPEED, beam->acceleration);
	}
	else
	{
		VectorScale(right, 0.4*SPHERE_LASER_SPEED, beam->velocity);		// Move to the right
		VectorScale(right, -SPHERE_LASER_SPEED, beam->acceleration);
	}
	AddEffect(NULL, beam); 

	count = GetScaledCount((int)(25 + size * 2.5), 0.3);

	//make the particles
	for(i=0; i < count;i++)
	{
		ce = ClientParticle_new(PART_16x16_SPARK_B, LightColor, 666);
	
		ce->scale=flrand(8, 24.0) + size*2;
		ce->scale *=0.4;
		ce->acceleration[2] = 0;
		ce->d_alpha=-768.0;
		VectorMA(ce->origin, flrand(0, len), fwd, ce->origin);
		VectorMA(ce->velocity, flrand(-15, 15), right, ce->velocity);
		VectorMA(ce->velocity, flrand(-15, 15), up, ce->velocity);
		VectorMA(ce->origin, flrand(-size*.4, size*.4), right, ce->origin);
		VectorMA(ce->origin, flrand(-size*.4, size*.4), up, ce->origin);
 		AddParticleToList(exp1, ce);
	}
	if (Flags & CEF_FLAG6)
	{
		// make the flares at the end of the line
		exp1 = ClientEntity_new(Type,Flags | CEF_ADDITIVE_PARTS, tempSpot, NULL, 500);
		exp1->r.model = sphere_models + 6;
		exp1->r.flags |= RF_TRANS_ADD | RF_TRANS_ADD_ALPHA | RF_TRANSLUCENT;
		exp1->r.frame = 0;
		exp1->radius=128;
		exp1->r.scale= 1;
		exp1->d_scale = 1;
		exp1->d_alpha = -2.0;
		AddEffect(NULL, exp1);
	}

	// create a scorchmark if necessary
//	VectorSubtract(beam->r.endpos, beam->r.startpos, dir);
//	VectorNormalize(dir);

	// Looks silly if it makes a burn
//	if	(Flags & CEF_FLAG7)
//		FXClientScorchmark(beam->r.endpos, dir);

}




// PLAYER SPHERE OF ANNIHILATION EXPLOSION

// ****************************************************************************
// FXSpherePlayerExplodeThink -
// ****************************************************************************

static qboolean FXSpherePlayerExplodeThink(struct client_entity_s *self,centity_t *Owner)
{
	if (fxi.cl->time > self->nextEventTime)
	{
		self->d_alpha = -5.0;
		self->dlight->d_intensity = -self->radius*2.0;

		if (fxi.cl->time > self->nextEventTime + 1000)
		{
			return false;
		}
	}
	else
	{
		self->dlight->intensity=(FX_SPHERE_EXPLOSION_BASE_RADIUS*self->r.scale*1.7);
	}
		
	return(true);
}


static qboolean FXSpherePlayerExplodeAddToView(struct client_entity_s *self,centity_t *Owner)
{
	self->r.angles[0]+=(M_PI/32.0)*(fxi.cl->time-self->lastThinkTime)/50.0;
	self->r.angles[1]+=(M_PI/27.0)*(fxi.cl->time-self->lastThinkTime)/50.0;

	self->lastThinkTime = fxi.cl->time;
		
	return(true);
}


static qboolean FXSpherePlayerExplodeGlowballThink(client_entity_t *glowball,centity_t *owner)
{
	vec3_t angvect;

	// Update the angle of the spark.
	VectorMA(glowball->direction, (float)(fxi.cl->time-glowball->lastThinkTime)/1000.0, glowball->velocity2, glowball->direction);

	glowball->radius = ((SPHERE_RADIUS_MAX-SPHERE_RADIUS_MIN) * 
							((fxi.cl->time - glowball->SpawnDelay) / 100.0) / (SPHERE_MAX_CHARGES+2));

	// Update the position of the spark.
	AngleVectors(glowball->direction, angvect, NULL, NULL);

	VectorMA(glowball->origin, glowball->radius, angvect, glowball->r.origin);

	glowball->lastThinkTime = fxi.cl->time;

	return true;
}


static qboolean FXSpherePlayerExplodeGlowballTerminate(client_entity_t *glowball, centity_t *owner)
{
	// Don't instantly delete yourself.  Don't accept any more updates and die out within a second.
	glowball->d_alpha = -5.0;						// Fade out.
	glowball->updateTime = 1000;	// Die in one second.
	glowball->Update = RemoveSelfAI;

	return true;
}



// ****************************************************************************
// FXSphereOfAnnihilationExplode -
// ****************************************************************************
void FXSpherePlayerExplode(centity_t *Owner, int Type, int Flags, vec3_t Origin)
{
	vec3_t				Dir;
	byte				Size;
	client_entity_t		*explosion, *glowball;
	paletteRGBA_t		LightColor={255,255,255,255}, haloColor={100,100,255,64};
	int					I, count;
	vec3_t				angvect;

	fxi.GetEffect(Owner,Flags,clientEffectSpawners[FX_WEAPON_SPHEREPLAYEREXPLODE].formatString,Dir,&Size);

	// Create an expanding ball of blue fire.
	explosion=ClientEntity_new(Type,Flags | CEF_ADDITIVE_PARTS,Origin,NULL,50);

	explosion->r.model = sphere_models + 3;
	explosion->r.flags=RF_FULLBRIGHT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	explosion->r.scale=0.01;
	explosion->color.c=0xffffffff;
	explosion->alpha = 1.0;
	explosion->d_alpha = 0.0;
	explosion->d_scale=1.5;
	explosion->SpawnInfo = (int)Size;
	explosion->radius=SPHERE_RADIUS_MIN + ((SPHERE_RADIUS_MAX-SPHERE_RADIUS_MIN) / SPHERE_MAX_CHARGES * explosion->SpawnInfo);
	explosion->dlight=CE_DLight_new(LightColor, explosion->radius/0.7 ,0);
	explosion->AddToView=FXSpherePlayerExplodeAddToView;
	explosion->Update=FXSpherePlayerExplodeThink;
	explosion->updateTime = ((explosion->SpawnInfo+1)*100);
	explosion->nextEventTime = fxi.cl->time + explosion->updateTime;
	explosion->lastThinkTime = fxi.cl->time;
	
	AddEffect(NULL, explosion);

	FXSpherePlayerExplodeThink(explosion,NULL);
	// Add some glowing blast particles.

	VectorScale(Dir,SMOKE_SPEED,Dir);

	count = GetScaledCount(40, 0.3);

	for(I=0;I<count;I++)
	{
		glowball = ClientEntity_new(Type, Flags & (~CEF_OWNERS_ORIGIN), Origin, NULL, 5000);
		glowball->r.model = sphere_models + 7;
		glowball->r.flags = RF_FULLBRIGHT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		glowball->AddToView = FXSpherePlayerExplodeGlowballThink;
		glowball->alpha = 1.0;
		glowball->d_alpha = 0.0;
		glowball->r.scale = 1.0;
		glowball->d_scale = 3.0;
		glowball->Update = FXSpherePlayerExplodeGlowballTerminate;
		glowball->lastThinkTime = glowball->SpawnDelay = fxi.cl->time;
		glowball->nextThinkTime = fxi.cl->time + ((explosion->SpawnInfo+1)*100);

		VectorClear(glowball->direction);
		glowball->direction[YAW] = flrand(0, 360.0);	// This angle is kept at a constant distance from org.
		glowball->direction[PITCH] = flrand(0, 360.0);

		glowball->velocity2[YAW] = flrand(-90.0, 90.0);
		if (glowball->velocity2[YAW] < 0)				// Assure that the sparks are moving around at a pretty good clip.
			glowball->velocity2[YAW] -= 90.0;
		else
			glowball->velocity2[YAW] += 90.0;

		glowball->velocity2[PITCH] = flrand(-90.0, 90.0);	// This is a velocity around the sphere.
		if (glowball->velocity2[PITCH] < 0)		// Assure that the sparks are moving around at a pretty good clip.
			glowball->velocity2[PITCH] -= 90.0;
		else
			glowball->velocity2[PITCH] += 90.0;

		AngleVectors(glowball->direction, angvect, NULL, NULL);
		VectorCopy(Origin, glowball->origin);
		VectorCopy(Origin, glowball->r.origin);

		AddEffect(NULL, glowball);
	}

	// Now make a big mutha flash
	explosion = ClientEntity_new(Type, Flags, Origin, NULL, 250);
	explosion->r.model = sphere_models + 4;		// hp_halo
	explosion->r.flags |= RF_TRANS_ADD | RF_TRANS_ADD_ALPHA | RF_TRANSLUCENT;
	explosion->r.frame = 1;
	explosion->radius= 128;
	explosion->d_alpha= -4.0;
	explosion->r.scale= 1.0;
	explosion->d_scale = -4.0;

	AddEffect(NULL, explosion);
}
