//
// p_types.h
//
// Heretic II
// Copyright 1998 Raven Software
//

#ifndef _P_TYPES_H_
#define _P_TYPES_H_

#include "q_shared.h"
#include "q_clientserver.h"
#include "g_itemstats.h"

// Forward define 'playerinfo_t' for use in 'panimframe_t' and 'panimmove_t'.

typedef struct playerinfo_s playerinfo_t;

// ************************************************************************************************
// panimframe_t
// ------------
// ************************************************************************************************

typedef struct
{
	int		framenum;
	void	(*movefunc)(playerinfo_t *playerinfo,float var1,float var2,float var3);
	float	var1,var2,var3;
	void	(*actionfunc)(playerinfo_t *playerinfo,float var4);
	float	var4;
	void	(*thinkfunc)(playerinfo_t *playerinfo);
} panimframe_t;

// ************************************************************************************************
// panimmove_t
// -----------
// ************************************************************************************************

typedef struct
{
	int				numframes;
	panimframe_t	*frame;
	void			(*endfunc)(playerinfo_t *playerinfo);
} panimmove_t;

// ************************************************************************************************
// paceldata_t
// -----------
// ************************************************************************************************

typedef struct 
{
	panimmove_t	*move;
	short		fly;
	short		lockmove;
	int			playerflags;
} paceldata_t;

// ************************************************************************************************
// pacelsizes_t
// ------------
// ************************************************************************************************

typedef struct
{
	vec3_t	boundbox[2];
	int		altmove;
	float	viewheight;
	float	waterheight;
} pacelsizes_t;

// ************************************************************************************************
// gitem_armour_t
// --------------
// ************************************************************************************************

typedef struct
{
	float	max_armor;
	float	normal_protection;
	float	spell_protection;
} gitem_armor_t;

// ************************************************************************************************
// weaponready_e
// -------------
// Indicates what actual weapon model the player has readied.
// ************************************************************************************************

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

// ************************************************************************************************
// armortype_e
// -----------
// Indicates what actual armor the player is wearing.
// ************************************************************************************************

enum armortype_e
{
	ARMOR_TYPE_NONE,
	ARMOR_TYPE_SILVER,
	ARMOR_TYPE_GOLD
};

// ************************************************************************************************
// bowtype_e
// -----------
// Indicates what actual bow the player has currently on his back.
// ************************************************************************************************

enum bowtype_e
{
	BOW_TYPE_NONE,
	BOW_TYPE_REDRAIN,
	BOW_TYPE_PHOENIX
};

// ************************************************************************************************
// stafftype_e
// -----------
// Indicates what powerup level of the staff the player has.
// ************************************************************************************************

enum stafftype_e
{
	STAFF_LEVEL_NONE,
	STAFF_LEVEL_BASIC,
	STAFF_LEVEL_POWER1,
	STAFF_LEVEL_POWER2,
	STAFF_LEVEL_MAX
};

// ************************************************************************************************
// helltype_e
// -----------
// Indicates what powerup level of the staff the player has.
// ************************************************************************************************

enum helltype_e
{
	HELL_TYPE_NONE,
	HELL_TYPE_BASIC,
	HELL_TYPE_POWER
};

// ************************************************************************************************
// ammo_t
// ------
// ************************************************************************************************

typedef enum
{
	MODEL_HEALTH1,
	MODEL_HEALTH2,
	AMMO_BULLETS,
	AMMO_SHELLS,
	AMMO_ROCKETS,
	AMMO_GRENADES,
	AMMO_CELLS,
	AMMO_SLUGS
} ammo_t;

#define PICKUP_MIN  0, 0, 0
#define PICKUP_MAX  0, 0, 0

// ************************************************************************************************
// PNOISE_XXX
// ----------
// Noise types for 'PlayerNoise'.
// ************************************************************************************************

#define PNOISE_SELF		0
#define PNOISE_WEAPON	1
#define PNOISE_IMPACT	2


// ************************************************************************************************
// IT_XXX
// ------
// Held in 'gitem_t'->flags.
// ************************************************************************************************

#define	IT_WEAPON		1		// use makes active weapon
#define	IT_AMMO			2
#define IT_ARMOR		4
#define IT_STAY_COOP	8
#define IT_PUZZLE		16
#define IT_DEFENSE		32
#define IT_OFFENSE		64
#define IT_HEALTH		128

// ************************************************************************************************
// gitem_t
// -------
// ************************************************************************************************

typedef struct gitem_s
{
	// Spawning name.

	char		*classname;
	char		*pickup_name;

	short		msg_pickup;		// Pickup string id.
	short		msg_nouse;		// Can`t use.

	// Access function pointers.

	qboolean	(*pickup)(struct edict_s *ent, struct edict_s *other);
	void		(*use)(playerinfo_t *playerinfo, struct gitem_s *item);
	void		(*drop)(struct edict_s *ent, struct gitem_s *item);
	void		(*weaponthink)(struct edict_s *WeaponOwner,char *Format,...);
	
	char		*pickup_sound;
	char		*world_model;
	int			world_model_flags;

	vec3_t		mins;			// Bounding box
	vec3_t		maxs;			// Bounding box

	int			playeranimseq;	// The ASEQ_ player sequence that should be engaged when this item is used.
	int			altanimseq;		// Powerup animation sequence

	int			MaxActive;		// Maximum allowable active uses of items of this type by a single
								// player, at any instant in time. -1 indicates no limit.

	// Client side information.

	int			count_width;	// Number of digits to display by icon.

	int			quantity;		// For ammo, how much. For weapons, how much is used per shot.
	char		*ammo;			// For weapons
	int			flags;			// IT_XXX.

	void		*info;
	int			tag;

	char		*icon;
} gitem_t;

#ifdef PLAYER_DLL

extern PLAYER_API gitem_t *p_itemlist;
extern PLAYER_API int p_num_items;

extern PLAYER_API int GetItemIndex(gitem_t* x);
extern PLAYER_API gitem_t *GetItemByIndex(int index);
extern PLAYER_API gitem_t *FindItemByClassname(char *classname);
extern PLAYER_API gitem_t *FindItem(char *pickupname);
extern PLAYER_API void InitItems(void);

#define	ITEM_INDEX(x) GetItemIndex(x)

#else

extern int (*P_GetItemIndex)(gitem_t* x);
extern gitem_t* (*P_GetItemByIndex)(int index);
extern gitem_t* (*P_FindItemByClassname)(char *classname);
extern gitem_t* (*P_FindItem)(char *pickupname);
extern void (*P_InitItems)(void);

#define	ITEM_INDEX(x) P_GetItemIndex(x)

#endif	// PLAYER_DLL

// ************************************************************************************************
// inventory_t
// -----------
// Holds the players inventory.
// ************************************************************************************************

typedef struct inventory_s
{
	int	Items[MAX_ITEMS];		// No held of each item type.
	float Timer[MAX_ITEMS];		// Timer (if this item requires one).
} inventory_t;

// ************************************************************************************************
// client_persistant_t
// -------------------
// Client data that stays across multiple level loads.
// ************************************************************************************************

typedef struct
{
	// ********************************************************************************************
	// User info.
	// ********************************************************************************************

	char		userinfo[MAX_INFO_STRING];
	char		netname[16];
	char		sounddir[MAX_QPATH];
	int			autoweapon;

	// A loadgame will leave valid entities that just don't have a connection yet.

	qboolean	connected;

	// ********************************************************************************************
	// Values that are saved from and restored to 'edict_t's when changing levels.
	// ********************************************************************************************

	// Health.

	int			health;
	int			max_health;
	
	short		mission_num1;
	short		mission_num2;

	// Visible model attributes.

	int			weaponready;
	byte		armortype;			// Current armour Corvus is wearing.
	byte		bowtype;			// Current bow and what kind (when it is on Corvus' back too).
	byte		stafflevel;			// Current powerup level for the staff.
	byte		helltype;			// Current skin on the hellstaff.
	byte		handfxtype;			// Current spell effect Corvus has attached to his refpoints.
	float		armor_count; 		// Not used on client.
	short		skintype;			// Skin index that reflects plague stages and alternate skins
	unsigned int altparts;			// Missing hands, heads etc.
	
	// Inventory.

	inventory_t	inventory;
	inventory_t	old_inventory;
	int			selected_item;

	// Ammo capacities.

	int			max_offmana;
	int			max_defmana;
	int			max_redarrow;
	int			max_phoenarr;
	int			max_hellstaff;

	// Offenses and defenses.

	gitem_t		*weapon,*lastweapon,
				*defence,*lastdefence,
				*newweapon;

	// For calculating total unit score in co-op games.
	int			score;

} client_persistant_t;

// ************************************************************************************************
// FL_XXX
// ------
// Held in 'edict_t'->flags.
// ************************************************************************************************

#define	FL_FLY				0x00000001
#define	FL_SWIM				0x00000002	// implied immunity to drowining
#define FL_SUSPENDED		0x00000004
#define	FL_INWATER			0x00000008
#define	FL_GODMODE			0x00000010
#define	FL_NOTARGET			0x00000020
#define FL_IMMUNE_SLIME		0x00000040
#define FL_IMMUNE_LAVA		0x00000080
#define	FL_PARTIALGROUND	0x00000100	// not all corners are valid
#define	FL_INLAVA			0x00000200	// INWATER is set when in lava, but this is a modifier so we know when we leave
#define	FL_TEAMSLAVE		0x00000400	// not the first on the team
#define FL_NO_KNOCKBACK		0x00000800
#define	FL_INSLIME			0x00001000	// INWATER is set when in muck, but this is a modifier so we know when we leave
#define FL_LOCKMOVE			0x00002000	// Move updates should not process, actor can only move explicitly
#define FL_DONTANIMATE		0x00004000	// stop animating
#define FL_AVERAGE_CHICKEN	0x00008000	// Currently a chicken.
#define	FL_AMPHIBIAN		0x00010000	// Does not drown on land or in water, but is damaged by muck and lava
#define FL_SUPER_CHICKEN	0x00020000	// Ah yeah...
#define FL_RESPAWN			0x80000000	// used for item respawning

#define FL_CHICKEN			(FL_AVERAGE_CHICKEN | FL_SUPER_CHICKEN)

// ************************************************************************************************
// movetype_t
// ----------
// 'edict_t'->movetype values.
// ************************************************************************************************

typedef enum physicsType_e
{
	// new system
	PHYSICSTYPE_NONE,			// MOVETYPE_NONE			0
	PHYSICSTYPE_STATIC,	
	PHYSICSTYPE_NOCLIP,			// MOVETYPE_NOCLIP			2
	PHYSICSTYPE_FLY,			// MOVETYPE_FLY				3
	PHYSICSTYPE_STEP,			// MOVETYPE_WALK, MOVETYPE_STEP, MOVETYPE_TOSS, MOVETYPE_BOUNCE
	PHYSICSTYPE_PUSH,			// MOVETYPE_PUSH			5
	PHYSICSTYPE_STOP,			// MOVETYPE_STOP			6
	MOVETYPE_FLYMISSILE,		// MOVETYPE_FYMISSILE (extra size to monsters)
	PHYSICSTYPE_SCRIPT_ANGULAR,	// MOVETYPE_SCRIPT_ANGULAR	8
	NUM_PHYSICSTYPES,

	MOVETYPE_NONE,			// never moves
	MOVETYPE_NOCLIP,		// origin and angles change with no interaction
	MOVETYPE_PUSH,			// no clip to world, push on box contact
	MOVETYPE_STOP,			// no clip to world, stops on box contact
	MOVETYPE_WALK,			// gravity
	MOVETYPE_STEP,			// gravity, special edge handling
	MOVETYPE_FLY,
	MOVETYPE_TOSS,			// gravity
	MOVETYPE_BOUNCE,

	MOVETYPE_SCRIPT_ANGULAR,	// moves with the rotation of another entity
} physicsType_t;

// ************************************************************************************************
// DEAD_XXX
// --------
// Held in 'edict_t'->deadflag.
// ************************************************************************************************

typedef enum deadState_e
{
	DEAD_NO=0,
	DEAD_DYING,
	DEAD_DEAD,
	NUM_DEADSTATE
} deadState_t;

// ************************************************************************************************
// HANDFX_XXX
// ----------
// Hand effects, glowing for spells. Can be used for staff and bow, or others that are used by the
// upper torso half and toggle.
// ************************************************************************************************

typedef enum handfx_e
{
	HANDFX_NONE=0,		
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
	HANDFX_MAX,
} handfx_t;

// Unique sound IDs required for correct prediction of sounds played from player.dll. Each ID MUST
// be used ONCE only as it identifies the exact sound call in the player .dll that is responsible
// for playing a sound. These can be be compared with ID's recieved from the server sent sound
// packets (generated by running player.dll code on the server) so that we can decide whether to
// play the server sent sound or not (it may have already been played on the client by client
// prediction. IDs must never exeed +127.

enum
{
	SND_PRED_NULL	= -1,
	SND_PRED_ID0	=  0,
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
	SND_PRED_MAX		= 127
};

// Unique client-effect IDs required for correct prediction of effects started from player.dll. Each ID MUST
// be used ONCE only as it identifies the exact client-effects call in the player .dll that is responsible
// for starting a client-effect. These can be be compared with ID's recieved from the server sent client-effects
// (generated by running player.dll code on the server) so that we can decide whether to start the server sent
// client-effect or not (it may have already been started on the client by client prediction. IDs must never
// exeed +127.

enum
{
	EFFECT_PRED_NULL	= -1,
	EFFECT_PRED_ID0		=  0,
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
	EFFECT_PRED_MAX		=127
};

// ************************************************************************************************
// playerinfo_t
// ------------
// This is the information needed by the player animation system on both the client and server.
// ************************************************************************************************

typedef struct playerinfo_s
{
	// ********************************************************************************************
	// Inputs only.
	// ********************************************************************************************

	// Client side function callbacks (approximating functionality of server function callbacks).

	void (*CL_Sound)(byte EventId,vec3_t origin,int channel,char *soundname,float fvol,int attenuation,float timeofs);
	void (*CL_Trace)(vec3_t start,vec3_t mins,vec3_t maxs,vec3_t end,int brushmask,int flags,trace_t *trace);
	int (*CL_CreateEffect)(byte EventId,void *owner,unsigned short type,int flags,vec3_t position,char *format,...);
	void (*CL_RemoveEffects)(byte EventId,void *owner,int fx);
	
	// Server (game) function callbacks (approximating functionality of client-side function callbacks).

	void (*G_L_Sound)(edict_t *entity,int sound_num);
	void (*G_Sound)(byte EventId,float leveltime,edict_t *entity,int channel,int sound_num,float volume,float attenuation,float timeofs);
	void (*G_Trace)(vec3_t start,vec3_t mins,vec3_t maxs,vec3_t end,edict_t *passent,int contentmask,trace_t *trace);
	void (*G_CreateEffect)(byte EventId,entity_state_t *state,int type,int flags,vec3_t origin,char *format,...);
	void (*G_RemoveEffects)(byte Eventid,entity_state_t *state,int type);

	// Server (game) function callbacks that have no client side equivalent.
	
	int (*G_SoundIndex)(char *name);
	void (*G_SoundRemove)(char *name);
	void (*G_UseTargets)(edict_t *ent,edict_t *activator);
	entity_state_t *(*G_GetEntityStatePtr)(edict_t *entity);
	int (*G_BranchLwrClimbing)(playerinfo_t *playerinfo);
	qboolean (*G_PlayerActionCheckRopeGrab)(playerinfo_t *playerinfo, float stomp_org);
	void (*G_PlayerClimbingMoveFunc)(playerinfo_t *playerinfo, float height, float var2, float var3);
	qboolean (*G_PlayerActionCheckPuzzleGrab)(playerinfo_t *playerinfo);
	void (*G_PlayerActionTakePuzzle)(playerinfo_t *playerinfo);
	qboolean (*G_PlayerActionUsePuzzle)(playerinfo_t *playerinfo);
	qboolean (*G_PlayerActionCheckPushPull_Ent)(edict_t *ent);
	void (*G_PlayerActionMoveItem)(playerinfo_t *playerinfo,float distance);
	qboolean (*G_PlayerActionCheckPushButton)(playerinfo_t *playerinfo);
	void (*G_PlayerActionPushButton)(playerinfo_t *playerinfo);
	qboolean (*G_PlayerActionCheckPushLever)(playerinfo_t *playerinfo);
	void (*G_PlayerActionPushLever)(playerinfo_t *playerinfo);
	qboolean (*G_HandleTeleport)(playerinfo_t *playerinfo);
	void (*G_PlayerActionShrineEffect)(playerinfo_t *playerinfo);
	void (*G_PlayerActionChickenBite)(playerinfo_t *playerinfo);
	void (*G_PlayerFallingDamage)(playerinfo_t *playerinfo,float delta);
	void (*G_PlayerSpellShieldAttack)(playerinfo_t *playerinfo);
	void (*G_PlayerSpellStopShieldAttack)(playerinfo_t *playerinfo);
	void (*G_PlayerVaultKick)(playerinfo_t *playerinfo);
	void (*G_PlayerActionCheckRopeMove)(playerinfo_t *playerinfo);
	void (*G_gamemsg_centerprintf)(edict_t *ent, short msg);
	void (*G_levelmsg_centerprintf)(edict_t *ent, short msg);
	void (*G_WeapNext)(edict_t *ent);
	void (*G_UseItem)(edict_t *ent, char *s);

	// Common client & server (game) function callbacks.

	int (*PointContents)(vec3_t point);
	void (*SetJointAngles)(playerinfo_t *playerinfo);
	void (*ResetJointAngles)(playerinfo_t *playerinfo);
	void (*PlayerActionSwordAttack)(playerinfo_t *playerinfo,int value);
	void (*PlayerActionSpellFireball)(playerinfo_t *playerinfo);
	void (*PlayerActionSpellBlast)(playerinfo_t *playerinfo);
	void (*PlayerActionSpellArray)(playerinfo_t *playerinfo,int value);
	void (*PlayerActionSpellSphereCreate)(playerinfo_t *playerinfo,qboolean *Charging);
	void (*PlayerActionSpellFirewall)(playerinfo_t *playerinfo);
	void (*PlayerActionSpellBigBall)(playerinfo_t *playerinfo);
	void (*PlayerActionRedRainBowAttack)(playerinfo_t *playerinfo);
	void (*PlayerActionPhoenixBowAttack)(playerinfo_t *playerinfo);
	void (*PlayerActionHellstaffAttack)(playerinfo_t *playerinfo);
	void (*PlayerActionSpellDefensive)(playerinfo_t *playerinfo);
	qboolean (*G_EntIsAButton)(edict_t *ent);
	int (*irand)(playerinfo_t *playerinfo,int mn,int mx);
	
	// Indicates whether this playerinfo_t is held on the client or server.

	qboolean			isclient;

	// This is client only and records the highest level time the anim has run... we use this to
	// prevent multiple sounds etc. Logic is basically if(!ishistory) playsound...

	float				Highestleveltime;
	qboolean			ishistory;

	// Pointer to the associated player's edict_t.

	void				*self;

	// Game .dll variables.

	float				leveltime;
	float				quickturnEndTime;

	// Server variables.

	float				sv_gravity;
	float				sv_cinematicfreeze;		// Not used on client.
	float				sv_jumpcinematic;		// Jumping through cinematic. Not used on client.

	// From edict_t.

	float				ideal_yaw;
	void				*groundentity;

	// From pmove_t.

	csurface_t			*GroundSurface;
	cplane_t			GroundPlane;
	int					GroundContents;

	// Pointer to entity_state_t of player's enemy edict.

	entity_state_t		*enemystate;

	// Spell / weapon aiming direction (from g_client_t).

	vec3_t				aimangles;

	// Deathmatch flags.

	int	dmflags;

	// ********************************************************************************************
	// Inputs & outputs.
	// ********************************************************************************************

	// Data that must be maintatined over the duration of a level.

	client_persistant_t	pers;

	// Last usercmd_t.

	usercmd_t			pcmd;

	// Status of controller buttons.

	int					buttons;
	int					oldbuttons;
	int					latched_buttons;
	int					remember_buttons;

	// Weapons & defenses.

	qboolean			autoaim;				// Set on client from a flag.
	int					switchtoweapon;
	int					weap_ammo_index;
	int					def_ammo_index;			// Not used on client.
	int					weaponcharge;
	float				defensive_debounce;		// Used on client? Defensive spell delay.
	byte				meteor_count;

	// Visible model attributes.
	
	byte				plaguelevel;			// Current plague level: 0=none, 2=max.


	// Shrine stuff. Used by the player to determine the time for the torch to be lit, reflection
	// to work and invisibilty to work (xxx_timer).

	float				light_timer;			// Not used on client.
	float				reflect_timer;			// FIXME not transmitted yet.
	float				ghost_timer;			// FIXME not transmitted yet.
	float				powerup_timer;
	float				lungs_timer;			// Not used on client.
	float				shield_timer;			// FIXME not transmitted yet.
	float				speed_timer;			// FIXME not transmitted yet.
	float				block_timer;			// FIXME not transmitted yet.

	float				cinematic_starttime;	// Not used on client. Time cinematic started.
	float				cin_shield_timer;		// What the shield timer was set at the beginning of the cinematic
	int					c_mode;					// Show cinematics is on

	// Movement & animation.

	int					flags;
	float				fwdvel,sidevel,upvel;
	float				turncmd;
	float				waterheight;
	vec3_t				LastWatersplashPos;		// Not used on client.
	vec3_t				oldvelocity;
	qboolean			chargingspell;
	float				quickturn_rate;

	// From edict_t.

	vec3_t				origin;
	vec3_t				angles;
	vec3_t				velocity;
	vec3_t				mins,maxs;
	void				*enemy;					// Not used on client.
	void				*target;				// Not used on client.
	void				*target_ent;			// Not used on client.
	void				*targetEnt;				// FIXME - always 0 on client, but checked by client.
	float				nextthink;				// Not used on client.
	float				viewheight;
	float				knockbacktime;			// FIXME Used on client, but not transmitted yet?  --Pat
	int					watertype;
	int					waterlevel;	
	int					deadflag;
	int					movetype;
	int					edictflags;

	// From entity_state_t.

	int					frame,swapFrame;
	int					effects;
	int					renderfx;
	int					skinnum;
	int					clientnum;
	fmnodeinfo_t		fmnodeinfo[MAX_FM_MESH_NODES];

	// From pmove_state_t.

	int					pm_flags,pm_w_flags;

	// ********************************************************************************************
	// Outputs only.
	// ********************************************************************************************

	// From playerstate_t.

	vec3_t				offsetangles;

	qboolean			advancedstaff;

	// Torso angle twisting stuff which is derived entirely from various inputs to the animation
	// system.

	qboolean			headjointonly;
	vec3_t				targetjointangles;
	qboolean			showscores;				// Set layout stat.
	qboolean			showpuzzleinventory;	// Set layout stat.
	
	// ********************************************************************************************
	// Internal state info.
	// ********************************************************************************************

	int					seqcmd[20];
	panimmove_t			*uppermove,*lowermove;
	int					uppermove_index,lowermove_index;
	panimframe_t		*upperframeptr,*lowerframeptr;
	int					upperframe,lowerframe;
	qboolean			upperidle,loweridle;
	int					upperseq,lowerseq;
	float				idletime;
	vec3_t				grabloc;
	float				grabangle;

} playerinfo_t;

#endif // _P_TYPES_H_