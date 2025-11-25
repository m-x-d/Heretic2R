//
// fx_rope.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "g_playstats.h"
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

static qboolean RopeCheckToHide(struct client_entity_s* self, centity_t* owner)
{
	// Get the entity.
	const centity_t* grab = &fxi.server_entities[self->LifeTime];

	// If the flag isn't set, then we're supposed to disappear.
	if (!(grab->current.effects & EF_ALTCLIENTFX) && self->SpawnInfo < fx_time)
	{
		// Clear this out and kill the effect.
		self->AddToView = NULL;
		return false;
	}

	return true;
}

#pragma region ========================== ATTACHED ROPE SEGMENTS ==========================

static qboolean RopeTopDrawAttached(struct client_entity_s* self, centity_t* owner)
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

static qboolean RopeMiddleDrawAttached(struct client_entity_s* self, centity_t* owner)
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

static qboolean RopeBottomDrawAttached(struct client_entity_s* self, centity_t* owner)
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
	const float new_time = (float)fx_time / 100.0f;

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
	self->lastThinkTime = fx_time;

	return true;
}

#pragma endregion

#pragma region ========================== UNATTACHED ROPE SEGMENTS ==========================

static qboolean RopeTopDraw(struct client_entity_s* self, centity_t* owner)
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
	const float new_time = (float)fx_time / 100.0f;

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
	self->lastThinkTime = fx_time;

	return true;
}

#pragma endregion

void FXRope(centity_t* owner, int type, const int flags, vec3_t origin)
{
	short grab_id;
	short end_id;
	byte model_type;
	vec3_t top;
	vec3_t grab_pos;
	vec3_t end_pos;
	fxi.GetEffect(owner, CEF_BROADCAST, clientEffectSpawners[FX_ROPE].formatString, &grab_id, &end_id, &model_type, &top, &grab_pos, &end_pos);

	// This is set if the effect should be attached to something.
	const qboolean is_attached = (flags & CEF_FLAG6);
	const float radius = (fabsf(grab_pos[2]) + fabsf(end_pos[2])) * 2.0f;
	const int r_flags = (R_DETAIL >= DETAIL_HIGH ? RF_LM_COLOR : 0); //mxd

	// Create the rope piece that hangs from the top to the end of the rope, while the player is NOT on it.
	if (!is_attached)
	{
		client_entity_t* rope = ClientEntity_new(FX_ROPE, 0, origin, NULL, 0); //mxd. next_think_time 17 in original logic. Set to 0 to update each frame.

		rope->radius = radius;
		rope->r.model = &rope_models[model_type];
		rope->r.spriteType = SPRITE_LINE;
		rope->r.flags = r_flags; //mxd
		rope->r.scale = 3.0f;
		rope->lastThinkTime = fx_time;
		
		// End of the rope.
		rope->extra = (void*)&fxi.server_entities[end_id];
		rope->LifeTime = grab_id;

		// Set up the end vector start and endpos, for linear interpolation.
		VectorCopy(fxi.server_entities[end_id].current.origin, rope->startpos);
		VectorCopy(rope->startpos, rope->endpos);
		VectorCopy(top, rope->direction);

		rope->AddToView = RopeTopDraw;
		rope->Update = RopeTopDraw;

		AddEffect(NULL, rope);
	}
	else // Create the fake rope that is attached to the player.
	{
		// Top of the rope.
		client_entity_t* rope_top = ClientEntity_new(FX_ROPE, CEF_OWNERS_ORIGIN, origin, NULL, 1000);

		rope_top->radius = radius;
		rope_top->r.model = &rope_models[model_type];
		rope_top->r.spriteType = SPRITE_LINE;
		rope_top->r.flags = r_flags; //mxd
		rope_top->r.scale = 3.0f;
		rope_top->SpawnInfo = fx_time + 1000;
		rope_top->LifeTime = grab_id;

		VectorCopy(top, rope_top->direction);
		VectorCopy(top, rope_top->r.startpos);

		vec3_t vec;
		VectorSubtract(owner->origin, end_pos, vec);
		rope_top->r.tile = VectorLength(vec);

		rope_top->AddToView = RopeTopDrawAttached;
		rope_top->Update = RopeCheckToHide;

		AddEffect(owner, rope_top);
		RopeTopDrawAttached(rope_top, owner);

		// Middle of the rope.
		client_entity_t* rope_mid = ClientEntity_new(FX_ROPE, CEF_OWNERS_ORIGIN, origin, NULL, 1000);

		rope_mid->radius = radius;
		rope_mid->r.model = &rope_models[model_type];
		rope_mid->r.spriteType = SPRITE_LINE;
		rope_mid->r.flags = r_flags; //mxd
		rope_mid->r.scale = 3.0f;
		rope_mid->r.tile = 1.0f;
		rope_mid->LifeTime = grab_id;
		rope_mid->SpawnInfo = fx_time + 1000;

		rope_mid->AddToView = RopeMiddleDrawAttached;
		rope_mid->Update = RopeCheckToHide;

		AddEffect(owner, rope_mid);
		RopeMiddleDrawAttached(rope_mid, owner);

		// Bottom of the rope.
		for (int i = 0; i < (int)ROPE_BOTTOM_SEGMENTS; i++)
		{
			client_entity_t* rope_bottom = ClientEntity_new(FX_ROPE, CEF_OWNERS_ORIGIN, origin, NULL, 1000);

			rope_bottom->radius = radius;
			rope_bottom->r.model = &rope_models[model_type];
			rope_bottom->r.spriteType = SPRITE_LINE;
			rope_bottom->r.flags = r_flags; //mxd
			rope_bottom->r.scale = 3.0f;
			rope_bottom->lastThinkTime = fx_time;

			// End of the rope_bottom.
			rope_bottom->extra = (void*)&fxi.server_entities[end_id];
			rope_bottom->LifeTime = grab_id;

			// Set up the end vector start and endpos, for linear interpolation.
			VectorCopy(end_pos, rope_bottom->startpos2);
			VectorCopy(rope_bottom->startpos2, rope_bottom->endpos2);

			// The segment number of this piece.
			rope_bottom->SpawnInfo = fx_time + 1000;
			rope_bottom->SpawnDelay = i;

			VectorCopy(top, rope_bottom->direction);

			rope_bottom->AddToView = RopeBottomDrawAttached;
			rope_bottom->Update = RopeCheckToHide;

			AddEffect(owner, rope_bottom);
			RopeBottomDrawAttached(rope_bottom, owner);
		}
	}
}