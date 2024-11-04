//
// p_animactor.c
//
// Heretic II
// Copyright 1998 Raven Software
//

#include "player.h"
#include "p_types.h"
#include "p_actions.h"
#include "p_anim_data.h"
#include "p_animactor.h"
#include "p_anims.h"
#include "p_anim_branch.h"
#include "p_ctrl.h"
#include "p_main.h"
#include "angles.h"
#include "g_teleport.h"
#include "vector.h"
#include "utilities.h"
#include "fx.h"

#define PLAYER_SCREAM_THRESHOLD	-600

/*
===============
NormalizeAngle
===============
*/

float NormalizeAngle(float angle)
{	
	// Returns the remainder.

	angle = fmod(angle, ANGLE_360);
	
	// Makes the angle signed.

	if(angle >= ANGLE_180)
	{
		angle -= ANGLE_360;
	}
	if(angle <= -ANGLE_180)
	{
		angle += ANGLE_360;
	}
	
	return(angle);
}

// ************************************************************************************************
// CalcJointAngles
// ----------------
// ************************************************************************************************

static void CalcJointAngles(playerinfo_t *playerinfo) 
{
	vec3_t			targetvector;

	// Adjust the player model's joint angles. The rules are:
	// If there is a target to look at, the torso and head will shift to face the target.
	// If the player is standing still, the torso will shift to face where the view is going.
	// If there is a target to look at, the torso will shift to face the target instead.
	// If the player is moving, only the head will shift, unless there is a target, in which case it will face it.

	playerinfo->headjointonly=false;

	VectorClear(playerinfo->targetjointangles);

	if(playerinfo->enemystate==NULL)
	{	
		// No target to be seen...

		if(playerinfo->pm_w_flags)
		{
			// ...and we're swimming...
			
			// PITCH.

			playerinfo->targetjointangles[PITCH]=-(playerinfo->aimangles[PITCH]-playerinfo->angles[PITCH])*ANGLE_TO_RAD;

			if(playerinfo->targetjointangles[PITCH]>ANGLE_90) 
				playerinfo->targetjointangles[PITCH]=ANGLE_90;
			else if(playerinfo->targetjointangles[PITCH]<-ANGLE_90) 
				playerinfo->targetjointangles[PITCH]=-ANGLE_90;

			if(playerinfo->targetjointangles[PITCH]>=0) 
				playerinfo->targetjointangles[PITCH]/=3.0;
			else
				playerinfo->targetjointangles[PITCH]/=1.5;

			if(playerinfo->pm_w_flags&(WF_DIVING|WF_SWIMFREE))
			{
				// ...and we're below the surface, so just allow the head to PITCH. Of course,
				// we need invert the angle too.
				
				playerinfo->headjointonly=true;

				playerinfo->targetjointangles[PITCH]=-playerinfo->targetjointangles[PITCH];
			}
		}
		else if(playerinfo->pm_flags&PMF_STANDSTILL)
		{	
			// ...and we're standing still with our feet on something solid, so allow head and torso
			// to PITCH and YAW.
						
			// PITCH.

			playerinfo->targetjointangles[PITCH]=-(playerinfo->aimangles[PITCH]-playerinfo->angles[PITCH])*ANGLE_TO_RAD;

			if (playerinfo->targetjointangles[PITCH]>ANGLE_90) 
				playerinfo->targetjointangles[PITCH]=ANGLE_90;
			else if (playerinfo->targetjointangles[PITCH]<-ANGLE_90) 
				playerinfo->targetjointangles[PITCH]=-ANGLE_90;
			
			playerinfo->targetjointangles[PITCH]/=3.0;
			
			// YAW.

			playerinfo->targetjointangles[YAW]=((playerinfo->aimangles[YAW]-playerinfo->angles[YAW])*ANGLE_TO_RAD);

			if(playerinfo->targetjointangles[YAW]<-ANGLE_180)
				playerinfo->targetjointangles[YAW]+=ANGLE_360;
			else if(playerinfo->targetjointangles[YAW]>ANGLE_180)
				playerinfo->targetjointangles[YAW]-=ANGLE_360;

			playerinfo->targetjointangles[YAW]/=3.0;
		}
		else
		{	
			// ...and we're moving - on land or flying or whatever.
			
			playerinfo->headjointonly=true;

			// PITCH.

			playerinfo->targetjointangles[PITCH]=-(playerinfo->aimangles[PITCH]-playerinfo->angles[PITCH])*ANGLE_TO_RAD;
			
			if (playerinfo->targetjointangles[PITCH]>ANGLE_90) 
				playerinfo->targetjointangles[PITCH]=ANGLE_90;
			else if (playerinfo->targetjointangles[PITCH]<-ANGLE_90) 
				playerinfo->targetjointangles[PITCH]=-ANGLE_90;
			
			playerinfo->targetjointangles[PITCH]/=3.0;

			// YAW.

			playerinfo->targetjointangles[YAW]=0;
		}
	}
	else 
	{
		// We have a target...

		if(!playerinfo->pm_w_flags)
		{
			// ...and we aren't swimming, so calculate angles to target.

			VectorCopy(playerinfo->enemystate->origin,targetvector);
			VectorSubtract(targetvector, playerinfo->origin, targetvector);
			vectoangles(targetvector, playerinfo->targetjointangles);

			// PITCH.

			playerinfo->targetjointangles[PITCH] -= playerinfo->angles[PITCH];
			playerinfo->targetjointangles[PITCH] *= ANGLE_TO_RAD;
			playerinfo->targetjointangles[PITCH] = NormalizeAngle(playerinfo->targetjointangles[PITCH]);

			if (playerinfo->targetjointangles[PITCH] > ANGLE_90) 
				playerinfo->targetjointangles[PITCH] = ANGLE_90;
			else if (playerinfo->targetjointangles[PITCH] < -ANGLE_90) 
				playerinfo->targetjointangles[PITCH] = -ANGLE_90;

			playerinfo->targetjointangles[PITCH] /= 3.0;

			// YAW.

			playerinfo->targetjointangles[YAW] -= playerinfo->angles[YAW];
			playerinfo->targetjointangles[YAW] *= ANGLE_TO_RAD;
			playerinfo->targetjointangles[YAW] = NormalizeAngle(playerinfo->targetjointangles[YAW]);

			if (playerinfo->targetjointangles[YAW] > ANGLE_90) 
				playerinfo->targetjointangles[YAW] = ANGLE_90;
			else if (playerinfo->targetjointangles[YAW] < -ANGLE_90) 
				playerinfo->targetjointangles[YAW] = -ANGLE_90;

			playerinfo->targetjointangles[YAW] /= 3.0;
		}
	}
}

PLAYER_API void TurnOffPlayerEffects(playerinfo_t *playerinfo)
{
	// Make sure all effects are removed.

	switch(playerinfo->pers.handfxtype)
	{	
		case HANDFX_FIREBALL:
		case HANDFX_MISSILE:
		case HANDFX_SPHERE:
		case HANDFX_MACEBALL:
			
			if(playerinfo->effects)
				if(!playerinfo->isclient)
					playerinfo->G_RemoveEffects(EFFECT_PRED_ID26,
												playerinfo->G_GetEntityStatePtr(playerinfo->self),
												FX_SPELLHANDS);
				else
					playerinfo->CL_RemoveEffects(EFFECT_PRED_ID26,
												 playerinfo->self,
												 FX_SPELLHANDS);
			
			break;

		case HANDFX_REDRAIN:
		case HANDFX_POWERREDRAIN:
			
			if(playerinfo->effects)
				if(!playerinfo->isclient)
					playerinfo->G_RemoveEffects(EFFECT_PRED_ID27,
												playerinfo->G_GetEntityStatePtr(playerinfo->self),
												FX_WEAPON_REDRAINGLOW);
				else
					playerinfo->CL_RemoveEffects(EFFECT_PRED_ID27,
												playerinfo->self,
												FX_WEAPON_REDRAINGLOW);
			
			break;

		case HANDFX_PHOENIX:
		case HANDFX_POWERPHOENIX:
		case HANDFX_FIREWALL:
			
			if(playerinfo->effects)
				if(!playerinfo->isclient)
					playerinfo->G_RemoveEffects(EFFECT_PRED_ID28,
												playerinfo->G_GetEntityStatePtr(playerinfo->self),
												FX_FIREHANDS);
				else
					playerinfo->CL_RemoveEffects(EFFECT_PRED_ID28,
												 playerinfo->self,
												 FX_FIREHANDS);
			
			break;

		case HANDFX_STAFF1:
		case HANDFX_STAFF2:
		case HANDFX_STAFF3:
			
			if(playerinfo->effects)
			{
				if(!playerinfo->isclient)
					playerinfo->G_RemoveEffects(EFFECT_PRED_ID29,
												playerinfo->G_GetEntityStatePtr(playerinfo->self),
												FX_STAFF);
				else
					playerinfo->CL_RemoveEffects(EFFECT_PRED_ID29,
												 playerinfo->self,
												 FX_STAFF);

				playerinfo->effects&=~EF_BLOOD_ENABLED;
			}
			
			break;

		case HANDFX_NONE:
		
		default:
			
			// Nothing to remove.

			break;
	}

	playerinfo->pers.handfxtype=HANDFX_NONE;
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
