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

#define	NUM_STAFF_MODELS	1

static struct model_s* hpstaff_models[NUM_STAFF_MODELS];

void PreCacheHPStaff()
{
	//Staff Trail
	hpstaff_models[0] = fxi.RegisterModel("sprites/fx/hpproj1_2.sp2");
}

/*

	High Priestess Staff Effects

*/

enum
{
	HP_STAFF_INIT,
	HP_STAFF_TRAIL,
} HighPriestessStaff_e;

/*-----------------------------------------------
	FXHPStaff
-----------------------------------------------*/

qboolean HPStaffTrailThink(struct client_entity_s* self, centity_t* owner)
{
	client_entity_t* Trail;
	matrix3_t		RotationMatrix;

	// This tells if we are wasting our time, because the reference points are culled.
	if (!RefPointsValid((centity_t*)(self->extra)))
		return true;

	Trail = ClientEntity_new(FX_HP_STAFF, CEF_DONT_LINK, self->r.origin, NULL, 2000);

	Trail->r.flags |= RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;

	Trail->r.model = hpstaff_models;
	Trail->r.scale = 0.75;
	Trail->alpha = 0.5;
	Trail->d_alpha = -2.0;
	Trail->d_scale = -2.0;
	Trail->radius = 500;

	Matrix3FromAngles(((centity_t*)(self->extra))->lerp_angles, RotationMatrix);

	Matrix3MultByVec3(RotationMatrix,
		((centity_t*)(self->extra))->referenceInfo->references[PRIESTESS_STAFF].placement.origin,
		Trail->r.origin);

	VectorAdd(((centity_t*)(self->extra))->origin, Trail->r.origin, Trail->r.origin);
	Trail->r.origin[2] -= 36;

	AddEffect(NULL, Trail);

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