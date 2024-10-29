#ifndef CLIENT_ENTITIES_H
#define CLIENT_ENTITIES_H

#ifndef CLIENT_H
#define CLIENT_H
#include "client.h"
#endif

#include "Message.h"
#include "ce_Message.h"

typedef enum ClassID_e
{
	CID_DEBRIS = 0,
	NUM_CLASSIDS
} ClassID_t;

typedef struct CE_ClassStatics_s
{
	CE_MsgReceiver_t msgReceivers[NUM_MESSAGES];
} CE_ClassStatics_t;

typedef qboolean (*UpdateEffect_t)(struct client_entity_s *this, centity_t *owner);

typedef struct client_entity_s
{
	MsgQueue_t	msgQ;
	CE_MessageHandler_t msgHandler;

	int			classID;
	
	entity_t r;							// ends up being sent to the renderer

	struct client_entity_s *next;		// next client entity, if any

	// Note: Since a client_entity's origin (inside the entity e field) is
	// controled soley by it's velocity and acceleration, it doesn't need to
	// have it's orign lerped by the renderer.

	vec3_t origin;						// Used by (non-world) effects that
										// have an owning centity_t.
	
	vec3_t velocity;
	vec3_t acceleration;

	union {
		vec3_t direction;				
		vec3_t startpos;
	};

	union {
		vec3_t up;
		vec3_t right;					// This means you can't have a right AND up, but both usually aren't needed..
		vec3_t endpos;
	};

	float radius;						// used to cull objects, 0 is a point object, values
										// greater than 0 are increasing less likely to be culled

	float d_scale;						// also affects the radius as well as r.scale
										// if d_scale causes r.scale to become <= 0
										// the CEF_DISAPPEARED will be set, and the effect
										// will no longer be added to the render list
										// If at a later time, scale is set positive and
										// CEF_DISAPPEARED is disabled, the radius will have to
										// be reset as well.

	float alpha;						// r.color.a is set directly from this
										// a float is needed so small d_alpha at high frame
										// rate aren't lost completly due to rounding

	float d_alpha;						

	int effectID;

	int flags;

	int startTime;						// time the client_entity_t was
										// created

	int nextThinkTime;					// next time Update will be run
	int updateTime;

	UpdateEffect_t Update;				// run every nextThinkTime, if it
										// returns false the entity will be
										// removed

	UpdateEffect_t AddToView;			// run everytime the entity is added to the view
										// if it returns false, the entity will not be added
										// to the view (it won't be removed though)

	// light stuff
	struct CE_DLight_s *dlight;

	// particle stuff
	paletteRGBA_t color;				// used to set the color of spawned particles in some cases

	struct client_particle_s *p_root;	// root of particle list

	// For models only - NOTE - DO NOT UNION THESE - they are used internally
	short		refMask;					// Current refpoints attached to client entity
	short		refPoint;					// used for entites linked to one of another models 
	
	// anim stuff				// For CE spawners	
	
	union {
		int		LifeTime;
		int		nextEventTime;				// For user timed stuff.
	};

	union {
		float	Scale;
		int		SpawnDelay;
		float	elasticity;
		float	xscale;
	};

	union {
		float	AnimSpeed;			
		float	SpawnData;
		int		lastThinkTime;					// last time updated
		float	yscale;
	};

	union {
		int		NoOfAnimFrames;		
		int		SpawnInfo;
		float	yaw;
	};

	// For spritelines
	float		d_scale2;					// Delta Scale of the second line endpoint.  Needs CEF_USE_SCALE2 flag.

	// sprite line data
	union {
		struct {							// For lines that use interpolation
			vec3_t startpos2;
			vec3_t endpos2;
		};
		struct {
			vec3_t velocity2;				// For lines that need seperate vel and acceleration on the second point
			vec3_t acceleration2;
		};
	};

	void *extra;						// extra whatever
} client_entity_t;

extern client_entity_t *clientEnts;
extern CE_ClassStatics_t classStatics[NUM_CLASSIDS];

client_entity_t *ClientEntity_new(int type, int flags, vec3_t origin, vec3_t direction, int nextThinkTime);
fmnodeinfo_t *FMNodeInfo_new();
void AddEffectToList(client_entity_t **root, client_entity_t *fx);
void RemoveEffectList(client_entity_t **root);
void RemoveOwnedEffectList(centity_t *owner);
void RemoveEffectTypeList(client_entity_t **root, enum FX_Type_e, centity_t *owner);
void RemoveEffectsFromCent(centity_t *cent);
void PrepAddEffectsToView();
int AddEffectsToView(client_entity_t **root, centity_t *owner);
void AddEffect(centity_t *owner, client_entity_t *fx);
int UpdateEffects(client_entity_t **root, centity_t *owner);
qboolean AddEntityToView(entity_t *ent);
void InsertInCircularList(client_entity_t *self);

#define		MAX_ENTRIES_IN_CIRCLE_LIST 70
extern client_entity_t *CircularList[MAX_ENTRIES_IN_CIRCLE_LIST + 1];
extern int			total_circle_entries;

#endif