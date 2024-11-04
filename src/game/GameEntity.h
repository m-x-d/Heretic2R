#include "g_local.h"

typedef struct sv_entity_s
{
	entity_state_t	s;
	struct gclient_s	*client;	// NULL if not a player
									// the server expects the first part
									// of gclient_s to be a player_state_t
									// but the rest of it is opaque

//	qboolean	inuse;				// replace with svflags & SVF_INUSE

	int			linkcount;

	// FIXME: move these fields to a server private sv_entity_t (probably not necessary)
	link_t		area;				// linked to a division node or leaf (zeroed in g_save.c is currently the only use of this in the Game DLL)
	
	int			num_clusters;		// if -1, use headnode instead (not referenced in Game DLL)
	int			clusternums[MAX_ENT_CLUSTERS];	// (not referenced in Game DLL)
	int			headnode;			// unused if num_clusters != -1 (not referenced in Game DLL)
	int			areanum, areanum2;	// (areanum is checked in g_ai.c)

	int			svflags;
	vec3_t		mins, maxs;
	vec3_t		absmin, absmax, size;
	solid_t		solid;
	int			clipmask;
	struct sv_entity_s *owner;
} sv_entity_t;

typedef struct g_PolyInfo_s
{
	char		*message;			// text printed to con for door, polys, triggers, etc.

} g_PolyInfo_t;

typedef struct g_PhysicsInfo_s
{
	int			movetype;			// used by physics, set on everything

	float		speed;				// used by physics

	vec3_t		velocity;
	vec3_t		avelocity;
} g_PhysicsInfo_t;

#if 0
typedef struct g_PlayerInfo_s
{
} g_PlayerInfo_s;

typedef struct g_MonsterInfo_s
{
} g_MonsterInfo_s;

typedef struct g_ActorInfo_s
{
} g_ActorInfo_s;

typedef struct g_ItemInfo_s
{
} g_ItemInfo_t;
#endif

typedef struct GameEntity_s
{
	MsgQueue_t	msgQ;
	G_MessageHandler_t msgHandler;

	int			classID;

	void		(*think)(struct GameEntity_s *self);

	int			flags;

	int			spawnflags;			// this is being used in a rather nasty way,
									// being & with magic numbers all over the place

	union
	{
	float		nextthink;
	float		freetime;			// sv.time when the object was freed
									// set in G_FreeEdict and used in G_Spawn only
	};

	char		*classname;

} GameEntity_t;