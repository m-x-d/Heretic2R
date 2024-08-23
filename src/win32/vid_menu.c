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

// New in H2:
#define NUM_DRIVERNAMES			16
#define MAX_DRIVERNAME_LENGTH	20

static cvar_t* gl_driver;

static int gl_drivers_count;
static char gl_drivernames[NUM_DRIVERNAMES - 2][MAX_DRIVERNAME_LENGTH];
static char* gl_drivername_labels[NUM_DRIVERNAMES];

static void VID_MenuSetDetail(int detail)
{
	NOT_IMPLEMENTED
}

static void VID_Menu_GetDriverNames(void)
{
	char buffer[MAX_QPATH];

	gl_drivername_labels[0] = "Software";
	gl_drivername_labels[1] = "OpenGL32";
	gl_drivers_count = 2;

	for (int i = 0; i < NUM_DRIVERNAMES - gl_drivers_count; i++)
	{
		Com_sprintf(buffer, sizeof(buffer), "gl_driver%d", i);
		const cvar_t* cvar = Cvar_Get(buffer, "", CVAR_ARCHIVE);
		const uint len = strlen(cvar->string);

		if (len == 0)
			break;

		if (len > MAX_DRIVERNAME_LENGTH - 1)
		{
			Com_Printf("*** ERROR : Invalid driver name %s\n", cvar->string);
			strcpy_s(cvar->string, len, "Invalid Driver");
		}

		strncpy_s(gl_drivernames[i], sizeof(gl_drivernames[i]), cvar->string, MAX_DRIVERNAME_LENGTH - 1);

		gl_drivername_labels[gl_drivers_count] = gl_drivernames[i];
		gl_drivers_count++;
	}

	gl_drivername_labels[gl_drivers_count] = NULL;
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
		//TODO: this initializes 'Software' menu, so above check should be inverted?
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