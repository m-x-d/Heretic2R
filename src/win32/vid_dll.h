//
// vid_dll.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "q_shared.h"

#define MAX_REFLIBS	16 //mxd

typedef struct //mxd
{
	char title[32];	// Reflib title ("Software", "OpenGL 1.3" etc.).
	char id[8];		// Value to store in vid_ref cvar ("soft", "gl1" etc.).
} reflib_info_t;

extern reflib_info_t reflib_infos[MAX_REFLIBS]; //mxd
extern int num_reflib_infos; //mxd

typedef struct vidmode_s //mxd
{
	char description[32]; // Q2: char*
	int width;
	int height;
	int mode;
} vidmode_t;

extern vidmode_t* vid_modes; //mxd. Static array in Q2 / H2. H2 has no mode 10.
extern int num_vid_modes; //mxd

extern cvar_t* vid_gamma;
extern cvar_t* vid_brightness; // H2
extern cvar_t* vid_contrast; // H2
extern cvar_t* vid_ref;
extern cvar_t* vid_fullscreen;

extern qboolean vid_restart_required; // H2

extern void VID_PreMenuInit(void); //mxd
extern void VID_Printf(int print_level, const char* fmt, ...); //mxd
H2R_NORETURN extern void VID_Error(int err_level, const char* fmt, ...); //mxd