#include "g_local.h"
#include "g_DefaultMessageHandler.h"
#include "g_Skeletons.h"
#include "m_player.h"
#include "P_NewMove.h"
#include "p_main.h"
#include "p_ctrl.h"
#include "p_chicken.h"
#include "p_funcs.h"
#include "Angles.h"
#include "compfmod.h"
#include "FX.h"
#include "vector.h"
#include "random.h"
#include "Utilities.h"
#include "g_playstats.h"
#include "g_HitLocation.h"
#include "g_misc.h"
#include "g_Physics.h"
#include "p_main.h"
#include "g_itemstats.h"
#include "cl_strings.h"
#include "p_actions.h"
#include "p_anim_branch.h"
#include "qcommon.h"

// FIXME: include headers.
extern void	InitPlayerinfo(edict_t *ent);
extern void	SetupPlayerinfo(edict_t *ent);
extern void	WritePlayerinfo(edict_t *ent);
extern void PlayerChickenDeath(edict_t *ent);
extern qboolean AddWeaponToInventory(gitem_t *item,edict_t *player);
extern void AddDefenseToInventory(gitem_t *item,edict_t *player);
extern void CheckContinuousAutomaticEffects(edict_t *self);
extern void CalculatePIV(edict_t *player);

#define SWIM_ADJUST_AMOUNT	16
#define FOV_DEFAULT			75.0

// NOTENOTE: The precious, delicate player bbox coords!

vec3_t	mins = {-14, -14, -34};
vec3_t	maxs = { 14,  14,  25};

void ClientUserinfoChanged (edict_t *ent, char *userinfo);
extern void PlayerKillShrineFX(edict_t *self);

void SP_misc_teleporter_dest (edict_t *ent);


/*QUAKED info_player_start (1 0 0) (-16 -16 -24) (16 16 32)
The normal starting point for a level.
*/
void SP_info_player_start(void)
{
}

/*QUAKED info_player_deathmatch (1 0 1) (-16 -16 -24) (16 16 32)
potential spawning position for deathmatch games
*/
void SP_info_player_deathmatch(edict_t *self)
{
	if (!deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}
//	SP_misc_teleporter_dest (self);
}

/*QUAKED info_player_coop (1 0 1) (-16 -16 -24) (16 16 32)
potential spawning position for coop games
*/
void SP_info_player_coop(edict_t *self)
{
	if(!coop->value)
	{
		G_FreeEdict (self);
		return;
	}
//	SP_misc_teleporter_dest (self);
}


/*QUAKED info_player_intermission (1 0 1) (-16 -16 -24) (16 16 32)
The deathmatch intermission point will be at one of these
Use 'angles' instead of 'angle', so you can set pitch or roll as well as yaw.  'pitch yaw roll'
*/

void SP_info_player_intermission(void)
{
}


int	SexedSoundIndex (edict_t *ent, char *base)
{
	char	buffer[MAX_QPATH];

	Com_sprintf (buffer, sizeof(buffer), "%s/%s.wav", ent->client->playerinfo.pers.sounddir, base);

	return gi.soundindex(buffer);
}



void ClientSetSkinType(edict_t *ent, char *skinname)
{
	playerinfo_t *playerinfo;

	playerinfo = &(ent->client->playerinfo);

	SetupPlayerinfo_effects(ent);
 	P_PlayerUpdateModelAttributes(playerinfo);
	WritePlayerinfo_effects(ent);
	
}


int player_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	// player pain is handled at the end of the frame in P_DamageFeedback
	return(0);
}

void BleederThink (edict_t *self)
{
	vec3_t	bleed_spot, bleed_dir, forward, right, up;
	int		damage;

	if(!self->owner)
		goto byebye;

	if(!self->owner->client)
		goto byebye;

	if(!self->owner->s.modelindex)
		goto byebye;

	if(!(self->owner->client->playerinfo.flags&PLAYER_FLAG_BLEED))
		goto byebye;

	if(self->owner->health <= 0)
		goto byebye;

	//FIXME: this will be a client effect attached to ref points
	damage = irand(1, 3);

	AngleVectors(self->owner->s.angles, forward, right, up);
	VectorMA(self->owner->s.origin, self->pos1[0], forward, bleed_spot);
	VectorMA(bleed_spot, self->pos1[1], right, bleed_spot);
	VectorMA(bleed_spot, self->pos1[2], up, bleed_spot);

	VectorScale(forward, self->movedir[0], bleed_dir);
	VectorMA(bleed_dir, self->movedir[1], right, bleed_dir);
	VectorMA(bleed_dir, self->movedir[2], up, bleed_dir);
	VectorScale(bleed_dir, damage*3, bleed_dir);

	if(self->owner->materialtype == MAT_INSECT)
		gi.CreateEffect(NULL, FX_BLOOD, CEF_FLAG8, bleed_spot, "ub", bleed_dir, damage);
	else
		gi.CreateEffect(NULL, FX_BLOOD, 0, bleed_spot, "ub", bleed_dir, damage);

	if(!irand(0,3))//25%chance to do damage
		T_Damage(self->owner, self, self->activator, bleed_dir, bleed_spot, bleed_dir, damage, 0, DAMAGE_NO_BLOOD|DAMAGE_NO_KNOCKBACK|DAMAGE_BLEEDING|DAMAGE_AVOID_ARMOR,MOD_BLEED);//armor doesn't stop it

	self->nextthink = level.time + flrand(0.1, 0.5);
	return;

byebye:
	G_SetToFree(self);
	return;

}

void SpawnBleeder (edict_t *self, edict_t *other, vec3_t bleed_dir, vec3_t bleed_spot)//, byte refpoint)
{
	edict_t	*bleeder;

	self->client->playerinfo.flags |= PLAYER_FLAG_BLEED;

	bleeder = G_Spawn();
	bleeder->owner = self;
	bleeder->activator = other;
	bleeder->classname = "bleeder";
	VectorCopy(bleed_spot, bleeder->pos1);
	VectorCopy(bleed_dir, bleeder->movedir);
	bleeder->think = BleederThink;
	bleeder->nextthink = level.time + 0.1;
//when refpoints on arms and head in for corvus, do this:
/*	gi.CreateEffect(&self->s.,
			FX_LINKEDBLOOD,
			0,
			self->s.origin,
			"bb",
			180,
			refpoint);*/
}

void player_repair_skin (edict_t *self)
{//FIXME: make sure it doesn't turn on a hand without the arm!
	int i, num_allowed_dmg_skins, to_fix;
	int	found_dmg_skins = 0;
	int	checked = 0;
	int hurt_nodes[NUM_PLAYER_NODES];

	if(!self->client)
		return;

	if(!self->s.modelindex)
		return;

	num_allowed_dmg_skins = 5 - floor(self->health/20);
#ifdef _DEVEL
	gi.dprintf("Allowed damaged nodes: %d\n", num_allowed_dmg_skins);
#endif

	if(num_allowed_dmg_skins <= 0)
	{//restore all nodes
		for(i = 0; i < NUM_PLAYER_NODES; i++)
		{
			if(i == MESH__STOFF||
				i == MESH__BOFF||
				i == MESH__ARMOR||
				i == MESH__STAFACTV||
				i == MESH__BLADSTF||
				i == MESH__HELSTF||
				i == MESH__BOWACTV)
				continue;//these shouldn't be fucked with
			else
			{
#ifdef _DEVEL
				gi.dprintf("Healed player skin on node %d\n", i);
#endif
				self->client->playerinfo.pers.altparts &= ~(1<<i);
				self->s.fmnodeinfo[i].flags &= ~FMNI_USE_SKIN;
				self->s.fmnodeinfo[i].skin = self->s.skinnum;
			}
		}
		SetupPlayerinfo_effects(self);
		P_PlayerUpdateModelAttributes(&self->client->playerinfo);
		WritePlayerinfo_effects(self);
		return;
	}

	for(i = 0; i<NUM_PLAYER_NODES; i++)
	{//how many nodes are hurt
		if(i == MESH__STOFF||
			i == MESH__BOFF||
			i == MESH__ARMOR||
			i == MESH__STAFACTV||
			i == MESH__BLADSTF||
			i == MESH__HELSTF||
			i == MESH__BOWACTV)
			continue;//these shouldn't be fucked with

		if(!(self->s.fmnodeinfo[i].flags&FMNI_NO_DRAW)&&(self->s.fmnodeinfo[i].flags&FMNI_USE_SKIN))
		{
			hurt_nodes[found_dmg_skins] = i;
			found_dmg_skins++;
		}
	}

#ifdef _DEVEL
	gi.dprintf("Found damaged nodes: %d\n", found_dmg_skins);
#endif
	if(found_dmg_skins<=num_allowed_dmg_skins)//no healing
		return;

	to_fix = found_dmg_skins - num_allowed_dmg_skins;

	while(to_fix > 0 && checked<100)
	{//heal num damaged nodes over allowed
		i = hurt_nodes[irand(0, (found_dmg_skins - 1))];
		if(!(self->s.fmnodeinfo[i].flags&FMNI_NO_DRAW))
		{
			if(self->s.fmnodeinfo[i].flags&FMNI_USE_SKIN)
			{
#ifdef _DEVEL
				gi.dprintf("Healed player skin on node %d\n", i);
#endif
				self->s.fmnodeinfo[i].flags &= ~FMNI_USE_SKIN;
				self->s.fmnodeinfo[i].skin = self->s.skinnum;

				self->client->playerinfo.pers.altparts &= ~(1<<i);

				if(i == MESH__LARM)
					self->client->playerinfo.flags &= ~PLAYER_FLAG_NO_LARM;
				else if(i == MESH__RARM)
					self->client->playerinfo.flags &= ~PLAYER_FLAG_NO_RARM;
				
				to_fix--;
				checked++;//to protect against infinite loops, this IS random after all
			}
		}
	}
	
	SetupPlayerinfo_effects(self);
	P_PlayerUpdateModelAttributes(&self->client->playerinfo);
	WritePlayerinfo_effects(self);
}

void ResetPlayerBaseNodes (edict_t *ent)
{
	if(!ent->client)
		return;

	ent->client->playerinfo.flags &= ~PLAYER_FLAG_BLEED;

	ent->client->playerinfo.flags &= ~PLAYER_FLAG_NO_LARM;
	ent->client->playerinfo.flags &= ~PLAYER_FLAG_NO_RARM;

	ent->client->playerinfo.pers.altparts = 0;

	ent->s.fmnodeinfo[MESH_BASE2].flags &= ~FMNI_NO_DRAW;
	ent->s.fmnodeinfo[MESH__BACK].flags &= ~FMNI_NO_DRAW;
	ent->s.fmnodeinfo[MESH__RARM].flags &= ~FMNI_NO_DRAW;
	ent->s.fmnodeinfo[MESH__LARM].flags &= ~FMNI_NO_DRAW;
	ent->s.fmnodeinfo[MESH__HEAD].flags &= ~FMNI_NO_DRAW;
	ent->s.fmnodeinfo[MESH__RLEG].flags &= ~FMNI_NO_DRAW;
	ent->s.fmnodeinfo[MESH__LLEG].flags &= ~FMNI_NO_DRAW;

	ent->s.fmnodeinfo[MESH_BASE2].flags &= ~FMNI_USE_SKIN;
	ent->s.fmnodeinfo[MESH__BACK].flags &= ~FMNI_USE_SKIN;
	ent->s.fmnodeinfo[MESH__RARM].flags &= ~FMNI_USE_SKIN;
	ent->s.fmnodeinfo[MESH__LARM].flags &= ~FMNI_USE_SKIN;
	ent->s.fmnodeinfo[MESH__HEAD].flags &= ~FMNI_USE_SKIN;
	ent->s.fmnodeinfo[MESH__RLEG].flags &= ~FMNI_USE_SKIN;
	ent->s.fmnodeinfo[MESH__LLEG].flags &= ~FMNI_USE_SKIN;

	ent->s.fmnodeinfo[MESH_BASE2].skin = ent->s.skinnum;
	ent->s.fmnodeinfo[MESH__BACK].skin = ent->s.skinnum;
	ent->s.fmnodeinfo[MESH__RARM].skin = ent->s.skinnum;
	ent->s.fmnodeinfo[MESH__LARM].skin = ent->s.skinnum;
	ent->s.fmnodeinfo[MESH__HEAD].skin = ent->s.skinnum;
	ent->s.fmnodeinfo[MESH__RLEG].skin = ent->s.skinnum;
	ent->s.fmnodeinfo[MESH__LLEG].skin = ent->s.skinnum;

	// FIXME: Turn hands back on too? But two pairs, which one? Shouldn't PlayerUpdateModelAttributes do that?

	SetupPlayerinfo_effects(ent);
	P_PlayerUpdateModelAttributes(&ent->client->playerinfo);
	WritePlayerinfo_effects(ent);
}

#define BIT_BASE2		0//		MESH_BASE2		0 - front
#define BIT_BACK		1//		MESH__BACK		1 - back
#define BIT_STOFF		2//		MESH__STOFF		2 - staff on leg
#define BIT_BOFF		4//		MESH__BOFF		3 - bow on shoulder
#define BIT_ARMOR		8//		MESH__ARMOR		4 - armor
#define BIT_RARM		16//	MESH__RARM		5 - right shoulder to wrist
#define BIT_RHANDHI		32//	MESH__RHANDHI	6 - right hand flat
#define BIT_STAFACTV	64//	MESH__STAFACTV	7 - right hand fist & staff stub
#define BIT_BLADSTF		128//	MESH__BLADSTF	8 - staff (active)
#define BIT_HELSTF		256//	MESH__HELSTF	9 - hellstaff
#define BIT_LARM		512//	MESH__LARM		10 - left shoulder to wrist
#define BIT_LHANDHI		1024//	MESH__LHANDHI	11 - left hand flat
#define BIT_BOWACTV		2048//	MESH__BOWACTV	12 - left hand fist & bow
#define BIT_RLEG		4096//	MESH__RLEG		13 - right leg
#define BIT_LLEG		8192//	MESH__LLEG		14 - left leg
#define BIT_HEAD		16384//	MESH__HEAD		15 - head

int Bit_for_MeshNode_player [16] =
{
	BIT_BASE2,//	0 - front
	BIT_BACK,//		1 - back
	BIT_STOFF,//	2 - staff on leg
	BIT_BOFF,//		3 - bow on shoulder
	BIT_ARMOR,//	4 - armor
	BIT_RARM,//		5 - right shoulder to wrist
	BIT_RHANDHI,//	6 - right hand flat
	BIT_STAFACTV,//	7 - right hand fist & staff stub
	BIT_BLADSTF,//	8 - staff (active)
	BIT_HELSTF,//	9 - hellstaff
	BIT_LARM,//		10 - left shoulder to wrist
	BIT_LHANDHI,//	11 - left hand flat
	BIT_BOWACTV,//	12 - left hand fist & bow
	BIT_RLEG,//		13 - right leg
	BIT_LLEG,//		14 - left leg
	BIT_HEAD,//		15 - head
};

qboolean canthrownode_player (edict_t *self, int BP, int *throw_nodes)
{//see if it's on, if so, add it to throw_nodes
	//turn it off on thrower
	if(!(self->s.fmnodeinfo[BP].flags & FMNI_NO_DRAW))
	{
		*throw_nodes |= Bit_for_MeshNode_player[BP];
		self->s.fmnodeinfo[BP].flags |= FMNI_NO_DRAW;
		return true;
	}
	return false;
}

void player_dropweapon (edict_t *self, int damage, int whichweaps)
{//FIXME: OR in the BIT_... to playerinfo->altparts!
	vec3_t handspot, forward, right, up;

	//Current code doesn't really support dropping weapons!!!
	if(deathmatch->value)
	{
		if(!((int)dmflags->value&DF_DISMEMBER))
		{
			if(self->health > 0)
			{
				return;
			}
		}
	}
	else if(self->health > 0)
		return;

	//FIXME: use refpoints for this?
	VectorClear(handspot);
	AngleVectors(self->s.angles,forward,right,up);
	VectorMA(handspot,5,forward,handspot);
	VectorMA(handspot,8,right,handspot);
	VectorMA(handspot,-6,up,handspot);

	if(whichweaps & BIT_BLADSTF && !(self->s.fmnodeinfo[MESH__BLADSTF].flags & FMNI_NO_DRAW))
	{
//		self->client->playerinfo.stafflevel = 0;
		ThrowWeapon(self, &handspot, BIT_BLADSTF, damage, 0);
		self->s.fmnodeinfo[MESH__BLADSTF].flags |= FMNI_NO_DRAW;
		self->s.fmnodeinfo[MESH__STAFACTV].flags |= FMNI_NO_DRAW;
		self->s.fmnodeinfo[MESH__RHANDHI].flags &= ~FMNI_NO_DRAW;
	}
	if(whichweaps & BIT_HELSTF && !(self->s.fmnodeinfo[MESH__HELSTF].flags & FMNI_NO_DRAW))
	{
//		self->client->playerinfo.helltype = 0;
		ThrowWeapon(self, &handspot, BIT_HELSTF, damage, 0);
		self->s.fmnodeinfo[MESH__HELSTF].flags |= FMNI_NO_DRAW;
		self->s.fmnodeinfo[MESH__STAFACTV].flags |= FMNI_NO_DRAW;
		self->s.fmnodeinfo[MESH__RHANDHI].flags &= ~FMNI_NO_DRAW;
	}
	if(whichweaps & BIT_BOWACTV && !(self->s.fmnodeinfo[MESH__BOWACTV].flags & FMNI_NO_DRAW))
	{
//		self->client->playerinfo.bowtype = 0;
		ThrowWeapon(self, &handspot, BIT_BOFF, damage, 0);
		self->s.fmnodeinfo[MESH__BOFF].flags |= FMNI_NO_DRAW;
		self->s.fmnodeinfo[MESH__BOWACTV].flags |= FMNI_NO_DRAW;
		self->s.fmnodeinfo[MESH__LHANDHI].flags &= ~FMNI_NO_DRAW;
	}
}

void player_dismember (edict_t *self, edict_t *other, int damage, int HitLocation)
{//FIXME: Make sure you can still dismember and gib player while dying
	int				throw_nodes = 0;
	vec3_t			gore_spot, right, blood_dir, blood_spot;
	qboolean dismember_ok = false;
	qboolean inpolevault = false;

	if(HitLocation & hl_MeleeHit)
	{
		dismember_ok = true;
		HitLocation &= ~hl_MeleeHit;
	}

	//dismember living players in deathmatch only if that dmflag set!
	if(deathmatch->value)
	{
		if(!((int)dmflags->value&DF_DISMEMBER))
		{
			if(self->health > 0)// && !(self->flags&FL_GODMODE))
			{
				dismember_ok = false;
			}
		}
		if(dismember_ok)
		{
			if(self->client->playerinfo.frame > FRAME_vault3 &&
				self->client->playerinfo.frame < FRAME_vault15)
				inpolevault = true;
			else
				inpolevault = false;

			if(inpolevault)
			{
				//Horizontal, in air, need to alter hitloc
				switch(HitLocation)
				{
				case hl_Head:
					HitLocation = hl_TorsoFront;
					break;
				case hl_TorsoFront:
					HitLocation = hl_TorsoFront;
					break;
				case hl_TorsoBack:
					HitLocation = hl_Head;
					break;
				}
			}

			if(self->health > 0 && !irand(0,2) &&
				HitLocation != hl_Head &&
				HitLocation != hl_ArmUpperLeft &&
				HitLocation != hl_ArmUpperRight)
			{//deathmatch hack
				if(irand(0,1))
					HitLocation = hl_ArmUpperLeft;
				else
					HitLocation = hl_ArmUpperRight;
			}
		}
	}
	else if(self->health > 0)// && !(self->flags&FL_GODMODE))
		dismember_ok = false;

	if(!dismember_ok)
	{
		if(damage <= 3 && self->health>10)
			return;

		if(damage < 10 && self->health>85)
			return;
	}

	if(HitLocation<1)
		return;

	if(HitLocation>hl_Max)
		return;

//FIXME: special manipulations of hit locations depending on anim

	VectorClear(gore_spot);
	switch(HitLocation)
	{
		case hl_Head:
			if(self->s.fmnodeinfo[MESH__HEAD].flags & FMNI_NO_DRAW)
				break;
			if(self->s.fmnodeinfo[MESH__HEAD].flags & FMNI_USE_SKIN)
				damage*=1.5;//greater chance to cut off if previously damaged

			// NOTE I AM CUTTING DOWN THE DECAP CHANCE JUST A LITTLE BIT...  HAPPENED TOO OFTEN.
//			if(flrand(0,self->health) < damage*0.5 && dismember_ok)
			if(flrand(0,self->health) < damage*0.4 && dismember_ok)
			{
//				player_dropweapon (self, (int)damage, (BIT_BOWACTV|BIT_BLADSTF|BIT_HELSTF));

				canthrownode_player(self, MESH__HEAD,&throw_nodes);

				gore_spot[2]+=18;
				ThrowBodyPart(self, &gore_spot, throw_nodes, damage, 0);

				VectorAdd(self->s.origin, gore_spot, gore_spot);
				SprayDebris(self,gore_spot,8,damage);

				if(self->health>0)
				{
					self->health = 1;
					T_Damage (self, other, other, vec3_origin, vec3_origin, vec3_origin, 10, 20,DAMAGE_AVOID_ARMOR,MOD_STAFF);
				}

				goto finish;
			}
			else
			{
//				if(flrand(0,self->health)<damage*0.25)
//					player_dropweapon (self, (int)damage, (BIT_BOWACTV|BIT_BLADSTF|BIT_HELSTF));
				self->client->playerinfo.pers.altparts |= (1<<MESH__HEAD);
				self->s.fmnodeinfo[MESH__HEAD].flags |= FMNI_USE_SKIN;
				self->s.fmnodeinfo[MESH__HEAD].skin = self->s.skinnum+1;
			}
			break;
		case hl_TorsoFront://split in half?
			if(self->s.fmnodeinfo[MESH_BASE2].flags & FMNI_NO_DRAW)
				break;
			if(self->s.fmnodeinfo[MESH_BASE2].flags & FMNI_USE_SKIN)
				damage*=1.5;//greater chance to cut off if previously damaged
			if(flrand(0,self->health)<damage*0.3&&dismember_ok)
			{
				self->client->playerinfo.flags |= (PLAYER_FLAG_NO_LARM|PLAYER_FLAG_NO_RARM);
				gore_spot[2]+=12;
				canthrownode_player(self, MESH_BASE2,&throw_nodes);
				canthrownode_player(self, MESH__BACK,&throw_nodes);
				canthrownode_player(self, MESH__LARM,&throw_nodes);
				canthrownode_player(self, MESH__RARM,&throw_nodes);
				canthrownode_player(self, MESH__HEAD,&throw_nodes);
				canthrownode_player(self, MESH__LHANDHI,&throw_nodes);
				canthrownode_player(self, MESH__RHANDHI,&throw_nodes);

//				player_dropweapon (self, (int)damage, (BIT_BOWACTV|BIT_BLADSTF|BIT_HELSTF));
				ThrowBodyPart(self, &gore_spot, throw_nodes, damage, 1);
				VectorAdd(self->s.origin, gore_spot, gore_spot);
				SprayDebris(self,gore_spot,12,damage);

				if(self->health>0)
				{
					self->health = 1;
					T_Damage (self, other, other, vec3_origin, vec3_origin, vec3_origin, 10, 20,DAMAGE_AVOID_ARMOR,MOD_STAFF);
				}
				goto finish;
			}
			else
			{
//				if(flrand(0,self->health)<damage*0.5)
//					player_dropweapon (self, (int)damage, (BIT_BOWACTV|BIT_BLADSTF|BIT_HELSTF));
				self->client->playerinfo.pers.altparts |= (1<<MESH_BASE2);
				self->s.fmnodeinfo[MESH_BASE2].flags |= FMNI_USE_SKIN;			
				self->s.fmnodeinfo[MESH_BASE2].skin = self->s.skinnum+1;
			}
			break;
		case hl_TorsoBack://split in half?
			if(self->s.fmnodeinfo[MESH__BACK].flags & FMNI_NO_DRAW)
				break;
			if(self->s.fmnodeinfo[MESH__BACK].flags & FMNI_USE_SKIN)
				damage*=1.5;//greater chance to cut off if previously damaged
			if(flrand(0,self->health)<damage*0.3&&dismember_ok)
			{
				self->client->playerinfo.flags |= (PLAYER_FLAG_NO_LARM|PLAYER_FLAG_NO_RARM);
				gore_spot[2]+=12;
				canthrownode_player(self, MESH_BASE2,&throw_nodes);
				canthrownode_player(self, MESH__BACK,&throw_nodes);
				canthrownode_player(self, MESH__LARM,&throw_nodes);
				canthrownode_player(self, MESH__RARM,&throw_nodes);
				canthrownode_player(self, MESH__HEAD,&throw_nodes);
				canthrownode_player(self, MESH__LHANDHI,&throw_nodes);
				canthrownode_player(self, MESH__RHANDHI,&throw_nodes);

//				player_dropweapon (self, (int)damage, (BIT_BOWACTV|BIT_BLADSTF|BIT_HELSTF));
				ThrowBodyPart(self, &gore_spot, throw_nodes, damage, 1);
				VectorAdd(self->s.origin, gore_spot, gore_spot);
				SprayDebris(self,gore_spot,12,damage);

				if(self->health>0)
				{
					self->health = 1;
					T_Damage (self, other, other, vec3_origin, vec3_origin, vec3_origin, 10, 20,DAMAGE_AVOID_ARMOR,MOD_STAFF);
				}
				goto finish;
			}
			else
			{
//				if(flrand(0,self->health)<damage*0.5)
//					player_dropweapon (self, (int)damage, (BIT_BOWACTV|BIT_BLADSTF|BIT_HELSTF));
				self->client->playerinfo.pers.altparts |= (1<<MESH__BACK);
				self->s.fmnodeinfo[MESH__BACK].flags |= FMNI_USE_SKIN;			
				self->s.fmnodeinfo[MESH__BACK].skin = self->s.skinnum+1;
			}
			break;
		case hl_ArmUpperLeft:
		case hl_ArmLowerLeft://left arm
			if(self->s.fmnodeinfo[MESH__LARM].flags & FMNI_NO_DRAW)
				break;
			if(self->s.fmnodeinfo[MESH__LARM].flags & FMNI_USE_SKIN)
				damage*=1.5;//greater chance to cut off if previously damaged
			if(flrand(0,self->health) < damage && dismember_ok)
			{
				if(canthrownode_player(self, MESH__LARM, &throw_nodes))
				{
					self->client->playerinfo.flags |= PLAYER_FLAG_NO_LARM;
					player_dropweapon (self, (int)damage, BIT_BOWACTV);
					canthrownode_player(self, MESH__LHANDHI, &throw_nodes);
					AngleVectors(self->s.angles,NULL,right,NULL);
					gore_spot[2]+=self->maxs[2]*0.3;
					VectorMA(gore_spot,-10,right,gore_spot);
					ThrowBodyPart(self, &gore_spot, throw_nodes, damage, 0);

					VectorSet(blood_dir, 0, -1, 0);
					VectorSet(blood_spot, 0, -12, 10);
					SpawnBleeder(self, other, blood_dir, blood_spot);//, CORVUS_LARM);
				}
			}
			else
			{
//				if(flrand(0,self->health)<damage*0.4)
//					player_dropweapon (self, (int)damage, BIT_BOWACTV);
				self->client->playerinfo.pers.altparts |= (1<<MESH__LARM);
				self->s.fmnodeinfo[MESH__LARM].flags |= FMNI_USE_SKIN;			
				self->s.fmnodeinfo[MESH__LARM].skin = self->s.skinnum+1;
			}
			break;
		case hl_ArmUpperRight:
		case hl_ArmLowerRight://right arm
			//Knock weapon out of hand?
			if(self->s.fmnodeinfo[MESH__RARM].flags & FMNI_NO_DRAW)
				break;
			if(self->s.fmnodeinfo[MESH__RARM].flags & FMNI_USE_SKIN)
				damage*=1.5;//greater chance to cut off if previously damaged
			if(flrand(0,self->health) < damage && dismember_ok)
			{
				if(canthrownode_player(self, MESH__RARM, &throw_nodes))
				{
					self->client->playerinfo.flags |= PLAYER_FLAG_NO_RARM;
					player_dropweapon (self, (int)damage, BIT_HELSTF|BIT_BLADSTF);
					canthrownode_player(self, MESH__RHANDHI, &throw_nodes);
					AngleVectors(self->s.angles,NULL,right,NULL);
					gore_spot[2]+=self->maxs[2]*0.3;
					VectorMA(gore_spot,10,right,gore_spot);
					ThrowBodyPart(self, &gore_spot, throw_nodes, damage, 0);

					VectorSet(blood_dir, 0, 1, 0);
					VectorSet(blood_spot, 0, 12, 10);
					SpawnBleeder(self, other, blood_dir, blood_spot);//, CORVUS_RARM);

					if(inpolevault)//oops!  no staff! fall down!
						P_KnockDownPlayer(&self->client->playerinfo);
				}
			}
			else
			{
//				if(flrand(0,self->health)<damage*0.75)
//					player_dropweapon (self, (int)damage, BIT_HELSTF|BIT_BLADSTF);
				self->client->playerinfo.pers.altparts |= (1<<MESH__RARM);
				self->s.fmnodeinfo[MESH__RARM].flags |= FMNI_USE_SKIN;			
				self->s.fmnodeinfo[MESH__RARM].skin = self->s.skinnum+1;
			}
			break;

		case hl_LegUpperLeft:
		case hl_LegLowerLeft://left leg
			if(self->health>0)
			{//still alive
				if(self->s.fmnodeinfo[MESH__LLEG].flags & FMNI_USE_SKIN)
					break;
				self->client->playerinfo.pers.altparts |= (1<<MESH__LLEG);
				self->s.fmnodeinfo[MESH__LLEG].flags |= FMNI_USE_SKIN;			
				self->s.fmnodeinfo[MESH__LLEG].skin = self->s.skinnum+1;
			}
			else
			{
				if(self->s.fmnodeinfo[MESH__LLEG].flags & FMNI_NO_DRAW)
					break;
				if(canthrownode_player(self, MESH__LLEG, &throw_nodes))
				{
					AngleVectors(self->s.angles,NULL,right,NULL);
					gore_spot[2]+=self->maxs[2]*0.3;
					VectorMA(gore_spot,-10,right,gore_spot);
					ThrowBodyPart(self, &gore_spot, throw_nodes, damage, 0);
				}
				break;
			}
			break;
		case hl_LegUpperRight:
		case hl_LegLowerRight://right leg
			if(self->health>0)
			{//still alive
				if(self->s.fmnodeinfo[MESH__RLEG].flags & FMNI_USE_SKIN)
					break;
				self->client->playerinfo.pers.altparts |= (1<<MESH__RLEG);
				self->s.fmnodeinfo[MESH__RLEG].flags |= FMNI_USE_SKIN;
				self->s.fmnodeinfo[MESH__RLEG].skin = self->s.skinnum+1;
			}
			else
			{
				if(self->s.fmnodeinfo[MESH__RLEG].flags & FMNI_NO_DRAW)
					break;
				if(canthrownode_player(self, MESH__RLEG, &throw_nodes))
				{
					AngleVectors(self->s.angles,NULL,right,NULL);
					gore_spot[2]+=self->maxs[2]*0.3;
					VectorMA(gore_spot,-10,right,gore_spot);
					ThrowBodyPart(self, &gore_spot, throw_nodes, damage, 0);
				}
				break;
			}
			break;

		default:
//			if(flrand(0,self->health)<damage*0.25)
//				player_dropweapon (self, (int)damage, (BIT_BOWACTV|BIT_BLADSTF|BIT_HELSTF));
			break;
	}
	if(throw_nodes)
	{
		self->pain_debounce_time = 0;
		if(!P_BranchCheckDismemberAction(&self->client->playerinfo, self->client->playerinfo.pers.weapon->tag))
		{
			P_PlayerInterruptAction(&self->client->playerinfo);
			P_PlayerAnimSetUpperSeq(&self->client->playerinfo, ASEQ_NONE);
			if(irand(0, 1))
				P_PlayerAnimSetLowerSeq(&self->client->playerinfo, ASEQ_PAIN_A);
			else
				P_PlayerAnimSetLowerSeq(&self->client->playerinfo, ASEQ_PAIN_B);
		}
	}

finish:

	SetupPlayerinfo_effects(self);
	P_PlayerUpdateModelAttributes(&self->client->playerinfo);
	WritePlayerinfo_effects(self);
}

void player_decap (edict_t *self, edict_t *other)
{
	int				throw_nodes = 0;
	vec3_t			gore_spot;

	//FIXME: special manipulations of hit locations depending on anim.

	VectorClear(gore_spot);
	if(self->s.fmnodeinfo[MESH__HEAD].flags & FMNI_NO_DRAW)
		return;

	player_dropweapon (self, 100, (BIT_BOWACTV|BIT_BLADSTF|BIT_HELSTF));

	canthrownode_player(self, MESH__HEAD,&throw_nodes);

	gore_spot[2]+=18;
	
	ThrowBodyPart(self, &gore_spot, throw_nodes, 0, 0);

	VectorAdd(self->s.origin, gore_spot, gore_spot);

	SprayDebris(self, gore_spot, 8, 100);

	if(self->health > 0)
	{
		self->health = 0;
		self->client->meansofdeath = MOD_DECAP;
		player_die(self, other, other, 100, gore_spot);
	}

	SetupPlayerinfo_effects(self);
	P_PlayerUpdateModelAttributes(&self->client->playerinfo);
	WritePlayerinfo_effects(self);
}

void player_leader_effect(void)
{
	int			i;
	int			score = 1;
	int			num_scored = 0;
	int			total_player = 0;
	edict_t		*ent;

	// if we don't want leader effects, bump outta here.
	if(!(((int)dmflags->value) & DF_SHOW_LEADER))
		return;

	// now we decide if anyone is a leader here, and if they are, we put the glow around them.
	// first, search through all clients and see what the leading score is.
	for (i=0; i<game.maxclients; i++)
	{
		ent = &g_edicts[i];
		// are we a player thats playing ?
		if (ent->client && ent->inuse)
		{
			total_player++;
			if (ent->client->resp.score == score)
				num_scored++;
			else
			if (ent->client->resp.score > score)
			{
				num_scored = 0;
				score = ent->client->resp.score;
			}
		}
	}

	// if more than 3 people have it, no one is the leader
	if ((num_scored > 3) || (total_player == num_scored))
		score = 999999;

	// now loop through and turn off the persistant effect of anyone that has below the leader score
	// and turn it on for anyone that does have it, if its not already turned on
	for (i=0; i<game.maxclients; i++)
	{
		ent = &g_edicts[i];
		// are we a player thats playing ?
		if (ent->client)
		{
			// are we a leader ?
			if (ent->client->resp.score == score && ent->inuse)
			{
				if (!ent->Leader_PersistantCFX)
					ent->Leader_PersistantCFX = gi.CreatePersistantEffect
						(&ent->s, FX_SHOW_LEADER, CEF_BROADCAST|CEF_OWNERS_ORIGIN, NULL, "" );
			}
			// if not, then if we have the effect, remove it
			else
			if (ent->Leader_PersistantCFX)
			{
				gi.RemovePersistantEffect(ent->Leader_PersistantCFX, REMOVE_LEADER);
				gi.RemoveEffects(&ent->s, FX_SHOW_LEADER);
				ent->Leader_PersistantCFX =0;
			}

		}
	}

}


// ************************************************************************************************
// ClientObituary
// --------------
// ************************************************************************************************

static const short KillSelf[MOD_MAX] =
{
	0,			// MOD_UNKNOWN		 

	0,			// MOD_STAFF		 
	0,			// MOD_FIREBALL	 
	0,			// MOD_MMISSILE	 
	0,			// MOD_SPHERE		 
	0,			// MOD_SPHERE_SPL	 
	0,			// MOD_IRONDOOM	 
	0,			// MOD_FIREWALL	 
	0,			// MOD_STORM		 
	0,			// MOD_PHOENIX		 
	0,			// MOD_PHOENIX_SPL	 
	0,			// MOD_HELLSTAFF	 

	0,			// MOD_P_STAFF		 
	0,			// MOD_P_FIREBALL	 
	0,			// MOD_P_MMISSILE	 
	0,			// MOD_P_SPHERE	 
	0,			// MOD_P_SPHERE_SPL 
	0,			// MOD_P_IRONDOOM	 
	0,			// MOD_P_FIREWALL	 
	0,			// MOD_P_STORM		 
	0,			// MOD_P_PHOENIX	 
	0,			// MOD_P_PHOENIX_SPL
	0,			// MOD_P_HELLSTAFF	 

	0,			// MOD_KICKED		 
	0,			// MOD_METEORS		 
	0,			// MOD_ROR			 
	0,			// MOD_SHIELD		 
	0,			// MOD_CHICKEN		 
	0,			// MOD_TELEFRAG	 
	GM_OBIT_WATER,			// MOD_WATER		 
	GM_OBIT_SLIME,			// MOD_SLIME		 
	GM_OBIT_LAVA,			// MOD_LAVA		 
	GM_OBIT_CRUSH,			// MOD_CRUSH		 
	GM_OBIT_FALLING,		// MOD_FALLING		 
	GM_OBIT_SUICIDE,		// MOD_SUICIDE		 
	GM_OBIT_BARREL,			// MOD_BARREL		 
	GM_OBIT_EXIT,			// MOD_EXIT		 
	GM_OBIT_BURNT,			// MOD_BURNT		 
	GM_OBIT_BLEED,			// MOD_BLEED		 
	0,			// MOD_SPEAR		 
	0,			// MOD_DIED		 
	GM_OBIT_EXPL,			// MOD_KILLED_SLF	 
	0,			// MOD_DECAP
	GM_OBIT_TORN_SELF	//MOD_TORN
};

static const short KillBy[MOD_MAX] = 
{
	0,			// MOD_UNKNOWN		 

	GM_OBIT_STAFF,			// MOD_STAFF		 
	GM_OBIT_FIREBALL,	  	// MOD_FIREBALL	 
	GM_OBIT_MMISSILE,	  	// MOD_MMISSILE	 
	GM_OBIT_SPHERE,			// MOD_SPHERE		 
	GM_OBIT_SPHERE_SPL,	  	// MOD_SPHERE_SPL	 
	GM_OBIT_IRONDOOM,	  	// MOD_IRONDOOM	 
	GM_OBIT_FIREWALL,	  	// MOD_FIREWALL	 
	GM_OBIT_STORM,			// MOD_STORM		 
	GM_OBIT_PHOENIX,	  	// MOD_PHOENIX		 
	GM_OBIT_PHOENIX_SPL,  	// MOD_PHOENIX_SPL	 
	GM_OBIT_HELLSTAFF,	  	// MOD_HELLSTAFF	 

	GM_OBIT_STAFF,			// MOD_P_STAFF		 
	GM_OBIT_FIREBALL,		// MOD_P_FIREBALL	 
	GM_OBIT_MMISSILE,		// MOD_P_MMISSILE	 
	GM_OBIT_SPHERE,			// MOD_P_SPHERE	 
	GM_OBIT_SPHERE_SPL,		// MOD_P_SPHERE_SPL 
	GM_OBIT_IRONDOOM,		// MOD_P_IRONDOOM	 
	GM_OBIT_FIREWALL,		// MOD_P_FIREWALL	 
	GM_OBIT_STORM,			// MOD_P_STORM		 
	GM_OBIT_PHOENIX,		// MOD_P_PHOENIX	 
	GM_OBIT_PHOENIX_SPL,	// MOD_P_PHOENIX_SPL
	GM_OBIT_HELLSTAFF,		// MOD_P_HELLSTAFF	 

	GM_OBIT_KICKED,			// MOD_KICKED		 
	GM_OBIT_METEORS,		// MOD_METEORS		 
	GM_OBIT_ROR,			// MOD_ROR			 
	GM_OBIT_SHIELD,			// MOD_SHIELD		 
	GM_OBIT_CHICKEN,		// MOD_CHICKEN		 
	GM_OBIT_TELEFRAG,		// MOD_TELEFRAG	 
	0,			// MOD_WATER		 
	0,			// MOD_SLIME		 
	0,			// MOD_LAVA		 
	0,			// MOD_CRUSH		 
	0,			// MOD_FALLING		 
	0,			// MOD_SUICIDE		 
	0,			// MOD_BARREL		 
	0,			// MOD_EXIT		 
	GM_OBIT_BURNT,			// MOD_BURNT		 
	GM_OBIT_BLEED,			// MOD_BLEED		 
	0,			// MOD_SPEAR		 
	0,			// MOD_DIED		 
	0,			// MOD_KILLED_SLF	 
	0,			// MOD_DECAP
	GM_OBIT_TORN	//MOD_TORN
};

void ClientObituary(edict_t *self, edict_t *inflictor, edict_t *attacker)
{
	short		message;
	int			friendlyFire;

	assert(self->client);

	if(!(deathmatch->value || coop->value))
	{
		// No obituaries in single player.

		return;
	}

	friendlyFire=self->client->meansofdeath&MOD_FRIENDLY_FIRE;
	self->client->meansofdeath&=~MOD_FRIENDLY_FIRE;

	if(deathmatch->value || coop->value)
	{
		self->enemy = attacker;
		
		if(attacker && attacker->client && attacker != self)
		{
			message = KillBy[self->client->meansofdeath];

			if(message)
			{
				gi.Obituary(PRINT_MEDIUM, (short)(message + irand(0, 2)), self->s.number, attacker->s.number);
				
				if(deathmatch->value)
				{
					if(friendlyFire)
						attacker->client->resp.score--;
					else
				   		attacker->client->resp.score++;

					player_leader_effect();
				}
				return;
			}
		}

		// Wasn't an awarded a frag, check for suicide messages.
		message = KillSelf[self->client->meansofdeath];
		
		if(message)
		{
			gi.Obituary(PRINT_MEDIUM, (short)(message + irand(0, 2)), self->s.number, 0);

			if(deathmatch->value)
			{
				self->client->resp.score--;
			   	player_leader_effect();
			}
			
			self->enemy = NULL;
			return;
		}
	}

	gi.Obituary(PRINT_MEDIUM, (short)(GM_OBIT_DIED + irand(0, 2)), self->s.number, 0);	

	if (deathmatch->value)
	{
		self->client->resp.score--;
		player_leader_effect();
	}
}

void player_make_gib(edict_t *self, edict_t *attacker)
{
	byte		magb;
	float		mag;
	int	i, num_limbs;

	if(self->client)
	{
		//FIXME: Have a generic GibParts effect that throws flesh and several body parts - much cheaper.

		num_limbs = irand(1, 3);
		for(i = 0; i < num_limbs; i++)
			player_dismember(self, attacker, flrand(80, 160), irand(hl_Head, hl_LegLowerRight) | hl_MeleeHit);
	}

	mag = VectorLength(self->mins);
	magb = Clamp(mag, 1.0, 255.0);
   	
	gi.CreateEffect(NULL,
					FX_FLESH_DEBRIS,
   					0,
   					self->s.origin,
   					"bdb",
   					irand(10, 30), self->mins, magb);

   	self->takedamage = DAMAGE_NO;
}

int player_die(edict_t *self, edict_t *inflictor, edict_t *attacker,int damage,vec3_t point)
{//FIXME: Make sure you can still dismember and gib player while dying
	int		i;

	assert(self->client);

	VectorClear(self->avelocity);

	if(self->health < -99)
		self->health = -99;//looks better on stat bar display

	self->takedamage=DAMAGE_NO;
	self->movetype=PHYSICSTYPE_STEP;

	self->s.angles[PITCH]=0.0;
	self->s.angles[ROLL]=0.0;

	self->s.sound=0;

	self->maxs[2]=-8;

	self->solid=SOLID_NOT;

	// tell the leader client effect that this client is dead - so if we drawing the effect, please stop.
	self->s.effects |= EF_CLIENT_DEAD;

	// Get the player off of the rope!

	if (self->client->playerinfo.flags & PLAYER_FLAG_ONROPE)
	{
		// Turn off the rope graphic immediately.

		self->targetEnt->count = 0;
		self->targetEnt->rope_grab->s.effects &= ~EF_ALTCLIENTFX;

		self->monsterinfo.jump_time = level.time + 10;
		self->client->playerinfo.flags |= PLAYER_FLAG_RELEASEROPE;
		self->client->playerinfo.flags &= ~PLAYER_FLAG_ONROPE;

		self->client->playerinfo.flags |= PLAYER_FLAG_FALLBREAK | PLAYER_FLAG_FALLING;
	}
	// Get rid of the player's persistent effect.
	if (self->PersistantCFX)
	{	
		gi.RemovePersistantEffect(self->PersistantCFX, REMOVE_DIE);
		self->PersistantCFX = 0;
	}

	if (self->Leader_PersistantCFX)
	{
		gi.RemovePersistantEffect(self->Leader_PersistantCFX, REMOVE_LEADER_DIE);
		self->Leader_PersistantCFX =0;
	}

	// remove any persistant meteor effects
	for (i=0; i<4; i++)
	{
		if (self->client->Meteors[i])
		{
			if (self->client->Meteors[i]->PersistantCFX)
			{
				gi.RemovePersistantEffect(self->client->Meteors[i]->PersistantCFX, REMOVE_METEOR);
				gi.RemoveEffects(&self->s, FX_SPELL_METEORBARRIER+i);
				self->client->Meteors[i]->PersistantCFX = 0;
			}
			G_SetToFree(self->client->Meteors[i]);
			self->client->Meteors[i] = NULL;
		}
	}
	// we now own no meteors at all
	self->client->playerinfo.meteor_count = 0;

	// Create a persistant FX_REMOVE_EFFECTS effect - this is a special hack. If we just created
	// a regular FX_REMOVE_EFFECTS effect, it will overwrite the next FX_PLAYER_PERSISTANT sent
	// out. Luverly jubberly!!!
	gi.CreatePersistantEffect(&self->s,FX_REMOVE_EFFECTS,CEF_BROADCAST|CEF_OWNERS_ORIGIN,NULL,"s",0); 

	// Get rid of all the stuff set up in PlayerFirstSeenInit...
	gi.RemoveEffects(&self->s, FX_SHADOW);
	gi.RemoveEffects(&self->s, FX_WATER_PARTICLES);
	gi.RemoveEffects(&self->s, FX_CROSSHAIR);

	// Remove any shrine effects we have going.
	PlayerKillShrineFX(self);
	
	// Remove any sound effects we may be generating.
	gi.sound(self, CHAN_WEAPON, gi.soundindex("misc/null.wav"), 1, ATTN_NORM,0);

	if((self->health<-40) && !(self->flags & FL_CHICKEN))
	{	
		gi.sound(self,CHAN_BODY,gi.soundindex("*gib.wav"),1,ATTN_NORM,0);
		
		player_make_gib(self, attacker);
	   
		self->s.modelindex=0;
		// Won`t get sent to client if mi 0 unless flag is set
		self->svflags |= SVF_ALWAYS_SEND;
		self->s.effects |= EF_NODRAW_ALWAYS_SEND | EF_ALWAYS_ADD_EFFECTS;
	   	self->deadflag=DEAD_DEAD;
	   	
		self->client->playerinfo.deadflag=DEAD_DEAD;
	}
	else
	{	
		// Make player die a normal death.

		self->health=-1;

		if(!self->deadflag)
		{
			self->client->respawn_time=level.time+1.0;
			self->client->ps.pmove.pm_type=PM_DEAD;
	
			// If player died in a deathmatch or coop, show scores.

			Cmd_Score_f(self);
	
   			// Check if a chicken?

   			if (self->flags & FL_CHICKEN)
   			{
				// We're a chicken, so die a chicken's death.

   				PlayerChickenDeath(self);
   				player_make_gib(self, attacker);
  		   		self->s.modelindex=0;
				// Won`t get sent to client if mi 0 unless flag is set
				self->svflags |= SVF_ALWAYS_SEND;
				self->s.effects |= EF_NODRAW_ALWAYS_SEND | EF_ALWAYS_ADD_EFFECTS;
   			}
			else if ( (self->client->playerinfo.flags & PLAYER_FLAG_SURFSWIM) || (self->waterlevel >= 2) )
   			{
   				P_PlayerAnimSetLowerSeq(&self->client->playerinfo, ASEQ_DROWN);
   				gi.sound(self,CHAN_BODY,gi.soundindex("*drowndeath.wav"),1,ATTN_NORM,0);
   			}
			else if ( !stricmp(inflictor->classname, "plague_mist"))
			{
   				P_PlayerAnimSetLowerSeq(&self->client->playerinfo, ASEQ_DEATH_CHOKE);
				gi.sound(self,CHAN_BODY,gi.soundindex("*chokedeath.wav"),1,ATTN_NORM,0);
			}
			else if ( self->fire_damage_time == -1 )
			{
   				P_PlayerAnimSetLowerSeq(&self->client->playerinfo, ASEQ_DEATH_B);
				if (blood_level && (int)(blood_level->value) <= VIOLENCE_BLOOD)	// Don't scream bloody murder in Germany. 
					gi.sound(self,CHAN_BODY,gi.soundindex("*death1.wav"),1,ATTN_NORM,0);
				else
					gi.sound(self,CHAN_BODY,gi.soundindex("*firedeath.wav"),1,ATTN_NORM,0);
			}
   			else
   			{	// "Normal" deaths.
				vec3_t fwd;
				float speed;

				// Check if the player had a velocity forward or backward during death.
				AngleVectors(self->s.angles, fwd, NULL, NULL);
				speed = DotProduct(fwd, self->velocity);
				speed += flrand(-16.0, 16.0);		// Add a spot of randomness to it.

				if (speed > 16.0)
				{	// Fly forward
					P_PlayerAnimSetLowerSeq(&self->client->playerinfo, ASEQ_DEATH_FLYFWD);
				}
				else if (speed < -16.0)
				{	// Fly backward
					P_PlayerAnimSetLowerSeq(&self->client->playerinfo, ASEQ_DEATH_FLYBACK);
				}
				else
				{	// Jes' flop to the ground.
					P_PlayerAnimSetLowerSeq(&self->client->playerinfo, ASEQ_DEATH_A);
				}

   				if (irand(0,1))
   					gi.sound(self,CHAN_BODY,gi.soundindex("*death1.wav"),1,ATTN_NORM,0);
   				else
   					gi.sound(self,CHAN_BODY,gi.soundindex("*death2.wav"),1,ATTN_NORM,0);
   			}

			// Make sure it doesn't try and finish an animation.

			P_PlayerAnimSetUpperSeq(&self->client->playerinfo, ASEQ_NONE);
			self->client->playerinfo.upperidle = true;

			// If we're not a chicken, don't set the dying flag.

			if (!(self->client->playerinfo.edictflags & FL_CHICKEN))	// We're not set as a chicken
			{
				// Not a chicken so set the dying flag.

				self->deadflag=DEAD_DYING;
				self->client->playerinfo.deadflag=DEAD_DYING;
			}
			else
			{
				// I WAS a chicken, but not any more, I'm dead and an Elf again.

				self->client->playerinfo.edictflags &= ~FL_CHICKEN;
			}
		}
	}

	ClientObituary(self, inflictor, attacker);

	gi.linkentity(self);

	return(0);
}

/*
=======================================================================

  SelectSpawnPoint

=======================================================================
*/

/*
================
PlayersRangeFromSpot

Returns the distance to the nearest player from the given spot
================
*/
float	PlayersRangeFromSpot (edict_t *spot)
{
	edict_t	*player;
	float	bestplayerdistance;
	vec3_t	v;
	int		n;
	float	playerdistance;


	bestplayerdistance = 9999999;

	for (n = 1; n <= maxclients->value; n++)
	{
		player = &g_edicts[n];

		if (!player->inuse)
			continue;

		if (player->health <= 0)
			continue;

		VectorSubtract (spot->s.origin, player->s.origin, v);
		playerdistance = VectorLength (v);

		if (playerdistance < bestplayerdistance)
			bestplayerdistance = playerdistance;
	}

	return bestplayerdistance;
}

/*
================
SelectRandomDeathmatchSpawnPoint

go to a random point, but NOT the two points closest
to other players
================
*/
edict_t *SelectRandomDeathmatchSpawnPoint (void)
{
	edict_t	*spot, *spot1, *spot2;
	int		count = 0;
	int		selection;
	float	range, range1, range2;

	spot = NULL;
	range1 = range2 = 99999;
	spot1 = spot2 = NULL;

	while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL)
	{
//		if(spot->damage_debounce_time > level.time)
//			continue;//was just used

//		if(irand(0,1))
//			continue;//50% chance to skip it- helps make it more random

		count++;
		range = PlayersRangeFromSpot(spot);
		if (range < range1)
		{
			range1 = range;
			spot1 = spot;
		}
		else if (range < range2)
		{
			range2 = range;
			spot2 = spot;
		}
	}

	if (!count)
		return NULL;

	if (count <= 2)
	{
		spot1 = spot2 = NULL;
	}
	else
		count -= 2;

	selection = irand(0, count -1 );

	spot = NULL;
	do
	{
		spot = G_Find (spot, FOFS(classname), "info_player_deathmatch");
		if (spot == spot1 || spot == spot2)
			selection++;
	} while(selection--);

	return spot;
}

/*
================
SelectFarthestDeathmatchSpawnPoint

================
*/
edict_t *SelectFarthestDeathmatchSpawnPoint (void)
{
	edict_t	*bestspot;
	float	bestdistance, bestplayerdistance;
	edict_t	*spot;


	spot = NULL;
	bestspot = NULL;
	bestdistance = 0;
	while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL)
	{
//		if(spot->damage_debounce_time > level.time)
//			continue;//was just used

		bestplayerdistance = PlayersRangeFromSpot (spot);

		if (bestplayerdistance > bestdistance)
		{
			bestspot = spot;
			bestdistance = bestplayerdistance;
		}
	}

	if (bestspot)
	{
		return bestspot;
	}

	// if there is a player just spawned on each and every start spot
	// we have no choice to turn one into a telefrag meltdown
	spot = G_Find (NULL, FOFS(classname), "info_player_deathmatch");

	return spot;
}

edict_t *SelectDeathmatchSpawnPoint (void)
{
//	if ( (int)(dmflags->value) & DF_SPAWN_FARTHEST)
	return SelectFarthestDeathmatchSpawnPoint ();
//	else
//		return SelectRandomDeathmatchSpawnPoint ();
}

edict_t *SelectCoopSpawnPoint (edict_t *ent)
{
	int		index;
	edict_t	*spot = NULL;
	char	*target;

	index = ent->client - game.clients;

	// Player 0 starts in normal player spawn point.

	if (!index)
		return NULL;

	spot = NULL;

	// Assume there are four coop spots at each spawnpoint.

	while (1)
	{
		spot = G_Find (spot, FOFS(classname), "info_player_coop");
			
		if (!spot)
			return NULL;	// we didn't have enough...

//		try not to use the same spot for a bit to prevent telefrags
//		if(spot->damage_debounce_time > level.time)
//			continue;

		target = spot->targetname;
		if (!target)
			target = "";
		if ( Q_stricmp(game.spawnpoint, target) == 0 )
		{	
			// This is a coop spawn point for one of the clients here.

			index--;
			
			if (!index)
				return spot;// this is it
		}
	}

	return spot;
}

/*
===========
SelectSpawnPoint

Chooses a player start, deathmatch start, coop start, etc
============
*/
void	SelectSpawnPoint (edict_t *ent,vec3_t origin, vec3_t angles)
{
	edict_t	*spot = NULL;
	trace_t	tr;
	vec3_t	endpos;

	if (deathmatch->value)
		spot = SelectDeathmatchSpawnPoint ();
	else if (coop->value)
		spot = SelectCoopSpawnPoint (ent);

	// Find a single player start spot.

	if (!spot)
	{
		while ((spot = G_Find (spot, FOFS(classname), "info_player_start")) != NULL)
		{
			if (!game.spawnpoint[0] && !spot->targetname)
				break;

			if (!game.spawnpoint[0] || !spot->targetname)
				continue;

			if (Q_stricmp(game.spawnpoint, spot->targetname) == 0)
				break;
		}

		if (!spot)
		{
			if (!game.spawnpoint[0])
			{
				// There wasn't a spawnpoint without a target, so use any.

				spot = G_Find (spot, FOFS(classname), "info_player_start");
			}
			if (!spot)
				gi.error ("Couldn't find spawn point %s\n", game.spawnpoint);
		}
	}

	//debounce tim eon use to help prevent telefragging
	//spot->damage_debounce_time = level.time + 0.3;

	// Do a trace to the floor to find where to put player.

	VectorCopy(spot->s.origin, endpos);
	endpos[2] -= 1000;
	gi.trace (spot->s.origin, vec3_origin, vec3_origin, endpos, NULL, CONTENTS_WORLD_ONLY|MASK_PLAYERSOLID,&tr);

	VectorCopy(tr.endpos,origin);
	origin[2] -= mins[2];

	// ???

	VectorCopy (spot->s.angles, angles);
}

//======================================================================

int body_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	BecomeDebris(self);
	return true;
}

void InitBodyQue (void)
{
	int		i;
	edict_t	*ent;

	level.body_que = 0;

	for (i=0; i<BODY_QUEUE_SIZE ; i++)
	{
		ent = G_Spawn();
		ent->classname = "bodyque";
	}
}

int player_body_die(edict_t *self,edict_t *inflictor,edict_t *attacker,int damage,vec3_t point)
{
	byte	magb;
	float	mag;
	vec3_t	mins;

	VectorCopy(self->mins,mins);
	mins[2]=-30.0;

	gi.sound(self,CHAN_BODY,gi.soundindex("misc/fleshbreak.wav"),1,ATTN_NORM,0);

	mag = VectorLength(mins);
	magb = Clamp(mag, 1.0, 255.0);
   	
	gi.CreateEffect(NULL,
					FX_FLESH_DEBRIS,
   					0,
   					self->s.origin,
   					"bdb",
   					irand(10, 30), mins, magb);

	gi.unlinkentity(self);

	VectorClear(self->mins);
	VectorClear(self->maxs);
	VectorClear(self->absmin);
	VectorClear(self->absmax);
	VectorClear(self->size);
	self->movetype=PHYSICSTYPE_NONE;
	self->solid=SOLID_NOT;
	self->clipmask=0;
	self->takedamage=DAMAGE_NO;
	self->materialtype=MAT_NONE;
	self->health=0;
	self->die=NULL;
	self->deadflag=DEAD_DEAD;
   	self->s.modelindex=0;

	gi.linkentity(self);

	return(1);
}

void CopyToBodyQue (edict_t *ent)
{
	edict_t	*body;
	vec3_t	origin;

	// Grab a body que and cycle to the next one.

	if(!ent->s.modelindex)
	{
		// Safety - was gibbed?

		return;
	}

	if(level.body_que == -1)
	{
		VectorCopy(ent->s.origin, origin);
		origin[2] += (ent->mins[2] + 8.0f);
		
		// Put in the pretty effect when removing the corpse first.

		gi.CreateEffect(NULL, FX_CORPSE_REMOVE, 0, origin, "");
		
		// No body que on this level.

		return;
	}

	body = &g_edicts[((int)maxclients->value) + 1 + level.body_que];
	level.body_que = (level.body_que + 1) % BODY_QUEUE_SIZE;

	gi.unlinkentity (ent);

	// If the body was being used, then lets put an effect on it before removing it.

	if (body->inuse&&(body->s.modelindex!=0))
	{
		VectorCopy(body->s.origin, origin);
		origin[2] += (body->mins[2] + 8.0f);
		gi.CreateEffect(NULL, FX_CORPSE_REMOVE, 0, origin, "");
	}

	gi.unlinkentity (body);

	body->s=ent->s;
	body->s.number=body-g_edicts;
	body->s.skeletalType=SKEL_NULL;
	body->s.effects&=~(EF_JOINTED|EF_SWAPFRAME);
	body->s.rootJoint=NULL_ROOT_JOINT;
	body->s.swapFrame=NO_SWAP_FRAME;
	body->owner=ent->owner;
	VectorScale(ent->mins,0.5,body->mins);
	VectorScale(ent->maxs,0.5,body->maxs);
	body->maxs[2]=10;
	VectorCopy(ent->absmin,body->absmin);
	VectorCopy(ent->absmax,body->absmax);
	body->absmax[2]=10;
	VectorCopy(ent->size,body->size);
	body->svflags=ent->svflags|SVF_DEADMONSTER; // Stops player getting stuck.
	body->movetype=PHYSICSTYPE_STEP;
	body->solid=SOLID_BBOX;
	body->clipmask=MASK_PLAYERSOLID;
	body->takedamage=DAMAGE_YES;
	body->materialtype=MAT_FLESH;
	body->health=25;
	body->deadflag=DEAD_NO;
	body->die=player_body_die;

	gi.linkentity (body);

	// Clear out any client effectsBuffer_t on the corpse (inherited from the player who just died)
	// as the engine will take care of deallocating any effects still on the player.

	memset(&body->s.clientEffects,0,sizeof(EffectsBuffer_t));

	// FIXME: Re-create certain client effects that were on the player when he died (e.g. fire).

	return;
}

void respawn (edict_t *self)
{
	if (deathmatch->value || coop->value)
	{
		// FIXME: make bodyque objects obey gravity.

		if(!(self->flags & FL_CHICKEN) && !((int)dm_no_bodies->value))
		{
			// We're not set as a chicken, so duplicate ourselves.

			CopyToBodyQue (self);
		}

		// Create a persistant FX_REMOVE_EFFECTS effect - this is a special hack. If we just created
		// a regular FX_REMOVE_EFFECTS effect, it will overwrite the next FX_PLAYER_PERSISTANT sent
		// out. Luverly jubberly!!!

		gi.CreatePersistantEffect(&self->s,FX_REMOVE_EFFECTS,CEF_BROADCAST|CEF_OWNERS_ORIGIN,NULL,"s",0); 
	
		if(deathmatch->value)
		{
			// Respawning in deathmatch always means a complete reset of the player's model.

			self->client->complete_reset=1;
		}
		else if(coop->value)
		{
			// Respawning in coop always means a partial reset of the player's model.

			self->client->complete_reset=0;
		}

		PutClientInServer (self);

		// Do the teleport sound.

		gi.sound(self,CHAN_WEAPON,gi.soundindex("weapons/teleport.wav"),1,ATTN_NORM,0);
	
		// Add a teleportation effect.

		gi.CreateEffect(&self->s, FX_PLAYER_TELEPORT_IN, CEF_OWNERS_ORIGIN, self->s.origin, NULL);

		// Hold in place briefly.

		self->client->ps.pmove.pm_time = 50;


		return;
	}

	// Restart the entire server.

	gi.AddCommandString ("menu_loadgame\n");
}

//==============================================================


extern void PlayerRestartShrineFX(edict_t *self);


void SpawnInitialPlayerEffects(edict_t *ent)
{
	PlayerRestartShrineFX(ent);

	// Don't need to keep track of this persistant effect, since its started but never stopped.

	gi.CreatePersistantEffect(&ent->s, FX_PLAYER_PERSISTANT, 
		CEF_BROADCAST | CEF_OWNERS_ORIGIN, NULL, ""); 

	if (deathmatch->value || coop->value)
	   	player_leader_effect();
}

/*
==================
SaveClientData

Some information that should be persistant, like health, is still stored in the edict structure, so
it needs to be mirrored out to the game.client structure(s) before all the edicts are wiped.
==================
*/

void SaveClientData (void)
{
	int		i;
	edict_t	*ent;

	for(i=0;i<game.maxclients;i++)
	{
		ent=&g_edicts[1+i];

		if(!ent->inuse)
			continue;

		game.clients[i].playerinfo.pers.health=ent->health;
		if (coop->value && game.clients[i].playerinfo.pers.health < 25)
			game.clients[i].playerinfo.pers.health=25;
		game.clients[i].playerinfo.pers.max_health=ent->max_health;

		game.clients[i].playerinfo.pers.mission_num1 = ent->client->ps.mission_num1;
		game.clients[i].playerinfo.pers.mission_num2 = ent->client->ps.mission_num2;
		
		if(coop->value)
			game.clients[i].playerinfo.pers.score=ent->client->resp.score;
	}
}

/*
==================
FetchClientEntData
==================
*/

void FetchClientEntData (edict_t *ent)
{
	ent->health=ent->client->playerinfo.pers.health;
	if (coop->value && ent->health < 25)
		ent->health = 25;
	ent->max_health=ent->client->playerinfo.pers.max_health;

	ent->client->ps.mission_num1 = ent->client->playerinfo.pers.mission_num1;
	ent->client->ps.mission_num2 = ent->client->playerinfo.pers.mission_num2;
	
	if(coop->value)
		ent->client->resp.score=ent->client->playerinfo.pers.score;
}

// ************************************************************************************************
// GiveLevelItems
// --------------
// If additional starting weapons and defences are specified by the current map, give them to the
// player (to support players joining a coop game midway through).
// ************************************************************************************************

void GiveLevelItems(edict_t *player)
{
	gclient_t	*client;
	gitem_t		*item,*weapon;

	client=player->client;

	weapon=NULL;

	if(level.offensive_weapons&1)
	{	
		item=P_FindItem("staff");
		if(AddWeaponToInventory(item,player))
		{
			if((ITEM_INDEX(item) > ITEM_INDEX(weapon))&&(client->playerinfo.pers.autoweapon))
			{
				weapon=item;
				client->playerinfo.pers.newweapon=item;
				client->playerinfo.switchtoweapon=WEAPON_READY_SWORDSTAFF;
			}
		}
	}

	if(level.offensive_weapons&2)
	{	
		item=P_FindItem("fball");
		if(AddWeaponToInventory(item,player))
		{
			if((ITEM_INDEX(item) > ITEM_INDEX(weapon))&&(client->playerinfo.pers.autoweapon))
			{
				weapon=item;
				client->playerinfo.pers.newweapon=item;
				client->playerinfo.switchtoweapon=WEAPON_READY_HANDS;
			}
		}
	}

	if(level.offensive_weapons&4)
	{	
		item=P_FindItem("hell");
		if(AddWeaponToInventory(item,player))
		{
			if((ITEM_INDEX(item) > ITEM_INDEX(weapon))&&(client->playerinfo.pers.autoweapon))
			{
				weapon=item;
				client->playerinfo.pers.newweapon=item;
				client->playerinfo.switchtoweapon=WEAPON_READY_HELLSTAFF;
			}
		}
	}

	if(level.offensive_weapons&8)
	{	
		item=P_FindItem("array");
		if(AddWeaponToInventory(item,player))
		{
			if((ITEM_INDEX(item) > ITEM_INDEX(weapon))&&(client->playerinfo.pers.autoweapon))
			{
				weapon=item;
				client->playerinfo.pers.newweapon=item;
				client->playerinfo.switchtoweapon=WEAPON_READY_HANDS;
			}
		}
	}

	if(level.offensive_weapons&16)
	{	
		item=P_FindItem("rain");
		if(AddWeaponToInventory(item,player))
		{
			if((ITEM_INDEX(item) > ITEM_INDEX(weapon))&&(client->playerinfo.pers.autoweapon))
			{
				weapon=item;
				client->playerinfo.pers.newweapon=item;
				client->playerinfo.switchtoweapon=WEAPON_READY_BOW;
			}
		}
	}

	if(level.offensive_weapons&32)
	{	
		item=P_FindItem("sphere");
		if(AddWeaponToInventory(item,player))
		{
			if((ITEM_INDEX(item) > ITEM_INDEX(weapon))&&(client->playerinfo.pers.autoweapon))
			{
				weapon=item;
				client->playerinfo.pers.newweapon=item;
				client->playerinfo.switchtoweapon=WEAPON_READY_HANDS;
			}
		}
	}

	if(level.offensive_weapons&64)
	{	
		item=P_FindItem("phoen");
		if(AddWeaponToInventory(item,player))
		{
			if((ITEM_INDEX(item) > ITEM_INDEX(weapon))&&(client->playerinfo.pers.autoweapon))
			{
				weapon=item;
				client->playerinfo.pers.newweapon=item;
				client->playerinfo.switchtoweapon=WEAPON_READY_BOW;
			}
		}
	}

	if(level.offensive_weapons&128)
	{	
		item=P_FindItem("mace");
		if(AddWeaponToInventory(item,player))
		{
			if((ITEM_INDEX(item) > ITEM_INDEX(weapon))&&(client->playerinfo.pers.autoweapon))
			{
				weapon=item;
				client->playerinfo.pers.newweapon=item;
				client->playerinfo.switchtoweapon=WEAPON_READY_HANDS;
			}
		}
	}

	if(level.offensive_weapons&256)
	{	
		item=P_FindItem("fwall");
		if(AddWeaponToInventory(item,player))
		{
			if((ITEM_INDEX(item) > ITEM_INDEX(weapon))&&(client->playerinfo.pers.autoweapon))
			{
				weapon=item;
				client->playerinfo.pers.newweapon=item;
				client->playerinfo.switchtoweapon=WEAPON_READY_HANDS;
			}
		}
	}

	if(level.defensive_weapons&1)
	{	
		item=P_FindItem("ring");
		AddDefenseToInventory(item,player);
	}

	if(level.defensive_weapons&2)
	{
		item=P_FindItem("lshield");
		AddDefenseToInventory(item,player);
	}

	if(level.defensive_weapons&4)
	{	
		item=P_FindItem("tele");
		AddDefenseToInventory(item,player);
	}

	if(level.defensive_weapons&8)
	{	
		item=P_FindItem("morph");
		AddDefenseToInventory(item,player);
	}

	if(level.defensive_weapons&16)
	{	
		item=P_FindItem("meteor");
		AddDefenseToInventory(item,player);	
	}

	SetupPlayerinfo_effects(player);
	P_PlayerUpdateModelAttributes(&player->client->playerinfo);
	WritePlayerinfo_effects(player);
}

// ************************************************************************************************
// InitClientPersistant
// --------------------
// ************************************************************************************************

void InitClientPersistant(edict_t *player)
{
	gclient_t *client;
	gitem_t	*item;

	client=player->client;

	memset(&client->playerinfo.pers, 0, sizeof(client->playerinfo.pers));	

	// ********************************************************************************************
	// Set up player's health.
	// ********************************************************************************************

	client->playerinfo.pers.health = 100;
	
	// ********************************************************************************************
	// Set up maximums amounts for health, mana and ammo for bows and hellstaff.
	// ********************************************************************************************

	client->playerinfo.pers.max_health		= 100;
	client->playerinfo.pers.max_offmana		= MAX_OFF_MANA;
	client->playerinfo.pers.max_defmana		= MAX_DEF_MANA;
	client->playerinfo.pers.max_redarrow	= MAX_RAIN_AMMO;
	client->playerinfo.pers.max_phoenarr	= MAX_PHOENIX_AMMO;
	client->playerinfo.pers.max_hellstaff	= MAX_HELL_AMMO;

	// ********************************************************************************************
	// Give defensive and offensive weapons to player.
	// ********************************************************************************************

	client->playerinfo.pers.weapon=0;
	client->playerinfo.pers.defence=0;

	// Give just the sword-staff and flying-fist to the player as starting weapons.

	item = P_FindItem("staff");
	AddWeaponToInventory(item,player);
	client->playerinfo.pers.selected_item = ITEM_INDEX(item);
	client->playerinfo.pers.weapon = item;
	client->playerinfo.pers.lastweapon = item;
	client->playerinfo.weap_ammo_index = 0;

	if(!(((int)dmflags->value)&DF_NO_OFFENSIVE_SPELL))
	{
		item=P_FindItem("fball");
		AddWeaponToInventory(item,player);
		client->playerinfo.pers.selected_item = ITEM_INDEX(item);
		client->playerinfo.pers.weapon = item;
		client->playerinfo.pers.lastweapon = item;
		client->playerinfo.weap_ammo_index = ITEM_INDEX(P_FindItem(item->ammo));
	}

	item=P_FindItem("powerup");
	AddDefenseToInventory(item,player);
	client->playerinfo.pers.defence = item;

	// ********************************************************************************************
	// Start player with half offensive and defensive mana - as instructed by Brian P.
	// ********************************************************************************************

	item = P_FindItem("Off-mana");
	client->playerinfo.pers.inventory.Items[ITEM_INDEX(item)] = client->playerinfo.pers.max_offmana / 2;

	item = P_FindItem("Def-mana");
	client->playerinfo.pers.inventory.Items[ITEM_INDEX(item)] = client->playerinfo.pers.max_defmana / 2;

#ifdef G_NOAMMO
	
	// Start with all weapons if G_NOAMMO is defined.

	gi.dprintf("Starting with unlimited ammo.\n");

	item = P_FindItem("hell");
	client->playerinfo.pers.inventory.Items[ITEM_INDEX(item)] = 1;

	item = P_FindItem("array");
	client->playerinfo.pers.inventory.Items[ITEM_INDEX(item)] = 1;

	item = P_FindItem("rain");
	client->playerinfo.pers.inventory.Items[ITEM_INDEX(item)] = 1;

	item = P_FindItem("sphere");
	client->playerinfo.pers.inventory.Items[ITEM_INDEX(item)] = 1;

	item = P_FindItem("phoen");
	client->playerinfo.pers.inventory.Items[ITEM_INDEX(item)] = 1;

	item = P_FindItem("mace");
	client->playerinfo.pers.inventory.Items[ITEM_INDEX(item)] = 1;

	item = P_FindItem("fwall");
	client->playerinfo.pers.inventory.Items[ITEM_INDEX(item)] = 1;

	item = P_FindItem("meteor");
	client->playerinfo.pers.inventory.Items[ITEM_INDEX(item)] = 1;
															   
	item = P_FindItem("morph");
	client->playerinfo.pers.inventory.Items[ITEM_INDEX(item)] = 1;

	client->bowtype = BOW_TYPE_REDRAIN;
	client->armortype = ARMOR_TYPE_SILVER;

#endif // G_NOAMMO

	client->playerinfo.pers.connected = true;
}

/*
===========
PutClientInServer

Called when a player connects to a server or respawns in a deathmatch.
============
*/

void PutClientInServer (edict_t *ent)
{
	int					index;
	vec3_t				spawn_origin, spawn_angles;
	gclient_t			*client;
	int					i;
	client_persistant_t	saved;
	client_respawn_t	resp;
	int					complete_reset;
	int					plaguelevel;

	// ********************************************************************************************
	// Find a spawn point. Do it before setting health back up, so farthest ranging doesn't count
	// this client.
	// ********************************************************************************************

	SelectSpawnPoint (ent,spawn_origin, spawn_angles);

	index = ent-g_edicts-1;
	client = ent->client;

	// The player's starting plague skin is determined by the worldspawn's s.skinnum.
	if (!deathmatch->value)
	{	// We set this up now because the ClientUserinfoChanged needs to know the plaguelevel.
		client->playerinfo.plaguelevel = g_edicts[0].s.skinnum;

		if (client->playerinfo.plaguelevel >= PLAGUE_NUM_LEVELS)
			client->playerinfo.plaguelevel = PLAGUE_NUM_LEVELS-1;
		else if (client->playerinfo.plaguelevel < 0)
			client->playerinfo.plaguelevel = 0;
	}

	// ********************************************************************************************
	// Deathmatch wipes most client data every spawn.
	// ********************************************************************************************

	if (deathmatch->value)
	{
		char userinfo[MAX_INFO_STRING];

		resp = client->resp;

		memcpy (userinfo, client->playerinfo.pers.userinfo, sizeof(userinfo));
		InitClientPersistant (ent);
		ClientUserinfoChanged (ent, userinfo);
	}
	else if (coop->value)
	{
		char	userinfo[MAX_INFO_STRING];

		resp = client->resp;

		memcpy (userinfo, client->playerinfo.pers.userinfo, sizeof(userinfo));

		ClientUserinfoChanged (ent, userinfo);

		if (resp.score > client->playerinfo.pers.score)
			client->playerinfo.pers.score = resp.score;
	}
	else
	{
		char	userinfo[MAX_INFO_STRING];
		memcpy (userinfo, client->playerinfo.pers.userinfo, sizeof(userinfo));

		ClientUserinfoChanged (ent, userinfo);

		memset (&resp, 0, sizeof(resp));
	}


	// Complete or partial reset of the player's model?

	if(!deathmatch->value)
	{
		complete_reset=client->complete_reset;
	}
	else
	{
		// Deathmatch always means a complete reset of the player's model.

		complete_reset=1;		
	}

	// ********************************************************************************************
	// Initialise the player's gclient_t.
	// ********************************************************************************************

	// Clear everything but the persistant data.

	plaguelevel = client->playerinfo.plaguelevel;	// Save me too.
	saved = client->playerinfo.pers;
	memset (client, 0, sizeof(gclient_t));
	client->playerinfo.pers = saved;

	// Initialise...

	if (client->playerinfo.pers.health <= 0)
		InitClientPersistant(ent);
	
	client->resp = resp;

	// Rsestore data that is persistant accross level changes.

	FetchClientEntData (ent);

	// ********************************************************************************************
	// Initialize the player's edict_t.
	// ********************************************************************************************

	ent->groundentity = NULL;
	ent->client = &game.clients[index];
	ent->s.clientnum = index;
	ent->takedamage = DAMAGE_AIM;
	ent->materialtype = MAT_FLESH;
	ent->movetype = PHYSICSTYPE_STEP;
	ent->viewheight = 0;
	ent->just_deleted = ent->client_sent = 0;
	ent->inuse = true;
	ent->s.scale = 1.0f;
	ent->classname = "player";
	ent->mass = 200;
	ent->solid = SOLID_BBOX;
	ent->deadflag = DEAD_NO;
	ent->air_finished = level.time + HOLD_BREATH_TIME;
	ent->clipmask = MASK_PLAYERSOLID;
	ent->Leader_PersistantCFX = 0;

	// Default to making us not invunerable (may change later).

	ent->client->shrine_framenum = 0;

	// A few Multiplayer reset safeguards... i.e. if we were teleporting when we died, we aren't now.

	client->playerinfo.flags &= ~PLAYER_FLAG_TELEPORT;
	client->tele_dest[0] = client->tele_dest[1] = client->tele_dest[2] = 0;
	client->tele_count = 0;
	ent->s.color.c = 0x00000000;	// Restore model visibility.

	ent->fire_damage_time = 0;
	ent->fire_timestamp = 0;

	ent->model = "players/male/tris.fm";
	
	ent->pain = player_pain;
	ent->die = player_die;
	ent->waterlevel = 0;
	ent->watertype = 0;
	ent->flags &= ~FL_NO_KNOCKBACK;
	ent->svflags &= ~SVF_DEADMONSTER;

	VectorCopy (mins, ent->mins);
	VectorCopy (maxs, ent->maxs);
	VectorCopy (mins, ent->intentMins);
	VectorCopy (maxs, ent->intentMaxs);
	VectorClear (ent->velocity);

	// ********************************************************************************************
	// Initialize the player's gclient_t and playerstate_t.
	// ********************************************************************************************
	
	client->ps.pmove.origin[0] = spawn_origin[0]*8;
	client->ps.pmove.origin[1] = spawn_origin[1]*8;
	client->ps.pmove.origin[2] = spawn_origin[2]*8;

	client->ps.fov = atoi(Info_ValueForKey(client->playerinfo.pers.userinfo, "fov"));

	if (client->ps.fov < 1)
		client->ps.fov = FOV_DEFAULT;
	else if (client->ps.fov > 160)
		client->ps.fov = 160;

	VectorClear(client->ps.offsetangles);

	// Set the delta angles.

	for (i=0 ; i<3 ; i++)
		client->ps.pmove.delta_angles[i] = ANGLE2SHORT(spawn_angles[i] - client->resp.cmd_angles[i]);

	// Reset the camera delta angles.

	for (i=0 ; i<3 ; i++)
		client->ps.pmove.camera_delta_angles[i]=0;

	client->ps.remote_id=-1;

	// ********************************************************************************************
	// Initialize the player's entity_state_t.
	// ********************************************************************************************

	// Zero the current animation frame.

	ent->s.frame=0;

	// Modelindex is always 255 for player models.

	ent->s.modelindex=255;		
	
	// Set up the model's origin, making sure it's off the ground.

	VectorCopy (spawn_origin, ent->s.origin);
	ent->s.origin[2] += 1;
	VectorCopy (ent->s.origin, ent->s.old_origin);

	ent->s.angles[PITCH] = 0;
	ent->s.angles[YAW] = spawn_angles[YAW];
	ent->s.angles[ROLL] = 0;
	VectorCopy (ent->s.angles, client->ps.viewangles);
	VectorCopy (ent->s.angles, client->v_angle);

	if(!KillBox(ent))
	{
		// could't spawn in?
	}
	
	ent->s.effects=(EF_CAMERA_NO_CLIP|EF_SWAPFRAME|EF_JOINTED|EF_PLAYER);

	// Set up skeletal info. Note, skeleton has been created already.

	ent->s.skeletalType = SKEL_CORVUS;

	// Link us into the physics system.

	gi.linkentity (ent);

	// ********************************************************************************************
	// Initialize the player's playerinfo_t.
	// ********************************************************************************************
	
	client->playerinfo.plaguelevel = plaguelevel;

	// Set the player's current offensive and defensive ammo indexes.
	
	if (client->playerinfo.pers.weapon->ammo)
		client->playerinfo.weap_ammo_index = ITEM_INDEX(P_FindItem(client->playerinfo.pers.weapon->ammo));

	if (client->playerinfo.pers.defence)
		client->playerinfo.def_ammo_index = ITEM_INDEX(P_FindItem(client->playerinfo.pers.defence->ammo));

	VectorCopy(spawn_origin,client->playerinfo.origin);
	VectorClear(client->playerinfo.velocity);

	// Make the player have the right attributes - armor that sort of thing.

	SetupPlayerinfo_effects(ent);
	P_PlayerUpdateModelAttributes(&ent->client->playerinfo);
	WritePlayerinfo_effects(ent);

	// Make sure the skin attributes are transferred.

	ClientSetSkinType(ent, Info_ValueForKey (ent->client->playerinfo.pers.userinfo, "skin"));

	if(deathmatch->value||coop->value)
	{
		// Reset the player's fmodel nodes when spawning in deathmatch or coop.
		
		ResetPlayerBaseNodes(ent);

		// Just in case we were on fire when we died.

		gi.RemoveEffects(&ent->s, FX_FIRE_ON_ENTITY);

		// Make us invincible for a few seconds after spawn.

		ent->client->shrine_framenum = level.time + 3.3;
	}

	InitPlayerinfo(ent);

	SetupPlayerinfo(ent);

	P_PlayerInit(&ent->client->playerinfo,complete_reset);

	WritePlayerinfo(ent);

	SpawnInitialPlayerEffects(ent);

	if(coop->value)
		GiveLevelItems(ent);

	if(((int)dmflags->value)&DF_NO_OFFENSIVE_SPELL)
	{
		// For blade only DMing, ensure we start with staff in our hand.

		gitem_t *item;
		
		item=P_FindItem("staff");
		client->playerinfo.pers.newweapon=item;
		client->playerinfo.switchtoweapon=WEAPON_READY_SWORDSTAFF;
	}
}

/*
=====================
InitClientResp
=====================
*/

void InitClientResp (gclient_t *client)
{
	memset (&client->resp, 0, sizeof(client->resp));
	client->resp.enterframe = level.framenum;
	client->resp.coop_respawn = client->playerinfo.pers;
}

/*
=====================
ClientBeginDeathmatch

A client has just connected to the server in deathmatch mode, so clear everything out before starting them.
=====================
*/
void ClientBeginDeathmatch (edict_t *ent)
{
	G_InitEdict (ent);

	InitClientResp (ent->client);

	// Locate ent at a spawn point.

	PutClientInServer (ent);

	// Do the teleport sound and client effect and announce the player's entry into the
	// level.

	gi.sound(ent,CHAN_WEAPON,gi.soundindex("weapons/teleport.wav"),1,ATTN_NORM,0);
	gi.CreateEffect(&ent->s, FX_PLAYER_TELEPORT_IN, CEF_OWNERS_ORIGIN, ent->s.origin, NULL);
	gi.Obituary(PRINT_HIGH, GM_ENTERED, ent->s.number, 0);

	// Make sure all view stuff is valid.

	ClientEndServerFrame (ent);
}

/*
===========
ClientBegin

Called when a client has finished connecting, and is ready to be placed into the game. This will
happen every level load.
============
*/
void ClientBegin (edict_t *ent)
{
	int	i;

	ent->client = game.clients + (ent - g_edicts - 1);

	if (deathmatch->value)
	{
		ClientBeginDeathmatch (ent);

		return;
	}

	// If there is already a body waiting for us (a loadgame), just take it, otherwise spawn one
	// from scratch.

	if (ent->inuse == true)
	{
		// The client has cleared the client side cl.inputangles upon connecting to the server, which
		// is different from the state when the game is saved, so we need to compensate with 
		// delta_angles.
		
		for(i=0;i<3;i++)
			ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(ent->client->v_angle[i]);

		// The client has cleared the client side cl.viewangles upon connecting to the server, which
		// is different from the state when the game is saved, so we need to compensate with 
		// camara_delta_angles.
		
		for(i=0;i<3;i++)
			ent->client->ps.pmove.camera_delta_angles[i]=
				ANGLE2SHORT(ent->client->ps.viewangles[i])-ent->client->ps.pmove.delta_angles[i];

		SpawnInitialPlayerEffects(ent);

		// The player has a body waiting from a (just) loaded game, so we want to do just a partial
		// reset of the player's model.

		ent->client->complete_reset=0;
	}
	else
	{
		// A spawn point will completely reinitialize the entity except for the persistant data
		// that was initialized at ClientConnect() time.

		G_InitEdict (ent);
		ent->classname = "player";
		InitClientResp (ent->client);
		PutClientInServer (ent);
	
		// All resets should be partial, until ClientConnect() gets called again for a new game,
		// respawn() occurs (which will do the correct reset type).

		ent->client->complete_reset=0;
	}

	if(level.intermissiontime)
	{
		MoveClientToIntermission(ent,false);
	}
	else
	{
		// Send effect if in a multiplayer game.

		if (game.maxclients > 1)
		{
			// Do the teleport sound and client effect and announce the player's entry into the
			// level.

			gi.sound(ent,CHAN_WEAPON,gi.soundindex("weapons/teleport.wav"),1,ATTN_NORM,0);
			gi.CreateEffect(&ent->s, FX_PLAYER_TELEPORT_IN, CEF_OWNERS_ORIGIN, ent->s.origin, NULL);
			gi.Obituary (PRINT_HIGH, GM_ENTERED, ent->s.number, 0);
		}
	}

	// Make sure all view stuff is valid.

	ClientEndServerFrame (ent);
}

/*
===========
ClientUserInfoChanged

Called whenever the player updates a userinfo variable. The game can override any of the settings
in place (forcing skins or names, etc) before copying it off.
============
*/

void ClientUserinfoChanged (edict_t *ent, char *userinfo)
{
	char	*s, skin[MAX_QPATH], filename[MAX_QPATH];
	int		playernum;
	FILE	*f;
	qboolean found=false;

	assert(ent->client);
//	assert(ent->client->playerinfo);

	// check for malformed or illegal info strings
	if (!Info_Validate(userinfo))
	{
		strcpy (userinfo, "\\name\\badinfo\\skin\\male/Corvus");
	}

	// Set name.

	s = Info_ValueForKey (userinfo, "name");
	strncpy (ent->client->playerinfo.pers.netname, s, sizeof(ent->client->playerinfo.pers.netname)-1);

	// Set skin.

	s = Info_ValueForKey (userinfo, "skin");

	playernum = ent-g_edicts-1;

	// Please note that this function became very long with the various limitations of coop and single-play skins...
	if (deathmatch->value)
	{	// In DM any skins are okay.
		if (!strchr(s, '/'))				// Backward compatibility, if not model, then assume male
			sprintf(skin, "male/%s", s);
		else
			strcpy(skin, s);

		sprintf(filename, "players/%s.m8", skin);
		if (FS_FOpenFile (filename, &f) != -1)
		{
			FS_FCloseFile(f);		// Just checking for the existence of the file.
		}
		else
		{
			if (strstr(s, "female/"))
			{	// This was a female skin, fall back to Kiera.
				strcpy(skin, "female/Kiera");
			}
			else
			{	// Anything else, assume that it was male.
				strcpy(skin, "male/Corvus");
			}
		}

		gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s", ent->client->playerinfo.pers.netname, skin) );
	}
	else if (coop->value)
	{	// In coop only allow skins that have full plague levels...
		if (!strchr(s, '/'))				// Backward compatibility, if not model, then assume male
			sprintf(skin, "male/%s", s);
		else
			strcpy(skin, s);

		sprintf(filename, "players/%s.m8", skin);
		if (FS_FOpenFile (filename, &f) != -1)
		{
			FS_FCloseFile(f);		// Just checking for the existence of the file.
			if (allowillegalskins->value)
			{
				found=true;		// All we need is the base skin.
			}
			else
			{
				sprintf(filename, "players/%sP1.m8", skin);
				if (FS_FOpenFile (filename, &f) != -1)
				{
					FS_FCloseFile(f);		// Just checking for the existence of the file.
					sprintf(filename, "players/%sP2.m8", skin);
					if (FS_FOpenFile (filename, &f) != -1)
					{
						FS_FCloseFile(f);
						found=true;
					}
				}
			}
		}

		if (!found)
		{	// Not all three skins were found.
			if (strstr(s, "female/"))
			{	// This was a female skin, fall back to Kiera.
				strcpy(skin, "female/Kiera");
			}
			else
			{	// Anything else, assume that it was male.
				strcpy(skin, "male/Corvus");
			}
		}

		// Combine name and skin into a configstring.
		switch(ent->client->playerinfo.plaguelevel)
		{
		case 1:		// Plague level 1
			if (allowillegalskins->value)
			{	// Do the check for a valid skin in case an illegal skin has been let through.
				sprintf(filename, "players/%sP1.m8", skin);
				if (FS_FOpenFile (filename, &f) != -1)
				{	// The plague1 skin exists.
					FS_FCloseFile(f);		// Just checking for the existence of the file.
					gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%sP1", ent->client->playerinfo.pers.netname, skin) );
				}
				else
				{	// Just use the basic skin, then.
					gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s", ent->client->playerinfo.pers.netname, skin) );
				}
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%sP1", ent->client->playerinfo.pers.netname, skin) );
			}
			break;
		case 2:		// Plague level 2
			if (allowillegalskins->value)
			{	// Do the check for a valid skin in case an illegal skin has been let through.
				sprintf(filename, "players/%sP2.m8", skin);
				if (FS_FOpenFile (filename, &f) != -1)
				{	// The plague1 skin exists.
					FS_FCloseFile(f);		// Just checking for the existence of the file.
					gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%sP2", ent->client->playerinfo.pers.netname, skin) );
				}
				else
				{	// No plague 2 skin, try for a plague 1 skin.
					sprintf(filename, "players/%sP1.m8", skin);
					if (FS_FOpenFile (filename, &f) != -1)
					{	// The plague1 skin exists.
						FS_FCloseFile(f);		// Just checking for the existence of the file.
						gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%sP1", ent->client->playerinfo.pers.netname, skin) );
					}
					else
					{	// Just use the basic skin, then.
						gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s", ent->client->playerinfo.pers.netname, skin) );
					}
				}
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%sP2", ent->client->playerinfo.pers.netname, skin) );
			}
			break;
		default:
			gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s", ent->client->playerinfo.pers.netname, skin) );
		}
	}
	else
	{	// Single player.  This is CORVUS ONLY unless allowillegalskins is engaged
		if (allowillegalskins->value)
		{	// Allow any skin at all.
			if (!strchr(s, '/'))				// Backward compatibility, if not model, then assume male
				sprintf(skin, "male/%s", s);
			else
				strcpy(skin, s);

			sprintf(filename, "players/%s.m8", skin);
			if (FS_FOpenFile (filename, &f) != -1)
			{
				FS_FCloseFile(f);		// Just checking for the existence of the file.
			}
			else
			{
				strcpy(skin, "male/Corvus");
			}

			// Combine name and skin into a configstring.
			switch(ent->client->playerinfo.plaguelevel)
			{
			case 1:		// Plague level 1
				sprintf(filename, "players/%sP1.m8", skin);
				if (FS_FOpenFile (filename, &f) != -1)
				{	// The plague1 skin exists.
					FS_FCloseFile(f);		// Just checking for the existence of the file.
					gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%sP1", ent->client->playerinfo.pers.netname, skin) );
				}
				else
				{	// Just use the basic skin, then.
					gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s", ent->client->playerinfo.pers.netname, skin) );
				}
				break;
			case 2:		// Plague level 2
				sprintf(filename, "players/%sP2.m8", skin);
				if (FS_FOpenFile (filename, &f) != -1)
				{	// The plague1 skin exists.
					FS_FCloseFile(f);		// Just checking for the existence of the file.
					gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%sP2", ent->client->playerinfo.pers.netname, skin) );
				}
				else
				{	// No plague 2 skin, try for a plague 1 skin.
					sprintf(filename, "players/%sP1.m8", skin);
					if (FS_FOpenFile (filename, &f) != -1)
					{	// The plague1 skin exists.
						FS_FCloseFile(f);		// Just checking for the existence of the file.
						gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%sP1", ent->client->playerinfo.pers.netname, skin) );
					}
					else
					{	// Just use the basic skin, then.
						gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s", ent->client->playerinfo.pers.netname, skin) );
					}
				}
				break;
			default:
				gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s", ent->client->playerinfo.pers.netname, skin) );
				break;
			}
		}
		else
		{	// JUST care about Corvus
			switch(ent->client->playerinfo.plaguelevel)
			{
			case 1:		// Plague level 1
				gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\male/CorvusP1", ent->client->playerinfo.pers.netname) );
				break;
			case 2:		// Plague level 2
				gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\male/CorvusP2", ent->client->playerinfo.pers.netname) );
				break;
			default:
				gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\male/Corvus", ent->client->playerinfo.pers.netname) );
				break;
			}	
		}
	}

	// Change skins, but lookup the proper skintype.
	ClientSetSkinType(ent, s);

	// FOV.

	ent->client->ps.fov = atoi(Info_ValueForKey(userinfo, "fov"));

	if (ent->client->ps.fov < 1)
		ent->client->ps.fov = FOV_DEFAULT;
	else if (ent->client->ps.fov > 160)
		ent->client->ps.fov = 160;

	// Autoweapon changeup.

	s = Info_ValueForKey (userinfo, "autoweapon");
	if (strlen(s))
	{
		ent->client->playerinfo.pers.autoweapon = atoi(s);
	}

	// Save off the userinfo in case we want to check something later.

	strncpy (ent->client->playerinfo.pers.userinfo, userinfo, sizeof(ent->client->playerinfo.pers.userinfo)-1);
}


/*
===========
ClientConnect

Called when a player begins connecting to the server.
The game can refuse entrance to a client by returning false.
If the client is allowed, the connection process will continue
and eventually get to ClientBegin()
Changing levels will NOT cause this to be called again.
============
*/
qboolean ClientConnect (edict_t *ent, char *userinfo, qboolean loadgame)
{
	char	*value;

	// Check to see if they are on the banned IP list.

	value = Info_ValueForKey (userinfo, "ip");
	if (SV_FilterPacket(value))
		return false;

	// Check for a password.

	value = Info_ValueForKey (userinfo, "password");
	
	if (strcmp(password->string, value) != 0)
		return false;

	// Ok, they can connect.

	ent->client = game.clients + (ent - g_edicts - 1);

	// If there isn't already a body waiting for us (a loadgame), spawn one from scratch. otherwise,
	// just take what's there already.

	if (ent->inuse == false)
	{
		// Clear the respawning variables.

		InitClientResp (ent->client);

		if (!ent->client->playerinfo.pers.weapon)
		{
			InitClientPersistant (ent);

			// This is the very frist time that this player has entered the game (be it single player,
			// coop or deathmatch) so we want to do a complete reset of the player's model.

			ent->client->complete_reset=1;
		}
	}
	else
	{
		// The player has a body waiting from a (just) loaded game, so we want to do just a partial
		// reset of the player's model.

		ent->client->complete_reset=0;
	}

	ClientUserinfoChanged (ent, userinfo);

	if (game.maxclients > 1)
		gi.dprintf ("%s connected\n", ent->client->playerinfo.pers.netname);

	ent->client->playerinfo.pers.connected = true;

	return true;
}

/*
===========
ClientDisconnect

called when a player drops from the server
============
*/
void ClientDisconnect (edict_t *ent)
{
	int		playernum;

	if (!ent->client)
		return;

	// Inform other players that the disconnecting client has left the game.

	gi.Obituary (PRINT_HIGH, GM_DISCON, ent->s.number, 0);

	// Do the teleport sound.

	gi.sound(ent,CHAN_WEAPON,gi.soundindex("weapons/teleport.wav"),1,ATTN_NORM,0);

	// Send teleport effect.

	gi.CreateEffect(&ent->s, FX_PLAYER_TELEPORT_OUT, CEF_OWNERS_ORIGIN, ent->s.origin, NULL);

	// Clean up after leaving.

	if (ent->Leader_PersistantCFX)
	{
		gi.RemovePersistantEffect(ent->Leader_PersistantCFX, REMOVE_LEADER_CLIENT);
		gi.RemoveEffects(&ent->s, FX_SHOW_LEADER);
		ent->Leader_PersistantCFX =0;
	}

	// If we're on a rope...

	if (ent->client->playerinfo.flags & PLAYER_FLAG_ONROPE)
	{
		// ..unhook the rope graphic from the disconnecting player.

		ent->targetEnt->count = 0;
		ent->targetEnt->rope_grab->s.effects &= ~EF_ALTCLIENTFX;
		ent->targetEnt->enemy=NULL;
		ent->targetEnt=NULL;
	}

	gi.unlinkentity (ent);
	ent->s.modelindex = 0;
	ent->solid = SOLID_NOT;
	ent->inuse = false;
	ent->just_deleted = SERVER_DELETED;
	ent->classname = "disconnected";
	ent->client->playerinfo.pers.connected = false;
	playernum = ent-g_edicts-1;
	gi.configstring (CS_PLAYERSKINS+playernum, "");

	// Redo the leader effect cos this guy has gone, and he might have had it.

	player_leader_effect();
}

//==============================================================

edict_t	*pm_passent;

// The pmove() routine doesn't need to know about passent and contentmask.

void PM_trace (vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end,trace_t *trace)
{
	// NOTENOTE All right, pmove doesn't need to know the gory details, but I need to be able to detect a water surface, bub.
	// Hence, if the mins and max are NULL, then wask out water (cheezy I know, but blame me) ---Pat

	if (mins == NULL && maxs == NULL)
	{
		gi.trace (start, vec3_origin, vec3_origin, end, pm_passent, MASK_PLAYERSOLID | MASK_WATER,trace);
	}
	else if (pm_passent->health > 0)
	{
		gi.trace (start, mins, maxs, end, pm_passent, MASK_PLAYERSOLID,trace);
	}
	else
	{
		gi.trace (start, mins, maxs, end, pm_passent, MASK_DEADSOLID,trace);
	}
}

unsigned CheckBlock (void *b, int c)
{
	int	v,i;
	v = 0;
	for (i=0 ; i<c ; i++)
		v+= ((byte *)b)[i];
	return v;
}

void PrintPmove (pmove_t *pm)
{
	unsigned	c1, c2;

	c1 = CheckBlock (&pm->s, sizeof(pm->s));
	c2 = CheckBlock (&pm->cmd, sizeof(pm->cmd));
}

/*
// ==============
// ClientThink
// -----------
// This will be called once for each client-frame received from a client. So during a server frame,
// for a given client, ClientThink() probably be called several times,
// ==============
*/
extern edict_t	*TestEntityPosition(edict_t *self);

void ClientThink (edict_t *ent, usercmd_t *ucmd)
{
	gclient_t	*client;
	edict_t		*other;
	int			i, j;
	pmove_t		pm;
	vec3_t		LOSOrigin,ang;
	float		knockback;
	edict_t		*TargetEnt;

	level.current_entity = ent;
	client = ent->client;
		
	CheckContinuousAutomaticEffects(ent);

	// ********************************************************************************************
	// Handle an active intermission.
	// ********************************************************************************************

	if (level.intermissiontime)
	{
		client->ps.pmove.pm_type = PM_INTERMISSION;

		// Can exit intermission after five seconds
		
		if (level.time > level.intermissiontime + 5.0 && (ucmd->buttons & BUTTON_ANY) )
			level.exitintermission = true;

		return;
	}

	// ********************************************************************************************
	// Movement stuff.
	// ********************************************************************************************

	if (ent->movetype == PHYSICSTYPE_NOCLIP)
		client->ps.pmove.pm_type = PM_SPECTATOR;
	else if ((ent->s.modelindex != 255) && !(ent->flags & FL_CHICKEN))	// We're not set as a chicken
		client->ps.pmove.pm_type = PM_GIB;
	else if (ent->deadflag)
		client->ps.pmove.pm_type = PM_DEAD;
	else
		client->ps.pmove.pm_type = PM_NORMAL;

	client->ps.pmove.gravity = sv_gravity->value;

	// If we are not currently on a rope, then clear out any ropes as valid for a check.

	if (!(client->playerinfo.flags & PLAYER_FLAG_ONROPE))
	{
		ent->targetEnt = NULL;
	}

	// If we are turn-locked, then set the PMF_LOCKTURN flag that informs the client of this (the
	// client-side camera needs to know).

	if ((client->playerinfo.flags & PLAYER_FLAG_TURNLOCK) && (client->ps.pmove.pm_type == PM_NORMAL))
	{
		client->ps.pmove.pm_flags|=PMF_LOCKTURN;
	}
	else
	{
		client->playerinfo.turncmd+=SHORT2ANGLE(ucmd->angles[YAW]-client->oldcmdangles[YAW]);
		client->ps.pmove.pm_flags&=~PMF_LOCKTURN;
	}

	// Save the cmd->angles away so we may calculate the delta (on client->turncmd above) in the
	// next frame.

	client->oldcmdangles[0]=ucmd->angles[0];
	client->oldcmdangles[1]=ucmd->angles[1];
	client->oldcmdangles[2]=ucmd->angles[2];

	pm_passent = ent;

	// Set up inputs for a Pmove().

	memset (&pm, 0, sizeof(pm));

	pm.s = client->ps.pmove;

	for (i=0 ; i<3 ; i++)
	{
		pm.s.origin[i] = ent->s.origin[i]*8;
		pm.s.velocity[i] = ent->velocity[i]*8;
	}
	
	if (memcmp(&client->old_pmove, &pm.s, sizeof(pm.s)))
	{
		pm.snapinitial = true;
	}

	pm.cmd = *ucmd;
	client->pcmd = *ucmd;

	if (ent->movetype != PHYSICSTYPE_NOCLIP)
	{
		pm.cmd.forwardmove = client->playerinfo.fwdvel;
		pm.cmd.sidemove = client->playerinfo.sidevel;
		pm.cmd.upmove = client->playerinfo.upvel;
	}

	if(client->RemoteCameraLockCount>0)
	{
		pm.cmd.forwardmove =0;
		pm.cmd.sidemove = 0;
		pm.cmd.upmove = 0;
	}

	// Input the DESIRED waterheight.
	// FIXME: This should be retrieved from the animation frame eventually.

	pm.desiredWaterHeight = 15.00;
	pm.waterheight = client->playerinfo.waterheight;
	pm.waterlevel = ent->waterlevel;
	pm.viewheight = ent->viewheight;
	pm.watertype = ent->watertype;
	pm.groundentity = ent->groundentity;

	// This is a scale of 0 to 1 describing how much knockback to take into account.

	knockback = client->playerinfo.knockbacktime - level.time;
	if (knockback > 1.0)
		knockback = 1.0;
	else if (knockback < 0.0)
		knockback = 0.0;
	pm.knockbackfactor = knockback;

	// Handle lockmove cases.

	if((client->playerinfo.flags&(PLAYER_FLAG_LOCKMOVE_WAS_SET|PLAYER_FLAG_USE_ENT_POS))&&!(client->ps.pmove.pm_flags&PMF_LOCKMOVE))
	{ 
		// Lockmove was set last frame, but isn't now, so we copy the player edict's origin and
		// velocity values to the client for use in Pmove(). NOTE: Pmove() on the SERVER needs
		// pointers to specify vectors to be read and written for the origin and velocity. So
		// be careful if you screw around with this crazy code.
				
		client->playerinfo.flags &= ~PLAYER_FLAG_USE_ENT_POS;

		VectorCopy(ent->s.origin, client->playerinfo.origin);
		VectorCopy(ent->velocity, client->playerinfo.velocity);
	}

	// Check to add into movement velocity through crouch and duck if underwater.

	if (!ent->deadflag && ent->waterlevel > 2)
	{
		// NOTENOTE: If they're pressing both, nullify it.

		if (client->playerinfo.seqcmd[ACMDL_CROUCH])
		{
			client->playerinfo.velocity[2] -= SWIM_ADJUST_AMOUNT;
		}
		
		if (client->playerinfo.seqcmd[ACMDL_JUMP])
		{
			client->playerinfo.velocity[2] += SWIM_ADJUST_AMOUNT;
		}
	}
	else if (!ent->deadflag && ent->waterlevel > 1)	// On the surface trying to go down???
	{
		// NOTENOTE: If they're pressing both, nullify it.

		if (client->playerinfo.seqcmd[ACMDL_CROUCH])
		{
			pm.s.w_flags |= WF_SINK;
			client->playerinfo.velocity[2] -= SWIM_ADJUST_AMOUNT;
		}
		
		if (client->playerinfo.seqcmd[ACMDL_JUMP])
		{
			client->playerinfo.velocity[2] += SWIM_ADJUST_AMOUNT;
		}
	}		

	pm.origin = client->playerinfo.origin;
	pm.velocity = client->playerinfo.velocity;

	// If not the chicken, and not explicitly resizing the bounding box...

	if ( (!(client->playerinfo.edictflags & FL_CHICKEN)) && (!(client->playerinfo.flags & PLAYER_FLAG_RESIZED)) )
	{
		// Resize the player's bounding box.

		VectorCopy(mins, ent->intentMins);
		VectorCopy(maxs, ent->intentMaxs);
		
		ent->physicsFlags |= PF_RESIZE;

		pm.intentMins = ent->intentMins;
		pm.intentMaxs = ent->intentMaxs;
	}
	else
	{
		// Otherwise we don't want to resize.

		if ( (client->playerinfo.edictflags & FL_AVERAGE_CHICKEN) )
		{
			VectorSet(ent->mins,-8,-8,-14);
			VectorSet(ent->maxs,8,8,14);
		}
		else if ( (client->playerinfo.edictflags & FL_SUPER_CHICKEN) )
		{
			VectorSet(ent->mins,-16,-16,-36);
			VectorSet(ent->maxs,16,16,36);
		}

		pm.intentMins = ent->mins;
		pm.intentMaxs = ent->maxs;
	}

	pm.GroundSurface = client->playerinfo.GroundSurface;
	pm.GroundPlane = client->playerinfo.GroundPlane;
	pm.GroundContents = client->playerinfo.GroundContents;

	pm.self = ent;

	pm.trace = PM_trace;	// Adds default parms.
	pm.pointcontents = gi.pointcontents;

	pm.viewheight = ent->viewheight;

	VectorCopy(ent->mins, pm.mins);
	VectorCopy(ent->maxs, pm.maxs);

	// set up speed up if we have hit the run shrine recently
	if (client->playerinfo.speed_timer > level.time)
		pm.run_shrine = true;
	else
		pm.run_shrine = false;

	// set up speed up if we have been hit recently
	if (client->playerinfo.effects & EF_HIGH_MAX)
		pm.high_max = true;
	else
		pm.high_max = false;

	// Perform a Pmove().

	gi.Pmove(&pm, true);

	if(ent->waterlevel)
		client->playerinfo.flags |= FL_INWATER;
	else
		client->playerinfo.flags &= ~FL_INWATER;

	client->playerinfo.flags &= ~(PLAYER_FLAG_COLLISION | PLAYER_FLAG_SLIDE);

	if (pm.s.c_flags & PC_COLLISION)
	{
		client->playerinfo.flags |= PLAYER_FLAG_COLLISION;
	}
	
	if ((pm.s.c_flags & PC_SLIDING))
	{
		client->playerinfo.flags |= PLAYER_FLAG_SLIDE;

		if(Vec3NotZero(pm.GroundPlane.normal))
		{
			vectoangles(pm.GroundPlane.normal, ang);
			ent->ideal_yaw = ang[YAW];
		}
	}
	else if (pm.s.w_flags & WF_DIVE)
	{
		client->playerinfo.flags |= PLAYER_FLAG_DIVE;
	}

	// Save the results of the above Pmove().

	client->ps.pmove = pm.s;
	client->old_pmove = pm.s;

	client->playerinfo.GroundSurface=pm.GroundSurface;
	memcpy(&client->playerinfo.GroundPlane,&pm.GroundPlane,sizeof(cplane_t));
	client->playerinfo.GroundContents=pm.GroundContents;

	// If we're move-locked, don't update the edict's origin and velocity, otherwise copy the
	// origin and velocity from playerinfo (which have been written by Pmove()) into the edict's
	// origin and velocity.

	if((client->ps.pmove.pm_flags&PMF_LOCKMOVE))
	{
		client->playerinfo.flags |= PLAYER_FLAG_LOCKMOVE_WAS_SET;
	}
	else
	{
		client->playerinfo.flags &= ~PLAYER_FLAG_LOCKMOVE_WAS_SET;

		VectorCopy(client->playerinfo.origin, ent->s.origin);
		VectorCopy(client->playerinfo.velocity, ent->velocity);
	}

	// Update other player stuff.

	VectorCopy(pm.mins, ent->mins);
	VectorCopy(pm.maxs, ent->maxs);

	client->resp.cmd_angles[0] = SHORT2ANGLE(ucmd->angles[0]);
	client->resp.cmd_angles[1] = SHORT2ANGLE(ucmd->angles[1]);
	client->resp.cmd_angles[2] = SHORT2ANGLE(ucmd->angles[2]);

	client->playerinfo.waterlevel = pm.waterlevel;
	client->playerinfo.waterheight = pm.waterheight;
	client->playerinfo.watertype = pm.watertype;


	ent->waterlevel = pm.waterlevel;
	ent->viewheight = pm.viewheight;
	ent->watertype = pm.watertype;
	ent->groundentity = pm.groundentity;

	if (pm.groundentity)
	{
		ent->groundentity_linkcount = pm.groundentity->linkcount;
	}

	if(!ent->deadflag)
	{
		VectorCopy(pm.viewangles,client->v_angle);
		
		client->aimangles[0]=SHORT2ANGLE(ucmd->aimangles[0]);
		client->aimangles[1]=SHORT2ANGLE(ucmd->aimangles[1]);
		client->aimangles[2]=SHORT2ANGLE(ucmd->aimangles[2]);

		VectorCopy(client->aimangles,client->ps.viewangles);
	}

	gi.linkentity (ent);

	// Process touch triggers that the client could activate.

	if (ent->movetype != PHYSICSTYPE_NOCLIP)
		G_TouchTriggers (ent);

	// Touch other objects.

	for (i=0 ; i<pm.numtouch ; i++)
	{
		other = pm.touchents[i];
		for (j=0 ; j<i ; j++)
			if (pm.touchents[j] == other)
				break;
		if (j != i)
			continue;	// duplicated
		if (!other->touch)
			continue;
		other->touch (other, ent, NULL, NULL);
	}

	client->playerinfo.oldbuttons = client->playerinfo.buttons;
	client->playerinfo.buttons = ucmd->buttons;
	client->playerinfo.latched_buttons |= client->playerinfo.buttons & ~client->playerinfo.oldbuttons;
	client->playerinfo.remember_buttons |= client->playerinfo.buttons;

	// Save the light level that the player is standing on for monster sighting AI.

	ent->light_level = ucmd->lightlevel;
	
	// ********************************************************************************************
	// Handle autotargeting by looking for the nearest monster that:
	// a) Lies in a 35 degree degree horizontal, 180 degree vertical cone from the player's facing.
	// b) Lies within 0 to 500 meters of the player.
	// c) Is visible (i.e. LOS exists from player to target).
	// ********************************************************************************************
	
	// Get the origin of the LOS (from player to target) used in identifying potential targets.

	VectorCopy(ent->s.origin,LOSOrigin);
	LOSOrigin[2]+=ent->viewheight;

	// Handle autotaiming etc.
	
	TargetEnt=ent->enemy=NULL;
	client->ps.AutotargetEntityNum=0;

	if(client->playerinfo.autoaim)
	{	
		// Autoaiming is active so look for an enemy to autotarget.

		TargetEnt=FindNearestVisibleActorInFrustum(ent,
												   ent->client->aimangles,
												   0.0,500.0,
												   35*ANGLE_TO_RAD,160*ANGLE_TO_RAD,
												   SVF_MONSTER,
												   LOSOrigin,
												   NULL,NULL);
		if(TargetEnt!=NULL)
		{
			// An enemy was successfully autotargeted, so store away the pointer to our enemy.

			ent->enemy=TargetEnt;
			client->ps.AutotargetEntityNum=ent->enemy->s.number;
		}
	}

	CalculatePIV(ent);
}

/*
==============
ClientBeginServerFrame

This will be called once for each server frame, before running
any other entities in the world.
==============
*/
void ClientBeginServerFrame (edict_t *ent)
{
	gclient_t	*client;
	int			buttonMask;

	if (level.intermissiontime)
		return;

	client = ent->client;

	if (ent->deadflag & DEAD_DEAD)
	{
		// Wait for any button just going down.

		if ( level.time > client->respawn_time)
		{
			// In deathmatch, only wait for attack button.

			if (deathmatch->value)
				buttonMask = BUTTON_ATTACK;
			else
				buttonMask = -1;

			if ( ( client->playerinfo.latched_buttons & buttonMask ) ||
				(deathmatch->value && ((int)dmflags->value & DF_FORCE_RESPAWN) ) )
			{
				respawn(ent);

				client->playerinfo.latched_buttons = 0;
			}
		}
		return;
	}

	client->playerinfo.latched_buttons = 0;
}
