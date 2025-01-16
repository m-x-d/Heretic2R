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

enum HighPriestessStaff_e
{
	HP_STAFF_INIT,
	HP_STAFF_TRAIL,
};

static qboolean HPStaffTrailThink(const struct client_entity_s* self, centity_t* owner)
{
	const centity_t* actual_owner = (centity_t*)self->extra;

	// This tells if we are wasting our time, because the reference points are culled.
	if (!RefPointsValid(actual_owner))
		return true;

	client_entity_t* trail = ClientEntity_new(FX_HP_STAFF, CEF_DONT_LINK, self->r.origin, NULL, 2000);

	trail->radius = 500.0f;
	trail->r.flags = RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
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

/*-----------------------------------------------
	PriestessEffectStayAlive
-----------------------------------------------*/

qboolean PriestessEffectStayAlive(struct client_entity_s* self, centity_t* owner)
{
	return true;
}

/*-----------------------------------------------
	PriestessFirstSeenInit
-----------------------------------------------*/

qboolean PriestessFirstSeenInit(struct client_entity_s* self, centity_t* owner)
{
	self->refMask |= PRIESTESS_MASK;

	EnableRefPoints(owner->referenceInfo, self->refMask);

	self->AddToView = NULL;
	self->Update = HPStaffTrailThink;

	HPStaffTrailThink(self, owner);

	return true;
}

/*-----------------------------------------------
	FXHPStaff
-----------------------------------------------*/

void FXHPStaff(centity_t* Owner, int Type, int Flags, vec3_t Origin)
{
	client_entity_t* self;
	short				entID;
	byte				type;

	fxi.GetEffect(Owner, Flags, clientEffectSpawners[FX_HP_STAFF].formatString, &type, &entID);

	switch (type)
	{
		case HP_STAFF_INIT:

			self = ClientEntity_new(Type, Flags | CEF_NO_DRAW | CEF_ABSOLUTE_PARTS, Origin, NULL, 17);

			self->Update = NULL;
			self->AddToView = PriestessFirstSeenInit;
			self->Update = PriestessEffectStayAlive;
			self->extra = (void*)(&fxi.server_entities[entID]);

			AddEffect(Owner, self);

			PriestessEffectStayAlive(self, Owner);
			break;

		case HP_STAFF_TRAIL:

			//Add trailing code here
			break;

		default:

			//Effect was passed with invalid effect modifier type
			assert(0);
			break;
	}
}