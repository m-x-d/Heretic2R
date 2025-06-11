//
// gl_misc.c
//
// Copyright 1998 Raven Software
//

#include "gl_local.h"
#include "Vector.h"
#include "vid.h"

void GL_ScreenShot_f(void)
{
	uint i;
	FILE* f;
	char picname[80];
	char filename[MAX_OSPATH];

	//mxd. Skipping gl_screenshot_broken logic.

	// Create the screenshots directory if it doesn't exist.
	Com_sprintf(filename, sizeof(filename), "%s/scrnshot/", ri.FS_Userdir()); // H2: FS_Gamedir -> FS_Userdir //TODO: "scrnshot" -> "screenshots"?
	ri.FS_CreatePath(filename);
	strcpy_s(picname, sizeof(picname), "Htic2-00.tga"); //TODO: save in PNG format? "Htic2" -> "Heretic2R"?

	for (i = 0; i < 100; i++)
	{
		picname[6] = (char)(i / 10 + '0');
		picname[7] = (char)(i % 10 + '0');
		Com_sprintf(filename, sizeof(filename), "%s/scrnshot/%s", ri.FS_Userdir(), picname); // H2: FS_Gamedir -> FS_Userdir

		if (fopen_s(&f, filename, "rb") != 0) //mxd. fopen -> fopen_s
			break;

		fclose(f);
	}

	if (i == 100)
	{
		ri.Con_Printf(PRINT_ALL, "SCR_ScreenShot_f: couldn't create a file: too many screenshots!\n"); //mxd. Com_Printf() -> ri.Con_Printf().
		return;
	}

	const uint tga_size = viddef.width * viddef.height * 3 + 18;
	byte* buffer = malloc(tga_size);

	if (buffer == NULL) // H2: extra sanity check
	{
		ri.Con_Printf(PRINT_ALL, "SCR_ScreenShot_f: unable to malloc %i bytes\n", tga_size); //mxd. Com_Printf() -> ri.Con_Printf().
		return;
	}

	memset(buffer, 0, 18);

	// Set TGA header
	buffer[2] = 2; // Uncompressed type
	buffer[12] = (byte)viddef.width;
	buffer[13] = (byte)(viddef.width >> 8);
	buffer[14] = (byte)viddef.height;
	buffer[15] = (byte)(viddef.height >> 8);
	buffer[16] = 24; // Pixel size

	qglReadPixels(0, 0, viddef.width, viddef.height, GL_RGB, GL_UNSIGNED_BYTE, buffer + 18);

	// Swap rgb to bgr
	float brightness = 0.0f;
	for (i = 18; i < tga_size; i += 3)
	{
		const byte temp = buffer[i];
		buffer[i] = buffer[i + 2];
		buffer[i + 2] = temp;

		// H2: store total brightness
		brightness += (float)(buffer[i] + buffer[i + 1] + buffer[i + 2]);
	}

	uint written_size = 0;
	if (fopen_s(&f, filename, "wb") == 0) //mxd. fopen -> fopen_s
	{
		written_size = fwrite(buffer, 1, tga_size, f);
		fclose(f);
	}
	free(buffer);

	if (tga_size != written_size) // H2: extra sanity check
	{
		ri.Con_Printf(PRINT_ALL, "Error writing '%s'\n", filename);
		return;
	}

	// H2: new brightness warnings
	brightness /= (float)(tga_size - 18);

	if (brightness < 5.0f)
		ri.Con_Printf(PRINT_ALL, "**WARNING** Overly dark image '%s' (brightness = %2.1f)\n", filename, brightness); //mxd. Com_Printf() -> ri.Con_Printf().
	else if (brightness > 225.0f)
		ri.Con_Printf(PRINT_ALL, "**WARNING** Overly bright image '%s' (brightness = %2.1f)\n", filename, brightness); //mxd. Com_Printf() -> ri.Con_Printf().
	else
		ri.Con_Printf(PRINT_ALL, "Wrote '%s' (brightness = %2.1f)\n", filename, brightness); //mxd. Com_Printf() -> ri.Con_Printf().
}

void GL_Strings_f(void)
{
	ri.Con_Printf(PRINT_ALL, "GL_VENDOR: %s\n", gl_config.vendor_string); //mxd. Com_Printf() -> ri.Con_Printf().
	ri.Con_Printf(PRINT_ALL, "GL_RENDERER: %s\n", gl_config.renderer_string); //mxd. Com_Printf() -> ri.Con_Printf().
	ri.Con_Printf(PRINT_ALL, "GL_VERSION: %s\n", gl_config.version_string); //mxd. Com_Printf() -> ri.Con_Printf().
	//Com_Printf("GL_EXT: %s\n", gl_config.extensions_string); //mxd. Disabled, because Com_Printf can't handle strings longer than 1024 chars. //TODO: implement?
}

void GL_SetDefaultState(void)
{
	qglClearColor(1.0f, 0.0f, 0.5f, 0.5f);
	qglCullFace(GL_FRONT);
	qglEnable(GL_TEXTURE_2D);

	qglEnable(GL_ALPHA_TEST);
	qglAlphaFunc(GL_GREATER, 0.666f);

	qglDisable(GL_DEPTH_TEST);
	qglDisable(GL_CULL_FACE);
	qglDisable(GL_BLEND);

	qglColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	qglPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	qglShadeModel(GL_FLAT);

	GL_TextureMode(gl_texturemode->string);

	qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_min); //mxd. Q2/H2: qglTexParameterf
	qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max); //mxd. Q2/H2: qglTexParameterf

	qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); //mxd. Q2/H2: qglTexParameterf
	qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); //mxd. Q2/H2: qglTexParameterf

	qglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // H2_1.07: GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR.

	GL_TexEnv(GL_REPLACE);

	if (qglPointParameterfEXT)
	{
		const float attenuations[3] =
		{
			gl_particle_att_a->value,
			gl_particle_att_b->value,
			gl_particle_att_c->value
		};

		qglEnable(GL_POINT_SMOOTH);
		qglPointParameterfEXT(GL_POINT_SIZE_MIN_EXT, gl_particle_min_size->value);
		qglPointParameterfEXT(GL_POINT_SIZE_MAX_EXT, gl_particle_max_size->value);
		qglPointParameterfvEXT(GL_DISTANCE_ATTENUATION_EXT, attenuations);
	}

	//mxd. Required 'GL_EXT_shared_texture_palette' extension is unsupported since GeForceFX...
	//if (qglColorTableEXT && (int)gl_ext_palettedtexture->value)
		//qglEnable(GL_SHARED_TEXTURE_PALETTE_EXT);
}

// Q2 counterpart
void GL_UpdateSwapInterval(void)
{
	if (gl_swapinterval->modified)
	{
		gl_swapinterval->modified = false;

		if (!gl_state.stereo_enabled) //TODO: always false. Remove?
		{
#ifdef _WIN32
			if (qwglSwapIntervalEXT)
				qwglSwapIntervalEXT((int)gl_swapinterval->value);
#endif
		}
	}
}

// Transforms vector to screen space?
void TransformVector(const vec3_t v, vec3_t out)
{
	out[0] = DotProduct(v, vright);
	out[1] = DotProduct(v, vup);
	out[2] = DotProduct(v, vpn);
}