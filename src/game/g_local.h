// g_local.h -- local definitions for game module.

#ifndef	G_LOCAL_H
#define G_LOCAL_H

#include "q_shared.h"
#include "q_ClientServer.h"
#include "buoy.h"

#if 0
#define G_NOAMMO
#endif

// Defined so I can insert hooks into code without breaking anything
#define PAGAN	0

// Define GAME_INCLUDE so that game.h does not define the short, server-visible 'gclient_t' and
// 'edict_t' structures because we define the game versions in this file.

#define	GAME_INCLUDE

#include "game.h"
#include "player.h"

// The "gameversion" client command will print this plus compile date.

#if DEMO_CODE
#define	GAMEVERSION	"Heretic2Dmo"
#else
#define	GAMEVERSION	"Heretic2v16"
#endif

// Protocol bytes that can be directly added to messages.

// volume mask for ent->sound_data - makes room for attn value in the lower bits
#define ENT_VOL_MASK	0xf8

// ************************************************************************************************
// AI_MOOD_XXX
// -------------
// Held in 'edict_t'->ai_mood. Used by higher level AI functions to relay states to lower functions
// ************************************************************************************************

#define	AI_MOOD_NORMAL		0		//Not using any high level functionality (TEMP)
#define	AI_MOOD_ATTACK		1		//Used in conjuntion with ai_mood_flags to attack the target
#define	AI_MOOD_NAVIGATE	2		//Just walk towards the guide, ignoring everything else
#define	AI_MOOD_STAND		3		//Just stand there and wait to be adivsed
#define	AI_MOOD_PURSUE		4		//Run towards your enemy but don't attack
#define	AI_MOOD_FALLBACK	5		//Back away from your enemy, but face him
#define	AI_MOOD_DELAY		6		//Same as stand, but will allow interruption anywhere
#define AI_MOOD_WANDER		7		//Wandering around buoy to buoy in a walk
#define AI_MOOD_JUMP		8		//Jump towards goalentity
#define AI_MOOD_REST		9		//The Ogle at rest
#define AI_MOOD_POINT_NAVIGATE	10	//Navigate to a point, not an entity
#define AI_MOOD_FLEE		11		//run away!
#define AI_MOOD_BACKUP		12		//backstep while attacking
#define AI_MOOD_WALK		13		//walking, no buoys
#define AI_MOOD_EAT			14		//sitting around, eating

// ************************************************************************************************
// AI_MOOD_FLAG_XXX
// -------------
// Held in 'edict_t'->ai_mood_flags. Used in conjuction with ai_mood
// ************************************************************************************************

#define	AI_MOOD_FLAG_MISSILE		0x00000001		//Check for a missile attack
#define	AI_MOOD_FLAG_MELEE			0x00000002		//Check for a melee attack
#define AI_MOOD_FLAG_WHIP			0x00000004		//Check for a whipping attack (no damage)
#define AI_MOOD_FLAG_PREDICT		0x00000008		//Monster will predict movement on target
#define AI_MOOD_FLAG_IGNORE			0x00000010		//Monster will ignore moods
#define AI_MOOD_FLAG_FORCED_BUOY	0x00000020		//Monster will head towards it's forced_buoy
#define AI_MOOD_FLAG_IGNORE_ENEMY	0x00000040		//Monster will ignore it's enemy unless attacked or otherwise directed
#define AI_MOOD_FLAG_BACKSTAB		0x00000080		//Monster will advance on and attack enemy only from behind
#define AI_MOOD_FLAG_DUMB_FLEE		0x00000100		//Monster will flee by simply running directly away from player
#define AI_MOOD_FLAG_GOTO_FIXED		0x00000200		//Monster will become fixed upon getting to it's forced_buoy
#define AI_MOOD_FLAG_GOTO_STAND		0x00000400		//Monster will stand upon getting to it's forced_buoy
#define AI_MOOD_FLAG_GOTO_WANDER	0x00000800		//Monster will wander upon getting to it's forced_buoy
#define AIMF_CANT_FIND_ENEMY		0x00001000		//Monster can't find enemy with buoys or vision
#define AIMF_SEARCHING				0x00002000		//Monster now in dumb search mode...

// ************************************************************************************************
// SPAWNFLAG_XXX
// -------------
// Held in 'edict_t'->spawnflags. These are set with checkboxes on each entity in the map editor.
// ************************************************************************************************

#define	SPAWNFLAG_NOT_EASY			0x00000100
#define	SPAWNFLAG_NOT_MEDIUM		0x00000200
#define	SPAWNFLAG_NOT_HARD			0x00000400
#define	SPAWNFLAG_NOT_DEATHMATCH	0x00000800
#define	SPAWNFLAG_NOT_COOP			0x00001000

// ************************************************************************************************
// Timing constants that define the world heartbeat.
// ************************************************************************************************

#define	FRAMETIME			0.1
#define MONSTER_THINK_INC   0.099
#define FRAMES_PER_SECOND	10.0

// ************************************************************************************************
// TAG_XXX
// -------
// Memory tags to allow dynamic memory to be selectively cleaned up.
// ************************************************************************************************

#define	TAG_GAME	765			// clear when unloading the dll
#define	TAG_LEVEL	766			// clear when loading a new level

// ************************************************************************************************
// damage_t
// --------
// ************************************************************************************************

typedef enum
{
	DAMAGE_NO,
	DAMAGE_YES,	// Will take damage if hit.
	DAMAGE_AIM,	// Auto targeting recognizes this.
	DAMAGE_NO_RADIUS,	// Will not take damage from radius blasts
} damage_t;

#define GIB_ORGANIC 1

#define BODY_QUEUE_SIZE		8

// ************************************************************************************************
// RANGE_XXX
// ---------
// ************************************************************************************************

#define RANGE_MELEE	0
#define RANGE_NEAR	1
#define RANGE_MID	2
#define RANGE_FAR	3

#define MELEE_DISTANCE	80

// ************************************************************************************************
// ARMOR_XXX
// ----------
// Armor types.
// ************************************************************************************************

#define ARMOR_NONE		0
#define ARMOR_JACKET	1
#define ARMOR_COMBAT	2
#define ARMOR_BODY		3
#define ARMOR_SHARD		4

// ************************************************************************************************
// SHRINE_XXX
// ----------
// ************************************************************************************************

enum
{
	SHRINE_MANA,
	SHRINE_LUNGS,
	SHRINE_ARMOR_SILVER,
	SHRINE_ARMOR_GOLD,
	SHRINE_LIGHT,
	SHRINE_SPEED,
	SHRINE_HEAL,
	SHRINE_STAFF,
	SHRINE_GHOST,
	SHRINE_REFLECT,
	SHRINE_POWERUP,
	SHRINE_RANDOM
};

// ************************************************************************************************
// SFL_CROSS_TRIGGER_XXX
// ---------------------
// game.'serverflags' values.
// ************************************************************************************************

#define SFL_CROSS_TRIGGER_1		0x00000001
#define SFL_CROSS_TRIGGER_2		0x00000002
#define SFL_CROSS_TRIGGER_3		0x00000004
#define SFL_CROSS_TRIGGER_4		0x00000008
#define SFL_CROSS_TRIGGER_5		0x00000010
#define SFL_CROSS_TRIGGER_6		0x00000020
#define SFL_CROSS_TRIGGER_7		0x00000040
#define SFL_CROSS_TRIGGER_8		0x00000080
#define SFL_CROSS_TRIGGER_MASK	0x000000ff

#define MAX_MESSAGESTRINGS 1000
typedef struct
{
	char *string;
	char *wav;
} trig_message_t;

#ifdef __cplusplus
extern "C"
{
#endif
	extern trig_message_t message_text[];
#ifdef __cplusplus
}
#endif

unsigned	*messagebuf;

// ************************************************************************************************
// game_locals_t
// -------------
// This structure is left intact through an entire game. It should be initialized at game.dll load
// time and read from / written to the 'server.ssv' file for savegames.
// ************************************************************************************************

typedef struct
{
	char		helpmessage1[512];
	char		helpmessage2[512];

	// Flash F1 icon if not 0, play sound and increment only if 1, 2, or 3.

	int			helpchanged;

	// [maxclients].

	gclient_t	*clients;

	// Needed for co-op respawns... can't store spawnpoint in level, because it would get
	// overwritten by the savegame restore.

	char		spawnpoint[512];	

	// Store latched cvars that we want to get at often here.

	int			maxclients;
	int			maxentities;

	//updated every frame in DM, for pick-up and shrine respawn times
	int			num_clients;

	// Cross level triggers.

	int			serverflags;

	// Items.

	int			num_items;

	// Flag that we've autosaved.

	qboolean	autosaved;

	qboolean	entitiesSpawned;

} game_locals_t;

#include "ICScript.h"

// ************************************************************************************************
// alertent_t
// ---------
// This structure is used for alert entities, which are spawned a lot
// ************************************************************************************************
#define MAX_ALERT_ENTS	1024//no more that 1024 alertents allowed
typedef struct alertent_s alertent_t;
struct alertent_s
{
	alertent_t	*next_alert;
	alertent_t	*prev_alert;
	edict_t		*enemy;
	vec3_t		origin;
	qboolean	inuse;
	int			alert_svflags;
	float		lifetime;
};

// ************************************************************************************************
// level_locals_t
// --------------
// This structure is cleared as each map is entered. It is read/written to the 'level.sav' file for
// savegames.
// ************************************************************************************************

typedef struct
{
	int			framenum;
	float		time;

	char		level_name[MAX_QPATH];	// The descriptive name (e.g. 'Outer Base').
	char		mapname[MAX_QPATH];		// The server name (e.g. 'base1').
	char		nextmap[MAX_QPATH];		// Go here when fraglimit is hit.

	// Intermission state information.

	float		intermissiontime;		// Time the intermission was started.
	char		*changemap;
	int			exitintermission;
	vec3_t		intermission_origin;
	vec3_t		intermission_angle;

	edict_t		*sight_client;			// Changed once each frame for coop games.

	edict_t		*sight_entity;
	int			sight_entity_framenum;

	int			pic_health;

	int			total_secrets;
	int			found_secrets;

	int			total_goals;
	int			found_goals;

	int			total_monsters;
	int			killed_monsters;

	float		far_clip_dist_f;
	float		fog;
	float		fog_density;

	edict_t		*current_entity;			// Entity running from G_RunFrame().
	int			body_que;					// Dead bodies.

	ICScript_t	inGameCin;
	qboolean	cinActive;

	buoy_t		buoy_list[MAX_MAP_BUOYS];	//Buoy information for this map
	int			active_buoys;				//Number of actual buoys on the level
	int			fucked_buoys;				//Number of buoys that can't be fixed
	int			fixed_buoys;				//Number of buoys that had to be fixed
	
	int			player_buoy[MAX_CLIENTS];				//stores current bestbuoy for a player enemy (if any)
	int			player_last_buoy[MAX_CLIENTS];		//when player_buoy is invalid, saves it here so monsters can check it first instead of having to do a whole search


	int			offensive_weapons,
				defensive_weapons;

	alertent_t	alertents[MAX_ALERT_ENTS];	//all the alert ents on the map
	int			num_alert_ents;				//Number of actual alert entities on the level
	alertent_t	*alert_entity,				//the alert entity linked list start
				*last_alert;				//the last entity in alert entity linked list

	qboolean	fighting_beast;				//fighting a beast, do extra checks with trace instant weapons

} level_locals_t;

// ************************************************************************************************
// MOD_XXX
// -------
// Means of death.
// ************************************************************************************************

typedef enum
{
	MOD_UNKNOWN			,

	MOD_STAFF			,
	MOD_FIREBALL		,
	MOD_MMISSILE		,
	MOD_SPHERE			,
	MOD_SPHERE_SPL		,
	MOD_IRONDOOM		,
	MOD_FIREWALL		,
	MOD_STORM			,
	MOD_PHOENIX			,
	MOD_PHOENIX_SPL		,
	MOD_HELLSTAFF		,

	MOD_P_STAFF			,
	MOD_P_FIREBALL		,
	MOD_P_MMISSILE		,
	MOD_P_SPHERE	 	,
	MOD_P_SPHERE_SPL 	,
	MOD_P_IRONDOOM		,
	MOD_P_FIREWALL		,
	MOD_P_STORM			,
	MOD_P_PHOENIX	 	,
	MOD_P_PHOENIX_SPL	,
	MOD_P_HELLSTAFF		,

	MOD_KICKED			,
	MOD_METEORS			,
	MOD_ROR				,
	MOD_SHIELD			,
	MOD_CHICKEN			,
	MOD_TELEFRAG		,
	MOD_WATER			,
	MOD_SLIME			,
	MOD_LAVA			,
	MOD_CRUSH			,
	MOD_FALLING			,
	MOD_SUICIDE			,
	MOD_BARREL			,
	MOD_EXIT			,
	MOD_BURNT			,
	MOD_BLEED			,
	MOD_SPEAR			,
	MOD_DIED			,
	MOD_KILLED_SLF		,
	MOD_DECAP			,
	MOD_TORN			,
	MOD_MAX
} MOD_t;

#define MOD_FRIENDLY_FIRE	0x8000000

// ************************************************************************************************
// spawn_temp_t
// ------------
// This is used to hold entity field values that can be set from the editor, but aren't actualy
// present in 'edict_t' during gameplay.
// ************************************************************************************************

typedef struct
{
	// Sky variables.

	char	*sky;
	float	skyrotate;
	vec3_t	skyaxis;
	
	// Nextmap. Is this used?

	char	*nextmap;

	int		lip;
	int		distance;
	int		height;
	char	*noise;
	float	pausetime;
	char	*item;
	char	*gravity;

	float	minyaw;
	float	maxyaw;
	float	minpitch;
	float	maxpitch;
	int		rotate;
	float	zangle;
	char	*file;
	int		radius;

	// Weapons to be given to the player on spawning.

	int		offensive;
	int		defensive;
	int		spawnflags2;
	
	// Time to wait (in seconds) for all clients to have joined a map in coop.

	int		cooptimeout;

	// Scripting stuff.

	char	*script;
	char	*parms[16];
} spawn_temp_t;

// ************************************************************************************************
// moveinfo_t
// ----------
// This is used to hold information pertaining to an entity's movement.
// ************************************************************************************************

typedef struct
{
	// Fixed data.

	vec3_t	start_origin;
	vec3_t	start_angles;
	vec3_t	end_origin;
	vec3_t	end_angles;

	int		sound_start;
	int		sound_middle;
	int		sound_end;

	float	accel;
	float	speed;
	float	decel;
	float	distance;

	float	wait;

	// State data.

	int		state;
	vec3_t	dir;
	float	current_speed;
	float	move_speed;
	float	next_speed;
	float	remaining_distance;
	float	decel_distance;
	void	(*endfunc)(edict_t *);
} moveinfo_t;

// ************************************************************************************************
// AI_XXX
// ------
// Monster AI flags.
// ************************************************************************************************

#define AI_STAND_GROUND			0x00000001
#define AI_TEMP_STAND_GROUND	0x00000002
#define AI_SOUND_TARGET			0x00000004
#define AI_LOST_SIGHT			0x00000008
#define AI_PURSUIT_LAST_SEEN	0x00000010
#define AI_PURSUE_NEXT			0x00000020
#define AI_PURSUE_TEMP			0x00000040
#define AI_HOLD_FRAME			0x00000080
#define AI_GOOD_GUY				0x00000100
#define AI_BRUTAL				0x00000200
#define AI_NOSTEP				0x00000400	//1024
#define AI_DUCKED				0x00000800
#define AI_COMBAT_POINT			0x00001000
#define AI_EATING				0x00002000
#define AI_RESURRECTING			0x00004000
#define AI_FLEE					0x00008000
#define AI_FALLBACK				0x00010000
#define AI_COWARD				0x00020000	//Babies (FLEE to certain distance & WATCH)
#define AI_AGRESSIVE			0x00040000	//never run away
#define AI_SHOVE				0x00080000	//shove others out of the way.
#define AI_DONT_THINK			0x00100000	//animate, don't think or move
#define AI_SWIM_OK				0x00200000	//ok to go in water
#define AI_OVERRIDE_GUIDE		0x00400000	
#define AI_NO_MELEE				0x00800000	//not allowed to melee
#define AI_NO_MISSILE			0x01000000	//not allowed to missile
#define AI_USING_BUOYS			0x02000000	//Using Buoyah! Navigation System(tm)
#define AI_STRAIGHT_TO_ENEMY	0x04000000	//Charge straight at enemy no matter what anything else tells you
#define AI_NIGHTVISION			0x08000000	//light level does not effect this monster's vision or aim
#define AI_NO_ALERT				0x10000000	//monster does not pay attemntion to alerts

// ************************************************************************************************
// AS_XXX
// ------
// Monster attack states.
// ************************************************************************************************

#define AS_STRAIGHT	1
#define AS_SLIDING	2
#define	AS_MELEE	3
#define	AS_MISSILE	4
#define	AS_DIVING	5

// ************************************************************************************************
// C_ANIM_XXX
// ------
// Cinmatic Animation flags
// ************************************************************************************************
#define C_ANIM_MOVE		1
#define C_ANIM_REPEAT	2
#define C_ANIM_DONE		4
#define C_ANIM_IDLE		8

// ************************************************************************************************
// OBJ_XXX
// ------
// Flags for object entities
// ************************************************************************************************
#define OBJ_INVULNERABLE	1
#define OBJ_ANIMATE			2
#define OBJ_EXPLODING		4
#define OBJ_NOPUSH			8


// ************************************************************************************************
// SIGHT_XXX
// ------
// Type of target aquisition
// ************************************************************************************************
#define SIGHT_SOUND_TARGET 0		//Heard the target make this noise
#define SIGHT_VISIBLE_TARGET 1		//Saw this target
#define SIGHT_ANNOUNCED_TARGET 2	//Target was announced by another monster


// ************************************************************************************************
// mframe_t
// --------
// ************************************************************************************************

typedef struct
{
	int		framenum;								// Index to current animation frame.
	void	(*aifunc)(edict_t *self, float dist);	// AI function for this animation frame.
	float	dist;									// Distance the AI function should move the
													// entity this frame.
	void	(*thinkfunc)(edict_t *self);			// Think function for this frame.
} mframe_t;

// ************************************************************************************************
// mmove_t
// -------
// ************************************************************************************************

typedef struct
{
	int			framecount;					// Number of frames in the animation frame array.
	mframe_t	*frame;
	void		(*endfunc)(edict_t *self);
} mmove_t;


// ************************************************************************************************
// animframe_t
// -----------
// ************************************************************************************************

typedef struct
{
	int		framenum;
	void	(*movefunc)(edict_t *self, float var1, float var2, float var3);
	float	var1, var2, var3;
	void	(*actionfunc)(edict_t *self, float var4);
	float	var4;
	void	(*thinkfunc)(edict_t *self);
} animframe_t;

// ************************************************************************************************
// animmove_t
// ----------
// ************************************************************************************************

typedef struct
{
	int			numframes;
	animframe_t	*frame;
	void		(*endfunc)(edict_t *self);
} animmove_t;


// ************************************************************************************************
// c_animflags_t
// ----------
// ************************************************************************************************
typedef struct 
{
	qboolean moving;		// Does this action support moving
	qboolean repeat;		// Does this action support repeating
	qboolean turning;	// Does this action support turning
} c_animflags_t;


// ************************************************************************************************
// monsterinfo_t
// -------------
// ************************************************************************************************

typedef struct
{
// Not used in new system
	char		*otherenemyname;				// ClassName of secondary enemy (other than player).
												// E.g. a Rat's secondary enemy is a gib.

	animmove_t	*currentmove;
	int			aiflags;
	int			aistate;						// Last order given to the monster (ORD_XXX).
	int			currframeindex;					// Index to current monster frame.
	int			nextframeindex;					// Used to force the next frameindex.
	float		thinkinc;						// Time between thinks for this entity.
	float		scale;

	void		(*idle)(edict_t *self);
	void		(*search)(edict_t *self);
	void		(*dodge)(edict_t *self, edict_t *other, float eta);
	int			(*attack)(edict_t *self);
	void		(*sight)(edict_t *self, edict_t *other);
	void		(*dismember)(edict_t *self, int damage, int HitLocation);
	qboolean	(*alert)(edict_t *self, alertent_t *alerter, edict_t *enemy);
	qboolean	(*checkattack)(edict_t *self);

	float		pausetime;
	float		attack_finished;
	float		flee_finished;					// When a monster is done fleeing
	float		chase_finished;					// When the monster can look for secondary monsters.

	vec3_t		saved_goal;
	float		search_time;
	float		misc_debounce_time;
	vec3_t		last_sighting;
	int			attack_state;
	int			lefty;
	float		idle_time;
	int			linkcount;

	int			searchType;
	vec3_t		nav_goal;
	float		jump_time;

	int			stepState;

	int			ogleflags;		//Ogles have special spawnflags stored in here at spawntime

	int			supporters;		//Number of supporting monsters (with common type) in the area when awoken
	
	float		sound_finished;	//Amount of time until the monster will be finishing talking (used for voices)
	float		sound_start;	//The amount of time to wait before playing the pending sound
	int			sound_pending;	//This monster is waiting to make a sound (used for voices) (0 if false, else sound ID)
	
	// Cinematic fields
	int			c_dist;			// Distance left to move
	int			c_repeat;		// # of times to repeat the anim cycle
	void (*c_callback)(struct edict_s *self);	// Callback function when action is done
	int			c_anim_flag;		// Shows if current cinematic anim supports moving, turning, or repeating
	qboolean	c_mode;			// in cinematic mode or not?
	edict_t		*c_ent;			// entity passed from a cinematic command

	qboolean	awake;			// has found an anemy AND gone after it.
	qboolean	roared;			// Gorgon has roared or been woken up by a roar

	float		last_successful_enemy_tracking_time;	//last time successfully saw enemy or found a path to him
	float		coop_check_debounce_time;
} monsterinfo_t;

// ************************************************************************************************
// aceldata_t
// ----------
// ************************************************************************************************

typedef struct 
{
	animmove_t	*move;
	short		fly;
	short		lockmove;
	int			playerflags;
} aceldata_t;

// ************************************************************************************************
// acelsizes_t
// -----------
// ************************************************************************************************

typedef struct
{
	vec3_t	boundbox[2];
	int		altmove;
	float	viewheight;
	float	waterheight;
} acelsizes_t;

// The structure for each monster class.

#define	FOFS(x)		(int)&(((edict_t *)0)->x)
#define	STOFS(x)	(int)&(((spawn_temp_t *)0)->x)
#define	LLOFS(x)	(int)&(((level_locals_t *)0)->x)
#define	CLOFS(x)	(int)&(((gclient_t *)0)->x)
#define	BYOFS(x)	(int)&(((buoy_t *)0)->x)

extern	game_locals_t	game;
#ifdef __cplusplus
extern "C"
{
#endif
extern	level_locals_t	level;
extern	edict_t			*g_edicts;
extern	game_import_t	gi;
extern	spawn_temp_t	st;
extern	game_export_t	globals;
#ifdef __cplusplus
}
#endif

extern	int				sm_meat_index;
extern	int				snd_fry;

extern	cvar_t			*maxentities;
extern	cvar_t			*deathmatch;
extern	cvar_t			*coop;
extern	cvar_t			*dmflags;
extern	cvar_t			*advancedstaff;
extern	cvar_t			*skill;
extern	cvar_t			*fraglimit;
extern	cvar_t			*timelimit;
extern	cvar_t			*password;
extern	cvar_t			*g_select_empty;
extern	cvar_t			*dedicated;
extern	cvar_t			*filterban;

extern	cvar_t			*sv_gravity;
extern	cvar_t			*sv_friction;
extern	cvar_t			*sv_maxvelocity;

extern	cvar_t			*gun_x,*gun_y,*gun_z;
extern	cvar_t			*sv_rollspeed;
extern	cvar_t			*sv_rollangle;

extern	cvar_t			*run_pitch;
extern	cvar_t			*run_roll;
extern	cvar_t			*bob_up;
extern	cvar_t			*bob_pitch;
extern	cvar_t			*bob_roll;

extern	cvar_t			*sv_cheats;
extern	cvar_t			*sv_nomonsters;
extern  cvar_t			*blood_level;
extern	cvar_t			*showbuoys;
extern	cvar_t			*showlitebuoys;
extern	cvar_t			*mgai_debug;
extern	cvar_t			*deactivate_buoys;
extern	cvar_t			*anarchy;
extern	cvar_t			*impact_damage;
extern	cvar_t			*cheating_monsters;
extern	cvar_t			*singing_ogles;
extern	cvar_t			*no_runshrine;
extern	cvar_t			*no_tornado;
extern	cvar_t			*no_irondoom;
extern	cvar_t			*no_phoenix;
extern	cvar_t			*no_morph;
extern	cvar_t			*no_shield;
extern	cvar_t			*no_teleport;
extern	cvar_t			*log_file_name;
extern	cvar_t			*log_file_header;
extern	cvar_t			*log_file_footer;
extern	cvar_t			*log_file_line_header;

#ifdef __cplusplus
extern "C"
{
#endif
	extern	cvar_t			*sv_cinematicfreeze;
	extern	cvar_t			*sv_jumpcinematic;
#ifdef __cplusplus
}
#endif

extern	cvar_t			*sv_freezemonsters;

extern	cvar_t			*maxclients;
extern	cvar_t			*sv_maplist;

extern	cvar_t			*autorotate;
extern	cvar_t			*blood;

extern	cvar_t			*checkanim;		// specifies whether monsters should check to see if most of the
										// distance of a move animation is unobstructed before setting it
extern	cvar_t			*allowillegalskins;

extern	cvar_t			*monster_speeds;
extern	cvar_t			*pvs_cull;

extern	cvar_t			*game_test; // sfs--for testing the speed impact of code changes
extern	cvar_t			*dm_no_bodies;

extern	cvar_t			*player_dll;

extern	cvar_t			*flood_msgs;
extern	cvar_t			*flood_persecond;
extern	cvar_t			*flood_waitdelay;
extern	cvar_t			*flood_killdelay;

extern	edict_t			*g_edicts;

extern	int				self_spawn;
#define world			(&g_edicts[0])

// ************************************************************************************************
// 'DROPPED_XXX'.
// --------------
// ************************************************************************************************

#define DROPPED_ITEM		0x00008000
#define	DROPPED_PLAYER_ITEM	0x00010000

// fields are needed for spawning from the entity string
// and saving / loading games

#define FFL_SPAWNTEMP		1

// ************************************************************************************************
// fieldtype_t
// -----------
// ************************************************************************************************

typedef enum {
	F_INT, 
	F_FLOAT,
	F_LSTRING,			// string on disk, pointer in memory, TAG_LEVEL
	F_GSTRING,			// string on disk, pointer in memory, TAG_GAME
	F_VECTOR,
	F_ANGLEHACK,
	F_EDICT,			// index on disk, pointer in memory
	F_ITEM,				// index on disk, pointer in memory
	F_CLIENT,			// index on disk, pointer in memory
	F_RGBA,
	F_RGB,
	F_IGNORE,
} fieldtype_t;

// ************************************************************************************************
// field_t
// -------
// ************************************************************************************************

typedef struct
{
	char	*name;
	int		ofs;
	fieldtype_t	type;
	int		flags;
} field_t;

#ifdef __cplusplus
extern "C"
{
#endif
	extern	field_t fields[];
#ifdef __cplusplus
}
#endif

//
// g_cmds.c
//

void Cmd_Help_f (edict_t *ent);
void Cmd_Score_f (edict_t *ent);

//
// g_items.c
//

void PrecacheItem (gitem_t *it);
void G_InitItems (void);
void SetItemNames (void);
edict_t *Drop_Item (edict_t *ent, gitem_t *item);
void SetRespawn (edict_t *ent);
void SpawnItem (edict_t *ent, gitem_t *item);
void SpawnItemEffect(edict_t *ent, gitem_t *item);
gitem_t	*IsItem(edict_t *ent);
void Think_Weapon (edict_t *ent);
int ArmorIndex (edict_t *ent);
qboolean Add_Ammo (edict_t *ent, gitem_t *item, int count);

//
// g_utils.c
//

qboolean	KillBox (edict_t *ent);
void	G_ProjectSource (vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result);

#ifdef __cplusplus
extern "C"
{
#endif
	edict_t *G_Find (edict_t *from, int fieldofs, char *match);
	edict_t	*G_Spawn (void);
#ifdef __cplusplus
}
#endif

edict_t *oldfindradius (edict_t *from, vec3_t org, float rad);
edict_t *findradius (edict_t *from, vec3_t org, float rad);
edict_t *findinblocking (edict_t *from, edict_t *checkent);
edict_t *findinbounds(edict_t *from, vec3_t min, vec3_t max);
edict_t *oldfindinbounds(edict_t *from, vec3_t min, vec3_t max);
edict_t *finddistance (edict_t *from, vec3_t org, float mindist, float maxdist);
edict_t *findonpath(edict_t *startent, vec3_t startpos, vec3_t endpos, vec3_t mins, vec3_t maxs, vec3_t *resultpos);
edict_t *G_PickTarget (char *targetname);

//commonly used functions
int range (edict_t *self, edict_t *other);
qboolean clear_visible (edict_t *self, edict_t *other);
qboolean visible (edict_t *self, edict_t *other);
qboolean visible_pos (edict_t *self, vec3_t spot2);
qboolean infront (edict_t *self, edict_t *other);
qboolean infront_pos (edict_t *self, vec3_t pos);
qboolean ahead (edict_t *self, edict_t *other);

void G_UseTargets (edict_t *ent, edict_t *activator);
void G_SetMovedir (vec3_t angles, vec3_t movedir);
void G_InitEdict (edict_t *e);
void G_FreeEdict (edict_t *e);
void G_SetToFree (edict_t *);
void G_TouchTriggers (edict_t *ent);
void G_TouchSolids (edict_t *ent);
void G_LinkMissile(edict_t *ent);
char *G_CopyString (char *in);
char *vtos (vec3_t v);
float vectoyaw (vec3_t vec);

//
// g_combat.c
//

qboolean OnSameTeam (edict_t *ent1, edict_t *ent2);
qboolean CanDamage (edict_t *targ, edict_t *inflictor);
qboolean CanDamageFromLoc (edict_t *targ, edict_t *inflictor, vec3_t origin);
void T_Damage(edict_t *targ, edict_t *inflictor, edict_t *attacker, vec3_t dir, vec3_t point, vec3_t normal, 
					int damage, int knockback, int dflags,int MeansOfDeath);
void T_DamageRadius(edict_t *inflictor, edict_t *attacker, edict_t *ignore, float radius, 
						float maxdamage, float mindamage, int dflags,int MeansOfDeath);
void T_DamageRadiusFromLoc(vec3_t origin, edict_t *inflictor, edict_t *attacker, edict_t *ignore, float radius, 
							float maxdamage, float mindamage, int dflags,int MeansOfDeath);

// ************************************************************************************************
// DAMAGE_XXX
// ----------
// ************************************************************************************************

#define DAMAGE_NORMAL				0x00000000	// No modifiers to damage
#define DAMAGE_RADIUS				0x00000001	// damage was indirect
#define DAMAGE_NO_KNOCKBACK			0x00000002	// do not affect velocity, just view angles
#define DAMAGE_ALL_KNOCKBACK		0x00000004  // Ignore damage
#define DAMAGE_EXTRA_KNOCKBACK		0x00000008	// throw in some extra z
#define DAMAGE_NO_PROTECTION		0x00000010  // invulnerability, and godmode have no effect
#define DAMAGE_NO_BLOOD				0x00000020  // don't spawn any blood
#define DAMAGE_EXTRA_BLOOD			0x00000040	// Lots of blood
#define DAMAGE_SPELL				0x00000080  // this came from a spell, - for use in calcing armor effects
#define DAMAGE_DISMEMBER			0x00000100  // Force this hit to use dismemberment message
#define DAMAGE_ATTACKER_IMMUNE		0x00000200  // Inflictor receives no effect
#define DAMAGE_ATTACKER_KNOCKBACK	0x00000400  // Inflictor takes knockback only
#define DAMAGE_REDRAIN				0x00000800	// Red rain acid damage
#define DAMAGE_BUBBLE				0x00001000	// Drowning damage
#define DAMAGE_FIRE					0x00002000  // Fire damage  
#define DAMAGE_ALIVE_ONLY			0x00004000	// Only damage living things made of flesh
#define DAMAGE_BLEEDING				0x00008000	// No protection
#define DAMAGE_AVOID_ARMOR			0x00010000	// don't do the armor effect
#define DAMAGE_DOUBLE_DISMEMBER		0x00020000  // Force this hit to use dismemberment message with TWICE the chance of cutting
#define DAMAGE_HURT_FRIENDLY		0x00040000  // Always hurt friendly entities (e.g. fellow coop players).
#define DAMAGE_POWERPHOENIX			0x00080000	// Extra knockback to shooter, 1/4 damage.
#define DAMAGE_FIRE_LINGER			0x00100000	// Do extra fire linger damage.
#define DAMAGE_ENEMY_MAX			0x00200000	// Do maximum damage directly to the enemy in radius
#define DAMAGE_ONFIRE				0x00400000	// If the damage is FROM a fire...
#define DAMAGE_PHOENIX				0x00800000	// Phoenix-oriented damage.  Do minimal fire for show, but short duration.

#define DAMAGE_SUFFOCATION			(DAMAGE_NO_KNOCKBACK|DAMAGE_NO_BLOOD|DAMAGE_BUBBLE|DAMAGE_AVOID_ARMOR)
#define DAMAGE_LAVA					(DAMAGE_NO_KNOCKBACK|DAMAGE_NO_BLOOD|DAMAGE_FIRE|DAMAGE_AVOID_ARMOR)
#define DAMAGE_SLIME				(DAMAGE_NO_KNOCKBACK|DAMAGE_NO_BLOOD|DAMAGE_AVOID_ARMOR)
#define DAMAGE_BURNING				(DAMAGE_ONFIRE|DAMAGE_NO_KNOCKBACK|DAMAGE_NO_BLOOD|DAMAGE_FIRE|DAMAGE_AVOID_ARMOR)

//
// g_monster.c
//

void M_droptofloor (edict_t *ent);
void monster_think (edict_t *self);
qboolean walkmonster_start (edict_t *self);
qboolean swimmonster_start (edict_t *self);
qboolean flymonster_start (edict_t *self);
void AttackFinished (edict_t *self, float time);
void PauseTime(edict_t *self, float time);
void monster_death_use (edict_t *self);
void M_CatagorizePosition (edict_t *ent);
qboolean M_CheckAttack (edict_t *self);
void M_CheckGround (edict_t *ent);

//
// g_misc.c
//

void ThrowClientHead (edict_t *self, int damage);
void ThrowGib (edict_t *self, char *gibname, int damage, int type);
void BecomeExplosion1(edict_t *self);

//
// g_ai.c
//

void AI_SetSightClient (void);
void ai_stand (edict_t *self, float dist);
void ai_move (edict_t *self, float dist);
void ai_walk (edict_t *self, float dist);
void ai_turn (edict_t *self, float dist);
void ai_run (edict_t *self, float dist);
void ai_charge (edict_t *self, float dist);
void ai_eat (edict_t *self, float dist);
void ai_generic(edict_t *self);
void ai_flee(edict_t *self, float dist);
int range (edict_t *self, edict_t *other);
void FoundTarget (edict_t *self, qboolean setsightent);
qboolean infront (edict_t *self, edict_t *other);
qboolean visible (edict_t *self, edict_t *other);
qboolean FacingIdeal(edict_t *self);

//
// g_client.c
//

void respawn (edict_t *ent);
void BeginIntermission (edict_t *targ);
void PutClientInServer (edict_t *ent);
void InitClientPersistant (edict_t *player);
void InitClientResp (gclient_t *client);
void InitBodyQue (void);
void ClientBeginServerFrame (edict_t *ent);
int	SexedSoundIndex (edict_t *ent, char *base);

//
// g_player.c
//

int player_pain (edict_t *self, edict_t *other, float kick, int damage);
int player_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);
void player_dismember (edict_t *self, edict_t *other, int damage, int HitLocation);
void ResetPlayerBaseNodes (edict_t *ent);
void player_repair_skin (edict_t *self);

//
// g_svcmds.c
//

void	ServerCommand (void);
qboolean SV_FilterPacket (char *from);

//
// p_view.c
//

extern void ClientEndServerFrame(edict_t *ent);
extern qboolean CheckButton(edict_t *self);
extern void	SetupPlayerinfo(edict_t *ent);
extern void	WritePlayerinfo(edict_t *ent);
extern void	SetupPlayerinfo_effects(edict_t *ent);
extern void	WritePlayerinfo_effects(edict_t *ent);

//
// p_hud.c
//

void MoveClientToIntermission(edict_t *client,qboolean log_file);
void MoveClientsToIntermission(vec3_t ViewOrigin,vec3_t ViewAngles);
void G_SetStats (edict_t *ent);
void ValidateSelectedItem (edict_t *ent);
void SelectPrevItem (edict_t *ent, int itflags);
void SelectNextItem (edict_t *ent, int itflags);
void DeathmatchScoreboardMessage (edict_t *client, edict_t *killer,qboolean log_file);

//
// g_pweapon.c
//

void PlayerNoise(edict_t *who, vec3_t where, int type);

//
// m_move.c
//

qboolean M_CheckBottom (edict_t *ent);
qboolean M_CheckTop (edict_t *ent);
qboolean M_walkmove (edict_t *ent, float yaw, float dist);
void M_MoveToGoal (edict_t *ent, float dist);
float M_ChangeYaw (edict_t *ent);
void M_ChangePitch (edict_t *ent);
void M_MoveAwayFromGoal (edict_t *ent, float dist);

//
// g_phys.c
//

void G_RunEntity (edict_t *ent);

//
// g_main.c
//

void SaveClientData (void);
void FetchClientEntData (edict_t *ent);


//
// g_breakable.c
//
void KillBrush(edict_t *targ,edict_t *inflictor,edict_t *attacker,int damage);



//
// g_obj.c
//
void ObjectInit(edict_t *self,int health,int mass, int materialtype,int solid);


//
// g_spawnf.c
//

//sfs--this is used to get a classname for guys spawned while game is running
#ifdef __cplusplus
extern "C"
{
#endif
	char *ED_NewString (char *string);
	void ED_CallSpawn (edict_t *ent);
#ifdef __cplusplus
}
#endif

//============================================================================








// ************************************************************************************************
// TRYSTEP_
// --------
// Used for ai_trystep (g_ai)
// ************************************************************************************************

#define TRYSTEP_OK			0
#define TRYSTEP_ALLSOLID	1
#define TRYSTEP_STARTSOLID	2
#define TRYSTEP_OFFEDGE		3
#define TRYSTEP_NOSUPPORT	4
#define TRYSTEP_INWATER		5

// ************************************************************************************************
// client_respawn_t
// ----------------
// Client data that stays across deathmatch respawns.
// ************************************************************************************************

typedef struct
{
	client_persistant_t	coop_respawn;		// What to set 'client'->pers to on a respawn.
	int					enterframe;			// The level.framenum when the client entered the game.
	int					score;				// Frags, etc.
	vec3_t				cmd_angles;			// Angles sent over in the last command.
	int					game_helpchanged;
	int					helpchanged;
} client_respawn_t;

// ************************************************************************************************
// gclient_t
// ---------
// This structure is cleared on each PutClientInServer() except for 'client->pers'.
// ************************************************************************************************

typedef struct gclient_s
{
	// The following two fields are known to the server.

	player_state_t		ps;						// Communicated by server to clients.
	int					ping;

	// All other fields below are private to the game.
	
	client_respawn_t	resp;
	pmove_state_t		old_pmove;				// For detecting out-of-pmove changes.

	// Damage stuff. Sum up damage over an entire frame.

	qboolean			damage_gas;				// Did damage come from plague mist?
	int					damage_blood;			// Damage taken out of health.
	int					damage_knockback;		// Impact damage.
	vec3_t				damage_from;			// Origin for vector calculation.

	//

	usercmd_t			pcmd;
	short				oldcmdangles[3];
	vec3_t				aimangles;				// Spell / weapon aiming direction.
	vec3_t				oldviewangles;
	vec3_t				v_angle;				// Entity facing angles.
	float				bobtime;				// So off-ground doesn't change it.
	float				next_drown_time;
	int					old_waterlevel;

	// Client can respawn when time > respawn_time.

	float				respawn_time;
	int					complete_reset;

	//  Remote and walkby camera stuff.

	int					RemoteCameraLockCount;
	int					RemoteCameraNumber;
	int					savedtargetcount;
	edict_t				*savedtarget;

	// Teleport stuff.

	vec3_t				tele_dest;
	vec3_t				tele_angles;
	int					tele_count;
	int					tele_type;				/// Note only a byte of this is used.
	int					old_solid;

	// Weapon / defense stuff.

	edict_t				*lastentityhit;
	edict_t				*Meteors[4];
	vec3_t				laststaffpos;
	float				laststaffuse;

	// Powerup timers.

	float				invincible_framenum;

	// Shrine stuff.

	float				shrine_framenum;

	// Data for the player obituaries

	MOD_t				meansofdeath;	

	// Anti flooding vars

	float				flood_locktill;			// locked from talking
	float				flood_when[10];			// when messages were said
	int					flood_whenhead;			// head pointer for when said
	float				flood_nextnamechange;	// next time for valid nick change
	float				flood_nextkill;			// next time for suicide

	playerinfo_t		playerinfo;
} gclient_t;

#include "g_BoundingForm.h"
#include "g_Edict.h"
#ifdef __cplusplus	//this is for ds.cpp
#include "Vector.h"
#endif
qboolean FindTarget (edict_t *self);
void MG_PostDeathThink (edict_t *self);
qboolean movable (edict_t *ent);
qboolean EntReflecting(edict_t *ent, qboolean checkmonster, qboolean checkplayer);
void SkyFly (edict_t *self);

//For simplicity of use.. take it out later

#define BUOY_DEBUG	showbuoys->value
#define BUOY_DEBUG_LITE	showlitebuoys->value
#define MGAI_DEBUG	mgai_debug->value
#define DEACTIVATE_BUOYS deactivate_buoys->value
#define ANARCHY anarchy->value
#define IMPACT_DAMAGE impact_damage->value
#define CHEATING_MONSTERS cheating_monsters->value

// Scripts ds.cpp
#ifndef __cplusplus
	void ProcessScripts(void);
	void ShutdownScripts(qboolean Complete);
	void SaveScripts(FILE *FH, qboolean DoGlobals);
	void LoadScripts(FILE *FH, qboolean DoGlobals);
#endif

#endif // G_LOCAL_H

