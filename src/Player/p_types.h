//
// p_types.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "q_shared.h"
#include "q_clientserver.h"

// Forward define 'playerinfo_t' for use in 'panimframe_t' and 'panimmove_t'.
typedef struct playerinfo_s playerinfo_t;

typedef struct
{
	int framenum;
	void (*movefunc)(playerinfo_t* playerinfo, float var1, float var2, float var3);
	float var1;
	float var2;
	float var3;
	void (*actionfunc)(playerinfo_t* playerinfo, float var4);
	float var4;
	void (*thinkfunc)(playerinfo_t* playerinfo);
} panimframe_t;

#define PANIMMOVE(arr, endfunc)	{ ARRAY_SIZE(arr), arr, endfunc } //mxd. panimmove_t initializer macro. Added, so we don't have to type numframes manually.

typedef struct
{
	const int numframes;
	const panimframe_t* frame;
	void (*const endfunc)(playerinfo_t* playerinfo);
} panimmove_t;

typedef struct // Initialized in PlayerSeqData[] and PlayerChickenData[].
{
	panimmove_t* move;
	short fly;
	short lockmove;
	int playerflags;
} paceldata_t;

typedef struct
{
	float max_armor;
	float normal_protection;
	float spell_protection;
} gitem_armor_t;

// Indicates what actual weapon model the player has readied.
enum weaponready_e
{
	WEAPON_READY_NONE,
	WEAPON_READY_HANDS,
	WEAPON_READY_STAFFSTUB,
	WEAPON_READY_SWORDSTAFF,
	WEAPON_READY_HELLSTAFF,	
	WEAPON_READY_BOW,

	WEAPON_READY_MAX
};

// Indicates what actual armor the player is wearing.
enum armortype_e
{
	ARMOR_TYPE_NONE,
	ARMOR_TYPE_SILVER,
	ARMOR_TYPE_GOLD
};

// Indicates what actual bow the player has currently on his back.
enum bowtype_e
{
	BOW_TYPE_NONE,
	BOW_TYPE_REDRAIN,
	BOW_TYPE_PHOENIX
};

// Indicates what powerup level of the staff the player has.
enum stafftype_e
{
	STAFF_LEVEL_NONE,
	STAFF_LEVEL_BASIC,
	STAFF_LEVEL_POWER1,
	STAFF_LEVEL_POWER2,
	STAFF_LEVEL_MAX
};

// Indicates what powerup level of the hellstaff the player has.
enum helltype_e
{
	HELL_TYPE_NONE,
	HELL_TYPE_BASIC,
	HELL_TYPE_POWER
};

// Held in gitem_t.flags.
#define IT_WEAPON		1 // Use makes active weapon
#define IT_AMMO			2
#define IT_ARMOR		4 //TODO: used, but never set?
//#define IT_STAY_COOP	8 //mxd. Unused.
#define IT_PUZZLE		16
#define IT_DEFENSE		32
#define IT_OFFENSE		64
#define IT_HEALTH		128

typedef struct gitem_s
{
	// Spawning name.
	char* classname;
	char* pickup_name;

	short msg_pickup;	// Pickup string id.
	short msg_nouse;	// Can`t use.

	// Access function pointers.
	qboolean (*pickup)(const struct edict_s* ent, struct edict_s* other);
	void (*use)(playerinfo_t* playerinfo, struct gitem_s* item);
	void (*drop)(struct edict_s* ent, struct gitem_s* item);
	void (*weaponthink)(struct edict_s* WeaponOwner, char* Format, ...);

	char* pickup_sound;

	vec3_t mins; // Bounding box.
	vec3_t maxs; // Bounding box.

	int playeranimseq;	// The ASEQ_ player sequence that should be engaged when this item is used.
	int altanimseq;		// Powerup animation sequence.

	// Client side information.
	int count_width; // Number of digits to display by icon.

	int quantity; // For ammo, how much. For weapons, how much is used per shot.
	char* ammo; // For weapons
	int flags; // IT_XXX.

	int tag;
	char* icon;
} gitem_t;

#ifdef PLAYER_DLL
	extern PLAYER_API gitem_t* p_itemlist;
	extern PLAYER_API int p_num_items;

	extern PLAYER_API int GetItemIndex(const gitem_t* item);
	extern PLAYER_API gitem_t* GetItemByIndex(int index);
	extern PLAYER_API gitem_t* FindItemByClassname(const char* classname);
	extern PLAYER_API gitem_t* FindItem(const char* pickup_name);
	extern PLAYER_API void InitItems(void);

	#define ITEM_INDEX(x) GetItemIndex(x)
#else
	extern int (*P_GetItemIndex)(const gitem_t* item);
	extern gitem_t* (*P_GetItemByIndex)(int index);
	extern gitem_t* (*P_FindItemByClassname)(const char* classname);
	extern gitem_t* (*P_FindItem)(const char* pickup_name);
	extern void (*P_InitItems)(void);

	#define ITEM_INDEX(x) P_GetItemIndex(x)
#endif	// PLAYER_DLL

// Holds the players inventory.
typedef struct inventory_s
{
	int Items[MAX_ITEMS]; // No held of each item type.
	float Timer[MAX_ITEMS]; // Timer (if this item requires one).
} inventory_t;

#define PLAYER_HEALTH	100 //mxd. Used by gamex86 and client.

// Client data that stays across multiple level loads.
typedef struct client_persistant_s
{
	// User info.
	char userinfo[MAX_INFO_STRING];
	char netname[16];
	char sounddir[MAX_QPATH]; //TODO: unused?
	int autoweapon; //TODO: change type to qboolean.

	// A loadgame will leave valid entities that just don't have a connection yet.
	qboolean connected;

	// Values that are saved from and restored to 'edict_t's when changing levels.

	// Health.
	int health;
	int max_health;

	short mission_num1;
	short mission_num2;

	// Visible model attributes.
	int weaponready;
	byte armortype;		// Current armour Corvus is wearing.
	byte bowtype;		// Current bow and what kind (when it is on Corvus' back too).
	byte stafflevel;	// Current powerup level for the staff.
	byte helltype;		// Current skin on the hellstaff.
	byte handfxtype;	// Current spell effect Corvus has attached to his refpoints.
	float armor_count;	// Not used on client.
	short skintype;		// Skin index that reflects plague stages and alternate skins.
	uint altparts;		// Missing hands, heads etc.

	// Inventory.
	inventory_t inventory;
	inventory_t old_inventory;
	int selected_item;

	// Ammo capacities.
	int max_offmana;
	int max_defmana;
	int max_redarrow;
	int max_phoenarr;
	int max_hellstaff;

	// Offenses and defenses.
	gitem_t* weapon;
	gitem_t* lastweapon;
	gitem_t* defence; //mxd. Named 'defense' in player_state_t //TODO: rename to match?
	gitem_t* lastdefence; //mxd. Named 'lastdefense' in player_state_t //TODO: rename to match?
	gitem_t* newweapon;

	// For calculating total unit score in co-op games.
	int score;
} client_persistant_t;

// FL_XXX
// Held in 'edict_t'->flags.
#define FL_FLY				0x00000001
#define FL_SWIM				0x00000002	// Implied immunity to drowining.
#define FL_SUSPENDED		0x00000004
#define FL_INWATER			0x00000008
#define FL_GODMODE			0x00000010
#define FL_NOTARGET			0x00000020
#define FL_IMMUNE_SLIME		0x00000040
#define FL_IMMUNE_LAVA		0x00000080
#define FL_PARTIALGROUND	0x00000100	// Not all corners are valid.
#define FL_INLAVA			0x00000200	// INWATER is set when in lava, but this is a modifier so we know when we leave.
#define FL_TEAMSLAVE		0x00000400	// Not the first on the team.
#define FL_NO_KNOCKBACK		0x00000800
#define FL_INSLIME			0x00001000	// INWATER is set when in muck, but this is a modifier so we know when we leave.
#define FL_LOCKMOVE			0x00002000	// Move updates should not process, actor can only move explicitly.
#define FL_DONTANIMATE		0x00004000	// Stop animating. //TODO: set, but never used?
#define FL_AVERAGE_CHICKEN	0x00008000	// Currently a chicken.
#define FL_AMPHIBIAN		0x00010000	// Does not drown on land or in water, but is damaged by muck and lava.
#define FL_SUPER_CHICKEN	0x00020000	// Ah yeah...
#define FL_RESPAWN			0x80000000	// Used for item respawning.

#define FL_CHICKEN (FL_AVERAGE_CHICKEN | FL_SUPER_CHICKEN)

// 'edict_t'->movetype values.
typedef enum physicsType_e
{
	PHYSICSTYPE_NONE,
	PHYSICSTYPE_STATIC,
	PHYSICSTYPE_NOCLIP,
	PHYSICSTYPE_FLY,
	PHYSICSTYPE_STEP,
	PHYSICSTYPE_PUSH,
	PHYSICSTYPE_STOP,
	MOVETYPE_FLYMISSILE,
	PHYSICSTYPE_SCRIPT_ANGULAR,

	NUM_PHYSICSTYPES
} physicsType_t;

// DEAD_XXX
// Held in 'edict_t'->dead_state.
typedef enum deadState_e
{
	DEAD_NO,
	DEAD_DYING,
	DEAD_DEAD
} deadState_t;

// HANDFX_XXX
// Hand effects, glowing for spells. Can be used for staff and bow, or others that are used by the upper torso half and toggle.
typedef enum handfx_e
{
	HANDFX_NONE,
	HANDFX_FIREBALL,
	HANDFX_MISSILE,
	HANDFX_SPHERE,
	HANDFX_MACEBALL,
	HANDFX_FIREWALL,
	HANDFX_REDRAIN,
	HANDFX_POWERREDRAIN,
	HANDFX_PHOENIX,
	HANDFX_POWERPHOENIX,
	HANDFX_STAFF1,
	HANDFX_STAFF2,
	HANDFX_STAFF3,

	HANDFX_MAX
} handfx_t;

// Unique sound IDs required for correct prediction of sounds played from player.dll.
// Each ID MUST be used ONCE only as it identifies the exact sound call in the player .dll that is responsible for playing a sound.
// These can be be compared with ID's received from the server sent sound packets (generated by running player.dll code on the server)
// so that we can decide whether to play the server sent sound or not (it may have already been played on the client by client prediction.
// IDs must never exceed +127.
enum
{
	SND_PRED_NULL	= -1,
	SND_PRED_ID0	= 0,
	SND_PRED_ID1,
	SND_PRED_ID2,
	SND_PRED_ID3,
	SND_PRED_ID4,	// Unused.
	SND_PRED_ID5,
	SND_PRED_ID6,
	SND_PRED_ID7,
	SND_PRED_ID8,
	SND_PRED_ID9,
	SND_PRED_ID10,
	SND_PRED_ID11,
	SND_PRED_ID12,
	SND_PRED_ID13,
	SND_PRED_ID14,
	SND_PRED_ID15,
	SND_PRED_ID16,
	SND_PRED_ID17,
	SND_PRED_ID18,
	SND_PRED_ID19,
	SND_PRED_ID20,
	SND_PRED_ID21,
	SND_PRED_ID22,
	SND_PRED_ID23,
	SND_PRED_ID24,
	SND_PRED_ID25,
	SND_PRED_ID26,
	SND_PRED_ID27,
	SND_PRED_ID28,
	SND_PRED_ID29,
	SND_PRED_ID30,
	SND_PRED_ID31,
	SND_PRED_ID32,
	SND_PRED_ID33,
	SND_PRED_ID34,
	SND_PRED_ID35,
	SND_PRED_ID36,
	SND_PRED_ID37,
	SND_PRED_ID38,
	SND_PRED_ID39,
	SND_PRED_ID40,
	SND_PRED_ID41,
	SND_PRED_ID42,
	SND_PRED_ID43,
	SND_PRED_ID44,
	SND_PRED_ID45,
	SND_PRED_ID46,
	SND_PRED_ID47,
	SND_PRED_ID48,
	SND_PRED_ID49,
	SND_PRED_ID50,
	SND_PRED_ID51,
	SND_PRED_ID52,
	SND_PRED_ID53,

	SND_PRED_MAX	= 127
};

// Unique client-effect IDs required for correct prediction of effects started from player.dll.
// Each ID MUST be used ONCE only as it identifies the exact client-effects call in the player .dll that is responsible for starting a client-effect.
// These can be be compared with ID's received from the server sent client-effects (generated by running player.dll code on the server)
// so that we can decide whether to start the server sent client-effect or not (it may have already been started on the client by client prediction.
// IDs must never exceed +127.
enum
{
	EFFECT_PRED_NULL	= -1,
	EFFECT_PRED_ID0		= 0,
	EFFECT_PRED_ID1,
	EFFECT_PRED_ID2,
	EFFECT_PRED_ID3,
	EFFECT_PRED_ID4,
	EFFECT_PRED_ID5,
	EFFECT_PRED_ID6,
	EFFECT_PRED_ID7,
	EFFECT_PRED_ID8,
	EFFECT_PRED_ID9,
	EFFECT_PRED_ID10,
	EFFECT_PRED_ID11,
	EFFECT_PRED_ID12,
	EFFECT_PRED_ID13,
	EFFECT_PRED_ID14,
	EFFECT_PRED_ID15,
	EFFECT_PRED_ID16,
	EFFECT_PRED_ID17,
	EFFECT_PRED_ID18,
	EFFECT_PRED_ID19,
	EFFECT_PRED_ID20,
	EFFECT_PRED_ID21,
	EFFECT_PRED_ID22,
	EFFECT_PRED_ID23,
	EFFECT_PRED_ID24,
	EFFECT_PRED_ID25,
	EFFECT_PRED_ID26,
	EFFECT_PRED_ID27,
	EFFECT_PRED_ID28,
	EFFECT_PRED_ID29,
	EFFECT_PRED_ID30,
	EFFECT_PRED_ID31,
	EFFECT_PRED_ID32,
	EFFECT_PRED_ID33,
	EFFECT_PRED_ID34,

	EFFECT_PRED_MAX		= 127
};

// playerinfo_t
// This is the information needed by the player animation system on both the client and server.
typedef struct playerinfo_s
{
	// Inputs only.

	// Client side function callbacks (approximating functionality of server function callbacks).
	void (*CL_Sound)(byte event_id, const vec3_t origin, int channel, const char* soundname, float fvol, int attenuation, float timeofs);
	void (*CL_Trace)(const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int brushmask, int flags, trace_t* trace);
	int (*CL_CreateEffect)(byte event_id, const void* owner, ushort type, int flags, const vec3_t position, const char* format, ...);
	void (*CL_RemoveEffects)(byte event_id, const void* owner, int fx);
	
	// Server (game) function callbacks (approximating functionality of client-side function callbacks).
	void (*G_L_Sound)(edict_t* entity, int sound_num);
	void (*G_Sound)(byte event_id, float leveltime, const edict_t* entity, int channel, int sound_num, float volume, float attenuation, float timeofs);
	void (*G_Trace)(const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, const edict_t* passent, uint contentmask, trace_t* trace);
	void (*G_CreateEffect)(byte event_id, entity_state_t* state, int type, int flags, const vec3_t origin, const char* format, ...);
	void (*G_RemoveEffects)(byte event_id, entity_state_t* state, int type);

	// Server (game) function callbacks that have no client side equivalent.
	int (*G_SoundIndex)(const char* name);
	void (*G_SoundRemove)(const char* name);
	void (*G_UseTargets)(edict_t* ent, edict_t* activator);
	entity_state_t* (*G_GetEntityStatePtr)(edict_t* entity);
	int (*G_BranchLwrClimbing)(playerinfo_t* playerinfo);
	qboolean (*G_PlayerActionCheckRopeGrab)(playerinfo_t* playerinfo, float stomp_org);
	void (*G_PlayerClimbingMoveFunc)(playerinfo_t* playerinfo, float height, float var2, float var3);
	qboolean (*G_PlayerActionCheckPuzzleGrab)(playerinfo_t* playerinfo);
	void (*G_PlayerActionTakePuzzle)(const playerinfo_t* playerinfo);
	qboolean (*G_PlayerActionUsePuzzle)(const playerinfo_t* playerinfo);
	qboolean (*G_PlayerActionCheckPushPull_Ent)(const edict_t* ent);
	void (*G_PlayerActionMoveItem)(const playerinfo_t* playerinfo, float distance);
	qboolean (*G_PlayerActionCheckPushButton)(const playerinfo_t* playerinfo);
	void (*G_PlayerActionPushButton)(const playerinfo_t* playerinfo);
	qboolean (*G_PlayerActionCheckPushLever)(const playerinfo_t* playerinfo);
	void (*G_PlayerActionPushLever)(const playerinfo_t* playerinfo);
	qboolean (*G_HandleTeleport)(const playerinfo_t* playerinfo);
	void (*G_PlayerActionShrineEffect)(const playerinfo_t* playerinfo);
	void (*G_PlayerActionChickenBite)(const playerinfo_t* playerinfo);
	void (*G_PlayerFallingDamage)(const playerinfo_t* playerinfo, float delta);
	void (*G_PlayerSpellShieldAttack)(const playerinfo_t* playerinfo);
	void (*G_PlayerSpellStopShieldAttack)(const playerinfo_t* playerinfo);
	void (*G_PlayerVaultKick)(const playerinfo_t* playerinfo);
	void (*G_PlayerActionCheckRopeMove)(playerinfo_t* playerinfo);
	void (*G_gamemsg_centerprintf)(const edict_t* ent, short msg);
	void (*G_levelmsg_centerprintf)(const edict_t* ent, short msg);
	void (*G_WeapNext)(const edict_t* ent);
	void (*G_UseItem)(edict_t* ent, char* s);

	// Common client & server (game) function callbacks.
	int (*PointContents)(const vec3_t point);
	void (*SetJointAngles)(const playerinfo_t* playerinfo);
	void (*ResetJointAngles)(const playerinfo_t* playerinfo);
	void (*PlayerActionSwordAttack)(const playerinfo_t* playerinfo, int value);
	void (*PlayerActionSpellFireball)(const playerinfo_t* playerinfo);
	void (*PlayerActionSpellBlast)(const playerinfo_t* playerinfo);
	void (*PlayerActionSpellArray)(const playerinfo_t* playerinfo, int value);
	void (*PlayerActionSpellSphereCreate)(const playerinfo_t* playerinfo, qboolean* charging);
	void (*PlayerActionSpellFirewall)(const playerinfo_t* playerinfo);
	void (*PlayerActionSpellBigBall)(const playerinfo_t* playerinfo);
	void (*PlayerActionRedRainBowAttack)(const playerinfo_t* playerinfo);
	void (*PlayerActionPhoenixBowAttack)(const playerinfo_t* playerinfo);
	void (*PlayerActionHellstaffAttack)(const playerinfo_t* playerinfo);
	void (*PlayerActionSpellDefensive)(playerinfo_t* playerinfo);
	qboolean (*G_EntIsAButton)(const edict_t* ent);
	int (*irand)(const playerinfo_t* playerinfo, int min, int max);
	
	// Indicates whether this playerinfo_t is held on the client or server.
	qboolean isclient;

	// This is client only and records the highest level time the anim has run...
	// We use this to prevent multiple sounds etc. Logic is basically if(!ishistory) playsound...
	float Highestleveltime;
	qboolean ishistory;

	// Pointer to the associated player's edict_t.
	void* self;

	// Game .dll variables.
	float leveltime;
	float quickturnEndTime;

	// Server variables.
	float sv_gravity;
	float sv_cinematicfreeze; // Not used on client.
	float sv_jumpcinematic; // Jumping through cinematic. Not used on client.

	// From edict_t.
	float ideal_yaw;
	void* groundentity;

	// From pmove_t.
	csurface_t* GroundSurface;
	cplane_t GroundPlane;
	int GroundContents;

	entity_state_t* enemystate; // Pointer to entity_state_t of player's enemy edict.
	vec3_t aimangles; // Spell / weapon aiming direction (from g_client_t).
	int dmflags; // Deathmatch flags.

	// Inputs & outputs.

	client_persistant_t pers; // Data that must be maintained over the duration of a level.
	usercmd_t pcmd; // Last usercmd_t.

	// Status of controller buttons.
	int buttons;
	int oldbuttons;
	int latched_buttons;
	int remember_buttons;

	// Weapons & defenses.
	qboolean autoaim; // Set on client from a flag.
	int switchtoweapon;
	int weap_ammo_index;
	int def_ammo_index; // Not used on client.
	int weaponcharge;
	float defensive_debounce; // Used on client? Defensive spell delay.
	byte meteor_count;

	// Visible model attributes.
	byte plaguelevel; // Current plague level: 0=none, 2=max.

	// Shrine stuff. Used by the player to determine the time for the torch to be lit, reflection to work and invisibility to work (xxx_timer).
	float light_timer;		// Not used on client.
	float reflect_timer;	// FIXME not transmitted yet.
	float ghost_timer;		// FIXME not transmitted yet.
	float powerup_timer;
	float lungs_timer;		// Not used on client.
	float shield_timer;		// FIXME not transmitted yet.
	float speed_timer;		// FIXME not transmitted yet.
	float block_timer;		// FIXME not transmitted yet.

	float cinematic_starttime;	// Not used on client. Time cinematic started.
	float cin_shield_timer;		// What the shield timer was set at the beginning of the cinematic.
	int c_mode;					// Show cinematics is on.

	// Movement & animation.
	int flags;
	float fwdvel;
	float sidevel;
	float upvel;
	float turncmd;
	float waterheight;
	vec3_t LastWatersplashPos; // Not used on client. //TODO: only Z-coord is used.
	vec3_t oldvelocity;
	qboolean chargingspell;
	float quickturn_rate;		// Rotational velocity (degrees/second).

	// From edict_t.
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;
	vec3_t mins;
	vec3_t maxs;
	void* enemy;		// Not used on client.
	void* target;		// Not used on client.
	void* target_ent;	// Not used on client.
	void* targetEnt;	// FIXME - always 0 on client, but checked by client.
	float nextthink;	// Not used on client.
	float viewheight;
	float knockbacktime; // FIXME Used on client, but not transmitted yet?  --Pat
	int watertype;
	int waterlevel;	
	int deadflag; //TODO: rename to dead_state, change type to deadState_t.
	int movetype;
	int edictflags;

	// From entity_state_t.
	int frame;
	int swapFrame;
	int effects;
	int renderfx;
	int skinnum;
	int clientnum;
	fmnodeinfo_t fmnodeinfo[MAX_FM_MESH_NODES];

	// From pmove_state_t.
	int pm_flags;
	int pm_w_flags;

	// Outputs only.

	// From playerstate_t.
	vec3_t offsetangles;
	qboolean advancedstaff;

	// Torso angle twisting stuff which is derived entirely from various inputs to the animation system.
	qboolean headjointonly;
	vec3_t targetjointangles;

	qboolean showscores; // Set layout stat.
	qboolean showpuzzleinventory; // Set layout stat.

	// Internal state info.
	int seqcmd[20]; // == ACMD_MAX
	panimmove_t* uppermove;
	panimmove_t* lowermove;
	int uppermove_index; // Sent across network --mxd. //TODO: no longer used for anything.
	int lowermove_index; // Sent across network --mxd. //TODO: no longer used for anything.
	panimframe_t* upperframeptr;
	panimframe_t* lowerframeptr;
	int upperframe;
	int lowerframe;
	qboolean upperidle;
	qboolean loweridle;
	int upperseq; // Sent across network --mxd.
	int lowerseq; // Sent across network --mxd.
	float idletime;
	vec3_t grabloc;
	float grabangle;
} playerinfo_t;