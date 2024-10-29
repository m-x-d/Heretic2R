//
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
#include "Utilities.h"
#include "Reference.h"
#include "Matrix.h"
#include "g_playstats.h"

#define FIST_DELTA_FORWARD	8.0
#define FIST_DELTA_THETA	0.12
#define	FIST_SPIRAL_RAD		0.75
#define	FIST_SCALE			0.20

#define	NUM_FF_MODELS	1
#define	NUM_GL_MODELS	5
#define	NUM_SW_MODELS	1
#define	NUM_SP_MODELS	4

static struct model_s *globe_models[NUM_GL_MODELS];
static struct model_s *ins_models[NUM_FF_MODELS];
static struct model_s *sword_models[NUM_SW_MODELS];
static struct model_s *spear_models[NUM_SP_MODELS];

void PreCacheIEffects()
{
	ins_models[0] = fxi.RegisterModel("sprites/spells/spark_blue.sp2");//sprites/spells/bluball.sp2");
	
	sword_models[0] = fxi.RegisterModel("sprites/spells/patball.sp2");

	spear_models[0] = fxi.RegisterModel("sprites/Spells/spark_red.sp2");
	spear_models[1] = fxi.RegisterModel("sprites/Spells/flyingfist.sp2");
	spear_models[2] = fxi.RegisterModel("sprites/Spells/spark_yellow.sp2");
	spear_models[3] = fxi.RegisterModel("sprites/fx/halo.sp2");

	globe_models[0] = fxi.RegisterModel("sprites/spells/shboom.sp2");
	globe_models[1] = fxi.RegisterModel("sprites/fx/halo.sp2");
	globe_models[2] = fxi.RegisterModel("Sprites/Spells/spark_blue.sp2");//"sprites/spells/glowball.sp2");
	globe_models[3] = fxi.RegisterModel("models/spells/sphere/tris.fm");
	globe_models[4] = fxi.RegisterModel("sprites/fx/neon.sp2");//bluball.sp2");//bluball.sp2");
}

// --------------------------------------------------------------

#define FX_GLOBE_FLY_SPEED				600.0
#define	FX_SOFT_GLOBE_AURA_SCALE		0.6
#define	FX_GLOBE_AURA_SCALE				0.8
#define FX_GLOBE_EXPLOSION_BASE_RADIUS	89.0
#define NUM_SPEAR_EXPLODES				8

enum 
{
	FX_I_SWORD,
	FX_I_SPEAR,
	FX_I_SP_MSL_HIT,
	FX_I_GLOBE,
	FX_I_GLOW,
	FX_I_STAFF,
	FX_I_ST_MSL_HIT,
	FX_I_RREFS,
	FX_I_SPEAR2,
	FX_I_SP_MSL_HIT2
};

static qboolean FXHellboltTrailElementThink(struct client_entity_s *self,centity_t *owner);
static qboolean FXHellboltTrailThink(struct client_entity_s *self,centity_t *owner);
static qboolean FXHellboltModelThink(struct client_entity_s *self,centity_t *owner);
static qboolean FXHellboltSmokePuffThink(struct client_entity_s *self,centity_t *owner);

// --------------------------------------------------------------


static qboolean FXGlobeOfOuchinessSphereThink(struct client_entity_s *self,centity_t *owner);
static qboolean FXGlobeOfOuchinessAuraElementThink(struct client_entity_s *self,centity_t *owner);
static qboolean FXGlobeOfOuchinessAuraThink(struct client_entity_s *self,centity_t *owner);
static qboolean FXGlobeOfOuchinessGlowballSparkThink(struct client_entity_s *self,centity_t *owner);
static qboolean FXGlobeOfOuchinessGlowballThink(struct client_entity_s *self,centity_t *owner);
static qboolean FXGlobeOfOuchinessGlowballSpawnerThink(struct client_entity_s *self,centity_t *owner);
static qboolean FXGlobeOfOuchinessSmokePuffThink(struct client_entity_s *self,centity_t *owner);

// ************************************************************************************************
// FXInsectStaffTrailThink
// ************************************************************************************************

static qboolean FXInsectStaffTrailThink(struct client_entity_s *self,centity_t *owner)
{
	vec3_t			TrailStart,Trail;
	float			TrailLength,DeltaTrailLength;
	vec3_t			Right,Up;
	int				NoOfIntervals;
	float			Theta,DeltaTheta;
	client_entity_t	*TrailEnt;

	VectorCopy(owner->origin,TrailStart);
	VectorSubtract(owner->origin, self->origin, Trail);

	self->r.scale = flrand(0.8, 1.3);
	if((TrailLength=VectorNormalize(Trail))>0.05)
	{
		PerpendicularVector(Right,Trail);
		CrossProduct(Trail,Right,Up);

		DeltaTrailLength = FIST_DELTA_FORWARD;
		VectorScale(Trail, FIST_DELTA_FORWARD, Trail);

		NoOfIntervals=(int)(TrailLength/DeltaTrailLength);

		if(NoOfIntervals > 40)
			return(false);
		
		Theta=fxi.cl->time*FIST_DELTA_THETA;
		DeltaTheta=(fxi.cls->frametime*FIST_DELTA_THETA)/NoOfIntervals;

		while(TrailLength>0.0)
		{

			TrailEnt=ClientEntity_new(FX_I_EFFECTS,
									  self->flags&~(CEF_OWNERS_ORIGIN|CEF_NO_DRAW),
									  TrailStart,
									  NULL,
									  1000);

			TrailEnt->r.model = ins_models;
			VectorMA(TrailStart,FIST_SPIRAL_RAD*cos(Theta),Right,TrailEnt->r.origin);
			VectorMA(TrailStart,FIST_SPIRAL_RAD*sin(Theta),Up,TrailEnt->r.origin);
			
			TrailEnt->r.flags=RF_TRANSLUCENT|RF_TRANS_ADD;
			
			VectorRandomCopy(self->velocity, TrailEnt->velocity, flrand(0, 4));

			TrailEnt->r.scale = FIST_SCALE+flrand(0.0, 0.05);
			TrailEnt->d_alpha = flrand(-1.75, -2);
			TrailEnt->d_scale = flrand(-0.75, -1.0);
			TrailEnt->radius=20.0;
			
			AddEffect(NULL,TrailEnt);

			TrailLength-=DeltaTrailLength;
			
			Theta+=DeltaTheta;
			
			VectorAdd(TrailStart,Trail,TrailStart);
		}
	}

	VectorCopy(owner->origin, self->origin);
	VectorCopy(TrailStart, self->r.origin);
	return true;
}

// ************************************************************************************************
// FXInsectStaff
// ************************************************************************************************

void FXInsectStaff(centity_t *owner,int type,int flags,vec3_t origin)
{
	client_entity_t	*Trail;
	paletteRGBA_t	LightColor={255,64,32,255};

	Trail=ClientEntity_new(type,flags,origin,NULL,17);

	Trail->r.model = ins_models;
	Trail->r.flags = RF_TRANSLUCENT | RF_TRANS_ADD;
	Trail->r.scale = flrand(0.8, 1.3);
	Trail->d_alpha = 0.0f;
	Trail->d_scale = 0.0f;
	Trail->r.color.c = 0xffffffff;
	Trail->radius = 20.0;

	Trail->Update=FXInsectStaffTrailThink;

	if(r_detail->value > DETAIL_NORMAL)
		Trail->dlight=CE_DLight_new(LightColor,150.0f,0.0f);

	AddEffect(owner,Trail);
}

// ************************************************************************************************
// FXInsectStaffExplode
// ************************************************************************************************

void FXInsectStaffExplode(centity_t *owner,int type,int flags,vec3_t origin, vec3_t Dir)
{
	client_entity_t	*SmokePuff;
	int				I;
	paletteRGBA_t	LightColor={255,64,32,255};

	if(flags & CEF_FLAG6)
		FXClientScorchmark(origin, Dir);

	VectorScale(Dir,32.0,Dir);

	I = GetScaledCount(irand(8,12), 0.8);

	while(I--)
	{
		if (!I)
			SmokePuff=ClientEntity_new(type,flags,origin,NULL,500);
		else
			SmokePuff=ClientEntity_new(type,flags,origin,NULL,1000);

		SmokePuff->r.model = ins_models;
		SmokePuff->r.flags |=RF_FULLBRIGHT|RF_TRANSLUCENT|RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		SmokePuff->r.frame=0;

		VectorRandomCopy(Dir, SmokePuff->velocity, flrand(16.0, 128.0));

		SmokePuff->acceleration[2] = flrand(-40, -60);

		SmokePuff->d_scale=flrand(-1.75,-2);
		SmokePuff->d_alpha=-0.2;
			
		SmokePuff->radius=20.0;

		if(!I)
		{
			SmokePuff->dlight=CE_DLight_new(LightColor,150.0f,0.0f);
			VectorClear(SmokePuff->velocity);
		}	

		SmokePuff->Scale=flrand(0.25, 0.35);

		AddEffect(NULL,SmokePuff);
	}
}


// ****************************************************************************
// FXGlobeOfOuchinessGlobeThink -
// ****************************************************************************

static qboolean FXGlobeOfOuchinessGlobeThink(struct client_entity_s *self,centity_t *owner)
{
	self->r.scale = flrand(0.35, 0.50);

	return(true);
}

// ****************************************************************************
// FXGlobeOfOuchinessAuraThink -
// ****************************************************************************

static qboolean FXGlobeOfOuchinessAuraThink(struct client_entity_s *self,centity_t *owner)
{
	vec3_t			TrailStart,Trail;
	float			TrailLength,DeltaTrailLength;
	vec3_t			Right,Up;
	client_entity_t	*TrailEnt;
	int i;

	//

	VectorCopy(owner->origin,TrailStart);
	VectorSubtract(owner->lerp_origin,owner->origin,Trail);

	if((TrailLength=VectorNormalize(Trail))<0.001)
	{
		TrailLength+=2.0;
	}
	
	//

	PerpendicularVector(Right,Trail);
	CrossProduct(Trail,Right,Up);

	DeltaTrailLength=FX_GLOBE_FLY_SPEED;
	VectorScale(Trail,FX_GLOBE_FLY_SPEED,Trail);

	i=0;
	while(TrailLength>0.0)
	{
		//
		i++;
		if (i>40)
			return true;
		TrailEnt=ClientEntity_new(FX_I_EFFECTS,
								  self->flags&~(CEF_OWNERS_ORIGIN|CEF_NO_DRAW),
								  TrailStart,
								  NULL,
								  500);

		TrailEnt->r.model = globe_models;
		TrailEnt->r.flags=RF_TRANSLUCENT|RF_TRANS_ADD|RF_TRANS_ADD_ALPHA;
		if (r_detail->value < DETAIL_NORMAL)
			TrailEnt->Scale=FX_SOFT_GLOBE_AURA_SCALE+flrand(0.0, 0.1);
		else
			TrailEnt->Scale=FX_GLOBE_AURA_SCALE+flrand(0.0, 0.1);
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
// FXGlobeOfOuchiness -
// ****************************************************************************

void FXInsectGlobe(centity_t *owner,int type,int flags,vec3_t origin, short CasterEntnum)
{
	client_entity_t	*GlobeThinker,
					*AuraThinker;
	paletteRGBA_t	LightColor={0,0,255,255};
	int				caster_update;

	// Create a fiery blue aura around the globe.

	if (r_detail->value < DETAIL_NORMAL)
		caster_update = 125;
	else
		caster_update = 100;

	AuraThinker=ClientEntity_new(type,flags,origin,NULL,caster_update);

	AuraThinker->flags|=CEF_NO_DRAW;
	AuraThinker->dlight=CE_DLight_new(LightColor,150.0,0.0);
	AuraThinker->Update=FXGlobeOfOuchinessAuraThink;
	AuraThinker->extra=owner;//(void *)(&fxi.server_entities[CasterEntnum]);// The caster's centity_t.

	AddEffect(owner,AuraThinker);

	FXGlobeOfOuchinessAuraThink(AuraThinker,owner);

	// Create the globe of ouchiness itself.

	GlobeThinker = ClientEntity_new(type, flags, origin, NULL, 100);

	GlobeThinker->r.model = globe_models + 1;
	GlobeThinker->r.flags |= RF_TRANSLUCENT|RF_TRANS_ADD;
	GlobeThinker->r.scale = flrand(0.15, 0.20);

	GlobeThinker->r.color.r=irand(128, 180);
	GlobeThinker->r.color.g=irand(128, 180);
	GlobeThinker->r.color.b=irand(180, 255);
	
	GlobeThinker->radius = 70.0;
	GlobeThinker->Update = FXGlobeOfOuchinessGlobeThink;
	GlobeThinker->AddToView = LinkedEntityUpdatePlacement;
	
	AddEffect(owner, GlobeThinker);
}

// ****************************************************************************
// FXGlobeOfOuchinessGlowballThink -
// ****************************************************************************

static qboolean FXGlobeOfOuchinessGlowballThink(struct client_entity_s *self,centity_t *owner)
{
	client_entity_t	*Spark;

	if((owner->current.effects&EF_MARCUS_FLAG1))
		self->color.r++;

	if(self->color.r>3)
	{
		// Create a trailing spark.

		Spark=ClientEntity_new(FX_I_EFFECTS,
							   self->flags&~(CEF_OWNERS_ORIGIN),
							   self->r.origin,
							   NULL,
							   500);
		
		Spark->r.model = globe_models + 2;
		Spark->r.flags|=RF_FULLBRIGHT|RF_TRANSLUCENT|RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		Spark->r.scale = FIST_SCALE+flrand(0.0, 0.05);
		Spark->d_alpha = flrand(-1.75, -2);
		Spark->d_scale = flrand(-0.75, -1.0);
		Spark->radius=20.0;

		AddEffect(NULL,Spark);
	}

	// 'self->extra' refers to the globe's centity_t.

	if(self->color.r<16)
	{
		self->velocity[0]*=3.0;
		self->velocity[0]+=6.0*(owner->origin[0]-self->r.origin[0]);
		self->velocity[0]*=0.265;

		self->velocity[1]*=3.0;
		self->velocity[1]+=6.0*(owner->origin[1]-self->r.origin[1]);
		self->velocity[1]*=0.265;
		
		self->velocity[2]*=3.0;
		self->velocity[2]+=6.0*(owner->origin[2]-self->r.origin[2]);
		self->velocity[2]*=0.265;

		return(true);
	}
	else
	{	
		return(false);
	}
}

// ****************************************************************************
// FXGlobeOfOuchinessGlowballSpawnerThink -
// ****************************************************************************

static qboolean FXGlobeOfOuchinessGlowballSpawnerThink(struct client_entity_s *self,centity_t *owner)
{
	client_entity_t	*Glowball;
	vec3_t			Forward,Right,
					Forward2,Right2,
					Temp;
	matrix3_t		RotationMatrix;

	// This tells if we are wasting our time, because the reference points are culled.
	if (!RefPointsValid((centity_t *)(self->extra)))
		return true;

	// If the spell is still building, create some swirling blue Glowballs.

	if(owner->current.effects&EF_MARCUS_FLAG1)
	{	
		// 'self->extra' refers to the caster's centity_t.

		Glowball=ClientEntity_new(FX_I_EFFECTS,
								  self->flags&~(CEF_NO_DRAW|CEF_OWNERS_ORIGIN),
								  ((centity_t *)(self->extra))->origin,
								  NULL,
								  50);

		self->flags|=CEF_DONT_LINK;
		
		VectorCopy(((centity_t *)(self->extra))->current.angles,Temp);
		VectorScale(Temp,180.0/M_PI,Temp);
		AngleVectors(Temp,Forward,Right,NULL);

		VectorCopy(owner->current.angles,Temp);
		VectorScale(Temp,180.0/M_PI,Temp);
		AngleVectors(Temp,Forward2,Right2,NULL);

		// Make me spawn from my caster's left / right hands (alternating).

		Matrix3FromAngles(((centity_t *)(self->extra))->lerp_angles,RotationMatrix);

		if(!(self->color.g&1))
		{
			Matrix3MultByVec3(RotationMatrix,
				((centity_t *)(self->extra))->referenceInfo->references[INSECT_STAFF].placement.origin,
					  Glowball->r.origin);
		}
		else
		{
			Matrix3MultByVec3(RotationMatrix,
				  ((centity_t *)(self->extra))->referenceInfo->references[INSECT_SWORD].placement.origin,
					  Glowball->r.origin);
		}

		VectorAdd(((centity_t *)(self->extra))->origin,Glowball->r.origin,Glowball->r.origin);

		// Set my velocity and accelaration.

		Glowball->velocity[0]=Forward2[0]*175.0+flrand(-25.0, 25.0);

		if(self->color.g&1)
			Glowball->velocity[0]=-Glowball->velocity[0];

		Glowball->velocity[1]=Forward2[1]*175.0+flrand(-25.0, 25.0);

		if(!(self->color.g&1))
			Glowball->velocity[1]=-Glowball->velocity[1];
		
		Glowball->velocity[2]=flrand(-200.0, 100.0);

		VectorClear(Glowball->acceleration);

		// Fill in the rest of my info.
		Glowball->r.model = globe_models + 2;

		Glowball->r.flags=RF_TRANSLUCENT|RF_TRANS_ADD;
		Glowball->r.color.r= irand(128, 180);
		Glowball->r.color.g= irand(128, 180);
		Glowball->r.color.b= irand(180, 255);
		Glowball->color.r=1;
		Glowball->radius=20.0;

		Glowball->extra=(void *)owner;
		Glowball->Update=FXGlobeOfOuchinessGlowballThink;

		AddEffect(owner,Glowball);

		FXGlobeOfOuchinessGlowballThink(Glowball,owner);

		self->color.g++;
	}

	return(true);
}

// ****************************************************************************
// FXGlobeOfOuchinessGlowballs -
// ****************************************************************************

void FXInsectGlow(centity_t *owner,int type,int flags,vec3_t origin, short CasterEntnum)
{
	client_entity_t	*GlowballSpawner;
	int				caster_update;

	// Create a spawner that will create the glowballs.

	if (r_detail->value < DETAIL_NORMAL)
		caster_update = 250;
	else
		caster_update = 100;

	GlowballSpawner=ClientEntity_new(type,flags,origin,NULL,caster_update);

	GlowballSpawner->flags|=CEF_NO_DRAW;
	GlowballSpawner->color.g=0;
	GlowballSpawner->Update=FXGlobeOfOuchinessGlowballSpawnerThink;
	GlowballSpawner->extra=(void *)(&fxi.server_entities[CasterEntnum]);
//	GlowballSpawner->extra=(void *)owner;

	AddEffect(owner,GlowballSpawner);
}

qboolean InsectFirstSeenInit(struct client_entity_s *self, centity_t *owner)
{
	self->refMask |= INSECT_MASK;	

	EnableRefPoints(owner->referenceInfo, self->refMask);

	self->AddToView = NULL;
	self->Update = KeepSelfAI;
	return true;
}

void FXInsectReadyRefs (centity_t *owner,int type,int flags,vec3_t origin)
{
	client_entity_t		*self;

	flags |= CEF_NO_DRAW;
	self = ClientEntity_new(type, flags, origin, NULL, 17);

	self->Update = NULL;
	self->AddToView = InsectFirstSeenInit;

	AddEffect(owner, self);
}

//HELLSTAFF

// ************************************************************************************************
// FXHellbolt
// ************************************************************************************************

static qboolean FXISpMslThink (struct client_entity_s *self, centity_t *owner)
{
	if(self->LifeTime < fxi.cl->time)
		return (false);

	self->r.frame++;
	if(self->r.frame>2)
		self->r.frame = 0;
	
	return (true);
}

void FXISpear(centity_t *owner, int type, int flags, vec3_t origin, vec3_t vel)
{
	client_entity_t	*hellbolt;	
	paletteRGBA_t	LightColor = {255, 128, 64, 255};

	hellbolt = ClientEntity_new(type, flags | CEF_DONT_LINK, origin, NULL, 10000);

	hellbolt->r.model = spear_models;
	hellbolt->r.flags = RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;

	hellbolt->r.scale = 1.0;
	hellbolt->r.color = LightColor;
	hellbolt->d_alpha = 0.0;
	hellbolt->radius = 10.0F;
	VectorCopy(vel, hellbolt->velocity);
	hellbolt->AddToView = LinkedEntityUpdatePlacement;

	if(r_detail->value > DETAIL_NORMAL)
		hellbolt->dlight = CE_DLight_new(LightColor, 150.0f, -300.0f);

	AddEffect(owner, hellbolt);
}

qboolean FXISpear2Update(struct client_entity_s *self, centity_t *owner)
{
	paletteRGBA_t		LightColor = {255, 128, 255, 255};
	client_particle_t	*spark;
	int					i;
	float				dist;
	vec3_t				dir;

	self->r.color.a = irand(128, 136);
	self->r.scale = flrand(0.1, 0.5);

	if(!VectorCompare(owner->lerp_origin, self->startpos2))
		VectorCopy(owner->lerp_origin, self->startpos2);

	VectorSubtract(owner->lerp_origin, self->startpos2, dir);
	dist = VectorNormalize(dir);

	for(i = 0; i < 10; i++)
	{
		spark = ClientParticle_new(PART_16x16_SPARK_Y, self->r.color, 200);
		spark->type |= PFL_ADDITIVE;

		spark->acceleration[2] = 0.5;
		
		spark->scale = flrand(5, 6);
		spark->d_scale = flrand(-13, -20);
		
		spark->color.r = irand(240, 255);
		spark->color.g = irand(240, 255);
		spark->color.b = irand(240, 255);

		spark->color.a = irand(64, 196);
		spark->d_alpha = -128;

		VectorAdd(self->startpos2, spark->origin, spark->origin);
		VectorMA(spark->origin, dist/i, dir, spark->origin);
		spark->origin[0] += flrand(-2, 2);
		spark->origin[1] += flrand(-2, 2);
		spark->origin[2] += flrand(-2, 2);

		AddParticleToList(self, spark);
	}

	return (true);
}

void FXISpear2(centity_t *owner, int type, int flags, vec3_t origin)
{
	client_entity_t	*hellbolt;	
	paletteRGBA_t	LightColor = {255, 128, 255, 255};

	hellbolt = ClientEntity_new(type, CEF_OWNERS_ORIGIN | CEF_ABSOLUTE_PARTS, origin, NULL, 20);

	hellbolt->r.model = spear_models + 3;
	hellbolt->r.flags = RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;

	hellbolt->r.color.c = 0xffffffff;
	hellbolt->r.scale = flrand(0.2, 0.4);
	hellbolt->d_alpha = 0.0;
	hellbolt->radius = 10.0F;
	hellbolt->AddToView = LinkedEntityUpdatePlacement;
	VectorCopy(owner->current.origin, hellbolt->startpos2);

	if(r_detail->value > DETAIL_NORMAL)
		hellbolt->dlight = CE_DLight_new(LightColor, 150.0f, -300.0f);

	hellbolt->Update = FXISpear2Update;

	AddEffect(owner, hellbolt);

//
	hellbolt = ClientEntity_new(type, CEF_OWNERS_ORIGIN, origin, NULL, 10000);

	hellbolt->r.model = spear_models + 2;
	hellbolt->r.flags = RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;

	hellbolt->r.color.c = 0x33ffffff;
	hellbolt->r.scale = flrand(1, 2);
	hellbolt->d_alpha = 0.0;
	hellbolt->radius = 10.0F;
	hellbolt->AddToView = LinkedEntityUpdatePlacement;


	AddEffect(owner, hellbolt);
}
// ************************************************************************************************
// FXISpMslHit
// ---------------------
// ************************************************************************************************

void FXISpMslHit(centity_t *owner, int type, int flags, vec3_t origin, vec3_t Dir)
{
	client_entity_t	*smokepuff;
	int				i;
	paletteRGBA_t	lightcolor = {255, 96, 48, 255};

	if(flags & CEF_FLAG6)
	{
		FXClientScorchmark(origin, Dir);
	}
	Vec3ScaleAssign(32.0, Dir);

	for(i = 0; i < NUM_SPEAR_EXPLODES; i++)
	{
		smokepuff = ClientEntity_new(type, flags, origin, NULL, 500);

		smokepuff->r.model = spear_models + 1;
		smokepuff->r.flags = RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		smokepuff->r.scale = flrand(0.2, 0.3);
		smokepuff->r.color = lightcolor;

		VectorRandomCopy(Dir, smokepuff->velocity, 64);
		VectorSet(smokepuff->acceleration, 0.0, 0.0, GetGravity() * 0.3);
 
		smokepuff->radius = 200.0;
		smokepuff->d_scale = -0.5;
		smokepuff->d_alpha = -2.0;

		if(!i)
		{
			fxi.S_StartSound(smokepuff->r.origin, -1, CHAN_WEAPON, fxi.S_RegisterSound("weapons/HellHit.wav"), 1, ATTN_NORM, 0);
			smokepuff->dlight = CE_DLight_new(lightcolor, 150.0f, 0.0f);
			VectorClear(smokepuff->velocity);
		}	
		AddEffect(NULL,smokepuff);
	}
}

void FXISpMslHit2(centity_t *owner, int type, int flags, vec3_t origin, vec3_t Dir)
{
	client_entity_t	*smokepuff;
	int				i;

	Vec3ScaleAssign(32.0, Dir);

	for(i = 0; i < NUM_SPEAR_EXPLODES; i++)
	{
		smokepuff = ClientEntity_new(type, flags, origin, NULL, 500);

		smokepuff->r.model = spear_models + 2;
		smokepuff->r.flags = RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		smokepuff->r.scale = flrand(0.7, 1);

		VectorRandomCopy(Dir, smokepuff->velocity, 64);
		VectorSet(smokepuff->acceleration, 0.0, 0.0, GetGravity() * 0.3);
 
		smokepuff->radius = 200.0;
		smokepuff->d_scale = -0.7;
		smokepuff->d_alpha = -2.0;

		if(!i)
		{
			fxi.S_StartSound(smokepuff->r.origin, -1, CHAN_WEAPON, fxi.S_RegisterSound("weapons/HellHit.wav"), 1, ATTN_NORM, 0);
			VectorClear(smokepuff->velocity);
		}	
		AddEffect(NULL,smokepuff);
	}

	smokepuff = ClientEntity_new(type, CEF_OWNERS_ORIGIN, origin, NULL, 500);

	smokepuff->r.model = spear_models + 3;
	smokepuff->r.frame = 1;
	smokepuff->r.flags = RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;

	smokepuff->r.color.c = 0x77ffffff;
	smokepuff->r.scale = flrand(0.3, 0.5);
	smokepuff->d_scale = 2;
	smokepuff->d_alpha = -2;
	smokepuff->radius = 10.0F;

	AddEffect(NULL, smokepuff);
}
//Insect reference points
//	INSECT_STAFF,
//	INSECT_SWORD,
//	INSECT_SPEAR,
//	INSECT_RIGHTFOOT,
//	INSECT_LEFTFOOT,
static qboolean FXStaffElementThink(struct client_entity_s *self,centity_t *owner)
{
	float	Frac,
			Multiplier;
	int		FrameNo;

	Frac=(fxi.cl->time-self->startTime)/100.0;

	if(self->AnimSpeed>0.0)
	{
		Frac*=self->AnimSpeed;
	}

	if((FrameNo=floor(Frac))>=(self->NoOfAnimFrames-1))
	{
		return(false);
	}
	else
	{
		Multiplier=1.0-Frac/(self->NoOfAnimFrames-1);

		self->r.color.r=self->color.r*Multiplier;
		self->r.color.b=self->color.g*Multiplier;
		self->r.color.g=self->color.b*Multiplier;
		
		self->r.frame=FrameNo+1;

		return(true);
	}
}

// ************************************************************************************************
// FXStaffThink
// -----------------
// ************************************************************************************************

static qboolean FXISwordTrailThink(struct client_entity_s *self,centity_t *owner)
{
	int				I;
	int				NoOfIntervals;
	client_entity_t	*TrailEnt;
	vec3_t			diff, newpoint, last_org, current_org;
	matrix3_t		RotationMatrix;
	float			incr;

	if(self->LifeTime < fxi.cl->time)
		return (false);

	// This tells if we are wasting our time, because the reference points are culled.
	if (!RefPointsValid(owner))
		return false;		// Remove the effect in this case.

	self->updateTime = 17;		// FIXME : With a next think time this effect does not look right

	I=self->NoOfAnimFrames;

	//extrapolate down length of sword from hand!
	
	Matrix3MultByVec3(RotationMatrix,
		((centity_t *)(self->extra))->referenceInfo->references[INSECT_SWORD].placement.origin,
			  current_org);

	Matrix3MultByVec3(RotationMatrix,
		((centity_t *)(self->extra))->referenceInfo->oldReferences[INSECT_SWORD].placement.origin,
			  last_org);

	// If this reference point hasn't changed since the last frame, return.
	VectorSubtract(	current_org, last_org, diff);

	if (Q_fabs(diff[0] + diff[1] + diff[2]) < .1)
		return(true);

	NoOfIntervals=(int)(VectorLength(diff)*.5);
	if(NoOfIntervals > 40)
		return(false);

	incr = VectorNormalize(diff)/NoOfIntervals;

	Matrix3FromAngles(((centity_t *)(self->extra))->lerp_angles, RotationMatrix);

	while (NoOfIntervals >= 0)
	{
		VectorMA(last_org, incr, diff, newpoint);
		TrailEnt=ClientEntity_new(FX_SPELLHANDS, self->flags & ~CEF_NO_DRAW, newpoint, 0, 100);
		VectorCopy(newpoint, TrailEnt->origin);
		TrailEnt->r.model = sword_models;
		TrailEnt->alpha=.3;
		TrailEnt->r.flags=RF_TRANSLUCENT|RF_TRANS_ADD|RF_TRANS_ADD_ALPHA;
		TrailEnt->r.frame=1;
		TrailEnt->d_scale=-0.25;
		TrailEnt->d_alpha=-0.1;
		TrailEnt->color.c=0x50000018;
		TrailEnt->r.scale=self->xscale*2.0;
		TrailEnt->startTime=fxi.cl->frame.servertime-100;
		TrailEnt->AnimSpeed=0.20;
		TrailEnt->NoOfAnimFrames=2;
		TrailEnt->Update=FXStaffElementThink;
		TrailEnt->AddToView=OffsetLinkedEntityUpdatePlacement;			
		AddEffect(owner,TrailEnt);

		FXStaffElementThink(TrailEnt,owner);
		
		VectorCopy(newpoint, last_org);
		NoOfIntervals--;
	}

	return(true);
}

// ************************************************************************************************
// FXStaff
// ------------
// ************************************************************************************************

// This effect spawns 70+ client fx which will cause problems

void FXISwordTrail(centity_t *owner,int type,int flags,vec3_t origin)
{
	short			Refpoints;
	client_entity_t	*trail;
	int				I;

	Refpoints=0;

	Refpoints = (1<<INSECT_SWORD);

	if(!ReferencesInitialized(owner))
	{
		return;
	}

	for(I=0;I<16;I++)
	{
		if(!(Refpoints & (1 << I)))
			continue;

		trail=ClientEntity_new(type,flags,origin,0,17);

		trail->Update = FXISwordTrailThink;
		trail->flags |= CEF_NO_DRAW;
		trail->NoOfAnimFrames = I;

		trail->color.c = 0x50285020;
		trail->xscale = .175;

		trail->LifeTime = fxi.cl->time + 180;
		trail->extra = (void *)owner;

		AddEffect(owner,trail);
	}
}

/*==============================

  Insect Effects Handler

  ==============================*/

void FXIEffects(centity_t *owner,int type,int flags, vec3_t origin)
{
	paletteRGBA_t	LightColor={0,0,255,255};
	vec3_t			vel;
	byte			fx_index;
	
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_I_EFFECTS].formatString, &fx_index, &vel);//fixme- make this 1 dir and 1 float

	switch (fx_index)
	{
		case FX_I_SWORD:
			FXISwordTrail(owner, type, flags, origin);
			break;

		case FX_I_SPEAR:
			FXISpear(owner, type, flags, origin, vel);
			break;
		
		case FX_I_SP_MSL_HIT:
			FXISpMslHit(owner, type, flags, origin, vel);
			break;

		case FX_I_GLOBE:
			FXInsectGlobe(owner, type, flags, origin, (short)(vel[0]));
			break;

		case FX_I_GLOW:
			FXInsectGlow(owner, type, flags, origin, (short)(vel[0]));
			break;

		case FX_I_STAFF:
			FXInsectStaff(owner, type, flags, origin);
			break;

		case FX_I_ST_MSL_HIT:
			FXInsectStaffExplode(owner, type, flags, origin, vel);
			break;

		case FX_I_RREFS:
			FXInsectReadyRefs (owner, type, flags, origin);
			break;

		case FX_I_SPEAR2:
			FXISpear2(owner, type, flags, origin);
			break;

		case FX_I_SP_MSL_HIT2:
			FXISpMslHit2(owner, type, flags, origin, vel);
			break;

		default:
			break;
	}
}

