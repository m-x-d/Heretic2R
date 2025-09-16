//
// ref.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "qcommon.h"

#define REF_API_VERSION		4 // H2: 3. Changed because of H2R rendering API changes.

// These are the maximum number that may be rendered on any given frame.
#define MAX_DLIGHTS			32
#define MAX_ENTITIES		128	
#define MAX_ALPHA_ENTITIES	(MAX_ENTITIES * 2) //TODO: increase? Exceeded relatively often...
#define MAX_SERVER_ENTITIES	MAX_ENTITIES
#define MAX_PARTICLES		4096
#define MAX_LIGHTSTYLES		256

#define NUM_PARTICLE_TYPES	62 // This doesn't use the macro because of referencing weirdness.

#define CONCHAR_SIZE		8 //mxd. Conchars char size. Each char is 8x8 pixels.
#define CONCHAR_LINE_HEIGHT	10 //mxd. Not a console line height!

typedef struct entity_s
{
	struct model_s** model; // Opaque type outside refresh. // Q2: struct model_s*
	float angles[3];
	float origin[3];
	int frame;

	// Model scale.
	float scale;

	// Scale of model - but only for client entity models - not server-side models.
	// Required for scaling mins and maxs that are used to cull models - mins and maxs are scaled on the server side,
	// but not on the client side when the models are loaded in.
	float cl_scale;

	// Distance to the camera origin, gets set every frame by AddEffectsToView.
	float depth;

	paletteRGBA_t color;
	int flags;

	union
	{
		int rootJoint;	// rootJoint of the entities skeleton.
		int spriteType;
	};

	union
	{
		// Info for fmodels and bmodels.
		struct
		{
			float oldorigin[3];	
			int oldframe;

			float backlerp;	// 0.0 = current, 1.0 = old.

			int skinnum;
			struct image_s* skin;		// NULL for inline skin.
			struct image_s** skins;		// Pointer to the list of clientinfo skins.
			char skinname[MAX_QPATH];	// For specific path to skin.

			paletteRGB_t absLight;
			byte padFor_3byte_absLight;

			// Client entities which use a flexible model will need to fill this in, and then release it when they die.
			// Happily most client entities are sprites.
			fmnodeinfo_t* fmnodeinfo;

			int swapCluster;	// Cluster to swap on.

			int swapFrame;		// Frame to swap clustered verts in for.
			int oldSwapFrame;	// Previous frame to swap clustered verts in for.

			struct LERPedReferences_s* referenceInfo;
			struct LERPedReferences_s* oldReferenceInfo;

			int padToUnionSize[4];	// Use this space up to add any more non-sprite fields that may be needed.
		};

		// Info for dynamic sprites.
		float verts[4][4];			// verts for dynamic sprites [x, y, s, t].

		// Info for variable sprites.
		struct
		{
			float (*verts_p)[4];		// Pointer to verts for variable sprites.
			int numVerts;
			int padToUnionSize2[11];	// Use this space up to add any more variable sprite fields.
		};

		// Info for line sprites.
		struct
		{
			float startpos[3];
			float endpos[3];
			float scale2;
			float tile;
			float tileoffset;
			int padToUnionSize3[7];		// Use this space up to add any more line sprite fields.
		};
	};
} entity_t;

typedef struct dlight_s
{
	vec3_t origin;
	paletteRGBA_t color;
	float intensity;
} dlight_t;

typedef struct lightstyle_s
{
	float rgb[3];	// 0.0 - 2.0.
	float white;	// Highest of rgb.
} lightstyle_t;

typedef struct particle_s
{
	vec3_t origin;
	paletteRGBA_t color;
	float scale;
	int type;
} particle_t;

typedef struct refdef_s
{
	int x;	// In virtual screen coordinates.
	int y;
	int width;
	int height;
	int area;

	float fov_x;
	float fov_y;
	float vieworg[3];
	float clientmodelorg[3];
	float viewangles[3];
	float blend[4];	// rgba 0-1 full screen blend.
	float time;		// Time used to auto-animate.
	int rdflags;	// RDF_UNDERWATER, etc.

	byte* areabits;	// If not NULL, only areas with set bits will be drawn.

	lightstyle_t* lightstyles;	// [MAX_LIGHTSTYLES]

	int num_entities;
	entity_t** entities; // Q2: entity_t*

	int num_alpha_entities;
	entity_t** alpha_entities;

	int num_dlights;
	dlight_t* dlights;

	int num_particles;
	particle_t* particles;

	int anum_particles;
	particle_t* aparticles;
} refdef_t;

//mxd. Scaling modes for DrawStretchPic().
typedef enum DrawStretchPicScaleMode_e
{
	DSP_NONE,
	DSP_SCALE_SCREEN,	// Scale to viddef.width x viddef.height rectangle.
	DSP_SCALE_4x3,		// Scale to centered 4x3 rectangle.
} DrawStretchPicScaleMode_t;

// Functions exported by the refresh module.
typedef struct refexport_s
{
	// If api_version is different, the dll cannot be used.
	int api_version;

	// Refresh library title (like "OpenGL 1.3").
	const char* title; //mxd

	// Called when the library is loaded.
	qboolean (*Init)(void);

	// Called before the library is unloaded.
	void (*Shutdown)(void);

	// All data that will be used in a level should be registered before rendering any frames to prevent disk hits,
	// but they can still be registered at a later time if necessary.

	// EndRegistration will free any remaining data that wasn't registered.
	// Any model_s or skin_s pointers from before the BeginRegistration are no longer valid after EndRegistration.

	// Skins and images need to be differentiated, because skins are flood filled to eliminate mip map edge errors,
	// and pics have an implicit "pics/" prepended to the name (a pic name that starts with a slash will not use the "pics/" prefix or the ".pcx" postfix).
	void (*BeginRegistration)(const char* map);
	struct model_s* (*RegisterModel)(const char* name);
	struct image_s* (*RegisterSkin)(const char* name, qboolean* retval);
	struct image_s* (*RegisterPic)(const char* name);
	void (*SetSky)(const char* name, float rotate, const vec3_t axis);
	void (*EndRegistration)(void);

	int (*GetReferencedID)(const struct model_s* model);

	int (*RenderFrame)(const refdef_t* fd);

	void (*DrawGetPicSize)(int* w, int* h, const char* name);
	void (*DrawPic)(int x, int y, int scale, const char* name, float alpha); //mxd. +scale arg.
	void (*DrawStretchPic)(int x, int y, int w, int h, const char* name, float alpha, DrawStretchPicScaleMode_t mode); //mxd. qboolean scale -> DrawStretchPicScaleMode_t mode.
	void (*DrawChar)(int x, int y, int scale, int c, paletteRGBA_t color); //mxd. +scale arg.
	void (*DrawTileClear)(int x, int y, int w, int h, const char* name);
	void (*DrawFill)(int x, int y, int w, int h, paletteRGBA_t color); //mxd. r, g, b -> color.
	void (*DrawFadeScreen)(paletteRGBA_t color);
	void (*DrawBigFont)(int x, int y, const char* text, float alpha);
	int (*BF_Strlen)(const char* text);
	void (*BookDrawPic)(const char* name, float scale);

	// Draw images for cinematic rendering (which can have a different palette).
	void (*DrawInitCinematic)(int width, int height);
	void (*DrawCloseCinematic)(void);
	void (*DrawCinematic)(const byte* data, const paletteRGB_t* palette);
	void (*Draw_Name)(const vec3_t origin, const char* name, paletteRGBA_t color);

	// Video mode and refresh state management entry points.
	void (*BeginFrame)(float camera_separation);
	void (*EndFrame)(void);

	int (*FindSurface)(const vec3_t start, const vec3_t end, struct Surface_s* surface);

#ifdef __A3D_GEOM
	void (*A3D_RenderGeometry)(void* pA3D, void* pGeom, void* pMat, void* pGeomStatus);
#endif

	// Called by GLimp_InitGraphics() *after* creating window, passing the SDL_Window* (void* so we don't spill SDL.h here).
	qboolean (*InitContext)(void* sdl_window); // YQ2

	// Shuts down rendering (OpenGL) context.
	void (*ShutdownContext)(void); // YQ2

	// Called by GLimp_InitGraphics() before creating window, returns flags for SDL window creation, returns -1 on error.
	int (*PrepareForWindow)(void); // YQ2

#ifdef _DEBUG
	//mxd. Debug draw logic.
	void (*AddDebugBox)(const vec3_t center, float size, paletteRGBA_t color, float lifetime);
	void (*AddDebugBbox)(const vec3_t mins, const vec3_t maxs, paletteRGBA_t color, float lifetime);
	void (*AddDebugEntityBbox)(const edict_t* ent, paletteRGBA_t color);

	void (*AddDebugLine)(const vec3_t start, const vec3_t end, paletteRGBA_t color, float lifetime);
	void (*AddDebugArrow)(const vec3_t start, const vec3_t end, paletteRGBA_t color, float lifetime);
	void (*AddDebugDirection)(const vec3_t start, const vec3_t angles_deg, float size, paletteRGBA_t color, float lifetime);
	void (*AddDebugMarker)(const vec3_t center, float size, paletteRGBA_t color, float lifetime);
#endif
} refexport_t;

// Functions imported by the refresh module.
typedef struct refimport_s
{
	struct CL_SkeletalJoint_s* skeletalJoints;
	struct ArrayedListNode_s* jointNodes;

	void (*Sys_Error)(int err_level, const char* fmt, ...);
	void (*Com_Error)(int code, const char* fmt, ...);
	void (*Con_Printf)(int print_level, const char* fmt, ...);

	cvar_t* (*Cvar_Get)(const char* name, const char* value, int flags);
	cvar_t* (*Cvar_Set)(const char* name, const char* value);
	void (*Cvar_SetValue)(const char* name, float value);

	void (*Cmd_AddCommand)(const char* name, void (*cmd)(void));
	void (*Cmd_RemoveCommand)(const char* name);

	// This is used for the screen flash - there is a reason for doing this...
	int (*Is_Screen_Flashing)(void);
	void (*Deactivate_Screen_Flash)(void);

	// Files will be memory mapped read only.
	// The returned buffer may be part of a larger pak file, or a discrete file from anywhere in the quake search path.
	// A -1 return means the file does not exist.
	// NULL can be passed for buf to just determine existence.
	int (*FS_LoadFile)(const char* name, void** buf);
	void (*FS_FreeFile)(void* buf);

	// Called with image data of width * height pixel which 'comp' bytes per pixel (must be 3 or 4 for RGB or RGBA).
	// Expects the pixels data to be row-wise, starting at top left.
	void (*Vid_WriteScreenshot)(int width, int height, int comp, const void* data); // YQ2

	qboolean (*Vid_GetModeInfo)(int* width, int* height, int mode);
	qboolean (*GLimp_InitGraphics)(int width, int height); // YQ2

#ifdef _DEBUG
	//mxd. Debug logic.
	char* (*pv)(const vec3_t v); // vtos() from g_utils.c, basically...
	char* (*psv)(const short* v);

	void (*DBG_IDEPrint)(const char* fmt, ...);
	void (*DBG_HudPrint)(int slot, const char* label, const char* fmt, ...);
#endif
} refimport_t;

// This is the only function actually exported at the linker level.
typedef refexport_t (*GetRefAPI_t)(refimport_t);