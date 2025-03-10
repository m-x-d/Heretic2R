//
// g_func.c
//
// Copyright 1998 Raven Software
//

#include "g_combat.h" //mxd
#include "g_debris.h" //mxd
#include "g_DefaultMessageHandler.h"
#include "m_stats.h"
#include "mg_guide.h" //mxd
#include "FX.h"
#include "Random.h"
#include "Vector.h"
#include "g_local.h"

#pragma region ========================== CID to classname ==========================

typedef enum MonsterSpawnerID_e
{
	MS_NOTHING = 0,
	MS_RAT,
	MS_PLAGUEELF,
	MS_SPREADER,
	MS_GORGON,
	MS_CHKROKTK,
	MS_TCHEKRIK_MALE,
	MS_TCHEKRIK_FEMALE,
	MS_TCHEKRIK_MOTHERS,
	MS_HIGH_PRIESTESS,
	MS_OGLE,
	MS_SERAPH_OVERLORD,
	MS_SERAPH_GUARD,
	MS_ASSASSIN,
	MS_MORCALAVIN,
	MS_DRANOR,
	MS_SIDHE_GUARD,
	MS_SIERNAN,
	MS_SSITHRA_SCOUT,
	MS_SSITHRA_VICTIM,
	MS_MSSITHRA,
	MS_HARPY,
	MS_FISH,
	MS_CHICKEN,
	MS_SSITHRA,
	MS_GKROKON,
	MS_RAT_GIANT,
	MS_PALACE_PLAGUE_GUARD,
	MS_INVISIBLE_PALACE_PLAGUE_GUARD,

	MS_MAX
} MonsterSpawnerID_t;

static char* monster_spawner_classnames[] =
{
	"monster_nothing",
	"monster_rat",				// MS_RAT
	"monster_plagueElf",		// MS_PLAGUEELF
	"monster_spreader",			// MS_SPREADER
	"monster_gorgon",			// MS_GORGON
	"monster_rat",				// MS_CHKROKTK
	"monster_tcheckrik_male",	// TCHEKRIK_MALE
	"monster_tcheckrik_female",	// TCHEKRIK_FEMALE
	"monster_plagueElf",		// TCHEKRIK_MOTHERS
	"monster_highPriestess",	// HIGH_PRIESTESS
	"monster_ogle",				// MS_OGLE
	"monster_seraph_overlord",	// MS_SERAPH_OVERLORD
	"monster_seraph_guard",		// MS_SERAPH_GUARD
	"monster_assassin",			// MS_ASSASSIN
	"monster_morcalavin",		// MS_MORCALAVIN
	"character_dranor",			// MS_DRANOR
	"monster_plagueElf",		// MS_SIDHE_GUARD
	"character_siernan1",		// MS_SIERNAN
	"character_ssithra_scout",	// MS_SSITHRA_SCOUT
	"character_ssithra_victim", // MS_SSITHRA_VICTIM
	"monster_mssithra",			// MS_MSSITHRA
	"monster_harpy",			// MS_HARPY
	"monster_fish",				// MS_FISH
	"monster_chicken",			// MS_CHICKEN
	"monster_ssithra",			// MS_SSITHRA
	"monster_gkrokon",			// MS_GKROKON
	"monster_rat_giant",		// MS_RAT_GIANT
	"monster_palace_plague_guard", //MS_PALACE_PLAGUE_GUARD,
	"monster_palace_plague_guard_invisible", //MS_INVISIBLE_PALACE_PLAGUE_GUARD,
};

static int cid_for_spawner_style[] =
{
	CID_NONE,
	CID_RAT,
	CID_PLAGUEELF,
	CID_SPREADER,
	CID_GORGON,
	CID_GKROKON,
	CID_TCHECKRIK,
	CID_TCHECKRIK,
	CID_MOTHER,
	CID_HIGHPRIESTESS,
	CID_OGLE,
	CID_SERAPH_OVERLORD,
	CID_SERAPH_GUARD,
	CID_ASSASSIN,
	CID_MORK,
	CID_DRANOR,
	CID_PLAGUEELF,
	CID_C_SIERNAN1,
	CID_SSITHRA_SCOUT,
	CID_SSITHRA_VICTIM,
	CID_MSSITHRA,
	CID_HARPY,
	CID_FISH,
	CID_CHICKEN,
	CID_SSITHRA,
	CID_GKROKON,
	CID_RAT,
	CID_PLAGUEELF,
	CID_PLAGUEELF,
};

#pragma endregion

// PLATS
// Movement options:

// linear
// smooth start, hard stop
// smooth start, smooth stop

// start
// end
// acceleration
// speed
// deceleration
// begin sound
// end sound
// target fired when reaching end
// wait at end

// Object characteristics that use move segments:

// PHYSICSTYPE_PUSH, or PHYSICSTYPE_STOP
// action when touched
// action when blocked
// action when used -- disabled?
// auto trigger spawning

#define STATE_TOP			0
#define STATE_BOTTOM		1
#define STATE_UP			2
#define STATE_DOWN			3

#define SF_DOOR_START_OPEN	1
#define SF_DOOR_REVERSE		2
#define SF_DOOR_CRUSHER		4
#define SF_DOOR_NOMONSTER	8
#define SF_DOOR_TOGGLE		32
#define SF_DOOR_X_AXIS		64
#define SF_DOOR_Y_AXIS		128
#define SF_DOOR_SWINGAWAY	8192

#pragma region ====================== Support routines for movement (changes in origin using velocity) ======================

static void MoveDone(edict_t* ent) //mxd. Named 'Move_Done' in original logic.
{
	VectorClear(ent->velocity);
	ent->think = NULL;

	if (ent->moveinfo.endfunc != NULL)
		ent->moveinfo.endfunc(ent);
}

static void MoveFinal(edict_t* ent) //mxd. Named 'Move_Final' in original logic.
{
	VectorScale(ent->moveinfo.dir, ent->moveinfo.remaining_distance / FRAMETIME, ent->velocity);

	ent->think = MoveDone;
	ent->nextthink = level.time + FRAMETIME;
}

static void MoveBegin(edict_t* ent) //mxd. Named 'Move_Begin' in original logic.
{
	if ((ent->moveinfo.speed * FRAMETIME) >= ent->moveinfo.remaining_distance)
	{
		MoveFinal(ent);
		return;
	}

	VectorScale(ent->moveinfo.dir, ent->moveinfo.speed, ent->velocity);

	const float frames = floorf(ent->moveinfo.remaining_distance / ent->moveinfo.speed / FRAMETIME);
	ent->moveinfo.remaining_distance -= frames * ent->moveinfo.speed * FRAMETIME;
	ent->nextthink = level.time + (frames * FRAMETIME);
	ent->think = MoveFinal;
}

static void AccelMoveThink(edict_t* ent); //TODO: move to g_funcs.h

static void MoveCalc(edict_t* ent, const vec3_t dest, void(*func)(edict_t*)) //mxd. Named 'Move_Calc' in original logic.
{
	VectorClear(ent->velocity);

	VectorSubtract(dest, ent->s.origin, ent->moveinfo.dir);
	ent->moveinfo.remaining_distance = VectorNormalize(ent->moveinfo.dir);
	ent->moveinfo.endfunc = func;

	if (ent->moveinfo.speed == ent->moveinfo.accel && ent->moveinfo.speed == ent->moveinfo.decel)
	{
		if (level.current_entity == ((ent->flags & FL_TEAMSLAVE) ? ent->teammaster : ent))
		{
			MoveBegin(ent);
		}
		else
		{
			ent->nextthink = level.time + FRAMETIME;
			ent->think = MoveBegin;
		}
	}
	else
	{
		// Accelerative.
		ent->moveinfo.current_speed = 0.0f;
		ent->nextthink = level.time + FRAMETIME;
		ent->think = AccelMoveThink;
	}
}

#pragma endregion

#pragma region ================== Support routines for angular movement of trains (changes in angle using avelocity) ==================

static void FuncTrainAngleMoveCalc(edict_t* self, const edict_t* ent, const vec3_t dest) //mxd. Named 'TrainAngleMove_Calc' in original logic.
{
	VectorClear(self->avelocity);

	if (ent == NULL || Vec3IsZero(ent->s.angles))
		return;

	// Put angles into wacky order (required).
	const vec3_t angles = { ent->s.angles[1], ent->s.angles[2], ent->s.angles[0] };

	// Length to travel.
	const float len = VectorSeparation(self->s.origin, dest);

	// Divide by speed to get time to reach dest.
	const float travel_time = (len / self->moveinfo.speed) + self->moveinfo.wait;

	if (travel_time < FRAMETIME)
	{
		VectorScale(angles, 1.0f / FRAMETIME, self->avelocity);
	}
	else
	{
		// Scale the destdelta vector by the time spent traveling to get velocity.
		// 0.97 is used because we want the train to change angles a little slow, that way it never overshoots the angle it should be at.
		// In train_next the final angle is set using en_angles.
		const float hold_time = 0.97f / travel_time;
		VectorScale(angles, hold_time, self->avelocity);
	}

	VectorAdd(self->s.angles, angles, self->moveinfo.end_angles);
}

static void AngleMoveDone(edict_t* ent) //mxd. Named 'AngleMove_Done' in original logic.
{
	VectorClear(ent->avelocity);
	ent->think = NULL;

	if (ent->moveinfo.endfunc != NULL)
		ent->moveinfo.endfunc(ent);
}

static void AngleMoveFinal(edict_t* ent) //mxd. Named 'AngleMove_Final' in original logic.
{
	vec3_t move;
	const vec3_t* src_angles = ((ent->moveinfo.state == STATE_UP) ? &ent->moveinfo.end_angles : &ent->moveinfo.start_angles); //mxd
	VectorSubtract(*src_angles, ent->s.angles, move);

	VectorScale(move, 1.0f / FRAMETIME, ent->avelocity);

	ent->nextthink = level.time + FRAMETIME;
	ent->think = AngleMoveDone;
}

static void AngleMoveBegin(edict_t* ent) //mxd. Named 'AngleMove_Begin' in original logic.
{
	// Set dest_delta to the vector needed to move.
	vec3_t dest_delta;
	const vec3_t* src_angles = ((ent->moveinfo.state == STATE_UP) ? &ent->moveinfo.end_angles : &ent->moveinfo.start_angles); //mxd
	VectorSubtract(*src_angles, ent->s.angles, dest_delta);

	// Calculate length of vector.
	const float len = VectorLength(dest_delta);

	// Divide by speed to get time to reach dest.
	const float travel_time = len / ent->moveinfo.speed;

	if (travel_time < FRAMETIME)
	{
		AngleMoveFinal(ent);
		return;
	}

	// Scale the dest_delta vector by the time spent traveling to get velocity.
	VectorScale(dest_delta, 1.0f / travel_time, ent->avelocity);

	// Set nextthink to trigger a think when dest is reached.
	const float frames = floorf(travel_time / FRAMETIME);
	ent->nextthink = level.time + frames * FRAMETIME;
	ent->think = AngleMoveFinal;
}

static void AngleMoveCalc(edict_t* ent, void(*func)(edict_t*)) //mxd. Named 'AngleMove_Calc' in original logic.
{
	VectorClear(ent->avelocity);
	ent->moveinfo.endfunc = func;

	if (level.current_entity == ((ent->flags & FL_TEAMSLAVE) ? ent->teammaster : ent))
	{
		AngleMoveBegin(ent);
	}
	else
	{
		ent->nextthink = level.time + FRAMETIME;
		ent->think = AngleMoveBegin;
	}
}

#pragma endregion

#pragma region ========================== AccelMoveThink ==========================

static float AccelerationDistance(const float target, const float rate) //mxd. #define in original logic.
{
	return target * ((target / rate) + 1.0f) / 2.0f;
}

static void FuncPlatCalcAcceleratedMove(moveinfo_t* info) //mxd. Named 'plat_CalcAcceleratedMove' in original logic.
{
	info->move_speed = info->speed;

	if (info->remaining_distance < info->accel)
	{
		info->current_speed = info->remaining_distance;
		return;
	}

	const float accel_dist = AccelerationDistance(info->speed, info->accel);
	float decel_dist = AccelerationDistance(info->speed, info->decel);

	if ((info->remaining_distance - accel_dist - decel_dist) < 0.0f)
	{
		const float f = (info->accel + info->decel) / (info->accel * info->decel);
		info->move_speed = (-2.0f + sqrtf(4.0f - (4.0f * f) * (-2.0f * info->remaining_distance))) / (2.0f * f);
		decel_dist = AccelerationDistance(info->move_speed, info->decel);
	}

	info->decel_distance = decel_dist;
}

static void FuncPlatAccelerate(moveinfo_t* info) //mxd. Named 'plat_Accelerate' in original logic.
{
	// Are we decelerating?
	if (info->remaining_distance <= info->decel_distance)
	{
		if (info->remaining_distance < info->decel_distance)
		{
			if (info->next_speed != 0.0f)
			{
				info->current_speed = info->next_speed;
				info->next_speed = 0.0f;
			}
			else if (info->current_speed > info->decel)
			{
				info->current_speed -= info->decel;
			}
		}

		return;
	}

	// Are we at full speed and need to start decelerating during this move?
	if (info->current_speed == info->move_speed)
	{
		if ((info->remaining_distance - info->current_speed) < info->decel_distance)
		{
			const float p1_distance = info->remaining_distance - info->decel_distance;
			const float p2_distance = info->move_speed * (1.0f - (p1_distance / info->move_speed));
			const float distance = p1_distance + p2_distance;
			info->current_speed = info->move_speed;
			info->next_speed = info->move_speed - info->decel * (p2_distance / distance);

			return;
		}
	}

	// Are we accelerating?
	if (info->current_speed < info->speed)
	{
		const float old_speed = info->current_speed;

		// Figure simple acceleration up to move_speed.
		info->current_speed += info->accel;
		info->current_speed = min(info->speed, info->current_speed);

		// Are we accelerating throughout this entire move?
		if (info->remaining_distance - info->current_speed >= info->decel_distance)
			return;

		// During this move we will accelerate from current_speed to move_speed and cross over the decel_distance;
		// Figure the average speed for the entire move.
		const float p1_distance = info->remaining_distance - info->decel_distance;
		const float p1_speed = (old_speed + info->move_speed) / 2.0f;
		const float p2_distance = info->move_speed * (1.0f - (p1_distance / p1_speed));
		const float distance = p1_distance + p2_distance;
		info->current_speed = (p1_speed * (p1_distance / distance)) + (info->move_speed * (p2_distance / distance));
		info->next_speed = info->move_speed - info->decel * (p2_distance / distance);
	}

	// We are at constant velocity (move_speed).
}

// The team has completed a frame of movement, so change the speed for the next frame.
static void AccelMoveThink(edict_t* ent) //mxd. Named 'Think_AccelMove' in original logic.
{
	ent->moveinfo.remaining_distance -= ent->moveinfo.current_speed;

	if (ent->moveinfo.current_speed == 0.0f) // Starting or blocked.
		FuncPlatCalcAcceleratedMove(&ent->moveinfo);

	FuncPlatAccelerate(&ent->moveinfo);

	// Will the entire move complete on next frame?
	if (ent->moveinfo.remaining_distance <= ent->moveinfo.current_speed)
	{
		MoveFinal(ent);
	}
	else
	{
		VectorScale(ent->moveinfo.dir, ent->moveinfo.current_speed * 10.0f, ent->velocity);
		ent->nextthink = level.time + FRAMETIME;
		ent->think = AccelMoveThink;
	}
}

#pragma endregion

#pragma region ========================== func_plat ==========================

#define SF_PLAT_LOW_TRIGGER	1 //mxd

static void FuncPlatPlayMoveStartSound(edict_t* ent) //mxd. Added to reduce code duplication.
{
	if (!(ent->flags & FL_TEAMSLAVE))
	{
		if (ent->moveinfo.sound_start > 0)
			gi.sound(ent, CHAN_NO_PHS_ADD + CHAN_VOICE, ent->moveinfo.sound_start, 1.0f, ATTN_IDLE, 0.0f);

		ent->s.sound = (byte)ent->moveinfo.sound_middle;
		ent->s.sound_data = (255 & ENT_VOL_MASK) | ATTN_IDLE;
	}
}

static void FuncPlatPlayMoveEndSound(edict_t* ent) //mxd. Added to reduce code duplication.
{
	if (!(ent->flags & FL_TEAMSLAVE))
	{
		if (ent->moveinfo.sound_end > 0)
			gi.sound(ent, CHAN_NO_PHS_ADD + CHAN_VOICE, ent->moveinfo.sound_end, 1.0f, ATTN_IDLE, 0.0f);

		ent->s.sound = 0;
	}
}

static void FuncPlatGoDown(edict_t* ent); //TODO: move to g_funcs.h

static void FuncPlatHitTop(edict_t* ent) //mxd. Named 'plat_hit_top' in original logic.
{
	FuncPlatPlayMoveEndSound(ent); //mxd

	ent->moveinfo.state = STATE_TOP;
	ent->think = FuncPlatGoDown;
	ent->nextthink = level.time + 3.0f;
}

static void FuncPlatHitBottom(edict_t* ent) //mxd. Named 'plat_hit_bottom' in original logic.
{
	FuncPlatPlayMoveEndSound(ent); //mxd
	ent->moveinfo.state = STATE_BOTTOM;
}

static void FuncPlatGoDown(edict_t* ent) //mxd. Named 'plat_go_down' in original logic.
{
	FuncPlatPlayMoveStartSound(ent); //mxd
	ent->moveinfo.state = STATE_DOWN;
	MoveCalc(ent, ent->moveinfo.end_origin, FuncPlatHitBottom);
}

static void FuncPlatGoUp(edict_t* ent) //mxd. Named 'plat_go_up' in original logic.
{
	FuncPlatPlayMoveStartSound(ent); //mxd
	ent->moveinfo.state = STATE_UP;
	MoveCalc(ent, ent->moveinfo.start_origin, FuncPlatHitTop);
}

static void FuncPlatBlocked(edict_t* self, edict_t* other) //mxd. Named 'plat_blocked' in original logic.
{
	//TODO: invalid logic? Checks for both presence and absence of SVF_MONSTER flag! Last check is '!(other->svflags & SVF_BOSS)' in FuncDoorBlocked().
	if ((other->svflags & SVF_MONSTER) && other->client == NULL && !(other->svflags & SVF_MONSTER))
	{
		// Give it a chance to go away on it's own terms (like gibs).
		T_Damage(other, self, self, vec3_origin, other->s.origin, vec3_origin, 3000, 1, DAMAGE_AVOID_ARMOR, MOD_CRUSH);

		// If it's still there, nuke it.
		if (other->health > 0)
			BecomeDebris(other);

		return;
	}

	if (self->spawnflags & SF_DOOR_CRUSHER)
	{
		T_Damage(other, self, self, vec3_origin, other->s.origin, vec3_origin, self->dmg * 10, 1, 0, MOD_CRUSH);
		return;
	}

	T_Damage(other, self, self, vec3_origin, other->s.origin, vec3_origin, self->dmg, 1, 0, MOD_CRUSH);

	if (self->moveinfo.state == STATE_UP)
		FuncPlatGoDown(self);
	else if (self->moveinfo.state == STATE_DOWN)
		FuncPlatGoUp(self);
}

static void FuncPlatUse(edict_t* ent, edict_t* other, edict_t* activator) //mxd. Named 'Use_Plat' in original logic.
{
	if (ent->think == NULL) // Already down otherwise.
		FuncPlatGoDown(ent);
}

static void FuncPlatCenterTouch(edict_t* ent, edict_t* other, cplane_t* plane, csurface_t* surf) //mxd. Named 'Touch_Plat_Center' in original logic.
{
	if (other->client == NULL || other->health <= 0)
		return;

	ent = ent->enemy; // Now point at the plat, not the trigger.

	if (ent->moveinfo.state == STATE_BOTTOM)
		FuncPlatGoUp(ent);
	else if (ent->moveinfo.state == STATE_TOP)
		ent->nextthink = level.time + 1.0f; // The player is still on the plat, so delay going down.
}

static void FuncPlatSpawnInsideTrigger(edict_t* ent) //mxd. Named 'plat_spawn_inside_trigger' in original logic.
{
	// Middle trigger.
	edict_t* trigger = G_Spawn();

	trigger->touch = FuncPlatCenterTouch;
	trigger->movetype = PHYSICSTYPE_NONE;
	trigger->solid = SOLID_TRIGGER;
	trigger->enemy = ent;

	vec3_t t_maxs;
	t_maxs[0] = ent->maxs[0] - 25.0f;
	t_maxs[1] = ent->maxs[1] - 25.0f;
	t_maxs[2] = ent->maxs[2] + 8.0f;

	vec3_t t_mins;
	t_mins[0] = ent->mins[0] + 25.0f;
	t_mins[1] = ent->mins[1] + 25.0f;
	t_mins[2] = t_maxs[2] - (ent->pos1[2] - ent->pos2[2] + (float)st.lip);

	if (ent->spawnflags & SF_PLAT_LOW_TRIGGER)
		t_maxs[2] = t_mins[2] + 8.0f;

	for (int i = 0; i < 2; i++)
	{
		if (t_maxs[i] - t_mins[i] <= 0.0f)
		{
			t_mins[i] = (ent->mins[i] + ent->maxs[i]) * 0.5f;
			t_maxs[i] = t_mins[i] + 1.0f;
		}
	}

	VectorCopy(t_mins, trigger->mins);
	VectorCopy(t_maxs, trigger->maxs);

	gi.linkentity(trigger);
}

static void FuncRotateDeactivate(edict_t* self, G_Message_t* msg) //mxd. Named 'FuncRotate_Deactivate' in original logic.
{
	VectorClear(self->velocity);
	VectorClear(self->avelocity);
}

static void FuncRotateActivate(edict_t* self, G_Message_t* msg) //mxd. Named 'FuncRotate_Activate' in original logic.
{
	self->use(self, NULL, NULL);
	gi.linkentity(self);
}

void FuncRotateStaticsInit(void)
{
	classStatics[CID_FUNC_ROTATE].msgReceivers[G_MSG_SUSPEND] = FuncRotateDeactivate;
	classStatics[CID_FUNC_ROTATE].msgReceivers[G_MSG_UNSUSPEND] = FuncRotateActivate;
}

void FuncDoorStaticsInit(void)
{
	classStatics[CID_FUNC_DOOR].msgReceivers[G_MSG_SUSPEND] = FuncRotateDeactivate;
	classStatics[CID_FUNC_DOOR].msgReceivers[G_MSG_UNSUSPEND] = FuncRotateActivate;
}

static void FuncDoorSetSounds(edict_t* ent); //TODO: move to g_funcs.h

// QUAKED func_plat (0 .5 .8) ? PLAT_LOW_TRIGGER
// Plats are always drawn in the extended position, so they will light correctly.
// If the plat is the target of another trigger or button, it will start out disabled in the extended position until it is triggered,
// when it will lower and become a normal plat.

// Spawnflags:
// PLAT_LOW_TRIGGER - When set, platform trigger height is 8.

// Variables:
// speed	- overrides default 200.
// accel	- overrides default 500.
// lip		- overrides default 8 pixel lip.
// height	- is set, that will determine the amount the plat moves, instead of being implicitly determined by the model's height.
// sounds:
//		0)	Silent.
//		1)	Generic door.
//		2)	Heavy stone door.
//		3)  For swing arm on palace level.
//		4)  For stone bridge in palace level.
//		5)  Small/medium wood door swinging.
//		6)  Large/huge wood door swinging.
//		7)  Medium sized stone/wood door sliding.
//		8)  Large stone/wood sliding door or portcullis.
//		9)  Average metal door swinging.
//		10) Fast sliding doors.
//		11) Hive, Metal, Multipaneled sliding.
//		12) Huge stone door swinging.
//		13) Medium/large elevator.
//		14) Crane (warehouse).
//		15) Hammer-like pump in oglemine1.
//		16) Sliding metal table in cloudlabs.
//		17) Lab table which rotates up to ceiling - cloublabs.
//		18) Piston sound.
//		19) Short, sharp metal clang.
//		20) Something going under water.
//		21) The bam sound.
void SP_func_plat(edict_t* ent)
{
	VectorClear(ent->s.angles);

	ent->solid = SOLID_BSP;
	ent->movetype = PHYSICSTYPE_PUSH;
	ent->clipmask = MASK_PLAYERSOLID;
	ent->blocked = FuncPlatBlocked;
	ent->use = FuncPlatUse;

	if (ent->speed == 0.0f)
		ent->speed = 20.0f;
	else
		ent->speed *= 0.1f;

	if (ent->accel == 0.0f)
		ent->accel = 5.0f;
	else
		ent->accel *= 0.1f;

	if (ent->decel == 0.0f)
		ent->decel = 5.0f;
	else
		ent->decel *= 0.1f;

	if (ent->dmg == 0)
		ent->dmg = 2;

	if (st.lip == 0)
		st.lip = 8;

	// pos1 is the top position, pos2 is the bottom
	VectorCopy(ent->s.origin, ent->pos1);
	VectorCopy(ent->s.origin, ent->pos2);

	if (st.height > 0)
		ent->pos2[2] -= (float)st.height;
	else
		ent->pos2[2] -= ent->maxs[2] - ent->mins[2] - (float)st.lip;

	if (ent->targetname != NULL)
	{
		ent->moveinfo.state = STATE_UP;
	}
	else
	{
		ent->moveinfo.state = STATE_BOTTOM;
		VectorCopy(ent->pos2, ent->s.origin);
	}

	ent->moveinfo.speed = ent->speed;
	ent->moveinfo.accel = ent->accel;
	ent->moveinfo.decel = ent->decel;
	ent->moveinfo.wait = ent->wait;

	VectorCopy(ent->pos1, ent->moveinfo.start_origin);
	VectorCopy(ent->s.angles, ent->moveinfo.start_angles);
	VectorCopy(ent->pos2, ent->moveinfo.end_origin);
	VectorCopy(ent->s.angles, ent->moveinfo.end_angles);

	VectorSubtract(ent->maxs, ent->mins, ent->s.bmodel_origin);
	Vec3ScaleAssign(0.5f, ent->s.bmodel_origin);
	VectorAdd(ent->mins, ent->s.bmodel_origin, ent->s.bmodel_origin);

	FuncDoorSetSounds(ent);

	gi.setmodel(ent, ent->model);
	gi.linkentity(ent);

	FuncPlatSpawnInsideTrigger(ent); // The "start moving" trigger.
}

#pragma endregion

#pragma region ========================== func_rotating ==========================

#define SF_START_ON			1 //mxd
#define SF_REVERSE			2 //mxd
#define SF_X_AXIS			4 //mxd
#define SF_Y_AXIS			8 //mxd
#define SF_TOUCH_PAIN		16 //mxd
#define SF_STOP				32 //mxd
#define SF_ANIMATED			64 //mxd
#define SF_ANIMATED_FAST	128 //mxd
#define SF_CRUSHER			256 //mxd

static void FuncRotatingSetSounds(edict_t* ent) //mxd. Named 'rotate_sounds' in original logic.
{
	switch (ent->sounds)
	{
		case 1: ent->moveinfo.sound_middle = gi.soundindex("doors/stoneloop.wav"); break;
		case 2: ent->moveinfo.sound_middle = gi.soundindex("objects/hugewheel.wav"); break;
		case 3: ent->moveinfo.sound_middle = gi.soundindex("objects/pizzawheel.wav"); break;
		case 4: ent->moveinfo.sound_middle = gi.soundindex("objects/spankers.wav"); break;

		default:
			ent->moveinfo.sound_start = 0;
			ent->moveinfo.sound_middle = 0;
			ent->moveinfo.sound_end = 0;
			break;
	}
}

static void FuncRotatingBlocked(edict_t* self, edict_t* other) //mxd. Named 'rotating_blocked' in original logic.
{
	T_Damage(other, self, self, vec3_origin, other->s.origin, vec3_origin, self->dmg, 1, 0, MOD_CRUSH);
}

static void FuncRotatingTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf) //mxd. Named 'rotating_touch' in original logic.
{
	if (Vec3NotZero(self->avelocity))
		FuncRotatingBlocked(self, other);
}

static void FuncRotatingUse(edict_t* self, edict_t* other, edict_t* activator) //mxd. Named 'rotating_use' in original logic.
{
	if (Vec3NotZero(self->avelocity))
	{
		self->s.sound = 0;
		VectorClear(self->avelocity);
		self->touch = NULL;
	}
	else
	{
		self->s.sound = (byte)self->moveinfo.sound_middle;
		self->s.sound_data = (255 & ENT_VOL_MASK) | ATTN_IDLE;
		VectorScale(self->movedir, self->speed, self->avelocity);

		if (self->spawnflags & SF_TOUCH_PAIN)
			self->touch = FuncRotatingTouch;
	}
}

// QUAKED func_rotating (0 .5 .8) ? START_ON REVERSE X_AXIS Y_AXIS TOUCH_PAIN STOP ANIMATED ANIMATED_FAST CRUSHER
// You need to have an origin brush as part of this entity. The center of that brush will be the point around which it is rotated.
// It will rotate around the Z axis by default. You can check either the X_AXIS or Y_AXIS box to change that.

// Spawnflags:
// REVERSE	- Will cause the it to rotate in the opposite direction.
// STOP		- Mean it will stop moving instead of pushing entities.

// Variables:
// speed	- Determines how fast it moves (default 100).
// dmg		- Damage to inflict when blocked (default 2).
// sounds:
//		0 - Silent.
//		1 - Generic rotate.
//		2 - Huge wheel ogles push in cloudlabs.
//		3 - Rock crusher which turns at end of conveyor on ogle2.
//		4 - 'Spanking' paddles on gauntlet.
void SP_func_rotating(edict_t* ent)
{
	ent->classID = CID_FUNC_ROTATE;
	ent->msgHandler = DefaultMsgHandler;

	ent->solid = SOLID_BSP;
	ent->movetype = ((ent->spawnflags & SF_STOP) ? PHYSICSTYPE_STOP : PHYSICSTYPE_PUSH);

	FuncRotatingSetSounds(ent);

	// Set the axis of rotation.
	VectorClear(ent->movedir);

	if (ent->spawnflags & SF_X_AXIS)
		ent->movedir[2] = 1.0f;
	else if (ent->spawnflags & SF_Y_AXIS)
		ent->movedir[0] = 1.0f;
	else // Z_AXIS
		ent->movedir[1] = 1.0f;

	// Check for reverse rotation.
	if (ent->spawnflags & SF_REVERSE)
		VectorNegate(ent->movedir, ent->movedir);

	if (ent->speed == 0.0f)
		ent->speed = 100.0f;

	if (ent->dmg == 0)
		ent->dmg = 2;

	ent->use = FuncRotatingUse;

	if (ent->dmg > 0)
		ent->blocked = FuncRotatingBlocked;

	if (ent->spawnflags & SF_START_ON)
		ent->use(ent, NULL, NULL);

	if (ent->spawnflags & SF_ANIMATED)
		ent->s.effects |= EF_ANIM_ALL;

	if (ent->spawnflags & SF_ANIMATED_FAST)
		ent->s.effects |= EF_ANIM_ALLFAST;

	//TODO: neither SF_CRUSHER nor SF_DOOR_CRUSHER flags seem to be used by func_rotating logic...
	if (ent->spawnflags & SF_CRUSHER) // Because of a mixup in flags.
		ent->spawnflags |= SF_DOOR_CRUSHER;

	VectorSubtract(ent->maxs, ent->mins, ent->s.bmodel_origin);
	Vec3ScaleAssign(0.5f, ent->s.bmodel_origin);
	VectorAdd(ent->mins, ent->s.bmodel_origin, ent->s.bmodel_origin);

	gi.setmodel(ent, ent->model);
	gi.linkentity(ent);
}

#pragma endregion

#pragma region ========================== func_button ==========================

#define SF_TOUCH	1 //mxd

static void FuncButtonMove(edict_t* self); //TODO: move to header.

static void FuncButtonOnDeathMessage(edict_t* self, G_Message_t* msg) //mxd. Named 'button_killed2' in original logic.
{
	self->activator = self->enemy;
	FuncButtonMove(self);
	self->health = self->max_health;
}

void ButtonStaticsInit(void) //TODO: rename to FuncButtonStaticsInit.
{
	classStatics[CID_BUTTON].msgReceivers[MSG_DEATH] = FuncButtonOnDeathMessage;
}

static void FuncButtonDone(edict_t* self) //mxd. Named 'button_done' in original logic.
{
	self->moveinfo.state = STATE_BOTTOM;
	self->s.frame = 0;
}

static void FuncButtonReturn(edict_t* self) //mxd. Named 'button_return' in original logic.
{
	self->moveinfo.state = STATE_DOWN;
	MoveCalc(self, self->moveinfo.start_origin, FuncButtonDone);
	self->s.frame = 0;

	if (self->health > 0)
		self->takedamage = DAMAGE_YES;
}

static void FuncButtonWait(edict_t* self) //mxd. Named 'button_wait' in original logic.
{
	self->moveinfo.state = STATE_TOP;
	G_UseTargets(self, self->activator);
	self->s.frame = 1;

	if (self->moveinfo.wait >= 0)
	{
		self->nextthink = level.time + self->moveinfo.wait;
		self->think = FuncButtonReturn;
	}
}

static void FuncButtonMove(edict_t* self) //mxd. Named 'button_fire' in original logic.
{
	if (self->moveinfo.state == STATE_UP || self->moveinfo.state == STATE_TOP)
		return;

	if (self->moveinfo.sound_start > 0 && !(self->flags & FL_TEAMSLAVE))
		gi.sound(self, CHAN_NO_PHS_ADD + CHAN_VOICE, self->moveinfo.sound_start, 1.0f, ATTN_IDLE, 0.0f);

	self->moveinfo.state = STATE_UP;
	MoveCalc(self, self->moveinfo.end_origin, FuncButtonWait);
}

static void FuncButtonUse(edict_t* self, edict_t* other, edict_t* activator) //mxd. Named 'button_use' in original logic.
{
	self->activator = activator;
	FuncButtonMove(self);
}

static void FuncButtonTouch(edict_t* self, trace_t* trace) //mxd. Named 'button_touch' in original logic.
{
	edict_t* other = trace->ent;

	if (other->client != NULL && other->health > 0)
	{
		self->activator = other;
		FuncButtonMove(self);
	}
}

static void FuncButtonSetSounds(edict_t* self) //mxd. Named 'button_sounds' in original logic.
{
	switch (self->sounds)
	{
		case 1: self->moveinfo.sound_start = gi.soundindex("doors/basicbutton.wav"); break;
		case 2: self->moveinfo.sound_start = gi.soundindex("doors/clankybutton.wav"); break;
		case 3: self->moveinfo.sound_start = gi.soundindex("doors/steambutton.wav"); break;
		default: break;
	}
}

// QUAKED func_button (0 .5 .8) ? TOUCH
// When a button is touched, it moves some distance in the direction of it's angle, triggers all of it's targets,
// waits "wait" time, then returns to it's original position where it can be triggered again.

// Spawnflags:
// TOUCH - Player can touch button to set it off.

// Variables:
// angle	- Determines the opening direction.
// target	- All entities with a matching targetname will be used.
// speed	- Override the default 40 speed.
// wait		- Override the default 1 second wait (-1 = never return).
// lip		- Override the default 4 pixel lip remaining at end of move.
// health	- If set, the button must be killed instead of touched.
// sounds:
//		0) Silent.
//		1) Basic Button.
//		2) Clanky Button.
//		3) Steam Button.
void SP_func_button(edict_t* ent)
{
	G_SetMovedir(ent->s.angles, ent->movedir);
	ent->movetype = PHYSICSTYPE_STOP;
	ent->solid = SOLID_BSP;
	ent->takedamage = DAMAGE_NO;

	gi.setmodel(ent, ent->model);
	gi.linkentity(ent);

	FuncButtonSetSounds(ent);

	if (ent->speed == 0.0f)
		ent->speed = 40.0f;

	if (ent->accel == 0.0f)
		ent->accel = ent->speed;

	if (ent->decel == 0.0f)
		ent->decel = ent->speed;

	if (ent->wait == 0.0f)
		ent->wait = 3.0f;

	if (st.lip == 0)
		st.lip = 4;

	VectorCopy(ent->s.origin, ent->pos1);

	vec3_t abs_movedir;
	VectorAbs(ent->movedir, abs_movedir);

	const float dist = DotProduct(abs_movedir, ent->size) - (float)st.lip;
	VectorMA(ent->pos1, dist, ent->movedir, ent->pos2);

	ent->use = FuncButtonUse;

	if (ent->health > 0)
	{
		ent->max_health = ent->health;
		ent->takedamage = DAMAGE_YES;
	}

	if (ent->targetname == NULL || (ent->spawnflags & SF_TOUCH))
		ent->isBlocking = FuncButtonTouch;

	ent->moveinfo.state = STATE_BOTTOM;
	ent->moveinfo.speed = ent->speed;
	ent->moveinfo.accel = ent->accel;
	ent->moveinfo.decel = ent->decel;
	ent->moveinfo.wait = ent->wait;

	VectorCopy(ent->pos1, ent->moveinfo.start_origin);
	VectorCopy(ent->s.angles, ent->moveinfo.start_angles);
	VectorCopy(ent->pos2, ent->moveinfo.end_origin);
	VectorCopy(ent->s.angles, ent->moveinfo.end_angles);

	ent->msgHandler = DefaultMsgHandler;
}

#pragma endregion

#pragma region ========================== func_door, func_door_rotating, func_water ==========================

#define DOOR_MOVE_LOOP	(-2.0f) //mxd

static void FuncDoorUseAreaportals(const edict_t* self, const qboolean open) //mxd. Named 'door_use_areaportals' in original logic.
{
	if (self->target == NULL)
		return;

	edict_t* target = NULL;
	while ((target = G_Find(target, FOFS(targetname), self->target)) != NULL)
		if (Q_stricmp(target->classname, "func_areaportal") == 0)
			gi.SetAreaPortalState(target->style, open);
}

static void FuncDoorGoDown(edict_t* self); //TODO: move to header.

static void FuncDoorHitTop(edict_t* self) //mxd. Named 'door_hit_top' in original logic.
{
	FuncPlatPlayMoveEndSound(self); //mxd
	self->moveinfo.state = STATE_TOP;

	if (self->spawnflags & SF_DOOR_TOGGLE)
		return;

	if (self->moveinfo.wait >= 0.0f)
	{
		self->think = FuncDoorGoDown;
		self->nextthink = level.time + self->moveinfo.wait;
	}
	else if (self->moveinfo.wait == DOOR_MOVE_LOOP)
	{
		self->think = FuncDoorGoDown;
		self->nextthink = level.time + FRAMETIME; // Next frame is soon enough to fire this off.
	}
}

static void FuncDoorGoUp(edict_t* self, edict_t* activator); //TODO: move to header.

static void FuncDoorHitBottom(edict_t* self) //mxd. Named 'door_hit_bottom' in original logic.
{
	FuncPlatPlayMoveEndSound(self); //mxd
	self->moveinfo.state = STATE_BOTTOM;

	if (self->moveinfo.wait == DOOR_MOVE_LOOP) // Endless cycle.
		FuncDoorGoUp(self, NULL);
	else
		FuncDoorUseAreaportals(self, false);
}

static void FuncDoorGoDown(edict_t* self) //mxd. Named 'door_go_down' in original logic.
{
	FuncPlatPlayMoveStartSound(self); //mxd

	if (self->max_health > 0)
	{
		self->takedamage = DAMAGE_YES;
		self->health = self->max_health;
	}

	self->moveinfo.state = STATE_DOWN;

	if (strcmp(self->classname, "func_door") == 0)
		MoveCalc(self, self->moveinfo.start_origin, FuncDoorHitBottom);
	else if (strcmp(self->classname, "func_door_rotating") == 0)
		AngleMoveCalc(self, FuncDoorHitBottom);
}

static void FuncDoorGoUp(edict_t* self, edict_t* activator) //mxd. Named 'door_go_up' in original logic.
{
	if (self->moveinfo.state == STATE_UP)
		return; // Already going up.

	if (self->moveinfo.state == STATE_TOP)
	{
		// Reset top wait time.
		if (self->moveinfo.wait >= 0)
			self->nextthink = level.time + self->moveinfo.wait;
		else if (self->moveinfo.wait == DOOR_MOVE_LOOP)
			self->nextthink = level.time;

		return;
	}

	FuncPlatPlayMoveStartSound(self); //mxd
	self->moveinfo.state = STATE_UP;

	if (strcmp(self->classname, "func_door") == 0)
		MoveCalc(self, self->moveinfo.end_origin, FuncDoorHitTop);
	else if (strcmp(self->classname, "func_door_rotating") == 0)
		AngleMoveCalc(self, FuncDoorHitTop);

	G_UseTargets(self, activator);
	FuncDoorUseAreaportals(self, true);
}

// Checks to see if a rotating door will get in activator's way when it opens.
static qboolean FuncDoorSmartSideCheck(const edict_t* self, const edict_t* activator) //mxd. Named 'smart_door_side_check' in original logic.
{
	if (activator == NULL)
		return false;

	// Make a plane containing the origins of the origin brush, the door, and a point which is the sum of movedir
	// (slightly rearranged (x, z, y)) and one of the others.

	vec3_t door_points[3];
	VectorCopy(self->s.origin, door_points[0]); // Origin brush origin.
	VectorAdd(self->s.origin, self->mins, door_points[1]);
	VectorMA(door_points[1], 0.5f, self->size, door_points[1]); // Door center.

	door_points[2][0] = self->s.origin[0] + self->movedir[2];
	door_points[2][1] = self->s.origin[1] + self->movedir[0];
	door_points[2][2] = self->s.origin[2] + self->movedir[1]; // Third point.

	vec3_t in_plane[2];
	VectorSubtract(door_points[1], door_points[0], in_plane[0]);
	VectorSubtract(door_points[2], door_points[0], in_plane[1]);

	vec3_t normal;
	CrossProduct(in_plane[0], in_plane[1], normal);

	vec3_t to_player;
	VectorSubtract(activator->s.origin, door_points[1], to_player);

	return DotProduct(normal, to_player) < 0.0f;
}

static void FuncDoorUse(edict_t* self, edict_t* other, edict_t* activator) //mxd. Named 'smart_door_side_check' in original logic.
{
	if (Vec3IsZero(self->avelocity) && strcmp(self->classname, "func_door_rotating") == 0 && (self->spawnflags & SF_DOOR_SWINGAWAY))
	{
		if (FuncDoorSmartSideCheck(self, activator))
		{
			VectorNegate(self->movedir, self->movedir);
			VectorNegate(self->moveinfo.end_angles, self->moveinfo.end_angles);
		}
	}

	if (self->flags & FL_TEAMSLAVE)
		return;

	// Trigger all paired doors.
	if ((self->spawnflags & SF_DOOR_TOGGLE) && (self->moveinfo.state == STATE_UP || self->moveinfo.state == STATE_TOP))
	{
		for (edict_t* ent = self; ent != NULL; ent = ent->teamchain)
		{
			ent->message = NULL;
			ent->isBlocking = NULL;
			FuncDoorGoDown(ent);
		}
	}
	else
	{
		for (edict_t* ent = self; ent != NULL; ent = ent->teamchain)
		{
			ent->message = NULL;
			ent->isBlocking = NULL;
			FuncDoorGoUp(ent, activator);
		}
	}
}

static void FuncDoorTriggerTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf) //mxd. Named 'Touch_DoorTrigger' in original logic.
{
	if (other->health <= 0 || (!(other->svflags & SVF_MONSTER) && other->client == NULL))
		return;

	if ((self->owner->spawnflags & SF_DOOR_NOMONSTER) && (other->svflags & SVF_MONSTER))
		return;

	if (level.time < self->touch_debounce_time)
		return;

	self->touch_debounce_time = level.time + 1.0f;
	FuncDoorUse(self->owner, other, other);
}

static void FuncDoorCalcMoveSpeedThink(edict_t* self) //mxd. Named 'Think_CalcMoveSpeed' in original logic.
{
	if (self->flags & FL_TEAMSLAVE)
	{
		self->think = NULL;
		return; // Only the team master does this.
	}

	// Find the smallest distance any member of the team will be moving.
	float min_dist = Q_fabs(self->moveinfo.distance);

	for (const edict_t* ent = self->teamchain; ent != NULL; ent = ent->teamchain)
	{
		const float dist = Q_fabs(ent->moveinfo.distance);
		min_dist = min(dist, min_dist);
	}

	const float time = min_dist / self->moveinfo.speed;

	// Adjust speeds so they will all complete at the same time.
	for (edict_t* ent = self; ent != NULL; ent = ent->teamchain)
	{
		const float new_speed = Q_fabs(ent->moveinfo.distance) / time;
		const float ratio = new_speed / ent->moveinfo.speed;

		if (ent->moveinfo.accel == ent->moveinfo.speed)
			ent->moveinfo.accel = new_speed;
		else
			ent->moveinfo.accel *= ratio;

		if (ent->moveinfo.decel == ent->moveinfo.speed)
			ent->moveinfo.decel = new_speed;
		else
			ent->moveinfo.decel *= ratio;

		ent->moveinfo.speed = new_speed;
	}

	gi.linkentity(self);
	self->think = NULL;
}

static void FuncDoorSpawnDoorTriggerThink(edict_t* self) //mxd. Named 'Think_SpawnDoorTrigger' in original logic.
{
	self->think = NULL;

	if (self->flags & FL_TEAMSLAVE)
		return; // Only the team leader spawns a trigger.

	vec3_t mins;
	vec3_t maxs;
	VectorCopy(self->absmin, mins);
	VectorCopy(self->absmax, maxs);

	for (const edict_t* ent = self->teamchain; ent != NULL; ent = ent->teamchain)
	{
		AddPointToBounds(ent->absmin, mins, maxs);
		AddPointToBounds(ent->absmax, mins, maxs);
	}

	// Expand on XY axis.
	for (int i = 0; i < 2; i++)
	{
		mins[i] -= 60.0f;
		maxs[i] += 60.0f;
	}

	edict_t* trigger = G_Spawn();

	VectorCopy(mins, trigger->mins);
	VectorCopy(maxs, trigger->maxs);
	trigger->owner = self;
	trigger->solid = SOLID_TRIGGER;
	trigger->movetype = PHYSICSTYPE_NONE;
	trigger->touch = FuncDoorTriggerTouch;

	if (self->spawnflags & SF_DOOR_START_OPEN)
		FuncDoorUseAreaportals(self, true);

	FuncDoorCalcMoveSpeedThink(self);

	gi.linkentity(trigger);
}

static void FuncDoorBlocked(edict_t* self, edict_t* other) //mxd. Named 'door_blocked' in original logic.
{
	if ((other->svflags & SVF_MONSTER) && other->client == NULL && !(other->svflags & SVF_BOSS))
	{
		// Give it a chance to go away on it's own terms (like gibs).
		T_Damage(other, self, self, vec3_origin, other->s.origin, vec3_origin, 3000, 1, DAMAGE_AVOID_ARMOR, MOD_CRUSH);

		// If it's still there, nuke it.
		if (other->health > 0)
			BecomeDebris(other);

		return;
	}

	if (self->spawnflags & SF_DOOR_CRUSHER)
	{
		T_Damage(other, self, self, vec3_origin, other->s.origin, vec3_origin, self->dmg * 10, 1, 0, MOD_CRUSH);
		return;
	}

	T_Damage(other, self, self, vec3_origin, other->s.origin, vec3_origin, self->dmg, 1, 0, MOD_CRUSH);

	// If a door has a negative wait, it would never come back if blocked, (unless -2) so let it just squash the object to death real fast.
	if (self->moveinfo.wait >= 0.0f || self->moveinfo.wait == DOOR_MOVE_LOOP)
	{
		if (self->moveinfo.state == STATE_DOWN)
		{
			for (edict_t* ent = self->teammaster; ent != NULL; ent = ent->teamchain)
				FuncDoorGoUp(ent, ent->activator);
		}
		else
		{
			for (edict_t* ent = self->teammaster; ent != NULL; ent = ent->teamchain)
				FuncDoorGoDown(ent);
		}
	}
}

static int FuncDoorKilled(edict_t* self, edict_t* inflictor, edict_t* attacker, int damage, vec3_t point) //mxd. Named 'door_killed' in original logic.
{
	for (edict_t* ent = self->teammaster; ent != NULL; ent = ent->teamchain)
	{
		ent->health = ent->max_health;
		ent->takedamage = DAMAGE_NO;
	}

	FuncDoorUse(self->teammaster, attacker, attacker);
	return 0;
}

static void FuncDoorTouch(edict_t* self, trace_t* trace) //mxd. Named 'door_killed' in original logic.
{
	const edict_t* other = trace->ent;

	if (other->client != NULL && level.time >= self->touch_debounce_time)
	{
		self->touch_debounce_time = level.time + 5.0f;
		gi.levelmsg_centerprintf(other, (short)Q_atoi(self->message));
	}
}

static void FuncDoorSetSounds(edict_t* ent) //mxd. Named 'door_sounds' in original logic.
{
	switch ((DoorSoundID_t)ent->sounds)
	{
		case DS_GENERIC:
			ent->moveinfo.sound_start = gi.soundindex("doors/gendoorstart.wav");
			ent->moveinfo.sound_middle = 0;
			ent->moveinfo.sound_end = gi.soundindex("doors/gendoorstop.wav");
			break;

		case DS_HEAVYSTONE:
			ent->moveinfo.sound_start = gi.soundindex("doors/stonestart.wav");
			ent->moveinfo.sound_middle = gi.soundindex("doors/stoneloop.wav");
			ent->moveinfo.sound_end = gi.soundindex("doors/stoneend.wav");
			break;

		case DS_SWINGARM:
			ent->moveinfo.sound_start = gi.soundindex("doors/bigcreak.wav");
			break;

		case DS_SWINGBRIDGE:
			ent->moveinfo.sound_start = gi.soundindex("doors/stoneloop.wav");
			ent->moveinfo.sound_middle = 0;
			ent->moveinfo.sound_end = gi.soundindex("doors/stoneend.wav");
			break;

		case DS_MEDIUMWOOD:
			ent->moveinfo.sound_start = gi.soundindex("doors/kchunk2.wav");
			ent->moveinfo.sound_middle = gi.soundindex("doors/creak4.wav");
			ent->moveinfo.sound_end = gi.soundindex("doors/doorclose1.wav");
			break;

		case DS_HUGEWOOD:
			ent->moveinfo.sound_start = gi.soundindex("doors/kchunk1.wav");
			ent->moveinfo.sound_middle = gi.soundindex("doors/creak2.wav");
			ent->moveinfo.sound_end = gi.soundindex("doors/doorshut1.wav");
			break;

		case DS_MEDIUMSTONE:
			ent->moveinfo.sound_start = gi.soundindex("doors/kchunk7.wav");
			ent->moveinfo.sound_middle = gi.soundindex("doors/stndoor.wav");
			ent->moveinfo.sound_end = gi.soundindex("doors/thud7.wav");
			break;

		case DS_LARGESTONE:
			ent->moveinfo.sound_start = gi.soundindex("doors/kchunk6.wav");
			ent->moveinfo.sound_middle = gi.soundindex("doors/stoneloop.wav");
			ent->moveinfo.sound_end = gi.soundindex("doors/thud3.wav");
			break;

		case DS_MEDIUMMETAL:
			ent->moveinfo.sound_start = gi.soundindex("doors/kchunk3.wav");
			ent->moveinfo.sound_middle = gi.soundindex("doors/metal1.wav");
			ent->moveinfo.sound_end = gi.soundindex("doors/thud2.wav");
			break;

		case DS_FASTSLIDING:
			ent->moveinfo.sound_start = gi.soundindex("doors/fastdoor.wav");
			ent->moveinfo.sound_middle = 0;
			ent->moveinfo.sound_end = 0;
			break;

		case DS_METALSLIDING:
			ent->moveinfo.sound_start = gi.soundindex("doors/kchunk5.wav");
			ent->moveinfo.sound_middle = 0;
			ent->moveinfo.sound_end = gi.soundindex("doors/thud2.wav");
			break;

		case DS_HUGESTONE:
			ent->moveinfo.sound_start = gi.soundindex("doors/kchunk5.wav");
			ent->moveinfo.sound_middle = gi.soundindex("objects/creak2a.wav");
			ent->moveinfo.sound_end = gi.soundindex("doors/thud4.wav");
			break;

		case DS_HUGEELEVATOR:
			ent->moveinfo.sound_start = gi.soundindex("doors/elevatorstart.wav");
			ent->moveinfo.sound_middle = gi.soundindex("doors/elevatormove.wav");
			ent->moveinfo.sound_end = gi.soundindex("doors/elevatorstop.wav");
			break;

		case DS_CRANEWAREHOUSE:
			ent->moveinfo.sound_start = gi.soundindex("doors/kchunk6.wav");
			ent->moveinfo.sound_middle = gi.soundindex("objects/winch2.wav");
			ent->moveinfo.sound_end = gi.soundindex("objects/cratedown.wav");
			break;

		case DS_HAMMERPUMP:
			ent->moveinfo.sound_start = gi.soundindex("objects/oilpump.wav");
			ent->moveinfo.sound_middle = 0;
			ent->moveinfo.sound_end = 0;
			break;

		case DS_METALTABLE:
			ent->moveinfo.sound_start = gi.soundindex("objects/slabslide.wav");
			ent->moveinfo.sound_middle = 0;
			ent->moveinfo.sound_end = 0;
			break;

		case DS_LABTABLE:
			ent->moveinfo.sound_start = gi.soundindex("objects/globebottomstart.wav");
			ent->moveinfo.sound_end = gi.soundindex("objects/globebottomend.wav");
			break;

		case DS_PISTON:
			ent->moveinfo.sound_start = gi.soundindex("objects/piston.wav");
			break;

		case DS_CLANG:
			ent->moveinfo.sound_start = gi.soundindex("objects/klang.wav");
			break;

		case DS_UNDERWATER:
			ent->moveinfo.sound_start = gi.soundindex("objects/submerge.wav");
			break;

		case DS_BAM:
			ent->moveinfo.sound_start = gi.soundindex("objects/bam1.wav");
			break;

		case DS_NONE:
		default:
			ent->moveinfo.sound_start = 0;
			ent->moveinfo.sound_middle = 0;
			ent->moveinfo.sound_end = 0;
			break;
	}
}

// QUAKED func_door (0 .5 .8) ? START_OPEN x CRUSHER NOMONSTER ANIMATED TOGGLE ANIMATED_FAST

// Spawnflags:
// START_OPEN	- The door to moves to its destination when spawned, and operate in reverse.
//				  It is used to temporarily or permanently close off an area when triggered (not useful for touch or takedamage doors).
// NOMONSTER	- Monsters will not trigger this door.
// TOGGLE		- Wait in both the start and end states for a trigger event.

// Variables:
// message		- Is printed when the door is touched if it is a trigger door and it hasn't been fired yet.
// angle		- Determines the opening direction.
// targetname	- If set, no touch field will be spawned and a remote button or trigger field activates the door.
// health		- If set, door must be shot open.
// height		- If set, tells how far up door opens.
// speed		- Movement speed (default 100).
// wait			- Wait before returning (default 3, -1 = never return,-2 = never stop cycle).
// lip			- Lip remaining at end of move (default 8).
// dmg			- Damage to inflict when blocked (default 2).
// sounds:
//		0)	Silent.
//		1)	Generic door.
//		2)	Heavy stone door.
//		3)  For swing arm on palace level.
//		4)  For stone bridge in palace level.
//		5)  Small/medium wood door swinging.
//		6)  Large/huge wood door swinging.
//		7)  Medium sized stone/wood door sliding.
//		8)  Large stone/wood sliding door or portcullis.
//		9)  Average metal door swinging.
//		10) Fast sliding doors.
//		11) Hive, Metal, Multipaneled sliding.
//		12) Huge stone door swinging.
//		13) Medium/large elevator.
//		14) Crane (warehouse).
//		15) Hammer-like pump in oglemine1.
//		16) Sliding metal table in cloudlabs.
//		17) Lab table which rotates up to ceiling - cloublabs.
//		18) Piston sound.
//		19) Short, sharp metal clang.
//		20) Something going under water.
//		21) The bam sound.
void SP_func_door(edict_t* self)
{
	FuncDoorSetSounds(self);
	G_SetMovedir(self->s.angles, self->movedir);

	self->classID = CID_FUNC_DOOR;
	self->msgHandler = DefaultMsgHandler;

	self->movetype = PHYSICSTYPE_PUSH;
	self->solid = SOLID_BSP;
	self->blocked = FuncDoorBlocked;
	self->use = FuncDoorUse;

	gi.setmodel(self, self->model);
	gi.linkentity(self);

	if (self->speed == 0.0f)
		self->speed = 100.0f;

	if (self->accel == 0.0f)
		self->accel = self->speed;

	if (self->decel == 0.0f)
		self->decel = self->speed;

	if (self->wait == 0.0f)
		self->wait = 3.0f;

	if (st.lip == 0)
		st.lip = 8;

	if (self->dmg == 0)
		self->dmg = 2;

	// Calculate second position.
	VectorCopy(self->s.origin, self->pos1);

	vec3_t abs_movedir;
	VectorAbs(self->movedir, abs_movedir);

	if (st.height == 0)
		self->moveinfo.distance = DotProduct(abs_movedir, self->size) - (float)st.lip;
	else
		self->moveinfo.distance = abs_movedir[0] * self->size[0] + abs_movedir[1] * self->size[1] + abs_movedir[2] * (float)st.height;

	VectorMA(self->pos1, self->moveinfo.distance, self->movedir, self->pos2);

	// If it starts open, switch the positions.
	if (self->spawnflags & SF_DOOR_START_OPEN)
	{
		VectorCopy(self->pos2, self->s.origin);
		VectorCopy(self->pos1, self->pos2);
		VectorCopy(self->s.origin, self->pos1);
	}

	self->moveinfo.state = STATE_BOTTOM;

	if (self->health > 0)
	{
		self->takedamage = DAMAGE_YES;
		self->die = FuncDoorKilled;
		self->max_health = self->health;
	}
	else if (self->targetname != NULL && self->message != NULL)
	{
		gi.soundindex("misc/talk.wav");
		self->isBlocking = FuncDoorTouch;
	}

	self->moveinfo.speed = self->speed;
	self->moveinfo.accel = self->accel;
	self->moveinfo.decel = self->decel;
	self->moveinfo.wait = self->wait;
	VectorCopy(self->pos1, self->moveinfo.start_origin);
	VectorCopy(self->s.angles, self->moveinfo.start_angles);
	VectorCopy(self->pos2, self->moveinfo.end_origin);
	VectorCopy(self->s.angles, self->moveinfo.end_angles);

	VectorSubtract(self->maxs, self->mins, self->s.bmodel_origin);
	Vec3ScaleAssign(0.5f, self->s.bmodel_origin);
	VectorAdd(self->mins, self->s.bmodel_origin, self->s.bmodel_origin);

	if (self->spawnflags & 16)
		self->s.effects |= EF_ANIM_ALL;

	if (self->spawnflags & 64)
		self->s.effects |= EF_ANIM_ALLFAST;

	// To simplify logic elsewhere, make non-teamed doors into a team of one.
	if (self->team == NULL)
		self->teammaster = self;

	self->nextthink = level.time + FRAMETIME;

	if (self->health > 0 || self->targetname != NULL)
		self->think = FuncDoorCalcMoveSpeedThink;
	else
		self->think = FuncDoorSpawnDoorTriggerThink;
}

// QUAKED func_door_rotating (0 .5 .8) ? START_OPEN REVERSE CRUSHER NOMONSTER ANIMATED TOGGLE X_AXIS Y_AXIS SWINGAWAY
// You need to have an origin brush as part of this entity. The center of that brush will be the point around which it is rotated.
// It will rotate around the Z axis by default. You can check either the X_AXIS or Y_AXIS box to change that.

// Spawnflags:
// START_OPEN	- The door to moves to its destination when spawned, and operate in reverse.
//				  It is used to temporarily or permanently close off an area when triggered (not useful for touch or takedamage doors).
// REVERSE		- Will cause the door to rotate in the opposite direction.
// NOMONSTER	- Monsters will not trigger this door.
// TOGGLE		- Wait in both the start and end states for a trigger event.
// SWINGAWAY	- Door will always swing away from the activator.

// Variables:
// distance		- How many degrees the door will be rotated.
// message		- Is printed when the door is touched if it is a trigger door and it hasn't been fired yet.
// angle		- Determines the opening direction.
// targetname	- If set, no touch field will be spawned and a remote button or trigger field activates the door.
// health		- If set, door must be shot open.
// speed		- Movement speed (default 100).
// wait			- Wait before returning (default 3, -1 = never return,-2 = never stop cycle).
// dmg			- Damage to inflict when blocked (default 2).
// sounds:
//		0)	Silent.
//		1)	Generic door.
//		2)	Heavy stone door.
//		3)  For swing arm on palace level.
//		4)  For stone bridge in palace level.
//		5)  Small/medium wood door swinging.
//		6)  Large/huge wood door swinging.
//		7)  Medium sized stone/wood door sliding.
//		8)  Large stone/wood sliding door or portcullis.
//		9)  Average metal door swinging.
//		10) Fast sliding doors.
//		11) Hive, Metal, Multipaneled sliding.
//		12) Huge stone door swinging.
//		13) Medium/large elevator.
//		14) Crane (warehouse).
//		15) Hammer-like pump in oglemine1.
//		16) Sliding metal table in cloudlabs.
//		17) Lab table which rotates up to ceiling - cloublabs.
//		18) Piston sound.
//		19) Short, sharp metal clang.
//		20) Something going under water.
//		21) The bam sound.
void SP_func_door_rotating(edict_t* ent)
{
	VectorClear(ent->s.angles);

	// Set the axis of rotation.
	VectorClear(ent->movedir);

	if (ent->spawnflags & SF_DOOR_X_AXIS)
		ent->movedir[2] = 1.0f;
	else if (ent->spawnflags & SF_DOOR_Y_AXIS)
		ent->movedir[0] = 1.0f;
	else // Z_AXIS
		ent->movedir[1] = 1.0f;

	// Check for reverse rotation.
	if (ent->spawnflags & SF_DOOR_REVERSE)
		VectorNegate(ent->movedir, ent->movedir);

	if (st.distance == 0)
	{
		gi.dprintf("%s at %s with no distance set\n", ent->classname, vtos(ent->s.origin));
		st.distance = 90;
	}

	gi.setmodel(ent, ent->model);
	gi.linkentity(ent);

	VectorCopy(ent->s.angles, ent->pos1);
	VectorMA(ent->s.angles, (float)st.distance, ent->movedir, ent->pos2);
	ent->moveinfo.distance = (float)st.distance;

	ent->movetype = PHYSICSTYPE_PUSH;
	ent->solid = SOLID_BSP;
	ent->blocked = FuncDoorBlocked;
	ent->use = FuncDoorUse;

	if (ent->speed == 0.0f)
		ent->speed = 100;

	if (ent->accel == 0.0f)
		ent->accel = ent->speed;

	if (ent->decel == 0.0f)
		ent->decel = ent->speed;

	if (ent->wait == 0.0f)
		ent->wait = 3.0f;

	if (ent->dmg == 0)
		ent->dmg = 2;

	FuncDoorSetSounds(ent);

	// If it starts open, switch the positions.
	if (ent->spawnflags & SF_DOOR_START_OPEN)
	{
		VectorCopy(ent->pos2, ent->s.angles);
		VectorCopy(ent->pos1, ent->pos2);
		VectorCopy(ent->s.angles, ent->pos1);
		VectorNegate(ent->movedir, ent->movedir);
	}

	if (ent->health > 0)
	{
		ent->takedamage = DAMAGE_YES;
		ent->die = FuncDoorKilled;
		ent->max_health = ent->health;
	}

	if (ent->targetname != NULL && ent->message != NULL)
	{
		gi.soundindex("misc/talk.wav");
		ent->isBlocking = FuncDoorTouch;
	}

	ent->moveinfo.state = STATE_BOTTOM;
	ent->moveinfo.speed = ent->speed;
	ent->moveinfo.accel = ent->accel;
	ent->moveinfo.decel = ent->decel;
	ent->moveinfo.wait = ent->wait;
	VectorCopy(ent->s.origin, ent->moveinfo.start_origin);
	VectorCopy(ent->pos1, ent->moveinfo.start_angles);
	VectorCopy(ent->s.origin, ent->moveinfo.end_origin);
	VectorCopy(ent->pos2, ent->moveinfo.end_angles);

	VectorSubtract(ent->maxs, ent->mins, ent->s.bmodel_origin);
	Vec3ScaleAssign(0.5f, ent->s.bmodel_origin);
	VectorAdd(ent->mins, ent->s.bmodel_origin, ent->s.bmodel_origin);

	if (ent->spawnflags & 16)
		ent->s.effects |= EF_ANIM_ALL;

	ent->nextthink = level.time + FRAMETIME;

	if (ent->health || ent->targetname)
		ent->think = FuncDoorCalcMoveSpeedThink;
	else
		ent->think = FuncDoorSpawnDoorTriggerThink;
}

// QUAKED func_water (0 .5 .8) ? START_OPEN
// func_water is a moveable water brush. It must be targeted to operate. Use a non-water texture at your own risk.

// Spawnflags:
// START_OPEN - Causes the water to move to its destination when spawned and operate in reverse.

// Variables:
// angle	- Determines the opening direction (up or down only)
// speed	- Movement speed (default 25).
// wait		- Wait before returning (default -1, -1 = TOGGLE).
// lip		- Lip remaining at end of move (default 0).
// sounds: (yes, these need to be changed)
//		0)	No sound.
//		1)	Water.
//		2)	Lava.
void SP_func_water(edict_t* self)
{
	G_SetMovedir(self->s.angles, self->movedir);

	self->movetype = PHYSICSTYPE_PUSH;
	self->solid = SOLID_BSP;
	gi.setmodel(self, self->model);
	gi.linkentity(self);

	// Calculate second position.
	VectorCopy(self->s.origin, self->pos1);

	vec3_t abs_movedir;
	VectorAbs(self->movedir, abs_movedir);

	self->moveinfo.distance = DotProduct(abs_movedir, self->size) - (float)st.lip;
	VectorMA(self->pos1, self->moveinfo.distance, self->movedir, self->pos2);

	// If it starts open, switch the positions.
	if (self->spawnflags & SF_DOOR_START_OPEN)
	{
		VectorCopy(self->pos2, self->s.origin);
		VectorCopy(self->pos1, self->pos2);
		VectorCopy(self->s.origin, self->pos1);
	}

	VectorCopy(self->pos1, self->moveinfo.start_origin);
	VectorCopy(self->s.angles, self->moveinfo.start_angles);
	VectorCopy(self->pos2, self->moveinfo.end_origin);
	VectorCopy(self->s.angles, self->moveinfo.end_angles);

	VectorSubtract(self->maxs, self->mins, self->s.bmodel_origin);
	Vec3ScaleAssign(0.5f, self->s.bmodel_origin);
	VectorAdd(self->mins, self->s.bmodel_origin, self->s.bmodel_origin);

	self->moveinfo.state = STATE_BOTTOM;

	if (self->speed == 0.0f)
		self->speed = 25.0f;

	self->moveinfo.accel = self->speed;
	self->moveinfo.decel = self->speed;
	self->moveinfo.speed = self->speed;

	if (self->wait == 0.0f)
		self->wait = -1.0f;

	self->moveinfo.wait = self->wait;

	self->use = FuncDoorUse;

	if (self->wait == -1.0f)
		self->spawnflags |= SF_DOOR_TOGGLE;

	self->classname = "func_door";
}

#pragma endregion

#pragma region ========================== func_train ==========================

#define SF_TRAIN_START_ON		1
#define SF_TRAIN_TOGGLE			2
#define SF_TRAIN_BLOCK_STOPS	4
#define SF_TRAIN_HAS_ORIGIN		8 //mxd
#define SF_TRAIN_NO_CLIP		16 //mxd

static void FuncTrainNext(edict_t* self); //TODO: move to header.

static void FuncTrainAnim(edict_t* self) //mxd. Named 'train_anim' in original logic.
{
	if (self->s.frame == 0 && self->moveinfo.sound_middle > 0) // Start sound if there is one.
		gi.sound(self, CHAN_VOICE, self->moveinfo.sound_middle, 1.0f, ATTN_NORM, 0.0f);

	if (self->s.frame + 1 < self->count)
	{
		self->s.frame++;
		self->nextthink = level.time + FRAMETIME;
		self->think = FuncTrainAnim;
	}
	else
	{
		FuncTrainNext(self);
	}
}

static void FuncTrainAnimBackwards(edict_t* self) //mxd. Named 'train_animbackwards' in original logic.
{
	if (self->s.frame + 1 == self->count && self->moveinfo.sound_middle > 0) // Start sound if there is one.
		gi.sound(self, CHAN_VOICE, self->moveinfo.sound_middle, 1.0f, ATTN_NORM, 0.0f);

	if (self->s.frame > 0)
	{
		self->s.frame--;
		self->nextthink = level.time + FRAMETIME;
		self->think = FuncTrainAnimBackwards;
	}
	else
	{
		FuncTrainNext(self);
	}
}

static void FuncTrainBlocked(edict_t* self, edict_t* other) //mxd. Named 'train_blocked' in original logic.
{
	if ((other->svflags & SVF_MONSTER) && other->client == NULL && !(other->svflags & SVF_BOSS))
	{
		// Give it a chance to go away on it's own terms (like gibs).
		T_Damage(other, self, self, vec3_origin, other->s.origin, vec3_origin, 3000, 1, DAMAGE_AVOID_ARMOR, MOD_CRUSH);

		// If it's still there, nuke it.
		if (other->health > 0)
			BecomeDebris(other);
	}
	else if (self->dmg > 0 && level.time >= self->touch_debounce_time)
	{
		self->touch_debounce_time = level.time + 0.5f;
		T_Damage(other, self, self, vec3_origin, other->s.origin, vec3_origin, self->dmg, 1, 0, MOD_CRUSH);
	}
}

static void FuncTrainWait(edict_t* self) //mxd. Named 'train_wait' in original logic.
{
	if (self->target_ent->moveinfo.sound_middle > 0)
		gi.sound(self->target_ent, CHAN_VOICE, self->target_ent->moveinfo.sound_middle, 1.0f, ATTN_NORM, 0.0f);

	if (self->target_ent->pathtarget != NULL)
	{
		edict_t* ent = self->target_ent;
		char* save_target = ent->target;
		ent->target = ent->pathtarget;

		G_UseTargets(ent, self->activator);
		ent->target = save_target;

		// Make sure we didn't get killed by a killtarget.
		if (!self->inuse)
			return;
	}

	if (self->moveinfo.wait != 0.0f)
	{
		if (self->moveinfo.wait > 0.0f)
		{
			self->nextthink = level.time + self->moveinfo.wait;
			self->think = FuncTrainNext;
		}
		else if (self->moveinfo.wait == -3.0f)
		{
			BecomeDebris(self);
			return;
		}
		else if (self->moveinfo.wait == -4.0f)	// Make model animate.
		{
			if (self->s.frame + 1 < self->count)
				FuncTrainAnim(self);
			else
				FuncTrainAnimBackwards(self);
		}
		else if (self->spawnflags & SF_TRAIN_TOGGLE) // && wait < 0
		{
			FuncTrainNext(self);
			self->spawnflags &= ~SF_TRAIN_START_ON;
			VectorClear(self->velocity);
			self->nextthink = 0.0f;
		}

		FuncPlatPlayMoveEndSound(self); //mxd
	}
	else
	{
		FuncTrainNext(self);
	}
}

static void FuncTrainNext(edict_t* self) //mxd. Named 'train_next' in original logic.
{
	for (int i = 0; i < 2; i++)
	{
		if (self->target == NULL)
			return;

		edict_t* ent = G_PickTarget(self->target);

		if (ent == NULL)
		{
			gi.dprintf("train_next: bad target %s\n", self->target);
			return;
		}

		self->target = ent->target;

		// Check for SF_TELEPORT path_corner spawnflag.
		if (ent->spawnflags & 1)
		{
			if (i > 0)
			{
				gi.dprintf("connected teleport path_corners, see %s at %s\n", ent->classname, vtos(ent->s.origin));
				return;
			}

			VectorSubtract(ent->s.origin, self->mins, self->s.origin);
			VectorCopy(self->s.origin, self->s.old_origin);
			gi.linkentity(self);

			continue;
		}

		self->moveinfo.wait = ent->wait;
		self->target_ent = ent;

		FuncPlatPlayMoveStartSound(ent); //mxd

		vec3_t dest;
		if (self->spawnflags & SF_TRAIN_HAS_ORIGIN)
			VectorCopy(ent->s.origin, dest);
		else
			VectorSubtract(ent->s.origin, self->mins, dest);

		self->moveinfo.state = STATE_TOP;

		VectorCopy(self->s.origin, self->moveinfo.start_origin);
		VectorCopy(dest, self->moveinfo.end_origin);

		if (ent->speed > 0.0f)
		{
			self->moveinfo.speed = self->speed = ent->speed;
			self->moveinfo.accel = self->moveinfo.decel = self->moveinfo.speed;
		}

		MoveCalc(self, dest, FuncTrainWait);

		self->spawnflags |= SF_TRAIN_START_ON;

		VectorCopy(self->moveinfo.end_angles, self->s.angles); // Snap the train to the last ending angle.
		FuncTrainAngleMoveCalc(self, ent, dest); // Recalculate new angles.

		return;
	}
}

void FuncTrainResume(edict_t* self) //mxd. Named 'train_next' in original logic. //TODO: add to header.
{
	const edict_t* ent = self->target_ent;

	vec3_t dest;
	VectorSubtract(ent->s.origin, self->mins, dest);

	self->moveinfo.state = STATE_TOP;
	VectorCopy(self->s.origin, self->moveinfo.start_origin);
	VectorCopy(dest, self->moveinfo.end_origin);

	MoveCalc(self, dest, FuncTrainWait);
	self->spawnflags |= SF_TRAIN_START_ON;
}

static void FuncTrainFind(edict_t* self) //mxd. Named 'func_train_find' in original logic.
{
	if (self->target == NULL)
	{
		gi.dprintf("train_find: no target\n");
		self->think = NULL;

		return;
	}

	const edict_t* ent = G_PickTarget(self->target);

	if (ent == NULL)
	{
		gi.dprintf("train_find: target %s not found\n", self->target);
		self->think = NULL;

		return;
	}

	self->target = ent->target;

	if (Vec3NotZero(self->s.origin))
		VectorCopy(ent->s.origin, self->s.origin);
	else
		VectorSubtract(ent->s.origin, self->mins, self->s.origin);

	gi.linkentity(self);

	// If not triggered, start immediately.
	if (self->targetname == NULL)
		self->spawnflags |= SF_TRAIN_START_ON;

	if (self->spawnflags & SF_TRAIN_START_ON)
	{
		self->nextthink = level.time + FRAMETIME;
		self->think = FuncTrainNext;
		self->activator = self;
	}
	else
	{
		self->think = NULL;
	}
}

static void FuncTrainUse(edict_t* self, edict_t* other, edict_t* activator) //mxd. Named 'train_use' in original logic.
{
	self->activator = activator;

	if (Vec3NotZero(self->velocity))
		return;

	if (self->spawnflags & SF_TRAIN_START_ON)
	{
		if (!(self->spawnflags & SF_TRAIN_TOGGLE))
			return;

		self->spawnflags &= ~SF_TRAIN_START_ON;
		VectorClear(self->velocity);
		self->nextthink = 0.0f;
	}
	else if (self->target_ent != NULL)
	{
		FuncTrainResume(self);
	}
	else
	{
		FuncTrainNext(self);
	}
}

// QUAKED func_train (0 .5 .8) ? START_ON TOGGLE BLOCK_STOPS HASORIGIN NO_CLIP PUSHPULL
// Trains are moving platforms that players can ride. The targets origin specifies the min point of the train at each corner.
// The train spawns at the first target it is pointing at. If the train is the target of a button or trigger, it will not begin moving until activated.
// This means if it has a targetname it won't move unless triggered.

// Spawnflags:
// HASORIGIN	- Makes train move from an origin brush rather than the lower left point of the train.
// NO_CLIP		- Train will not block anything.

// Variables:
// speed	- default 100.
// dmg		- default 2.
// noise	- Looping file to play when the train is in motion.
//			- objects/piston.wav for large steam pistons in ogle2 and cloudlabs.
//			- objects/winch2.wav for wooden ore hauler going across river.
// rotate	- Speed train should rotate at.
// wait		- -1: Stop and don't move again until triggered.
//			  -3: Stop and explode.
//			  -4: Go through animations (only if a model)
// file		- Specifies the train is a model. This is the exact directory of the model (example: models/objects/broom/tris.fm).
// count	- Number of frames in animation (only if a model)
// materialtype:
//			0 = MAT_WOOD
//			1 = MAT_GREYSTONE (default)
//			2 = MAT_CLOTH
//			3 = MAT_METAL
//			9 = MAT_BROWNSTONE
//			10 = MAT_NONE - just makes smoke.
void SP_func_train(edict_t* self)
{
	self->movetype = PHYSICSTYPE_PUSH;
	self->solid = ((self->spawnflags & SF_TRAIN_NO_CLIP) ? SOLID_NOT : SOLID_BBOX);

	if (self->spawnflags & SF_TRAIN_BLOCK_STOPS)
		self->dmg = 0;
	else if (self->dmg == 0)
		self->dmg = 100;

	if (st.file != NULL)
	{
		self->s.modelindex = (byte)gi.modelindex(st.file);
		VectorCopy(self->s.angles, self->moveinfo.end_angles);
	}
	else
	{
		VectorClear(self->s.angles);
		gi.setmodel(self, self->model);
	}

	if (st.noise != NULL)
		self->moveinfo.sound_middle = gi.soundindex(st.noise);

	if (self->speed == 0.0f)
		self->speed = 100.0f;

	if (self->materialtype == 0) //TODO: MAT_STONE (0) can't be set...
		self->materialtype = MAT_GREYSTONE;

	self->moveinfo.speed = self->speed;
	self->moveinfo.accel = self->speed;
	self->moveinfo.decel = self->speed;

	self->blocked = FuncTrainBlocked;
	self->use = FuncTrainUse;

	VectorClear(self->movedir);

	if (st.rotate != 0)
		VectorScale(self->movedir, (float)st.rotate, self->avelocity);
	else
		VectorClear(self->avelocity);

	vec3_t space;
	VectorSubtract(self->maxs, self->mins, space);
	const float space_cube = space[0] * space[1] * space[2];
	self->mass = (int)(space_cube / 64.0f);

	VectorSubtract(self->maxs, self->mins, self->s.bmodel_origin);
	Vec3ScaleAssign(0.5f, self->s.bmodel_origin);
	VectorAdd(self->mins, self->s.bmodel_origin, self->s.bmodel_origin);

	gi.linkentity(self);

	if (self->target != NULL)
	{
		// Start trains on the second frame, to make sure their targets have had a chance to spawn.
		self->nextthink = level.time + FRAMETIME;
		self->think = FuncTrainFind;
	}
	else
	{
		gi.dprintf("func_train without a target at %s\n", vtos(self->absmin));
	}
}

#pragma endregion

#pragma region ========================== func_timer ==========================

#define SF_START_ON	1 //mxd

static void FuncTimerThink(edict_t* self) //mxd. Named 'func_timer_think' in original logic.
{
	G_UseTargets(self, self->activator);
	self->nextthink = level.time + self->wait + flrand(-self->random, self->random);
}

static void FuncTimerUse(edict_t* self, edict_t* other, edict_t* activator) //mxd. Named 'func_timer_use' in original logic.
{
	self->activator = activator;

	// If on, turn it off.
	if (self->nextthink > 0.0f)
		self->nextthink = 0.0f;
	else if (self->delay > 0.0f) // Turn it on.
		self->nextthink = level.time + self->delay;
	else
		FuncTimerThink(self);
}

// QUAKED func_timer (0.3 0.1 0.6) (-8 -8 -8) (8 8 8) START_ON
// These can used but not touched.

// Variables:
// wait			- Base time between triggering all targets (default 1).
// random		- Wait variance (default 0). So, the basic time between firing is a random time between (wait - random) and (wait + random).
// delay		- Delay before first firing when turned on (default  0).
// pausetime	- Additional delay used only the very first time and only if spawned with START_ON.
void SP_func_timer(edict_t* self)
{
	if (self->wait == 0.0f)
		self->wait = 1.0f;

	self->use = FuncTimerUse;
	self->think = FuncTimerThink;

	if (self->random >= self->wait)
	{
		self->random = self->wait - FRAMETIME;
		gi.dprintf("func_timer at %s has random >= wait\n", vtos(self->s.origin));
	}

	if (self->spawnflags & SF_START_ON)
	{
		self->nextthink = level.time + 1.0f + st.pausetime + self->delay + self->wait + flrand(-self->random, self->random);
		self->activator = self;
	}

	self->svflags = SVF_NOCLIENT;
}

#pragma endregion

#pragma region ========================== func_door_secret ==========================

#define SF_SECRET_ALWAYS_SHOOT	1
#define SF_SECRET_1ST_LEFT		2
#define SF_SECRET_1ST_DOWN		4

static void FuncDoorSecretMove1(edict_t* self);
static void FuncDoorSecretMove2(edict_t* self);
static void FuncDoorSecretMove3(edict_t* self);
static void FuncDoorSecretMove4(edict_t* self);
static void FuncDoorSecretMove5(edict_t* self);
static void FuncDoorSecretMove6(edict_t* self);
static void FuncDoorSecretDone(edict_t* self);

static void FuncDoorSecretUse(edict_t* self, edict_t* other, edict_t* activator) //mxd. Named 'door_secret_use' in original logic.
{
	// Make sure we're not already moving.
	if (!VectorCompare(self->s.origin, vec3_origin))
		return;

	if (self->moveinfo.sound_start > 0)
		gi.sound(self, CHAN_NO_PHS_ADD + CHAN_VOICE, self->moveinfo.sound_start, 1.0f, ATTN_IDLE, 0.0f);

	MoveCalc(self, self->pos1, FuncDoorSecretMove1);
	FuncDoorUseAreaportals(self, true);
}

static void FuncDoorSecretMove1(edict_t* self) //mxd. Named 'door_secret_move1' in original logic.
{
	self->nextthink = level.time + 1.0f;
	self->think = FuncDoorSecretMove2;
}

static void FuncDoorSecretMove2(edict_t* self) //mxd. Named 'door_secret_move2' in original logic.
{
	if (self->moveinfo.sound_middle > 0)
		gi.sound(self, CHAN_NO_PHS_ADD + CHAN_VOICE, self->moveinfo.sound_middle, 1.0f, ATTN_IDLE, 0.0f);

	MoveCalc(self, self->pos2, FuncDoorSecretMove3);
}

void FuncDoorSecretMove3 (edict_t *self)
{
	if (self->moveinfo.sound_end)
		gi.sound (self, CHAN_NO_PHS_ADD+CHAN_VOICE, self->moveinfo.sound_end, 1, ATTN_IDLE, 0);

	if (self->wait == -1)
		return;
	self->nextthink = level.time + self->wait;
	self->think = FuncDoorSecretMove4;
}

void FuncDoorSecretMove4 (edict_t *self)
{
	if (self->moveinfo.sound_middle)
		gi.sound (self, CHAN_NO_PHS_ADD+CHAN_VOICE, self->moveinfo.sound_middle, 1, ATTN_IDLE, 0);

	MoveCalc (self, self->pos1, FuncDoorSecretMove5);
}

void FuncDoorSecretMove5 (edict_t *self)
{
	if (self->moveinfo.sound_end)
		gi.sound (self, CHAN_NO_PHS_ADD+CHAN_VOICE, self->moveinfo.sound_end, 1, ATTN_IDLE, 0);

	self->nextthink = level.time + 1.0;
	self->think = FuncDoorSecretMove6;
}

void FuncDoorSecretMove6 (edict_t *self)
{
	if (self->moveinfo.sound_start)
		gi.sound (self, CHAN_NO_PHS_ADD+CHAN_VOICE, self->moveinfo.sound_start, 1, ATTN_IDLE, 0);

	MoveCalc (self, vec3_origin, FuncDoorSecretDone);
}

void FuncDoorSecretDone (edict_t *self)
{
	if (!(self->targetname) || (self->spawnflags & SF_SECRET_ALWAYS_SHOOT))
	{
		self->health = 0;
		self->takedamage = DAMAGE_YES;
	}
	FuncDoorUseAreaportals (self, false);
}

void door_secret_blocked  (edict_t *self, edict_t *other)
{
	if ((other->svflags & SVF_MONSTER) && (!other->client) && !(other->svflags & SVF_BOSS))
	{
		// give it a chance to go away on it's own terms (like gibs)
		T_Damage (other, self, self, vec3_origin, other->s.origin, vec3_origin, 3000, 1, DAMAGE_AVOID_ARMOR,MOD_CRUSH);
		// if it's still there, nuke it
		if(other->health > 0)
			BecomeDebris(other);
		return;
	}

	if (level.time < self->touch_debounce_time)
		return;
	self->touch_debounce_time = level.time + 0.5;

	T_Damage (other, self, self, vec3_origin, other->s.origin, vec3_origin, self->dmg, 1, 0,MOD_CRUSH);
}

int door_secret_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	self->takedamage = DAMAGE_NO;
	FuncDoorSecretUse (self, attacker, attacker);
	return(0);

}

void SP_func_door_secret (edict_t *ent)
{
	vec3_t	forward, right, up;
	float	side;
	float	width;
	float	length;

	FuncDoorSetSounds(ent);

//	ent->moveinfo.sound_start = gi.soundindex  ("doors/stonestart.wav");
//	ent->moveinfo.sound_middle = gi.soundindex  ("doors/stoneloop.wav");
//	ent->moveinfo.sound_end = gi.soundindex  ("doors/stoneend.wav");

	ent->movetype = PHYSICSTYPE_PUSH;
	ent->solid = SOLID_BSP;
	gi.setmodel (ent, ent->model);
	gi.linkentity (ent);

	ent->blocked = door_secret_blocked;
	ent->use = FuncDoorSecretUse;

	if (!(ent->targetname) || (ent->spawnflags & SF_SECRET_ALWAYS_SHOOT))
	{
		ent->health = 0;
		ent->takedamage = DAMAGE_YES;
		ent->die = door_secret_die;
	}

	if (!ent->dmg)
		ent->dmg = 2;

	if (!ent->wait)
		ent->wait = 5;

	ent->moveinfo.accel =
	ent->moveinfo.decel =
	ent->moveinfo.speed = 50;

	// calculate positions
	AngleVectors (ent->s.angles, forward, right, up);
	VectorClear (ent->s.angles);
	side = 1.0 - (ent->spawnflags & SF_SECRET_1ST_LEFT);
	if (ent->spawnflags & SF_SECRET_1ST_DOWN)
		width = Q_fabs(DotProduct(up, ent->size));
	else
		width = Q_fabs(DotProduct(right, ent->size));
	length = Q_fabs(DotProduct(forward, ent->size));
	if (ent->spawnflags & SF_SECRET_1ST_DOWN)
		VectorMA (ent->s.origin, -1 * width, up, ent->pos1);
	else
		VectorMA (ent->s.origin, side * width, right, ent->pos1);

	if (st.lip)
		length = length - st.lip;

	VectorMA (ent->pos1, length, forward, ent->pos2);
	if (ent->health)
	{
		ent->takedamage = DAMAGE_YES;
		ent->die = FuncDoorKilled;
		ent->max_health = ent->health;
	}
	else if (ent->targetname && ent->message)
	{
		gi.soundindex ("misc/talk.wav");
		ent->isBlocking = FuncDoorTouch;
	}
	
	ent->classname = "func_door";

}

#pragma endregion

void monsterspawner_go (edict_t *self)
{
	edict_t	*monster;
	vec3_t angle,holdorigin;
	vec3_t forward;
	trace_t trace;
	edict_t	*victim = NULL;
	buoy_t	*start_buoy, *end_buoy;
	int		end_buoy_index;
	int		num_attempts = 0;
	byte	o_mintel;
	vec3_t	buoydist;

	if (self->count<=0)
	{
		self->think = NULL;
		return;
	}

	VectorCopy(self->s.origin,holdorigin);
	holdorigin[2] -= 8;

	gi.trace(self->s.origin,self->mins, self->maxs, holdorigin, self, MASK_MONSTERSOLID,&trace);
	if (trace.fraction != 1)
		return;

	monster = G_Spawn();
	monster->classname = ED_NewString(monster_spawner_classnames[self->style]);

//copy my designer-modified fields to the monster to overrride defaults
	monster->classID = cid_for_spawner_style[self->style];
	if(self->mintel)
		monster->mintel = self->mintel;
	else
		monster->mintel = MaxBuoysForClass[monster->classID];
	monster->melee_range = self->melee_range;
	monster->missile_range = self->missile_range;
	monster->min_missile_range = self->min_missile_range;
	monster->bypass_missile_chance = self->bypass_missile_chance;
	monster->jump_chance = self->jump_chance;
	monster->wakeup_distance = self->wakeup_distance;
	
	monster->s.scale = self->s.scale;

	VectorScale(STDMinsForClass[monster->classID], monster->s.scale, monster->mins);
	VectorScale(STDMaxsForClass[monster->classID], monster->s.scale, monster->maxs);

	if (self->maxrange)
	{
		VectorClear(angle);
		angle[0] = flrand(0,360);
		AngleVectors(angle, forward, NULL, NULL);
		VectorMA(self->s.origin, self->maxrange, forward, monster->s.origin);
	}
	else if(self->spawnflags & 2)//randombuoy
	{
		//STEP 0: who are we after?
		if(self->enemy)
		{
			if(self->enemy->client)
			{
				victim = self->enemy;
			}
		}

		if(!victim)
			victim = level.sight_client;

		if(!victim)//no players
		{
			G_FreeEdict(monster);
			return;
		}
		
pickbuoy:
		num_attempts++;//avoid infinite loops
		if(num_attempts>100)
		{
			G_FreeEdict(monster);
			return;//can't find any buoys close enough
		}

		//step1: pick a random buoy
		start_buoy = &level.buoy_list[irand(0, level.active_buoys - 1)];

		//step2: make sure the buoy is within a certain range of the player (500)

		VectorSubtract(start_buoy->origin, victim->s.origin, buoydist);

		if(VectorLengthSquared(buoydist) >  250000)//more than 500 away
			goto pickbuoy;

		//step3: make sure the buoy is not visible to the player (unless assassin)
		if(monster->classID != CID_ASSASSIN)
		{
			if(visible_pos(victim, start_buoy->origin))
				goto pickbuoy;
		}
		//step4: if the player_buoy is defined, pick it, if not, find player's buoy
		if(level.player_buoy[victim->s.number] > NULL_BUOY)//could use player_last_buoy, but may not be reliable, and don't want to spend the time checking
			end_buoy = &level.buoy_list[level.player_buoy[victim->s.number]];
		else
		{
			end_buoy_index = MG_SetFirstBuoy(victim);
			if(end_buoy_index == NULL_BUOY)
			{
				G_FreeEdict(monster);
				return;//can't find a buoy for player
			}
			
			end_buoy = &level.buoy_list[end_buoy_index];
		}

		//step5: make sure the buoy is within 1/2 the mintel (no more than 10) buoys of the player's buoy
		monster->lastbuoy = NULL_BUOY;
		
		if(monster->mintel > 7)
			o_mintel = 7;
		else if(monster->mintel < 3)
			o_mintel = 3;
		else			
			o_mintel = monster->mintel;

		monster->mintel = ceil(monster->mintel * 0.5);
		if(!FindNextBuoy(monster, start_buoy->id, end_buoy->id))
		{
			monster->mintel = o_mintel;
			goto pickbuoy;//can't make connection within 1/2 mintel steps
		}
		monster->mintel = o_mintel;
			
		//step6: make sure nothing blocking is standing there
		monster->clipmask = MASK_MONSTERSOLID;
		if(!MG_MonsterAttemptTeleport(monster, start_buoy->origin, true))//ignorLOS since we checked above and can't see smonster at this point yet
			goto pickbuoy;//can't teleport there, there's something obstructing that spot- try another

		if(BUOY_DEBUG)
			gi.dprintf("%s monsterspawn-teleported to buoy %s\n", monster->classname, start_buoy->targetname);
		//step7: ok, put them there, let's continue
	}
	else
		VectorCopy(self->s.origin, monster->s.origin);

	VectorCopy(self->s.angles, monster->s.angles);
	ED_CallSpawn(monster);

	--self->count;
	if (self->count > 0)
		monster->owner = self;

	if(self->enemy && !(self->spawnflags & 1))
	{//was activated
		if(self->enemy->client)
		{//monster_start_go will check their enemy and do a FoundTarget
			monster->enemy = self->enemy;
		}
		self->enemy = NULL;
	}

	if ((self->count > 0) && !(self->spawnflags & 1))//this ! was inside quotes, ! is before & in order of operations
	{
		self->think = monsterspawner_go;
		self->nextthink = level.time + self->wait;
	}
	else
	{
		self->think = NULL;
	}
}

void monsterspawner_use(edict_t *self, edict_t *other, edict_t *activator)
{
	self->enemy = activator;
	monsterspawner_go(self);
}

/*QUAKED func_monsterspawner (0 .5 .8) (-8 -8 -8) (8 8 8) ONDEATH RANDOMBUOY PEACEFUL
Triggerable monster spawner
-----SPAWNFLAGS--------------
ONDEATH - the next monster will not spawn until the current one is dead  
RANDOMBUOY - the monster will be teleported to a random buoy that the player cannot see
PEACEFUL - Monsters are NOT spawned agry at ent that triggered spawner (monstrers will spawn angry only if the spawner was triggered by a player)
-----------KEYS--------------
count - number of monsters to spawn before stopping (default 1)
distance - radius which monster can spawn from monsterspawner
style - type of monster to spawn
 0 - nothing will spawn
 1 - RAT,
 2 - PLAGUEELF,
 3 - SPREADER,
 4 - GORGON,
 5 - CHKROKTK,
 6 - TCHEKRIK_MALE,
 7 - TCHEKRIK_FEMALE,
 8 - TCHEKRIK_MOTHERS,
 9 - HIGH_PRIESTESS,
10 - OGLE,
11 - SERAPH_OVERLORD,
12 - SERAPH_GUARD,
13 - ASSASSIN,
14 - MORCALAVIN,
15 - DRANOR,
16 - SIDHE_GUARD,
17 - SIERNAN,
18 - SSITHRA_SCOUT,
19 - SSITHRA_VICTIM,
20 - Mutant Ssithra
21 - Harpy
22 - Fish
23 - Chicken
24 - Plague Ssithra
25 - Gkrokon
26 - Giant Rat
27 - Palace Plague Guard
28 - Invisible Palace Plague Guard



wait - time to wait between spawnings (default 10)


The following fields can be applied to the func_monsterspawner so that the monsters spawned from it will have the values you give them.  The defaults are the monster's normal defaults.  Monsters who do not use buoys may not use all of these fields (ie: rats, harpies, fish, imps, etc.)

mintel - monster intelligence- this basically tells a monster how many buoys away an enemy has to be for it to give up.

melee_range - How close the player has to be, maximum, for the monster to go into melee.  If this is zero, the monster will never melee.  If it is negative, the monster will try to keep this distance from the player.  If the monster has a backup, he'll use it if too clode, otherwise, a negative value here means the monster will just stop running at the player at this distance.
	Examples:
		melee_range = 60 - monster will start swinging it player is closer than 60
		melee_range = 0 - monster will never do a mele attack
		melee_range = -100 - monster will never do a melee attack and will back away (if it has that ability) when player gets too close

missile_range - Maximum distance the player can be from the monster to be allowed to use it's ranged attack.

min_missile_range - Minimum distance the player can be from the monster to be allowed to use it's ranged attack.

bypass_missile_chance - Chance that a monster will NOT fire it's ranged attack, even when it has a clear shot.  This, in effect, will make the monster come in more often than hang back and fire.  A percentage (0 = always fire/never close in, 100 = never fire/always close in).- must be whole number

jump_chance - every time the monster has the opportunity to jump, what is the chance (out of 100) that he will... (100 = jump every time)- must be whole number

wakeup_distance - How far (max) the player can be away from the monster before it wakes up.  This just means that if the monster can see the player, at what distance should the monster actually notice him and go for him.
*/
void SP_func_monsterspawner (edict_t *self)
{

	self->solid = SOLID_NOT;

	if (self->style == MS_NOTHING)
	{
		gi.dprintf ("func_monsterspawner with a style of 0 at %s\n", vtos(self->s.origin));
	}
	else if (self->style >= MS_MAX)
	{
		gi.dprintf ("func_monsterspawner with a bad style of %d at %s\n",
			self->style, vtos(self->s.origin));
	}

	if (!self->count)
		self->count = 1;

	if (!self->wait)
		self->wait = 10;

	if(!self->s.scale)
		self->s.scale = 1.0f;

	if (self->targetname)
		self->use = monsterspawner_use;
	else
	{
		self->think = monsterspawner_go;
		self->nextthink = level.time + self->wait;
	}

	self->maxrange = st.distance;

	self->enemy = NULL;

	gi.linkentity (self);

}


/*QUAK-ED func_killbox (1 0 0) ?
Kills everything inside when fired, irrespective of protection.
*/
/*
void use_killbox (edict_t *self, edict_t *other, edict_t *activator)
{
	KillBox (self);
}
*/
/*
void SP_func_killbox (edict_t *ent)
{
	gi.setmodel (ent, ent->model);
	ent->use = use_killbox;
	ent->svflags = SVF_NOCLIENT;
}

*/


//===========================
//  These are just temp entities until the actual code is written for each monster
//===========================

/*QUAKED monster_chkroktk (1 .5 0) (-16 -16 -26) (16 16 26) AMBUSH ASLEEP
The  chkroktk
*/
void SP_monster_chkroktk (edict_t *self)
{
	self->style = MS_CHKROKTK;
	monsterspawner_go(self);
}



/*QUAKED character_sidhe_guard (1 .5 0) (-16 -16 -26) (16 16 26) AMBUSH ASLEEP
The Sidhe Guard
*/
void SP_character_sidhe_guard (edict_t *self)
{
	self->style = MS_SIDHE_GUARD;
	monsterspawner_go(self);
}

