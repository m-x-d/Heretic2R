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

#define REF_VERSION	"GL 2.0" // H2_1.07: "GL 2.1"; Q2: "GL 0.01".

typedef enum //mxd. Changed in H2
{
	it_skin		= 1,
	it_sprite	= 2,
	it_wall		= 4,
	it_pic		= 5,
	it_sky		= 6
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

// H2. Font character definition struct
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

#define TEXNUM_LIGHTMAPS	1024
#define TEXNUM_IMAGES		1153

#define MAX_GLTEXTURES		2048 // Q2: 1024

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

extern qboolean disablerendering; // H2 //TODO: unused. Remove?

extern image_t* r_notexture;
extern image_t* r_particletexture;
extern image_t* r_aparticletexture; // H2
extern image_t* r_reflecttexture; // H2
extern image_t* draw_chars; // H2. Local to gl_draw.c in original logic.
extern image_t* r_font1; // H2
extern image_t* r_font2; // H2

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
extern glxy_t* font1; // H2
extern glxy_t* font2; // H2
extern qboolean gl_alphatest_broken; // H2 //TODO: remove?

extern void Draw_InitLocal(void);
extern void Draw_GetPicSize(int* w, int* h, const char* name);
extern void Draw_Render(int x, int y, int w, int h, const image_t* image, float alpha); // H2
extern void Draw_Pic(int x, int y, const char* name, float alpha);
extern void Draw_StretchPic(int x, int y, int w, int h, const char* name, float alpha, qboolean scale);
extern void Draw_Char(int x, int y, int c, paletteRGBA_t color);
extern image_t* Draw_FindPic(const char* name); // Q2: not in gl_local.h
extern void Draw_TileClear(int x, int y, int w, int h, const char* pic);
extern void Draw_Fill(int x, int y, int w, int h, byte r, byte g, byte b);
extern void Draw_FadeScreen(paletteRGBA_t color);
extern void ShutdownFonts(void); // H2

// gl_draw_book.c
extern int BF_Strlen(const char* text); // H2
extern void Draw_BigFont(int x, int y, const char* text, float alpha); // H2
extern void Draw_BookPic(int w, int h, const char* name, float scale); // H2

// gl_image.c
#define GL_TEX_SOLID_FORMAT 3 //mxd. gl_tex_solid_format var in Q2
#define GL_TEX_ALPHA_FORMAT 4 //mxd. gl_tex_alpha_format var in Q2

extern int gl_filter_min;
extern int gl_filter_max;

extern void InitGammaTable(void); // H2
extern image_t* GL_GetFreeImage(void); // H2
extern void GL_EnableMultitexture(qboolean enable);
extern void GL_SelectTexture(GLenum texture);
extern void GL_TexEnv(GLint mode);
extern void GL_Bind(int texnum);
extern void GL_BindImage(const image_t* image); // H2
extern void GL_MBind(GLenum target, int texnum);
extern void GL_MBindImage(GLenum target, const image_t* image); // H2
extern void GL_TextureMode(const char* string);
extern void GL_SetFilter(const image_t* image); // H2
extern void GL_ImageList_f(void);
extern void GL_UploadPaletted(int level, const byte* data, const paletteRGB_t* palette, int width, int height); // H2
extern image_t* GL_FindImage(const char* name, imagetype_t type);
extern void GL_FreeImageNoHash(image_t* image); // H2
extern void GL_FreeUnusedImages(void);
extern void GL_GammaAffect(void); // H2
extern void GL_InitImages(void);
extern void GL_ShutdownImages(void);
extern void GL_DisplayHashTable(void); // H2

// gl_light.c
extern void R_RenderDlights(void);
extern void R_MarkLights(dlight_t* light, int bit, const mnode_t* node);
extern void R_PushDlights(void);
extern void R_LightPoint(vec3_t p, vec3_t color);
extern void R_SetCacheState(msurface_t* surf);
extern void R_BuildLightMap(msurface_t* surf, byte* dest, int stride);

// gl_main.c
extern vec3_t vup;
extern vec3_t vpn;
extern vec3_t vright;
extern vec3_t r_origin;

extern float r_world_matrix[16];

extern refdef_t r_newrefdef;
extern int c_brush_polys;
extern int c_alias_polys;
extern int r_viewcluster;
extern int r_viewcluster2;
extern int r_oldviewcluster;
extern int r_oldviewcluster2;

extern model_t* r_worldmodel;
extern entity_t* currententity;
extern model_t* currentmodel;
extern cplane_t frustum[4];
extern int r_framecount;

extern qboolean R_CullBox(vec3_t mins, vec3_t maxs);
extern void R_RotateForEntity(const entity_t* e);
extern void HandleTrans(const entity_t* e); // H2
extern void CleanupTrans(const entity_t* e); // H2
extern void R_DrawNullModel(void); // Q2: local to gl_rmain.c

// gl_misc.c
extern void GL_ScreenShot_f(void);
extern void GL_Strings_f(void);
extern void GL_SetDefaultState(void);
extern void GL_UpdateSwapInterval(void);
extern void TransformVector(const vec3_t v, vec3_t out); // H2

// gl_model.c (Mod_XX function declarations are in gl_model.h)
extern int registration_sequence;
struct model_s* R_RegisterModel(const char* name);

// gl_sky.c
extern void R_AddSkySurface(const msurface_t* fa);
extern void R_ClearSkyBox(void);
extern void R_DrawSkyBox(void);

// gl_sprite.c
extern void R_DrawSpriteModel(entity_t* e); // Q2: local to gl_rmain.c

// gl_surf.c
extern int c_visible_lightmaps;
extern int c_visible_textures;

extern void R_DrawBrushModel(entity_t* e);
extern void R_MarkLeaves(void);
extern void R_SortAndDrawAlphaSurfaces(void); // H2
extern void R_DrawWorld(void);

// gl_warp.c
extern void EmitUnderWaterPolys(const msurface_t* fa); // H2
extern void EmitQuakeFloorPolys(const msurface_t* fa); // H2
extern void GL_SubdivideSurface(msurface_t* fa);
extern void EmitWaterPolys(const msurface_t* fa, qboolean undulate); // H2: extra 'undulate' arg.

#pragma region ========================== GL config stuff  ==========================

#define GL_RENDERER_DEFAULT 0 //mxd. Don't bother with ancient videocards.

// Q2 counterpart
typedef struct
{
	int renderer; //TODO: unused
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
extern void GLimp_BeginFrame(float camera_separation);
extern void GLimp_EndFrame(void);
extern qboolean GLimp_Init(void* hinstance, void* hWnd); //mxd. Changed return type.
extern void GLimp_Shutdown(void);
extern rserr_t GLimp_SetMode(int* pwidth, int* pheight, int mode, qboolean fullscreen, qboolean create_window);
extern void GLimp_AppActivate(qboolean active);
extern void GLimp_EnableLogging(qboolean enable);
extern void GLimp_LogNewFrame(void);

#pragma endregion