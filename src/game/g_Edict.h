//
// g_Edict.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "g_ClassStatics.h"
#include "g_Message.h"
#include "Message.h"

#define MAX_BUOY_BRANCHES	3

#ifdef __cplusplus
	class CScript;
#endif

struct edict_s
{
	// This is sent to the server as part of each client frame.
	entity_state_t s;

	// NULL if not a player. The server expects the first part of a 'gclient_s' to be a 'player_state_t' but the rest of it is opaque.
	struct gclient_s* client;

	// House keeping information not used by the game logic.
	qboolean inuse;
	int just_deleted;	// Used to delete stuff entities properly on the client.
	int client_sent;	// Used to delete stuff entities properly on the client.
	int linkcount;

	// FIXME: move these fields to a server private sv_entity_t
	link_t area; // Linked to a division node or leaf.
	int num_clusters; // If -1, use headnode instead.
	int clusternums[MAX_ENT_CLUSTERS];
	int headnode; // Unused if num_clusters is -1.
	int areanum;
	int areanum2;

	int svflags;

	edict_t* groundentity;		// Entity serving as ground.
	int groundentity_linkcount;	// If self and groundentity's don't match, groundentity should be cleared.
	vec3_t groundNormal;		// Normal of the ground.

	vec3_t intentMins; // If PF_RESIZE is set, then physics will attempt to change the ents bounding form to the new one indicated.
	vec3_t intentMaxs; // If it was successfully resized, the PF_RESIZE is turned off otherwise it will remain on.  

	solid_t solid;
	int clipmask;

	edict_t* owner;

	vec3_t mins;
	vec3_t maxs;
	vec3_t absmin;
	vec3_t absmax;
	vec3_t size;

	// Called when self is the collidee in a collision, resulting in the impediment or bouncing of trace->ent.
	void (*isBlocking)(edict_t* self, struct trace_s* trace);

	// DO NOT MODIFY ANYTHING ABOVE THIS! THE SERVER EXPECTS THE FIELDS IN THAT ORDER!
	// All the fields below this are used by the game only and can be re-arranged, modified etc.
	// =================================================================================================================

	MsgQueue_t msgQ;
	G_MessageHandler_t msgHandler;
	enum ClassID_e classID; //mxd. int in original version.

	void (*think)(edict_t* self);
	void (*ai)(edict_t* self);
	int flags; //TODO: change to uint?
	float freetime; // Server time when the object was freed.
	char* classname;
	int spawnflags; //TODO: change to uint?

	// Used by the game physics.
	physicsType_t movetype; //mxd. int in original logic.
	int physicsFlags; //TODO: change to uint?

	edict_t* blockingEntity; //TODO: set, but never used!
	int blockingEntity_linkcount; // If self and blockingEntity's don't match, blockingEntity should be cleared. //TODO: set, but never used!
	vec3_t blockingNormal; // Normal of the blocking surface. //TODO: set, but never used!

	// Called when self bounces off of something and continues to move unimpeded.
	void (*bounced)(edict_t* self, struct trace_s* trace);

	// Called when self is the collider in a collision, resulting in the impediment of self's movement.
	void (*isBlocked)(edict_t* self, struct trace_s* trace);

	float friction;		// Friction multiplier; defaults to 1.0.
	float elasticity;	// Used to determine whether something will stop, slide, or bounce on impact.

	// Used by anything that can collide (physics).
	void (*touch)(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf);

	// Used to indicate teams (a way to group things).
	char* team; // Team name.

	union
	{
		edict_t* teamchain;
		edict_t* targetEnt;
		edict_t* slavechain; //TODO: unused.
		edict_t* rope_grab; //Used to by the rope to hold the part of the rope which the player is holding.
	};

	union
	{
		edict_t* guidemaster; //TODO: unused.
		edict_t* teammaster;
	};

	float nextthink;

	// Fields used by only one class of game entity (monster, player, poly, trigger, etc).
	// =================================================================================================================

	char* model;	// Model name.
	char* message;	// Index to message_text for printing and wave files.
	char* text_msg;	// Text printed to con for door, polys, triggers, etc.

	// These really all could be changed to ints or hashed or something (currently, we do a search on all the active edicts using strcmps).
	// We should be able to assign indexes in the BSP, by doing the string strcmps at BSP time.
	// The player seem to use any of the target stuff.

	char* target;		// Name of the entity's target.
	char* targetname;	// What other entities use to target-lock this entity.
	char* scripttarget;
	char* killtarget;	// Used only in G_UseTargets, which fires all of its targets.
	char* pathtarget;	// Used by trains, also used by monsters in some way as well.

	union
	{
		char* deathtarget; // Target to be triggered on my death.
		char* pathtargetname; // Used to target buoys to doors or plats or etc.
	};

	union
	{
		char* combattarget;	// Monsters are the primary user of this.
		char* jumptarget;	// For buoys only.
	};

	union
	{
		edict_t* target_ent; // Used by player, trains, and turrets. Monsters should be able to use this for current target as well.
		edict_t* slave; //TODO: used ONLY to store next rope segment by rope logic. Rename?
		edict_t* rope_end; // Used by the rope to store the rope end entity.
	};

	vec3_t movedir; // Used by just about everything that moves, but not used in physics.

	float air_finished; // Used by things that can breath (monsters and player).

	edict_t* goalentity; // Used primarily by monsters.
	edict_t* movetarget; // Used by monsters and poly/triggers.

	float yaw_speed;	// Used by monsters and player.
	float ideal_yaw;	// Used by monsters and player.
	float ideal_pitch;	// Used by monsters and player. //TODO: used, but never set?
	float yawOffset;	// Used in CreateMove_Step //TODO: used, but never set?

	float accel; // Used mostly in g_func.c.
	float decel; // Used mostly in g_func.c.

	float timestamp; // Used by a couple of objects.

	// Used by just about every type of entity.
	void (*use)(edict_t* self, edict_t* other, edict_t* activator);

	int health;		// Used by anything that can be destroyed.
	int max_health;	// Used by anything that can be destroyed.
	int bloodType;	// Type of stuff to spawn off when hit. //TODO: never used for intended purpose!

	union
	{
		int deadflag; // More like a dead state, used by things that can die. Would probably be better off with a more general state.
		int deadState;
	};

	union
	{
		qboolean show_hostile; // Only used by monsters (or g_ai.c at least) - not really sure what for.
		void (*TriggerActivated)(edict_t* self, edict_t* activator);
	};

	char* map;

	int viewheight; // Height above origin where eyesight is determined used by anything which can "see", player and monsters.
	float reflected_time; // Used by objects to tell if they've been repulsed by something.

	damage_t takedamage; //mxd. int in original logic.

	// Unless something will do both normal and radius damage, we only need one field.
	// In fact we may want to move this into class statics or something.

	int dmg; // The damage something does.
	float dmg_radius; // The radius of damage.

	int sounds; // Used by a trigger and a splash, could be a class static.

	union
	{
		int count; // Used by polys, triggers and items.
		int curr_model; // Used by player during cinematics.
	};

	int targeted; // Used by Ogle to denote a targeted action queued up.
	int lastbuoy; // Used to save a buoy in checking.

	edict_t* chain;		// Used by items and player in the body queue.
	edict_t* enemy;		// Used by monsters, player, and a poly or two.
	edict_t* oldenemy;	// Used by monsters.
	edict_t* activator;	// Entity that used something, used by monsters, items, and polys.

	// Used by player only.

	edict_t* mynoise; // Can go in client only. //TODO: unused!
	edict_t* mynoise2; //TODO: unused!

	edict_t* last_buoyed_enemy; // Used by monsters.

	int noise_index; // Used only by targets.
	float volume; //TODO: hijacked by m_beast, never used for intended purpose.

	union
	{
		float attenuation;	// Used only by target_speaker. //TODO: unused!
		float maxrange;		// Used for ai.
	};

	// Timing variables.

	float wait;		// Used by polys, triggers and targets.
	float delay;	// Delay before firing targets. Used by a few polys and targets.
	float random;	// Used by func_timer and spl_meteorbarrier.

	// Used to delay monster 5 before going after a player sound. Only set on player.
	union
	{
		float teleport_time; //TODO: unused!
		float time; // misc. time for whatever
	};

	// Move these to clientinfo?

	int light_level; // Set on player, checked by monsters.
	int style; // Also used as areaportal number used by items.
	gitem_t* item; // For bonus items. Used by player, triggers, and monsters.

	// What it's made of, i.e. MAT_XXX. Used to determine gibs to throw.
	// Curently used only by the barrel, but applicable to anything generically gibbable.

	MaterialID_t materialtype; //mxd. int in original logic.
	int PersistantCFX; // Index to client effect linked to edict.
	int Leader_PersistantCFX; // None of this should really go in here.. Really it should be in the client, but its 2 in the morning, so fuck it.

	vec3_t velocity; // Linear velocity.
	vec3_t avelocity; // Angular velocity.
	vec3_t knockbackvel;

	// Used for determining effects of liquids in the environment.

	float speed;
	int watertype;	// Used to indicate current liquid actor is in.
	int waterlevel;	// Used by monsters and players.

	int mass;
	float gravity; // Per-entity gravity multiplier (1.0 is normal). Used for lowgrav artifact, flares. //mxd. Lowgrav artifact?

	// Not currently used by anyone, but it's a part of physics. Probably should remove it.
	void (*prethink) (edict_t* ent); //TODO: unused. Remove?

	// Move into the moveinfo structure? Used by polys and turret and in physics. //mxd. Turret??
	void (*blocked)(edict_t* self, edict_t* other);

	// Used by animating entities.
	int curAnimID;
	int lastAnimID; //TODO: set, but never used.

	// Used by monsters and player.
	int (*pain)(edict_t* self, edict_t* other, float kick, int damage);

	// Used by monsters, player, and some polys.
	int (*die)(edict_t* self, edict_t* inflictor, edict_t* attacker, int damage, vec3_t point);

	// Used by the Morph Ovum and Chicken.
	void (*oldthink)(edict_t* self);

	float touch_debounce_time;	// Used by polys and triggers.
	float pain_debounce_time;	// Used by monsters and player.
	float damage_debounce_time;	// Used by monsters and player.
	float attack_debounce_time;	// Used by monsters.
	int reflect_debounce_time;	// Used by reflecting projectiles.
	float impact_debounce_time;	// Impact damage debounce time.

	float fire_damage_time;	// Fire damage length.
	float fire_timestamp;	// Timestamp weapons and damaged entities, so that the same weapon can't hurt an entity twice.

	// Used by shrines.
	void (*oldtouch)(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf);

	union
	{
		int shrine_type;
		int morph_timer;
		int buoy_index;
		qboolean* sphere_charging_ptr; //mxd. Used by Sphere of Annihilation spell.
	};

	// Only set in trigger_push_touch and probably only on players.
	union
	{
		float last_buoy_time;
		float fly_sound_debounce_time; //TODO: unused.
	};

	union
	{
		float last_move_time; // Only used by target_earthquake (poly/trigger).
		float old_yaw; // Used by the Seraph to return to his exact position and angles.
	};

	vec3_t pos1;
	vec3_t pos2;

	// Common data blocks.
	moveinfo_t moveinfo;		// 120 bytes
	monsterinfo_t monsterinfo;	// 156 bytes

	vec3_t v_angle_ofs; // View angle offset - for when monsters look around, for line of sight checks.

	int ai_mood;		// Used by high level ai to relay simple moods to lower level functions (INTEGRAL FOR SWITCH).
	int ai_mood_flags;	// Used in conjunction with ai_mood to provide more information to the lower functions.
	byte mintel; // Number of buoys allowed to follow.

	char* target2;
	vec3_t last_org;

	// Next 4 in a table in m_stats.
	float min_melee_range;	// Min. distance at which it is ok for this monster to use it's melee attack.
	float melee_range;		// Max. distance to do a melee attack, if negative, closest monster should get to enemy.

	float min_missile_range;	// Min. distance to do a missile attack.
	float missile_range;		// Max. distance to do a missile attack.
	int bypass_missile_chance;	// Chance to not use missile attack even if you can (0-100).

	// Called only when monster cannot attack player.
	void (*cant_attack_think)(edict_t* self, float enemydist, qboolean enemyvis, qboolean enemyinfront);

	int jump_chance; // Chance to jump when have opportunity.
	float wakeup_distance; // How far the player can be when I see him to wake me up.

	float evade_debounce_time; // How long to evade for.
	float oldenemy_debounce_time; // How long to hunt enemy before looking for oldenemy again.

	float best_move_yaw;

	float mood_nextthink;
	void (*mood_think)(edict_t* self); // Your mood setting function.

	float next_pre_think; // Any prethinking you want a monster to do.
	void (*pre_think)(edict_t* self); // Nextthink time for prethinks.

	float next_post_think; // Any postthinking you want a monster to do.
	void (*post_think)(edict_t* self); // Nextthink time for postthinks.

	int forced_buoy; // Monster is forced to go to this buoy.
	buoy_t* enemy_buoy; // Monster's enemy's closest buoy. //TODO: unused!
	float pathfind_nextthink;
	edict_t* nextbuoy[MAX_BUOY_BRANCHES];

	float dead_size; // For dead thinking.

	// New monster stuff.
	char* wakeup_target;	// Target to fire when find an enemy.
	char* pain_target;		// Target to fire when take pain (only once). //TODO: used in DefaultMsgHandler(), but never set!
	char* homebuoy;

	float alert_time; // Time when a monster is no longer startled.
	alertent_t* last_alert; // Last alert_ent to startle me, if it's the same one, skip it when going through the list.

	edict_t* placeholder; // Used by assassin to hold his teleport destination.
	float jump_time; // Time that a monster's protection from falling damage runs out after a jump.
	int red_rain_count; // Number of red rains you can have at once.
	int deathtype; // How you died. //TODO: unused!
	edict_t* fire_damage_enemy; // Who burnt you to death - for proper burning death credit.

#ifndef __cplusplus
	void* Script;
#else
	CScript* Script;
#endif
};