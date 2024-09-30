//
// vid.h -- video driver defs
//
// Copyright 1998 Raven Software
//

#pragma once

#define MIN_GAMMA	0.1			// These also need to be defined in gl_local.h
#define MAX_GAMMA	4.0

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
	byte* pixels;
} viddef_t;

extern viddef_t viddef;	// Global video state

// Video module initialisation etc
void VID_Init(void);
void VID_Shutdown(void);
void VID_CheckChanges(void);

//void VID_MenuInit(void); //mxd. Moved to menu_video.h
//void VID_PreMenuInit(void); //mxd. Moved to vid_dll.h
//void VID_MenuDraw(void); //mxd. No longer needed.
//const char* VID_MenuKey(int); //mxd. No longer needed.