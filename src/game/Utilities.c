#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN

#include <windows.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>

#include "Utilities.h"
#include "Angles.h"
#include "g_local.h"
#include "Vector.h"
#include "g_HitLocation.h"
#include "g_Physics.h"
#include "Random.h"
#include "FX.H"
#include "p_main.h"

//kill specific entitys at the begining of a cinematic
void remove_non_cinematic_entites(edict_t *owner)
{
	int i;
	edict_t	*ent = NULL;

	// firstly, search for RED RAIN DAMAGE entites
	while (1)
	{
		ent = G_Find (ent, FOFS(classname), "Spell_RedRain");
		if (!ent)
			break;
		if (!owner || (ent->owner && (ent->owner == owner)))
		{
			// kill the rain sound
			gi.sound(ent, CHAN_VOICE, gi.soundindex("misc/null.wav"), 1, ATTN_NORM,0);
			// remove the entity
			gi.RemoveEffects(&ent->s, FX_WEAPON_REDRAIN);
			G_SetToFree(ent);
		}
	}

	// then remove red rain arrows - in case of hitting something and starting the red rain
	ent = NULL;
	while (1)
	{
		ent = G_Find (ent, FOFS(classname), "Spell_RedRainArrow");
		if (!ent)
			break;
		if (!owner || (ent->owner && (ent->owner == owner)))
		{
			// kill the rain arrow travel sound
			gi.sound(ent, CHAN_VOICE, gi.soundindex("misc/null.wav"), 1, ATTN_NORM,0);
			// remove the entity
			gi.RemoveEffects(&ent->s, FX_WEAPON_REDRAINMISSILE);
			G_SetToFree(ent);
		}
	}

	// look for Powered up Mace balls - they've got to go too.
	ent = NULL;
	while (1)
	{
		ent = G_Find (ent, FOFS(classname), "Spell_Maceball");
		if (!ent)
			break;
		if (!owner || (ent->owner && (ent->owner == owner)))
		{
			// remove the entity
			G_SetToFree(ent);
		}
	}

	// look for Phoenix arrows - we should remove them
	ent = NULL;
	while (1)
	{
		ent = G_Find (ent, FOFS(classname), "Spell_PhoenixArrow");
		if (!ent)
			break;
		if (!owner || (ent->owner && (ent->owner == owner)))
		{
			// kill the phoneix arrow travel sound
			gi.sound(ent, CHAN_WEAPON, gi.soundindex("misc/null.wav"), 1, ATTN_NORM,0);
			// remove the entity
			G_SetToFree(ent);
		}
	}

	// look for Sphere - we should remove them
	ent = NULL;
	while (1)
	{
		ent = G_Find (ent, FOFS(classname), "Spell_SphereOfAnnihilation");
		if (!ent)
			break;
		if (!owner || (ent->owner && (ent->owner == owner)))
		{
			// kill the sphere grow sound 
			gi.sound(ent, CHAN_WEAPON, gi.soundindex("misc/null.wav"), 1, ATTN_NORM,0);
			// remove the entity
			G_SetToFree(ent);
		}
	}

	// look for meteor barriers - we should remove them
	ent = NULL;
	while (1)
	{
		ent = G_Find (ent, FOFS(classname), "Spell_MeteorBarrier");
		if (!ent)
			break;
		if (!owner || (ent->owner && (ent->owner == owner)))
		{
			// remove any persistant meteor effects
			if (ent->PersistantCFX)
			{
				gi.RemovePersistantEffect(ent->PersistantCFX, REMOVE_METEOR);
				gi.RemoveEffects(&ent->owner->s, FX_SPELL_METEORBARRIER+ent->health);
				ent->PersistantCFX = 0;
			}
			// kill the meteorbarrier ambient sound 
			ent->owner->client->Meteors[ent->health] = NULL;

			// now we've been cast, remove us from the count of meteors the caster owns, and turn off his looping sound if need be
			ent->owner->client->playerinfo.meteor_count &= ~(1<<ent->health);
			if (!ent->owner->client->playerinfo.meteor_count)
				ent->owner->s.sound = 0;

			// remove the entity
			G_SetToFree(ent);
		}
	}

	// Hide all other players and make them not clip
	ent = NULL;
	for (i=0 ; i<maxclients->value ; i++)
	{
		ent = g_edicts + 1 + i;
		if (!ent->inuse || !ent->client)
			continue;

		ent->client->playerinfo.cinematic_starttime = level.time;

		if (ent->client->playerinfo.powerup_timer > ent->client->playerinfo.cinematic_starttime)
			gi.RemoveEffects(&ent->s, FX_TOME_OF_POWER);

		if (ent->client->playerinfo.speed_timer > ent->client->playerinfo.cinematic_starttime)
			gi.RemoveEffects(&ent->s, FX_FOOT_TRAIL);

		if (ent->client->playerinfo.shield_timer > ent->client->playerinfo.leveltime)
		{
			gi.RemoveEffects(&ent->s, FX_SPELL_LIGHTNINGSHIELD);
			ent->client->playerinfo.cin_shield_timer = ent->client->playerinfo.shield_timer;
			ent->client->playerinfo.shield_timer = 0;
		}

		// No looping sound attached.
		ent->s.sound = 0;

		ent->curr_model = ent->s.modelindex;	// Temp holder, should be fine because player isn't doing anything during cinematics
		ent->client->playerinfo.c_mode = 1;		// Show it's in cinematic mode
		ent->s.modelindex = 0;
		ent->solid = SOLID_NOT;
	}

}
void player_shrine_light_effect(edict_t *self);
void shrine_light_core(edict_t *self, edict_t *other);

void reinstate_non_cinematic_entites(edict_t *owner)
{
	int i;
	edict_t	*ent = NULL;

	// Put client entities back in game
	ent = NULL;
	for (i=0 ; i<maxclients->value ; i++)
	{
		ent = g_edicts + 1 + i;
		if (!ent->inuse || !ent->client)
			continue;

		if (level.time > ent->client->playerinfo.cinematic_starttime)
		{

			if (ent->client->playerinfo.light_timer > ent->client->playerinfo.cinematic_starttime)
			{
				ent->client->playerinfo.light_timer += level.time - ent->client->playerinfo.cinematic_starttime;
				ent->s.effects |= EF_LIGHT_ENABLED;
				gi.CreateEffect(&ent->s, FX_PLAYER_TORCH, CEF_OWNERS_ORIGIN, NULL, "");
			}

			if (ent->client->playerinfo.reflect_timer > ent->client->playerinfo.cinematic_starttime)
			{
				ent->client->playerinfo.reflect_timer += level.time - ent->client->playerinfo.cinematic_starttime;
				ent->s.renderfx |= RF_REFLECTION;
			}

			if (ent->client->playerinfo.ghost_timer > ent->client->playerinfo.cinematic_starttime)
			{
				ent->client->playerinfo.ghost_timer += level.time - ent->client->playerinfo.cinematic_starttime;
				ent->s.renderfx |= RF_TRANS_GHOST;
			}

			if (ent->client->playerinfo.powerup_timer > ent->client->playerinfo.cinematic_starttime)
			{
				ent->client->playerinfo.powerup_timer += level.time - ent->client->playerinfo.cinematic_starttime;
				ent->s.effects |= EF_POWERUP_ENABLED;
				ent->client->playerinfo.effects |= EF_POWERUP_ENABLED;
				gi.CreateEffect(&ent->s, FX_TOME_OF_POWER, CEF_OWNERS_ORIGIN, NULL, "");
			}

			if (ent->client->playerinfo.cin_shield_timer > ent->client->playerinfo.cinematic_starttime)
			{
				ent->client->playerinfo.shield_timer = 
					ent->client->playerinfo.cin_shield_timer + level.time - ent->client->playerinfo.cinematic_starttime;
				ent->PersistantCFX = gi.CreatePersistantEffect(&ent->s, FX_SPELL_LIGHTNINGSHIELD, CEF_OWNERS_ORIGIN|CEF_BROADCAST, NULL, "");
			}

			if (ent->client->playerinfo.speed_timer > ent->client->playerinfo.cinematic_starttime)
			{
				ent->client->playerinfo.speed_timer += level.time - ent->client->playerinfo.cinematic_starttime;
				ent->s.effects |= EF_SPEED_ACTIVE;
				ent->client->playerinfo.effects |= EF_SPEED_ACTIVE;
				gi.CreateEffect(&ent->s, FX_FOOT_TRAIL, CEF_OWNERS_ORIGIN, NULL, "");
			}

			// since we messed around with model stuff, like armor nodes and the like, lets update the model.
			SetupPlayerinfo_effects(ent);
			P_PlayerUpdateModelAttributes(&ent->client->playerinfo);
			WritePlayerinfo_effects(ent);

		}
		
		ent->client->playerinfo.c_mode = 0;	// Show cinematic mode is off
		ent->s.modelindex = ent->curr_model;
		ent->solid = SOLID_BBOX;
	}

}

void GetEdictCenter(edict_t *self, vec3_t out)
{
	VectorAverage(self->mins, self->maxs, out);
	Vec3AddAssign(self->s.origin, out);
}

float NormalizeAngle(float angle) 
{
#if	1
	// Returns the remainder
	angle = fmod(angle, ANGLE_360);
	// Makes the angle signed
	if(angle >= ANGLE_180)
	{
		angle -= ANGLE_360;
	}
	if(angle <= -ANGLE_180)
	{
		angle += ANGLE_360;
	}
#else
	if (Q_fabs(angle) > 15 * ANGLE_360)
	{
		angle = (float)atan2(sin(angle), cos(angle));

		return angle;
	}

	while (angle < -ANGLE_180)
	{ 
		angle += ANGLE_360;
	}

	while (angle >= ANGLE_180)
	{ 
		angle -= ANGLE_360;
	}
#endif
	return angle;
}

float AddNormalizedAngles(float angle1, float angle2)
{
	float sum;

	if(angle1 >= 0)
	{
		if(angle2 >= 0)
		{
			sum = angle1 + angle2;

			if(sum >= ANGLE_180)
			{
				return (sum - ANGLE_360);
			}
		}
		else
		{
			return (angle1 + angle2);
		}
	}
	else
	{
		if(angle2 < 0)
		{
			sum = angle1 + angle2;

			if(sum < -ANGLE_180)
			{
				return (sum + ANGLE_360);
			}
		}
		else
		{
			return (angle1 + angle2);
		}
	}

	return sum;	
}

qboolean ok_to_autotarget(edict_t *shooter, edict_t *target)
{
	if((!target->inuse)||(target->solid==SOLID_NOT)||(target->health<=0)||(target==shooter)||(target->svflags&SVF_NO_AUTOTARGET))
		return(false);

	// Don't allow us to find our caster , if there is one.

	if (shooter->owner)
	{
		if (target == shooter->owner)
			return(false);
	}

	// Now test against single player / deathmatch / coop specifics.

	if(deathmatch->value)
	{
		// Only want to find other clients in deathmatch.

		if(!target->client)
			return(false);
	}
	else
	if (coop->value)
	{
		if(target->client && ((int)dmflags->value & DF_HURT_FRIENDS))
			return(true);
		else
		if((!(target->svflags&SVF_MONSTER))&&(!(target->svflags&SVF_ALLOW_AUTO_TARGET))) 
			return(false);
	}
	else
	{
		// Find just monsters in single player / coop. - unless the hurt friends flag is set
		if((!(target->svflags&SVF_MONSTER))&&(!(target->svflags&SVF_ALLOW_AUTO_TARGET))) 
			return(false);
	}

	return(true);
}

// ************************************************************************************************
// FindNearestVisibleActorInFrustum
// --------------------------------
// I copied FindNearestActorInFrustum() and modified it so that it can take line-of-sight into
// account if specified (i.e. LOSStartPos is not NULL). Additionally I relaxed the constraint that
// the horizontal search arc has to be [-180.0<=hFOV<=+180.0] so that homing missiles can see all
// around themselves when looking for a targetet 'lock'. -Marcus
// ************************************************************************************************

#define NEW_FINDNEAR (0)

#if !NEW_FINDNEAR


edict_t *FindNearestVisibleActorInFrustum(edict_t *Finder,vec3_t FinderAngles,
										  float nearDist,float farDist,
										  double hFOV,double vFOV,
										  long Flags,
										  vec3_t LOSStartPos,
										  vec3_t BBMin,vec3_t BBMax)
{
	vec3_t	distVect,
			_BBMin,_BBMax,
			TempVec;
	edict_t *end,*best,*ent;
	float	curDist,nearDist2,
			bestDist,
			curYaw,curPitch,
			minHFOV,maxHFOV,minVFOV,maxVFOV;
	float	baseYaw,distTemp,mag;
	trace_t Trace;

	assert(nearDist>=0.0);

	if(LOSStartPos)
	{	
		if(!BBMin)
			VectorClear(_BBMin);
		else
			VectorCopy(BBMin,_BBMin);

		if(!BBMax)
			VectorClear(_BBMax);
		else
			VectorCopy(BBMax,_BBMax);
	}

	bestDist = farDist * farDist;
	nearDist2 = nearDist * nearDist;

	minHFOV = -hFOV * 0.5;
	maxHFOV = -minHFOV;

	minVFOV = -vFOV * 0.5;
	maxVFOV = -minVFOV;

	baseYaw = NormalizeAngle(FinderAngles[YAW] * ANGLE_TO_RAD);
	best = NULL;

	end = &g_edicts[globals.num_edicts];

	for(ent = g_edicts + 1; ent < end; ent++)
	{
		// Ignore certain entities altogether.
		if(!ok_to_autotarget(Finder, ent))
			continue;

		// don't target ghosting players.
		if (ent->client && (ent->client->playerinfo.ghost_timer > level.time))
			continue;

		// Get the center (in world terms) of the entity (actually the center according to it's
		// bounding box).
		GetEdictCenter(ent, TempVec);

		// Ok, we can see the entity (or don't care whether we can or can't) so make the checks to
		// see if it lies within the specified frustum parameters.

		VectorSubtract(TempVec, Finder->s.origin, distVect);

		distTemp = distVect[Y] * distVect[Y] + distVect[X] * distVect[X];
		curDist = distTemp + distVect[Z] * distVect[Z];

		if((curDist >= nearDist2) && (curDist <= bestDist))
		{
			mag = sqrt(distTemp);

			curYaw = atan2(distVect[Y]/mag,distVect[X]/mag);

			curYaw = AddNormalizedAngles(curYaw,-baseYaw);

			if((curYaw>=minHFOV)&&(curYaw<=maxHFOV))
			{
				mag=sqrt(distVect[Y]*distVect[Y]+distVect[Z]*distVect[Z]);

				curPitch=asin(distVect[Z]/mag);

				if((curPitch>=minVFOV)&&(curPitch<=maxVFOV))
				{
					// If LOSStartPos is not NULL, we need a line of sight to the entity (see above), else
					// skip to the next entity.

					if(LOSStartPos)
					{	
						if(gi.inPVS(LOSStartPos, TempVec))
						{//cheaper than a trace
							gi.trace(LOSStartPos,				// Start pos.
										   _BBMin,					// Bounding box min.
										   _BBMax,					// Bounding box max.
										   TempVec,					// End pos.
										   Finder,					// Ignore this edict.
										   CONTENTS_SOLID,&Trace);	  		// Contents mask.

							if((Trace.fraction!=1.0)||(Trace.startsolid))
							{
								continue;
							}
						}
						else
							continue;
					}

					bestDist=curDist;
					best=ent;
				}
			}
		}
	}

	return(best);
}

#else


static float *SortOrigin;

int DistSort (void const *a, void const *b)
{
	vec3_t tmp;
	float da,db;
	edict_t *e;

	e=*(edict_t **)a;
	VectorSubtract(e->s.origin,SortOrigin,tmp);
	da=DotProduct(tmp,tmp);
	e=*(edict_t **)b;
	VectorSubtract(e->s.origin,SortOrigin,tmp);
	db=DotProduct(tmp,tmp);

	if (da < db)
		return -1;
	if (da > db)
		return 1;
	return 0;
}

edict_t *FindNearestVisibleActorInFrustum(edict_t *Finder,vec3_t FinderAngles,
										  float nearDist,float farDist,
										  double hFOV,double vFOV,
										  long Flags,
										  vec3_t LOSStartPos,
										  vec3_t BBMin,vec3_t BBMax)
{
	edict_t *touchlist[MAX_EDICTS];
	int i,num;
	vec3_t	minBox,maxBox;
	vec3_t	distVect,
			_BBMin,_BBMax,
			TempVec;
	edict_t *best,*ent;
	float	curDist,nearDist2,
			bestDist,
			curYaw,curPitch,
			minHFOV,maxHFOV,minVFOV,maxVFOV;
	float	baseYaw,distTemp,mag;
	trace_t Trace;

	assert(nearDist>=0.0);

	if(LOSStartPos)
	{	
		if(!BBMin)
			VectorClear(_BBMin);
		else
			VectorCopy(BBMin,_BBMin);

		if(!BBMax)
			VectorClear(_BBMax);
		else
			VectorCopy(BBMax,_BBMax);
	}

	bestDist = farDist * farDist;
	nearDist2 = nearDist * nearDist;

	minHFOV = -hFOV * 0.5;
	maxHFOV = -minHFOV;

	minVFOV = -vFOV * 0.5;
	maxVFOV = -minVFOV;

	baseYaw = NormalizeAngle(FinderAngles[YAW] * ANGLE_TO_RAD);
	best = NULL;

	for (i=0;i<3;i++)
	{
		minBox[i]=Finder->s.origin[i]-farDist;
		maxBox[i]=Finder->s.origin[i]+farDist;
	}
	num = gi.BoxEdicts(minBox,maxBox, touchlist, MAX_EDICTS, AREA_SOLID);
	if (!num)
		return 0;
	SortOrigin=Finder->s.origin;
	qsort(touchlist,num,sizeof(edict_t *),DistSort);

	for(i=0;i<num;i++)
	{
		ent=touchlist[i];
		// Ignore certain entities altogether.
		if(!ok_to_autotarget(Finder, ent))
			continue;

		// Get the center (in world terms) of the entity (actually the center according to it's
		// bounding box).
		GetEdictCenter(ent, TempVec);

		// Ok, we can see the entity (or don't care whether we can or can't) so make the checks to
		// see if it lies within the specified frustum parameters.

		VectorSubtract(TempVec, Finder->s.origin, distVect);

		distTemp = distVect[Y] * distVect[Y] + distVect[X] * distVect[X];
		curDist = distTemp + distVect[Z] * distVect[Z];

		if((curDist >= nearDist2) && (curDist <= bestDist))
		{
			mag = sqrt(distTemp);

			curYaw = atan2(distVect[Y]/mag,distVect[X]/mag);

			curYaw = AddNormalizedAngles(curYaw,-baseYaw);

			if((curYaw>=minHFOV)&&(curYaw<=maxHFOV))
			{
				mag=sqrt(distVect[Y]*distVect[Y]+distVect[Z]*distVect[Z]);

				curPitch=asin(distVect[Z]/mag);

				if((curPitch>=minVFOV)&&(curPitch<=maxVFOV))
				{
					// If LOSStartPos is not NULL, we need a line of sight to the entity (see above), else
					// skip to the next entity.

					if(LOSStartPos)
					{	
							gi.trace(LOSStartPos,				// Start pos.
										   _BBMin,					// Bounding box min.
										   _BBMax,					// Bounding box max.
										   TempVec,					// End pos.
										   Finder,					// Ignore this edict.
										   CONTENTS_SOLID,&Trace);	  		// Contents mask.

							if((Trace.fraction!=1.0)||(Trace.startsolid))
							{
								continue;
							}
					}

					bestDist=curDist;
					best=ent;
					break;
				}
			}
		}
	}

	return(best);
}


#endif


edict_t *FindSpellTargetInRadius(edict_t *searchent, float radius, vec3_t searchpos,
									  vec3_t mins, vec3_t maxs)
{
	vec3_t	distVect,
			_mins,_maxs,
			entpos;
	edict_t *ent,*best;
	float	curDist, bestDist;
	trace_t Trace;

	assert(radius>=0.0);
	assert(searchpos);

	if(!mins)
		VectorClear(_mins);
	else
		VectorCopy(mins, _mins);

	if(!maxs)
		VectorClear(_maxs);
	else
		VectorCopy(maxs, _maxs);

	bestDist = radius * radius;

	best = NULL;
	ent = NULL;
	while(ent = findradius(ent, searchpos, radius))
	{
		// Ignore certain entities altogether.
		if (ent == searchent || ent == searchent->owner)
			continue;

		if (!ent->takedamage && ent->health <= 0)
			continue;

		if(!ok_to_autotarget(searchent, ent))
			continue;

		// don't target ghosting players, or target players in coop.
		if (ent->client && (ent->client->playerinfo.ghost_timer > level.time))
			continue;

		// don't target team members in team deathmatching, if they are on the same team, and friendly fire is not enabled.
		if (((int)(dmflags->value) & (DF_MODELTEAMS | DF_SKINTEAMS)) && !((int)dmflags->value & DF_HURT_FRIENDS) && deathmatch->value)
		{
			if (OnSameTeam(ent, searchent->owner))
				continue;
		}

		// Get the center (in world terms) of the entity (actually the center according to it's
		// bounding box).
		GetEdictCenter(ent, entpos);

		// Ok, we can see the entity (or don't care whether we can or can't) so make the checks to
		// see if it lies within the specified frustum parameters.

		VectorSubtract(entpos, searchpos, distVect);

		curDist = distVect[Y] * distVect[Y] + distVect[X] * distVect[X] + distVect[Z] * distVect[Z];

		if(curDist <= bestDist)
		{
			if(gi.inPVS(searchpos, entpos))
			{//cheaper than a trace
				gi.trace(searchpos,						// Start pos.
							   _mins,					// Bounding box min.
							   _maxs,					// Bounding box max.
							   entpos,					// End pos.
							   searchent,				// Ignore this edict.
							   CONTENTS_SOLID, &Trace);	  		// Contents mask.

				if((Trace.fraction!=1.0)||(Trace.startsolid))
				{
					continue;
				}
				else
				{
					bestDist=curDist;
					best=ent;
				}
			}
		}
	}

	return(best);
}







// Pretty much the same as the above routine
// Except it is only for client to client

#define MAX_PLAYER_VIEW		1024.0F

// FIXME : Need to use cameras origin, not players origin

void CalculatePIV(edict_t *player)
{
	int			i, PIV;
	edict_t		*target;
	float		FOV;
	vec3_t		endpos, dist, movedir;
	vec3_t		mins, maxs, angles, org;
	trace_t		trace;
	int			frameidx, playeridx;

	// if we have no names on through deathmatch flags, don't send them down
	if(deathmatch->value && ((int)dmflags->value & DF_NONAMES))
	{
		player->client->ps.PIV = 0;
		return;
	}

	// Only update data once every 8 frames
	frameidx = level.framenum & 7;
	playeridx = (player->s.number - 1) & 7;
	if(frameidx != playeridx)
	{
		return;
	}


	PIV = 0;
	FOV	= cos(player->client->ps.fov * ANGLE_TO_RAD * 0.5);

	// Grab camera angles
	angles[0] = SHORT2ANGLE(player->client->playerinfo.pcmd.camera_viewangles[0]);
	angles[1] = SHORT2ANGLE(player->client->playerinfo.pcmd.camera_viewangles[1]);
	angles[2] = SHORT2ANGLE(player->client->playerinfo.pcmd.camera_viewangles[2]);
	AngleVectors(angles, movedir, NULL, NULL);

	// Grab camera coords
	org[0] = player->client->playerinfo.pcmd.camera_vieworigin[0] * 0.125F;
	org[1] = player->client->playerinfo.pcmd.camera_vieworigin[1] * 0.125F;
	org[2] = player->client->playerinfo.pcmd.camera_vieworigin[2] * 0.125F;

	VectorScale(player->mins, 0.25F, mins);
	VectorScale(player->maxs, 0.25F, maxs);

	// FIXME : Need some way of knowing whether client is valid or not
	for(i = 0, target = g_edicts + 1; i < game.maxclients; i++, target++)
	{
		assert(target->client);
		// Don`t do an in view check on yourself
		if(player == target)
		{
			continue;
		}
		if(!target->inuse)
		{
			continue;
		}
		if (target->s.renderfx & RF_TRANS_GHOST)
		{	// Can't target ghosts.
			continue;
		}
		if (target->light_level < 16)
		{	// Too dark to see
			continue;
		}
		// Get center of enemy
		GetEdictCenter(target, endpos);
		VectorSubtract(endpos, org, dist);

		// Check range to other player
		if(VectorNormalize(dist) > MAX_PLAYER_VIEW)
		{
			continue;
		}
		// Check in players FOV
		if(DotProduct(dist, movedir) < FOV)
		{
			continue;
		}
		if(!gi.inPVS(org, endpos))
		{
			continue;
		}
		gi.trace(org, mins, maxs, endpos, player, MASK_PLAYERSOLID,&trace);
		if(trace.ent == target)
		{
			PIV |= 1 << i;
		}
	}
	player->client->ps.PIV = PIV;
}

void GetVectorsToActor(edict_t *self, edict_t *actor, vec3_t vec)
{
	vec3_t		dest, source;

	GetEdictCenter(self, source);
	GetEdictCenter(actor, dest);
	VectorSubtract(dest, source, vec);
	VectorNormalize(vec);
}

void QPlaySound(edict_t *self, int sound, int channel)
{
	gi.sound (self, channel, classStatics[self->classID].resInfo->sounds[sound], 1, ATTN_NORM, 0);
}

void StartICScript(char *name)
{
	assert(!level.cinActive);

	level.cinActive = true;

	ICScript_Con(&level.inGameCin, name);
}

#define EXTRA_KNOCKBACK_PRE_MULT 2
#define EXTRA_KNOCKBACK_POST_Z_MULT 1.25

void CalculateKnockBack(vec3_t dir, float knockback, int flags, float mass, vec3_t vel)
{
	if(flags & DAMAGE_EXTRA_KNOCKBACK)
	{
		knockback *= EXTRA_KNOCKBACK_PRE_MULT;
	}

	VectorScale(dir, (KNOCK_BACK_MULTIPLIER * (float)knockback) / mass, vel);

	if(flags & DAMAGE_EXTRA_KNOCKBACK)
	{
		vel[2] *= EXTRA_KNOCKBACK_POST_Z_MULT;
	}
}

void PostKnockBack(edict_t *target, vec3_t dir, float knockback, int flags)
{
	vec3_t	vel;

	CalculateKnockBack(dir, knockback, flags, target->mass, vel);

	QPostMessage(target, G_MSG_KNOCKEDBACK, PRI_PHYSICS, "fffi", vel[0], vel[1], vel[2], flags);
}


// Gets aiming vector to enemy or uses default aimangles

void GetAimVelocity(edict_t *enemy, vec3_t org, vec_t speed, vec3_t AimAngles, vec3_t out)
{
	float	h_offs, v_offs;

	if(enemy)
	{
		VectorAverage(enemy->mins, enemy->maxs, out);		// Get center of model

		if(skill->value)
		{//if skill = 0, aim for center of chest, otherwise, offset it some
			h_offs = enemy->maxs[0] * 0.75;
			v_offs = enemy->maxs[2] * 0.5;
			out[0] += flrand(-h_offs, h_offs);
			out[1] += flrand(-h_offs, h_offs);
			out[2] += flrand(-v_offs, v_offs);
		}
		else
			out[2] += enemy->maxs[2] /2;

		Vec3AddAssign(enemy->s.origin, out);
		Vec3SubtractAssign(org, out);																					
		VectorNormalize(out);
	}
	else
	{
		AngleVectors(AimAngles, out, NULL, NULL);
	}
	Vec3ScaleAssign(speed, out);
}

void SetAnim(edict_t *self, int anim)
{
	monsterinfo_t *monsterinfo = &self->monsterinfo;

	assert(classStatics[self->classID].resInfo);
	assert(classStatics[self->classID].resInfo->animations);

	monsterinfo->currentmove = classStatics[self->classID].resInfo->animations[anim];

	//only reset the anim index if the new anim is diff. from your
	//current anim
	if(self->curAnimID != anim)
	{
		monsterinfo->currframeindex = 0;
		monsterinfo->nextframeindex = 0;
	}

	self->lastAnimID = self->curAnimID;

	self->curAnimID = anim;
}

// Returns true if it is time to think

qboolean ThinkTime(edict_t *self)
{
	if(!self->think)
	{
		return(false);
	}
	if(self->nextthink <= TIME_EPSILON)
	{
		return(false);
	}
	// Need an epsilon value to account for floating point error
	// The epsilon can be large because level.time goes up in increments of 0.1
	if((self->nextthink - level.time) > TIME_EPSILON) 
	{
		return(false);
	}
	return(true);
}

// end