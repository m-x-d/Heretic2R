//
// m_priestess.c
//
// Copyright 1998 Raven Software
//

#include <float.h> //mxd
#include "m_priestess.h"
#include "m_priestess_shared.h"
#include "m_priestess_anim.h"
#include "g_DefaultMessageHandler.h"
#include "g_monster.h"
#include "mg_guide.h" //mxd
#include "m_stats.h"
#include "Random.h"
#include "Utilities.h"
#include "Vector.h"
#include "g_local.h"

static void create_priestess_proj(const edict_t* self, edict_t* proj); //TODO: remove.

// Number of frames the priestess is in the air.
#define PRIESTESS_JUMP_FRAMES	10.0f //mxd. Named 'PRIESTESS_JUMPFRAMES' in original logic.
#define PRIESTESS_HOP_DISTANCE	0.0f //mxd. Named 'PRIESTESS_HOPDIST' in original logic.

#pragma region ========================== High Priestess Base Info ==========================

static const animmove_t* animations[NUM_ANIMS] =
{
	&priestess_move_stand1,
	&priestess_move_attack1_go,
	&priestess_move_attack1_loop,
	&priestess_move_attack1_end,
	&priestess_move_attack2,
	&priestess_move_backup,
	&priestess_move_death,
	&priestess_move_idle,
	&priestess_move_jump,
	&priestess_move_pain,
	&priestess_move_idle_pose,
	&priestess_move_pose_trans,
	&priestess_move_shield_go,
	&priestess_move_shield_end,
	&priestess_move_dodge_left,
	&priestess_move_dodge_right,
	&priestess_move_walk,
	&priestess_move_jump_forward,
	&priestess_move_jump_back,
	&priestess_move_jump_right,
	&priestess_move_jump_left,
	&priestess_move_jump_pounce,
	&priestess_move_pounce_attack,
	&priestess_move_attack3_go,
	&priestess_move_attack3_loop,
	&priestess_move_attack3_end,
	&priestess_move_jump_attack
};

static int sounds[NUM_SOUNDS];

enum HighPriestessAttackStates_e
{
	AS_QUEENS_FURY,
	AS_BROODS_SACRIFICE,
	AS_HEAVENS_RAIN,
	AS_LIGHT_MISSILE,
	AS_POUNCE,
	AS_JUMP_RIGHT,
	AS_JUMP_LEFT,
};

#pragma endregion

void priestess_teleport_go(edict_t* self)
{
	self->takedamage = DAMAGE_NO;
	gi.sound(self, CHAN_AUTO, sounds[SND_TPORT_OUT], 1.0f, ATTN_NORM, 0.0f);
	gi.CreateEffect(NULL, FX_HP_MISSILE, 0, self->s.origin, "vb", self->s.origin, HPTELEPORT_START);
}

void priestess_teleport_end(edict_t* self)
{
	gi.sound(self, CHAN_AUTO, sounds[SND_TPORT_IN], 1.0f, ATTN_NORM, 0.0f);
	gi.CreateEffect(NULL, FX_HP_MISSILE, 0, self->s.origin, "vb", self->s.origin, HPTELEPORT_END);
}

void priestess_teleport_move(edict_t* self)
{
	const vec3_t mins = { -24.0f, -24.0f, -36.0f };
	const vec3_t maxs = { 24.0f, 24.0f, 36.0f }; //BUGFIX: mxd. Same as mins in original logic.

	float best_dist = FLT_MAX; //mxd. 9999999 in original logic.

	edict_t* path_corner = NULL;
	const edict_t* best_corner = NULL;
	while ((path_corner = G_Find(path_corner, FOFS(classname), "path_corner")) != NULL)
	{
		if (Q_stricmp(path_corner->targetname, "priestess") != 0) //mxd. stricmp -> Q_stricmp.
			continue;

		const float enemy_dist = vhlen(self->enemy->s.origin, path_corner->s.origin);
		const float start_dist = vhlen(self->s.origin, path_corner->s.origin);

		if (enemy_dist < 64.0f || start_dist < 64.0f || enemy_dist >= best_dist || !AI_IsVisible(path_corner, self->enemy))
			continue;

		vec3_t test_pos;
		VectorCopy(path_corner->s.origin, test_pos);
		test_pos[2] += maxs[2];

		trace_t	trace;
		gi.trace(test_pos, mins, maxs, test_pos, self, MASK_MONSTERSOLID, &trace);

		if (trace.startsolid || trace.allsolid)
			continue;

		if (trace.ent != NULL && Q_stricmp(trace.ent->classname, "player") == 0) //mxd. stricmp -> Q_stricmp.
			continue;

		best_dist = enemy_dist;
		best_corner = path_corner;
	}

	if (best_corner != NULL)
	{
		// ULTRA HACK!
		VectorCopy(best_corner->s.origin, self->monsterinfo.nav_goal);
		self->s.origin[0] += 2000.0f; //TODO: is there better way to hide her?..
		gi.linkentity(self);

		// Spawn a fake entity to sit where the priestess will teleport to assure there's no telefragging.
		edict_t* blocker = G_Spawn();

		VectorCopy(mins, blocker->mins);
		VectorCopy(maxs, blocker->maxs);

		blocker->solid = SOLID_BBOX;
		blocker->movetype = PHYSICSTYPE_NONE;

		//TODO: if the player touches this entity somehow, he's thrown back.
		self->movetarget = blocker; //TODO: add priestess_teleport_blocker name.

		gi.linkentity(blocker);
	}
	else
	{
		SetAnim(self, ANIM_SHIELD_END);
	}
}

void priestess_teleport_self_effects(edict_t* self)
{
	self->s.renderfx |= RF_ALPHA_TEXTURE;
	self->s.color.c = 0xffffffff;
}

void priestess_delta_alpha(edict_t* self, float amount)
{
	const int alpha = self->s.color.a + (int)amount;
	self->s.color.a = (byte)ClampI(alpha, 0, 255);
}

void priestess_stop_alpha(edict_t* self)
{
	self->takedamage = DAMAGE_YES;
	self->s.renderfx &= ~RF_ALPHA_TEXTURE;
	self->s.color.c = 0xffffffff;
}

void priestess_teleport_return(edict_t* self)
{
	if (self->movetarget != NULL && self->movetarget != self->enemy) // Free the teleport blocker entity.
		G_FreeEdict(self->movetarget);

	vec3_t start;
	VectorCopy(self->monsterinfo.nav_goal, start);
	start[2] += 36.0f;

	vec3_t end;
	VectorCopy(self->monsterinfo.nav_goal, end);
	end[2] -= 128.0f;

	trace_t trace;
	gi.trace(start, self->mins, self->maxs, end, self, MASK_MONSTERSOLID, &trace);

	if (trace.allsolid || trace.startsolid)
	{
		// The priestess has become lodged in something!
		assert(0); //TODO: handle this... somehow. Try picking different path corner?
		return;
	}

	VectorCopy(trace.endpos, self->s.origin);
	gi.linkentity(self);

	SetAnim(self, ANIM_SHIELD_END);
}

static void PriestessProjectile1DrunkenThink(edict_t* self) //mxd. Named 'priestess_proj1_drunken' in original logic.
{
	VectorRandomCopy(self->velocity, self->velocity, 64.0f);
	self->nextthink = level.time + FRAMETIME; //mxd. Use define.
}

/*-----------------------------------------------
	priestess_proj1_think
-----------------------------------------------*/

void priestess_proj1_think( edict_t *self )
{
	vec3_t olddir, newdir, huntdir;
	float oldvelmult , newveldiv, speed_mod;

	//No enemy, stop tracking
	if (!self->enemy)
	{
		self->think = NULL;
		return;
	}

	//Enemy is dead, stop tracking
	if (self->enemy->health <= 0)
	{
		self->think = NULL;
		return;
	}

	//Timeout?
	if (self->monsterinfo.attack_finished < level.time)
	{
		gi.sound (self, CHAN_BODY, sounds[SND_BALLHIT], 1, ATTN_NORM, 0);

		gi.CreateEffect(&self->s,
					FX_HP_MISSILE,
					CEF_OWNERS_ORIGIN,
					self->s.origin,
					"vb",
					vec3_origin,
					HPMISSILE1_EXPLODE);

		self->think = G_FreeEdict;
		self->nextthink = level.time + 0.1;
		
		return;
	}

	VectorCopy(self->velocity, olddir);
	VectorNormalize(olddir);

	VectorSubtract(self->enemy->s.origin, self->s.origin, huntdir);
	VectorNormalize(huntdir);

	oldvelmult = 1.2;
	newveldiv = 1/(oldvelmult + 1);
	
	VectorScale(olddir, oldvelmult, olddir);
	VectorAdd(olddir, huntdir, newdir);
	VectorScale(newdir, newveldiv, newdir);

	speed_mod = DotProduct( olddir , newdir );

	if (speed_mod < 0.05)
		speed_mod = 0.05;

	newveldiv *= self->ideal_yaw * speed_mod;

	VectorScale(olddir, oldvelmult, olddir);
	VectorAdd(olddir, huntdir, newdir);
	VectorScale(newdir, newveldiv, newdir);

	VectorCopy(newdir, self->velocity);
	self->nextthink = level.time + 0.1;
}

/*-----------------------------------------------
	priestess_proj2_die
-----------------------------------------------*/

static void priestess_proj2_die(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	gi.sound (self, CHAN_AUTO, sounds[SND_BUGHIT], 1, ATTN_NORM, 0);

	gi.CreateEffect(&self->s,
				FX_HP_MISSILE,
				CEF_OWNERS_ORIGIN,
				self->s.origin,
				"vb",
				vec3_origin,
				HPMISSILE3_EXPLODE);

	self->think = G_FreeEdict;
	self->nextthink = level.time + 0.1;
}

/*-----------------------------------------------
	priestess_proj2_think
-----------------------------------------------*/

void priestess_proj2_think( edict_t *self )
{
	//Timeout?
	if (self->monsterinfo.attack_finished < level.time)
	{
		gi.sound (self, CHAN_AUTO, sounds[SND_BUGHIT], 1, ATTN_NORM, 0);
	
		gi.CreateEffect(&self->s,
					FX_HP_MISSILE,
					CEF_OWNERS_ORIGIN,
					self->s.origin,
					"vb",
					vec3_origin,
					HPMISSILE3_EXPLODE);

		self->think = G_FreeEdict;
		self->nextthink = level.time + 0.1;
		
		return;
	}

	VectorScale(self->velocity, self->missile_range, self->velocity);

	self->velocity[0] += irand(-8, 8);
	self->velocity[1] += irand(-8, 8);
	self->velocity[2] += irand(-8, 8);

	self->nextthink = level.time + 0.1;
}

/*-----------------------------------------------
	priestess_proj1_blocked
-----------------------------------------------*/

void priestess_proj1_blocked( edict_t *self, trace_t *trace )
{	
	edict_t	*proj;
	vec3_t	hitDir;
	int		damage;
	byte	exp;

	if (trace->ent == self->owner)
		return;

	if (!stricmp(trace->ent->classname, "HPriestess_Missile"))
		return;

	//Reflection stuff
	if(EntReflecting(trace->ent, true, true))
	{
		proj = G_Spawn();

		create_priestess_proj(self,proj);
		proj->owner = self->owner;
		proj->ideal_yaw = self->ideal_yaw;

		Create_rand_relect_vect(self->velocity, proj->velocity);
		Vec3ScaleAssign(proj->ideal_yaw,proj->velocity);
		vectoangles(proj->velocity, proj->s.angles);

		switch ( self->monsterinfo.attack_state )
		{
		case AS_QUEENS_FURY:
		case AS_LIGHT_MISSILE:
			exp = HPMISSILE1_EXPLODE;
			break;
		
		case AS_BROODS_SACRIFICE:
			exp = HPMISSILE3_EXPLODE;
			break;

		case AS_HEAVENS_RAIN:
			exp = HPMISSILE1_EXPLODE;
			break;
		}

		gi.CreateEffect(&self->s,
					FX_HP_MISSILE,
					CEF_OWNERS_ORIGIN,
					self->s.origin,
					"vb",
					vec3_origin,
					(unsigned char) exp);

		gi.linkentity(proj); 

		G_SetToFree(self);

		return;
	}

	//Do the rest of the stuff
	switch ( self->monsterinfo.attack_state )
	{
	case AS_QUEENS_FURY:
		exp = HPMISSILE1_EXPLODE;
		damage = irand(HP_DMG_FURY_MIN, HP_DMG_FURY_MAX);
		gi.sound (self, CHAN_AUTO, sounds[SND_HOMINGHIT], 1, ATTN_NORM, 0);
		break;
	
	case AS_BROODS_SACRIFICE:
		exp = HPMISSILE3_EXPLODE;
		damage = irand(HP_DMG_BROOD_MIN, HP_DMG_BROOD_MAX);
		gi.sound (self, CHAN_AUTO, sounds[SND_BUGHIT], 1, ATTN_NORM, 0);
		break;

	case AS_HEAVENS_RAIN:
		exp = HPMISSILE2_EXPLODE;
		damage = HP_DMG_RAIN;
		gi.sound (self, CHAN_AUTO, sounds[SND_ZAPHIT], 1, ATTN_NORM, 0);
		break;

	case AS_LIGHT_MISSILE:
		exp = HPMISSILE1_EXPLODE;
		damage = irand(HP_DMG_MISSILE_MIN, HP_DMG_MISSILE_MAX);
		gi.sound (self, CHAN_AUTO, sounds[SND_BALLHIT], 1, ATTN_NORM, 0);		
		break;

	default:
		assert(0);
		break;
	}

	if ( trace->ent->takedamage )
	{
		VectorCopy( self->velocity, hitDir );
		VectorNormalize( hitDir );

		T_Damage( trace->ent, self, self->owner, hitDir, self->s.origin, trace->plane.normal, damage, 0, DAMAGE_SPELL | DAMAGE_NO_KNOCKBACK,MOD_DIED );
	}

	gi.CreateEffect(&self->s,
				FX_HP_MISSILE,
				CEF_OWNERS_ORIGIN,
				self->s.origin,
				"vb",
				vec3_origin,
				(unsigned char) exp);

	self->think = G_FreeEdict;
	self->nextthink = level.time + 0.1;
}

/*-----------------------------------------------
			priestess_proj1_touch
-----------------------------------------------*/

void priestess_proj1_touch( edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surface )
{
	vec3_t	hitDir;
	byte	exp;
	int		damage;

	if (other == self->owner)
		return;

	if (!stricmp(other->classname, "HPriestess_Missile"))
		return;

	//Do the rest of the stuff
	switch ( self->monsterinfo.attack_state )
	{
	case AS_QUEENS_FURY:
		exp = HPMISSILE1_EXPLODE;
		damage = irand(HP_DMG_FURY_MIN, HP_DMG_FURY_MAX);
		gi.sound (self, CHAN_AUTO, sounds[SND_HOMINGHIT], 1, ATTN_NORM, 0);
		break;
	
	case AS_BROODS_SACRIFICE:
		exp = HPMISSILE3_EXPLODE;
		damage = irand(HP_DMG_BROOD_MIN, HP_DMG_BROOD_MAX);
		gi.sound (self, CHAN_AUTO, sounds[SND_BUGHIT], 1, ATTN_NORM, 0);
		break;

	case AS_HEAVENS_RAIN:
		exp = HPMISSILE2_EXPLODE;
		damage = HP_DMG_RAIN;
		gi.sound (self, CHAN_AUTO, sounds[SND_ZAPHIT], 1, ATTN_NORM, 0);
		break;

	case AS_LIGHT_MISSILE:
		exp = HPMISSILE1_EXPLODE;
		damage = irand(HP_DMG_MISSILE_MIN, HP_DMG_MISSILE_MAX);
		gi.sound (self, CHAN_AUTO, sounds[SND_BALLHIT], 1, ATTN_NORM, 0);
		break;

	default:
		assert(0);
		break;
	}

	if ( other->takedamage )
	{
		VectorCopy( self->velocity, hitDir );
		VectorNormalize( hitDir );

		T_Damage( other, self, self->owner, hitDir, self->s.origin, plane->normal, damage, 0, DAMAGE_SPELL | DAMAGE_NO_KNOCKBACK,MOD_DIED );
	}

	if (other == self->owner)
		return;

	if (!stricmp(other->classname, "HPriestess_Missile"))
		return;

	self->think = G_FreeEdict;
	self->nextthink = level.time + 0.1;
}

/*-----------------------------------------------
	create_priestess_proj
-----------------------------------------------*/

// create the guts of the high priestess projectile
static void create_priestess_proj(const edict_t *self,edict_t *proj)
{
	proj->svflags |= SVF_ALWAYS_SEND;
	proj->movetype = PHYSICSTYPE_FLY;
	proj->gravity = 0;
	proj->solid = SOLID_BBOX;
	proj->classname = "HPriestess_Missile";
	proj->dmg = 1.0;
	proj->s.scale = 1.0;
	proj->clipmask = MASK_SHOT;
	proj->nextthink = level.time + 0.1;
	
	proj->bounced = priestess_proj1_blocked;
	proj->isBlocking = priestess_proj1_blocked;
	proj->isBlocked = priestess_proj1_blocked;
	//proj->touch = priestess_proj1_touch;

	proj->s.effects=EF_MARCUS_FLAG1|EF_CAMERA_NO_CLIP;
	proj->enemy = self->enemy;

	VectorSet(proj->mins, -2.0, -2.0, -2.0);	
	VectorSet(proj->maxs,  2.0,  2.0,  2.0);
	VectorCopy(self->s.origin, proj->s.origin);
}

/*-----------------------------------------------
	priestess_fire1
-----------------------------------------------*/

//Hand thrown light missiles
void priestess_fire1( edict_t *self, float pitch_ofs, float yaw_ofs, float roll_ofs )
{
	edict_t	*proj;
	vec3_t	vf, vr, predPos;
	vec3_t	ang, vel, startOfs, angles;
	int		i;

	if (!self->enemy)
		return;

	//Only predict once for all the missiles
	M_PredictTargetPosition ( self->enemy, self->enemy->velocity, 1, predPos );

	AngleVectors(self->s.angles, vf, vr, NULL);

	VectorMA(self->s.origin, -8,  vf, startOfs);
	VectorMA(startOfs, -16, vr, startOfs);
	startOfs[2] += 32;

	VectorSubtract(predPos, startOfs, vf);
	VectorNormalize(vf);
	
	vectoangles( vf, angles );

	i = irand(2,3);

	while (i--)
	{
		// Spawn the projectile
		proj = G_Spawn();

		proj->monsterinfo.attack_state = AS_LIGHT_MISSILE;
		create_priestess_proj(self,proj);
		proj->owner = self;
		
		VectorCopy(startOfs, proj->s.origin);
		VectorCopy(angles, ang);

		ang[PITCH]  = flrand( -4, 4 ) + -ang[PITCH];
		ang[YAW] 	+= flrand( -4, 4 );

		AngleVectors( ang, vel, NULL, NULL );

		VectorScale(vel, irand(500,600), proj->velocity);

		vectoangles(proj->velocity, proj->s.angles);

		gi.sound (self, CHAN_AUTO, sounds[SND_3BALLATK], 1, ATTN_NORM, 0);

		//One in ten wander off drunkenly
		if (!irand(0,10))
			proj->think = PriestessProjectile1DrunkenThink;

		gi.CreateEffect(&proj->s,
					FX_HP_MISSILE,
					CEF_OWNERS_ORIGIN,
					NULL,
					"vb",
					proj->velocity,
					HPMISSILE2);

		gi.linkentity(proj); 
	}
}

/*-----------------------------------------------
	priestess_fire2
-----------------------------------------------*/

//Tracking, anime style missiles
void priestess_fire2( edict_t *self, float pitch_ofs, float yaw_ofs, float roll_ofs )
{
	edict_t	*proj;
	vec3_t	vf, vr, ang;

	// Spawn the projectile

	proj = G_Spawn();

	create_priestess_proj(self,proj);

	proj->monsterinfo.attack_state = AS_QUEENS_FURY;
	proj->owner = self;
	
	AngleVectors(self->s.angles, vf, vr, NULL);

	VectorCopy(self->s.origin, proj->s.origin);
	
	VectorMA(self->s.origin, 30, vf, proj->s.origin);
	VectorMA(proj->s.origin, 8,	 vr, proj->s.origin);
	proj->s.origin[2] += 56;

	proj->ideal_yaw = 400;

	vectoangles( vf, ang );

	ang[PITCH] -= irand(   5, 75 );
	ang[YAW]   += irand( -60, 60 );

	AngleVectors( ang, vf, NULL, NULL );

	VectorScale( vf, proj->ideal_yaw, proj->velocity );

	proj->monsterinfo.attack_finished= level.time + 2;

	vectoangles(proj->velocity, proj->s.angles);

	if (!irand(0,15))
		proj->think = PriestessProjectile1DrunkenThink;
	else
		proj->think=priestess_proj1_think;

	gi.sound (self, CHAN_AUTO, sounds[SND_HOMINGATK], 1, ATTN_NORM, 0);

	gi.CreateEffect(&proj->s,
				FX_HP_MISSILE,
				CEF_OWNERS_ORIGIN,
				proj->s.origin,
				"vb",
				proj->s.origin,
				HPMISSILE1);

	gi.linkentity(proj); 
}

/*-----------------------------------------------
	priestess_fire3
-----------------------------------------------*/

//The light bugs
void priestess_fire3( edict_t *self, float pitch_ofs, float yaw_ofs, float roll_ofs )
{
	edict_t	*proj;
	vec3_t	vf, vr, ang, startPos;
	float	len;

	// Spawn the projectile

	proj = G_Spawn();

	create_priestess_proj(self,proj);

	proj->takedamage = DAMAGE_YES;
	proj->die = priestess_proj2_die;

	proj->monsterinfo.attack_state = AS_BROODS_SACRIFICE;
	proj->owner = self;
	
	AngleVectors(self->s.angles, vf, vr, NULL);
	VectorNormalize(vf);

	VectorCopy(self->s.origin, proj->s.origin);
	
	VectorMA(self->s.origin, 30, vf, proj->s.origin);
	VectorMA(proj->s.origin, 8,	 vr, proj->s.origin);
	proj->s.origin[2] += 56;
	VectorCopy(proj->s.origin, startPos);

	len = M_DistanceToTarget(self, self->enemy);

	len /= 200;

	proj->ideal_yaw = irand(500*len,750*len);
	proj->missile_range = flrand(0.65, 0.75);

	VectorSubtract(self->enemy->s.origin, proj->s.origin, vf);
	VectorNormalize(vf);

	vectoangles( vf, ang );

	ang[PITCH] *= -1;

	ang[PITCH] += irand( -10, 5 );
	ang[YAW]   += irand( -35, 35 );

	AngleVectors( ang, vf, NULL, NULL );

	VectorScale( vf, proj->ideal_yaw, proj->velocity );

	proj->monsterinfo.attack_finished= level.time + 5;

	vectoangles(proj->velocity, proj->s.angles);

	proj->think=priestess_proj2_think;

	gi.sound (self, CHAN_AUTO, sounds[SND_BUGS], 1, ATTN_NORM, 0);

	gi.CreateEffect(&proj->s,
				FX_HP_MISSILE,
				CEF_OWNERS_ORIGIN,
				startPos,
				"vb",
				proj->velocity,
				HPMISSILE3);

	gi.linkentity(proj); 
}

/*-----------------------------------------------
	priestess_fire4
-----------------------------------------------*/

//Big special light show of doom and chaos and destruction... or something...
void priestess_fire4( edict_t *self, float pitch_ofs, float yaw_ofs, float roll_ofs )
{
	trace_t trace;
	vec3_t	vf, vr, /*ang,*/ startPos, endPos;
	vec3_t  mins = { -1, -1, -1 };
	vec3_t  maxs = {  1,  1,  1 };
	float	len;

	if (self->monsterinfo.sound_finished < level.time)
	{
		gi.sound (self, CHAN_AUTO, sounds[SND_ZAP], 1, ATTN_NORM, 0);
		self->monsterinfo.sound_finished = level.time + 5;
	}

	AngleVectors(self->s.angles, vf, vr, NULL);

	VectorCopy(self->s.origin, startPos);
	
	VectorMA(self->s.origin, 30, vf, startPos);
	VectorMA(startPos, 8, vr, startPos);
	startPos[2] += 56;

	//The 5 to 8 effects are spawn on the other side, no reason to send each one
	gi.CreateEffect(NULL,
				FX_HP_MISSILE,
				0,
				startPos,
				"vb",
				startPos,
				HPMISSILE4);
	
	AngleVectors(self->s.angles, vf, vr, NULL);

	VectorCopy(self->s.origin, startPos);
	
	VectorMA(self->s.origin, 30, vf, startPos);
	VectorMA(startPos, 8, vr, startPos);
	startPos[2] += 56;

	if ( self->monsterinfo.misc_debounce_time < level.time )
	{
		VectorSubtract(self->enemy->s.origin, startPos, vf);
		len = VectorNormalize(vf);

		VectorMA( startPos, len, vf, endPos );

		gi.trace( startPos, mins, maxs, endPos, self, MASK_SHOT ,&trace);

		if (trace.ent == self->enemy)
		{
			T_Damage(trace.ent, self, self, vf, trace.endpos, trace.plane.normal, 
					irand(HP_DMG_FIRE_MIN, HP_DMG_FIRE_MAX), 0, DAMAGE_DISMEMBER,MOD_DIED);

			gi.sound (self, CHAN_AUTO, sounds[SND_ZAPHIT], 1, ATTN_NORM, 0);
		}

		gi.CreateEffect(NULL,
					FX_HP_MISSILE,
					0,
					startPos,
					"vb",
					trace.endpos,
					HPMISSILE5);

		self->monsterinfo.misc_debounce_time = level.time + flrand(0.2, 0.4);
	}
}








































/*

	Priestess Helper Functions

*/

/*-----------------------------------------------
	priestess_attack1_pause
-----------------------------------------------*/

void priestess_attack1_pause( edict_t *self )
{
	if (self->monsterinfo.search_time--)
	{
	}
	else
	{
		priestess_pause(self);
	}
}

/*-----------------------------------------------
	priestess_attack3_loop 
-----------------------------------------------*/

void priestess_attack3_loop ( edict_t *self )
{
	vec3_t	spawnSpot, vf, vr;

	SetAnim(self, ANIM_ATTACK3_LOOP);
	self->monsterinfo.attack_finished = level.time + 4;

	AngleVectors(self->s.angles, vf, vr, NULL);

	VectorCopy(self->s.origin, spawnSpot);
	
	VectorMA(self->s.origin, 30, vf, spawnSpot);
	VectorMA(spawnSpot, 8,	 vr, spawnSpot);
	spawnSpot[2] += 56;

	//RIGHT HERE!
	self->monsterinfo.jump_time = level.time + 10;
	self->monsterinfo.attack_state = irand(0,2);

	//Don't repeat an attack (people want to see them all!)
	if (self->monsterinfo.lefty == self->monsterinfo.attack_state)
	{
		switch ( self->monsterinfo.attack_state )
		{
		case AS_QUEENS_FURY:
			
			self->monsterinfo.attack_state += irand(1,2);
			break;

		case AS_BROODS_SACRIFICE:
			
			if (irand(0,1))
				self->monsterinfo.attack_state = AS_QUEENS_FURY;
			else
				self->monsterinfo.attack_state = AS_HEAVENS_RAIN;
			break;

		case AS_HEAVENS_RAIN:

			self->monsterinfo.attack_state -= irand(1,2);
			break;
		}
	}

	self->monsterinfo.lefty = self->monsterinfo.attack_state;

	switch ( self->monsterinfo.attack_state )
	{
	case AS_QUEENS_FURY:
		gi.CreateEffect(NULL,
					FX_HP_MISSILE,
					0,
					spawnSpot,
					"vb",
					vec3_origin,
					HPMISSILE1_LIGHT);
		
		break;
	
	case AS_BROODS_SACRIFICE:
		self->monsterinfo.attack_finished = level.time + 2;
		gi.CreateEffect(NULL,
					FX_HP_MISSILE,
					0,
					spawnSpot,
					"vb",
					vec3_origin,
					HPMISSILE3_LIGHT);
		break;

	case AS_HEAVENS_RAIN:
		gi.CreateEffect(NULL,
					FX_HP_MISSILE,
					0,
					spawnSpot,
					"vb",
					vec3_origin,
					HPMISSILE4_LIGHT);

		gi.CreateEffect(NULL,
					FX_LENSFLARE,
					CEF_FLAG8,
					spawnSpot,
					"bbbf",
					(byte) 128,
					(byte) 128,
					(byte) 128,
					0.9);

		break;
	}
}

/*-----------------------------------------------
	priestess_attackc_loop_fire
-----------------------------------------------*/

void priestess_attack3_loop_fire ( edict_t *self )
{
	if (self->monsterinfo.attack_finished < level.time)
	{
		SetAnim(self, ANIM_ATTACK3_END);
		return;
	}

	//NOTE: These effects are not necessarily called each frame (hence the irands)
	switch ( self->monsterinfo.attack_state )
	{
	case AS_QUEENS_FURY:

		if (self->monsterinfo.search_time < level.time)
		{
			priestess_fire2( self, 0, 0, 0 );
			self->monsterinfo.search_time = level.time + 0.25;
		}
		break;

	case AS_BROODS_SACRIFICE:
	
		if (self->monsterinfo.search_time < level.time)
		{
			priestess_fire3( self, 0, 0, 0 );
			self->monsterinfo.search_time = level.time + 0.15;
		}
		break;

	case AS_HEAVENS_RAIN:
		
		priestess_fire4( self, 0, 0, 0 );
		break;
	}
}

/*-----------------------------------------------
	priestess_pounce_attack
-----------------------------------------------*/

void priestess_pounce_attack ( edict_t *self )
{
	float len;

	if (M_ValidTarget(self, self->enemy))
	{
		len = M_DistanceToTarget(self, self->enemy);

		if (len < 64)
		{
			SetAnim(self, ANIM_POUNCE_ATTACK);
		}
		else if (len < 128)
		{
			SetAnim(self, ANIM_ATTACK2);
		}
		else
		{
			priestess_pause(self);
		}
	}
}

/*-----------------------------------------------
	priestess_pounce
-----------------------------------------------*/
void priestess_jump_attack ( edict_t *self )
{
	vec3_t	predPos, jumpVel;
	float	jumpDist, moveDist, hopDist;

	//Find out where the player will be when we would probably land
	M_PredictTargetPosition( self->enemy, self->enemy->velocity, PRIESTESS_JUMP_FRAMES+2, predPos);

	//Find the vector to that spot and the length
	VectorSubtract(predPos, self->s.origin, jumpVel);
	moveDist = VectorNormalize(jumpVel);
	
	//Velocity is applied per tenth of a frame, so take the distance, divide by the number of frames in the air, and FRAMETIME
	jumpDist = ( moveDist * PRIESTESS_JUMP_FRAMES ) * FRAMETIME;

	//Now get the height to keep her in the air long enough to complete this jump
	hopDist = ( PRIESTESS_HOP_DISTANCE + ( ( sv_gravity->value * PRIESTESS_JUMP_FRAMES ) / 4 ) ) * FRAMETIME;
	
	//Setup the vector for the jump
	VectorScale( jumpVel, jumpDist, jumpVel );
	jumpVel[2] = hopDist;

	//Set the priestess in motion
	VectorCopy( jumpVel, self->velocity );
//	self->groundentity = NULL;
}

void priestess_pounce ( edict_t *self )
{
	vec3_t	predPos, jumpVel;
	float	jumpDist, moveDist, hopDist;

	if (!self->enemy)
		return;

	//Find out where the player will be when we would probably land
	M_PredictTargetPosition( self->enemy, self->enemy->velocity, PRIESTESS_JUMP_FRAMES+2, predPos);

	//Find the vector to that spot and the length
	VectorSubtract(predPos, self->s.origin, jumpVel);
	moveDist = VectorNormalize(jumpVel);
	
	//Velocity is applied per tenth of a frame, so take the distance, divide by the number of frames in the air, and FRAMETIME
	jumpDist = ( moveDist * PRIESTESS_JUMP_FRAMES ) * FRAMETIME;

	//Now get the height to keep her in the air long enough to complete this jump
	hopDist = ( PRIESTESS_HOP_DISTANCE + ( ( sv_gravity->value * PRIESTESS_JUMP_FRAMES ) / 4 ) ) * FRAMETIME;
	
	//Setup the vector for the jump
	VectorScale( jumpVel, jumpDist, jumpVel );
	jumpVel[2] = hopDist;

	//Set her in motion
	VectorCopy( jumpVel, self->velocity );
//	self->groundentity = NULL;
}

/*-----------------------------------------------
	priestess_strike
-----------------------------------------------*/

void priestess_strike ( edict_t *self, float damage )
{
	trace_t	trace;
	edict_t *victim;
	vec3_t	soff, eoff, mins, maxs, bloodDir, direction;

	//FIXME: Take out the mults here, done this way to speed up tweaking (sue me)
	switch ( self->s.frame )
	{
	case FRAME_attackB8:
		VectorSet(soff, 16*4, -16*5, 16*3);
		VectorSet(eoff, 16*3,  16*5, -8);
		break;

	case FRAME_attackB14:
		VectorSet(soff, 16*2,  16*5, 16*4);
		VectorSet(eoff, 16*5, -16*5,-16*2);
		break;

	case FRAME_jumpatt12:
		VectorSet(soff, 16*2, 0, 16*5);
		VectorSet(eoff, 16*5, 4,	4);
		break;
	}
	
	VectorSet(mins, -4, -4, -4);
	VectorSet(maxs,  4,  4,  4);

	VectorSubtract(soff, eoff, bloodDir);
	VectorNormalize(bloodDir);

	victim = M_CheckMeleeLineHit(self, soff, eoff, mins, maxs, &trace, direction);	

	//Did something get hit?
	if (victim)
	{
		if (victim == self)
		{
			//Create a spark effect
			gi.CreateEffect(NULL, FX_SPARKS, CEF_FLAG6, trace.endpos, "d", direction);
			gi.sound (self, CHAN_WEAPON, sounds[SND_SWIPEWALL], 1, ATTN_NORM, 0);
		}
		else
		{
			//Hurt whatever we were whacking away at
			T_Damage(victim, self, self, direction, trace.endpos, bloodDir, damage, damage*2, DAMAGE_DISMEMBER,MOD_DIED);
			gi.sound (self, CHAN_WEAPON, sounds[SND_SWIPE], 1, ATTN_NORM, 0);
		}
	}
	else
	{
		//Play swoosh sound
		gi.sound (self, CHAN_AUTO, sounds[SND_SWIPEMISS], 1, ATTN_NORM, 0);
	}	
}

/*-----------------------------------------------
	priestess_move
-----------------------------------------------*/

void priestess_move( edict_t *self, float vf, float vr, float vu )
{
}

/*-----------------------------------------------
	priestess_jump_right
-----------------------------------------------*/

void priestess_jump_right( edict_t *self )
{
	vec3_t	vr;

	AngleVectors(self->s.angles, NULL, vr, NULL);
	VectorScale( vr, 300, vr );
	vr[2] = 150;

	VectorCopy(vr, self->velocity);
//	self->groundentity = NULL;
}

/*-----------------------------------------------
	priestess_jump_left
-----------------------------------------------*/

void priestess_jump_left( edict_t *self )
{
	vec3_t	vr;

	AngleVectors(self->s.angles, NULL, vr, NULL);
	VectorScale( vr, -300, vr );
	vr[2] = 150;

	VectorCopy(vr, self->velocity);
//	self->groundentity = NULL;
}

/*-----------------------------------------------
	priestess_jump_forward
-----------------------------------------------*/

void priestess_jump_forward( edict_t *self )
{
	vec3_t	vf;

	AngleVectors(self->s.angles, vf, NULL, NULL);
	VectorScale( vf, 300, vf );
	vf[2] = 150;

	VectorCopy(vf, self->velocity);
//	self->groundentity = NULL;
}

/*-----------------------------------------------
	priestess_jump_back
-----------------------------------------------*/

void priestess_jump_back( edict_t *self )
{
	vec3_t	vf;

	AngleVectors(self->s.angles, vf, NULL, NULL);
	VectorScale( vf, -300, vf );
	vf[2] = 150;

	VectorCopy(vf, self->velocity);
//	self->groundentity = NULL;
}

/*-----------------------------------------------
	priestess_pause
-----------------------------------------------*/

void priestess_pause( edict_t *self )
{
	qboolean	clear_LOS;
	float		len;
	int			chance;

	chance = irand(0,100);

	if (M_ValidTarget(self, self->enemy))
	{
		len = M_DistanceToTarget(self, self->enemy);

		clear_LOS = AI_IsVisible(self, self->enemy);

		if (!clear_LOS && chance < 75)
		{
			SetAnim(self, ANIM_SHIELD_GO);
			return;
		}

		chance = irand(0,100);

		if (len < 64)
		{
			if (chance < 20)
				SetAnim(self, ANIM_ATTACK2);
			else if (chance < 40)
				SetAnim(self, ANIM_BACKUP);
			else
				SetAnim(self, ANIM_JUMP_BACK);
		}
		else
		{
			if (chance < 40 && self->monsterinfo.jump_time < level.time)
			{
				SetAnim(self, ANIM_ATTACK3_GO);
			}
			else if (chance < 40 && self->monsterinfo.attack_state != AS_LIGHT_MISSILE)
			{
				self->monsterinfo.search_time = 2;
				self->monsterinfo.attack_state = AS_LIGHT_MISSILE;
				SetAnim(self, ANIM_ATTACK1_GO);
			}
			else if (chance < 80 && self->monsterinfo.attack_state != AS_POUNCE)
			{
				self->monsterinfo.attack_state = AS_POUNCE;
				
				if (len > 256)
					SetAnim(self, ANIM_JUMP_POUNCE);
				else
					SetAnim(self, ANIM_JUMP_ATTACK);
			}
			else if (chance < 90 && self->monsterinfo.attack_finished < level.time)
			{
				SetAnim(self, ANIM_SHIELD_GO);
				self->monsterinfo.attack_finished = level.time + 5;
			}
			else if (self->monsterinfo.attack_state != AS_JUMP_RIGHT)
			{
				self->monsterinfo.attack_state = AS_JUMP_RIGHT;
				SetAnim(self, ANIM_JUMP_RIGHT);
			}
			else 
			{
				self->monsterinfo.attack_state = AS_JUMP_LEFT;
				SetAnim(self, ANIM_JUMP_LEFT);
			}
		}

		//SetAnim(self, ANIM_SHIELD_GO);
		//self->monsterinfo.attack_state = AS_LIGHT_MISSILE;
		//self->monsterinfo.search_time = 2;
		//SetAnim(self, ANIM_STAND1);
		//SetAnim(self, ANIM_ATTACK3_GO);
		//SetAnim(self, ANIM_ATTACK1_GO);
		//SetAnim(self, ANIM_JUMP_ATTACK);
		//SetAnim(self, ANIM_JUMP_POUNCE);
		 
		return;
	}

	QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
}

/*-----------------------------------------------
	priestess_idle
-----------------------------------------------*/

void priestess_idle(edict_t *self)
{
}

/*-----------------------------------------------
	priestess_dead
-----------------------------------------------*/

void priestess_dead( edict_t *self )
{
	/* CINEMATICS HERE */

	self->mood_nextthink = -1;//never mood_think again
	self->maxs[2] = self->mins[2] + 16;
	
	if (self->PersistantCFX)
	{
		gi.RemovePersistantEffect(self->PersistantCFX, REMOVE_PRIESTESS);
		self->PersistantCFX = 0;
	}
	gi.RemoveEffects(&self->s, 0);
	
	gi.linkentity (self);

	self->think = G_FreeEdict;
	self->nextthink = level.time + 0.1;
}

/*

	Priestess Message Functions

*/

/*-----------------------------------------------
	priestess_death
-----------------------------------------------*/

void priestess_death( edict_t *self, G_Message_t *msg )
{
	self->msgHandler = DeadMsgHandler;

	if(self->dead_state == DEAD_DEAD)
		return;

	self->dead_state = DEAD_DEAD;
	self->takedamage = DAMAGE_NO;

	self->dmg = 0;
	self->health = 0;
	self->max_health = 0;

	M_ShowLifeMeter(0, 0);
	
	SetAnim(self, ANIM_DEATH);
}


/*-----------------------------------------------
	priestess_evade
-----------------------------------------------*/

void priestess_evade( edict_t *self, G_Message_t *msg )
{
	if (irand(0,1))
		SetAnim(self, ANIM_DODGE_LEFT);
	else
		SetAnim(self, ANIM_DODGE_RIGHT);
}

/*-----------------------------------------------
	priestess_stand
-----------------------------------------------*/

void priestess_stand(edict_t *self, G_Message_t *msg)
{
	SetAnim(self, ANIM_STAND1);
}

/*-----------------------------------------------
	priestess_missile
-----------------------------------------------*/

void priestess_missile(edict_t *self, G_Message_t *msg)
{
	SetAnim(self, ANIM_ATTACK2);
}

/*-----------------------------------------------
	priestess_run
-----------------------------------------------*/

void priestess_run(edict_t *self, G_Message_t *msg)
{
	SetAnim(self, ANIM_WALK);
}

/*-----------------------------------------------
	priestess_pain
-----------------------------------------------*/

void priestess_pain(edict_t *self, G_Message_t *msg)
{
	int	temp, damage;
	int	force_pain;

	ParseMsgParms(msg, "eeiii", &temp, &temp, &force_pain, &damage, &temp);

	if (self->curAnimID == ANIM_ATTACK3_GO || self->curAnimID == ANIM_ATTACK3_LOOP || 
		self->curAnimID == ANIM_SHIELD_GO)
		return;

	//Weighted random based on health compared to the maximum it was at
	if (force_pain || ((irand(0, self->max_health+50) > self->health) && !irand(0,2)))
	{
		if (irand(0,1))
			gi.sound (self, CHAN_AUTO, sounds[SND_PAIN1], 1, ATTN_NORM, 0);
		else
			gi.sound (self, CHAN_AUTO, sounds[SND_PAIN2], 1, ATTN_NORM, 0);

		SetAnim(self, ANIM_PAIN);
	}
}

void priestess_postthink(edict_t *self)
{
	//Only display a lifemeter if we have an enemy
	if (self->enemy)
	{
		if (self->dmg < self->max_health)
		{
			M_ShowLifeMeter(self->dmg, self->dmg);
			self->dmg+=50;
		}
		else
		{
			M_ShowLifeMeter(self->health, self->max_health);
		}
	}

	self->next_post_think = level.time + 0.05;
}


/*

	Priestess Spawn Functions

*/

void HighPriestessStaticsInit(void)
{
	static ClassResourceInfo_t resInfo;

	classStatics[CID_HIGHPRIESTESS].msgReceivers[MSG_STAND]	= priestess_stand;
	classStatics[CID_HIGHPRIESTESS].msgReceivers[MSG_MISSILE] = priestess_missile;
	classStatics[CID_HIGHPRIESTESS].msgReceivers[MSG_RUN] = priestess_run;
	classStatics[CID_HIGHPRIESTESS].msgReceivers[MSG_EVADE] = priestess_evade;
	classStatics[CID_HIGHPRIESTESS].msgReceivers[MSG_DEATH] = priestess_death;
	classStatics[CID_HIGHPRIESTESS].msgReceivers[MSG_PAIN] = priestess_pain;
	
	resInfo.numAnims = NUM_ANIMS;
	resInfo.animations = animations;
	resInfo.modelIndex = gi.modelindex("models/monsters/highpriestess/tris.fm");
	resInfo.numSounds = NUM_SOUNDS;
	resInfo.sounds = sounds;

	sounds[SND_PAIN1]=gi.soundindex("monsters/highpriestess/pain1.wav");	
	sounds[SND_PAIN2]=gi.soundindex("monsters/highpriestess/pain2.wav");	
	sounds[SND_FALL]=gi.soundindex("monsters/highpriestess/fall.wav");	
	sounds[SND_3BALLATK]=gi.soundindex("monsters/highpriestess/3ballatk.wav");	
	sounds[SND_BALLHIT]=gi.soundindex("monsters/highpriestess/ballhit.wav");	
	sounds[SND_WHIRL]=gi.soundindex("weapons/stafftwirl_2.wav");	
	sounds[SND_BUGS]=gi.soundindex("monsters/highpriestess/bugs.wav");	
	sounds[SND_BUGBUZZ]=gi.soundindex("monsters/highpriestess/bugbuzz.wav");	
	sounds[SND_BUGHIT]=gi.soundindex("monsters/highpriestess/bughit.wav");	
	sounds[SND_ZAP]=gi.soundindex("monsters/highpriestess/zap.wav");	
	sounds[SND_ZAPHIT]=gi.soundindex("monsters/highpriestess/zaphit.wav");	
	sounds[SND_HOMINGATK]=gi.soundindex("monsters/highpriestess/homatk.wav");	
	sounds[SND_HOMINGHIT]=gi.soundindex("monsters/highpriestess/homhit.wav");	
	sounds[SND_TPORT_IN]=gi.soundindex("monsters/highpriestess/tportin.wav");	
	sounds[SND_TPORT_OUT]=gi.soundindex("monsters/highpriestess/tpotout.wav");	
	sounds[SND_SWIPE]=gi.soundindex("weapons/staffswing_2.wav");	
	sounds[SND_SWIPEHIT]=gi.soundindex("weapons/staffhit_2.wav");	
	sounds[SND_SWIPEMISS]=gi.soundindex("monsters/seraph/guard/attack_miss.wav");	
	sounds[SND_SWIPEWALL]=gi.soundindex("weapons/staffhitwall.wav");	

	classStatics[CID_HIGHPRIESTESS].resInfo = &resInfo;
}

/*QUAKED monster_high_priestess (1 .5 0) (-24 -24 0) (24 24 72)

The High Priestess (what more do you need to know?!?!)

"wakeup_target" - monsters will fire this target the first time it wakes up (only once)

"pain_target" - monsters will fire this target the first time it gets hurt (only once)

*/
void SP_monster_high_priestess (edict_t *self)
{
	if ((deathmatch->value == 1) && !((int)sv_cheats->value & self_spawn))
	{
		return;
	}

	if (!M_WalkmonsterStart(self))			// Failed initialization
		return;

	self->msgHandler = DefaultMsgHandler;

	if (!self->health)
	{
		self->health = HP_HEALTH;
	}

	//Apply to the end result (whether designer set or not)
	self->max_health = self->health = MonsterHealth(self->health);

	self->mass = HP_MASS;
	self->yaw_speed = 24;

	self->movetype = PHYSICSTYPE_STEP;
	self->solid=SOLID_BBOX;
	self->clipmask = MASK_MONSTERSOLID;

	self->s.origin[2] += 36;
	VectorSet(self->mins, -24, -24, -36);
	VectorSet(self->maxs, 24, 24, 36);

	self->materialtype = MAT_INSECT;

	self->s.modelindex = classStatics[CID_HIGHPRIESTESS].resInfo->modelIndex;
	self->s.skinnum = 0;

	self->monsterinfo.jump_time = level.time + 15;
	self->monsterinfo.otherenemyname = "monster_rat";	

	if (self->monsterinfo.scale)
	{
		self->s.scale = self->monsterinfo.scale = MODEL_SCALE;
	}

	MG_InitMoods( self );

	//Setup her reference points
	self->PersistantCFX = gi.CreatePersistantEffect(&self->s,
							FX_HP_STAFF,
							CEF_OWNERS_ORIGIN | CEF_BROADCAST,
							vec3_origin,
							"bs",
							HP_STAFF_INIT,
							self->s.number);

	QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);

	self->post_think = priestess_postthink;
	self->next_post_think = level.time + 0.1;

	self->svflags|=SVF_BOSS;
}
