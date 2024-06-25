//
// gl_main.c
//
// Copyright 1998 Raven Software
//

#include "gl_local.h"
#include "m_Reference.h"
#include "q_Surface.h"
#include "vid.h"

viddef_t viddef; // H2: renamed from vid, defined in vid.h?
refimport_t ri;

float gldepthmin;
float gldepthmax;

glconfig_t gl_config;
glstate_t gl_state;

image_t* r_notexture; // Used for missing textures
image_t* r_particletexture;
image_t* r_aparticletexture;
image_t* r_reflecttexture;
image_t* r_font1;
image_t* r_font2;

cvar_t* r_norefresh;
cvar_t* r_fullbright;
cvar_t* r_drawentities;
cvar_t* r_drawworld;
cvar_t* r_novis;
cvar_t* r_nocull;
cvar_t* r_lerpmodels;
cvar_t* r_speeds;

cvar_t* r_lightlevel; // FIXME: This is a HACK to get the client's light level

cvar_t* r_farclipdist;
cvar_t* r_fog;
cvar_t* r_fog_mode;
cvar_t* r_fog_density;
cvar_t* r_fog_startdist;
cvar_t* r_fog_color_r;
cvar_t* r_fog_color_g;
cvar_t* r_fog_color_b;
cvar_t* r_fog_color_a;
cvar_t* r_fog_color_scale;
cvar_t* r_fog_lightmap_adjust;
cvar_t* r_fog_underwater;
cvar_t* r_fog_underwater_mode;
cvar_t* r_fog_underwater_density;
cvar_t* r_fog_underwater_startdist;
cvar_t* r_fog_underwater_color_r;
cvar_t* r_fog_underwater_color_g;
cvar_t* r_fog_underwater_color_b;
cvar_t* r_fog_underwater_color_a;
cvar_t* r_fog_underwater_color_scale;
cvar_t* r_fog_underwater_lightmap_adjust;
cvar_t* r_underwater_color;
cvar_t* r_frameswap;
cvar_t* r_references;

cvar_t* gl_nosubimage;
cvar_t* gl_allow_software; //TODO: ignored. Remove?

cvar_t* gl_particle_min_size;
cvar_t* gl_particle_max_size;
cvar_t* gl_particle_size;
cvar_t* gl_particle_att_a;
cvar_t* gl_particle_att_b;
cvar_t* gl_particle_att_c;
cvar_t* gl_noartifacts;

cvar_t* gl_modulate;
cvar_t* gl_log;
cvar_t* gl_bitdepth; //TODO: ignored (Win7+ can't into 8 and 16-bit color modes). Remove?
cvar_t* gl_lightmap;
cvar_t* gl_shadows;
cvar_t* gl_dynamic;
cvar_t* gl_nobind;
cvar_t* gl_round_down;
cvar_t* gl_showtris;
cvar_t* gl_reporthash;
cvar_t* gl_ztrick;
cvar_t* gl_finish;
cvar_t* gl_clear;
cvar_t* gl_cull;
cvar_t* gl_polyblend;
cvar_t* gl_flashblend;
cvar_t* gl_playermip;
cvar_t* gl_monolightmap;
cvar_t* gl_driver;
cvar_t* gl_texturemode;
cvar_t* gl_lockpvs;

cvar_t* gl_drawflat;
cvar_t* gl_devel1;
cvar_t* gl_trans33;
cvar_t* gl_trans66;
cvar_t* gl_picmip;
cvar_t* gl_skinmip;
cvar_t* gl_bookalpha;

cvar_t* gl_ext_swapinterval;
cvar_t* gl_ext_gamma;
cvar_t* gl_ext_palettedtexture;
cvar_t* gl_ext_multitexture;
cvar_t* gl_ext_pointparameters;
cvar_t* gl_drawmode;

cvar_t* gl_drawbuffer;
cvar_t* gl_swapinterval;
cvar_t* gl_sortmulti;

cvar_t* gl_saturatelighting;

cvar_t* gl_3dlabs_broken;
cvar_t* gl_lostfocus_broken;
cvar_t* gl_fog_broken;
cvar_t* gl_envmap_broken;
cvar_t* gl_screenshot_broken;

cvar_t* vid_fullscreen;
cvar_t* vid_gamma;
cvar_t* vid_brightness;
cvar_t* vid_contrast;

cvar_t* vid_ref;

cvar_t* vid_mode; // gl_mode in Q2
cvar_t* menus_active;
cvar_t* cl_camera_under_surface;
cvar_t* quake_amount;

// New in H2
static void GL_Fog(void)
{
	NOT_IMPLEMENTED
}

// New in H2
static void GL_WaterFog(void)
{
	NOT_IMPLEMENTED
}

static void R_Clear(void)
{
	if ((int)gl_ztrick->value) //TODO: mxd. No fog rendering when gl_ztrick is enabled. Curious...
	{
		static int trickframe;

		if ((int)gl_clear->value)
			qglClear(GL_COLOR_BUFFER_BIT);

		trickframe++;
		if (trickframe & 1)
		{
			gldepthmin = 0.0f;
			gldepthmax = 0.49999f;
			qglDepthFunc(GL_LEQUAL);
		}
		else
		{
			gldepthmin = 1.0f;
			gldepthmax = 0.5f;
			qglDepthFunc(GL_GEQUAL);
		}
	}
	else
	{
		// H2: extra fog rendering logic.
		// mxd. Removed gl_fog_broken cvar logic, moved cl_camera_under_surface logic to separate else if case.
		if (!(int)r_fog->value || r_farclipdist->value < r_fog_startdist->value)
		{
			qglDisable(GL_FOG);

			if ((int)gl_clear->value)
				qglClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			else
				qglClear(GL_DEPTH_BUFFER_BIT);
		}
		else if ((int)cl_camera_under_surface->value)
		{
			GL_WaterFog();
		}
		else
		{
			GL_Fog();
		}

		gldepthmin = 0.0f;
		gldepthmax = 1.0f;
		qglDepthFunc(GL_LEQUAL);
	}

	qglDepthRange((double)gldepthmin, (double)gldepthmax);
}

void R_Register(void)
{
	r_norefresh = Cvar_Get("r_norefresh", "0", 0);
	r_fullbright = Cvar_Get("r_fullbright", "0", 0);
	r_drawentities = Cvar_Get("r_drawentities", "1", 0);
	r_drawworld = Cvar_Get("r_drawworld", "1", 0);
	r_novis = Cvar_Get("r_novis", "0", 0);
	r_nocull = Cvar_Get("r_nocull", "0", 0);
	r_lerpmodels = Cvar_Get("r_lerpmodels", "1", 0);
	r_speeds = Cvar_Get("r_speeds", "0", 0);

	r_lightlevel = Cvar_Get("r_lightlevel", "0", 0);

	// NEW:
	r_farclipdist = Cvar_Get("r_farclipdist", "4096.0", 0);
	r_fog = Cvar_Get("r_fog", "0", 0);
	r_fog_mode = Cvar_Get("r_fog_mode", "1", 0);
	r_fog_density = Cvar_Get("r_fog_density", "0.004", 0);
	r_fog_startdist = Cvar_Get("r_fog_startdist", "50.0", 0);
	r_fog_color_r = Cvar_Get("r_fog_color_r", "1.0", 0);
	r_fog_color_g = Cvar_Get("r_fog_color_g", "1.0", 0);
	r_fog_color_b = Cvar_Get("r_fog_color_b", "1.0", 0);
	r_fog_color_a = Cvar_Get("r_fog_color_a", "0.0", 0);
	r_fog_color_scale = Cvar_Get("r_fog_color_scale", "1.0", 0);
	r_fog_lightmap_adjust = Cvar_Get("r_fog_lightmap_adjust", "5.0", 0);
	r_fog_underwater = Cvar_Get("r_fog_underwater", "0", 0);
	r_fog_underwater_mode = Cvar_Get("r_fog_underwater_mode", "1", 0);
	r_fog_underwater_density = Cvar_Get("r_fog_underwater_density", "0.0015", 0);
	r_fog_underwater_startdist = Cvar_Get("r_fog_underwater_startdist", "100.0", 0);
	r_fog_underwater_color_r = Cvar_Get("r_fog_underwater_color_r", "1.0", 0);
	r_fog_underwater_color_g = Cvar_Get("r_fog_underwater_color_g", "1.0", 0);
	r_fog_underwater_color_b = Cvar_Get("r_fog_underwater_color_b", "1.0", 0);
	r_fog_underwater_color_a = Cvar_Get("r_fog_underwater_color_a", "0.0", 0);
	r_fog_underwater_color_scale = Cvar_Get("r_fog_underwater_color_scale", "1.0", 0);
	r_fog_underwater_lightmap_adjust = Cvar_Get("r_fog_underwater_lightmap_adjust", "5.0", 0);
	r_underwater_color = Cvar_Get("r_underwater_color", "0x70c06000", 0);
	r_frameswap = Cvar_Get("r_frameswap", "1.0", 0);
	r_references = Cvar_Get("r_references", "1.0", 0);

	gl_nosubimage = Cvar_Get("gl_nosubimage", "0", 0);
	gl_allow_software = Cvar_Get("gl_allow_software", "0", 0);

	gl_particle_min_size = Cvar_Get("gl_particle_min_size", "2", CVAR_ARCHIVE);
	gl_particle_max_size = Cvar_Get("gl_particle_max_size", "40", CVAR_ARCHIVE);
	gl_particle_size = Cvar_Get("gl_particle_size", "40", CVAR_ARCHIVE);
	gl_particle_att_a = Cvar_Get("gl_particle_att_a", "0.01", CVAR_ARCHIVE);
	gl_particle_att_b = Cvar_Get("gl_particle_att_b", "0.0", CVAR_ARCHIVE);
	gl_particle_att_c = Cvar_Get("gl_particle_att_c", "0.01", CVAR_ARCHIVE);
	gl_noartifacts = Cvar_Get("gl_noartifacts", "0", 0); // NEW

	gl_modulate = Cvar_Get("gl_modulate", "1", CVAR_ARCHIVE);
	gl_log = Cvar_Get("gl_log", "0", 0);
	gl_bitdepth = Cvar_Get("gl_bitdepth", "0", 0);
	gl_lightmap = Cvar_Get("gl_lightmap", "0", 0);
	gl_shadows = Cvar_Get("gl_shadows", "0", CVAR_ARCHIVE);
	gl_dynamic = Cvar_Get("gl_dynamic", "1", 0);
	gl_nobind = Cvar_Get("gl_nobind", "0", 0);
	gl_round_down = Cvar_Get("gl_round_down", "1", 0);
	gl_showtris = Cvar_Get("gl_showtris", "0", 0);
	gl_reporthash = Cvar_Get("gl_reporthash", "0", 0);
	gl_ztrick = Cvar_Get("gl_ztrick", "0", 0);
	gl_finish = Cvar_Get("gl_finish", "0", 0);
	gl_clear = Cvar_Get("gl_clear", "0", 0);
	gl_cull = Cvar_Get("gl_cull", "1", 0);
	gl_polyblend = Cvar_Get("gl_polyblend", "1", 0);
	gl_flashblend = Cvar_Get("gl_flashblend", "0", 0);
	gl_playermip = Cvar_Get("gl_playermip", "0", 0);
	gl_monolightmap = Cvar_Get("gl_monolightmap", "0", 0);
	gl_driver = Cvar_Get("gl_driver", "opengl32", CVAR_ARCHIVE);
	gl_texturemode = Cvar_Get("gl_texturemode", "GL_LINEAR_MIPMAP_NEAREST", CVAR_ARCHIVE);
	// Missing: gl_texturealphamode, gl_texturesolidmode?
	gl_lockpvs = Cvar_Get("gl_lockpvs", "0", 0);

	// NEW:
	gl_drawflat = Cvar_Get("gl_drawflat", "0", 0);
	gl_devel1 = Cvar_Get("gl_devel1", "0", 0);
	gl_trans33 = Cvar_Get("gl_trans33", "1", 0);
	gl_trans66 = Cvar_Get("gl_trans66", "1", 0);
	gl_picmip = Cvar_Get("gl_picmip", "0", CVAR_ARCHIVE);
	gl_skinmip = Cvar_Get("gl_skinmip", "0", CVAR_ARCHIVE);
	gl_bookalpha = Cvar_Get("gl_bookalpha", "1.0", 0);

	gl_ext_swapinterval = Cvar_Get("gl_ext_swapinterval", "1", CVAR_ARCHIVE);
	gl_ext_gamma = Cvar_Get("gl_ext_gamma", "1", CVAR_ARCHIVE);
	gl_ext_palettedtexture = Cvar_Get("gl_ext_palettedtexture", "1", CVAR_ARCHIVE); //TODO: ignored. Remove?
	gl_ext_multitexture = Cvar_Get("gl_ext_multitexture", "1", CVAR_ARCHIVE);
	gl_ext_pointparameters = Cvar_Get("gl_ext_pointparameters", "1", CVAR_ARCHIVE);
	gl_drawmode = Cvar_Get("gl_drawmode", "0", 0);

	gl_drawbuffer = Cvar_Get("gl_drawbuffer", "GL_BACK", 0);
	gl_swapinterval = Cvar_Get("gl_swapinterval", "1", CVAR_ARCHIVE);
	gl_sortmulti = Cvar_Get("gl_sortmulti", "0", CVAR_ARCHIVE); // NEW

	gl_saturatelighting = Cvar_Get("gl_saturatelighting", "0", 0);

	// ri.Cvar_Get() in Q2
	gl_3dlabs_broken = ri.Cvar_FullSet("gl_3dlabs_broken", "1", 0);
	gl_lostfocus_broken = ri.Cvar_FullSet("gl_lostfocus_broken", "0", 0); // NEW
	gl_fog_broken = ri.Cvar_FullSet("gl_fog_broken", "0", 0); // NEW
	gl_envmap_broken = ri.Cvar_FullSet("gl_envmap_broken", "0", 0); // NEW
	gl_screenshot_broken = ri.Cvar_FullSet("gl_screenshot_broken", "0", 0); // NEW

	vid_fullscreen = Cvar_Get("vid_fullscreen", "0", CVAR_ARCHIVE);
	vid_gamma = Cvar_Get("vid_gamma", "0.5", CVAR_ARCHIVE);
	vid_brightness = Cvar_Get("vid_brightness", "0.5", CVAR_ARCHIVE); // NEW
	vid_contrast = Cvar_Get("vid_contrast", "0.5", CVAR_ARCHIVE); // NEW

	vid_ref = Cvar_Get("vid_ref", "gl", CVAR_ARCHIVE);

	// NEW:
	vid_mode = Cvar_Get("vid_mode", "3", CVAR_ARCHIVE);
	menus_active = Cvar_Get("menus_active", "0", 0);
	cl_camera_under_surface = Cvar_Get("cl_camera_under_surface", "0", 0);
	quake_amount = Cvar_Get("quake_amount", "0", 0);

	ri.Cmd_AddCommand("imagelist", GL_ImageList_f);
	ri.Cmd_AddCommand("screenshot", GL_ScreenShot_f);
	ri.Cmd_AddCommand("modellist", Mod_Modellist_f);
	ri.Cmd_AddCommand("gl_strings", GL_Strings_f);

	InitGammaTable(); // NEW
}

qboolean R_SetMode(void)
{
	if (vid_fullscreen->modified && !gl_config.allow_cds)
	{
		Com_Printf("R_SetMode() - CDS not allowed with this driver\n");
		Cvar_SetValue("vid_fullscreen", (float)!(int)vid_fullscreen->value);
		vid_fullscreen->modified = false;
	}

	vid_fullscreen->modified = false;
	vid_mode->modified = false;

	rserr_t err = GLimp_SetMode(&viddef.width, &viddef.height, (int)vid_mode->value, (int)vid_fullscreen->value, true);
	if (err == rserr_ok)
	{
		gl_state.prev_mode = (int)vid_mode->value;
		return true;
	}

	if (err == rserr_invalid_fullscreen)
	{
		Cvar_SetValue("vid_fullscreen", 0);
		vid_fullscreen->modified = false;
		Com_Printf("ref_gl::R_SetMode() - fullscreen unavailable in this mode\n");

		err = GLimp_SetMode(&viddef.width, &viddef.height, (int)vid_mode->value, false, true);
		if (err == rserr_ok)
			return true;
	}
	else if (err == rserr_invalid_mode)
	{
		Cvar_SetValue("vid_mode", (float)gl_state.prev_mode);
		vid_mode->modified = false;
		Com_Printf("ref_gl::R_SetMode() - invalid mode\n");
	}

	// Try setting it back to something safe
	err = GLimp_SetMode(&viddef.width, &viddef.height, gl_state.prev_mode, false, true);
	if (err != rserr_ok)
	{
		Com_Printf("ref_gl::R_SetMode() - could not revert to safe mode\n");
		return false;
	}

	return true;
}

qboolean R_Init(void* hinstance, void* hWnd)
{
	extern float r_turbsin[256];

	for (int j = 0; j < 256; j++)
		r_turbsin[j] *= 0.5f;

	Com_Printf("ref_gl version: "REF_VERSION"\n"); // ri.Con_Printf in Q2 (here and below)
	R_Register();

	// Initialize our QGL dynamic bindings
	char driver_path[256];
	Com_sprintf(driver_path, 256, "Drivers/%s", gl_driver->string); // H2: extra local driver QGL_Init call 

	if (!QGL_Init(driver_path) && !QGL_Init(gl_driver->string))
	{
		QGL_Shutdown();
		Com_Printf("%s - could not load \"%s\"\n", __func__, gl_driver->string);
		return false; //mxd. Decompiled code still returns -1 here...
	}

	// Initialize OS - specific parts of OpenGL
	if (!GLimp_Init(hinstance, hWnd))
	{
		QGL_Shutdown();
		return false; //mxd. Decompiled code still returns -1 here...
	}

	// Set our "safe" modes
	gl_state.prev_mode = 3;

	// Create the window and set up the context
	if (!R_SetMode())
	{
		QGL_Shutdown();
		Com_Printf("ref_gl::R_Init() - could not R_SetMode()\n");
		return false; //mxd. Decompiled code still returns -1 here...
	}

	// Get our various GL strings
	gl_config.vendor_string = (const char*)(*qglGetString)(GL_VENDOR);
	Com_Printf("GL_VENDOR: %s\n", gl_config.vendor_string);
	gl_config.renderer_string = (const char*)(*qglGetString)(GL_RENDERER);
	Com_Printf("GL_RENDERER: %s\n", gl_config.renderer_string);
	gl_config.version_string = (const char*)(*qglGetString)(GL_VERSION);
	Com_Printf("GL_VERSION: %s\n", gl_config.version_string);
	gl_config.extensions_string = (const char*)(*qglGetString)(GL_EXTENSIONS);
	//Com_Printf("GL_EXT: hidden\n", gl_config.extensions_string); //mxd. Modern extensions_string is longer than Com_Printf can handle...

	//mxd. Skip copious amounts of ancient videocard checks, assume everything works...
	gl_config.renderer = GL_RENDERER_DEFAULT;
	gl_config.allow_cds = true;

	// Grab extensions
#ifdef _WIN32
	if (strstr(gl_config.extensions_string, "WGL_EXT_swap_control"))
	{
		qwglSwapIntervalEXT = (BOOL (WINAPI*)(int))qwglGetProcAddress("wglSwapIntervalEXT");
		Com_Printf("...enabling WGL_EXT_swap_control\n");
	}
	else
	{
		Com_Printf("...WGL_EXT_swap_control not found\n");
	}
#endif

	if (strstr(gl_config.extensions_string, "GL_EXT_point_parameters"))
	{
		if ((int)gl_ext_pointparameters->value)
		{
			qglPointParameterfEXT = (void (APIENTRY*)(GLenum, GLfloat))qwglGetProcAddress("glPointParameterfEXT");
			qglPointParameterfvEXT = (void (APIENTRY*)(GLenum, const GLfloat*))qwglGetProcAddress("glPointParameterfvEXT");
			Com_Printf("...using GL_EXT_point_parameters\n");
		}
		else
		{
			Com_Printf("...ignoring GL_EXT_point_parameters\n");
		}
	}
	else
	{
		Com_Printf("...GL_EXT_point_parameters not found\n");
	}

	//mxd. Skip qglColorTableEXT logic. Required 'GL_EXT_shared_texture_palette' extension is unsupported since GeForceFX
	//https://community.khronos.org/t/does-not-support-ext-paletted-texture-on-geforcefx

	if (strstr(gl_config.extensions_string, "GL_ARB_multitexture"))
	{
		if ((int)gl_ext_multitexture->value)
		{
			Com_Printf("...using GL_ARB_multitexture\n");
			qglMultiTexCoord2fARB = (void (APIENTRY*)(GLenum, GLfloat, GLfloat))qwglGetProcAddress("glMultiTexCoord2fARB");
			qglActiveTextureARB = (void (APIENTRY*)(GLenum))qwglGetProcAddress("glActiveTextureARB");
		}
		else
		{
			Com_Printf("...ignoring GL_ARB_multitexture\n");
		}
	}
	else
	{
		Com_Printf("...GL_ARB_multitexture not found\n");
	}

	//mxd. GL_SGIS_multitexture logic skipped
	//mxd. Glide version check logic skipped
	//mxd. "WE DO NOT SUPPORT THE POWERVR" logic skipped

	GL_SetDefaultState();
	//GL_DrawStereoPattern(); // Enabled in H2. Did H3D paid iD the money they owed them?..
	GL_InitImages();
	Mod_Init();
	Draw_InitLocal();

	const GLenum err = qglGetError();
	if (err != GL_NO_ERROR)
	{
		Com_Printf("glGetError() = 0x%x\n", err);
		return false;
	}

	return true; //mxd. Return value missing in Q2
}

void R_Shutdown(void)
{
	NOT_IMPLEMENTED
}

void R_BeginFrame(const float camera_separation)
{
	gl_state.camera_separation = camera_separation;

	// Change modes if necessary
	if (vid_mode->modified || vid_fullscreen->modified)
	{
		// FIXME: only restart if CDS is required
		cvar_t* ref = Cvar_Get("vid_ref", "gl", 0); //TODO: can't we just use vid_ref global var here? 
		ref->modified = true;
	}

	if (gl_log->modified)
	{
		GLimp_EnableLogging((qboolean)gl_log->value);
		gl_log->modified = false;
	}

	if ((int)gl_log->value)
		GLimp_LogNewFrame();

	// Changed
	if (vid_gamma->modified || vid_brightness->modified || vid_contrast->modified)
	{
		InitGammaTable();
		GL_GammaAffect();

		vid_gamma->modified = false;
		vid_brightness->modified = false;
		vid_contrast->modified = false;
	}

	GLimp_BeginFrame(camera_separation);

	// Go into 2D mode
	qglViewport(0, 0, viddef.width, viddef.height);
	qglMatrixMode(GL_PROJECTION);
	qglLoadIdentity();
	qglOrtho(0.0, viddef.width, viddef.height, 0.0, -99999.0, 99999.0);
	qglMatrixMode(GL_MODELVIEW);
	qglLoadIdentity();
	qglDisable(GL_DEPTH_TEST);
	qglDisable(GL_CULL_FACE);
	qglDisable(GL_BLEND);
	qglEnable(GL_ALPHA_TEST);
	qglColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	// Draw buffer stuff
	if (gl_drawbuffer->modified)
	{
		gl_drawbuffer->modified = false;

		if (!gl_state.stereo_enabled || gl_state.camera_separation == 0.0f)
		{
			if (Q_stricmp(gl_drawbuffer->string, "GL_FRONT") == 0)
				qglDrawBuffer(GL_FRONT);
			else
				qglDrawBuffer(GL_BACK);
		}
	}

	// Texturemode stuff
	if (gl_texturemode->modified)
	{
		GL_TextureMode(gl_texturemode->string);
		gl_texturemode->modified = false;
	}

	// Missing: gl_texturealphamode and gl_texturesolidmode logic

	// Swapinterval stuff
	GL_UpdateSwapInterval();

	// Clear screen if desired
	R_Clear();
}

int R_RenderFrame(refdef_t* fd)
{
	NOT_IMPLEMENTED
	return 0;
}

// Referenced by GetRefAPI only:
void R_BeginRegistration(char* model);
struct model_s* R_RegisterModel(char* name);
struct image_s* R_RegisterSkin(char* name, qboolean* retval);
void R_SetSky(char* name, float rotate, vec3_t axis);
void R_EndRegistration(void);

void Draw_InitCinematic(int w, int h, char* overlay, char* backdrop);
void Draw_CloseCinematic(void);
void Draw_Cinematic(int cols, int rows, byte* data, paletteRGB_t* palette, float alpha);

void Draw_Name(vec3_t origin, char* name, paletteRGBA_t color);
int FindSurface(vec3_t start, vec3_t end, struct Surface_s* surface);

refexport_t GetRefAPI(refimport_t rimp)
{
	refexport_t re;

	ri = rimp;

	re.api_version = API_VERSION;
	re.render = false; //mxd. Shut up compiler...

	re.BeginRegistration = R_BeginRegistration;
	re.RegisterModel = R_RegisterModel;
	re.RegisterSkin = R_RegisterSkin;
	re.RegisterPic = Draw_FindPic;
	re.SetSky = R_SetSky;
	re.EndRegistration = R_EndRegistration;
	re.GetReferencedID = GetReferencedID;

	re.RenderFrame = R_RenderFrame;

	re.DrawGetPicSize = Draw_GetPicSize;
	re.DrawPic = Draw_Pic;
	re.DrawStretchPic = Draw_StretchPic;
	re.DrawChar = Draw_Char;
	re.DrawTileClear = Draw_TileClear;
	re.DrawFill = Draw_Fill;
	re.DrawFadeScreen = Draw_FadeScreen;
	// Missing: Draw_StretchRaw

	re.DrawBigFont = Draw_BigFont;
	re.BF_Strlen = BF_Strlen;
	re.BookDrawPic = Draw_BookPic;
	re.DrawInitCinematic = Draw_InitCinematic;
	re.DrawCloseCinematic = Draw_CloseCinematic;
	re.DrawCinematic = Draw_Cinematic;
	re.Draw_Name = Draw_Name;

	re.Init = R_Init;
	re.Shutdown = R_Shutdown;

	// Missing: R_SetPalette
	re.BeginFrame = R_BeginFrame;
	re.EndFrame = GLimp_EndFrame;
	re.AppActivate = GLimp_AppActivate;
	re.FindSurface = FindSurface;

	// Missing: Swap_Init();
	// Unbound: A3D_RenderGeometry();

	return re;
}