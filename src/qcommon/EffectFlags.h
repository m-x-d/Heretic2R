//
// EffectFlags.h
//
// Copyright 1998 Raven Software
//

#pragma once

// EF_XXX
// Effects (held by entity_state_t.effects) are things that are handled on the client side.
// (lights, particles, frame animations) and happen constantly on the given entity.
// An entity that has effects will be sent to the client even if it has a zero index model.

//#define EF_ROTATE				0x00000001	// Rotate the entity's model (used for bonus items). //mxd. Unused.
//#define EF_CL_PHYS_ALWAYS_BOUNCE 0x00000002 //mxd. Unused.
#define EF_JOINTED				0x00000004
#define EF_SWAPFRAME			0x00000008
#define EF_DISABLE_ALL_CFX		0x00000010	// All effects linked to this entity will no longer be rendered, but will continue to think.

#define EF_PLAYER				0x00000020	// Safe flag, never changes. Is a player/client.

#define EF_NODRAW_ALWAYS_SEND	0x00000040	// Prevents invisible (i.e. modelindex = 0) entities from being culled by the server
											// when it builds frames to send to the client.

#define EF_MARCUS_FLAG1			0x00000080	// Used for toggling various client effects.
#define EF_CAMERA_NO_CLIP		0x00000100	// Client-side camera's LOS won't clip against any entities that have this flag set.
#define EF_CAMERA_ONLY			0x00000200	// Client-side camera LOS traces will clip against any entities that have this flag set, but other traces won't.

#define EF_ALTCLIENTFX			0x00000400	// A message flag to send to the client effects, for user's purpose.
#define EF_ALWAYS_ADD_EFFECTS	0x00000800	// Any effects attached to the entity will always be added to the view. This overrides EF_DISABLE_*_CFX.

#define EF_ON_FIRE				0x00001000	// Tells that the entity is on fire.
#define EF_TRAILS_ENABLED		0x00002000	// The hand/staff trails enabled. If removed, the trails remove themselves.
#define EF_BLOOD_ENABLED		0x00004000	// Bloody trails on the weapon.
#define EF_DISABLE_EXTRA_FX		0x00008000	// Remove effects that are not owned by an entity with this flag, but need this info.
											// This should be set when something dies and needs the effects to reflect this.

#define EF_MACE_ROTATE			0x00010000	// Make the mace ball model rotate around its axes - save network traffic.
#define EF_CLIENT_DEAD			0x00020000	// This client on the server is dead. Easiest way to get this flag to CFX.
#define EF_POWERUP_ENABLED		0x00040000	// Tells the client effects that Corvus powerup is enabled.
#define EF_SPEED_ACTIVE			0x00080000	// Tells the client effects that Corvus speedup is enabled.
#define EF_HIGH_MAX				0x00100000	// Tells the client effects that Corvus speedup is enabled.
#define EF_LIGHT_ENABLED		0x00200000	// Tells the client effects that Corvus' personal torch is activated.

#define EF_CHICKEN				0x00400000	// The flag that tells the system that the player is a chicken, and not Corvus.
#define EF_ANIM_ALL				0x00800000	// Automatically cycle through all frames at 2hz.
#define EF_ANIM_ALLFAST			0x01000000	// Automatically cycle through all frames at 10hz.

#define EF_FAST_MOVER			0x02000000 //mxd. Double max. interpolated move distance in AddServerEntities() (very special monster_harpy flag).

// CEF_XXX
// Flags specified when a client-effect is created; don't even think about expanding this beyond 1 byte!
// This is 'cos only the first byte is sent across the net (and only a flag is actually set).

#define CEF_OWNERS_ORIGIN		0x00000001	// Use the owner's origin only, with no additional displacement.
#define CEF_BROADCAST			0x00000002	// Sent to all clients.
#define CEF_ENTNUM16			0x00000004	// Index is a short.
#define CEF_MULTICAST			0x00000008	// Places the effect into the world buffer instead of the owner's buffer
											// (no effect on independent effects), resulting in the effect being more reliably sent
											// at the expense of an extra byte or two.

#define CEF_DONT_LINK			0x00000010	// Used to stop client effects from being linked to their owner's movement.
											// In this case, CEF_OWNERS_ORIGIN causes the owner's origin to be used for initialization only.

#define CEF_FLAG6				0x00000020
#define CEF_FLAG7				0x00000040
#define CEF_FLAG8				0x00000080

// Client-effect Flags relevant only in the Client Effects DLL.

#define CEF_FRAME_LERP			0x00010000	//mxd. Enable frame interpolation for this client entity. When set, client_entity_t->r.oldframe must be updated appropriately.
#define CEF_VIEWSTATUSCHANGED	0x00020000	// If this flag is set, do not think when the CEF_CULLED flag is set.
#define CEF_USE_VELOCITY2		0x00040000	// Sprite lines. Read and apply the velocity2 and acceleration2 fields of the line.
#define CEF_USE_SCALE2			0x00080000	// Sprite lines. Read and apply the scale2 value to the endpoint.
#define CEF_AUTO_ORIGIN			0x00100000	// Sprite lines. Read just the origin of the line to the centerpoint after any movement.

#define CEF_PULSE_ALPHA			0x00200000	// Particle/fx d_alpha: when hits 1.0 alpha, reverse and start fading out.
#define CEF_ABSOLUTE_PARTS		0x00400000	// Particle origins represent absolute positions.
#define CEF_ADDITIVE_PARTS		0x00800000	// Particles are additively transparent (temporary).

#define CEF_DROPPED				0x01000000	// Entity was dropped from the view due to an excessive number of entities in the view.
#define CEF_NOMOVE				0x02000000	// Velocity and acceleration are not applied to origin in update.
											// Acceleration is not applied to velocity in update.
											// Allows vel and accel to be used for something else for static entities.

#define CEF_CULLED				0x04000000	// Culled from view this frame (set or unset) in AddEffectsToView().
#define CEF_CLIP_TO_WORLD		0x08000000	// Turns on collision detection with the world. Additionally, the
											// entity needs to have a message handler in order to receive MSG_COLLISION.

//mxd. Free slot at 0x10000000 (originally CEF_CLIP_TO_ENTITIES).

#define CEF_DISAPPEARED			0x20000000	// Alpha faded out, or scaled to nothing needs to be turned off if entity
											// later scales up or fades back in.

#define CEF_CHECK_OWNER			0x40000000	// If we are owned, then check to see if our owner has been server culled before it gets to the client.
#define CEF_NO_DRAW				0x80000000	// Doesn't get added to the render list.

#define EFFECT_PRED_INFO		0x4000
#define EFFECT_FLAGS			0x8000 //mxd

//mxd. Trace flags (used by fxi.Trace() / CL_Trace()). Part of CEF_ flags in original logic.

#define CTF_CLIP_TO_WORLD		0x00000001	// Turns on collision detection with the world model.
#define CTF_CLIP_TO_BMODELS		0x00000002	// Turns on collision detection with brush models. 
#define CTF_CLIP_TO_ENTITIES	0x00000004	// Turns on collision detection with server entities (not client-only entities).
#define CTF_IGNORE_PLAYER		0x00000008	// Turns off collision detection with player.
// NOTE: CTF_CLIP_TO_BMODELS / CTF_CLIP_TO_ENTITIES only clip against entities in the current frame, not all entities in the world.

//mxd. Legacy flags for compatibility reasons.
#define CTF_CLIP_TO_WORLD_LEGACY	0x08000000 // CEF_CLIP_TO_WORLD in original logic. Converted to CTF_CLIP_TO_WORLD in CL_Trace().
#define CTF_CLIP_TO_ENTITIES_LEGACY	0x10000000 // CEF_CLIP_TO_ENTITIES in original logic. Converted to (CTF_CLIP_TO_BMODELS | CTF_CLIP_TO_ENTITIES) in CL_Trace().

#define CTF_CLIP_TO_ALL			(CTF_CLIP_TO_WORLD | CTF_CLIP_TO_BMODELS | CTF_CLIP_TO_ENTITIES)