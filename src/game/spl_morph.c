//
// spl_morph.c
//
// Copyright 1998 Raven Software
//

#include "spl_morph.h" //mxd
#include "g_cmds.h" //mxd
#include "g_monster.h"
#include "g_Physics.h"
#include "g_playstats.h"
#include "g_teleport.h"
#include "g_Shrine.h" //mxd
#include "g_Skeletons.h"
#include "m_chicken_anim.h"
#include "p_main.h"
#include "p_anims.h"
#include "p_client.h" //mxd
#include "FX.h"
#include "Random.h"
#include "Vector.h"
#include "g_local.h"

#define OVUM_SPEED		400.0f
#define OVUM_RADIUS		2.0f
#define ANGLE_INC		(360.0f / NUM_OF_OVUMS)

char	chicken_text[] = "monster_chicken";

void create_morph(edict_t *morph);

// Fade in the chicken - for MONSTERS only.
static void MonsterMorphFadeIn(edict_t* self) //mxd. Named 'MorphFadeIn' in original version.
{
	self->s.color.a += MORPH_TELE_FADE;
	self->nextthink = level.time + 0.1f;

	if (--self->morph_timer == 0)
		self->think = walkmonster_start_go;
}

// Fade out the chicken model till its gone - for MONSTERS only.
static void MonsterMorphFadeOut(edict_t* self) //mxd. Named 'MorphFadeOut' in original version.
{
	self->s.color.a -= MORPH_TELE_FADE;
	self->nextthink = level.time + 0.1f;

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
	chicken->map = self->classname;
	chicken->target = self->target;

	// Time we stay a chicken.
	chicken->time = level.time + 20.0f;

	ED_CallSpawn(chicken);

	chicken->s.color.c = 0xffffff; //TODO: should also set alpha to 255?
	chicken->morph_timer = MORPH_TELE_TIME;
	chicken->think = MonsterMorphFadeIn;

	gi.CreateEffect(&chicken->s, FX_PLAYER_TELEPORT_IN, CEF_OWNERS_ORIGIN | CEF_FLAG6, NULL, "");
	gi.sound(chicken, CHAN_WEAPON, gi.soundindex("weapons/teleport.wav"), 1.0f, ATTN_NORM, 0.0f);

	G_SetToFree(self);
}

// Done morphing, clean up after ourselves - for PLAYER only.
void CleanUpPlayerMorph(edict_t* self) //mxd. Named 'CleanUpMorph' in original version.
{
	VectorClear(self->client->tele_dest);

	self->client->tele_count = 0;
	self->client->playerinfo.edictflags &= ~FL_LOCKMOVE;
	self->client->playerinfo.renderfx &= ~RF_TRANSLUCENT;
	self->client->playerinfo.flags &= ~PLAYER_FLAG_MORPHING;
	self->client->shrine_framenum = level.time - 1.0f;

	self->s.color.a = 255;
}

// We are done being a chicken, let's be Corvus again - switch models from chicken back to Corvus and do teleport fade in - for PLAYER only.
void ResetPlayerMorph(edict_t* self) //mxd. Named 'reset_morph_to_elf' in original version.
{
	// We have no damage, and no motion type.
	self->takedamage = DAMAGE_AIM;
	self->movetype = PHYSICSTYPE_STEP;
	self->health = self->max_health;

	// Move the camera back to where it should be, and reset our lungs and stuff.
	self->viewheight = 0;
	self->mass = 200;
	self->deadflag = DEAD_NO;
	self->air_finished = level.time + HOLD_BREATH_TIME;

	self->s.scale = 1.0f;

	// Set the model back to Corvus.
	self->model = "models/player/corvette/tris.fm"; //TODO: there's no such model!
	self->pain = player_pain;
	self->die = player_die;
	self->flags &= ~FL_NO_KNOCKBACK;
	self->gravity = 1.0f;

	// Reset our skins.
	self->client->playerinfo.effects = 0;
	self->client->playerinfo.skinnum = 0;
	self->client->playerinfo.clientnum = self - g_edicts - 1;
	self->s.modelindex = 255; // Will use the skin-specified model.
	self->client->playerinfo.frame = 0;

	// Turn our skeleton back on.
	self->s.skeletalType = SKEL_CORVUS;
	self->client->playerinfo.effects |= (EF_SWAPFRAME | EF_JOINTED | EF_CAMERA_NO_CLIP | EF_PLAYER);
	self->client->playerinfo.effects &= ~EF_CHICKEN;
	self->client->playerinfo.edictflags &= ~FL_CHICKEN;
	self->client->playerinfo.renderfx &= ~RF_IGNORE_REFS;

	// Reset our mins and maxs. And then let the physics move us out of anyone else's bounding box.
	VectorCopy(player_mins, self->intentMins);
	VectorCopy(player_maxs, self->intentMaxs);
	self->physicsFlags |= PF_RESIZE;

	// Reset our thinking.
	self->think = self->oldthink;
	self->nextthink = level.time + 0.1f;

	// Reset our animations.
	P_PlayerBasicAnimReset(&self->client->playerinfo);
	self->client->playerinfo.upperframe = 43; //mxd. FRAME_asscrtch24 ?..
	self->client->playerinfo.lowerframe = 43;

	P_PlayerUpdateModelAttributes(&self->client->playerinfo);
	P_PlayerAnimSetLowerSeq(&self->client->playerinfo, ASEQ_NONE);
	P_PlayerAnimSetLowerSeq(&self->client->playerinfo, ASEQ_IDLE_WIPE_BROW);

	// Draw the teleport splash at the destination.
	gi.CreateEffect(&self->s, FX_PLAYER_TELEPORT_IN, CEF_BROADCAST | CEF_OWNERS_ORIGIN | CEF_FLAG6, self->s.origin, "");

	// Restart the loop and tell us next time we aren't de-materializing.
	self->client->tele_count = TELE_TIME;
	VectorSet(self->client->tele_dest, -1.0f, -1.0f, -1.0f);
}

// Modify a chicken into a player - first call. Start the teleport effect on the chicken. For PLAYER only.
static void MorphChickenToPlayer(edict_t* self)
{
	// If we are teleporting or morphing, forget it.
	if (self->client->playerinfo.flags & (PLAYER_FLAG_TELEPORT | PLAYER_FLAG_MORPHING))
		return;

	self->client->playerinfo.flags |= PLAYER_FLAG_MORPHING; // Set the player as teleporting.
	self->client->tele_count = TELE_TIME_OUT; // Time taken over de-materialization.
	self->client->shrine_framenum = level.time + 10.0f; // Make us invulnerable for a couple of seconds.
	self->client->tele_type = 1; // Tell us how we triggered the teleport.
	self->flags |= FL_LOCKMOVE; // Make the player stand still. //TODO: sets 'self->client->playerinfo.flags' in MorphPlayerToChicken().

	// Clear the velocity and hold them in place briefly.
	VectorClear(self->velocity);
	self->client->ps.pmove.pm_time = 50;

	// Allow the player to fade out.
	self->s.color.c = 0xffffffff;
	self->s.renderfx |= RF_TRANSLUCENT;

	// Make us not think at all.
	self->think = NULL;

	// Make it so that the stuff that does the de-materialization in G_ANIM_ACTOR knows we are fading out, not in.
	VectorClear(self->client->tele_dest);

	// Draw the teleport splash at the teleport source.
	gi.CreateEffect(&self->s, FX_PLAYER_TELEPORT_OUT, CEF_OWNERS_ORIGIN | CEF_FLAG6, NULL, "");

	// Do the teleport sound.
	gi.sound(self, CHAN_WEAPON, gi.soundindex("weapons/teleport.wav"), 1.0f, ATTN_NORM, 0.0f);
}

// Watch the chicken to see if we should become the elf again. For PLAYER only.
static void ChickenPlayerThink(edict_t* self) //mxd. Named 'watch_chicken' in original version.
{
	// Are we done yet?
	if (self->morph_timer <= (int)level.time)
		MorphChickenToPlayer(self);

	self->nextthink = level.time + 0.1f;
}

// Switch the models from player to chicken and then make us re-appear ala teleport. For PLAYER only.
void PerformPlayerMorph(edict_t* self) //mxd. Named 'Perform_Morph' in original version.
{
	static const vec3_t mins = { -16.0f, -16.0f, -36.0f }; //mxd. Made local static.
	static const vec3_t maxs = {  16.0f,  16.0f,  36.0f }; //mxd. Made local static.

	// Change out our model.
	self->model = "models/monsters/chicken2/tris.fm";
	self->s.modelindex = (byte)gi.modelindex("models/monsters/chicken2/tris.fm");

	self->client->playerinfo.effects &= ~(EF_JOINTED | EF_SWAPFRAME);
	self->client->playerinfo.effects |= EF_CHICKEN;
	self->s.skeletalType = SKEL_NULL;
	self->client->playerinfo.renderfx |= RF_IGNORE_REFS;

	qboolean super_chicken = (irand(0, 10) == 0);

	// Check if super-chicken can fit here.
	if (super_chicken)
	{
		vec3_t pos;
		VectorCopy(self->s.origin, pos);
		pos[2] += 2.0f;

		trace_t trace;
		gi.trace(pos, mins, maxs, pos, self, MASK_PLAYERSOLID, &trace);

		if (trace.fraction < 1.0f || trace.startsolid || trace.allsolid)
			super_chicken = false;
	}

	if (super_chicken)
	{
		// Reset our motion stuff.
		self->health = 999;
		self->mass = 3000;
		self->yaw_speed = 30.0f;
		self->gravity = 1.0f;

		self->monsterinfo.scale = 2.5f;
		self->s.scale = 2.5f;

		VectorSet(self->mins, -16.0f, -16.0f, -48.0f);
		VectorSet(self->maxs,  16.0f,  16.0f,  64.0f);

		self->client->playerinfo.edictflags |= FL_SUPER_CHICKEN;
	}
	else
	{
		self->health = 1;
		self->mass = 30;
		self->yaw_speed = 20.0f;
		self->gravity = 0.6f;

		self->monsterinfo.scale = MODEL_SCALE;

		// Set new mins and maxs.
		VectorSet(self->intentMins, -8.0f, -8.0f, -12.0f);
		VectorSet(self->intentMaxs,  8.0f,  8.0f,  12.0f);

		self->client->playerinfo.edictflags |= FL_AVERAGE_CHICKEN;
	}

	// Not being knocked back, and stepping like a chicken.
	self->movetype = PHYSICSTYPE_STEP;
	VectorClear(self->knockbackvel);

	// Reset which skin we use.
	self->client->playerinfo.skinnum = 0;
	self->client->playerinfo.clientnum = self - g_edicts - 1;

	// Reset our thinking.
	self->oldthink = self->think;
	self->think = ChickenPlayerThink;
	self->nextthink = level.time + 0.1f;

	self->physicsFlags |= PF_RESIZE;

	for (int i = 0; i < MAX_FM_MESH_NODES; i++)
		self->client->playerinfo.fmnodeinfo[i].flags &= ~FMNI_NO_DRAW;

	// Reset our animation.
	P_PlayerAnimSetLowerSeq(&self->client->playerinfo, ASEQ_STAND);

	// Draw the teleport splash at the destination.
	gi.CreateEffect(&self->s, FX_PLAYER_TELEPORT_IN, CEF_BROADCAST | CEF_OWNERS_ORIGIN | CEF_FLAG6, self->s.origin, "");

	// Restart the loop and tell us next time we aren't de-materializing.
	self->client->tele_count = TELE_TIME;
	VectorSet(self->client->tele_dest, -1.0f, -1.0f, -1.0f);
}

// Modify a player into a chicken - first call. Start the teleport effect on the player. For PLAYER only.
void MorphPlayerToChicken(edict_t* self, edict_t* caster) //TODO: remove unused arg.
{
	// If we are teleporting or morphing, forget it.
	if (self->client->playerinfo.flags & (PLAYER_FLAG_TELEPORT | PLAYER_FLAG_MORPHING))
		return;

	// Remove any hand or weapon effects.
	P_TurnOffPlayerEffects(&self->client->playerinfo);

	// Remove any shrine effects he has.
	PlayerKillShrineFX(self);

	self->client->playerinfo.flags |= PLAYER_FLAG_MORPHING; // Set the player as teleporting.
	self->client->tele_count = TELE_TIME_OUT; // Time taken over de-materialization.
	self->client->shrine_framenum = level.time + 10.0f; // Make us invulnerable for a couple of seconds.
	self->client->tele_type = 1; // Tell us how we triggered the teleport.
	self->client->playerinfo.flags |= FL_LOCKMOVE; // Make the player still.

	// Clear the velocity and hold them in place briefly.
	VectorClear(self->velocity);
	self->client->ps.pmove.pm_time = 50;

	// Allow the player to fade out.
	self->s.color.c = 0xffffffff;
	self->s.renderfx |= RF_TRANSLUCENT;

	// Make it so that the stuff that does the de-materialization in G_ANIM_ACTOR knows we are fading out, not in.
	VectorClear(self->client->tele_dest);

	// Tell us how long we have to be a chicken.
	self->morph_timer = (int)level.time + MORPH_DURATION;

	// Draw the teleport splash at the teleport source.
	gi.CreateEffect(&self->s, FX_PLAYER_TELEPORT_OUT, CEF_OWNERS_ORIGIN | CEF_FLAG6, NULL, "");

	// Do the teleport sound.
	gi.sound(self, CHAN_WEAPON, gi.soundindex("weapons/teleport.wav"), 1.0f, ATTN_NORM, 0.0f);
}

edict_t *MorphReflect(edict_t *self, edict_t *other, vec3_t vel)
{
	edict_t	*morph;
	byte 	yaw, pitch;

   	// create a new missile to replace the old one - this is necessary cos physics will do nasty shit
   	// with the existing one,since we hit something. Hence, we create a new one totally.
   	morph = G_Spawn();
   	create_morph(morph);
   	morph->reflect_debounce_time = self->reflect_debounce_time -1;
	morph->reflected_time=self->reflected_time;
   	morph->owner = other;
   	morph->enemy = self->enemy;
   	VectorCopy(self->s.origin, morph->s.origin);
	VectorCopy(vel, morph->velocity);
	VectorNormalize2(morph->velocity, morph->movedir);
   	AnglesFromDir(morph->movedir, morph->s.angles);
   	G_LinkMissile(morph); 
   	yaw = Q_ftol((morph->s.angles[YAW]/6.2831) * 255.0);
   	pitch = Q_ftol((morph->s.angles[PITCH]/6.2831) * 255.0);
   	gi.CreateEffect(&morph->s, FX_SPELL_MORPHMISSILE, CEF_OWNERS_ORIGIN|CEF_FLAG6, NULL, "bb", yaw,pitch);

   	// kill the existing missile, since its a pain in the ass to modify it so the physics won't screw it. 
   	G_SetToFree(self);

   	// Do a nasty looking blast at the impact point
   	gi.CreateEffect(&morph->s, FX_LIGHTNING_HIT, CEF_OWNERS_ORIGIN, NULL, "t", morph->velocity);
	return(morph);
}


// ****************************************************************************
// MorphMissile touch
// ****************************************************************************

// This called when missile touches anything (world or edict)

void MorphMissileTouch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surface)
{

	// has the target got reflection turned on ?
	if(EntReflecting(other, true, true) && self->reflect_debounce_time)
	{
	   	Create_rand_relect_vect(self->velocity, self->velocity);
	   	Vec3ScaleAssign(OVUM_SPEED/2, self->velocity);
	   	MorphReflect(self, other, self->velocity);

		return;
	}

	// Turn target into a chicken if monster or player
	if(((other->svflags & SVF_MONSTER) && !(other->svflags&SVF_BOSS) && !(other->monsterinfo.c_mode)) || 
				((other->client)&&(deathmatch->value)))
	{
		//Don't turn a super chicken back to a player
		if ( (other->client) && (other->client->playerinfo.edictflags & FL_SUPER_CHICKEN) )
		{
			// Turn off the client effect
			gi.sound(other,CHAN_WEAPON,gi.soundindex("misc/null.wav"),1,ATTN_NORM,0);
			gi.CreateEffect(NULL, FX_SPELL_MORPHEXPLODE, 0, self->s.origin, "d", self->movedir);
			G_SetToFree(self);
			return;
		}

		// don't target team members in team deathmatching, if they are on the same team, and friendly fire is not enabled.
		if ((other->client && (int)(dmflags->value) & (DF_MODELTEAMS | DF_SKINTEAMS)) && !((int)dmflags->value & DF_HURT_FRIENDS) && deathmatch->value)
		{
			if (OnSameTeam(other, self->owner))
			{
				// Turn off the client effect
				gi.sound(other,CHAN_WEAPON,gi.soundindex("misc/null.wav"),1,ATTN_NORM,0);
				gi.CreateEffect(NULL, FX_SPELL_MORPHEXPLODE, 0, self->s.origin, "d", self->movedir);
				G_SetToFree(self);
				return;
			}
		}

		if (other->svflags & SVF_MONSTER ) 
		{
			// deal with the existing bad guy
			other->think = MonsterMorphFadeOut;
			other->nextthink = level.time + 0.1;
			other->touch = NULL;
			other->morph_timer = MORPH_TELE_TIME;
			other->enemy = self->owner;
			VectorClear(other->velocity);
			gi.CreateEffect(&other->s, FX_PLAYER_TELEPORT_OUT, CEF_OWNERS_ORIGIN|CEF_FLAG6, NULL, "" ); 
		}
		else
			MorphPlayerToChicken(other, self->owner);

		if (deathmatch->value)
		{
			//There shouldn't be any monsters in deathmatch.. but...
			assert(other->client);

			if ( (other->client) && (other->client->playerinfo.edictflags & FL_SUPER_CHICKEN) )
				gi.sound(other,CHAN_VOICE,gi.soundindex("weapons/supercrow.wav"),1,ATTN_NONE,0);
			else
				gi.sound(other,CHAN_VOICE,gi.soundindex("weapons/crow.wav"),1,ATTN_NONE,0);
		}
		else
		{
			gi.sound(other,CHAN_VOICE,gi.soundindex("weapons/crow.wav"),1,ATTN_NORM,0);
		}

		gi.sound(other,CHAN_WEAPON,gi.soundindex("misc/null.wav"),1,ATTN_NORM,0);
		gi.CreateEffect(NULL, FX_SPELL_MORPHEXPLODE, 0, self->s.origin, "d", self->movedir);
	}
	// else we hit a wall / object
	else
	{
		if(plane && (plane->normal))
			// Start the explosion
			gi.CreateEffect(NULL, FX_SPELL_MORPHEXPLODE, 0, self->s.origin, "d", plane->normal);
		else
			gi.CreateEffect(NULL, FX_SPELL_MORPHEXPLODE, 0, self->s.origin, "d", self->movedir);
	}

	// Turn off the client effect
	G_SetToFree(self);				// Allow time to get to client
}

// ****************************************************************************
// MorphMissile think
// ****************************************************************************

void MorphMissileThink(edict_t *self)
{
	self->svflags |= SVF_NOCLIENT;			// No messages to client after it has received velocity
	self->think = NULL;						// Not required to think anymore
}


// create the guts of the morph ovum
void create_morph(edict_t *morph)
{
	morph->s.effects |= EF_ALWAYS_ADD_EFFECTS;
	morph->svflags |= SVF_ALWAYS_SEND;
	morph->movetype = MOVETYPE_FLYMISSILE;

	// set up our collision boxes
	VectorSet(morph->mins, -OVUM_RADIUS, -OVUM_RADIUS, -OVUM_RADIUS);
	VectorSet(morph->maxs, OVUM_RADIUS, OVUM_RADIUS, OVUM_RADIUS);

	morph->solid = SOLID_BBOX;
	morph->clipmask = MASK_MONSTERSOLID;
	morph->touch = MorphMissileTouch;
	morph->think = MorphMissileThink;
	morph->classname = "Spell_MorphArrow";
	morph->nextthink = level.time + 0.1;
}

// ****************************************************************************
// SpellCastMorph
// ****************************************************************************

void SpellCastMorph(edict_t *Caster, vec3_t StartPos, vec3_t AimAngles, vec3_t unused, float value)
{
	edict_t		*morph;
	int			i;
	byte 			yaw;
	float			current_ang;
	vec3_t		temp_angles;
	short	morpharray[NUM_OF_OVUMS];

//	if (!(Caster->client->playerinfo.edictflags & FL_CHICKEN))
//	{
//		MorphPlayerToChicken2(Caster, Caster);
//		return;
//	}

	// first ovum gets sent out along our aiming angle
	current_ang = AimAngles[YAW];
	for (i=0; i<NUM_OF_OVUMS; i++)
	{
		// create each of the server side entities that are the morph ovum spells
		morph = G_Spawn();
		VectorCopy(StartPos, morph->s.origin);

		// decide its direction
		morph->s.angles[YAW] = current_ang;
		VectorScale(morph->s.angles, ANGLE_TO_RAD, temp_angles);
		DirFromAngles(temp_angles, morph->velocity);
		Vec3ScaleAssign(OVUM_SPEED,morph->velocity);

		create_morph(morph);
		morph->reflect_debounce_time = MAX_REFLECT;
		morph->owner = Caster;
		G_LinkMissile(morph);

		// if we are the first effect, calculate our yaw
		if (!i)
			yaw = Q_ftol((morph->s.angles[YAW]/360.0) * 255.0);
		// Store the entity numbers for sending with the effect.
		morpharray[i] = morph->s.number;

		//increment current angle to get circular radius of ovums
		current_ang+= ANGLE_INC;
	}

	// create the client effect that gets seen on screen
	gi.CreateEffect(&Caster->s, FX_SPELL_MORPHMISSILE_INITIAL, CEF_OWNERS_ORIGIN, NULL, "bssssss", 
			yaw, 
			morpharray[0],
			morpharray[1],
			morpharray[2],
			morpharray[3],
			morpharray[4],
			morpharray[5]);
}

// end

 