//
// fx_spellhands.c
//
// Heretic II
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Client Entities.h"
#include "ResourceManager.h"
#include "FX.h"
#include "Vector.h"
#include "Matrix.h"
#include "Random.h"
#include "Utilities.h"
#include "Reference.h"
#include "ce_DLight.h"
#include "Particle.h"
#include "g_playstats.h"

// ************************************************************************************************
// SpellHandsTrails
// ----------------
// ************************************************************************************************

typedef struct
{
	char	*SpriteName;
	float	DeltaForward,
			AnimSpeed,
			Scale,
			DScale;
} trail_t;

static trail_t SpellHandsTrails[NUM_SPELLHANDS]=
{
	{"sprites/spells/flyingfist.sp2",2.0,0.25,0.3,-1.25},
	{"sprites/spells/spellhands_blue.sp2",1.5,0.25,0.4,-0.25},
	{"sprites/spells/spark_ind.sp2",2.0,0.25,1.0,-1.25},
};

// --------------------------------------------------------------

#define	NUM_HAND_MODELS	1

struct model_s *hands_models[NUM_HAND_MODELS];

void PreCacheHands()
{
}

// --------------------------------------------------------------


static qboolean FXSpellHandsThink(struct client_entity_s *Self,centity_t *Owner)
{
	int				part_type;
	vec3_t			Trailend,Trailstart;
	vec3_t			Real_Trailend,Real_Trailstart;
	vec3_t			TrailDelta;
	float		   	TrailLength;

	client_particle_t	*ce;
	paletteRGBA_t		color;
	matrix3_t			rotation;

	// If we've timed out, stop the effect (allow for fading)
	if (Self->LifeTime > 0)
	{
		if (Self->LifeTime < fxi.cl->time)
		{	// Lifetime is up
			Self->Update=RemoveSelfAI;
			Self->updateTime = fxi.cl->time + 500;
			return true;
		}
	}
	else 
	{	// If we're not on a time limit, check the EF flag
		if (!(Owner->current.effects & EF_TRAILS_ENABLED))
		{
			Self->Update=RemoveSelfAI;
			Self->updateTime = fxi.cl->time + 500;
			return true;
		}
	}

	// This tells if we are wasting our time, because the reference points are culled.
	if (!RefPointsValid(Owner))
		return true;

	// Calculate the end position of the trail.
	VectorCopy(Owner->referenceInfo->references[Self->refPoint].placement.origin,Trailend);

	// we now have trail start and trail end in
	VectorCopy(Self->origin,Trailstart);

	// update where trail ends
	VectorCopy(Trailend,Self->origin);

	// Allow us adequate time to set up valid 'old' data because the reference points lag behind by
	// a frame.
	if((Self->AnimSpeed+=1.0)<2.0)
		return(true);

	// Create a rotation matrix
	Matrix3FromAngles(Owner->lerp_angles, rotation);
	// make the trail start and end a trail in real space
	Matrix3MultByVec3(rotation, Trailstart, Real_Trailstart);
	Matrix3MultByVec3(rotation, Trailend, Real_Trailend);

	// figure out the differences between them
	VectorSubtract(Real_Trailend, Real_Trailstart, TrailDelta);

	// set the trail length
	TrailLength = GetScaledCount(4, 0.75);

	// scale that difference by the number of particles we are going to draw
	Vec3ScaleAssign(1.0/TrailLength, TrailDelta);

	// decide which particle type to use
	if (Self->SpawnInfo == 0)
		part_type = PART_16x16_SPARK_R;
	else
	if (Self->SpawnInfo == 1)
		part_type = PART_16x16_SPARK_B;
	else
		part_type = PART_16x16_SPARK_I;

	// ensure the particles are completely drawn
   	color.c = 0xffffffff;

	// Now draw the trail.
	while(TrailLength-->0.0f)
	{
		ce = ClientParticle_new(part_type, color, 400);
		VectorCopy(Real_Trailstart,ce->origin);
		ce->scale = 8.0F;
		ce->acceleration[2] = 0.0f; 
		VectorSet(ce->velocity, flrand(-8.0, 8.0), flrand(-8.0, 8.0), flrand(-8.0, 8.0));

		AddParticleToList(Self, ce);

		VectorAdd(Real_Trailstart,TrailDelta,Real_Trailstart);
	}

	return(true);
}

// ************************************************************************************************
// FXSpellHands
// ------------
// ************************************************************************************************

void FXSpellHands(centity_t *Owner,int Type,int Flags,vec3_t Origin)
{
	short			Refpoints;
	client_entity_t	*Trail;
	char			lifetime;
	int				I;
	int				cast_speed;

	fxi.GetEffect(Owner, Flags, clientEffectSpawners[FX_SPELLHANDS].formatString, &lifetime);

	if (Flags & CEF_FLAG6)
		Refpoints=(1 << CORVUS_LEFTHAND) | (1 << CORVUS_RIGHTHAND);
	else
		Refpoints=(1 << CORVUS_RIGHTHAND);

	// Add a fiery trail effect to the player's hands / feet etc.

	if (r_detail->value < DETAIL_NORMAL)
		cast_speed = 75;
	else
		cast_speed = 50;

	for(I=0;I<16;I++)
	{
		if(!(Refpoints & (1 << I)))
			continue;

		Trail=ClientEntity_new(Type,Flags|CEF_NO_DRAW|CEF_ADDITIVE_PARTS,Origin,0,cast_speed);
		Trail->r.flags=RF_TRANSLUCENT|RF_TRANS_ADD|RF_TRANS_ADD_ALPHA;
		Trail->Update=FXSpellHandsThink;
		Trail->SpawnInfo= ((Flags&(CEF_FLAG7|CEF_FLAG8)) >> 6);
		Trail->AddToView = LinkedEntityUpdatePlacement;			
		
		if (lifetime > 0)
			Trail->LifeTime = fxi.cl->time + (lifetime * 100);
		else
			Trail->LifeTime = -1;

		Trail->refPoint = I;
		
		// Hack: used as a counter.
		Trail->AnimSpeed=0.0;

		AddEffect(Owner,Trail);
	}
}

