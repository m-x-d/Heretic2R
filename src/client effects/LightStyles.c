#include "Client Effects.h"

/*
==============================================================

LIGHT STYLE MANAGEMENT

==============================================================
*/

typedef struct
{
	int		length;
	float	value[3];
	float	map[MAX_QPATH];
} clightstyle_t;

clightstyle_t	cl_lightstyle[MAX_LIGHTSTYLES];
int			lastofs;

/*
=====================
V_AddLightStyle

Uses:
1 - 2 int comp
1 OR
4 offsets
2 additions
4 Assignments
=====================
*/
void V_AddLightStyle (int style, float r, float g, float b)
{
	lightstyle_t	*ls;

	if (style < 0 || style > MAX_LIGHTSTYLES)
	{
		assert(0);
		Com_Error (ERR_DROP, "Bad light style %i", style);
	}
	else
	{
		ls = &fxi.cls->r_lightstyles[style];

		ls->white = r+g+b;
		ls->rgb[0] = r;
		ls->rgb[1] = g;
		ls->rgb[2] = b;
	}
}

/*
================
CL_ClearLightStyles
================
*/
void CL_ClearLightStyles (void)
{
	memset (cl_lightstyle, 0, sizeof(cl_lightstyle));
	lastofs = -1;
}

/*
================
CL_RunLightStyles
================
*/
void CL_RunLightStyles (void)
{
	int		ofs;
	int		i;
	clightstyle_t	*ls;

	ofs = fxi.cl->time / 100;

	if(ofs == lastofs)
	{
		return;
	}

	lastofs = ofs;

	for(i = 0, ls = cl_lightstyle; i < MAX_LIGHTSTYLES ; ++i, ++ls)
	{
		if (!ls->length)
		{
			ls->value[0] = ls->value[1] = ls->value[2] = 1.0;
			continue;
		}

		if (ls->length == 1)
		{
			ls->value[0] = ls->value[1] = ls->value[2] = ls->map[0];
		}
		else
		{
			ls->value[0] = ls->value[1] = ls->value[2] = ls->map[ofs%ls->length];
		}
	}
}


void CL_SetLightstyle (int i)
{
	char	*s;
	int		j, k;

	s = fxi.cl->configstrings[i+CS_LIGHTS];

	j = strlen (s);

	if (j >= MAX_QPATH)
	{
		assert(0);
		fxi.Com_Error (ERR_DROP, "svc_lightstyle length=%i", j);
	}

	cl_lightstyle[i].length = j;

	for (k=0 ; k<j ; k++)
	{
		cl_lightstyle[i].map[k] = (float)(s[k]-'a')/(float)('m'-'a');
	}
}

/*
================
CL_AddLightStyles
================
*/
void CL_AddLightStyles (void)
{
	int		i;
	clightstyle_t	*ls;

	for (i=0,ls=cl_lightstyle ; i<MAX_LIGHTSTYLES ; i++, ls++)
	{
		V_AddLightStyle (i, ls->value[0], ls->value[1], ls->value[2]);
	}
}

