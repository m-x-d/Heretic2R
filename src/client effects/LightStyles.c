//
// LightStyles.c -- Light styles management.
//
// Copyright 1998 Raven Software
//

#include "LightStyles.h" //mxd
#include "Client Effects.h"
#include "Vector.h" //mxd

typedef struct
{
	int length;
	float value[3];
	float map[MAX_QPATH];
} clightstyle_t;

static clightstyle_t cl_lightstyles[MAX_LIGHTSTYLES];
static int lastofs;

static void V_AddLightStyle(const int style, const float r, const float g, const float b)
{
	if (style < 0 || style >= MAX_LIGHTSTYLES) //BUGFIX: mxd. 'style > MAX_LIGHTSTYLES' in original logic.
	{
		fxi.Com_Error(ERR_DROP, "Bad light style %i", style); //mxd. Com_Error() -> fxi.Com_Error().
	}
	else
	{
		lightstyle_t* ls = &fxi.cls->r_lightstyles[style];

		ls->white = r + g + b;
		VectorSet(ls->rgb, r, g, b);
	}
}

void CL_ClearLightStyles(void)
{
	memset(cl_lightstyles, 0, sizeof(cl_lightstyles));
	lastofs = -1;
}

void CL_RunLightStyles(void)
{
	float value; //mxd
	const int ofs = fxi.cl->time / 100;

	if (ofs == lastofs)
		return;

	lastofs = ofs;

	clightstyle_t* ls = &cl_lightstyles[0];
	for (int i = 0; i < MAX_LIGHTSTYLES; i++, ls++)
	{
		if (ls->length == 0)
			value = 1.0f;
		else if (ls->length == 1)
			value = ls->map[0];
		else
			value = ls->map[ofs % ls->length];

		VectorSet(ls->value, value, value, value);
	}
}

void CL_SetLightstyle(const int index)
{
	const char* s = fxi.cl->configstrings[index + CS_LIGHTS];
	const int len = (int)strlen(s);

	if (len >= MAX_QPATH)
	{
		assert(0);
		fxi.Com_Error(ERR_DROP, "svc_lightstyle length=%i", len);
	}

	cl_lightstyles[index].length = len;

	for (int i = 0; i < len; i++)
		cl_lightstyles[index].map[i] = (float)(s[i] - 'a') / (float)('m' - 'a');
}

void CL_AddLightStyles(void)
{
	clightstyle_t* ls = &cl_lightstyles[0];
	for (int i = 0; i < MAX_LIGHTSTYLES; i++, ls++)
		V_AddLightStyle(i, ls->value[0], ls->value[1], ls->value[2]);
}