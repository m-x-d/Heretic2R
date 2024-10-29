//
// fx_staff.c
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
#include "Reference.h"
#include "Utilities.h"
#include "g_playstats.h"

#define BLOOD_SPEED					40.0F
#define NUM_BLOOD_SPURT_PARTS		20
#define NUM_INSECT_BLOOD_PARTICLES	12

extern int ref_soft;

#define	NUM_BLOOD_MODELS	2
static struct model_s *splat_models[NUM_BLOOD_MODELS];
void PreCacheSplat()
{
	splat_models[0] = fxi.RegisterModel("sprites/fx/bsplat.sp2");
	splat_models[1] = fxi.RegisterModel("sprites/fx/ysplat.sp2");
}

int InsectBloodParticle [NUM_INSECT_BLOOD_PARTICLES] =
{
	PART_4x4_GREEN,         
	PART_4x4_YELLOW,        
	PART_8x8_GLOBBIT1,         
	PART_8x8_GLOBBIT2,         
	PART_16x16_MIST,       
	PART_16x16_GLOB,      
	PART_16x16_SPARK_G,
	PART_16x16_SPARK_Y,
	PART_32x32_GREENBLOOD,
	PART_16x16_GREENBLOOD,
	PART_4x4_GREENBLOOD1,
	PART_4x4_GREENBLOOD2
};

client_entity_t *DoBloodSplash(vec3_t loc, int amount, qboolean yellow_blood)
{
	client_entity_t		*splash;
	client_particle_t	*drop;
	int					i, j;
	paletteRGBA_t		pal;
	paletteRGBA_t		pal2;
	float				speed, grav;
	int					size;
	int					bpart;

	splash = ClientEntity_new(FX_BLOOD, CEF_NO_DRAW, loc, NULL, 1000);
	AddEffect(NULL, splash);

	speed = 16 + 4*amount;
	grav = GetGravity()*0.5;
	pal.c = 0xffffffff;
	pal2.c = 0xff0000ff;
	if (amount>500)
		amount=500;
	for (i=0; i<amount; i++)
	{
		size = i&0x03;	// size=0-3
		switch(size)
		{
		case 0:	// Tiny particles.
			for (j=0; j<6; j++)
			{
				if (ref_soft)
				{
					if(yellow_blood)
						bpart = PART_4x4_GREENBLOOD1;
					else
						bpart = PART_4x4_BLOOD1;
					drop = ClientParticle_new(bpart | PFL_SOFT_MASK, pal2, 650);
				}
				else
				{
					if(yellow_blood)
						bpart = irand(PART_4x4_GREENBLOOD1, PART_4x4_GREENBLOOD2);
					else
						bpart = irand(PART_4x4_BLOOD1, PART_4x4_BLOOD2);
					drop = ClientParticle_new(bpart, pal, 800);
				}
				VectorSet(drop->velocity, flrand(-speed, speed), flrand(-speed, speed), flrand(speed*2.0, speed*4.0));
				drop->acceleration[2] = grav;
				drop->d_alpha = 0.0;
				drop->d_scale = -1.0;
				AddParticleToList(splash, drop);
			}
			break;
		case 1:	// Some larger globs
			for (j=0; j<3; j++)
			{
				if (ref_soft)
				{
					if(yellow_blood)
						bpart = PART_8x8_GLOBBIT1;
					else
						bpart = PART_8x8_BLOOD;
					drop = ClientParticle_new(bpart | PFL_SOFT_MASK, pal2, 650);
				}
				else
				{
					if(yellow_blood)
						bpart = irand(PART_8x8_GLOBBIT1, PART_8x8_GLOBBIT2);
					else
						bpart = PART_8x8_BLOOD;
					drop = ClientParticle_new(bpart, pal, 800);
				}
				VectorSet(drop->velocity, flrand(-speed, speed), flrand(-speed, speed), flrand(speed*2.0, speed*4.0));
				drop->acceleration[2] = grav;
				drop->d_alpha = 0.0;
				drop->scale = 2.0;
				drop->d_scale = -2.0;
				AddParticleToList(splash, drop);
			}
			break;
		case 2:	// Some big blobs
			for (j=0; j<2; j++)
			{
				if(yellow_blood)
					bpart = PART_16x16_GREENBLOOD;
				else
					bpart = PART_16x16_BLOOD;
				drop = ClientParticle_new(bpart, pal, 1000);
				VectorSet(drop->velocity, flrand(-speed, speed), flrand(-speed, speed), flrand(speed, speed*2.0));
				drop->acceleration[2] = grav*0.5;
				drop->d_alpha = flrand(-512.0, -256.0);
				drop->scale = 1.0;
				drop->d_scale = flrand(4.0, 8.0);
				AddParticleToList(splash, drop);
			}
			break;
		case 3:	// A big splash
			if(yellow_blood)
				bpart = PART_32x32_GREENBLOOD;
			else
				bpart = PART_32x32_BLOOD;
			drop = ClientParticle_new(bpart, pal, 500);
			VectorSet(drop->velocity, flrand(-speed, speed), flrand(-speed, speed), flrand(0, speed));
			drop->scale = 4.0;
			drop->acceleration[2] = 0.0;
			drop->d_scale = flrand(48.0, 64.0);
			drop->d_alpha = flrand(-1024.0, -512.0);
			AddParticleToList(splash, drop);
			break;
		}
	}

	return splash;
}

void DoBloodTrail(client_entity_t *spawner, int amount)
{
	client_particle_t	*drop;
	int					i, j;
	paletteRGBA_t		pal;
	paletteRGBA_t		pal2;
	float				speed, grav, range;
	int					size;
	qboolean			yellow_blood = false;

	if((spawner->SpawnInfo&SIF_FLAG_MASK) == MAT_INSECT)//insect blood is yellow-green
		yellow_blood = true;

	if(amount == -1)
		speed = 0;
	else
		speed = 8 + 4*amount;
	grav = GetGravity()*0.5;
	range = (float)amount;
	pal.c = 0xffffffff;
	pal2.c = 0xff0000ff;
	if (amount>500)
		amount=500;
	for (i=0; i<amount; i++)
	{
		size = i&0x01;	// size=0-3
		switch(size)
		{
		case 0:	// Tiny particles.
			for (j=0; j<3; j++)
			{
				if (ref_soft)
				{
					if(yellow_blood)
						drop = ClientParticle_new(InsectBloodParticle[irand(0, NUM_INSECT_BLOOD_PARTICLES - 1)] | PFL_SOFT_MASK, pal2, 800);
					else
						drop = ClientParticle_new(PART_4x4_BLOOD1 | PFL_SOFT_MASK, pal2, 800);
				}
				else
				{
					if(yellow_blood)
						drop = ClientParticle_new(InsectBloodParticle[irand(0, NUM_INSECT_BLOOD_PARTICLES - 1)], pal, 800);
					else
						drop = ClientParticle_new(irand(PART_4x4_BLOOD1, PART_4x4_BLOOD2), pal, 800);
				}
				VectorSet(drop->velocity, flrand(-speed, speed), flrand(-speed, speed), flrand(speed*2.0, speed*4.0));
				VectorMA(drop->velocity, 0.5, spawner->velocity, drop->velocity);
				VectorSet(drop->origin, flrand(-range, range), flrand(-range, range), flrand(-range, range));
				VectorAdd(drop->origin, spawner->r.origin, drop->origin);
				drop->acceleration[2] = grav;
				drop->d_alpha = 0.0;
				drop->d_scale = -1.0;
				AddParticleToList(spawner, drop);
			}
			break;
		case 1:	// Some larger globs
			if(yellow_blood)
				drop = ClientParticle_new(InsectBloodParticle[irand(0, NUM_INSECT_BLOOD_PARTICLES - 1)], pal, 800);
			else
				drop = ClientParticle_new(PART_8x8_BLOOD, pal, 800);
			VectorSet(drop->velocity, flrand(-speed, speed), flrand(-speed, speed), flrand(speed*2.0, speed*4.0));
			VectorMA(drop->velocity, 0.5, spawner->velocity, drop->velocity);
			VectorSet(drop->origin, flrand(-range, range), flrand(-range, range), flrand(-range, range));
			VectorAdd(drop->origin, spawner->r.origin, drop->origin);
			drop->acceleration[2] = grav;
			drop->d_alpha = 0.0;
			drop->scale = 2.0;
			drop->d_scale = -2.0;
			AddParticleToList(spawner, drop);
			break;
		}
	}
}

// --------------------------------------------------------------
// Find exact plane to decal the bloodmark to

static qboolean GetTruePlane(vec3_t origin, vec3_t direction)
{
	trace_t		trace;
	vec3_t		end;
	vec3_t		mins, maxs;

	VectorClear(mins);
	VectorClear(maxs);

	VectorMA(origin, 16.0, direction, end);

	fxi.Trace(origin, mins, maxs, end, MASK_DRIP, CEF_CLIP_TO_WORLD, &trace);
	if(trace.fraction != 1.0)
	{
		// Set the new endpos and plane (should be exact)
		VectorCopy(trace.endpos, origin);
		VectorCopy(trace.plane.normal, direction);

		VectorMA(origin, 0.25, direction, origin);
		return(true);
	}
	return(false);
}

qboolean BloodSplatDripUpdate (client_entity_t *self, centity_t *owner);
qboolean BloodSplatSplishUpdate (client_entity_t *self, centity_t *owner)
{
	client_particle_t	*p;
	paletteRGBA_t		color = {180, 140, 110, 160};
	vec3_t				vel;
	qboolean			yellow_blood = false;
	int					bpart;

	if(self->flags&CEF_FLAG8)
		yellow_blood = true;

	if (self->SpawnInfo>500)
		return true;
	while(self->SpawnInfo>0)
	{
		if (ref_soft)
		{
			if(yellow_blood)
				bpart = InsectBloodParticle[irand(0, NUM_INSECT_BLOOD_PARTICLES - 1)];
			else
				bpart = PART_4x4_BLOOD1;
			p = ClientParticle_new(bpart|PFL_SOFT_MASK, color, 800);
		}
		else
		{
			if(yellow_blood)
				bpart = InsectBloodParticle[irand(0, NUM_INSECT_BLOOD_PARTICLES - 1)];
			else
				bpart = irand(PART_4x4_BLOOD1, PART_4x4_BLOOD2);
			p = ClientParticle_new(bpart, color, 800);
		}
		p->acceleration[2] = GetGravity() * 0.2;
		VectorRandomCopy(self->endpos2, vel, 10.0F);
		VectorScale(vel, flrand(2, 5), p->velocity);
		p->d_alpha = 0;
		p->scale = flrand(0.4, 0.8);
		VectorAdd(self->startpos2, self->startpos, p->origin);
		AddParticleToList(self, p);
		self->SpawnInfo--;
	}
	fxi.S_StartSound(p->origin, -1, CHAN_AUTO,
		fxi.S_RegisterSound(va("ambient/waterdrop%c.wav", irand('1', '3'))), flrand(0.5, 0.8), ATTN_STATIC, 0);

	if(!irand(0, 2))
		VectorSet(self->startpos, flrand(-1, 1), flrand(-1, 1), 0);

	self->Update = BloodSplatDripUpdate;
	self->updateTime = irand(400, 1600);

	return (true);
}

qboolean BloodSplatDripUpdate (client_entity_t *self, centity_t *owner)
{
	client_particle_t	*p;
	paletteRGBA_t		color = {150, 140, 110, 160};
	int					num_drips, i;
	float				scale, grav_mod, driptime;
	qboolean			yellow_blood = false;
	int					bpart;

	if(!AttemptRemoveSelf(self, owner))
		return (false);

	if(self->flags&CEF_FLAG8)
		yellow_blood = true;

	//drip- fiXME: make p duration based on self->radius?
	driptime = self->radius * 6;
	scale = flrand(0.2, 0.4);
	num_drips = irand(7, 15);
	for(i = 0; i < num_drips; i++)
	{
		if (ref_soft)
		{
			if(yellow_blood)
				bpart = InsectBloodParticle[irand(0, NUM_INSECT_BLOOD_PARTICLES - 1)];
			else
				bpart = PART_4x4_BLOOD1;
			p = ClientParticle_new(bpart | PFL_SOFT_MASK, color, 1600);
		}
		else
		{
			if(yellow_blood)
				bpart = InsectBloodParticle[irand(0, NUM_INSECT_BLOOD_PARTICLES - 1)];
			else
				bpart = irand(PART_4x4_BLOOD1, PART_4x4_BLOOD2);
			p = ClientParticle_new(bpart, color, 3200);
		}
		grav_mod = (0.4 + (i * 0.025));
		p->acceleration[2] = GetGravity() * grav_mod;
		p->d_alpha = 0;
		p->scale = scale + (i * 0.08);
		VectorCopy(self->startpos, p->origin);

		AdvanceParticle(p, 7 * i);
		
		AddParticleToList(self, p);

		if(driptime >= 17)
		{
			p->duration = driptime * 3 * grav_mod;
			self->SpawnInfo++;
		}
	}

	if(self->SpawnInfo && driptime >= 17)
	{//splash
		self->Update = BloodSplatSplishUpdate;
		self->updateTime = driptime;
	}
	else
	{
		if(!irand(0, 3))
			VectorSet(self->startpos, flrand(-1, 1), flrand(-1, 1), 0);

		self->updateTime = irand(400, 1600);
	}

	return (true);
}

void ThrowBlood(vec3_t origin, vec3_t tnormal, qboolean dark, qboolean yellow, qboolean trueplane)
{
	client_entity_t		*bloodmark;
	vec3_t				normal;
	int					brightness;

	VectorCopy(tnormal, normal);
	VectorScale(normal, -1, normal);

	if(trueplane || GetTruePlane(origin, normal))
	{
		bloodmark = ClientEntity_new(FX_BLOOD, CEF_NOMOVE, origin, tnormal, 1000);

		bloodmark->r.angles[ROLL] = flrand(0, ANGLE_360);

		if(yellow)
			bloodmark->r.model = splat_models + 1;
		else
			bloodmark->r.model = splat_models;
		bloodmark->r.flags = 0;
		bloodmark->r.flags |= RF_FIXED | RF_ALPHA_TEXTURE;
		bloodmark->r.frame = irand(0,4);

		if(dark)		
		{
			brightness = irand(32, 72);
			bloodmark->r.color.r = brightness;
			bloodmark->r.color.g = brightness;
			bloodmark->r.color.b = brightness;
		}
		else
		{
			brightness = irand(72, 128);
			bloodmark->r.color.r = brightness;
			bloodmark->r.color.g = brightness;
			bloodmark->r.color.b = brightness;
		}
		bloodmark->r.color.a = 255;
		bloodmark->alpha = 1.0;
		
		bloodmark->radius = 10.0;
		bloodmark->r.scale = flrand(0.2, 0.45);

		if(tnormal[2] <= -0.7 && !irand(0, 2) && bloodmark->r.frame != 2 && bloodmark->r.frame != 4)
		{
			trace_t		extratrace;
			vec3_t		endpos;

			VectorMA(origin, 256, tnormal, endpos);
			fxi.Trace(origin, vec3_origin, vec3_origin, endpos, MASK_DRIP, CEF_CLIP_TO_WORLD, &extratrace);
			if(extratrace.fraction<1.0 && extratrace.fraction > 0.0625)//bewteen 16 and 256
			{
				if(extratrace.plane.normal[2] >= 0.7)
				{
					bloodmark->radius = extratrace.fraction * 256;
					VectorSubtract(extratrace.endpos, origin, bloodmark->startpos2);
					VectorCopy(extratrace.plane.normal, bloodmark->endpos2);
					ThrowBlood(extratrace.endpos, extratrace.plane.normal, dark, yellow, false);
				}
			}

			VectorClear(bloodmark->startpos);
			bloodmark->LifeTime = 0;
			if(yellow)
				bloodmark->flags |= CEF_FLAG8;
			bloodmark->Update = BloodSplatDripUpdate;
		}
		else
		{
			bloodmark->Update = AttemptRemoveSelf;
		}
		
		AddEffect(NULL, bloodmark);
		InsertInCircularList(bloodmark);
	}
}

void FXBloodTrail(centity_t *owner, int type, int flags, vec3_t origin)
{
	vec3_t			normal;
	
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_BLOOD_TRAIL].formatString, &normal);

	VectorMA(origin, 0.25, normal, origin);
	ThrowBlood(origin, normal, flags&CEF_FLAG7, flags&CEF_FLAG6, true);
}

// ClientEffect FX_BLOOD
void FXBlood(centity_t *owner, int type, int flags, vec3_t origin)
{
	client_entity_t		*spawner;
	byte				amount;
	vec3_t				velocity;
	qboolean			yellow_blood = false;

	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_BLOOD].formatString, &velocity, &amount);
	
	if(flags&CEF_FLAG8)
		yellow_blood = true;

	// lets add level of detail here to the amount we pump out
	switch((int)r_detail->value)
	{
	// half as much
	case DETAIL_LOW:
		amount = ((float)amount * 0.5);	 
		break;

	// 3 quarters
	case DETAIL_NORMAL:
		amount = ((float)amount * 0.75);	 
		break;

	default:
		break;
	}
	if (!amount)
		amount = 1;

	spawner = DoBloodSplash(origin, amount, yellow_blood);
	VectorCopy(velocity, spawner->velocity);
}

// ------------------------------------------------------------------------------

#define	NUM_BLOOD_PARTS		3

static qboolean LinkedBloodThink(client_entity_t *spawner, centity_t *owner)
{
	client_particle_t	*p;
	client_entity_t		*ce;
	vec3_t				org, vel;
	int					i;
	int					bpart;
	qboolean			yellow_blood;

	spawner->updateTime = irand(40, 60);

	spawner->LifeTime -= 50;
	if(spawner->LifeTime < 0)					// Effect finished
		return(false);

	if(spawner->LifeTime < 800)					// Effect needs to stay alive until particles die
		return(true);

	if(spawner->flags&CEF_FLAG8)//yellow
		yellow_blood = true;

	// This tells if we are wasting our time, because the reference points are culled.
	if (!RefPointsValid(owner))
		return false;		// Just end the effect.

	VectorGetOffsetOrigin(owner->referenceInfo->references[spawner->SpawnInfo].placement.origin, owner->current.origin, owner->current.angles[YAW], org);
	ce = ClientEntity_new(-1, 0, org, NULL, 800);
	ce->flags |= CEF_NO_DRAW | CEF_NOMOVE;
	ce->radius = 32.0;
	AddEffect(NULL, ce);

	VectorSubtract(org, spawner->origin, vel);
	Vec3ScaleAssign(5.0, vel);

	for(i = 0; i < NUM_BLOOD_PARTS; i++)
	{
		if(yellow_blood)
			bpart = InsectBloodParticle[irand(0, NUM_INSECT_BLOOD_PARTICLES - 1)];
		else
			bpart = irand(PART_4x4_BLOOD1, PART_4x4_BLOOD2);
		p = ClientParticle_new(bpart, spawner->color, 800);
		Vec3AddAssign(vel, p->velocity);
		p->acceleration[2] = GetGravity();
		p->d_alpha = 0.0;
		p->d_scale = -1.0;
		AddParticleToList(ce, p);
	}

	// Remember current origin for calc of velocity
	VectorCopy(org, spawner->origin);
	return(true);
}

void FXLinkedBlood(centity_t *owner, int type, int flags, vec3_t origin)
{
	client_entity_t		*spawner;
	byte				refpointidx, life;
	int					count, i;
	int					lifetime = 0;

	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_LINKEDBLOOD].formatString, &life, &refpointidx);
	lifetime = life * 50;
	count = 1;
	if(life > 1600)
	{
		count = (life - 1600) / 100;
		life = 1600;
		if(count > 10)				// Max out saftey check
			count = 10; 
	}

	for(i = 0; i < count; i++)
	{
		spawner = ClientEntity_new(type, flags, origin, NULL, Q_ftol(fxi.cls->frametime * 2000.0));

		spawner->LifeTime = life;
		spawner->flags |= CEF_NO_DRAW;
		spawner->color.c = 0xFFFFFFFF;
		spawner->Update = LinkedBloodThink;
		spawner->SpawnInfo = refpointidx;
		spawner->AddToView = OffsetLinkedEntityUpdatePlacement;

		AddEffect(owner, spawner);
	}
}

/*
void FXTrailBlood(centity_t *owner, int type, int flags, vec3_t origin)
{
	client_entity_t		*spawner;
	byte				amount;
	vec3_t				velocity;
	qboolean			yellow_blood = false;

	if(flags&CEF_FLAG8)
		yellow_blood = true;

	spawner = DoBloodSplash(origin, amount, yellow_blood);
	VectorCopy(velocity, spawner->velocity);
}
*/
// end
