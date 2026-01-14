//
// fx_HighPriestessStaff.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Matrix.h"
#include "Reference.h"
#include "Utilities.h"
#include "Vector.h"

static struct model_s* hpstaff_model;

void PreCacheHPStaff(void)
{
	hpstaff_model = fxi.RegisterModel("sprites/fx/hpproj1_2.sp2"); // Staff Trail.
}

static qboolean HPStaffTrailUpdate(client_entity_t* self, centity_t* owner) //mxd. Named 'HPStaffTrailThink' in original logic.
{
	const centity_t* actual_owner = (centity_t*)self->extra;

	// This tells if we are wasting our time, because the reference points are culled.
	if (!RefPointsValid(actual_owner))
		return true;

	client_entity_t* trail = ClientEntity_new(FX_HP_STAFF, CEF_DONT_LINK, self->r.origin, NULL, 2000);

	trail->radius = 500.0f;
	trail->r.flags = (RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA);
	trail->r.model = &hpstaff_model;
	trail->r.scale = 0.75f;
	trail->alpha = 0.5f;
	trail->d_alpha = -2.0f;
	trail->d_scale = -2.0f;

	matrix3_t rotation;
	Matrix3FromAngles(actual_owner->lerp_angles, rotation);
	Matrix3MultByVec3(rotation, actual_owner->referenceInfo->references[PRIESTESS_STAFF].placement.origin, trail->r.origin);

	VectorAdd(actual_owner->origin, trail->r.origin, trail->r.origin);
	trail->r.origin[2] -= 36.0f;

	AddEffect(NULL, trail);

	return true;
}

static qboolean PriestessFirstSeenInit(client_entity_t* self, centity_t* owner)
{
	self->AddToView = NULL;
	self->Update = HPStaffTrailUpdate;

	HPStaffTrailUpdate(self, owner);

	return true;
}

void FXHPStaff(centity_t* owner, const int type, int flags, vec3_t origin)
{
	byte fx_type;
	short ent_id;
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_HP_STAFF].formatString, &fx_type, &ent_id);

	switch (fx_type)
	{
		case HP_STAFF_INIT:
			flags |= (CEF_NO_DRAW | CEF_ABSOLUTE_PARTS);
			client_entity_t* self = ClientEntity_new(type, flags, origin, NULL, 17);

			self->AddToView = PriestessFirstSeenInit;
			self->Update = KeepSelfAI;
			self->extra = (void*)(&fxi.server_entities[ent_id]);

			AddEffect(owner, self);
			break;

		case HP_STAFF_TRAIL: // Add trailing code here.
			break;

		default: // Effect was passed with invalid effect modifier type.
			assert(0);
			break;
	}
}