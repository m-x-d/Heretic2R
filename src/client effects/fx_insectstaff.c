//
// fx_insectstaff.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "ce_DLight.h"
#include "g_playstats.h"
#include "Matrix.h"
#include "Particle.h"
#include "Random.h"
#include "Reference.h"
#include "Utilities.h"
#include "Vector.h"

#define FIST_DELTA_FORWARD			8.0f
#define FIST_DELTA_THETA			0.12f
#define FIST_SPIRAL_RAD				0.75f
#define FIST_SCALE					0.2f

#define FX_GLOBE_FLY_SPEED			600.0f
#define FX_SOFT_GLOBE_AURA_SCALE	0.6f
#define FX_GLOBE_AURA_SCALE			0.8f
#define NUM_SPEAR_EXPLODES			8

//mxd. Mirrored in m_tcheckirk.h
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

static struct model_s* insect_model;
static struct model_s* sword_model;
static struct model_s* globe_models[5];
static struct model_s* spear_models[4];

void PreCacheIEffects(void)
{
	insect_model = fxi.RegisterModel("sprites/spells/spark_blue.sp2");

	sword_model = fxi.RegisterModel("sprites/spells/patball.sp2");

	spear_models[0] = fxi.RegisterModel("sprites/Spells/spark_red.sp2");
	spear_models[1] = fxi.RegisterModel("sprites/Spells/flyingfist.sp2");
	spear_models[2] = fxi.RegisterModel("sprites/Spells/spark_yellow.sp2");
	spear_models[3] = fxi.RegisterModel("sprites/fx/halo.sp2");

	globe_models[0] = fxi.RegisterModel("sprites/spells/shboom.sp2");
	globe_models[1] = fxi.RegisterModel("sprites/fx/halo.sp2");
	globe_models[2] = fxi.RegisterModel("Sprites/Spells/spark_blue.sp2");
	globe_models[3] = fxi.RegisterModel("models/spells/sphere/tris.fm");
	globe_models[4] = fxi.RegisterModel("sprites/fx/neon.sp2");
}

static qboolean FXInsectStaffTrailThink(struct client_entity_s* self, const centity_t* owner)
{
	vec3_t trail_start, trail;

	VectorCopy(owner->origin, trail_start);
	VectorSubtract(owner->origin, self->origin, trail);

	self->r.scale = flrand(0.8f, 1.3f);
	float trail_length = VectorNormalize(trail);

	if (trail_length > 0.05f)
	{
		vec3_t right;
		PerpendicularVector(right, trail);

		vec3_t up;
		CrossProduct(trail, right, up);

		VectorScale(trail, FIST_DELTA_FORWARD, trail);

		const int no_of_intervals = (int)(trail_length / FIST_DELTA_FORWARD);
		if (no_of_intervals > 40)
			return false;

		float theta = (float)fxi.cl->time * FIST_DELTA_THETA;
		const float delta_theta = fxi.cls->frametime * FIST_DELTA_THETA / (float)no_of_intervals;
		const int flags = (int)(self->flags & ~(CEF_OWNERS_ORIGIN | CEF_NO_DRAW)); //mxd

		while (trail_length > 0.0f)
		{
			client_entity_t* trail_ent = ClientEntity_new(FX_I_EFFECTS, flags, trail_start, NULL, 1000);

			trail_ent->r.model = &insect_model;
			VectorMA(trail_start, FIST_SPIRAL_RAD * cosf(theta), right, trail_ent->r.origin);
			VectorMA(trail_start, FIST_SPIRAL_RAD * sinf(theta), up, trail_ent->r.origin);

			trail_ent->r.flags = RF_TRANSLUCENT | RF_TRANS_ADD;

			VectorRandomCopy(self->velocity, trail_ent->velocity, flrand(0.0f, 4.0f));

			trail_ent->r.scale = FIST_SCALE + flrand(0.0f, 0.05f);
			trail_ent->d_alpha = flrand(-1.75f, -2.0f);
			trail_ent->d_scale = flrand(-0.75f, -1.0f);
			trail_ent->radius = 20.0f;

			AddEffect(NULL, trail_ent);

			trail_length -= FIST_DELTA_FORWARD;
			theta += delta_theta;

			VectorAdd(trail_start, trail, trail_start);
		}
	}

	VectorCopy(owner->origin, self->origin);
	VectorCopy(trail_start, self->r.origin);

	return true;
}

static void FXInsectStaff(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	const paletteRGBA_t light_color = { .r = 255,.g = 64,.b = 32,.a = 255 };

	client_entity_t* trail = ClientEntity_new(type, flags, origin, NULL, 17);

	trail->r.model = &insect_model;
	trail->r.flags = RF_TRANSLUCENT | RF_TRANS_ADD;
	trail->r.scale = flrand(0.8f, 1.3f);
	trail->r.color = color_white;
	trail->radius = 20.0f;

	trail->Update = FXInsectStaffTrailThink;

	if (r_detail->value > DETAIL_NORMAL)
		trail->dlight = CE_DLight_new(light_color, 150.0f, 0.0f);

	AddEffect(owner, trail);
}

static void FXInsectStaffExplode(const int type, const int flags, vec3_t origin, vec3_t dir)
{
	const paletteRGBA_t light_color = { .r = 255,.g = 64,.b = 32,.a = 255 };

	if (flags & CEF_FLAG6)
		FXClientScorchmark(origin, dir);

	VectorScale(dir, 32.0f, dir);

	const int count = GetScaledCount(irand(8, 12), 0.8f);

	for (int i = 0; i < count; i++)
	{
		const qboolean is_last_puff = (i == count - 1); //mxd
		const int next_think_time = (is_last_puff ? 500 : 1000); //mxd

		client_entity_t* smoke_puff = ClientEntity_new(type, flags, origin, NULL, next_think_time);

		smoke_puff->r.model = &insect_model;
		smoke_puff->r.flags = RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		smoke_puff->acceleration[2] = flrand(-40.0f, -60.0f);
		smoke_puff->d_scale = flrand(-1.75f, -2.0f);
		smoke_puff->d_alpha = -0.2f;
		smoke_puff->radius = 20.0f;

		if (is_last_puff)
			smoke_puff->dlight = CE_DLight_new(light_color, 150.0f, 0.0f);
		else
			VectorRandomCopy(dir, smoke_puff->velocity, flrand(16.0f, 128.0f));

		smoke_puff->Scale = flrand(0.25f, 0.35f); //TODO: supposed to set r.scale? Where is this used?

		AddEffect(NULL, smoke_puff);
	}
}

static qboolean FXGlobeOfOuchinessGlobeThink(struct client_entity_s* self, centity_t* owner)
{
	self->r.scale = flrand(0.35f, 0.5f);
	return true;
}

static qboolean FXGlobeOfOuchinessAuraThink(const struct client_entity_s* self, const centity_t* owner)
{
	vec3_t trail_start;
	VectorCopy(owner->origin, trail_start);

	vec3_t trail_dir;
	VectorSubtract(owner->lerp_origin, owner->origin, trail_dir);

	float trail_length = VectorNormalize(trail_dir);
	if (trail_length < 0.001f)
		trail_length += 2.0f;

	vec3_t right;
	PerpendicularVector(right, trail_dir);

	vec3_t up;
	CrossProduct(trail_dir, right, up);

	VectorScale(trail_dir, FX_GLOBE_FLY_SPEED, trail_dir);

	const int flags = (int)(self->flags & ~(CEF_OWNERS_ORIGIN | CEF_NO_DRAW));
	const float base_scale = (r_detail->value < DETAIL_NORMAL) ? FX_SOFT_GLOBE_AURA_SCALE : FX_GLOBE_AURA_SCALE; //mxd
	const float delta_scale = (r_detail->value < DETAIL_NORMAL) ? -1.0f : -0.5f; //mxd

	for (int i = 0; i < 41; i++)
	{
		if (trail_length <= 0.0f)
			return true;

		client_entity_t* trail = ClientEntity_new(FX_I_EFFECTS, flags, trail_start, NULL, 500);

		trail->r.model = &globe_models[0];
		trail->r.flags = RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		trail->Scale = base_scale + flrand(0.0f, 0.1f); //TODO: supposed to set r.scale? Where is this used?
		COLOUR_SET(trail->color, irand(128, 180), irand(128, 180), irand(64, 80)); //mxd. Use macro.
		trail->alpha = 0.7f;
		trail->d_alpha = -1.0f;
		trail->d_scale = delta_scale;
		trail->radius = 70.0f;

		AddEffect(NULL, trail);

		VectorAdd(trail_start, trail_dir, trail_start);
		trail_length -= FX_GLOBE_FLY_SPEED;
	}

	return true;
}

static void FXInsectGlobe(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	const paletteRGBA_t light_color = { .r = 0,.g = 0,.b = 255,.a = 255 };

	// Create a fiery blue aura around the globe.
	const int caster_update = ((r_detail->value < DETAIL_NORMAL) ? 125 : 100);
	client_entity_t* aura_thinker = ClientEntity_new(type, flags, origin, NULL, caster_update);

	aura_thinker->flags |= CEF_NO_DRAW;
	aura_thinker->dlight = CE_DLight_new(light_color, 150.0f, 0.0f);
	aura_thinker->Update = FXGlobeOfOuchinessAuraThink;
	aura_thinker->extra = owner; // The caster's centity_t.

	AddEffect(owner, aura_thinker);
	FXGlobeOfOuchinessAuraThink(aura_thinker, owner);

	// Create the globe of ouchiness itself.
	client_entity_t* globe_thinker = ClientEntity_new(type, flags, origin, NULL, 100);

	globe_thinker->r.model = &globe_models[1];
	globe_thinker->r.flags = RF_TRANSLUCENT | RF_TRANS_ADD;
	globe_thinker->r.scale = flrand(0.15f, 0.2f);

	COLOUR_SET(globe_thinker->r.color, irand(128, 180), irand(128, 180), irand(180, 255)); //mxd. Use macro.

	globe_thinker->radius = 70.0f;
	globe_thinker->AddToView = LinkedEntityUpdatePlacement;
	globe_thinker->Update = FXGlobeOfOuchinessGlobeThink;

	AddEffect(owner, globe_thinker);
}

static qboolean FXGlobeOfOuchinessGlowballThink(struct client_entity_s* self, const centity_t* owner)
{
	if (owner->current.effects & EF_MARCUS_FLAG1)
		self->color.r++;

	if (self->color.r > 3)
	{
		// Create a trailing spark.
		client_entity_t* spark = ClientEntity_new(FX_I_EFFECTS, self->flags & ~CEF_OWNERS_ORIGIN, self->r.origin, NULL, 500);

		spark->r.model = globe_models + 2;
		spark->r.flags = RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		spark->r.scale = FIST_SCALE + flrand(0.0f, 0.05f);
		spark->d_alpha = flrand(-1.75f, -2.0f);
		spark->d_scale = flrand(-0.75f, -1.0f);
		spark->radius = 20.0f;

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

static qboolean FXGlobeOfOuchinessGlowballSpawnerThink(struct client_entity_s* self, centity_t* owner)
{
	// 'self->extra' refers to the caster's centity_t.
	const centity_t* caster = (centity_t*)self->extra;

	// This tells if we are wasting our time, because the reference points are culled.
	if (!RefPointsValid(caster) || !(owner->current.effects & EF_MARCUS_FLAG1))
		return true;

	// If the spell is still building, create some swirling blue Glowballs.
	const int flags = (int)(self->flags & ~(CEF_NO_DRAW | CEF_OWNERS_ORIGIN));
	client_entity_t* glowball = ClientEntity_new(FX_I_EFFECTS, flags, caster->origin, NULL, 50);

	self->flags |= CEF_DONT_LINK;

	vec3_t temp;
	VectorCopy(owner->current.angles, temp);
	VectorScale(temp, 180.0f / ANGLE_180, temp);

	vec3_t owner_fwd;
	AngleVectors(temp, owner_fwd, NULL, NULL);

	// Make me spawn from my caster's left / right hands (alternating).
	matrix3_t rotation;
	Matrix3FromAngles(caster->lerp_angles, rotation);

	const qboolean is_sword_fx = (self->color.g & 1); //mxd
	const int ref_index = (is_sword_fx ? INSECT_SWORD : INSECT_STAFF); //mxd
	Matrix3MultByVec3(rotation, caster->referenceInfo->references[ref_index].placement.origin, glowball->r.origin);

	VectorAdd(caster->origin, glowball->r.origin, glowball->r.origin);

	// Set my velocity.
	glowball->velocity[0] = owner_fwd[0] * 175.0f + flrand(-25.0f, 25.0f) * (is_sword_fx ? -1.0f : 1.0f);
	glowball->velocity[1] = owner_fwd[1] * 175.0f + flrand(-25.0f, 25.0f) * (is_sword_fx ? 1.0f : -1.0f);
	glowball->velocity[2] = flrand(-200.0f, 100.0f);

	// Fill in the rest of my info.
	glowball->r.model = &globe_models[2];

	glowball->r.flags = RF_TRANSLUCENT | RF_TRANS_ADD;
	COLOUR_SET(glowball->r.color, irand(128, 180), irand(128, 180), irand(180, 255)); //mxd. Use macro.
	glowball->color.r = 1;
	glowball->radius = 20.0f;

	glowball->extra = (void*)owner;
	glowball->Update = FXGlobeOfOuchinessGlowballThink;

	AddEffect(owner, glowball);
	FXGlobeOfOuchinessGlowballThink(glowball, owner);

	self->color.g++;

	return true;
}

static void FXInsectGlow(centity_t* owner, const int type, const int flags, vec3_t origin, const short caster_entnum)
{
	// Create a spawner that will create the glowballs.
	const int caster_update = ((r_detail->value < DETAIL_NORMAL) ? 250 : 100);
	client_entity_t* glowball_spawner = ClientEntity_new(type, flags, origin, NULL, caster_update);

	glowball_spawner->flags |= CEF_NO_DRAW;
	glowball_spawner->Update = FXGlobeOfOuchinessGlowballSpawnerThink;
	glowball_spawner->extra = (void*)(&fxi.server_entities[caster_entnum]);

	AddEffect(owner, glowball_spawner);
}

static qboolean InsectFirstSeenInit(struct client_entity_s* self, const centity_t* owner)
{
	self->refMask |= INSECT_MASK;
	EnableRefPoints(owner->referenceInfo, self->refMask);

	self->AddToView = NULL;
	self->Update = KeepSelfAI;

	return true;
}

static void FXInsectReadyRefs(centity_t* owner, const int type, int flags, const vec3_t origin)
{
	flags |= CEF_NO_DRAW;
	client_entity_t* self = ClientEntity_new(type, flags, origin, NULL, 17);

	self->Update = NULL;
	self->AddToView = InsectFirstSeenInit;

	AddEffect(owner, self);
}

static void FXInsectSpear(centity_t* owner, const int type, const int flags, const vec3_t origin, const vec3_t vel)
{
	const paletteRGBA_t light_color = { .r = 255, .g = 128, .b = 64, .a = 255 };

	client_entity_t* hellbolt = ClientEntity_new(type, flags | CEF_DONT_LINK, origin, NULL, 10000);

	hellbolt->r.model = &spear_models[0];
	hellbolt->r.flags = RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	hellbolt->r.color = light_color;
	hellbolt->radius = 10.0f;
	VectorCopy(vel, hellbolt->velocity);
	hellbolt->AddToView = LinkedEntityUpdatePlacement;

	if (r_detail->value > DETAIL_NORMAL)
		hellbolt->dlight = CE_DLight_new(light_color, 150.0f, -300.0f);

	AddEffect(owner, hellbolt);
}

static qboolean FXInsectSpear2Update(struct client_entity_s* self, const centity_t* owner)
{
	self->r.color.a = (byte)irand(128, 136);
	self->r.scale = flrand(0.1f, 0.5f);

	if (!VectorCompare(owner->lerp_origin, self->startpos2))
		VectorCopy(owner->lerp_origin, self->startpos2);

	vec3_t dir;
	VectorSubtract(owner->lerp_origin, self->startpos2, dir);
	const float dist = VectorNormalize(dir);

	for (int i = 0; i < 10; i++)
	{
		client_particle_t* spark = ClientParticle_new(PART_16x16_SPARK_Y, self->r.color, 200);
		spark->type |= PFL_ADDITIVE;

		spark->acceleration[2] = 0.5f;

		spark->scale = flrand(5.0f, 6.0f);
		spark->d_scale = flrand(-13.0f, -20.0f);

		COLOUR_SETA(spark->color, irand(240, 255), irand(240, 255), irand(240, 255), irand(64, 196)); //mxd. Use macro.
		spark->d_alpha = -128.0f;

		VectorAdd(self->startpos2, spark->origin, spark->origin);
		VectorMA(spark->origin, dist / (float)i, dir, spark->origin);

		for (int c = 0; c < 3; c++)
			spark->origin[c] += flrand(-2.0f, 2.0f);

		AddParticleToList(self, spark);
	}

	return true;
}

static void FXInsectSpear2(centity_t* owner, const int type, const vec3_t origin)
{
	const paletteRGBA_t light_color = { .r = 255, .g = 128, .b = 255, .a = 255 };

	// Setup halo.
	client_entity_t* halo = ClientEntity_new(type, CEF_OWNERS_ORIGIN | CEF_ABSOLUTE_PARTS, origin, NULL, 20);

	halo->r.model = &spear_models[3]; // Halo sprite.
	halo->r.flags = RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;

	halo->r.scale = flrand(0.2f, 0.4f);
	halo->radius = 10.0f;
	VectorCopy(owner->current.origin, halo->startpos2);

	if (r_detail->value > DETAIL_NORMAL)
		halo->dlight = CE_DLight_new(light_color, 150.0f, -300.0f);

	halo->AddToView = LinkedEntityUpdatePlacement;
	halo->Update = FXInsectSpear2Update;

	AddEffect(owner, halo);

	// Setup spark.
	client_entity_t* spark = ClientEntity_new(type, CEF_OWNERS_ORIGIN, origin, NULL, 10000);

	spark->r.model = &spear_models[2]; // Yellow spark sprite.
	spark->r.flags = RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;

	spark->r.color.c = 0x33ffffff;
	spark->r.scale = flrand(1.0f, 2.0f);
	spark->radius = 10.0f;
	spark->AddToView = LinkedEntityUpdatePlacement;

	AddEffect(owner, spark);
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
		TrailEnt->r.model = &sword_model;
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
			FXInsectSpear(owner, type, flags, origin, vel);
			break;
		
		case FX_I_SP_MSL_HIT:
			FXISpMslHit(owner, type, flags, origin, vel);
			break;

		case FX_I_GLOBE:
			FXInsectGlobe(owner, type, flags, origin);
			break;

		case FX_I_GLOW:
			FXInsectGlow(owner, type, flags, origin, (short)(vel[0]));
			break;

		case FX_I_STAFF:
			FXInsectStaff(owner, type, flags, origin);
			break;

		case FX_I_ST_MSL_HIT:
			FXInsectStaffExplode(type, flags, origin, vel);
			break;

		case FX_I_RREFS:
			FXInsectReadyRefs (owner, type, flags, origin);
			break;

		case FX_I_SPEAR2:
			FXInsectSpear2(owner, type, origin);
			break;

		case FX_I_SP_MSL_HIT2:
			FXISpMslHit2(owner, type, flags, origin, vel);
			break;

		default:
			break;
	}
}

