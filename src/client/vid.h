//
// vid.h -- video driver defs
//
// Copyright 1998 Raven Software
//

#pragma once

#include "q_Typedef.h" //mxd

#define	MIN_GAMMA	0.1			// These also need to be defined in gl_local.h
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

void VID_MenuInit(void);
void VID_PreMenuInit(void);
void VID_MenuDraw(void);
char* VID_MenuKey(int);