//
// g_local.h -- local definitions for game module.
//
// Copyright 1998 Raven Software
//

#pragma once

#include "q_shared.h"
#include "q_ClientServer.h"
#include "buoy.h"

// Define GAME_INCLUDE so that game.h does not define the short, server-visible 'gclient_t' and 'edict_t' structures
// because we define the game versions in this file.
#define GAME_INCLUDE

#include "game.h"
#include "g_HitLocation.h" //mxd
#include "p_item.h" //mxd

// The "gameversion" client command will print this plus compile date.
#define GAMEVERSION		"Heretic2v16"

//mxd. Skill defines.
#define SKILL_EASY		0.0f
#define SKILL_MEDIUM	1.0f
#define SKILL_HARD		2.0f
#define SKILL_VERYHARD	3.0f

// Protocol bytes that can be directly added to messages.

// Volume mask for ent->sound_data - makes room for attn value in the lower bits.
#define ENT_VOL_MASK	0xf8

// Held in 'edict_t'->ai_mood. Used by higher level AI functions to relay states to lower functions.
#define AI_MOOD_NORMAL			0	// Not using any high level functionality (TEMP).
#define AI_MOOD_ATTACK			1	// Used in conjunction with ai_mood_flags to attack the target.
#define AI_MOOD_NAVIGATE		2	// Just walk towards the guide, ignoring everything else.
#define AI_MOOD_STAND			3	// Just stand there and wait to be advised.
#define AI_MOOD_PURSUE			4	// Run towards your enemy but don't attack.
#define AI_MOOD_FALLBACK		5	// Back away from your enemy, but face him.
#define AI_MOOD_DELAY			6	// Same as stand, but will allow interruption anywhere.
#define AI_MOOD_WANDER			7	// Wandering around buoy to buoy in a walk.
#define AI_MOOD_JUMP			8	// Jump towards goalentity.
#define AI_MOOD_REST			9	// The Ogle at rest.
#define AI_MOOD_POINT_NAVIGATE	10	// Navigate to a point, not an entity.
#define AI_MOOD_FLEE			11	// Run away!
#define AI_MOOD_BACKUP			12	// Backstep while attacking.
#define AI_MOOD_WALK			13	// Walking, no buoys.
#define AI_MOOD_EAT				14	// Sitting around, eating.

// Held in 'edict_t'->ai_mood_flags. Used in conjunction with ai_mood.
#define AI_MOOD_FLAG_MISSILE		0x00000001	// Check for a missile attack.
#define AI_MOOD_FLAG_MELEE			0x00000002	// Check for a melee attack.
#define AI_MOOD_FLAG_WHIP			0x00000004	// Check for a whipping attack (no damage).
#define AI_MOOD_FLAG_PREDICT		0x00000008	// Monster will predict movement on target.
#define AI_MOOD_FLAG_IGNORE			0x00000010	// Monster will ignore moods.
#define AI_MOOD_FLAG_FORCED_BUOY	0x00000020	// Monster will head towards it's forced_buoy.
#define AI_MOOD_FLAG_IGNORE_ENEMY	0x00000040	// Monster will ignore it's enemy unless attacked or otherwise directed.
#define AI_MOOD_FLAG_BACKSTAB		0x00000080	// Monster will advance on and attack enemy only from behind.
#define AI_MOOD_FLAG_DUMB_FLEE		0x00000100	// Monster will flee by simply running directly away from player.
#define AI_MOOD_FLAG_GOTO_FIXED		0x00000200	// Monster will become fixed upon getting to it's forced_buoy.
#define AI_MOOD_FLAG_GOTO_STAND		0x00000400	// Monster will stand upon getting to it's forced_buoy.
#define AI_MOOD_FLAG_GOTO_WANDER	0x00000800	// Monster will wander upon getting to it's forced_buoy.
#define AIMF_CANT_FIND_ENEMY		0x00001000	// Monster can't find enemy with buoys or vision. //TODO: consistent name?
#define AIMF_SEARCHING				0x00002000	// Monster now in dumb search mode... //TODO: consistent name?

// Held in 'edict_t'->spawnflags. These are set with checkboxes on each entity in the map editor.
#define SPAWNFLAG_NOT_EASY			0x00000100
#define SPAWNFLAG_NOT_MEDIUM		0x00000200
#define SPAWNFLAG_NOT_HARD			0x00000400
#define SPAWNFLAG_NOT_DEATHMATCH	0x00000800
#define SPAWNFLAG_NOT_COOP			0x00001000

// Timing constants that define the world heartbeat.
#define FRAMETIME			0.1f
#define THINK_NEVER			(-1.0f) //mxd
#define MONSTER_THINK_INC	0.099f
#define FRAMES_PER_SECOND	10.0f

// Memory tags to allow dynamic memory to be selectively cleaned up.
#define TAG_GAME	765	// Clear when unloading the dll.
#define TAG_LEVEL	766	// Clear when loading a savegame or a new level.

typedef enum damage_s
{
	DAMAGE_NO,
	DAMAGE_YES, // Will take damage if hit.
	DAMAGE_AIM, // Auto targeting recognizes this.
	DAMAGE_NO_RADIUS, // Will not take damage from radius blasts.
} damage_t;

// Damage flags.
#define DAMAGE_NORMAL				0x00000000	// No modifiers to damage.
#define DAMAGE_RADIUS				0x00000001	// Damage was indirect.
#define DAMAGE_NO_KNOCKBACK			0x00000002	// Do not affect velocity, just view angles.
#define DAMAGE_ALL_KNOCKBACK		0x00000004	// Ignore damage.
#define DAMAGE_EXTRA_KNOCKBACK		0x00000008	// Throw in some extra z.
#define DAMAGE_NO_PROTECTION		0x00000010	// Invulnerability and godmode have no effect.
#define DAMAGE_NO_BLOOD				0x00000020	// Don't spawn any blood.
#define DAMAGE_EXTRA_BLOOD			0x00000040	// Lots of blood.
#define DAMAGE_SPELL				0x00000080	// This came from a spell - for use in calcing armor effects.
#define DAMAGE_DISMEMBER			0x00000100	// Force this hit to use dismemberment message.
#define DAMAGE_ATTACKER_IMMUNE		0x00000200	// Inflictor receives no effect.
#define DAMAGE_ATTACKER_KNOCKBACK	0x00000400	// Inflictor takes knockback only.
#define DAMAGE_REDRAIN				0x00000800	// Red rain acid damage.
#define DAMAGE_BUBBLE				0x00001000	// Drowning damage.
#define DAMAGE_FIRE					0x00002000	// Fire damage.
#define DAMAGE_ALIVE_ONLY			0x00004000	// Only damage living things made of flesh.
#define DAMAGE_BLEEDING				0x00008000	// No protection.
#define DAMAGE_AVOID_ARMOR			0x00010000	// Don't do the armor effect.
#define DAMAGE_DOUBLE_DISMEMBER		0x00020000	// Force this hit to use dismemberment message with TWICE the chance of cutting.
#define DAMAGE_HURT_FRIENDLY		0x00040000	// Always hurt friendly entities (e.g. fellow coop players).
#define DAMAGE_POWERPHOENIX			0x00080000	// Extra knockback to shooter, 1/4 damage.
#define DAMAGE_FIRE_LINGER			0x00100000	// Do extra fire linger damage.
#define DAMAGE_ENEMY_MAX			0x00200000	// Do maximum damage directly to the enemy in radius.
#define DAMAGE_ONFIRE				0x00400000	// If the damage is FROM a fire...
#define DAMAGE_PHOENIX				0x00800000	// Phoenix-oriented damage. Do minimal fire for show, but short duration.

#define DAMAGE_SUFFOCATION			(DAMAGE_NO_KNOCKBACK | DAMAGE_NO_BLOOD | DAMAGE_BUBBLE | DAMAGE_AVOID_ARMOR)
#define DAMAGE_LAVA					(DAMAGE_NO_KNOCKBACK | DAMAGE_NO_BLOOD | DAMAGE_FIRE | DAMAGE_AVOID_ARMOR)
#define DAMAGE_SLIME				(DAMAGE_NO_KNOCKBACK | DAMAGE_NO_BLOOD | DAMAGE_AVOID_ARMOR)
#define DAMAGE_BURNING				(DAMAGE_NO_KNOCKBACK | DAMAGE_NO_BLOOD | DAMAGE_FIRE | DAMAGE_AVOID_ARMOR | DAMAGE_ONFIRE)

#define BODY_QUEUE_SIZE	8

// RANGE_XXX //TODO: convert to enum?
#define RANGE_MELEE	0
#define RANGE_NEAR	1
#define RANGE_MID	2
#define RANGE_FAR	3

#define MELEE_DISTANCE	80

// This structure is left intact through an entire game.
// It should be initialized at game.dll load time and read from / written to the 'server.ssv' file for savegames.
typedef struct
{
	// [maxclients].
	gclient_t* clients;

	// Needed for co-op respawns... can't store spawnpoint in level, because it would get overwritten by the savegame restore.
	char spawnpoint[512];

	// Store latched cvars that we want to get at often here.
	int maxclients;
	int maxentities;

	// Updated every frame in DM, for pick-up and shrine respawn times.
	int num_clients;

	// Cross-level triggers.
	int serverflags;

	// Items.
	int num_items;

	// Flag that we've autosaved.
	qboolean autosaved; //TODO: set, but never used.
	qboolean entitiesSpawned;
} game_locals_t;

// This structure is used for alert entities, which are spawned a lot.
#define MAX_ALERT_ENTS	1024

typedef struct alertent_s alertent_t;
struct alertent_s
{
	alertent_t* next_alert;
	alertent_t* prev_alert;
	edict_t* enemy;
	vec3_t origin;
	qboolean inuse;
	int alert_svflags;
	float lifetime;
};

// This structure is cleared as each map is entered. It is read/written to the 'level.sav' file for savegames.
typedef struct
{
	int framenum;
	float time;

	char level_name[MAX_QPATH];	// The descriptive name (e.g. 'Outer Base').
	char mapname[MAX_QPATH];	// The server name (e.g. 'base1').
	char nextmap[MAX_QPATH];	// Go here when fraglimit is hit.

	// Intermission state information.
	float intermissiontime;		// Time the intermission was started.
	char* changemap;
	qboolean exitintermission; //mxd. int in original logic.
	vec3_t intermission_origin;
	vec3_t intermission_angle;

	edict_t* sight_client;		// Changed once each frame for coop games.

	edict_t* sight_entity;
	int sight_entity_framenum;

	float far_clip_dist_f;
	float fog;
	float fog_density;

	edict_t* current_entity; // Entity running from G_RunFrame().
	int body_que; // Dead bodies.

	buoy_t buoy_list[MAX_MAP_BUOYS]; // Buoy information for this map.
	int active_buoys;	// Number of actual buoys on the level.
	int fucked_buoys;	// Number of buoys that can't be fixed.
	int fixed_buoys;	// Number of buoys that had to be fixed.

	int player_buoy[MAX_CLIENTS];		// Stores current bestbuoy for a player enemy (if any).
	int player_last_buoy[MAX_CLIENTS];	// When player_buoy is invalid, saves it here so monsters can check it first instead of having to do a whole search.

	int offensive_weapons;
	int defensive_weapons;

	alertent_t alertents[MAX_ALERT_ENTS]; // All the alert ents on the map.
	int num_alert_ents;			// Number of actual alert entities on the level.
	alertent_t* alert_entity;	// The alert entity linked list start.
	alertent_t* last_alert;		// The last entity in alert entity linked list.

	qboolean fighting_beast; // Fighting a beast, do extra checks with trace instant weapons.
} level_locals_t;

// Means of death.
typedef enum
{
	MOD_UNKNOWN,

	MOD_STAFF,
	MOD_FIREBALL,
	MOD_MMISSILE,
	MOD_SPHERE,
	MOD_SPHERE_SPL,
	MOD_IRONDOOM,
	MOD_FIREWALL,
	MOD_STORM,
	MOD_PHOENIX,
	MOD_PHOENIX_SPL,
	MOD_HELLSTAFF,

	MOD_P_STAFF,
	MOD_P_FIREBALL,
	MOD_P_MMISSILE,
	MOD_P_SPHERE,
	MOD_P_SPHERE_SPL,
	MOD_P_IRONDOOM,
	MOD_P_FIREWALL,
	MOD_P_STORM,
	MOD_P_PHOENIX,
	MOD_P_PHOENIX_SPL,
	MOD_P_HELLSTAFF,

	MOD_KICKED,
	MOD_METEORS,
	MOD_ROR,
	MOD_SHIELD,
	MOD_CHICKEN,
	MOD_TELEFRAG,
	MOD_WATER,
	MOD_SLIME,
	MOD_LAVA,
	MOD_CRUSH,
	MOD_FALLING,
	MOD_SUICIDE,
	MOD_BARREL,
	MOD_EXIT,
	MOD_BURNT,
	MOD_BLEED,
	MOD_SPEAR,
	MOD_DIED,
	MOD_KILLED_SLF,
	MOD_DECAP,
	MOD_TORN,

	MOD_MAX
} MOD_t;

#define MOD_FRIENDLY_FIRE	0x8000000

// This is used to hold entity field values that can be set from the editor, but aren't actually present in 'edict_t' during gameplay.
typedef struct
{
	// Sky variables.
	char* sky;
	float skyrotate;
	vec3_t skyaxis;

	char* nextmap;

	int lip;
	int distance;
	int height;
	char* noise;
	float pausetime;
	char* item;
	char* gravity;

	float minyaw;
	float maxyaw;
	float minpitch;
	float maxpitch;
	int rotate;
	float zangle;
	char* file;
	int radius;

	// Weapons to be given to the player on spawning.
	int offensive;
	int defensive;
	int spawnflags2;

	// Time to wait (in seconds) for all clients to have joined a map in coop.
	int cooptimeout;

	// Scripting stuff.
	char* script;
	char* parms[16];
} spawn_temp_t;

// This is used to hold information pertaining to an entity's movement.
// NOTE: mxd. Can't change struct size, otherwise compatibility with original game dlls will break!
typedef struct
{
	// Fixed data.
	vec3_t start_origin;
	vec3_t start_angles;
	vec3_t end_origin;
	vec3_t end_angles;

	int sound_start;
	int sound_middle;
	int sound_end;

	float accel;
	float speed;
	float decel;
	float distance;

	float wait;

	// State data.
	int state;
	vec3_t dir;
	float current_speed;
	float move_speed;
	float next_speed;
	float remaining_distance;
	float decel_distance;
	void (*endfunc)(edict_t*);
} moveinfo_t;

// Monster AI flags.
#define AI_STAND_GROUND			0x00000001
#define AI_TEMP_STAND_GROUND	0x00000002
#define AI_SOUND_TARGET			0x00000004 //TODO: used, but never set?
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
#define AI_COWARD				0x00020000	// Babies (FLEE to certain distance & WATCH).
#define AI_AGRESSIVE			0x00040000	// Never run away.
#define AI_SHOVE				0x00080000	// Shove others out of the way.
#define AI_DONT_THINK			0x00100000	// Animate, don't think or move.
#define AI_SWIM_OK				0x00200000	// Ok to go in water.
#define AI_OVERRIDE_GUIDE		0x00400000	
#define AI_NO_MELEE				0x00800000	// Not allowed to melee.
#define AI_NO_MISSILE			0x01000000	// Not allowed to missile.
#define AI_USING_BUOYS			0x02000000	// Using Buoyah! Navigation System(tm).
#define AI_STRAIGHT_TO_ENEMY	0x04000000	// Charge straight at enemy no matter what anything else tells you.
#define AI_NIGHTVISION			0x08000000	// Light level does not effect this monster's vision or aim.
#define AI_NO_ALERT				0x10000000	// Monster does not pay attention to alerts.

// Monster attack states.
#define AS_STRAIGHT				1
#define AS_SLIDING				2
#define	AS_MELEE				3
#define	AS_MISSILE				4
#define	AS_DIVING				5

// Cinematic animation flags.
#define C_ANIM_MOVE				1
#define C_ANIM_REPEAT			2
#define C_ANIM_DONE				4
#define C_ANIM_IDLE				8

// Type of target acquisition.
#define SIGHT_SOUND_TARGET		0 // Heard the target make this noise. //TODO: set, but never used by sight processing message logic.
#define SIGHT_VISIBLE_TARGET	1 // Saw this target. //TODO: set, but never used by sight processing message logic.
#define SIGHT_ANNOUNCED_TARGET	2 // Target was announced by another monster. //TODO: unused.

typedef struct
{
	const int framenum;
	void (*const movefunc)(edict_t* self, float var1, float var2, float var3);
	const float var1;
	const float var2;
	const float var3;
	void (*const actionfunc)(edict_t* self, float var4);
	const float var4;
	void (*const thinkfunc)(edict_t* self);
} animframe_t;

#define ANIMMOVE(arr, endfunc)	{ ARRAY_SIZE(arr), arr, endfunc } //mxd. animmove_t initializer macro. Added, so we don't have to type numframes manually.

typedef struct
{
	const int numframes;
	const animframe_t* frame;
	void (*const endfunc)(edict_t* self);
} animmove_t;

// NOTE: mxd. Can't change struct size, otherwise compatibility with original game dlls will break!
typedef struct
{
	// Not used in new system.
	char* otherenemyname; // ClassName of secondary enemy (other than player). E.g. a Rat's secondary enemy is a gib.

	const animmove_t* currentmove;
	int aiflags;
	int aistate;		// Last order given to the monster (ORD_XXX).
	int currframeindex;	// Index to current monster frame.
	int nextframeindex;	// Used to force the next frameindex.
	float thinkinc;		// Time between thinks for this entity.
	float scale;

	void (*idle)(edict_t* self); //TODO: used, but never assigned?
	void (*search)(edict_t* self); //TODO: unused.
	void (*dodge)(edict_t* self, edict_t* other, float eta); //TODO: unused.
	int (*attack)(edict_t* self); //TODO: unused.
	void (*sight)(edict_t* self, edict_t* other); //TODO: unused.
	void (*dismember)(edict_t* self, int damage, HitLocation_t hl); //mxd. Changed 'hl' arg type from int.
	qboolean (*alert)(edict_t* self, alertent_t* alerter, edict_t* enemy);
	qboolean (*checkattack)(edict_t* self);

	float pausetime;
	float attack_finished;

	union
	{
		float flee_finished; // When a monster is done fleeing.
		qboolean morcalavin_quake_finished; //mxd
		float rope_player_current_swing_speed; //mxd
	};

	union
	{
		float chase_finished;	// When the monster can look for secondary monsters.
		float rope_player_initial_swing_speed; //mxd
	};

	union
	{
		vec3_t saved_goal;
		vec3_t rope_player_swing_direction; //mxd
	};

	union
	{
		float search_time;
		float priestess_attack_delay; //mxd
		float rope_sound_debounce_time; //mxd
	};
	
	float misc_debounce_time;
	vec3_t last_sighting;
	int attack_state;

	union
	{
		int lefty;
		int morcalavin_taunt_counter; //mxd
	};
	
	float idle_time;
	int linkcount;

	int searchType;
	vec3_t nav_goal;

	union
	{
		float jump_time;
		float morcalavin_teleport_attack_time; //mxd
		float ogle_sing_time; //mxd
		float rope_jump_debounce_time; // Delay after jumping from rope before trying to grab another rope -- mxd.
	};

	int morcalavin_battle_phase; //mxd. Named 'stepState' in original logic.
	int ogleflags;			// Ogles have special spawnflags stored in here at spawntime.
	int supporters;			// Number of supporting monsters (with common type) in the area when awoken.
	float sound_finished;	// Amount of time until the monster will be finishing talking (used for voices).

	union
	{
		float sound_start; // The amount of time to wait before playing the pending sound.
		float morcalavin_taunt_time; //mxd
	};
	
	int sound_pending;		// This monster is waiting to make a sound (used for voices) (0 if false, else sound ID).

	// Cinematic fields.
	int c_dist;		// Distance left to move.
	int c_repeat;	// # of times to repeat the anim cycle.
	void (*c_callback)(struct edict_s* self); // Callback function when action is done. Used only by script system --mxd.
	int c_anim_flag;	// Shows if current cinematic anim supports moving, turning, or repeating.
	qboolean c_mode;	// In cinematic mode or not?
	edict_t* c_ent;		// Entity passed from a cinematic command.

	qboolean awake;		// Has found an enemy AND gone after it.
	qboolean roared;	// Gorgon has roared or been woken up by a roar.

	float last_successful_enemy_tracking_time; // Last time successfully saw enemy or found a path to him.
	float coop_check_debounce_time;
} monsterinfo_t;

// The structure for each monster class.
#define FOFS(x)		((int)&(((edict_t*)0)->x))
#define STOFS(x)	((int)&(((spawn_temp_t*)0)->x))
#define LLOFS(x)	((int)&(((level_locals_t*)0)->x))
#define CLOFS(x)	((int)&(((gclient_t*)0)->x))
#define BYOFS(x)	((int)&(((buoy_t*)0)->x))

extern game_locals_t game;

#ifdef __cplusplus
extern "C"
{
#endif
	extern level_locals_t level;
	extern edict_t* g_edicts;
	extern game_import_t gi;
	extern spawn_temp_t st;
	extern game_export_t globals;
#ifdef __cplusplus
}
#endif

extern int sm_meat_index; //TODO: mxd. Never set -> always 0!

extern cvar_t* maxentities;
extern cvar_t* deathmatch;
extern cvar_t* coop;
extern cvar_t* dmflags;
extern cvar_t* advancedstaff;
extern cvar_t* cl_autoaim; //mxd
extern cvar_t* skill;
extern cvar_t* fraglimit;
extern cvar_t* timelimit;
extern cvar_t* password;
extern cvar_t* dedicated;
extern cvar_t* filterban;

extern cvar_t* sv_gravity;
extern cvar_t* sv_friction;
extern cvar_t* sv_maxvelocity;

extern cvar_t* sv_cheats;
extern cvar_t* sv_nomonsters;
extern cvar_t* blood_level;
extern cvar_t* deactivate_buoys;
extern cvar_t* anarchy;
extern cvar_t* impact_damage;
extern cvar_t* cheating_monsters;
extern cvar_t* singing_ogles;
extern cvar_t* no_runshrine;
extern cvar_t* no_tornado;
extern cvar_t* no_irondoom;
extern cvar_t* no_phoenix;
extern cvar_t* no_morph;
extern cvar_t* no_shield;
extern cvar_t* no_teleport;
extern cvar_t* log_file_name;
extern cvar_t* log_file_header;
extern cvar_t* log_file_footer;
extern cvar_t* log_file_line_header;

#ifdef __cplusplus
extern "C"
{
#endif
	extern cvar_t* sv_cinematicfreeze;
	extern cvar_t* sv_jumpcinematic;
#ifdef __cplusplus
}
#endif

extern cvar_t* sv_freezemonsters;

extern cvar_t* maxclients;
extern cvar_t* sv_maplist;

extern cvar_t* allowillegalskins;

extern cvar_t* dm_no_bodies;

extern cvar_t* player_dll;

extern cvar_t* flood_msgs;
extern cvar_t* flood_persecond;
extern cvar_t* flood_waitdelay;
extern cvar_t* flood_killdelay;

//mxd. For simplicity of use...
#define FLOOD_MSGS			((int)flood_msgs->value)
#define BLOOD_LEVEL			((int)blood_level->value)
#define MAXCLIENTS			((int)maxclients->value)
#define COOP				((int)coop->value)
#define DEATHMATCH			((int)deathmatch->value)
#define DMFLAGS				((int)dmflags->value)
#define FRAGLIMIT			((int)fraglimit->value)
#define SKILL				((int)skill->value)
#define DEDICATED			((int)dedicated->value)
#define SV_CINEMATICFREEZE	((int)sv_cinematicfreeze->value)
#define SV_FREEZEMONSTERS	((int)sv_freezemonsters->value)
#define SV_CHEATS			((int)sv_cheats->value)

extern qboolean self_spawn; //mxd. int -> qboolean.
#define world (&g_edicts[0])

#define DROPPED_ITEM		0x00008000

// g_utils.c
extern void KillBox(edict_t* ent); //mxd. qboolean KillBox in original logic (always returned true).
extern void G_ProjectSource(const vec3_t point, const vec3_t distance, const vec3_t forward, const vec3_t right, vec3_t result);
extern qboolean PossessCorrectItem(const edict_t* ent, const gitem_t* item); //mxd

#ifdef __cplusplus
extern "C"
{
#endif
	// g_utils.c funcs used by cs_CScript.cpp...
	extern edict_t* G_Find(edict_t* from, int fieldofs, const char* match);
	extern edict_t* G_Spawn(void);

	//mxd. Utilities.c funcs used ONLY by cs_CScript.cpp...
	extern void RemoveNonCinematicEntities(void);
	extern void ReinstateNonCinematicEntities(void);
#ifdef __cplusplus
}
#endif

extern edict_t* FindInRadius_Old(edict_t* from, vec3_t org, float radius);
extern edict_t* FindInRadius(edict_t* from, const vec3_t org, float radius);
extern edict_t* FindInBlocking(edict_t* from, const edict_t* check_ent);
extern edict_t* FindInBounds(const edict_t* from, const vec3_t min, const vec3_t max);
extern edict_t* G_PickTarget(const char* targetname);

// Commonly used functions.
extern void G_UseTargets(edict_t* ent, edict_t* activator);
extern void G_SetMovedir(vec3_t angles, vec3_t movedir);
extern void G_InitEdict(edict_t* self);
extern void G_FreeEdict(edict_t* self);
extern void G_SetToFree(edict_t* self);
extern void G_TouchTriggers(edict_t* ent);
extern void G_LinkMissile(edict_t* self);
extern char* vtos(const vec3_t v);
extern float VectorYaw(const vec3_t v);
extern void VectorRotate(const vec3_t in, float yaw_deg, vec3_t out); //mxd

// g_svcmds.c
extern void ServerCommand(void);
extern qboolean SV_FilterPacket(const char* from);

// g_phys.c
extern void G_RunEntity(edict_t* ent);

// g_spawnf.c
//sfs--this is used to get a classname for guys spawned while game is running
#ifdef __cplusplus
extern "C"
{
#endif
	extern char* ED_NewString(const char* string);
	extern void ED_CallSpawn(edict_t* ent);
#ifdef __cplusplus
}
#endif

// Client data that stays across deathmatch respawns.
typedef struct
{
	client_persistant_t coop_respawn; // What to set 'client'->pers to on a respawn. //TODO: set, but never used?
	int enterframe; // The level.framenum when the client entered the game.
	int score; // Frags, etc.
	vec3_t cmd_angles; // Angles sent over in the last command.
} client_respawn_t;

// This structure is cleared on each PutClientInServer() except for 'client->pers'.
typedef struct gclient_s
{
	// The following two fields are known to the server.
	player_state_t ps; // Communicated by server to clients.
	int ping;

	// All other fields below are private to the game.
	client_respawn_t resp;
	pmove_state_t old_pmove; // For detecting out-of-pmove changes.

	// Damage stuff. Sum up damage over an entire frame.
	qboolean damage_gas;	// Did damage come from plague mist?
	int damage_blood;		// Damage taken out of health.
	int damage_knockback;	// Impact damage.
	vec3_t damage_from;		// Origin for vector calculation.

	usercmd_t pcmd;
	short oldcmdangles[3];
	vec3_t aimangles; // Spell / weapon aiming direction.
	vec3_t oldviewangles;
	vec3_t v_angle; // Entity facing angles.
	float bobtime; // So off-ground doesn't change it.
	float next_drown_time;
	int old_waterlevel;

	// Client can respawn when time > respawn_time.
	float respawn_time;
	int complete_reset; //TODO: change type to qboolean.

	// Remote and walkby camera stuff.
	int RemoteCameraLockCount;
	int RemoteCameraNumber;
	int savedtargetcount; //TODO: unused.
	edict_t* savedtarget; //TODO: unused.

	// Teleport stuff.
	vec3_t tele_dest;
	vec3_t tele_angles;
	int tele_count;
	int tele_type; // Note: only a byte of this is used.
	int old_solid; //TODO: set, but never used.

	// Weapon / defense stuff.
	edict_t* lastentityhit;
	edict_t* Meteors[4];
	vec3_t laststaffpos;
	float laststaffuse;

	// Powerup timers.
	float invincible_framenum;

	// Shrine stuff.
	float shrine_framenum;

	// Data for the player obituaries.
	MOD_t meansofdeath;

	// Anti flooding vars
	float flood_locktill;		// Locked from talking.
	float flood_when[10];		// When messages were said.
	int flood_whenhead;			// Head pointer for when said.
	float flood_nextnamechange;	// Next time for valid nick change.
	float flood_nextkill;		// Next time for suicide.

	playerinfo_t playerinfo;
} gclient_t;

#include "g_Edict.h"

extern qboolean FindTarget(edict_t* self);

// For simplicity of use... take it out later. //TODO: mxd. Take it out... later?
#define DEACTIVATE_BUOYS	deactivate_buoys->value
#define ANARCHY				anarchy->value
#define IMPACT_DAMAGE		impact_damage->value
#define CHEATING_MONSTERS	cheating_monsters->value