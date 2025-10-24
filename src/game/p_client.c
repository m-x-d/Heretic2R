//
// p_client.c
//
// Copyright 1998 Raven Software
//

#include "p_client.h" //mxd
#include "cl_strings.h"
#include "g_combat.h" //mxd
#include "g_HitLocation.h"
#include "g_items.h" //mxd
#include "g_itemstats.h"
#include "g_main.h" //mxd
#include "g_debris.h" //mxd
#include "g_Physics.h"
#include "g_playstats.h"
#include "g_Shrine.h" //mxd
#include "g_Skeletons.h"
#include "m_player.h"
#include "p_anims.h" //mxd
#include "p_funcs.h"
#include "p_hud.h" //mxd
#include "p_main.h"
#include "p_view.h" //mxd
#include "FX.h"
#include "Random.h"
#include "Utilities.h"
#include "Vector.h"
#include "qcommon.h"
#include "g_local.h"

#define SWIM_ADJUST_AMOUNT	16.0f
#define FOV_DEFAULT			75.0f

const vec3_t player_mins = { -14.0f, -14.0f, -34.0f };
const vec3_t player_maxs = {  14.0f,  14.0f,  25.0f };

// QUAKED info_player_start (1 0 0) (-16 -16 -24) (16 16 32)
// The normal starting point for a level.
void SP_info_player_start(edict_t* self) { }

// QUAKED info_player_deathmatch (1 0 1) (-16 -16 -24) (16 16 32)
// Potential spawning position for deathmatch games.
void SP_info_player_deathmatch(edict_t* self)
{
	if (!DEATHMATCH)
		G_FreeEdict(self);
}

// QUAKED info_player_coop (1 0 1) (-16 -16 -24) (16 16 32)
// Potential spawning position for coop games.
void SP_info_player_coop(edict_t* self)
{
	if (!COOP)
		G_FreeEdict(self);
}

// QUAKED info_player_intermission (1 0 1) (-16 -16 -24) (16 16 32)
// The deathmatch intermission point will be at one of these.
// Use 'angles' instead of 'angle', so you can set pitch or roll as well as yaw. 'pitch yaw roll'.
void SP_info_player_intermission(edict_t* self) { }

// Player pain is handled at the end of the frame in P_DamageFeedback().
void PlayerPain(edict_t* self, edict_t* other, float kick, int damage) //mxd. Named 'player_pain' in original logic. //TODO: can be removed?
{
}

static void BleederThink(edict_t* self)
{
	if (self->owner == NULL || self->owner->client == NULL || self->owner->s.modelindex == 0 || self->owner->health <= 0 ||
		!(self->owner->client->playerinfo.flags & PLAYER_FLAG_BLEED))
	{
		G_SetToFree(self);
		return;
	}

	//FIXME: this will be a client effect attached to ref points.
	const int damage = irand(1, 3);

	vec3_t forward;
	vec3_t right;
	vec3_t up;
	AngleVectors(self->owner->s.angles, forward, right, up);

	vec3_t bleed_pos;
	VectorMA(self->owner->s.origin, self->pos1[0], forward, bleed_pos);
	VectorMA(bleed_pos, self->pos1[1], right, bleed_pos);
	VectorMA(bleed_pos, self->pos1[2], up, bleed_pos);

	vec3_t bleed_dir;
	VectorScale(forward, self->movedir[0], bleed_dir);
	VectorMA(bleed_dir, self->movedir[1], right, bleed_dir);
	VectorMA(bleed_dir, self->movedir[2], up, bleed_dir);
	VectorScale(bleed_dir, (float)damage * 3.0f, bleed_dir);

	const int flags = ((self->owner->materialtype == MAT_INSECT) ? CEF_FLAG8 : 0); //mxd
	gi.CreateEffect(NULL, FX_BLOOD, flags, bleed_pos, "ub", bleed_dir, damage);

	if (irand(0, 3) == 0) // 25% chance to do damage.
		T_Damage(self->owner, self, self->activator, bleed_dir, bleed_pos, bleed_dir, damage, 0, DAMAGE_NO_BLOOD | DAMAGE_NO_KNOCKBACK | DAMAGE_BLEEDING | DAMAGE_AVOID_ARMOR, MOD_BLEED); // Armor doesn't stop it.

	self->nextthink = level.time + flrand(0.1f, 0.5f);
}

static void SpawnBleeder(edict_t* self, edict_t* other, vec3_t bleed_dir, vec3_t bleed_spot)
{
	self->client->playerinfo.flags |= PLAYER_FLAG_BLEED;

	edict_t* bleeder = G_Spawn();

	bleeder->owner = self;
	bleeder->activator = other;
	bleeder->classname = "bleeder";
	VectorCopy(bleed_spot, bleeder->pos1);
	VectorCopy(bleed_dir, bleeder->movedir);

	bleeder->think = BleederThink;
	bleeder->nextthink = level.time + FRAMETIME; //mxd. Use define.
}

static qboolean ShouldRepairPlayerNode(const int index) //mxd. Added to reduce code duplication.
{
	return (index != MESH__STOFF && index != MESH__BOFF && index != MESH__ARMOR && index != MESH__STAFACTV &&
			index != MESH__BLADSTF && index != MESH__HELSTF && index != MESH__BOWACTV); // These shouldn't be messed with.
}

void PlayerRepairSkin(edict_t* self) //mxd. Named 'player_repair_skin' in original logic.
{
	//FIXME: make sure it doesn't turn on a hand without the arm!
	if (self->client == NULL || self->s.modelindex == 0)
		return;

	const int num_allowed_dmg_skins = 5 - self->health / 20;

	if (num_allowed_dmg_skins <= 0)
	{
		// Restore all nodes.
		for (int i = 0; i < NUM_PLAYER_NODES; i++)
		{
			if (!ShouldRepairPlayerNode(i))
				continue;

			self->client->playerinfo.pers.altparts &= ~(1 << i);
			self->s.fmnodeinfo[i].flags &= ~FMNI_USE_SKIN;
			self->s.fmnodeinfo[i].skin = self->s.skinnum;
		}

		Player_UpdateModelAttributes(self); //mxd
		return;
	}

	int hurt_nodes[NUM_PLAYER_NODES];
	int	found_dmg_skins = 0;

	for (int i = 0; i < NUM_PLAYER_NODES; i++)
	{
		// How many nodes are hurt.
		if (ShouldRepairPlayerNode(i) && !(self->s.fmnodeinfo[i].flags & FMNI_NO_DRAW) && (self->s.fmnodeinfo[i].flags & FMNI_USE_SKIN))
		{
			hurt_nodes[found_dmg_skins] = i;
			found_dmg_skins++;
		}
	}

	if (found_dmg_skins <= num_allowed_dmg_skins) // No healing.
		return;

	int to_fix = found_dmg_skins - num_allowed_dmg_skins;
	int	checked = 0;

	while (to_fix > 0 && checked < 100)
	{
		// Heal num damaged nodes over allowed.
		const int index = hurt_nodes[irand(0, found_dmg_skins - 1)];

		//mxd. FMNI_NO_DRAW and FMNI_USE_SKIN flags are already checked when filling hurt_nodes array.
		self->s.fmnodeinfo[index].flags &= ~FMNI_USE_SKIN;
		self->s.fmnodeinfo[index].skin = self->s.skinnum;

		self->client->playerinfo.pers.altparts &= ~(1 << index);

		if (index == MESH__LARM)
			self->client->playerinfo.flags &= ~PLAYER_FLAG_NO_LARM;
		else if (index == MESH__RARM)
			self->client->playerinfo.flags &= ~PLAYER_FLAG_NO_RARM;

		to_fix--;
		checked++; // To protect against infinite loops, this IS random after all.
	}

	Player_UpdateModelAttributes(self); //mxd
}

void ResetPlayerBaseNodes(edict_t* ent) //TODO: rename to PlayerResetBaseNodes()?
{
	if (ent->client == NULL)
		return;

	ent->client->playerinfo.flags &= ~(PLAYER_FLAG_BLEED | PLAYER_FLAG_NO_LARM | PLAYER_FLAG_NO_RARM);
	ent->client->playerinfo.pers.altparts = 0;

	ent->s.fmnodeinfo[MESH_BASE2].flags &= ~(FMNI_NO_DRAW | FMNI_USE_SKIN);
	ent->s.fmnodeinfo[MESH__BACK].flags &= ~(FMNI_NO_DRAW | FMNI_USE_SKIN);
	ent->s.fmnodeinfo[MESH__RARM].flags &= ~(FMNI_NO_DRAW | FMNI_USE_SKIN);
	ent->s.fmnodeinfo[MESH__LARM].flags &= ~(FMNI_NO_DRAW | FMNI_USE_SKIN);
	ent->s.fmnodeinfo[MESH__HEAD].flags &= ~(FMNI_NO_DRAW | FMNI_USE_SKIN);
	ent->s.fmnodeinfo[MESH__RLEG].flags &= ~(FMNI_NO_DRAW | FMNI_USE_SKIN);
	ent->s.fmnodeinfo[MESH__LLEG].flags &= ~(FMNI_NO_DRAW | FMNI_USE_SKIN);

	ent->s.fmnodeinfo[MESH_BASE2].skin = ent->s.skinnum;
	ent->s.fmnodeinfo[MESH__BACK].skin = ent->s.skinnum;
	ent->s.fmnodeinfo[MESH__RARM].skin = ent->s.skinnum;
	ent->s.fmnodeinfo[MESH__LARM].skin = ent->s.skinnum;
	ent->s.fmnodeinfo[MESH__HEAD].skin = ent->s.skinnum;
	ent->s.fmnodeinfo[MESH__RLEG].skin = ent->s.skinnum;
	ent->s.fmnodeinfo[MESH__LLEG].skin = ent->s.skinnum;

	// FIXME: Turn hands back on too? But two pairs, which one? Shouldn't PlayerUpdateModelAttributes do that?
	Player_UpdateModelAttributes(ent); //mxd
}

#define BIT_BASE2		0		// MESH_BASE2		0 - Front.
#define BIT_BACK		1		// MESH__BACK		1 - Back.
#define BIT_STOFF		2		// MESH__STOFF		2 - Staff on leg.
#define BIT_BOFF		4		// MESH__BOFF		3 - Bow on shoulder.
#define BIT_ARMOR		8		// MESH__ARMOR		4 - Armor.
#define BIT_RARM		16		// MESH__RARM		5 - Right shoulder to wrist.
#define BIT_RHANDHI		32		// MESH__RHANDHI	6 - Right hand flat.
#define BIT_STAFACTV	64		// MESH__STAFACTV	7 - Right hand fist & staff stub.
#define BIT_BLADSTF		128		// MESH__BLADSTF	8 - Staff (active).
#define BIT_HELSTF		256		// MESH__HELSTF		9 - Hellstaff.
#define BIT_LARM		512		// MESH__LARM		10 - Left shoulder to wrist.
#define BIT_LHANDHI		1024	// MESH__LHANDHI	11 - Left hand flat.
#define BIT_BOWACTV		2048	// MESH__BOWACTV	12 - Left hand fist & bow.
#define BIT_RLEG		4096	// MESH__RLEG		13 - Right leg.
#define BIT_LLEG		8192	// MESH__LLEG		14 - Left leg.
#define BIT_HEAD		16384	// MESH__HEAD		15 - Head.

static qboolean PlayerCanThrowNode(edict_t* self, const int body_part, int* throw_nodes) //mxd. Named 'canthrownode_player' in original version.
{
	static int bit_to_meshnode[16] = //mxd. Made local static.
	{
		BIT_BASE2,		// 0 - Front.
		BIT_BACK,		// 1 - Back.
		BIT_STOFF,		// 2 - Staff on leg.
		BIT_BOFF,		// 3 - Bow on shoulder.
		BIT_ARMOR,		// 4 - Armor.
		BIT_RARM,		// 5 - Right shoulder to wrist.
		BIT_RHANDHI,	// 6 - Right hand flat.
		BIT_STAFACTV,	// 7 - Right hand fist & staff stub.
		BIT_BLADSTF,	// 8 - Staff (active).
		BIT_HELSTF,		// 9 - Hellstaff.
		BIT_LARM,		// 10 - Left shoulder to wrist.
		BIT_LHANDHI,	// 11 - Left hand flat.
		BIT_BOWACTV,	// 12 - Left hand fist & bow.
		BIT_RLEG,		// 13 - Right leg.
		BIT_LLEG,		// 14 - Left leg.
		BIT_HEAD,		// 15 - Head.
	};

	// See if it's on, if so, add it to throw_nodes. Turn it off on thrower.
	if (!(self->s.fmnodeinfo[body_part].flags & FMNI_NO_DRAW))
	{
		*throw_nodes |= bit_to_meshnode[body_part];
		self->s.fmnodeinfo[body_part].flags |= FMNI_NO_DRAW;

		return true;
	}

	return false;
}

static void DropWeapons(edict_t* self, const int damage, const int which_weapons) //mxd. Named 'player_dropweapon' in original version.
{
	//FIXME: OR in the BIT_... to playerinfo->altparts!
	// Current code doesn't really support dropping weapons!!!
	if (DEATHMATCH)
	{
		if (!(DMFLAGS & DF_DISMEMBER) && self->health > 0)
			return;
	}
	else if (self->health > 0)
	{
		return;
	}

	//FIXME: use refpoints for this?
	vec3_t forward;
	vec3_t right;
	vec3_t up;
	AngleVectors(self->s.angles, forward, right, up);

	vec3_t hand_spot = { 0 };
	VectorMA(hand_spot, 5.0f, forward, hand_spot);
	VectorMA(hand_spot, 8.0f, right, hand_spot);
	VectorMA(hand_spot, -6.0f, up, hand_spot);

	//TODO: can these 3 cases happen on the same call?
	if ((which_weapons & BIT_BLADSTF) && !(self->s.fmnodeinfo[MESH__BLADSTF].flags & FMNI_NO_DRAW))
	{
		ThrowWeapon(self, &hand_spot, BIT_BLADSTF, (float)damage, 0);

		self->s.fmnodeinfo[MESH__BLADSTF].flags |= FMNI_NO_DRAW;
		self->s.fmnodeinfo[MESH__STAFACTV].flags |= FMNI_NO_DRAW;
		self->s.fmnodeinfo[MESH__RHANDHI].flags &= ~FMNI_NO_DRAW;
	}

	if ((which_weapons & BIT_HELSTF) && !(self->s.fmnodeinfo[MESH__HELSTF].flags & FMNI_NO_DRAW))
	{
		ThrowWeapon(self, &hand_spot, BIT_HELSTF, (float)damage, 0);

		self->s.fmnodeinfo[MESH__HELSTF].flags |= FMNI_NO_DRAW;
		self->s.fmnodeinfo[MESH__STAFACTV].flags |= FMNI_NO_DRAW;
		self->s.fmnodeinfo[MESH__RHANDHI].flags &= ~FMNI_NO_DRAW;
	}

	if (which_weapons & BIT_BOWACTV && !(self->s.fmnodeinfo[MESH__BOWACTV].flags & FMNI_NO_DRAW))
	{
		ThrowWeapon(self, &hand_spot, BIT_BOFF, (float)damage, 0);

		self->s.fmnodeinfo[MESH__BOFF].flags |= FMNI_NO_DRAW;
		self->s.fmnodeinfo[MESH__BOWACTV].flags |= FMNI_NO_DRAW;
		self->s.fmnodeinfo[MESH__LHANDHI].flags &= ~FMNI_NO_DRAW;
	}
}

#pragma region ========================== PlayerDismember() logic ==========================

static void PlayerThrowHead(edict_t* self, edict_t* other, float damage, const qboolean dismember_ok) //mxd
{
	if (self->s.fmnodeinfo[MESH__HEAD].flags & FMNI_NO_DRAW)
		return;

	if (self->s.fmnodeinfo[MESH__HEAD].flags & FMNI_USE_SKIN)
		damage *= 1.5f; // Greater chance to cut off if previously damaged.

	//NOTE: I'm cutting down the decap chance just a little bit... Happened too often.
	if (dismember_ok && flrand(0.0f, (float)self->health) < damage * 0.4f)
	{
		int thrown_nodes = 0;
		PlayerCanThrowNode(self, MESH__HEAD, &thrown_nodes);

		vec3_t gore_spot = { 0.0f, 0.0f, 18.0f };
		ThrowBodyPart(self, &gore_spot, thrown_nodes, damage, 0);

		VectorAdd(self->s.origin, gore_spot, gore_spot);
		SprayDebris(self, gore_spot, 8, damage);

		if (self->health > 0)
		{
			self->health = 1;
			T_Damage(self, other, other, vec3_origin, vec3_origin, vec3_origin, 10, 20, DAMAGE_AVOID_ARMOR, MOD_STAFF);
		}
	}
	else
	{
		self->client->playerinfo.pers.altparts |= (1 << MESH__HEAD);
		self->s.fmnodeinfo[MESH__HEAD].flags |= FMNI_USE_SKIN;
		self->s.fmnodeinfo[MESH__HEAD].skin = self->s.skinnum + 1;
	}
}

static void PlayerThrowTorso(edict_t* self, edict_t* other, float damage, const qboolean dismember_ok, const int node_index) //mxd
{
	if (self->s.fmnodeinfo[node_index].flags & FMNI_NO_DRAW)
		return;

	if (self->s.fmnodeinfo[node_index].flags & FMNI_USE_SKIN)
		damage *= 1.5f; // Greater chance to cut off if previously damaged.

	if (dismember_ok && flrand(0.0f, (float)self->health) < damage * 0.3f)
	{
		self->client->playerinfo.flags |= (PLAYER_FLAG_NO_LARM | PLAYER_FLAG_NO_RARM);

		int thrown_nodes = 0;
		PlayerCanThrowNode(self, MESH_BASE2, &thrown_nodes);
		PlayerCanThrowNode(self, MESH__BACK, &thrown_nodes);
		PlayerCanThrowNode(self, MESH__LARM, &thrown_nodes);
		PlayerCanThrowNode(self, MESH__RARM, &thrown_nodes);
		PlayerCanThrowNode(self, MESH__HEAD, &thrown_nodes);
		PlayerCanThrowNode(self, MESH__LHANDHI, &thrown_nodes);
		PlayerCanThrowNode(self, MESH__RHANDHI, &thrown_nodes);

		vec3_t gore_spot = { 0.0f, 0.0f, 12.0f };
		ThrowBodyPart(self, &gore_spot, thrown_nodes, damage, 1);

		VectorAdd(self->s.origin, gore_spot, gore_spot);
		SprayDebris(self, gore_spot, 12, damage);

		if (self->health > 0)
		{
			self->health = 1;
			T_Damage(self, other, other, vec3_origin, vec3_origin, vec3_origin, 10, 20, DAMAGE_AVOID_ARMOR, MOD_STAFF);
		}
	}
	else
	{
		self->client->playerinfo.pers.altparts |= (1 << node_index);
		self->s.fmnodeinfo[node_index].flags |= FMNI_USE_SKIN;
		self->s.fmnodeinfo[node_index].skin = self->s.skinnum + 1;
	}
}

static int PlayerThrowLeftArm(edict_t* self, edict_t* other, float damage, const qboolean dismember_ok) //mxd
{
	int thrown_nodes = 0;

	if (self->s.fmnodeinfo[MESH__LARM].flags & FMNI_NO_DRAW)
		return thrown_nodes;

	if (self->s.fmnodeinfo[MESH__LARM].flags & FMNI_USE_SKIN)
		damage *= 1.5f; // Greater chance to cut off if previously damaged.

	if (dismember_ok && flrand(0.0f, (float)self->health) < damage)
	{
		if (PlayerCanThrowNode(self, MESH__LARM, &thrown_nodes))
		{
			self->client->playerinfo.flags |= PLAYER_FLAG_NO_LARM;
			DropWeapons(self, (int)damage, BIT_BOWACTV);

			PlayerCanThrowNode(self, MESH__LHANDHI, &thrown_nodes);

			vec3_t right;
			AngleVectors(self->s.angles, NULL, right, NULL);

			vec3_t gore_spot = { 0.0f, 0.0f, self->maxs[2] * 0.3f };
			VectorMA(gore_spot, -10.0f, right, gore_spot);
			ThrowBodyPart(self, &gore_spot, thrown_nodes, damage, 0);

			vec3_t blood_dir = { 0.0f, -1.0f,  0.0f };
			vec3_t blood_spot = { 0.0f, -12.0f, 10.0f };
			SpawnBleeder(self, other, blood_dir, blood_spot);
		}
	}
	else
	{
		self->client->playerinfo.pers.altparts |= (1 << MESH__LARM);
		self->s.fmnodeinfo[MESH__LARM].flags |= FMNI_USE_SKIN;
		self->s.fmnodeinfo[MESH__LARM].skin = self->s.skinnum + 1;
	}

	return thrown_nodes;
}

static int PlayerThrowRightArm(edict_t* self, edict_t* other, float damage, const qboolean dismember_ok, const qboolean in_polevault) //mxd
{
	int thrown_nodes = 0;

	// Knock weapon out of hand?
	if (self->s.fmnodeinfo[MESH__RARM].flags & FMNI_NO_DRAW)
		return thrown_nodes;

	if (self->s.fmnodeinfo[MESH__RARM].flags & FMNI_USE_SKIN)
		damage *= 1.5f; // Greater chance to cut off if previously damaged.

	if (dismember_ok && flrand(0.0f, (float)self->health) < damage)
	{
		if (PlayerCanThrowNode(self, MESH__RARM, &thrown_nodes))
		{
			self->client->playerinfo.flags |= PLAYER_FLAG_NO_RARM;
			DropWeapons(self, (int)damage, BIT_HELSTF | BIT_BLADSTF);

			PlayerCanThrowNode(self, MESH__RHANDHI, &thrown_nodes);

			vec3_t right;
			AngleVectors(self->s.angles, NULL, right, NULL);

			vec3_t gore_spot = { 0.0f, 0.0f, self->maxs[2] * 0.3f };
			VectorMA(gore_spot, 10.0f, right, gore_spot);
			ThrowBodyPart(self, &gore_spot, thrown_nodes, damage, 0);

			vec3_t blood_dir = { 0.0f, 1.0f,  0.0f };
			vec3_t blood_spot = { 0.0f, 12.0f, 10.0f };
			SpawnBleeder(self, other, blood_dir, blood_spot);

			if (in_polevault) // Oops! No staff! Fall down!
				P_KnockDownPlayer(&self->client->playerinfo);
		}
	}
	else
	{
		self->client->playerinfo.pers.altparts |= (1 << MESH__RARM);
		self->s.fmnodeinfo[MESH__RARM].flags |= FMNI_USE_SKIN;
		self->s.fmnodeinfo[MESH__RARM].skin = self->s.skinnum + 1;
	}

	return thrown_nodes;
}

static int PlayerThrowLeg(edict_t* self, const float damage, const int node_index) //mxd
{
	int thrown_nodes = 0;

	if (self->health > 0)
	{
		// Still alive.
		if (self->s.fmnodeinfo[node_index].flags & FMNI_USE_SKIN)
			return thrown_nodes;

		self->client->playerinfo.pers.altparts |= (1 << node_index);
		self->s.fmnodeinfo[node_index].flags |= FMNI_USE_SKIN;
		self->s.fmnodeinfo[node_index].skin = self->s.skinnum + 1;
	}
	else
	{
		if (self->s.fmnodeinfo[node_index].flags & FMNI_NO_DRAW)
			return thrown_nodes;

		if (PlayerCanThrowNode(self, node_index, &thrown_nodes))
		{
			vec3_t right;
			AngleVectors(self->s.angles, NULL, right, NULL);

			vec3_t gore_spot = { 0.0f, 0.0f, self->maxs[2] * 0.3f };
			VectorMA(gore_spot, -10.0f, right, gore_spot); //TODO: shouldn't this offset to the left instead for MESH__LLEG?
			ThrowBodyPart(self, &gore_spot, thrown_nodes, damage, 0);
		}
	}

	return thrown_nodes;
}

void PlayerDismember(edict_t* self, edict_t* other, const int damage, HitLocation_t hl) //mxd. Named 'player_dismember' in original logic.
{
	//FIXME: Make sure you can still dismember and gib player while dying.
	qboolean dismember_ok = false;
	qboolean in_polevault = false;

	if (hl & hl_MeleeHit)
	{
		dismember_ok = true;
		hl &= ~hl_MeleeHit;
	}

	// Dismember living players in deathmatch only if that dmflag set!
	if (DEATHMATCH)
	{
		if (!(DMFLAGS & DF_DISMEMBER) && self->health > 0)
			dismember_ok = false;

		if (dismember_ok)
		{
			in_polevault = ((self->client->playerinfo.frame > FRAME_vault3 && self->client->playerinfo.frame < FRAME_vault15)); //mxd

			if (in_polevault)
			{
				// Horizontal, in air, need to alter hit_location.
				switch (hl)
				{
					case hl_Head:
						hl = hl_TorsoFront;
						break;

					case hl_TorsoFront:
						hl = hl_TorsoFront; //TODO: shouldn't this be hl_TorsoBack?..
						break;

					case hl_TorsoBack:
						hl = hl_Head;
						break;
				}
			}

			if (self->health > 0 && irand(0, 2) == 0 && hl != hl_Head && hl != hl_ArmUpperLeft && hl != hl_ArmUpperRight)
				hl = (irand(0, 1) ? hl_ArmUpperLeft : hl_ArmUpperRight); // Deathmatch hack.
		}
	}
	else if (self->health > 0)
	{
		dismember_ok = false;
	}

	if (!dismember_ok && ((damage < 4 && self->health > 10) || (damage < 10 && self->health > 85)))
		return;

	if (hl <= hl_NoneSpecific || hl >= hl_Max) //mxd. 'hit_location > hl_Max' in original version.
		return;

	//FIXME: special manipulations of hit locations depending on anim.
	int thrown_nodes = 0;

	switch (hl)
	{
		case hl_Head:
			PlayerThrowHead(self, other, (float)damage, dismember_ok); //mxd
			break;

		case hl_TorsoFront: // Split in half?
			PlayerThrowTorso(self, other, (float)damage, dismember_ok, MESH_BASE2); //mxd
			break;

		case hl_TorsoBack: // Split in half?
			PlayerThrowTorso(self, other, (float)damage, dismember_ok, MESH__BACK); //mxd
			break;

		case hl_ArmUpperLeft: // Left arm.
		case hl_ArmLowerLeft:
			thrown_nodes = PlayerThrowLeftArm(self, other, (float)damage, dismember_ok); //mxd
			break;

		case hl_ArmUpperRight: // Right arm.
		case hl_ArmLowerRight:
			thrown_nodes = PlayerThrowRightArm(self, other, (float)damage, dismember_ok, in_polevault); //mxd
			break;

		case hl_LegUpperLeft: // Left leg.
		case hl_LegLowerLeft:
			thrown_nodes = PlayerThrowLeg(self, (float)damage, MESH__LLEG); //mxd
			break;

		case hl_LegUpperRight: // Right leg.
		case hl_LegLowerRight:
			thrown_nodes = PlayerThrowLeg(self, (float)damage, MESH__RLEG); //mxd
			break;

		default:
			break;
	}

	if (thrown_nodes > 0)
	{
		self->pain_debounce_time = 0;

		if (!P_BranchCheckDismemberAction(&self->client->playerinfo, self->client->playerinfo.pers.weapon->tag))
		{
			P_PlayerInterruptAction(&self->client->playerinfo);
			P_PlayerAnimSetUpperSeq(&self->client->playerinfo, ASEQ_NONE);
			P_PlayerAnimSetLowerSeq(&self->client->playerinfo, irand(ASEQ_PAIN_A, ASEQ_PAIN_B));
		}
	}

	Player_UpdateModelAttributes(self); //mxd
}

#pragma endregion

void PlayerDecapitate(edict_t* self, edict_t* other) //mxd. Named 'player_decap' in original logic.
{
	//FIXME: special manipulations of hit locations depending on anim.
	if (self->s.fmnodeinfo[MESH__HEAD].flags & FMNI_NO_DRAW)
		return;

	DropWeapons(self, 100, (BIT_BOWACTV | BIT_BLADSTF | BIT_HELSTF));

	int throw_nodes = 0;
	PlayerCanThrowNode(self, MESH__HEAD, &throw_nodes);

	vec3_t gore_spot = { 0.0f, 0.0f, 18.0f };
	ThrowBodyPart(self, &gore_spot, throw_nodes, 0, 0);

	VectorAdd(self->s.origin, gore_spot, gore_spot);
	SprayDebris(self, gore_spot, 8, 100);

	if (self->health > 0)
	{
		self->health = 0;
		self->client->meansofdeath = MOD_DECAP;
		PlayerDie(self, other, other, 100, gore_spot);
	}

	Player_UpdateModelAttributes(self); //mxd
}

static void PlayerLeaderEffect(void) //mxd. Named 'player_leader_effect' in original logic.
{
	// If we don't want leader effects, bump outta here.
	if (!(DMFLAGS & DF_SHOW_LEADER))
		return;

	int max_score = 1;
	int scored_players = 0;
	int total_players = 0;

	// Now we decide if anyone is a leader here, and if they are, we put the glow around them.
	// First, search through all clients and see what the leading score is.
	for (int i = 0; i < game.maxclients; i++)
	{
		const edict_t* ent = &g_edicts[i];

		// Are we a player that's playing?
		if (!ent->inuse || ent->client == NULL)
			continue;

		total_players++;

		if (ent->client->resp.score == max_score)
		{
			scored_players++;
		}
		else if (ent->client->resp.score > max_score)
		{
			scored_players = 0;
			max_score = ent->client->resp.score;
		}
	}

	// If more than 3 people have it, no one is the leader.
	if (scored_players > 3 || total_players == scored_players)
		max_score = 999999;

	// Now loop through and turn off the persistent effect of anyone that has below the leader score.
	// And turn it on for anyone that does have it, if its not already turned on.
	for (int i = 0; i < game.maxclients; i++)
	{
		edict_t* ent = &g_edicts[i];

		// Are we a player that's playing?
		if (ent->client == NULL)
			continue;

		// Are we a leader?
		if (ent->inuse && ent->client->resp.score == max_score)
		{
			if (ent->Leader_PersistantCFX == 0)
				ent->Leader_PersistantCFX = gi.CreatePersistantEffect(&ent->s, FX_SHOW_LEADER, CEF_BROADCAST | CEF_OWNERS_ORIGIN, NULL, "");
		}
		else if (ent->Leader_PersistantCFX > 0) // If not, then if we have the effect, remove it.
		{
			gi.RemovePersistantEffect(ent->Leader_PersistantCFX, REMOVE_LEADER);
			gi.RemoveEffects(&ent->s, FX_SHOW_LEADER);

			ent->Leader_PersistantCFX = 0;
		}
	}
}

static void ClientObituary(edict_t* self, edict_t* attacker)
{
#pragma region ========================== Message arrays ==========================

	static const short killed_self[MOD_MAX] =
	{
		0,	// MOD_UNKNOWN

		0,	// MOD_STAFF
		0,	// MOD_FIREBALL
		0,	// MOD_MMISSILE
		0,	// MOD_SPHERE
		0,	// MOD_SPHERE_SPL
		0,	// MOD_IRONDOOM
		0,	// MOD_FIREWALL
		0,	// MOD_STORM
		0,	// MOD_PHOENIX
		0,	// MOD_PHOENIX_SPL
		0,	// MOD_HELLSTAFF

		0,	// MOD_P_STAFF
		0,	// MOD_P_FIREBALL
		0,	// MOD_P_MMISSILE
		0,	// MOD_P_SPHERE
		0,	// MOD_P_SPHERE_SPL
		0,	// MOD_P_IRONDOOM
		0,	// MOD_P_FIREWALL
		0,	// MOD_P_STORM
		0,	// MOD_P_PHOENIX
		0,	// MOD_P_PHOENIX_SPL
		0,	// MOD_P_HELLSTAFF

		0,					// MOD_KICKED
		0,					// MOD_METEORS
		0,					// MOD_ROR
		0,					// MOD_SHIELD
		0,					// MOD_CHICKEN
		0,					// MOD_TELEFRAG
		GM_OBIT_WATER,		// MOD_WATER
		GM_OBIT_SLIME,		// MOD_SLIME
		GM_OBIT_LAVA,		// MOD_LAVA
		GM_OBIT_CRUSH,		// MOD_CRUSH
		GM_OBIT_FALLING,	// MOD_FALLING
		GM_OBIT_SUICIDE,	// MOD_SUICIDE
		GM_OBIT_BARREL,		// MOD_BARREL
		GM_OBIT_EXIT,		// MOD_EXIT
		GM_OBIT_BURNT,		// MOD_BURNT
		GM_OBIT_BLEED,		// MOD_BLEED
		0,					// MOD_SPEAR
		0,					// MOD_DIED
		GM_OBIT_EXPL,		// MOD_KILLED_SLF
		0,					// MOD_DECAP
		GM_OBIT_TORN_SELF	// MOD_TORN
	};

	static const short killed_by[MOD_MAX] =
	{
		0,						// MOD_UNKNOWN

		GM_OBIT_STAFF,			// MOD_STAFF
		GM_OBIT_FIREBALL,		// MOD_FIREBALL
		GM_OBIT_MMISSILE,		// MOD_MMISSILE
		GM_OBIT_SPHERE,			// MOD_SPHERE
		GM_OBIT_SPHERE_SPL,		// MOD_SPHERE_SPL
		GM_OBIT_IRONDOOM,		// MOD_IRONDOOM
		GM_OBIT_FIREWALL,		// MOD_FIREWALL
		GM_OBIT_STORM,			// MOD_STORM
		GM_OBIT_PHOENIX,		// MOD_PHOENIX
		GM_OBIT_PHOENIX_SPL,	// MOD_PHOENIX_SPL
		GM_OBIT_HELLSTAFF,		// MOD_HELLSTAFF

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
		0,						// MOD_WATER
		0,						// MOD_SLIME
		0,						// MOD_LAVA
		0,						// MOD_CRUSH
		0,						// MOD_FALLING
		0,						// MOD_SUICIDE
		0,						// MOD_BARREL
		0,						// MOD_EXIT
		GM_OBIT_BURNT,			// MOD_BURNT
		GM_OBIT_BLEED,			// MOD_BLEED
		0,						// MOD_SPEAR
		0,						// MOD_DIED
		0,						// MOD_KILLED_SLF
		0,						// MOD_DECAP
		GM_OBIT_TORN			// MOD_TORN
	};

#pragma endregion

	assert(self->client != NULL);

	if (!DEATHMATCH && !COOP)
		return; // No obituaries in single player.

	const qboolean friendly_fire = (self->client->meansofdeath & MOD_FRIENDLY_FIRE);
	self->client->meansofdeath &= ~MOD_FRIENDLY_FIRE;

	//mxd. Skip extra DEATHMATCH || COOP check.
	self->enemy = attacker;

	if (attacker != NULL && attacker->client != NULL && attacker != self)
	{
		const short message = killed_by[self->client->meansofdeath];

		if (message > 0)
		{
			gi.Obituary(PRINT_MEDIUM, (short)(message + irand(0, 2)), self->s.number, attacker->s.number);

			if (DEATHMATCH)
			{
				attacker->client->resp.score += (friendly_fire ? -1 : 1);
				PlayerLeaderEffect();
			}

			return;
		}
	}

	// Wasn't awarded a frag, check for suicide messages.
	const short message = killed_self[self->client->meansofdeath];

	if (message > 0)
	{
		gi.Obituary(PRINT_MEDIUM, (short)(message + irand(0, 2)), self->s.number, 0);

		if (DEATHMATCH)
		{
			self->client->resp.score--;
			PlayerLeaderEffect();
		}

		self->enemy = NULL;
	}
	else
	{
		gi.Obituary(PRINT_MEDIUM, (short)(GM_OBIT_DIED + irand(0, 2)), self->s.number, 0);

		if (DEATHMATCH)
		{
			self->client->resp.score--;
			PlayerLeaderEffect();
		}
	}
}

static void PlayerMakeGib(edict_t* self, edict_t* attacker)
{
	if (self->client != NULL)
	{
		//FIXME: Have a generic GibParts effect that throws flesh and several body parts - much cheaper.
		const int num_limbs = irand(1, 3);

		for (int i = 0; i < num_limbs; i++)
			PlayerDismember(self, attacker, irand(80, 160), irand(hl_Head, hl_LegLowerRight) | hl_MeleeHit); //mxd. 'damage' arg set using flrand() in original version.
	}

	const byte mag = (byte)(Clamp(VectorLength(self->mins), 1.0f, 255.0f));
	gi.CreateEffect(NULL, FX_FLESH_DEBRIS, 0, self->s.origin, "bdb", irand(10, 30), self->mins, mag);
	self->takedamage = DAMAGE_NO;
}

void PlayerDie(edict_t* self, edict_t* inflictor, edict_t* attacker, int damage, const vec3_t point) //mxd. Named 'player_die' in original logic.
{
	//FIXME: Make sure you can still dismember and gib player while dying.
	assert(self->client != NULL);

	VectorClear(self->avelocity);

	self->health = max(-99, self->health); // Looks better on stat bar display.
	self->takedamage = DAMAGE_NO;
	self->movetype = PHYSICSTYPE_STEP;
	self->solid = SOLID_NOT;
	self->maxs[2] = -8.0f;

	self->s.angles[PITCH] = 0.0f;
	self->s.angles[ROLL] = 0.0f;
	self->s.sound = 0;

	// Tell the leader client effect that this client is dead to stop drawing the effect.
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

		self->client->playerinfo.flags |= (PLAYER_FLAG_FALLBREAK | PLAYER_FLAG_FALLING);
	}

	// Get rid of the player's persistent effect.
	if (self->PersistantCFX > 0)
	{
		gi.RemovePersistantEffect(self->PersistantCFX, REMOVE_DIE);
		self->PersistantCFX = 0;
	}

	if (self->Leader_PersistantCFX > 0)
	{
		gi.RemovePersistantEffect(self->Leader_PersistantCFX, REMOVE_LEADER_DIE);
		self->Leader_PersistantCFX = 0;
	}

	// Remove any persistent meteor effects.
	for (int i = 0; i < 4; i++)
	{
		if (self->client->Meteors[i] == NULL)
			continue;

		if (self->client->Meteors[i]->PersistantCFX > 0)
		{
			gi.RemovePersistantEffect(self->client->Meteors[i]->PersistantCFX, REMOVE_METEOR);
			gi.RemoveEffects(&self->s, FX_SPELL_METEORBARRIER + i);

			self->client->Meteors[i]->PersistantCFX = 0;
		}

		G_SetToFree(self->client->Meteors[i]);
		self->client->Meteors[i] = NULL;
	}

	// We now own no meteors at all.
	self->client->playerinfo.meteor_count = 0;

	// Create a persistent FX_REMOVE_EFFECTS effect - this is a special hack.
	// If we create a regular FX_REMOVE_EFFECTS effect, it will overwrite the next FX_PLAYER_PERSISTANT effect sent out.
	gi.CreatePersistantEffect(&self->s, FX_REMOVE_EFFECTS, CEF_BROADCAST | CEF_OWNERS_ORIGIN, NULL, "s", 0);

	// Get rid of all the stuff set up in PlayerFirstSeenInit...
	gi.RemoveEffects(&self->s, FX_SHADOW);
	gi.RemoveEffects(&self->s, FX_WATER_PARTICLES);
	gi.RemoveEffects(&self->s, FX_CROSSHAIR);

	// Remove any shrine effects we have going.
	PlayerKillShrineFX(self);

	// Remove any sound effects we may be generating.
	gi.sound(self, CHAN_WEAPON, gi.soundindex("misc/null.wav"), 1.0f, ATTN_NORM, 0.0f);

	if (self->health < -40 && !(self->flags & FL_CHICKEN))
	{
		// Gib player.
		gi.sound(self, CHAN_BODY, gi.soundindex("*gib.wav"), 1.0f, ATTN_NORM, 0.0f);

		PlayerMakeGib(self, attacker);

		self->s.modelindex = 0;

		// Won't get sent to client if modelindex is 0 unless SVF_ALWAYS_SEND flag is set.
		self->svflags |= SVF_ALWAYS_SEND;
		self->s.effects |= (EF_NODRAW_ALWAYS_SEND | EF_ALWAYS_ADD_EFFECTS);
		self->dead_state = DEAD_DEAD;

		self->client->playerinfo.deadflag = DEAD_DEAD;
	}
	else
	{
		// Make player die a normal death.
		self->health = -1;

		if (self->dead_state == DEAD_NO)
		{
			self->client->respawn_time = level.time + 1.0f;
			self->client->ps.pmove.pm_type = PM_DEAD;

			// If player died in a deathmatch or coop, show scores.
			Cmd_Score_f(self);

			// Check if a chicken?
			if (self->flags & FL_CHICKEN)
			{
				// We're a chicken, so die a chicken's death.
				PlayerChickenDeath(self);
				PlayerMakeGib(self, attacker);
				self->s.modelindex = 0;

				// Won't get sent to client if modelindex is 0 unless SVF_ALWAYS_SEND flag is set.
				self->svflags |= SVF_ALWAYS_SEND;
				self->s.effects |= EF_NODRAW_ALWAYS_SEND | EF_ALWAYS_ADD_EFFECTS;
			}
			else if ((self->client->playerinfo.flags & PLAYER_FLAG_SURFSWIM) || self->waterlevel > 1)
			{
				P_PlayerAnimSetLowerSeq(&self->client->playerinfo, ASEQ_DROWN);
				gi.sound(self, CHAN_BODY, gi.soundindex("*drowndeath.wav"), 1.0f, ATTN_NORM, 0.0f);
			}
			else if (Q_stricmp(inflictor->classname, "plague_mist") == 0) //mxd. stricmp -> Q_stricmp
			{
				P_PlayerAnimSetLowerSeq(&self->client->playerinfo, ASEQ_DEATH_CHOKE);
				gi.sound(self, CHAN_BODY, gi.soundindex("*chokedeath.wav"), 1.0f, ATTN_NORM, 0.0f);
			}
			else if (self->fire_damage_time == -1.0f)
			{
				P_PlayerAnimSetLowerSeq(&self->client->playerinfo, ASEQ_DEATH_B);

				const char* snd_name = ((blood_level != NULL && (int)(blood_level->value) <= VIOLENCE_BLOOD) ? "*death1.wav" : "*firedeath.wav"); //mxd //TODO: why blood_level NULL check? Inited in InitGame(), accessed without NULL check in G_RunFrame()...
				gi.sound(self, CHAN_BODY, gi.soundindex(snd_name), 1.0f, ATTN_NORM, 0.0f);
			}
			else // "Normal" deaths.
			{
				// Check if the player had a velocity forward or backward during death.
				vec3_t fwd;
				AngleVectors(self->s.angles, fwd, NULL, NULL);

				const float speed = DotProduct(fwd, self->velocity) + flrand(-16.0f, 16.0f); // Add a spot of randomness to it.

				int aseq_index = ASEQ_DEATH_A; // Jes' flop to the ground.
				if (speed > 16.0f)
					aseq_index = ASEQ_DEATH_FLYFWD; // Fly forward.
				else if (speed < -16.0f)
					aseq_index = ASEQ_DEATH_FLYBACK; // Fly backward.

				P_PlayerAnimSetLowerSeq(&self->client->playerinfo, aseq_index);
				gi.sound(self, CHAN_BODY, gi.soundindex(va("*death%i.wav", irand(1, 2))), 1.0f, ATTN_NORM, 0.0f);
			}

			// Make sure it doesn't try and finish an animation.
			P_PlayerAnimSetUpperSeq(&self->client->playerinfo, ASEQ_NONE);
			self->client->playerinfo.upperidle = true;

			// If we're not a chicken, don't set the dying flag.
			if (!(self->client->playerinfo.edictflags & FL_CHICKEN))
			{
				self->dead_state = DEAD_DYING;
				self->client->playerinfo.deadflag = DEAD_DYING;
			}
			else
			{
				// I WAS a chicken, but not any more, I'm dead and an Elf again.
				self->client->playerinfo.edictflags &= ~FL_CHICKEN;
			}
		}
	}

	ClientObituary(self, attacker);
	gi.linkentity(self);
}

#pragma region ========================== SelectSpawnPoint logic ==========================

// Returns the distance to the nearest player from the given spot.
static float PlayersRangeFromSpot(const edict_t* spot)
{
	float best_player_dist = 9999999.0f; //TODO: why not FLT_MAX?..

	for (int i = 1; i <= MAXCLIENTS; i++)
	{
		const edict_t* player = &g_edicts[i];

		if (!player->inuse || player->health < 1)
			continue;

		vec3_t v;
		VectorSubtract(spot->s.origin, player->s.origin, v);
		const float player_dist = VectorLength(v);

		best_player_dist = min(player_dist, best_player_dist);
	}

	return best_player_dist;
}

static edict_t* SelectDeathmatchSpawnPoint(void) //mxd. 'SelectFarthestDeathmatchSpawnPoint' in original logic.
{
	edict_t* spot = NULL;
	edict_t* best_spot = NULL;
	float best_dist = 0.0f;

	while ((spot = G_Find(spot, FOFS(classname), "info_player_deathmatch")) != NULL)
	{
		const float best_player_dist = PlayersRangeFromSpot(spot);

		if (best_player_dist > best_dist)
		{
			best_spot = spot;
			best_dist = best_player_dist;
		}
	}

	if (best_spot != NULL)
		return best_spot;

	// If a player just spawned on each and every start spot, we have no choice to turn one into a telefrag meltdown...
	return G_Find(NULL, FOFS(classname), "info_player_deathmatch");
}

static edict_t* SelectCoopSpawnPoint(const edict_t* ent)
{
	int index = ent->client - game.clients;

	// Player 0 starts in normal player spawn point.
	if (index == 0)
		return NULL;

	edict_t* spot = NULL;

	// Assume there are four coop spots at each spawnpoint.
	while ((spot = G_Find(spot, FOFS(classname), "info_player_coop")) != NULL)
	{
		const char* target = spot->targetname;
		if (target == NULL)
			target = "";

		if (Q_stricmp(game.spawnpoint, target) == 0 && --index == 0)
			return spot; // This is a coop spawn point for one of the clients here.
	}

	return NULL; // We didn't have enough...
}

// Chooses a player start, deathmatch start, coop start, etc.
void SelectSpawnPoint(const edict_t* ent, vec3_t origin, vec3_t angles)
{
	edict_t* spot = NULL;

	if (DEATHMATCH)
		spot = SelectDeathmatchSpawnPoint();
	else if (COOP)
		spot = SelectCoopSpawnPoint(ent);

	// Find a single player start spot.
	if (spot == NULL)
	{
		while ((spot = G_Find(spot, FOFS(classname), "info_player_start")) != NULL)
		{
			if (!game.spawnpoint[0] && spot->targetname == NULL)
				break;

			if (!game.spawnpoint[0] || spot->targetname == NULL)
				continue;

			if (Q_stricmp(game.spawnpoint, spot->targetname) == 0)
				break;
		}

		if (spot == NULL)
		{
			if (!game.spawnpoint[0])
				spot = G_Find(spot, FOFS(classname), "info_player_start"); // There wasn't a spawnpoint without a target, so use any.

			if (spot == NULL)
				gi.error("Couldn't find spawn point %s\n", game.spawnpoint);
		}
	}

	// Do a trace to the floor to find where to put player.
	vec3_t end_pos;
	VectorCopy(spot->s.origin, end_pos);
	end_pos[2] -= 1000.0f;

	trace_t tr;
	gi.trace(spot->s.origin, vec3_origin, vec3_origin, end_pos, NULL, CONTENTS_WORLD_ONLY | MASK_PLAYERSOLID, &tr);

	VectorCopy(tr.endpos, origin);
	origin[2] -= player_mins[2];

	// ???
	VectorCopy(spot->s.angles, angles);
}

#pragma endregion

#pragma region ========================== Respawn / body que logic ==========================

void InitBodyQue(void)
{
	level.body_que = 0;

	for (int i = 0; i < BODY_QUEUE_SIZE; i++)
	{
		edict_t* ent = G_Spawn();
		ent->classname = "bodyque";
	}
}

static void PlayerBodyDie(edict_t* self, edict_t* inflictor, edict_t* attacker, int damage, const vec3_t point) //mxd. Named 'player_body_die' in original logic.
{
	gi.sound(self, CHAN_BODY, gi.soundindex("misc/fleshbreak.wav"), 1.0f, ATTN_NORM, 0.0f);

	vec3_t mins;
	VectorCopy(self->mins, mins);
	mins[2] = -30.0f;

	const byte mag = (byte)(Clamp(VectorLength(mins), 1.0f, 255.0f));

	gi.CreateEffect(NULL, FX_FLESH_DEBRIS, 0, self->s.origin, "bdb", irand(10, 30), mins, mag);
	gi.unlinkentity(self);

	VectorClear(self->mins);
	VectorClear(self->maxs);
	VectorClear(self->absmin);
	VectorClear(self->absmax);
	VectorClear(self->size);

	self->movetype = PHYSICSTYPE_NONE;
	self->solid = SOLID_NOT;
	self->clipmask = 0;
	self->takedamage = DAMAGE_NO;
	self->materialtype = MAT_NONE;
	self->health = 0;
	self->die = NULL;
	self->dead_state = DEAD_DEAD;
	self->s.modelindex = 0;

	gi.linkentity(self);
}

static void CopyToBodyQue(edict_t* ent)
{
	if (ent->s.modelindex == 0)
		return; // Safety - was gibbed?

	if (level.body_que == -1)
	{
		vec3_t origin;
		VectorCopy(ent->s.origin, origin);
		origin[2] += ent->mins[2] + 8.0f;

		// Put in the pretty effect when removing the corpse first.
		gi.CreateEffect(NULL, FX_CORPSE_REMOVE, 0, origin, "");

		// No body que on this level.
		return;
	}

	// Grab a body que and cycle to the next one.
	edict_t* body = &g_edicts[level.body_que + MAXCLIENTS + 1];
	level.body_que = (level.body_que + 1) % BODY_QUEUE_SIZE;

	gi.unlinkentity(ent);

	// If the body was being used, then lets put an effect on it before removing it.
	if (body->inuse && body->s.modelindex != 0)
	{
		vec3_t origin;
		VectorCopy(body->s.origin, origin);
		origin[2] += body->mins[2] + 8.0f;

		gi.CreateEffect(NULL, FX_CORPSE_REMOVE, 0, origin, "");
	}

	gi.unlinkentity(body);

	body->s = ent->s;
	body->s.number = (short)(body - g_edicts);
	body->s.skeletalType = SKEL_NULL;
	body->s.effects &= ~(EF_JOINTED | EF_SWAPFRAME);
	body->s.rootJoint = NULL_ROOT_JOINT;
	body->s.swapFrame = NO_SWAP_FRAME;
	body->owner = ent->owner;

	VectorScale(ent->mins, 0.5f, body->mins);
	VectorScale(ent->maxs, 0.5f, body->maxs);
	body->maxs[2] = 10.0f;

	VectorCopy(ent->absmin, body->absmin);
	VectorCopy(ent->absmax, body->absmax);
	body->absmax[2] = 10.0f;

	VectorCopy(ent->size, body->size);
	body->svflags = ent->svflags | SVF_DEADMONSTER; // Stops player getting stuck.
	body->movetype = PHYSICSTYPE_STEP;
	body->solid = SOLID_BBOX;
	body->clipmask = MASK_PLAYERSOLID;
	body->takedamage = DAMAGE_YES;
	body->materialtype = MAT_FLESH;
	body->health = 25;
	body->dead_state = DEAD_NO;
	body->die = PlayerBodyDie;

	gi.linkentity(body);

	// Clear out any client effectsBuffer_t on the corpse (inherited from the player who just died)
	// as the engine will take care of de-allocating any effects still on the player.
	memset(&body->s.clientEffects, 0, sizeof(EffectsBuffer_t));

	//FIXME: Re-create certain client effects that were on the player when he died (e.g. fire).
}

void ClientRespawn(edict_t* self) //TODO: rename to Respawn().
{
	if (!DEATHMATCH && !COOP)
	{
		// Restart the entire server.
		gi.AddCommandString("menu_loadgame\n");
		return;
	}

	// FIXME: make bodyque objects obey gravity.
	if (!(self->flags & FL_CHICKEN) && !(int)dm_no_bodies->value)
		CopyToBodyQue(self); // We're not set as a chicken, so duplicate ourselves.

	// Create a persistent FX_REMOVE_EFFECTS effect - this is a special hack.
	// If we create a regular FX_REMOVE_EFFECTS effect, it will overwrite the next FX_PLAYER_PERSISTANT sent out.
	gi.CreatePersistantEffect(&self->s, FX_REMOVE_EFFECTS, CEF_BROADCAST | CEF_OWNERS_ORIGIN, NULL, "s", 0);

	// Respawning in deathmatch always means a complete reset of the player's model.
	// Respawning in coop always means a partial reset of the player's model.
	self->client->complete_reset = (DEATHMATCH ? 1 : 0);

	PutClientInServer(self);

	// Do the teleport sound and effect.
	gi.sound(self, CHAN_WEAPON, gi.soundindex("weapons/teleport.wav"), 1.0f, ATTN_NORM, 0.0f);
	gi.CreateEffect(&self->s, FX_PLAYER_TELEPORT_IN, CEF_OWNERS_ORIGIN, self->s.origin, NULL);

	// Hold in place briefly.
	self->client->ps.pmove.pm_time = 50; // Q2: 14 //TODO: does nothing on its own. Should also set PMF_TIME_TELEPORT flag. Or not needed at all?..
}

#pragma endregion

void SpawnInitialPlayerEffects(edict_t* ent)
{
	PlayerRestartShrineFX(ent);

	// Don't need to keep track of this persistent effect, since its started but never stopped.
	gi.CreatePersistantEffect(&ent->s, FX_PLAYER_PERSISTANT, CEF_BROADCAST | CEF_OWNERS_ORIGIN, NULL, "");

	if (DEATHMATCH || COOP)
		PlayerLeaderEffect();
}

// Some information that should be persistent, like health, is still stored in the edict structure,
// so it needs to be mirrored out to the game.client structure(s) before all the edicts are wiped.
void SaveClientData(void)
{
	for (int i = 1; i <= game.maxclients; i++)
	{
		const edict_t* client = &g_edicts[i];

		if (!client->inuse)
			continue;

		client_persistant_t* pers = &game.clients[i].playerinfo.pers; //mxd
		pers->health = client->health;

		if (COOP)
		{
			pers->health = max(25, pers->health);
			pers->score = client->client->resp.score;
		}

		pers->max_health = client->max_health;

		pers->mission_num1 = client->client->ps.mission_num1;
		pers->mission_num2 = client->client->ps.mission_num2;
	}
}

static void FetchClientEntData(edict_t* client)
{
	const client_persistant_t* pers = &client->client->playerinfo.pers;
	client->health = pers->health;

	if (COOP)
	{
		client->health = max(25, client->health);
		client->client->resp.score = pers->score;
	}

	client->max_health = pers->max_health;

	client->client->ps.mission_num1 = pers->mission_num1;
	client->client->ps.mission_num2 = pers->mission_num2;
}

static void GiveWeapon(const edict_t* player, const char* name, const enum weaponready_e weapon_id) //mxd. Added to reduce code duplication.
{
	gitem_t* item = P_FindItem(name);

	if (AddWeaponToInventory(item, player))
	{
		playerinfo_t* info = &player->client->playerinfo;

		if (info->pers.autoweapon && (info->pers.newweapon == NULL || ITEM_INDEX(item) > ITEM_INDEX(info->pers.newweapon)))
		{
			info->pers.newweapon = item;
			info->switchtoweapon = weapon_id;
		}
	}
}

// If additional starting weapons and defenses are specified by the current map, give them to the player
// (to support players joining a coop game midway through).
static void GiveLevelItems(edict_t* player)
{
	// Add weapons.
	if (level.offensive_weapons & 1)
		GiveWeapon(player, "staff", WEAPON_READY_SWORDSTAFF); //mxd

	if (level.offensive_weapons & 2)
		GiveWeapon(player, "fball", WEAPON_READY_HANDS); //mxd

	if (level.offensive_weapons & 4)
		GiveWeapon(player, "hell", WEAPON_READY_HELLSTAFF); //mxd

	if (level.offensive_weapons & 8)
		GiveWeapon(player, "array", WEAPON_READY_HANDS); //mxd

	if (level.offensive_weapons & 16)
		GiveWeapon(player, "rain", WEAPON_READY_BOW); //mxd

	if (level.offensive_weapons & 32)
		GiveWeapon(player, "sphere", WEAPON_READY_HANDS); //mxd

	if (level.offensive_weapons & 64)
		GiveWeapon(player, "phoen", WEAPON_READY_BOW); //mxd

	if (level.offensive_weapons & 128)
		GiveWeapon(player, "mace", WEAPON_READY_HANDS); //mxd

	if (level.offensive_weapons & 256)
		GiveWeapon(player, "fwall", WEAPON_READY_HANDS); //mxd

	// Add defenses.
	if (level.defensive_weapons & 1)
		AddDefenseToInventory(P_FindItem("ring"), player);

	if (level.defensive_weapons & 2)
		AddDefenseToInventory(P_FindItem("lshield"), player);

	if (level.defensive_weapons & 4)
		AddDefenseToInventory(P_FindItem("tele"), player);

	if (level.defensive_weapons & 8)
		AddDefenseToInventory(P_FindItem("morph"), player);

	if (level.defensive_weapons & 16)
		AddDefenseToInventory(P_FindItem("meteor"), player);

	Player_UpdateModelAttributes(player); //mxd
}

static void InitClientPersistant(const edict_t* player)
{
	playerinfo_t* info = &player->client->playerinfo; //mxd

	memset(&info->pers, 0, sizeof(info->pers));

	// Set up player's health.
	info->pers.health = PLAYER_HEALTH; //mxd. Use define.

	// Set up maximums amounts for health, mana and ammo for bows and hellstaff.
	info->pers.max_health = PLAYER_HEALTH; //mxd. Use define.
	info->pers.max_offmana = MAX_OFF_MANA;
	info->pers.max_defmana = MAX_DEF_MANA;
	info->pers.max_redarrow = MAX_RAIN_AMMO;
	info->pers.max_phoenarr = MAX_PHOENIX_AMMO;
	info->pers.max_hellstaff = MAX_HELL_AMMO;

	// Give defensive and offensive weapons to player.
	info->pers.weapon = NULL;
	info->pers.defence = NULL;

	// Give just the sword-staff and flying-fist to the player as starting weapons.
	gitem_t* item = P_FindItem("staff");
	AddWeaponToInventory(item, player);
	info->pers.selected_item = ITEM_INDEX(item);
	info->pers.weapon = item;
	info->pers.lastweapon = item;
	info->weap_ammo_index = 0;

	if (!(DMFLAGS & DF_NO_OFFENSIVE_SPELL))
	{
		item = P_FindItem("fball");
		AddWeaponToInventory(item, player);
		info->pers.selected_item = ITEM_INDEX(item);
		info->pers.weapon = item;
		info->pers.lastweapon = item;
		info->weap_ammo_index = ITEM_INDEX(P_FindItem(item->ammo));
	}

	// Give Tome of Power to the player.
	item = P_FindItem("powerup");
	AddDefenseToInventory(item, player);
	info->pers.defence = item;

	// Start player with half offensive and defensive mana - as instructed by Brian P.
	item = P_FindItem("Off-mana");
	info->pers.inventory.Items[ITEM_INDEX(item)] = info->pers.max_offmana / 2;

	item = P_FindItem("Def-mana");
	info->pers.inventory.Items[ITEM_INDEX(item)] = info->pers.max_defmana / 2;

	info->pers.connected = true;
}

// Called when a player connects to a server or respawns in a deathmatch.
static void PutClientInServer(edict_t* ent)
{
	// Find a spawn point. Do it before setting health back up, so farthest ranging doesn't count this client.
	vec3_t spawn_origin;
	vec3_t spawn_angles;
	SelectSpawnPoint(ent, spawn_origin, spawn_angles);

	const int index = ent - g_edicts - 1;
	gclient_t* client = ent->client;

	// The player's starting plague skin is determined by the worldspawn's s.skinnum.
	// We set this up now because the ClientUserinfoChanged needs to know the plaguelevel.
	if (!DEATHMATCH)
		client->playerinfo.plaguelevel = min((byte)world->s.skinnum, PLAGUE_NUM_LEVELS - 1);

	char userinfo[MAX_INFO_STRING];
	memcpy(userinfo, client->playerinfo.pers.userinfo, sizeof(userinfo));
	client_respawn_t resp;

	// Deathmatch wipes most client data every spawn.
	if (DEATHMATCH)
	{
		resp = client->resp;
		InitClientPersistant(ent);
		ClientUserinfoChanged(ent, userinfo);
	}
	else if (COOP)
	{
		resp = client->resp;
		ClientUserinfoChanged(ent, userinfo);

		if (resp.score > client->playerinfo.pers.score)
			client->playerinfo.pers.score = resp.score;
	}
	else
	{
		ClientUserinfoChanged(ent, userinfo);
		memset(&resp, 0, sizeof(resp));
	}

	// Complete or partial reset of the player's model?
	const qboolean complete_reset = (DEATHMATCH ? true : client->complete_reset);

	// Initialise the player's gclient_t.

	// Clear everything but the persistent data.
	const byte plague_level = client->playerinfo.plaguelevel; // Save me too.
	const client_persistant_t saved = client->playerinfo.pers;
	memset(client, 0, sizeof(gclient_t));
	client->playerinfo.pers = saved;

	// Initialise...
	if (client->playerinfo.pers.health <= 0)
		InitClientPersistant(ent);

	client->resp = resp;

	// Restore data that is persistent across level changes.
	FetchClientEntData(ent);

	// Initialize the player's edict_t.
	ent->groundentity = NULL;
	ent->client = &game.clients[index];
	ent->s.clientnum = (short)index;
	ent->takedamage = DAMAGE_AIM;
	ent->materialtype = MAT_FLESH;
	ent->movetype = PHYSICSTYPE_STEP;
	ent->viewheight = 0;
	ent->just_deleted = 0;
	ent->client_sent = 0;
	ent->inuse = true;
	ent->s.scale = 1.0f;
	ent->classname = "player";
	ent->mass = 200;
	ent->solid = SOLID_BBOX;
	ent->dead_state = DEAD_NO;
	ent->air_finished = level.time + HOLD_BREATH_TIME;
	ent->clipmask = MASK_PLAYERSOLID;
	ent->PersistantCFX = 0; //mxd
	ent->Leader_PersistantCFX = 0;

	// Default to making us not invulnerable (may change later).
	ent->client->shrine_framenum = 0;

	// A few multiplayer reset safeguards... i.e. if we were teleporting when we died, we aren't now.
	client->playerinfo.flags &= ~PLAYER_FLAG_TELEPORT;
	VectorClear(client->tele_dest);
	client->tele_count = 0;
	ent->s.color.c = 0; // Restore model visibility. //TODO: this makes player model render as translucent in GL_DrawFlexFrameLerp().

	ent->fire_damage_time = 0.0f;
	ent->fire_timestamp = 0.0f;

	ent->model = "players/male/tris.fm";

	ent->pain = PlayerPain;
	ent->die = PlayerDie;
	ent->waterlevel = 0;
	ent->watertype = 0;
	ent->flags &= ~FL_NO_KNOCKBACK;
	ent->svflags &= ~SVF_DEADMONSTER;

	VectorCopy(player_mins, ent->mins);
	VectorCopy(player_maxs, ent->maxs);
	VectorCopy(player_mins, ent->intentMins);
	VectorCopy(player_maxs, ent->intentMaxs);
	VectorClear(ent->velocity);

	// Initialize the player's gclient_t and playerstate_t.

	for (int i = 0; i < 3; i++)
		client->ps.pmove.origin[i] = (short)(spawn_origin[i] * 8.0f);

	client->ps.fov = (float)(Q_atoi(Info_ValueForKey(client->playerinfo.pers.userinfo, "fov")));

	if (client->ps.fov < 1.0f)
		client->ps.fov = FOV_DEFAULT;
	else if (client->ps.fov > 160.0f)
		client->ps.fov = 160.0f;

	VectorClear(client->ps.offsetangles);

	// Set the delta angles, reset the camera delta angles.
	for (int i = 0; i < 3; i++)
	{
		client->ps.pmove.delta_angles[i] = ANGLE2SHORT(spawn_angles[i] - client->resp.cmd_angles[i]);
		client->ps.pmove.camera_delta_angles[i] = 0;
	}

	client->ps.remote_id = -1;

	// Initialize the player's entity_state_t.

	ent->s.frame = 0; // Zero the current animation frame.
	ent->s.modelindex = 255; // Modelindex is always 255 for player models.

	// Set up the model's origin, making sure it's off the ground.
	VectorCopy(spawn_origin, ent->s.origin);
	ent->s.origin[2] += 1.0f;
	VectorCopy(ent->s.origin, ent->s.old_origin);

	VectorSet(ent->s.angles, 0.0f, spawn_angles[YAW], 0.0f);
	VectorCopy(ent->s.angles, client->ps.viewangles);
	VectorCopy(ent->s.angles, client->v_angle);

	KillBox(ent);

	ent->s.effects = (EF_CAMERA_NO_CLIP | EF_SWAPFRAME | EF_JOINTED | EF_PLAYER);

	// Set up skeletal info. Note, skeleton has been created already.
	ent->s.skeletalType = SKEL_CORVUS;

	// Link us into the physics system.
	gi.linkentity(ent);

	// Initialize the player's playerinfo_t.

	client->playerinfo.plaguelevel = plague_level;

	// Set the player's current offensive and defensive ammo indexes.
	if (client->playerinfo.pers.weapon->ammo != NULL)
		client->playerinfo.weap_ammo_index = ITEM_INDEX(P_FindItem(client->playerinfo.pers.weapon->ammo));

	if (client->playerinfo.pers.defence != NULL)
		client->playerinfo.def_ammo_index = ITEM_INDEX(P_FindItem(client->playerinfo.pers.defence->ammo));

	VectorCopy(spawn_origin, client->playerinfo.origin);
	VectorClear(client->playerinfo.velocity);

	// Make the player have the right attributes - armor that sort of thing.
	Player_UpdateModelAttributes(ent); //mxd

	if (DEATHMATCH || COOP)
	{
		// Reset the player's fmodel nodes when spawning in deathmatch or coop.
		ResetPlayerBaseNodes(ent);

		// Just in case we were on fire when we died.
		gi.RemoveEffects(&ent->s, FX_FIRE_ON_ENTITY);

		// Make us invincible for a few seconds after spawn.
		ent->client->shrine_framenum = level.time + 3.3f;
	}

	InitPlayerinfo(ent);
	SetupPlayerinfo(ent);
	P_PlayerInit(&ent->client->playerinfo, complete_reset);
	WritePlayerinfo(ent);

	SpawnInitialPlayerEffects(ent);

	if (COOP)
		GiveLevelItems(ent);

	// For blade only DMing, ensure we start with staff in our hand.
	if (DMFLAGS & DF_NO_OFFENSIVE_SPELL)
	{
		client->playerinfo.pers.newweapon = P_FindItem("staff");
		client->playerinfo.switchtoweapon = WEAPON_READY_SWORDSTAFF;
	}
}

static void InitClientResp(gclient_t* client)
{
	memset(&client->resp, 0, sizeof(client->resp));
	client->resp.enterframe = level.framenum;
	client->resp.coop_respawn = client->playerinfo.pers;
}

// A client has just connected to the server in deathmatch mode, so clear everything out before starting them.
static void ClientBeginDeathmatch(edict_t* ent)
{
	G_InitEdict(ent);
	InitClientResp(ent->client);

	// Locate ent at a spawn point.
	PutClientInServer(ent);

	// Do the teleport sound and client effect and announce the player's entry into the level.
	gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/teleport.wav"), 1.0f, ATTN_NORM, 0.0f);
	gi.CreateEffect(&ent->s, FX_PLAYER_TELEPORT_IN, CEF_OWNERS_ORIGIN, ent->s.origin, NULL);
	gi.Obituary(PRINT_HIGH, GM_ENTERED, ent->s.number, 0);

	// Make sure all view stuff is valid.
	ClientEndServerFrame(ent);
}

// Called when a client has finished connecting, and is ready to be placed into the game.
// This will happen on every level load.
void ClientBegin(edict_t* ent)
{
	ent->client = &game.clients[ent - g_edicts - 1];

	if (DEATHMATCH)
	{
		ClientBeginDeathmatch(ent);
		return;
	}

	// If there is already a body waiting for us (a loadgame), just take it, otherwise spawn one from scratch.
	if (ent->inuse)
	{
		// The client has cleared the client side cl.inputangles and cl.viewangles upon connecting to the server,
		// which is different from the state when the game is saved, so we need to compensate with delta_angles and camera_delta_angles.
		for (int i = 0; i < 3; i++)
		{
			ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(ent->client->v_angle[i]);
			ent->client->ps.pmove.camera_delta_angles[i] = (short)(ANGLE2SHORT(ent->client->ps.viewangles[i]) - ent->client->ps.pmove.delta_angles[i]);
		}

		SpawnInitialPlayerEffects(ent);
	}
	else
	{
		// A spawn point will completely reinitialize the entity except for the persistent data that was initialized at ClientConnect() time.
		G_InitEdict(ent);
		ent->classname = "player";
		InitClientResp(ent->client);
		PutClientInServer(ent);
	}

	// All resets should be partial, until ClientConnect() gets called again for a new game and respawn() occurs
	// (which will do the correct reset type).
	ent->client->complete_reset = 0;

	if (level.intermissiontime > 0.0f)
	{
		MoveClientToIntermission(ent, false);
	}
	else if (game.maxclients > 1) // Send effect if in a multiplayer game.
	{
		// Do the teleport sound and client effect and announce the player's entry into the level.
		gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/teleport.wav"), 1.0f, ATTN_NORM, 0.0f);
		gi.CreateEffect(&ent->s, FX_PLAYER_TELEPORT_IN, CEF_OWNERS_ORIGIN, ent->s.origin, NULL);
		gi.Obituary(PRINT_HIGH, GM_ENTERED, ent->s.number, 0);
	}

	// Make sure all view stuff is valid.
	ClientEndServerFrame(ent);
}

#pragma region ========================== ClientUserinfoChanged logic ==========================

//mxd. Split from ClientUserinfoChanged().
static void SetDMPlayerSkin(const edict_t* ent, const char* skin_name, const int player_num)
{
	char skin[MAX_QPATH];

	// In DM any skins are okay.
	if (strchr(skin_name, '/') == NULL) // Backward compatibility, if not model, then assume male.
		sprintf_s(skin, sizeof(skin), "male/%s", skin_name); //mxd. sprintf -> sprintf_s
	else
		strcpy_s(skin, sizeof(skin), skin_name); //mxd. strcpy -> strcpy_s

	char filename[MAX_QPATH];
	sprintf_s(filename, sizeof(filename), "players/%s.m8", skin); //mxd. sprintf -> sprintf_s

	FILE* f;
	if (FS_FOpenFile(filename, &f) != -1)
	{
		FS_FCloseFile(f); // Just checking for the existence of the file.
	}
	else
	{
		if (strstr(skin_name, "female/")) // This was a female skin, fall back to Kiera.
			strcpy_s(skin, sizeof(skin), "female/Kiera"); //mxd. strcpy -> strcpy_s
		else // Anything else, assume that it was male.
			strcpy_s(skin, sizeof(skin), "male/Corvus"); //mxd. strcpy -> strcpy_s
	}

	gi.configstring(CS_PLAYERSKINS + player_num, va("%s\\%s", ent->client->playerinfo.pers.netname, skin));
}

//mxd. Split from ClientUserinfoChanged().
static void SetCoopPlayerSkin(const edict_t* ent, const char* skin_name, const int player_num)
{
	char skin[MAX_QPATH];
	qboolean found = false;

	// In coop only allow skins that have full plague levels...
	if (strchr(skin_name, '/') == NULL) // Backward compatibility, if not model, then assume male.
		sprintf_s(skin, sizeof(skin), "male/%s", skin_name); //mxd. sprintf -> sprintf_s
	else
		strcpy_s(skin, sizeof(skin), skin_name); //mxd. strcpy -> strcpy_s

	char filename[MAX_QPATH];
	sprintf_s(filename, sizeof(filename), "players/%s.m8", skin); //mxd. sprintf -> sprintf_s

	FILE* f;
	if (FS_FOpenFile(filename, &f) != -1)
	{
		FS_FCloseFile(f); // Just checking for the existence of the file.

		if ((int)allowillegalskins->value)
		{
			found = true; // All we need is the base skin.
		}
		else
		{
			sprintf_s(filename, sizeof(filename), "players/%sP1.m8", skin); //mxd. sprintf -> sprintf_s

			if (FS_FOpenFile(filename, &f) != -1)
			{
				FS_FCloseFile(f); // Just checking for the existence of the file.

				sprintf_s(filename, sizeof(filename), "players/%sP2.m8", skin); //mxd. sprintf -> sprintf_s

				if (FS_FOpenFile(filename, &f) != -1)
				{
					FS_FCloseFile(f); // Just checking for the existence of the file.
					found = true;
				}
			}
		}
	}

	if (!found)
	{
		// Not all three skins were found.
		if (strstr(skin_name, "female/") != NULL)
			strcpy_s(skin, sizeof(skin), "female/Kiera"); // This was a female skin, fall back to Kiera. //mxd. strcpy -> strcpy_s
		else
			strcpy_s(skin, sizeof(skin), "male/Corvus"); // Anything else, assume that it was male. //mxd. strcpy -> strcpy_s
	}

	// Combine name and skin into a configstring.
	switch (ent->client->playerinfo.plaguelevel)
	{
		case 1: // Plague level 1
			if ((int)allowillegalskins->value)
			{
				// Do the check for a valid skin in case an illegal skin has been let through.
				sprintf_s(filename, sizeof(filename), "players/%sP1.m8", skin); //mxd. sprintf -> sprintf_s

				if (FS_FOpenFile(filename, &f) != -1)
				{
					// The plague1 skin exists.
					FS_FCloseFile(f); // Just checking for the existence of the file.
					gi.configstring(CS_PLAYERSKINS + player_num, va("%s\\%sP1", ent->client->playerinfo.pers.netname, skin));
				}
				else
				{
					// Just use the basic skin, then.
					gi.configstring(CS_PLAYERSKINS + player_num, va("%s\\%s", ent->client->playerinfo.pers.netname, skin));
				}
			}
			else
			{
				gi.configstring(CS_PLAYERSKINS + player_num, va("%s\\%sP1", ent->client->playerinfo.pers.netname, skin));
			}
			break;

		case 2: // Plague level 2
			if ((int)allowillegalskins->value)
			{
				// Do the check for a valid skin in case an illegal skin has been let through.
				sprintf_s(filename, sizeof(filename), "players/%sP2.m8", skin); //mxd. sprintf -> sprintf_s

				if (FS_FOpenFile(filename, &f) != -1)
				{
					// The plague1 skin exists.
					FS_FCloseFile(f); // Just checking for the existence of the file.
					gi.configstring(CS_PLAYERSKINS + player_num, va("%s\\%sP2", ent->client->playerinfo.pers.netname, skin));
				}
				else
				{
					// No plague level 2 skin, try for a plague level 1 skin.
					sprintf_s(filename, sizeof(filename), "players/%sP1.m8", skin); //mxd. sprintf -> sprintf_s

					if (FS_FOpenFile(filename, &f) != -1)
					{
						// The plague1 skin exists.
						FS_FCloseFile(f); // Just checking for the existence of the file.
						gi.configstring(CS_PLAYERSKINS + player_num, va("%s\\%sP1", ent->client->playerinfo.pers.netname, skin));
					}
					else
					{	// Just use the basic skin, then.
						gi.configstring(CS_PLAYERSKINS + player_num, va("%s\\%s", ent->client->playerinfo.pers.netname, skin));
					}
				}
			}
			else
			{
				gi.configstring(CS_PLAYERSKINS + player_num, va("%s\\%sP2", ent->client->playerinfo.pers.netname, skin));
			}
			break;

		default:
			gi.configstring(CS_PLAYERSKINS + player_num, va("%s\\%s", ent->client->playerinfo.pers.netname, skin));
	}
}

//mxd. Split from ClientUserinfoChanged().
static void SetSPPlayerSkin(const edict_t* ent, const char* skin_name, const int player_num)
{
	// Single player. This is CORVUS ONLY unless allowillegalskins is engaged
	if ((int)allowillegalskins->value)
	{
		char skin[MAX_QPATH];

		// Allow any skin at all.
		if (strchr(skin_name, '/') == NULL) // Backward compatibility, if not model, then assume male
			sprintf_s(skin, sizeof(skin), "male/%s", skin_name); //mxd. sprintf -> sprintf_s
		else
			strcpy_s(skin, sizeof(skin), skin_name); //mxd. strcpy -> strcpy_s

		char filename[MAX_QPATH];
		sprintf_s(filename, sizeof(filename), "players/%s.m8", skin); //mxd. sprintf -> sprintf_s

		FILE* f;
		if (FS_FOpenFile(filename, &f) != -1)
			FS_FCloseFile(f); // Just checking for the existence of the file.
		else
			strcpy_s(skin, sizeof(skin), "male/Corvus"); //mxd. strcpy -> strcpy_s

		// Combine name and skin into a configstring.
		switch (ent->client->playerinfo.plaguelevel)
		{
			case 1: // Plague level 1.
				sprintf_s(filename, sizeof(filename), "players/%sP1.m8", skin); //mxd. sprintf -> sprintf_s

				if (FS_FOpenFile(filename, &f) != -1)
				{
					// The plague1 skin exists.
					FS_FCloseFile(f); // Just checking for the existence of the file.
					gi.configstring(CS_PLAYERSKINS + player_num, va("%s\\%sP1", ent->client->playerinfo.pers.netname, skin));
				}
				else
				{
					// Just use the basic skin, then.
					gi.configstring(CS_PLAYERSKINS + player_num, va("%s\\%s", ent->client->playerinfo.pers.netname, skin));
				}
				break;

			case 2: // Plague level 2.
				sprintf_s(filename, sizeof(filename), "players/%sP2.m8", skin); //mxd. sprintf -> sprintf_s

				if (FS_FOpenFile(filename, &f) != -1)
				{
					// The plague1 skin exists.
					FS_FCloseFile(f); // Just checking for the existence of the file.
					gi.configstring(CS_PLAYERSKINS + player_num, va("%s\\%sP2", ent->client->playerinfo.pers.netname, skin));
				}
				else
				{
					// No plague 2 skin, try for a plague 1 skin.
					sprintf_s(filename, sizeof(filename), "players/%sP1.m8", skin); //mxd. sprintf -> sprintf_s

					if (FS_FOpenFile(filename, &f) != -1)
					{
						// The plague1 skin exists.
						FS_FCloseFile(f); // Just checking for the existence of the file.
						gi.configstring(CS_PLAYERSKINS + player_num, va("%s\\%sP1", ent->client->playerinfo.pers.netname, skin));
					}
					else
					{	// Just use the basic skin, then.
						gi.configstring(CS_PLAYERSKINS + player_num, va("%s\\%s", ent->client->playerinfo.pers.netname, skin));
					}
				}
				break;

			default:
				gi.configstring(CS_PLAYERSKINS + player_num, va("%s\\%s", ent->client->playerinfo.pers.netname, skin));
				break;
		}
	}
	else
	{
		// Just care about Corvus.
		switch (ent->client->playerinfo.plaguelevel)
		{
			case 1: // Plague level 1.
				gi.configstring(CS_PLAYERSKINS + player_num, va("%s\\male/CorvusP1", ent->client->playerinfo.pers.netname));
				break;

			case 2: // Plague level 2.
				gi.configstring(CS_PLAYERSKINS + player_num, va("%s\\male/CorvusP2", ent->client->playerinfo.pers.netname));
				break;

			default:
				gi.configstring(CS_PLAYERSKINS + player_num, va("%s\\male/Corvus", ent->client->playerinfo.pers.netname));
				break;
		}
	}
}

// Called whenever the player updates a userinfo variable.
// The game can override any of the settings in place (forcing skins or names, etc) before copying it off.
void ClientUserinfoChanged(edict_t* ent, char* userinfo) //TODO: add int userinfo_size arg?
{
	assert(ent->client != NULL);

	// Check for malformed or illegal info strings.
	if (!Info_Validate(userinfo))
		strcpy_s(userinfo, MAX_INFO_STRING, "\\name\\badinfo\\skin\\male/Corvus"); //mxd. strcpy -> strcpy_s

	// Set name.
	const char* name = Info_ValueForKey(userinfo, "name");
	client_persistant_t* pers = &ent->client->playerinfo.pers; //mxd
	const int name_size = sizeof(pers->netname); //mxd
	strncpy_s(pers->netname, name_size, name, name_size - 1); //mxd. strncpy -> strncpy_s

	// Set skin.
	const char* skin_name = Info_ValueForKey(userinfo, "skin");
	const int player_num = ent - g_edicts - 1;

	// Please note that this function became very long with the various limitations of coop and single-play skins...
	if (DEATHMATCH)
		SetDMPlayerSkin(ent, skin_name, player_num); //mxd
	else if (COOP)
		SetCoopPlayerSkin(ent, skin_name, player_num); //mxd
	else // Single player.
		SetSPPlayerSkin(ent, skin_name, player_num); //mxd

	// Change skins, but lookup the proper skintype.
	Player_UpdateModelAttributes(ent);

	// FOV.
	ent->client->ps.fov = (float)(Q_atoi(Info_ValueForKey(userinfo, "fov")));

	if (ent->client->ps.fov < 1.0f)
		ent->client->ps.fov = FOV_DEFAULT;
	else if (ent->client->ps.fov > 160.0f)
		ent->client->ps.fov = 160.0f;

	// Autoweapon changeup.
	const char* autoweapon = Info_ValueForKey(userinfo, "autoweapon");
	if (autoweapon[0] != 0) //mxd. strlen(str) -> str[0] check.
		ent->client->playerinfo.pers.autoweapon = Q_atoi(autoweapon);

	// Save off the userinfo in case we want to check something later.
	const int userinfo_size = sizeof(pers->userinfo); //mxd
	strncpy_s(pers->userinfo, userinfo_size, userinfo, userinfo_size - 1); //mxd. strncpy -> strncpy_s
}

#pragma endregion

// Called when a player begins connecting to the server.
// The game can refuse entrance to a client by returning false.
// If the client is allowed, the connection process will continue and eventually get to ClientBegin().
// Changing levels will NOT cause this to be called again.
qboolean ClientConnect(edict_t* ent, char* userinfo)
{
	// Check to see if they are on the banned IP list.
	const char* ip = Info_ValueForKey(userinfo, "ip");
	if (SV_FilterPacket(ip))
		return false;

	// Check for a password.
	const char* pwd = Info_ValueForKey(userinfo, "password");
	if (strcmp(password->string, pwd) != 0)
		return false;

	// Ok, they can connect.
	ent->client = &game.clients[ent - g_edicts - 1];

	// If there isn't already a body waiting for us (a loadgame), spawn one from scratch. Otherwise, just take what's there already.
	if (!ent->inuse)
	{
		// Clear the respawning variables.
		InitClientResp(ent->client);

		if (ent->client->playerinfo.pers.weapon == NULL)
		{
			InitClientPersistant(ent);

			// This is the very first time that this player has entered the game (be it single player, coop or deathmatch)
			// so we want to do a complete reset of the player's model.
			ent->client->complete_reset = 1;
		}
	}
	else
	{
		// The player has a body waiting from a (just) loaded game, so we want to do just a partial reset of the player's model.
		ent->client->complete_reset = 0;
	}

	ClientUserinfoChanged(ent, userinfo);

	if (game.maxclients > 1)
		gi.dprintf("%s connected\n", ent->client->playerinfo.pers.netname);

	ent->client->playerinfo.pers.connected = true;

	return true;
}

// Called when a player drops from the server.
void ClientDisconnect(edict_t* ent)
{
	if (ent->client == NULL)
		return;

	// Inform other players that the disconnecting client has left the game.
	gi.Obituary(PRINT_HIGH, GM_DISCON, ent->s.number, 0);

	// Do the teleport sound and effect.
	gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/teleport.wav"), 1.0f, ATTN_NORM, 0.0f);
	gi.CreateEffect(&ent->s, FX_PLAYER_TELEPORT_OUT, CEF_OWNERS_ORIGIN, ent->s.origin, NULL);

	// Clean up after leaving.
	if (ent->Leader_PersistantCFX > 0)
	{
		gi.RemovePersistantEffect(ent->Leader_PersistantCFX, REMOVE_LEADER_CLIENT);
		gi.RemoveEffects(&ent->s, FX_SHOW_LEADER);
		ent->Leader_PersistantCFX = 0;
	}

	// If we're on a rope, unhook the rope graphic from the disconnecting player.
	if (ent->client->playerinfo.flags & PLAYER_FLAG_ONROPE)
	{
		ent->targetEnt->count = 0;
		ent->targetEnt->rope_grab->s.effects &= ~EF_ALTCLIENTFX;
		ent->targetEnt->enemy = NULL;
		ent->targetEnt = NULL;
	}

	gi.unlinkentity(ent);
	ent->s.modelindex = 0;
	ent->solid = SOLID_NOT;
	ent->inuse = false;
	ent->just_deleted = SERVER_DELETED;
	ent->classname = "disconnected";
	ent->client->playerinfo.pers.connected = false;

	const int player_num = ent - g_edicts - 1;
	gi.configstring(CS_PLAYERSKINS + player_num, "");

	// Redo the leader effect, because this guy has gone, and he might have had it.
	PlayerLeaderEffect();
}

static edict_t* pm_passent;

// The pmove() routine doesn't need to know about passent and contentmask.
static void PM_trace(const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, trace_t* trace)
{
	//NOTE: pmove doesn't need to know the gory details, but I need to be able to detect a water surface.
	// Hence, if the mins and max are NULL, then wask out water (cheezy I know, but blame me) ---Pat
	if (mins == NULL && maxs == NULL)
	{
		gi.trace(start, vec3_origin, vec3_origin, end, pm_passent, MASK_PLAYERSOLID | MASK_WATER, trace);
		return;
	}

	const int mask = ((pm_passent->health > 0) ? MASK_PLAYERSOLID : MASK_DEADSOLID); //mxd
	gi.trace(start, mins, maxs, end, pm_passent, mask, trace);
}

// This will be called once for each client-frame received from a client.
// So during a server frame, for a given client, ClientThink() probably be called several times.
void ClientThink(edict_t* ent, usercmd_t* ucmd)
{
	level.current_entity = ent;
	gclient_t* client = ent->client;

	CheckContinuousAutomaticEffects(ent);

	// Handle an active intermission.
	if (level.intermissiontime > 0.0f)
	{
		client->ps.pmove.pm_type = PM_INTERMISSION;

		// Can exit intermission after five seconds
		if (level.time > level.intermissiontime + 5.0f && (ucmd->buttons & BUTTON_ANY))
			level.exitintermission = true;

		return;
	}

	// Movement stuff.
	if (ent->movetype == PHYSICSTYPE_NOCLIP)
		client->ps.pmove.pm_type = PM_SPECTATOR;
	else if ((ent->s.modelindex != 255) && !(ent->flags & FL_CHICKEN)) // We're not set as a chicken.
		client->ps.pmove.pm_type = PM_GIB;
	else if (ent->dead_state != DEAD_NO)
		client->ps.pmove.pm_type = PM_DEAD;
	else
		client->ps.pmove.pm_type = PM_NORMAL;

	client->ps.pmove.gravity = (short)sv_gravity->value;

	// If we are not currently on a rope, then clear out any ropes as valid for a check.
	if (!(client->playerinfo.flags & PLAYER_FLAG_ONROPE))
		ent->targetEnt = NULL;

	// If we are turn-locked, then set the PMF_LOCKTURN flag that informs the client of this (the client-side camera needs to know).
	if ((client->playerinfo.flags & PLAYER_FLAG_TURNLOCK) && client->ps.pmove.pm_type == PM_NORMAL)
	{
		client->ps.pmove.pm_flags |= PMF_LOCKTURN;
	}
	else
	{
		client->playerinfo.turncmd += SHORT2ANGLE(ucmd->angles[YAW] - client->oldcmdangles[YAW]);
		client->ps.pmove.pm_flags &= ~PMF_LOCKTURN;
	}

	// Save the cmd->angles away so we may calculate the delta (on client->turncmd above) in the next frame.
	for (int i = 0; i < 3; i++)
		client->oldcmdangles[i] = ucmd->angles[i];

	pm_passent = ent;

	// Set up inputs for a Pmove().
	pmove_t pm = { 0 };
	pm.s = client->ps.pmove;

	for (int i = 0; i < 3; i++)
	{
		pm.s.origin[i] = (short)(ent->s.origin[i] * 8.0f);
		pm.s.velocity[i] = (short)(ent->velocity[i] * 8.0f);
	}

	pm.cmd = *ucmd;
	client->pcmd = *ucmd;

	if (ent->movetype != PHYSICSTYPE_NOCLIP)
	{
		pm.cmd.forwardmove = (short)client->playerinfo.fwdvel;
		pm.cmd.sidemove = (short)client->playerinfo.sidevel;
		pm.cmd.upmove = (short)client->playerinfo.upvel;
	}

	if (client->RemoteCameraLockCount > 0)
	{
		pm.cmd.forwardmove = 0;
		pm.cmd.sidemove = 0;
		pm.cmd.upmove = 0;
	}

	// Input the DESIRED waterheight.
	// FIXME: This should be retrieved from the animation frame eventually.
	pm.desiredWaterHeight = 15.0f;
	pm.waterheight = client->playerinfo.waterheight;
	pm.waterlevel = ent->waterlevel;
	pm.watertype = ent->watertype;
	pm.groundentity = ent->groundentity;

	// This is a scale of 0 to 1 describing how much knockback to take into account.
	float knockback = client->playerinfo.knockbacktime - level.time;
	pm.knockbackfactor = Clamp(knockback, 0.0f, 1.0f);

	// Handle lockmove cases.
	if ((client->playerinfo.flags & (PLAYER_FLAG_LOCKMOVE_WAS_SET | PLAYER_FLAG_USE_ENT_POS)) && !(client->ps.pmove.pm_flags & PMF_LOCKMOVE))
	{
		// Lockmove was set last frame, but isn't now, so we copy the player edict's origin and velocity values to the client for use in Pmove().
		// NOTE: Pmove() on the SERVER needs pointers to specify vectors to be read and written for the origin and velocity.
		// So be careful if you screw around with this crazy code.
		client->playerinfo.flags &= ~PLAYER_FLAG_USE_ENT_POS;

		VectorCopy(ent->s.origin, client->playerinfo.origin);
		VectorCopy(ent->velocity, client->playerinfo.velocity);
	}

	// Check to add into movement velocity through crouch and duck if underwater.
	if (ent->dead_state == DEAD_NO)
	{
		if (ent->waterlevel > 2)
		{
			// NOTENOTE: If they're pressing both, nullify it.
			if (client->playerinfo.seqcmd[ACMDL_CROUCH])
				client->playerinfo.velocity[2] -= SWIM_ADJUST_AMOUNT;

			if (client->playerinfo.seqcmd[ACMDL_JUMP])
				client->playerinfo.velocity[2] += SWIM_ADJUST_AMOUNT;
		}
		else if (ent->waterlevel > 1) // On the surface trying to go down?
		{
			// NOTENOTE: If they're pressing both, nullify it.
			if (client->playerinfo.seqcmd[ACMDL_CROUCH])
			{
				pm.s.w_flags |= WF_SINK;
				client->playerinfo.velocity[2] -= SWIM_ADJUST_AMOUNT;
			}

			if (client->playerinfo.seqcmd[ACMDL_JUMP])
				client->playerinfo.velocity[2] += SWIM_ADJUST_AMOUNT;
		}
	}

	pm.origin = client->playerinfo.origin;
	pm.velocity = client->playerinfo.velocity;

	// If not the chicken, and not explicitly resizing the bounding box...
	if (!(client->playerinfo.edictflags & FL_CHICKEN) && !(client->playerinfo.flags & PLAYER_FLAG_RESIZED))
	{
		// Resize the player's bounding box.
		VectorCopy(player_mins, ent->intentMins);
		VectorCopy(player_maxs, ent->intentMaxs);

		ent->physicsFlags |= PF_RESIZE;

		pm.intentMins = ent->intentMins;
		pm.intentMaxs = ent->intentMaxs;
	}
	else
	{
		// Otherwise we don't want to resize.
		if (client->playerinfo.edictflags & FL_AVERAGE_CHICKEN)
		{
			VectorSet(ent->mins, -8.0f, -8.0f, -14.0f);
			VectorSet(ent->maxs, 8.0f, 8.0f, 14.0f);
		}
		else if (client->playerinfo.edictflags & FL_SUPER_CHICKEN)
		{
			VectorSet(ent->mins, -16.0f, -16.0f, -36.0f);
			VectorSet(ent->maxs, 16.0f, 16.0f, 36.0f);
		}

		pm.intentMins = ent->mins;
		pm.intentMaxs = ent->maxs;
	}

	pm.GroundSurface = client->playerinfo.GroundSurface;
	pm.GroundPlane = client->playerinfo.GroundPlane;
	pm.GroundContents = client->playerinfo.GroundContents;

	pm.self = ent;

	pm.trace = PM_trace; // Adds default parms.
	pm.pointcontents = gi.pointcontents;

	pm.viewheight = (float)ent->viewheight;

	VectorCopy(ent->mins, pm.mins);
	VectorCopy(ent->maxs, pm.maxs);

	// Setup speed up if we have hit the run shrine recently.
	pm.run_shrine = (client->playerinfo.speed_timer > level.time);

	// Setup speed up if we have been hit recently.
	pm.high_max = (client->playerinfo.effects & EF_HIGH_MAX);

	// Perform a Pmove().
	gi.Pmove(&pm, true);

	if (ent->waterlevel > 0)
		client->playerinfo.flags |= FL_INWATER;
	else
		client->playerinfo.flags &= ~FL_INWATER;

	client->playerinfo.flags &= ~(PLAYER_FLAG_COLLISION | PLAYER_FLAG_SLIDE);

	if (pm.s.c_flags & PC_COLLISION)
		client->playerinfo.flags |= PLAYER_FLAG_COLLISION;

	if (pm.s.c_flags & PC_SLIDING)
	{
		client->playerinfo.flags |= PLAYER_FLAG_SLIDE;

		if (Vec3NotZero(pm.GroundPlane.normal))
		{
			vec3_t ang;
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

	client->playerinfo.GroundSurface = pm.GroundSurface;
	memcpy(&client->playerinfo.GroundPlane, &pm.GroundPlane, sizeof(cplane_t));
	client->playerinfo.GroundContents = pm.GroundContents;

	// If we're move-locked, don't update the edict's origin and velocity, otherwise copy the origin and velocity
	// from playerinfo (which have been written by Pmove()) into the edict's origin and velocity.
	if (client->ps.pmove.pm_flags & PMF_LOCKMOVE)
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

	for (int i = 0; i < 3; i++)
		client->resp.cmd_angles[i] = SHORT2ANGLE(ucmd->angles[i]);

	client->playerinfo.waterlevel = pm.waterlevel;
	client->playerinfo.waterheight = pm.waterheight;
	client->playerinfo.watertype = pm.watertype;

	ent->waterlevel = pm.waterlevel;
	ent->viewheight = (int)pm.viewheight;
	ent->watertype = pm.watertype;
	ent->groundentity = pm.groundentity;

	if (pm.groundentity != NULL)
		ent->groundentity_linkcount = pm.groundentity->linkcount;

	if (ent->dead_state == DEAD_NO)
	{
		VectorCopy(pm.viewangles, client->v_angle);

		for (int i = 0; i < 3; i++)
			client->aimangles[i] = SHORT2ANGLE(ucmd->aimangles[i]);

		VectorCopy(client->aimangles, client->ps.viewangles);
	}

	gi.linkentity(ent);

	// Process touch triggers that the client could activate.
	if (ent->movetype != PHYSICSTYPE_NOCLIP)
		G_TouchTriggers(ent);

	// Touch other objects.
	for (int i = 0; i < pm.numtouch; i++)
	{
		edict_t* other = pm.touchents[i];

		if (other->touch == NULL)
			continue;

		int j;
		for (j = 0; j < i; j++)
			if (pm.touchents[j] == other)
				break;

		if (j == i) // Not duplicated.
			other->touch(other, ent, NULL, NULL);
	}

	client->playerinfo.oldbuttons = client->playerinfo.buttons;
	client->playerinfo.buttons = ucmd->buttons;
	client->playerinfo.latched_buttons |= client->playerinfo.buttons & ~client->playerinfo.oldbuttons;
	client->playerinfo.remember_buttons |= client->playerinfo.buttons;

	// Save the light level that the player is standing on for monster sighting AI.
	ent->light_level = ucmd->lightlevel;

	// Handle auto-aiming. //TODO: if have autotarget entity, try re-targeting it first, then look for new one?
	qboolean have_autotarget = (client->ps.AutotargetEntityNum > 0); //mxd
	ent->enemy = NULL;
	client->ps.AutotargetEntityNum = 0;

	if (client->playerinfo.autoaim)
	{
		// Get the origin of the LOS (from player to target) used in identifying potential targets.
		vec3_t los_origin;
		VectorCopy(ent->s.origin, los_origin);
		los_origin[2] += (float)ent->viewheight;

		//mxd. Use increased FOVs when we already have autotarget. Fixes frequent switching between auto-targeting and not when enemy is moving near the edge of FOVs.
		const float h_fov = (have_autotarget ? ANGLE_60 : ANGLE_35);
		const float v_fov = (have_autotarget ? ANGLE_40 : ANGLE_30);

		// Autoaiming is active so look for an enemy to auto-target. //mxd. far_dist:500, h_fov:35, v_fov:160 in original logic.
		edict_t* target_ent = FindNearestVisibleActorInFrustum(ent, client->aimangles, 0.0f, 1024.0f, h_fov, v_fov, true, los_origin);

		if (target_ent != NULL)
		{
			// An enemy was successfully auto-targeted, so store away the pointer to our enemy.
			ent->enemy = target_ent;
			client->ps.AutotargetEntityNum = ent->enemy->s.number;
		}
	}

	CalculatePIV(ent);
}

// This will be called once for each server frame, before running any other entities in the world.
void ClientBeginServerFrame(edict_t* ent)
{
	if (level.intermissiontime > 0.0f)
		return;

	gclient_t* client = ent->client;

	if (ent->dead_state != DEAD_DEAD) //mxd. inverted check. 'ent->deadflag & DEAD_DEAD' in original version. Deadflags are never OR'ed anywhere.
	{
		client->playerinfo.latched_buttons = 0;
		return;
	}

	// Wait for any button just going down.
	if (level.time > client->respawn_time)
	{
		// In deathmatch, only wait for attack button.
		const int button_mask = (DEATHMATCH ? BUTTON_ATTACK : -1);

		if ((client->playerinfo.latched_buttons & button_mask) || (DEATHMATCH && (DMFLAGS & DF_FORCE_RESPAWN)))
		{
			ClientRespawn(ent);
			client->playerinfo.latched_buttons = 0;
		}
	}
}