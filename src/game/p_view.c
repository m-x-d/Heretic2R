#include "g_Skeletons.h"
#include "g_teleport.h"
#include "m_player.h"
//#include "p_types.h"
#include "player.h"
#include "p_animactor.h"
#include "p_anims.h"
#include "p_ctrl.h"
#include "p_funcs.h"
#include "p_main.h"
#include "p_weapon.h"
#include "angles.h"
#include "fx.h"
#include "random.h"
#include "vector.h"
#include "utilities.h"
#include "g_playstats.h"

static	edict_t		*current_player;
static	gclient_t	*current_client;
float				xyspeed;
float				bobmove;
int					bobcycle;		// odd cycles are right foot going forward
float				bobfracsin;		// sin(bobfrac*M_PI)

extern void Cmd_WeapPrev_f (edict_t *ent);
extern void Cmd_Use_f (edict_t *ent, char *s);
extern void PrintLocalBuoyInfo(vec3_t org);


// ** setup a looping sound on the client
void G_set_looping_sound(edict_t *self, int sound_num)
{
	self->s.sound = sound_num;
	self->s.sound_data = (255 & ENT_VOL_MASK) | ATTN_NORM;
}


// ************************************************************************************************
// ClientServerRand
// ----------------
// ************************************************************************************************

static int ClientServerRand(playerinfo_t *playerinfo,int mn,int mx)
{
	int t;

	if (mn>=mx)
		return(mn);

	t=(int)(playerinfo->leveltime*10.0f);
	t=(t>>7)^(t>>10)^(t>>5);
	t%=(1+mx-mn);

	return(t+mn);
}

// ************************************************************************************************
// InitPlayerinfo
// --------------
// ************************************************************************************************

void InitPlayerinfo(edict_t *ent)
{
	// Client side function callbacks (approximating functionality of server function callbacks).

	ent->client->playerinfo.CL_Sound=NULL;	
	ent->client->playerinfo.CL_Trace=NULL;
	ent->client->playerinfo.CL_CreateEffect=NULL;

	// Server (game) function callbacks (approximating functionality of client-side function callbacks).

	ent->client->playerinfo.G_L_Sound=G_set_looping_sound;
	ent->client->playerinfo.G_Sound=gi.soundevent;
	ent->client->playerinfo.G_Trace=gi.trace;
	ent->client->playerinfo.G_CreateEffect=gi.CreateEffectEvent;
	ent->client->playerinfo.G_RemoveEffects=gi.RemoveEffectsEvent;

	// Server (game) function callbacks that have no client side equivalent.

	ent->client->playerinfo.G_SoundIndex=gi.soundindex;	
	ent->client->playerinfo.G_SoundRemove=gi.soundremove;	
	ent->client->playerinfo.G_UseTargets=G_UseTargets;
	ent->client->playerinfo.G_GetEntityStatePtr=G_GetEntityStatePtr;
	ent->client->playerinfo.G_BranchLwrClimbing=G_BranchLwrClimbing;
	ent->client->playerinfo.G_PlayerActionCheckRopeGrab=G_PlayerActionCheckRopeGrab;
	ent->client->playerinfo.G_PlayerClimbingMoveFunc=G_PlayerClimbingMoveFunc;
	ent->client->playerinfo.G_PlayerActionUsePuzzle=G_PlayerActionUsePuzzle;
	ent->client->playerinfo.G_PlayerActionCheckPuzzleGrab=G_PlayerActionCheckPuzzleGrab;
	ent->client->playerinfo.G_PlayerActionTakePuzzle=G_PlayerActionTakePuzzle;
	ent->client->playerinfo.G_PlayerActionCheckPushPull_Ent=G_PlayerActionCheckPushPull_Ent;
	ent->client->playerinfo.G_PlayerActionMoveItem=G_PlayerActionMoveItem;
	ent->client->playerinfo.G_PlayerActionCheckPushButton=G_PlayerActionCheckPushButton;
	ent->client->playerinfo.G_PlayerActionPushButton=G_PlayerActionPushButton;
	ent->client->playerinfo.G_PlayerActionCheckPushLever=G_PlayerActionCheckPushLever;
	ent->client->playerinfo.G_PlayerActionPushLever=G_PlayerActionPushLever;
	ent->client->playerinfo.G_HandleTeleport=G_HandleTeleport;
	ent->client->playerinfo.G_PlayerActionShrineEffect=G_PlayerActionShrineEffect;
	ent->client->playerinfo.G_PlayerActionChickenBite=G_PlayerActionChickenBite;
	ent->client->playerinfo.G_PlayerFallingDamage=G_PlayerFallingDamage;
	ent->client->playerinfo.G_PlayerSpellShieldAttack=G_PlayerSpellShieldAttack;
	ent->client->playerinfo.G_PlayerSpellStopShieldAttack=G_PlayerSpellStopShieldAttack;
	ent->client->playerinfo.G_PlayerVaultKick = G_PlayerVaultKick;
	ent->client->playerinfo.G_PlayerActionCheckRopeMove=G_PlayerActionCheckRopeMove;
	ent->client->playerinfo.G_gamemsg_centerprintf=gi.gamemsg_centerprintf;
	ent->client->playerinfo.G_levelmsg_centerprintf=gi.levelmsg_centerprintf;
	ent->client->playerinfo.G_WeapNext=Cmd_WeapPrev_f;
	ent->client->playerinfo.G_UseItem=Cmd_Use_f;

	// Common client & server (game) function callbacks.
	
	ent->client->playerinfo.PointContents=gi.pointcontents;
	ent->client->playerinfo.SetJointAngles=G_SetJointAngles;
	ent->client->playerinfo.ResetJointAngles=G_ResetJointAngles;
	ent->client->playerinfo.PlayerActionSwordAttack=G_PlayerActionSwordAttack;
	ent->client->playerinfo.PlayerActionSpellFireball=G_PlayerActionSpellFireball;
	ent->client->playerinfo.PlayerActionSpellBlast=G_PlayerActionSpellBlast;
	ent->client->playerinfo.PlayerActionSpellArray=G_PlayerActionSpellArray;
	ent->client->playerinfo.PlayerActionSpellSphereCreate=G_PlayerActionSpellSphereCreate;
	ent->client->playerinfo.PlayerActionSpellBigBall=G_PlayerActionSpellBigBall;
	ent->client->playerinfo.PlayerActionSpellFirewall=G_PlayerActionSpellFirewall;
	ent->client->playerinfo.PlayerActionRedRainBowAttack=G_PlayerActionRedRainBowAttack;
	ent->client->playerinfo.PlayerActionPhoenixBowAttack=G_PlayerActionPhoenixBowAttack;
	ent->client->playerinfo.PlayerActionHellstaffAttack=G_PlayerActionHellstaffAttack;
	ent->client->playerinfo.PlayerActionSpellDefensive=G_PlayerActionSpellDefensive;	
	ent->client->playerinfo.G_EntIsAButton=G_EntIsAButton;
	ent->client->playerinfo.irand=ClientServerRand;

	// So we know we are server (game) side.

	ent->client->playerinfo.isclient=false;
	ent->client->playerinfo.ishistory=false;
}

// ************************************************************************************************
// SetupPlayerinfo
// ---------------
// ************************************************************************************************

void SetupPlayerinfo(edict_t *ent)
{
	int	i;

	// ********************************************************************************************
	// Inputs only.
	// ********************************************************************************************

	// Pointer to the associated player's edict_t.

	ent->client->playerinfo.self=ent;
	
	// Game .dll variables.

	ent->client->playerinfo.leveltime=level.time;
	
	// Server variables.

	ent->client->playerinfo.sv_gravity=sv_gravity->value;
	ent->client->playerinfo.sv_cinematicfreeze=sv_cinematicfreeze->value;
	ent->client->playerinfo.sv_jumpcinematic=sv_jumpcinematic->value;

	// From edict_t.

	ent->client->playerinfo.ideal_yaw=ent->ideal_yaw;
	ent->client->playerinfo.groundentity=ent->groundentity;

	// Pointer to entity_state_t of player's enemy edict.
	
	if(ent->enemy)
		ent->client->playerinfo.enemystate=&ent->enemy->s;
	else
		ent->client->playerinfo.enemystate=NULL;

	// Spell / weapon aiming direction.

	VectorCopy(ent->client->aimangles,ent->client->playerinfo.aimangles);

	// Deathmatch flags - only set this if we are in death match.

	if (deathmatch->value)
		ent->client->playerinfo.dmflags = DF_DEATHMATCH_SET | (int)dmflags->value;		// Send the high bit if deathmatch.
	else
		ent->client->playerinfo.dmflags=0;

	ent->client->playerinfo.advancedstaff = (int)(advancedstaff->value);

	// ********************************************************************************************
	// Inputs & outputs.
	// ********************************************************************************************

	// If we are in a cinematic, remove certain commands from the ucmd_t the server received from
	// the client. NOTE: THIS IS HIGHLY SUBJECTIVE. REQUIRES VIGOUROUS TESTING.
	// Basically, just killing all buttons pressed while a cinematic is running - Probably not the best way to do this
	// Jake 9/28/98
	// need to reget this constantly, since it changes on the fly.

	memcpy(&ent->client->playerinfo.pcmd,&ent->client->pcmd,sizeof(usercmd_t));	

	if (sv_cinematicfreeze->value)
	{
		ent->client->pcmd.buttons = 0;
		ent->client->pcmd.sidemove = 0;
		ent->client->pcmd.forwardmove = 0;
		ent->client->playerinfo.pcmd.buttons = 0;
		ent->client->playerinfo.pcmd.sidemove = 0;
		ent->client->playerinfo.pcmd.forwardmove = 0;
		ent->client->playerinfo.buttons = 0;
		ent->client->playerinfo.remember_buttons = 0;
	}

	// From edict_t.

	VectorCopy(ent->s.origin,ent->client->playerinfo.origin);
	VectorCopy(ent->s.angles,ent->client->playerinfo.angles);
	VectorCopy(ent->velocity,ent->client->playerinfo.velocity);
	VectorCopy(ent->mins,ent->client->playerinfo.mins);
	VectorCopy(ent->maxs,ent->client->playerinfo.maxs);
	ent->client->playerinfo.enemy=ent->enemy;
	ent->client->playerinfo.target=ent->target;
	ent->client->playerinfo.targetEnt=ent->targetEnt;
	ent->client->playerinfo.target_ent=ent->target_ent;
	ent->client->playerinfo.nextthink=ent->nextthink;
	ent->client->playerinfo.viewheight=ent->viewheight;
	ent->client->playerinfo.watertype=ent->watertype;
	ent->client->playerinfo.waterlevel=ent->waterlevel;		
	ent->client->playerinfo.deadflag=ent->deadflag;
	ent->client->playerinfo.movetype=ent->movetype;
	ent->client->playerinfo.edictflags=ent->flags;

	// From entity_state_t.

	ent->client->playerinfo.frame=ent->s.frame,
	ent->client->playerinfo.swapFrame=ent->s.swapFrame;
	ent->client->playerinfo.effects=ent->s.effects;
	ent->client->playerinfo.renderfx=ent->s.renderfx;
	ent->client->playerinfo.skinnum=ent->s.skinnum;
	ent->client->playerinfo.clientnum=ent->s.clientnum;

	for(i=0;i<MAX_FM_MESH_NODES;i++)
	{
		ent->client->playerinfo.fmnodeinfo[i]=ent->s.fmnodeinfo[i];
	}

	// From pmove_state_t.

	ent->client->playerinfo.pm_flags=ent->client->ps.pmove.pm_flags;
	ent->client->playerinfo.pm_w_flags=ent->client->ps.pmove.w_flags;
}

// ************************************************************************************************
// WritePlayerinfo
// ---------------
// ************************************************************************************************

void WritePlayerinfo(edict_t *ent)
{
	int	i;
	
	// ********************************************************************************************
	// Inputs & outputs.
	// ********************************************************************************************

	memcpy(&ent->client->pcmd,&ent->client->playerinfo.pcmd,sizeof(usercmd_t));

	// From edict_t.

	VectorCopy(ent->client->playerinfo.origin,ent->s.origin);
	VectorCopy(ent->client->playerinfo.angles,ent->s.angles);
	VectorCopy(ent->client->playerinfo.velocity,ent->velocity);
	VectorCopy(ent->client->playerinfo.mins,ent->mins);
	VectorCopy(ent->client->playerinfo.maxs,ent->maxs);
	ent->enemy=ent->client->playerinfo.enemy;
	ent->targetEnt=ent->client->playerinfo.targetEnt;
	ent->target_ent=ent->client->playerinfo.target_ent;
	ent->target=ent->client->playerinfo.target;
	ent->nextthink=ent->client->playerinfo.nextthink;
	ent->viewheight=ent->client->playerinfo.viewheight;
	ent->watertype=ent->client->playerinfo.watertype;
	ent->waterlevel=ent->client->playerinfo.waterlevel;
	ent->deadflag=ent->client->playerinfo.deadflag;
	ent->movetype=ent->client->playerinfo.movetype;
	ent->flags=ent->client->playerinfo.edictflags;

	// From entity_state_t.

	ent->s.frame=ent->client->playerinfo.frame,
	ent->s.swapFrame=ent->client->playerinfo.swapFrame;
	ent->s.effects=ent->client->playerinfo.effects;
	ent->s.renderfx=ent->client->playerinfo.renderfx;
	ent->s.skinnum=ent->client->playerinfo.skinnum;
	ent->s.clientnum=ent->client->playerinfo.clientnum;

	for(i=0;i<MAX_FM_MESH_NODES;i++)
	{
		ent->s.fmnodeinfo[i]=ent->client->playerinfo.fmnodeinfo[i];
	}

	// From pmove_state_t.

	ent->client->ps.pmove.pm_flags=ent->client->playerinfo.pm_flags;
	ent->client->ps.pmove.w_flags=ent->client->playerinfo.pm_w_flags;

	// ********************************************************************************************
	// Outputs only.
	// ********************************************************************************************

	// From playerstate_t.

	VectorCopy(ent->client->playerinfo.offsetangles,ent->client->ps.offsetangles);
}

// ************************************************************************************************
// SetupPlayerinfo_effects
// -----------------------
// ************************************************************************************************

void SetupPlayerinfo_effects(edict_t *ent)
{
	int i;

	ent->client->playerinfo.effects = ent->s.effects;
	ent->client->playerinfo.renderfx = ent->s.renderfx;
	ent->client->playerinfo.skinnum = ent->s.skinnum;
	ent->client->playerinfo.clientnum = ent->s.clientnum;

	for(i = 0; i < MAX_FM_MESH_NODES; i++)
	{
		ent->client->playerinfo.fmnodeinfo[i] = ent->s.fmnodeinfo[i];
	}
}

// ************************************************************************************************
// WritePlayerinfo_effects
// -----------------------
// ************************************************************************************************

void WritePlayerinfo_effects(edict_t *ent)
{
	int i;

	ent->s.effects = ent->client->playerinfo.effects;
	ent->s.renderfx = ent->client->playerinfo.renderfx;
	ent->s.skinnum = ent->client->playerinfo.skinnum;
	ent->s.clientnum = ent->client->playerinfo.clientnum;

	for(i = 0; i < MAX_FM_MESH_NODES; i++)
	{
		ent->s.fmnodeinfo[i] = ent->client->playerinfo.fmnodeinfo[i];
	}
}

// ************************************************************************************************
// PlayerTimerUpdate
// -----------------
// Deal with incidental player stuff, like setting the personal light to OFF if its should be.
// ************************************************************************************************

void PlayerTimerUpdate(edict_t *ent)
{
	playerinfo_t *playerinfo;

	playerinfo=&ent->client->playerinfo;

	// Disable light when we should.

	if (playerinfo->light_timer < level.time)
		playerinfo->effects &= ~EF_LIGHT_ENABLED;

	// Disable speed when we should.

	if (playerinfo->speed_timer < level.time)
		playerinfo->effects &= ~EF_SPEED_ACTIVE;

	// Disable max speed when we should.

	if (playerinfo->knockbacktime < level.time)
		playerinfo->effects &= ~EF_HIGH_MAX;

	// Disable powerup when we should.

	if (playerinfo->powerup_timer < level.time)
		playerinfo->effects &= ~EF_POWERUP_ENABLED;

	// Disable relection when we should.

	if (playerinfo->reflect_timer < level.time)
	{
		// Were we relfective last frame?

		if (playerinfo->renderfx &RF_REFLECTION) 
		{
			playerinfo->renderfx &= ~RF_REFLECTION;
			
			// Unset the skin.

			P_PlayerUpdateModelAttributes(&ent->client->playerinfo);
		}
	}

	// Disable ghosting when we should.

	if (playerinfo->ghost_timer < level.time)
		playerinfo->renderfx &= ~RF_TRANS_GHOST;
}

/*
===============
P_DamageFeedback

Handles color blends and view kicks
===============
*/

void P_DamageFeedback (edict_t *player)
{
	gclient_t	*client;
	int			count;

	client=player->client;

	// Flash the backgrounds behind the status numbers.

	client->ps.stats[STAT_FLASHES]=0;

	if (client->damage_blood)
		client->ps.stats[STAT_FLASHES]|=1;
	
	// Total up the points of damage shot at the player this frame.
	
	if((count=client->damage_blood)==0)
	{
		// Didn't take any damage.
		return;
	}

	//Check for gasssss damage
	if (player->pain_debounce_time < level.time && client->damage_gas)
	{
		if ( client->playerinfo.loweridle && client->playerinfo.upperidle )
			P_PlayerAnimSetLowerSeq(&client->playerinfo, ASEQ_PAIN_A);

		P_PlayerPlayPain(&client->playerinfo, 1);
	}
	else if (((!irand(0, 4)) || count > 8) && (player->pain_debounce_time < level.time)) // Play pain animation.
	{
		if ( client->playerinfo.loweridle && client->playerinfo.upperidle )
			P_PlayerAnimSetLowerSeq(&client->playerinfo, ASEQ_PAIN_A);
		
		if (count <= 4)
			P_PlayerPlayPain(&client->playerinfo, 2);
		else
			P_PlayerPlayPain(&client->playerinfo, 0);

		player->pain_debounce_time = level.time + 0.5;
	}

	// Reset the player's pain_debounce_time.

	if (level.time > player->pain_debounce_time)
		player->pain_debounce_time = level.time + 0.7;		

	// Clear damage totals.

	client->damage_blood = 0;
	client->damage_knockback = 0;
}

/*
=============
P_WorldEffects
=============
*/

void P_WorldEffects (void)
{
	int		waterlevel,old_waterlevel;
	vec3_t	Origin,
			Dir;

	if (current_player->client->playerinfo.deadflag > DEAD_NO)
		return;

	// If we are in no clip, or we have special lungs, we don't need air.

	if(current_player->movetype==PHYSICSTYPE_NOCLIP)
	{
		current_player->air_finished=level.time+HOLD_BREATH_TIME;

		if(current_player->movetype==PHYSICSTYPE_NOCLIP)
			return;
	}

	waterlevel=current_player->waterlevel;
	old_waterlevel=current_client->old_waterlevel;
	current_client->old_waterlevel=waterlevel;

	//
	// If the current player just entered a water volume, play a sound and start a water-ripple
	// client effect.
	//

	if(!old_waterlevel&&waterlevel)
	{
		// Clear damage_debounce, so the pain sound will play immediately.

		current_player->damage_debounce_time=level.time-1;

		if (current_player->watertype & CONTENTS_LAVA)
		{
			gi.sound(current_player, CHAN_BODY, gi.soundindex("player/inlava.wav"), 1, ATTN_NORM, 0);
			current_player->flags |= FL_INLAVA;
		}
		else if (current_player->watertype & CONTENTS_SLIME)
		{
			gi.sound(current_player, CHAN_BODY, gi.soundindex("player/muckin.wav"), 1, ATTN_NORM, 0);
			current_player->flags |= FL_INSLIME;
		}
		else 
		{
			gi.sound(current_player, CHAN_BODY, gi.soundindex("player/Water Enter.wav"),1,ATTN_NORM,0);
		}
		current_player->flags|=FL_INWATER;

		VectorCopy(current_player->s.origin,Origin);

		Origin[2]+=current_player->client->playerinfo.waterheight;

		// Fixme: Need to determine the actual water surface normal - if we have any sloping water??

		Dir[0]=0.0;
		Dir[1]=0.0;
		Dir[2]=1.0;

		VectorCopy(Origin,current_player->client->playerinfo.LastWatersplashPos);

		gi.CreateEffect(NULL,
						FX_WATER_ENTRYSPLASH,
						CEF_FLAG7,
						Origin,
						"bd",
						128|96,			// FIXME: Size propn. to entry velocity?
						Dir);
	}
	else if (old_waterlevel&&!waterlevel)
	{

		//
		// If the current player just completely exited a water volume, play a sound.
		//

		// INWATER is set whether in lava, slime or water.
		if (current_player->flags & FL_INLAVA)
		{
			gi.sound (current_player, CHAN_BODY, gi.soundindex("player/inlava.wav"), 1, ATTN_NORM, 0);
			current_player->flags &= ~FL_INLAVA;
		}
		else if (current_player->flags & FL_INSLIME)
		{
			gi.sound (current_player, CHAN_BODY, gi.soundindex("player/muckexit.wav"), 1, ATTN_NORM, 0);
			current_player->flags &= ~FL_INSLIME;
		}
		else 
		{
			gi.sound (current_player, CHAN_BODY, gi.soundindex("player/Water Exit.wav"), 1, ATTN_NORM, 0);
		}
		current_player->flags&=~FL_INWATER;

		VectorCopy(current_player->s.origin,Origin);

		Origin[2]=current_player->client->playerinfo.LastWatersplashPos[2];

		// Fixme: Need to determine the actual water surface normal - if we have any sloping water??

		Dir[0]=0.0;
		Dir[1]=0.0;
		Dir[2]=1.0;

		VectorCopy(Origin,current_player->client->playerinfo.LastWatersplashPos);

		gi.CreateEffect(NULL,
						FX_WATER_ENTRYSPLASH,
						0,
						Origin,
						"bd",
						96,				// FIXME: Size propn. to exit velocity.
						Dir);
	}

	//
	// Start a waterwake effect if the current player has been in water and still is in water.
	//

	if(waterlevel && (old_waterlevel&&waterlevel<3) && (VectorLength(current_player->velocity)!=0.0))
	{
	// no ripples while in cinematics
	if (sv_cinematicfreeze->value)
		return;

		if((((int)(current_client->bobtime+bobmove))!=bobcycle) && (!sv_cinematicfreeze->value))
		{
			// FIXME: Doing more work then we need to here???? How about re-writing this so that it
			// is always active on the client and does water tests itself? We'll see - currently not
			// enough info. is available on the client to try this.

			vec3_t	Angles,
					Temp;
			byte	angle_byte;
			
			VectorCopy(current_player->velocity,Temp);

			VectorNormalize(Temp);

			vectoangles(Temp,Angles);

			VectorCopy(current_player->s.origin,Origin);

			Origin[2]+=current_player->client->playerinfo.waterheight;

			angle_byte = Q_ftol(((Angles[YAW] + DEGREE_180)/360.0) * 255.0);

			gi.CreateEffect(NULL,
							FX_WATER_WAKE,
							0,
							Origin,
							"sbv",							
							(short)current_player->s.number,
							angle_byte,
							current_player->velocity);
		}
	}

	//
	// Check for head just coming out of water.
	//

	if (old_waterlevel == 3 && waterlevel != 3)
	{
		if (current_player->air_finished < level.time)
		{	
			// Gasp for air.
			if (irand(0,1))
				gi.sound (current_player, CHAN_BODY, gi.soundindex("*gasp1.wav"), 1, ATTN_NORM, 0);
			else
				gi.sound (current_player, CHAN_BODY, gi.soundindex("*gasp2.wav"), 1, ATTN_NORM, 0);

		}
		else  if (current_player->air_finished < level.time + 11)
		{	
			// Broke surface, low on air
			gi.sound (current_player, CHAN_BODY, gi.soundindex("*waterresurface.wav"), 1, ATTN_NORM, 0);
		}
	}

	// ********************************************************************************************
	// Handle drowning.
	// ********************************************************************************************

	if (waterlevel == 3)
	{
		if(current_player->watertype & CONTENTS_SLIME)
		{
			// Slime should kill really quick.

			current_player->dmg=25;

			// Play a gurp sound instead of a normal pain sound.
			
			if (irand(0, 1))
				gi.sound (current_player, CHAN_VOICE, gi.soundindex("*drowning1.wav"), 1, ATTN_NORM, 0);
			else
				gi.sound (current_player, CHAN_VOICE, gi.soundindex("*drowning2.wav"), 1, ATTN_NORM, 0);		

			current_player->pain_debounce_time = level.time;

			T_Damage(current_player,
					 world,
					 world,
					 vec3_origin,
					 current_player->s.origin,
					 vec3_origin,
					 current_player->dmg,
					 0,
					 DAMAGE_SUFFOCATION,
					 MOD_SLIME);		
		}
		else if ((current_player->air_finished + current_player->client->playerinfo.lungs_timer) < level.time)
		{	
			// If out of air, start drowning.

			if (current_player->client->next_drown_time < level.time && current_player->health > 0)
			{
				current_player->client->next_drown_time = level.time + 1;

				// Take more damage the longer underwater.

				current_player->dmg += 2;

				if (current_player->dmg > 15)
					current_player->dmg = 15;

				// Play a gurp sound instead of a normal pain sound.
				
				if (irand(0, 1))
					gi.sound (current_player, CHAN_VOICE, gi.soundindex("*drowning1.wav"), 1, ATTN_NORM, 0);
				else
					gi.sound (current_player, CHAN_VOICE, gi.soundindex("*drowning2.wav"), 1, ATTN_NORM, 0);		

				current_player->pain_debounce_time = level.time;

				T_Damage(current_player,
						 world,
						 world,
						 vec3_origin,
						 current_player->s.origin,
						 vec3_origin,
						 current_player->dmg,
						 0,
						 DAMAGE_SUFFOCATION,
						 MOD_WATER);
			}
			
		}
		// else, we aren't drowning yet, but we may well need to decrement the amount of extra lungs we have from shrines
		// since we still have lungs, reset air finished till we don't anymore.
		else
		if (current_player->client->playerinfo.lungs_timer)
		{
			// floatinf point inaccuracy never lets us equal zero by ourselves
			if (current_player->client->playerinfo.lungs_timer < 0.1)
				current_player->client->playerinfo.lungs_timer = 0.0;
			else
			{
				current_player->client->playerinfo.lungs_timer -= 0.1;
				current_player->air_finished = level.time + HOLD_BREATH_TIME;
			}
		}

		if (old_waterlevel != 3)
		{	// We weren't underwater before this, so play a submerged sound
			gi.sound (current_player, CHAN_VOICE, gi.soundindex("player/underwater.wav"), 1, ATTN_IDLE, 0);
		}

		// Play an underwater sound every 4 seconds!
		if (((int)(level.time/4.0))*4.0 == level.time)
		{	// Underwater ambient
			// Play local only?
			gi.sound (current_player, CHAN_BODY, gi.soundindex("player/underwater.wav"), 1, ATTN_IDLE, 0);
		}
	}
	else
	{
		current_player->air_finished = level.time + HOLD_BREATH_TIME;
		current_player->dmg = 2;
	}

	// ********************************************************************************************
	// Handle lava sizzle damage.
	// ********************************************************************************************

	if (waterlevel && (current_player->watertype&(CONTENTS_LAVA)) )
	{
		if (current_player->health > 0 && current_player->pain_debounce_time <= level.time)
		{
			gi.sound (current_player, CHAN_VOICE, gi.soundindex("player/lavadamage.wav"), 1, ATTN_NORM, 0);
			
			current_player->pain_debounce_time = level.time + 1;
		}

		T_Damage(current_player,
				 world,
				 world,
				 vec3_origin,
				 current_player->s.origin,
				 vec3_origin,
				 (waterlevel>2)?25:(3*waterlevel),
				 0,
				 DAMAGE_LAVA,
				 MOD_LAVA);
	}
}

/*
===============
G_SetClientSound
===============
*/

void G_SetClientSound (edict_t *ent)
{
	char	*weap;

	if (ent->client->playerinfo.pers.weapon)
		weap = ent->client->playerinfo.pers.weapon->classname;
	else
		weap = "";
// this seems like a silly thing to do ?
///	ent->s.sound = 0;
}

// ************************************************************************************************
// ClientEndServerFrame
// --------------------
// Called for each player at the end of the server frame and right after spawning.
// ************************************************************************************************

void ClientEndServerFrame (edict_t *ent)
{
	float	bobtime;
	int		i,index;
	
	current_player = ent;
	current_client = ent->client;

	// ********************************************************************************************
	// If end of unit layout is displayed, don't give the player any normal movement attributes.
	// ********************************************************************************************

	if (level.intermissiontime)
	{
		current_client->ps.fov=75;
		G_SetStats(ent);
		
		return;
	}

	// ********************************************************************************************
	// Apply world effect, e.g. burn from lava, etc.
	// ********************************************************************************************

	P_WorldEffects ();

	// ********************************************************************************************
	// Set the player entity's model angles.
	// ********************************************************************************************

	if(ent->deadflag==DEAD_NO)
	{
		// PITCH.

		if((ent->client->ps.pmove.w_flags & (WF_DIVING | WF_SWIMFREE)))
		{
			if(ent->client->v_angle[PITCH] > 180.0)
				ent->s.angles[PITCH] = -(-360.0 + ent->client->v_angle[PITCH]);
			else
				ent->s.angles[PITCH] = -ent->client->v_angle[PITCH];
		}
		else
		{
			ent->s.angles[PITCH] = 0.0;
		}

		// YAW and ROLL.

		ent->s.angles[YAW] = ent->client->v_angle[YAW];
		ent->s.angles[ROLL] = 0.0;
	}	

	// ********************************************************************************************
	// Handle calcs for cyclic effects like walking / swimming.
	// ********************************************************************************************

	xyspeed = sqrt(ent->velocity[0] * ent->velocity[0] + ent->velocity[1] * ent->velocity[1]);

	if (xyspeed < 5)
	{
		// Start at beginning of cycle again.

		bobmove = 0;
		current_client->bobtime = 0;
	}
	else if(ent->groundentity && !current_player->waterlevel)
	{	
		// So bobbing only cycles when on ground.

		if(xyspeed > 210)
			bobmove = 0.25;
		else if(xyspeed > 100)
			bobmove = 0.125;
		else
			bobmove = 0.0625;
	}
	else if(current_player->waterlevel)
	{
		// So bobbing only cycles when in water.

		if(xyspeed > 100)
			bobmove = 1.0;
		else if(xyspeed > 50)
			bobmove = 0.5;
		else
			bobmove = 0.25;
	}

	bobtime = (current_client->bobtime += bobmove);
	bobcycle = (int)bobtime;
	bobfracsin = Q_fabs(sin(bobtime * M_PI));

	// ********************************************************************************************
	// Calculate damage (if any) from hitting the floor and apply the damage taken this frame from
	// ALL sources.
	// ********************************************************************************************

	SetupPlayerinfo(ent);
	
	P_PlayerFallingDamage(&ent->client->playerinfo);
	P_DamageFeedback(ent);

	WritePlayerinfo(ent);

	// ********************************************************************************************
	// Generate client-side status display data
	// ********************************************************************************************

	G_SetStats(ent);
	G_SetClientSound(ent);

	// ********************************************************************************************
	// Handle player animation.
	// ********************************************************************************************

	SetupPlayerinfo(ent);

	P_PlayerUpdateCmdFlags(&ent->client->playerinfo);

	if(BUOY_DEBUG) // Note this is a bit of a hack
	{
		if(ent->client->playerinfo.seqcmd[ACMDL_ACTION] == true)
		{
			PrintLocalBuoyInfo(ent->s.origin);
		}
	}
	
	P_PlayerUpdate(&ent->client->playerinfo);
	P_AnimUpdateFrame(&ent->client->playerinfo);
	PlayerTimerUpdate(ent);

	WritePlayerinfo(ent);

	// ********************************************************************************************
	// Save velocity and viewangles away for use next game frame.
	// ********************************************************************************************

	VectorCopy(ent->velocity,ent->client->playerinfo.oldvelocity);
	VectorCopy(ent->client->ps.viewangles,ent->client->oldviewangles);

	// ********************************************************************************************
	// If the deathmatch scoreboard is up then update it.
	// ********************************************************************************************

	if(ent->client->playerinfo.showscores && deathmatch->value && (!(level.framenum&31)))
	{
		DeathmatchScoreboardMessage(ent,ent->enemy,false);
		
		gi.unicast(ent,false);
	}

	// ********************************************************************************************
	//  Reflect remote camera views(s) in the client's playerstate.
	// ********************************************************************************************

	if(current_client->RemoteCameraLockCount>0)
		current_client->ps.remote_id=current_client->RemoteCameraNumber;
	else
		current_client->ps.remote_id=-1;

	// ********************************************************************************************
	// Reflect inventory changes in the client's playetstate.
	// ********************************************************************************************

	current_client->ps.NoOfItems=i=0;

	for(index=0;index<MAX_ITEMS;index++)
	{
		if(current_client->playerinfo.pers.inventory.Items[index]!=current_client->playerinfo.pers.old_inventory.Items[index])
		{
			current_client->ps.inventory_changes[i]=index;

			current_client->ps.inventory_remaining[i]=current_client->playerinfo.pers.inventory.Items[index];

			current_client->playerinfo.pers.old_inventory.Items[index]=current_client->playerinfo.pers.inventory.Items[index];
			
			i++;
		}
	}

	current_client->ps.NoOfItems=i;

	// ********************************************************************************************
	// Reflect changes to the client's origin and velocity due to the current player animation, in
	// the client's playerstate.
	// ********************************************************************************************

	current_client->ps.pmove.origin[0]=ent->s.origin[0]*8.0;
	current_client->ps.pmove.origin[1]=ent->s.origin[1]*8.0;
	current_client->ps.pmove.origin[2]=ent->s.origin[2]*8.0;

	current_client->ps.pmove.velocity[0]=ent->velocity[0]*8.0;
	current_client->ps.pmove.velocity[1]=ent->velocity[1]*8.0;
	current_client->ps.pmove.velocity[2]=ent->velocity[2]*8.0;

	// ********************************************************************************************
	//  Reflect viewheight changes in client's playerstate.
	// ********************************************************************************************

	current_client->ps.viewheight=ent->viewheight;

	// ********************************************************************************************
	// Write all the shit that animation system modifies out to the playerstate (for prediction).
	// ********************************************************************************************

	current_client->ps.maxs[0]=current_client->playerinfo.maxs[0];
	current_client->ps.maxs[1]=current_client->playerinfo.maxs[1];
	current_client->ps.maxs[2]=current_client->playerinfo.maxs[2];

	current_client->ps.mins[0]=current_client->playerinfo.mins[0];
	current_client->ps.mins[1]=current_client->playerinfo.mins[1];
	current_client->ps.mins[2]=current_client->playerinfo.mins[2];

	current_client->ps.NonNullgroundentity=(byte)(current_client->playerinfo.groundentity?1:0);
	current_client->ps.GroundPlane=current_client->playerinfo.GroundPlane;
	current_client->ps.GroundContents=current_client->playerinfo.GroundContents;
	current_client->ps.GroundSurface.flags=
		(current_client->playerinfo.GroundSurface!=NULL)?current_client->playerinfo.GroundSurface->flags:0;

	current_client->ps.watertype=current_client->playerinfo.watertype;
	current_client->ps.waterlevel=current_client->playerinfo.waterlevel;
	current_client->ps.waterheight=current_client->playerinfo.waterheight;

	VectorCopy(current_client->playerinfo.grabloc,current_client->ps.grabloc);
	current_client->ps.grabangle=current_client->playerinfo.grabangle;

	current_client->ps.fwdvel=current_client->playerinfo.fwdvel;
	current_client->ps.sidevel=current_client->playerinfo.sidevel;
	current_client->ps.upvel=current_client->playerinfo.upvel;

	current_client->ps.flags=current_client->playerinfo.flags;

	current_client->ps.edictflags=current_client->playerinfo.edictflags;

	current_client->ps.oldvelocity_z=current_client->playerinfo.oldvelocity[2];

	current_client->ps.upperseq=current_client->playerinfo.upperseq;
	current_client->ps.lowerseq=current_client->playerinfo.lowerseq;

	current_client->ps.upperframe=current_client->playerinfo.upperframe;
	current_client->ps.lowerframe=current_client->playerinfo.lowerframe;

	current_client->ps.upperidle=(byte)((current_client->playerinfo.upperidle==true)?1:0);
	current_client->ps.loweridle=(byte)((current_client->playerinfo.loweridle==true)?1:0);
	
	current_client->ps.uppermove_index=current_client->playerinfo.uppermove_index;
	current_client->ps.lowermove_index=current_client->playerinfo.lowermove_index;

	current_client->ps.weapon=(byte)ITEM_INDEX(current_client->playerinfo.pers.weapon);
	current_client->ps.defense=(byte)ITEM_INDEX(current_client->playerinfo.pers.defence);
	current_client->ps.lastweapon=(byte)ITEM_INDEX(current_client->playerinfo.pers.lastweapon);
	current_client->ps.lastdefense=(byte)ITEM_INDEX(current_client->playerinfo.pers.lastdefence);
	current_client->ps.weaponready=(byte)current_client->playerinfo.pers.weaponready;	
	current_client->ps.switchtoweapon=(byte)current_client->playerinfo.switchtoweapon;
	current_client->ps.newweapon=(byte)ITEM_INDEX(current_client->playerinfo.pers.newweapon);
	current_client->ps.weap_ammo_index=(byte)current_client->playerinfo.weap_ammo_index;
	current_client->ps.def_ammo_index=(byte)current_client->playerinfo.def_ammo_index;
	current_client->ps.weaponcharge=(byte)current_client->playerinfo.weaponcharge;
	current_client->ps.armortype=(byte)current_client->playerinfo.pers.armortype;
	current_client->ps.bowtype=(byte)current_client->playerinfo.pers.bowtype;
	current_client->ps.stafflevel=(byte)current_client->playerinfo.pers.stafflevel;
	current_client->ps.helltype=(byte)current_client->playerinfo.pers.helltype;
	current_client->ps.meteor_count=(byte)current_client->playerinfo.meteor_count;
	current_client->ps.handfxtype=(byte)current_client->playerinfo.pers.handfxtype;
	current_client->ps.plaguelevel=(byte)current_client->playerinfo.plaguelevel;
	current_client->ps.skintype=(byte)current_client->playerinfo.pers.skintype;
	current_client->ps.altparts=(byte)current_client->playerinfo.pers.altparts;
	current_client->ps.deadflag=current_client->playerinfo.deadflag;
	current_client->ps.ideal_yaw=ent->ideal_yaw;
	current_client->ps.leveltime=level.time;
	current_client->ps.idletime=current_client->playerinfo.idletime;
	current_client->ps.quickturnEndTime=current_client->playerinfo.quickturnEndTime;
	current_client->ps.powerup_timer=current_client->playerinfo.powerup_timer;
	current_client->ps.quickturn_rate=current_client->playerinfo.quickturn_rate;

	current_client->ps.dmflags=current_client->playerinfo.dmflags;
	current_client->ps.advancedstaff=current_client->playerinfo.advancedstaff;

	current_client->ps.cinematicfreeze=current_client->playerinfo.sv_cinematicfreeze;
}