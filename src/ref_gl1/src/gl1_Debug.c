//
// gl1_Debug.c
//
// Copyright 1998 Raven Software
//

#include "gl1_Debug.h"
#include "gl1_Local.h"
#include "game.h"
#include "Vector.h"

typedef enum
{
	DPT_NONE,
	DPT_LINE,
	DPT_ARROW,
	DPT_MARKER,
	DPT_BOX,
	DPT_BBOX,
	DPT_ENTITY_BBOX,
} DebugPrimitiveType_e;

typedef struct DebugObjectInfo_s
{
	DebugPrimitiveType_e type;
	vec3_t verts[8];
	const struct edict_s* ent;
	paletteRGBA_t color;
	float lifetime;
} DebugPrimitive_t;

#define MAX_DEBUG_PRIMITIVES	512
static DebugPrimitive_t dbg_primitives[MAX_DEBUG_PRIMITIVES];

static void SetDebugBoxVerts(DebugPrimitive_t* box, const vec3_t mins, const vec3_t maxs)
{
	// Init top verts...
	VectorSet(box->verts[0], mins[0], mins[1], maxs[2]);
	VectorSet(box->verts[1], mins[0], maxs[1], maxs[2]);
	VectorSet(box->verts[2], maxs[0], maxs[1], maxs[2]);
	VectorSet(box->verts[3], maxs[0], mins[1], maxs[2]);

	// Init bottom verts...
	VectorSet(box->verts[4], mins[0], mins[1], mins[2]);
	VectorSet(box->verts[5], mins[0], maxs[1], mins[2]);
	VectorSet(box->verts[6], maxs[0], maxs[1], mins[2]);
	VectorSet(box->verts[7], maxs[0], mins[1], mins[2]);
}

static DebugPrimitive_t* InitDebugPrimitive(const vec3_t mins, const vec3_t maxs, const paletteRGBA_t color, const float lifetime, const DebugPrimitiveType_e type)
{
	// Find free slot.
	DebugPrimitive_t* p = NULL;

	for (int i = 0; i < MAX_DEBUG_PRIMITIVES; i++)
	{
		p = &dbg_primitives[i];

		if (p->type == DPT_NONE || (p->lifetime != -1.0f && p->lifetime < r_newrefdef.time))
			break;
	}

	// Init primitive.
	if (p == NULL)
		return NULL;

	p->type = type;
	p->color = color;
	p->ent = NULL;

	if (lifetime == -1.0f)
		p->lifetime = lifetime;
	else
		p->lifetime = r_newrefdef.time + max(lifetime, 0.025f); // Convert to absolute time, make sure it lives for at least a frame...

	if (type == DPT_BOX || type == DPT_BBOX || type == DPT_ENTITY_BBOX)
		SetDebugBoxVerts(p, mins, maxs);

	return p;
}

void RI_AddDebugBox(const vec3_t center, float size, const paletteRGBA_t color, const float lifetime)
{
	size *= 0.5f;
	const vec3_t mins = { center[0] - size, center[1] - size, center[2] - size };
	const vec3_t maxs = { center[0] + size, center[1] + size, center[2] + size };

	// Find free slot...
	const DebugPrimitive_t* box = InitDebugPrimitive(mins, maxs, color, lifetime, DPT_BOX);

	if (box == NULL)
		ri.Con_Printf(PRINT_DEVELOPER, "RI_AddDebugBox: failed to add box at [%f %f %f]...", center[0], center[1], center[2]);
}

void RI_AddDebugBbox(const vec3_t mins, const vec3_t maxs, const paletteRGBA_t color, const float lifetime)
{
	// Find free slot...
	const DebugPrimitive_t* box = InitDebugPrimitive(mins, maxs, color, lifetime, DPT_BBOX);

	if (box == NULL)
	{
		vec3_t center;
		VectorAverage(mins, maxs, center);
		ri.Con_Printf(PRINT_DEVELOPER, "RI_AddDebugBbox: failed to add bbox at [%f %f %f]...", center[0], center[1], center[2]);
	}
}

void RI_AddDebugEntityBbox(const struct edict_s* ent, const paletteRGBA_t color)
{
	if (ent == NULL)
		return;

	// Find free slot...
	DebugPrimitive_t* box = InitDebugPrimitive(ent->mins, ent->maxs, color, -1.0f, DPT_ENTITY_BBOX);

	if (box != NULL)
	{
		box->ent = ent;
	}
	else
	{
		vec3_t center;
		VectorAverage(ent->mins, ent->maxs, center);
		ri.Con_Printf(PRINT_DEVELOPER, "RI_AddDebugEntityBbox: failed to add entity bbox at [%f %f %f]...", center[0], center[1], center[2]);
	}
}

void RI_AddDebugLine(const vec3_t start, const vec3_t end, const paletteRGBA_t color, const float lifetime)
{
	// Find free slot...
	DebugPrimitive_t* line = InitDebugPrimitive(start, end, color, lifetime, DPT_LINE);

	if (line != NULL)
	{
		VectorCopy(start, line->verts[0]);
		VectorCopy(end, line->verts[1]);
	}
	else
	{
		ri.Con_Printf(PRINT_DEVELOPER, "RI_AddDebugLine: failed to add line at [%f %f %f] -> [%f %f %f]...", start[0], start[1], start[2], end[0], end[1], end[2]);
	}
}

void RI_AddDebugArrow(const vec3_t start, const vec3_t end, const paletteRGBA_t color, const float lifetime)
{
#define ARROWHEAD_SIZE	16.0f

	// Find free slot...
	DebugPrimitive_t* arrow = InitDebugPrimitive(start, end, color, lifetime, DPT_ARROW);

	if (arrow != NULL)
	{
		VectorCopy(start, arrow->verts[0]); // Start.
		VectorCopy(end, arrow->verts[1]); // End.

		// Arrowhead.
		vec3_t dir;
		VectorSubtract(end, start, dir);
		float len = VectorNormalize(dir) * 0.5f;
		len = min(len, ARROWHEAD_SIZE);

		vec3_t arrowhead_dir;
		VectorGetOffsetOrigin(dir, vec3_origin, 30.0f, arrowhead_dir);
		VectorNormalize(arrowhead_dir);

		float angle = 45.0f;
		for (int i = 0; i < 4; i++)
		{
			vec3_t v;
			RotatePointAroundVector(v, dir, arrowhead_dir, angle);
			Vec3ScaleAssign(len, v);
			VectorSubtract(end, v, arrow->verts[i + 2]);

			angle += 90.0f;
		}
	}
	else
	{
		ri.Con_Printf(PRINT_DEVELOPER, "RI_AddDebugArrow: failed to add arrow at [%f %f %f] -> [%f %f %f]...", start[0], start[1], start[2], end[0], end[1], end[2]);
	}
}

void RI_AddDebugMarker(const vec3_t center, const float size, const paletteRGBA_t color, const float lifetime)
{
	// Find free slot...
	DebugPrimitive_t* marker = InitDebugPrimitive(NULL, NULL, color, lifetime, DPT_MARKER);

	if (marker != NULL)
	{
		const float hs = size * 0.5f;

		VectorSet(marker->verts[0], center[0] + hs, center[1], center[2]);
		VectorSet(marker->verts[1], center[0] - hs, center[1], center[2]);

		VectorSet(marker->verts[2], center[0], center[1] + hs, center[2]);
		VectorSet(marker->verts[3], center[0], center[1] - hs, center[2]);

		VectorSet(marker->verts[4], center[0], center[1], center[2] + hs);
		VectorSet(marker->verts[5], center[0], center[1], center[2] - hs);
	}
	else
	{
		ri.Con_Printf(PRINT_DEVELOPER, "RI_AddDebugMarker: failed to add marker at [%f %f %f]...", center[0], center[1], center[2]);
	}
}

static void DrawDebugBox(const DebugPrimitive_t* box)
{
	glColor3ub(box->color.r, box->color.g, box->color.b);

	// Top...
	glBegin(GL_LINE_LOOP);
	glVertex3fv(box->verts[0]);
	glVertex3fv(box->verts[1]);
	glVertex3fv(box->verts[2]);
	glVertex3fv(box->verts[3]);
	glEnd();

	// Bottom...
	glBegin(GL_LINE_LOOP);
	glVertex3fv(box->verts[4]);
	glVertex3fv(box->verts[5]);
	glVertex3fv(box->verts[6]);
	glVertex3fv(box->verts[7]);
	glEnd();

	// Sides...
	glBegin(GL_LINES);
	for (int i = 0; i < 4; i++)
	{
		glVertex3fv(box->verts[i]);
		glVertex3fv(box->verts[i + 4]);
	}
	glEnd();
}

static void DrawDebugLine(const DebugPrimitive_t* line)
{
	glColor3ub(line->color.r, line->color.g, line->color.b);

	glBegin(GL_LINES);
	glVertex3fv(line->verts[0]);
	glVertex3fv(line->verts[1]);
	glEnd();
}

static void DrawDebugArrow(const DebugPrimitive_t* arrow)
{
	glColor3ub(arrow->color.r, arrow->color.g, arrow->color.b);

	glBegin(GL_LINES);
	glVertex3fv(arrow->verts[0]); // Start.
	glVertex3fv(arrow->verts[1]); // End.

	// Arrowhead.
	for (int i = 2; i < 6; i++)
	{
		glVertex3fv(arrow->verts[i]);
		glVertex3fv(arrow->verts[1]);
	}

	glEnd();
}

static void DrawDebugMarker(const DebugPrimitive_t* marker)
{
	glColor3ub(marker->color.r, marker->color.g, marker->color.b);

	glBegin(GL_LINES);
	for (int i = 0; i < 6; i += 2)
	{
		glVertex3fv(marker->verts[i + 0]);
		glVertex3fv(marker->verts[i + 1]);
	}
	glEnd();
}

// Draw all debug primitives.
void R_DrawDebugPrimitives(void)
{
	vec3_t mins;
	vec3_t maxs;

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);

	// Draw primitives...
	DebugPrimitive_t* p = &dbg_primitives[0];
	for (int i = 0; i < MAX_DEBUG_PRIMITIVES; i++, p++)
	{
		// Check for expired primitives (except for DPT_ENTITY_BBOX, those expire when associated entity expires).
		if (p->type != DPT_NONE && p->type != DPT_ENTITY_BBOX && p->lifetime > -1.0f && p->lifetime < r_newrefdef.time)
			p->type = DPT_NONE;

		if (p->type == DPT_NONE)
			continue;

		switch (p->type)
		{
			case DPT_ENTITY_BBOX:
				if (p->ent == NULL)
				{
					p->type = DPT_NONE;
					continue;
				}

				// Entity position could change.
				VectorAdd(p->ent->s.origin, p->ent->mins, mins);
				VectorAdd(p->ent->s.origin, p->ent->maxs, maxs);

				SetDebugBoxVerts(p, mins, maxs);
				DrawDebugBox(p);
				break;

			case DPT_BOX:
			case DPT_BBOX:
				DrawDebugBox(p);
				break;

			case DPT_LINE:
				DrawDebugLine(p);
				break;

			case DPT_ARROW:
				DrawDebugArrow(p);
				break;

			case DPT_MARKER:
				DrawDebugMarker(p);
				break;

			default:
				break;
		}
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
}