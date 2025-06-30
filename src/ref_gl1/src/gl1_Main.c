//
// gl1_Main.c
//
// Copyright 1998 Raven Software
//

#include "gl1_Debug.h"
#include "gl1_Draw.h"
#include "gl1_DrawBook.h"
#include "gl1_DrawCinematic.h"
#include "gl1_FindSurface.h"
#include "gl1_Image.h"
#include "gl1_Misc.h"
#include "gl1_Model.h"
#include "gl1_SDL.h"
#include "gl1_Sky.h"
#include "turbsin.h"
#include "gl1_Local.h"

#define REF_DECLSPEC	__declspec(dllexport)

glconfig_t gl_config;
glstate_t gl_state;

refimport_t ri;

#pragma region ========================== CVARS  ==========================

cvar_t* r_norefresh;
cvar_t* r_fullbright;
cvar_t* r_drawentities;
cvar_t* r_drawworld;
cvar_t* r_novis;
cvar_t* r_nocull;
cvar_t* r_lerpmodels;
static cvar_t* r_speeds;

cvar_t* r_lightlevel; // FIXME: This is a HACK to get the client's light level

cvar_t* r_farclipdist;
cvar_t* r_fog;
cvar_t* r_fog_mode;
cvar_t* r_fog_density;
cvar_t* r_fog_startdist;
static cvar_t* r_fog_color_r;
static cvar_t* r_fog_color_g;
static cvar_t* r_fog_color_b;
static cvar_t* r_fog_color_a;
cvar_t* r_fog_lightmap_adjust;
cvar_t* r_fog_underwater;
static cvar_t* r_fog_underwater_mode;
static cvar_t* r_fog_underwater_density;
static cvar_t* r_fog_underwater_startdist;
static cvar_t* r_fog_underwater_color_r;
static cvar_t* r_fog_underwater_color_g;
static cvar_t* r_fog_underwater_color_b;
static cvar_t* r_fog_underwater_color_a;
static cvar_t* r_underwater_color;
cvar_t* r_frameswap;
cvar_t* r_references;

cvar_t* gl_noartifacts;

cvar_t* gl_modulate;
static cvar_t* gl_log;
cvar_t* gl_lightmap;
cvar_t* gl_dynamic;
cvar_t* gl_nobind;
cvar_t* gl_showtris;
static cvar_t* gl_reporthash;
static cvar_t* gl_ztrick;
static cvar_t* gl_finish;
static cvar_t* gl_clear;
static cvar_t* gl_cull;
static cvar_t* gl_polyblend;
cvar_t* gl_flashblend;
cvar_t* gl_texturemode;
cvar_t* gl_lockpvs;

cvar_t* gl_drawflat;
cvar_t* gl_trans33;
cvar_t* gl_trans66;
cvar_t* gl_picmip;
cvar_t* gl_skinmip;
cvar_t* gl_bookalpha;

cvar_t* gl_drawmode;
cvar_t* gl_drawbuffer;
cvar_t* gl_swapinterval;
cvar_t* gl_sortmulti;
cvar_t* gl_saturatelighting;

cvar_t* vid_fullscreen;
cvar_t* vid_gamma;
cvar_t* vid_brightness;
cvar_t* vid_contrast;

cvar_t* vid_ref;

cvar_t* vid_mode; // gl_mode in Q2
cvar_t* menus_active;
cvar_t* cl_camera_under_surface;
cvar_t* quake_amount;

#pragma endregion

static void R_Register(void)
{
	r_norefresh = ri.Cvar_Get("r_norefresh", "0", 0);
	r_fullbright = ri.Cvar_Get("r_fullbright", "0", 0);
	r_drawentities = ri.Cvar_Get("r_drawentities", "1", 0);
	r_drawworld = ri.Cvar_Get("r_drawworld", "1", 0);
	r_novis = ri.Cvar_Get("r_novis", "0", 0);
	r_nocull = ri.Cvar_Get("r_nocull", "0", 0);
	r_lerpmodels = ri.Cvar_Get("r_lerpmodels", "1", 0);
	r_speeds = ri.Cvar_Get("r_speeds", "0", 0);

	r_lightlevel = ri.Cvar_Get("r_lightlevel", "0", 0);

	// H2:
	r_farclipdist = ri.Cvar_Get("r_farclipdist", "4096.0", 0);
	r_fog = ri.Cvar_Get("r_fog", "0", 0);
	r_fog_mode = ri.Cvar_Get("r_fog_mode", "1", 0);
	r_fog_density = ri.Cvar_Get("r_fog_density", "0.004", 0);
	r_fog_startdist = ri.Cvar_Get("r_fog_startdist", "50.0", 0);
	r_fog_color_r = ri.Cvar_Get("r_fog_color_r", "1.0", 0);
	r_fog_color_g = ri.Cvar_Get("r_fog_color_g", "1.0", 0);
	r_fog_color_b = ri.Cvar_Get("r_fog_color_b", "1.0", 0);
	r_fog_color_a = ri.Cvar_Get("r_fog_color_a", "0.0", 0);
	r_fog_lightmap_adjust = ri.Cvar_Get("r_fog_lightmap_adjust", "5.0", 0);
	r_fog_underwater_mode = ri.Cvar_Get("r_fog_underwater_mode", "1", 0);
	r_fog_underwater_density = ri.Cvar_Get("r_fog_underwater_density", "0.0015", 0);
	r_fog_underwater_startdist = ri.Cvar_Get("r_fog_underwater_startdist", "100.0", 0);
	r_fog_underwater_color_r = ri.Cvar_Get("r_fog_underwater_color_r", "1.0", 0);
	r_fog_underwater_color_g = ri.Cvar_Get("r_fog_underwater_color_g", "1.0", 0);
	r_fog_underwater_color_b = ri.Cvar_Get("r_fog_underwater_color_b", "1.0", 0);
	r_fog_underwater_color_a = ri.Cvar_Get("r_fog_underwater_color_a", "0.0", 0);
	r_underwater_color = ri.Cvar_Get("r_underwater_color", "0x70c06000", 0);
	r_frameswap = ri.Cvar_Get("r_frameswap", "1.0", 0);
	r_references = ri.Cvar_Get("r_references", "1.0", 0);

	gl_noartifacts = ri.Cvar_Get("gl_noartifacts", "0", 0); // H2

	gl_modulate = ri.Cvar_Get("gl_modulate", "1", CVAR_ARCHIVE);
	gl_log = ri.Cvar_Get("gl_log", "0", 0);
	gl_lightmap = ri.Cvar_Get("gl_lightmap", "0", 0);
	gl_dynamic = ri.Cvar_Get("gl_dynamic", "1", 0);
	gl_nobind = ri.Cvar_Get("gl_nobind", "0", 0);
	gl_showtris = ri.Cvar_Get("gl_showtris", "0", 0);
	gl_reporthash = ri.Cvar_Get("gl_reporthash", "0", 0);
	gl_ztrick = ri.Cvar_Get("gl_ztrick", "0", 0);
	gl_finish = ri.Cvar_Get("gl_finish", "0", 0);
	gl_clear = ri.Cvar_Get("gl_clear", "0", 0);
	gl_cull = ri.Cvar_Get("gl_cull", "1", 0);
	gl_polyblend = ri.Cvar_Get("gl_polyblend", "1", 0);
	gl_flashblend = ri.Cvar_Get("gl_flashblend", "0", 0);
	gl_texturemode = ri.Cvar_Get("gl_texturemode", "GL_LINEAR_MIPMAP_NEAREST", CVAR_ARCHIVE);
	gl_lockpvs = ri.Cvar_Get("gl_lockpvs", "0", 0);

	// H2:
	gl_drawflat = ri.Cvar_Get("gl_drawflat", "0", 0);
	gl_trans33 = ri.Cvar_Get("gl_trans33", "0.33", 0); // H2_1.07: 0.33 -> 1
	gl_trans66 = ri.Cvar_Get("gl_trans66", "0.66", 0); // H2_1.07: 0.66 -> 1
	gl_picmip = ri.Cvar_Get("gl_picmip", "0", CVAR_ARCHIVE);
	gl_skinmip = ri.Cvar_Get("gl_skinmip", "0", CVAR_ARCHIVE);
	gl_bookalpha = ri.Cvar_Get("gl_bookalpha", "1.0", 0);

	gl_drawmode = ri.Cvar_Get("gl_drawmode", "0", 0);
	gl_drawbuffer = ri.Cvar_Get("gl_drawbuffer", "GL_BACK", 0);
	gl_swapinterval = ri.Cvar_Get("gl_swapinterval", "1", CVAR_ARCHIVE);
	gl_sortmulti = ri.Cvar_Get("gl_sortmulti", "0", CVAR_ARCHIVE); // H2
	gl_saturatelighting = ri.Cvar_Get("gl_saturatelighting", "0", 0);

	vid_fullscreen = ri.Cvar_Get("vid_fullscreen", "0", CVAR_ARCHIVE);
	vid_gamma = ri.Cvar_Get("vid_gamma", "0.5", CVAR_ARCHIVE);
	vid_brightness = ri.Cvar_Get("vid_brightness", "0.5", CVAR_ARCHIVE); // H2
	vid_contrast = ri.Cvar_Get("vid_contrast", "0.5", CVAR_ARCHIVE); // H2

	vid_ref = ri.Cvar_Get("vid_ref", "gl", CVAR_ARCHIVE);

	// H2:
	vid_mode = ri.Cvar_Get("vid_mode", "3", CVAR_ARCHIVE);
	menus_active = ri.Cvar_Get("menus_active", "0", 0);
	cl_camera_under_surface = ri.Cvar_Get("cl_camera_under_surface", "0", 0);
	quake_amount = ri.Cvar_Get("quake_amount", "0", 0);

	ri.Cmd_AddCommand("imagelist", GL_ImageList_f);
	ri.Cmd_AddCommand("screenshot", GL_ScreenShot_f);
	ri.Cmd_AddCommand("modellist", Mod_Modellist_f);
	ri.Cmd_AddCommand("gl_strings", GL_Strings_f);

	InitGammaTable(); // H2
}

static qboolean R_SetMode(void)
{
	NOT_IMPLEMENTED
	return false;
}

static qboolean R_Init(void)
{
	for (int j = 0; j < 256; j++)
		turbsin[j] *= 0.5f;

	ri.Con_Printf(PRINT_ALL, "Refresh: "REF_VERSION"\n"); //mxd. Com_Printf() -> ri.Con_Printf() (here and below).
	R_Register();

	//mxd. Initialize OpenGL dynamic bindings. Must be called after GLimp_Init().
	if (!gladLoadGLLoader(R_GetProcAddress))
	{
		ri.Con_Printf(PRINT_ALL, "ref_gl::R_Init() - OpenGL initialization failed!\n");
		return false;
	}

	//mxd. Check OpenGL version.
	if (!GLAD_GL_VERSION_1_3)
	{
		ri.Con_Printf(PRINT_ALL, "ref_gl::R_Init() - Unsupported OpenGL version. Expected 1.3, got %i.%i!\n", GLVersion.major, GLVersion.minor);
		return false;
	}

	// Set our "safe" mode.
	gl_state.prev_mode = 4; // H2: 3.

	// Create the window and set up the context.
	if (!R_SetMode())
	{
		ri.Con_Printf(PRINT_ALL, "ref_gl::R_Init() - could not R_SetMode()\n");
		return false; //mxd. Decompiled code still returns -1 here...
	}

	ri.Vid_MenuInit(); // YQ2

	// Get our various GL strings.
	gl_config.vendor_string = (const char*)glGetString(GL_VENDOR);
	ri.Con_Printf(PRINT_ALL, "GL_VENDOR: %s\n", gl_config.vendor_string);

	gl_config.renderer_string = (const char*)glGetString(GL_RENDERER);
	ri.Con_Printf(PRINT_ALL, "GL_RENDERER: %s\n", gl_config.renderer_string);

	gl_config.version_string = (const char*)glGetString(GL_VERSION);
	ri.Con_Printf(PRINT_ALL, "GL_VERSION: %s\n", gl_config.version_string);

	gl_config.extensions_string = (const char*)glGetString(GL_EXTENSIONS);
	//Com_Printf("GL_EXTENSIONS: %s\n", gl_config.extensions_string); // H2_1.07: "GL_EXT: hidden\n" //mxd. Modern extensions_string is longer than Com_Printf can handle... 

	// YQ2: Anisotropic texture filtering.
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &gl_config.max_anisotropy);
	ri.Con_Printf(PRINT_ALL, "Max. anisotropy: %i.\n", (int)gl_config.max_anisotropy);

	R_SetDefaultState();
	GL_InitImages();
	Mod_Init();
	Draw_InitLocal();

	const GLenum err = glGetError(); //TODO: missing in YQ2 version. Not needed?
	if (err != GL_NO_ERROR)
	{
		ri.Con_Printf(PRINT_ALL, "glGetError() = 0x%x\n", err);
		return false;
	}

	return true; //mxd. Return value missing in Q2
}

static void R_Shutdown(void)
{
	NOT_IMPLEMENTED
}

static void R_BeginFrame(const float camera_separation)
{
	NOT_IMPLEMENTED
}

static int R_RenderFrame(const refdef_t* fd)
{
	NOT_IMPLEMENTED
	return 0;
}

static int R_GetReferencedID(const struct model_s* model)
{
	NOT_IMPLEMENTED
	return 0;
}

REF_DECLSPEC refexport_t GetRefAPI(const refimport_t rimp)
{
	refexport_t re;

	ri = rimp;

	re.api_version = REF_API_VERSION;
	re.render = false; //mxd. Avoid compiler warning.

	re.BeginRegistration = R_BeginRegistration;
	re.RegisterModel = R_RegisterModel;
	re.RegisterSkin = R_RegisterSkin;
	re.RegisterPic = Draw_FindPic;
	re.SetSky = R_SetSky;
	re.EndRegistration = R_EndRegistration;
	re.GetReferencedID = R_GetReferencedID;

	re.RenderFrame = R_RenderFrame;

	re.DrawGetPicSize = Draw_GetPicSize;
	re.DrawPic = Draw_Pic;
	re.DrawStretchPic = Draw_StretchPic;
	re.DrawChar = Draw_Char;
	re.DrawTileClear = Draw_TileClear;
	re.DrawFill = Draw_Fill;
	re.DrawFadeScreen = Draw_FadeScreen;

	re.DrawBigFont = Draw_BigFont;
	re.BF_Strlen = BF_Strlen;
	re.BookDrawPic = Draw_BookPic;
	re.DrawInitCinematic = Draw_InitCinematic;
	re.DrawCloseCinematic = Draw_CloseCinematic;
	re.DrawCinematic = Draw_Cinematic;
	re.Draw_Name = Draw_Name;

	re.Init = R_Init;
	re.Shutdown = R_Shutdown;

	re.BeginFrame = R_BeginFrame;
	re.EndFrame = R_EndFrame;
	re.FindSurface = FindSurface;

#ifdef _DEBUG
	//mxd. Debug draw logic.
	re.AddDebugBox = R_AddDebugBox;
	re.AddDebugBbox = R_AddDebugBbox;
	re.AddDebugEntityBbox = R_AddDebugEntityBbox;

	re.AddDebugLine = R_AddDebugLine;
	re.AddDebugArrow = R_AddDebugArrow;
#endif

	// Unbound: A3D_RenderGeometry();
	//TODO: ri.Vid_RequestRestart(RESTART_NO) YQ2 logic.

	return re;
}