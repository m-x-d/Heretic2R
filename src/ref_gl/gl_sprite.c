//
// gl_sprite.c -- sprite rendering
//
// Copyright 1998 Raven Software
//

#include "gl_local.h"
#include "q_Sprite.h"
#include "Vector.h"

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
	(*qglVertex3fv)(point);

	qglTexCoord2f(0.0f, 0.0f);
	VectorMA(e->origin, yb, up, point);
	VectorMA(point, xl, right, point);
	qglVertex3fv(point);

	qglTexCoord2f(1.0f, 0.0f);
	VectorMA(e->origin, yb, up, point);
	VectorMA(point, xr, right, point);
	(*qglVertex3fv)(point);

	qglTexCoord2f(1.0f, 1.0f);
	VectorMA(e->origin, yt, up, point);
	VectorMA(point, xr, right, point);
	qglVertex3fv(point);

	qglEnd();
}

static void R_DrawDynamicSprite(entity_t* e, dsprframe_t* frame, vec3_t up, vec3_t right)
{
	NOT_IMPLEMENTED
}

static void R_DrawVariableSprite(entity_t* e, dsprframe_t* frame, vec3_t up, vec3_t right)
{
	NOT_IMPLEMENTED
}

static void R_DrawLineSprite(entity_t* e, dsprframe_t* frame, vec3_t* up)
{
	NOT_IMPLEMENTED
}

void R_DrawSpriteModel(entity_t* e)
{
	vec3_t fixed_up;
	vec3_t fixed_right;
	vec3_t dir;
	float* up;
	float* right;

	if (currentmodel->skins[e->frame] == NULL)
		return;

	// Don't even bother culling, because it's just a single polygon without a surface cache
	dsprite_t* psprite = currentmodel->extradata;

	//mxd. #if 0-ed in Q2
	if (e->frame < 0 || e->frame >= psprite->numframes)
	{
		ri.Con_Printf(PRINT_ALL, "no such sprite frame %i\n", e->frame);
		e->frame = 0;
	}

	e->frame %= psprite->numframes;
	dsprframe_t* frame = &psprite->frames[e->frame];

	// All-new logic from here and down!!!
	qglShadeModel(GL_SMOOTH);
	qglTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); //mxd. qglTexEnvf -> qglTexEnvi

	HandleTrans(e);
	GL_BindImage(currentmodel->skins[e->frame]);

	if (e->flags & RF_NODEPTHTEST)
		qglDisable(GL_DEPTH_TEST);

	if (e->flags & RF_FIXED)
	{
		DirAndUpFromAngles(currententity->angles, dir, fixed_up);
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
			R_DrawLineSprite(e, frame, &vpn);
			break;

		default: //mxd. Avoid compiler warnings...
			Sys_Error("Unknown sprite type: %i!", e->spriteType);
			break;
	}

	if (e->flags & RF_NODEPTHTEST)
		qglEnable(GL_DEPTH_TEST);

	CleanupTrans(e);

	qglTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE); //mxd. qglTexEnvf -> qglTexEnvi
	qglShadeModel(GL_FLAT);
	qglColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}