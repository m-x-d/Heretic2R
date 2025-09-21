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

void PreCacheStaff(void)
{
	staff_models[0] = fxi.RegisterModel("sprites/spells/patball.sp2");
	staff_models[1] = fxi.RegisterModel("sprites/fx/halo.sp2");
	staff_models[2] = fxi.RegisterModel("sprites/spells/wflame2.sp2");
	staff_models[3] = fxi.RegisterModel("sprites/fx/steam.sp2");
	staff_models[4] = fxi.RegisterModel("sprites/fx/haloblue.sp2");
}

static qboolean StaffElementThink(struct client_entity_s* self, centity_t* owner)
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

static qboolean StaffLevel2Think(struct client_entity_s* self, centity_t* owner)
{
	// If we've timed out, stop the effect (allow for fading).
	if (self->LifeTime > 0 && self->LifeTime < fxi.cl->time)
	{
		self->Update = RemoveSelfAI;
		self->nextThinkTime = fxi.cl->time + 500; //BUGFIX: mxd. sets updateTime in original logic (makes no sense: updateTime is ADDED to fxi.cl->time in UpdateEffects()).

		return true;
	}

	// This tells if we are wasting our time, because the reference points are culled.
	if (!RefPointsValid(owner))
		return false; // Remove the effect in this case.

	// If this reference point hasn't changed since the last frame, return.
	vec3_t diff;
	const int ref_index = self->NoOfAnimFrames;
	VectorSubtract(owner->referenceInfo->references[ref_index].placement.origin, owner->referenceInfo->oldReferences[ref_index].placement.origin, diff);

	if (Q_fabs(diff[0] + diff[1] + diff[2]) < 0.1f)
		return true;

	const int num_of_intervals = GetScaledCount((int)(VectorLength(diff) * 0.5f), 1.0f);
	if (num_of_intervals > 40)
		return false;

	// Average out the two right hand positions to get a pivot point.
	vec3_t cur_pivot;
	VectorCopy(owner->referenceInfo->oldReferences[CORVUS_RIGHTHAND].placement.origin, cur_pivot);

	vec3_t delta_pivot;
	VectorSubtract(owner->referenceInfo->references[CORVUS_RIGHTHAND].placement.origin, cur_pivot, delta_pivot);
	VectorScale(delta_pivot, 1.0f / (float)num_of_intervals, delta_pivot);

	vec3_t cur_normal;
	VectorCopy(owner->referenceInfo->oldReferences[ref_index].placement.direction, cur_normal);

	vec3_t delta_normal;
	VectorSubtract(owner->referenceInfo->references[ref_index].placement.direction, cur_normal, delta_normal);
	VectorScale(delta_normal, 1.0f / (float)num_of_intervals, delta_normal);

	vec3_t dir;
	VectorCopy(cur_normal, dir);
	VectorNormalize(dir);

	for (int i = 0; i < num_of_intervals; i++)
	{
		vec3_t trail_org;
		VectorMA(cur_pivot, STAFF_LENGTH, dir, trail_org);

		client_entity_t* trail = ClientEntity_new(FX_SPELLHANDS, (int)(self->flags & ~CEF_NO_DRAW), trail_org, NULL, 2000);

		trail->r.model = &staff_models[STAFF_TRAIL2];
		trail->r.flags = RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		trail->r.scale = flrand(0.2f, 0.3f);
		trail->d_scale = flrand(-1.0f, -0.5f);
		trail->d_alpha = -2.0f;

		VectorSet(trail->velocity, flrand(-8.0f, 8.0f), flrand(-8.0f, 8.0f), flrand(64.0f, 128.0f)); //mxd. Original logic uses irand() here.

		for (int c = 0; c < 3; c++)
			trail->origin[c] += flrand(-1.0f, 1.0f); //mxd. Original logic uses irand() here.

		// Attach a dynamic light to the last one.
		if (i == num_of_intervals - 1 && R_DETAIL >= DETAIL_NORMAL)
		{
			const byte c = (byte)irand(8, 16);
			const paletteRGBA_t color = { .r = (byte)irand(236, 255), .g = 64 + c, .b = 16 + c, .a = (byte)irand(80, 192) };
			trail->dlight = CE_DLight_new(color, flrand(50.0f, 150.0f), -100.0f); //mxd. Original logic uses irand() here.
		}

		matrix3_t rotation;
		Matrix3FromAngles(owner->lerp_angles, rotation);

		vec3_t origin;
		Matrix3MultByVec3(rotation, trail->origin, origin);
		VectorAdd(owner->origin, origin, trail->r.origin);

		AddEffect(NULL, trail);

		// Create smoke?
		if (irand(0, 3) == 0)
		{
			client_entity_t* smoke = ClientEntity_new(FX_SPELLHANDS, (int)(self->flags & ~CEF_NO_DRAW), trail_org, NULL, 5000);

			smoke->r.model = &staff_models[STAFF_TRAIL_SMOKE];
			smoke->r.flags = RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
			smoke->r.scale = flrand(0.1f, 0.15f);
			smoke->d_scale = 1.0f;
			smoke->alpha = 0.75f;
			smoke->d_alpha = -1.0f;

			VectorSet(smoke->velocity, flrand(-16.0f, 16.0f), flrand(-16.0f, 16.0f), flrand(64.0f, 128.0f)); //mxd. Original logic uses irand() here.

			const int c = irand(32, 64);
			COLOUR_SETA(smoke->r.color, c, c, c, 128);

			// Attach a dynamic light to the last one.
			if (i == num_of_intervals - 1 && R_DETAIL >= DETAIL_NORMAL)
			{
				const byte c2 = (byte)irand(8, 16);
				const paletteRGBA_t color = { .r = (byte)irand(236, 255), .g = 64 + c2, .b = 16 + c2, .a = (byte)irand(80, 192) };
				smoke->dlight = CE_DLight_new(color, flrand(50.0f, 150.0f), -100.0f); //mxd. Original logic uses irand() here.
			}

			Matrix3FromAngles(owner->lerp_angles, rotation);
			Matrix3MultByVec3(rotation, smoke->origin, origin);
			VectorAdd(owner->origin, origin, smoke->r.origin);

			AddEffect(NULL, smoke);
		}

		VectorAdd(cur_pivot, delta_pivot, cur_pivot);
		VectorAdd(cur_normal, delta_normal, cur_normal);
		VectorNormalize2(cur_normal, dir);
	}

	return true;
}

static qboolean StaffLevel3Think(struct client_entity_s* self, centity_t* owner)
{
	// If we've timed out, stop the effect (allow for fading).
	if (self->LifeTime > 0 && self->LifeTime < fxi.cl->time)
	{
		self->Update = RemoveSelfAI;
		self->nextThinkTime = fxi.cl->time + 500; //BUGFIX: mxd. sets updateTime in original logic (makes no sense: updateTime is ADDED to fxi.cl->time in UpdateEffects()).

		return true;
	}

	// This tells if we are wasting our time, because the reference points are culled.
	if (!RefPointsValid(owner))
		return false; // Remove the effect in this case.

	// If this reference point hasn't changed since the last frame, return.
	vec3_t diff;
	const int ref_index = self->NoOfAnimFrames;
	VectorSubtract(owner->referenceInfo->references[ref_index].placement.origin, owner->referenceInfo->oldReferences[ref_index].placement.origin, diff);

	if (Q_fabs(diff[0] + diff[1] + diff[2]) < 0.1f)
		return true;

	const int num_of_intervals = GetScaledCount((int)(VectorLength(diff) * 0.5f), 1.0f);
	if (num_of_intervals > 40)
		return false;

	// Take the before and after points and try to draw an arc.

	// Average out the two right hand positions to get a pivot point.
	vec3_t cur_pivot;
	VectorCopy(owner->referenceInfo->oldReferences[CORVUS_RIGHTHAND].placement.origin, cur_pivot);

	vec3_t delta_pivot;
	VectorSubtract(owner->referenceInfo->references[CORVUS_RIGHTHAND].placement.origin, cur_pivot, delta_pivot);
	VectorScale(delta_pivot, 1.0f / (float)num_of_intervals, delta_pivot);

	vec3_t cur_normal;
	VectorCopy(owner->referenceInfo->oldReferences[ref_index].placement.direction, cur_normal);

	vec3_t delta_normal;
	VectorSubtract(owner->referenceInfo->references[ref_index].placement.direction, cur_normal, delta_normal);
	VectorScale(delta_normal, 1.0f / (float)num_of_intervals, delta_normal);

	vec3_t dir;
	VectorCopy(cur_normal, dir);
	VectorNormalize(dir);

	for (int i = 0; i < num_of_intervals; i++)
	{
		vec3_t trail_org;
		VectorMA(cur_pivot, STAFF_LENGTH, dir, trail_org);

		client_entity_t* trail = ClientEntity_new(FX_SPELLHANDS, (int)(self->flags & ~CEF_NO_DRAW), trail_org, NULL, 500);

		trail->r.model = &staff_models[STAFF_TRAIL3];
		trail->r.flags = RF_TRANSLUCENT | RF_TRANS_ADD;
		trail->r.scale = 0.3f;
		trail->d_scale = -0.75f;
		trail->alpha = 0.75f;
		trail->d_alpha = -4.0f;

		const int c = irand(128, 208);
		if (owner->current.effects & EF_BLOOD_ENABLED)
			COLOUR_SETA(trail->r.color, irand(236, 255), c, c, irand(80, 192)); //mxd. Use macro.
		else
			COLOUR_SETA(trail->r.color, c, c, irand(236, 255), irand(80, 192)); //mxd. Use macro.

		// Attach a dynamic light to the last one.
		if (i == num_of_intervals - 1 && R_DETAIL >= DETAIL_NORMAL)
			trail->dlight = CE_DLight_new(trail->r.color, 100.0f, -100.0f);

		trail->AddToView = OffsetLinkedEntityUpdatePlacement;

		AddEffect(owner, trail);

		VectorAdd(cur_pivot, delta_pivot, cur_pivot);
		VectorAdd(cur_normal, delta_normal, cur_normal);
		VectorNormalize2(cur_normal, dir);
	}

	return true;
}

static qboolean StaffLevel1Think(struct client_entity_s* self, centity_t* owner)
{
	// If we've timed out, stop the effect (allow for fading).
	if (self->LifeTime > 0 && self->LifeTime < fxi.cl->time)
	{
		self->Update = RemoveSelfAI;
		self->nextThinkTime = fxi.cl->time + 500; //BUGFIX: mxd. sets updateTime in original logic (makes no sense: updateTime is ADDED to fxi.cl->time in UpdateEffects()).

		return true;
	}

	// This tells if we are wasting our time, because the reference points are culled.
	if (!RefPointsValid(owner))
		return false; // Remove the effect in this case.

	self->updateTime = MIN_UPDATE_TIME; //FIXME: with a next think time this effect does not look right.

	// If this reference point hasn't changed since the last frame, return.
	vec3_t diff;
	const int ref_index = self->NoOfAnimFrames;
	VectorSubtract(owner->referenceInfo->references[ref_index].placement.origin, owner->referenceInfo->oldReferences[ref_index].placement.origin, diff);

	if (Q_fabs(diff[0] + diff[1] + diff[2]) < 0.1f)
		return true;

	const int num_of_intervals = GetScaledCount((int)(VectorLength(diff) * 0.75f), 1.0f);
	if (num_of_intervals > 40)
		return false;

	// Take the before and after points and try to draw an arc.

	// Average out the two right hand positions to get a pivot point.
	vec3_t cur_pivot;
	VectorCopy(owner->referenceInfo->oldReferences[CORVUS_RIGHTHAND].placement.origin, cur_pivot);

	vec3_t delta_pivot;
	VectorSubtract(owner->referenceInfo->references[CORVUS_RIGHTHAND].placement.origin, cur_pivot, delta_pivot);
	VectorScale(delta_pivot, 1.0f / (float)num_of_intervals, delta_pivot);

	vec3_t cur_normal;
	VectorCopy(owner->referenceInfo->oldReferences[ref_index].placement.direction, cur_normal);

	vec3_t delta_normal;
	VectorSubtract(owner->referenceInfo->references[ref_index].placement.direction, cur_normal, delta_normal);
	VectorScale(delta_normal, 1.0f / (float)num_of_intervals, delta_normal);

	vec3_t dir;
	VectorCopy(cur_normal, dir);
	VectorNormalize(dir);

	for (int i = 0; i < num_of_intervals; i++)
	{
		// Get the position of this sprite.
		vec3_t trail_org;
		VectorMA(cur_pivot, STAFF_LENGTH, dir, trail_org);

		client_entity_t* trail = ClientEntity_new(FX_SPELLHANDS, (int)(self->flags & ~CEF_NO_DRAW), trail_org, NULL, 1500);

		trail->r.model = &staff_models[STAFF_TRAIL]; // patball sprite.
		trail->r.flags = RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		trail->r.frame = 1;
		trail->d_scale = -0.5f;
		trail->alpha = 0.5f;
		trail->d_alpha = -1.0f;
		trail->r.scale = self->xscale;
		trail->r.color.c = ((owner->current.effects & EF_BLOOD_ENABLED) ? 0x50000018 : self->color.c);
		trail->AddToView = OffsetLinkedEntityUpdatePlacement;

		AddEffect(owner, trail);

		VectorAdd(cur_pivot, delta_pivot, cur_pivot);
		VectorAdd(cur_normal, delta_normal, cur_normal);
		VectorNormalize2(cur_normal, dir);
	}

	return true;
}

void FXStaff(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	byte powerlevel;
	char lifetime;
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_STAFF].formatString, &powerlevel, &lifetime);

	if (!ReferencesInitialized(owner))
		return;

	// Add a fiery trail effect to the player's staff.
	client_entity_t* trail = ClientEntity_new(type, CEF_OWNERS_ORIGIN | CEF_DONT_LINK, origin, NULL, 17);

	trail->flags |= CEF_NO_DRAW;
	trail->NoOfAnimFrames = CORVUS_BLADE;
	trail->SpawnData = powerlevel;
	trail->LifeTime = ((lifetime > 0) ? fxi.cl->time + lifetime * 100 : (int)lifetime);

	switch (powerlevel)
	{
		case 1: // Blue.
		default:
			trail->Update = StaffLevel1Think;
			trail->color.c = 0x02201010;
			trail->xscale = 0.175f;
			break;

			//NOTE: These were swapped after the functions were created.
		case 2: // Fire.
			trail->Update = StaffLevel3Think;
			trail->color = color_white;
			trail->xscale = 0.2f;
			break;

		case 3: // Energy blast.
		case 4:
			trail->Update = StaffLevel2Think;
			trail->color = color_white;
			trail->xscale = 0.225f;
			break;
	}

	AddEffect(owner, trail);
}

static qboolean StaffCreateThink(struct client_entity_s* self, centity_t* owner)
{
	vec3_t start_pt;
	vec3_t end_pt;
	uint color;

	self->updateTime = MIN_UPDATE_TIME; //FIXME: with a next think time this effect does not look right.

	// This tells if we are wasting our time, because the reference points are culled.
	if (!RefPointsValid(owner))
		return false; // Remove the effect in this case.

	// If this reference point hasn't changed since the last frame, return.
	switch (self->refPoint)
	{
		case STAFF_TYPE_HELL:
			VectorAdd(owner->referenceInfo->references[CORVUS_RIGHTHAND].placement.origin, owner->referenceInfo->references[CORVUS_STAFF].placement.origin, start_pt);
			VectorScale(start_pt, 0.5f, start_pt);
			VectorCopy(owner->referenceInfo->references[CORVUS_HELL_HEAD].placement.origin, end_pt);
			color = 0xff2020ff;
			break;

		case STAFF_TYPE_SWORD:
		default:
			VectorCopy(owner->referenceInfo->references[CORVUS_STAFF].placement.origin, start_pt);
			VectorCopy(owner->referenceInfo->references[CORVUS_BLADE].placement.origin, end_pt);
			color = 0xff20ff20;
			break;
	}

	vec3_t diff;
	VectorSubtract(end_pt, start_pt, diff);

	const int num_of_intervals = (int)(VectorLength(diff) * 0.5f);
	if (num_of_intervals > 40)
		return false;

	VectorScale(diff, 1.0f / (float)num_of_intervals, diff);

	vec3_t trail_org;
	VectorCopy(start_pt, trail_org); // This rides on the assumption that the normal given is already a unit norm. //TODO: the normal given is NOT a unit norm!

	for (int i = 0; i < num_of_intervals; i++)
	{
		client_entity_t* trail = ClientEntity_new(FX_SPELLHANDS, (int)(self->flags & ~CEF_NO_DRAW), trail_org, NULL, 100);

		trail->r.model = &staff_models[self->classID];
		trail->alpha = 0.8f - (float)self->NoOfAnimFrames * 0.1f;
		trail->r.flags = (RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA);
		trail->AddToView = OffsetLinkedEntityUpdatePlacement;

		if (self->classID == STAFF_TRAIL || self->refPoint == STAFF_TYPE_HELL)
		{
			trail->r.frame = 1;
			trail->d_scale = -0.25f;
			trail->d_alpha = -0.1f;
			trail->color.c = color;
			trail->r.scale = (float)self->NoOfAnimFrames * 0.05f;
			trail->AnimSpeed = 0.2f;
			trail->NoOfAnimFrames = 2;
			trail->Update = StaffElementThink;

			AddEffect(owner, trail);
			StaffElementThink(trail, owner);
		}
		else if (self->classID == STAFF_TRAIL2)
		{
			trail->r.scale = flrand(0.1f, 0.2f);
			trail->d_scale = flrand(-0.5f, -0.25f);
			trail->d_alpha = -2.0f;

			VectorSet(trail->velocity, flrand(-8.0f, 8.0f), flrand(-8.0f, 8.0f), flrand(64.0f, 128.0f)); //mxd. Original logic uses irand() here.

			for (int c = 0; c < 3; c++)
				trail->origin[c] += flrand(-1.0f, 1.0f); //mxd. Original logic uses irand() here.

			AddEffect(owner, trail);
		}
		else if (self->classID == STAFF_TRAIL3)
		{
			trail->r.flags = (RF_TRANSLUCENT | RF_TRANS_ADD);
			trail->r.scale = 0.2f;
			trail->d_scale = -0.35f;
			trail->alpha = 0.75f;
			trail->d_alpha = -4.0f;

			if (owner->current.effects & EF_BLOOD_ENABLED)
			{
				trail->r.color.c = 0x50000018;
			}
			else
			{
				const int c = irand(128, 208);
				COLOUR_SETA(trail->r.color, c, c, irand(236, 255), irand(80, 192)); //mxd. Use macro.
			}

			AddEffect(owner, trail);
		}

		VectorAdd(trail_org, diff, trail_org);
	}

	self->NoOfAnimFrames--;

	return self->NoOfAnimFrames > 0;
}

void FXStaffCreate(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	// This tells if we are wasting our time, because the reference points are culled.
	if (!RefPointsValid(owner))
		return; // Abandon the effect in this case.

	client_entity_t* staff_fx = ClientEntity_new(type, flags, origin, NULL, 17);

	if (flags & CEF_FLAG7) // Blue.
		staff_fx->classID = STAFF_TRAIL3;
	else if (flags & CEF_FLAG8) // Flames.
		staff_fx->classID = STAFF_TRAIL2;
	else // Normal.
		staff_fx->classID = STAFF_TRAIL;

	staff_fx->flags |= CEF_NO_DRAW;
	staff_fx->NoOfAnimFrames = 7;
	staff_fx->refPoint = (short)((flags & CEF_FLAG6) ? STAFF_TYPE_HELL : STAFF_TYPE_SWORD);
	staff_fx->Update = StaffCreateThink;

	AddEffect(owner, staff_fx);
}

//mxd. Added to reduce code duplication.
static void CreatePuff(centity_t* owner, const int flags, vec3_t origin)
{
	client_entity_t* puff = ClientEntity_new(FX_SPELLHANDS, (int)(flags & ~CEF_NO_DRAW), origin, NULL, 100);

	puff->r.model = &staff_models[STAFF_HALO];
	puff->r.flags = RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	puff->r.frame = 1;
	puff->r.scale = 0.5f;
	puff->d_scale = -0.3f;
	puff->alpha = 0.75f;
	puff->d_alpha = -0.2f;
	puff->AnimSpeed = 0.2f;
	puff->NoOfAnimFrames = 2;
	puff->Update = StaffElementThink;
	puff->AddToView = OffsetLinkedEntityUpdatePlacement;

	AddEffect(owner, puff);
	StaffElementThink(puff, owner);
}

void FXStaffCreatePoof(centity_t* owner, int type, const int flags, vec3_t origin)
{
	// This tells if we are wasting our time, because the reference points are culled.
	if (!RefPointsValid(owner))
		return; // Remove the effect in this case.

	vec3_t spawn_pt;
	const int ref_id = ((flags & CEF_FLAG6) ? CORVUS_HELL_HEAD : CORVUS_BLADE); //mxd
	VectorCopy(owner->referenceInfo->references[ref_id].placement.origin, spawn_pt);
	CreatePuff(owner, flags, spawn_pt); //mxd

	if (!(flags & CEF_FLAG6)) // Just for the sword staff.
	{
		vec3_t halo_org;
		VectorCopy(owner->referenceInfo->references[CORVUS_STAFF].placement.origin, halo_org);
		CreatePuff(owner, flags, halo_org); //mxd
	}
}

static qboolean StaffRemoveThink(struct client_entity_s* self, centity_t* owner)
{
	vec3_t start_pt;
	vec3_t end_pt;

	vec3_t trail_org;
	uint color;

	self->updateTime = MIN_UPDATE_TIME; //FIXME: with a next think time this effect does not look right.

	// This tells if we are wasting our time, because the reference points are culled.
	if (!RefPointsValid(owner))
		return false; // Remove the effect in this case.

	// If this reference point hasn't changed since the last frame, return.
	switch (self->refPoint)
	{
		case STAFF_TYPE_HELL:
			VectorAdd(owner->referenceInfo->references[CORVUS_RIGHTHAND].placement.origin, owner->referenceInfo->references[CORVUS_STAFF].placement.origin, start_pt);
			VectorScale(start_pt, 0.5f, start_pt);
			VectorCopy(owner->referenceInfo->references[CORVUS_HELL_HEAD].placement.origin, end_pt);
			color = 0xff2020ff;
			break;

		case STAFF_TYPE_SWORD:
		default:
			VectorCopy(owner->referenceInfo->references[CORVUS_STAFF].placement.origin, start_pt);
			VectorCopy(owner->referenceInfo->references[CORVUS_BLADE].placement.origin, end_pt);
			color = 0xff20ff20;
			break;
	}

	vec3_t diff;
	VectorSubtract(end_pt, start_pt, diff);

	const int num_of_intervals = (int)(VectorLength(diff) * 0.5f);
	if (num_of_intervals > 40)
		return false;

	VectorScale(diff, 1.0f / (float)num_of_intervals, diff);
	VectorCopy(start_pt, trail_org); // This rides on the assumption that the normal given is already a unit norm. //TODO: the normal given is NOT a unit norm!

	for (int i = 0; i < num_of_intervals; i++)
	{
		client_entity_t* trail = ClientEntity_new(FX_SPELLHANDS, (int)(self->flags & ~CEF_NO_DRAW), trail_org, NULL, 100);

		trail->r.model = &staff_models[self->classID];
		trail->r.flags = (RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA);
		trail->alpha = 0.6f - (float)self->NoOfAnimFrames * 0.1f;
		trail->AddToView = OffsetLinkedEntityUpdatePlacement;

		if (self->classID == STAFF_TRAIL || self->refPoint == STAFF_TYPE_HELL)
		{
			trail->r.frame = 1;
			trail->d_scale = -0.25f;
			trail->d_alpha = -0.1f;
			trail->color.c = color;
			trail->r.scale = (float)self->NoOfAnimFrames * 0.05f;
			trail->AnimSpeed = 0.2f;
			trail->NoOfAnimFrames = 2;
			trail->Update = StaffElementThink;

			AddEffect(owner, trail);
			StaffElementThink(trail, owner);
		}
		else if (self->classID == STAFF_TRAIL2)
		{
			trail->r.scale = flrand(0.1f, 0.2f);
			trail->d_scale = flrand(-1.0f, -0.5f);
			trail->d_alpha = -2.0f;

			VectorSet(trail->velocity, flrand(-8.0f, 8.0f), flrand(-8.0f, 8.0f), flrand(64.0f, 128.0f)); //mxd. Original logic uses irand() here.

			for (int c = 0; c < 3; c++)
				trail->origin[c] += flrand(-1.0f, 1.0f); //mxd. Original logic uses irand() here.

			AddEffect(owner, trail);
		}
		else if (self->classID == STAFF_TRAIL3)
		{
			trail->r.flags = (RF_TRANSLUCENT | RF_TRANS_ADD);
			trail->r.scale = 0.2f;
			trail->d_scale = -0.35f;
			trail->alpha = 0.75f;
			trail->d_alpha = -4.0f;

			if (owner->current.effects & EF_BLOOD_ENABLED)
			{
				trail->r.color.c = 0x50000018;
			}
			else
			{
				const int c = irand(128, 208);
				COLOUR_SETA(trail->r.color, c, c, irand(236, 255), irand(80, 192)); //mxd. Use macro.
			}

			AddEffect(owner, trail);
		}

		VectorAdd(trail_org, diff, trail_org);
	}

	self->NoOfAnimFrames++;

	return self->NoOfAnimFrames < 6;
}

void FXStaffRemove(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	if (!ReferencesInitialized(owner))
		return;

	client_entity_t* staff_fx = ClientEntity_new(type, flags, origin, NULL, 17);

	if (flags & CEF_FLAG7) // Blue.
		staff_fx->classID = STAFF_TRAIL3;
	else if (flags & CEF_FLAG8) // Flames.
		staff_fx->classID = STAFF_TRAIL2;
	else // Normal.
		staff_fx->classID = STAFF_TRAIL;

	staff_fx->flags |= CEF_NO_DRAW;
	staff_fx->NoOfAnimFrames = 1;
	staff_fx->refPoint = (short)((flags & CEF_FLAG6) ? STAFF_TYPE_HELL : STAFF_TYPE_SWORD);
	staff_fx->Update = StaffRemoveThink;

	AddEffect(owner, staff_fx);
}