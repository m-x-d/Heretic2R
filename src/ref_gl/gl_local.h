//
// gl_local.h
//
// Copyright 1998 Raven Software
//

#pragma once

#ifdef _WIN32
	#include <windows.h>
#endif

#include <GL/gl.h>
#include "ref.h"
#include "qgl.h"

#define	REF_VERSION	"GL 2.1" //mxd. GL 0.01 in Q2

typedef enum //mxd. Changed in H2
{
	it_undefined,
	it_skin,
	it_sprite,
	it_wall1, //TODO: better name...
	it_wall2, //TODO: better name...
	it_pic, //mxd. Or it_book?
	it_sky,
	it_wall3 //TODO: better name...
} imagetype_t;

typedef struct image_s //mxd. Changed in H2. Original size: 104 bytes
{
	struct image_s* next;
	char name[MAX_QPATH];				// Game path, including extension
	imagetype_t type;
	int width;
	int height;
	int registration_sequence;			// 0 = free
	struct msurface_t* texturechain;	// For sort-by-texture world drawing
	struct msurface_t* multitexturechain;
	int texnum;							// gl texture binding
	byte has_alpha;
	byte num_frames;
	//byte pad[2];
	struct palette_s* palette;			// .M8 palette
} image_t;

// New in H2. Font character definition struct
typedef struct glxy_s
{
	float xl;
	float yt;
	float xr;
	float yb;
	int w;
	int h;
	int baseline;
} glxy_t;

#define TEXNUM_IMAGES	1153

#define MAX_GLTEXTURES	2048 // Q2: 1024

typedef enum
{
	rserr_ok,
	rserr_invalid_fullscreen,
	rserr_invalid_mode,
	rserr_unknown
} rserr_t;

#include "gl_model.h" //mxd. MUST be below image_t definition...

extern float gldepthmin;
extern float gldepthmax;

extern image_t gltextures[MAX_GLTEXTURES];
extern int numgltextures;

extern qboolean disablerendering; // New in H2 //TODO: unused. Remove?

extern image_t* r_notexture;
extern image_t* r_particletexture;
extern image_t* r_aparticletexture; // New in H2
extern image_t* r_reflecttexture; // New in H2
extern image_t* draw_chars; // Local to gl_draw.c in H2
extern image_t* r_font1; // New in H2
extern image_t* r_font2; // New in H2

#pragma region ========================== CVARS  ==========================

extern cvar_t* r_norefresh;
extern cvar_t* r_fullbright;
extern cvar_t* r_drawentities;
extern cvar_t* r_drawworld;
extern cvar_t* r_novis;
extern cvar_t* r_nocull;
extern cvar_t* r_lerpmodels;
extern cvar_t* r_speeds;

extern cvar_t* r_lightlevel;

extern cvar_t* r_farclipdist;
extern cvar_t* r_fog;
extern cvar_t* r_fog_mode;
extern cvar_t* r_fog_density;
extern cvar_t* r_fog_startdist;
extern cvar_t* r_fog_color_r;
extern cvar_t* r_fog_color_g;
extern cvar_t* r_fog_color_b;
extern cvar_t* r_fog_color_a;
extern cvar_t* r_fog_color_scale;
extern cvar_t* r_fog_lightmap_adjust;
extern cvar_t* r_fog_underwater;
extern cvar_t* r_fog_underwater_mode;
extern cvar_t* r_fog_underwater_density;
extern cvar_t* r_fog_underwater_startdist;
extern cvar_t* r_fog_underwater_color_r;
extern cvar_t* r_fog_underwater_color_g;
extern cvar_t* r_fog_underwater_color_b;
extern cvar_t* r_fog_underwater_color_a;
extern cvar_t* r_fog_underwater_color_scale;
extern cvar_t* r_fog_underwater_lightmap_adjust;
extern cvar_t* r_underwater_color;
extern cvar_t* r_frameswap;
extern cvar_t* r_references;

extern cvar_t* gl_nosubimage;

extern cvar_t* gl_particle_min_size;
extern cvar_t* gl_particle_max_size;
extern cvar_t* gl_particle_size;
extern cvar_t* gl_particle_att_a;
extern cvar_t* gl_particle_att_b;
extern cvar_t* gl_particle_att_c;
extern cvar_t* gl_noartifacts;

extern cvar_t* gl_modulate;
extern cvar_t* gl_log;
extern cvar_t* gl_bitdepth;
extern cvar_t* gl_lightmap;
extern cvar_t* gl_shadows;
extern cvar_t* gl_dynamic;
extern cvar_t* gl_nobind;
extern cvar_t* gl_round_down;
extern cvar_t* gl_showtris;
extern cvar_t* gl_reporthash;
extern cvar_t* gl_ztrick;
extern cvar_t* gl_finish;
extern cvar_t* gl_clear;
extern cvar_t* gl_cull;
extern cvar_t* gl_polyblend;
extern cvar_t* gl_flashblend;
extern cvar_t* gl_playermip;
extern cvar_t* gl_monolightmap; //TODO: remove
extern cvar_t* gl_driver; //TODO: remove
extern cvar_t* gl_texturemode;
extern cvar_t* gl_lockpvs;

extern cvar_t* gl_drawflat;
extern cvar_t* gl_devel1;
extern cvar_t* gl_trans33;
extern cvar_t* gl_trans66;
extern cvar_t* gl_picmip;
extern cvar_t* gl_skinmip;
extern cvar_t* gl_bookalpha;

extern cvar_t* gl_ext_swapinterval;
extern cvar_t* gl_ext_gamma;
extern cvar_t* gl_ext_palettedtexture;
extern cvar_t* gl_ext_multitexture;
extern cvar_t* gl_ext_pointparameters;
extern cvar_t* gl_drawmode;

extern cvar_t* gl_drawbuffer;
extern cvar_t* gl_swapinterval;
extern cvar_t* gl_sortmulti;

extern cvar_t* gl_saturatelighting;

//TODO: remove
extern cvar_t* gl_3dlabs_broken;
extern cvar_t* gl_lostfocus_broken;
extern cvar_t* gl_fog_broken;
extern cvar_t* gl_envmap_broken;
extern cvar_t* gl_screenshot_broken;

extern cvar_t* vid_fullscreen;
extern cvar_t* vid_gamma;
extern cvar_t* vid_brightness;
extern cvar_t* vid_contrast;

extern cvar_t* vid_ref;

extern cvar_t* vid_mode;
extern cvar_t* menus_active;
extern cvar_t* cl_camera_under_surface;
extern cvar_t* quake_amount;

#pragma endregion

// gl_draw.c
void Draw_InitLocal(void);
void Draw_GetPicSize(int* w, int* h, char* name);
void Draw_Pic(int x, int y, char* name, float alpha);
void Draw_StretchPic(int x, int y, int w, int h, char* name, float alpha, qboolean scale);
void Draw_Char(int x, int y, int c, paletteRGBA_t color);
image_t* Draw_FindPic(char* name); // Q2: not in gl_local.h
void Draw_TileClear(int x, int y, int w, int h, char* pic);
void Draw_Fill(int x, int y, int w, int h, byte r, byte g, byte b);
void Draw_FadeScreen(paletteRGBA_t color);

// gl_draw_book.c
int BF_Strlen(char* text); // New in H2
void Draw_BigFont(int x, int y, char* text, float alpha); // New in H2
void Draw_BookPic(int w, int h, char* name, float scale); // New in H2

// gl_image.c
#define GL_TEX_SOLID_FORMAT 3 //mxd. gl_tex_solid_format var in Q2
#define GL_TEX_ALPHA_FORMAT 4 //mxd. gl_tex_alpha_format var in Q2

extern int gl_filter_min;
extern int gl_filter_max;

void InitGammaTable(void); // New in H2
image_t* GL_GetFreeImage(void); // New in H2
void GL_TexEnv(GLint mode);
void GL_BindImage(const image_t* image); // New in H2
void GL_TextureMode(char* string);
void GL_SetFilter(const image_t* image); // New in H2
void GL_ImageList_f(void);
void GL_UploadPaletted(int level, const byte* data, const palette_t* palette, int width, int height); // New in H2
image_t* GL_FindImage(char* name, imagetype_t type);
void GL_FreeImageNoHash(image_t* image); // New in H2
void GL_FreeUnusedImages(void);
void GL_GammaAffect(void); // New in H2
void GL_InitImages(void);
void GL_ShutdownImages(void);

// gl_misc.c
void GL_ScreenShot_f(void);
void GL_Strings_f(void);
void GL_SetDefaultState(void);
void GL_UpdateSwapInterval(void);

// gl_model.c
extern int registration_sequence;

void Mod_Modellist_f(void);

#pragma region ========================== GL config stuff  ==========================

#define GL_RENDERER_DEFAULT 0 //mxd. Don't bother with ancient videocards

// Q2 counterpart
typedef struct
{
	int renderer;
	const char* renderer_string;
	const char* vendor_string;
	const char* version_string;
	const char* extensions_string;

	qboolean allow_cds; //TODO: always true. Remove?
} glconfig_t;

typedef struct
{
	float inverse_intensity;
	qboolean fullscreen;

	int prev_mode;
	// Missing: unsigned char *d_16to8table;
	int lightmap_textures;
	int	currenttextures[2];
	int currenttmu;

	//TODO: Broken implementation (http://www.benryves.com/products/stereoquake/). Probably unusable nowadays. Remove?
	float camera_separation;
	qboolean stereo_enabled;

	//TODO: unused?
	byte originalRedGammaTable[256];
	byte originalGreenGammaTable[256];
	byte originalBlueGammaTable[256];
} glstate_t;

extern glconfig_t gl_config;
extern glstate_t gl_state;

#pragma endregion

#pragma region ========================== IMPORTED FUNCTIONS ==========================

extern refimport_t ri;

#pragma endregion

#pragma region ========================== IMPLEMENTATION-SPECIFIC FUNCTIONS ==========================

// glw_imp.c
void GLimp_BeginFrame(float camera_separation);
void GLimp_EndFrame(void);
qboolean GLimp_Init(void* hinstance, void* hWnd); //mxd. Changed return type
void GLimp_Shutdown(void);
rserr_t GLimp_SetMode(int* pwidth, int* pheight, int mode, qboolean fullscreen, qboolean create_window);
void GLimp_AppActivate(qboolean active);
void GLimp_EnableLogging(qboolean enable);
void GLimp_LogNewFrame(void);

#pragma endregion