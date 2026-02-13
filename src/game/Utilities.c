//
// Utilities.c
//
// Copyright 1998 Raven Software
//

#include "Utilities.h"
#include "g_cmds.h" //mxd
#include "p_client.h" //mxd
#include "p_view.h" //mxd
#include "FX.h"
#include "Random.h"
#include "Vector.h"
#include "g_local.h"

// Kill specific entities at the beginning of a cinematic.
void RemoveNonCinematicEntities(void) // mxd. Named 'remove_non_cinematic_entites' in original version.
{
	edict_t* ent = NULL;

	// Firstly, search for RED RAIN DAMAGE entities.
	while ((ent = G_Find(ent, FOFS(classname), "Spell_RedRain")) != NULL)
	{
		gi.sound(ent, CHAN_VOICE, gi.soundindex("misc/null.wav"), 1.0f, ATTN_NORM, 0.0f); // Kill the rain sound.
		gi.RemoveEffects(&ent->s, FX_WEAPON_REDRAIN); // Remove the entity.

		G_SetToFree(ent);
	}

	ent = NULL;

	// Then remove red rain arrows - in case of hitting something and starting the red rain.
	while ((ent = G_Find(ent, FOFS(classname), "Spell_RedRainArrow")) != NULL)
	{
		gi.sound(ent, CHAN_VOICE, gi.soundindex("misc/null.wav"), 1.0f, ATTN_NORM, 0.0f); // Kill the rain arrow travel sound.
		gi.RemoveEffects(&ent->s, FX_WEAPON_REDRAINMISSILE); // Remove the entity.

		G_SetToFree(ent);
	}

	ent = NULL;

	// Look for Powered up Mace balls - they've got to go too.
	while ((ent = G_Find(ent, FOFS(classname), "Spell_Maceball")) != NULL)
		G_SetToFree(ent); // Remove the entity.

	ent = NULL;

	// Look for Phoenix arrows - we should remove them.
	while ((ent = G_Find(ent, FOFS(classname), "Spell_PhoenixArrow")) != NULL)
	{
		gi.sound(ent, CHAN_WEAPON, gi.soundindex("misc/null.wav"), 1.0f, ATTN_NORM, 0.0f); // Kill the phoenix arrow travel sound.
		G_SetToFree(ent); // Remove the entity.
	}

	ent = NULL;

	// Look for Spheres of Annihilation - we should remove them.
	while ((ent = G_Find(ent, FOFS(classname), "Spell_SphereOfAnnihilation")) != NULL)
	{
		gi.sound(ent, CHAN_WEAPON, gi.soundindex("misc/null.wav"), 1, ATTN_NORM, 0); // Kill the sphere grow sound.
		G_SetToFree(ent); // Remove the entity.
	}

	ent = NULL;

	// Look for meteor barriers - we should remove them.
	while ((ent = G_Find(ent, FOFS(classname), "Spell_MeteorBarrier")) != NULL)
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

	// Hide all other players and make them not clip.
	for (int i = 0; i < MAXCLIENTS; i++)
	{
		edict_t* player = &g_edicts[i + 1];

		if (!player->inuse || player->client == NULL)
			continue;

		playerinfo_t* info = &player->client->playerinfo; //mxd
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

void ReinstateNonCinematicEntities(void) // mxd. Named 'reinstate_non_cinematic_entites' in original version.
{
	// Put client entities back in game.
	for (int i = 0; i < MAXCLIENTS; i++)
	{
		edict_t* player = &g_edicts[i + 1];

		if (!player->inuse || player->client == NULL)
			continue;

		playerinfo_t* info = &player->client->playerinfo; //mxd

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
				gi.CreateEffect(&player->s, FX_TOME_OF_POWER, CEF_OWNERS_ORIGIN, NULL, ""); //TODO: since we now have fx_tome fade-in animation, we'll need to pass some sort of "resume effect" or "skip fade-in animation" effect arg here...
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
			Player_UpdateModelAttributes(player); //mxd
		}

		info->c_mode = 0; // Show cinematic mode is off.
		player->s.modelindex = (byte)player->curr_model;
		player->solid = SOLID_BBOX;
	}
}

void GetEdictCenter(const edict_t* self, vec3_t out)
{
	VectorAverage(self->mins, self->maxs, out);
	Vec3AddAssign(self->s.origin, out);
}

//mxd. Copy of ClampAngleRad() from netmsg_write.c...
float NormalizeAngleRad(float angle) //TODO: never used. Remove?
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

//mxd. Degrees version of ClampAngleRad() from netmsg_write.c...
float NormalizeAngleDeg(float angle)
{
	// Returns the remainder.
	angle = fmodf(angle, 360.0f);

	// Makes the angle signed.
	if (angle > 180.0f)
		angle -= 360.0f;

	if (angle < -180.0f)
		angle += 360.0f;

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

qboolean OkToAutotarget(const edict_t* shooter, const edict_t* target) // mxd. Named 'ok_to_autotarget' in original version.
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
// can see all around themselves when looking for a targeted 'lock'. -Marcus
edict_t* FindNearestVisibleActorInFrustum(const edict_t* finder, const vec3_t finder_angles, const float near_dist, const float far_dist, const float h_fov, const float v_fov, const qboolean scale_fov_by_dist, const vec3_t los_start_pos) //mxd. Remove 'bb_min' and 'bb_max' args (never used), add 'scale_fov_by_dist' arg.
{
	assert(near_dist >= 0.0f);

	const float horiz_fov = h_fov * 0.5f;
	const float vert_fov =  v_fov * 0.5f;

	const float base_yaw = NormalizeAngleDeg(finder_angles[YAW]) * ANGLE_TO_RAD;
	const float base_pitch = NormalizeAngleDeg(finder_angles[PITCH]) * ANGLE_TO_RAD; //mxd

	float best_dist = far_dist;
	float best_yaw = ANGLE_360; //mxd
	edict_t* best = NULL;

	const edict_t* end = &g_edicts[globals.num_edicts];

	for (edict_t* e = &g_edicts[1]; e < end; e++)
	{
		// Ignore certain entities altogether.
		if (!OkToAutotarget(finder, e))
			continue;

		// Don't target ghosting players.
		if (e->client != NULL && e->client->playerinfo.ghost_timer > level.time)
			continue;

		// Get the center (in world terms) of the entity (actually the center according to it's bounding box).
		vec3_t end_pos;
		GetEdictCenter(e, end_pos);

		// Get direction to entity.
		vec3_t dir;
		VectorSubtract(end_pos, los_start_pos, dir); //mxd. Original logic uses 'finder->s.origin' instead of 'los_start_pos' here.

		const float cur_dist = VectorNormalize(dir);

		// Check if within expected range.
		if (cur_dist < near_dist || cur_dist > far_dist)
			continue;

		//mxd. Scale FOV by distance?
		const float fov_scaler = (scale_fov_by_dist ? Clamp(1.0f - (cur_dist / far_dist), 0.1f, 1.0f) : 1.0f);

		// Check if in horizontal FOV.
		float cur_yaw = atan2f(dir[YAW], dir[PITCH]); // See AnglesFromDir() --mxd.
		cur_yaw = fabsf(AddNormalizedAngles(cur_yaw, -base_yaw)); //mxd. +fabsf().

		if (cur_yaw > horiz_fov * fov_scaler || cur_yaw > best_yaw) //mxd. Also check best_yaw.
			continue;

		// Check distance.
		if (cur_dist > best_dist + 256.0f) //mxd. Allow targets further than current best when they are closer to finder_angles direction.
			continue;

		// Check if in vertical FOV.
		float cur_pitch = asinf(dir[ROLL]); // See AnglesFromDir() --mxd.
		cur_pitch = fabsf(AddNormalizedAngles(cur_pitch, base_pitch)); //H2_BUGFIX: mxd. Original logic doesn't do this.

		if (cur_pitch > vert_fov * fov_scaler)
			continue;

		// Check line of sight to the entity.
		if (!gi.inPVS(los_start_pos, end_pos)) // Cheaper than a trace.
			continue;

		trace_t trace;
		gi.trace(los_start_pos, vec3_origin, vec3_origin, end_pos, finder, CONTENTS_SOLID, &trace);

		if (trace.startsolid || trace.fraction != 1.0f)
			continue;

		// Valid result.
		best_yaw = cur_yaw; //mxd
		best_dist = min(best_dist, cur_dist);
		best = e;
	}

	return best;
}

edict_t* FindSpellTargetInRadius(const edict_t* search_ent, const float radius, const vec3_t search_pos, const vec3_t mins, const vec3_t maxs)
{
	vec3_t bb_mins;
	vec3_t bb_maxs;

	assert(radius >= 0.0f);
	assert(search_pos != NULL);

	if (mins == NULL)
		VectorClear(bb_mins);
	else
		VectorCopy(mins, bb_mins);

	if (maxs == NULL)
		VectorClear(bb_maxs);
	else
		VectorCopy(maxs, bb_maxs);

	float best_dist_sq = radius * radius;

	edict_t* best = NULL;
	edict_t* ent = NULL;

	while ((ent = FindInRadius(ent, search_pos, radius)) != NULL)
	{
		// Ignore certain entities altogether.
		if (ent == search_ent || ent == search_ent->owner)
			continue;

		if (ent->takedamage == DAMAGE_NO && ent->health <= 0)
			continue;

		if (!OkToAutotarget(search_ent, ent))
			continue;

		// Don't target ghosting players, or target players in coop.
		if (ent->client && (ent->client->playerinfo.ghost_timer > level.time))
			continue;

		// Don't target team members in team deathmatch, if they are on the same team, and friendly fire is not enabled.
		if (DEATHMATCH && (DMFLAGS & (DF_MODELTEAMS | DF_SKINTEAMS)) && !(DMFLAGS & DF_HURT_FRIENDS))
			if (OnSameTeam(ent, search_ent->owner))
				continue;

		// Get the center (in world terms) of the entity (actually the center according to it's bounding box).
		vec3_t ent_pos;
		GetEdictCenter(ent, ent_pos);

		// Ok, we can see the entity (or don't care whether we can or can't) so make the checks to
		// see if it lies within the specified frustum parameters.

		const float cur_dist_sq = VectorSeparationSquared(ent_pos, search_pos);

		if (cur_dist_sq > best_dist_sq || !gi.inPVS(search_pos, ent_pos)) // Cheaper than a trace.
			continue;

		trace_t trace;
		gi.trace(search_pos, bb_mins, bb_maxs, ent_pos, search_ent, CONTENTS_SOLID, &trace);

		if (!trace.startsolid && trace.fraction == 1.0f)
		{
			best_dist_sq = cur_dist_sq;
			best = ent;
		}
	}

	return best;
}

void CalculatePIV(const edict_t* player)
{
#define MAX_PLAYER_VIEW		1024.0f

	// If we have no names on through deathmatch flags, don't send them down.
	if (DEATHMATCH && (DMFLAGS & DF_NONAMES))
	{
		player->client->ps.PIV = 0;
		return;
	}

	// Only update data once every 8 frames.
	const int frame_idx = level.framenum & 7;
	const int player_idx = (player->s.number - 1) & 7;

	if (frame_idx != player_idx)
		return;

	int piv = 0;
	const float fov = cosf(player->client->ps.fov * ANGLE_TO_RAD * 0.5f);

	// Grab camera angles.
	vec3_t angles;
	for (int i = 0; i < 3; i++)
		angles[i] = SHORT2ANGLE(player->client->playerinfo.pcmd.camera_viewangles[i]);

	vec3_t move_dir;
	AngleVectors(angles, move_dir, NULL, NULL);

	// Grab camera coords.
	vec3_t org;
	for (int i = 0; i < 3; i++)
		org[i] = (float)player->client->playerinfo.pcmd.camera_vieworigin[i] / 8.0f;

	vec3_t mins;
	vec3_t maxs;
	VectorScale(player->mins, 0.25f, mins);
	VectorScale(player->maxs, 0.25f, maxs);

	//FIXME: need some way of knowing whether client is valid or not.
	for (int i = 0; i < game.maxclients; i++)
	{
		const edict_t* target = &g_edicts[i + 1];

		assert(target->client != NULL);

		// Don`t do an in-view check on yourself.
		if (player == target || !target->inuse)
			continue;

		// Can't target ghosts or too dark to see.
		if ((target->s.renderfx & RF_TRANS_GHOST) || target->light_level < 16)
			continue;

		// Get center of enemy.
		vec3_t end_pos;
		GetEdictCenter(target, end_pos);

		vec3_t dist;
		VectorSubtract(end_pos, org, dist);

		// Check range to other player.
		if (VectorNormalize(dist) > MAX_PLAYER_VIEW)
			continue;

		// Check in players FOV.
		if (DotProduct(dist, move_dir) < fov || !gi.inPVS(org, end_pos))
			continue;

		trace_t trace;
		gi.trace(org, mins, maxs, end_pos, player, MASK_PLAYERSOLID, &trace);

		if (trace.ent == target)
			piv |= 1 << i;
	}

	player->client->ps.PIV = piv;
}

static void CalculateKnockBack(const vec3_t dir, float knockback, const int flags, const float mass, vec3_t vel)
{
#define EXTRA_KNOCKBACK_PRE_MULT		2
#define EXTRA_KNOCKBACK_POST_Z_MULT		1.25f

	if (flags & DAMAGE_EXTRA_KNOCKBACK)
		knockback *= EXTRA_KNOCKBACK_PRE_MULT;

	VectorScale(dir, KNOCK_BACK_MULTIPLIER * knockback / mass, vel);

	if (flags & DAMAGE_EXTRA_KNOCKBACK)
		vel[2] *= EXTRA_KNOCKBACK_POST_Z_MULT;
}

void PostKnockBack(edict_t* target, const vec3_t dir, const float knockback, const int flags)
{
	vec3_t vel;
	CalculateKnockBack(dir, knockback, flags, (float)target->mass, vel);
	G_PostMessage(target, G_MSG_KNOCKEDBACK, PRI_PHYSICS, "fffi", vel[0], vel[1], vel[2], flags);
}

// Gets aiming vector to enemy or uses default aimangles.
void GetAimVelocity(const edict_t* enemy, const vec3_t org, const float speed, const vec3_t aim_angles, vec3_t out)
{
	if (enemy != NULL)
	{
		VectorAverage(enemy->mins, enemy->maxs, out); // Get center of model

		if (SKILL > SKILL_EASY)
		{
			// If skill = 0, aim for center of chest, otherwise, offset it some.
			const float h_offs = enemy->maxs[0] * 0.75f;
			const float v_offs = enemy->maxs[2] * 0.5f;

			out[0] += flrand(-h_offs, h_offs);
			out[1] += flrand(-h_offs, h_offs);
			out[2] += flrand(-v_offs, v_offs);
		}
		else
		{
			out[2] += enemy->maxs[2] / 2.0f;
		}

		Vec3AddAssign(enemy->s.origin, out);
		Vec3SubtractAssign(org, out);
		VectorNormalize(out);
	}
	else
	{
		AngleVectors(aim_angles, out, NULL, NULL);
	}

	Vec3ScaleAssign(speed, out);
}

//mxd. Calculate direction towards crosshair position calculated in Get_Crosshair().
static void AdjustAimDirection(const edict_t* caster, const vec3_t start_pos, const vec3_t aim_angles, const float view_offset_z, vec3_t direction)
{
	// Replicate relevant case from Get_Crosshair()...
	vec3_t forward;
	AngleVectors(aim_angles, forward, NULL, NULL);

	vec3_t end;
	const vec3_t view_pos = { caster->s.origin[0], caster->s.origin[1], caster->s.origin[2] + (float)caster->viewheight + view_offset_z };
	VectorMA(view_pos, 4096.0f, forward, end);

	// Adjust aim angles to match crosshair logic...
	trace_t trace;
	gi.trace(view_pos, vec3_origin, vec3_origin, end, caster, MASK_SHOT, &trace);

	VectorSubtract(trace.endpos, start_pos, direction);
	VectorNormalize(direction);
}

//mxd. Calculate velocity towards crosshair position calculated in Get_Crosshair().
void AdjustAimVelocity(const edict_t* caster, const vec3_t start_pos, const vec3_t aim_angles, const float projectle_speed, const float view_offset_z, vec3_t velocity)
{
	vec3_t dir;
	AdjustAimDirection(caster, start_pos, aim_angles, view_offset_z, dir);
	VectorScale(dir, projectle_speed, velocity);
}

//mxd. Adjust aim_angles to aim at crosshair position calculated in Get_Crosshair().
void AdjustAimAngles(const edict_t* caster, const vec3_t start_pos, const vec3_t aim_angles, const float view_offset_z, vec3_t angles)
{
	vec3_t dir;
	AdjustAimDirection(caster, start_pos, aim_angles, view_offset_z, dir);

	vectoangles(dir, angles);
	angles[PITCH] *= -1.0f; //TODO: this pitch inconsistency needs fixing...
}

void SetAnim(edict_t* self, const int anim)
{
	assert(classStatics[self->classID].resInfo != NULL);
	assert(classStatics[self->classID].resInfo->animations != NULL);

	self->monsterinfo.currentmove = classStatics[self->classID].resInfo->animations[anim];

	// Only reset the anim index if the new anim is different from your current anim.
	if (self->curAnimID != anim)
	{
		self->monsterinfo.currframeindex = 0;
		self->monsterinfo.nextframeindex = 0;
	}

	self->lastAnimID = self->curAnimID;
	self->curAnimID = anim;
}

// Returns true if it is time to think.
qboolean ThinkTime(const edict_t* self)
{
	// Need an epsilon value to account for floating point error.
	// The epsilon can be large because level.time goes up in increments of 0.1.
	if (self->think != NULL && self->nextthink > TIME_EPSILON)
		return (self->nextthink - level.time <= TIME_EPSILON);

	return false;
}

#define SF_ANIMATED	2 //mxd

// Spawns a client model animation.
// spawnflags & 2 is a designer flag whether to animate or not.
// If the model is supposed to animate, the hi bit of the type is set.
// If the model is static, then the default frame stored on the client is used.
// Valid scale ranges from 1/50th to 5.
void SpawnClientAnim(edict_t* self, byte type, const char* sound) //mxd. Defined in g_misc.c in original logic.
{
	if (self->spawnflags & SF_ANIMATED) // Animate it.
	{
		type |= 0x80;

		if (sound != NULL)
		{
			self->s.sound = (byte)gi.soundindex(sound);
			self->s.sound_data = (255 & ENT_VOL_MASK) | ATTN_STATIC;
		}
	}

	const int scale = (byte)(self->s.scale * 50.0f);
	assert((scale > 0) && (scale < 255));
	const byte b_skin = (byte)self->s.skinnum;

	self->PersistantCFX = gi.CreatePersistantEffect(&self->s, FX_ANIMATE, CEF_BROADCAST, self->s.origin, "bbbv", type, (byte)scale, b_skin, self->s.angles);
	self->s.effects |= EF_ALWAYS_ADD_EFFECTS;
}

// A check to see if ent should reflect.
qboolean EntReflecting(const edict_t* ent, const qboolean check_monster, const qboolean check_player) //mxd. Defined in g_misc.c in original logic.
{
	if (ent == NULL)
		return false;

	if (check_monster && (ent->svflags & SVF_MONSTER) && (ent->svflags & SVF_REFLECT))
		return true;

	if (check_player && ent->client != NULL)
	{
		const playerinfo_t* info = &ent->client->playerinfo; //mxd

		if (info->reflect_timer > level.time)
			return true;

		// Possibly, we might want to reflect this if the player has gold armor.
		if (info->pers.armortype == ARMOR_TYPE_GOLD && info->pers.armor_count > 0.0f && irand(0, 100) < 30)
			return true;
	}

	return false;
}

void SkyFly(edict_t* self) //mxd. Defined in g_misc.c in original logic. //TODO: replace with G_SetToFree()?
{
	G_SetToFree(self);
}