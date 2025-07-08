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
#include "gl1_SDL.h"
#include "gl1_Sky.h"
#include "turbsin.h"
#include "gl1_Local.h"
#include "vid.h"

#define REF_DECLSPEC	__declspec(dllexport)

viddef_t viddef; // H2: renamed from vid, defined in vid.h?
refimport_t ri;

model_t* r_worldmodel;
model_t* currentmodel;

float gldepthmin;
float gldepthmax;

glconfig_t gl_config;
glstate_t gl_state;

refdef_t r_newrefdef; // Screen size info.

int r_framecount; // Used for dlight push checking.

int r_viewcluster;
int r_viewcluster2;
int r_oldviewcluster;
int r_oldviewcluster2;

#pragma region ========================== CVARS  ==========================

cvar_t* r_norefresh;
cvar_t* r_fullbright;
cvar_t* r_drawentities;
cvar_t* r_drawworld;
cvar_t* r_novis;
cvar_t* r_nocull;
cvar_t* r_lerpmodels;
static cvar_t* r_speeds;
cvar_t* r_vsync; // YQ2

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

static void R_Fog(void) // H2: GL_Fog
{
	NOT_IMPLEMENTED
}

static void R_WaterFog(void) // H2: GL_WaterFog
{
	NOT_IMPLEMENTED
}

static void R_Clear(void)
{
	if ((int)gl_ztrick->value) //TODO: mxd. No fog rendering when gl_ztrick is enabled. Curious...
	{
		static int trickframe;

		if ((int)gl_clear->value)
			glClear(GL_COLOR_BUFFER_BIT);

		trickframe++;

		if (trickframe & 1)
		{
			gldepthmin = 0.0f;
			gldepthmax = 0.49999f;
			glDepthFunc(GL_LEQUAL);
		}
		else
		{
			gldepthmin = 1.0f;
			gldepthmax = 0.5f;
			glDepthFunc(GL_GEQUAL);
		}
	}
	else
	{
		// H2: extra fog rendering logic. //mxd. Removed gl_fog_broken cvar checks.
		if ((int)cl_camera_under_surface->value) //TODO: r_fog_underwater cvar check seems logical here, but isn't present in original dll.
		{
			R_WaterFog();
		}
		//mxd. Removed 'r_fog_startdist->value < r_farclipdist->value' check, because it's relevant only for fog mode 0.
		// Also there's no r_fog_underwater_startdist check in GL_WaterFog case in original .dll.
		else if ((int)r_fog->value)
		{
			R_Fog();
		}
		else
		{
			glDisable(GL_FOG);

			if ((int)gl_clear->value)
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			else
				glClear(GL_DEPTH_BUFFER_BIT);
		}

		gldepthmin = 0.0f;
		gldepthmax = 1.0f;
		glDepthFunc(GL_LEQUAL);
	}

	glDepthRange((double)gldepthmin, (double)gldepthmax);
}

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
	r_vsync = ri.Cvar_Get("r_vsync", "1", CVAR_ARCHIVE); // YQ2

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

	ri.Cmd_AddCommand("imagelist", R_ImageList_f);
	ri.Cmd_AddCommand("screenshot", R_ScreenShot_f);
	ri.Cmd_AddCommand("modellist", Mod_Modellist_f);
	ri.Cmd_AddCommand("gl_strings", R_Strings_f);

	R_InitGammaTable(); // H2
}

// Changes the video mode.
static rserr_t SetMode_impl(int* pwidth, int* pheight, const int mode) // YQ2
{
	ri.Con_Printf(PRINT_ALL, "Setting mode %d:", mode);

	if (!ri.Vid_GetModeInfo(pwidth, pheight, mode))
	{
		ri.Con_Printf(PRINT_ALL, " invalid mode\n");
		return RSERR_INVALID_MODE;
	}

	ri.Con_Printf(PRINT_ALL, " %dx%d\n", *pwidth, *pheight);

	return (ri.GLimp_InitGraphics(*pwidth, *pheight) ? RSERR_OK : RSERR_INVALID_MODE);
}

static qboolean R_SetMode(void)
{
	rserr_t err = SetMode_impl(&viddef.width, &viddef.height, (int)vid_mode->value);

	if (err == RSERR_OK)
	{
		gl_state.prev_mode = (int)vid_mode->value;

		ri.Cvar_SetValue("vid_fullscreen", (int)vid_mode->value == 0 ? 1.0f : 0.0f); //mxd. Fullscreen when Mode 0, windowed otherwise.
		vid_fullscreen->modified = false;

		return true;
	}

	if (err == RSERR_INVALID_MODE)
	{
		ri.Con_Printf(PRINT_ALL, "ref_gl::R_SetMode() - invalid mode\n");

		// Trying again would result in a crash anyway, give up already (this would happen if your initing fails at all and your resolution already was 640x480).
		if ((int)vid_mode->value == gl_state.prev_mode)
			return false;

		ri.Cvar_SetValue("vid_mode", (float)gl_state.prev_mode);
		vid_mode->modified = false;
	}
	else
	{
		ri.Con_Printf(PRINT_ALL, "ref_gl::R_SetMode() - unknown error %i!\n", err);
		return false;
	}

	// Try setting it back to something safe.
	err = SetMode_impl(&viddef.width, &viddef.height, (int)vid_mode->value);

	if (err == RSERR_OK)
	{
		ri.Cvar_SetValue("vid_fullscreen", (int)vid_mode->value == 0 ? 1.0f : 0.0f); //mxd. Fullscreen when Mode 0, windowed otherwise.
		vid_fullscreen->modified = false;

		return true;
	}

	ri.Con_Printf(PRINT_ALL, "ref_gl::R_SetMode() - could not revert to safe mode\n");
	return false;
}

static qboolean R_Init(void)
{
	for (int j = 0; j < 256; j++)
		turbsin[j] *= 0.5f;

	ri.Con_Printf(PRINT_ALL, "Refresh: "REF_VERSION"\n"); //mxd. Com_Printf() -> ri.Con_Printf() (here and below).
	R_Register();

	// Set our "safe" mode.
	gl_state.prev_mode = 1; // H2: 3.

	// Create the window and set up the context.
	if (!R_SetMode())
	{
		ri.Con_Printf(PRINT_ALL, "ref_gl::R_Init() - could not R_SetMode()\n");
		return false; //mxd. Decompiled code still returns -1 here...
	}

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

	//mxd. Check max. supported texture size. H2 expects at least 128x128. H2R expects at least 512x512 (for cinematics rendering without frame chopping shenanigans). Probably not needed: even GF2 supports 2048x2048 textures.
	int max_texture_size;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_texture_size);
	if (max_texture_size < 512)
	{
		ri.Con_Printf(PRINT_ALL, "ref_gl::R_Init() - maximum supported texture size too low! Expected at least 512, got %i\n", max_texture_size);
		return false;
	}

	R_SetDefaultState();
	R_InitImages();
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
	ShutdownFonts(); // H2

	ri.Cmd_RemoveCommand("modellist");
	ri.Cmd_RemoveCommand("screenshot");
	ri.Cmd_RemoveCommand("imagelist");
	ri.Cmd_RemoveCommand("gl_strings");

	Mod_FreeAll();
	R_ShutdownImages();

	// Shutdown OS-specific OpenGL stuff like contexts, etc.
	R_ShutdownContext(); // YQ2
}

static void R_BeginFrame(const float camera_separation) //TODO: remove camera_separation arg?
{
	// Changed.
	if (vid_gamma->modified || vid_brightness->modified || vid_contrast->modified)
	{
		R_InitGammaTable();
		R_GammaAffect();

		vid_gamma->modified = false;
		vid_brightness->modified = false;
		vid_contrast->modified = false;
	}

	// Go into 2D mode.
	glViewport(0, 0, viddef.width, viddef.height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, viddef.width, viddef.height, 0.0, -99999.0, 99999.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glEnable(GL_ALPHA_TEST);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	// Draw buffer stuff.
	if (gl_drawbuffer->modified)
	{
		glDrawBuffer((Q_stricmp(gl_drawbuffer->string, "GL_FRONT") == 0) ? GL_FRONT : GL_BACK);
		gl_drawbuffer->modified = false;
	}

	// Texturemode stuff.
	if (gl_texturemode->modified)
	{
		R_TextureMode(gl_texturemode->string);
		gl_texturemode->modified = false;
	}

	// Missing: gl_texturealphamode and gl_texturesolidmode logic

	// Swapinterval stuff.
	if (r_vsync->modified) // YQ2
	{
		R_SetVsync();
		r_vsync->modified = false;
	}

	// Clear screen if desired.
	R_Clear();
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

	re.PrepareForWindow = R_PrepareForWindow; // YQ2
	re.InitContext = R_InitContext; // YQ2
	re.ShutdownContext = R_ShutdownContext; // YQ2

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