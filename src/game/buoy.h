//
// buoy.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "q_Typedef.h"

#define BUOY_JUMP			1
#define BUOY_ACTIVATE		2

#define PATHDIR_FORWARD		0
#define PATHDIR_BACKWARD	1

#define SEARCH_COMMON		0
#define SEARCH_BUOY			1

#define MAX_BUOY_DIST		1024.0f //mxd. int -> float.
#define MAX_MAP_BUOYS		256
#define MAX_BUOY_BRANCHES	3

#define BUOY_RADIUS			24.0f //mxd

#define NULL_BUOY			(-1)

typedef struct buoy_s buoy_t;

struct buoy_s
{
	int nextbuoy[MAX_BUOY_BRANCHES]; // Linking buoys (index inside array).
	int modflags;	// modflags replaces spawnflags.
	int opflags;	// opflags hold SF_DONT_TRY and the like.
	vec3_t origin;	// Where it is in the world.
	int id;			// This buoy's id number.
	char* pathtarget;
	float wait;
	float delay;
	float temp_dist; // To be used by ents searching for a buoy; no need to init or cleanup.
	float temp_e_dist;

	float jump_fspeed;
	float jump_yaw;
	float jump_uspeed;
	int jump_target_id; // This buoy's id number.

	char* target; // Saving these two to make debugging info. //TODO: assigned, but never used. Remove?
	char* targetname;	// Useful to the designer.
	char* jump_target;	// Keep around for debug.
};

extern void SP_info_buoy(edict_t* self);
extern buoy_t* FindNextBuoy(edict_t* self, int start_buoy_id, int final_buoy_id); //mxd

//mxd. Required by save system...
extern void LinkBuoyInfo(edict_t* self);