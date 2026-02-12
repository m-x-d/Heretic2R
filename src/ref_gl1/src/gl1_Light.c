//
// gl1_Light.c
//
// Copyright 1998 Raven Software
//

#include "gl1_Light.h"
#include "gl1_Matrix4.h"
#include "Vector.h"

#define DLIGHT_CUTOFF	64.0f

byte minlight[256]; // YQ2

static int r_dlightframecount; //mxd. Made static.
static float s_blocklights[34 * 34 * 3];

static vec3_t pointcolor;
static vec3_t lightspot; // DQII

typedef struct BmodelTransform_s //mxd
{
	matrix4_t matrix;
	qboolean updated;
} BmodelTransform_t;

static BmodelTransform_t r_bmodel_transforms[MAX_ENTITIES]; //mxd

#pragma region ========================== DYNAMIC LIGHTS RENDERING ==========================

// Q2 counterpart (except for dlight color handling).
static void R_RenderDlight(const dlight_t* light)
{
	vec3_t v;
	const float rad = light->intensity * 0.35f;

	glBegin(GL_TRIANGLE_FAN);

	glColor3f((float)light->color.r / 255.0f * 0.2f, (float)light->color.g / 255.0f * 0.2f, (float)light->color.b / 255.0f * 0.2f);
	for (int i = 0; i < 3; i++)
		v[i] = light->origin[i] - vpn[i] * rad;
	glVertex3fv(v);

	glColor3f(0.0f, 0.0f, 0.0f);
	for (int i = 16; i >= 0; i--)
	{
		const float a = (float)i / 16.0f * ANGLE_360;
		const float sin_a = sinf(a); //mxd. Avoid calculating the same value 3 times...
		const float cos_a = cosf(a); //mxd. Avoid calculating the same value 3 times...

		for (int j = 0; j < 3; j++)
			v[j] = light->origin[j] + vright[j] * cos_a * rad + vup[j] * sin_a * rad;

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

static void R_SetPointColor(const msurface_t* surf, const int ds, const int dt, vec3_t color) //mxd. KMQ2 interpolated lighting logic.
{
	VectorClear(color);

	int r00 = 0;
	int g00 = 0;
	int b00 = 0;
	int r01 = 0;
	int g01 = 0;
	int b01 = 0;
	int r10 = 0;
	int g10 = 0;
	int b10 = 0;
	int r11 = 0;
	int g11 = 0;
	int b11 = 0;

	const int dsfrac = ds & 15;
	const int dtfrac = dt & 15;
	const int light_smax = (surf->extents[0] >> 4) + 1;
	const int light_tmax = (surf->extents[1] >> 4) + 1;
	const int line3 = light_smax * 3;
	const byte* lightmap = surf->samples + ((dt >> 4) * light_smax + (ds >> 4)) * 3;

	for (int maps = 0; maps < MAXLIGHTMAPS && surf->styles[maps] != 255; maps++)
	{
		vec3_t scale;
		for (int c = 0; c < 3; c++)
			scale[c] = r_newrefdef.lightstyles[surf->styles[maps]].rgb[c];

		r00 += (int)((float)lightmap[0] * scale[0]);
		g00 += (int)((float)lightmap[1] * scale[1]);
		b00 += (int)((float)lightmap[2] * scale[2]);

		r01 += (int)((float)lightmap[3] * scale[0]);
		g01 += (int)((float)lightmap[4] * scale[1]);
		b01 += (int)((float)lightmap[5] * scale[2]);

		r10 += (int)((float)lightmap[line3 + 0] * scale[0]);
		g10 += (int)((float)lightmap[line3 + 1] * scale[1]);
		b10 += (int)((float)lightmap[line3 + 2] * scale[2]);

		r11 += (int)((float)lightmap[line3 + 3] * scale[0]);
		g11 += (int)((float)lightmap[line3 + 4] * scale[1]);
		b11 += (int)((float)lightmap[line3 + 5] * scale[2]);

		lightmap += light_smax * light_tmax * 3;
	}

	color[0] += (float)(((((((r11 - r10) * dsfrac >> 4) + r10) - (((r01 - r00) * dsfrac >> 4) + r00)) * dtfrac) >> 4) + (((r01 - r00) * dsfrac >> 4) + r00)) / 255.0f;
	color[1] += (float)(((((((g11 - g10) * dsfrac >> 4) + g10) - (((g01 - g00) * dsfrac >> 4) + g00)) * dtfrac) >> 4) + (((g01 - g00) * dsfrac >> 4) + g00)) / 255.0f;
	color[2] += (float)(((((((b11 - b10) * dsfrac >> 4) + b10) - (((b01 - b00) * dsfrac >> 4) + b00)) * dtfrac) >> 4) + (((b01 - b00) * dsfrac >> 4) + b00)) / 255.0f;

	Vec3ScaleAssign(gl_modulate->value, color);
}

void R_ResetBmodelTransforms(void) //mxd
{
	for (int i = 0; i < r_newrefdef.num_entities; i++)
		r_bmodel_transforms[i].updated = false;
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
	VectorLerp(start, frac, end, mid);

	// Go down front side.
	const int r = R_RecursiveLightPoint(node->children[side], start, mid);

	// Hit something.
	if (r >= 0)
		return r;

	// Didn't hit anything.
	if ((back < 0.0f) == side)
		return -1;

	VectorCopy(mid, lightspot); // DQII

	// Check for impact on this node.
	msurface_t* surf = &r_worldmodel->surfaces[node->firstsurface];
	for (int i = 0; i < node->numsurfaces; i++, surf++)
	{
		if (surf->samples == NULL)
			continue; // No lightmap data. Was 'return 0' in original logic --mxd.

		if (surf->flags & (SURF_DRAWTURB | SURF_DRAWSKY | SURF_SKIPDRAW)) //mxd. Also skip SURF_NODRAW surfaces.
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

		R_SetPointColor(surf, ds, dt, pointcolor); //mxd

		return 1;
	}

	// Go down back side.
	return R_RecursiveLightPoint(node->children[!side], mid, end);
}

void R_LightPoint(const vec3_t p, vec3_t color, const qboolean check_bmodels)
{
	if (r_worldmodel->lightdata == NULL)
	{
		VectorSet(color, 1.0f, 1.0f, 1.0f);
		return;
	}

	float dist_z;
	const vec3_t end = VEC3_INITA(p, 0.0f, 0.0f, -3072.0f); // Q2: p[2] - 2048
	const int r = R_RecursiveLightPoint(r_worldmodel->nodes, p, end);

	if (r == -1)
	{
		VectorSet(color, 0.25f, 0.25f, 0.25f); // Q2: VectorCopy(vec3_origin, color)
		dist_z = end[2]; // DQII
	}
	else
	{
		VectorCopy(pointcolor, color);
		dist_z = lightspot[2]; // DQII
	}

	//mxd. Ported DQII R_LightPoint logic (https://github.com/mhQuake/DirectQII/blob/4a2ae6383f74ae3deda327b19748f0924d212daf/DirectQII/r_light.c#L156).
	if (check_bmodels)
	{
		// Find bmodels under the lightpoint - move the point to bmodel space, trace down, then check.
		// If r < 0, it didn't find a bmodel, otherwise it did (a bmodel under a valid world hit will hit here too).
		for (int i = 0; i < r_newrefdef.num_entities; i++)
		{
			const entity_t* e = r_newrefdef.entities[i];

			if ((e->flags & RF_TRANSLUCENT) || e->model == NULL || *e->model == NULL || (*e->model)->type != mod_brush)
				continue;

			const model_t* mdl = *e->model;

			if (Vec3IsZero(e->origin))
			{
				//mxd. For non-rotating bmodels, check bbox.
				if (p[0] < mdl->mins[0] || p[0] > mdl->maxs[0] ||
					p[1] < mdl->mins[1] || p[1] > mdl->maxs[1] ||
					p[2] < mdl->mins[2] || end[2] > mdl->maxs[2])
				{
					continue;
				}
			}
			else
			{
				//mxd. For bmodels with defined origin, skip when not within model radius.
				if (p[0] < e->origin[0] - mdl->radius || p[0] > e->origin[0] + mdl->radius ||
					p[1] < e->origin[1] - mdl->radius || p[1] > e->origin[1] + mdl->radius ||
					p[2] < e->origin[2] - mdl->radius || end[2] > e->origin[2] + mdl->radius)
				{
					continue;
				}
			}

			//mxd. Lazily update bmodel transform...
			BmodelTransform_t* t = &r_bmodel_transforms[i];
			if (!t->updated)
			{
				R_MatrixIdentity(&t->matrix);
				R_MatrixTranslate(&t->matrix, e->origin);
				R_MatrixRotate(&t->matrix, e->angles);

				t->updated = true;
			}

			// Move start and end points into the entity's frame of reference.
			vec3_t e_start;
			R_VectorInverseTransform(&t->matrix, e_start, p);

			vec3_t e_end;
			R_VectorInverseTransform(&t->matrix, e_end, end);

			// And run the recursive light point on it too.
			if (R_RecursiveLightPoint(mdl->nodes + mdl->firstnode, e_start, e_end) == -1)
				continue;

			// A bmodel under a valid world hit will hit here too, so take the highest lightspot on all hits.
			vec3_t cur_spot;
			R_VectorTransform(&t->matrix, cur_spot, lightspot); // Move lightspot back to world space.

			if (cur_spot[2] > dist_z)
			{
				// Found a bmodel so copy it over.
				VectorCopy(pointcolor, color);
				dist_z = cur_spot[2];
			}
		}
	}

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

void R_InitMinlight(void) //mxd
{
	const float ml = Clamp(gl_minlight->value, 0.0f, 255.0f);
	gl_state.minlight_set = (ml != 0.0f);

	if (gl_state.minlight_set)
	{
		for (int i = 0; i < 256; i++)
		{
			const int inf = (int)((255.0f - ml) * (float)i / 255.0f + ml);
			minlight[i] = (byte)ClampI(inf, 0, 255);
		}
	}
	else
	{
		for (int i = 0; i < 256; i++)
			minlight[i] = (byte)i;
	}
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

			// Rescale all the color components if the intensity of the greatest channel exceeds 1.0.
			if (max > 255)
			{
				const float t = 255.0f / (float)max;

				r = (int)((float)r * t);
				g = (int)((float)g * t);
				b = (int)((float)b * t);
			}

			if (gl_state.minlight_set) // YQ2
			{
				r = minlight[r];
				g = minlight[g];
				b = minlight[b];
			}

			dest[0] = (byte)r;
			dest[1] = (byte)g;
			dest[2] = (byte)b;
			dest[3] = 255; //mxd. Alpha was ONLY used for the mono lightmap case.
		}
	}
}