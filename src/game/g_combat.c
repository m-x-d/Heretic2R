// g_combat.c

#include "g_local.h"
#include "Utilities.h"
#include "g_HitLocation.h"
#include "g_DefaultMessageHandler.h"
#include "FX.h"
#include "vector.h"
#include "random.h"
#include "g_misc.h"
#include "p_main.h"
#include "g_playstats.h"
#include "buoy.h"
#include "g_itemstats.h"
#include "m_stats.h"

gitem_armor_t silver_armor_info	= {MAX_SILVER_ARMOR, SILVER_HIT_MULT, SILVER_SPELL_MULT};
gitem_armor_t gold_armor_info	= {MAX_GOLD_ARMOR, GOLD_HIT_MULT, GOLD_SPELL_MULT};

void pitch_roll_for_slope (edict_t *forwhom, vec3_t *slope);
void MG_PostDeathThink (edict_t *self);
extern void AlertMonsters (edict_t *self, edict_t *enemy, float lifetime, qboolean ignore_shadows);

/* 
============
CanDamage

Returns true if the inflictor can directly damage the target.  Used for
explosions and melee attacks.
============
*/

qboolean CanDamage (edict_t *targ, edict_t *inflictor)
{
	vec3_t	dest, diff;
	trace_t	trace;

	// bmodels need special checking because their origin is 0,0,0
	if (targ->movetype == PHYSICSTYPE_PUSH || targ->classID == CID_BBRUSH)
	{
		VectorAdd (targ->absmin, targ->absmax, dest);
		VectorScale (dest, 0.5, dest);
		gi.trace (inflictor->s.origin, vec3_origin, vec3_origin, dest, inflictor, MASK_SOLID,&trace);
		
		if (trace.fraction == 1.0)
			return true;
		
		if (trace.ent == targ)
			return true;
		
		return false;
	}

	// Try a basic trace straight to the origin.  This takes care of 99% of the tests.
	gi.trace (inflictor->s.origin, vec3_origin, vec3_origin, targ->s.origin, inflictor, MASK_SOLID,&trace);
	if (trace.fraction == 1.0)
		return true;

	// Well, a trace from origin to origin didn't work, so try tracing to the edges of the victim.

	// If there are no edges, let's skip the rest of these checks..
	if (Vec3IsZero(targ->mins) || Vec3IsZero(targ->maxs))
		return false;
	
	// First figure out which two sides of the victim to check.
	VectorSubtract(inflictor->s.origin, targ->s.origin, diff);

	// If the X is greater than the Y difference, then the perpendicular edges, north and south, should be checked.
	if (fabs(diff[0]) > fabs(diff[1]))
	{	// check north and south edges.
		// South edge
		VectorCopy(targ->s.origin, dest);
		dest[1] += targ->mins[1];
		gi.trace(inflictor->s.origin, vec3_origin, vec3_origin, dest, inflictor, MASK_SOLID,&trace);
		if (trace.fraction > .99)
			return true;

		// North edge
		VectorCopy(targ->s.origin, dest);
		dest[1] += targ->maxs[1];
		gi.trace(inflictor->s.origin, vec3_origin, vec3_origin, dest, inflictor, MASK_SOLID,&trace);
		if (trace.fraction > .99)
			return true;
	}
	else
	{	// check east and west edges.
		// West edge
		VectorCopy(targ->s.origin, dest);
		dest[0] += targ->mins[0];
		gi.trace(inflictor->s.origin, vec3_origin, vec3_origin, dest, inflictor, MASK_SOLID,&trace);
		if (trace.fraction > .99)
			return true;

		// East edge
		VectorCopy(targ->s.origin, dest);
		dest[0] += targ->maxs[0];
		gi.trace(inflictor->s.origin, vec3_origin, vec3_origin, dest, inflictor, MASK_SOLID,&trace);
		if (trace.fraction > .99)
			return true;
	}

	// Since the side checks didn't work, check the top and bottom.
	// bottom edge
	VectorCopy(targ->s.origin, dest);
	dest[2] += targ->mins[2];
	gi.trace(inflictor->s.origin, vec3_origin, vec3_origin, dest, inflictor, MASK_SOLID,&trace);
	if (trace.fraction > .99)
		return true;

	// top edge
	VectorCopy(targ->s.origin, dest);
	dest[2] += targ->maxs[2];
	gi.trace(inflictor->s.origin, vec3_origin, vec3_origin, dest, inflictor, MASK_SOLID,&trace);
	if (trace.fraction > .99)
		return true;

	// None of the traces were successful, so no good.
	return false;
}


// Same function, except the origin point of the damage doesn't have to be the same as the inflictor's
qboolean CanDamageFromLoc (edict_t *targ, edict_t *inflictor, vec3_t origin)
{
	vec3_t	dest, diff;
	trace_t	trace;

	// bmodels need special checking because their origin is 0,0,0
	if (targ->movetype == PHYSICSTYPE_PUSH || targ->classID == CID_BBRUSH)
	{
		VectorAdd (targ->absmin, targ->absmax, dest);
		VectorScale (dest, 0.5, dest);
		gi.trace (origin, vec3_origin, vec3_origin, dest, inflictor, MASK_SOLID,&trace);
		
		if (trace.fraction == 1.0)
			return true;
		
		if (trace.ent == targ)
			return true;
		
		return false;
	}

	// Try a basic trace straight to the origin.  This takes care of 99% of the tests.
	gi.trace (origin, vec3_origin, vec3_origin, targ->s.origin, inflictor, MASK_SOLID,&trace);
	if (trace.fraction == 1.0)
		return true;

	// Well, a trace from origin to origin didn't work, so try tracing to the edges of the victim.

	// If there are no edges, let's skip the rest of these checks..
	if (Vec3IsZero(targ->mins) || Vec3IsZero(targ->maxs))
		return false;
	
	// First figure out which two sides of the victim to check.
	VectorSubtract(origin, targ->s.origin, diff);

	// If the X is greater than the Y difference, then the perpendicular edges, north and south, should be checked.
	if (fabs(diff[0]) > fabs(diff[1]))
	{	// check north and south edges.
		// South edge
		VectorCopy(targ->s.origin, dest);
		dest[1] += targ->mins[1];
		gi.trace(origin, vec3_origin, vec3_origin, dest, inflictor, MASK_SOLID,&trace);
		if (trace.fraction > .99)
			return true;

		// North edge
		VectorCopy(targ->s.origin, dest);
		dest[1] += targ->maxs[1];
		gi.trace(origin, vec3_origin, vec3_origin, dest, inflictor, MASK_SOLID,&trace);
		if (trace.fraction > .99)
			return true;
	}
	else
	{	// check east and west edges.
		// West edge
		VectorCopy(targ->s.origin, dest);
		dest[0] += targ->mins[0];
		gi.trace(origin, vec3_origin, vec3_origin, dest, inflictor, MASK_SOLID,&trace);
		if (trace.fraction > .99)
			return true;

		// East edge
		VectorCopy(targ->s.origin, dest);
		dest[0] += targ->maxs[0];
		gi.trace(origin, vec3_origin, vec3_origin, dest, inflictor, MASK_SOLID,&trace);
		if (trace.fraction > .99)
			return true;
	}

	// Since the side checks didn't work, check the top and bottom.
	// bottom edge
	VectorCopy(targ->s.origin, dest);
	dest[2] += targ->mins[2];
	gi.trace(origin, vec3_origin, vec3_origin, dest, inflictor, MASK_SOLID,&trace);
	if (trace.fraction > .99)
		return true;

	// top edge
	VectorCopy(targ->s.origin, dest);
	dest[2] += targ->maxs[2];
	gi.trace(origin, vec3_origin, vec3_origin, dest, inflictor, MASK_SOLID,&trace);
	if (trace.fraction > .99)
		return true;

	// None of the traces were successful, so no good.
	return false;
}

void SpawnReward(edict_t *self, edict_t *attacker)
{
	gitem_t	*item, *lookup;
	edict_t *newitem;
	vec3_t	forward, holdorigin;
	float	health_chance, off_chance, def_chance;
	char	*item_name;
	int		off_amount, off_max, def_amount, def_max, index, chance;

	//MG: Assassins always give you something
	if (self->classID != CID_ASSASSIN)
	{
		//Randomly refuse to give them anything
		if (coop->value)
		{
			if ( !irand(0, (maxclients->value+1)/2) )
				return;
		}
		else if (skill->value < 3.0)
		{
			if (irand(0, ITEM_REWARD_CHANCE + (2*skill->value)))
			{	// Easy: 1 in 2 chance, normal: 1 in 4 chance; Hard: 1 in 6 chance
				return;
			}
		}
		else
		{	// No rewards in skills at or above 3.0.
			return;
		}
	}
	
	//Only intelligent monsters produce items, not creatures (and not Ogles)
	if ( (self->classID == CID_RAT) || (self->classID == CID_HARPY) || (self->classID == CID_GKROKON) || (self->classID == CID_GORGON) || (self->classID == CID_FISH) || (self->classID == CID_OGLE) )
		return;

	//Bosses don't spawn a reward either
	if ( self->svflags & SVF_BOSS )
		return;

	//Check the health amount on the attacker
	health_chance = (attacker->health < attacker->max_health) ? ( (float) attacker->health / (float) attacker->max_health) : 9999;

	//Check the offensive mana amount on the attacker
	lookup = P_FindItemByClassname("item_mana_offensive_half");
	index = ITEM_INDEX(lookup);
	off_max = attacker->client->playerinfo.pers.max_offmana;
	off_amount = attacker->client->playerinfo.pers.inventory.Items[index];

	off_chance = (off_amount < off_max) ? ( (float) off_amount / (float) off_max ) : 9999;

	//Check the offensive mana amount on the attacker
	lookup = P_FindItemByClassname("item_mana_defensive_half");
	index = ITEM_INDEX(lookup);
	def_max = attacker->client->playerinfo.pers.max_defmana;
	def_amount = attacker->client->playerinfo.pers.inventory.Items[index];

	def_chance = (def_amount < def_max) ? ( (float) def_amount / (float) def_max ) : 9999;
	
	//We don't need anything
	if ( (health_chance == 9999) && (off_chance == 9999) && (def_chance == 9999))
		return;

	//Determine what they get
	if ( (health_chance < off_chance) && (health_chance < def_chance) )
	{
		item_name = "item_health_half";
	}
	else if ( (off_chance < health_chance) && (off_chance < def_chance) )
	{
		item_name = "item_mana_offensive_half";
	}
	else if ( (def_chance < health_chance) && (def_chance < off_chance) )
	{
		item_name = "item_mana_defensive_half";
	}
	else
	{
		chance = irand(0,2);

		if (chance==0)
			item_name = "item_health_half";
		else if (chance==1)
			item_name = "item_mana_offensive_half";
		else
			item_name = "item_mana_defensive_half";
	}

	//We know what we want to give them, so create it!
	item = P_FindItemByClassname(item_name);

	newitem = G_Spawn();

	newitem->movetype = PHYSICSTYPE_STEP;
	AngleVectors(self->s.angles,forward,NULL,NULL);
	VectorCopy(self->s.origin,newitem->s.origin);

	SpawnItem(newitem, item);

	VectorCopy(newitem->s.origin,holdorigin);
	
	//Make the effect
	gi.CreateEffect(NULL, FX_PICKUP, 0, holdorigin, "");
}

/*
============
Killed
============
*/

void Killed (edict_t *targ, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point, int mod)
{
	vec3_t hitdir;

	if (targ->classID == CID_MORK)
	{
		targ->die(targ, inflictor, attacker, damage, vec3_origin);		
		return;
	}

	if (attacker->svflags & SVF_MONSTER)
	{//clear special enemy attack stuff
		//FIXME?  Make attacking monster look for a new target?
		//OR: Search for all monsters with targ as enemy and
		//find new target for them?
		attacker->monsterinfo.aiflags &= ~AI_STRAIGHT_TO_ENEMY;
	}

	if (targ->classID != CID_BBRUSH)	
		targ->enemy = attacker;

	if ((targ->svflags & SVF_MONSTER) && (targ->deadflag != DEAD_DEAD))
	{
		MG_RemoveBuoyEffects(targ);
		//What about if off ledge or on steep slope- slide off?
		pitch_roll_for_slope(targ, NULL);
	
		targ->dead_size = Q_fabs( (targ->maxs[2] - targ->mins[2]) ) * 0.5;
		MG_PostDeathThink(targ);
		
		if(!(targ->svflags & SVF_WAIT_NOTSOLID))
			targ->svflags |= SVF_DEADMONSTER;	// now treat as a different content type
		if (!(targ->monsterinfo.aiflags & AI_GOOD_GUY))
		{
			level.killed_monsters++;
		}
		targ->enemy = attacker;
		AlertMonsters (targ, attacker, 7, true);
		//spawns an ent that will alert other monsters to my enemy's presence for 7 seconds

		//Spawn a reward for the kill
		if (attacker->client)
			SpawnReward(targ, attacker);
	}
	if (targ->movetype == PHYSICSTYPE_PUSH || targ->movetype == PHYSICSTYPE_STOP || targ->movetype == PHYSICSTYPE_NONE)
	{	
		// Doors, triggers, breakable brushes, etc. die with their own KillBrush() routine.
		if (targ->classID == CID_BBRUSH)
		{
			KillBrush(targ,inflictor,attacker,damage);
		}
		else
		{
			if((!targ->classID || !classStatics[targ->classID].msgReceivers[MSG_PAIN]) && targ->die)
			{
				targ->die(targ, inflictor, attacker, damage, vec3_origin);
			}
			else
			{
				QPostMessage(targ,MSG_DEATH,PRI_DIRECTIVE,"eeei",targ,inflictor,attacker,damage);
			}
		}
		return;
	}

	// Also, put the flag of fire on the entity- makes fire lower when die
	if(targ->size[2] > 24)
		targ->s.effects |= EF_DISABLE_EXTRA_FX;

	if (targ->deadflag != DEAD_DEAD)
	{
//		ClientObituary(self, inflictor, attacker);

		targ->touch = NULL;

		monster_death_use (targ);
	}

	if(targ->client)
	{	//FIXME: Make sure you can still dismember and gib player while dying
		targ->client->meansofdeath = mod;
		player_die(targ, inflictor, attacker, damage, point);
	}
	else
	{
		//CID_RAT SHOULD NOT BE ZERO!!! CID_NONE SHOULD BE
		if((!targ->classID || !classStatics[targ->classID].msgReceivers[MSG_PAIN]) && targ->die)
		{
			targ->die(targ, inflictor, attacker, damage, vec3_origin);
		}
		else
			QPostMessage(targ,MSG_DEATH,PRI_DIRECTIVE,"eeei",targ,inflictor,attacker,damage);
	}
	
	if(Vec3IsZero(targ->velocity) && (damage != 12345))
	{//fall back some!
		VectorSubtract(targ->s.origin, inflictor->s.origin, hitdir);
		hitdir[2] = 50;
		VectorNormalize(hitdir);
		VectorMA(targ->velocity, damage*3, hitdir, targ->velocity);
	}
}

/*
============
M_ReactToDamage
============
*/
void M_ReactToDamage (edict_t *targ, edict_t *attacker)
{
	if (!(attacker->client) && !(attacker->svflags & SVF_MONSTER))
		return;

	if (attacker == targ)
		return;

	//FIXME: in SP, after player dead, allow this?  Or Make attacker lose it's enemy?
	if(!ANARCHY && attacker->classID == targ->classID)
		return;//monsters of same type won't fight each other

	if (targ->classID == CID_OGLE && (!targ->monsterinfo.awake || attacker->client))//ogles do their own checks to get angry at their first enemy
		return;

	if (attacker == targ->enemy)
	{//ok, no more stalking- now we get serious
		targ->ai_mood_flags &= ~AI_MOOD_FLAG_BACKSTAB;
		if(!targ->monsterinfo.awake)
			FoundTarget(targ, true);
		return;
	}

	if (!attacker->takedamage)//world, etc.
		return;

	if(targ->monsterinfo.c_mode)//don't anger cinematic monsters
		return;

	if (attacker->client)
	{
		targ->monsterinfo.chase_finished = level.time + 4;	// When the monster can notice secondary enemies

		if (targ->enemy && targ->enemy->client)
			targ->oldenemy = targ->enemy;
		targ->enemy = attacker;
		FoundTarget (targ, true);
		return;
	}

	if (targ->monsterinfo.aiflags & AI_GOOD_GUY)
	{
		if (!(attacker->client) && !(attacker->monsterinfo.aiflags & AI_GOOD_GUY))
		{
			targ->enemy = attacker;
			FoundTarget (targ, true);
			return;
		}
	}

	// if attacker is a client or
	// it's the same base (walk/swim/fly) type and a different classname and it's a monster that sprays too much
	// get mad at them
	if (((targ->flags & (FL_FLY|FL_SWIM)) == (attacker->flags & (FL_FLY|FL_SWIM))) &&
		 (targ->classID != attacker->classID)&&
		 (targ->enemy))//targ has an enemy, otherwise always get mad
	{
		if (targ->enemy->client)
			targ->oldenemy = targ->enemy;
		targ->enemy = attacker;
		FoundTarget (targ, true);
	}
	else// otherwise get mad at whoever they are mad at (help our buddy)
	{
		if (attacker->enemy)		// This really should be an assert, but there are problems with this.
		{
			if(attacker->enemy==targ && attacker->classID==targ->classID && !(targ->monsterinfo.aiflags&AI_AGRESSIVE))
			{//attacker was shooting at me(targ) and is my class, but I'm not agressive so I didn't hit him first
				if(irand(0,10)<7)
				{//run away!
					if(targ->enemy==attacker&&irand(0,10)<3&&stricmp(attacker->classname, "player"))
					{
						targ->monsterinfo.flee_finished = 0;
					}
					else if(targ->monsterinfo.flee_finished < level.time + 7.0)
					{
						targ->monsterinfo.aiflags |= AI_FLEE;
						targ->monsterinfo.flee_finished = level.time + flrand(3.0, 7.0);
					}
				}
				targ->enemy = attacker;
				FoundTarget (targ, true);
			}
			else if(attacker->enemy != targ && (!targ->enemy || targ->enemy->health <= 0))
			{//attacker wasn't after me and my enemy is invalid or don't have one... go after atacker's enemy
				if (targ->enemy)
				{
					if (targ->enemy->client||ANARCHY)
						targ->oldenemy = targ->enemy;
				}
				targ->enemy = attacker->enemy;
				FoundTarget (targ, true);
			}
			else if( (attacker->classID!=targ->classID&&!irand(0,2)) ||ANARCHY)
			{//30% chance to get mad (only if they're not my class), or always get mad if ANARCHY
				if (targ->enemy)
				{
					if (targ->enemy->client||ANARCHY)
						targ->oldenemy = targ->enemy;
				}
				targ->enemy = attacker;
				FoundTarget (targ, true);
			}
		}
		else
		{//attacker's on crack, kill him
			targ->enemy = attacker;
			FoundTarget (targ, true);
		}
	}
}

// ************************************************************************************************
// CheckTeamDamage
// ---------------
// ************************************************************************************************

qboolean CheckTeamDamage (edict_t *targ, edict_t *attacker)
{
	//FIXME: Make the next line real and uncomment this block.
	//if ((ability to damage a teammate == OFF) && (targ's team == attacker's team))
		return(false);
}

qboolean flammable (edict_t *targ)
{
	if(targ->materialtype == MAT_CLOTH||
		targ->materialtype == MAT_FLESH||	
		targ->materialtype == MAT_POTTERY||	
		targ->materialtype == MAT_LEAF||	
		targ->materialtype == MAT_WOOD||	
		targ->materialtype == MAT_INSECT)
		return (true);

	return (false);
}

/*
============
T_Damage

targ		entity that is being damaged
inflictor	entity that is causing the damage
attacker	entity that caused the inflictor to damage targ

example:	targ=monster, inflictor=rocket, attacker=player

dir			direction of the attack:  
					Directional vector (velocity is acceptable), in the direction the force is GOING.  
					Normalized in the function, if (0,0,0) then vector from inflictor to target is used.
					Used for knockback (scale force by this vector)
					Also used for blood and puffs when objects are struck
					CANNOT BE NULL.
point		point at which the damage is being inflicted
					Absolute point at which the damage is generated.
					Used for hit locations, and blood (generation point)
					CANNOT BE NULL
normal		normal vector from that point
					Directional vector, assumed to be normalized.(of course)
					Used for blood from monsters and players (squirts in this direction).
					Checked for NULL
damage		amount of damage being inflicted
knockback	force to be applied against targ as a result of the damage

dflags		these flags are used to control how T_Damage works:

DAMAGE_RADIUS			damage was indirect (from a nearby explosion)
DAMAGE_NO_KNOCKBACK		do not affect velocity, just view angles
DAMAGE_NO_PROTECTION	kills godmode, armor, everything
DAMAGE_DISMEMBER		to force MSG_DISMEMBER to be used
============
*/
void T_Damage(edict_t *targ, edict_t *inflictor, edict_t *attacker, vec3_t pdir, vec3_t ppoint, vec3_t pnormal, 
			  int damage, int knockback, int dflags,int MeansOfDeath)
{
	vec3_t			hit_spot;
	gclient_t		*client;
	int				take, dsm_dmg;
	HitLocation_t	hl;
 	gitem_armor_t	*info;
	int				force_pain = 0;
	qboolean		was_dead = false;
	vec3_t			dir, normal, point;
	float			knockbacktime;
	char			armor_sound[50];
	int				armorabsorb, orig_dmg;
	int				violence=VIOLENCE_DEFAULT;

	// Friendly-fire avoidance. If enabled, you can't hurt teammates (but you can hurt yourself).
	// Knockback still occurs though.
	if (targ->client && attacker->client && targ != attacker)
	{	// Both different players, let's check if this will do damage!
		if (coop->value)
		{
			if(!((int)(dmflags->value) & DF_HURT_FRIENDS)&&(!(dflags&DAMAGE_HURT_FRIENDLY)))
				damage = 0;
			else
				MeansOfDeath |= MOD_FRIENDLY_FIRE;
		}
		else if (deathmatch->value)
		{
			if ((int)(dmflags->value) & (DF_MODELTEAMS|DF_SKINTEAMS))
			{
				if (OnSameTeam (targ, attacker))
				{
					if(!((int)(dmflags->value) & DF_HURT_FRIENDS)&&(!(dflags&DAMAGE_HURT_FRIENDLY)))
						damage = 0;
					else
						MeansOfDeath |= MOD_FRIENDLY_FIRE;
				}
			}
		}
	}

	// ************************

	if (blood_level)
		violence = blood_level->value;

	// Prevent players harming fellow players in non-deathmatch games. The attacking player can
	// still harm himself of course.

//	if((!deathmatch->value)&&(targ!=attacker)&&(targ->client)&&(attacker->client)&&(!(dflags&DAMAGE_HURT_FRIENDLY)))
//		return;

	if (!targ->takedamage)
		return;

	if (targ->client)
	{
		if (targ->client->RemoteCameraLockCount)	// In a remote camera so he can't be hurt
			return;

		if (targ->client->playerinfo.c_mode)		// In cinematic mode so he can't be hurt
			return;
	}

	if(dflags & DAMAGE_ALIVE_ONLY)
		if (targ->materialtype != MAT_FLESH||targ->health<=0)
			return;

	if(targ->svflags&SVF_NO_PLAYER_DAMAGE)
		if(attacker->client)
			return;

	if(targ->health <= 0)
	{
		if(targ->client)//can't keep killing a dead player
			return;

		if(targ->classID)
		{
			if(classStatics[targ->classID].msgReceivers[MSG_DEATH_PAIN])
			{
				if(dflags & DAMAGE_SUFFOCATION || dflags & DAMAGE_BLEEDING || dflags == DAMAGE_BURNING)
					return;
				else
					was_dead = true;
			}
		}
	}

	// if we are on a shrine or teleporting we are not to be harmed	- assuming we haven't been telefragged, in which case kill us dead
	if (targ->client && (targ->client->shrine_framenum > level.time) && !(dflags & DAMAGE_NO_PROTECTION) )
		return;

//So we don't overwrite the passed-in vectors!!!
	if(pdir && Vec3NotZero(pdir))
		VectorCopy(pdir, dir);
	else
		VectorSet(dir, 0, 0, -1);

	if(pnormal && Vec3NotZero(pnormal))
		VectorCopy(pnormal, normal);
	else
		VectorSet(normal, 0, 0, 1);
	
	if(ppoint)
		VectorCopy(ppoint, point);
	else
		VectorCopy(inflictor->s.origin, point);
	
	if (deathmatch->value == 0)
	{	// Not deathmatch game.
		// In easy skill-level, the player only takes half damage.
		if ((skill->value == 0) && targ->client)
			damage = ceil(damage * 0.5);
	}
	else
	{	// In deathmatch game.
		float temp;

		// Factor in deathmatch to increase hit points
		if (targ->client)
		{
			temp = 4.0-skill->value;
			if (temp < 1.0)
				damage = (int)(damage * 4.0);
			else 
				damage = ceil(damage * 2.0 / (4.0 - skill->value));		// Skill 0: 1/2 damage, skill 1: 2/3 damage, skill 2: full
		}
	}

	if (!damage && ((int)(dmflags->value) & DF_HURT_FRIENDS)&&(!(dflags&DAMAGE_HURT_FRIENDLY)) 
		&& !(deathmatch->value && ((int)(dmflags->value) & (DF_MODELTEAMS|DF_SKINTEAMS))))
		damage = 1;

	orig_dmg = damage;

	// deal with armor - if we have any
	// are we a player ?
	if (targ->client && !(dflags&DAMAGE_BLEEDING))
	{
		// figure out where the armor details are located at
		// do we actually have any armor anyway ?
		// dont let armor effect us if we are drowning, in slime, or in lava
		if (!(dflags & DAMAGE_AVOID_ARMOR) &&
			targ->client->playerinfo.pers.armor_count &&
			(dflags != DAMAGE_SUFFOCATION) &&
			(dflags != DAMAGE_SLIME) &&
			(dflags != DAMAGE_LAVA)) 
		{
			if (targ->client->playerinfo.pers.armortype == ARMOR_TYPE_SILVER)
				info = &silver_armor_info;
			else
				info = &gold_armor_info;

			// Figure out how much the armor takes
			armorabsorb = damage;

			// effect damage dependant on what type of effect hit us
			if (dflags & DAMAGE_SPELL)
				damage *= info->spell_protection;
			else
				damage *= info->normal_protection;

			// make the little sparkle effect at the hit point
			gi.CreateEffect(NULL,
							FX_ARMOR_HIT,
							0,
							point,
							"d",
							normal);
				
			if (dflags & DAMAGE_SPELL)
				gi.sound(targ,CHAN_WEAPON,gi.soundindex("weapons/spellric.wav"),2,ATTN_NORM,0);
			else
			{
				// don't always make sound - being attacked by rats makes this go off incesantly
				if (!(irand(0,2)))
				{
					sprintf(armor_sound, "weapons/armorric%d.wav",irand(1,3));
					gi.sound(targ,CHAN_WEAPON,gi.soundindex(armor_sound),2,ATTN_NORM,0);
				}
			}

			// be sure we still have some damage
			if (!damage)
			{
	  			damage = 1;
				armorabsorb = 1;
			}
			else 
			{	// Everything not taken by the player is taken by the armor
				armorabsorb -= damage;
				if (armorabsorb > targ->client->playerinfo.pers.armor_count)
				{
					damage += armorabsorb - targ->client->playerinfo.pers.armor_count;
					armorabsorb = targ->client->playerinfo.pers.armor_count;
				}
			}

			targ->client->playerinfo.pers.armor_count -= armorabsorb;
			// dec armor count. are we down to zero armor ?
			if (targ->client->playerinfo.pers.armor_count <= 0)
			{
				// stop drawing the armor
				targ->client->playerinfo.pers.armortype = ARMOR_NONE;
				targ->client->playerinfo.pers.armor_count = 0;

				SetupPlayerinfo_effects(targ);
				P_PlayerUpdateModelAttributes(&targ->client->playerinfo);
				WritePlayerinfo_effects(targ);

				// Play the out-of-armor sound.
				gi.sound(targ, CHAN_WEAPON, gi.soundindex("weapons/armorgone.wav"), 1, ATTN_NORM, 0);
			}
		}
	}

	client = targ->client;

	VectorNormalize(dir);

	// The player does bonus damage for suprising a monster.
	// Bad idea for us.
//	if (!(dflags & DAMAGE_RADIUS) && (targ->svflags & SVF_MONSTER) && (attacker->client) && (!targ->enemy) && (targ->health > 0))
//		damage *= 2;

	if (targ->flags & FL_NO_KNOCKBACK || targ->svflags & SVF_BOSS ||
			(targ->svflags & SVF_MONSTER && sv_freezemonsters->value == 2.0))	// Freezemonster = 2 means no knockback 
	{			
		knockback = 0;
	}

	// Figure  out the knockback momentum to impart to the target.
	if (!(dflags & DAMAGE_NO_KNOCKBACK) && !(targ->svflags & SVF_BOSS))
	{//hey, knockback of less than about 25 isn't going to do squat...
		if ((knockback) && 
				(targ->movetype != PHYSICSTYPE_NONE) && 
				(targ->movetype != PHYSICSTYPE_PUSH) && 
				(targ->movetype != PHYSICSTYPE_STOP))
		{
			vec3_t	kvel;
			float	mass, force, upvel;

			if(Vec3IsZero(dir))
			{
				VectorSubtract(targ->s.origin, inflictor->s.origin, dir);
				if(dir[2]<0)
					dir[2] = 0;
				VectorNormalize(dir);
			}

			mass = VectorLength(targ->size) * 3;
			/*
			if (targ->mass < 50)
				mass = 50;
			else
				mass = targ->mass;*/

//			if (targ->client  && attacker == targ)
//				force = 900.0 * (float)knockback / mass; //rocketjump hack...  More knockback to source.
//			else
				force = 600.0 * (float)knockback / mass;

			// Players are not as affected by velocities when they are on the ground, so increase what players experience.
			if (targ->client && targ->groundentity)
				force *= 4.0;
			else if (targ->client)	// && !(targ->groundentity)
				force *= 0.25;	// Too much knockback

			if (dflags & DAMAGE_EXTRA_KNOCKBACK)
				force *= 3.0;

			if (force > 512)	// Cap this speed so it doesn't get insane
				force=512;
			VectorScale (dir, force, kvel);

			if (targ->client)	// Don't force players up quite so much as monsters.
				upvel=30;
			else
				upvel=60;
			// Now if the player isn't being forced DOWN very far, let's force them UP a bit.
			if ((dir[2] > -0.5 || targ->groundentity) && kvel[2] < upvel && force > 30)
			{	// Don't knock UP the player more than we do across...
				if (force < upvel)
					kvel[2] = force;
				else
					kvel[2] = upvel;
			}

			VectorAdd (targ->velocity, kvel, targ->velocity);

			if (targ->client)	// If player, then set the player flag that will affect this.
			{
				targ->client->playerinfo.flags |= PLAYER_FLAG_USE_ENT_POS;
				// The knockbacktime indicates how long this knockback affects the player.
				if (force>500)
					knockbacktime = level.time + 1.25;
				else
					knockbacktime = level.time + (force/400.0);
				if (knockbacktime > targ->client->playerinfo.knockbacktime)
				{
					targ->client->playerinfo.knockbacktime = knockbacktime;
					if (MeansOfDeath == MOD_TORN)
					{
						// since we are bing knocked back, let our top speed be higher
						targ->client->playerinfo.effects |= EF_HIGH_MAX;
						targ->s.effects |= EF_HIGH_MAX;
					}
					else
					{
						// since we are bing knocked back, let our top speed be higher
						targ->client->playerinfo.effects &= ~EF_HIGH_MAX;
						targ->s.effects &= ~EF_HIGH_MAX;
					}

				}
			} 

			//so knockback doesn't gib them unless it really really should
			if(force<300)
				targ->jump_time = level.time + 0.5;
		}
	}

	take = damage;

	// If the target has godmode in effect, they take no damage.
	if (targ->flags & FL_GODMODE && !(dflags & DAMAGE_NO_PROTECTION))
		take = 0;

	// If the player is invincible, or on a shrine, they take no damage.
	if ( (client && client->invincible_framenum > level.framenum ) && !(dflags & DAMAGE_NO_PROTECTION))
	{
		if (targ->pain_debounce_time < level.time)
		{
//			gi.sound(targ, CHAN_ITEM, gi.soundindex("items/protect4.wav"), 1, ATTN_NORM, 0);
			targ->pain_debounce_time = level.time + 2;
		}

		take = 0;
	}
	
	// If target and attacker are on the same team, don't inflict any damage.
	
	if(CheckTeamDamage (targ, attacker))
		return;

	// Okay, we got all the way here, so do the damage.
	if(take && !(targ->svflags & SVF_MONSTER && sv_freezemonsters->value != 0.0) && !(dflags & DAMAGE_ALL_KNOCKBACK))
	{
		int scale;
		int duration;

		//not damage_burning because that is coming from you being on fire already...
		if (dflags & DAMAGE_FIRE && flammable(targ) && dflags != DAMAGE_BURNING && !(targ->svflags & SVF_BOSS))
		{//FIXME: not on BBRUSHES - have no origin!

			if (dflags & DAMAGE_FIRE_LINGER)
				duration = orig_dmg*0.4;
			else
				duration = orig_dmg*0.2;
	
			if (!duration)
			{
				duration = 1;
			}
			else if (dflags & DAMAGE_PHOENIX)
			{	// The phoenix is just too damn powerful if it can do serious fire damage too...
				duration = 1;		// This makes a burning death if the player should die, but it goes out right away.
			}
			else if (duration > 8)
			{
				duration = 8;
			}

			scale = (int)(VectorLength(targ->size)*(0.5 * 0.25));	//is scled up drediculously on other side, quarter it
			if (scale > 255)
				scale = 255;

			if(!(gi.pointcontents(targ->s.origin) & (CONTENTS_WATER|CONTENTS_SLIME)))
			{//need to check if under water!
				if(targ->fire_damage_time < level.time && !(targ->svflags & SVF_BOSS))
				{//not already on fire
					if(targ->client && deathmatch->value)
						targ->fire_damage_time = level.time + duration * 0.25;//burn for 3.2 seconds- length of effect, if effect length changed, this should too!
					else
						targ->fire_damage_time = level.time + duration * 0.5;//burn for 3.2 seconds- length of effect, if effect length changed, this should too!
	
					if (!was_dead)
						targ->s.effects &= ~EF_DISABLE_EXTRA_FX;	// The flag causes the fire to stop generating.
					targ->s.effects |= EF_ON_FIRE;
					gi.CreateEffect(&targ->s, FX_FIRE_ON_ENTITY, CEF_OWNERS_ORIGIN, NULL, "bbb", (int)scale, 255, 1);//we'll turn this off manually
				}
				else
					targ->fire_damage_time += duration;

				// Always set the damage enemy to the most recent entity doing damage.
				targ->fire_damage_enemy = attacker;
			}
		}
		else if(!(dflags & DAMAGE_NO_BLOOD))
		{
			if ((targ->svflags & SVF_MONSTER) || (client))
			{
				int bloodamt;
				vec3_t vel, diff, loc;
				
				if (take > 80)
					bloodamt = 20;
				else
					bloodamt = take/4;

				// Normal will be NULL from time to time.  FIXME:  Hellbolts will damage with a plane.normal that is null.
				if (normal==NULL || Vec3IsZero(normal))
				{
					VectorClear(vel);
					VectorCopy(point, loc);
				}
				else
				{
					VectorScale(normal, -64.0, vel);

					// Now let's try moving the hit point towards the hit object.
					VectorSubtract(targ->s.origin, point, diff);
					// We can't be assured that the vertical origin is the center...
					diff[2] += (targ->maxs[2] + targ->mins[2])*0.5;
					// Take half that distance, since the hit always tends to be on the outside of the bbox.
					VectorMA(point, 0.5, diff, loc);
				}

				if(violence == VIOLENCE_NONE)
					gi.CreateEffect(NULL, FX_HITPUFF, CEF_FLAG6, point, "db", dir, 5);
				else if(targ->materialtype == MAT_INSECT)
					gi.CreateEffect(NULL, FX_BLOOD, CEF_FLAG8, loc, "ub", vel, bloodamt);
				else
					gi.CreateEffect(NULL, FX_BLOOD, 0, loc, "ub", vel, bloodamt);
			}
			else
			{
				if ((targ->svflags & SVF_DEADMONSTER ||targ->materialtype == MAT_INSECT||targ->materialtype == MAT_FLESH) && violence > VIOLENCE_NONE)
				{
					if(targ->materialtype == MAT_INSECT)
						gi.CreateEffect(NULL, FX_BLOOD, CEF_FLAG8, point, "ub", dir, 8);
					else
						gi.CreateEffect(NULL, FX_BLOOD, 0, point, "ub", dir, 8);
				}
				else
					gi.CreateEffect(NULL, FX_HITPUFF, 0, point, "db", dir, 5);
			}
		}
		else if (dflags & DAMAGE_BUBBLE)
		{
			vec3_t bubbleloc;

			VectorSet(bubbleloc, flrand(-10, 10), flrand(-10,10), targ->viewheight);
			VectorAdd(bubbleloc, targ->s.origin, bubbleloc);
			gi.CreateEffect(NULL, FX_BUBBLE, 0, bubbleloc, "");
		}

		targ->health -= take;
			
		if(targ!=attacker && violence > VIOLENCE_BLOOD)//can't dismember yourself
		{
			if(attacker==inflictor)
				VectorCopy(point, hit_spot);
			else
				VectorCopy(inflictor->s.origin, hit_spot);

			if(targ->classID != CID_HARPY)//use new hitlocation function
				hl = MG_GetHitLocation(targ, inflictor, point, dir);
			else if (!(targ->svflags & SVF_MONSTER)&&!(client))//target not a monster or client
				hl = T_GetHitLocation(targ, inflictor, hit_spot);
			else
				hl = T_GetHitLocation(targ, attacker, hit_spot);

			if(dflags&DAMAGE_DISMEMBER)
				hl |= hl_MeleeHit;//only melee can dismember Add the 16th bit to it for melee hit
			
			if(!(targ->svflags & SVF_PARTS_GIBBED) && !(dflags & DAMAGE_SUFFOCATION) && !(dflags & DAMAGE_BLEEDING))
			{//don't dismember someone who's already gibbed or gibbing, no dismember damage from suffocation or bleeding
				if(dflags&DAMAGE_DOUBLE_DISMEMBER)
					dsm_dmg = take * 2;
				else
					dsm_dmg = take;

				if(targ->client)
					player_dismember(targ, attacker, dsm_dmg, hl);
				else
					QPostMessage(targ, MSG_DISMEMBER, PRI_DIRECTIVE, "ii", dsm_dmg, hl);
			}
		}

		if (targ->health <= 0)
		{
//			if ((targ->svflags & SVF_MONSTER) || (client))
//				targ->flags |= FL_NO_KNOCKBACK;

			// Target has died, so kill it good and dead.
			if(was_dead)
			{//FIXME: if on fire, Become a charred husk, no gib.
				if(!(dflags & DAMAGE_SUFFOCATION) && !(dflags & DAMAGE_BLEEDING) && dflags != DAMAGE_BURNING)
				{//drowning, bleeding and burning do not gib
					if(targ->health<=-100)
					{
						if(targ->think != BecomeDebris && targ->think != G_SetToFree)
						{
							targ->post_think = BecomeDebris;
							targ->next_post_think = level.time;
						}
					}
					else if(violence > VIOLENCE_BLOOD)
					{
						hl|=hl_MeleeHit;//force dismember
						QPostMessage(targ, MSG_DEATH_PAIN, PRI_DIRECTIVE,"ii", take, hl);
					}
				}
				return;
			}

			if(targ->client)
			{
				if(dflags & DAMAGE_FIRE && !(targ->svflags & SVF_BOSS))
				{
					scale = (int)(VectorLength(targ->size)*(0.5*8.0));	// eight times the value is sent over, no more than 32 wide.
					if (scale > 255)
						scale = 255;
					targ->fire_damage_time = -1;//so we know we died from fire

					//spawn a fire to keep burning for ~ 6 secs
					if (!was_dead)
						targ->s.effects &= ~EF_DISABLE_EXTRA_FX;	// The flag causes the fire to stop generating.
					targ->s.effects |= EF_ON_FIRE;			// The flag causes the fire to stop generating.
					gi.CreateEffect(&targ->s, FX_FIRE_ON_ENTITY, CEF_OWNERS_ORIGIN, NULL, "bbb", (int)scale, 40, 0);
				}
			}

			if(!targ->takedamage)//already killed by decapitation or some other killing dismemberment
				return;

			Killed (targ, inflictor, attacker, take, point, MeansOfDeath);
			
			return;
		}
	}

	if (targ->svflags & SVF_MONSTER && sv_freezemonsters->value != 0.0)
	{
		// Do do anything.  Frozen monsters take no damage, don't die.
	}
	else if (targ->svflags & SVF_MONSTER)
	{
		if(!targ->enemy)
			force_pain = true;
		else
			force_pain = false;

		targ->spawnflags &= ~MSF_AMBUSH;
		targ->targetname = NULL;

		M_ReactToDamage (targ, attacker);

		if (!(targ->monsterinfo.aiflags & AI_DUCKED) && (take) && 
			(targ->pain_debounce_time  < level.time))
		{
			if(targ->classID == CID_ASSASSIN)
				QPostMessage(targ,MSG_PAIN,PRI_DIRECTIVE,"eeiii", inflictor, attacker, force_pain, take, hl);
			else
				QPostMessage(targ,MSG_PAIN,PRI_DIRECTIVE,"eeiii", targ, attacker, force_pain, take, hl);
			
			// In Nightmare skill-level, monsters don't go into pain frames often.

			if (skill->value >= 3)
				targ->pain_debounce_time = level.time + 5;
		}
	}
	else if (client)
	{
		if (!(targ->flags & FL_GODMODE) && (take))
			QPostMessage(targ,MSG_PAIN,PRI_DIRECTIVE,"eeiii", targ, attacker, knockback, take, hl);
	}
	else if (take)
	{
		if (targ->pain)
		{
			if(!targ->classID || !classStatics[targ->classID].msgReceivers[MSG_PAIN])
			{
				if(!stricmp(targ->classname, "NATE"))
					targ->activator = inflictor;
				targ->pain(targ, attacker, knockback, take);//pass spot too
			}
			else
				QPostMessage(targ,MSG_PAIN,PRI_DIRECTIVE,"eeiii", targ, attacker, knockback, take, hl);
		}
	}

	// Add to the damage inflicted on a player this frame. The total will be turned into screen
	// blends and view angle kicks at the end of the frame.

	if(client)
	{
		client->damage_gas = (!stricmp(inflictor->classname, "plague_mist") || !stricmp(inflictor->classname, "spreader_grenade")) ? true : false;

		client->damage_blood += take;
		client->damage_knockback += knockback;
		VectorCopy (point, client->damage_from);
	}
}

// ************************************************************************************************
// T_DamageRadius
// --------------
// ************************************************************************************************

void T_DamageRadius(edict_t *inflictor, edict_t *attacker, edict_t *ignore, float radius, 
							float maxdamage, float mindamage, int dflags,int MeansOfDeath)
{
	float	points, dist;
	edict_t	*ent = NULL;
	vec3_t	v, center;
	vec3_t	dir, hitspot;
	edict_t *damageenemy = NULL;

	assert(radius>0);

	if (dflags & DAMAGE_ENEMY_MAX)
	{
		damageenemy = inflictor->enemy;

		if (damageenemy && damageenemy->takedamage)
		{
			VectorAdd (damageenemy->mins, damageenemy->maxs, center);
			VectorMA (damageenemy->s.origin, 0.5, center, center);
			VectorSubtract (inflictor->s.origin, center, v);
			dist = VectorNormalize(v);
			VectorScale(v, -1, dir);
			VectorMA(center, damageenemy->maxs[0], v, hitspot);//estimate a good hit spot

			T_Damage(damageenemy, inflictor, attacker, dir, hitspot, vec3_origin,
					(int)maxdamage, (int)maxdamage, DAMAGE_RADIUS|dflags,MeansOfDeath);
		} 
	}
	while ((ent = findradius(ent, inflictor->s.origin, radius)) != NULL)
	{
		if (ent == ignore)
			continue;
		if (!ent->takedamage||ent->takedamage==DAMAGE_NO_RADIUS)
			continue;
		if (ent == attacker && dflags & DAMAGE_ATTACKER_IMMUNE)
			continue;
		if ((dflags & DAMAGE_ALIVE_ONLY) && (ent->materialtype != MAT_FLESH||ent->health<=0))
			continue;
		if (ent==damageenemy)	// We already dealt with the damageenemy above...
			continue;

		VectorAdd (ent->mins, ent->maxs, center);
		VectorMA (ent->s.origin, 0.5, center, center);
		VectorSubtract (inflictor->s.origin, center, v);
		dist = VectorNormalize(v);
		VectorScale(v, -1, dir);

		// Scale from maxdamage at center to mindamage at outer edge
		points = maxdamage - ((maxdamage-mindamage) * (dist/radius));

		if (points > 0)
		{
			if (CanDamage (ent, inflictor))
			{
				VectorMA(center, ent->maxs[0], v, hitspot);//estimate a good hit spot

				if (ent == attacker)
				{
					if (dflags & DAMAGE_ATTACKER_KNOCKBACK)
						T_Damage(ent, inflictor, attacker, dir, hitspot, vec3_origin, //hitspot was ent->s.origin
									0, (int)points, DAMAGE_RADIUS|dflags, MOD_KILLED_SLF);
					else if (dflags & DAMAGE_POWERPHOENIX)
						T_Damage(ent, inflictor, attacker, dir, hitspot, vec3_origin,	// extra knockback, .25 damage.
									(int)(points*0.25), (int)(points*0.5), DAMAGE_RADIUS|dflags,MOD_KILLED_SLF);
					else
						T_Damage(ent, inflictor, attacker, dir, hitspot, vec3_origin,
									(int)points, (int)points, DAMAGE_RADIUS|dflags, MOD_KILLED_SLF);
				}
				else
					T_Damage(ent, inflictor, attacker, dir, hitspot, vec3_origin,
								(int)points, (int)points, DAMAGE_RADIUS|dflags,MeansOfDeath);
			}
		}
	}
}


// Same function, except the origin point of the damage doesn't have to be the same as the inflictor's
void T_DamageRadiusFromLoc(vec3_t origin, edict_t *inflictor, edict_t *attacker, edict_t *ignore, float radius, 
							float maxdamage, float mindamage, int dflags,int MeansOfDeath)
{
	float	points, dist;
	edict_t	*ent = NULL;
	vec3_t	v, center, dir;
	vec3_t	hitspot;


	assert(radius>0);

	while ((ent = findradius(ent, origin, radius)) != NULL)
	{
		if (ent == ignore)
			continue;
		if (!ent->takedamage||ent->takedamage==DAMAGE_NO_RADIUS)
			continue;
		if (ent == attacker && dflags & DAMAGE_ATTACKER_IMMUNE)
			continue;
		if ((dflags & DAMAGE_ALIVE_ONLY) && (ent->materialtype != MAT_FLESH||ent->health<=0))
			continue;

		// if we are reflecting, stop us from taking damage
		if((EntReflecting(ent, true, true)))
			continue;

		VectorAdd (ent->mins, ent->maxs, center);
		VectorMA (ent->s.origin, 0.5, center, center);
		VectorSubtract (origin, center, v);
		dist = VectorNormalize(v);
		VectorScale(v, -1, dir);
		// Scale from maxdamage at center to mindamage at outer edge
		points = maxdamage - ((maxdamage-mindamage) * (dist/radius));
 
		if (points > 0)
		{
			if (CanDamageFromLoc (ent, inflictor, origin))
			{
				VectorMA(center, ent->maxs[0], v, hitspot);//estimate a good hit spot

				if (ent == attacker)
				{
					if (dflags & DAMAGE_ATTACKER_KNOCKBACK)
						T_Damage(ent, inflictor, attacker, dir, hitspot, vec3_origin, //hitspot was ent->s.origin
									0, (int)points, DAMAGE_RADIUS|dflags, MOD_KILLED_SLF);
					else if (dflags & DAMAGE_POWERPHOENIX)
						T_Damage(ent, inflictor, attacker, dir, hitspot, vec3_origin,	// extra knockback, .25 damage.
									(int)(points*0.25), (int)(points*2.0), DAMAGE_RADIUS|dflags,MOD_KILLED_SLF);
					else
						T_Damage(ent, inflictor, attacker, dir, hitspot, vec3_origin,
									(int)points, (int)points, DAMAGE_RADIUS|dflags,MOD_KILLED_SLF);
				}
				else
					T_Damage(ent, inflictor, attacker, dir, hitspot, vec3_origin,
								(int)points, (int)points, DAMAGE_RADIUS|dflags, MeansOfDeath);
			}
		}
	}
}
