//
// m_plagueSsitra.c
//
// Copyright 1998 Raven Software
//

#include <float.h> //mxd
#include "m_plaguesSithra.h"
#include "m_plaguesSithra_shared.h"
#include "m_plaguesSithra_anim.h"
#include "g_debris.h" //mxd
#include "g_DefaultMessageHandler.h"
#include "g_obj.h" //mxd
#include "mg_ai.h" //mxd
#include "mg_guide.h" //mxd
#include "m_stats.h"
#include "Random.h"
#include "Utilities.h"
#include "Vector.h"
#include "g_monster.h"
#include "g_local.h"

#pragma region ========================== Plague Ssithra Base Info ==========================

static const animmove_t* animations[NUM_ANIMS] =
{
	&ssithra_move_idle1,
	&ssithra_move_walk1,
	&ssithra_move_backpedal1,
	&ssithra_move_bound1,
	&ssithra_move_death_a1,
	&ssithra_move_death_b1,
	&ssithra_move_dive1,
	&ssithra_move_duckshoot1,
	&ssithra_move_duck1,
	&ssithra_move_gallop1,
	&ssithra_move_fjump,
	&ssithra_move_idlebasic1,
	&ssithra_move_idleright1,
	&ssithra_move_melee1,
	&ssithra_move_meleest,
	&ssithra_move_namor1,
	&ssithra_move_pain_a1,
	&ssithra_move_shoot1,
	&ssithra_move_startle1,
	&ssithra_move_swimforward1,
	&ssithra_move_swimwander,
	&ssithra_move_water_death1,
	&ssithra_move_water_idle1,
	&ssithra_move_water_pain_a1,
	&ssithra_move_water_pain_b1,
	&ssithra_move_water_shoot1,
	&ssithra_move_run1,
	&ssithra_move_spinright,
	&ssithra_move_spinright_go,
	&ssithra_move_spinleft,
	&ssithra_move_spinleft_go,
	&ssithra_move_faceandnamor,
	&ssithra_move_dead_a,
	&ssithra_move_lookright,
	&ssithra_move_lookleft,
	&ssithra_move_transup,
	&ssithra_move_transdown,
	&ssithra_move_headless,
	&ssithra_move_headlessloop,
	&ssithra_move_death_c,
	&ssithra_move_dead_b,
	&ssithra_move_dead_water,
	&ssithra_move_sliced,
	&ssithra_move_delay,
	&ssithra_move_duckloop,
	&ssithra_move_unduck,
	&ssithra_move_lunge
};

static int sounds[NUM_SOUNDS];

#pragma endregion

#pragma region ========================== Utility functions =========================

static qboolean SsithraCheckInWater(edict_t* self) //mxd. Named 'ssithraCheckInWater' in original logic.
{
	// In water?
	if ((self->flags & FL_INWATER) && !(self->flags & FL_INLAVA) && !(self->flags & FL_INSLIME) && (self->waterlevel > 2 || self->groundentity == NULL))
	{
		self->monsterinfo.aiflags |= AI_NO_MELEE;
		return true;
	}

	if (!(self->s.fmnodeinfo[MESH__LEFTARM].flags & FMNI_NO_DRAW))
		self->monsterinfo.aiflags &= ~AI_NO_MELEE;

	return false;
}

static void SsithraDecideStand(edict_t* self) //mxd. Named 'ssithra_decide_stand' in original logic.
{
	if (SsithraCheckInWater(self))
	{
		SetAnim(self, ANIM_WATER_IDLE);
		return;
	}

	switch (self->curAnimID)
	{
		case ANIM_STAND1:
		case ANIM_IDLEBASIC:
			if (irand(0, 10) < 7) //mxd. flrand() in original logic.
				SetAnim(self, ANIM_STAND1);
			else if (irand(0, 10) < 7) //mxd. flrand() in original logic.
				SetAnim(self, ANIM_IDLERIGHT);
			else if (irand(0, 10) < 5) //mxd. flrand() in original logic.
				SetAnim(self, ANIM_LOOKRIGHT);
			else
				SetAnim(self, ANIM_LOOKLEFT);
			break;

		case ANIM_IDLERIGHT:
		case ANIM_LOOKLEFT:
		case ANIM_LOOKRIGHT:
			SetAnim(self, ((irand(0, 10) < 6) ? ANIM_STAND1 : ANIM_IDLEBASIC)); //mxd. flrand() in original logic.
			break;

		default:
			SetAnim(self, ANIM_STAND1);
			break;
	}
}

static qboolean SsithraHaveWaterLedgeNearEnemy(edict_t* self) //mxd. Named 'ssithraWaterLedgeNearEnemy' in original logic.
{
	vec3_t target_origin;

	if ((self->spawnflags & MSF_FIXED) || !MG_TryGetTargetOrigin(self, target_origin))
		return false;

	vec3_t enemy_dir;
	VectorSubtract(target_origin, self->s.origin, enemy_dir);
	VectorNormalize(enemy_dir);

	vec3_t end_pos;
	VectorMA(self->s.origin, 128.0f, enemy_dir, end_pos);

	trace_t trace;
	gi.trace(self->s.origin, self->mins, self->maxs, end_pos, self, MASK_SOLID, &trace);

	return (trace.fraction < 1.0f); // When trace.fraction == 1, no ledge to jump up on.
}

static void SsithraTryJump(edict_t* self) //mxd. Named 'ssithraWhichJump' in original logic.
{
	vec3_t target_origin;

	if ((self->spawnflags & MSF_FIXED) || !MG_TryGetTargetOrigin(self, target_origin))
		return;

	if (SsithraCheckInWater(self))
	{
		if (!(gi.pointcontents(target_origin) & CONTENTS_WATER))
			SetAnim(self, ANIM_NAMOR);

		return;
	}

	SetAnim(self, ANIM_BOUND);

	vec3_t forward;
	AngleVectors(self->s.angles, forward, NULL, NULL);

	VectorScale(forward, SSITHRA_HOP_VELOCITY, self->velocity);
	self->velocity[2] = SSITHRA_HOP_VELOCITY + 32.0f;
}

void SsithraCheckJump(edict_t* self) //mxd. Named 'ssithraCheckJump' in original logic.
{
	if (self->spawnflags & MSF_FIXED)
		return;

	vec3_t target_origin;
	vec3_t target_mins;

	if (self->monsterinfo.searchType == SEARCH_BUOY)
	{
		if (self->buoy_index < 0 || self->buoy_index > level.active_buoys)
			return;

		VectorCopy(level.buoy_list[self->buoy_index].origin, target_origin);
		VectorClear(target_mins);
	}
	else
	{
		if (self->goalentity == NULL)
			return;

		VectorCopy(self->goalentity->s.origin, target_origin);
		VectorCopy(self->goalentity->mins, target_mins);
	}

	if (!MG_IsInforntPos(self, target_origin))
		return;

	// Jumping down?
	if (target_origin[2] < self->s.origin[2] - 28.0f)
	{
		// Setup the trace
		if (SsithraCheckInWater(self))
			return;

		vec3_t s_maxs;
		VectorCopy(self->maxs, s_maxs);
		s_maxs[2] += 32.0f;

		vec3_t source;
		VectorCopy(self->s.origin, source);

		vec3_t forward;
		AngleVectors(self->s.angles, forward, NULL, NULL);

		VectorMA(source, 128.0f, forward, source);

		trace_t trace;
		gi.trace(self->s.origin, self->mins, s_maxs, source, self, MASK_MONSTERSOLID, &trace);

		if (trace.fraction == 1.0f)
		{
			// Clear ahead and above.
			vec3_t source2;
			VectorCopy(source, source2);
			source2[2] -= 1024.0f;

			// Trace down.
			gi.trace(source, self->mins, self->maxs, source2, self, MASK_ALL, &trace);

			if (trace.fraction == 1.0f || trace.startsolid || trace.allsolid) // Check down - too far or allsolid.
				return;

			if (trace.contents != CONTENTS_SOLID)
			{
				// Jumping into water?
				if ((trace.contents & (CONTENTS_WATER | CONTENTS_SLIME)) || trace.ent == self->enemy)
				{
					vec3_t dir;
					VectorSubtract(trace.endpos, self->s.origin, dir);
					VectorNormalize(dir);
					self->ideal_yaw = VectorYaw(dir);

					if (self->monsterinfo.jump_time < level.time)
					{
						// Check depth.
						VectorCopy(trace.endpos, source);
						VectorCopy(source, source2);
						source2[2] -= 64.0f;

						const vec3_t mins = { self->mins[0], self->mins[1], 0.0f };
						const vec3_t maxs = { self->maxs[0], self->maxs[1], 1.0f };

						gi.trace(source, mins, maxs, source2, self, MASK_SOLID, &trace);

						if (trace.fraction < 1.0f || trace.startsolid || trace.allsolid)
							SsithraTryJump(self);
						else
							SetAnim(self, ANIM_DIVE);

						self->monsterinfo.jump_time = level.time + 1.0f;
					}
				}
			}
			else
			{
				vec3_t dir;
				VectorSubtract(trace.endpos, self->s.origin, dir);
				VectorNormalize(dir);
				self->ideal_yaw = VectorYaw(dir);

				if (self->monsterinfo.jump_time < level.time)
				{
					SsithraTryJump(self);
					self->monsterinfo.jump_time = level.time + 1.0f;
				}
			}
		} // Else not clear infront.

		return;
	}

	// Check if we should jump up.
	qboolean jump_up_check = (vhlen(self->s.origin, target_origin) < 200.0f);

	if (!jump_up_check)
	{
		vec3_t source;
		VectorCopy(self->s.origin, source);
		source[2] -= 10.0f;

		if (gi.pointcontents(source) & CONTENTS_WATER)
		{
			//FIXME: swimming can bring origin out of water!
			vec3_t forward;
			AngleVectors(self->s.angles, forward, NULL, NULL);
			VectorMA(self->s.origin, 72.0f, forward, source);

			trace_t trace;
			gi.trace(self->s.origin, self->mins, self->maxs, source, self, MASK_SOLID, &trace);

			if (trace.fraction < 1.0f)
				jump_up_check = true;

			// Shore is within 72 units of me.
		}
		else // Enemy far away, in front, and water in front of me.
		{
			// Check if water in front.
			vec3_t forward;
			AngleVectors(self->s.angles, forward, NULL, NULL);
			VectorMA(self->s.origin, 48.0f, forward, source);

			trace_t trace;
			gi.trace(self->s.origin, self->mins, self->maxs, source, self, MASK_SOLID, &trace);

			VectorCopy(trace.endpos, source);
			source[2] -= 128.0f;

			gi.trace(trace.endpos, self->mins, self->maxs, source, self, MASK_SOLID | MASK_WATER, &trace);

			if (trace.fraction < 1.0f && (trace.contents & CONTENTS_WATER))
			{
				VectorCopy(trace.endpos, source);

				vec3_t source2;
				VectorCopy(source, source2);
				source[2] -= 64.0f;

				const vec3_t mins = { self->mins[0], self->mins[1], 0.0f };
				const vec3_t maxs = { self->maxs[0], self->maxs[1], 1.0f };

				gi.trace(source, mins, maxs, source2, self, MASK_SOLID, &trace);

				if (trace.fraction < 1.0f || trace.allsolid || trace.startsolid)
					SsithraTryJump(self);
				else
					SetAnim(self, ANIM_DIVE);

				return;
			}
		}
	}

	if (!jump_up_check)
		return;

	// Jumping up?
	if (target_origin[2] > self->s.origin[2] + 28.0f || !(self->monsterinfo.aiflags & AI_FLEE))
	{
		vec3_t source;
		VectorCopy(self->s.origin, source);

		//FIXME: what about if running away?
		const float height_diff = (target_origin[2] + target_mins[2]) - (self->s.origin[2] + self->mins[2]) + 32.0f;
		source[2] += height_diff;

		trace_t trace;
		gi.trace(self->s.origin, self->mins, self->maxs, source, self, MASK_ALL, &trace);

		if (trace.fraction == 1.0f)
		{
			// Clear above.
			vec3_t forward;
			AngleVectors(self->s.angles, forward, NULL, NULL);

			vec3_t source2;
			VectorMA(source, 64.0f, forward, source2);
			source2[2] -= 24.0f;

			// Trace forward and down a little.
			gi.trace(source, self->mins, self->maxs, source2, self, MASK_ALL, &trace);

			if (trace.fraction < 0.1f || trace.allsolid || trace.startsolid || trace.ent == (struct edict_s*)-1) // Can't jump up, no ledge.
				return;

			vec3_t dir;
			VectorSubtract(trace.endpos, self->s.origin, dir);
			VectorNormalize(dir);
			self->ideal_yaw = VectorYaw(dir);

			if (self->monsterinfo.jump_time < level.time)
			{
				SsithraTryJump(self);
				self->monsterinfo.jump_time = level.time + 1.0f;
			}
		}

		return;
	}

	// Check to jump over something.
	vec3_t save_org;
	VectorCopy(self->s.origin, save_org);
	const qboolean can_move = M_walkmove(self, self->s.angles[YAW], 64.0f);
	VectorCopy(save_org, self->s.origin);

	if (!can_move)
	{
		vec3_t forward;
		AngleVectors(self->s.angles, forward, NULL, NULL);

		vec3_t end_pos;
		VectorMA(self->s.origin, 128.0f, forward, end_pos);

		vec3_t mins;
		VectorCopy(self->mins, mins);
		mins[2] += 24.0f; // Can clear it.

		trace_t trace;
		gi.trace(self->s.origin, mins, self->maxs, end_pos, self, MASK_SOLID, &trace);

		if (trace.allsolid || trace.startsolid || (trace.fraction < 1.0f && trace.ent != self->enemy))
			return;

		// Go for it!
		ssithra_jump(self, 128.0f, trace.fraction * 200.0f, 0.0f);
		SetAnim(self, ANIM_BOUND);
	}
}

static void SsithraSplit(edict_t* self, const int body_part) //mxd. Named 'ssithraSplit' in original logic.
{
	// Blood stripe.
	vec3_t right;
	vec3_t up;
	AngleVectors(self->s.angles, NULL, right, up);

	vec3_t p1 = { 0 };
	VectorMA(p1, 6.0f, up, p1);
	VectorMA(p1, 10.0f, right, p1);

	vec3_t p2 = { 0 };
	VectorMA(p2, -6.0f, up, p2);
	VectorMA(p2, -10.0f, right, p2);

	vec3_t dir;
	VectorSubtract(p2, p1, dir);
	VectorNormalize(dir);
	Vec3ScaleAssign(40.0f, dir);

	// Why doesn't this work?
	gi.CreateEffect(&self->s, FX_BLOOD, 0, p1, "ub", dir, 20);

	Vec3AddAssign(self->s.origin, p2);
	SprayDebris(self, p2, 6, 200);

	// Spawn top part.
	edict_t* top_half = G_Spawn();

	top_half->svflags |= (SVF_MONSTER | SVF_DEADMONSTER);
	top_half->s.renderfx |= RF_FRAMELERP;
	top_half->s.effects |= EF_CAMERA_NO_CLIP;
	top_half->takedamage = DAMAGE_AIM;
	top_half->health = 25;
	top_half->max_health = top_half->health;
	top_half->clipmask = MASK_MONSTERSOLID;

	top_half->dead_state = DEAD_DEAD;
	top_half->monsterinfo.thinkinc = MONSTER_THINK_INC;
	top_half->monsterinfo.nextframeindex = -1;
	top_half->friction = 0.1f;

	VectorCopy(self->s.origin, top_half->s.origin);
	VectorCopy(top_half->s.origin, top_half->s.old_origin);
	top_half->s.origin[2] += 10.0f;

	VectorCopy(self->s.angles, top_half->s.angles);

	top_half->think = SsithraSlideOffThink;
	top_half->nextthink = level.time + FRAMETIME * 10.0f;

	top_half->materialtype = MAT_FLESH;
	top_half->mass = 300;
	self->mass = top_half->mass;

	top_half->movetype = PHYSICSTYPE_STEP;
	top_half->solid = SOLID_BBOX;
	top_half->owner = self;

	VectorSet(top_half->mins, -16.0f, -16.0f, self->mins[2]);
	VectorSet(top_half->maxs, 16.0f, 16.0f, 16.0f);

	VectorSet(self->maxs, 16.0f, 16.0f, 0.0f);

	//FIXME: sometimes top half appears too low and forward?
	VectorClear(self->knockbackvel);
	VectorClear(self->velocity);

	top_half->s.modelindex = self->s.modelindex;
	top_half->s.frame = (short)((self->s.frame == 0) ? FRAME_startle32 : self->s.frame);
	top_half->s.skinnum = self->s.skinnum;
	top_half->s.scale = self->s.scale; //BUGFIX: mxd. 'top_half->s.scale = top_half->s.scale' in original logic.
	top_half->monsterinfo.otherenemyname = "obj_barrel"; //TODO: why?..

	int node_num = 1;
	for (int which_node = 1; which_node <= 16384; which_node *= 2, node_num++) // Bitwise.
	{
		if (body_part & which_node)
		{
			// Turn on this node on top and keep them.
			top_half->s.fmnodeinfo[node_num] = self->s.fmnodeinfo[node_num]; // Copy skins and flags and colors.
			top_half->s.fmnodeinfo[node_num].flags &= ~FMNI_NO_DRAW;
			self->s.fmnodeinfo[node_num].flags |= FMNI_NO_DRAW;
		}
		else
		{
			// Turn off this node on top.
			top_half->s.fmnodeinfo[node_num].flags |= FMNI_NO_DRAW;
		}
	}

	top_half->s.fmnodeinfo[MESH__CAPBOTTOMUPPERTORSO].flags &= ~FMNI_NO_DRAW;
	self->s.fmnodeinfo[MESH__CAPLOWERTORSO].flags &= ~FMNI_NO_DRAW;
	self->s.fmnodeinfo[MESH__RIGHT2SPIKE].flags |= FMNI_NO_DRAW;

	self->nextthink = FLT_MAX; //mxd. 9999999999999999 in original logic.
}

static qboolean SsithraCanThrowNode(edict_t* self, const int node_id, int* throw_nodes) //mxd. Named 'canthrownode' in original logic.
{
	static const int bit_for_mesh_node[NUM_MESH_NODES] = //mxd. Made local static.
	{
		BIT_POLY,
		BIT_LOWERTORSO,
		BIT_CAPLOWERTORSO,
		BIT_LEFTLEG,
		BIT_RIGHTLEG,
		BIT_UPPERTORSO,
		BIT_CAPTOPUPPERTORSO,
		BIT_CAPBOTTOMUPPERTORSO,
		BIT_LEFTARM,
		BIT_RIGHTARM,
		BIT_HEAD,
		BIT_CENTERSPIKE,
		BIT_LEFT1SPIKE,
		BIT_RIGHT1SPIKE,
		BIT_RIGHT2SPIKE,
		BIT_CAPHEAD
	};

	// See if it's on, if so, add it to throw_nodes. Turn it off on thrower.
	if (!(self->s.fmnodeinfo[node_id].flags & FMNI_NO_DRAW))
	{
		*throw_nodes |= bit_for_mesh_node[node_id];
		self->s.fmnodeinfo[node_id].flags |= FMNI_NO_DRAW;

		return true;
	}

	return false;
}

static HitLocation_t SsithraConvertDeadHitLocation(const edict_t* self, const HitLocation_t hl) //mxd. Named 'ssithra_convert_hitloc_dead' in original logic.
{
	const qboolean fellback = (self->curAnimID == ANIM_DEATH_A);

	switch (hl)
	{
		case hl_Head:
			return (fellback ? hl_TorsoFront : hl_TorsoBack);

		case hl_TorsoFront: // Split in half?
			if (fellback)
				return (irand(0, 1) == 0 ? hl_LegUpperRight : hl_LegUpperLeft);
			return hl_Head;

		case hl_TorsoBack: // Split in half?
			if (fellback)
				return hl_Head;
			return (irand(0, 1) == 0 ? hl_LegUpperRight : hl_LegUpperLeft);

		case hl_ArmUpperLeft:
			return hl_ArmLowerLeft;

		case hl_ArmLowerLeft: // Left arm.
			return hl_ArmUpperLeft;

		case hl_ArmUpperRight:
			return hl_ArmLowerRight;

		case hl_ArmLowerRight: // Right arm.
			return hl_ArmUpperRight;

		case hl_LegUpperLeft:
			return hl_LegLowerLeft;

		case hl_LegLowerLeft: // Left leg.
			return hl_LegUpperLeft;

		case hl_LegUpperRight:
			return hl_LegLowerRight;

		case hl_LegLowerRight: // Right leg.
			return hl_LegUpperRight;

		default:
			return irand(hl_Head, hl_LegLowerRight);
	}
}

static void SsithraArrowInit(edict_t* arrow) //mxd. Named 'create_ssith_arrow' in original logic.
{
	arrow->movetype = MOVETYPE_FLYMISSILE;
	arrow->solid = SOLID_BBOX;
	arrow->classname = "Ssithra_Arrow";
	arrow->touch = SsithraArrowTouch;
	arrow->enemy = NULL;
	arrow->clipmask = MASK_SHOT;
	arrow->s.scale = 0.75f;
	arrow->s.effects |= EF_CAMERA_NO_CLIP;
	arrow->svflags |= SVF_ALWAYS_SEND;
	arrow->s.modelindex = (byte)gi.modelindex("models/objects/exarrow/tris.fm");
}

// The arrow needs to bounce.
static void ReflectedSsithraArrowInit(edict_t* self, edict_t* arrow) //mxd. Named 'make_arrow_reflect' in original logic.
{
	SsithraArrowInit(arrow);

	arrow->s.modelindex = self->s.modelindex;
	VectorCopy(self->s.origin, arrow->s.origin);
	arrow->owner = self->owner;
	arrow->enemy = self->enemy;

	arrow->touch = self->touch;
	arrow->think = G_FreeEdict;
	arrow->nextthink = self->nextthink;

	Create_rand_relect_vect(self->velocity, arrow->velocity);

	vectoangles(arrow->velocity, arrow->s.angles);
	arrow->s.angles[YAW] += 90.0f;

	Vec3ScaleAssign(SSITHRA_SPOO_SPEED / 2.0f, arrow->velocity);

	G_LinkMissile(arrow);
}

static void SsithraDoArrow(edict_t* self) //mxd. Named 'ssithraDoArrow' in original logic. Removed unused 'z_offs' arg.
{
	if (self->s.fmnodeinfo[MESH__RIGHTARM].flags & FMNI_NO_DRAW)
		return;

	gi.sound(self, CHAN_WEAPON, sounds[SND_ARROW1], 1.0f, ATTN_NORM, 0.0f);
	self->monsterinfo.attack_finished = level.time + 0.4f;

	edict_t* arrow = G_Spawn();

	SsithraArrowInit(arrow);

	arrow->owner = self;
	arrow->enemy = self->enemy;

	vec3_t forward;
	vec3_t right;
	AngleVectors(self->s.angles, forward, right, NULL);

	VectorCopy(self->s.origin, arrow->s.origin);
	VectorMA(arrow->s.origin, 16.0f, forward, arrow->s.origin);
	VectorMA(arrow->s.origin, -4.0f, right, arrow->s.origin);
	arrow->s.origin[2] += 16.0f;

	vec3_t check_lead = { 0 };

	if (SKILL > SKILL_MEDIUM)
	{
		ExtrapolateFireDirection(self, arrow->s.origin, SSITHRA_SPOO_SPEED, self->enemy, 0.3f, check_lead);
	}
	else
	{
		vec3_t enemy_dir;
		VectorSubtract(self->enemy->s.origin, arrow->s.origin, enemy_dir);
		VectorNormalize(enemy_dir);

		if (DotProduct(enemy_dir, forward) >= 0.3f)
			forward[2] = enemy_dir[2];
	}

	if (Vec3IsZero(check_lead))
		VectorScale(forward, SSITHRA_SPOO_SPEED, arrow->velocity);
	else
		VectorScale(check_lead, SSITHRA_SPOO_SPEED, arrow->velocity);

	VectorCopy(arrow->velocity, arrow->movedir);
	VectorNormalize(arrow->movedir);

	vectoangles(arrow->movedir, arrow->s.angles);
	arrow->s.angles[PITCH] = anglemod(-arrow->s.angles[PITCH]);
	arrow->s.angles[YAW] += 90.0f;

	gi.CreateEffect(&arrow->s, FX_M_EFFECTS, CEF_OWNERS_ORIGIN, arrow->s.origin, "bv", FX_MSSITHRA_ARROW, arrow->velocity);

	G_LinkMissile(arrow);

	arrow->think = G_FreeEdict;
	arrow->nextthink = level.time + 3.0f;
}

static void SsithraDoDuckArrow(edict_t* self, const float z_offs) //mxd. Named 'ssithraDoDuckArrow' in original logic.
{
	if (self->s.fmnodeinfo[MESH__RIGHTARM].flags & FMNI_NO_DRAW)
		return;

	gi.sound(self, CHAN_WEAPON, sounds[SND_ARROW_FIRE], 1.0f, ATTN_NORM, 0.0f);
	self->monsterinfo.attack_finished = level.time + 0.4f;

	edict_t* arrow = G_Spawn();

	SsithraArrowInit(arrow);

	arrow->touch = SsithraDuckArrowTouch;
	arrow->owner = self;
	arrow->enemy = self->enemy;

	vec3_t forward;
	vec3_t right;
	AngleVectors(self->s.angles, forward, right, NULL);

	VectorCopy(self->s.origin, arrow->s.origin);
	VectorMA(arrow->s.origin, self->s.scale * 12.0f, forward, arrow->s.origin);
	VectorMA(arrow->s.origin, self->s.scale * 4.0f, right, arrow->s.origin);
	arrow->s.origin[2] += z_offs;

	arrow->s.scale = 1.5f;

	vec3_t check_lead = { 0 };

	if (SKILL > SKILL_MEDIUM)
	{
		ExtrapolateFireDirection(self, arrow->s.origin, SSITHRA_SPOO_SPEED, self->enemy, 0.3f, check_lead);
	}
	else
	{
		vec3_t enemy_dir;
		VectorSubtract(self->enemy->s.origin, arrow->s.origin, enemy_dir);
		VectorNormalize(enemy_dir);

		if (DotProduct(enemy_dir, forward) >= 0.3f)
			forward[2] = enemy_dir[2];
	}

	if (Vec3IsZero(check_lead))
		VectorScale(forward, SSITHRA_SPOO_SPEED * 1.5f, arrow->velocity);
	else
		VectorScale(check_lead, SSITHRA_SPOO_SPEED * 1.5f, arrow->velocity);

	VectorCopy(arrow->velocity, arrow->movedir);
	VectorNormalize(arrow->movedir);

	vectoangles(arrow->movedir, arrow->s.angles);
	arrow->s.angles[PITCH] = anglemod(-arrow->s.angles[PITCH]);
	arrow->s.angles[YAW] += 90.0f;

	gi.CreateEffect(&arrow->s, FX_M_EFFECTS, CEF_OWNERS_ORIGIN | CEF_FLAG6, arrow->s.origin, "bv", FX_MSSITHRA_ARROW, arrow->velocity);

	G_LinkMissile(arrow);

	arrow->think = SsithraArrowExplodeThink;
	arrow->nextthink = level.time + 5.0f;
}

static void SsithraForwardJump(edict_t* self) //mxd. Named 'ssithraJumpEvade' in original logic.
{
	vec3_t forward;
	AngleVectors(self->s.angles, forward, NULL, NULL);

	VectorSet(self->movedir, 0.0f, 0.0f, 300.0f);
	VectorMA(self->movedir, 200.0f, forward, self->movedir);
	SetAnim(self, ANIM_FJUMP);
}

static qboolean SsithraAlert(edict_t* self, alertent_t* alerter, edict_t* enemy) //mxd. Named 'ssithraAlerted' in original logic.
{
	// Not looking around, not a sound-alert.
	if (self->alert_time < level.time && !(alerter->alert_svflags & SVF_ALERT_NO_SHADE) && SKILL < SKILL_VERYHARD && !(self->monsterinfo.aiflags & AI_NIGHTVISION) && enemy->light_level < irand(6, 77))
		return false;

	// The alert action happened in front of me, but the enemy is behind or the alert is behind me.
	if (!MG_IsInforntPos(self, alerter->origin))
	{
		// 50% chance of startling them up if not already in startle anim.
		if (irand(0, 1) == 1 && self->curAnimID != ANIM_IDLEBASIC)
		{
			// Startle me, but don't wake up just yet.
			if (alerter->lifetime < level.time + 4.0f)
				self->alert_time = level.time + 4.0f; // Be ready for 4 seconds to wake up if alerted again.
			else
				self->alert_time = alerter->lifetime; // Be alert as long as the alert sticks around.

			vec3_t save_angles;
			VectorCopy(self->v_angle_ofs, save_angles);
			VectorSet(self->v_angle_ofs, 0.0f, -90.0f, 0.0f);

			// Fancy way of seeing if explosion was to right.
			SetAnim(self, (MG_IsInforntPos(self, alerter->origin) ? ANIM_IDLERIGHT : ANIM_STARTLE)); //mxd. Inline ssithraLookRight() / ssithraStartle(). //FIXME: if already looking right, see you.
			VectorCopy(save_angles, self->v_angle_ofs);

			return false;
		}

		// Spin around and wake up!
		self->spawnflags |= MSF_SSITHRA_SPIN;
	}
	else if (!AI_IsInfrontOf(self, enemy))
	{
		// 50% chance of startling them up if not already in startle anim.
		if (irand(0, 1) == 1 && self->curAnimID != ANIM_IDLEBASIC)
		{
			// Startle me, but don't wake up just yet.
			self->alert_time = level.time + 4.0f; // Be ready to wake up for next 4 seconds.

			vec3_t save_angles;
			VectorCopy(self->v_angle_ofs, save_angles);
			VectorSet(self->v_angle_ofs, 0.0f, -90.0f, 0.0f);

			// Fancy way of seeing if explosion was to right.
			SetAnim(self, (AI_IsInfrontOf(self, enemy) ? ANIM_IDLERIGHT : ANIM_STARTLE)); //mxd. Inline ssithraLookRight() / ssithraStartle(). //FIXME: if already looking right, see you.
			VectorCopy(save_angles, self->v_angle_ofs);

			return false;
		}

		// Spin around and wake up!
		self->spawnflags |= MSF_SSITHRA_SPIN;
	}

	self->enemy = ((enemy->svflags & SVF_MONSTER) ? alerter->enemy : enemy);
	AI_FoundTarget(self, true);

	return true;
}

#pragma endregion

#pragma region ========================== Message handlers ==========================

static void SsithraStandMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'ssithra_stand' in original logic.
{
	if (self->ai_mood == AI_MOOD_DELAY)
	{
		SetAnim(self, ANIM_DELAY);
		return;
	}

	if (SsithraCheckInWater(self))
	{
		SetAnim(self, ANIM_WATER_IDLE);
		return;
	}

	switch (self->curAnimID)
	{
		case ANIM_STAND1:
			SetAnim(self, irand(0, 10) < 8 ? ANIM_STAND1 : ANIM_IDLEBASIC); //mxd. flrand() in original logic.
			break;

		case ANIM_IDLERIGHT:
			SetAnim(self, irand(0, 10) < 6 ? ANIM_STAND1 : ANIM_IDLEBASIC); //mxd. flrand() in original logic.
			break;

		default:
			SetAnim(self, ANIM_STAND1);
			break;
	}
}

static void SsithraWalkMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'ssithra_walk' in original logic.
{
	SetAnim(self, ((self->spawnflags & MSF_FIXED) ? ANIM_DELAY : ANIM_WALK1));
}

static void SsithraRunMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'ssithra_gallop' in original logic.
{
	if (self->curAnimID == ANIM_SPINRIGHT)
	{
		SetAnim(self, ANIM_SPINRIGHT_GO);
		return;
	}

	if (self->curAnimID == ANIM_SPINLEFT)
	{
		SetAnim(self, ANIM_SPINLEFT_GO);
		return;
	}

	if (self->enemy == NULL || self->enemy->health <= 0)
	{
		SetAnim(self, ANIM_STAND1);
		return;
	}

	if (self->spawnflags & MSF_FIXED)
	{
		SetAnim(self, ANIM_DELAY);
		return;
	}

	if (self->spawnflags & MSF_SSITHRA_NAMOR) // Out of water jump.
	{
		self->spawnflags &= ~MSF_SSITHRA_NAMOR;
		SetAnim(self, ANIM_NAMOR);

		return;
	}

	if (self->spawnflags & MSF_SSITHRA_SPIN) // Spin.
	{
		self->spawnflags &= ~MSF_SSITHRA_SPIN;
		SetAnim(self, (irand(0, 1) == 1 ? ANIM_SPINRIGHT : ANIM_SPINLEFT));

		return;
	}

	SetAnim(self, ANIM_RUN1);
}

static void SsithraWatchMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'ssithra_idlebasic' in original logic.
{
	SetAnim(self, ANIM_IDLEBASIC);
}

static void SsithraJumpMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'ssithraMsgJump' in original logic.
{
	if (self->spawnflags & MSF_FIXED)
		SetAnim(self, ANIM_DELAY);
	else
		SsithraTryJump(self);
}

static void SsithraDeathPainMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'ssithra_dead_pain' in original logic.
{
	if (msg != NULL && !(self->svflags & SVF_PARTS_GIBBED))
		DismemberMsgHandler(self, msg);
}

static void SsithraPainMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'ssithra_pain' in original logic.
{
	//FIXME: make part fly dir the vector from hit loc to sever loc.
	if (self->dead_state == DEAD_DEAD) // Dead but still being hit.
		return;

	int temp;
	int damage;
	qboolean force_pain;
	ParseMsgParms(msg, "eeiii", &temp, &temp, &force_pain, &damage, &temp);

	if (!force_pain)
	{
		if (self->pain_debounce_time > 0.0f && (irand(0, 10) < 5 || self->groundentity == NULL))
			return;

		if (self->pain_debounce_time > level.time)
			return;
	}

	ssithra_uncrouch(self);

	self->pain_debounce_time = level.time + 2.0f;
	gi.sound(self, CHAN_VOICE, sounds[irand(SND_PAIN1, SND_PAIN2)], 1.0f, ATTN_NORM, 0.0f);

	if (SsithraCheckInWater(self))
		SetAnim(self, ((self->curAnimID == ANIM_SWIMFORWARD) ? ANIM_WATER_PAIN_B : ANIM_WATER_PAIN_A)); //FIXME: underwater pain sound?
	else
		SetAnim(self, ANIM_PAIN_A);
}

static void SsithraDeathMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'ssithra_death' in original logic.
{
	//FIXME: still cut off limbs as dying?
	if (self->monsterinfo.aiflags & AI_DONT_THINK)
	{
		gi.sound(self, CHAN_BODY, sounds[SND_DIE], 1.0f, ATTN_NORM, 0.0f);
		SetAnim(self, irand(ANIM_DEATH_A, ANIM_DEATH_B));

		return;
	}

	self->msgHandler = DyingMsgHandler;

	if (self->dead_state == DEAD_DEAD) // Dead but still being hit.
		return;

	self->dead_state = DEAD_DEAD;

	if (self->health <= -80) // Gib death.
	{
		// Throw variable number of body parts. //TODO: there's a slight chance of throwing the same body part twice!
		for (int i = 0; i < irand(1, 3); i++)
			SsithraDismember(self, irand(80, 160), irand(hl_Head, hl_LegLowerRight) | hl_MeleeHit);

		gi.sound(self, CHAN_BODY, sounds[SND_GIB], 1.0f, ATTN_NORM, 0.0f);
		self->think = BecomeDebris;
		self->nextthink = level.time + FRAMETIME; //mxd. Use define.

		return;
	}

	ssithra_uncrouch(self);

	if (SsithraCheckInWater(self))
	{
		SetAnim(self, ANIM_WATER_DEATH);
		return;
	}

	if (self->health == -69)
	{
		//FIXME: maybe allow dead bodies to be chopped? Make BBOX small?
		gi.linkentity(self);

		self->msgHandler = DeadMsgHandler;
		self->flags |= FL_DONTANIMATE;
		self->svflags |= SVF_DEADMONSTER; // Now treat as a different content type.

		SetAnim(self, ANIM_DEAD_B);
	}
	else if (self->health == -33)
	{
		SetAnim(self, ANIM_DEATH_C);
	}
	else if (irand(0, 10) < 4 || self->health > -10) // Barely dead.
	{
		SetAnim(self, ANIM_DEATH_B);
	}
	else
	{
		SetAnim(self, ANIM_DEATH_A);
	}
}

static void SsithraMeleeMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'ssithra_melee' in original logic.
{
	if (!M_ValidTarget(self, self->enemy))
	{
		QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
		return;
	}

	if (SsithraCheckInWater(self))
	{
		QPostMessage(self, MSG_MISSILE, PRI_DIRECTIVE, NULL);
		return;
	}

	if (!(self->spawnflags & MSF_FIXED) && !(self->monsterinfo.aiflags & AI_NO_MISSILE) && vhlen(self->enemy->s.origin, self->s.origin) - 16.0f < flrand(0.0f, self->melee_range))
	{
		SetAnim(self, ANIM_BACKPEDAL);
		return;
	}

	if (!(self->spawnflags & MSF_FIXED) && M_DistanceToTarget(self, self->enemy) > self->melee_range * 2.0f)
		SetAnim(self, ANIM_MELEE);
	else
		SetAnim(self, ANIM_MELEE_STAND);
}

static void SsithraMissileMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'ssithra_missile' in original logic.
{
	if (!M_ValidTarget(self, self->enemy))
	{
		QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
		return;
	}

	if (SsithraCheckInWater(self))
	{
		if (M_DistanceToTarget(self, self->enemy) < self->melee_range)
			SetAnim(self, (self->curAnimID == ANIM_SWIMFORWARD ? ANIM_TRANSUP : ANIM_WATER_SHOOT));
		else
			ssithra_arrow(self);
	}
	else
	{
		const int chance = ((self->spawnflags & MSF_SSITHRA_CLOTHED) ? 20 : 80);

		if (irand(0, SKILL * 100) > chance)
			SetAnim(self, ANIM_DUCKSHOOT);
		else
			SetAnim(self, ANIM_SHOOT);
	}
}

static void SsithraFallbackMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'ssithra_backup' in original logic.
{
	if (!M_ValidTarget(self, self->enemy))
	{
		QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
		return;
	}

	if (self->spawnflags & MSF_FIXED)
		SetAnim(self, ANIM_DELAY);
	else
		SetAnim(self, (SsithraCheckInWater(self) ? ANIM_WATER_SHOOT : ANIM_BACKPEDAL));
}

static void SsithraEvadeMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'ssithra_evade' in original logic.
{
	edict_t* projectile;
	HitLocation_t hl;
	float eta;
	ParseMsgParms(msg, "eif", &projectile, &hl, &eta);

	int duck_chance;
	int jump_chance;

	switch (hl)
	{
		case hl_Head:
			duck_chance = 90;
			jump_chance = 0;
			break;

		case hl_TorsoFront:
		case hl_TorsoBack:
			duck_chance = 70;
			jump_chance = 0;
			break;

		case hl_ArmUpperLeft: // Left arm.
		case hl_ArmUpperRight: // Right arm.
			duck_chance = 60;
			jump_chance = 0;
			break;

		case hl_ArmLowerLeft: // Left arm.
		case hl_ArmLowerRight: // Right arm.
			duck_chance = 20;
			jump_chance = 30;
			break;

		case hl_LegUpperLeft:
		case hl_LegUpperRight:
			duck_chance = 0;
			jump_chance = 50;
			break;

		case hl_LegLowerLeft: // Left leg.
		case hl_LegLowerRight: // Right leg.
			duck_chance = 0;
			jump_chance = 90;
			break;

		default:
			duck_chance = 20;
			jump_chance = 10;
			break;
	}

	if (irand(0, 100) < jump_chance && !(self->spawnflags & MSF_FIXED))
	{
		SsithraForwardJump(self);
	}
	else if (irand(0, 100) < duck_chance)
	{
		self->evade_debounce_time = level.time + eta;
		ssithra_crouch(self);
	}
}

static void SsithraCheckMoodMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'ssithra_check_mood' in original logic.
{
	ParseMsgParms(msg, "i", &self->ai_mood);
	ssithra_check_mood(self);
}

static void SsithraVoiceSightMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'ssithra_sight' in original logic.
{
#define SSITHRA_SUPPORT_RADIUS 200

	if (self->targetname != NULL || self->monsterinfo.c_mode)
		return; // Cinematic waiting to be activated, don't do this.

	// Have we already said something?
	if (self->monsterinfo.supporters != -1) //TODO: always -1! Remove or call M_FindSupport() somewhere?..
		return;

	byte sight_type;
	edict_t* enemy = NULL;
	ParseMsgParms(msg, "be", &sight_type, &enemy);

	// See if we are the first to see the player.
	if (M_CheckAlert(self, SSITHRA_SUPPORT_RADIUS))
		gi.sound(self, CHAN_BODY, sounds[irand(SND_SIGHT1, SND_SIGHT6)], 1.0f, ATTN_NORM, 0.0f);
}

#pragma endregion

#pragma region ========================== Edict callbacks ===========================

static void SsithraBlocked(edict_t* self, trace_t* trace) //mxd. Named 'ssithra_blocked' in original logic.
{
	if (trace->ent == NULL || trace->ent->movetype == PHYSICSTYPE_NONE || trace->ent->movetype == PHYSICSTYPE_PUSH)
		return;

	const float strength = VectorLength(self->velocity);

	if (strength < 50.0f)
		return;

	vec3_t hit_dir;
	VectorCopy(self->velocity, hit_dir);
	hit_dir[2] = max(0.0f, hit_dir[2]);

	VectorNormalize(hit_dir);
	VectorScale(hit_dir, strength, hit_dir);
	VectorAdd(trace->ent->velocity, hit_dir, trace->ent->knockbackvel);

	if (!(self->spawnflags & MSF_FIXED))
		ssithra_jump(self, 150.0f, 200.0f, 0.0f);
}

static void SsithraSlideFallThink(edict_t* self) //mxd. Named 'ssithraSlideFall' in original logic.
{
	if (self->mins[2] < 0.0f)
	{
		if (self->mins[2] <= -6.0f)
			self->mins[2] += 6.0f;
		else
			self->mins[2] = 0.0f;

		self->nextthink = level.time + FRAMETIME;
	}
	else
	{
		self->friction = 1.0f;
		SetAnim(self->owner, ANIM_SLICED);

		self->owner->msgHandler = DyingMsgHandler;
		self->owner->nextthink = level.time;

		self->think = NULL;
		self->nextthink = THINK_NEVER; //mxd. Use define.
	}
}

static void SsithraSlideOffThink(edict_t* self) //mxd. Named 'ssithraSlideOff' in original logic.
{
	vec3_t right;
	AngleVectors(self->s.angles, NULL, right, NULL);
	VectorScale(right, 100.0f, self->velocity);

	self->think = SsithraSlideFallThink;
	self->nextthink = level.time + FRAMETIME;
}

static qboolean SsithraThrowHead(edict_t* self, float damage, const qboolean dismember_ok, int* throw_nodes) //mxd. Added to simplify logic.
{
	vec3_t gore_spot = { 0.0f, 0.0f, 18.0f };

	if (self->s.fmnodeinfo[MESH__HEAD].flags & FMNI_NO_DRAW)
		return false;

	// Is the pain skin engaged?
	if (self->s.fmnodeinfo[MESH__HEAD].flags & FMNI_USE_SKIN)
		damage *= 1.5f; // Greater chance to cut off if previously damaged.

	if (dismember_ok && flrand(0.0f, (float)self->health) < damage * 0.3f)
	{
		SsithraCanThrowNode(self, MESH__HEAD, throw_nodes);
		SsithraCanThrowNode(self, MESH__CENTERSPIKE, throw_nodes);
		SsithraCanThrowNode(self, MESH__LEFT1SPIKE, throw_nodes);
		SsithraCanThrowNode(self, MESH__RIGHT1SPIKE, throw_nodes);
		SsithraCanThrowNode(self, MESH__RIGHT2SPIKE, throw_nodes);

		self->s.fmnodeinfo[MESH__CAPTOPUPPERTORSO].flags &= ~FMNI_NO_DRAW;

		ThrowBodyPart(self, &gore_spot, *throw_nodes, damage, 0);

		Vec3AddAssign(self->s.origin, gore_spot);
		SprayDebris(self, gore_spot, 8, damage);

		if (self->health > 0 && irand(0, 10) < 3 && !(self->s.fmnodeinfo[MESH__RIGHTARM].flags & FMNI_NO_DRAW))
		{
			// Shooting blind, headless, FIX: make it so can still chop off arms or legs here.
			SetAnim(self, ANIM_HEADLESS);
			self->msgHandler = DyingMsgHandler;
		}
		else
		{
			self->health = 1;
			T_Damage(self, self, self, vec3_origin, gore_spot, vec3_origin, 10, 20, 0, MOD_DIED);
		}

		return true;
	}

	// Set the pain skin.
	self->s.fmnodeinfo[MESH__HEAD].flags |= FMNI_USE_SKIN;
	self->s.fmnodeinfo[MESH__HEAD].skin = self->s.skinnum + 1;

	if (flrand(0.0f, (float)self->health / 4.0f) < damage)
	{
		// No red spray with these, particles?
		if (irand(0, 10) < 3 && SsithraCanThrowNode(self, MESH__CENTERSPIKE, throw_nodes))
			ThrowBodyPart(self, &gore_spot, *throw_nodes, damage, 0);

		if (irand(0, 10) < 3 && SsithraCanThrowNode(self, MESH__RIGHT1SPIKE, throw_nodes))
			ThrowBodyPart(self, &gore_spot, *throw_nodes, damage, 0);

		if (irand(0, 10) < 3 && SsithraCanThrowNode(self, MESH__RIGHT2SPIKE, throw_nodes))
			ThrowBodyPart(self, &gore_spot, *throw_nodes, damage, 0);

		if (irand(0, 10) < 3 && SsithraCanThrowNode(self, MESH__LEFT1SPIKE, throw_nodes))
			ThrowBodyPart(self, &gore_spot, *throw_nodes, damage, 0);
	}

	return false;
}

static void SsithraThrowTorso(edict_t* self, float damage, const qboolean dismember_ok, int* throw_nodes) //mxd. Added to simplify logic.
{
	vec3_t gore_spot = { 0.0f, 0.0f, 12.0f };

	if (self->s.fmnodeinfo[MESH__UPPERTORSO].flags & FMNI_NO_DRAW)
		return;

	if (self->s.fmnodeinfo[MESH__UPPERTORSO].flags & FMNI_USE_SKIN)
		damage *= 1.5f; // Greater chance to cut off if previously damaged.

	if (dismember_ok && flrand(0.0f, (float)self->health) < damage * 0.3f)
	{
		// Seal up the caps left by this split.
		self->s.fmnodeinfo[MESH__CAPBOTTOMUPPERTORSO].flags &= ~FMNI_NO_DRAW;
		self->s.fmnodeinfo[MESH__CAPLOWERTORSO].flags &= ~FMNI_NO_DRAW;

		SsithraCanThrowNode(self, MESH__UPPERTORSO, throw_nodes);
		SsithraCanThrowNode(self, MESH__CAPBOTTOMUPPERTORSO, throw_nodes);
		SsithraCanThrowNode(self, MESH__CAPTOPUPPERTORSO, throw_nodes);
		SsithraCanThrowNode(self, MESH__LEFTARM, throw_nodes);
		SsithraCanThrowNode(self, MESH__RIGHTARM, throw_nodes);
		SsithraCanThrowNode(self, MESH__HEAD, throw_nodes);
		SsithraCanThrowNode(self, MESH__CENTERSPIKE, throw_nodes);
		SsithraCanThrowNode(self, MESH__LEFT1SPIKE, throw_nodes);
		SsithraCanThrowNode(self, MESH__RIGHT1SPIKE, throw_nodes);
		SsithraCanThrowNode(self, MESH__RIGHT2SPIKE, throw_nodes);

		if (self->health > 0 && irand(0, 10) < 3) // Slide off.
		{
			SsithraSplit(self, *throw_nodes);
		}
		else
		{
			ThrowBodyPart(self, &gore_spot, *throw_nodes, damage, FRAME_partrest1);

			Vec3AddAssign(self->s.origin, gore_spot);
			SprayDebris(self, gore_spot, 12, damage);
			SetAnim(self, ANIM_SLICED);
		}

		self->msgHandler = DyingMsgHandler;
	}
	else
	{
		// Set the pain skin.
		self->s.fmnodeinfo[MESH__UPPERTORSO].flags |= FMNI_USE_SKIN;
		self->s.fmnodeinfo[MESH__UPPERTORSO].skin = self->s.skinnum + 1;
	}
}

static void SsithraThrowArm(edict_t* self, float damage, const qboolean dismember_ok, int* throw_nodes, const int mesh_part) //mxd. Added to simplify logic.
{
	if (self->s.fmnodeinfo[mesh_part].flags & FMNI_NO_DRAW)
		return;

	if (self->s.fmnodeinfo[mesh_part].flags & FMNI_USE_SKIN)
		damage *= 1.5f; // Greater chance to cut off if previously damaged.

	if (dismember_ok && flrand(0.0f, (float)self->health) < damage * 0.75f)
	{
		if (SsithraCanThrowNode(self, mesh_part, throw_nodes))
		{
			vec3_t right;
			AngleVectors(self->s.angles, NULL, right, NULL);

			vec3_t gore_spot = { 0.0f, 0.0f, self->maxs[2] * 0.3f };
			const float side = ((mesh_part == MESH__LEFTARM) ? -1.0f : 1.0f);
			VectorMA(gore_spot, 10 * side, right, gore_spot);
			ThrowBodyPart(self, &gore_spot, *throw_nodes, damage, 0);
		}
	}
	else
	{
		// Set the pain skin.
		self->s.fmnodeinfo[mesh_part].flags |= FMNI_USE_SKIN;
		self->s.fmnodeinfo[mesh_part].skin = self->s.skinnum + 1;
	}
}

static void SsithraThrowLeg(edict_t* self, const float damage, int* throw_nodes, const int mesh_part) //mxd. Added to simplify logic.
{
	if (self->health > 0)
	{
		// Still alive.
		if (self->s.fmnodeinfo[mesh_part].flags & FMNI_USE_SKIN)
			return;

		self->s.fmnodeinfo[mesh_part].flags |= FMNI_USE_SKIN;
		self->s.fmnodeinfo[mesh_part].skin = self->s.skinnum + 1;
	}
	else
	{
		if (self->s.fmnodeinfo[mesh_part].flags & FMNI_NO_DRAW)
			return;

		if (SsithraCanThrowNode(self, mesh_part, throw_nodes))
		{
			vec3_t right;
			AngleVectors(self->s.angles, NULL, right, NULL);

			vec3_t gore_spot = { 0.0f, 0.0f, self->maxs[2] * 0.3f };
			const float side = ((mesh_part == MESH__LEFTLEG) ? -1.0f : 1.0f); //BUGFIX: original logic uses -10 for both legs.
			VectorMA(gore_spot, 10 * side, right, gore_spot);
			ThrowBodyPart(self, &gore_spot, *throw_nodes, damage, 0);
		}
	}
}

static void SsithraDismember(edict_t* self, int damage, HitLocation_t hl) //mxd. Named 'ssithra_dismember' in original logic.
{
	//FIXME: make part fly dir the vector from hit loc to sever loc.
	qboolean dismember_ok = false;

	if (hl & hl_MeleeHit)
	{
		dismember_ok = true;
		hl &= ~hl_MeleeHit;
	}

	if (hl <= hl_NoneSpecific || hl >= hl_Max) //mxd. '> hl_Max' in original logic.
		return;

	if (self->health > 0)
	{
		switch (self->curAnimID)
		{
			// Hit front chest during shoot or melee, may have hit arms.
			case ANIM_DUCKSHOOT:
			case ANIM_SHOOT:
			case ANIM_WATER_SHOOT:
			case ANIM_HEADLESS:
			case ANIM_HEADLESSLOOP:
				if (hl == hl_TorsoFront && irand(0, 10) < 4)
					hl = hl_ArmLowerRight;
				break;

			case ANIM_MELEE:
			case ANIM_MELEE_STAND:
				if (hl == hl_TorsoFront && irand(0, 10) < 4)
					hl = hl_ArmLowerLeft;
				break;

			default:
				break;
		}

		if (irand(0, 10) < 4 &&
			((hl == hl_ArmUpperLeft && self->s.fmnodeinfo[MESH__LEFTARM].flags & FMNI_NO_DRAW) || (hl == hl_ArmUpperRight && self->s.fmnodeinfo[MESH__RIGHTARM].flags & FMNI_NO_DRAW) ||
				((hl == hl_TorsoFront || hl == hl_TorsoBack) && (self->s.fmnodeinfo[MESH__RIGHTARM].flags & FMNI_NO_DRAW) && (self->s.fmnodeinfo[MESH__LEFTARM].flags & FMNI_NO_DRAW))))
		{
			hl = hl_Head; // Decapitation.
		}
	}
	else
	{
		hl = SsithraConvertDeadHitLocation(self, hl);
	}

	int throw_nodes = 0;

	switch (hl)
	{
		case hl_Head:
			if (SsithraThrowHead(self, (float)damage, dismember_ok, &throw_nodes))
				return;
			break;

		case hl_TorsoFront: // Split in half?
		case hl_TorsoBack:
			SsithraThrowTorso(self, (float)damage, dismember_ok, &throw_nodes);
			break;

		case hl_ArmUpperLeft: // Left arm.
		case hl_ArmLowerLeft:
			SsithraThrowArm(self, (float)damage, dismember_ok, &throw_nodes, MESH__LEFTARM);
			break;

		case hl_ArmUpperRight: // Right arm.
		case hl_ArmLowerRight:
			SsithraThrowArm(self, (float)damage, dismember_ok, &throw_nodes, MESH__RIGHTARM);
			break;

		case hl_LegUpperLeft: // Left leg.
		case hl_LegLowerLeft:
			SsithraThrowLeg(self, (float)damage, &throw_nodes, MESH__LEFTLEG);
			break;

		case hl_LegUpperRight: // Right leg.
		case hl_LegLowerRight:
			SsithraThrowLeg(self, (float)damage, &throw_nodes, MESH__RIGHTLEG);
			break;

		default:
			break;
	}

	if (throw_nodes > 0)
		self->pain_debounce_time = 0.0f;

	if ((self->s.fmnodeinfo[MESH__LEFTARM].flags & FMNI_NO_DRAW) && (self->s.fmnodeinfo[MESH__RIGHTARM].flags & FMNI_NO_DRAW))
	{
		self->monsterinfo.aiflags |= AI_COWARD;
		self->ai_mood_flags &= ~AI_MOOD_FLAG_BACKSTAB;
		self->spawnflags &= ~MSF_FIXED;
	}
	else
	{
		if (self->s.fmnodeinfo[MESH__LEFTARM].flags & FMNI_NO_DRAW)
		{
			self->monsterinfo.aiflags |= AI_NO_MELEE;
			self->ai_mood_flags &= ~AI_MOOD_FLAG_BACKSTAB;
		}

		if (self->s.fmnodeinfo[MESH__RIGHTARM].flags & FMNI_NO_DRAW)
		{
			self->monsterinfo.aiflags |= AI_NO_MISSILE;
			self->ai_mood_flags &= ~AI_MOOD_FLAG_BACKSTAB;
			self->spawnflags &= ~MSF_FIXED;
		}
	}
}

static void SsithraArrowTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surface) //mxd. Named 'ssithraArrowTouch' in original logic.
{
	if (surface != NULL && (surface->flags & SURF_SKY))
	{
		SkyFly(self);
		return;
	}

	if (EntReflecting(other, true, true))
	{
		edict_t* arrow = G_Spawn();

		ReflectedSsithraArrowInit(self, arrow);
		gi.CreateEffect(NULL, FX_M_EFFECTS, CEF_FLAG6, self->s.origin, "bv", FX_MSSITHRA_EXPLODE, self->movedir);
		G_SetToFree(self);

		return;
	}

	if (other->takedamage != DAMAGE_NO)
	{
		vec3_t normal;

		if (plane != NULL)
			VectorCopy(plane->normal, normal);
		else
			VectorCopy(vec3_up, normal);

		const int damage = irand(SSITHRA_DMG_MIN, SSITHRA_DMG_MAX); //mxd. flrand() in original logic.
		T_Damage(other, self, self->owner, self->movedir, self->s.origin, normal, damage, 0, 0, MOD_DIED);
	}

	gi.CreateEffect(NULL, FX_M_EFFECTS, CEF_FLAG6, self->s.origin, "bv", FX_MSSITHRA_EXPLODE, self->movedir);
	VectorClear(self->velocity);
	G_FreeEdict(self);
}

static void SsithraArrowExplodeThink(edict_t* self) //mxd. Named 'ssithraArrowExplode' in original logic.
{
	gi.CreateEffect(NULL, FX_M_EFFECTS, 0, self->s.origin, "bv", FX_MSSITHRA_EXPLODE, self->movedir);

	const int damage = irand(SSITHRA_BIGARROW_DMG_MIN, SSITHRA_BIGARROW_DMG_MAX);
	T_DamageRadius(self, self->owner, self->owner, 64.0f, (float)damage, (float)damage / 2.0f, DAMAGE_ATTACKER_IMMUNE, MOD_DIED);

	G_FreeEdict(self);
}

static void SsithraDuckArrowTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surface) //mxd. Named 'ssithraDuckArrowTouch' in original logic.
{
	if (surface != NULL && (surface->flags & SURF_SKY))
	{
		SkyFly(self);
		return;
	}

	if (plane != NULL) //mxd. Original logic does plane->normal NULL check here (always true).
		VectorCopy(plane->normal, self->movedir);

	//NOTENOTE: NO REFLECTION FOR THIS MISSILE!
	if (other->takedamage != DAMAGE_NO)
	{
		self->dmg = irand(SSITHRA_DMG_MIN * 2, SSITHRA_DMG_MAX * 2);
		SsithraArrowExplodeThink(self);
	}
	else
	{
		VectorClear(self->velocity);

		self->s.effects |= EF_MARCUS_FLAG1;
		self->dmg = irand(SSITHRA_DMG_MIN, SSITHRA_DMG_MAX);

		self->think = SsithraArrowExplodeThink;
		self->nextthink = level.time + flrand(0.5f, 1.5f);
	}
}

#pragma endregion

#pragma region ========================== Action functions ==========================

void ssithra_decide_run(edict_t* self) //mxd. Named 'ssithra_decide_gallop' in original logic.
{
	if (self->spawnflags & MSF_FIXED)
	{
		SetAnim(self, ANIM_DELAY);
		return;
	}

	VectorClear(self->velocity);
	self->ssithra_watersplash_spawned = false;

	SetAnim(self, (SsithraCheckInWater(self) ? ANIM_SWIMFORWARD : ANIM_RUN1));
	ssithra_check_mood(self);
}

void ssithra_decide_swimforward(edict_t* self)
{
	//FIXME: climb out of water check!
	self->ssithra_watersplash_spawned = false;
	VectorClear(self->velocity);

	if (!SsithraCheckInWater(self))
		SetAnim(self, ANIM_RUN1); // Not actually in water!
	else if (self->curAnimID == ANIM_WATER_SHOOT)
		SetAnim(self, ANIM_TRANSDOWN);

	ssithra_check_mood(self);
}

void ssithra_check_ripple(edict_t* self) //mxd. Named 'ssithraCheckRipple' in original logic.
{
	// No ripples while in cinematics.
	if (SV_CINEMATICFREEZE)
		return;

	vec3_t top;
	VectorCopy(self->s.origin, top);
	top[2] += self->maxs[2] * 0.75f;

	vec3_t bottom;
	VectorCopy(self->s.origin, bottom);
	bottom[2] += self->mins[2];

	trace_t trace;
	gi.trace(top, vec3_origin, vec3_origin, bottom, self, MASK_WATER, &trace);

	if (trace.fraction < 1.0f)
	{
		vec3_t forward;
		AngleVectors(self->s.angles, forward, NULL, NULL);
		Vec3ScaleAssign(200.0f, forward);

		const byte b_angle = (byte)((self->s.angles[YAW] + DEGREE_180) / 360.0f * 255.0f);
		gi.CreateEffect(NULL, FX_WATER_WAKE, 0, trace.endpos, "sbv", self->s.number, b_angle, forward);
	}
}

void ssithra_ai_run(edict_t* self, float distance) //mxd. Originally defined in m_plagueSsithra_anim.c.
{
	if (SsithraCheckInWater(self))
	{
		MG_SwimFlyToGoal(self, distance); // Really need to get rid of this!
		MG_Pathfind(self, false);
	}
	else
	{
		MG_AI_Run(self, distance);
	}
}

void ssithra_set_view_angle_offsets(edict_t* self, float pitch_offset, float yaw_offset, float roll_offset) //mxd. Named 'ssithraVOfs' in original logic.
{
	VectorSet(self->v_angle_ofs, pitch_offset, yaw_offset, roll_offset);
}

void ssithra_try_out_of_water_jump(edict_t* self) //mxd. Named 'ssithra_check_namor' in original logic.
{
	//FIXME: climb out of water check!
	vec3_t target_origin;

	if ((self->spawnflags & MSF_FIXED) || !MG_TryGetTargetOrigin(self, target_origin))
		return;

	if (!(gi.pointcontents(target_origin) & CONTENTS_WATER) && MG_IsVisiblePos(self, target_origin) && SsithraHaveWaterLedgeNearEnemy(self))
		SetAnim(self, ANIM_FACEANDNAMOR);
}

void ssithra_check_bound(edict_t* self) //mxd. Named 'ssithraBoundCheck' in original logic.
{
	//FIXME: do checks and traces first.
	if (self->spawnflags & MSF_FIXED)
		return;

	if (SsithraCheckInWater(self))
	{
		if (self->curAnimID != ANIM_SWIMFORWARD)
			SetAnim(self, ANIM_SWIMFORWARD);

		return;
	}

	vec3_t forward;
	vec3_t up;
	AngleVectors(self->s.angles, forward, NULL, up);

	vec3_t start_pos;
	VectorCopy(self->s.origin, start_pos);

	vec3_t end_pos;
	VectorMA(start_pos, 48.0f, forward, end_pos); // Forward.

	trace_t trace;
	gi.trace(start_pos, self->mins, self->maxs, end_pos, self, MASK_SOLID, &trace);

	VectorCopy(trace.endpos, start_pos);
	VectorMA(start_pos, -128.0f, up, end_pos); // Down.

	gi.trace(start_pos, self->mins, self->maxs, end_pos, self, MASK_SOLID | MASK_WATER, &trace);

	// If it's a step down or less, no jump.
	if (Q_fabs(trace.endpos[2] - self->s.origin[2]) <= 18.0f)
		return;

	if (trace.fraction == 1.0f || trace.allsolid || trace.startsolid)
		return; // Too far to jump down, or in solid.

	if (trace.contents & (CONTENTS_WATER | CONTENTS_SLIME))
	{
		VectorCopy(trace.endpos, start_pos);
		VectorMA(start_pos, -64.0f, up, end_pos); // Down from water surface.

		const vec3_t mins = { self->mins[0], self->mins[1], 0.0f };
		const vec3_t maxs = { self->maxs[0], self->maxs[1], 1.0f };

		gi.trace(start_pos, mins, maxs, end_pos, self, MASK_SOLID, &trace);

		if (trace.fraction < 1.0f || trace.allsolid || trace.startsolid)
			SsithraTryJump(self);
		else
			SetAnim(self, ANIM_DIVE);
	}
	else
	{
		SetAnim(self, ANIM_GALLOP);
	}
}

void ssithra_check_dive(edict_t* self) //mxd. Named 'ssithraDiveCheck' in original logic.
{
	//FIXME: do checks and traces first.
	if (self->spawnflags & MSF_FIXED)
		return;

	vec3_t target_origin;
	vec3_t targ_mins;

	if (self->monsterinfo.searchType == SEARCH_BUOY)
	{
		if (self->buoy_index < 0 || self->buoy_index > level.active_buoys)
			return;

		VectorCopy(level.buoy_list[self->buoy_index].origin, target_origin);
		VectorClear(targ_mins);
	}
	else
	{
		if (self->goalentity == NULL)
			return;

		VectorCopy(self->goalentity->s.origin, target_origin);
		VectorCopy(self->goalentity->mins, targ_mins);
	}

	if (SsithraCheckInWater(self))
	{
		SetAnim(self, ANIM_SWIMFORWARD);
		return;
	}

	if (!MG_IsInforntPos(self, target_origin))
		return;

	// Make sure the enemy isn't right here and accessible before diving in.
	if (vhlen(target_origin, self->s.origin) < 96.0f && // Close enough?
		Q_fabs((target_origin[2] + targ_mins[2]) - (self->s.origin[2] + self->mins[2])) < 18.0f && // Relatively same stepheight.
		!(gi.pointcontents(target_origin) & CONTENTS_WATER) && !(self->monsterinfo.aiflags & AI_FLEE))
	{
		return;
	}

	vec3_t forward;
	vec3_t up;
	AngleVectors(self->s.angles, forward, NULL, up);

	vec3_t start_pos;
	VectorCopy(self->s.origin, start_pos);

	vec3_t end_pos;
	VectorMA(start_pos, 48.0f, forward, end_pos); // Forward.

	trace_t trace;
	gi.trace(start_pos, self->mins, self->maxs, end_pos, self, MASK_SOLID, &trace);

	VectorCopy(trace.endpos, start_pos);
	VectorMA(start_pos, -128.0f, up, end_pos); // Down.

	gi.trace(start_pos, self->mins, self->maxs, end_pos, self, MASK_SOLID | MASK_WATER, &trace);

	if (trace.fraction == 1.0f || trace.allsolid || trace.startsolid)
		return; // Too far to jump down, or in solid.

	if (trace.contents & (CONTENTS_WATER | CONTENTS_SLIME))
	{
		VectorCopy(trace.endpos, start_pos);
		VectorMA(start_pos, -64.0f, up, end_pos); // Down from water surface.

		const vec3_t mins = { self->mins[0], self->mins[1], 0.0f };
		const vec3_t maxs = { self->maxs[0], self->maxs[1], 1.0f };

		gi.trace(start_pos, mins, maxs, end_pos, self, MASK_SOLID, &trace);

		if (trace.fraction < 1.0f || trace.allsolid || trace.startsolid)
			SsithraTryJump(self);
		else
			SetAnim(self, ANIM_DIVE);
	}
}

void ssithra_apply_jump(edict_t* self) //mxd. Named 'ssithraApplyJump' in original logic.
{
	if (self->spawnflags & MSF_FIXED)
		return;

	self->jump_time = level.time + 1.0f;
	VectorCopy(self->movedir, self->velocity);
	VectorNormalize(self->movedir);
}

void ssithra_jump(edict_t* self, float up_speed, float forward_speed, float right_speed) //mxd. Named 'ssithraJump' in original logic.
{
	//FIXME: do checks and traces first.
	if (self->spawnflags & MSF_FIXED)
		return;

	if ((self->s.fmnodeinfo[MESH__LEFTLEG].flags & FMNI_NO_DRAW) || (self->s.fmnodeinfo[MESH__RIGHTLEG].flags & FMNI_NO_DRAW))
	{
		up_speed *= 2.0f;
		forward_speed /= 2.0f;
	}

	vec3_t forward;
	vec3_t right;
	vec3_t up;
	AngleVectors(self->s.angles, forward, right, up);

	VectorMA(self->velocity, up_speed, up, self->velocity);
	VectorMA(self->velocity, forward_speed, forward, self->velocity);
	VectorMA(self->velocity, right_speed, right, self->velocity);
}

void ssithra_out_of_water_jump(edict_t* self) //mxd. Named 'ssithraNamorJump' in original logic.
{
	vec3_t target_origin;

	if ((self->spawnflags & MSF_FIXED) || !MG_TryGetTargetOrigin(self, target_origin))
		return;

	//FIXME: jumps too high sometimes?
	self->ssithra_watersplash_spawned = false;

	vec3_t top;
	VectorCopy(self->s.origin, top);
	top[2] += 512.0f;

	trace_t trace;
	gi.trace(self->s.origin, vec3_origin, vec3_origin, top, self, MASK_SOLID, &trace);

	VectorCopy(trace.endpos, top);
	gi.trace(top, vec3_origin, vec3_origin, self->s.origin, self, MASK_SOLID | MASK_WATER, &trace);

	// How far above my feet is waterlevel?
	vec3_t diff;
	VectorSubtract(trace.endpos, self->s.origin, diff);
	const float watersurf_zdist = VectorLength(diff) - self->mins[2]; // Adjust for my feet.

	// How high above water level is player?
	const float enemy_zdiff = target_origin[2] - trace.endpos[2];

	//FIXME: aim a little to side if enemy close so don't land on top of him? Or hit him if land on top?
	ssithra_jump(self, (watersurf_zdist + enemy_zdiff) * 2.0f + 200.0f, 100.0f, 0.0f);
}

// Simple addition of velocity, if on ground or not.
void ssithra_set_forward_velocity(edict_t* self, float forward_dist) //mxd. Named 'ssithraForward' in original logic.
{
	SsithraCheckInWater(self);

	if (self->groundentity != NULL) // On ground.
	{
		VectorClear(self->velocity);
	}
	else // In air (or water?).
	{
		vec3_t forward;
		AngleVectors(self->s.angles, forward, NULL, NULL);

		Vec3ScaleAssign(forward_dist, forward);
		forward[2] = self->velocity[2];

		VectorCopy(forward, self->velocity);
	}
}

void ssithra_try_spawn_water_exit_splash(edict_t* self) //mxd. Named 'ssithraCheckLeaveWaterSplash' in original logic.
{
	if (self->ssithra_watersplash_spawned || SsithraCheckInWater(self))
		return;

	vec3_t dir;
	VectorCopy(self->velocity, dir);
	VectorNormalize(dir);

	vec3_t end_pos;
	VectorMA(self->s.origin, -256.0f, dir, end_pos);

	trace_t trace;
	gi.trace(self->s.origin, vec3_origin, vec3_origin, end_pos, self, MASK_WATER, &trace);

	if (trace.fraction < 1.0f)
	{
		gi.sound(self, CHAN_BODY, sounds[SND_NAMOR], 1.0f, ATTN_NORM, 0.0f);

		// FIXME: Size proportional to exit velocity.
		const vec3_t fx_dir = { 0.0f, 0.0f, 300.0f }; //TODO: normalized in ssithra_try_spawn_water_entry_splash(). Which is correct?
		gi.CreateEffect(NULL, FX_WATER_ENTRYSPLASH, 0, trace.endpos, "bd", 128 | 96, fx_dir);

		self->ssithra_watersplash_spawned = true;
	}
}

void ssithra_try_spawn_water_entry_splash(edict_t* self) //mxd. Named 'ssithraCheckHitWaterSplash' in original logic.
{
	if (self->ssithra_watersplash_spawned)
		return;

	if (Q_fabs(self->velocity[0]) + Q_fabs(self->velocity[1]) < 200.0f)
	{
		vec3_t end_pos;
		VectorCopy(self->s.origin, end_pos);
		end_pos[2] -= 128.0f;

		trace_t trace;
		gi.trace(self->s.origin, self->mins, self->maxs, end_pos, self, MASK_ALL, &trace);

		if (trace.fraction < 1.0f && !trace.allsolid && !trace.startsolid && !(trace.contents & CONTENTS_WATER) && !(trace.contents & CONTENTS_SLIME))
		{
			// Not going to hit water!
			SetAnim(self, ANIM_BOUND);
			return;
		}
	}

	if (self->flags & FL_INWATER)
	{
		vec3_t dir;
		VectorNormalize2(self->velocity, dir);

		vec3_t end_pos;
		VectorMA(self->s.origin, -256.0f, dir, end_pos);

		trace_t trace;
		gi.trace(self->s.origin, vec3_origin, vec3_origin, end_pos, self, MASK_WATER, &trace);
		gi.trace(trace.endpos, vec3_origin, vec3_origin, self->s.origin, self, MASK_WATER, &trace);

		if (trace.fraction < 1.0f)
		{
			gi.sound(self, CHAN_BODY, sounds[SND_INWATER], 1.0f, ATTN_NORM, 0.0f);
			gi.sound(self, CHAN_BODY, gi.soundindex("player/Water Enter.wav"), 1.0f, ATTN_NORM, 0.0f); //TODO: why 2 similar sounds? Use SND_INWATER only?

			vec3_t fx_dir = { 0.0f, 0.0f, self->velocity[2] };
			VectorNormalize(fx_dir);

			// FIXME: Size proportional to entry velocity.
			gi.CreateEffect(NULL, FX_WATER_ENTRYSPLASH, CEF_FLAG7, trace.endpos, "bd", 128 | 96, fx_dir);

			self->ssithra_watersplash_spawned = true;
		}
	}
}

void ssithra_check_faced_out_of_water_jump(edict_t* self) //mxd. Named 'ssithraCheckFacedNamor' in original logic.
{
	if (!(self->spawnflags & MSF_FIXED) && Q_fabs(self->ideal_yaw - self->s.angles[YAW]) < self->yaw_speed)
		SetAnim(self, ANIM_NAMOR);
}

void ssithra_pain_react(edict_t* self)
{
	if (self->enemy == NULL)
	{
		SsithraDecideStand(self);
	}
	else if (self->enemy->health <= 0 || self->enemy == self || self->enemy->takedamage == DAMAGE_NO)
	{
		self->enemy = NULL;
		SsithraDecideStand(self);
	}
	else
	{
		// Go get him!
		ssithra_decide_run(self);
	}
}

void ssithra_dead(edict_t* self)
{
	//FIXME: maybe allow dead bodies to be chopped? Make BBOX small?
	self->dead_state = DEAD_DEAD;
	self->msgHandler = DeadMsgHandler;
	self->svflags |= SVF_DEADMONSTER; // Now treat as a different content type.
	self->flags |= FL_DONTANIMATE;

	M_EndDeath(self);
}

void ssithra_water_dead(edict_t* self) //mxd. Named 'ssithraWaterDead' in original logic.
{
	self->dead_state = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;

	self->think = M_DeadFloatThink; //mxd. fish_deadfloat() in original logic.
	self->nextthink = level.time + FRAMETIME; //mxd. Use define.

	gi.linkentity(self);
}

void ssithra_collapse(edict_t* self) //mxd. Named 'ssithraCollapse' in original logic.
{
	if (irand(0, 10) < 5)
	{
		SetAnim(self, ANIM_HEADLESSLOOP);
		self->msgHandler = DyingMsgHandler;
	}
	else
	{
		self->svflags &= ~SVF_DEADMONSTER; // Now treat as a different content type.
		self->msgHandler = DefaultMsgHandler;

		vec3_t gore_spot;
		VectorCopy(self->s.origin, gore_spot);
		gore_spot[2] += self->maxs[2] * 0.75f;

		self->health = 1;
		T_Damage(self, self, self, vec3_origin, gore_spot, vec3_origin, 10, 20, 0, MOD_DIED);
		self->health = -33;
	}
}

void ssithra_kill_self(edict_t* self) //mxd. Named 'ssithraKillSelf' in original logic.
{
	self->svflags &= ~SVF_DEADMONSTER; // Now treat as a different content type.
	self->msgHandler = DefaultMsgHandler;
	self->dead_state = DEAD_NO;

	vec3_t gore_spot;
	VectorCopy(self->s.origin, gore_spot);
	gore_spot[2] += 12.0f;

	self->health = 1;
	T_Damage(self, self, self, vec3_origin, gore_spot, vec3_origin, 10, 20, 0, MOD_DIED);
	self->health = -69;
}

void ssithra_sound(edict_t* self, float sound_num, float channel, float attenuation) //mxd. Named 'ssithraSound' in original logic.
{
	if ((int)attenuation == 0) //TODO: use ATTN_NORM in animframe_t instead.
		attenuation = ATTN_NORM;
	else if ((int)attenuation == -1) //TODO: use ATTN_NONE in animframe_t instead.
		attenuation = ATTN_NONE;

	if ((int)sound_num == SND_SWIM && irand(0, 10) < 5)
		sound_num = SND_SWIM2;

	gi.sound(self, (int)channel, sounds[(int)sound_num], 1.0f, attenuation, 0.0f);
}

void ssithra_growl_sound(edict_t* self) //mxd. Named 'ssithraGrowlSound' in original logic.
{
	if (irand(0, 3) == 0)
		gi.sound(self, CHAN_VOICE, sounds[irand(SND_GROWL1, SND_GROWL3)], 1.0f, ATTN_IDLE, 0.0f);
}

void ssithra_swipe(edict_t* self) //mxd. Named 'ssithraSwipe' in original logic.
{
	// Use melee swipe function in g_monster.
	const vec3_t start_offset = { 16.0f, -16.0f, 24.0f };
	const vec3_t end_offset = { 50.0f, 0.0f, -8.0f };

	const vec3_t mins = { -2.0f, -2.0f, -2.0f };
	const vec3_t maxs = {  2.0f,  2.0f,  2.0f };

	trace_t trace;
	vec3_t direction;
	edict_t* victim = M_CheckMeleeLineHit(self, start_offset, end_offset, mins, maxs, &trace, direction);

	if (victim == NULL)
		return; //TODO: play swoosh sound?

	if (victim != self)
	{
		// Hurt whatever we were whacking away at.
		gi.sound(self, CHAN_WEAPON, sounds[SND_SWIPEHIT], 1.0f, ATTN_NORM, 0.0f);

		vec3_t blood_dir;
		VectorSubtract(start_offset, end_offset, blood_dir);
		VectorNormalize(blood_dir);

		const qboolean ssithra_alpha = (self->spawnflags & MSF_SSITHRA_ALPHA);
		const int damage = (int)(flrand(SSITHRA_DMG_MIN, SSITHRA_DMG_MAX) * (ssithra_alpha ? 1.2f : 1.0f));
		const int knockback = (ssithra_alpha ? 10 : 0);

		T_Damage(victim, self, self, direction, trace.endpos, blood_dir, damage, knockback, 0, MOD_DIED);
	}
}

void ssithra_start_duck_arrow(edict_t* self) //mxd. Named 'ssithraStartDuckArrow' in original logic.
{
	vec3_t forward;
	vec3_t right;
	AngleVectors(self->s.angles, forward, right, NULL);

	vec3_t start_pos;
	VectorMA(self->s.origin, self->s.scale * 18.0f, forward, start_pos);
	VectorMA(start_pos, self->s.scale * 4.0f, right, start_pos);

	gi.sound(self, CHAN_WEAPON, sounds[SND_ARROW_CHARGE], 1.0f, ATTN_NORM, 0.0f);
	gi.CreateEffect(NULL, FX_M_EFFECTS, 0, self->s.origin, "bv", FX_MSSITHRA_ARROW_CHARGE, start_pos);
}

void ssithra_arrow(edict_t* self) //mxd. Named 'ssithraArrow' in original logic.
{
	//FIXME: adjust for up/down.
	if (self->enemy == NULL)
	{
		SsithraDecideStand(self);
		return;
	}

	if (self->enemy->health <= 0)
	{
		self->enemy = NULL;
		SsithraDecideStand(self);

		return;
	}

	if (self->monsterinfo.attack_finished > level.time)
		return;

	if (self->spawnflags & MSF_SSITHRA_ALPHA)
		SsithraDoDuckArrow(self, self->maxs[2] * 0.8f);
	else
		SsithraDoArrow(self);
}

void ssithra_panic_arrow(edict_t* self) //mxd. Named 'ssithraPanicArrow' in original logic.
{
	//FIXME: adjust for up/down.
	if (self->s.fmnodeinfo[MESH__RIGHTARM].flags & FMNI_NO_DRAW)
	{
		if (self->curAnimID == ANIM_HEADLESS || self->curAnimID == ANIM_HEADLESSLOOP)
			ssithra_kill_self(self);

		return;
	}

	gi.sound(self, CHAN_WEAPON, sounds[SND_ARROW2], 1.0f, ATTN_NORM, 0.0f);
	self->monsterinfo.attack_finished = level.time + 0.4f;

	edict_t* arrow = G_Spawn();

	SsithraArrowInit(arrow);

	arrow->owner = self;

	vec3_t fire_dir;
	VectorAdd(self->s.angles, self->v_angle_ofs, fire_dir);

	vec3_t forward;
	AngleVectors(fire_dir, forward, NULL, NULL);
	VectorCopy(self->s.origin, arrow->s.origin);

	VectorMA(arrow->s.origin, 12.0f, forward, arrow->s.origin);
	VectorCopy(self->movedir, arrow->movedir);

	VectorScale(forward, SSITHRA_SPOO_SPEED, arrow->velocity);

	vectoangles(arrow->velocity, arrow->s.angles);
	arrow->s.angles[YAW] += 90.0f;

	//FIXME: redo these - make them look like squid ink?
	gi.CreateEffect(&arrow->s, FX_SSITHRA_ARROW, CEF_OWNERS_ORIGIN, NULL, "bv", FX_SS_MAKE_ARROW, arrow->velocity);

	G_LinkMissile(arrow);

	arrow->think = G_FreeEdict;
	arrow->nextthink = level.time + 3.0f;
}

void ssithra_water_shoot(edict_t* self)
{
	SetAnim(self, ANIM_WATER_SHOOT);
}

void ssithra_check_loop(edict_t* self) //mxd. Named 'ssithraCheckLoop' in original logic.
{
#define SSITHRA_MELEE_RANGE	64.0f
#define SSITHRA_JUMP_RANGE	128.0f

	// See if should fire again.
	if (self->enemy == NULL || !AI_IsVisible(self, self->enemy) || !AI_IsInfrontOf(self, self->enemy) || irand(0, 100) < self->bypass_missile_chance)
		return;

	vec3_t diff;
	VectorSubtract(self->enemy->s.origin, self->s.origin, diff);
	const float dist = VectorLength(diff);

	const float min_seperation = self->maxs[0] + self->enemy->maxs[0];

	// Don't loop if enemy is close enough.
	if (dist < min_seperation + SSITHRA_MELEE_RANGE)
	{
		QPostMessage(self, MSG_MELEE, PRI_DIRECTIVE, NULL);
		return;
	}

	if (dist < min_seperation + SSITHRA_JUMP_RANGE && irand(0, 10) < 3)
	{
		VectorScale(diff, 3.0f, self->movedir);
		self->movedir[2] += 150.0f;
		SetAnim(self, ANIM_LUNGE);

		return;
	}

	self->monsterinfo.currframeindex -= 2;
}

void ssithra_check_duck_arrow(edict_t* self) //mxd. Named 'ssithraCheckDuckArrow' in original logic.
{
	if (M_ValidTarget(self, self->enemy) && MG_IsAheadOf(self, self->enemy))
		SsithraDoDuckArrow(self, -18.0f);
}

void ssithra_check_unduck(edict_t* self) //mxd. Named 'ssithraCheckUnDuck' in original logic.
{
	SetAnim(self, ((self->evade_debounce_time < level.time) ? ANIM_UNDUCK : ANIM_DUCKLOOP));
}

void ssithra_crouch(edict_t* self) //mxd. Named 'ssithraCrouch' in original logic.
{
	self->maxs[2] = 0.0f;
	self->viewheight = -6;
	gi.linkentity(self);

	SetAnim(self, ANIM_DUCKSHOOT);
}

void ssithra_uncrouch(edict_t* self) //mxd. Named 'ssithraUnCrouch' in original logic.
{
	self->maxs[2] = STDMaxsForClass[self->classID][2] * self->s.scale;
	self->viewheight = (int)(self->maxs[2] * 0.8f);
	gi.linkentity(self);
}

void ssithra_check_mood(edict_t* self) //mxd. Named 'SsithraCheckMood', returned qboolean in original logic.
{
	if ((self->spawnflags & MSF_FIXED) && self->curAnimID == ANIM_DELAY && self->enemy != NULL)
	{
		self->monsterinfo.searchType = SEARCH_COMMON;
		MG_FaceGoal(self, true);
	}

	self->mood_think(self);

	switch (self->ai_mood)
	{
		case AI_MOOD_ATTACK:
			if (self->ai_mood_flags & AI_MOOD_FLAG_MISSILE)
				QPostMessage(self, MSG_MISSILE, PRI_DIRECTIVE, NULL);
			else
				QPostMessage(self, MSG_MELEE, PRI_DIRECTIVE, NULL);
			break;

		case AI_MOOD_PURSUE:
		case AI_MOOD_NAVIGATE:
			QPostMessage(self, MSG_RUN, PRI_DIRECTIVE, NULL);
			break;

		case AI_MOOD_WALK:
			QPostMessage(self, MSG_WALK, PRI_DIRECTIVE, NULL);
			break;

		case AI_MOOD_STAND:
			SsithraDecideStand(self);
			break;

		case AI_MOOD_DELAY:
			SetAnim(self, ANIM_DELAY);
			break;

		case AI_MOOD_WANDER:
			SetAnim(self, (SsithraCheckInWater(self) ? ANIM_SWIMWANDER : ANIM_WALK1));
			break;

		case AI_MOOD_BACKUP:
			QPostMessage(self, MSG_FALLBACK, PRI_DIRECTIVE, NULL);
			break;

		case AI_MOOD_JUMP:
			SetAnim(self, ((self->spawnflags & MSF_FIXED) ? ANIM_DELAY : ANIM_FJUMP));
			break;

		default:
			break;
	}
}

#pragma endregion

void SsithraStaticsInit(void)
{
	static ClassResourceInfo_t res_info; //mxd. Made local static.

	classStatics[CID_SSITHRA].msgReceivers[MSG_STAND] = SsithraStandMsgHandler;
	classStatics[CID_SSITHRA].msgReceivers[MSG_WALK] = SsithraWalkMsgHandler;
	classStatics[CID_SSITHRA].msgReceivers[MSG_RUN] = SsithraRunMsgHandler;
	classStatics[CID_SSITHRA].msgReceivers[MSG_MELEE] = SsithraMeleeMsgHandler;
	classStatics[CID_SSITHRA].msgReceivers[MSG_MISSILE] = SsithraMissileMsgHandler;
	classStatics[CID_SSITHRA].msgReceivers[MSG_WATCH] = SsithraWatchMsgHandler;
	classStatics[CID_SSITHRA].msgReceivers[MSG_PAIN] = SsithraPainMsgHandler;
	classStatics[CID_SSITHRA].msgReceivers[MSG_DEATH] = SsithraDeathMsgHandler;
	classStatics[CID_SSITHRA].msgReceivers[MSG_DISMEMBER] = DismemberMsgHandler;
	classStatics[CID_SSITHRA].msgReceivers[MSG_JUMP] = SsithraJumpMsgHandler;
	classStatics[CID_SSITHRA].msgReceivers[MSG_FALLBACK] = SsithraFallbackMsgHandler;
	classStatics[CID_SSITHRA].msgReceivers[MSG_DEATH_PAIN] = SsithraDeathPainMsgHandler;
	classStatics[CID_SSITHRA].msgReceivers[MSG_EVADE] = SsithraEvadeMsgHandler;
	classStatics[CID_SSITHRA].msgReceivers[MSG_CHECK_MOOD] = SsithraCheckMoodMsgHandler;
	classStatics[CID_SSITHRA].msgReceivers[MSG_VOICE_SIGHT] = SsithraVoiceSightMsgHandler;

	res_info.numAnims = NUM_ANIMS;
	res_info.animations = animations;
	res_info.modelIndex = gi.modelindex("models/monsters/ssithra/tris.fm");

	sounds[SND_PAIN1] = gi.soundindex("monsters/pssithra/pain1.wav");
	sounds[SND_PAIN2] = gi.soundindex("monsters/pssithra/pain2.wav");
	sounds[SND_DIE] = gi.soundindex("monsters/pssithra/die.wav");
	sounds[SND_GIB] = gi.soundindex("monsters/pssithra/gib.wav");
	sounds[SND_SWIPEHIT] = gi.soundindex("monsters/pssithra/swipehit.wav");
	sounds[SND_ARROW1] = gi.soundindex("monsters/pssithra/arrow1.wav");
	sounds[SND_ARROW2] = gi.soundindex("monsters/pssithra/arrow2.wav");
	sounds[SND_GROWL1] = gi.soundindex("monsters/pssithra/growl1.wav");
	sounds[SND_GROWL2] = gi.soundindex("monsters/pssithra/growl2.wav");
	sounds[SND_GROWL3] = gi.soundindex("monsters/pssithra/growl3.wav");
	sounds[SND_INWATER] = gi.soundindex("monsters/pssithra/inwater.wav");
	sounds[SND_NAMOR] = gi.soundindex("monsters/pssithra/namor.wav");
	sounds[SND_LAND] = gi.soundindex("monsters/pssithra/land.wav");
	sounds[SND_SWIPE] = gi.soundindex("monsters/pssithra/swipe.wav");
	sounds[SND_SWIM] = gi.soundindex("monsters/pssithra/swim.wav");
	sounds[SND_SWIM2] = gi.soundindex("monsters/pssithra/swim2.wav");

	sounds[SND_SIGHT1] = gi.soundindex("monsters/pssithra/ssithvoice1.wav");
	sounds[SND_SIGHT2] = gi.soundindex("monsters/pssithra/ssithvoice2.wav");
	sounds[SND_SIGHT3] = gi.soundindex("monsters/pssithra/ssithvoice3.wav");
	sounds[SND_SIGHT4] = gi.soundindex("monsters/pssithra/ssithvoice4.wav");
	sounds[SND_SIGHT5] = gi.soundindex("monsters/pssithra/ssithvoice5.wav");
	sounds[SND_SIGHT6] = gi.soundindex("monsters/pssithra/ssithvoice6.wav");

	sounds[SND_ARROW_CHARGE] = gi.soundindex("monsters/pssithra/guncharge.wav");
	sounds[SND_ARROW_FIRE] = gi.soundindex("monsters/pssithra/gunfire.wav");

	res_info.numSounds = NUM_SOUNDS;
	res_info.sounds = sounds;

	classStatics[CID_SSITHRA].resInfo = &res_info;
}

// QUAKED monster_ssithra (1 .5 0) (-16 -16 -32) (16 16 26) AMBUSH ASLEEP 4 NAMOR Spin ToughGuy Clothed FIXED WANDER MELEE_LEAD STALK COWARD EXTRA1 EXTRA2 EXTRA3 EXTRA4
// The plague ssithra.

// Spawnflags:
// AMBUSH		- Will not be woken up by other monsters or shots from player.
// ASLEEP		- Will not appear until triggered.
// WALKING		- Use WANDER instead.
// WANDER		- Monster will wander around aimlessly (but follows buoys).
// MELEE_LEAD	- Monster will try to cut you off when you're running and fighting him, works well if there are a few monsters in a group, half doing this, half not.
// STALK		- Monster will only approach and attack from behind. If you're facing the monster it will just stand there.
//				  Once the monster takes pain, however, it will stop this behaviour and attack normally.
// COWARD		- Monster starts off in flee mode (runs away from you when woken up).

// Variables:
// homebuoy					- Monsters will head to this buoy if they don't have an enemy ("homebuoy" should be targetname of the buoy you want them to go to).
// wakeup_target			- Monsters will fire this target the first time it wakes up (only once).
// pain_target				- Monsters will fire this target the first time it gets hurt (only once).
// mintel					- Monster intelligence - this basically tells a monster how many buoys away an enemy has to be for it to give up (default 28).
// melee_range				- How close the player has to be for the monster to go into melee. If this is zero, the monster will never melee.
//							  If it is negative, the monster will try to keep this distance from the player.
//							  If the monster has a backup, he'll use it if too close, otherwise, a negative value here means the monster will just stop
//							  running at the player at this distance (default 48).
//							 Examples:
//								melee_range = 60 - monster will start swinging it player is closer than 60.
//								melee_range = 0 - monster will never do a melee attack.
//								melee_range = -100 - monster will never do a melee attack and will back away (if it has that ability) when player gets too close.
// missile_range			- Maximum distance the player can be from the monster to be allowed to use it's ranged attack (default 512).
// min_missile_range		- Minimum distance the player can be from the monster to be allowed to use it's ranged attack (default 48).
// bypass_missile_chance	- Chance that a monster will NOT fire it's ranged attack, even when it has a clear shot. This, in effect, will make the monster
//							  come in more often than hang back and fire. A percentage (0 = always fire/never close in, 100 = never fire/always close in) - must be whole number (default 25).
// jump_chance				- Every time the monster has the opportunity to jump, what is the chance (out of 100) that he will... (100 = jump every time) - must be whole number (default 100).
// wakeup_distance			- How far (max) the player can be away from the monster before it wakes up. This means that if the monster can see the player,
//							  at what distance should the monster actually notice him and go for him (default 1024).
// NOTE: A value of zero will result in defaults, if you actually want zero as the value, use -1.
void SP_monster_plague_ssithra(edict_t* self)
{
	static const int mesh_part_ids[] = { MESH__CENTERSPIKE, MESH__LEFT1SPIKE, MESH__RIGHT1SPIKE, MESH__RIGHT2SPIKE }; //mxd.

	if (SKILL == SKILL_EASY)
	{
		if (irand(0, 2) == 0) // 30% - but won't fire explosives.
			self->spawnflags |= MSF_SSITHRA_CLOTHED;
	}
	else if (SKILL == SKILL_MEDIUM)
	{
		if (irand(0, 3) == 0) // 25%
			self->spawnflags |= MSF_SSITHRA_CLOTHED;
	}
	else
	{
		if (irand(0, 1) == 0) // 50%
			self->spawnflags |= MSF_SSITHRA_CLOTHED;
	}

	if (self->spawnflags & MSF_SSITHRA_NAMOR)
		self->spawnflags |= MSF_AMBUSH;

	// Generic Monster Initialization.
	if (!M_WalkmonsterStart(self)) //mxd. M_Start -> M_WalkmonsterStart.
		return; // Failed initialization.

	self->msgHandler = DefaultMsgHandler;
	self->monsterinfo.alert = SsithraAlert;
	self->monsterinfo.dismember = SsithraDismember;
	self->isBlocked = SsithraBlocked;
	self->touch = M_Touch;

	self->materialtype = MAT_FLESH;
	self->flags |= (FL_IMMUNE_SLIME | FL_AMPHIBIAN);
	self->svflags |= SVF_WAIT_NOTSOLID;

	SsithraCheckInWater(self);

	if (self->health <= 0)
		self->health = SSITHRA_HEALTH;

	// Apply to the end result (whether designer set or not).
	self->health = MonsterHealth(self->health);
	self->max_health = self->health;

	self->mass = SSITHRA_MASS;
	self->yaw_speed = 20.0f;

	self->solid = SOLID_BBOX;
	self->movetype = PHYSICSTYPE_STEP;
	VectorClear(self->knockbackvel);

	self->monsterinfo.supporters = -1;
	VectorCopy(STDMinsForClass[self->classID], self->mins);
	VectorCopy(STDMaxsForClass[self->classID], self->maxs);
	self->viewheight = (int)(self->maxs[2] * 0.8f);

	self->s.modelindex = (byte)classStatics[CID_SSITHRA].resInfo->modelIndex;
	self->s.skinnum = ((self->spawnflags & MSF_SSITHRA_CLOTHED) ? 2 : 0);

	// Turn off dismemberment caps, can't see them, so save some polys.
	self->s.fmnodeinfo[MESH__CAPLOWERTORSO].flags |= FMNI_NO_DRAW;
	self->s.fmnodeinfo[MESH__CAPTOPUPPERTORSO].flags |= FMNI_NO_DRAW;
	self->s.fmnodeinfo[MESH__CAPBOTTOMUPPERTORSO].flags |= FMNI_NO_DRAW;
	self->s.fmnodeinfo[MESH__CAPHEAD].flags |= FMNI_NO_DRAW;

	qboolean alpha = true;

	if (!(self->spawnflags & MSF_SSITHRA_ALPHA))
	{
		for (uint i = 0; i < ARRAY_SIZE(mesh_part_ids); i++)
		{
			if (irand(0, 10) < 6)
			{
				const int mesh_id = mesh_part_ids[i];

				if (irand(0, 10) < 5)
				{
					self->s.fmnodeinfo[mesh_id].flags |= FMNI_USE_SKIN;
					self->s.fmnodeinfo[mesh_id].skin = self->s.skinnum + 1;
				}
				else
				{
					self->s.fmnodeinfo[mesh_id].flags |= FMNI_NO_DRAW;
				}

				alpha = false;
			}
		}
	}

	if (alpha) // Tough guy!
	{
		//FIXME: other ssithras won't attack this guy and will follow him.
		self->health += 75;
		self->spawnflags |= MSF_SSITHRA_ALPHA;

		self->s.scale = MODEL_SCALE + 0.5f;
		COLOUR_SET(self->s.color, 255, 255, 128); //mxd. Use define.
	}
	else
	{
		self->s.scale = MODEL_SCALE + 0.1f; //TODO: do this only when self->s.scale == 0.0f?
		COLOUR_SET(self->s.color, 200 + irand(-50, 50), 200 + irand(-50, 50), 200 + irand(-50, 50)); //mxd. Use define.
	}

	self->monsterinfo.scale = self->s.scale;
	self->monsterinfo.otherenemyname = "obj_barrel";

	// Setup my mood function.
	MG_InitMoods(self);

	if (irand(0, 2) == 0)
		self->ai_mood_flags |= AI_MOOD_FLAG_PREDICT;

	QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
}

// QUAKED obj_corpse_ssithra (1 .5 0) (-30 -12 -2) (30 12 2) INVULNERABLE
// A dead plague ssithra.

// Spawnflags:
// INVULNERABLE - It can't be hurt.

// Variables:
// style - skin of ssithra (0: damaged skin, 1: normal skin).
void SP_obj_corpse_ssithra(edict_t* self)
{
	self->s.origin[2] += 26.0f;

	VectorSet(self->mins, -30.0f, -12.0f, -2.0f);
	VectorSet(self->maxs, 30.0f, 12.0f, 2.0f);

	self->s.modelindex = (byte)gi.modelindex("models/monsters/ssithra/tris.fm");
	self->s.frame = FRAME_death_a12; // Ths is the reason the function can't be put in g_obj.c.

	self->s.skinnum = ((self->style == 0) ? 1 : 0); // Setting the skinnum correctly. //TODO: also handle alpha damaged (style 2) and alpha normal (style 3) skins? 
	self->spawnflags |= SF_OBJ_NOPUSH; // Can't be pushed.
	self->svflags |= SVF_DEADMONSTER; // Doesn't block walking.

	ObjectInit(self, 120, 80, MAT_FLESH, SOLID_BBOX);
}