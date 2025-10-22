//
// p_animactor.c
//
// Copyright 1998 Raven Software
//

#include "Player.h"
#include "p_actions.h"
#include "p_anim_data.h"
#include "p_animactor.h"
#include "p_anim_branch.h"
#include "p_main.h"
#include "p_utility.h" //mxd
#include "g_Edict.h" //mxd
#include "Angles.h"
#include "Vector.h"
#include "FX.h"
#include "Random.h"
#include "SurfaceProps.h"

// H2_BUGFIX: mxd. Otherwise, in some cases targetjointangles calculated in CalcJointAngles() may overshoot joint destAngles read by MSG_ReadJoints()...
static void SnapJointAnglesToNetworkPrecision(vec3_t angles)
{
	for (int i = 0; i < 3; i++)
		angles[i] = (float)((int)(angles[i] * RAD_TO_BYTEANGLE)) / RAD_TO_BYTEANGLE;
}

static void CalcJointAngles(playerinfo_t* info)
{
	// Adjust the player model's joint angles.
	// The rules are:
	// If there is a target to look at, the torso and head will shift to face the target.
	// If the player is standing still, the torso will shift to face where the view is going.
	// If there is a target to look at, the torso will shift to face the target instead.
	// If the player is moving, only the head will shift, unless there is a target, in which case it will face it.

	info->headjointonly = false;
	VectorClear(info->targetjointangles);

	// We have a target and we aren't swimming, so calculate angles to target.
	if (info->enemystate != NULL && info->pm_w_flags == 0)
	{
		vec3_t targetvector;
		VectorCopy(info->enemystate->origin, targetvector);
		Vec3SubtractAssign(info->origin, targetvector);
		vectoangles(targetvector, info->targetjointangles);

		// PITCH.
		info->targetjointangles[PITCH] = ClampAngleDeg(info->targetjointangles[PITCH] - info->angles[PITCH]) * ANGLE_TO_RAD;
		info->targetjointangles[PITCH] = Clamp(info->targetjointangles[PITCH], -ANGLE_90, ANGLE_90);
		info->targetjointangles[PITCH] /= 3.0f;

		// YAW.
		info->targetjointangles[YAW] = ClampAngleDeg(info->targetjointangles[YAW] - info->angles[YAW]) * ANGLE_TO_RAD;
		info->targetjointangles[YAW] = Clamp(info->targetjointangles[YAW], -ANGLE_90, ANGLE_90);
		info->targetjointangles[YAW] /= 3.0f;

		return;
	}

	// No target to be seen and we're swimming...
	if (info->pm_w_flags != 0)
	{
		// PITCH.
		info->targetjointangles[PITCH] = -(info->aimangles[PITCH] - info->angles[PITCH]) * ANGLE_TO_RAD;
		info->targetjointangles[PITCH] = Clamp(info->targetjointangles[PITCH], -ANGLE_90, ANGLE_90);

		if (info->targetjointangles[PITCH] >= 0.0f)
			info->targetjointangles[PITCH] /= 3.0f;
		else
			info->targetjointangles[PITCH] /= 1.5f;

		// ...and we're below the surface, so just allow the head to PITCH.
		if (info->pm_w_flags & (WF_DIVING | WF_SWIMFREE))
		{
			info->headjointonly = true;
			info->targetjointangles[PITCH] *= -1.0f; // Of course, we need invert the angle too.
		}

		return;
	}

	// No target to be seen and we're standing still with our feet on something solid, so allow head and torso to PITCH and YAW.
	if (info->pm_flags & PMF_STANDSTILL)
	{
		// PITCH.
		info->targetjointangles[PITCH] = -(info->aimangles[PITCH] - info->angles[PITCH]) * ANGLE_TO_RAD;
		info->targetjointangles[PITCH] = Clamp(info->targetjointangles[PITCH], -ANGLE_90, ANGLE_90);
		info->targetjointangles[PITCH] /= 3.0f;

		// YAW.
		info->targetjointangles[YAW] = (info->aimangles[YAW] - info->angles[YAW]) * ANGLE_TO_RAD;

		if (info->targetjointangles[YAW] < -ANGLE_180)
			info->targetjointangles[YAW] += ANGLE_360;
		else if (info->targetjointangles[YAW] > ANGLE_180)
			info->targetjointangles[YAW] -= ANGLE_360;

		info->targetjointangles[YAW] /= 3.0f;

		return;
	}

	// No target to be seen and we're moving - on land or flying or whatever.
	info->headjointonly = true;

	// PITCH.
	info->targetjointangles[PITCH] = -(info->aimangles[PITCH] - info->angles[PITCH]) * ANGLE_TO_RAD;
	info->targetjointangles[PITCH] = Clamp(info->targetjointangles[PITCH], -ANGLE_90, ANGLE_90);
	info->targetjointangles[PITCH] /= 3.0f;
}

PLAYER_API void TurnOffPlayerEffects(playerinfo_t* info)
{
	if (info->effects != 0)
	{
		// Make sure all effects are removed.
		switch (info->pers.handfxtype)
		{
			case HANDFX_FIREBALL:
			case HANDFX_MISSILE:
			case HANDFX_SPHERE:
			case HANDFX_MACEBALL:
				P_RemoveEffects(info, EFFECT_PRED_ID26, FX_SPELLHANDS); //mxd
				break;

			case HANDFX_REDRAIN:
			case HANDFX_POWERREDRAIN:
				P_RemoveEffects(info, EFFECT_PRED_ID27, FX_WEAPON_REDRAINGLOW); //mxd
				break;

			case HANDFX_PHOENIX:
			case HANDFX_POWERPHOENIX:
			case HANDFX_FIREWALL:
				P_RemoveEffects(info, EFFECT_PRED_ID28, FX_FIREHANDS); //mxd
				break;

			case HANDFX_STAFF1:
			case HANDFX_STAFF2:
			case HANDFX_STAFF3:
				P_RemoveEffects(info, EFFECT_PRED_ID29, FX_STAFF); //mxd
				break;

			case HANDFX_NONE:
			default: // Nothing to remove.
				break;
		}
	}

	info->pers.handfxtype = HANDFX_NONE;
}

PLAYER_API void AnimUpdateFrame(playerinfo_t* info)
{
	// Check for death.
	if (info->deadflag == DEAD_DEAD)
		return;

	//mxd. Update idle time (originally done in BranchLwrStanding(), BranchUprReadyHands(), BranchUprReadySwordStaff(), BranchUprReadyHellStaff(), BranchUprReadyBow() and BranchUprReady()).
	if ((info->lowerseq != ASEQ_STAND && info->lowerseq != ASEQ_IDLE_READY) || info->upperseq != ASEQ_NONE) //mxd. Add upperseq check.
		info->idletime = info->leveltime;

	if ((info->flags & PLAYER_FLAG_KNOCKDOWN) && info->deadflag == DEAD_NO)
	{
		// We don't want to do this again next frame.
		info->flags &= ~PLAYER_FLAG_KNOCKDOWN;
		PlayerInterruptAction(info);
		PlayerAnimSetLowerSeq(info, ASEQ_KNOCKDOWN);

		return;
	}

	// Handle teleporting (and chicken morphing) only on game side.
	if (!info->isclient && info->G_HandleTeleport(info))
		return;

	// Handle a dive request.
	if ((info->flags & PLAYER_FLAG_DIVE) && (info->seqcmd[ACMDL_FWD] || info->seqcmd[ACMDL_CROUCH]))
	{
		info->flags &= ~PLAYER_FLAG_DIVE;
		info->pm_w_flags |= WF_DIVING;
		info->pm_w_flags &= ~(WF_SURFACE | WF_DIVE);

		PlayerAnimSetLowerSeq(info, ASEQ_DIVE);
	}

	edict_t* self = info->self; //mxd

	// Auto grab a rope.
	if (info->flags & PLAYER_FLAG_RELEASEROPE)
	{
		if (info->flags & PLAYER_FLAG_ONROPE)
		{
			// Turn off the rope graphic immediately.
			self->targetEnt->count = 0;
			self->targetEnt->rope_grab->s.effects &= ~EF_ALTCLIENTFX;
			self->targetEnt->enemy = NULL;
			self->targetEnt = NULL;

			self->monsterinfo.jump_time = info->leveltime + 2.0f;
			info->flags &= ~(PLAYER_FLAG_RELEASEROPE | PLAYER_FLAG_ONROPE);

			if (!(info->edictflags & FL_CHICKEN))
				PlayerAnimSetLowerSeq(info, (self->health <= 0 ? ASEQ_DEATH_A : ASEQ_CLIMB_OFF));
		}
		else
		{
			info->flags &= ~PLAYER_FLAG_RELEASEROPE;
		}
	}
	else if (!(info->flags & PLAYER_FLAG_ONROPE) && !(info->flags & PLAYER_FLAG_RELEASEROPE) &&
		info->targetEnt != NULL && info->groundentity == NULL && self->monsterinfo.jump_time < info->leveltime &&
		PlayerActionCheckRopeGrab(info, 0) && info->deadflag == DEAD_NO) // Climb a rope?
	{
		self->monsterinfo.jump_time = info->leveltime + 4.0f;
		info->flags |= PLAYER_FLAG_ONROPE;

		P_Sound(info, SND_PRED_ID37, CHAN_VOICE, "player/ropegrab.wav", 0.75f); //mxd
		PlayerAnimSetLowerSeq(info, ASEQ_CLIMB_ON);
	}

	// Think rate handled different on client.
	if (!info->isclient)
		info->nextthink = info->leveltime + 0.1f; // FRAMETIME

	if (!(info->edictflags & FL_CHICKEN) && info->deadflag == DEAD_NO)
	{
		if (info->flags & PLAYER_FLAG_SLIDE)
		{
			// Make sure the player doesn't try to slide underwater
			if (info->waterlevel < 2)
			{
				// See if the player is in a jump.
				if (info->flags & PLAYER_FLAG_COLLISION &&
					(info->lowerseq == ASEQ_POLEVAULT1_W || info->lowerseq == ASEQ_POLEVAULT1_R || info->lowerseq == ASEQ_POLEVAULT2))
				{
					// Check for an autovault.
					if (info->upperidle && info->seqcmd[ACMDL_BACK])
					{
						// Otherwise do a backflip.
						info->upvel += 225.0f;
						PlayerAnimSetLowerSeq(info, ASEQ_JUMPFLIPB);
						P_Sound(info, SND_PRED_ID38, CHAN_VOICE, "*offwall.wav", 0.75f); //mxd

						return;
					}
				}

				const float yaw_delta = fabsf(info->ideal_yaw - info->angles[YAW]);
				const int slide_seq = (yaw_delta > 90.0f && yaw_delta < 270.0f ? ASEQ_SLIDE_BACKWARD : ASEQ_SLIDE_FORWARD); //mxd

				if (info->lowerseq != slide_seq)
					PlayerAnimSetLowerSeq(info, slide_seq);
			}
		}
		else if (info->flags & PLAYER_FLAG_COLLISION)
		{
			// See if the player is in a jump.
			switch (info->lowerseq)
			{
				case ASEQ_POLEVAULT2:
				case ASEQ_POLEVAULT1_W:
				case ASEQ_POLEVAULT1_R:
				case ASEQ_JUMPFWD_SGO:
				case ASEQ_JUMPFWD_WGO:
				case ASEQ_JUMPFWD_RGO:
				case ASEQ_JUMPFWD:
				case ASEQ_FORWARD_FLIP_L:
				case ASEQ_FORWARD_FLIP_R:
					// Check for an autovault.
					if (info->upperidle && info->waterlevel < 2 && !PlayerActionCheckVault(info))
					{
						if (info->seqcmd[ACMDL_BACK])
						{
							// Otherwise do a backflip.
							info->upvel += 225.0f;
							PlayerAnimSetLowerSeq(info, ASEQ_JUMPFLIPB);
							P_Sound(info, SND_PRED_ID39, CHAN_VOICE, "*offwall.wav", 0.75f); //mxd
						}
						else if (PlayerSeqData2[info->lowerseq].collideseq != ASEQ_NONE)
						{
							// Check to see what to play on a collision.
							PlayerAnimSetLowerSeq(info, PlayerSeqData2[info->lowerseq].collideseq);
						}
					}
					break;

				case ASEQ_RUNF_GO:
				case ASEQ_RUNF:
				case ASEQ_WALKF_GO:
				case ASEQ_WALKF:
				case ASEQ_SSWIMF_GO:
				case ASEQ_SSWIMF:
				case ASEQ_SSWIMF_END:
				case ASEQ_SSWIM_FAST_GO:
				case ASEQ_SSWIM_FAST:
					// Check for an autovault.
					if (info->upperidle && info->waterlevel < 2)
						PlayerActionCheckVault(info);
					break;

				default:
					break;
			}
		}
	}

	// If we are a chicken, don't do this.
	if (info->seqcmd[ACMDL_JUMP] && !(info->edictflags & FL_CHICKEN) && !(info->watertype & CONTENTS_SLIME))
	{
		switch (info->lowerseq)
		{
			case ASEQ_RUNF_GO:
			case ASEQ_RUNF:
			case ASEQ_RUNF_END:
				PlayerAnimSetLowerSeq(info, BranchLwrRunning(info));
				break;

			case ASEQ_WALKF_GO:
			case ASEQ_WALKF:
			case ASEQ_WALKF_END:
				PlayerAnimSetLowerSeq(info, BranchLwrWalking(info));
				break;

			case ASEQ_STAND:
				PlayerAnimSetLowerSeq(info, BranchLwrStanding(info));
				break;
		}
	}

	///////////////////////////////////////////////// LOWER FRAME HANDLER /////////////////////////////////////////////////

	const panimmove_t* move = info->lowermove;
	assert(move);

	if (info->lowerframe >= move->numframes - 1 && move->endfunc != NULL)
	{
		move->endfunc(info);

		// Re-grab move, endfunc is very likely to change it.
		move = info->lowermove;
		assert(move);

		// Check for death.
		if (info->deadflag == DEAD_DEAD)
			return;
	}

	if (info->lowerframeptr < move->frame || info->lowerframeptr >= move->frame + move->numframes)
	{
		info->lowerframeptr = move->frame;
		info->lowerframe = 0;
	}
	else
	{
		info->lowerframe++;
		if (info->lowerframe >= move->numframes)
		{
			info->lowerframe = 0;
			info->lowerframeptr = move->frame;
		}
		else
		{
			info->lowerframeptr = move->frame + info->lowerframe;
		}
	}

	info->frame = info->lowerframeptr->framenum;

	if (info->lowerframeptr->movefunc != NULL)
		info->lowerframeptr->movefunc(info, info->lowerframeptr->var1, info->lowerframeptr->var2, info->lowerframeptr->var3);

	if (info->lowerframeptr->actionfunc != NULL)
		info->lowerframeptr->actionfunc(info, info->lowerframeptr->var4);

	if (info->lowerframeptr->thinkfunc != NULL)
		info->lowerframeptr->thinkfunc(info);

	if (PlayerSeqData2[info->lowerseq].nosplit)
	{
		// Straighten out joints, i.e. no torso aiming.
		info->ResetJointAngles(info);
		info->swapFrame = info->frame;

		return;
	}

	///////////////////////////////////////////////// UPPER FRAME HANDLER /////////////////////////////////////////////////

	if (info->upperidle)
		PlayerAnimUpperIdle(info);

	if (info->upperseq > 0)
	{
		move = info->uppermove;
		assert(move);

		if (info->upperframe >= move->numframes - 1 && move->endfunc != NULL)
		{
			move->endfunc(info);

			// Re-grab move, endfunc is very likely to change it.
			move = info->uppermove;
			assert(move);

			// Check for death.
			if (info->deadflag == DEAD_DEAD)
				return;
		}

		if (info->upperseq > 0)
		{
			if (info->upperframeptr < move->frame || info->upperframeptr >= move->frame + move->numframes)
			{
				info->upperframeptr = move->frame;
				info->upperframe = 0;
			}
			else
			{
				info->upperframe++;

				if (info->upperframe >= move->numframes)
				{
					info->upperframe = 0;
					info->upperframeptr = move->frame;
				}
				else
				{
					info->upperframeptr = move->frame + info->upperframe;
				}
			}

			info->swapFrame = info->upperframeptr->framenum;

			if (info->upperframeptr->movefunc != NULL)
				info->upperframeptr->movefunc(info, info->upperframeptr->var1, info->upperframeptr->var2, info->upperframeptr->var3);

			if (info->upperframeptr->actionfunc != NULL)
				info->upperframeptr->actionfunc(info, info->upperframeptr->var4);

			if (info->upperframeptr->thinkfunc != NULL)
				info->upperframeptr->thinkfunc(info);

			// Check if the lower frame is idle, if so, force ours.
			if (info->loweridle && !(PlayerSeqData[info->upperseq].playerflags & PLAYER_FLAG_LEAVELOWER))
				info->frame = info->swapFrame;

			if (PlayerSeqData2[info->upperseq].nosplit && !(info->edictflags & FL_CHICKEN))
			{
				// Straighten out joints, i.e. no torso aiming.
				info->ResetJointAngles(info);
				return;
			}
		}
		else
		{
			info->swapFrame = info->frame;
		}
	}
	else
	{
		info->swapFrame = info->frame;

		if (PlayerSeqData2[info->lowerseq].nosplit)
			return; // No torso aiming.
	}

	// Handle torso twisting (but only when we are in Elven form).
	if (!(info->edictflags & FL_CHICKEN))
	{
		CalcJointAngles(info); // Calculate joint angle values.
		SnapJointAnglesToNetworkPrecision(info->targetjointangles); //mxd
		info->SetJointAngles(info); // Now set joints in motion.
	}
}

PLAYER_API void PlayerFallingDamage(playerinfo_t* info) // Called by CL_PredictMovement_impl() and ClientEndServerFrame() --mxd.
{
	float delta = info->velocity[2] - info->oldvelocity[2]; // Falling -200 to standstill 0 gives a delta of 200.

	if (info->groundentity == NULL) //TODO: this seems unnecessary. There are already waterlevel and PLAYER_FLAG_FALLING checks below. Also, there are no lava/slime materials or landing sounds...
	{
		if (info->flags & PLAYER_FLAG_FALLING)
		{
			vec3_t endpos;
			VectorCopy(info->origin, endpos);
			endpos[2] += info->mins[2];

			if (info->waterlevel == 3 || (info->waterlevel == 1 && (info->PointContents(endpos) & (CONTENTS_SLIME | CONTENTS_LAVA))))
				PlayerIntLand(info, delta); // We were falling, and we're now underwater so we should STOP FALLING.
		}

		return;
	}

	// Never take falling damage if completely underwater.
	if (info->waterlevel == 3)
		return;

	//mxd. PLAYER_FLAG_FALLING is set when SWITCHING TO jump sequence (e.g. ASEQ_JUMP[NNN]_GO), so we can get here before jump velocity is applied (and groundentity is cleared).
	//mxd. So, skip logic when not falling down (delta > 0) to avoid interrupting/switching current animation...
	if ((info->flags & PLAYER_FLAG_FALLING) && info->waterlevel <= 2 && delta > 0.0f)
		PlayerIntLand(info, delta);

	delta = fabsf(delta) + FLOAT_ZERO_EPSILON; // It's now positive no matter what.

	if (info->waterlevel == 2)
		delta *= 0.25f;
	else if (info->waterlevel == 1)
		delta *= 0.5f;

	if (info->seqcmd[ACMDL_CROUCH])
		delta *= 0.75f; // Rolling absorbs some falling damage.

	if (delta < 1.0f)
		return;

	if (delta < 15.0f)
		P_CreateEffect(info, EFFECT_PRED_ID11, info->self, FX_FOOTSTEP,  CEF_OWNERS_ORIGIN, info->origin, ""); //mxd
	else if (delta <= 30.0f)
		P_CreateEffect(info, EFFECT_PRED_ID12, info->self, FX_FALLSHORT, CEF_OWNERS_ORIGIN, info->origin, ""); //mxd
	else if (!info->isclient) // Apply damage to player entity if we are running server (game) side.
		info->G_PlayerFallingDamage(info, delta);
}

PLAYER_API void PlayerIntLand(playerinfo_t* info, const float landspeed) //mxd. Defined in p_ctrl.c in original version.
{
	qboolean hardfall = false;

	// Initialise the appropriate landing sound.
	const char* material = GetClientGroundSurfaceMaterialName(info);

	char land_sound[64];
	strcpy_s(land_sound, sizeof(land_sound), "player/"); //mxd. strcpy -> strcpy_s.

	// Okay, lame, but if a material is not found, then it ignores the string being set up.
	if (material != NULL)
		strcat_s(land_sound, sizeof(land_sound), material); //mxd. strcat -> strcat_s.

	if (info->edictflags & FL_CHICKEN)
	{
		info->flags &= ~PLAYER_FLAG_FALLING;
	}
	else if (info->advancedstaff && info->seqcmd[ACMDU_ATTACK] && (info->upperseq == ASEQ_WSWORD_DOWNSTAB || info->upperseq == ASEQ_WSWORD_STABHOLD))
	{
		PlayerInterruptAction(info);

		if (landspeed > 600.0f) //mxd. Flipped check.
		{
			// Land hard, so roll.
			PlayerAnimSetLowerSeq(info, ASEQ_ROLLDIVEF_W);
			info->fwdvel = 150.0f;
			strcat_s(land_sound, sizeof(land_sound), "roll.wav"); //mxd. strcat -> strcat_s.
			hardfall = true;
		}
		else
		{
			// Land heavy.
			PlayerAnimSetLowerSeq(info, ASEQ_WSWORD_LOWERPULLOUT);
			info->fwdvel = 0.0f;
			info->velocity[0] *= 0.5f;
			info->velocity[1] *= 0.5f;
			strcpy_s(land_sound, sizeof(land_sound), "*offwall.wav"); //mxd. strcpy -> strcpy_s.
		}
	}
	else if (info->seqcmd[ACMDL_CROUCH])
	{
		PlayerInterruptAction(info);

		if (landspeed > 50.0f)
		{
			PlayerAnimSetLowerSeq(info, ASEQ_ROLLDIVEF_W);
			strcat_s(land_sound, sizeof(land_sound), "roll.wav"); //mxd. strcat -> strcat_s.
		}
		else
		{
			PlayerAnimSetLowerSeq(info, ASEQ_CROUCH_GO);
			strcat_s(land_sound, sizeof(land_sound), "land1.wav"); //mxd. strcat -> strcat_s.
		}
	}
	else if (info->lowerseq == ASEQ_FORWARD_FLIP_L || info->lowerseq == ASEQ_FORWARD_FLIP_R ||
		info->upperseq == ASEQ_FORWARD_FLIP_L || info->upperseq == ASEQ_FORWARD_FLIP_R)
	{
		PlayerInterruptAction(info);

		if (info->maxs[2] == 25.0f) //TODO: have a global CROUCHING_MAX_Z and STANDING_MAX_Z
		{
			// Ready to stand up.
			PlayerAnimSetLowerSeq(info, ASEQ_CROUCH_GO);
			strcat_s(land_sound, sizeof(land_sound), "land1.wav"); //mxd. strcat -> strcat_s.
		}
		else
		{
			// Still crouch height.
			PlayerAnimSetLowerSeq(info, ASEQ_ROLL_FROM_FFLIP);
			strcat_s(land_sound, sizeof(land_sound), "roll.wav"); //mxd. strcat -> strcat_s.
		}
	}
	else if (info->seqcmd[ACMDL_WALK_F])
	{
		if (landspeed > 600.0f)
		{
			// Can't avoid heavy fall/rolling.
			PlayerInterruptAction(info);
			PlayerAnimSetLowerSeq(info, ASEQ_ROLLDIVEF_W);
			strcat_s(land_sound, sizeof(land_sound), "roll.wav"); //mxd. strcat -> strcat_s.
		}
		else
		{
			// Drop straight into a walk.
			PlayerAnimSetLowerSeq(info, ASEQ_WALKF);
			strcat_s(land_sound, sizeof(land_sound), "land1.wav"); //mxd. strcat -> strcat_s.
		}
	}
	else if (info->seqcmd[ACMDL_RUN_F])
	{
		if (landspeed > 600.0f)
		{
			// Can't avoid heavy fall/rolling.
			PlayerInterruptAction(info);
			PlayerAnimSetLowerSeq(info, ASEQ_ROLLDIVEF_W);
			strcat_s(land_sound, sizeof(land_sound), "roll.wav"); //mxd. strcat -> strcat_s.
		}
		else
		{
			// Drop straight into a run.
			PlayerAnimSetLowerSeq(info, ASEQ_RUNF);
			strcat_s(land_sound, sizeof(land_sound), "land1.wav"); //mxd. strcat -> strcat_s.
		}
	}
	else if (info->seqcmd[ACMDL_JUMP])
	{
		if (landspeed > 600.0f)
		{
			// Can't avoid heavy fall/rolling.
			PlayerInterruptAction(info);
			PlayerAnimSetLowerSeq(info, ASEQ_ROLLDIVEF_W);
			strcat_s(land_sound, sizeof(land_sound), "roll.wav"); //mxd. strcat -> strcat_s.
		}
		else
		{
			// Drop straight into another jump.
			PlayerAnimSetLowerSeq(info, ASEQ_JUMPSTD_GO);
			strcat_s(land_sound, sizeof(land_sound), va("walk%i.wav", irand(1, 2))); //mxd. strcat -> strcat_s.
		}
	}
	else
	{
		if (landspeed < 50.0f)
		{
			PlayerAnimSetLowerSeq(info, ASEQ_STAND);
			info->fwdvel = 150.0f;
			strcat_s(land_sound, sizeof(land_sound), "shuffle1.wav"); //mxd. strcat -> strcat_s.
		}
		else if (landspeed < 300.0f)
		{
			// Land light.
			PlayerAnimSetLowerSeq(info, ASEQ_LANDLIGHT);
			info->fwdvel = 0.0f;
			info->velocity[0] *= 0.5f;
			info->velocity[1] *= 0.5f;
			strcat_s(land_sound, sizeof(land_sound), "land1.wav"); //mxd. strcat -> strcat_s.
		}
		else if (landspeed < 600.0f)
		{
			// Land heavy.
			PlayerAnimSetLowerSeq(info, ASEQ_LANDHEAVY);
			info->fwdvel = 0.0f;
			info->velocity[0] *= 0.5f;
			info->velocity[1] *= 0.5f;
			strcat_s(land_sound, sizeof(land_sound), "land2.wav"); //mxd. strcat -> strcat_s.
		}
		else
		{
			// Land hard, so roll.
			PlayerInterruptAction(info);
			PlayerAnimSetLowerSeq(info, ASEQ_ROLLDIVEF_W);
			info->fwdvel = 150.0f;
			strcat_s(land_sound, sizeof(land_sound), "roll.wav"); //mxd. strcat -> strcat_s.
			hardfall = true;
		}
	}

	// Play the appropriate landing sound.
	if (material != NULL)
		P_Sound(info, SND_PRED_ID51, CHAN_VOICE, land_sound, 1.0f); //mxd

	// Play grunt sound?
	if (hardfall)
		P_Sound(info, SND_PRED_ID52, CHAN_FOOTSTEP, "*fall.wav", 1.0f); //mxd

	info->flags &= ~PLAYER_FLAG_FALLING;

	// Don't do dust in the water!
	if (info->waterlevel == 0)
		P_CreateEffect(info, EFFECT_PRED_ID15, info->self, FX_DUST_PUFF, CEF_OWNERS_ORIGIN, info->origin, ""); //mxd
}