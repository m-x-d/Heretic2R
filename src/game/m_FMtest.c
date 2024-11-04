#include "g_local.h"
#include "Random.h"
#include "vector.h"

void MakeSolidObject(edict_t *ent, char *Model, float MinX, float MinY, float MinZ, 
					 float MaxX, float MaxY, float MaxZ)
{
	ent->movetype = PHYSICSTYPE_NONE;
	ent->solid = SOLID_BBOX;
	VectorSet (ent->mins, MinX, MinY, MinZ);
	VectorSet (ent->maxs, MaxX, MaxY, MaxZ);
	gi.setmodel(ent, Model);

	gi.linkentity (ent);
}

void flag_think (edict_t *self)
{
	self->s.frame++;
	if (self->s.frame > 10)
		self->s.frame = 0;

/*	self->s.fmnodeinfo[1].skin++;
	if (self->s.fmnodeinfo[1].skin > 5)
		self->s.fmnodeinfo[1].skin = 0;


	self->s.fmnodeinfo[0].flags &= ~FMNI_NO_DRAW;
	if (random() > 0.5)
		self->s.fmnodeinfo[0].flags |= FMNI_NO_DRAW;

	self->s.fmnodeinfo[1].flags &= ~FMNI_NO_DRAW;
	if (random() > 0.5)
		self->s.fmnodeinfo[1].flags |= FMNI_NO_DRAW;

	self->s.fmnodeinfo[2].flags &= ~FMNI_NO_DRAW;
	if (random() > 0.5)
		self->s.fmnodeinfo[2].flags |= FMNI_NO_DRAW;

	self->s.fmnodeinfo[3].flags &= ~FMNI_NO_DRAW;
	if (random() > 0.5)
		self->s.fmnodeinfo[3].flags |= FMNI_NO_DRAW;*/

	self->nextthink = level.time + FRAMETIME;
}

/*QUAKED misc_flag (1 .5 0) (-10 -10 0) (10 10 80)
*/
void SP_misc_flag (edict_t *ent)
{
	MakeSolidObject(ent, "models/rj5/tris.fm", -10, -10, 0, 10, 10, 80);

	ent->think = flag_think;
	ent->nextthink = level.time + flrand(0.0F, 1.0F);

/*	ent->s.fmnodeinfo[1].skin = 2;
	
	ent->s.fmnodeinfo[0].color.r = 255;
	ent->s.fmnodeinfo[0].color.a = 120;
	ent->s.fmnodeinfo[0].flags |= FMNI_USE_COLOR;
	
	ent->s.fmnodeinfo[1].color.g = 255;
	ent->s.fmnodeinfo[1].color.a = 255;
	ent->s.fmnodeinfo[1].flags |= FMNI_USE_COLOR;
	
	ent->s.fmnodeinfo[2].color.b = 255;
	ent->s.fmnodeinfo[2].color.a = 200;
	ent->s.fmnodeinfo[2].flags |= FMNI_USE_COLOR;
	
	ent->s.fmnodeinfo[3].color.r = 255;
	ent->s.fmnodeinfo[3].color.g = 255;
	ent->s.fmnodeinfo[3].color.a = 80;
	ent->s.fmnodeinfo[3].flags |= FMNI_USE_COLOR;*/
}
