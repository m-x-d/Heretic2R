//
// q_shared.h -- included first by ALL program modules (unfortunately this is true for all code from id).
//
// Copyright 1998 Raven Software
//

#pragma once

#include <direct.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "H2Common.h"
#include "Heretic2.h"
#include "q_Typedef.h"
#include "Debug.h" //mxd

#define H2R_NORETURN	__declspec(noreturn) //mxd

#define ARRAY_SIZE(a)	(sizeof((a))/sizeof((a)[0])) //mxd. Because it's strange to depend on windows header for this.

//mxd. Player/monster step size. Used by pmove.c, m_move.c and mg_ai.c...
#define STEP_SIZE			18.0f
#define RAT_STEP_DOWN_SIZE	130.0f //mxd. Special case for RATS.

#define MAX_STRING_CHARS	1024	// Max length of a string passed to Cmd_TokenizeString.
#define MAX_STRING_TOKENS	80		// Max tokens resulting from Cmd_TokenizeString.
#define MAX_TOKEN_CHARS		128		// Max length of an individual token.

#define MAX_QPATH			64		// Max length of a quake game pathname.
#define MAX_OSPATH			260		// Q2/H2: 128. Max length of a filesystem pathname (same as MAX_PATH).

// Game print flags.
#define PRINT_LOW			0		// Pickup messages.
#define PRINT_MEDIUM		1		// Death messages.
#define PRINT_HIGH			2		// Critical messages.
#define PRINT_CHAT			3		// Chat messages.
#define PRINT_CAPTION		4		// Captioning at bottom.
#define PRINT_TEAM			5		// Chat message to team members.

#define ERR_FATAL			0		// Exit the entire game with a popup window.
#define ERR_DROP			1		// Print to console and disconnect from game.
#define ERR_DISCONNECT		2		// Don't kill server.

#define PRINT_ALL			0
#define PRINT_DEVELOPER		1		// Only print when "developer 1".
#define PRINT_ALERT			2

#ifndef M_PI
	#define M_PI			3.14159265358979323846f // Matches value in gcc v2 math.h.
#endif

#define TIME_EPSILON		0.01f

#define VectorCopy_Macro(a,b)			((b)[0] = (a)[0], (b)[1] = (a)[1], (b)[2] = (a)[2])

//mxd. paletteRGBA_t handling macros.
#define COLOUR_SET(c, cr, cg, cb)		((c).r=(byte)(cr), (c).g=(byte)(cg), (c).b=(byte)(cb), (c).a=255)
#define COLOUR_SETA(c, cr, cg, cb, ca)	((c).r=(byte)(cr), (c).g=(byte)(cg), (c).b=(byte)(cb), (c).a=(byte)(ca))
#define COLOUR_COPY(src, dst)			((dst).r=(src).r, (dst).g=(src).g, (dst).b=(src).b)
#define COLOUR_COPYA(src, dst)			((dst).r=(src).r, (dst).g=(src).g, (dst).b=(src).b, (dst).a=(src).a)

H2COMMON_API extern int Q_log2(int val);
H2COMMON_API extern int Q_sign(int val); //mxd
H2COMMON_API extern float Q_signf(float val); //mxd

H2COMMON_API extern void ClearBounds(vec3_t mins, vec3_t maxs);
H2COMMON_API extern void AddPointToBounds(const vec3_t v, vec3_t mins, vec3_t maxs);

H2COMMON_API extern float anglemod(float a);
H2COMMON_API extern float anglemod_old(float a);
H2COMMON_API extern float LerpAngle(float a1, float a2, float frac);
H2COMMON_API extern void LerpAngles(const vec3_t angle_a, float frac, const vec3_t angle_b, vec3_t out); //mxd
H2COMMON_API extern float LerpFloat(float f1, float f2, float frac); //mxd

#ifdef __cplusplus //mxd. Needed, so the Script system code could build...
extern "C" {
#endif
	H2COMMON_API extern float SnapAngleToNetworkPrecision(float a); //mxd
#ifdef __cplusplus
}
#endif

typedef enum
{
	P_BLACK, //TODO: unused.
	P_RED,
	P_GREEN,
	P_YELLOW,
	P_BLUE, //TODO: unused.
	P_PURPLE,
	P_CYAN,
	P_WHITE, // Used via colour_names cvar (among other things) --mxd.

	P_HBLACK, //TODO: unused.
	P_HRED, //TODO: unused.
	P_HGREEN, //TODO: unused.
	P_HYELLOW, //TODO: unused.
	P_HBLUE, //TODO: unused.
	P_HPURPLE, //TODO: unused.
	P_HCYAN, //TODO: unused.
	P_HWHITE, //TODO: unused.

	P_DESIGNER, // Used via colour_level cvar --mxd.
	P_PROGRAMMER, // Used via colour_game cvar --mxd.
	P_OBJ_NORMAL,
	P_OBJ_BOLD,
	P_OBIT, // Used via colour_obituary cvar (among other things) --mxd.
	P_CAPTION,
	P_CHAT, // Used via colour_chat cvar --mxd.
	P_TEAM, // Used via colour_teamchat cvar (among other things) --mxd.

	P_VERSION,
	P_FRAGS,
	P_ALTFRAGS,
	P_MENUFIELD,
	P_MSGBOX, //TODO: unused.
	P_HEADER,
	P_CRED_TITLE,
	P_CRED_CONTENT,
	P_FRAGNAME,
	P_ERROR, //mxd

	P_MAX_COLORS //mxd
} PalIdx_t;

extern H2COMMON_API paletteRGBA_t TextPalette[P_MAX_COLORS];

H2COMMON_API extern char* COM_SkipPath(char* pathname);
H2COMMON_API extern void COM_StripExtension(const char* in, char* out);
H2COMMON_API extern void COM_FileBase(const char* in, char* out);
H2COMMON_API extern void COM_FilePath(const char* in, char* out);
H2COMMON_API extern void COM_DefaultExtension(char* path, const char* extension);

H2COMMON_API extern char* COM_Parse(char** data_p);
H2COMMON_API extern void Com_sprintf(char* dest, int size, const char* fmt, ...);
H2COMMON_API extern void Com_PageInMemory(const byte *buffer, int size); //TODO: unused. Remove?


H2COMMON_API extern short BigShort(short l);
H2COMMON_API extern int BigLong(int l);
H2COMMON_API extern float BigFloat(float f);

#define LittleShort(x)	(x)
#define LittleLong(x)	(x)
#define LittleFloat(x)	(x)

H2COMMON_API extern float Clamp(float src, float min, float max);
H2COMMON_API extern int ClampI(int src, int min, int max);
H2COMMON_API extern float Approach(float curr, float dest, float rate);
H2COMMON_API extern char* va(const char* format, ...);

#define pb(v)	((v) ? "TRUE " : "FALSE") //mxd. Print bool.
H2COMMON_API extern char* pv(const vec3_t v); //mxd. vtos() from g_utils.c, basically...
H2COMMON_API extern char* psv(const short* v); //mxd

// Key / value info strings.
#define MAX_INFO_KEY		64
#define MAX_INFO_VALUE		64
#define MAX_INFO_STRING		512

H2COMMON_API extern char* Info_ValueForKey(const char* s, const char* key);
H2COMMON_API extern void Info_RemoveKey(char* s, const char* key);
H2COMMON_API extern void Info_SetValueForKey(char* s, const char* key, const char* value);
H2COMMON_API extern qboolean Info_Validate(const char* s);
H2COMMON_API extern void Set_Com_Printf(void (*toSet)(const char* fmt, ...));

#pragma region ========================== SYSTEM SPECIFIC ==========================

struct cplane_s;

#ifdef __cplusplus //mxd. Needed, so code in game/ds.cpp could build...
extern "C"
{
#endif
H2COMMON_API extern const vec3_t vec3_origin;
H2COMMON_API extern const vec3_t vec3_left; //mxd
H2COMMON_API extern const vec3_t vec3_right; //mxd
H2COMMON_API extern const vec3_t vec3_up;
H2COMMON_API extern const vec3_t vec3_down; //mxd
#ifdef __cplusplus
}
#endif

extern int curtime; // Time returned by last Sys_Milliseconds() call.

extern long long Sys_Microseconds(void); // YQ2
extern void Sys_Nanosleep(int nanosec); // YQ2
extern void Sys_Mkdir(const char* path);
extern qboolean Sys_IsDir(const char* path); // YQ2
extern qboolean Sys_IsFile(const char* path); // YQ2
extern qboolean Sys_GetWorkingDir(char* buffer, size_t len); // YQ2
extern qboolean Sys_GetOSUserDir(char* buffer, size_t len); //mxd

// Directory searching.
#define SFF_ARCH	0x01
#define SFF_HIDDEN	0x02
#define SFF_RDONLY	0x04
#define SFF_SUBDIR	0x08
#define SFF_SYSTEM	0x10

// Pass in an attribute mask of things you wish to REJECT.
extern char* Sys_FindFirst(const char* path, uint musthave, uint canthave);
extern char* Sys_FindNext(uint musthave, uint canthave);
extern void Sys_FindClose(void);

H2R_NORETURN Q2DLL_DECLSPEC extern void Sys_Error(const char* error, ...);

#ifdef __cplusplus //mxd. Needed, so code in game/ds.cpp could build...
extern "C"
{
#endif
Q2DLL_DECLSPEC extern void Com_Printf(const char* fmt, ...);
#ifdef __cplusplus
}
#endif

Q2DLL_DECLSPEC extern void Com_ColourPrintf(PalIdx_t colour, const char* fmt, ...);

#pragma endregion

#pragma region ========================== CVARS (console variables) ==========================

#define CVAR_ARCHIVE	1	// Set to cause it to be saved to vars.rc.
#define CVAR_USERINFO	2	// Added to userinfo  when changed.
#define CVAR_SERVERINFO	4	// Added to serverinfo when changed.
#define CVAR_NOSET		8	// Don't allow change from console at all, but can be set from the command line.
#define CVAR_LATCH		16	// Save changes until server restart.

// Nothing outside the Cvar_*() functions should modify these fields!
typedef struct cvar_s
{
	char* name;
	char* string;
	char* latched_string; // For CVAR_LATCH vars.
	int flags;
	qboolean modified; // Set each time the cvar is changed.
	float value;
	struct cvar_s* next;
} cvar_t;

// Creates the variable if it doesn't exist, or returns the existing one if it exists.
// The value will not be changed, but flags will be ORed in that allows variables to be unarchived without needing bitflags.
Q2DLL_DECLSPEC extern cvar_t* Cvar_Get(const char* var_name, const char* var_value, int flags);

#pragma endregion

#pragma region ========================== Palette info ==========================

typedef struct paletteRGB_s
{
	byte r;
	byte g;
	byte b;
} paletteRGB_t;

#pragma endregion

#pragma region ========================== Additional info for flex models with mesh nodes ==========================

#define MAX_FM_MESH_NODES	16

typedef struct
{
	int frame;
	paletteRGBA_t color;
	byte flags;
	int skin;
} fmnodeinfo_t;

// Flags.
#define FMNI_USE_FRAME			0x1		//TODO: unused
#define FMNI_USE_COLOR			0x2
#define FMNI_USE_SKIN			0x4
#define FMNI_NO_LERP			0x8		//TODO: unused
#define FMNI_NO_DRAW			0x10
#define FMNI_USE_REFLECT		0x20	//TODO: checked, but never set?

#pragma endregion

#pragma region ========================== Contents flags ==========================

// Contents flags are separate bits.
// A given brush can contribute multiple content bits.
// Multiple brushes can be in a single leaf.

// Lower bits are stronger, and will eat weaker brushes completely.
#define CONTENTS_EMPTY			0x00000000 // Nothing.
#define CONTENTS_SOLID			0x00000001 // An eye is never valid in a solid.
#define CONTENTS_WINDOW			0x00000002 // Translucent, but not watery.
#define CONTENTS_ILLUSIONARY	0x00000004 // Was CONTENTS_AUX.
#define CONTENTS_LAVA			0x00000008
#define CONTENTS_SLIME			0x00000010
#define CONTENTS_WATER			0x00000020
#define CONTENTS_MIST			0x00000040
#define LAST_VISIBLE_CONTENTS	CONTENTS_MIST

// Remaining contents are non-visible, and don't eat brushes.
#define CONTENTS_AREAPORTAL		0x00008000
#define CONTENTS_PLAYERCLIP		0x00010000
#define CONTENTS_MONSTERCLIP	0x00020000

// Currents can be added to any other contents, and may be mixed.
#define CONTENTS_CURRENT_0		0x00040000
#define CONTENTS_CURRENT_90		0x00080000
#define CONTENTS_CURRENT_180	0x00100000
#define CONTENTS_CURRENT_270	0x00200000
#define CONTENTS_CURRENT_UP		0x00400000
#define CONTENTS_CURRENT_DOWN	0x00800000

// Removed before bsping an entity.
#define CONTENTS_ORIGIN			0x01000000

// Should never be on a brush, only in game (i.e. set by CM_ functions at map load time).
#define CONTENTS_MONSTER		0x02000000
#define CONTENTS_DEADMONSTER	0x04000000

// Brushes to be added after vis leaves.
#define CONTENTS_DETAIL			0x08000000

// Auto set if any surface has transparency, e.g. water.
#define CONTENTS_TRANSLUCENT	0x10000000

// This flag is special in that it is not stored in the .bsp by QuakeEd.
// It is passed into the trace functions to say that anything with CONTENTS_CAMERANOBLOCK should be ignored.
#define CONTENTS_CAMERABLOCK	0x20000000	// Was CONTENTS_LADDER.

// This flag is special in that it is NOT passed into the trace functions, but may be stored in the
// .bsp by QuakeEd to say that traces with CONTENTS_CAMERABLOCK as the mask will ignore any brushes with this flag.
#define CONTENTS_CAMERANOBLOCK	0x40000000

// Only do the trace against the world, not entities within it. Not stored in the .bsp and passed only as an argument to trace functions.
#define CONTENTS_WORLD_ONLY		0x80000000

#pragma endregion

#pragma region ========================== Contents masks ==========================

#define MASK_ALL			0x7fffffff
#define MASK_SOLID			(CONTENTS_SOLID | CONTENTS_WINDOW)
#define MASK_PLAYERSOLID	(CONTENTS_SOLID | CONTENTS_PLAYERCLIP | CONTENTS_WINDOW | CONTENTS_MONSTER)
#define MASK_DEADSOLID		(CONTENTS_SOLID | CONTENTS_PLAYERCLIP | CONTENTS_WINDOW)
#define MASK_MONSTERSOLID	(CONTENTS_SOLID | CONTENTS_MONSTERCLIP | CONTENTS_WINDOW | CONTENTS_MONSTER)
#define MASK_WATER			(CONTENTS_WATER | CONTENTS_LAVA | CONTENTS_SLIME)
#define MASK_OPAQUE			(CONTENTS_SOLID | CONTENTS_SLIME | CONTENTS_LAVA)
#define MASK_SHOT			(CONTENTS_SOLID | CONTENTS_MONSTER | CONTENTS_WINDOW | CONTENTS_DEADMONSTER)
#define MASK_CURRENT		(CONTENTS_CURRENT_0 | CONTENTS_CURRENT_90 | CONTENTS_CURRENT_180 | CONTENTS_CURRENT_270 | CONTENTS_CURRENT_UP | CONTENTS_CURRENT_DOWN)
#define MASK_DRIP			(CONTENTS_SOLID | CONTENTS_WATER | CONTENTS_LAVA | CONTENTS_SLIME | CONTENTS_WINDOW)

#pragma endregion

#pragma region ========================== Surface flags ==========================

#define SURF_LIGHT			0x1			// Value will hold the light strength.
#define SURF_SLICK			0x2			// Affects game physics.
#define SURF_SKY			0x4			// Don't draw, but add to skybox.
#define SURF_WARP			0x8			// Turbulent water warp.
#define SURF_TRANS33		0x10
#define SURF_TRANS66		0x20
#define SURF_FLOWING		0x40		// Scroll towards angle.
#define SURF_NODRAW			0x80		// Don't bother referencing the texture.
#define SURF_TALL_WALL		0x400		// Face doesn't get broken up as normal.
#define SURF_ALPHA_TEXTURE	0x800		// Texture has alpha in it, and should show through in bsp process.
#define SURF_ANIMSPEED		0x1000		// Value will hold the anim speed in fps.
#define SURF_UNDULATE		0x2000		// Rock surface up and down...
#define SURF_QUAKE			0x4000		// Rock surface up and down when quake value on.

//mxd. Helper define to check for non-lightmapped surfaces.
#define SURF_FULLBRIGHT		(SURF_SKY | SURF_TRANS33 | SURF_TRANS66 | SURF_WARP | SURF_TALL_WALL) // H2: extra SURF_TALL_WALL flag.

#pragma endregion

// gi.BoxEdicts() can return a list of either solid or trigger entities.
// FIXME: eliminate AREA_ distinction?
#define AREA_SOLID		1
#define AREA_TRIGGERS	2

typedef struct cplane_s
{
	vec3_t normal;
	float dist;
	byte type;		// For fast side tests.
	byte signbits;	// signx + (signy << 1) + (signz << 1)
	byte pad[2];
} cplane_t;

typedef struct cmodel_s
{
	vec3_t mins;
	vec3_t maxs;
	vec3_t origin; // For sounds or lights.
	int headnode;
} cmodel_t;

typedef struct csurface_s
{
	char name[40];
	int flags;
	int value;
} csurface_t;

// A trace is returned when a box is swept through the world.
typedef struct trace_s
{
	byte allsolid;		// If true, plane is not valid.
	byte startsolid;	// If true, the initial point was in a solid area.
	byte succeeded;		// Not always set, just in special cases, subjective.
	byte architecture;	// Set if the ??? moved collided with world (not entities).
						// Needed because the player move code doesn't know anything about the location or nature of edicts.
	float fraction;		// Delta between start and end positions, 1.0 = didn't hit anything (traveled full distance).
	vec3_t endpos;		// Final position.
	cplane_t plane;		// Surface normal at impact.
	csurface_t* surface;// Surface hit.
	int contents;		// Contents on other side of surface hit.
	struct edict_s* ent;// Not set by CM_*() functions.
} trace_t;

typedef enum
{
	PM_NORMAL,			// Can accelerate and turn, clips.
	PM_SPECTATOR,		// Can accelerate and turn, no clip.
	PM_DEAD,			// No acceleration or turning, dead.
	PM_GIB,				// No acceleration or turning, dead, different bounding box.
	PM_FREEZE,			// Running a server demo.
	PM_INTERMISSION,	// An existing remote camera view is already underway.
} pmtype_t;

// pmove->pm_flags (a byte!).
#define PMF_STANDSTILL		0x0001
#define PMF_ON_ROPE			0x0002 //mxd
#define PMF_ON_GROUND		0x0004
#define PMF_TIME_LAND		0x0008	// pm_time is time before re-jump.
#define PMF_TIME_TELEPORT	0x0010	// pm_time is non-moving time. // Never set in original logic --mxd.
#define PMF_NO_PREDICTION	0x0020	// Temporarily disables prediction. //TODO: unused.
#define PMF_LOCKMOVE		0x0040
#define PMF_LOCKTURN		0x0080

#define PC_COLLISION		0x0001	// Collided on a move.
#define PC_SLIDING			0x0002	// Sliding down a steep slope.

#define WF_SURFACE			0x0001	// On the surface.
#define WF_DIVE				0x0002	// Dive on next animation.
#define WF_DIVING			0x0004	// Currently diving.
#define WF_SWIMFREE			0x0008	// Currently swimming freely underwater.
#define WF_SINK				0x0010	// Sink below the surface of the water.

// pmove_state_t

// This structure contains the information necessary for client side movement prediction.
// It MUST be communicated bit-accurate from the server to the client to guarantee that prediction stays in sync, so no floats are used.
// If any part of the game code modifies this struct, it will result in a prediction error of some degree.
// NOTE: mxd. Can't change struct size, otherwise compatibility with original game dlls will break!
typedef struct
{
	pmtype_t pm_type;
	short origin[3];	// 12.3
	short velocity[3];	// 12.3
	byte pm_flags;		// Ducked, jump_held, etc.
	byte w_flags;		// Water state.
	byte c_flags;		// Collision.
	byte pm_time;		// Each unit = 8 ms.
	short gravity;

	// Added to command angles to get view direction. Changed by spawns, rotating objects and teleporters.
	short delta_angles[3];
	short camera_delta_angles[3];
} pmove_state_t;

// BUTTON_XXX
#define BUTTON_ATTACK		1
#define BUTTON_DEFEND		2
#define BUTTON_ACTION		4
#define BUTTON_CREEP		8
#define BUTTON_RUN			16
#define BUTTON_AUTOAIM		32
#define BUTTON_LOOKAROUND	64
#define BUTTON_QUICKTURN	128
#define BUTTON_INVENTORY	256
#define BUTTON_ANY			512	// Any key whatsoever.

// Sent to the server each client frame.
// NOTE: mxd. Can't change struct size (36 bytes), otherwise compatibility with original game dlls will break!
typedef struct usercmd_s
{
	byte msec;
	short buttons;
	short angles[3];
	short aimangles[3];
	short camera_vieworigin[3];
	short camera_viewangles[3];
	short forwardmove;
	short sidemove;
	short upmove;
	byte prediction; //mxd. Unused. Can't remove.
	byte lightlevel; // Light level the player is standing on.
} usercmd_t;

#define MAXTOUCH	32

// NOTE: mxd. Can't change struct size (348 bytes), otherwise compatibility with original game dlls will break!
typedef struct
{
	// State (in / out).
	pmove_state_t s;

	// Command (in).
	usercmd_t cmd;
	qboolean snapinitial; //mxd. Unused. Can't remove.
	qboolean run_shrine;
	qboolean high_max;

	// In and out.
	float waterheight;
	float desiredWaterHeight;

	// Results (out).
	int numtouch;
	struct edict_s* touchents[MAXTOUCH];

	float viewheight;
	vec3_t viewangles;			// Clamped.
	vec3_t camera_viewangles;	// Camera angles from client.
	
	float* origin;
	float* velocity;
	vec3_t mins; // Bounding-box size.
	vec3_t maxs;
	float* intentMins;
	float* intentMaxs;

	struct edict_s* groundentity;
	csurface_t* GroundSurface;
	cplane_t GroundPlane;
	int GroundContents;

	int watertype;
	int waterlevel;
	float knockbackfactor;

	// Callbacks to test the world.
	void (*trace)(const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, trace_t* trace);
	int (*pointcontents)(const vec3_t point);

	struct edict_s* self;
} pmove_t;

// entity_state_t->renderfx flags.
#define RF_MINLIGHT			0x00000001		// Always have some light (viewmodel).
#define RF_REFLECTION		0x00000002		// Use GL spherical mapping, if available.
#define RF_WEAPONMODEL		0x00000004		// Only draw through eyes.
#define RF_FULLBRIGHT		0x00000008		// Always draw full intensity.
#define RF_DEPTHHACK		0x00000010		// For view weapon Z crunching.
#define RF_TRANSLUCENT		0x00000020
#define RF_FRAMELERP		0x00000040 //TODO: set, but never used. Frame interpolation is always on.
#define RF_CUSTOMSKIN		0x00000080		// Skin is an index in image_precache. //TODO: unused.
#define RF_GLOW				0x00000100		// Pulse lighting for bonus items.
#define RF_SCALE_XYZ		0x00000200
#define RF_SCALE_XY			0x00000400
#define RF_SCALE_Z			0x00000800
#define RF_PRIMITIVE		0x00001000		// Line, or other primitive runtime generated by the render DLL. //TODO: unused.
#define RF_FIXED			0x00002000		// The sprite has a fixed direction and up vector.
											// By default, a sprite is always oriented to the view (no effect on models).

#define RF_TRANS_ADD		0x00004000		// Additive transparency.
#define RF_TRANS_ADD_ALPHA	0x00008000		// Adds emulation of alpha for additive transparent objects using tint.
#define RF_TRANS_GHOST		0x00010000		// Like subtractive translucency.
#define RF_ALPHA_TEXTURE	0x00020000		// Object has an alpha texture map.
#define RF_LM_COLOR			0x00040000		//mxd. Use lightmap color instead of entity color (sprites without RF_TRANS_ADD or RF_TRANS_GHOST flags only).
#define RF_NODEPTHTEST		0x00080000		// Turns off depth testing for sprites only.
#define RF_IGNORE_REFS		0x00100000		// Don't update the ref points for a model.
#define RF_NODRAW			0x00200000 //TODO: not implemented.
#define RF_CULL_LARGE		0x00400000		// If set on a poly that is really close to the near clip plane and occupies
											// a significant amount of screen real-estate, the poly will be culled. Used for particles in software.

#define RF_TRANS_ANY		(RF_TRANS_ADD | RF_TRANS_GHOST | RF_TRANSLUCENT)

// player_state_t->refdef flags.
#define RDF_UNDERWATER		0x00000001		// Warp the screen as appropriate.
#define RDF_NOWORLDMODEL	0x00000002		// Used for player configuration screen.

// Sound channels.
// Channel 0 never willingly overrides.
// Other channels (1-7) always override a playing sound on that channel.
#define CHAN_AUTO				0
#define CHAN_WEAPON				1
#define CHAN_VOICE				2
#define CHAN_ITEM				3
#define CHAN_BODY				4
#define CHAN_FOOTSTEP			5
#define CHAN_FOOTSTEP2			6
#define CHAN_WEAPON2			7

// Modifier flags.
#define CHAN_NO_PHS_ADD			8	// Send to all clients, not just ones in PHS (ATTN 0 will also do this).
#define CHAN_RELIABLE			16	// Send by reliable message, not datagram.

// Sound attenuation values.
#define ATTN_NONE				0	// Full volume the entire level.
#define ATTN_NORM				1
#define ATTN_IDLE				2
#define ATTN_STATIC				3	// Diminish very rapidly with distance.
#define ATTN_VERYSTATIC			4 //TODO: unused.
#define ATTN_LEFT				256 //TODO: unused.
#define ATTN_RIGHT				512 //TODO: unused.

// player_state->stats[] indexes.

// * - MUST BE SEQUENTIAL!!!
#define STAT_HEALTH_ICON		0		// Icon for health.
#define STAT_HEALTH				1		// Health value.
#define STAT_AMMO_ICON			2		// Icon for ammo.
#define STAT_AMMO				3		// Amount of ammo.
#define STAT_WEAPON_ICON		4		// Current offensive weapon.
#define STAT_WEAPON				5
#define STAT_DEFENCE_ICON		6		// Current defensive weapon.
#define STAT_DEFENCE			7
#define STAT_OFFMANA_ICON		8		// * Icon describing offensive mana.
#define STAT_OFFMANA_BACK		9		// Amount of offensive mana.
#define STAT_OFFMANA			10		// Icon describing defensive mana.
#define STAT_DEFMANA_ICON		11		// * Amount of defensive mana.
#define STAT_DEFMANA_BACK		12
#define STAT_DEFMANA			13
#define STAT_FRAGS_ICON			14
#define STAT_FRAGS				15		// Which status to print.
#define STAT_FLASHES			16		// Cleared each frame, 1 = health.
#define STAT_LAYOUTS			17
#define STAT_PUZZLE_ITEM1		18
#define STAT_PUZZLE_ITEM2		19
#define STAT_PUZZLE_ITEM3		20
#define STAT_PUZZLE_ITEM4		21
#define STAT_PUZZLE_COUNT		22
#define STAT_POWERUP_ICON		23		// * THESE MUST BE SEQUENTIAL !!!!!
#define STAT_POWERUP_BACK		24
#define STAT_POWERUP_TIMER		25
#define STAT_LUNG_ICON			26		// *
#define STAT_LUNG_BACK			27
#define STAT_LUNG_TIMER			28
#define STAT_LIFEBAR_XSIZE		29		// *
#define STAT_LIFEBAR_YSIZE		30
#define STAT_LIFEBAR_ICON		31
#define STAT_LIFEBAR_BACK		32
#define STAT_LIFEBAR_VALUE		33
#define STAT_ARMOUR_ICON		34
#define STAT_ARMOUR				35

#define MAX_STATS				48

// dmflags->value flags.
#define DF_WEAPONS_STAY			0x0001
#define DF_NO_SHRINE			0x0002
#define DF_NONAMES				0x0004
#define DF_NO_HEALTH			0x0008
#define DF_SHOW_LEADER			0x0010
#define DF_SHRINE_CHAOS			0x0020
#define DF_SAME_LEVEL			0x0040
#define DF_FORCE_RESPAWN		0x0080
#define DF_SKINTEAMS			0x0100
#define DF_MODELTEAMS			0x0200
#define DF_ALLOW_EXIT			0x0400
#define DF_INFINITE_MANA		0x0800
#define DF_HURT_FRIENDS			0x1000
#define DF_NO_OFFENSIVE_SPELL	0x2000	// This means staff-only combat!!!
#define DF_NO_DEFENSIVE_SPELL	0x4000
#define DF_DISMEMBER			0x8000

#define DF_DEATHMATCH_SET		0x80000000	// High bit indicates deathmatch, so that it can be transmitted in playerinfo.

#pragma region ========================== ELEMENTS COMMUNICATED ACROSS THE NET ==========================

#define ANGLE2SHORT(x)	((int)((x) * 65536 / 360) & 65535)
#define SHORT2ANGLE(x)	((x) * (360.0f / 65536.0f))

#define DEG2RAD(a) (((a) * M_PI) / 180.0f)

// Config strings are a general means of communication from the server to all connected clients.
// Each config string can be at most MAX_QPATH characters.
#define CS_NAME				0
#define CS_CDTRACK			1
#define CS_SKY				2
#define CS_SKYAXIS			3	// %f %f %f format.
#define CS_SKYROTATE		4
#define CS_LEVEL_NUMBER		5
#define CS_STATUSBAR		6	// Display program string (this seems to take up several slots - hence must be last).
#define CS_MAXCLIENTS		30
#define CS_MAPCHECKSUM		31	// For catching cheater maps.
#define CS_MODELS			32
#define CS_SOUNDS			(CS_MODELS + MAX_MODELS)
#define CS_IMAGES			(CS_SOUNDS + MAX_SOUNDS)
#define CS_LIGHTS			(CS_IMAGES + MAX_IMAGES)
#define CS_ITEMS			(CS_LIGHTS + MAX_LIGHTSTYLES)
#define CS_PLAYERSKINS		(CS_ITEMS + MAX_ITEMS)
#define CS_WELCOME			(CS_PLAYERSKINS + MAX_CLIENTS)  // Give us 4 welcome string messages so we can have a total of 256 characters per message.
#define MAX_CONFIGSTRINGS	(CS_WELCOME + 4)

#pragma endregion

#pragma region ========================== EffectsBuffer_t ==========================

#define ENTITY_FX_BUF_SIZE			192
#define ENTITY_FX_BUF_BLOCK_SIZE	256
#define MAX_PERSISTANT_EFFECTS		512

#define FX_BUF_SIZE16				128 //mxd
#define FX_BUF_MAX_EFFECTS			FX_BUF_SIZE16 //mxd

typedef struct EffectsBuffer_s
{
	byte* buf;
	int bufSize;
	int freeBlock;
	int numEffects;
} EffectsBuffer_t;

typedef struct PerEffectsBuffer_s
{
	byte buf[ENTITY_FX_BUF_SIZE];
	int bufSize;
	int freeBlock;
	int numEffects;
	int send_mask;
	int demo_send_mask;
	int fx_num;
} PerEffectsBuffer_t;

#pragma endregion

#pragma region ========================== entity_state_t ==========================

// This is the information conveyed from the server to clients in an update message, about entities that the client will need to render.
typedef struct entity_state_s
{
	short number;			// Edict index.
	short frame;			// Model's current animation frame index.

	vec3_t origin;			// Model's position and orientation in the world.
	vec3_t angles;			// Model's rotation (in degrees).
	vec3_t old_origin;		// Used for lerping (and hijacked for other uses).

	byte modelindex;		// Tells client which model do draw.
	// byte Padding here.

	short clientnum;		// In Quake 2, the client num was passed in skinnum. We need this value, however.
	
	char skinname[MAX_QPATH];	// For specific path to skin.
	int skinnum;
	
	float scale;			// Model scale.
	int effects;			// EF_XXX.
	int renderfx;			// RF_XXX.
	
	paletteRGBA_t color;	// What's this exactly? //TODO: yeah, what's this?..
	paletteRGB_t absLight;	// Entire absLight gets sent if any component changes.
	
	// Client prediction: 8*(bits 0-4) is x/y radius, 8*(bits 5-9) is z min, 8*(bits 10-15) is z max.
	// Note that gi.linkentity() sets this up.
	short solid;

	byte sound;				// For looping sounds, to guarantee shutoff.
	byte sound_data;		// For looping sounds, so we can set volume and attenuation.

	vec3_t bmodel_origin;	// Required so we can place sounds for bmodels correctly in 3d space
	EffectsBuffer_t clientEffects;	// Header block for list of all client effects attached to this entity.
	fmnodeinfo_t fmnodeinfo[MAX_FM_MESH_NODES];	// Specifies which parts (nodes) of the model are on/off, node colors etc.
	
	short skeletalType;		// Skeletal info. Only relevant for player models (i.e. have U_JOINTED).
	short rootJoint;
	short swapFrame;

	byte usageCount;		// Used by the client to verify is this still the same entity it last had.
} entity_state_t;

#pragma endregion

#pragma region ========================== player_state_t ==========================

// This is the information needed in addition to the 'pmove_state_t' to render a view.
// There will only be 10 'player_state_t's sent each second, but the number of pmove_state_t changes will be relative to client frame rates.
typedef struct
{
	pmove_state_t pmove;		// For prediction (must be communicated bit precise).
	vec3_t viewangles;			// For fixed views and serverdemos.
	
	vec3_t remote_vieworigin;	// For remote camera views.
	vec3_t remote_viewangles;
	int remote_id;

	short viewheight;			// Z displacement of player's head from origin.
	vec3_t offsetangles;		// Deltas added to the player model's client determined angles.
	float fov;					// Horizontal field of view.
	int rdflags;				// Refdef flags used during rendering.
	int AutotargetEntityNum;	// Index of edict currently targeted by the player's auto-targeting facility.
	byte map_percentage;		// For showing the amount of current level completed.
	float fog_density;			// Current fog density for the player.

	short mission_num1;			// Id numbers for primary and secondary mission statements in strings.txt.		
	short mission_num2;

	byte cinematicfreeze;		// Shows if currently in a cinematic
	int PIV;					// Current state of players in this clients view (one bit per player).
	short stats[MAX_STATS];		// Fast status bar updates.

	// These are NEVER sent across the net, but are purely client-side repositories needed by
	// prediction and are filled from the player's entity_state_t.
	int frame;
	int swapFrame;
	vec3_t angles;
	int effects;
	int skinnum;
	int renderfx;
	int clientnum;
	fmnodeinfo_t fmnodeinfo[MAX_FM_MESH_NODES];

	// All the stuff below is required for client prediction to function.

	vec3_t mins;				// Player's accurate mins and maxs.
	vec3_t maxs;

	byte NoOfItems;				// Inventory changes.
	byte inventory_changes[256];
	byte inventory_remaining[256];

	byte NonNullgroundentity;	// Ground attributes. // boolean
	cplane_t GroundPlane;
	int GroundContents;
	csurface_t GroundSurface;

	int watertype;				// Water attributes.
	int waterlevel;		
	float waterheight;

	vec3_t grabloc;				// Grabbing / pullup stuff.
	float grabangle;
	
	float fwdvel;				// Velocities imparted on a per animation frame basis.
	float sidevel;
	float upvel;

	int flags;					// PLAYER_FLAG_XXX.
	int edictflags;				// FL_XXX.

	float oldvelocity_z;		// Player edict's oldvelocity.

	// Animation sequencing state.
	int upperseq;
	int lowerseq;
	int upperframe;
	int lowerframe;
	byte upperidle;
	byte loweridle; // boolean
	int uppermove_index;
	int lowermove_index;
	byte weapon;
	byte defense;
	byte lastweapon;
	byte lastdefense;
	int weaponready;
	int switchtoweapon;
	byte newweapon;
	int weap_ammo_index;
	int def_ammo_index;
	byte weaponcharge;
	byte advancedstaff;
	int armortype;
	byte bowtype;
	byte stafflevel;
	byte helltype;
	byte meteor_count;
	byte handfxtype;
	byte plaguelevel;
	short skintype;
	uint altparts;

	int deadflag; 
	float ideal_yaw;
	float leveltime;
	float idletime;
	float quickturnEndTime;
	float quickturn_rate;
	float powerup_timer;

	// Deathmatch flags.
	int dmflags;
} player_state_t;

#pragma endregion

// The only q_shared.c function...
extern int BoxOnPlaneSide(const vec3_t emins, const vec3_t emaxs, const cplane_t* plane);

#pragma region ========================== Inlines ==========================

//mxd
_inline int Q_atoi(const char* s)
{
	return strtol(s, NULL, 10);
}

_inline int Q_stricmp(const char* s1, const char* s2)
{
#ifdef WIN32
	return _stricmp(s1, s2);
#else
	return strcasecmp(s1, s2);
#endif
}

_inline int Q_strncasecmp(const char* s1, const char* s2, const int n)
{
	return _strnicmp(s1, s2, n);
}

_inline int Q_strcasecmp(const char* s1, const char* s2) //mxd. Unused.
{
	return Q_stricmp(s1, s2);
}

#pragma endregion

#pragma region ========================== Sound flags ==========================

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
	AS_BUBBLES,			// 10

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
	AS_SPIT,			// 39

	AS_MAX
} AmbientSoundID_t;

typedef enum DoorSoundID_e
{
	DS_NONE,
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

// EAX sound presets.
enum
{
	EAX_ENVIRONMENT_GENERIC,	// Factory default.
	EAX_ENVIRONMENT_PADDEDCELL,
	EAX_ENVIRONMENT_ROOM,		// Standard environments.
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

	EAX_ENVIRONMENT_COUNT	// Total number of environments.
};

// EAX world preset types.
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

#pragma endregion

#pragma region ========================== Skin defines ==========================

// Indicates what skin Corvus has.
// When indicated on the model, each odd-numbered skin is the damaged version of the previous skin.

// For code clarity.
#define PLAGUE_NUM_LEVELS 3
#define DAMAGE_NUM_LEVELS 2

#define SKIN_DAMAGED	1
#define SKIN_PLAGUE1	(DAMAGE_NUM_LEVELS * 1)
#define SKIN_PLAGUE2	(DAMAGE_NUM_LEVELS * 2)
#define SKIN_REFLECTION	(DAMAGE_NUM_LEVELS)	// We don't maintain a skin for every plague level anymore.
#define SKIN_MAX		(SKIN_REFLECTION + 1)

#pragma endregion