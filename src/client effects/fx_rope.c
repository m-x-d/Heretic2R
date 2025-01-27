//
// fx_rope.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Vector.h"
#include "q_Sprite.h"

#define ROPE_SEGMENT_LENGTH		64.0f
#define ROPE_BOTTOM_SEGMENTS	4.0f

static struct model_s* rope_models[4];

void PreCacheRope(void)
{
	rope_models[0] = fxi.RegisterModel("sprites/fx/segment_rope.sp2");
	rope_models[1] = fxi.RegisterModel("sprites/fx/segment_chain.sp2");
	rope_models[2] = fxi.RegisterModel("sprites/fx/segment_vine.sp2");
	rope_models[3] = fxi.RegisterModel("sprites/fx/segment_tendril.sp2");
}

static qboolean RopeCheckToHide(struct client_entity_s* self, const centity_t* owner)
{
	// Get the entity.
	const centity_t* grab = &fxi.server_entities[self->LifeTime];

	// If the flag isn't set, then we're supposed to disappear.
	if (!(grab->current.effects & EF_ALTCLIENTFX) && self->SpawnInfo < fxi.cl->time)
	{
		// Clear this out and kill the effect.
		self->AddToView = NULL;
		return false;
	}

	return true;
}

#pragma region ========================== ATTACHED ROPE SEGMENTS ==========================

static qboolean FXRopeTopDrawAttached(struct client_entity_s* self, const centity_t* owner)
{
	if (!RopeCheckToHide(self, owner))
	{
		self->flags |= CEF_NO_DRAW;
		return false;
	}

	// Get our tile rate.
	vec3_t vec;
	VectorSubtract(self->direction, owner->origin, vec);
	self->r.tile = VectorLength(vec) / ROPE_SEGMENT_LENGTH;

	// This places us at the player's top most hand.
	VectorSet(self->r.endpos, owner->origin[0], owner->origin[1], owner->origin[2] + 32.0f);

	return true;
}

static qboolean FXRopeMiddleDrawAttached(struct client_entity_s* self, const centity_t* owner)
{
	if (!RopeCheckToHide(self, owner))
	{
		self->flags |= CEF_NO_DRAW;
		return false;
	}

	// These magic numbers place the rope at his hands.
	VectorSet(self->r.startpos, owner->origin[0], owner->origin[1], owner->origin[2] + 32.0f);
	VectorSet(self->r.endpos, owner->origin[0], owner->origin[1], owner->origin[2] + 8.0f);

	// Get the tile rate.
	self->r.tile = 24.0f / ROPE_SEGMENT_LENGTH;

	// Offset the sprite's tile so that there's no visible gap between the top section and the grab section (slight imprecision).
	self->r.tileoffset = fmodf(self->direction[2] - owner->origin[2], ROPE_SEGMENT_LENGTH) / ROPE_SEGMENT_LENGTH;

	return true;
}

static qboolean FXRopeBottomDrawAttached(struct client_entity_s* self, const centity_t* owner)
{
	if (!RopeCheckToHide(self, owner))
	{
		self->flags |= CEF_NO_DRAW;
		return false;
	}

	// Setup our entity to base positions on.
	const centity_t* end = (centity_t*)self->extra;

	// Setup the start position for the line.
	VectorSet(self->r.startpos, owner->origin[0], owner->origin[1], owner->origin[2] + 8.0f);

	// Find the lerp factor by determining where in the 1/10 of a second this frame lies.
	const float old_time = (float)self->lastThinkTime / 100.0f;
	const float new_time = (float)fxi.cl->time / 100.0f;

	if ((int)old_time < (int)new_time)
	{
		// Adjust the start and endpos for a new interpolation interval.		
		VectorCopy(self->endpos2, self->startpos2);
		VectorCopy(end->current.origin, self->endpos2);
	}

	// Get the lerp increment.
	const float lerp = new_time - floorf(new_time); //mxd. (int) -> floorf().

	// Find the difference in position for lerping.
	vec3_t diff_pos2;
	VectorSubtract(self->endpos2, self->startpos2, diff_pos2);

	// Now apply the lerp value to get the new endpos.
	VectorMA(self->startpos2, lerp, diff_pos2, self->r.endpos);

	// Setup the vector to the current position to the goal position.
	vec3_t vec;
	VectorSubtract(self->r.endpos, self->r.startpos, vec);
	const float dist = VectorNormalize(vec);

	// Find the number of segments, and their lengths
	const float c_length = dist / ROPE_BOTTOM_SEGMENTS; //FIXME: Find the actual length with curving
	self->r.tile = c_length / ROPE_SEGMENT_LENGTH;

	// Setup the endpos.
	VectorSet(self->r.endpos, owner->origin[0], owner->origin[1], owner->origin[2] + 8.0f);

	// Find the vector pointing down through the upper portion of the rope from the connect point of the rope to the player.
	vec3_t c_down;
	VectorSubtract(self->direction, owner->origin, c_down);
	VectorNormalize(c_down);

	// Set through the curve till we find this segment's position in it.
	for (int i = 0; i <= self->SpawnDelay; i++)
	{
		// Where the last segment ended is where the new one begins.
		VectorCopy(self->r.endpos, self->r.startpos);

		// Find the angle increment for this segment.
		vec3_t c_vec;
		VectorScale(c_down, (float)i / (ROPE_BOTTOM_SEGMENTS + 1.0f) / 2.0f, c_vec);

		// Get the new vector.
		VectorAdd(c_vec, vec, c_vec);

		// Find where this segment will end.
		VectorMA(self->r.startpos, c_length, c_vec, self->r.endpos);
	}

	// Store for lerping.
	self->lastThinkTime = fxi.cl->time;

	return true;
}

#pragma endregion

#pragma region ========================== UNATTACHED ROPE SEGMENTS ==========================

static qboolean FXRopeTopDraw(struct client_entity_s* self, centity_t* owner)
{
	const centity_t* end = (centity_t*)self->extra;
	const centity_t* grab = &fxi.server_entities[self->LifeTime];

	// If the rope entity has requested it, hide these segments.
	if (grab->current.effects & EF_ALTCLIENTFX)
	{
		self->flags |= CEF_NO_DRAW;

		VectorCopy(self->endpos, self->startpos);
		VectorCopy(end->current.origin, self->endpos);

		return true;
	}

	// Make sure we're visible.
	self->flags &= ~CEF_NO_DRAW;

	VectorCopy(self->origin, self->r.startpos);
	VectorCopy(end->current.origin, self->r.endpos);

	// Find the linear interpolation factor by determining where in the 1/10 of a second this frame lies.
	const float old_time = (float)self->lastThinkTime / 100.0f;
	const float new_time = (float)fxi.cl->time / 100.0f;

	if ((int)old_time < (int)new_time)
	{
		// We have crossed the boundary between 1/10 second interval. Adjust the start and endpos for a new interpolation interval.
		VectorCopy(self->endpos, self->startpos);
		VectorCopy(end->current.origin, self->endpos);
	}

	const float lerp = new_time - floorf(new_time); //mxd. (int) -> floorf().

	// Find the difference in position for lerping.
	vec3_t diff_pos;
	VectorSubtract(self->endpos, self->startpos, diff_pos);

	// Now apply the lerp value to get the new endpos.
	VectorMA(self->startpos, lerp, diff_pos, self->r.endpos);

	// Get our tile rate.
	VectorSubtract(self->origin, self->r.endpos, diff_pos);
	self->r.tile = VectorLength(diff_pos) / ROPE_SEGMENT_LENGTH;

	// Store for lerping.
	self->lastThinkTime = fxi.cl->time;

	return true;
}

#pragma endregion

void FXRope(centity_t *owner,int Type,int Flags,vec3_t Origin)
{
	client_entity_t	*rope, *ropeb, *ropem;
	centity_t		*grab, *end;
	qboolean		attached;
	vec3_t			top, vec, grab_pos, end_pos;
	float			radius;
	short			grab_id, end_id;
	byte			model_type;
	int				i;

	fxi.GetEffect(owner, CEF_BROADCAST, clientEffectSpawners[FX_ROPE].formatString, &grab_id, &end_id, &model_type, &top, &grab_pos, &end_pos);

	//This is set if the effect should be attached to something
	attached = (Flags & CEF_FLAG6);

	//Setup the entities
	end  = &fxi.server_entities[end_id];
	grab = &fxi.server_entities[grab_id];

	//Create the rope piece that hangs from the top to the end of the rope, while the player is NOT on it	
    
	radius = (Q_fabs(grab_pos[2]) + Q_fabs(end_pos[2])) * 2;

	if (!attached)
	{
		rope=ClientEntity_new(  FX_ROPE,
								0,
								Origin,
								NULL,
								17);

		rope->r.model = rope_models + model_type;
		
		rope->r.scale = 3;
		rope->radius = radius;

		rope->r.spriteType = SPRITE_LINE;
		rope->Update = FXRopeTopDraw;
		rope->AddToView = FXRopeTopDraw;
		
		//End of the rope
		rope->extra = (void *) &fxi.server_entities[end_id];
		rope->LifeTime = grab_id;

		// Set up the end vector start and endpos, for linear interpolation.
		VectorCopy(fxi.server_entities[end_id].current.origin, rope->startpos);
		VectorCopy(rope->startpos, rope->endpos);

		rope->lastThinkTime = fxi.cl->time;

		VectorCopy(top, rope->direction);
		
		AddEffect(NULL, rope); 
	}
	else
	{
		//Create the fake rope that is attached to the player

		//--------------------
		//
		//	Top of the rope

		rope=ClientEntity_new(  FX_ROPE,
								CEF_OWNERS_ORIGIN,
								Origin,
								NULL,
								1000);

		rope->r.model = rope_models + model_type;
		
		VectorCopy(top, rope->direction);	

		rope->r.scale = 3;
		rope->radius = radius;
		VectorCopy(top, rope->r.startpos);

		rope->r.spriteType = SPRITE_LINE;
		rope->AddToView = FXRopeTopDrawAttached;
		rope->Update = RopeCheckToHide;
		
		VectorSubtract(owner->origin, end_pos, vec);
		rope->r.tile = VectorLength(vec);
		
		rope->SpawnInfo = fxi.cl->time + 1000;

		rope->LifeTime = grab_id;

		AddEffect(owner, rope); 

		FXRopeTopDrawAttached(rope, owner);

		//--------------------
		//
		//	Middle of the rope

		ropem=ClientEntity_new( FX_ROPE,
								CEF_OWNERS_ORIGIN,
								Origin,
								NULL,
								1000);

		ropem->r.model = rope_models + model_type;
		
		ropem->r.scale = 3;
		ropem->radius = radius;

		ropem->r.tile = 1;
		ropem->r.spriteType = SPRITE_LINE;
		ropem->AddToView = FXRopeMiddleDrawAttached;
		ropem->Update = RopeCheckToHide;
		ropem->LifeTime = grab_id;

		ropem->SpawnInfo = fxi.cl->time + 1000;

		AddEffect(owner, ropem); 

		FXRopeMiddleDrawAttached(ropem, owner);

		//--------------------
		//
		//	Bottom of the rope

		for (i=0; i<(int)ROPE_BOTTOM_SEGMENTS; i++)
		{
			ropeb=ClientEntity_new( FX_ROPE,
									CEF_OWNERS_ORIGIN,
									Origin,
									NULL,
									1000);

			ropeb->r.model = rope_models + model_type;
			
			ropeb->r.scale = 3;
			ropeb->radius = radius;

			ropeb->r.spriteType = SPRITE_LINE;
			ropeb->Update = RopeCheckToHide;
			ropeb->AddToView = FXRopeBottomDrawAttached;
			
			//End of the ropeb
			ropeb->extra = (void *) &fxi.server_entities[end_id];
			ropeb->LifeTime = grab_id;

			// Set up the end vector start and endpos, for linear interpolation.
			VectorCopy(end_pos, ropeb->startpos2);
			VectorCopy(ropeb->startpos2, ropeb->endpos2);

			//The segment number of this piece
			ropeb->SpawnInfo = fxi.cl->time + 1000;
			ropeb->SpawnDelay = (int) i;

			ropeb->lastThinkTime = fxi.cl->time;

			VectorCopy(top, ropeb->direction);	
			
			AddEffect(owner, ropeb); 
	
			FXRopeBottomDrawAttached(ropeb, owner);
		}
	}
}
