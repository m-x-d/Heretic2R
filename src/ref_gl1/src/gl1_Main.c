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

static void R_Register(void)
{
	NOT_IMPLEMENTED
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