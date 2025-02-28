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



void SpellLightningShieldAttack(edict_t *self)
{
	edict_t *found=NULL;
	vec3_t	dir;
	int		damage;
	qboolean	onefound=false;

	assert(self->client);

	// find all the foundities in the area
	while((found = FindInRadius(found, self->s.origin, SHIELD_DAMAGE_RADIUS)))
	{	// Only attack monsters and players in deathmatch
		if (found == self)		// don't hurt yourself
			continue;
		

		// don't target team members in team deathmatching, if they are on the same team, and friendly fire is not enabled.
		if ((found->client && (int)(dmflags->value) & (DF_MODELTEAMS | DF_SKINTEAMS)) && !((int)dmflags->value & DF_HURT_FRIENDS) && deathmatch->value)
		{
			if (OnSameTeam(found, self))
			continue;
		}

		if ((!(found->svflags & SVF_MONSTER) || (found->svflags & SVF_DEADMONSTER)) &&	// If we have a monster here.
				!(found->client && deathmatch->value))									// Only attack players in deathmatch
			continue;

		// if we have reflection on, then no damage
		if(EntReflecting(found, true, true))
			continue;

		if (onefound)
		{
			if (irand(0, (SHIELD_ATTACK_CHANCE-1)) != 0)									// Don't attack everything we find.
				continue;				
		}
		else
		{	// More likely to find one if none found yet.
			if (irand(0, (SHIELD_ATTACK_CHANCE-2)) != 0)									// Don't attack everything we find.
				continue;				
		}

		VectorSubtract(found->s.origin, self->s.origin, dir);
		VectorNormalize(dir);
		damage = irand(SHIELD_DAMAGE_MIN, SHIELD_DAMAGE_MAX);

		T_Damage(found, self, self, dir, vec3_origin, vec3_origin, damage, 0, DAMAGE_SPELL,MOD_SHIELD); 
		
		gi.CreateEffect(NULL, FX_LIGHTNING, 0, 
				self->s.origin, "vbb", found->s.origin, (byte)SHIELD_LIGHTNING_WIDTH, (byte)0);

		// Do a nasty looking blast at the impact point
		gi.CreateEffect(&found->s, FX_LIGHTNING_HIT, CEF_OWNERS_ORIGIN, NULL, "t", dir);

		gi.sound(self, CHAN_WEAPON, gi.soundindex("weapons/ShieldAttack.wav"), 2, ATTN_NORM,0);

		onefound = true;
	}
}	





