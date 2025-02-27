//
// spl_powerup.c
//
// Copyright 1998 Raven Software
//

#include "spl_powerup.h" //mxd
#include "FX.h"
#include "g_itemstats.h"
#include "g_local.h"

void SpellCastPowerup(edict_t* caster)
{
	assert(caster->client != NULL);

	// Kill any tomes that may already be out there for this player.
	gi.RemoveEffects(&caster->s, FX_TOME_OF_POWER);

	// If we are a chicken, lets make us a player again.
	if (caster->flags & FL_CHICKEN)
	{
		caster->morph_timer = (int)level.time - 1;
	}
	else
	{
		// Add some time in on the timer for the powerup
		caster->client->playerinfo.powerup_timer = level.time + POWERUP_DURATION;

		// Turn on the light at the client effect end through client flags that are passed down.
		caster->s.effects |= EF_POWERUP_ENABLED;
		caster->client->playerinfo.effects |= EF_POWERUP_ENABLED;

		// Create the Tome of Power effect.
		gi.CreateEffect(&caster->s, FX_TOME_OF_POWER, CEF_OWNERS_ORIGIN, NULL, "");
	}

	// Do the SHRINE sound.
	gi.sound(caster, CHAN_ITEM, gi.soundindex("items/shrine5.wav"), 1.0f, ATTN_NORM, 0.0f);
}