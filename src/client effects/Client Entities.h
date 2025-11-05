//
// Client Entities.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "client.h"
#include "Message.h"
#include "ce_Message.h"
#include "FX.h"

typedef enum CE_ClassID_e
{
	CID_DEBRIS = 0,
	NUM_CLASSIDS
} CE_ClassID_t;

typedef struct CE_ClassStatics_s
{
	CE_MsgReceiver_t msgReceivers[NUM_MESSAGES];
} CE_ClassStatics_t;

typedef qboolean(*UpdateEffect_t)(struct client_entity_s* this, centity_t* owner);

typedef struct client_entity_s
{
	MsgQueue_t msgQ;
	CE_MessageHandler_t msgHandler;

	int classID;

	entity_t r; // Ends up being sent to the renderer.

	struct client_entity_s* next; // Next client entity, if any.

	// Note: since a client_entity's origin (inside the entity e field) is controlled solely by it's velocity and acceleration,
	// it doesn't need to have it's origin lerped by the renderer.

	vec3_t origin; // Used by (non-world) effects that have an owning centity_t.
	vec3_t velocity;
	vec3_t acceleration;

	union
	{
		vec3_t direction;
		vec3_t startpos;
	};

	union
	{
		vec3_t up;
		vec3_t right; // This means you can't have a right AND up, but both usually aren't needed..
		vec3_t endpos;
	};

	float radius; // Used to cull objects, 0 is a point object, values greater than 0 are increasing less likely to be culled.

	// Also affects the radius as well as r.scale. 
	// If d_scale causes r.scale to become <= 0, the CEF_DISAPPEARED will be set, and the effect will no longer be added to the render list.
	// If at a later time, scale is set positive and CEF_DISAPPEARED is disabled, the radius will have to be reset as well.
	float d_scale; 

	// r.color.a is set directly from this.
	// A float is needed, so small d_alpha at high frame rate aren't lost completely due to rounding.
	float alpha; 
	float d_alpha;
	int effectID;
	int flags;

	int startTime; // Time the client_entity_t was created.

	int nextThinkTime; // Next time Update will be run.
	int updateTime;

	UpdateEffect_t Update; // Run every nextThinkTime. If it returns false the entity will be removed.

	// Run every time the entity is added to the view.
	// If it returns false, the entity will not be added to the view (it won't be removed though). // Not implemented in original logic --mxd.
	UpdateEffect_t AddToView;

	// Light stuff.
	struct CE_DLight_s* dlight;

	// Particle stuff.
	paletteRGBA_t color; // Used to set the color of spawned particles in some cases.
	struct client_particle_s* p_root; // Root of particle list.
	short refPoint; // Used for entities linked to one of another models.

	//mxd. Frame interpolation stuff (CEF_FRAME_LERP).
	float framelerp_scale;
	int framelerp_time;

	//mxd. Looks like we'll need some extra debris props...
	int debris_last_bounce_time; // Last time debris bounced in Debris_Collision().
	int debris_last_trail_update_time;
	float debris_avelocity[2]; // Angular velocity (only PITCH and YAW are used).

	//mxd. Extra bloodsplat props...
	int bloodsplat_max_particles;
	int bloodsplat_cur_particles;
	struct client_entity_s* floor_bloodsplat;

	// Animation stuff for CE spawners.

	union
	{
		int LifeTime;
		int nextEventTime; // For user-timed stuff.
		int tome_fadein_end_time; //mxd
	};

	union
	{
		float Scale;
		int SpawnDelay;
		float elasticity;
		float xscale;
		int tome_fadeout_end_time; //mxd
	};

	union
	{
		float AnimSpeed;
		float SpawnData;
		int lastThinkTime; // Last time updated.
		float yscale;
	};

	union
	{
		int NoOfAnimFrames;
		int SpawnInfo;
		float yaw;
	};

	// For spritelines.
	float d_scale2; // Delta Scale of the second line endpoint. Needs CEF_USE_SCALE2 flag.

	// Sprite line data.
	union
	{
		// For lines that use interpolation.
		struct
		{
			vec3_t startpos2;
			vec3_t endpos2;
		};

		// For lines that need separate velocity and acceleration on the second point.
		struct
		{
			vec3_t velocity2;
			vec3_t acceleration2;
		};
	};

	void* extra; // Extra whatever.
} client_entity_t;

extern client_entity_t* clientEnts;
extern CE_ClassStatics_t ce_class_statics[NUM_CLASSIDS];

extern void InitEntityMngr(void); //mxd
extern void ReleaseEntityMngr(void); //mxd
extern client_entity_t* ClientEntity_new(int type, int flags, const vec3_t origin, const vec3_t direction, int next_think_time);

extern void InitFMNodeInfoMngr(void); //mxd
extern void ReleaseFMNodeInfoMngr(void); //mxd
extern fmnodeinfo_t* FMNodeInfo_new(void);

extern void AddEffectToList(client_entity_t** root, client_entity_t* fx);
extern void RemoveEffectFromList(client_entity_t** root, const centity_t* owner); //mxd
extern void RemoveEffectList(client_entity_t** root);
extern void RemoveOwnedEffectList(centity_t* owner);
extern void RemoveEffectTypeList(client_entity_t** root, FX_Type_t fx, const centity_t* owner);
extern void PrepAddEffectsToView(void);
extern int AddEffectsToView(client_entity_t** root, centity_t* owner);
extern void AddEffect(centity_t* owner, client_entity_t* fx);
extern int UpdateEffects(client_entity_t** root, centity_t* owner);
extern qboolean AddEntityToView(entity_t* ent);