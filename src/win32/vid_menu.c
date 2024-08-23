//
// vid_menu.c
//
// Copyright 1998 Raven Software
//

#include "client.h"
#include "menu.h"
#include "vid_dll.h"

static int s_current_menu_index;

static menulist_s s_ref_list[2];

static cvar_t* gl_driver; // New in H2
static int gl_drivers_count;
static char gl_drivernames[14][20];

static void VID_MenuSetDetail(int detail)
{
	NOT_IMPLEMENTED
}

static void VID_Menu_GetDriverNames(void)
{
	NOT_IMPLEMENTED
}

void VID_PreMenuInit(void)
{
	VID_Menu_GetDriverNames();

	if (gl_driver == NULL)
	{
		gl_driver = Cvar_Get("gl_driver", "opengl32", 0);
		vid_restart_required = true;
	}

	if (vid_mode == NULL)
	{
		if (Q_stricmp(vid_ref->string, "gl") == 0)
			vid_mode = Cvar_Get("vid_mode", "0", 0);
		else
			vid_mode = Cvar_Get("vid_mode", "3", 0);

		vid_restart_required = true;
	}

	if (vid_menu_mode == NULL)
		vid_menu_mode = Cvar_Get("vid_menu_mode", "3", 0);

	//mxd. Skip gl_ext_palettedtexture logic

	if (scr_viewsize == NULL)
		scr_viewsize = Cvar_Get("viewsize", "100", CVAR_ARCHIVE);

	if (Q_stricmp(vid_ref->string, "gl") == 0)
	{
		//TODO: this initializes 'Software' menu(?), so above check should be inverted?
		s_current_menu_index = 0;
		s_ref_list[0].curvalue = 0;
		s_ref_list[1].curvalue = 0;
	}
	else
	{
		//TODO: original code does 'Q_stricmp(vid_ref->string, "gl") == 0' check here again, so below code is never called. 
		s_current_menu_index = 1;
		s_ref_list[1].curvalue = 1;

		for (int i = 2; i < gl_drivers_count; i++)
		{
			if (Q_stricmp(gl_driver->string, gl_drivernames[i]) == 0)
			{
				s_ref_list[s_current_menu_index].curvalue = i;
				break;
			}
		}
	}

	if ((int)vid_fullscreen->value)
	{
		Cvar_SetValue("vid_xpos", 0.0f);
		Cvar_SetValue("vid_ypos", 0.0f);
	}

	const float detail = Cvar_VariableValue("r_detail");
	VID_MenuSetDetail((int)detail);
}