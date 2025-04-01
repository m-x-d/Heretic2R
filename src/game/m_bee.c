//
// m_bee.c
//
// Copyright 1998 Raven Software
//

#include "m_bee.h" //mxd
#include "g_obj.h" //mxd
#include "Vector.h"
#include "g_local.h"

void BeeStaticsInit(void) { } //TODO: remove?

// QUAKED monster_bee (1 .5 0) (-16 -16 -24) (16 16 16)
// The unimplemented bee.
void SP_monster_bee(edict_t* self)
{
	self->s.modelindex = (byte)gi.modelindex("models/monsters/bee/tris.fm"); //TODO: has unused 60-frame wing flap animation.

	VectorSet(self->mins, -2.0f, -2.0f, -25.0f);
	VectorSet(self->maxs, 2.0f, 2.0f, 25.0f);

	ObjectInit(self, 40, 40, MAT_WOOD, SOLID_BBOX); //TODO: why MAT_WOOD?..
}