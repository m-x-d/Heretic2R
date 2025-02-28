//
// spl_shield.c
//
// Copyright 1998 Raven Software
//

#include "spl_shield.h" //mxd
#include "g_cmds.h" //mxd
#include "g_combat.h" //mxd
#include "g_playstats.h"
#include "FX.h"
#include "Random.h"
#include "Vector.h"
#include "g_local.h"

void SpellCastShield(edict_t* caster, vec3_t startpos, vec3_t aimangles, vec3_t aimdir, float Value) //TODO: remove unuse args.
{
	assert(caster->client != NULL);
	caster->client->playerinfo.shield_timer = level.time + (float)SHIELD_DURATION;

	// Start the lightning effect.
	caster->PersistantCFX = gi.CreatePersistantEffect(&caster->s, FX_SPELL_LIGHTNINGSHIELD, CEF_OWNERS_ORIGIN | CEF_BROADCAST, NULL, "");

	gi.sound(caster, CHAN_WEAPON, gi.soundindex("weapons/Shield.wav"), 1.0f, ATTN_NORM, 0.0f);
	caster->s.sound = (byte)gi.soundindex("weapons/ShieldIdle.wav");
	caster->s.sound_data = (255 & ENT_VOL_MASK) | ATTN_NORM;
}

void SpellLightningShieldAttack(edict_t* self)
{
	assert(self->client != NULL);

	// Find all the entities in the area.
	edict_t* ent = NULL;
	qboolean ent_found = false;

	while ((ent = FindInRadius(ent, self->s.origin, SHIELD_DAMAGE_RADIUS)) != NULL)
	{
		if (ent == self) // Don't hurt yourself.
			continue;

		// Don't target team members in team deathmatch, if they are on the same team, and friendly fire is not enabled.
		if (DEATHMATCH && (DMFLAGS & (DF_MODELTEAMS | DF_SKINTEAMS)) && !(DMFLAGS & DF_HURT_FRIENDS) && ent->client != NULL && OnSameTeam(ent, self))
			continue;

		// Only attack [monsters] or [players in deathmatch].
		if ((!(ent->svflags & SVF_MONSTER) || (ent->svflags & SVF_DEADMONSTER)) && (!DEATHMATCH || ent->client == NULL))
			continue;

		// If we have reflection on, then no damage.
		if (EntReflecting(ent, true, true))
			continue;

		// More likely to find one if none found yet.
		const int chance_mod = (ent_found ? 1 : 2); //mxd
		if (irand(0, (SHIELD_ATTACK_CHANCE - chance_mod)) != 0) // Don't attack everything we find.
			continue;

		// Start lightning effect.
		vec3_t dir;
		VectorSubtract(ent->s.origin, self->s.origin, dir);
		VectorNormalize(dir);

		const int damage = irand(SHIELD_DAMAGE_MIN, SHIELD_DAMAGE_MAX);
		T_Damage(ent, self, self, dir, vec3_origin, vec3_origin, damage, 0, DAMAGE_SPELL, MOD_SHIELD);

		gi.CreateEffect(NULL, FX_LIGHTNING, 0, self->s.origin, "vbb", ent->s.origin, (byte)SHIELD_LIGHTNING_WIDTH, (byte)0);

		// Do a nasty looking blast at the impact point.
		gi.CreateEffect(&ent->s, FX_LIGHTNING_HIT, CEF_OWNERS_ORIGIN, NULL, "t", dir);

		gi.sound(self, CHAN_WEAPON, gi.soundindex("weapons/ShieldAttack.wav"), 2.0f, ATTN_NORM, 0.0f); //TODO: why 2.0 volume?

		ent_found = true;
	}
}