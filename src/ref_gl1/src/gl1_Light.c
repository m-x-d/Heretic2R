//
// gl1_Light.c
//
// Copyright 1998 Raven Software
//

#include "gl1_Light.h"
#include "Vector.h"

#define DLIGHT_CUTOFF	64.0f

static int r_dlightframecount; //mxd. Made static.
static float s_blocklights[34 * 34 * 3];

static vec3_t pointcolor;

#pragma region ========================== DYNAMIC LIGHTS RENDERING ==========================

// Q2 counterpart (except for dlight color handling).
static void R_RenderDlight(const dlight_t* light)
{
	vec3_t v;

	const float rad = light->intensity * 0.35f;
	VectorSubtract(light->origin, r_origin, v);

	glBegin(GL_TRIANGLE_FAN);

	glColor3f((float)light->color.r / 255.0f * 0.2f, (float)light->color.g / 255.0f * 0.2f, (float)light->color.b / 255.0f * 0.2f);
	for (int i = 0; i < 3; i++)
		v[i] = light->origin[i] - vpn[i] * rad;
	glVertex3fv(v);

	glColor3f(0.0f, 0.0f, 0.0f);
	for (int i = 16; i >= 0; i--)
	{
		const float a = (float)i / 16.0f * ANGLE_360;

		for (int j = 0; j < 3; j++)
			v[j] = light->origin[j] + vright[j] * cosf(a) * rad + vup[j] * sinf(a) * rad;

		glVertex3fv(v);
	}

	glEnd();
}

void R_RenderDlights(void)
{
	if (!(int)gl_flashblend->value) // H2_1.07: the check is inverted.
		return;

	r_dlightframecount = r_framecount + 1; // Because the count hasn't advanced yet for this frame.

	glDepthMask(GL_FALSE);
	glDisable(GL_TEXTURE_2D);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);

	dlight_t* l = &r_newrefdef.dlights[0];
	for (int i = 0; i < r_newrefdef.num_dlights; i++, l++)
		R_RenderDlight(l);

	glColor3f(1.0f, 1.0f, 1.0f);
	glDisable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // H2_1.07: GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR
	glDepthMask(GL_TRUE);
}

#pragma endregion

#pragma region ========================== DYNAMIC LIGHTS MANAGEMENT ==========================

// Q2 counterpart
void R_MarkLights(dlight_t* light, const int bit, const mnode_t* node)
{
	if (node->contents != -1)
		return;

	const cplane_t* splitplane = node->plane;
	const float dist = DotProduct(light->origin, splitplane->normal) - splitplane->dist;

	if (dist > light->intensity - DLIGHT_CUTOFF)
	{
		R_MarkLights(light, bit, node->children[0]);
		return;
	}

	if (dist < -light->intensity + DLIGHT_CUTOFF)
	{
		R_MarkLights(light, bit, node->children[1]);
		return;
	}

	// Mark the polygons.
	msurface_t* surf = &r_worldmodel->surfaces[node->firstsurface];
	for (int i = 0; i < node->numsurfaces; i++, surf++)
	{
		if (surf->dlightframe != r_dlightframecount)
		{
			surf->dlightbits = 0;
			surf->dlightframe = r_dlightframecount;
		}

		surf->dlightbits |= bit;
	}

	R_MarkLights(light, bit, node->children[0]);
	R_MarkLights(light, bit, node->children[1]);
}

// Q2 counterpart
void R_PushDlights(void)
{
	if (!(int)gl_flashblend->value)
	{
		r_dlightframecount = r_framecount + 1; // Because the count hasn't advanced yet for this frame.

		dlight_t* l = &r_newrefdef.dlights[0];
		for (int i = 0; i < r_newrefdef.num_dlights; i++, l++)
			R_MarkLights(l, 1 << i, r_worldmodel->nodes);
	}
}

static int R_RecursiveLightPoint(const mnode_t* node, const vec3_t start, const vec3_t end)
{
	// Didn't hit anything.
	if (node->contents != -1)
		return -1;

	// Calculate mid point.
	const cplane_t* plane = node->plane;
	const float front = DotProduct(start, plane->normal) - plane->dist;
	const float back = DotProduct(end, plane->normal) - plane->dist;
	const int side = (front < 0.0f);

	if ((back < 0.0f) == side)
		return R_RecursiveLightPoint(node->children[side], start, end);

	const float frac = front / (front - back);

	vec3_t mid;
	for (int c = 0; c < 3; c++)
		mid[c] = start[c] + (end[c] - start[c]) * frac;

	// Go down front side.
	const int r = R_RecursiveLightPoint(node->children[side], start, mid);

	// Hit something.
	if (r >= 0)
		return r;

	// Didn't hit anything.
	if ((back < 0.0f) == side)
		return -1;

	// Check for impact on this node.
	msurface_t* surf = &r_worldmodel->surfaces[node->firstsurface];
	for (int i = 0; i < node->numsurfaces; i++, surf++)
	{
		if (surf->samples == NULL)
			continue; // No lightmap data. Was 'return 0' in original logic --mxd.

		if (surf->flags & (SURF_DRAWTURB | SURF_DRAWSKY) || surf->texinfo->flags & SURF_NODRAW) //mxd. Also skip NODRAW surfaces.
			continue; // No lightmaps.

		const mtexinfo_t* tex = surf->texinfo;

		const int s = (int)(DotProduct(mid, tex->vecs[0]) + tex->vecs[0][3]);
		const int t = (int)(DotProduct(mid, tex->vecs[1]) + tex->vecs[1][3]);

		if (s < surf->texturemins[0] || t < surf->texturemins[1])
			continue;

		const int ds = s - surf->texturemins[0];
		const int dt = t - surf->texturemins[1];

		if (ds > surf->extents[0] || dt > surf->extents[1])
			continue;

		VectorClear(pointcolor);

		const byte* lightmap = surf->samples + ((dt >> 4) * ((surf->extents[0] >> 4) + 1) + (ds >> 4)) * 3;
		for (int maps = 0; maps < MAXLIGHTMAPS && surf->styles[maps] != 255; maps++)
		{
			for (int c = 0; c < 3; c++)
			{
				const float scale = gl_modulate->value * r_newrefdef.lightstyles[surf->styles[maps]].rgb[c];
				pointcolor[c] += (float)lightmap[c] * scale / 255.0f;
			}

			lightmap += ((surf->extents[0] >> 4) + 1) * ((surf->extents[1] >> 4) + 1) * 3;
		}

		return 1;
	}

	// Go down back side.
	return R_RecursiveLightPoint(node->children[!side], mid, end);
}

void R_LightPoint(const vec3_t p, vec3_t color)
{
	if (r_worldmodel->lightdata == NULL)
	{
		VectorSet(color, 1.0f, 1.0f, 1.0f);
		return;
	}

	const vec3_t end = { p[0], p[1], p[2] - 3072.0f }; // Q2: p[2] - 2048
	const int r = R_RecursiveLightPoint(r_worldmodel->nodes, p, end);

	if (r == -1)
		VectorSet(color, 0.25f, 0.25f, 0.25f); // Q2: VectorCopy(vec3_origin, color)
	else
		VectorCopy(pointcolor, color);

	// Add dynamic lights.
	dlight_t* dl = &r_newrefdef.dlights[0];
	vec3_t dl_color = { 0 }; //mxd
	for (int lnum = 0; lnum < r_newrefdef.num_dlights; lnum++, dl++)
	{
		vec3_t dist;
		VectorSubtract(p, dl->origin, dist); //mxd. Original logic uses 'currententity->origin' instead of 'p' here.
		const float add = (dl->intensity - VectorLength(dist)) / 256.0f;

		if (add > 0.0f)
			for (int i = 0; i < 3; i++)
				dl_color[i] += (float)dl->color.c_array[i] / 255.0f * add;
	}

	Vec3ScaleAssign(gl_modulate->value, dl_color); //mxd. Original logic scales 'color' var here (which is already scaled by gl_modulate in R_RecursiveLightPoint()).
	Vec3AddAssign(dl_color, color);
}

#pragma endregion

// Q2 counterpart (except for dlight color handling).
static void R_AddDynamicLights(const msurface_t* surf)
{
	const int smax = (surf->extents[0] >> 4) + 1;
	const int tmax = (surf->extents[1] >> 4) + 1;
	const mtexinfo_t* tex = surf->texinfo;

	for (int lnum = 0; lnum < r_newrefdef.num_dlights; lnum++)
	{
		if (!(surf->dlightbits & (1 << lnum)))
			continue; // Not lit by this light.

		const dlight_t* dl = &r_newrefdef.dlights[lnum];
		float fdist = DotProduct(dl->origin, surf->plane->normal) - surf->plane->dist;
		const float frad = dl->intensity - fabsf(fdist);
		// Rad is now the highest intensity on the plane.

		float fminlight = DLIGHT_CUTOFF; //TODO: make configurable?
		if (frad < fminlight)
			continue;

		fminlight = frad - fminlight;

		vec3_t impact;
		for (int i = 0; i < 3; i++)
			impact[i] = dl->origin[i] - surf->plane->normal[i] * fdist;

		const int local_0 = (int)(DotProduct(impact, tex->vecs[0]) + tex->vecs[0][3] - (float)surf->texturemins[0]);
		const int local_1 = (int)(DotProduct(impact, tex->vecs[1]) + tex->vecs[1][3] - (float)surf->texturemins[1]);

		float* pfBL = &s_blocklights[0];
		int ftacc = 0;
		for (int t = 0; t < tmax; t++, ftacc += 16)
		{
			const int td = abs(local_1 - ftacc);

			int fsacc = 0;
			for (int s = 0; s < smax; s++, fsacc += 16, pfBL += 3)
			{
				const int sd = abs(local_0 - fsacc);

				if (sd > td)
					fdist = (float)(sd + (td >> 1));
				else
					fdist = (float)(td + (sd >> 1));

				if (fdist < fminlight)
				{
					pfBL[0] += (frad - fdist) * ((float)dl->color.r * gl_modulate->value / 255.0f); // H2: different color handling. //mxd. Multiply by gl_modulate.
					pfBL[1] += (frad - fdist) * ((float)dl->color.g * gl_modulate->value / 255.0f);
					pfBL[2] += (frad - fdist) * ((float)dl->color.b * gl_modulate->value / 255.0f);
				}
			}
		}
	}
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
			int r = (int)bl[0];
			int g = (int)bl[1];
			int b = (int)bl[2];

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