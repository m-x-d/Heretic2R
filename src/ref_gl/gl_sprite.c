//
// gl_sprite.c -- sprite rendering
//
// Copyright 1998 Raven Software
//

#include "gl_local.h"
#include "q_Sprite.h"
#include "Vector.h"

// Standard square sprite
static void R_DrawStandardSprite(const entity_t* e, const dsprframe_t* frame, vec3_t up, vec3_t right)
{
	vec3_t point;

	const float xl = (float)-frame->origin_x * e->scale;
	const float xr = (float)(frame->width - frame->origin_x) * e->scale;
	const float yt = (float)-frame->origin_y * e->scale;
	const float yb = (float)(frame->height - frame->origin_y) * e->scale;

	qglBegin(GL_QUADS);

	qglTexCoord2f(0.0f, 1.0f);
	VectorMA(e->origin, yt, up, point);
	VectorMA(point, xl, right, point);
	qglVertex3fv(point);

	qglTexCoord2f(0.0f, 0.0f);
	VectorMA(e->origin, yb, up, point);
	VectorMA(point, xl, right, point);
	qglVertex3fv(point);

	qglTexCoord2f(1.0f, 0.0f);
	VectorMA(e->origin, yb, up, point);
	VectorMA(point, xr, right, point);
	qglVertex3fv(point);

	qglTexCoord2f(1.0f, 1.0f);
	VectorMA(e->origin, yt, up, point);
	VectorMA(point, xr, right, point);
	qglVertex3fv(point);

	qglEnd();
}

// Sprite with 4 variable verts(x, y scale and s, t); texture must be square.
static void R_DrawDynamicSprite(const entity_t* e, const dsprframe_t* frame, vec3_t up, vec3_t right)
{
	vec3_t point;

	qglBegin(GL_QUADS);

	qglTexCoord2f(e->verts[0][2], e->verts[0][3]);
	VectorMA(e->origin, e->scale * e->verts[0][1], up, point);
	VectorMA(point, e->scale * e->verts[0][0], right, point);
	qglVertex3fv(point);

	qglTexCoord2f(e->verts[1][2], e->verts[1][3]);
	VectorMA(e->origin, e->scale * e->verts[1][1], up, point);
	VectorMA(point, e->scale * e->verts[1][0], right, point);
	qglVertex3fv(point);

	qglTexCoord2f(e->verts[2][2], e->verts[2][3]);
	VectorMA(e->origin, e->scale * e->verts[2][1], up, point);
	VectorMA(point, e->scale * e->verts[2][0], right, point);
	qglVertex3fv(point);

	qglTexCoord2f(e->verts[3][2], e->verts[3][3]);
	VectorMA(e->origin, e->scale * e->verts[3][1], up, point);
	VectorMA(point, e->scale * e->verts[3][0], right, point);
	qglVertex3fv(point);

	qglEnd();
}

// Sprite with n variable verts(x, y scale and s, t); texture must be square. //TODO: seems unused, so can't test if it works correctly...
static void R_DrawVariableSprite(const entity_t* e, const dsprframe_t* frame, vec3_t up, vec3_t right)
{
	int i;
	float (*v)[4];
	vec3_t point;

	qglBegin(GL_POLYGON);

	for (i = 0, v = e->verts_p; i < e->numVerts; i++, v++)
	{
		qglTexCoord2f(*v[2], *v[3]);
		VectorMA(e->origin, e->scale * *v[1], up, point);
		VectorMA(point, e->scale * *v[0], right, point);
		qglVertex3fv(point);
	}

	qglEnd();
}

// Long linear semi-oriented sprite with two verts (xyz start and end) and a width.
static void R_DrawLineSprite(const entity_t* e, const dsprframe_t* frame, vec3_t up)
{
	vec3_t point;
	vec3_t start_offset;
	vec3_t end_offset;
	vec3_t dir;
	vec3_t diff;

	VectorSubtract(e->endpos, e->startpos, diff);
	CrossProduct(diff, up, dir);
	VectorNormalize(dir);

	VectorScale(dir, e->scale * 0.5f, start_offset);
	VectorScale(dir, e->scale2 * 0.5f, end_offset);

	const float tile = (e->tile > 0.0f ? e->tile : 1.0f);

	qglBegin(GL_QUADS);

	qglTexCoord2f(0.0f, e->tileoffset);
	VectorSubtract(e->startpos, start_offset, point);
	qglVertex3fv(point);

	qglTexCoord2f(1.0f, e->tileoffset);
	VectorAdd(e->startpos, start_offset, point);
	qglVertex3fv(point);

	qglTexCoord2f(1.0f, e->tileoffset + tile);
	VectorAdd(e->endpos, end_offset, point);
	qglVertex3fv(point);

	qglTexCoord2f(0.0f, e->tileoffset + tile);
	VectorSubtract(e->endpos, end_offset, point);
	qglVertex3fv(point);

	qglEnd();
}

void R_DrawSpriteModel(entity_t* e)
{
	float* up;
	float* right;

	if (currentmodel->skins[e->frame] == NULL)
		return;

	// Don't even bother culling, because it's just a single polygon without a surface cache
	const dsprite_t* psprite = currentmodel->extradata;

	//mxd. #if 0-ed in Q2
	if (e->frame < 0 || e->frame >= psprite->numframes)
	{
		ri.Con_Printf(PRINT_ALL, "no such sprite frame %i\n", e->frame);
		e->frame = 0;
	}

	e->frame %= psprite->numframes;
	const dsprframe_t* frame = &psprite->frames[e->frame];

	// All-new logic from here and down!!!
	qglShadeModel(GL_SMOOTH);
	qglTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); //mxd. qglTexEnvf -> qglTexEnvi

	HandleTrans(e);
	GL_BindImage(currentmodel->skins[e->frame]);

	if (e->flags & RF_NODEPTHTEST)
		qglDisable(GL_DEPTH_TEST);

	if (e->flags & RF_FIXED)
	{
		vec3_t dir;
		vec3_t fixed_up;
		DirAndUpFromAngles(currententity->angles, dir, fixed_up);

		vec3_t fixed_right;
		CrossProduct(fixed_up, dir, fixed_right);
		VectorNormalize(fixed_right);
		
		up = fixed_up;
		right = fixed_right;
	}
	else
	{
		up = vup;
		right = vright;
	}

	switch (e->spriteType)
	{
		case SPRITE_EDICT:
		case SPRITE_STANDARD:
			R_DrawStandardSprite(e, frame, up, right);
			break;

		case SPRITE_DYNAMIC:
			R_DrawDynamicSprite(e, frame, up, right);
			break;

		case SPRITE_VARIABLE:
			R_DrawVariableSprite(e, frame, up, right);
			break;

		case SPRITE_LINE:
			R_DrawLineSprite(e, frame, vpn);
			break;

		default: //mxd. Avoid compiler warnings...
			ri.Sys_Error(ERR_DROP, "R_DrawSpriteModel: unknown sprite type (%i)!", e->spriteType); //mxd. Sys_Error() -> ri.Sys_Error().
			break;
	}

	if (e->flags & RF_NODEPTHTEST)
		qglEnable(GL_DEPTH_TEST);

	CleanupTrans(e);

	qglTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE); //mxd. qglTexEnvf -> qglTexEnvi
	qglShadeModel(GL_FLAT);
	qglColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}