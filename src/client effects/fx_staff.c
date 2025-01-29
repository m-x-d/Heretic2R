//
// fx_staff.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Matrix.h"
#include "Random.h"
#include "Reference.h"
#include "Utilities.h"
#include "Vector.h"
#include "ce_DLight.h"
#include "q_Sprite.h"
#include "g_playstats.h"

#define STAFF_LENGTH		27
#define STAFF_TYPE_SWORD	3
#define STAFF_TYPE_HELL		4

enum 
{
	STAFF_TRAIL,
	STAFF_HALO,
	STAFF_TRAIL2,
	STAFF_TRAIL_SMOKE,
	STAFF_TRAIL3,

	NUM_MODELS
};

static struct model_s* staff_models[NUM_MODELS];
static struct model_s* staffhit_models[4];

void PreCacheStaffHit(void)
{
	staffhit_models[0] = fxi.RegisterModel("sprites/spells/patball.sp2");
	staffhit_models[1] = fxi.RegisterModel("sprites/fx/halo.sp2");
	staffhit_models[2] = fxi.RegisterModel("sprites/fx/firestreak.sp2");
	staffhit_models[3] = fxi.RegisterModel("sprites/fx/steam.sp2");
}

void PreCacheStaff(void)
{
	staff_models[0] = fxi.RegisterModel("sprites/spells/patball.sp2");
	staff_models[1] = fxi.RegisterModel("sprites/fx/halo.sp2");
	staff_models[2] = fxi.RegisterModel("sprites/spells/wflame2.sp2");
	staff_models[3] = fxi.RegisterModel("sprites/fx/steam.sp2");
	staff_models[4] = fxi.RegisterModel("sprites/fx/haloblue.sp2");
}

//mxd. Separated from FXStaffStrike().
static void FXStaffStrikeLevel2(const int flags, const vec3_t origin, const vec3_t direction)
{
	if (r_detail->value >= DETAIL_NORMAL)
		fxi.Activate_Screen_Flash(0x30FFFFFF);

	// Spawn a bright flash at the core of the explosion.
	client_entity_t* flash = ClientEntity_new(FX_WEAPON_STAFF_STRIKE, (int)(flags & ~CEF_NO_DRAW), origin, NULL, 1000);

	flash->r.model = &staffhit_models[1]; // halo sprite.
	flash->r.flags = RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	flash->r.scale = flrand(0.75f, 1.0f);
	flash->alpha = 0.75f;
	flash->d_alpha = -2.0f;
	flash->d_scale = -2.0f;
	flash->r.frame = 1;
	flash->r.color.c = 0xFF888888;
	flash->dlight = CE_DLight_new(flash->r.color, 150.0f, -100.0f);

	AddEffect(NULL, flash);

	// Spawn a hit explosion of lines.
	const int count = GetScaledCount(64, 0.85f);

	for (int i = 0; i < count; i++)
	{
		client_entity_t* streak = ClientEntity_new(FX_WEAPON_STAFF_STRIKE, (int)(flags & ~CEF_NO_DRAW), origin, NULL, 500);

		streak->r.model = &staffhit_models[0]; // patball sprite.
		streak->r.spriteType = SPRITE_LINE;
		streak->r.flags = RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		streak->r.scale = flrand(1.0f, 2.5f);
		streak->alpha = flrand(0.75f, 1.0f);
		streak->d_alpha = -2.0f;
		streak->d_scale = -1.0f;

		const int c = irand(128, 255);
		COLOUR_SETA(streak->r.color, c, c, irand(236, 255), irand(80, 192)); //mxd. Use macro.

		VectorRandomCopy(direction, streak->velocity, 1.25f);

		VectorCopy(origin, streak->r.startpos);
		VectorMA(streak->r.startpos, flrand(16.0f, 48.0f), streak->velocity, streak->r.endpos); //mxd. Original logic uses irand() here.

		VectorScale(streak->velocity, flrand(200.0f, 300.0f), streak->velocity);
		VectorSet(streak->acceleration, streak->velocity[0] * 0.1f, streak->velocity[1] * 0.1f, 0); //mxd. Original logic uses irand() here.

		AddEffect(NULL, streak);
	}
}

//mxd. Separated from FXStaffStrike().
static void FXStaffStrikeLevel3(const int flags, const vec3_t origin, const vec3_t direction)
{
	// Spawn a bright flash at the core of the explosion.
	client_entity_t* flash = ClientEntity_new(FX_WEAPON_STAFF_STRIKE, (int)(flags & ~CEF_NO_DRAW), origin, NULL, 1000);

	flash->r.model = &staffhit_models[1]; // Halo sprite.
	flash->r.flags = RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	flash->r.scale = flrand(0.75f, 1.0f);
	flash->alpha = 0.75f;
	flash->d_alpha = -2.0f;
	flash->d_scale = -2.0f;
	flash->r.frame = 1;

	const int white = irand(8, 16);
	COLOUR_SETA(flash->r.color, irand(236, 255), 64 + white, 16 + white, irand(80, 192)); //mxd. Use macro.

	flash->dlight = CE_DLight_new(flash->r.color, 150.0f, -100.0f);

	AddEffect(NULL, flash);

	// Spawn an explosion of lines.
	const int streak_count = GetScaledCount(16, 0.85f);

	for (int i = 0; i < streak_count; i++)
	{
		client_entity_t* streak = ClientEntity_new(FX_WEAPON_STAFF_STRIKE, (int)(flags & ~CEF_NO_DRAW), origin, NULL, 500);

		streak->r.model = &staffhit_models[2]; // firestreak sprite.
		streak->r.spriteType = SPRITE_LINE;
		streak->r.flags = RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		streak->r.scale = flrand(1.0f, 2.5f);
		streak->d_alpha = -1.0f;
		streak->d_scale = -1.0f;

		const int c = irand(128, 255);
		COLOUR_SETA(streak->r.color, c, c, irand(236, 255), irand(80, 192)); //mxd. Use macro.

		VectorRandomCopy(direction, streak->velocity, 1.25f);

		VectorCopy(origin, streak->r.endpos);
		VectorMA(streak->r.endpos, flrand(8.0f, 16.0f), streak->velocity, streak->r.startpos); //mxd. Original logic uses irand() here.

		VectorScale(streak->velocity, flrand(100.0f, 200.0f), streak->velocity); //mxd. Original logic uses irand() here.

		AddEffect(NULL, streak);
	}

	// Spawn smoke.
	const int smoke_count = GetScaledCount(4, 0.85f);

	for (int i = 0; i < smoke_count; i++)
	{
		client_entity_t* steam = ClientEntity_new(FX_WEAPON_STAFF_STRIKE, (int)(flags & ~CEF_NO_DRAW), origin, NULL, 1000);

		steam->r.model = &staffhit_models[3]; // steam sprite.
		steam->r.flags = RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		steam->r.scale = flrand(0.25f, 0.5f);
		steam->alpha = 0.9f;
		steam->d_alpha = -2.0f;
		steam->d_scale = 2.0f;

		const int c = irand(32, 64);
		COLOUR_SETA(steam->r.color, c, c, c, 128); //mxd. Use macro.

		VectorRandomCopy(direction, steam->velocity, 1.25f);

		VectorCopy(origin, steam->r.endpos);
		VectorMA(steam->r.endpos, flrand(16.0f, 48.0f), steam->velocity, steam->r.startpos); //mxd. Original logic uses irand() here.

		VectorScale(steam->velocity, flrand(10.0f, 50.0f), steam->velocity); //mxd. Original logic uses irand() here.
		steam->velocity[2] += 64.0f;

		AddEffect(NULL, steam);
	}
}

void FXStaffStrike(centity_t* owner, int type, const int flags, vec3_t origin)
{
	vec3_t dir;
	byte power_level;
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_WEAPON_STAFF_STRIKE].formatString, &dir, &power_level);

	switch (power_level)
	{
		case 2: FXStaffStrikeLevel2(flags, origin, dir); break; //mxd
		case 3: FXStaffStrikeLevel3(flags, origin, dir); break; //mxd
		default: break;
	}
}

static qboolean FXStaffElementThink(struct client_entity_s* self, centity_t* owner)
{
	float frac = (float)(fxi.cl->time - self->startTime) / 100.0f;

	if (self->AnimSpeed > 0.0f)
		frac *= self->AnimSpeed;

	const int frame_num = (int)(floorf(frac));
	if (frame_num >= self->NoOfAnimFrames - 1)
		return false;

	self->r.frame = frame_num + 1;

	const float multiplier = 1.0f - frac / (float)(self->NoOfAnimFrames - 1);
	self->r.color.r = (byte)((float)self->color.r * multiplier);
	self->r.color.b = (byte)((float)self->color.g * multiplier);
	self->r.color.g = (byte)((float)self->color.b * multiplier);

	return true;
}

// ************************************************************************************************
// FXStaffThink
// -----------------
// ************************************************************************************************

static qboolean FXTrailLevel2ThinkBurn(struct client_entity_s *Self,centity_t *owner)
{
	return true;
}

static qboolean FXStaffLevel2Think(struct client_entity_s *Self,centity_t *owner)
{
	int				I;
	int				NoOfIntervals, white;
	client_entity_t	*TrailEnt;
	paletteRGBA_t	color;
	vec3_t			dpivot, curpivot;
	vec3_t			dnormal, curnormal, adjnormal;
	vec3_t			diff, newpoint;
	int				model;

	matrix3_t	rotation;
	vec3_t		origin;

	// If we've timed out, stop the effect (allow for fading)
	if ( (Self->LifeTime > 0) && (Self->LifeTime < fxi.cl->time) )
	{
		Self->Update=RemoveSelfAI;
		Self->updateTime = fxi.cl->time + 500;
		return true;
	}

	// This tells if we are wasting our time, because the reference points are culled.
	if (!RefPointsValid(owner))
		return false;		// Remove the effect in this case.

	I=Self->NoOfAnimFrames;

	// If this reference point hasn't changed since the last frame, return.
	VectorSubtract(	owner->referenceInfo->references[I].placement.origin,
					owner->referenceInfo->oldReferences[I].placement.origin,
					diff);

	if (Q_fabs(diff[0] + diff[1] + diff[2]) < .1)
		return(true);
	
	NoOfIntervals=(int)(VectorLength(diff)*.5);
	if(NoOfIntervals > 40)
		return(false);
	NoOfIntervals = GetScaledCount(NoOfIntervals, 1.0);

	// Average out the two right hand positions to get a pivot point.
	VectorCopy(owner->referenceInfo->oldReferences[CORVUS_RIGHTHAND].placement.origin, curpivot);
	VectorSubtract(owner->referenceInfo->references[CORVUS_RIGHTHAND].placement.origin, curpivot, dpivot);
	VectorScale(dpivot, 1.0/NoOfIntervals, dpivot);

	VectorCopy(owner->referenceInfo->oldReferences[I].placement.direction, curnormal);
	VectorSubtract(owner->referenceInfo->references[I].placement.direction, curnormal, dnormal);
	VectorScale(dnormal, 1.0/NoOfIntervals, dnormal);
	VectorCopy(curnormal, adjnormal);  // This rides on the assumption that the normal given is already a unit norm.

	//FIXME: The above assumption isn't working!
	VectorNormalize(adjnormal);
	if(NoOfIntervals > 40)
		return(false);

	while (NoOfIntervals >= 0)
	{
		VectorMA(curpivot, STAFF_LENGTH, adjnormal, newpoint);
		
		TrailEnt=ClientEntity_new(FX_SPELLHANDS, Self->flags & ~CEF_NO_DRAW, newpoint, 0, 2000);
		
		VectorCopy(newpoint, TrailEnt->origin);
		
		model = irand(0,100);
		
		TrailEnt->r.model = staff_models + STAFF_TRAIL2;
		
		TrailEnt->r.frame = 0;

		TrailEnt->r.flags=RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;

		TrailEnt->r.scale = flrand(0.2, 0.3);
		TrailEnt->d_scale = flrand(-0.5, -1.0);

		TrailEnt->velocity[0] = irand(-8, 8);
		TrailEnt->velocity[1] = irand(-8, 8);
		TrailEnt->velocity[2] += irand(64, 128);

		TrailEnt->origin[0] += irand(-1, 1);
		TrailEnt->origin[1] += irand(-1, 1);
		TrailEnt->origin[2] += irand(-1, 1);

		TrailEnt->alpha = 1.0;
		TrailEnt->d_alpha = -2.0;

		//Attach a dynamic light to the last one
		if (NoOfIntervals==1 && (r_detail->value >= DETAIL_NORMAL))
		{
			white = irand(8, 16);

			color.r = 128 + irand(108, 127);
			color.g = 64  + white;
			color.b = 16  + white;
			color.a = 64  + irand(16, 128);

			TrailEnt->dlight = CE_DLight_new(color, irand(50.0f, 150.0F), -100.0F);
		}

		Matrix3FromAngles(owner->lerp_angles, rotation);

		Matrix3MultByVec3(rotation, TrailEnt->origin, origin);

		TrailEnt->r.origin[0] = owner->origin[0] + origin[0];
		TrailEnt->r.origin[1] = owner->origin[1] + origin[1];
		TrailEnt->r.origin[2] = owner->origin[2] + origin[2];

		AddEffect(NULL,TrailEnt);
		
		if (!irand(0,3))
		{
			TrailEnt=ClientEntity_new(FX_SPELLHANDS, Self->flags & ~CEF_NO_DRAW, newpoint, 0, 5000);
	
			TrailEnt->r.model = staff_models + STAFF_TRAIL_SMOKE;
			
			TrailEnt->r.frame = 0;

			TrailEnt->r.flags=RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;

			TrailEnt->r.scale = flrand(0.1, 0.15);
			TrailEnt->d_scale = 1.0;

			TrailEnt->alpha = 0.75;
			TrailEnt->d_alpha = -1.0;

			TrailEnt->velocity[0] = irand(-16, 16);
			TrailEnt->velocity[1] = irand(-16, 16);
			TrailEnt->velocity[2] += irand(64, 128);

			white = irand(32, 64);

			TrailEnt->r.color.r = TrailEnt->r.color.g = TrailEnt->r.color.b = white; 
			TrailEnt->r.color.a = 128;

			//Attach a dynamic light to the last one
			if (NoOfIntervals==1 && (r_detail->value >= DETAIL_NORMAL))
			{
				white = irand(8, 16);

				color.r = 128 + irand(108, 127);
				color.g = 64  + white;
				color.b = 16  + white;
				color.a = 64  + irand(16, 128);

				TrailEnt->dlight = CE_DLight_new(color, irand(50.0f, 150.0F), -100.0F);
			}

			Matrix3FromAngles(owner->lerp_angles, rotation);

			Matrix3MultByVec3(rotation, TrailEnt->origin, origin);

			TrailEnt->r.origin[0] = owner->origin[0] + origin[0];
			TrailEnt->r.origin[1] = owner->origin[1] + origin[1];
			TrailEnt->r.origin[2] = owner->origin[2] + origin[2];

			AddEffect(NULL,TrailEnt);
		}

		VectorAdd(curpivot, dpivot, curpivot);
		VectorAdd(curnormal, dnormal, curnormal);
		VectorNormalize2(curnormal, adjnormal);
		NoOfIntervals--;
	}

	return true;
}

static qboolean FXStaffLevel3Think(struct client_entity_s *Self,centity_t *owner)
{
	int				I;
	int				NoOfIntervals, white;
	client_entity_t	*TrailEnt;
	vec3_t			dpivot, curpivot;
	vec3_t			dnormal, curnormal, adjnormal;
	vec3_t			diff, newpoint;

	// If we've timed out, stop the effect (allow for fading)
	if ( (Self->LifeTime > 0) && (Self->LifeTime < fxi.cl->time) )
	{
		Self->Update=RemoveSelfAI;
		Self->updateTime = fxi.cl->time + 500;
		return true;
	}

	// This tells if we are wasting our time, because the reference points are culled.
	if (!RefPointsValid(owner))
		return false;		// Remove the effect in this case.

	I=Self->NoOfAnimFrames;

	// If this reference point hasn't changed since the last frame, return.
	VectorSubtract(	owner->referenceInfo->references[I].placement.origin,
					owner->referenceInfo->oldReferences[I].placement.origin,
					diff);

	if (Q_fabs(diff[0] + diff[1] + diff[2]) < .1)
		return(true);

	NoOfIntervals=(int)(VectorLength(diff)*.5);
	if(NoOfIntervals > 40)
		return(false);
	NoOfIntervals = GetScaledCount(NoOfIntervals, 1.0);

	// Take the before and after points and try to draw an arc.

	// Average out the two right hand positions to get a pivot point.
	VectorCopy(owner->referenceInfo->oldReferences[CORVUS_RIGHTHAND].placement.origin, curpivot);
	VectorSubtract(owner->referenceInfo->references[CORVUS_RIGHTHAND].placement.origin, curpivot, dpivot);
	VectorScale(dpivot, 1.0/NoOfIntervals, dpivot);

	VectorCopy(owner->referenceInfo->oldReferences[I].placement.direction, curnormal);
	VectorSubtract(owner->referenceInfo->references[I].placement.direction, curnormal, dnormal);
	VectorScale(dnormal, 1.0/NoOfIntervals, dnormal);
	VectorCopy(curnormal, adjnormal);  // This rides on the assumption that the normal given is already a unit norm.

	//FIXME: The above assumption isn't working!
	VectorNormalize(adjnormal);
	if(NoOfIntervals > 40)
		return(false);

	while (NoOfIntervals >= 0)
	{
		VectorMA(curpivot, STAFF_LENGTH, adjnormal, newpoint);
		
		TrailEnt=ClientEntity_new(FX_SPELLHANDS, Self->flags & ~CEF_NO_DRAW, newpoint, 0, 500);
		
		VectorCopy(newpoint, TrailEnt->origin);
		
		TrailEnt->r.model = staff_models + STAFF_TRAIL3;
		
		TrailEnt->r.frame = 0;

		TrailEnt->r.flags=RF_TRANSLUCENT | RF_TRANS_ADD;

		TrailEnt->r.scale = 0.3;
		TrailEnt->d_scale = -0.75;

		TrailEnt->alpha = 0.75;
		TrailEnt->d_alpha = -4;
		
		if (owner->current.effects & EF_BLOOD_ENABLED)
		{
			white = irand(128, 208);

			TrailEnt->r.color.r = 128 + irand(108, 127);
			TrailEnt->r.color.g = white;
			TrailEnt->r.color.b = white;
			TrailEnt->r.color.a = 64 + irand(16, 128);
		}
		else
		{
			white = irand(128, 208);

			TrailEnt->r.color.r = white;
			TrailEnt->r.color.g = white;
			TrailEnt->r.color.b = 128 + irand(108, 127);
			TrailEnt->r.color.a = 64 + irand(16, 128);
		}

		//Attach a dynamic light to the last one
		if (NoOfIntervals==1 && (r_detail->value >= DETAIL_NORMAL))
		{
			TrailEnt->dlight = CE_DLight_new(TrailEnt->r.color, 100.0F, -100.0F);
		}

		TrailEnt->AddToView=OffsetLinkedEntityUpdatePlacement;			
		
		AddEffect(owner,TrailEnt);

		VectorAdd(curpivot, dpivot, curpivot);
		VectorAdd(curnormal, dnormal, curnormal);
		VectorNormalize2(curnormal, adjnormal);
		NoOfIntervals--;
	}

	return true;
}

static qboolean FXStaffThink(struct client_entity_s *Self,centity_t *owner)
{
	int				I;
	int				NoOfIntervals;
	client_entity_t	*TrailEnt;
	vec3_t			dpivot, curpivot;
	vec3_t			dnormal, curnormal, adjnormal;
	vec3_t			diff, newpoint;

	// If we've timed out, stop the effect (allow for fading)
	if ( (Self->LifeTime > 0) && (Self->LifeTime < fxi.cl->time) )
	{
		Self->Update=RemoveSelfAI;
		Self->updateTime = fxi.cl->time + 500;
		return true;
	}

	// This tells if we are wasting our time, because the reference points are culled.
	if (!RefPointsValid(owner))
		return false;		// Remove the effect in this case.

	Self->updateTime = 17;		// FIXME : With a next think time this effect does not look right

	I=Self->NoOfAnimFrames;

	// If this reference point hasn't changed since the last frame, return.
	VectorSubtract(	owner->referenceInfo->references[I].placement.origin,
					owner->referenceInfo->oldReferences[I].placement.origin,
					diff);

	if (Q_fabs(diff[0] + diff[1] + diff[2]) < .1)
		return(true);

	NoOfIntervals=(int)(VectorLength(diff)*.75);
	if(NoOfIntervals > 40)
		return(false);
	NoOfIntervals = GetScaledCount(NoOfIntervals, 1.0);

	// Take the before and after points and try to draw an arc.

	// Average out the two right hand positions to get a pivot point.
	VectorCopy(owner->referenceInfo->oldReferences[CORVUS_RIGHTHAND].placement.origin, curpivot);
	VectorSubtract(owner->referenceInfo->references[CORVUS_RIGHTHAND].placement.origin, curpivot, dpivot);
	VectorScale(dpivot, 1.0/NoOfIntervals, dpivot);

	VectorCopy(owner->referenceInfo->oldReferences[I].placement.direction, curnormal);
	VectorSubtract(owner->referenceInfo->references[I].placement.direction, curnormal, dnormal);
	VectorScale(dnormal, 1.0/NoOfIntervals, dnormal);
	VectorCopy(curnormal, adjnormal);  // This rides on the assumption that the normal given is already a unit norm.

	VectorNormalize(adjnormal);

	while (NoOfIntervals >= 0)
	{
		//Get the position of this sprite
		VectorMA(curpivot, STAFF_LENGTH, adjnormal, newpoint);
		
		TrailEnt=ClientEntity_new(FX_SPELLHANDS, Self->flags & ~CEF_NO_DRAW, newpoint, 0, 1500);
		
		VectorCopy(newpoint, TrailEnt->origin);
		
		TrailEnt->r.model = staff_models;
		TrailEnt->r.frame = 1;
		
		TrailEnt->r.flags = RF_TRANSLUCENT|RF_TRANS_ADD|RF_TRANS_ADD_ALPHA;
	
		TrailEnt->d_scale = -0.5;
		TrailEnt->alpha = 0.5;
		TrailEnt->d_alpha = -1.0;
		
		if (owner->current.effects & EF_BLOOD_ENABLED)
		{
			TrailEnt->r.color.c=0x50000018;
			TrailEnt->r.scale=Self->xscale;
		}
		else
		{
			TrailEnt->r.color = Self->color;
			TrailEnt->r.scale=Self->xscale;
		}
		
		TrailEnt->AddToView=OffsetLinkedEntityUpdatePlacement;			
		
		AddEffect(owner,TrailEnt);

		//
		VectorAdd(curpivot, dpivot, curpivot);
		VectorAdd(curnormal, dnormal, curnormal);
		VectorNormalize2(curnormal, adjnormal);
		NoOfIntervals--;
	}

	return true;
}

// ************************************************************************************************
// FXStaff
// ------------
// ************************************************************************************************

// This effect spawns 70+ client fx which will cause problems

void FXStaff(centity_t *owner,int Type,int Flags,vec3_t Origin)
{
	short			Refpoints;
	client_entity_t	*trail;
	int				I;
	byte			powerlevel;
	char			lifetime;

	Refpoints=0;

	fxi.GetEffect(owner,Flags,clientEffectSpawners[FX_STAFF].formatString, &powerlevel, &lifetime);
	Refpoints = (1<<CORVUS_BLADE);

	if(!ReferencesInitialized(owner))
	{
		return;
	}

	// Add a fiery trail effect to the player's hands / feet etc.
	for(I=0;I<16;I++)
	{
		if(!(Refpoints & (1 << I)))
			continue;

		trail=ClientEntity_new(Type, CEF_OWNERS_ORIGIN | CEF_DONT_LINK, Origin, 0, 17);

		trail->flags |= CEF_NO_DRAW;
		trail->NoOfAnimFrames = I;

		switch(powerlevel)
		{
		case 1:		// Blue
		default:
			trail->Update = FXStaffThink;
			trail->color.c = 0x02201010;
			trail->xscale = .175;
			break;

		//NOTENOTE: These were swapped after the functions were created
		case 2:		// Fire
			trail->Update = FXStaffLevel3Think;
			trail->color.c = 0xffffffff;
			trail->xscale = .200;
			break;

		case 3:		// Energy Blast
		case 4:
			trail->Update = FXStaffLevel2Think;
			trail->color.c = 0xffffffff;
			trail->xscale = .225;
			break;
		}
		trail->SpawnData = powerlevel;

		if (lifetime > 0)
			trail->LifeTime = fxi.cl->time + (lifetime * 100);
		else
			trail->LifeTime = lifetime;

		AddEffect(owner,trail);
	}
}

// ************************************************************************************************
// FXStaffCreateThink
// -----------------
// ************************************************************************************************

static qboolean FXStaffCreateThink(struct client_entity_s *Self,centity_t *owner)
{
	int				NoOfIntervals;
	client_entity_t	*TrailEnt;
	vec3_t			startpt, endpt;
	vec3_t			diff, curpt;
	int				color;

	Self->updateTime = 17;		// FIXME : With a next think time this effect does not look right

	// This tells if we are wasting our time, because the reference points are culled.
	if (!RefPointsValid(owner))
		return false;		// Remove the effect in this case.

	// If this reference point hasn't changed since the last frame, return.
	switch(Self->refPoint)
	{
	case STAFF_TYPE_HELL:
		VectorAdd(owner->referenceInfo->references[CORVUS_RIGHTHAND].placement.origin, owner->referenceInfo->references[CORVUS_STAFF].placement.origin, startpt);
		VectorScale(startpt, 0.5, startpt);
		VectorCopy(owner->referenceInfo->references[CORVUS_HELL_HEAD].placement.origin, endpt);
		color = 0xff2020ff;
		break;
	case STAFF_TYPE_SWORD:
	default:
		VectorCopy(owner->referenceInfo->references[CORVUS_STAFF].placement.origin, startpt);
		VectorCopy(owner->referenceInfo->references[CORVUS_BLADE].placement.origin, endpt);
		color = 0xff20ff20;
		break;
	}
		
	VectorSubtract(endpt, startpt, diff);
	NoOfIntervals=(int)(VectorLength(diff)*.5);

	VectorScale(diff, 1.0/NoOfIntervals, diff);
	VectorCopy(startpt, curpt);  // This rides on the assumption that the normal given is already a unit norm.

	if(NoOfIntervals > 40)
		return(false);

	while (NoOfIntervals >= 0)
	{
		TrailEnt=ClientEntity_new(FX_SPELLHANDS, Self->flags & ~CEF_NO_DRAW, curpt, 0, 100);
		VectorCopy(curpt, TrailEnt->origin);
		TrailEnt->r.model = staff_models + Self->classID;
		TrailEnt->alpha=0.8 - (Self->NoOfAnimFrames*0.1);
		TrailEnt->r.flags=RF_TRANSLUCENT|RF_TRANS_ADD|RF_TRANS_ADD_ALPHA;
		TrailEnt->AddToView=OffsetLinkedEntityUpdatePlacement;			
		if(Self->classID == STAFF_TRAIL || Self->refPoint == STAFF_TYPE_HELL)
		{
			TrailEnt->r.frame=1;
			TrailEnt->d_scale=-0.25;
			TrailEnt->d_alpha=-0.1;
			TrailEnt->color.c=color;		  
			TrailEnt->r.scale=Self->NoOfAnimFrames*.05;
			TrailEnt->AnimSpeed=0.20;
			TrailEnt->NoOfAnimFrames=2;
			TrailEnt->Update=FXStaffElementThink;

			AddEffect(owner,TrailEnt);

			FXStaffElementThink(TrailEnt,owner);
		}
		else if(Self->classID == STAFF_TRAIL2)
		{
			TrailEnt->r.frame = 0;

			TrailEnt->r.flags=RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;

			TrailEnt->r.scale = flrand(0.1, 0.2);
			TrailEnt->d_scale = flrand(-0.25, -0.5);

			TrailEnt->velocity[0] = irand(-8, 8);
			TrailEnt->velocity[1] = irand(-8, 8);
			TrailEnt->velocity[2] += irand(64, 128);

			TrailEnt->origin[0] += irand(-1, 1);
			TrailEnt->origin[1] += irand(-1, 1);
			TrailEnt->origin[2] += irand(-1, 1);

			TrailEnt->alpha = 1.0;
			TrailEnt->d_alpha = -2.0;

			AddEffect(owner,TrailEnt);
		}
		else if(Self->classID == STAFF_TRAIL3)
		{
			TrailEnt->r.frame = 0;

			TrailEnt->r.flags=RF_TRANSLUCENT | RF_TRANS_ADD;

			TrailEnt->r.scale = 0.2;
			TrailEnt->d_scale = -0.35;

			TrailEnt->alpha = 0.75;
			TrailEnt->d_alpha = -4;
			
			if (owner->current.effects & EF_BLOOD_ENABLED)
			{
				TrailEnt->r.color.c = 0x50000018;
			}
			else
			{
				int	white;

				white = irand(128, 208);

				TrailEnt->r.color.r = white;
				TrailEnt->r.color.g = white;
				TrailEnt->r.color.b = 128 + irand(108, 127);
				TrailEnt->r.color.a = 64 + irand(16, 128);
			}

			AddEffect(owner,TrailEnt);
		}
		//
		VectorAdd(curpt, diff, curpt);
		NoOfIntervals--;
	}
	Self->NoOfAnimFrames-=1.0;
	if (Self->NoOfAnimFrames <= 0.0)
		return(false);
	else
		return(true);
}

// ************************************************************************************************
// FXStaffCreate
// ------------
// ************************************************************************************************

// This effect spawns 80+ client fx which will cause problems

void FXStaffCreate(centity_t *owner,int Type,int Flags,vec3_t Origin)
{
	client_entity_t *stafffx;
	byte			fxtype;

	// This tells if we are wasting our time, because the reference points are culled.
	if (!RefPointsValid(owner))
		return;				// Abandon the effect in this case.

	if (Flags & CEF_FLAG6)
		fxtype = STAFF_TYPE_HELL;
	else
		fxtype = STAFF_TYPE_SWORD;

	stafffx = ClientEntity_new(Type, Flags, Origin, 0, 17);

	if(Flags & CEF_FLAG7)//blue
		stafffx->classID = STAFF_TRAIL3;
	else if(Flags & CEF_FLAG8)//flames
		stafffx->classID = STAFF_TRAIL2;
	else//normal
		stafffx->classID = STAFF_TRAIL;

	stafffx->Update = FXStaffCreateThink;
	stafffx->flags |= CEF_NO_DRAW;
	stafffx->NoOfAnimFrames=7;
	stafffx->refPoint = fxtype;

	AddEffect(owner, stafffx);
}


qboolean FXSpellChangePuffThink(struct client_entity_s *Self,centity_t *owner);

// ************************************************************************************************
// FXStaffCreatePoof
// ------------
// ************************************************************************************************

void FXStaffCreatePoof(centity_t *owner,int Type,int Flags,vec3_t Origin)
{
	client_entity_t *stafffx;
	vec3_t spawnpt;
	paletteRGBA_t	LightColor;

	// This tells if we are wasting our time, because the reference points are culled.
	if (!RefPointsValid(owner))
		return;		// Remove the effect in this case.

	if(Flags & CEF_FLAG6)
	{
		VectorCopy(owner->referenceInfo->references[CORVUS_HELL_HEAD].placement.origin, spawnpt);
		LightColor.c = 0xff2020ff;
	}
	else
	{
		VectorCopy(owner->referenceInfo->references[CORVUS_BLADE].placement.origin, spawnpt);
		LightColor.c = 0xffff5050;
	}

	stafffx=ClientEntity_new(FX_SPELLHANDS, Flags & ~CEF_NO_DRAW, spawnpt, 0, 100);
	VectorCopy(spawnpt, stafffx->origin);
	stafffx->r.model = staff_models + STAFF_HALO;
	stafffx->alpha=.75;
	stafffx->r.flags=RF_TRANSLUCENT|RF_TRANS_ADD|RF_TRANS_ADD_ALPHA;
	stafffx->r.frame=1;
	stafffx->d_scale=-0.3;
	stafffx->d_alpha=-0.2;
	stafffx->color.r=255;
	stafffx->color.g=255;
	stafffx->color.b=255;
	stafffx->r.scale=0.5;
	stafffx->AnimSpeed=0.20;
	stafffx->NoOfAnimFrames=2;
	stafffx->Update=FXStaffElementThink;
	stafffx->AddToView=OffsetLinkedEntityUpdatePlacement;			
	AddEffect(owner,stafffx);
	FXStaffElementThink(stafffx,owner);


	if(!(Flags & CEF_FLAG6))	// Just for the sword staff
	{
		VectorCopy(owner->referenceInfo->references[CORVUS_STAFF].placement.origin, spawnpt);

		stafffx=ClientEntity_new(FX_SPELLHANDS, Flags & ~CEF_NO_DRAW, spawnpt, 0, 100);
		VectorCopy(spawnpt, stafffx->origin);
		stafffx->r.model = staff_models + STAFF_HALO;
		stafffx->alpha=.75;
		stafffx->r.flags=RF_TRANSLUCENT|RF_TRANS_ADD|RF_TRANS_ADD_ALPHA;
		stafffx->r.frame=1;
		stafffx->d_scale=-0.3;
		stafffx->d_alpha=-0.2;
		stafffx->color.r=255;
		stafffx->color.g=255;
		stafffx->color.b=255;
		stafffx->r.scale=0.5;
		stafffx->AnimSpeed=0.20;
		stafffx->NoOfAnimFrames=2;
		stafffx->Update=FXStaffElementThink;
		stafffx->AddToView=OffsetLinkedEntityUpdatePlacement;			
		AddEffect(owner,stafffx);
		FXStaffElementThink(stafffx,owner);
	}
}



// ************************************************************************************************
// FXStaffRemoveThink
// -----------------
// ************************************************************************************************

static qboolean FXStaffRemoveThink(struct client_entity_s *Self,centity_t *owner)
{
	int				NoOfIntervals;
	client_entity_t	*TrailEnt;
	vec3_t			startpt, endpt;
	vec3_t			diff, curpt;
	int color;

	Self->updateTime = 17;		// FIXME : With a next think time this effect does not look right

	// This tells if we are wasting our time, because the reference points are culled.
	if (!RefPointsValid(owner))
		return false;		// Remove the effect in this case.

	// If this reference point hasn't changed since the last frame, return.
	switch(Self->refPoint)
	{
	case STAFF_TYPE_HELL:
		VectorAdd(owner->referenceInfo->references[CORVUS_RIGHTHAND].placement.origin, owner->referenceInfo->references[CORVUS_STAFF].placement.origin, startpt);
		VectorScale(startpt, 0.5, startpt);
		VectorCopy(owner->referenceInfo->references[CORVUS_HELL_HEAD].placement.origin, endpt);
		color = 0xff2020ff;
		break;
	case STAFF_TYPE_SWORD:
	default:
		VectorCopy(owner->referenceInfo->references[CORVUS_STAFF].placement.origin, startpt);
		VectorCopy(owner->referenceInfo->references[CORVUS_BLADE].placement.origin, endpt);
		color = 0xff20ff20;
		break;
	}
		
	VectorSubtract(endpt, startpt, diff);
	NoOfIntervals=(int)(VectorLength(diff)*.5);

	VectorScale(diff, 1.0/NoOfIntervals, diff);
	VectorCopy(startpt, curpt);  // This rides on the assumption that the normal given is already a unit norm.

	if(NoOfIntervals > 40)
		return(false);

	while (NoOfIntervals >= 0)
	{
		TrailEnt=ClientEntity_new(FX_SPELLHANDS, Self->flags & ~CEF_NO_DRAW, curpt, 0, 100);
		VectorCopy(curpt, TrailEnt->origin);
		TrailEnt->r.model = staff_models + Self->classID;
		TrailEnt->alpha=0.6 - (Self->NoOfAnimFrames*0.1);
		TrailEnt->r.flags=RF_TRANSLUCENT|RF_TRANS_ADD|RF_TRANS_ADD_ALPHA;
		TrailEnt->AddToView=OffsetLinkedEntityUpdatePlacement;			
		if(Self->classID == STAFF_TRAIL || Self->refPoint == STAFF_TYPE_HELL)
		{
			TrailEnt->r.frame=1;
			TrailEnt->d_scale=-0.25;
			TrailEnt->d_alpha=-0.1;
			TrailEnt->color.c=color;
			TrailEnt->r.scale=Self->NoOfAnimFrames*.05;
			TrailEnt->AnimSpeed=0.20;
			TrailEnt->NoOfAnimFrames=2;
			TrailEnt->Update=FXStaffElementThink;

			AddEffect(owner,TrailEnt);
			FXStaffElementThink(TrailEnt,owner);
		}
		else if(Self->classID == STAFF_TRAIL2)
		{
			TrailEnt->r.frame = 0;

			TrailEnt->r.flags=RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;

			TrailEnt->r.scale = flrand(0.1, 0.2);
			TrailEnt->d_scale = flrand(-0.5, -1.0);

			TrailEnt->velocity[0] = irand(-8, 8);
			TrailEnt->velocity[1] = irand(-8, 8);
			TrailEnt->velocity[2] += irand(64, 128);

			TrailEnt->origin[0] += irand(-1, 1);
			TrailEnt->origin[1] += irand(-1, 1);
			TrailEnt->origin[2] += irand(-1, 1);

			TrailEnt->alpha = 1.0;
			TrailEnt->d_alpha = -2.0;

			AddEffect(owner,TrailEnt);
		}
		else if(Self->classID == STAFF_TRAIL3)
		{
			TrailEnt->r.frame = 0;

			TrailEnt->r.flags=RF_TRANSLUCENT | RF_TRANS_ADD;

			TrailEnt->r.scale = 0.2;
			TrailEnt->d_scale = -0.35;

			TrailEnt->alpha = 0.75;
			TrailEnt->d_alpha = -4;
			
			if (owner->current.effects & EF_BLOOD_ENABLED)
			{
				TrailEnt->r.color.c = 0x50000018;
			}
			else
			{
				int	white;

				white = irand(128, 208);

				TrailEnt->r.color.r = white;
				TrailEnt->r.color.g = white;
				TrailEnt->r.color.b = 128 + irand(108, 127);
				TrailEnt->r.color.a = 64 + irand(16, 128);
			}

			AddEffect(owner,TrailEnt);
		}

		//
		VectorAdd(curpt, diff, curpt);
		NoOfIntervals--;
	}
	Self->NoOfAnimFrames+=1.0;
	if (Self->NoOfAnimFrames > 5.0)
		return(false);
	else
		return(true);
}

// ************************************************************************************************
// FXStaffRemove
// ------------
// ************************************************************************************************

// This effect spawns 150+ client fx which will cause problems

void FXStaffRemove(centity_t *owner,int Type,int Flags,vec3_t Origin)
{
	client_entity_t *stafffx;
	byte			fxtype;

	if (Flags & CEF_FLAG6)
		fxtype = STAFF_TYPE_HELL;
	else
		fxtype = STAFF_TYPE_SWORD;

	if(!ReferencesInitialized(owner))
	{
		return;
	}

	stafffx = ClientEntity_new(Type, Flags, Origin, 0, 17);

	if(Flags & CEF_FLAG7)//blue
		stafffx->classID = STAFF_TRAIL3;
	else if(Flags & CEF_FLAG8)//flames
		stafffx->classID = STAFF_TRAIL2;
	else//normal
		stafffx->classID = STAFF_TRAIL;

	stafffx->Update = FXStaffRemoveThink;
	stafffx->flags |= CEF_NO_DRAW;
	stafffx->NoOfAnimFrames=1;
	stafffx->refPoint = fxtype;

	AddEffect(owner, stafffx);
}
