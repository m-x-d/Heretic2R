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
#include "menus/menu_misc.h"

#define MAX_SCREENSHOTS				10000 // 100 in Q2/H2
#define SCREENSHOT_FILENAME_FORMAT	"%sH2R-%04i.%s"

#define PNG_COMPRESSION	5
#define JPG_COMPRESSION	75

typedef struct ScreenshotSaveBuffer_s
{
	byte* data;
	int size;
	int offset;
} ScreenshotSaveBuffer_t;

// Called for every new BYTE when saving JPG, hence extra save buffer... --mxd.
static void VID_WriteScreenshotCallback(void* context, void* data, const int size)
{
	ScreenshotSaveBuffer_t* buf = context;

	memcpy_s(&buf->data[buf->offset], buf->size - buf->offset, data, size);
	buf->offset += size;
}

static int VID_GetFreeScreenshotIndex(const char* screenshots_dir) //mxd
{
	// Find a free screenshot filename index.
	for (int i = 0; i < MAX_SCREENSHOTS; i++)
	{
		qboolean file_exists = false;

		// Assume free only when no files with this index exist.
		for (uint c = 0; c < SSF_NUM_FORMATS; c++)
		{
			if (Sys_IsFile(va(SCREENSHOT_FILENAME_FORMAT, screenshots_dir, i, screenshot_formats[c])))
			{
				file_exists = true;
				break;
			}
		}

		if (!file_exists)
			return i;
	}

	// No dice...
	return -1;
}

// Writes a screenshot. This function is called with raw image data of width * height pixels, each pixel has 'comp' bytes.
// Must be 3 or 4, for RGB or RGBA. The pixels must be given row-wise, stating at the top-left.
void VID_WriteScreenshot(const int width, const int height, const int comp, const void* data)
{
	// Create the screenshots directory if it doesn't exist.
	char scr_dir[MAX_OSPATH];
	Com_sprintf(scr_dir, sizeof(scr_dir), "%s/screenshots/", FS_Userdir()); // H2: FS_Gamedir -> FS_Userdir
	FS_CreatePath(scr_dir);

	// Find a free screenshot filename index.
	const int screenshot_index = VID_GetFreeScreenshotIndex(scr_dir);

	if (screenshot_index == -1)
	{
		Com_Printf("VID_WriteScreenshot: couldn't create a file: too many screenshots!\n");
		return;
	}

	// Determine save format.
	const ScreenshotSaveFormat_t save_format = M_GetCurrentScreenshotSaveFormat();

	// Convert it to target format.
	stbi_flip_vertically_on_write(true);
	qboolean success;

	const int buf_size = width * height * comp; //mxd. Uncompressed data size SHOULD be enough to save compressed image, right?..
	ScreenshotSaveBuffer_t buf = { .offset = 0, .size = buf_size, .data = malloc(buf_size) };

	switch (save_format)
	{
		case SSF_JPG:
		default:
			success = stbi_write_jpg_to_func(VID_WriteScreenshotCallback, &buf, width, height, comp, data, JPG_COMPRESSION);
			break;

		case SSF_PNG: // png
			stbi_write_png_compression_level = PNG_COMPRESSION;
			success = stbi_write_png_to_func(VID_WriteScreenshotCallback, &buf, width, height, comp, data, 0);
			break;
	}

	char scr_filepath[MAX_OSPATH];
	Com_sprintf(scr_filepath, sizeof(scr_filepath), SCREENSHOT_FILENAME_FORMAT, scr_dir, screenshot_index, screenshot_formats[save_format]);

	// Save it.
	if (success)
	{
		FILE* f;
		int written_size = -1;

		if (fopen_s(&f, scr_filepath, "wb") == 0) //mxd. fopen -> fopen_s
		{
			written_size = (int)fwrite(buf.data, 1, buf.size, f);
			fclose(f);
		}

		success = (buf.size == written_size);
	}

	free(buf.data);

	if (success)
	{
		// H2: determine image brightness.
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