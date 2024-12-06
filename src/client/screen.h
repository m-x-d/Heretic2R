//
// screen.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "q_shared.h" //mxd
#include "vid.h" //mxd

void SCR_Init(void);
void SCR_UpdateScreen(void);

void SCR_BeginLoadingPlaque(void);
void SCR_EndLoadingPlaque(void);
void SCR_UpdateProgressbar(int unused, int section); //mxd
void SCR_DebugGraph(float value, uint color); //mxd. Re-added

extern qboolean scr_draw_loading_plaque; //mxd

extern float scr_con_current;
extern float scr_conlines;	// Lines of console to display

extern int sb_lines;

extern cvar_t* scr_viewsize;
extern cvar_t* scr_centertime; //mxd
extern cvar_t* crosshair;

extern vrect_t scr_vrect;	// Position of render window

void SCR_AddDirtyPoint(int x, int y);
void SCR_DirtyScreen(void);

// cl_smk.c
void SCR_PlayCinematic(const char *name);
void SCR_DrawCinematic(void); //mxd. Removed unused qboolean return type.
void SCR_RunCinematic(void);
void SCR_FinishCinematic(void);
void SCR_StopCinematic(void);
void SMK_Shutdown(void); //mxd