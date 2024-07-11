//
// gl_light.c
//
// Copyright 1998 Raven Software
//

#include "gl_local.h"
#include "Vector.h"

static int r_dlightframecount; //mxd. Made static
static float s_blocklights[34 * 34 * 3];

#define DLIGHT_CUTOFF	64

#pragma region ========================== DYNAMIC LIGHTS RENDERING ==========================

// Q2 counterpart (except for dlight color handling).
static void R_RenderDlight(const dlight_t* light)
{
	vec3_t v;

	const float rad = light->intensity * 0.35f;
	VectorSubtract(light->origin, r_origin, v);

	qglBegin(GL_TRIANGLE_FAN);

	qglColor3f((float)light->color.r / 255.0f * 0.2f, (float)light->color.g / 255.0f * 0.2f, (float)light->color.b / 255.0f * 0.2f);
	for (int i = 0; i < 3; i++)
		v[i] = light->origin[i] - vpn[i] * rad;
	qglVertex3fv(v);

	qglColor3f(0.0f, 0.0f, 0.0f);
	for (int i = 16; i >= 0; i--)
	{
		const float a = (float)i / 16.0f * ANGLE_360;

		for (int j = 0; j < 3; j++)
			v[j] = light->origin[j] + vright[j] * cosf(a) * rad + vup[j] * sinf(a) * rad;

		qglVertex3fv(v);
	}

	qglEnd();
}

void R_RenderDlights(void)
{
	int i;
	dlight_t* l;

	if ((int)gl_flashblend->value) //TODO: H2 check is opposite to Q2 check! A bug?
		return;

	r_dlightframecount = r_framecount + 1; // Because the count hasn't advanced yet for this frame

	qglDepthMask(GL_FALSE);
	qglDisable(GL_TEXTURE_2D);
	qglShadeModel(GL_SMOOTH);
	qglEnable(GL_BLEND);
	qglBlendFunc(GL_ONE, GL_ONE);

	for (i = 0, l = r_newrefdef.dlights; i < r_newrefdef.num_dlights; i++, l++)
		R_RenderDlight(l);

	qglColor3f(1.0f, 1.0f, 1.0f);
	qglDisable(GL_BLEND);
	qglEnable(GL_TEXTURE_2D);
	qglBlendFunc(GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR); // Q2: GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA
	qglDepthMask(GL_TRUE);
}

#pragma endregion

#pragma region ========================== DYNAMIC LIGHTS ==========================

// Q2 counterpart
void R_MarkLights(dlight_t* light, const int bit, const mnode_t* node)
{
	int i;
	msurface_t* surf;

	if (node->contents != -1)
		return;

	cplane_t* splitplane = node->plane;
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

	// Mark the polygons
	for (i = 0, surf = r_worldmodel->surfaces + node->firstsurface; i < node->numsurfaces; i++, surf++)
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
	int i;
	dlight_t* l;

	if (!(int)gl_flashblend->value)
	{
		r_dlightframecount = r_framecount + 1; // Because the count hasn't advanced yet for this frame

		for (i = 0, l = r_newrefdef.dlights; i < r_newrefdef.num_dlights; i++, l++)
			R_MarkLights(l, 1 << i, r_worldmodel->nodes);
	}
}

#pragma endregion

#pragma region ========================== LIGHT SAMPLING ==========================

vec3_t pointcolor;
cplane_t* lightplane; // Used as shadow plane
vec3_t lightspot;

static int RecursiveLightPoint(const mnode_t* node, vec3_t start, vec3_t end)
{
	int i;
	msurface_t* surf;
	vec3_t mid;
	
	// Didn't hit anything
	if (node->contents != -1)
		return -1;

	// Calculate mid point
	cplane_t* plane = node->plane;
	const float front = DotProduct(start, plane->normal) - plane->dist;
	const float back = DotProduct(end, plane->normal) - plane->dist;
	const int side = front < 0.0f;

	if ((back < 0.0f) == side)
		return RecursiveLightPoint(node->children[side], start, end);

	float frac = front / (front - back);

	for (int c = 0; c < 3; c++)
		mid[c] = start[c] + (end[c] - start[c]) * frac;

	// Go down front side
	const int r = RecursiveLightPoint(node->children[side], start, mid);

	// Hit something
	if (r >= 0)
		return r;

	// Didn't hit anything
	if ((back < 0.0f) == side)
		return -1;

	// Check for impact on this node
	VectorCopy(mid, lightspot);
	lightplane = plane;

	for (i = 0, surf = r_worldmodel->surfaces + node->firstsurface; i < node->numsurfaces; i++, surf++)
	{
		if (surf->flags & (SURF_DRAWTURB | SURF_DRAWSKY))
			continue; // No lightmaps

		mtexinfo_t* tex = surf->texinfo;

		const int s = (int)(DotProduct(mid, tex->vecs[0]) + tex->vecs[0][3]);
		const int t = (int)(DotProduct(mid, tex->vecs[1]) + tex->vecs[1][3]);

		if (s < surf->texturemins[0] || t < surf->texturemins[1])
			continue;

		const int ds = s - surf->texturemins[0];
		const int dt = t - surf->texturemins[1];

		if (ds > surf->extents[0] || dt > surf->extents[1])
			continue;

		if (surf->samples == NULL)
			return 0;

		VectorCopy(vec3_origin, pointcolor);

		byte* lightmap = surf->samples + ((dt >> 4) * ((surf->extents[0] >> 4) + 1) + (ds >> 4)) * 3;
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

	// Go down back side
	return RecursiveLightPoint(node->children[!side], mid, end);
}

void R_LightPoint(vec3_t p, vec3_t color)
{
	int lnum;
	dlight_t* dl;
	vec3_t end;

	if (r_worldmodel->lightdata == NULL)
	{
		VectorSet(color, 1.0f, 1.0f, 1.0f);
		return;
	}

	VectorSet(end, p[0], p[1], p[2] - 3072.0f); // Q2: p[2] - 2048
	const int r = RecursiveLightPoint(r_worldmodel->nodes, p, end);

	if (r == -1)
		VectorSet(color, 0.25f, 0.25f, 0.25f); // Q2: VectorCopy(vec3_origin, color)
	else
		VectorCopy(pointcolor, color);

	// Add dynamic lights
	for (lnum = 0, dl = r_newrefdef.dlights; lnum < r_newrefdef.num_dlights; lnum++, dl++)
	{
		vec3_t dist;
		VectorSubtract(currententity->origin, dl->origin, dist);
		const float add = (dl->intensity - VectorLength(dist)) / 256.0f;

		if (add > 0.0f)
		{
			for (int i = 0; i < 3; i++)
				color[i] += (float)dl->color.c_array[i] / 255.0f * add;
		}
	}

	VectorScale(color, gl_modulate->value, color);
}

#pragma endregion

// Q2 counterpart (except for dlight color handling).
static void R_AddDynamicLights(const msurface_t* surf)
{
	vec3_t impact;
	vec3_t local;
	int s;
	int t;
	int fsacc;
	int ftacc;

	int smax = (surf->extents[0] >> 4) + 1;
	int tmax = (surf->extents[1] >> 4) + 1;
	const mtexinfo_t* tex = surf->texinfo;

	for (int lnum = 0; lnum < r_newrefdef.num_dlights; lnum++)
	{
		if (!(surf->dlightbits & (1 << lnum)))
			continue; // Not lit by this light

		const dlight_t* dl = &r_newrefdef.dlights[lnum];
		float fdist = DotProduct(dl->origin, surf->plane->normal) - surf->plane->dist;
		float frad = dl->intensity - fabsf(fdist);
		// Rad is now the highest intensity on the plane

		float fminlight = DLIGHT_CUTOFF; // FIXME: make configurable?
		if (frad < fminlight)
			continue;

		fminlight = frad - fminlight;

		for (int i = 0; i < 3; i++)
			impact[i] = dl->origin[i] - surf->plane->normal[i] * fdist;

		local[0] = DotProduct(impact, tex->vecs[0]) + tex->vecs[0][3] - (float)surf->texturemins[0];
		local[1] = DotProduct(impact, tex->vecs[1]) + tex->vecs[1][3] - (float)surf->texturemins[1];

		float* pfBL = s_blocklights;
		for (t = 0, ftacc = 0; t < tmax; t++, ftacc += 16)
		{
			const int td = abs((int)local[1] - ftacc);

			for (s = 0, fsacc = 0; s < smax; s++, fsacc += 16, pfBL += 3)
			{
				const int sd = abs((int)local[0] - fsacc);

				if (sd > td)
					fdist = (float)(sd + (td >> 1));
				else
					fdist = (float)(td + (sd >> 1));

				if (fdist < fminlight)
				{
					pfBL[0] += (frad - fdist) * ((float)dl->color.r / 255.0f); // H2: different color handling
					pfBL[1] += (frad - fdist) * ((float)dl->color.g / 255.0f);
					pfBL[2] += (frad - fdist) * ((float)dl->color.b / 255.0f);
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

// Q2 counterpart (in H2, except for extra SURF_TALL_WALL flag)
void R_BuildLightMap(msurface_t* surf, byte* dest, int stride)
{
	if (surf->texinfo->flags & SURF_FULLBRIGHT) //mxd. SURF_FULLBRIGHT define
		ri.Sys_Error(ERR_DROP, "R_BuildLightMap called for non-lit surface");

	const int smax = (surf->extents[0] >> 4) + 1;
	const int tmax = (surf->extents[1] >> 4) + 1;
	const int size = smax * tmax;

	if (size > (int)sizeof(s_blocklights) >> 4)
		ri.Sys_Error(ERR_DROP, "Bad s_blocklights size");

	// Set to full bright if no light data
	if (surf->samples == NULL)
	{
		for (int i = 0; i < size * 3; i++)
			s_blocklights[i] = 255;
	}
	else
	{
		//mxd. Don't count the number of maps
		const byte* lightmap = surf->samples;

		// Add all the lightmaps //mxd. Skip nummaps == 1 logic
		memset(s_blocklights, 0, sizeof(s_blocklights[0]) * size * 3);

		for (int maps = 0; maps < MAXLIGHTMAPS && surf->styles[maps] != 255; maps++)
		{
			float scale[3];
			for (int i = 0; i < 3; i++)
				scale[i] = gl_modulate->value * r_newrefdef.lightstyles[surf->styles[maps]].rgb[i];

			//mxd. Skip scale == 1.0 logic
			float* bl = s_blocklights;
			for (int i = 0; i < size; i++, bl += 3)
				for (int j = 0; j < 3; j++)
					bl[j] += (float)lightmap[i * 3 + j] * scale[j];

			// Skip to next lightmap
			lightmap += size * 3;
		}

		// Add all the dynamic lights
		if (surf->dlightframe == r_framecount)
			R_AddDynamicLights(surf);
	}

	// Put into texture format
	stride -= (smax << 2);
	const float* bl = s_blocklights;

	//mxd. Skip gl_monolightmap logic.
	for (int i = 0; i < tmax; i++, dest += stride)
	{
		for (int j = 0; j < smax; j++, bl += 3, dest += 4)
		{
			int r = Q_ftol(bl[0]);
			int g = Q_ftol(bl[1]);
			int b = Q_ftol(bl[2]);

			// Catch negative lights
			r = max(r, 0);
			g = max(g, 0);
			b = max(b, 0);

			// Determine the brightest of the three color components
			const int max = max(r, max(g, b));

			// Alpha is ONLY used for the mono lightmap case.
			// For this reason we set it to the brightest of the color components so that things don't get too dim.
			int a = max;

			// Rescale all the color components if the intensity of the greatest channel exceeds 1.0
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