//
// gl1_Light.c
//
// Copyright 1998 Raven Software
//

#include "gl1_Light.h"

static float s_blocklights[34 * 34 * 3];

static void R_AddDynamicLights(const msurface_t* surf)
{
	NOT_IMPLEMENTED
}

// Q2 counterpart
void R_SetCacheState(msurface_t* surf)
{
	for (int maps = 0; maps < MAXLIGHTMAPS && surf->styles[maps] != 255; maps++)
		surf->cached_light[maps] = r_newrefdef.lightstyles[surf->styles[maps]].white;
}

// Q2 counterpart (in H2, except for extra SURF_TALL_WALL flag).
void R_BuildLightMap(const msurface_t* surf, byte* dest, int stride)
{
	if (surf->texinfo->flags & SURF_FULLBRIGHT) //mxd. SURF_FULLBRIGHT define.
		ri.Sys_Error(ERR_DROP, "R_BuildLightMap called for non-lit surface");

	const int smax = (surf->extents[0] >> 4) + 1;
	const int tmax = (surf->extents[1] >> 4) + 1;
	const int size = smax * tmax;

	if (size > (int)sizeof(s_blocklights) >> 4)
		ri.Sys_Error(ERR_DROP, "Bad s_blocklights size");

	// Set to full bright if no light data.
	if (surf->samples == NULL)
	{
		for (int i = 0; i < size * 3; i++)
			s_blocklights[i] = 255.0f;
	}
	else
	{
		//mxd. Don't count the number of maps.
		const byte* lightmap = surf->samples;

		// Add all the lightmaps //mxd. Skip nummaps == 1 logic.
		memset(s_blocklights, 0, sizeof(s_blocklights[0]) * size * 3);

		for (int maps = 0; maps < MAXLIGHTMAPS && surf->styles[maps] != 255; maps++)
		{
			float scale[3];
			for (int i = 0; i < 3; i++)
				scale[i] = gl_modulate->value * r_newrefdef.lightstyles[surf->styles[maps]].rgb[i];

			//mxd. Skip scale == 1.0 logic.
			float* bl = &s_blocklights[0];
			for (int i = 0; i < size; i++, bl += 3)
				for (int j = 0; j < 3; j++)
					bl[j] += (float)lightmap[i * 3 + j] * scale[j];

			// Skip to next lightmap.
			lightmap += size * 3;
		}

		// Add all the dynamic lights.
		if (surf->dlightframe == r_framecount)
			R_AddDynamicLights(surf);
	}

	// Put into texture format.
	stride -= (smax << 2);
	const float* bl = &s_blocklights[0];

	//mxd. Skip gl_monolightmap logic.
	for (int i = 0; i < tmax; i++, dest += stride)
	{
		for (int j = 0; j < smax; j++, bl += 3, dest += 4)
		{
			int r = Q_ftol(bl[0]);
			int g = Q_ftol(bl[1]);
			int b = Q_ftol(bl[2]);

			// Catch negative lights.
			r = max(r, 0);
			g = max(g, 0);
			b = max(b, 0);

			// Determine the brightest of the three color components.
			const int max = max(r, max(g, b));

			// Alpha is ONLY used for the mono lightmap case.
			// For this reason we set it to the brightest of the color components so that things don't get too dim.
			int a = max;

			// Rescale all the color components if the intensity of the greatest channel exceeds 1.0.
			if (max > 255)
			{
				const float t = 255.0f / (float)max;

				r = (int)((float)r * t);
				g = (int)((float)g * t);
				b = (int)((float)b * t);
				a = (int)((float)a * t);
			}

			dest[0] = (byte)r;
			dest[1] = (byte)g;
			dest[2] = (byte)b;
			dest[3] = (byte)a;
		}
	}
}