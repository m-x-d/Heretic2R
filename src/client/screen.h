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

void SCR_SizeUp(void);
void SCR_SizeDown(void);
void SCR_CenterPrint(char* str, PalIdx_t colour);
void SCR_CaptionPrint(char* str);
void SCR_ObituaryPrint(char* str, byte client1, byte client2, uint colour);
void SCR_BeginLoadingPlaque(void);
void SCR_EndLoadingPlaque(void);

void SCR_DebugGraph(float value, int color); //mxd. Re-added

void SCR_TouchPics(void);
void SCR_RunConsole(void);

extern qboolean scr_draw_loading_plaque; //mxd

extern float scr_con_current;
extern float scr_conlines;	// Lines of console to display

extern int sb_lines;

extern cvar_t* scr_viewsize;
extern cvar_t* crosshair;

extern vrect_t scr_vrect;	// Position of render window

void SCR_AddDirtyPoint(int x, int y);
void SCR_DirtyScreen(void);

// scr_cin.c
void SCR_PlayCinematic(const char *name);
qboolean SCR_DrawCinematic(void);
void SCR_RunCinematic(void);
void SCR_FinishCinematic(void);
void SCR_StopCinematic(void);