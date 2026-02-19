//
// spl_morph.c
//
// Copyright 1998 Raven Software
//

#include "spl_morph.h" //mxd
#include "g_cmds.h" //mxd
#include "g_monster.h"
#include "g_playstats.h"
#include "p_morph.h" //mxd
#include "FX.h"
#include "Random.h"
#include "Utilities.h" //mxd
#include "Vector.h"
#include "g_local.h"

#define OVUM_SPEED		400.0f
#define OVUM_RADIUS		2.0f
#define ANGLE_INC		(360.0f / NUM_OF_OVUMS)
#define MORPH_TIME		20.0f //mxd

// Fade in the chicken - for MONSTERS only.
void MonsterMorphFadeIn(edict_t* self) //mxd. Named 'MorphFadeIn' in original version.
{
	self->s.color.a += MORPH_TELE_FADE;
	self->nextthink = level.time + FRAMETIME; //mxd. Use define.

	if (--self->morph_timer == 0)
		self->think = M_WalkmonsterStartGo;
}

// Fade out the chicken model till its gone - for MONSTERS only.
void MonsterMorphFadeOut(edict_t* self) //mxd. Named 'MorphFadeOut' in original version.
{
	self->s.color.a -= MORPH_TELE_FADE;
	self->nextthink = level.time + FRAMETIME; //mxd. Use define.

	if (--self->morph_timer > 0)
		return;

	// Create The Chicken Object.
	edict_t* chicken = G_Spawn();
	chicken->classname = "monster_chicken";
	VectorCopy(self->s.origin, chicken->s.origin);

	// If we are looking at an original model that's not got an origin at the waist, move us up in the world.
	if (self->mins[2] == 0.0f)
		chicken->s.origin[2] += 16.0f;

	VectorCopy(self->s.angles, chicken->s.angles);
	chicken->enemy = self->enemy;

	// Keep some info around so we can return to our original persona.
	chicken->morph_classname = self->classname;
	chicken->target = self->target;
	chicken->morph_animation_frame = self->s.frame; //mxd. Store animation frame (restored in MorphChickenOut()).
	//TODO: also store health and skinnum?

	// Time we stay a chicken.
	chicken->time = level.time + MORPH_TIME; //mxd. Use define.

	ED_CallSpawn(chicken);

	chicken->s.color.c = 0xffffff; // White, 0 alpha.
	chicken->morph_timer = MORPH_TELE_TIME;
	chicken->think = MonsterMorphFadeIn;

	gi.CreateEffect(&chicken->s, FX_PLAYER_TELEPORT_IN, CEF_OWNERS_ORIGIN | CEF_FLAG6, NULL, "");
	gi.sound(chicken, CHAN_WEAPON, gi.soundindex("weapons/teleport.wav"), 1.0f, ATTN_NORM, 0.0f);

	G_SetToFree(self);
}

// This called when missile touches anything (world or edict).
void MorphMissileTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surface)
{
	// Has the target got reflection turned on?
	if (self->reflect_debounce_time > 0 && EntReflecting(other, true, true))
	{
		Create_rand_relect_vect(self->velocity, self->velocity);
		Vec3ScaleAssign(OVUM_SPEED / 2.0f, self->velocity);
		MorphReflect(self, other, self->velocity);

		return;
	}

	// Turn target into a chicken if monster or player.
	if (((other->svflags & SVF_MONSTER) && !(other->svflags & SVF_BOSS) && !other->monsterinfo.c_mode) || (other->client != NULL && DEATHMATCH))
	{
		if (other->client != NULL)
		{
			qboolean skip_morph = false; //mxd

			// Don't turn a super chicken back to a player.
			if (other->client->playerinfo.edictflags & FL_SUPER_CHICKEN)
				skip_morph = true;

			// Don't target team members in team deathmatching, if they are on the same team, and friendly fire is not enabled.
			if (DEATHMATCH && (DMFLAGS & (DF_MODELTEAMS | DF_SKINTEAMS)) && !(DMFLAGS & DF_HURT_FRIENDS) && OnSameTeam(other, self->owner))
				skip_morph = true;

			if (skip_morph)
			{
				// Turn off the client effect.
				gi.sound(other, CHAN_WEAPON, gi.soundindex("misc/null.wav"), 1.0f, ATTN_NORM, 0.0f);
				gi.CreateEffect(NULL, FX_SPELL_MORPHEXPLODE, 0, self->s.origin, "d", self->movedir);
				G_SetToFree(self);

				return;
			}
		}

		if (other->svflags & SVF_MONSTER)
		{
			// Deal with the existing bad guy.
			other->think = MonsterMorphFadeOut;
			other->nextthink = level.time + FRAMETIME; //mxd. Use define.
			other->touch = NULL;
			other->morph_timer = MORPH_TELE_TIME;
			other->enemy = self->owner;
			VectorClear(other->velocity);

			gi.CreateEffect(&other->s, FX_PLAYER_TELEPORT_OUT, CEF_OWNERS_ORIGIN | CEF_FLAG6, NULL, "");
		}
		else
		{
			MorphPlayerToChickenStart(other);
		}

		const char* snd_name = "weapons/crow.wav"; //mxd
		if (DEATHMATCH && other->client != NULL && (other->client->playerinfo.edictflags & FL_SUPER_CHICKEN)) // There shouldn't be any monsters in deathmatch, but...
			snd_name = "weapons/supercrow.wav"; //mxd

		gi.sound(other, CHAN_VOICE, gi.soundindex(snd_name), 1.0f, ATTN_NORM, 0.0f);

		// Turn off the client effect.
		gi.sound(other, CHAN_WEAPON, gi.soundindex("misc/null.wav"), 1.0f, ATTN_NORM, 0.0f);
		gi.CreateEffect(NULL, FX_SPELL_MORPHEXPLODE, 0, self->s.origin, "d", self->movedir);
	}
	else // We hit a wall or object.
	{
		//mxd. Play impact sound.
		gi.sound(self, CHAN_VOICE, gi.soundindex(va("Monsters/chicken/bite%i.wav", irand(1, 2))), 1.0f, ATTN_IDLE, 0.0f);

		// Start the explosion.
		vec3_t* dir = (plane != NULL ? &plane->normal : &self->movedir); //mxd
		gi.CreateEffect(NULL, FX_SPELL_MORPHEXPLODE, 0, self->s.origin, "d", *dir);
	}

	// Turn off the client effect.
	G_SetToFree(self);
}

void MorphMissileThink(edict_t* self)
{
	self->svflags |= SVF_NOCLIENT; // No messages to client after it has received velocity.
	self->think = NULL; // Not required to think anymore.
}

// Create the guts of the morph ovum.
static void CreateMorphOvum(edict_t* egg) //mxd. Named 'create_morph' in original version.
{
	egg->s.effects |= EF_ALWAYS_ADD_EFFECTS;
	egg->svflags |= SVF_ALWAYS_SEND;
	egg->movetype = MOVETYPE_FLYMISSILE;
	egg->classname = "Spell_MorphArrow";
	egg->solid = SOLID_BBOX;
	egg->clipmask = MASK_MONSTERSOLID;

	// Set up our collision boxes.
	VectorSet(egg->mins, -OVUM_RADIUS, -OVUM_RADIUS, -OVUM_RADIUS);
	VectorSet(egg->maxs,  OVUM_RADIUS,  OVUM_RADIUS,  OVUM_RADIUS);

	egg->touch = MorphMissileTouch;
	egg->think = MorphMissileThink;
	egg->nextthink = level.time + FRAMETIME; //mxd. Use define.
}

edict_t* MorphReflect(edict_t* self, edict_t* other, const vec3_t vel)
{
	// Create a new missile to replace the old one - this is necessary because physics will do nasty things
	// with the existing one, since we hit something. Hence, we create a new one totally.
	edict_t* egg = G_Spawn();

	// Copy everything across.
	CreateMorphOvum(egg);
	VectorCopy(self->s.origin, egg->s.origin);
	VectorCopy(vel, egg->velocity);
	VectorNormalize2(egg->velocity, egg->movedir);
	AnglesFromDir(egg->movedir, egg->s.angles);
	Vec3ScaleAssign(RAD_TO_ANGLE, egg->s.angles); //mxd. Convert to degrees (not done in original logic).
	egg->owner = other;
	egg->enemy = self->enemy;
	egg->reflect_debounce_time = self->reflect_debounce_time - 1; // So it doesn't infinitely reflect in one frame somehow.
	egg->reflected_time = self->reflected_time;

	G_LinkMissile(egg);

	// Create new trails for the new missile.
	const byte b_yaw = (byte)(egg->s.angles[YAW] * DEG_TO_BYTEANGLE);
	const byte b_pitch = (byte)(egg->s.angles[PITCH] * DEG_TO_BYTEANGLE);
	gi.CreateEffect(&egg->s, FX_SPELL_MORPHMISSILE, CEF_OWNERS_ORIGIN | CEF_FLAG6, NULL, "bb", b_yaw, b_pitch);

	// Kill the existing missile, since its a pain in the ass to modify it so the physics won't screw it.
	G_SetToFree(self);

	// Do a nasty looking blast at the impact point.
	gi.CreateEffect(&egg->s, FX_LIGHTNING_HIT, CEF_OWNERS_ORIGIN, NULL, "t", egg->velocity);

	return egg;
}

void SpellCastMorph(edict_t* caster, const vec3_t start_pos, const vec3_t aim_angles)
{
	short morph_array[NUM_OF_OVUMS];
	float current_ang = aim_angles[YAW];

	for (int i = 0; i < NUM_OF_OVUMS; i++)
	{
		// Create each of the server side entities that are the morph ovum spells.
		edict_t* egg = G_Spawn();

		// Decide its direction.
		egg->s.angles[YAW] = current_ang;

		vec3_t angles_rad;
		VectorScale(egg->s.angles, ANGLE_TO_RAD, angles_rad);

		vec3_t direction;
		DirFromAngles(angles_rad, direction);

		VectorMA(start_pos, OVUM_RADIUS * 3.0f, direction, egg->s.origin); //mxd. Offset from start_pos to avoid eggs immediately touching each other...
		VectorScale(direction, OVUM_SPEED, egg->velocity);

		CreateMorphOvum(egg);
		egg->reflect_debounce_time = MAX_REFLECT;
		egg->owner = caster;

		G_LinkMissile(egg);

		morph_array[i] = egg->s.number; // Store the entity numbers for sending with the effect.
		current_ang += ANGLE_INC; // Increment current angle to get circular radius of ovums.
	}

	// Create the client effect that gets seen on screen.
	const byte b_yaw = (byte)(aim_angles[YAW] * DEG_TO_BYTEANGLE);
	gi.CreateEffect(&caster->s, FX_SPELL_MORPHMISSILE_INITIAL, CEF_OWNERS_ORIGIN, NULL, "bssssss",
		b_yaw, morph_array[0], morph_array[1], morph_array[2], morph_array[3], morph_array[4], morph_array[5]);
}