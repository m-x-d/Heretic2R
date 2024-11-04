//
// spl_powerup.c
//
// Heretic II
// Copyright 1998 Raven Software
//

#include "g_local.h"
#include "fx.h"
#include "g_itemstats.h"


void SpellCastPowerup(edict_t *Caster, vec3_t StartPos, vec3_t AimAngles, vec3_t AimDir, float value)
{
	assert(Caster->client);

	// Kill any tomes that may already be out there for this player.

	gi.RemoveEffects(&Caster->s, FX_TOME_OF_POWER);

	// if we are a chicken, lets make us a player again
	if (Caster->flags & FL_CHICKEN)
	{
		Caster->morph_timer = level.time - 0.1;
	}
	else
	{
		// add some time in on the timer for the powerup
		Caster->client->playerinfo.powerup_timer = level.time + POWERUP_DURATION;

		// turn on the light at the client effect end through client flags that are passed down
		Caster->s.effects |= EF_POWERUP_ENABLED;
		Caster->client->playerinfo.effects |= EF_POWERUP_ENABLED;

		// create the tome of power
		gi.CreateEffect(&Caster->s, FX_TOME_OF_POWER, CEF_OWNERS_ORIGIN, NULL, "");
	}

	// start up the shrine powerup effect
//	gi.CreateEffect(&Caster->s, FX_SHRINE_POWERUP, CEF_OWNERS_ORIGIN, NULL, "");

	// do the SHRINE sound
	gi.sound(Caster, CHAN_ITEM,gi.soundindex("items/shrine5.wav"),1,ATTN_NORM,0);
}

// end
