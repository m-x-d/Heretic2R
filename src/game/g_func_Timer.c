//
// g_func_Timer.c -- Originally part of g_func.c
//
// Copyright 1998 Raven Software
//

#include "g_func_Timer.h"
#include "Random.h"

#define SF_START_ON	1 //mxd

static void FuncTimerThink(edict_t* self) //mxd. Named 'func_timer_think' in original logic.
{
	G_UseTargets(self, self->activator);
	self->nextthink = level.time + self->wait + flrand(-self->random, self->random);
}

static void FuncTimerUse(edict_t* self, edict_t* other, edict_t* activator) //mxd. Named 'func_timer_use' in original logic.
{
	self->activator = activator;

	// If on, turn it off.
	if (self->nextthink > 0.0f)
		self->nextthink = THINK_NEVER; //mxd. '0' in original logic. Changed for consistency sake.
	else if (self->delay > 0.0f) // Turn it on.
		self->nextthink = level.time + self->delay;
	else
		FuncTimerThink(self);
}

// QUAKED func_timer (0.3 0.1 0.6) (-8 -8 -8) (8 8 8) START_ON
// These can used but not touched.

// Variables:
// wait			- Base time between triggering all targets (default 1).
// random		- Wait variance (default 0). So, the basic time between firing is a random time between (wait - random) and (wait + random).
// delay		- Delay before first firing when turned on (default  0).
// pausetime	- Additional delay used only the very first time and only if spawned with START_ON.
void SP_func_timer(edict_t* self)
{
	if (self->wait == 0.0f)
		self->wait = 1.0f;

	self->use = FuncTimerUse;
	self->think = FuncTimerThink;

	if (self->random >= self->wait)
	{
		self->random = self->wait - FRAMETIME;
		gi.dprintf("func_timer at %s has random >= wait\n", vtos(self->s.origin));
	}

	if (self->spawnflags & SF_START_ON)
	{
		self->nextthink = level.time + 1.0f + st.pausetime + self->delay + self->wait + flrand(-self->random, self->random);
		self->activator = self;
	}

	self->svflags = SVF_NOCLIENT;
}