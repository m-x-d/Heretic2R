//
// Heretic II
// Copyright 1998 Raven Software
//
// spl_morph.c
//
// Created by John Scott, but written by Jake Simpson and finally rodgered to be work with client
// prediction by Marcus Whitlock.
// "Rodgered" ??? No wonder he was working late :)

#include "g_local.h"
#include "g_monster.h"
#include "g_Physics.h"
#include "g_playstats.h"
#include "g_teleport.h"
#include "g_Skeletons.h"
#include "g_volume_effect.h"
#include "m_chicken.h"
#include "m_chicken_anim.h"
#include "angles.h"
#include "fx.h"
#include "matrix.h"
#include "vector.h"
#include "Utilities.h"
#include "p_main.h"
#include "p_anims.h"
#include "random.h"
#include "h2common.h"

#define ARROW_SPEED			400.0F
#define ARROW_RADIUS			2.0F
#define ANGLE_INC				360/NUM_OF_OVUMS

char	chicken_text[] = "monster_chicken";

extern void ED_CallSpawn (edict_t *ent);
extern void PlayerKillShrineFX(edict_t *self);
extern void SpawnInitialPlayerEffects(edict_t *ent);
extern void MorphPlayerToChicken(edict_t *self, edict_t *caster);

extern	vec3_t	mins;
extern 	vec3_t	maxs;

void create_morph(edict_t *morph);

// *************************************************************************************************
// MorphFadeIn
// -----------
// Fade in the chicken - for MONSTERS only.
// *************************************************************************************************

void MorphFadeIn(edict_t *self)
{
	self->s.color.a += MORPH_TELE_FADE;
	self->nextthink = level.time + 0.1;
	if (!(--self->morph_timer))
	{
		self->think = walkmonster_start_go;
	}
}

// *************************************************************************************************
// MorphFadeOut
// ------------
// Fade out the chicken model till its gone - for MONSTERS only.
// *************************************************************************************************

void MorphFadeOut(edict_t *self)
{
	edict_t	*newent;

	self->s.color.a -= MORPH_TELE_FADE;
	self->nextthink = level.time + 0.1;
	if (!(--self->morph_timer))
	{
		// create the Chicken object
		newent = G_Spawn();
		newent->classname = chicken_text;
		VectorCopy(self->s.origin, newent->s.origin);

		// if we are looking at an original model thats not got an origin at the waist, move us up in the world
		if (self->mins[2] == 0)
			newent->s.origin[2] += 16;

		VectorCopy(self->s.angles, newent->s.angles);
		newent->enemy = self->enemy;
		// keep some info around so we can return to our original persona
		newent->map = self->classname;
		newent->target = self->target;
		// time we stay a chicken
		newent->time = level.time + 20;
		ED_CallSpawn(newent);
		newent->s.color.c = 0xffffff;
		newent->morph_timer = MORPH_TELE_TIME;
		newent->think = MorphFadeIn;
		gi.CreateEffect(&newent->s, FX_PLAYER_TELEPORT_IN, CEF_OWNERS_ORIGIN|CEF_FLAG6, NULL, "" ); 
		// do the teleport sound
		gi.sound(newent,CHAN_WEAPON,gi.soundindex("weapons/teleport.wav"),1,ATTN_NORM,0);

		G_SetToFree(self);
	}
}

// *************************************************************************************************
// CleanUpMorph
// ------------
// Done morphing, clean up after ourselves - for PLAYER only. Called from G_ANIMACTOR.C.
// *************************************************************************************************

void CleanUpMorph(edict_t *self)
{
	self->client->tele_dest[0] = self->client->tele_dest[1] = self->client->tele_dest[2] = 0;
	self->client->tele_count = 0;
	self->client->playerinfo.edictflags &= ~FL_LOCKMOVE;
	self->client->playerinfo.renderfx &= ~RF_TRANSLUCENT;
	self->client->playerinfo.flags &=~PLAYER_FLAG_MORPHING;
	self->client->shrine_framenum = level.time - 1;;
	self->s.color.a = 255;
}

// *************************************************************************************************
// reset_morph_to_elf
// ------------------
// We are done being a chicken, let's be Corvus again - switch models from chicken back to corvus
// and do teleport fade in - for PLAYER only. Called from G_ANIMACTOR.C.
// *************************************************************************************************

void reset_morph_to_elf(edict_t *ent)
{
	// we have no damage, and no motion type
	ent->takedamage = DAMAGE_AIM;
	ent->movetype = PHYSICSTYPE_STEP;
	ent->health = ent->max_health;

	// move the camera back to where it should be, and reset our lungs and stuff
	ent->viewheight = 0;
	ent->mass = 200;
	ent->deadflag = DEAD_NO;
	ent->air_finished = level.time + HOLD_BREATH_TIME;

	ent->s.scale = 1.0;

	// set the model back to corvux
#ifdef COMP_FMOD
	ent->model = "models/player/corvette/tris_c.fm";
#else
	ent->model = "models/player/corvette/tris.fm";
#endif
	ent->pain = player_pain;
	ent->die = player_die;
	ent->flags &= ~FL_NO_KNOCKBACK;
	ent->gravity = 1.0;

	// reset our skins
	ent->client->playerinfo.effects = 0;
	ent->client->playerinfo.skinnum = 0;
	ent->client->playerinfo.clientnum = ent - g_edicts - 1;
	ent->s.modelindex = 255;		// will use the skin specified model
	ent->client->playerinfo.frame = 0;

	// turn our skeleton back on
	ent->s.skeletalType = SKEL_CORVUS;
	ent->client->playerinfo.effects|=(EF_SWAPFRAME|EF_JOINTED|EF_CAMERA_NO_CLIP|EF_PLAYER);
	ent->client->playerinfo.effects&=~EF_CHICKEN;
	ent->client->playerinfo.edictflags &= ~FL_CHICKEN;
	ent->client->playerinfo.renderfx &= ~RF_IGNORE_REFS;

	// reset our mins and max's. And then let the physics move us out of anyone elses bounding box
	VectorCopy (mins, ent->intentMins);
	VectorCopy (maxs, ent->intentMaxs);
	ent->physicsFlags |= PF_RESIZE;

	// reset our thinking
	ent->think = ent->oldthink;
	ent->nextthink = level.time + 0.1;

	// reset our animations
	P_PlayerBasicAnimReset(&ent->client->playerinfo);
	ent->client->playerinfo.upperframe = 43;
	ent->client->playerinfo.lowerframe = 43;

	P_PlayerUpdateModelAttributes(&ent->client->playerinfo);
	P_PlayerAnimSetLowerSeq(&ent->client->playerinfo, ASEQ_NONE);
	P_PlayerAnimSetLowerSeq(&ent->client->playerinfo, ASEQ_IDLE_WIPE_BROW);

	// re-spawn anything that should be - shrine effects and the like
//	SpawnInitialPlayerEffects(ent);

	// draw the teleport splash at the destination
	gi.CreateEffect(&ent->s, FX_PLAYER_TELEPORT_IN, CEF_BROADCAST|CEF_OWNERS_ORIGIN|CEF_FLAG6, ent->s.origin, "");

	// restart the loop and tell us next time we aren't de-materialising
	ent->client->tele_count = TELE_TIME;
	ent->client->tele_dest[0] = ent->client->tele_dest[1] = ent->client->tele_dest[2] = -1;

}

// *************************************************************************************************
// MorphChickenToPlayer
// --------------------
// Modify a chicken into a player - first call. Start the teleport effect on the chicken.
// For PLAYER only.
// *************************************************************************************************

void MorphChickenToPlayer(edict_t *self)
{
	gclient_t	*playerinfo;
	
	playerinfo = self->client;

	// if we are teleporting or morphing, forget it
	if (self->client->playerinfo.flags & (PLAYER_FLAG_TELEPORT | PLAYER_FLAG_MORPHING))
		return;

	// set the player as teleporting
	self->client->playerinfo.flags |= PLAYER_FLAG_MORPHING;

	// time taken over dematerialisation
	self->client->tele_count = TELE_TIME_OUT;

	// make us invunerable for a couple of seconds
	self->client->shrine_framenum = level.time + 10;

	// tell us how we triggered the teleport
	self->client->tele_type = 1;

	// clear the velocity and hold them in place briefly
	VectorClear (self->velocity);
	self->client->ps.pmove.pm_time = 50;
	// make the player still
	self->flags |= FL_LOCKMOVE;
	// allow the player to fade out
	self->s.color.a = 255;
	self->s.color.r = 255;
	self->s.color.g = 255;
	self->s.color.b = 255;
	self->s.renderfx |= RF_TRANSLUCENT;

	// make us not think at all
	self->think = NULL;

	// make it so that the stuff that does the demateriasation in G_ANIM_ACTOR knows we are fading out, not in
	self->client->tele_dest[0] = self->client->tele_dest[1] = self->client->tele_dest[2] = 0;

	// draw the teleport splash at the teleport source
	gi.CreateEffect(&self->s, FX_PLAYER_TELEPORT_OUT, CEF_OWNERS_ORIGIN |CEF_FLAG6, NULL, "" );
	// do the teleport sound
	gi.sound(self,CHAN_WEAPON,gi.soundindex("weapons/teleport.wav"),1,ATTN_NORM,0);


}

// *************************************************************************************************
// watch_chicken
// -------------
// Watch the chicken to see if we should become the elf again. For PLAYER only.
// *************************************************************************************************

void watch_chicken(edict_t *self)
{

	// are we done yet ?
	if (self->morph_timer <= level.time)
	{
		MorphChickenToPlayer(self);
	}
	self->nextthink = level.time + 0.1;
}

// *************************************************************************************************
// Perform_Morph
// ------------
// Switch the models from player to chicken and then make us re-appear ala teleport. For PLAYER
// only. Called from G_ANIMACTOR.C.
// *************************************************************************************************

void Perform_Morph(edict_t *self)
{
	qboolean	super_chicken = false;
	trace_t		trace;
	vec3_t		mins = { -16, -16, -36};
	vec3_t		maxs = {  16,  16,  36};
	vec3_t		pos;
	int			i;

	// change out our model
	self->model = "models/monsters/chicken2/tris.fm";
	self->s.modelindex = gi.modelindex("models/monsters/chicken2/tris.fm");

	self->client->playerinfo.effects &= ~(EF_JOINTED|EF_SWAPFRAME);
	self->client->playerinfo.effects |= EF_CHICKEN;
	self->s.skeletalType = SKEL_NULL;
	self->client->playerinfo.renderfx |= RF_IGNORE_REFS;

	if (!irand(0,10))
		super_chicken = true;

	if (super_chicken)
	{
		VectorCopy(self->s.origin, pos);
		pos[2] += 2;

		gi.trace(pos, mins, maxs, pos, self, MASK_PLAYERSOLID,&trace);

		if (trace.fraction < 1 || trace.startsolid || trace.allsolid)
			super_chicken = false;
	}
	
	if (super_chicken)
	{
		// reset our motion stuff
		self->health = 999;
		self->mass = 3000;
		self->yaw_speed = 30;
		self->gravity = 1.0;

		self->monsterinfo.scale = 2.5;
		self->s.scale = 2.5;

		VectorSet(self->mins, -16, -16, -48);
		VectorSet(self->maxs,  16,  16,  64);

		self->client->playerinfo.edictflags |= FL_SUPER_CHICKEN;
	}
	else
	{
		self->health = 1;
		self->mass = 30;
		self->yaw_speed = 20;
		self->gravity = 0.6;

		self->monsterinfo.scale = MODEL_SCALE;

		// new mins and max's too
		VectorSet(self->intentMins,-8,-8,-12);
		VectorSet(self->intentMaxs,8,8,12);

		self->client->playerinfo.edictflags |= FL_AVERAGE_CHICKEN;
	}

	// not being knocked back, and stepping like a chicken
	self->movetype = PHYSICSTYPE_STEP;
	VectorClear(self->knockbackvel);

	// reseting which skin we use, and new scale
	self->client->playerinfo.skinnum = 0;
	self->client->playerinfo.clientnum = self - g_edicts - 1;

	// reset our thinking
	self->oldthink = self->think;
	self->think = watch_chicken;
	self->nextthink = level.time + 0.1;

	self->physicsFlags |= PF_RESIZE;

	for (i=0;i<MAX_FM_MESH_NODES;i++)
		self->client->playerinfo.fmnodeinfo[i].flags &= ~FMNI_NO_DRAW;

	// reset our animation
	P_PlayerAnimSetLowerSeq(&self->client->playerinfo, ASEQ_STAND);

	// draw the teleport splash at the destination
	gi.CreateEffect(&self->s, FX_PLAYER_TELEPORT_IN, CEF_BROADCAST|CEF_OWNERS_ORIGIN|CEF_FLAG6, self->s.origin, "");

	// restart the loop and tell us next time we aren't de-materialising
	self->client->tele_count = TELE_TIME;
	self->client->tele_dest[0] = self->client->tele_dest[1] = self->client->tele_dest[2] = -1;
}

// *************************************************************************************************
// MorphPlayerToChicken
// --------------------
// Modify a player into a chicken - first call. Start the teleport effect on the player. For PLAYER
// only.
// *************************************************************************************************

void MorphPlayerToChicken(edict_t *self, edict_t *caster)
{
	gclient_t	*playerinfo;
	
	playerinfo = self->client;

	// if we are teleporting or morphing, forget it
	if (self->client->playerinfo.flags & (PLAYER_FLAG_TELEPORT | PLAYER_FLAG_MORPHING))
		return;

	// remove any hand or weapon effects
	P_TurnOffPlayerEffects(&self->client->playerinfo);

	// remove any shrine effects he has
	PlayerKillShrineFX(self);

	// set the player as teleporting
	self->client->playerinfo.flags |= PLAYER_FLAG_MORPHING;

	// time taken over dematerialisation
	self->client->tele_count = TELE_TIME_OUT;

	// make us invunerable for a couple of seconds
	self->client->shrine_framenum = level.time + 10;

	// tell us how we triggered the teleport
	self->client->tele_type = 1;

	// clear the velocity and hold them in place briefly
	VectorClear (self->velocity);
	self->client->ps.pmove.pm_time = 50;

	// make the player still
	self->client->playerinfo.flags |= FL_LOCKMOVE;

	// allow the player to fade out
	self->s.color.a = 255;
	self->s.color.r = 255;
	self->s.color.g = 255;
	self->s.color.b = 255;
	self->s.renderfx |= RF_TRANSLUCENT;

	// make it so that the stuff that does the demateriasation in G_ANIM_ACTOR knows we are fading out, not in
	self->client->tele_dest[0] = self->client->tele_dest[1] = self->client->tele_dest[2] = 0;

	// tell us how long we have to be a chicken
	self->morph_timer = level.time + MORPH_DUR;

	// draw the teleport splash at the teleport source
	gi.CreateEffect(&self->s, FX_PLAYER_TELEPORT_OUT, CEF_OWNERS_ORIGIN | CEF_FLAG6, NULL, "");
		// do the teleport sound
	gi.sound(self,CHAN_WEAPON,gi.soundindex("weapons/teleport.wav"),1,ATTN_NORM,0);

}

// *************************************************************************************************
// MorphPlayerToChicken2
// ---------------------
// Modify a player into a chicken - first call. Start the teleport effect on the player. For PLAYER
// only. Temporary func. See Marcus for explaination.
// *************************************************************************************************

void MorphPlayerToChicken2(edict_t *self, edict_t *caster)
{
	gclient_t	*playerinfo;
	
	playerinfo = self->client;

	// if we are teleporting or morphing, forget it
	if (self->client->playerinfo.flags & (PLAYER_FLAG_TELEPORT | PLAYER_FLAG_MORPHING))
		return;

	// remove any hand or weapon effects
	P_TurnOffPlayerEffects(&self->client->playerinfo);

	// remove any shrine effects he has
	PlayerKillShrineFX(self);

	// set the player as teleporting
	self->client->playerinfo.flags |= PLAYER_FLAG_MORPHING;

	// time taken over dematerialisation
	self->client->tele_count = TELE_TIME_OUT;

	// make us invunerable for a couple of seconds
	self->client->shrine_framenum = level.time + 10;

	// tell us how we triggered the teleport
	self->client->tele_type = 1;

	// clear the velocity and hold them in place briefly
	VectorClear (self->velocity);
	self->client->ps.pmove.pm_time = 50;

	// make the player still
	self->client->playerinfo.flags |= FL_LOCKMOVE;

	// allow the player to fade out
	self->s.color.a = 255;
	self->s.color.r = 255;
	self->s.color.g = 255;
	self->s.color.b = 255;

	self->client->playerinfo.renderfx |= RF_TRANSLUCENT;

	// make it so that the stuff that does the demateriasation in G_ANIM_ACTOR knows we are fading out, not in
	self->client->tele_dest[0] = self->client->tele_dest[1] = self->client->tele_dest[2] = 0;

	// tell us how long we have to be a chicken
	self->morph_timer = level.time + MORPH_DUR;

	// draw the teleport splash at the teleport source
	gi.CreateEffect(&self->s, FX_PLAYER_TELEPORT_OUT, CEF_OWNERS_ORIGIN | CEF_FLAG6, NULL, "");
		// do the teleport sound
	gi.sound(self,CHAN_WEAPON,gi.soundindex("weapons/teleport.wav"),1,ATTN_NORM,0);

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
	   	Vec3ScaleAssign(ARROW_SPEED/2, self->velocity);
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
			other->think = MorphFadeOut;
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
	VectorSet(morph->mins, -ARROW_RADIUS, -ARROW_RADIUS, -ARROW_RADIUS);
	VectorSet(morph->maxs, ARROW_RADIUS, ARROW_RADIUS, ARROW_RADIUS);

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
		Vec3ScaleAssign(ARROW_SPEED,morph->velocity);

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

 