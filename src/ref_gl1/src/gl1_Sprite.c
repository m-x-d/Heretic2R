//
// gl1_Sprite.c -- sprite rendering
//
// Copyright 1998 Raven Software
//

#include "gl1_Sprite.h"
#include "gl1_Image.h"
#include "gl1_Misc.h"
#include "q_Sprite.h"
#include "Vector.h"

// Standard square sprite.
static void R_DrawStandardSprite(const entity_t* e, const dsprframe_t* frame, const vec3_t up, const vec3_t right)
{
	vec3_t point;

	const float xl = (float)-frame->origin_x * e->scale;
	const float xr = (float)(frame->width - frame->origin_x) * e->scale;
	const float yt = (float)-frame->origin_y * e->scale;
	const float yb = (float)(frame->height - frame->origin_y) * e->scale;

	glBegin(GL_QUADS);

	glTexCoord2f(0.0f, 1.0f);
	VectorMA(e->origin, yt, up, point);
	VectorMA(point, xl, right, point);
	glVertex3fv(point);

	glTexCoord2f(0.0f, 0.0f);
	VectorMA(e->origin, yb, up, point);
	VectorMA(point, xl, right, point);
	glVertex3fv(point);

	glTexCoord2f(1.0f, 0.0f);
	VectorMA(e->origin, yb, up, point);
	VectorMA(point, xr, right, point);
	glVertex3fv(point);

	glTexCoord2f(1.0f, 1.0f);
	VectorMA(e->origin, yt, up, point);
	VectorMA(point, xr, right, point);
	glVertex3fv(point);

	glEnd();
}

// Sprite with 4 variable verts(x, y scale and s, t); texture must be square.
static void R_DrawDynamicSprite(const entity_t* e, const vec3_t up, const vec3_t right)
{
	vec3_t point;

	glBegin(GL_QUADS);

	glTexCoord2f(e->verts[0].s, e->verts[0].t);
	VectorMA(e->origin, e->scale * e->verts[0].y, up, point);
	VectorMA(point, e->scale * e->verts[0].x, right, point);
	glVertex3fv(point);

	glTexCoord2f(e->verts[1].s, e->verts[1].t);
	VectorMA(e->origin, e->scale * e->verts[1].y, up, point);
	VectorMA(point, e->scale * e->verts[1].x, right, point);
	glVertex3fv(point);

	glTexCoord2f(e->verts[2].s, e->verts[2].t);
	VectorMA(e->origin, e->scale * e->verts[2].y, up, point);
	VectorMA(point, e->scale * e->verts[2].x, right, point);
	glVertex3fv(point);

	glTexCoord2f(e->verts[3].s, e->verts[3].t);
	VectorMA(e->origin, e->scale * e->verts[3].y, up, point);
	VectorMA(point, e->scale * e->verts[3].x, right, point);
	glVertex3fv(point);

	glEnd();
}

// Sprite with n variable verts(x, y scale and s, t); texture must be square. //TODO: seems unused, so can't test if it works correctly...
static void R_DrawVariableSprite(const entity_t* e, const vec3_t up, const vec3_t right)
{
	vec3_t point;

	glBegin(GL_POLYGON);

	svertex_t* v = &e->verts_p[0];
	for (int i = 0; i < e->numVerts; i++, v++)
	{
		glTexCoord2f(v->s, v->t);
		VectorMA(e->origin, e->scale * v->y, up, point);
		VectorMA(point, e->scale * v->x, right, point);
		glVertex3fv(point);
	}

	glEnd();
}

// Long linear semi-oriented sprite with two verts (xyz start and end) and a width.
static void R_DrawLineSprite(const entity_t* e, const vec3_t up)
{
	vec3_t diff;
	VectorSubtract(e->endpos, e->startpos, diff);

	vec3_t dir;
	CrossProduct(diff, up, dir);
	VectorNormalize(dir);

	vec3_t start_offset;
	VectorScale(dir, e->scale * 0.5f, start_offset);

	vec3_t end_offset;
	VectorScale(dir, e->scale2 * 0.5f, end_offset);

	const float tile = (e->tile > 0.0f ? e->tile : 1.0f);

	vec3_t point;

	glBegin(GL_QUADS);

	glTexCoord2f(0.0f, e->tileoffset);
	VectorSubtract(e->startpos, start_offset, point);
	glVertex3fv(point);

	glTexCoord2f(1.0f, e->tileoffset);
	VectorAdd(e->startpos, start_offset, point);
	glVertex3fv(point);

	glTexCoord2f(1.0f, e->tileoffset + tile);
	VectorAdd(e->endpos, end_offset, point);
	glVertex3fv(point);

	glTexCoord2f(0.0f, e->tileoffset + tile);
	VectorSubtract(e->endpos, end_offset, point);
	glVertex3fv(point);

	glEnd();
}

void R_DrawSpriteModel(entity_t* e)
{
	const model_t* mdl = *e->model; //mxd. Original logic uses 'currentmodel' global var instead.

	// Don't even bother culling, because it's just a single polygon without a surface cache.
	const dsprite_t* psprite = mdl->extradata;

	//mxd. #if 0-ed in Q2
	if (e->frame < 0 || e->frame >= psprite->numframes)
	{
		ri.Con_Printf(PRINT_DEVELOPER, "R_DrawSpriteModel: sprite '%s' with invalid frame %i!\n", mdl->name, e->frame);
		e->frame = 0;
	}

	e->frame %= psprite->numframes;

	if (mdl->skins[e->frame] == NULL) //mxd. Moved below e->frame sanity checks.
		return;

	// All-new logic from here and down!!!
	glShadeModel(GL_SMOOTH);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); //mxd. qglTexEnvf -> qglTexEnvi

	R_HandleTransparency(e);
	R_BindImage(mdl->skins[e->frame]);

	if (e->flags & RF_NODEPTHTEST)
		glDisable(GL_DEPTH_TEST);

	vec3_t up;
	vec3_t right;

	if (e->flags & RF_FIXED)
	{
		vec3_t dir;
		DirAndUpFromAngles(e->angles, dir, up); //mxd. Original logic uses 'currententity' global var instead.

		CrossProduct(up, dir, right);
		VectorNormalize(right);
	}
	else
	{
		VectorCopy(vup, up);
		VectorCopy(vright, right);
	}

	switch (e->spriteType)
	{
		case SPRITE_EDICT:
		case SPRITE_STANDARD:
			R_DrawStandardSprite(e, &psprite->frames[e->frame], up, right);
			break;

		case SPRITE_DYNAMIC:
			R_DrawDynamicSprite(e, up, right);
			break;

		case SPRITE_VARIABLE:
			R_DrawVariableSprite(e, up, right);
			break;

		case SPRITE_LINE:
			R_DrawLineSprite(e, vpn);
			break;

		default: //mxd. Avoid compiler warnings...
			ri.Sys_Error(ERR_DROP, "R_DrawSpriteModel: unknown sprite type (%i)!", e->spriteType); //mxd. Sys_Error() -> ri.Sys_Error().
			break;
	}

	if (e->flags & RF_NODEPTHTEST)
		glEnable(GL_DEPTH_TEST);

	R_CleanupTransparency(e);

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE); //mxd. qglTexEnvf -> qglTexEnvi
	glShadeModel(GL_FLAT);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}