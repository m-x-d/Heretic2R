//
// gl1_Local.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include <glad-GL1.3/glad.h> // Must be included before SDL.
#include "gl1_Debug.h"
#include "ref.h"

#define REF_TITLE			"OpenGL 1.3" // H2_1.07: "GL 2.1"; Q2: "GL 0.01".

#define MAX_TEXTURE_UNITS	2

#define TEXNUM_LIGHTMAPS	1024
#define TEXNUM_IMAGES		1153

#define MAX_GLTEXTURES		2048 // Q2: 1024

#define GL_TEX_SOLID_FORMAT 3 //mxd. gl_tex_solid_format var in Q2
#define GL_TEX_ALPHA_FORMAT 4 //mxd. gl_tex_alpha_format var in Q2

#pragma region ========================== CVARS ==========================

extern cvar_t* r_norefresh;
extern cvar_t* r_fullbright;
extern cvar_t* r_drawentities;
extern cvar_t* r_drawworld;
extern cvar_t* r_novis;
extern cvar_t* r_nocull;
extern cvar_t* r_lerpmodels;
extern cvar_t* r_vsync; // YQ2

extern cvar_t* r_lightlevel;

extern cvar_t* r_farclipdist;
extern cvar_t* r_fog;
extern cvar_t* r_fog_mode;
extern cvar_t* r_fog_density;
extern cvar_t* r_fog_startdist;
extern cvar_t* r_fog_lightmap_adjust;
extern cvar_t* r_fog_underwater;
extern cvar_t* r_fog_underwater_lightmap_adjust;
extern cvar_t* r_frameswap;
extern cvar_t* r_references;

extern cvar_t* gl_noartifacts;

extern cvar_t* gl_modulate;
extern cvar_t* gl_lightmap;
extern cvar_t* gl_dynamic;
extern cvar_t* gl_nobind;
extern cvar_t* gl_showtris;
extern cvar_t* gl_flashblend;
extern cvar_t* gl_texturemode;
extern cvar_t* gl_lockpvs;

extern cvar_t* gl_drawflat;
extern cvar_t* gl_trans33;
extern cvar_t* gl_trans66;
extern cvar_t* gl_bookalpha;

extern cvar_t* gl_drawbuffer;
extern cvar_t* gl_saturatelighting;

extern cvar_t* vid_gamma;
extern cvar_t* vid_brightness;
extern cvar_t* vid_contrast;

extern cvar_t* vid_ref;

extern cvar_t* vid_mode;
extern cvar_t* menus_active;
extern cvar_t* cl_camera_under_surface;
extern cvar_t* quake_amount;

#pragma endregion

typedef enum //mxd. Changed in H2
{
	it_skin = 1,
	it_sprite = 2,
	it_wall = 4,
	it_pic = 5,
	it_sky = 6
} imagetype_t;

typedef struct image_s //mxd. Changed in H2. Original size: 104 bytes
{
	struct image_s* next;
	char name[MAX_QPATH];				// Game path, including extension.
	imagetype_t type;
	int width;
	int height;
	int registration_sequence;			// 0 = free
	struct msurface_s* texturechain;	// For sort-by-texture world drawing.
	struct msurface_s* multitexturechain;
	int texnum;							// gl texture binding.
	byte has_alpha;
	byte num_frames;
	struct paletteRGB_s* palette;		// .M8 palette.
} image_t;

typedef enum
{
	RSERR_OK,
	RSERR_INVALID_MODE
} rserr_t;

extern float gldepthmin;
extern float gldepthmax;

extern image_t gltextures[MAX_GLTEXTURES];
extern int numgltextures;

extern image_t* r_notexture;
extern image_t* r_particletexture;
extern image_t* r_aparticletexture;
extern image_t* r_reflecttexture;
extern image_t* r_font1;
extern image_t* r_font2;

#include "gl1_Model.h" //mxd. MUST be below image_t definition...

extern model_t* r_worldmodel;

#pragma region ========================== GL config stuff  ==========================

typedef struct
{
	const char* renderer_string;
	const char* vendor_string;
	const char* version_string;
	const char* extensions_string;
	float max_anisotropy; // YQ2
} glconfig_t;

typedef struct
{
	float inverse_intensity;
	qboolean fullscreen;

	int prev_mode;
	int lightmap_textures;
	int currenttextures[MAX_TEXTURE_UNITS];
	int currenttmu;
} glstate_t;

extern glconfig_t gl_config;
extern glstate_t gl_state;

extern vec3_t vup;
extern vec3_t vpn;
extern vec3_t vright;
extern vec3_t r_origin;

extern float r_world_matrix[16];
extern float r_projection_matrix[16]; //mxd
extern cplane_t frustum[4];

extern refdef_t r_newrefdef;

extern int r_framecount;

extern int r_viewcluster;
extern int r_viewcluster2;
extern int r_oldviewcluster;
extern int r_oldviewcluster2;

extern int c_brush_polys;
extern int c_alias_polys;

#pragma endregion

#pragma region ========================== IMPORTED FUNCTIONS ==========================

extern refimport_t ri;

#pragma endregion