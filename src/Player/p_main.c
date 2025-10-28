//
// p_main.c
//
// Copyright 1998 Raven Software
//

#include "Player.h"
#include "p_anim_branch.h"
#include "p_anim_data.h"
#include "p_anims.h"
#include "p_main.h"
#include "p_weapon.h"
#include "p_utility.h" //mxd
#include "FX.h"
#include "m_player.h"
#include "Vector.h"

PLAYER_API void P_Init(void) //mxd. Originally defined in main.c.
{
	InitItems();
}

PLAYER_API void P_Shutdown(void) { } //mxd. Originally defined in main.c.

PLAYER_API player_export_t GetPlayerAPI(void) //mxd. Originally defined in main.c.
{
	player_export_t pe;

	pe.PlayerSeqData = PlayerSeqData;
	pe.PlayerChickenData = PlayerChickenData;
	pe.p_num_items = p_num_items;
	pe.p_itemlist = p_itemlist;

	return pe;
}

PLAYER_API void PlayerInit(playerinfo_t* info, const int complete_reset)
{
	if (complete_reset)
		PlayerAnimReset(info);
	else
		PlayerBasicAnimReset(info);

	info->flags = PLAYER_FLAG_NONE;
}

// Remove all special effects from the player.
PLAYER_API void PlayerClearEffects(const playerinfo_t* info)
{
	P_RemoveEffects(info, EFFECT_PRED_ID30, FX_REMOVE_EFFECTS); //mxd
}

PLAYER_API void PlayerUpdateCmdFlags(playerinfo_t* info)
{
	const usercmd_t* pcmd = &info->pcmd;

	// Look for the attack button being pressed.
	info->seqcmd[ACMDU_ATTACK] = (pcmd->buttons & BUTTON_ATTACK);

	//mxd. Look for the defend button being pressed.
	info->seqcmd[ACMDU_DEFSPELL] = (pcmd->buttons & BUTTON_DEFEND);

	// Look for the action button being pressed.
	info->seqcmd[ACMDL_ACTION] = (pcmd->buttons & BUTTON_ACTION);

	// Look for the quickturn button being pressed.
	info->seqcmd[ACMDL_QUICKTURN] = (pcmd->buttons & BUTTON_QUICKTURN);

	// Look for the jump / crouch buttons being pressed.
	info->seqcmd[ACMDL_JUMP] = (pcmd->upmove > 0);
	info->seqcmd[ACMDL_CROUCH] = (pcmd->upmove < 0);

	// Look for the turn left / turn right buttons being pressed.
	info->seqcmd[ACMDL_ROTATE_R] = (info->turncmd < -2 ? info->loweridle : false);
	info->seqcmd[ACMDL_ROTATE_L] = (info->turncmd >  2 ? info->loweridle : false);

	info->turncmd = 0;

	// Look for the autoaim button being pressed.
	info->autoaim = (pcmd->buttons & BUTTON_AUTOAIM);

	// Clear out ALL forward/backward movement flags.
	memset(&(info->seqcmd[ACMDL_CREEP_F]), 0, (ACMDL_BACK - ACMDL_CREEP_F + 1) * sizeof(int));

	// Look for forward/backpedal buttons being pressed.
	if (pcmd->forwardmove > 10)
	{
		info->seqcmd[ACMDL_FWD] = true;

		if (pcmd->buttons & BUTTON_CREEP)
			info->seqcmd[ACMDL_CREEP_F] = true;
		else if (pcmd->buttons & BUTTON_RUN)
			info->seqcmd[ACMDL_RUN_F] = true;
		else
			info->seqcmd[ACMDL_WALK_F] = true;
	}
	else if (pcmd->forwardmove < -10)
	{
		info->seqcmd[ACMDL_BACK] = true;

		if (pcmd->buttons & BUTTON_CREEP)
			info->seqcmd[ACMDL_CREEP_B] = true;
		else if (pcmd->buttons & BUTTON_RUN)
			info->seqcmd[ACMDL_RUN_B] = true;
		else
			info->seqcmd[ACMDL_WALK_B] = true;
	}

	// Look for strafe buttons being pressed.
	info->seqcmd[ACMDL_STRAFE_R] = (pcmd->sidemove > 2);
	info->seqcmd[ACMDL_STRAFE_L] = (pcmd->sidemove < -2);

	// Look for Inventory button being pressed.
	info->showpuzzleinventory = (pcmd->buttons & BUTTON_INVENTORY);

	info->fwdvel = 0.0f;
	info->sidevel = 0.0f;
	info->upvel = 0.0f;
}

PLAYER_API void PlayerUpdate(playerinfo_t* info)
{
	if (info->deadflag == DEAD_DEAD || info->deadflag == DEAD_DYING)
		return;

	vec3_t endpos;
	VectorCopy(info->origin, endpos);
	endpos[2] += info->mins[2];

	if (!(info->PointContents(endpos) & (CONTENTS_SLIME | CONTENTS_LAVA)))
	{
		// At the very first point, evaluate whether we are in a water or air sequence, and then whether the player is in water or air.
		if (info->waterlevel > 2 && PlayerSeqData2[info->lowerseq].waterseq == ASEQ_SSWIM_IDLE)
		{
			// Switch from surface swimming to underwater swimming animation.
			PlayerAnimSetLowerSeq(info, ASEQ_USWIM_IDLE);
		}
		else if (PlayerSeqData2[info->lowerseq].waterseq != ASEQ_NONE)
		{
			const qboolean in_water = (PlayerSeqData[info->lowerseq].playerflags & PLAYER_FLAG_WATER); //mxd

			// Check if we are leaving or entering water.
			const qboolean entering_water = (!in_water && info->waterlevel >= 2); //mxd
			const qboolean leaving_water = (in_water && (info->waterlevel < 1 || (info->waterlevel < 2 && info->groundentity != NULL))); //mxd

			// Waterseq here represents the proper sequence to go to when leaving or entering water.
			if (entering_water || leaving_water)
				PlayerAnimSetLowerSeq(info, PlayerSeqData2[info->lowerseq].waterseq);
		}
	}

	if (info->remember_buttons & BUTTON_DEFEND)
	{
		// Not a chicken, so... //TODO: but there are no chicken checks in sight?
		if (!info->isclient && info->pers.defence != NULL && !(info->flags & PLAYER_FLAG_ONROPE)) //mxd. +PLAYER_FLAG_ONROPE check.
		{
			if (Defence_CurrentShotsLeft(info, 0) > 0)
				info->PlayerActionSpellDefensive(info);
			else
				P_Sound(info, SND_PRED_ID50, CHAN_VOICE, "*nomana.wav", 0.75f); //mxd // Play a sound to tell the player they're out of mana.
		}

		info->remember_buttons &= ~BUTTON_DEFEND;
	}

	if (!info->isclient)
	{
		// Check to see if the lightning shield is engaged.
		if (info->shield_timer > info->leveltime)
			info->G_PlayerSpellShieldAttack(info);
		else
			info->G_PlayerSpellStopShieldAttack(info);
	}
}

// This function should be called anytime the player's skin, armor, weapon, damaged parts, etc. are changed.
PLAYER_API void PlayerUpdateModelAttributes(playerinfo_t* info)
{
	//FIXME: make sure to see if you HAVE the weapon node you turn off (dropped weapons)
	assert(info);

	// If we are chicken, we shouldn't be doing any of this stuff.
	if (info->edictflags & FL_CHICKEN)
		return;

	// Start by setting all the attached weapon types.
	// Check bow for existence and current ammo.
	// Until later in this function, have the bow default to on the player's back if existent, gone if not.

	// Since his left hand is not holding a bow as a default, turn it on.
	info->fmnodeinfo[MESH__LHANDHI].flags |= FMNI_NO_DRAW;
	info->fmnodeinfo[MESH__BOWACTV].flags |= FMNI_NO_DRAW;

	switch (info->pers.bowtype)
	{
		case BOW_TYPE_REDRAIN:
			info->fmnodeinfo[MESH__BOFF].flags &= ~FMNI_NO_DRAW;
			info->pers.altparts &= ~((1 << MESH__BOWACTV) | (1 << MESH__BOFF)); // No special texture.
			break;

		case BOW_TYPE_PHOENIX:
			info->fmnodeinfo[MESH__BOFF].flags &= ~FMNI_NO_DRAW;
			info->pers.altparts |= ((1 << MESH__BOWACTV) | (1 << MESH__BOFF));
			break;

		case BOW_TYPE_NONE:
		default:
			info->fmnodeinfo[MESH__BOFF].flags |= FMNI_NO_DRAW;
			break;
	}

	// Check staff for powerup.
	// Until later in this function, have the staff default to on the player's belt.
	info->fmnodeinfo[MESH__RHANDHI].flags |= FMNI_NO_DRAW;
	info->fmnodeinfo[MESH__STAFACTV].flags |= FMNI_NO_DRAW;
	info->fmnodeinfo[MESH__BLADSTF].flags |= FMNI_NO_DRAW;

	if (!(info->flags & PLAYER_FLAG_NO_LARM))
		info->fmnodeinfo[MESH__STOFF].flags &= ~FMNI_NO_DRAW;

	switch (info->pers.stafflevel)
	{
		case STAFF_LEVEL_POWER1:
		case STAFF_LEVEL_POWER2:
			info->pers.altparts |= ((1 << MESH__STAFACTV) | (1 << MESH__BLADSTF) | (1 << MESH__STOFF)); // Use alternate power texture...
			break;

		case STAFF_LEVEL_BASIC:
		default:
			info->pers.altparts &= ~((1 << MESH__STAFACTV) | (1 << MESH__BLADSTF) | (1 << MESH__STOFF)); // No special texture
			break;
	}

	// Check hellstaff for powerup.
	// Assume the hellstaff is currently not readied.
	info->fmnodeinfo[MESH__HELSTF].flags |= FMNI_NO_DRAW;

	switch (info->pers.helltype)
	{
		case HELL_TYPE_POWER:
			info->pers.altparts |= (1 << MESH__HELSTF); // Use alternate power texutre...
			break;

		case HELL_TYPE_BASIC:
			info->pers.altparts &= ~(1 << MESH__HELSTF);
			break;

		case HELL_TYPE_NONE:
		default:
			break;
	}

	// Check if the player's a ghost.
	if (info->ghost_timer > info->leveltime)
		info->renderfx |= RF_TRANS_GHOST;
	else
		info->renderfx &= ~RF_TRANS_GHOST;

	// Check armor and level...
	switch (info->pers.armortype)
	{
		case ARMOR_TYPE_SILVER:
			info->fmnodeinfo[MESH__ARMOR].flags &= ~FMNI_NO_DRAW;
			info->pers.altparts &= ~(1 << MESH__ARMOR);
			break;

		case ARMOR_TYPE_GOLD:
			info->fmnodeinfo[MESH__ARMOR].flags |= FMNI_USE_SKIN;
			info->fmnodeinfo[MESH__ARMOR].flags &= ~FMNI_NO_DRAW;
			info->pers.altparts |= 1 << MESH__ARMOR;
			info->fmnodeinfo[MESH__ARMOR].skin = info->skinnum + ((info->skinnum & 1) ? 0 : 1); // If the main skinnum is odd, then opposite.
			break;

		case ARMOR_TYPE_NONE:
		default:
			info->fmnodeinfo[MESH__ARMOR].flags |= FMNI_NO_DRAW;
			break;
	}

	// First get the proper skin and set it.
	// The reflection setting is very important to this.
	if (info->reflect_timer > info->leveltime)
	{
		// We are reflective.
		info->skinnum = SKIN_REFLECTION;
		info->renderfx |= RF_REFLECTION;

		// No pain or power skins if alttex (metal texture).
		// Also, make sure that the alternate skin is not used when the reflection map is on.
		for (int i = 1; i < 16; i++)
			info->fmnodeinfo[i].flags &= ~FMNI_USE_SKIN;
	}
	else
	{
		// We are not reflective.
		info->renderfx &= ~RF_REFLECTION;
		qboolean inverttex;

		// Set normal skin texture.
		// First check if the first "node" is damaged, because it is an exception to the rest.
		if (info->pers.altparts & (1 << MESH_BASE2))
		{
			// The front of the body is damaged.
			// This is a little weird, because the player's main skin is what defines the damage to the front chest node.
			// Hence if the chest front is damaged, then the *default* skin becomes damaged, and all the *non* damaged skins are exclusions.

			// All the others will use this playerinfo->skinnum if damaged, playerinfo->skinnum - 1 if not.
			inverttex = true;

			// We now don't set the skinnum to the plague level... It is up to the clientinfo to set up the right plague skin.
			info->skinnum = 1;
		}
		else
		{
			// Set the normal skin level.

			// All the others will use this playerinfo->skinnum + 1 if damaged, playerinfo->skinnum if not.
			inverttex = false;

			// We now don't set the skinnum to the plague level... It is up to the clientinfo to set up the right plague skin.
			info->skinnum = 0;
		}

		// Set node 0 to same skin as whole model.
		info->fmnodeinfo[MESH_BASE2].skin = info->skinnum;

		// Set appropriate textures and pain skins for fifteen other body parts.
		for (int i = 1; i < 16; i++)
		{
			if (info->pers.altparts & (1 << i))
			{
				// The part is damaged or powered.
				if (!inverttex)
				{
					info->fmnodeinfo[i].flags |= FMNI_USE_SKIN;
					info->fmnodeinfo[i].skin = info->skinnum + 1;
				}
				else
				{
					// The damaged skin is a default.
					info->fmnodeinfo[i].flags &= ~FMNI_USE_SKIN;
				}
			}
			else
			{
				// The part is not damaged or powered.
				if (!inverttex)
				{
					// The undamaged skin is a default.
					info->fmnodeinfo[i].flags &= ~FMNI_USE_SKIN;
				}
				else
				{
					info->fmnodeinfo[i].flags |= FMNI_USE_SKIN;
					info->fmnodeinfo[i].skin = info->skinnum - 1;
				}
			}
		}
	}

	// If the switch is valid.
	if (!BranchCheckDismemberAction(info, info->pers.weapon->tag))
		return;

	//FIXME: doesn't allow for dropping of weapons.
	// Now turn on the appropriate weapon bits.
	switch (info->pers.weaponready)
	{
		case WEAPON_READY_STAFFSTUB:
			// Staff in right hand.
			if (!(info->flags & PLAYER_FLAG_NO_RARM))
			{
				info->fmnodeinfo[MESH__STOFF].flags |= FMNI_NO_DRAW;
				info->fmnodeinfo[MESH__STAFACTV].flags &= ~FMNI_NO_DRAW;
			}

			// Empty left hand.
			if (!(info->flags & PLAYER_FLAG_NO_LARM))
				info->fmnodeinfo[MESH__LHANDHI].flags &= ~FMNI_NO_DRAW;
			break;

		case WEAPON_READY_SWORDSTAFF:
			// Staff in right hand.
			if (!(info->flags & PLAYER_FLAG_NO_RARM))
			{
				info->fmnodeinfo[MESH__STOFF].flags |= FMNI_NO_DRAW;
				info->fmnodeinfo[MESH__BLADSTF].flags &= ~FMNI_NO_DRAW;
				info->fmnodeinfo[MESH__STAFACTV].flags &= ~FMNI_NO_DRAW;
			}

			// Empty left hand.
			if (!(info->flags & PLAYER_FLAG_NO_LARM))
				info->fmnodeinfo[MESH__LHANDHI].flags &= ~FMNI_NO_DRAW;
			break;

		case WEAPON_READY_HELLSTAFF:
			// Staff in right hand.
			if (!(info->flags & PLAYER_FLAG_NO_RARM))
			{
				info->fmnodeinfo[MESH__STOFF].flags |= FMNI_NO_DRAW;
				info->fmnodeinfo[MESH__HELSTF].flags &= ~FMNI_NO_DRAW;
				info->fmnodeinfo[MESH__STAFACTV].flags &= ~FMNI_NO_DRAW;
			}

			// Empty left hand.
			if (!(info->flags & PLAYER_FLAG_NO_LARM))
				info->fmnodeinfo[MESH__LHANDHI].flags &= ~FMNI_NO_DRAW;
			break;

		case WEAPON_READY_BOW:
			// Empty right hand.
			if (!(info->flags & PLAYER_FLAG_NO_RARM))
				info->fmnodeinfo[MESH__RHANDHI].flags &= ~FMNI_NO_DRAW;

			// Bow in left hand.
			if (!(info->flags & PLAYER_FLAG_NO_LARM))
			{
				info->fmnodeinfo[MESH__BOFF].flags |= FMNI_NO_DRAW;
				info->fmnodeinfo[MESH__BOWACTV].flags &= ~FMNI_NO_DRAW;
			}
			break;

		case WEAPON_READY_HANDS:
		default:
			// Empty right hand.
			if (!(info->flags & PLAYER_FLAG_NO_RARM))
				info->fmnodeinfo[MESH__RHANDHI].flags &= ~FMNI_NO_DRAW;

			// Empty left hand.
			if (!(info->flags & PLAYER_FLAG_NO_LARM))
				info->fmnodeinfo[MESH__LHANDHI].flags &= ~FMNI_NO_DRAW;
			break;
	}
}

void PlayerSetHandFX(playerinfo_t* info, const int handfxtype, int lifetime)
{
	// To kill previous effects, we just Reset the EF_FLAG.

	// Start the appropriate hand effect:
	// CEF_FLAG6 = 1 - do both left and right hand, else just do right hand.
	// CEF_FLAG7 & 8 - spell color type: 0 - red, 1 - blue, 2 - green, 3 - yellow.

	info->effects &= ~EF_TRAILS_ENABLED;
	info->pers.handfxtype = (byte)handfxtype;

	switch (handfxtype)
	{
		case HANDFX_FIREBALL:
			// Red effect on the right throwing hand.
			if (lifetime == 0)
				lifetime = 4; // 0.4 seconds is normal fireball throw time.
			P_CreateEffect(info, EFFECT_PRED_ID16, info->self, FX_SPELLHANDS, CEF_OWNERS_ORIGIN, NULL, "b", (byte)lifetime); //mxd
			break;

		case HANDFX_MISSILE:
			// Green effect on the right throwing hand.
			if (lifetime == 0)
				lifetime = 6; // 0.6 seconds is normal fireball throw time.
			P_CreateEffect(info, EFFECT_PRED_ID17, info->self, FX_SPELLHANDS, CEF_OWNERS_ORIGIN | CEF_FLAG8, NULL, "b", (byte)lifetime); //mxd
			break;

		case HANDFX_SPHERE:
			// Blue effect on both hands.
			info->effects |= EF_TRAILS_ENABLED; // Set up for hand trails.
			P_CreateEffect(info, EFFECT_PRED_ID18, info->self, FX_SPELLHANDS, CEF_OWNERS_ORIGIN | CEF_FLAG6 | CEF_FLAG7, NULL, "b", -1); //mxd
			break;

		case HANDFX_FIREWALL:
			if (lifetime == 0)
				lifetime = 11; // 1.1 seconds is normal fireball throw time.
			P_CreateEffect(info, EFFECT_PRED_ID19, info->self, FX_FIREHANDS, CEF_OWNERS_ORIGIN | CEF_FLAG6, NULL, "b", (byte)lifetime); //mxd
			break;

		case HANDFX_STAFF1:
		case HANDFX_STAFF2:
		case HANDFX_STAFF3:
			info->effects &= ~EF_BLOOD_ENABLED;
			// Add a trail effect to the staff.
			int powerlevel = info->pers.stafflevel + (info->powerup_timer > info->leveltime ? 1 : 0);
			powerlevel = min(powerlevel, STAFF_LEVEL_MAX - 1);
			P_CreateEffect(info, EFFECT_PRED_ID20, info->self, FX_STAFF, CEF_OWNERS_ORIGIN, NULL, "bb", (byte)powerlevel, (byte)lifetime); //mxd
			break;

		case HANDFX_REDRAIN:
			info->effects |= EF_TRAILS_ENABLED; // Set up for hand trails.
			P_CreateEffect(info, EFFECT_PRED_ID21, info->self, FX_WEAPON_REDRAINGLOW, CEF_OWNERS_ORIGIN, NULL, "b", -1); //mxd
			break;

		case HANDFX_POWERREDRAIN:
			info->effects |= EF_TRAILS_ENABLED; // Set up for hand trails.
			P_CreateEffect(info, EFFECT_PRED_ID22, info->self, FX_WEAPON_REDRAINGLOW, CEF_OWNERS_ORIGIN | CEF_FLAG6, NULL, "b", -1); //mxd
			break;

		case HANDFX_PHOENIX:
			info->effects |= EF_TRAILS_ENABLED; // Set up for hand trails.
			P_CreateEffect(info, EFFECT_PRED_ID23, info->self, FX_FIREHANDS, CEF_OWNERS_ORIGIN, NULL, "b", -1); //mxd
			break;

		case HANDFX_POWERPHOENIX:
			info->effects |= EF_TRAILS_ENABLED; // Set up for hand trails.
			P_CreateEffect(info, EFFECT_PRED_ID24, info->self, FX_FIREHANDS, CEF_OWNERS_ORIGIN, NULL, "b", -1); //mxd
			break;

		case HANDFX_MACEBALL: // Nothing for these yet.
		case HANDFX_NONE: // Don't start anything.
		default:
			break;
	}
}