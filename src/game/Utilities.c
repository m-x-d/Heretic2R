//
// Utilities.c
//
// Copyright 1998 Raven Software
//

#include "Utilities.h"
#include "g_cmds.h" //mxd
#include "FX.h"
#include "Random.h"
#include "Vector.h"
#include "g_local.h"

// Kill specific entities at the beginning of a cinematic.
void remove_non_cinematic_entites(const edict_t* owner) //TODO: rename to RemoveNonCinematicEntities.
{
	edict_t* ent = NULL;

	// Firstly, search for RED RAIN DAMAGE entities.
	while ((ent = G_Find(ent, FOFS(classname), "Spell_RedRain")) != NULL)
	{
		if (owner == NULL || owner == ent->owner)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("misc/null.wav"), 1.0f, ATTN_NORM, 0.0f); // Kill the rain sound.
			gi.RemoveEffects(&ent->s, FX_WEAPON_REDRAIN); // Remove the entity.

			G_SetToFree(ent);
		}
	}

	ent = NULL;

	// Then remove red rain arrows - in case of hitting something and starting the red rain.
	while ((ent = G_Find(ent, FOFS(classname), "Spell_RedRainArrow")) != NULL)
	{
		if (owner == NULL || owner == ent->owner)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("misc/null.wav"), 1.0f, ATTN_NORM, 0.0f); // Kill the rain arrow travel sound.
			gi.RemoveEffects(&ent->s, FX_WEAPON_REDRAINMISSILE); // Remove the entity.

			G_SetToFree(ent);
		}
	}

	ent = NULL;

	// Look for Powered up Mace balls - they've got to go too.
	while ((ent = G_Find(ent, FOFS(classname), "Spell_Maceball")) != NULL)
		if (owner == NULL || owner == ent->owner)
			G_SetToFree(ent); // Remove the entity.

	ent = NULL;

	// Look for Phoenix arrows - we should remove them.
	while ((ent = G_Find(ent, FOFS(classname), "Spell_PhoenixArrow")) != NULL)
	{
		if (owner == NULL || owner == ent->owner)
		{
			gi.sound(ent, CHAN_WEAPON, gi.soundindex("misc/null.wav"), 1.0f, ATTN_NORM, 0.0f); // Kill the phoenix arrow travel sound.
			G_SetToFree(ent); // Remove the entity.
		}
	}

	ent = NULL;

	// Look for Spheres of Annihilation - we should remove them.
	while ((ent = G_Find(ent, FOFS(classname), "Spell_SphereOfAnnihilation")) != NULL)
	{
		if (owner == NULL || owner == ent->owner)
		{
			gi.sound(ent, CHAN_WEAPON, gi.soundindex("misc/null.wav"), 1, ATTN_NORM, 0); // Kill the sphere grow sound.
			G_SetToFree(ent); // Remove the entity.
		}
	}

	ent = NULL;

	// Look for meteor barriers - we should remove them.
	while ((ent = G_Find(ent, FOFS(classname), "Spell_MeteorBarrier")) != NULL)
	{
		if (owner == NULL || owner == ent->owner)
		{
			// Remove any persistent meteor effects.
			if (ent->PersistantCFX > 0)
			{
				gi.RemovePersistantEffect(ent->PersistantCFX, REMOVE_METEOR);
				gi.RemoveEffects(&ent->owner->s, FX_SPELL_METEORBARRIER + ent->health);
				ent->PersistantCFX = 0;
			}

			// Kill the meteor barrier ambient sound.
			ent->owner->client->Meteors[ent->health] = NULL;

			// Now we've been cast, remove us from the count of meteors the caster owns, and turn off his looping sound if need be.
			ent->owner->client->playerinfo.meteor_count &= ~(1 << ent->health);

			if (ent->owner->client->playerinfo.meteor_count == 0)
				ent->owner->s.sound = 0;

			// Remove the entity.
			G_SetToFree(ent);
		}
	}

	// Hide all other players and make them not clip.
	for (int i = 0; i < MAXCLIENTS; i++)
	{
		edict_t* player = &g_edicts[i + 1];
		playerinfo_t* info = &player->client->playerinfo; //mxd

		if (!player->inuse || player->client == NULL)
			continue;

		info->cinematic_starttime = level.time;

		if (info->powerup_timer > info->cinematic_starttime)
			gi.RemoveEffects(&player->s, FX_TOME_OF_POWER);

		if (info->speed_timer > info->cinematic_starttime)
			gi.RemoveEffects(&player->s, FX_FOOT_TRAIL);

		if (info->shield_timer > info->leveltime)
		{
			gi.RemoveEffects(&player->s, FX_SPELL_LIGHTNINGSHIELD);
			info->cin_shield_timer = info->shield_timer;
			info->shield_timer = 0;
		}

		// No looping sound attached.
		player->s.sound = 0;

		player->curr_model = player->s.modelindex; // Temp holder, should be fine because player isn't doing anything during cinematics.
		info->c_mode = 1; // Show it's in cinematic mode.
		player->s.modelindex = 0;
		player->solid = SOLID_NOT;
	}
}

void reinstate_non_cinematic_entites(edict_t* owner) //TODO: rename to ReinstateNonCinematicEntities, remove unused arg.
{
	// Put client entities back in game.
	for (int i = 0; i < MAXCLIENTS; i++)
	{
		edict_t* player = &g_edicts[i + 1];
		playerinfo_t* info = &player->client->playerinfo; //mxd

		if (!player->inuse || player->client == NULL)
			continue;

		if (level.time > info->cinematic_starttime)
		{

			if (info->light_timer > info->cinematic_starttime)
			{
				info->light_timer += level.time - info->cinematic_starttime;
				player->s.effects |= EF_LIGHT_ENABLED;
				gi.CreateEffect(&player->s, FX_PLAYER_TORCH, CEF_OWNERS_ORIGIN, NULL, "");
			}

			if (info->reflect_timer > info->cinematic_starttime)
			{
				info->reflect_timer += level.time - info->cinematic_starttime;
				player->s.renderfx |= RF_REFLECTION;
			}

			if (info->ghost_timer > info->cinematic_starttime)
			{
				info->ghost_timer += level.time - info->cinematic_starttime;
				player->s.renderfx |= RF_TRANS_GHOST;
			}

			if (info->powerup_timer > info->cinematic_starttime)
			{
				info->powerup_timer += level.time - info->cinematic_starttime;
				player->s.effects |= EF_POWERUP_ENABLED;
				info->effects |= EF_POWERUP_ENABLED;
				gi.CreateEffect(&player->s, FX_TOME_OF_POWER, CEF_OWNERS_ORIGIN, NULL, "");
			}

			if (info->cin_shield_timer > info->cinematic_starttime)
			{
				info->shield_timer = info->cin_shield_timer + level.time - info->cinematic_starttime;
				player->PersistantCFX = gi.CreatePersistantEffect(&player->s, FX_SPELL_LIGHTNINGSHIELD, CEF_OWNERS_ORIGIN | CEF_BROADCAST, NULL, "");
			}

			if (info->speed_timer > info->cinematic_starttime)
			{
				info->speed_timer += level.time - info->cinematic_starttime;
				player->s.effects |= EF_SPEED_ACTIVE;
				info->effects |= EF_SPEED_ACTIVE;
				gi.CreateEffect(&player->s, FX_FOOT_TRAIL, CEF_OWNERS_ORIGIN, NULL, "");
			}

			// Since we messed around with model stuff, like armor nodes and the like, lets update the model.
			SetupPlayerinfo_effects(player);
			P_PlayerUpdateModelAttributes(&player->client->playerinfo);
			WritePlayerinfo_effects(player);
		}

		info->c_mode = 0; // Show cinematic mode is off.
		player->s.modelindex = (byte)player->curr_model;
		player->solid = SOLID_BBOX;
	}
}

static void GetEdictCenter(const edict_t* self, vec3_t out)
{
	VectorAverage(self->mins, self->maxs, out);
	Vec3AddAssign(self->s.origin, out);
}

//mxd. Copy of ClampAngleRad() from netmsg_write.c...
static float NormalizeAngle(float angle)
{
	// Returns the remainder.
	angle = fmodf(angle, ANGLE_360);

	// Makes the angle signed.
	if (angle >= ANGLE_180)
		angle -= ANGLE_360;

	if (angle <= -ANGLE_180)
		angle += ANGLE_360;

	return angle;
}

static float AddNormalizedAngles(const float angle1, const float angle2)
{
	const float sum = angle1 + angle2;

	if (angle1 >= 0.0f)
	{
		if (angle2 >= 0.0f && sum >= ANGLE_180)
			return sum - ANGLE_360;
	}
	else
	{
		if (angle2 < 0.0f && sum < -ANGLE_180)
			return sum + ANGLE_360;
	}

	return sum;
}

qboolean ok_to_autotarget(const edict_t* shooter, const edict_t* target) //TODO: rename to OkToAutotarget.
{
	if (!target->inuse || target->solid == SOLID_NOT || target->health <= 0 || target == shooter || (target->svflags & SVF_NO_AUTOTARGET))
		return false;

	// Don't allow us to auto-target our caster, if there is one.
	if (shooter->owner != NULL && shooter->owner == target)
		return false;

	// Now test against deathmatch / coop / single player specifics.
	if (DEATHMATCH)
		return (target->client != NULL); // Only want to auto-target other clients in deathmatch.

	if (COOP && target->client != NULL && (DMFLAGS & DF_HURT_FRIENDS))
		return true;

	// Find just monsters in single player / coop, unless the hurt friends flag is set.
	return ((target->svflags & SVF_MONSTER) || (target->svflags & SVF_ALLOW_AUTO_TARGET));
}

// I copied FindNearestActorInFrustum() and modified it so that it can take line-of-sight into account if specified (i.e. LOSStartPos is not NULL).
// Additionally I relaxed the constraint that the horizontal search arc has to be [-180.0 <= hFOV <= +180.0], so that homing missiles
// can see all around themselves when looking for a targetet 'lock'. -Marcus
//TODO: remove unused 'flags' arg, change 'h_fov' and 'v_fov' types to float?
edict_t* FindNearestVisibleActorInFrustum(const edict_t* finder, const vec3_t finder_angles, const float near_dist, const float far_dist, const double h_fov, const double v_fov, long flags, const vec3_t los_start_pos, const vec3_t bb_min, const vec3_t bb_max)
{
	vec3_t bbmin;
	vec3_t bbmax;

	assert(near_dist >= 0.0f);

	// Initialize bbox.
	if (los_start_pos != NULL)
	{
		if (bb_min == NULL)
			VectorClear(bbmin);
		else
			VectorCopy(bb_min, bbmin);

		if (bb_max == NULL)
			VectorClear(bbmax);
		else
			VectorCopy(bb_max, bbmax);
	}

	float best_dist = far_dist * far_dist;
	const float near_dist_sq = near_dist * near_dist;

	const float min_horiz_fov = (float)(-h_fov * 0.5);
	const float max_horiz_fov = -min_horiz_fov;

	const float min_vert_fov = (float)(-v_fov * 0.5);
	const float max_vert_fov = -min_vert_fov;

	const float base_yaw = NormalizeAngle(finder_angles[YAW] * ANGLE_TO_RAD);
	edict_t* best = NULL;

	const edict_t* end = &g_edicts[globals.num_edicts];

	for (edict_t* e = &g_edicts[1]; e < end; e++)
	{
		// Ignore certain entities altogether.
		if (!ok_to_autotarget(finder, e))
			continue;

		// Don't target ghosting players.
		if (e->client != NULL && e->client->playerinfo.ghost_timer > level.time)
			continue;

		// Get the center (in world terms) of the entity (actually the center according to it's bounding box).
		vec3_t end_pos;
		GetEdictCenter(e, end_pos);

		// Ok, we can see the entity (or don't care whether we can or can't), so make the checks to
		// see if it lies within the specified frustum parameters.

		vec3_t dist_vec;
		VectorSubtract(end_pos, finder->s.origin, dist_vec);

		// Check distance.
		const float cur_dist_xy_sq = dist_vec[1] * dist_vec[1] + dist_vec[0] * dist_vec[0];
		const float cur_dist_sq = cur_dist_xy_sq + dist_vec[2] * dist_vec[2];

		if (cur_dist_sq < near_dist_sq || cur_dist_sq > best_dist)
			continue;

		// Check if in horizontal FOV.
		float mag = sqrtf(cur_dist_xy_sq);
		float cur_yaw = atan2f(dist_vec[1] / mag, dist_vec[0] / mag);
		cur_yaw = AddNormalizedAngles(cur_yaw, -base_yaw);

		if (cur_yaw < min_horiz_fov || cur_yaw > max_horiz_fov)
			continue;

		// Check if in vertical FOV.
		mag = sqrtf(dist_vec[1] * dist_vec[1] + dist_vec[2] * dist_vec[2]);
		const float cur_pitch = asinf(dist_vec[2] / mag);

		if (cur_pitch < min_vert_fov || cur_pitch > max_vert_fov)
			continue;

		// If los_start_pos is not NULL, we need a line of sight to the entity (see above), else skip to the next entity.
		if (los_start_pos != NULL)
		{
			if (!gi.inPVS(los_start_pos, end_pos)) // Cheaper than a trace.
				continue;

			trace_t trace;
			gi.trace(los_start_pos, bbmin, bbmax, end_pos, finder, CONTENTS_SOLID, &trace);

			if (trace.startsolid || trace.fraction != 1.0f)
				continue;
		}

		best_dist = cur_dist_sq;
		best = e;
	}

	return best;
}

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
	while(ent = FindInRadius(ent, searchpos, radius))
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

		curDist = distVect[1] * distVect[1] + distVect[0] * distVect[0] + distVect[2] * distVect[2];

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