//
// vid_Screenshot.c
//
// Copyright 2025 mxd
//

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_WRITE_NO_STDIO
#include <stb/stb_image_write.h>

#include "vid_Screenshot.h"
#include "qcommon.h"

#define MAX_SCREENSHOTS	10000 // 100 in Q2/H2
#define PNG_COMPRESSION	5

static void VID_WriteScreenshotCallback(void* context, void* data, const int size)
{
	const char* scr_filepath = context;
	int written_size = 0;

	FILE* f;
	if (fopen_s(&f, scr_filepath, "wb") == 0) //mxd. fopen -> fopen_s
	{
		written_size = (int)fwrite(data, 1, size, f);
		fclose(f);
	}

	if (size != written_size) // H2: extra sanity check.
		Com_Printf("Error writing '%s'!\n", scr_filepath);
}

// Writes a screenshot. This function is called with raw image data of width * height pixels, each pixel has 'comp' bytes.
// Must be 3 or 4, for RGB or RGBA. The pixels must be given row-wise, stating at the top-left.
void VID_WriteScreenshot(const int width, const int height, const int comp, const void* data)
{
	// Create the screenshots directory if it doesn't exist.
	char scr_dir[MAX_OSPATH];
	Com_sprintf(scr_dir, sizeof(scr_dir), "%s/screenshots/", FS_Userdir()); // H2: FS_Gamedir -> FS_Userdir
	FS_CreatePath(scr_dir);

	char scr_filepath[MAX_OSPATH];
	qboolean filename_found = false;

	// Find a file name to save it to.
	for (int i = 0; i < MAX_SCREENSHOTS; i++)
	{
		Com_sprintf(scr_filepath, sizeof(scr_filepath), "%sH2R-%04i.png", scr_dir, i);

		FILE* f;
		if (fopen_s(&f, scr_filepath, "rb") != 0) //mxd. fopen -> fopen_s
		{
			filename_found = true;
			break; // File doesn't exist.
		}

		fclose(f);
	}

	if (!filename_found)
	{
		Com_Printf("VID_WriteScreenshot: couldn't create a file: too many screenshots!\n");
		return;
	}

	// Now save it.
	stbi_write_png_compression_level = PNG_COMPRESSION;
	stbi_flip_vertically_on_write(true);

	if (stbi_write_png_to_func(VID_WriteScreenshotCallback, scr_filepath, width, height, comp, data, 0)) //mxd. stbi_write_png_to_func() frees 'data'.
	{
		// H2: extra brightness check logic.
		float brightness = 0.0f;
		const int data_size = width * height * comp;
		const byte* pixels = data;

		for (int i = 0; i < data_size; i += comp)
			brightness += (float)(pixels[i] + pixels[i + 1] + pixels[i + 2]);

		brightness = brightness / (float)data_size * 100.0f / 255.0f; //mxd. Rescale from 0 .. 255 to 0 .. 100.

		if (brightness < 2.0f) // H2: 5 (out of 255).
			Com_Printf("**WARNING** Overly dark image '%s' (brightness: %2.1f%%)\n", scr_filepath, brightness);
		else if (brightness > 88.0f) // H2: 225 (out of 255).
			Com_Printf("**WARNING** Overly bright image '%s' (brightness: %2.1f%%)\n", scr_filepath, brightness);
		else
			Com_Printf("Wrote '%s' (brightness: %2.1f%%)\n", scr_filepath, brightness);
	}
	else
	{
		Com_Printf("VID_WriteScreenshot: couldn't write '%s'!\n", scr_filepath);
	}
}