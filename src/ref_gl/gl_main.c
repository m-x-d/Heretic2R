//
// gl_main.c
//
// Copyright 1998 Raven Software
//

#include "gl_local.h"
#include "m_Reference.h"
#include "q_Surface.h"

refimport_t ri;

qboolean R_Init(void* hinstance, void* hWnd)
{
	Sys_Error("Not implemented!");
	return 0;
}

void R_Shutdown(void)
{
	Sys_Error("Not implemented!");
}

void R_BeginFrame(float camera_separation)
{
	Sys_Error("Not implemented!");
}

int R_RenderFrame(refdef_t* fd)
{
	Sys_Error("Not implemented!");
	return 0;
}

// Referenced by GetRefAPI only:
void R_BeginRegistration(char* model);
struct model_s* R_RegisterModel(char* name);
struct image_s* R_RegisterSkin(char* name, qboolean* retval);
struct image_s* Draw_FindPic(char* name);
void R_SetSky(char* name, float rotate, vec3_t axis);
void R_EndRegistration(void);

void Draw_InitCinematic(int w, int h, char* overlay, char* backdrop);
void Draw_CloseCinematic(void);
void Draw_Cinematic(int cols, int rows, byte* data, paletteRGB_t* palette, float alpha);

void Draw_Name(vec3_t origin, char* name, paletteRGBA_t color);
int FindSurface(vec3_t start, vec3_t end, struct Surface_s* surface);

refexport_t GetRefAPI(refimport_t rimp)
{
	refexport_t re;

	ri = rimp;

	re.api_version = API_VERSION;
	re.render = false; //mxd. Shut up compiler...

	re.BeginRegistration = R_BeginRegistration;
	re.RegisterModel = R_RegisterModel;
	re.RegisterSkin = R_RegisterSkin;
	re.RegisterPic = Draw_FindPic;
	re.SetSky = R_SetSky;
	re.EndRegistration = R_EndRegistration;
	re.GetReferencedID = GetReferencedID;

	re.RenderFrame = R_RenderFrame;

	re.DrawGetPicSize = Draw_GetPicSize;
	re.DrawPic = Draw_Pic;
	re.DrawStretchPic = Draw_StretchPic;
	re.DrawChar = Draw_Char;
	re.DrawTileClear = Draw_TileClear;
	re.DrawFill = Draw_Fill;
	re.DrawFadeScreen = Draw_FadeScreen;
	// Missing: Draw_StretchRaw

	re.DrawBigFont = Draw_BigFont;
	re.BF_Strlen = BF_Strlen;
	re.BookDrawPic = Draw_BookPic;
	re.DrawInitCinematic = Draw_InitCinematic;
	re.DrawCloseCinematic = Draw_CloseCinematic;
	re.DrawCinematic = Draw_Cinematic;
	re.Draw_Name = Draw_Name;

	re.Init = R_Init;
	re.Shutdown = R_Shutdown;

	// Missing: R_SetPalette
	re.BeginFrame = R_BeginFrame;
	re.EndFrame = GLimp_EndFrame;
	re.AppActivate = GLimp_AppActivate;
	re.FindSurface = FindSurface;

	// Missing: Swap_Init();
	// Unbound: A3D_RenderGeometry();

	return re;
}