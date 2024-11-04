#ifndef BUOY_H
#define BUOY_H

#define BUOY_JUMP		1
#define BUOY_ACTIVATE	2
#define BUOY_TURN		4

#define PATHDIR_ERROR	   -1
#define PATHDIR_FORWARD		0
#define PATHDIR_BACKWARD	1
#define PATHDIR_FORKED		2

#define SEARCH_COMMON	0
#define SEARCH_BUOY		1
#define SEARCH_WAYPOINT	2
#define SEARCH_STAND	3
#define SEARCH_TARGET	4

#define MAX_BUOY_DIST		1024
#define MAX_MAP_BUOYS		256
#define MAX_BUOY_BRANCHES	3

#define NF_FORK 1

#define NULL_BUOY	-1

typedef struct buoy_s buoy_t;

struct buoy_s
{
	int		nextbuoy[MAX_BUOY_BRANCHES];	//Linking buoys (index inside array)
	int		modflags, opflags;				//modflags replaces spawnflags, opflags hold SF_DONT_TRY and the like
	vec3_t	origin;							//Where it is in the world
	int		contents;						//If buoy is in water or not (actual return from a pointcontents call)
	int		id;								//This buoy's id number
	char	*pathtarget;
	float	wait;
	float   delay;
	float	temp_dist,temp_e_dist;			//sfs--to be used by ents searching for a buoy; no need to init or cleanup
											//		since the vals will just be used immediately & forgotten--
											//		er, if this is wasteful (for savegames, dunno if whole buoy
											//		struct is dumped into savegame, or each field specifically
											//		handled), these should be changed to a static array of floats
											//		in mg_guide.
	float	jump_fspeed;
	float	jump_yaw;
	float	jump_uspeed;
	int		jump_target_id;					//This buoy's id number
	char	*target;						//saving these two to make debugging info
	char	*targetname;					//useful to the designer
	char	*jump_target;					//keep around for debug
	float	print_debounce_time;			//wait between +action print info

//	Gil suggestion: unimplemented
//	int		failed_depth;					//depth at which this buoy failed
};

void info_buoy_link(edict_t *self);
int info_buoy_find_dir(edict_t *self, edict_t *pos1, edict_t *pos2);
void SP_info_buoy(edict_t *self);
void MG_RemoveBuoyEffects(edict_t *self);

#endif // G_BUOY_H