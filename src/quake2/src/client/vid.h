//
// vid.h -- Video driver definitions.
//
// Copyright 1998 Raven Software
//

#pragma once

#define DEF_WIDTH	640
#define DEF_HEIGHT	480

typedef struct vrect_s
{
	int x;
	int y;
	int width;
	int height;
} vrect_t;

typedef struct
{
	int width;
	int height;
} viddef_t;

extern viddef_t viddef; // Global video state.

// Video module initialization.
extern void VID_Init(void);
extern void VID_Shutdown(void);
extern void VID_CheckChanges(void);
extern void VID_InitModes(viddef_t* modes, int num_modes); //mxd