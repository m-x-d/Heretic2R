#include "Client Effects.h"
#include "Client Entities.h"
#include "Particle.h"
#include "ResourceManager.h"
#include "FX.h"
#include "Vector.h"
#include "Random.h"
#include "ce_Dlight.h"
#include "Utilities.h"
#include "g_playstats.h"
#include "Matrix.h"
#include "Reference.h"

extern void MakeBubble(vec3_t loc, client_entity_t *spawner);

#define	NUM_GENFX_MODELS	6
static struct model_s *genfx_models[NUM_GENFX_MODELS];
void PrecacheOgleHitPuff()
{
	genfx_models[0] = fxi.RegisterModel("sprites/fx/steam_add.sp2");
	genfx_models[1] = fxi.RegisterModel("models/debris/stone/schunk1/tris.fm");
	genfx_models[2] = fxi.RegisterModel("models/debris/stone/schunk2/tris.fm");
	genfx_models[3] = fxi.RegisterModel("models/debris/stone/schunk3/tris.fm");
	genfx_models[4] = fxi.RegisterModel("models/debris/stone/schunk4/tris.fm");
	genfx_models[5] = fxi.RegisterModel("sprites/fx/halo.sp2");
}

#define NUM_WATERPART_MODELS 1
static struct model_s *WaterParticle_models[NUM_WATERPART_MODELS];
#define NUM_COMPASS_MODELS 1
static struct model_s *compass_models[NUM_COMPASS_MODELS];
void PreCacheWaterParticles()
{
	WaterParticle_models[0] = fxi.RegisterModel("sprites/lens/flare3.sp2");
	compass_models[0] = fxi.RegisterModel("sprites/fx/compass.sp2");
}

// -----------------------------------------------------------------------------------------

#define PARTICLE_TRAIL_PUFF_TIME 1000 // puff's last for 1 sec

qboolean ParticleTrailAI(client_entity_t *this, centity_t *owner)
{
	client_entity_t		*effect;
	client_particle_t	*p;
	int					i;

	assert(owner);

	effect = ClientEntity_new(FX_PUFF, CEF_NO_DRAW, owner->current.old_origin, NULL, PARTICLE_TRAIL_PUFF_TIME);
	
	effect->flags |= CEF_NO_DRAW;

	for(i = 0; i < 40; i++)
	{
		p = ClientParticle_new(PART_4x4_WHITE, this->color, PARTICLE_TRAIL_PUFF_TIME);
		p->velocity[0] = flrand(-20.0F, 20.0F);
		p->velocity[1] = flrand(-20.0F, 20.0F);
		p->velocity[2] = flrand(30.0F, 80.0F);
		AddParticleToList(effect, p);
	}

	AddEffect(NULL, effect); // add the puff to the world

	return true;// actual puff spawner only goes away when it owner has a 
				// FX_REMOVE_EFFECTS sent on it
}

void CL_DiminishingTrail (vec3_t start, vec3_t end, centity_t *old, qboolean gib);

void GenericGibTrail(centity_t *owner, int type, int flags, vec3_t origin)
{
	qboolean ParticleTrailAI(client_entity_t *this, centity_t *owner);

	client_entity_t *effect;

	effect = ClientEntity_new(type, flags, origin, NULL, PARTICLE_TRAIL_THINK_TIME);
	
	effect->flags |= CEF_NO_DRAW;

	effect->color.c = 0xFF2020FF;
	
	effect->Update = ParticleTrailAI;

	assert(owner);

	AddEffect(owner, effect);

	ParticleTrailAI(effect, owner); // think once right away, to spawn the first puff
}

qboolean PebbleUpdate(struct client_entity_s *self, centity_t *owner)
{
	int curTime = fxi.cl->time;
	float d_time = (curTime - self->lastThinkTime) / 1000.0f;

	self->acceleration[2] -= 75;
	self->r.angles[0] += ANGLE_360*d_time;
	self->r.angles[1] += ANGLE_360*d_time;
	
	self->lastThinkTime = fxi.cl->time;

	if (curTime > self->LifeTime)
		return false;

	return true;
}
//Slight variation on the normal puff
void FXOgleHitPuff(centity_t *owner, int type, int flags, vec3_t origin)
{
	client_entity_t		*effect;
	vec3_t				dir, work;
	byte				count;
	int					i, chance ;
	float speed;
	
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_OGLE_HITPUFF].formatString, dir);			// normalized direction vector

	speed = VectorNormalize(dir);
	if(speed>1.0f)
		count = irand(10,15);
	else
		count = irand(1,4);

	for(i = 0; i < count; i++)
	{//puff!
		effect = ClientEntity_new(type, flags, origin, NULL, 500);

		effect->r.model = genfx_models;
		effect->r.flags |= RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;

		if(speed>1.0f)
		{
			VectorRandomCopy(dir, work, 0.5);
			VectorScale(work, speed, effect->velocity);
			effect->acceleration[2] = flrand(10.0, 50.0);  
		}
		else
		{
			VectorRandomCopy(dir, work, 0.5);
			
			if (flags & CEF_FLAG6)
			{
				VectorScale(work, flrand(8.0, 12.0), effect->velocity);
				effect->acceleration[2] = flrand(10.0, 50.0);  
			}
			else
			{
				VectorScale(work, 4.0, effect->velocity);
				effect->acceleration[2] = flrand(10.0, 50.0);  
			}
		}

		effect->alpha = 0.35;
		if(speed>1)
			effect->r.scale = flrand(0.3, 0.75);
		else
			effect->r.scale = 0.1;
		effect->d_scale = 2.0;
		effect->d_alpha = -2.0;
		effect->color.c = 0xFFFFFFFF;

		AddEffect(NULL, effect);	// add the effect as independent world effect
	}

	for(i = 0; i < count; i++)
	{//ROCK!
		effect = ClientEntity_new(type, flags, origin, NULL, 50);

		chance = irand(0,3);

		switch(chance)
		{
		case 0:
			effect->r.model = genfx_models + 1;
			break;
		case 1:
			effect->r.model = genfx_models + 2;
			break;
		case 2:
			effect->r.model = genfx_models + 3;
			break;
		case 3:
			effect->r.model = genfx_models + 4;
			break;
		}

		if(speed>1.0f)
		{
			VectorRandomCopy(dir, work, 0.5);
			VectorScale(work, speed, effect->velocity);
		}
		else
		{
			VectorRandomCopy(dir, work, 0.5);
			VectorScale(work, flrand(8.0, 16.0), effect->velocity);
		}
		
		if (flags & CEF_FLAG6 || speed > 1.0f)
		{
			effect->acceleration[0] += flrand(-75.0, 75.0);
			effect->acceleration[1] += flrand(-75.0, 75.0);
			effect->acceleration[2] = flrand(125.0, 250.0);
		}
		else
			effect->acceleration[2] = flrand(-50.0, 50.0);

		effect->Update = PebbleUpdate;
		effect->alpha = 1.0;
		if(speed>1)
			effect->r.scale = flrand(0.8, 1.5) * speed/100;
		else
			effect->r.scale = flrand(0.1, 0.25);
		effect->d_scale = 0.0;
		effect->d_alpha = 0.0;
		effect->color.c = 0xFFFFFFFF;
		effect->LifeTime = fxi.cl->time + 5000;

		AddEffect(NULL, effect);	// add the effect as independent world effect
	}
}

void FXGenericHitPuff(centity_t *owner, int type, int flags, vec3_t origin)
{
	client_entity_t		*effect;
	vec3_t				dir, work;
	byte				count;
	int					i;

	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_HITPUFF].formatString, dir, &count);			// normalized direction vector

	if (count>40)
		count=40;

	for(i = 0; i < count; i++)
	{
		effect = ClientEntity_new(type, flags, origin, NULL, 500);

		effect->r.model = genfx_models;

		effect->r.flags |= RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;

		VectorRandomCopy(dir, work, 0.5);
		VectorScale(work, 50.0, effect->velocity);
		effect->acceleration[2] = -100.0;									  
		effect->alpha = 0.5;
		effect->r.scale = 0.1;
		effect->d_scale = 1.0;
		effect->d_alpha = -1.0;
		effect->color.c = 0xFFFFFFFF;

		AddEffect(NULL, effect);	// add the effect as independent world effect
	}

	if (flags & CEF_FLAG6)
	{	// High-intensity impact point.
		effect = ClientEntity_new(type, flags | CEF_DONT_LINK, origin, NULL, 250);
		
		effect->r.model = genfx_models + 5;
		effect->r.frame = 0;
		effect->r.flags = RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		effect->r.scale = 0.4;
		effect->alpha = .95;
		effect->d_scale = -1.6;
		effect->d_alpha = -3.0;			// Alpha goes up to 1, then down to zero.
		effect->color.c = 0xc0ffffff;
		effect->radius = 10.0;
		effect->alpha = 0.8;
		effect->r.origin[2] += 8.0;

		AddEffect(NULL, effect);
	}
}

// -----------------------------------------

#define SINEAMT					1
#define SINESCALE				(256.0 / (2 * M_PI))
#define WATER_DENSITY			150.0
#define WATER_DIST				100.0
#define WATERPARTICLE_CLIPDIST	(WATER_DIST * WATER_DIST)

static qboolean water_particles_spawned;

void SetupWaterParticle(client_particle_t *p, qboolean recycle)
{
	int		ishade;
	float	shade;
	vec3_t	dist;

	ishade = irand(50, 150);
	p->color.r = (byte)ishade;
	p->color.g = (byte)ishade;
	p->color.b = (byte)ishade;
	p->color.a = irand(75, 150);

	shade = ((ishade * 0.04) - 3.1) * 0.35;

	p->velocity[0] = flrand(-1.0F, 1.0F);
	p->velocity[1] = flrand(-1.0F, 1.0F);
	p->velocity[2] = flrand(shade, 0.0);

	p->acceleration[2] = 0;
	p->scale = flrand(0.3, 0.7);

	dist[0] = flrand(-WATER_DIST, WATER_DIST);
	dist[1] = flrand(-WATER_DIST, WATER_DIST);
	dist[2] = flrand(-WATER_DIST, WATER_DIST);

	// If we are recycling, we want to respawn as far away as possible
	if(recycle)
	{
		VectorNormalize(dist);
		Vec3ScaleAssign(WATER_DIST, dist);
	}
	VectorAdd(fxi.cl->refdef.vieworg, dist, p->origin);
}

void CreateWaterParticles(client_entity_t *self)
{
	int					i;
	client_particle_t	*p;

	// Scale number of particles by detail level
	for(i = 0; i < (WATER_DENSITY * r_detail->value); i++)
	{
		p = ClientParticle_new(PART_4x4_WHITE | PFL_SOFT_MASK, self->color, 1000000);
		
		SetupWaterParticle(p, false);

		AddParticleToList(self, p);
	}
}

void UpdateWaterParticles(client_entity_t *self)
{
	client_particle_t	*p;
	vec3_t				part_dist;
	float				addVal, dist;

	for(p = self->p_root; p; p = p->next)
	{
		VectorSubtract(p->origin, fxi.cl->refdef.vieworg, part_dist);
		dist = VectorLengthSquared(part_dist);
		if(dist >= WATERPARTICLE_CLIPDIST)
		{
			SetupWaterParticle(p, true);
			continue;
		}

		addVal =  SINEAMT/128.0 * cl_turbsin[(int)((fxi.cl->time*0.001 + (self->origin[0] * 2.3 + p->origin[1])*.0015)*SINESCALE) & 255];
		addVal += SINEAMT/256.0 * cl_turbsin[(int)((fxi.cl->time*0.002 + (self->origin[1] * 2.3 + p->origin[0])*.0015)*SINESCALE) & 255];
				
		p->origin[2] += addVal;
		p->duration = fxi.cl->time + 10000000;
	}
}

qboolean WaterParticleGeneratorUpdate(client_entity_t *self, centity_t *owner)
{
	// Free up particles when we are not under water
	if(!cl_camera_under_surface->value)
	{
		if(water_particles_spawned)
		{
			FreeParticles(self);
			water_particles_spawned = false;
		}
	}
	else
	{
		if(!water_particles_spawned)
		{
			CreateWaterParticles(self);
			water_particles_spawned = true;
		}
		UpdateWaterParticles(self);
	}
 	return(true);
}


int wake_particle [6] =
{
	PART_4x4_WHITE,
	PART_8x8_BUBBLE,
	PART_16x16_WATERDROP,
	PART_32x32_WFALL,
	PART_32x32_STEAM,
	PART_32x32_BUBBLE
};


void DoWake(client_entity_t *self, centity_t *owner, int refpt)
{
	vec3_t				org, handpt, right, diff, diff2;
	client_particle_t	*p;
	matrix3_t			rotation;
	int					num_parts, i;
	paletteRGBA_t		LightColor={200, 255, 255, 140};//RGBA

	VectorSubtract(owner->referenceInfo->references[refpt].placement.origin, 
			owner->referenceInfo->oldReferences[refpt].placement.origin,
			diff);
	VectorSubtract(owner->origin, self->endpos, diff2);
	VectorAdd(diff, diff2, diff);
	num_parts = (int)(VectorLength(diff));
	if (num_parts > 6)
		num_parts = 6;

	// Let's take the origin and transform it to the proper coordinate offset from the owner's origin.
	VectorCopy(owner->referenceInfo->references[refpt].placement.origin, org);
	// Create a rotation matrix
	Matrix3FromAngles(owner->lerp_angles, rotation);
	Matrix3MultByVec3(rotation, org, handpt);
	VectorAdd(handpt, owner->origin, handpt);

	AngleVectors(owner->lerp_angles, NULL, right, NULL);

	for(i = 0; i < num_parts; i++)
	{
		if(r_detail->value > DETAIL_LOW)
			p = ClientParticle_new(wake_particle[irand(0, 5)], LightColor, irand(1000, 2000));
		else
			p = ClientParticle_new(wake_particle[irand(0, 5)]|PFL_SOFT_MASK, LightColor, irand(1000, 2000));

		VectorSet(p->origin, flrand(-4, 4), flrand(-4, 4), flrand(-4, 4));
		VectorAdd(handpt, p->origin, p->origin);
		
		p->scale = flrand(0.75, 1.5);
		p->color.a = irand(100, 200);

		VectorSet(p->velocity, flrand(-2, 2), flrand(-2, 2), flrand(-2.0, 2.0));

		if (irand(0, 1))
			VectorMA(p->velocity, flrand(-10, -2), right, p->velocity);
		else
			VectorMA(p->velocity, flrand(10, 2), right, p->velocity);

		p->acceleration[2] = 16;
//		p->d_alpha = flrand(-3, -1);
		p->d_scale = flrand(-0.15, -0.10);

		AddParticleToList(self, p);
	}
}


qboolean BubbleSpawner(client_entity_t *self, centity_t *owner)
{
	vec3_t	org;

	if(!cl_camera_under_surface->value)
		return(true);

	// Errr... what the hell, spawn some bubbles too.
	VectorSet(org, flrand(-20.0, 20.0), flrand(-20.0 ,20.0), flrand(-20.0 ,20.0));
	VectorAdd(org, owner->origin, org);
	MakeBubble(org, self);


	// Create a wake of bubbles!
	// ----------------------------------------------------
	// This tells if we are wasting our time, because the reference points are culled.
	if (r_detail->value < DETAIL_HIGH || !RefPointsValid(owner))
		return false;		// Remove the effect in this case.

	DoWake(self, owner, CORVUS_RIGHTHAND);
	DoWake(self, owner, CORVUS_LEFTHAND);
	DoWake(self, owner, CORVUS_RIGHTFOOT);
	DoWake(self, owner, CORVUS_LEFTFOOT);

	VectorCopy(owner->origin, self->endpos);

	return(true);
}


void FXWaterParticles(centity_t *owner, int type, int flags, vec3_t origin)
{			
	client_entity_t *effect;

	assert(owner);

	// Spawn static water particle handler
	effect = ClientEntity_new(type, flags | CEF_NO_DRAW | CEF_ABSOLUTE_PARTS| CEF_OWNERS_ORIGIN | CEF_VIEWSTATUSCHANGED, origin, NULL, PARTICLE_TRAIL_THINK_TIME);
	
	effect->AddToView = LinkedEntityUpdatePlacement;
	effect->radius = 100.0;
	effect->Update = WaterParticleGeneratorUpdate;

	AddEffect(owner, effect);

	// Spawn bubble spawner
	effect = ClientEntity_new(type, flags | CEF_NO_DRAW | CEF_ABSOLUTE_PARTS| CEF_OWNERS_ORIGIN | CEF_VIEWSTATUSCHANGED, origin, NULL, PARTICLE_TRAIL_THINK_TIME);

	effect->AddToView = LinkedEntityUpdatePlacement;
	effect->radius = 100.0;
	effect->Update = BubbleSpawner;
	VectorCopy(owner->origin, effect->endpos);

	AddEffect(owner, effect);
}

int DirectionalUpdate (client_entity_t *self, centity_t *owner)
{
	float				addVal;
	paletteRGBA_t		color = {255, 128, 128, 128};
	client_particle_t	*part;

	if(!compass->value)
	{
		self->updateTime = 2000;
		self->flags |= CEF_NO_DRAW;
		RemoveParticleList(&self->p_root);
		return (true);
	}

	self->flags &= ~CEF_NO_DRAW;

	VectorAdd(owner->lerp_origin, self->startpos, self->r.origin);
	
	addVal =  SINEAMT/2.0 * cl_turbsin[(int)((fxi.cl->time*0.001 + (self->r.origin[0] * 2.3 + self->startpos[1])*.0015)*SINESCALE) & 255];
	addVal += SINEAMT/4.0 * cl_turbsin[(int)((fxi.cl->time*0.002 + (self->r.origin[1] * 2.3 + self->startpos[0])*.0015)*SINESCALE) & 255];
			
	self->r.origin[2] += addVal;

	part = ClientParticle_new(self->classID, color, 100);

	VectorSet(part->origin, flrand(-12, 12), flrand(-12, 12), flrand(-8, 8));
	VectorAdd(self->r.origin, part->origin, part->origin);

	part->scale = flrand(1, 2);
	VectorSet(part->velocity, flrand(-5.0, 5.0), flrand(-5, 5.0), flrand(15.0, 22.0));
	part->acceleration[2] = 2;
	part->d_scale = flrand(-1.5, -2.0);
	part->d_alpha = flrand(-100.0, -50.0);
	part->duration = (color.a * 100.0) / -part->d_alpha;		// time taken to reach zero alpha

	AddParticleToList(self, part);
	
	self->updateTime = 20;

	return (true);
}

void FXCompass(centity_t *owner, int type, int flags, vec3_t origin)
{			
	client_entity_t *effect;
	int				rflags;

	assert(owner);

	flags |= CEF_ABSOLUTE_PARTS | CEF_DONT_LINK | CEF_ADDITIVE_PARTS | CEF_NO_DRAW;
	rflags = RF_GLOW | RF_TRANSLUCENT | RF_TRANS_ADD;
	// Spawn each directional
	//NORTH
	effect = ClientEntity_new(type, flags, origin, NULL, 25);
	
	effect->r.model = compass_models;
	effect->r.frame = 0;
	effect->classID = PART_16x16_SPARK_C;
	effect->r.flags = rflags;
	effect->radius = 100.0;
	VectorSet(effect->startpos, 0, 36, 0);
	effect->Update = DirectionalUpdate;

	AddEffect(owner, effect);

	//EAST
	effect = ClientEntity_new(type, flags, origin, NULL, 50);
	
	effect->r.model = compass_models;
	effect->r.frame = 1;
	effect->classID = PART_16x16_SPARK_Y;
	effect->r.flags = rflags;
	effect->radius = 100.0;
	VectorSet(effect->startpos, 36, 0, 0);
	effect->Update = DirectionalUpdate;

	AddEffect(owner, effect);

	//SOUTH
	effect = ClientEntity_new(type, flags, origin, NULL, 75);
	
	effect->r.model = compass_models;
	effect->r.frame = 2;
	effect->classID = PART_16x16_SPARK_G;
	effect->r.flags = rflags;
	effect->radius = 100.0;
	VectorSet(effect->startpos, 0, -36, 0);
	effect->Update = DirectionalUpdate;

	AddEffect(owner, effect);

	//WEST
	effect = ClientEntity_new(type, flags, origin, NULL, 100);
	
	effect->r.model = compass_models;
	effect->r.frame = 3;
	effect->classID = PART_16x16_SPARK_R;
	effect->r.flags = rflags;
	effect->radius = 100.0;
	VectorSet(effect->startpos, -36, 0, 0);
	effect->Update = DirectionalUpdate;

	AddEffect(owner, effect);
}

// -----------------------------------------

#define	NUM_FLAME_ITEMS		20
#define NUM_FLAME_PARTS		40
#define FLAME_ABSVEL		120

void FXCorpseRemove(centity_t *Owner, int Type, int Flags, vec3_t Origin)
{
	client_entity_t		*flameitem;
	float				curAng, vel, vel1;
	int					count, i;
	client_particle_t	*p;
	paletteRGBA_t		color;

	count = GetScaledCount(NUM_FLAME_ITEMS, 0.95);
	// Bound this between 8 and 16 sprites.
	if (count > 20)
		count=20;
	else if (count < 8)
		count=8;

	// create main client entity
	flameitem = ClientEntity_new(Type, Flags | CEF_NO_DRAW , Origin, NULL, 600);
	flameitem->radius = 10.0F;
	flameitem->color.c = 0xffffffff;
	AddEffect(NULL, flameitem);

	// are we destroying a rat ?
	if (Flags & CEF_FLAG6)
		vel1 = FLAME_ABSVEL/2;
	else
		vel1 = FLAME_ABSVEL;

	// large particles
	for(curAng = 0.0F; curAng < (M_PI * 2.0F); curAng += (M_PI * 2.0F) / count)
	{
		p = ClientParticle_new(PART_32x32_BLACKSMOKE, flameitem->color, 600);

		p->scale = 16.0;
		p->d_scale = -25.0;

		VectorSet(p->velocity, vel1 * cos(curAng), vel1 * sin(curAng), 0);
		VectorScale(p->velocity, -0.3, p->acceleration);
//		p->type |= PFL_ADDITIVE;

		AddParticleToList(flameitem, p);

	}

	color.c = 0xff4f4f4f;
	count = GetScaledCount(NUM_FLAME_PARTS, 0.1);
	// small particles
	for (i=0;i<count;i++)
	{

		p = ClientParticle_new(PART_4x4_WHITE, color, 600);

		p->scale = 1.0;
		p->d_scale = -1.0;

		curAng =  flrand(0,(M_PI * 2.0F));
		vel = flrand(vel1,vel1*2.5);
		VectorSet(p->velocity, vel * cos(curAng), vel * sin(curAng), 0);
		VectorScale(p->velocity, -0.3, p->acceleration);
		p->type |= PFL_ADDITIVE | PFL_SOFT_MASK;

		AddParticleToList(flameitem, p);

	}

}


/*
----------------------------------------

Leader effect routines

----------------------------------------
*/

#define LEADER_RAD		12
#define TOTAL_LEADER_EFFECTS 30
#define LEADER_EFFECTS_HEIGHT 30

// create the two circles that ring the player
static qboolean FXLeaderThink(struct client_entity_s *self, centity_t *owner)
{
	client_particle_t	*ce;
	paletteRGBA_t			color;

	if (!(--self->LifeTime))
	{
		self->LifeTime = TOTAL_LEADER_EFFECTS;
	}

	// if we are ghosted, don't do the effect
	if ((owner->current.renderfx & RF_TRANS_GHOST) || (owner->current.effects & EF_CLIENT_DEAD)) 
		return(true);

   	// create the ring of particles that goes up
   	color.c = 0x7fffffff;

   	// figure out how many particles we are going to use

   	ce = ClientParticle_new(PART_16x16_SPARK_Y, color, 800);
   	ce->acceleration[2] = 0.0; 
   	VectorSet(ce->origin, LEADER_RAD * cos(self->Scale), LEADER_RAD * sin(self->Scale), 4);
   	ce->scale = 8.0F;
   	AddParticleToList(self, ce);
   	// create the ring of particles that goes down

   	ce = ClientParticle_new(PART_16x16_SPARK_Y, color, 800);
   	ce->acceleration[2] = 0.0; 
   	VectorSet(ce->origin, LEADER_RAD * cos(self->Scale+3.14), LEADER_RAD * sin(self->Scale+3.14), 4);
   	ce->scale = 8.0F;
   	AddParticleToList(self, ce);

	// move the rings up/down next frame
	self->Scale += 0.17;

	return(true);
}

// create the entity the flight loops are on
void FXLeader(centity_t *owner, int type, int flags, vec3_t origin)
{
	client_entity_t		*glow;
														  
	glow = ClientEntity_new(type, flags | CEF_NO_DRAW | CEF_ADDITIVE_PARTS, origin, 0, 60);

	VectorClear(glow->origin);
	glow->Update = FXLeaderThink;
	glow->LifeTime = TOTAL_LEADER_EFFECTS;
	glow->AddToView = LinkedEntityUpdatePlacement;
	glow->Scale = 0;
	
	AddEffect(owner, glow);

}

#define FOOTTRAIL_RADIUS	2.0
#define FOOTTRAIL_SCALE	8.0
#define FOOTTRAIL_ACCEL	20.0


static qboolean FXFeetTrailThink(struct client_entity_s *self,centity_t *owner)
{

	client_particle_t	*flame;
	int					i;
	vec3_t				firestart, origin;
	matrix3_t			rotation;
	int					hand_flame_dur;
	paletteRGBA_t		color;
	int					count;
	vec3_t				curpos, diff;

	// This tells if we are wasting our time, because the reference points are culled.
	if (!RefPointsValid(owner))
		return true; 

	// if we are ghosted, don't do the effect
	if ((owner->current.renderfx & RF_TRANS_GHOST) || (owner->current.effects & EF_CLIENT_DEAD)) 
		return(true);

	if (!(owner->current.effects & EF_SPEED_ACTIVE))
	{
		self->Update=RemoveSelfAI;
		self->updateTime = fxi.cl->time + 1500;
		return true ;
	}

	// Let's take the origin and transform it to the proper coordinate offset from the owner's origin.
	VectorCopy(owner->referenceInfo->references[self->refPoint].placement.origin, firestart);
	// Create a rotation matrix
	Matrix3FromAngles(owner->lerp_angles, rotation);
	Matrix3MultByVec3(rotation, firestart, origin);
	VectorAdd(origin, owner->origin, origin);

	if (Vec3NotZero(self->origin))
	{
		
		// create small particles
		count = GetScaledCount(5, 0.5);
		VectorSubtract(self->origin, origin, diff);
		Vec3ScaleAssign((1.0 / count), diff);
		VectorClear(curpos);

		if (r_detail->value < DETAIL_NORMAL)
			hand_flame_dur = 1500;
		else
			hand_flame_dur = 2000;

		for(i = 0; i < count; i++)
		{
	  		color.c = 0xffffff40;

			flame = ClientParticle_new(PART_32x32_STEAM, color, hand_flame_dur);
			VectorSet(	flame->origin, 
						flrand(-FOOTTRAIL_RADIUS, FOOTTRAIL_RADIUS), 
						flrand(-FOOTTRAIL_RADIUS, FOOTTRAIL_RADIUS), 
						flrand(-FOOTTRAIL_RADIUS, FOOTTRAIL_RADIUS));
			VectorAdd(flame->origin, self->origin, flame->origin);
			VectorAdd(flame->origin, curpos, flame->origin);
		
			flame->scale = FOOTTRAIL_SCALE;
			VectorSet(flame->velocity, flrand(-5.0, 5.0), flrand(-5, 5.0), flrand(5.0, 15.0));
			flame->acceleration[2] = FOOTTRAIL_ACCEL;
			flame->d_scale = flrand(-10.0, -5.0);
			flame->d_alpha = flrand(-200.0, -160.0);
			flame->duration = (255.0 * 1000.0) / -flame->d_alpha;		// time taken to reach zero alpha

			AddParticleToList(self, flame);
			Vec3SubtractAssign(diff, curpos);
		}
	}
  
	VectorCopy(origin, self->origin);

	
	return(true);
}

// ************************************************************************************************
// FXFeetTrail
// ------------
// ************************************************************************************************

void FXFeetTrail(centity_t *owner,int type,int flags,vec3_t origin)
{
	short			refpoints;
	client_entity_t	*trail;
	int				i;
	int				flame_dur;

	refpoints=(1 << CORVUS_LEFTFOOT) | (1 << CORVUS_RIGHTFOOT);

	VectorClear(origin);

	if (r_detail->value > DETAIL_NORMAL)
		flame_dur = 50;
	else
		flame_dur = 75;

	// Add a fiery trail effect to the player's hands / feet etc.

	for(i=0;i<16;i++)
	{
		if(!(refpoints & (1 << i)))
			continue;

		trail=ClientEntity_new(type,flags,origin,0,flame_dur);

		VectorClear(trail->origin);
		trail->Update=FXFeetTrailThink;
		trail->flags|=CEF_NO_DRAW | CEF_OWNERS_ORIGIN | CEF_ABSOLUTE_PARTS;
		trail->radius = 40;
		trail->AddToView = LinkedEntityUpdatePlacement;			
		trail->refPoint = i;
		trail->color.c = 0xe5007fff;

		AddEffect(owner,trail);
	}
}


// end


