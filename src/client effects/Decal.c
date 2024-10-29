#include "Decal.h"
#include "Client Effects.h"

int num_decals;
Decal_t decallist[MAX_DECALS];

/*
=====================
V_AddDecal

Uses:
1 int comp
1 ++
1 offset
16 Assignments

There's a better way in Wolf which does more work on the client side. . .
=====================
*/
void V_AddDecal(vec3_t pos, vec3_t up, vec3_t right, vec3_t normal, int flags, int removetime, char *modelname)
{
	Decal_t *decal;

	if (fxi.cls->r_numdlights >= MAX_DLIGHTS)
	{
		return;
	}

	decal = &fxi.cls->r_decals[fxi.cls->r_numdecals++];

	VectorCopy(decal->pos, pos);
	VectorCopy(decal->up, up);
	VectorCopy(decal->right, right);
	VectorCopy(decal->normal, normal);
	decal->flags = flags;
	decal->removetime = removetime;
	decal->modelname = modelname;
}

void CL_ClearDecals()
{
	memset(decallist, 0, sizeof(Decal_t) * MAX_DECALS);
	num_decals = 0;
}

// this allows abstraction to a more efficient system if needed
Decal_t *CL_AllocateDecal()
{
	int i;

	for(i=0; i<MAX_DECALS; i++)
	{
		if(decallist[i].flags)
		{
			continue;
		}
		else
		{
			if(num_decals < i+1)
			{
				num_decals = i+1;
			}
			memset(&decallist[i], 0, sizeof(Decal_t));
			return &decallist[i];
		}
	}
	return 0;
}

// frame updating
void CL_UpdateDecals(void)
{
	int i;
	for(i=0; i<MAX_DECALS; i++)
	{
		if(decallist[i].flags)
		{
			if(decallist[i].flags & DECALFLAG_TIMED)			
			{
				if(fxi.cl->time > decallist[i].removetime)
				{
					memset(&decallist[i], 0, sizeof(Decal_t));
				}
			}
			if(decallist[i].flags & DECALFLAG_ANIMATE)
			{
				decallist[i].frame++;
			}
		}
	}
}

#if 1
// later we may want to add bound checks or the like here 
void CL_AddDecals(void)
{
	int i;
	vec3_t vpn, dir;  //vector to decal from viewer
	float	dist;

	AngleVectors (fxi.cl->refdef.viewangles, vpn, NULL, NULL);

	fxi.cls->r_numdecals = 0;
	for(i=0; i<MAX_DECALS; i++)
	{
		if(decallist[i].flags)
		{
			VectorSubtract(decallist[i].pos, fxi.cl->refdef.vieworg, dir);
			dist = VectorNormalize(dir);
			if(dist > 150)
			{	// if far enough away
				if(DotProduct(dir, vpn) < .4)
				{	// and not within the front cone
					continue;
				}
			}
			fxi.cls->r_decals[fxi.cls->r_numdecals++] = *(decallist + i);
		}
	}
}
#endif


/*
// an example of how to use the decal functions
void CL_DecalAdd(vec3_t pos, vec3_t up, vec3_t right, char *modelname, int flags)
{
	Decal_t *newdecal;

	if(newdecal = CL_AllocateDecal())
	{
		VectorCopy(pos, newdecal->pos);
		if(flags & DECALFLAG_ORIENTED)
		{
			VectorCopy(pos, newdecal->up);
			VectorCopy(pos, newdecal->right);
			CrossProduct(up, right, newdecal->normal);
		}
		strcpy(newdecal->modelname, modelname);
		newdecal->flags = flags | DECALFLAG_ACTIVE; //active guarantees at least one flag
	}
}
*/