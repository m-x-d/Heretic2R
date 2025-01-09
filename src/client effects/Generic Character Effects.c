//
// Generic Character Effects.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Client Entities.h"
#include "Particle.h"
#include "Vector.h"
#include "Random.h"
#include "Utilities.h"
#include "g_playstats.h"
#include "Matrix.h"
#include "Reference.h"
#include "turbsin.h"

static struct model_s* genfx_models[6];

void PrecacheOgleHitPuff(void)
{
	genfx_models[0] = fxi.RegisterModel("sprites/fx/steam_add.sp2");
	genfx_models[1] = fxi.RegisterModel("models/debris/stone/schunk1/tris.fm");
	genfx_models[2] = fxi.RegisterModel("models/debris/stone/schunk2/tris.fm");
	genfx_models[3] = fxi.RegisterModel("models/debris/stone/schunk3/tris.fm");
	genfx_models[4] = fxi.RegisterModel("models/debris/stone/schunk4/tris.fm");
	genfx_models[5] = fxi.RegisterModel("sprites/fx/halo.sp2");
}

void PreCacheWaterParticles(void) { } //TODO: remove?

static qboolean ParticleTrailAI(const client_entity_t* this, const centity_t* owner)
{
#define PARTICLE_TRAIL_PUFF_TIME 1000 // Puffs last for 1 sec.

	assert(owner);

	client_entity_t* effect = ClientEntity_new(FX_PUFF, CEF_NO_DRAW, owner->current.old_origin, NULL, PARTICLE_TRAIL_PUFF_TIME);

	for (int i = 0; i < 40; i++)
	{
		client_particle_t* p = ClientParticle_new(PART_4x4_WHITE, this->color, PARTICLE_TRAIL_PUFF_TIME);
		VectorSet(p->velocity, flrand(-20.0f, 20.0f), flrand(-20.0f, 20.0f), flrand(30.0f, 80.0f));
		AddParticleToList(effect, p);
	}

	AddEffect(NULL, effect); // Add the puff to the world.

	return true; // Actual puff spawner only goes away when it owner has a FX_REMOVE_EFFECTS sent on it.
}

void GenericGibTrail(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	assert(owner);

	client_entity_t* effect = ClientEntity_new(type, flags, origin, NULL, PARTICLE_TRAIL_THINK_TIME);
	effect->flags |= CEF_NO_DRAW;
	effect->color.c = 0xFF2020FF;
	effect->Update = ParticleTrailAI;

	AddEffect(owner, effect);
	ParticleTrailAI(effect, owner); // Think once right away, to spawn the first puff.
}

static qboolean PebbleUpdate(struct client_entity_s* self, centity_t* owner)
{
	const int cur_time = fxi.cl->time;
	const float d_time = (float)(cur_time - self->lastThinkTime) / 1000.0f;

	self->acceleration[2] -= 75.0f;
	self->r.angles[0] += ANGLE_360 * d_time;
	self->r.angles[1] += ANGLE_360 * d_time;

	self->lastThinkTime = cur_time;

	return cur_time <= self->LifeTime;
}

// Slight variation on the normal puff.
void FXOgleHitPuff(centity_t* owner, const int type, const int flags, const vec3_t origin)
{
	vec3_t dir;
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_OGLE_HITPUFF].formatString, dir); // Normalized direction vector.

	const float speed = VectorNormalize(dir);
	const int count = (speed > 1.0f ? irand(10, 15) : irand(1, 4));

	for (int i = 0; i < count; i++)
	{
		// Puff!
		client_entity_t* puff = ClientEntity_new(type, flags, origin, NULL, 500);

		puff->r.model = &genfx_models[0];
		puff->r.flags |= RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;

		vec3_t work;
		VectorRandomCopy(dir, work, 0.5f);

		if (speed > 1.0f)
			VectorScale(work, speed, puff->velocity);
		else if (flags & CEF_FLAG6)
			VectorScale(work, flrand(8.0f, 12.0f), puff->velocity);
		else
			VectorScale(work, 4.0f, puff->velocity);

		puff->acceleration[2] = flrand(10.0f, 50.0f);
		puff->alpha = 0.35f;
		puff->r.scale = (speed > 1.0f ? flrand(0.3f, 0.75f) : 0.1f);
		puff->d_scale = 2.0f;
		puff->d_alpha = -2.0f;

		AddEffect(NULL, puff); // Add the effect as independent world effect.
	}

	for (int i = 0; i < count; i++)
	{
		// Rock!
		client_entity_t* rock = ClientEntity_new(type, flags, origin, NULL, 50);

		rock->r.model = &genfx_models[irand(1, 4)];

		vec3_t work;
		VectorRandomCopy(dir, work, 0.5f);
		VectorScale(work, (speed > 1.0f ? speed : flrand(8.0f, 16.0f)), rock->velocity);

		if (flags & CEF_FLAG6 || speed > 1.0f)
			VectorSet(rock->acceleration, flrand(-75.0f, 75.0f), flrand(-75.0f, 75.0f), flrand(125.0f, 250.0f));
		else
			rock->acceleration[2] = flrand(-50.0f, 50.0f);

		rock->Update = PebbleUpdate;

		if (speed > 1.0f)
			rock->r.scale = flrand(0.8f, 1.5f) * speed / 100;
		else
			rock->r.scale = flrand(0.1f, 0.25f);

		rock->d_scale = 0.0f;
		rock->d_alpha = 0.0f;
		rock->LifeTime = fxi.cl->time + 5000;

		AddEffect(NULL, rock); // Add the effect as independent world effect.
	}
}

void FXGenericHitPuff(centity_t* owner, const int type, const int flags, const vec3_t origin)
{
	byte count;
	vec3_t dir;
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_HITPUFF].formatString, dir, &count); // Normalized direction vector.
	count = min(40, count);

	for (int i = 0; i < count; i++)
	{
		client_entity_t* fx = ClientEntity_new(type, flags, origin, NULL, 500);

		fx->r.model = &genfx_models[0];
		fx->r.flags |= RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;

		vec3_t work;
		VectorRandomCopy(dir, work, 0.5f);
		VectorScale(work, 50.0f, fx->velocity);
		fx->acceleration[2] = -100.0f;
		fx->alpha = 0.5f;
		fx->r.scale = 0.1f;
		fx->d_scale = 1.0f;
		fx->d_alpha = -1.0f;

		AddEffect(NULL, fx); // Add the effect as independent world effect.
	}

	if (flags & CEF_FLAG6)
	{
		// High-intensity impact point.
		client_entity_t* impact = ClientEntity_new(type, flags | CEF_DONT_LINK, origin, NULL, 250);

		impact->r.model = &genfx_models[5];
		impact->r.flags = RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		impact->r.scale = 0.4f;
		impact->d_scale = -1.6f;
		impact->d_alpha = -3.0f; // Alpha goes up to 1, then down to zero.
		impact->color.c = 0xc0ffffff;
		impact->radius = 10.0f;
		impact->alpha = 0.8f;
		impact->r.origin[2] += 8.0f;

		AddEffect(NULL, impact); // Add the effect as independent world effect.
	}
}

#define SINEAMT					1
#define SINESCALE				(256.0f / (2 * M_PI))
#define WATER_DENSITY			150.0f
#define WATER_DIST				100.0f
#define WATERPARTICLE_CLIPDIST	(WATER_DIST * WATER_DIST)

static void SetupWaterParticle(client_particle_t* p, const qboolean recycle)
{
	const byte ishade = (byte)irand(50, 150);
	p->color.r = ishade;
	p->color.g = ishade;
	p->color.b = ishade;
	p->color.a = (byte)irand(75, 150);

	const float min_vel_z = (((float)ishade * 0.04f) - 3.1f) * 0.35f;
	VectorSet(p->velocity, flrand(-1.0f, 1.0f), flrand(-1.0f, 1.0f), flrand(min_vel_z, 0.0f));

	p->acceleration[2] = 0.0f;
	p->scale = flrand(0.3f, 0.7f);

	vec3_t dist;
	VectorSet(dist, flrand(-WATER_DIST, WATER_DIST), flrand(-WATER_DIST, WATER_DIST), flrand(-WATER_DIST, WATER_DIST));

	// If we are recycling, we want to respawn as far away as possible.
	if (recycle)
	{
		VectorNormalize(dist);
		Vec3ScaleAssign(WATER_DIST, dist);
	}

	VectorAdd(fxi.cl->refdef.vieworg, dist, p->origin);
}

static void CreateWaterParticles(client_entity_t* self)
{
	// Scale number of particles by detail level.
	const int detail = (int)(WATER_DENSITY * r_detail->value);

	for (int i = 0; i < detail; i++)
	{
		client_particle_t* p = ClientParticle_new(PART_4x4_WHITE | PFL_SOFT_MASK, self->color, 1000000);
		SetupWaterParticle(p, false);
		AddParticleToList(self, p);
	}
}

static void UpdateWaterParticles(const client_entity_t* self)
{
	for (client_particle_t* p = self->p_root; p != NULL; p = p->next)
	{
		vec3_t part_dist;
		VectorSubtract(p->origin, fxi.cl->refdef.vieworg, part_dist);
		const float dist = VectorLengthSquared(part_dist);

		if (dist >= WATERPARTICLE_CLIPDIST)
		{
			SetupWaterParticle(p, true);
			continue;
		}

		float add_val = SINEAMT / 128.0f * turbsin[(int)(((float)fxi.cl->time * 0.001f + (self->origin[0] * 2.3f + p->origin[1]) * 0.0015f) * SINESCALE) & 255];
		add_val +=		SINEAMT / 256.0f * turbsin[(int)(((float)fxi.cl->time * 0.002f + (self->origin[1] * 2.3f + p->origin[0]) * 0.0015f) * SINESCALE) & 255];

		p->origin[2] += add_val;
		p->duration = fxi.cl->time + 10000000;
	}
}

static qboolean WaterParticleGeneratorUpdate(client_entity_t* self, centity_t* owner)
{
	static qboolean water_particles_spawned;

	if ((int)cl_camera_under_surface->value)
	{
		// Create particles when we are under water.
		if (!water_particles_spawned)
		{
			CreateWaterParticles(self);
			water_particles_spawned = true;
		}

		UpdateWaterParticles(self);
	}
	else
	{
		// Free up particles when we are not under water.
		if (water_particles_spawned)
		{
			FreeParticles(self);
			water_particles_spawned = false;
		}
	}

	return true;
}

static void DoWake(client_entity_t* self, const centity_t* owner, const int refpt)
{
	static int wake_particle[6] =
	{
		PART_4x4_WHITE,
		PART_8x8_BUBBLE,
		PART_16x16_WATERDROP,
		PART_32x32_WFALL,
		PART_32x32_STEAM,
		PART_32x32_BUBBLE
	};

	vec3_t org;
	vec3_t handpt;
	vec3_t right;
	vec3_t diff;
	vec3_t diff2;
	matrix3_t rotation;

	const paletteRGBA_t light_color = { .r = 200, .g = 255, .b = 255, .a = 140 };

	VectorSubtract(owner->referenceInfo->references[refpt].placement.origin, owner->referenceInfo->oldReferences[refpt].placement.origin, diff);
	VectorSubtract(owner->origin, self->endpos, diff2);
	VectorAdd(diff, diff2, diff);

	int num_parts = (int)(VectorLength(diff));
	num_parts = min(6, num_parts);

	// Let's take the origin and transform it to the proper coordinate offset from the owner's origin.
	VectorCopy(owner->referenceInfo->references[refpt].placement.origin, org);

	// Create a rotation matrix
	Matrix3FromAngles(owner->lerp_angles, rotation);
	Matrix3MultByVec3(rotation, org, handpt);
	VectorAdd(handpt, owner->origin, handpt);

	AngleVectors(owner->lerp_angles, NULL, right, NULL);

	for (int i = 0; i < num_parts; i++)
	{
		int type = wake_particle[irand(0, 5)];
		if ((int)r_detail->value == DETAIL_LOW)
			type |= PFL_SOFT_MASK;

		client_particle_t* p = ClientParticle_new(type, light_color, irand(1000, 2000));

		VectorSet(p->origin, flrand(-4.0f, 4.0f), flrand(-4.0f, 4.0f), flrand(-4.0f, 4.0f));
		VectorAdd(handpt, p->origin, p->origin);

		p->scale = flrand(0.75f, 1.5f);
		p->color.a = (byte)irand(100, 200);

		VectorSet(p->velocity, flrand(-2.0f, 2.0f), flrand(-2.0f, 2.0f), flrand(-2.0f, 2.0f));

		const float sign = (irand(0, 1) ? -1.0f : 1.0f);
		VectorMA(p->velocity, flrand(10, 2) * sign, right, p->velocity);

		p->acceleration[2] = 16.0f;
		p->d_scale = flrand(-0.15f, -0.1f);

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


