//
// p_animactor.c
//
// Copyright 1998 Raven Software
//

#include "Player.h"
#include "p_anim_data.h"
#include "p_animactor.h"
#include "p_anim_branch.h"
#include "p_ctrl.h"
#include "p_main.h"
#include "p_utility.h" //mxd
#include "g_teleport.h"
#include "Angles.h"
#include "Vector.h"
#include "FX.h"

static float ClampAngle(float angle) //mxd. Originally named NormalizeAngle (function with the same name already defined in Utilities.c).
{
	// Returns the remainder.
	angle = fmodf(angle, ANGLE_360);

	// Makes the angle signed.
	if (angle >= ANGLE_180)
		return angle - ANGLE_360;

	if (angle <= -ANGLE_180)
		return angle + ANGLE_360;

	return angle;
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
		VectorSubtract(targetvector, info->origin, targetvector);
		vectoangles(targetvector, info->targetjointangles);

		// PITCH.
		info->targetjointangles[PITCH] -= info->angles[PITCH];
		info->targetjointangles[PITCH] *= ANGLE_TO_RAD;
		info->targetjointangles[PITCH] = ClampAngle(info->targetjointangles[PITCH]);
		info->targetjointangles[PITCH] = Clamp(info->targetjointangles[PITCH], -ANGLE_90, ANGLE_90);
		info->targetjointangles[PITCH] /= 3.0f;

		// YAW.
		info->targetjointangles[YAW] -= info->angles[YAW];
		info->targetjointangles[YAW] *= ANGLE_TO_RAD;
		info->targetjointangles[YAW] = ClampAngle(info->targetjointangles[YAW]);
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

PLAYER_API void AnimUpdateFrame(playerinfo_t *playerinfo)
{
	panimmove_t	*move;
	float		yaw_delta;

	// Check for death.
	if (playerinfo->deadflag==DEAD_DEAD)
		return;

	if ((playerinfo->flags & PLAYER_FLAG_KNOCKDOWN) && (!(playerinfo->deadflag)))
	{
		// We don't want to do this again next frame.
		playerinfo->flags &= ~PLAYER_FLAG_KNOCKDOWN;
		PlayerInterruptAction(playerinfo);
		if (!(playerinfo->deadflag))
		{	// Don't do it if dying.
			PlayerAnimSetLowerSeq(playerinfo,ASEQ_KNOCKDOWN);
		}
		return;
	}

	// Handle teleporting (and chicken morphing) only on game side.
	if(!playerinfo->isclient)
	{
		if(playerinfo->G_HandleTeleport(playerinfo))
			return;
	}

	//Handle a dive request
	if (playerinfo->flags & PLAYER_FLAG_DIVE && (playerinfo->seqcmd[ACMDL_FWD] || playerinfo->seqcmd[ACMDL_CROUCH]) )
	{
		playerinfo->flags&=~PLAYER_FLAG_DIVE;
		playerinfo->pm_w_flags |= WF_DIVING;
		playerinfo->pm_w_flags &= ~(WF_SURFACE|WF_DIVE);

		PlayerAnimSetLowerSeq(playerinfo, ASEQ_DIVE);
	}

	//Auto grab a rope
	if (playerinfo->flags & PLAYER_FLAG_RELEASEROPE)
	{
		if (playerinfo->flags & PLAYER_FLAG_ONROPE)
		{
			//Turn off the rope graphic immediately 
			((edict_t *)playerinfo->self)->targetEnt->count = 0;
			((edict_t *)playerinfo->self)->targetEnt->rope_grab->s.effects &= ~EF_ALTCLIENTFX;
			((edict_t *)playerinfo->self)->targetEnt->enemy = NULL;
			((edict_t *)playerinfo->self)->targetEnt = NULL;

			((edict_t *)playerinfo->self)->monsterinfo.jump_time = playerinfo->leveltime + 2;
			playerinfo->flags &= ~PLAYER_FLAG_RELEASEROPE;
			playerinfo->flags &= ~PLAYER_FLAG_ONROPE;

			if(!(playerinfo->edictflags & FL_CHICKEN))
			{
				if (((edict_t *)playerinfo->self)->health <= 0)
				{
					PlayerAnimSetLowerSeq(playerinfo, ASEQ_DEATH_A);
				}
				else
				{
					PlayerAnimSetLowerSeq(playerinfo, ASEQ_CLIMB_OFF);
				}
			}
		}
		else
		{
			playerinfo->flags &= ~PLAYER_FLAG_RELEASEROPE;
		}
	}
	else if ( (!(playerinfo->flags & PLAYER_FLAG_ONROPE)) && 
			  (!(playerinfo->flags & PLAYER_FLAG_RELEASEROPE)) && 
				(playerinfo->targetEnt) && 
			  (!(playerinfo->groundentity)) && 
				(((edict_t *)playerinfo->self)->monsterinfo.jump_time < playerinfo->leveltime) &&
				(PlayerActionCheckRopeGrab(playerinfo,0)) &&
				(!(playerinfo->deadflag)) ) //Climb a rope?
	{
		((edict_t *)playerinfo->self)->monsterinfo.jump_time = playerinfo->leveltime + 4;
		playerinfo->flags |= PLAYER_FLAG_ONROPE;
		
		if(playerinfo->isclient)
		{
			playerinfo->CL_Sound(SND_PRED_ID37,
								 playerinfo->origin, 
									CHAN_VOICE, 
									"player/ropegrab.wav", 
									0.75, 
									ATTN_NORM, 
									0);
		}
		else
		{
			playerinfo->G_Sound(SND_PRED_ID37,
								playerinfo->leveltime,
								playerinfo->self, 
								CHAN_VOICE, 
								playerinfo->G_SoundIndex("player/ropegrab.wav"), 
								0.75, 
								ATTN_NORM, 
								0);
		}
		
		PlayerAnimSetLowerSeq(playerinfo, ASEQ_CLIMB_ON);
	}

	// Think rate handled different on client.

	if(!playerinfo->isclient)
		playerinfo->nextthink=playerinfo->leveltime+0.1;//FRAMETIME;
	
	if (!(playerinfo->edictflags & FL_CHICKEN) && (!(playerinfo->deadflag)))
	{
		//FIXME: Implement this with a debounce time
		/*
		if (!playerinfo->groundentity)
		{
			if (playerinfo->velocity[2] < PLAYER_SCREAM_THRESHOLD)
			{
				if(playerinfo->isclient)
					playerinfo->CL_Sound(playerinfo->origin, CHAN_VOICE, "player/falldeath1.wav", 0.75, ATTN_NORM, 0);
				else
					playerinfo->G_Sound(playerinfo->self, CHAN_VOICE, playerinfo->G_SoundIndex("player/falldeath1.wav"), 0.75, ATTN_NORM, 0);
			}
		}
		*/

		if (playerinfo->flags & PLAYER_FLAG_SLIDE)
		{
			//Make sure the player doesn't try to slide underwater
			if (playerinfo->waterlevel < 2)
			{
				if (playerinfo->flags & PLAYER_FLAG_COLLISION)
				{// See if the player is in a jump.

					switch(playerinfo->lowerseq)
					{
						case ASEQ_POLEVAULT2:
						case ASEQ_POLEVAULT1_W: 
						case ASEQ_POLEVAULT1_R:
						
						// Check for an autovault.

						if (playerinfo->upperidle)
						{
							if (playerinfo->seqcmd[ACMDL_BACK])
							{	
								// Otherwise do a backflip.

								playerinfo->upvel += 225;
								PlayerAnimSetLowerSeq(playerinfo, ASEQ_JUMPFLIPB);

								if(playerinfo->isclient)
									playerinfo->CL_Sound(SND_PRED_ID38,playerinfo->origin, CHAN_VOICE, "*offwall.wav", 0.75, ATTN_NORM, 0);
								else
									playerinfo->G_Sound(SND_PRED_ID38,playerinfo->leveltime,playerinfo->self, CHAN_VOICE, playerinfo->G_SoundIndex("*offwall.wav"), 0.75, ATTN_NORM, 0);
								return;
							}
						}
					
						break;
					}
				}

				yaw_delta = (float) Q_fabs(playerinfo->ideal_yaw - playerinfo->angles[YAW]);

				if (yaw_delta < 270.0 && yaw_delta > 90.0)
				{
					if (playerinfo->lowerseq != ASEQ_SLIDE_BACKWARD) 
					{
						PlayerAnimSetLowerSeq(playerinfo, ASEQ_SLIDE_BACKWARD);
					}
				}
				else if (playerinfo->lowerseq != ASEQ_SLIDE_FORWARD)
				{
					PlayerAnimSetLowerSeq(playerinfo, ASEQ_SLIDE_FORWARD);
				}
			}
		}
		else if (playerinfo->flags & PLAYER_FLAG_COLLISION)
		{		
			// See if the player is in a jump.

			switch(playerinfo->lowerseq)
			{
				//

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

				if ( (playerinfo->waterlevel < 2) && (playerinfo->upperidle) )
				{
					if (PlayerActionCheckVault(playerinfo, 0))
					{
						;	// If successful, do nothing else.
					}
					else if (playerinfo->seqcmd[ACMDL_BACK])
					{	
						// Otherwise do a backflip.

						playerinfo->upvel += 225;
						PlayerAnimSetLowerSeq(playerinfo, ASEQ_JUMPFLIPB);

						if(playerinfo->isclient)
							playerinfo->CL_Sound(SND_PRED_ID39,playerinfo->origin, CHAN_VOICE, "*offwall.wav", 0.75, ATTN_NORM, 0);
						else
							playerinfo->G_Sound(SND_PRED_ID39,playerinfo->leveltime,playerinfo->self, CHAN_VOICE, playerinfo->G_SoundIndex("*offwall.wav"), 0.75, ATTN_NORM, 0);
					}
					else if (PlayerSeqData2[playerinfo->lowerseq].collideseq != ASEQ_NONE)
					{	
						// Check to see what to play on a collision.
						PlayerAnimSetLowerSeq(playerinfo, PlayerSeqData2[playerinfo->lowerseq].collideseq);
					}
				}
				
				break;

				//

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

				if (playerinfo->waterlevel < 2 && playerinfo->upperidle)
				{
					if (PlayerActionCheckVault(playerinfo, 0))
					{
						;	// If successful, do nothing else.
					}
	/*				else if (PlayerSeqData2[playerinfo->lowerseq].collideseq != ASEQ_NONE)
					{	
						// Check to see what to play on a collision.

						PlayerAnimSetLowerSeq(playerinfo, PlayerSeqData2[playerinfo->lowerseq].collideseq);
					}
	*/				
				}
				break;
			
				default:
			
				// Check to see what to play on a collision.
					
				//if (PlayerSeqData2[playerinfo->lowerseq].collideseq != ASEQ_NONE)
				//	PlayerAnimSetLowerSeq(playerinfo, PlayerSeqData2[playerinfo->lowerseq].collideseq);

				break;
			}			
		}
	}
	
	// If we are a chicken, don't do this.

	if (playerinfo->seqcmd[ACMDL_JUMP] && !(playerinfo->edictflags & FL_CHICKEN))
	{
		if (!(playerinfo->watertype & CONTENTS_SLIME))
		{
			switch( playerinfo->lowerseq )
   			{
				//

   				case ASEQ_RUNF_GO:
   				case ASEQ_RUNF:
   				case ASEQ_RUNF_END:

   				PlayerAnimSetLowerSeq(playerinfo, BranchLwrRunning(playerinfo));
   				
				break;

				//

   				case ASEQ_WALKF_GO:
   				case ASEQ_WALKF:
   				case ASEQ_WALKF_END:
   				
				PlayerAnimSetLowerSeq(playerinfo, BranchLwrWalking(playerinfo));

				break;

				//

   				case ASEQ_STAND:
   				
				PlayerAnimSetLowerSeq(playerinfo, BranchLwrStanding(playerinfo));
   				
				break;
   			}
		}
	}

	// *************************
	// ** Lower frame handler **
	// *************************

	move = playerinfo->lowermove;
	assert(move);

	if (playerinfo->lowerframe >= move->numframes-1)
	{
		if (move->endfunc)
		{
			move->endfunc (playerinfo);

			// Regrab move, endfunc is very likely to change it.

			move = playerinfo->lowermove;
			assert(move);

			// Check for death.

			if(playerinfo->deadflag==DEAD_DEAD)
				return;
		}
	}

	if (playerinfo->lowerframeptr < move->frame || playerinfo->lowerframeptr >= move->frame + move->numframes)
	{
		playerinfo->lowerframeptr = move->frame;
		playerinfo->lowerframe = 0;
	}
	else
	{ 
		playerinfo->lowerframe++;
		if (playerinfo->lowerframe >= move->numframes)
		{
			playerinfo->lowerframe = 0;
			playerinfo->lowerframeptr = move->frame;
		}
		else
		{
			playerinfo->lowerframeptr = move->frame + playerinfo->lowerframe;
		}
	}
	playerinfo->frame = playerinfo->lowerframeptr->framenum;
	
	if (playerinfo->lowerframeptr->movefunc)
	{
		playerinfo->lowerframeptr->movefunc(playerinfo, 
				playerinfo->lowerframeptr->var1, playerinfo->lowerframeptr->var2, playerinfo->lowerframeptr->var3);
	}
	if (playerinfo->lowerframeptr->actionfunc)
	{
		playerinfo->lowerframeptr->actionfunc(playerinfo, playerinfo->lowerframeptr->var4);
	}

	if (playerinfo->lowerframeptr->thinkfunc)
		playerinfo->lowerframeptr->thinkfunc (playerinfo);

	if (PlayerSeqData2[playerinfo->lowerseq].nosplit)
	{
		// Straighten out joints, i.e. no torso aiming.

		playerinfo->ResetJointAngles(playerinfo);
	
		playerinfo->swapFrame = playerinfo->frame;
		
		return;
	}

	// *************************
	// ** Upper frame handler **
	// *************************
	
	if (playerinfo->upperidle)
	{
		PlayerAnimUpperIdle(playerinfo);
	}

	if (playerinfo->upperseq)
	{
		move = playerinfo->uppermove;
		assert(move);

		if (playerinfo->upperframe >= move->numframes-1)
		{
			if (move->endfunc)
			{
				move->endfunc (playerinfo);

				// Regrab move, endfunc is very likely to change it.

				move = playerinfo->uppermove;
				assert(move);

				// Check for death.

				if(playerinfo->deadflag==DEAD_DEAD)
					return;
			}
		}

		if (playerinfo->upperseq)
		{
			if (playerinfo->upperframeptr < move->frame || playerinfo->upperframeptr >= move->frame + move->numframes)
			{
				playerinfo->upperframeptr = move->frame;
				playerinfo->upperframe = 0;
			}
			else
			{
				playerinfo->upperframe++;
				if (playerinfo->upperframe >= move->numframes)
				{
					playerinfo->upperframe = 0;
					playerinfo->upperframeptr = move->frame;
				}
				else
				{
					playerinfo->upperframeptr = move->frame + playerinfo->upperframe;
				}
			}
			playerinfo->swapFrame = playerinfo->upperframeptr->framenum;
			
			if (playerinfo->upperframeptr->movefunc)
			{
				playerinfo->upperframeptr->movefunc(playerinfo, 
						playerinfo->upperframeptr->var1, playerinfo->upperframeptr->var2, playerinfo->upperframeptr->var3);
			}
			if (playerinfo->upperframeptr->actionfunc)
			{
				playerinfo->upperframeptr->actionfunc(playerinfo, playerinfo->upperframeptr->var4);
			}

			if (playerinfo->upperframeptr->thinkfunc)
				playerinfo->upperframeptr->thinkfunc (playerinfo);

			// Check if the lower frame is idle, if so, force ours.

			if((playerinfo->loweridle)&&(!(PlayerSeqData[playerinfo->upperseq].playerflags&PLAYER_FLAG_LEAVELOWER)))
			{
				playerinfo->frame = playerinfo->swapFrame;
			}

			if((PlayerSeqData2[playerinfo->upperseq].nosplit)&&(!(playerinfo->edictflags&FL_CHICKEN)))
			{
				// Straighten out joints, i.e. no torso aiming.

				playerinfo->ResetJointAngles(playerinfo);

				return;
			}
		}
		else
		{
			playerinfo->swapFrame = playerinfo->frame;
		}
	}
	else
	{
		playerinfo->swapFrame = playerinfo->frame;

		if (PlayerSeqData2[playerinfo->lowerseq].nosplit)
		{	
			// No torso aiming.

			return;
		}
	}

	// Handle torso twisting (but only when we are in Elven form).

	if(!(playerinfo->edictflags&FL_CHICKEN))
	{
		// Calculate joint angle values.

		CalcJointAngles(playerinfo);

		// Now set joints in motion.

		playerinfo->SetJointAngles(playerinfo);
	}
}

PLAYER_API void PlayerFallingDamage(playerinfo_t *playerinfo)
{
	float		delta;
	vec3_t		endpos;
	
	delta=playerinfo->velocity[2]-playerinfo->oldvelocity[2];//falling -200 to standstill 0 gives a delta of 200

	if(!playerinfo->groundentity)
	{
		// If we were falling, and we're now underwater, we should STOP FALLING, capiche?

		VectorCopy(playerinfo->origin,endpos);
		endpos[2]+=playerinfo->mins[2];

		if((playerinfo->flags&PLAYER_FLAG_FALLING)&&
		   (playerinfo->PointContents(endpos)&(CONTENTS_SLIME|CONTENTS_LAVA))&&
		   (playerinfo->waterlevel==1))
		{
			PlayerIntLand(playerinfo,delta);
		} 
		else if((playerinfo->waterlevel==3)&&(playerinfo->flags&PLAYER_FLAG_FALLING))
		{
			// We were falling, and we're now underwater so we should STOP FALLING. Capiche?

			PlayerIntLand(playerinfo,delta);
		}

		return;
	}

	if((playerinfo->flags&PLAYER_FLAG_FALLING)&&(playerinfo->waterlevel<=2))
	{
		PlayerIntLand(playerinfo,delta);
	}

	delta=delta*delta*0.0001;//it's now positive no matter what

	// Never take falling damage if completely underwater.

	if(playerinfo->waterlevel==3)
		return;
	if(playerinfo->waterlevel==2)
		delta*=0.25;
	if(playerinfo->waterlevel==1)
		delta*=0.5;

	if(playerinfo->seqcmd[ACMDL_CROUCH])
		delta*=0.75;//rolling absorbs some
 
	if(delta<1.0)
		return;

	if(delta<15.0)
	{
		// Unimplemented.

		if(!playerinfo->isclient)
			playerinfo->G_CreateEffect(EFFECT_PRED_ID11,
									   playerinfo->G_GetEntityStatePtr(playerinfo->self),
									   FX_FOOTSTEP,
									   CEF_OWNERS_ORIGIN,
									   playerinfo->origin,
									   "");
		else
			playerinfo->CL_CreateEffect(EFFECT_PRED_ID11,
										playerinfo->self,
										FX_FOOTSTEP,
										CEF_OWNERS_ORIGIN,
										playerinfo->origin,
										"");

		return;
	}

	if(delta > 30.0)
	{
		// Apply damage to player entity if we are running server (game) side.

		if(!playerinfo->isclient)
			playerinfo->G_PlayerFallingDamage(playerinfo,delta);
	}
	else
	{
		// Unimplemented.

		if(!playerinfo->isclient)
			playerinfo->G_CreateEffect(EFFECT_PRED_ID12,
									   playerinfo->G_GetEntityStatePtr(playerinfo->self),
									   FX_FALLSHORT,
									   CEF_OWNERS_ORIGIN,
									   playerinfo->origin,
									   "");
		else
			playerinfo->CL_CreateEffect(EFFECT_PRED_ID12,
										playerinfo->self,
										FX_FALLSHORT,
										CEF_OWNERS_ORIGIN,
										playerinfo->origin,
										"");

		return;
	}
}
