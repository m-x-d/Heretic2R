//
// screen.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "q_shared.h" //mxd
#include "vid.h" //mxd

extern qboolean scr_draw_loading_plaque; //mxd

extern float scr_con_current;

//mxd. Menu/UI/console scaling.
extern int ui_char_size;
extern int ui_scale;
extern int ui_line_height;
extern int ui_screen_width;
extern int ui_screen_offset_x;

extern cvar_t* scr_viewsize;
extern cvar_t* scr_centertime; //mxd
extern cvar_t* crosshair;

extern vrect_t scr_vrect; // Position of render window.

extern void SCR_Init(void);
extern void SCR_UpdateScreen(void);

extern void SCR_RunConsole(void);
extern void SCR_BeginLoadingPlaque(void);
extern void SCR_EndLoadingPlaque(void);
extern void SCR_UpdateProgressbar(int unused, int section); //mxd
extern void SCR_UpdateUIScale(void); //mxd
extern void SCR_DebugGraph(float value, uint color); //mxd. Re-added.

extern void SCR_AddDirtyPoint(int x, int y);
extern void SCR_DirtyScreen(void);

// cl_smk.c
extern void SCR_PlayCinematic(const char *name);
extern void SCR_DrawCinematic(void); //mxd. Removed unused qboolean return type.
extern void SCR_RunCinematic(void);
extern void SCR_FinishCinematic(void);
extern void SCR_StopCinematic(void);
extern void SMK_Shutdown(void); //mxd