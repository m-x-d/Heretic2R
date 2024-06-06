#ifndef QSHARED_H
#define QSHARED_H
// q_shared.h -- included first by ALL program modules (unfortuantly this is true for all code from id)

// unknown pragmas are SUPPOSED to be ignored, but....
#pragma warning(disable : 4244)     // MIPS -- truncation from double to float in MSDEV
#pragma warning(disable : 4018)     // signed/unsigned mismatch
#pragma warning(disable : 4305)		// truncation from const double to float

#include <direct.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "H2Common.h"
#include "q_Typedef.h"
//#include "player.h"

#define DEMO_CODE		0

#ifdef QUAKE2_STATIC
#define QUAKE2_API
#else
#ifdef QUAKE2
#define QUAKE2_API __declspec(dllexport)
#else
#define QUAKE2_API __declspec(dllimport)
#endif
#endif

// These are the only 2 references to C_ONLY
// id386 could do with removing
#define C_ONLY 1

#if defined _M_IX86 && !defined C_ONLY
#define id386	1
#else
#define id386	0
#endif

#ifndef NULL
#define NULL ((void *)0)
#endif


// angle indexes
#define	PITCH				0		// up / down
#define	YAW					1		// left / right
#define	ROLL				2		// fall over

#define	MAX_STRING_CHARS	1024	// max length of a string passed to Cmd_TokenizeString
#define	MAX_STRING_TOKENS	80		// max tokens resulting from Cmd_TokenizeString
#define	MAX_TOKEN_CHARS		128		// max length of an individual token

#define	MAX_QPATH			64		// max length of a quake game pathname
#define	MAX_OSPATH			128		// max length of a filesystem pathname

// game print flags
#define	PRINT_LOW			0		// pickup messages
#define	PRINT_MEDIUM		1		// death messages
#define	PRINT_HIGH			2		// critical messages
#define	PRINT_CHAT			3		// chat messages
#define	PRINT_CAPTION		4		// captioning at bottom
#define	PRINT_TEAM			5		// chat message to team members

#define	ERR_FATAL			0		// exit the entire game with a popup window
#define	ERR_DROP			1		// print to console and disconnect from game
#define	ERR_DISCONNECT		2		// don't kill server

#define	PRINT_ALL			0
#define PRINT_DEVELOPER		1		// only print when "developer 1"
#define PRINT_ALERT			2		

#ifndef M_PI
#define M_PI				3.14159265358979323846	// matches value in gcc v2 math.h
#endif
#define SQRT2				1.414213562
#define TIME_EPSILON		0.01

#define VectorCopy_Macro(a,b)			(b[0]=a[0],b[1]=a[1],b[2]=a[2])
#define VectorSubtract_Macro(a,b,c)		(c[0]=a[0]-b[0],c[1]=a[1]-b[1],c[2]=a[2]-b[2])

H2COMMON_API int Q_log2(int val);

H2COMMON_API void ClearBounds (vec3_t mins, vec3_t maxs);
H2COMMON_API void AddPointToBounds (vec3_t v, vec3_t mins, vec3_t maxs);

H2COMMON_API float	anglemod(float a);
H2COMMON_API float	anglemod_old(float a);
H2COMMON_API float LerpAngle (float a1, float a2, float frac);

int BoxOnPlaneSide (vec3_t emins, vec3_t emaxs, struct cplane_s *plane);

#define BOX_ON_PLANE_SIDE(emins, emaxs, p)	\
	(((p)->type < 3)?						\
	(										\
		((p)->dist <= (emins)[(p)->type])?	\
			1								\
		:									\
		(									\
			((p)->dist >= (emaxs)[(p)->type])?\
				2							\
			:								\
				3							\
		)									\
	)										\
	:										\
		BoxOnPlaneSide( (emins), (emaxs), (p)))

//=============================================

#define MAX_COLORS	33

extern H2COMMON_API paletteRGBA_t TextPalette[MAX_COLORS];

typedef enum
{
	P_BLACK			,				
	P_RED			,
	P_GREEN			,	
	P_YELLOW		,	
	P_BLUE 			,	
	P_PURPLE		,	
	P_CYAN			,	
	P_WHITE			,	

	P_HBLACK		,	
	P_HRED			,	
	P_HGREEN  		,	
	P_HYELLOW		,	
	P_HBLUE   		,	
	P_HPURPLE		,	
	P_HCYAN			,	
	P_HWHITE 		,	

	P_DESIGNER		,	
	P_PROGRAMMER	,	
	P_OBJ_NORMAL	,	
	P_OBJ_BOLD		,	
	P_OBIT			,	
	P_CAPTION		,	
	P_CHAT			,	
	P_TEAM 			,
				
	P_VERSION		,	
	P_FRAGS			,	
	P_ALTFRAGS		,	
	P_MENUFIELD		,	
	P_MSGBOX		,	
	P_HEADER		,
	P_CRED_TITLE	,
	P_CRED_CONTENT	,		
	P_FRAGNAME			

} PalIdx_t;

//=============================================

H2COMMON_API char *COM_SkipPath (char *pathname);
H2COMMON_API void COM_StripExtension (char *in, char *out);
H2COMMON_API void COM_FileBase (char *in, char *out);
H2COMMON_API void COM_FilePath (char *in, char *out);
H2COMMON_API void COM_DefaultExtension (char *path, char *extension);

H2COMMON_API char *COM_Parse (char **data_p);
H2COMMON_API void Com_sprintf (char *dest, int size, char *fmt, ...);
H2COMMON_API void Com_PageInMemory (byte *buffer, int size);

//=============================================

H2COMMON_API short	BigShort(short l);
H2COMMON_API int    BigLong(int l);
H2COMMON_API float	BigFloat(float f);

#define LittleShort(x)	(x)
#define LittleLong(x)	(x)
#define LittleFloat(x)	(x)

H2COMMON_API float Clamp(float src, float min, float max);
H2COMMON_API int ClampI(int src, int min, int max);
H2COMMON_API float Approach(float curr, float dest, float rate);
H2COMMON_API char	*va(char *format, ...);

//=============================================

//
// key / value info strings
//
#define	MAX_INFO_KEY		64
#define	MAX_INFO_VALUE		64
#define	MAX_INFO_STRING		512

#ifdef __cplusplus
extern "C"
{
#endif
H2COMMON_API char *Info_ValueForKey (char *s, char *key);
H2COMMON_API void Info_RemoveKey (char *s, char *key);
H2COMMON_API void Info_SetValueForKey (char *s, char *key, char *value);
H2COMMON_API qboolean Info_Validate (char *s);
H2COMMON_API void Set_Com_Printf(void (*toSet) (char *fmt, ...));

/*
==============================================================

SYSTEM SPECIFIC

==============================================================
*/

struct cplane_s;
extern H2COMMON_API vec3_t vec3_origin;
extern H2COMMON_API vec3_t vec3_up;

extern	int	curtime;		// time returned by last Sys_Milliseconds

int		Sys_Milliseconds (void);
void	Sys_Mkdir (char *path);

// large block stack allocation routines
void	*Hunk_Begin (int maxsize);
void	*Hunk_Alloc (int size);
void	Hunk_Free (void *buf);
int		Hunk_End (void);

// directory searching
#define SFF_ARCH    0x01
#define SFF_HIDDEN  0x02
#define SFF_RDONLY  0x04
#define SFF_SUBDIR  0x08
#define SFF_SYSTEM  0x10

/*
** pass in an attribute mask of things you wish to REJECT
*/
char	*Sys_FindFirst (char *path, unsigned musthave, unsigned canthave );
char	*Sys_FindNext ( unsigned musthave, unsigned canthave );
void	Sys_FindClose (void);


// this is only here so the functions in q_shared.c and q_shwin.c can link
void Sys_Error (char *error, ...);
void Com_Printf (char *msg, ...);
void Com_ColourPrintf (PalIdx_t colour, char *msg, ...);

#ifdef __cplusplus
}
#endif

/*
==========================================================

CVARS (console variables)

==========================================================
*/

#ifndef CVAR
#define	CVAR

#define	CVAR_ARCHIVE	1	// set to cause it to be saved to vars.rc
#define	CVAR_USERINFO	2	// added to userinfo  when changed
#define	CVAR_SERVERINFO	4	// added to serverinfo when changed
#define	CVAR_NOSET		8	// don't allow change from console at all,
							// but can be set from the command line
#define	CVAR_LATCH		16	// save changes until server restart

// nothing outside the Cvar_*() functions should modify these fields!
typedef struct cvar_s
{
	char		*name;
	char		*string;
	char		*latched_string;	// for CVAR_LATCH vars
	int			flags;
	qboolean	modified;	// set each time the cvar is changed
	float		value;
	struct cvar_s *next;
} cvar_t;

#endif		// CVAR

cvar_t *Cvar_Get (char *var_name, char *value, int flags);

/**************************************************************
 * palette info
 **************************************************************/

typedef struct paletteRGB_s
{
	struct
	{
		byte r,g,b;
	};
} paletteRGB_t;

/**************************************************************
 * additional info for flex models with mesh nodes
 **************************************************************/

#define MAX_FM_MESH_NODES	16		// also defined in ref_gl/fmodel.h

typedef struct
{
	int				frame;
	paletteRGBA_t	color;
	byte			flags;
	int				skin;
} fmnodeinfo_t;

// flags

#define FMNI_USE_FRAME		(1<<0)
#define FMNI_USE_COLOR		(1<<1)
#define FMNI_USE_SKIN		(1<<2)
#define FMNI_NO_LERP		(1<<3)
#define FMNI_NO_DRAW		(1<<4)
#define FMNI_USE_REFLECT	(1<<5)
/*
==============================================================

COLLISION DETECTION

==============================================================
*/

// ************************************************************************************************
// CONTENTS_XXX
// ------------
// Contents flags.
// ************************************************************************************************

// Lower bits are stronger, and will eat weaker brushes completely.

#define	CONTENTS_EMPTY			0x00000000	// nothing
#define	CONTENTS_SOLID			0x00000001	// An eye is never valid in a solid.
#define	CONTENTS_WINDOW			0x00000002	// Translucent, but not watery.
#define	CONTENTS_ILLUSIONARY	0x00000004  // Was CONTENTS_AUX.
#define	CONTENTS_LAVA			0x00000008
#define	CONTENTS_SLIME			0x00000010
#define	CONTENTS_WATER			0x00000020
#define	CONTENTS_MIST			0x00000040
#define	LAST_VISIBLE_CONTENTS	CONTENTS_MIST

// Remaining contents are non-visible, and don't eat brushes.

#define	CONTENTS_AREAPORTAL		0x00008000
#define	CONTENTS_PLAYERCLIP		0x00010000
#define	CONTENTS_MONSTERCLIP	0x00020000

// Currents can be added to any other contents, and may be mixed.

#define	CONTENTS_CURRENT_0		0x00040000
#define	CONTENTS_CURRENT_90		0x00080000
#define	CONTENTS_CURRENT_180	0x00100000
#define	CONTENTS_CURRENT_270	0x00200000
#define	CONTENTS_CURRENT_UP		0x00400000
#define	CONTENTS_CURRENT_DOWN	0x00800000

// Removed before bsping an entity.

#define	CONTENTS_ORIGIN			0x01000000

// Should never be on a brush, only in game (i.e. set by CM_ functions at map load time).

#define	CONTENTS_MONSTER		0x02000000
#define	CONTENTS_DEADMONSTER	0x04000000

// Brushes to be added after vis leaves.

#define	CONTENTS_DETAIL			0x08000000

// Auto set if any surface has transparency, e.g. water.

#define	CONTENTS_TRANSLUCENT	0x10000000

// This flag is special in that it is not stored in the .bsp by QuakeEd. It is passed into the trace
// functions to say that anything with CONTENTS_CAMERANOBLOCK should be ignored. So we can get away
// with defining it = CONTENTS_CAMERANOBLOCK.

#define	CONTENTS_CAMERABLOCK	0x20000000	// Was CONTENTS_LADDER.

// This flag is special in that it is NOT passed into the trace functions, but may be stored in the
//.bsp by QuakeEd to say that traces with CONTENTS_CAMERABLOCK as the mask will ignore any brushes
// with this flag.

#define	CONTENTS_CAMERANOBLOCK	0x40000000

// Only do the trace against the world, not entities within it. Not stored in the .bsp and passed
// only as an argument to trace fucntions.

#define CONTENTS_WORLD_ONLY		0x80000000

// ************************************************************************************************
// MASK_XXX
// --------
// Contents masks.
// ************************************************************************************************

#define	MASK_ALL			0x7fffffff
#define	MASK_SOLID			(CONTENTS_SOLID|CONTENTS_WINDOW)
#define	MASK_PLAYERSOLID	(CONTENTS_SOLID|CONTENTS_PLAYERCLIP|CONTENTS_WINDOW|CONTENTS_MONSTER)
#define	MASK_DEADSOLID		(CONTENTS_SOLID|CONTENTS_PLAYERCLIP|CONTENTS_WINDOW)
#define	MASK_MONSTERSOLID	(CONTENTS_SOLID|CONTENTS_MONSTERCLIP|CONTENTS_WINDOW|CONTENTS_MONSTER)
#define	MASK_WATER			(CONTENTS_WATER|CONTENTS_LAVA|CONTENTS_SLIME)
#define	MASK_OPAQUE			(CONTENTS_SOLID|CONTENTS_SLIME|CONTENTS_LAVA)
#define	MASK_SHOT			(CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_WINDOW|CONTENTS_DEADMONSTER)
#define MASK_CURRENT		(CONTENTS_CURRENT_0|CONTENTS_CURRENT_90|CONTENTS_CURRENT_180|CONTENTS_CURRENT_270|CONTENTS_CURRENT_UP|CONTENTS_CURRENT_DOWN)
#define MASK_DRIP			(CONTENTS_SOLID|CONTENTS_WATER|CONTENTS_LAVA|CONTENTS_SLIME|CONTENTS_WINDOW)

// ************************************************************************************************
// SURF_XXX
// --------
// ************************************************************************************************

#define	SURF_LIGHT			0x1			// Value will hold the light strength.
#define	SURF_SLICK			0x2			// Affects game physics.
#define	SURF_SKY			0x4			// Don't draw, but add to skybox.
#define	SURF_WARP			0x8			// Turbulent water warp.
#define	SURF_TRANS33		0x10
#define	SURF_TRANS66		0x20
#define	SURF_FLOWING		0x40		// Scroll towards angle.
#define	SURF_NODRAW			0x80		// Don't bother referencing the texture.
#define	SURF_TALL_WALL		0x00000400	// Face doesn't get broken up as normal.
#define	SURF_ALPHA_TEXTURE	0x00000800	// texture has alpha in it, and should show through in bsp process
#define	SURF_ANIMSPEED		0x00001000	// value will hold the anim speed in fps
#define SURF_UNDULATE		0x00002000	// rock surface up and down...
#define SURF_QUAKE			0x00004000	// rock surface up and down when quake value on

// gi.BoxEdicts() can return a list of either solid or trigger entities
// FIXME: eliminate AREA_ distinction?

#define	AREA_SOLID		1
#define	AREA_TRIGGERS	2


// plane_t structure
// !!! if this is changed, it must be changed in asm code too !!!
typedef struct cplane_s
{
	vec3_t	normal;
	float	dist;
	byte	type;			// for fast side tests
	byte	signbits;		// signx + (signy<<1) + (signz<<1)
	byte	pad[2];
} cplane_t;

// structure offset for asm code
#define CPLANE_NORMAL_X			0
#define CPLANE_NORMAL_Y			4
#define CPLANE_NORMAL_Z			8
#define CPLANE_DIST				12
#define CPLANE_TYPE				16
#define CPLANE_SIGNBITS			17
#define CPLANE_PAD0				18
#define CPLANE_PAD1				19

typedef struct cmodel_s
{
	vec3_t		mins, maxs;
	vec3_t		origin;		// for sounds or lights
	int			headnode;
} cmodel_t;

typedef struct csurface_s
{
	char		name[40];
	int			flags;
	int			value;
} csurface_t;

// ************************************************************************************************
// trace_t
// -------
// A trace is returned when a box is swept through the world.
// ************************************************************************************************

typedef struct trace_s
{
	byte		allsolid;		// if true, plane is not valid
	byte		startsolid;		// if true, the initial point was in a solid area
	byte		succeeded;		// not always set, just in special cases, subjective
	byte		architecture;	// set if the moved collided with world (not entities)
								// needed because the player move code doesn`t know anything
								// about the location or nature of edicts
	float		fraction;		// time completed, 1.0 = didn't hit anything
	vec3_t		endpos;			// final position
	cplane_t	plane;			// surface normal at impact
	csurface_t	*surface;		// surface hit
	int			contents;		// contents on other side of surface hit
	struct edict_s	*ent;		// not set by CM_*() functions
} trace_t;

// ************************************************************************************************
// pmtype_t
// --------
// ************************************************************************************************

typedef enum 
{
	PM_NORMAL,				// Can accelerate and turn, clips.
	PM_SPECTATOR,			// Can accelerate and turn, no clip.
	PM_DEAD,				// No acceleration or turning, dead.
	PM_GIB,					// No acceleration or turning, dead, different bounding box.
	PM_FREEZE,				// Running a server demo.
	PM_INTERMISSION,		// An existing remote camera view is already underway.
} pmtype_t;

// ************************************************************************************************
// PMF_XXX
// -------
// These are pmove->pm_flags.
// ************************************************************************************************

#define	PMF_STANDSTILL		0x0001
#define	PMF_ON_GROUND		0x0004
#define	PMF_TIME_LAND		0x0008	// pm_time is time before rejump
#define	PMF_TIME_TELEPORT	0x0010	// pm_time is non-moving time
#define PMF_NO_PREDICTION	0x0020	// temporarily disables prediction (used for grappling hook)
#define PMF_LOCKMOVE		0x0040
#define PMF_LOCKTURN		0x0080

#define PC_COLLISION		0x0001	// collided on a move
#define PC_SLIDING			0x0002	// sliding down a steep slope

#define	WF_SURFACE			0x0001	// On the surface
#define	WF_DIVE				0x0002	// Dive on next animation
#define	WF_DIVING			0x0004	// Currently diving
#define	WF_SWIMFREE			0x0008	// Currently swimming freely underwater
#define	WF_SINK				0x0010	// Sink below the surface of the water

// ************************************************************************************************
// pmove_state_t
// -------------
// This structure contains the information necessary for client side movement prediction. It MUST
// be communicated bit-accurate from the server to the client to guarantee that prediction stays in
// sync., so no floats are used. If any part of the game code modifies this struct, it will result
// in a prediction error of some degree.
// ************************************************************************************************

typedef struct
{
	pmtype_t	pm_type;
	short		origin[3];		// 12.3
	short		velocity[3];	// 12.3
	byte		pm_flags;		// ducked, jump_held, etc
	byte		w_flags;		// water state
	byte		c_flags;		// collision
	byte		pm_time;		// each unit = 8 ms
	short		gravity;
	short		delta_angles[3];// Added to command angles to get view direction. Changed by spawns,
								// rotating objects and teleporters.
	short		camera_delta_angles[3];// Added to command angles to get view direction. Changed by spawns,
								// rotating objects and teleporters.
} pmove_state_t;

// ************************************************************************************************
// BUTTON_XXX
// ----------
// ************************************************************************************************

#define	BUTTON_ATTACK		1
#define	BUTTON_DEFEND		2
#define	BUTTON_ACTION		4
#define BUTTON_CREEP		8
#define BUTTON_RUN			16
#define BUTTON_AUTOAIM		32
#define BUTTON_LOOKAROUND	64
#define BUTTON_QUICKTURN	128
#define BUTTON_INVENTORY	256
#define	BUTTON_ANY			512	// Any key whatsoever.

// ************************************************************************************************
// usercmd_t
// ---------
// Sent to the server each client frame.
// ************************************************************************************************

typedef struct usercmd_s
{
	byte	msec;
	short	buttons;
	short	angles[3],
			aimangles[3],
			camera_vieworigin[3],camera_viewangles[3],
			forwardmove,sidemove,upmove;
	byte	prediction;
	byte	lightlevel;	// Light level the player is standing on.
} usercmd_t;

// ************************************************************************************************
// pmove_t
// -------
// ************************************************************************************************

#define	MAXTOUCH	32

typedef struct
{
	// State (in / out).

	pmove_state_t	s;

	// Command (in).

	usercmd_t		cmd;
	qboolean		snapinitial;		// If s has been changed outside pmove.
	qboolean		run_shrine;
	qboolean		high_max;

	// In and out.

	float			waterheight;
	float			desiredWaterHeight;

	// Results (out).

	int				numtouch;
	struct edict_s	*touchents[MAXTOUCH];

	float			viewheight;
	vec3_t			viewangles;			// Clamped.
	vec3_t			camera_viewangles;	// Camera angles from client.
	
	float			*origin, *velocity;
	vec3_t			mins, maxs;			// Bounding-box size.
	float			*intentMins, *intentMaxs;

	struct edict_s	*groundentity;
	csurface_t		*GroundSurface;
	cplane_t		GroundPlane;
	int				GroundContents;

	int				watertype;
	int				waterlevel;
	float			knockbackfactor;

	// Callbacks to test the world.

	void			(*trace) (vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end,trace_t *trace);
	int				(*pointcontents) (vec3_t point);

	struct edict_s	*self;
} pmove_t;

// entity_state_t->renderfx flags
#define	RF_MINLIGHT			0x00000001		// allways have some light (viewmodel)
#define	RF_REFLECTION		0x00000002		// Use GL spherical mapping, if available
#define	RF_WEAPONMODEL		0x00000004		// only draw through eyes
#define	RF_FULLBRIGHT		0x00000008		// allways draw full intensity
#define	RF_DEPTHHACK		0x00000010		// for view weapon Z crunching
#define	RF_TRANSLUCENT		0x00000020
#define	RF_FRAMELERP		0x00000040
#define	RF_CUSTOMSKIN		0x00000080		// skin is an index in image_precache
#define	RF_GLOW				0x00000100		// pulse lighting for bonus items
#define RF_SCALE_XYZ		0x00000200
#define RF_SCALE_XY			0x00000400
#define RF_SCALE_Z			0x00000800
#define RF_PRIMITIVE		0x00001000		// line, or other primitive runtime generated by the render DLL
#define RF_FIXED			0x00002000		// the sprite has a fixed direction 
											// and up vector, by default, a
											// sprite is always oriented to the 
											// view (no effect on models)
#define RF_TRANS_ADD		0x00004000		// Additive transparency
#define RF_TRANS_ADD_ALPHA	0x00008000		// Adds emulation of alpha for additive transparent objects using tint
#define RF_TRANS_GHOST		0x00010000		// Like subtractive translucency
#define RF_ALPHA_TEXTURE	0x00020000		// Object has an alpha texture map
#define RF_NODEPTHTEST		0x00080000		// Turns off depth testing for sprites only
#define RF_IGNORE_REFS		0x00100000		// don't update the ref points for a model
#define RF_NODRAW			0x00200000
#define	RF_CULL_LARGE		0x00400000		// If set on a poly that is really close to the near clip plane and occupies
											// a signifiant amount of screen real-estate, the poly will be culled. Used
											// for particles in software.

#define RF_TRANS_ANY		(RF_TRANS_ADD | RF_TRANS_GHOST | RF_TRANSLUCENT)

// player_state_t->refdef flags
#define	RDF_UNDERWATER		0x00000001		// warp the screen as apropriate
#define RDF_NOWORLDMODEL	0x00000002		// used for player configuration screen

// sound channels
// channel 0 never willingly overrides
// other channels (1-7) allways override a playing sound on that channel
#define	CHAN_AUTO               0
#define	CHAN_WEAPON             1
#define	CHAN_VOICE              2
#define	CHAN_ITEM               3
#define	CHAN_BODY               4
#define CHAN_FOOTSTEP			5
#define CHAN_FOOTSTEP2			6
#define CHAN_WEAPON2			7
// modifier flags
#define	CHAN_NO_PHS_ADD			8	// send to all clients, not just ones in PHS (ATTN 0 will also do this)
#define	CHAN_RELIABLE			16	// send by reliable message, not datagram


// sound attenuation values
#define	ATTN_NONE               0	// full volume the entire level
#define	ATTN_NORM               1
#define	ATTN_IDLE               2
#define	ATTN_STATIC             3	// diminish very rapidly with distance
#define ATTN_VERYSTATIC			4
#define ATTN_LEFT				256
#define ATTN_RIGHT				512

// player_state->stats[] indexes

// * - MUST BE SEQUENTIAL !!!!!!!!

#define STAT_HEALTH_ICON		0		// Icon for health
#define	STAT_HEALTH				1		// Health value
#define	STAT_AMMO_ICON			2		// Icon for ammo
#define	STAT_AMMO				3		// Amount of ammo
#define	STAT_WEAPON_ICON 		4		// Current offensive weapon                 
#define	STAT_WEAPON		 		5		                                            
#define STAT_DEFENCE_ICON	  	6		// Current defensive weapon                 
#define STAT_DEFENCE			7		                                            
#define STAT_OFFMANA_ICON		8		// * Icon describing offensive mana           
#define STAT_OFFMANA_BACK		9		// Amount of offensive mana                 
#define STAT_OFFMANA			10		// Icon describing defensive mana           
#define STAT_DEFMANA_ICON	 	11		// * Amount of defensive mana                 
#define STAT_DEFMANA_BACK     	12		                                            
#define STAT_DEFMANA			13		                                            
#define	STAT_FRAGS_ICON     	14		// cleared each frame, 1 = health, 2 = armor
#define	STAT_FRAGS			  	15		// which status to print                    
#define	STAT_FLASHES			16		
#define	STAT_LAYOUTS		 	17		
#define	STAT_PUZZLE_ITEM1	 	18
#define	STAT_PUZZLE_ITEM2	 	19
#define	STAT_PUZZLE_ITEM3	 	20
#define	STAT_PUZZLE_ITEM4	 	21
#define	STAT_PUZZLE_COUNT	 	22
#define STAT_POWERUP_ICON		23		// * THESE MUST BE SEQUENTIAL !!!!!
#define STAT_POWERUP_BACK		24
#define STAT_POWERUP_TIMER		25
#define STAT_LUNG_ICON  		26		// *
#define STAT_LUNG_BACK  		27
#define STAT_LUNG_TIMER  		28
#define STAT_LIFEBAR_XSIZE		29		// *
#define STAT_LIFEBAR_YSIZE		30
#define STAT_LIFEBAR_ICON		31
#define STAT_LIFEBAR_BACK		32
#define STAT_LIFEBAR_VALUE		33
#define STAT_ARMOUR_ICON		34
#define STAT_ARMOUR				35

#define	MAX_STATS				48

// dmflags->value flags
#define	DF_WEAPONS_STAY			0x0001
#define	DF_NO_SHRINE			0x0002
#define DF_NONAMES				0x0004
#define	DF_NO_HEALTH			0x0008
#define DF_SHOW_LEADER			0x0010
#define	DF_SHRINE_CHAOS			0x0020
#define	DF_SAME_LEVEL			0x0040
#define DF_FORCE_RESPAWN		0x0080
#define DF_SKINTEAMS			0x0100
#define DF_MODELTEAMS			0x0200
#define DF_ALLOW_EXIT			0x0400
#define DF_INFINITE_MANA		0x0800
#define DF_HURT_FRIENDS			0x1000
#define DF_NO_OFFENSIVE_SPELL	0x2000			// This means staff-only combat!!!
#define DF_NO_DEFENSIVE_SPELL	0x4000
#define DF_DISMEMBER			0x8000

#define DF_DEATHMATCH_SET		0x80000000		// High bit indicates deathmatch, so that it can be transmitted in playerinfo.

/*
==========================================================

  ELEMENTS COMMUNICATED ACROSS THE NET

==========================================================
*/

#define	ANGLE2SHORT(x)	((int)((x)*65536/360) & 65535)
#define	SHORT2ANGLE(x)	((x)*(360.0/65536))

#define DEG2RAD( a ) ( a * M_PI ) / 180.0F

//
// config strings are a general means of communication from
// the server to all connected clients.
// Each config string can be at most MAX_QPATH characters.
//

#define	CS_NAME				0
#define	CS_CDTRACK			1
#define	CS_SKY				2
#define	CS_SKYAXIS			3		// %f %f %f format
#define	CS_SKYROTATE		4
#define CS_LEVEL_NUMBER		5
#define	CS_STATUSBAR		6		// display program string (this seems to take up several slots - hence must be last)
#define	CS_MAXCLIENTS		30
#define	CS_MAPCHECKSUM		31		// for catching cheater maps
#define	CS_MODELS			32
#define	CS_SOUNDS			(CS_MODELS+MAX_MODELS)
#define	CS_IMAGES			(CS_SOUNDS+MAX_SOUNDS)
#define	CS_LIGHTS			(CS_IMAGES+MAX_IMAGES)
#define	CS_ITEMS			(CS_LIGHTS+MAX_LIGHTSTYLES)
#define	CS_PLAYERSKINS		(CS_ITEMS+MAX_ITEMS)
#define CS_WELCOME			(CS_PLAYERSKINS+MAX_CLIENTS)  // give us 4 welcome string messages so we can have a total of 256 characters per message
#define	MAX_CONFIGSTRINGS	CS_WELCOME + 4

// ************************************************************************************************
// EffectsBuffer_t
// ---------------
// ************************************************************************************************

#define ENTITY_FX_BUF_SIZE			192
#define ENTITY_FX_BUF_BLOCK_SIZE	256
#define MAX_PERSISTANT_EFFECTS		512

typedef struct EffectsBuffer_s
{
	byte	*buf;
	int		bufSize;
	int		freeBlock;
	int		numEffects;
} EffectsBuffer_t;

typedef struct PerEffectsBuffer_s
{
	byte	buf[ENTITY_FX_BUF_SIZE];
	int		bufSize;
	int		freeBlock;
	int		numEffects;
	int		send_mask;
	int		demo_send_mask;
	int		fx_num;
} PerEffectsBuffer_t;

// ************************************************************************************************
// entity_state_t
// --------------
// This is the information conveyed from the server to clients in an update message, about entities
// that the client will need to render.
// ************************************************************************************************

typedef struct entity_state_s
{
	// Edict index.

	short			number;
	
	// Model's current animation frame index.

	short			frame;

	// Model's position and orientation  in the world.

	vec3_t			origin;
	vec3_t			angles;
	vec3_t			old_origin;		// Used for lerping (and hijacked for other uses).

	// Tells client which model do draw.

	byte			modelindex;
	// byte Padding here.

	short			clientnum;		// In Quake 2, the client num was passed in skinnum.  We need this value, however.
	
	// For specific path to skin.
	
	char			skinname[MAX_QPATH];
	int				skinnum;
	
	// Model scale.

	float			scale;
	
	// EF_XXX.

	int				effects;					
	
	// RF_XXX.

	int				renderfx;
	
	// What's this exactly?

	paletteRGBA_t	color;

	// Entire absLight gets sent if any component changes.

	paletteRGB_t	absLight;
	
	// Client prediction: 8*(bits 0-4) is x/y radius, 8*(bits 5-9) is z min, 8*(bits 10-15) is z max.
	// Note that gi.linkentity() sets this up.
	
	short			solid;			

	// For looping sounds, to guarantee shutoff.

	byte			sound;

	// For looping sounds, so we can set volume and attenuation.

	byte			sound_data;

	// required so we can place sounds for bmodels correctly in 3d space
	vec3_t			bmodel_origin;

	// Header block for list of all client effects attatched to this entity.

	EffectsBuffer_t clientEffects;
	
	// Specifies which parts (nodes) of the model are on/off, node colors etc.

	fmnodeinfo_t	fmnodeinfo[MAX_FM_MESH_NODES];
	
	// Skeletal info. Only relevant for player models (i.e. have U_JOINTED).

	short			skeletalType;
	short			rootJoint;

	short			swapFrame;

	// Used by the client to verify is this still the same entity it last had.

	byte			usageCount;
} entity_state_t;

// ************************************************************************************************
// player_state_t
// --------------
// This is the information needed in addition to the 'pmove_state_t' to render a view. There will
// only be 10 'player_state_t's sent each second, but the number of pmove_state_t changes will be
// relative to client frame rates.
// ************************************************************************************************

typedef struct
{
	// For prediction (must be communicated bit precise).

	pmove_state_t	pmove;

	// For fixed views and serverdemos.

	vec3_t			viewangles;
	
	// For remote camera views.			

	vec3_t			remote_vieworigin,
					remote_viewangles;
	int				remote_id;

	// Z displacement of player's head from origin.	

	short			viewheight;

	// Deltas added to the player model's client determined angles.

	vec3_t			offsetangles;
	
	// Horizontal field of view.

	float			fov;

	// Refdef flags used during rendering.

	int				rdflags;

	// Index of edict currently targeted by the player's auto-targeting facility.

	int				AutotargetEntityNum;

	// For showing the amount of current level completed.

	byte			map_percentage;

	// Current fog density for the player.

	float			fog_density;

	// Id number for primary and secondary mission statments in strings.txt.

	short			mission_num1;		
	short			mission_num2;

	byte			cinematicfreeze;	// Shows if currently in a cinematic

	// Current state of players in this clients view (one bit per player).

	int				PIV;

	// Fast status bar updates.

	short			stats[MAX_STATS];

	// These are NEVER sent accross the net, but are purely client-side repositiories needed by
	// prediction and are filled from the player's entity_state_t.

	int				frame,swapFrame;
	vec3_t			angles;
	int				effects,
					skinnum,
					renderfx,
					clientnum;
	fmnodeinfo_t	fmnodeinfo[MAX_FM_MESH_NODES];

	// ********************************************************************************************
	// All the stuff below is required for client prediction to function. NOTE still working on
	// this... expect additions / removals / packet size reduction.
	// ********************************************************************************************

	// Player's acurate mins and maxs.

	vec3_t			mins,maxs;

	// Inventory changes.
	
	byte			NoOfItems,
					inventory_changes[256],
					inventory_remaining[256];

	// Ground attributes.

	byte			NonNullgroundentity;		// boolean
	cplane_t		GroundPlane;
	int				GroundContents;
	csurface_t		GroundSurface;

	// Water attributes.

	int				watertype;
	int				waterlevel;		
	float			waterheight;

	// Grabbing / pullup stuff.

	vec3_t			grabloc;
	float			grabangle;
	
	// Velocities imparted on a per animation frame basis.

	float			fwdvel,sidevel,upvel;

	// PLAYER_FLAG_XXX.		

	int				flags;

	// FL_XXX.

	int				edictflags;

	// Player edict's oldvelocity.

	float			oldvelocity_z;

	// Animation sequencing state.

	int				upperseq,lowerseq;
	int				upperframe,lowerframe;
	byte			upperidle,loweridle;		// boolean
	int				uppermove_index,lowermove_index;
	byte			weapon,defense,lastweapon,lastdefense;
	int				weaponready;	
	int				switchtoweapon;
	byte			newweapon;
	int				weap_ammo_index,def_ammo_index;
	byte			weaponcharge;
	byte			advancedstaff;
	int				armortype;
	byte			bowtype;				
	byte			stafflevel;				
	byte			helltype;				
	byte			meteor_count;
	byte			handfxtype;				
	byte			plaguelevel;			
	short			skintype;
	unsigned int	altparts;

	int				deadflag; 
	float			ideal_yaw;
	float			leveltime;
	float			idletime;
	float			quickturnEndTime;
	float			quickturn_rate;
	float			powerup_timer;

	// Deathmatch flags.

	int				dmflags;

} player_state_t;

// *************************************************************
// Inlines
// *************************************************************

// microsoft's fabs seems to be ungodly slow...

_inline float Q_fabs (float _X)
{
	*((long*)&_X) &= 0x7fffffff;
	return (_X);
}

// Quick version of float to long (trunc/round undefined)

#pragma warning (disable:4035)
_inline __declspec( naked ) long Q_ftol( float f )
{
	static int tmp;
	__asm fld dword ptr [esp+4]
	__asm fistp tmp
	__asm mov eax, tmp
	__asm ret
}
#pragma warning (default:4035)

_inline int Q_stricmp (char *s1, char *s2)
{
#if defined(WIN32)
	return _stricmp (s1, s2);
#else
	return strcasecmp (s1, s2);
#endif
}

_inline int Q_strncasecmp (char *s1, char *s2, int n)
{
	return strnicmp (s1, s2, n);
}

_inline int Q_strcasecmp (char *s1, char *s2)
{
	return stricmp (s1, s2);
}


// For ambient sounds.
typedef enum AmbientSoundID_e
{
	AS_NOTHING = 0,
	AS_FIRE,
	AS_WATERLAPPING,	
	AS_SEAGULLS,
	AS_OCEAN,
	AS_BIRDS,
	AS_CRICKETS,
	AS_FROGS,
	AS_CRYING,
	AS_MOSQUITOES,
	AS_BUBBLES,    // 10

	AS_BELL,
	AS_FOOTSTEPS,
	AS_MOANS,
	AS_SEWERDRIPS,
	AS_WATERDRIPS,
	AS_HEAVYDRIPS,
	AS_SMALLFOUNTAIN,
	AS_LARGEFOUNTAIN,
	AS_SEWERWATER,
	AS_OUTSIDEWATERWAY,	// 20

	AS_WINDCHIME,
	AS_BIRD1,
	AS_BIRD2,
	AS_CAULDRONBUBBLE,
	AS_HUGEWATERFALL,
	AS_GONG,
	AS_MUDPOOL,
	AS_ROCKS,
	AS_WINDEERIE,
	AS_WINDNOISY,		// 30

	AS_WINDSOFTHI,
	AS_WINDSOFTLO,
	AS_WINDSTRONG1,
	AS_WINDSTRONG2,
	AS_WINDWHISTLE,
	AS_CONVEYOR,
	AS_BUCKETCONVEYOR,

	AS_CAVECREAK,
	AS_SPIT,		// 39

	AS_MAX
} AmbientSoundID_t;

typedef enum DoorSoundID_e
{
	DS_NONE = 0,
	DS_GENERIC,
	DS_HEAVYSTONE,
	DS_SWINGARM,
	DS_SWINGBRIDGE,
	DS_MEDIUMWOOD,
	DS_HUGEWOOD,
	DS_MEDIUMSTONE,
	DS_LARGESTONE,
	DS_MEDIUMMETAL,
	DS_FASTSLIDING,
	DS_METALSLIDING,
	DS_HUGESTONE,
	DS_HUGEELEVATOR,
	DS_CRANEWAREHOUSE,
	DS_HAMMERPUMP,
	DS_METALTABLE,
	DS_LABTABLE,
	DS_PISTON,
	DS_CLANG,
	DS_UNDERWATER,
	DS_BAM,
	DS_MAX
} DoorSoundID_t;

// EAX sound presets
enum
{
    EAX_ENVIRONMENT_GENERIC,                // factory default
    EAX_ENVIRONMENT_PADDEDCELL,
    EAX_ENVIRONMENT_ROOM,              // standard environments
    EAX_ENVIRONMENT_BATHROOM,
    EAX_ENVIRONMENT_LIVINGROOM,
    EAX_ENVIRONMENT_STONEROOM,
    EAX_ENVIRONMENT_AUDITORIUM,
    EAX_ENVIRONMENT_CONCERTHALL,
    EAX_ENVIRONMENT_CAVE,
    EAX_ENVIRONMENT_ARENA,
    EAX_ENVIRONMENT_HANGAR,
    EAX_ENVIRONMENT_CARPETEDHALLWAY,
    EAX_ENVIRONMENT_HALLWAY,
    EAX_ENVIRONMENT_STONECORRIDOR,
    EAX_ENVIRONMENT_ALLEY,
    EAX_ENVIRONMENT_FOREST,
    EAX_ENVIRONMENT_CITY,
    EAX_ENVIRONMENT_MOUNTAINS,
    EAX_ENVIRONMENT_QUARRY,
    EAX_ENVIRONMENT_PLAIN,
    EAX_ENVIRONMENT_PARKINGLOT,
    EAX_ENVIRONMENT_SEWERPIPE,
    EAX_ENVIRONMENT_UNDERWATER,
    EAX_ENVIRONMENT_DRUGGED,
    EAX_ENVIRONMENT_DIZZY,
    EAX_ENVIRONMENT_PSYCHOTIC,

    EAX_ENVIRONMENT_COUNT           // total number of environments
};

// EAX world preset types
enum
{
	EAX_GENERIC,
	EAX_ALL_STONE,
	EAX_ARENA,
	EAX_CITY_AND_SEWERS,
	EAX_CITY_AND_ALLEYS,
	EAX_FOREST,
	EAX_PSYCHOTIC,
};

// ************************************************************************************************
// Skin defines
// -----------
// Indicates what skin Corvus has.  
// When indicated on the model, each odd-numbered skin is the damaged version of the previous skin.
// ************************************************************************************************

// For code clarity
#define PLAGUE_NUM_LEVELS 3
#define DAMAGE_NUM_LEVELS 2

#define SKIN_DAMAGED	1
#define SKIN_PLAGUE1	(DAMAGE_NUM_LEVELS * 1)
#define SKIN_PLAGUE2	(DAMAGE_NUM_LEVELS * 2)

//#define SKIN_REFLECTION	(DAMAGE_NUM_LEVELS * PLAGUE_NUM_LEVELS)
#define SKIN_REFLECTION	(DAMAGE_NUM_LEVELS)		// We don't maintain a skin for every plague level anymore.

#define SKIN_MAX		(SKIN_REFLECTION + 1)

#endif	// QSHARED_H
