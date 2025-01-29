//
// fx_sphereofannihilation.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Matrix.h"
#include "Particle.h"
#include "Random.h"
#include "Reference.h"
#include "Utilities.h"
#include "Vector.h"
#include "ce_DLight.h"
#include "q_Sprite.h"
#include "g_playstats.h"

#define FX_SPHERE_FLY_SPEED					600.0f
#define FX_SPHERE_AURA_SCALE				1.2f
#define FX_SOFT_SPHERE_AURA_SCALE			0.9f
#define FX_SPHERE_EXPLOSION_BASE_RADIUS		89.0f
#define FX_SPHERE_EXPLOSION_SMOKE_SPEED		140.0f
#define FX_SPHERE_EXPLOSION_PITCH_INCREMENT	(ANGLE_180 / 32.0f) //mxd
#define FX_SPHERE_EXPLOSION_YAW_INCREMENT	(ANGLE_180 / 27.0f) //mxd

static struct model_s* sphere_models[8];

void PreCacheSphere(void)
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

static qboolean FXSphereOfAnnihilationSphereThink(struct client_entity_s* self, centity_t* owner)
{
	float detail_scale;
	if ((int)r_detail->value == DETAIL_LOW)
		detail_scale = 0.7f;
	else if ((int)r_detail->value == DETAIL_NORMAL)
		detail_scale = 0.85f;
	else
		detail_scale = 1.0f;

	self->r.scale = owner->current.scale * detail_scale;

	return true;
}

static qboolean FXSphereOfAnnihilationAuraThink(struct client_entity_s* self, centity_t* owner)
{
	// No aura trail on low level.
	if ((int)r_detail->value == DETAIL_LOW)
		return true;

	vec3_t trail_start;
	VectorCopy(owner->origin, trail_start);

	vec3_t trail_pos;
	VectorSubtract(owner->lerp_origin, owner->origin, trail_pos);

	// Copy the real origin to the entity origin, so the light will follow us.
	VectorCopy(self->r.origin, self->origin);

	float trail_length = VectorNormalize(trail_pos);
	if (trail_length < 0.001f)
		trail_length += 2.0f;

	vec3_t right;
	PerpendicularVector(right, trail_pos);

	vec3_t up;
	CrossProduct(trail_pos, right, up);

	VectorScale(trail_pos, FX_SPHERE_FLY_SPEED, trail_pos);

	const int duration = (((int)r_detail->value > DETAIL_NORMAL) ? 500 : 400);
	const int flags = (int)(self->flags & ~(CEF_OWNERS_ORIGIN | CEF_NO_DRAW)); //mxd

	for (int i = 0; i < 40 && trail_length > 0.0f; i++)
	{
		client_entity_t* trail = ClientEntity_new(FX_WEAPON_SPHERE, flags, trail_start, NULL, duration);

		trail->radius = 70.0f;
		trail->r.model = &sphere_models[0]; // shboom sprite.
		trail->r.flags = RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		trail->Scale = FX_SPHERE_AURA_SCALE + flrand(0.0f, 0.1f);
		COLOUR_SET(trail->color, irand(128, 180), irand(128, 180), irand(64, 80)); //mxd. Use macro.
		trail->alpha = 0.7f;
		trail->d_alpha = -1.0f;
		trail->d_scale = -0.5f;

		AddEffect(NULL, trail);

		trail_length -= FX_SPHERE_FLY_SPEED;
		VectorAdd(trail_start, trail_pos, trail_start);
	}

	return true;
}

void FXSphereOfAnnihilation(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	short caster_entnum;
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_WEAPON_SPHERE].formatString, &caster_entnum);

	// Create a fiery blue aura around the sphere.
	const int caster_update = (((int)r_detail->value < DETAIL_NORMAL) ? 125 : 100); //mxd
	client_entity_t* aura_thinker = ClientEntity_new(type, flags, origin, NULL, caster_update);

	aura_thinker->flags |= CEF_NO_DRAW;
	if (r_detail->value > DETAIL_LOW)
	{
		const paletteRGBA_t light_color = { .r = 0, .g = 0, .b = 255, .a = 255 };
		aura_thinker->dlight = CE_DLight_new(light_color, 150.0f, 0.0f);
	}

	aura_thinker->extra = (void*)(&fxi.server_entities[caster_entnum]); // The caster's centity_t.
	aura_thinker->AddToView = LinkedEntityUpdatePlacement;
	aura_thinker->Update = FXSphereOfAnnihilationAuraThink;

	AddEffect(owner, aura_thinker);
	FXSphereOfAnnihilationAuraThink(aura_thinker, owner);

	// Create the sphere of annihilation itself.
	client_entity_t* sphere_thinker = ClientEntity_new(type, flags, origin, NULL, 100);

	sphere_thinker->radius = 70.0f;
	sphere_thinker->r.model = &sphere_models[1]; // bluball sprite.
	sphere_thinker->r.flags = RF_TRANSLUCENT;
	sphere_thinker->r.scale = owner->current.scale;
	sphere_thinker->AddToView = LinkedEntityUpdatePlacement;
	sphere_thinker->Update = FXSphereOfAnnihilationSphereThink;

	AddEffect(owner, sphere_thinker);
}

static qboolean FXSphereOfAnnihilationGlowballThink(struct client_entity_s* self, centity_t* owner)
{
	if (owner->current.effects & EF_MARCUS_FLAG1)
		self->color.r++;

	int duration;
	if ((int)r_detail->value == DETAIL_LOW)
		duration = 300;
	else if ((int)r_detail->value == DETAIL_NORMAL)
		duration = 400;
	else
		duration = 500;

	if (self->color.r > 3)
	{
		// Create a trailing spark.
		client_entity_t* spark = ClientEntity_new(FX_WEAPON_SPHERE, self->flags & ~(CEF_OWNERS_ORIGIN), self->r.origin, NULL, duration);

		spark->radius = 20.0f;
		spark->r.model = &sphere_models[2]; // glowball sprite.
		spark->r.flags = RF_TRANSLUCENT | RF_TRANS_ADD;
		COLOUR_SET(spark->r.color, irand(128, 180), irand(128, 180), irand(180, 255)); //mxd. Use macro.
		spark->Scale = flrand(0.8f, 1.0f);
		spark->d_scale = -1.5f;

		AddEffect(NULL, spark);
	}

	if (self->color.r < 16)
	{
		for (int i = 0; i < 3; i++)
		{
			self->velocity[i] *= 3.0f;
			self->velocity[i] += 6.0f * (owner->origin[i] - self->r.origin[i]);
			self->velocity[i] *= 0.265f;
		}

		return true;
	}

	return false;
}

static qboolean FXSphereOfAnnihilationGlowballSpawnerThink(struct client_entity_s* self, centity_t* owner)
{
	// 'Self->extra' refers to the caster's centity_t.
	const centity_t* controller = (centity_t*)self->extra;

	// This tells if we are wasting our time, because the reference points are culled.
	if (controller != NULL && !RefPointsValid(controller)) // Only if we were SUPPOSED to have refpoints.
		return true;

	if ((owner->current.effects & EF_MARCUS_FLAG1) == 0)
		return true;

	// If the spell is still building, create some swirling blue Glowballs.
	vec3_t temp_origin;
	if (controller != NULL)
		VectorCopy(controller->origin, temp_origin);
	else
		VectorCopy(self->r.origin, temp_origin);

	client_entity_t* glowball = ClientEntity_new(FX_WEAPON_SPHEREGLOWBALLS, (int)(self->flags & ~(CEF_NO_DRAW | CEF_OWNERS_ORIGIN)), temp_origin, NULL, 50);

	glowball->flags |= CEF_DONT_LINK;

	// Make me spawn from my caster's left / right hands (alternating). Assuming we aren't a reflection type glowball.
	if (controller != NULL)
	{
		vec3_t angles;
		VectorCopy(controller->current.angles, angles);
		VectorScale(angles, RAD_TO_ANGLE, angles);

		vec3_t forward;
		vec3_t right;
		AngleVectors(angles, forward, right, NULL);

		matrix3_t rotation;
		Matrix3FromAngles(controller->lerp_angles, rotation);

		if (self->SpawnInfo != 0)
		{
			const int ref_point = ((self->color.g & 1) ? CORVUS_RIGHTHAND : CORVUS_LEFTHAND); //mxd
			Matrix3MultByVec3(rotation, controller->referenceInfo->references[ref_point].placement.origin, glowball->r.origin);
		}
		else
		{
			vec3_t fwd_ofs;
			VectorScale(forward, 16.0f, fwd_ofs); // Hard-coded for Celestial Watcher (monster_elflord). 
			Matrix3MultByVec3(rotation, fwd_ofs, glowball->r.origin);
		}

		VectorAdd(controller->origin, glowball->r.origin, glowball->r.origin);
	}
	else
	{
		vec3_t angles;
		VectorCopy(self->r.angles, angles);
		VectorScale(angles, RAD_TO_ANGLE, angles);

		vec3_t forward;
		vec3_t right;
		AngleVectors(angles, forward, right, NULL);

		for (int i = 0; i < 3; i++)
			glowball->r.origin[i] = self->r.origin[i] + flrand(-10.0f, 10.0f);
	}

	vec3_t angles2;
	VectorCopy(owner->current.angles, angles2);
	VectorScale(angles2, RAD_TO_ANGLE, angles2);

	vec3_t forward2;
	vec3_t right2;
	AngleVectors(angles2, forward2, right2, NULL);

	// Set my velocity and acceleration.
	glowball->velocity[0] = forward2[0] * 175.0f + flrand(-25.0f, 25.0f);
	glowball->velocity[1] = forward2[1] * 175.0f + flrand(-25.0f, 25.0f);
	glowball->velocity[2] = flrand(-200.0f, 100.0f);

	const int axis = ((self->color.g & 1) ? 0 : 1); //mxd
	glowball->velocity[axis] *= -1.0f;

	VectorClear(glowball->acceleration);

	// Fill in the rest of my info.
	glowball->radius = 20.0f;
	glowball->r.model = &sphere_models[2]; // glowball sprite.
	glowball->r.flags = RF_TRANSLUCENT | RF_TRANS_ADD;
	COLOUR_SET(glowball->r.color, irand(128, 180), irand(128, 180), irand(180, 255)); //mxd. Use macro.
	glowball->color.r = 1;
	glowball->extra = (void*)owner;
	glowball->Update = FXSphereOfAnnihilationGlowballThink;

	AddEffect(owner, glowball);
	FXSphereOfAnnihilationGlowballThink(glowball, owner);

	self->color.g++;

	return true;
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

	VectorScale(Dir,FX_SPHERE_EXPLOSION_SMOKE_SPEED,Dir);

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

		ce->velocity[0]+=flrand(-FX_SPHERE_EXPLOSION_SMOKE_SPEED,FX_SPHERE_EXPLOSION_SMOKE_SPEED);
		ce->velocity[1]+=flrand(-FX_SPHERE_EXPLOSION_SMOKE_SPEED,FX_SPHERE_EXPLOSION_SMOKE_SPEED);
		ce->velocity[2]+=flrand(-FX_SPHERE_EXPLOSION_SMOKE_SPEED,FX_SPHERE_EXPLOSION_SMOKE_SPEED);
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

	VectorScale(Dir,FX_SPHERE_EXPLOSION_SMOKE_SPEED,Dir);

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
