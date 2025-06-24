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
#include "gl1_Model.h"
#include "gl1_SDL.h"
#include "gl1_Sky.h"
#include "gl1_Local.h"

#define REF_DECLSPEC	__declspec(dllexport)

refimport_t ri;

static qboolean R_Init(void)
{
	NOT_IMPLEMENTED
	return false;
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