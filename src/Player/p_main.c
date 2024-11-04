//
// p_main.c
//
// Heretic II
// Copyright 1998 Raven Software
//

#include "player.h"
#include "p_animactor.h"
#include "p_anim_data.h"
#include "p_anims.h"
#include "p_main.h"
#include "p_weapon.h"
#include "p_types.h"
#include "fx.h"
#include "m_player.h"
#include "reference.h"
#include "vector.h"

//FIXME:  Include header

qboolean BranchCheckDismemberAction(playerinfo_t *playerinfo, int weapon);

void FMNodeUpdate(playerinfo_t *playerinfo,int weapon,int armor);

PLAYER_API void PlayerInit(playerinfo_t *playerinfo, int complete_reset)
{
	if(!complete_reset)
		PlayerBasicAnimReset(playerinfo);
	else
		PlayerAnimReset(playerinfo);	

	playerinfo->flags=PLAYER_FLAG_NONE;
}

PLAYER_API void PlayerClearEffects(playerinfo_t *playerinfo)
{
	// Remove all special effects from the player.

	if(!playerinfo->isclient)
		playerinfo->G_RemoveEffects(EFFECT_PRED_ID30,
									playerinfo->G_GetEntityStatePtr(playerinfo->self),
									FX_REMOVE_EFFECTS);
	else
		playerinfo->CL_RemoveEffects(EFFECT_PRED_ID30,
									 playerinfo->self,
									 FX_REMOVE_EFFECTS);
}

PLAYER_API void PlayerUpdateCmdFlags(playerinfo_t *playerinfo)
{
	usercmd_t *pcmd;

	pcmd=&(playerinfo->pcmd);

	// Look for the attack button being pressed.

	if (pcmd->buttons & BUTTON_ATTACK)
	{
		playerinfo->seqcmd[ACMDU_ATTACK] = true;
	}
	else
	{
		playerinfo->seqcmd[ACMDU_ATTACK] = false;
	}

	// Look for the action button being pressed.

	if (pcmd->buttons & BUTTON_ACTION)
	{
		playerinfo->seqcmd[ACMDL_ACTION] = true;
	}
	else
	{
		playerinfo->seqcmd[ACMDL_ACTION] = false;
	}

	// Look for the quickturn button being pressed.

	if (pcmd->buttons & BUTTON_QUICKTURN)
	{
		playerinfo->seqcmd[ACMDL_QUICKTURN] = true;
	}
	else
	{
		playerinfo->seqcmd[ACMDL_QUICKTURN] = false;
	}

	// Look for the jump / crouch buttons being pressed.

	if (pcmd->upmove > 0)
	{
		playerinfo->seqcmd[ACMDL_JUMP] = true;
		playerinfo->seqcmd[ACMDL_CROUCH] = false;
	}
	else if (pcmd->upmove < 0)
	{
		playerinfo->seqcmd[ACMDL_JUMP] = false;
		playerinfo->seqcmd[ACMDL_CROUCH] = true;
	}
	else 
	{
		playerinfo->seqcmd[ACMDL_JUMP] = false;
		playerinfo->seqcmd[ACMDL_CROUCH] = false;
	}

	// Look for the turn left / turn right buttons being pressed.

	if(Q_fabs(playerinfo->turncmd) > 2)
	{
		if (playerinfo->turncmd < -2)
		{
			if(playerinfo->loweridle)
				playerinfo->seqcmd[ACMDL_ROTATE_R] = true;
			else
				playerinfo->seqcmd[ACMDL_ROTATE_R] = false;
			playerinfo->seqcmd[ACMDL_ROTATE_L] = false;
		}
		else if (playerinfo->turncmd > 2)
		{
			if(playerinfo->loweridle)
				playerinfo->seqcmd[ACMDL_ROTATE_L] = true;
			else
				playerinfo->seqcmd[ACMDL_ROTATE_L] = false;
			playerinfo->seqcmd[ACMDL_ROTATE_R] = false;
		}
	}
	else
	{
		playerinfo->seqcmd[ACMDL_ROTATE_R] = false;
		playerinfo->seqcmd[ACMDL_ROTATE_L] = false;
	}

	playerinfo->turncmd = 0;

	// Look for the autoaim button being pressed.

	if (pcmd->buttons & BUTTON_AUTOAIM)
	{
		playerinfo->autoaim = true;
	}
	else
	{
		playerinfo->autoaim = false;
	}

	// Clear out ALL forward/backward movement flags.

	memset(&(playerinfo->seqcmd[ACMDL_CREEP_F]), 0, (ACMDL_BACK-ACMDL_CREEP_F+1)*sizeof(int));

	// Look for forward/backpeddle buttons being pressed.

	if (pcmd->forwardmove > 10)
	{
		playerinfo->seqcmd[ACMDL_FWD] = true;
		if (pcmd->buttons & BUTTON_CREEP)
			playerinfo->seqcmd[ACMDL_CREEP_F] = true;
		else if (pcmd->buttons & BUTTON_RUN)
			playerinfo->seqcmd[ACMDL_RUN_F] = true;
		else
			playerinfo->seqcmd[ACMDL_WALK_F] = true;
	}
	else if (pcmd->forwardmove < -10)
	{
		playerinfo->seqcmd[ACMDL_BACK] = true;
		if (pcmd->buttons & BUTTON_CREEP)
			playerinfo->seqcmd[ACMDL_CREEP_B] = true;
		else if (pcmd->buttons & BUTTON_RUN)
			playerinfo->seqcmd[ACMDL_RUN_B] = true;
		else
			playerinfo->seqcmd[ACMDL_WALK_B] = true;
	}
	else
	{
		// No movement.
	}

	if (pcmd->sidemove > 2)
	{
		playerinfo->seqcmd[ACMDL_STRAFE_R] = true;
		playerinfo->seqcmd[ACMDL_STRAFE_L] = false;
	}
	else if (pcmd->sidemove < -2)
	{
		playerinfo->seqcmd[ACMDL_STRAFE_L] = true;
		playerinfo->seqcmd[ACMDL_STRAFE_R] = false;
	}
	else
	{
		playerinfo->seqcmd[ACMDL_STRAFE_L] = false;
		playerinfo->seqcmd[ACMDL_STRAFE_R] = false;
	}

	playerinfo->showpuzzleinventory = false;
	if (pcmd->buttons & BUTTON_INVENTORY)
	{
		playerinfo->showpuzzleinventory = true;
	}
	playerinfo->fwdvel=0;
	playerinfo->sidevel=0;
	playerinfo->upvel=0;
}

int PlayerCheckSlide(playerinfo_t *playerinfo)
{
	trace_t	trace;
	vec3_t	vf, vr, vu;
	vec3_t	startpos, endpos, mins, maxs;
	float	dot;

	VectorCopy(playerinfo->origin, startpos);
	VectorCopy(playerinfo->origin, endpos);
	endpos[2] -= 32;

	VectorSet(mins, -4, -4, -4);
	VectorSet(maxs,  4,  4,  4);

	AngleVectors(playerinfo->angles, vf, vr, vu);

	if(playerinfo->isclient)
		playerinfo->CL_Trace(startpos,mins,maxs,endpos,MASK_PLAYERSOLID,CEF_CLIP_TO_WORLD,&trace);
	else
		playerinfo->G_Trace(startpos,mins,maxs,endpos,playerinfo->self,MASK_PLAYERSOLID,&trace);
	
	if (trace.fraction < 1)
	{
		dot = DotProduct(vf, trace.plane.normal);
	}
	
	return false;
}

PLAYER_API void PlayerUpdate(playerinfo_t *playerinfo)
{
	int		slideseq;
	vec3_t	endpos;
	
	if (playerinfo->deadflag==DEAD_DEAD || playerinfo->deadflag==DEAD_DYING)
		return;

	if (playerinfo->groundentity == NULL)
	{
		if ((slideseq = PlayerCheckSlide(playerinfo)))
		{
			PlayerAnimSetLowerSeq(playerinfo, slideseq);
		}
	}

	VectorCopy(playerinfo->origin, endpos);
	endpos[2] += playerinfo->mins[2];
	
	if ((playerinfo->PointContents(endpos) & (CONTENTS_SLIME|CONTENTS_LAVA))) 
	{
	}
	
	// At the very first point, evaluate whether we are in a water or air sequence, and then
	// whether the player is in water or air.

	else if (PlayerSeqData[playerinfo->lowerseq].playerflags & PLAYER_FLAG_WATER)
	{	
		// Then we SHOULD be in water for this particular move.

		if ((playerinfo->waterlevel > 2) && (PlayerSeqData2[playerinfo->lowerseq].waterseq == ASEQ_SSWIM_IDLE))
		{
			// We're completely under the water.

			PlayerAnimSetLowerSeq(playerinfo, ASEQ_USWIM_IDLE);
		}
		else if (playerinfo->waterlevel < 1 || (playerinfo->waterlevel < 2 && playerinfo->groundentity))
		{	
			// If we are not in the water at all currently OR if our toes are in yet our feet are
			// touching the ground then we abandon the water sequence. Waterseq here represents the
			// proper sequence to go to when LEAVING water.
		
			//NOTENOTE: This is the offending code that killed the pullups out of water.  Here's a patch...

			if (PlayerSeqData2[playerinfo->lowerseq].waterseq != ASEQ_NONE)
				PlayerAnimSetLowerSeq(playerinfo, PlayerSeqData2[playerinfo->lowerseq].waterseq);
		}
	}
	else
	{	
		// We should NOT be in water for this particular move.

		if ((playerinfo->waterlevel > 2) && (PlayerSeqData2[playerinfo->lowerseq].waterseq == ASEQ_SSWIM_IDLE))
		{
			// We're completely under the water.

			PlayerAnimSetLowerSeq(playerinfo, ASEQ_USWIM_IDLE);
		}
		else if (playerinfo->waterlevel >= 2)
		{	
			// We're now in water, go to the appropriate water sequence. Waterseq here represents
			// the proper sequence to go to when ENTERING water.
			if (PlayerSeqData2[playerinfo->lowerseq].waterseq != ASEQ_NONE)
				PlayerAnimSetLowerSeq(playerinfo, PlayerSeqData2[playerinfo->lowerseq].waterseq);
		}
	}

	if (playerinfo->remember_buttons & BUTTON_DEFEND)
	{
		// Not a chicken, so...

		if(!playerinfo->isclient && playerinfo->pers.defence)
		{
			if(Defence_CurrentShotsLeft(playerinfo, 0)>0)
			{
				playerinfo->PlayerActionSpellDefensive(playerinfo);
			}
			else
			{
				//Play a sound to tell the player they're out of mana
				if(playerinfo->isclient)
					playerinfo->CL_Sound(SND_PRED_ID50,playerinfo->origin, CHAN_VOICE, "*nomana.wav", 0.75, ATTN_NORM, 0);
				else
					playerinfo->G_Sound(SND_PRED_ID50,playerinfo->leveltime,playerinfo->self, CHAN_VOICE, playerinfo->G_SoundIndex("*nomana.wav"), 0.75, ATTN_NORM, 0);
			}
		}

		playerinfo->remember_buttons &= ~BUTTON_DEFEND;
	}

	if(!playerinfo->isclient)
	{
		// Check to see if the lightning shield is engaged.

		if (playerinfo->shield_timer > playerinfo->leveltime)
			playerinfo->G_PlayerSpellShieldAttack(playerinfo);
		else
			playerinfo->G_PlayerSpellStopShieldAttack(playerinfo);
	}
}

// This function should be called anytime the player's skin, armor, weapon, damaged parts, etc are changed.
PLAYER_API void PlayerUpdateModelAttributes(playerinfo_t *playerinfo)
{//FIXME: make sure to see if you HAVE the weapon node you turn off (dropped weapons)
	int			i;
	qboolean	ghost,inverttex;

	assert(playerinfo);

	// if we are chicken, we shouldn't be doing any of this stuff
	if(playerinfo->edictflags & FL_CHICKEN)
		return;

	// Start by setting all the attached weapon types.
	// Check bow for existence and current ammo
	// Until later in this function, have the bow default to on the player's back if existent, gone if not.

	// Since his left hand is not holding a bow as a default, turn it on.
	playerinfo->fmnodeinfo[MESH__LHANDHI].flags |= FMNI_NO_DRAW;
	playerinfo->fmnodeinfo[MESH__BOWACTV].flags |= FMNI_NO_DRAW;

	switch(playerinfo->pers.bowtype)
	{
		case BOW_TYPE_REDRAIN:
			
			playerinfo->fmnodeinfo[MESH__BOFF].flags &= ~FMNI_NO_DRAW;

			// No special texture.

			playerinfo->pers.altparts &= ~((1<<MESH__BOWACTV) | (1<<MESH__BOFF));
			
			break;
		
		case BOW_TYPE_PHOENIX:
			
			playerinfo->fmnodeinfo[MESH__BOFF].flags &= ~FMNI_NO_DRAW;
			playerinfo->pers.altparts |= ((1<<MESH__BOWACTV) | (1<<MESH__BOFF));
			
			break;
		
		case BOW_TYPE_NONE:	
		default:
			
			playerinfo->fmnodeinfo[MESH__BOFF].flags |= FMNI_NO_DRAW;
			
			break;
	}


	
	// Check staff for powerup.
	// Until later in this function, have the staff default to on the player's belt.
	
	playerinfo->fmnodeinfo[MESH__RHANDHI].flags |= FMNI_NO_DRAW;
	playerinfo->fmnodeinfo[MESH__STAFACTV].flags |= FMNI_NO_DRAW;
	playerinfo->fmnodeinfo[MESH__BLADSTF].flags |= FMNI_NO_DRAW;
	
	if (!(playerinfo->flags & PLAYER_FLAG_NO_LARM))
		playerinfo->fmnodeinfo[MESH__STOFF].flags &= ~FMNI_NO_DRAW;

	switch(playerinfo->pers.stafflevel)
	{	
		case STAFF_LEVEL_POWER1:
		case STAFF_LEVEL_POWER2:
			
			// Use alternate power texture...
			
			playerinfo->pers.altparts |= ((1<<MESH__STAFACTV) | (1<<MESH__BLADSTF) | (1<<MESH__STOFF));
			
			break;
		
		case STAFF_LEVEL_BASIC:
		default:
			
			// No special texture
			
			playerinfo->pers.altparts &= ~((1<<MESH__STAFACTV) | (1<<MESH__BLADSTF) | (1<<MESH__STOFF));
		
			break;
	}

	// Check hellstaff for powerup.
	// Assume the hellstaff is currently not readied.

	playerinfo->fmnodeinfo[MESH__HELSTF].flags |= FMNI_NO_DRAW;

	switch(playerinfo->pers.helltype)
	{
		case HELL_TYPE_POWER:
			
			// Use alternate power texutre...
			
			playerinfo->pers.altparts |= (1<<MESH__HELSTF);
			
			break;

		case HELL_TYPE_BASIC:
			
			playerinfo->pers.altparts &= ~(1<<MESH__HELSTF);
			
			break;
		
		case HELL_TYPE_NONE:
		default:
		
			break;
	}
		
	// Check if the player's a ghost.

	if (playerinfo->ghost_timer > playerinfo->leveltime)
	{	
		// Set the ghost time.
		
		playerinfo->renderfx |= RF_TRANS_GHOST;
		
		ghost = true;

	}
	else
	{
		playerinfo->renderfx &= ~RF_TRANS_GHOST;
		
		ghost = false;
	}
	
	// Check armor and level...

	switch(playerinfo->pers.armortype)
	{
		case ARMOR_TYPE_SILVER:
			playerinfo->fmnodeinfo[MESH__ARMOR].flags &= ~FMNI_NO_DRAW;
			playerinfo->pers.altparts &= ~(1<<MESH__ARMOR);
			
			break;

		case ARMOR_TYPE_GOLD:
			playerinfo->fmnodeinfo[MESH__ARMOR].flags |= FMNI_USE_SKIN;
			playerinfo->fmnodeinfo[MESH__ARMOR].flags &= ~FMNI_NO_DRAW;
			playerinfo->pers.altparts |= 1<<MESH__ARMOR;
			if (playerinfo->skinnum & 0x01)	// If the main skinnum is odd, then opposite
				playerinfo->fmnodeinfo[MESH__ARMOR].skin = playerinfo->skinnum;
			else
				playerinfo->fmnodeinfo[MESH__ARMOR].skin = playerinfo->skinnum+1;
			
			break;

		case ARMOR_TYPE_NONE:
		default:
			
			playerinfo->fmnodeinfo[MESH__ARMOR].flags |= FMNI_NO_DRAW;
			
			break;
	}
	
	// First get the proper skin and set it.
	// The reflection setting is very important to this.

	if (playerinfo->reflect_timer > playerinfo->leveltime)
	{	
		// We are reflective.
		playerinfo->skinnum = SKIN_REFLECTION;
		playerinfo->renderfx |= RF_REFLECTION;
	 
		// No pain or power skins if alttex (metal texture).
		// Also, make sure that the alternate skin is not used when the reflection map is on.

		for (i=1; i<16; i++)
		{
			playerinfo->fmnodeinfo[i].flags &= ~FMNI_USE_SKIN;
		}
	}
	else
	{
		playerinfo->renderfx &= ~RF_REFLECTION;

		// Set normal skin texture.
		// First check if the first "node" is damaged, because it is an exception to the rest.

		if (playerinfo->pers.altparts & (1<<MESH_BASE2))
		{	
			// The front of the body is damaged.
			// This is a little weird, because the player's main skin is what defines the damage to the front chest node.
			// Hence if the chest front is damaged, then the *default* skin becomes damaged, and all the *non* damaged skins are exlusions.
			
			//all the others will use this playerinfo->skinnum if damaged, playerinfo->skinnum - 1 if not
			inverttex = true;

			//set the main skin (and node 0) to damaged skin
//			playerinfo->skinnum = (playerinfo->plaguelevel * DAMAGE_NUM_LEVELS) + 1;
			// We now don't set the skinnum to the plague level...  It is up to the clientinfo to set up the right plague skin.
			playerinfo->skinnum = 1;
		}
		else
		{	
			// Set the normal skin level.

			//all the others will use this playerinfo->skinnum + 1 if damaged, playerinfo->skinnum if not
			inverttex = false;

			//set the main skin (and node 0) to damaged skin
//			playerinfo->skinnum = (playerinfo->plaguelevel * DAMAGE_NUM_LEVELS);
			// We now don't set the skinnum to the plague level...  It is up to the clientinfo to set up the right plague skin.
			playerinfo->skinnum = 0;
		}

		//set node 0 to same skin as whole model
		playerinfo->fmnodeinfo[MESH_BASE2].skin = playerinfo->skinnum;

		// Set appropriate textures and pain skins.
		// Fifteen other body parts.

		for (i=1; i<16; i++)
		{
			if (playerinfo->pers.altparts & (1<<i))
			{
				// The part is damaged or powered.

				if (!inverttex)
				{
					playerinfo->fmnodeinfo[i].flags |= FMNI_USE_SKIN;
					playerinfo->fmnodeinfo[i].skin = playerinfo->skinnum + 1;
				}
				else
				{
					// The damaged skin is a default.

					playerinfo->fmnodeinfo[i].flags &= ~FMNI_USE_SKIN;
				}
			}
			else
			{
				// The part is not damaged or powered.

				if (!inverttex)
				{
					// The undamaged skin is a default.

					playerinfo->fmnodeinfo[i].flags &= ~FMNI_USE_SKIN;
				}
				else
				{
					playerinfo->fmnodeinfo[i].flags |= FMNI_USE_SKIN;
					playerinfo->fmnodeinfo[i].skin = playerinfo->skinnum - 1;
				}
			}
		}
	}

	//If the switch is valid
	if (BranchCheckDismemberAction(playerinfo, playerinfo->pers.weapon->tag))
	{//FIXME: doesn't allow for dropping of weapons
		// Now turn on the appropriate weapon bits.
		switch(playerinfo->pers.weaponready)
		{
			case WEAPON_READY_STAFFSTUB:
				
				// Staff in right hand.
				if (!(playerinfo->flags & PLAYER_FLAG_NO_RARM))
				{
					playerinfo->fmnodeinfo[MESH__STOFF].flags |= FMNI_NO_DRAW;
					playerinfo->fmnodeinfo[MESH__STAFACTV].flags &= ~FMNI_NO_DRAW;
				}
				
				// Empty left hand.
				if (!(playerinfo->flags & PLAYER_FLAG_NO_LARM))
					playerinfo->fmnodeinfo[MESH__LHANDHI].flags &= ~FMNI_NO_DRAW;
				
				break;
			
			case WEAPON_READY_SWORDSTAFF:
				
				// Staff in right hand.
				if (!(playerinfo->flags & PLAYER_FLAG_NO_RARM))
				{
					playerinfo->fmnodeinfo[MESH__STOFF].flags |= FMNI_NO_DRAW;
					playerinfo->fmnodeinfo[MESH__BLADSTF].flags &= ~FMNI_NO_DRAW;
					playerinfo->fmnodeinfo[MESH__STAFACTV].flags &= ~FMNI_NO_DRAW;
				}
				
				// Empty left hand.
				if (!(playerinfo->flags & PLAYER_FLAG_NO_LARM))
					playerinfo->fmnodeinfo[MESH__LHANDHI].flags &= ~FMNI_NO_DRAW;
/*
				if (playerinfo->leveltime > 1.0)
				{
					if(!playerinfo->isclient)
						playerinfo->G_CreateEffect(playerinfo->G_GetEntityStatePtr(playerinfo->self),
												   FX_STAFF_CREATEPOOF,
												   CEF_OWNERS_ORIGIN,
												   NULL,
												   "");
				}
*/
				break;

			case WEAPON_READY_HELLSTAFF:
				
				// Staff in right hand.
				if (!(playerinfo->flags & PLAYER_FLAG_NO_RARM))
				{
					playerinfo->fmnodeinfo[MESH__STOFF].flags |= FMNI_NO_DRAW;
					playerinfo->fmnodeinfo[MESH__HELSTF].flags &= ~FMNI_NO_DRAW;
					playerinfo->fmnodeinfo[MESH__STAFACTV].flags &= ~FMNI_NO_DRAW;
				}
				
				// Empty left hand.
				if (!(playerinfo->flags & PLAYER_FLAG_NO_LARM))
					playerinfo->fmnodeinfo[MESH__LHANDHI].flags &= ~FMNI_NO_DRAW;
/*
				if (playerinfo->leveltime > 1.0)
				{
					if(!playerinfo->isclient)
						playerinfo->G_CreateEffect(playerinfo->G_GetEntityStatePtr(playerinfo->self),
												   FX_STAFF_CREATEPOOF,
												   CEF_OWNERS_ORIGIN|CEF_FLAG6,
												   NULL,
												   "");
				}
				*/
				
				break;

			case WEAPON_READY_BOW:
				
				// Empty right hand.
				if (!(playerinfo->flags & PLAYER_FLAG_NO_RARM))
					playerinfo->fmnodeinfo[MESH__RHANDHI].flags &= ~FMNI_NO_DRAW;
				
				// Bow in left hand.
				if (!(playerinfo->flags & PLAYER_FLAG_NO_LARM))
				{
					playerinfo->fmnodeinfo[MESH__BOFF].flags |= FMNI_NO_DRAW;
					playerinfo->fmnodeinfo[MESH__BOWACTV].flags &= ~FMNI_NO_DRAW;
				}
				
				break;

			case WEAPON_READY_HANDS:
			default:
				
				// Empty right hand.
				if (!(playerinfo->flags & PLAYER_FLAG_NO_RARM))
					playerinfo->fmnodeinfo[MESH__RHANDHI].flags &= ~FMNI_NO_DRAW;
				
				// Empty left hand.
				if (!(playerinfo->flags & PLAYER_FLAG_NO_LARM))
					playerinfo->fmnodeinfo[MESH__LHANDHI].flags &= ~FMNI_NO_DRAW;
				
				break;
			
		}
	}
}

void PlayerSetHandFX(playerinfo_t *playerinfo, int handfx, int lifetime)
{
	int powerlevel;
/*	
	// FIXME: This could be bad...
	
	if(playerinfo->handfxtype==handfx)
	{	
		// There should be nothing to do.

		return;
	}
*/	
	// Check currently in place effects.

/*	switch(playerinfo->pers.handfxtype)
	{	
		case HANDFX_FIREBALL:
		case HANDFX_MISSILE:
		case HANDFX_MACEBALL:
		case HANDFX_FIREWALL:
		case HANDFX_STAFF1:
		case HANDFX_STAFF2:
		case HANDFX_STAFF3:
			// Do nothing, these effects end on their own anyway.
			break;
			
		case HANDFX_SPHERE:
			if(playerinfo->effects)
				if(!playerinfo->isclient)
					playerinfo->G_RemoveEffects(EFFECT_PRED_ID31,
												playerinfo->G_GetEntityStatePtr(playerinfo->self),
												FX_SPELLHANDS);
				else
					playerinfo->CL_RemoveEffects(EFFECT_PRED_ID31,
												 playerinfo->self,
												 FX_SPELLHANDS);
			break;

		case HANDFX_REDRAIN:
		case HANDFX_POWERREDRAIN:
			
			if(playerinfo->effects)
				if(!playerinfo->isclient)
					playerinfo->G_RemoveEffects(EFFECT_PRED_ID32,
												playerinfo->G_GetEntityStatePtr(playerinfo->self),
												FX_WEAPON_REDRAINGLOW);
				else
					playerinfo->CL_RemoveEffects(EFFECT_PRED_ID32,
												 playerinfo->self,
												 FX_WEAPON_REDRAINGLOW);
			break;
		
		case HANDFX_PHOENIX:
		case HANDFX_POWERPHOENIX:
		
			if(playerinfo->effects)
				if(!playerinfo->isclient)
					playerinfo->G_RemoveEffects(EFFECT_PRED_ID33,
												playerinfo->G_GetEntityStatePtr(playerinfo->self),
												FX_FIREHANDS);
				else
					playerinfo->CL_RemoveEffects(EFFECT_PRED_ID33,
												 playerinfo->self,
												 FX_FIREHANDS);
			break;
		
		case HANDFX_NONE:
		default:

			// Nothing to remove.
		
			break;
	}
*/

	// To kill previous effects, we just Reset the EF_FLAG

	// Now start the appropriate hand effect on the hand effect -
	// CEF_FLAG6 = 1 = do both left and right hand, else just do right hand.
	// CEF_FLAG7 & 8 = spell color type.
	// 0 = red, 2 = green, 1 = blue 3 = yellow.

	playerinfo->effects &= ~EF_TRAILS_ENABLED;
	playerinfo->pers.handfxtype=handfx;

	switch(handfx)
	{
		case HANDFX_FIREBALL:
			// Red effect on the right throwing hand.
			if (lifetime == 0)
				lifetime = 4;		// .4 seconds is normal fireball throw time.
			if(!playerinfo->isclient)
				playerinfo->G_CreateEffect(EFFECT_PRED_ID16,
										   playerinfo->G_GetEntityStatePtr(playerinfo->self),
										   FX_SPELLHANDS,
										   CEF_OWNERS_ORIGIN,
										   NULL,
										   "b",
										   (byte)lifetime);
			else
				playerinfo->CL_CreateEffect(EFFECT_PRED_ID16,
											playerinfo->self,
										    FX_SPELLHANDS,
										    CEF_OWNERS_ORIGIN,
										    NULL,
										    "b",
										    (byte)lifetime);
			break;

		case HANDFX_MISSILE:
			// Green effect on the right throwing hand.
			if (lifetime == 0)
				lifetime = 6;		// .6 seconds is normal fireball throw time
			if(!playerinfo->isclient)
				playerinfo->G_CreateEffect(EFFECT_PRED_ID17,
										   playerinfo->G_GetEntityStatePtr(playerinfo->self),
										   FX_SPELLHANDS,
										   CEF_OWNERS_ORIGIN|CEF_FLAG8,
										   NULL,
										   "b",
										   (byte)lifetime);
			else
				playerinfo->CL_CreateEffect(EFFECT_PRED_ID17,
											playerinfo->self,
											FX_SPELLHANDS,
											CEF_OWNERS_ORIGIN|CEF_FLAG8,
											NULL,
											"b",
											(byte)lifetime);
			break;
		
		case HANDFX_FIREWALL:
			if (lifetime == 0)
				lifetime = 11;		// 1.1 seconds is normal fireball throw time
			if(!playerinfo->isclient)
				playerinfo->G_CreateEffect(EFFECT_PRED_ID19,
										   playerinfo->G_GetEntityStatePtr(playerinfo->self),
										   FX_FIREHANDS,
										   CEF_OWNERS_ORIGIN|CEF_FLAG6,
										   NULL,
										   "b",
										   lifetime);
			else
				playerinfo->CL_CreateEffect(EFFECT_PRED_ID19,
											playerinfo->self,
											FX_FIREHANDS,
											CEF_OWNERS_ORIGIN|CEF_FLAG6,
											NULL,
											"b",
											lifetime);
			break;
		
		case HANDFX_STAFF1:
		case HANDFX_STAFF2:
		case HANDFX_STAFF3:
			playerinfo->effects &= ~EF_BLOOD_ENABLED;
			// Add a trail effect to the staff.
			if (playerinfo->powerup_timer > playerinfo->leveltime)
				powerlevel = playerinfo->pers.stafflevel+1;
			else
				powerlevel = playerinfo->pers.stafflevel;

			if (powerlevel >= STAFF_LEVEL_MAX)
				powerlevel = STAFF_LEVEL_MAX-1;

			if(!playerinfo->isclient)
			{
				playerinfo->G_CreateEffect(EFFECT_PRED_ID20,
										   playerinfo->G_GetEntityStatePtr(playerinfo->self),
										   FX_STAFF,
										   CEF_OWNERS_ORIGIN,
										   NULL,
										   "bb",
										   (byte)powerlevel,
										   (byte)lifetime);
			}
			else
			{
				playerinfo->CL_CreateEffect(EFFECT_PRED_ID20,
											playerinfo->self,
											FX_STAFF,
											CEF_OWNERS_ORIGIN,
											NULL,
											"bb",
											(byte)powerlevel,
											(byte)lifetime);
			}
			break;
		
		case HANDFX_SPHERE:
			// Blue effect on both hands.
			if (lifetime == 0)
				lifetime = 8;
			playerinfo->effects |= EF_TRAILS_ENABLED;		// Set up for hand trails
			if(!playerinfo->isclient)
				playerinfo->G_CreateEffect(EFFECT_PRED_ID18,
										   playerinfo->G_GetEntityStatePtr(playerinfo->self),
										   FX_SPELLHANDS,
										   CEF_OWNERS_ORIGIN|CEF_FLAG6|CEF_FLAG7,
										   NULL,
										   "b",
										   -1);
			else
				playerinfo->CL_CreateEffect(EFFECT_PRED_ID18,
											playerinfo->self,
											FX_SPELLHANDS,
											CEF_OWNERS_ORIGIN|CEF_FLAG6|CEF_FLAG7,
											NULL,
											"b",
											-1);
			break;

		case HANDFX_REDRAIN:
			playerinfo->effects |= EF_TRAILS_ENABLED;		// Set up for hand trails
			if(!playerinfo->isclient)
			{
				playerinfo->G_CreateEffect(EFFECT_PRED_ID21,
										   playerinfo->G_GetEntityStatePtr(playerinfo->self),
										   FX_WEAPON_REDRAINGLOW,
										   CEF_OWNERS_ORIGIN,
										   NULL,
										   "b",
										   -1);
			}
			else
			{
				playerinfo->CL_CreateEffect(EFFECT_PRED_ID21,
											playerinfo->self,
											FX_WEAPON_REDRAINGLOW,
											CEF_OWNERS_ORIGIN,
											NULL,
											"b",
											-1);
			}
			break;
		
		case HANDFX_POWERREDRAIN:
			playerinfo->effects |= EF_TRAILS_ENABLED;		// Set up for hand trails
			if(!playerinfo->isclient)
				playerinfo->G_CreateEffect(EFFECT_PRED_ID22,
										   playerinfo->G_GetEntityStatePtr(playerinfo->self),
										   FX_WEAPON_REDRAINGLOW,
										   CEF_OWNERS_ORIGIN | CEF_FLAG6,
										   NULL,
										   "b",
										   -1);
			else
				playerinfo->CL_CreateEffect(EFFECT_PRED_ID22,
											playerinfo->self,
										    FX_WEAPON_REDRAINGLOW,
										    CEF_OWNERS_ORIGIN | CEF_FLAG6,
										    NULL,
										    "b",
										    -1);
			
			break;
		
		case HANDFX_PHOENIX:
			playerinfo->effects |= EF_TRAILS_ENABLED;		// Set up for hand trails
			if(!playerinfo->isclient)
				playerinfo->G_CreateEffect(EFFECT_PRED_ID23,
										   playerinfo->G_GetEntityStatePtr(playerinfo->self),
										   FX_FIREHANDS,
										   CEF_OWNERS_ORIGIN,
										   NULL,
										   "b",
										   -1);
			else
				playerinfo->CL_CreateEffect(EFFECT_PRED_ID23,
											playerinfo->self,
										    FX_FIREHANDS,
										    CEF_OWNERS_ORIGIN,
										    NULL,
										    "b",
										    -1);
			break;

		case HANDFX_POWERPHOENIX:
			playerinfo->effects |= EF_TRAILS_ENABLED;		// Set up for hand trails
			if(!playerinfo->isclient)
				playerinfo->G_CreateEffect(EFFECT_PRED_ID24,
										   playerinfo->G_GetEntityStatePtr(playerinfo->self),
										   FX_FIREHANDS,
										   CEF_OWNERS_ORIGIN,
										   NULL,
										   "b",
										   -1);
			else
				playerinfo->CL_CreateEffect(EFFECT_PRED_ID24,
											playerinfo->self,
										    FX_FIREHANDS,
										    CEF_OWNERS_ORIGIN,
										    NULL,
										    "b",
										    -1);			
			break;

		case HANDFX_MACEBALL:
			// Nothin' for these yet.
			break;
		
		case HANDFX_NONE:
		default:
		
			// Don't start anything.
		  
			break;
	}
}