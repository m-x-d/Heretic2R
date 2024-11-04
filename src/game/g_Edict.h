#ifndef G_EDICT_H
#define G_EDICT_H

#include "g_Message.h"
#include "Message.h"
#include "g_ClassStatics.h"
#include "g_NewSystem.h"

#define MAX_BUOY_BRANCHES 3

// ************************************************************************************************
// edict_s
// -------
// ************************************************************************************************
#ifndef __cplusplus
#else
	class CScript;
#endif

struct edict_s
{
	// This is sent to the server as part of each client frame.

	entity_state_t		s;

	// NULL if not a player. The server expects the first part of a 'gclient_s' to be a
	// 'player_state_t' but the rest of it is opaque.

	struct gclient_s	*client;

	// House keeping information not used by the game logic.

	qboolean			inuse;
	int					just_deleted;			// used to delete stuff entities properly on the client
	int					client_sent;			// used to delete stuff entities properly on the client
	int					linkcount;

	// FIXME: move these fields to a server private sv_entity_t

	link_t				area;				// Linked to a division node or leaf.
	int					num_clusters;		// If -1, use headnode instead.
	int					clusternums[MAX_ENT_CLUSTERS];
	int					headnode;			// Unused if num_clusters is -1.
	int					areanum,areanum2;

	int					svflags;

	edict_t				*groundentity;			// entity serving as ground
	int					groundentity_linkcount;	// if self and groundentity's don't match, groundentity should be cleared
	vec3_t				groundNormal;			// normal of the ground

	vec3_t				intentMins, intentMaxs;	// if PF_RESIZE is set, then physics will attempt to change
												// the ents bounding form to the new one indicated
												// If it was succesfully resized, the PF_RESIZE is turned off
												// otherwise it will remain on.

	solid_t				solid;
	int					clipmask;
	
	edict_t				*owner;

	vec3_t				mins, maxs;
	vec3_t				absmin,absmax,size;

	// called when self is the collidee in a collision, resulting in the impediment or bouncing of trace->ent
	void				(*isBlocking)(edict_t *self, struct trace_s *trace);

	// DO NOT MODIFY ANYTHING ABOVE THIS! THE SERVER EXPECTS THE FIELDS IN THAT ORDER! All the
	// fields below this are are used by the game only and can be re-aranged, modified etc.

	// ============================================================================================

	MsgQueue_t			msgQ;
	G_MessageHandler_t	msgHandler;
	int					classID;

	void				(*think)(edict_t *self);
	void				(*ai)(edict_t *self);
	int					flags;
	float				freetime;			// Server time when the object was freed.
	char				*classname;
	int					spawnflags;

	// Used by the game physics.

	int					movetype;

	int					physicsFlags;

	edict_t				*blockingEntity;			// entity serving as ground
	int					blockingEntity_linkcount;	// if self and blockingEntity's don't match, blockingEntity should be
													// cleared
	vec3_t				blockingNormal;				// normal of the blocking surface

	// called when self bounces off of something and continues to move unimpeded
	void				(*bounced)(edict_t *self, struct trace_s *trace);

	// called when self is the collider in a collision, resulting in the impediment of self's movement
	void				(*isBlocked)(edict_t *self, struct trace_s *trace);

	float				friction;		// friction multiplier; defaults to 1.0
	// Used to determine whether something will stop, slide, or bounce on impact
	float				elasticity;

	// Used by anything that can collide (physics).

	void				(*touch)(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);

	// Used to indicate teams, (a way to group things).

	char				*team;			// Team name.
	
	union {
	edict_t				*teamchain;
	edict_t				*targetEnt;
	edict_t				*slavechain;
	edict_t				*rope_grab;		//Used to by the rope to hold the part of the rope which the player is holding
	};
	
	union {
	edict_t				*guidemaster;
	edict_t				*teammaster;
	};

	float				nextthink;

	// Fields used by only one class of game entity (monster, player, poly, trigger, etc).

	char				*model;			// Model name, which could instead be stored in
	
	char				*message;		// index to message_text for printing and wave files

	char				*text_msg;		// Text printed to con for door, polys, triggers, etc.
								
	// These really all could be changed to ints or hashed or something (currently, we do a search
	// on all the active edicts using strcmps). We should be able to assign indexes in the BSP, by
	// doing the string strcmps at BSP time. The player seem to use any of the target stuff.

	char				*target;		// Name of the entity's target.
	char				*targetname;	// What other entities use to target-lock this entity.
	char				*scripttarget; 
	char				*killtarget;	// Used only in G_UseTargets, which fires all 
										// of its targets.
	char				*pathtarget;	// Used by trains, also used by monsters in some
										// way as well.
	union {
	char				*deathtarget;	// Target to be triggered on my death.
	char				*pathtargetname; // Used to target buoys to doors or plats or etc
	};

	union {
	char				*combattarget;	// Monsters are the primary user of this.
	char				*jumptarget;	// for buoys only
	};

	union {
	edict_t				*target_ent;	// Used by player, trains, and turrets. Monsters
										// should be able to use this for current target as well.
	edict_t				*slave;	
	edict_t				*rope_end;		//Used by the rope to store the rope end entity
	};

	vec3_t				movedir;		// Used by just about everything that moves, but not used in
										// physics.

	float				air_finished;	// Used by things that can breath (monsters and player).

	edict_t				*goalentity;	// Used primarily by monsters.
	edict_t				*movetarget;	// Used primarily by monsters, also a little use
										// by poly/trigger
	float				yaw_speed;		// Used by monsters and player.
	float				ideal_yaw;		// Used by monsters and player.
	float				ideal_pitch;	// Used by monsters and player.
	float				yawOffset;		// Used in CreateMove_Step

	float				accel;			// Used mostly in g_func.c.
	float				decel;			// Used mostly in g_func.c.

	float				timestamp;		// Used by a couple of ojects.

	// Used by just about every type of entity.

	void				(*use)(edict_t *self, edict_t *other, edict_t *activator);
								
	int					health;			// Used by anything that can be destroyed.
	int					max_health;		// Used by anything that can be destroyed.
	int					bloodType;		// type of stuff to spawn off when hit

	union {
	int					deadflag;		// More like a dead state, used by things that can die.
										// Would probably be better off with a more general state
										// (or two or three).
	int					deadState;
	};

	union {
	qboolean			show_hostile;	// Only used by monsters (or g_ai.c at least)- not really
										// sure what for.
	void				(*TriggerActivated)(edict_t *self, edict_t *activator);
	};

	char				*map;			// target_changelevel (used only by).

	int					viewheight;		// height above origin where eyesight is determined
										// used by anything which can "see", player and monsters
	float				reflected_time;	// used by objects to tell if they've been repulsed by something..
	
	// Except for a DAMAGE_AIM value in the turret and one commented out in the player this looks
	// like a flag indicating if it takes damage anyone ever heard of using a bit for a flag, or
	// even better a whole bunch of flags in an 'int'.

	int					takedamage;	

	// Unless something will do both normal and radius damage, we only need one field. In fact we
	// may want to move this into class statics or something.

	int					dmg;			// the damage something does.
	float				dmg_radius;		// the radius of damage.

	int					sounds;			// used by a trigger and a splash, could be a class static
	
	union {
	int					count;			// used by polys, triggers, and items.
	int					curr_model;		// used by player during cinematics
	};

	int					targeted;		// used by Ogle to denote a targeted action queued up
	int					lastbuoy;		// used to save a buoy in checking

	edict_t				*chain;			// used by items and player in the body queue.
	edict_t				*enemy;			// used by monsters, player, and a poly or two.
	edict_t				*oldenemy;		// used by monsters.
	edict_t				*activator;		// this that used something, used by monsters, items, and
										// polys.
	// Used by player only.

	edict_t				*mynoise;		// Can go in client only.
	edict_t				*mynoise2;

	edict_t				*last_buoyed_enemy;		// used by monsters.

	int					noise_index;	// Used only by targets
	
	float				volume;			// used only by target_speaker

	union {
	float				attenuation;	// used only by target_speaker
	float				maxrange;		// used for ai
	};

	// Timing variables.

	float				wait;			// Used by polys, triggers, and targets.
	float				delay;			// Delay before firing targets. Used by a few polys and targets.
	float				random;			// Used by func_timer and spl_meteorbarrier.

	// Used to delay monster 5 before going after a player sound. Only set on player.

	union
	{
	float				teleport_time;
	float				time;			// misc. time for whatever
	};

	// Move these to clientinfo?

	int					light_level;	// set on player, checked by monsters

	int					style;			// also used as areaportal number used by items.

	gitem_t				*item;			// For bonus items. Used by player, triggers, and monsters.

	// What it's made of, i.e. MAT_XXX. Used to determine gibs to throw. Curently used only by the
	// barrel, but applicable to anything generically gibbable.

	int					materialtype;
	int					PersistantCFX;	// index to client effect linked to edict
	int					Leader_PersistantCFX;	// non of this should really go in here.. really it should be in the client, but its 2 in the morning, so fuck it

	vec3_t				velocity;			// linear velocity
	vec3_t				avelocity;			// angular velocity
	vec3_t				knockbackvel;		

	// Used for determining effects of liquids in the environment.

	float				speed;			
	int					watertype;		// Used to indicate current liquid actor is in.
	int					waterlevel;		// Used by monsters and players and grenades.

	int					mass;
	float				gravity;		// Per entity gravity multiplier (1.0 is normal) Used for
										// lowgrav artifact, flares.

	// Not currently used by anyone, but it's a part of physics. Probably should remove it.
	void				(*prethink) (edict_t *ent);

	// Move into the moveinfo structure? Used by polys and turret and in physics.
	void				(*blocked)(edict_t *self, edict_t *other);

	// Used by animating entities.

	int					curAnimID;
	int					lastAnimID;

	// Used by monsters and player.

	int 				(*pain)(edict_t *self, edict_t *other, float kick, int damage);
								
	// Used by monsters, player, and some polys.

	int  				(*die)(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);
								
	// used by the Morph Ovum

	void				(*oldthink)(edict_t *self);

	// Used by polys and triggers.

	float				touch_debounce_time;

	// Used by monsters and player.

	float				pain_debounce_time;

	// Used by monsters and player.

	float				damage_debounce_time;
	float				attack_debounce_time;

	//used by reflecting projectiles
	int					reflect_debounce_time;

	float				impact_debounce_time;	// impact damage debounce time

	float				fire_damage_time;		// fire damage length
	float				fire_timestamp;			// timestamp weapons and damaged entities, 
												//		so that the same weapon can't hurt an entity twice

	// Used by shrines

	void				(*oldtouch)(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);
	union {
	int					shrine_type;
	int					morph_timer;
	int					buoy_index;
	};

	// Only set in trigger_push_touch and probably only on players.

	union {
	float				last_buoy_time;
	float				fly_sound_debounce_time;
	};

	union {
	float				last_move_time;	// Only used by target_earthquake (poly/trigger)
	float				old_yaw;		// Used by the Seraph to return to his exact position and angles
	};

	vec3_t				pos1,pos2;		// Used by polys and turrets.

	// Common data blocks

	moveinfo_t			moveinfo;		// 120 bytes
	monsterinfo_t		monsterinfo;	// 156 bytes

	vec3_t				v_angle_ofs;			//View Angle ofset- for when monsters look around, for line of sight checks

	int					ai_mood;		// Used by high level ai to relay simple moods to lower level functions (INTEGRAL FOR SWITCH)
	int					ai_mood_flags;	// Used in conjunction with ai_mood to provide more information to the lower functions
	byte				mintel;			//number of buoys allowed to follow

	char				*target2;
	
	vec3_t				last_org;

	//next 4 in a table in m_stats
	float				min_melee_range;// Min distance at which it is ok for this monster to use it's melee attack
	float				melee_range;	//Max distance to do a melee attack, if negative, closest monster should get to enemy
	float				missile_range;	//Max distance to do a missile attack
	float				min_missile_range;	//Least distance to do a missile attack
	int					bypass_missile_chance;//chance to not use missile attack even if you can (0-100)
	void				(*cant_attack_think)(edict_t *self, float enemydist, qboolean enemyvis, qboolean enemyinfront);//called only when monster cannot attack player

	int					jump_chance;//chance to jump when have opportunity

	float				wakeup_distance;//how far the player can be when I see him to wake me up

	float				evade_debounce_time;//How long to evade for
	float				oldenemy_debounce_time;//How long to hunt enemy before looking for oldenemy again

	float				best_move_yaw;
//	int					jump_chance;	//0 - 100 chance that monster will try to jump to get around when possible/neccessary - in a table in m_stats

	float				mood_nextthink;			//not used anymore?
	void				(*mood_think)(edict_t *self);//you mood setting function

	float				next_pre_think;//any prethinking you want a monster to do
	void				(*pre_think)(edict_t *self);//nextthink time for prethinks

	float				next_post_think;//any prethinking you want a monster to do
	void				(*post_think)(edict_t *self);//nextthink time for prethinks

	int					forced_buoy;	//monster is forced to go to this buoy
	buoy_t				*enemy_buoy;	//monster's enemy's closest buoy
	float				pathfind_nextthink;			//not used anymore?
	edict_t				*nextbuoy[MAX_BUOY_BRANCHES];

	float				dead_size;			//for dead thinking
	struct volume_effect_s	*volfx;

	//New monster stuff
	char				*wakeup_target;		//target to fire when find an enemy
	char				*pain_target;		//target to fire when take pain (only once)
	char				*homebuoy;			//yeah, buoyyyyyyy!

	float				alert_time;			//time when a monster is no longer startled
	alertent_t			*last_alert;		//last alert_ent to startle me, if it's the same one, skip it when going through the list

	edict_t				*placeholder;		//used by assassin to hold his teleport destination

	float				jump_time;			//time that a monster's protection from falling damage runs out after a jump

	int					red_rain_count;		//number of red rains you can have at once

	int					deathtype;			//how you died

	edict_t				*fire_damage_enemy;	//who burnt you to death- for proper burning death credit

#ifndef __cplusplus
	void			*Script;
#else
	CScript			*Script;
#endif
};

#endif // G_EDICT_H