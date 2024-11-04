#ifndef G_PHYSICSINFO_H
#define G_PHYSICSINFO_H

#if 0
#include "q_Typedef.h"


typedef struct PhysicsInfo_s
{
	int					type;

	int					flags;

	float				*origin;			// pointer to owning entities origin vector

	vec3_t				velocity;			// linear velocity
	vec3_t				avelocity;			// angular velocity

	int					mass;
	float				gravity;		// gravity multiplier; defaults to 1.0
	float				friction;		// friction multiplier; defaults to 1.0
	// Used to determine whether something will stop, slide, or bounce on impact
	float				elasticity;

	// Used for determining effects of liquids in the environment.
	int					watertype;		// Used to indicate current liquid actor is in.
	int					waterlevel;		// Used by monsters and players and grenades.


	// called when self is the collider in a collision
	void				(*isBlocked)(edict_t *self, struct trace_s *trace);

	// called when self is the collidee in a collision
	void				(*isBlocking)(edict_t *self, struct trace_s *trace);

	edict_t				*groundentity;			// entity serving as ground
	int					groundentity_linkcount;	// if self and groundentity's don't match, groundentity should be cleared
	vec3_t				groundNormal;			// normal of the ground

	edict_t				*blockingEntity;			// entity serving as ground
	int					blockingEntity_linkcount;	// if self and blockingEntity's don't match, blockingEntity should be
													// cleared
	vec3_t				blockingNormal;				// normal of the blocking surface

	struct BoundingForm_s	*intentBoundingForm;// if PF_RESIZE is set, then physics will attempt to change
												// the ents bounding form to the new one indicated
												// If it was succesfully resized, the PF_RESIZE is turned off
												// otherwise it will remain on.
} PhysicsInfo_t;

extern PhysicsInfo_t *PhysicsInfo_new();
extern void PhysicsInfo_delete(PhysicsInfo_t *toDelete);
#endif
#endif