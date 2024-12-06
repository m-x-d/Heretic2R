//
// vid_dll.h
//
// Copyright 1998 Raven Software
//

#pragma once

extern cvar_t* vid_gamma;
extern cvar_t* vid_ref;
extern cvar_t* vid_fullscreen;

extern qboolean vid_restart_required; // H2

void VID_PreMenuInit(void); //mxd
void VID_Printf(int print_level, const char* fmt, ...); //mxd
void VID_Error(int err_level, const char* fmt, ...); //mxd