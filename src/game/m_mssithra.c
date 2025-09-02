//
// m_mssithra.c
//
// Copyright 1998 Raven Software
//

#include "m_mssithra.h"
#include "m_mssithra_shared.h"
#include "m_mssithra_anim.h"
#include "g_DefaultMessageHandler.h"
#include "g_monster.h"
#include "g_playstats.h"
#include "m_stats.h"
#include "mg_guide.h" //mxd
#include "Random.h"
#include "Utilities.h"
#include "Vector.h"
#include "Player.h" //mxd
#include "g_local.h"

#pragma region ========================== Mutant Ssithra base info ==========================

static const animmove_t* animations[NUM_ANIMS] =
{
	&mssithra_move_claw1,
	&mssithra_move_death1,
	&mssithra_move_idle1,
	&mssithra_move_jump1,
	&mssithra_move_fjump,
	&mssithra_move_roar,
	&mssithra_move_shoota1,
	&mssithra_move_shootb1,
	&mssithra_move_walk1,
	&mssithra_move_backpedal1,
	&mssithra_move_run1,
	&mssithra_move_delay,
	&mssithra_move_shoot1_trans,
	&mssithra_move_shoot1_loop,
	&mssithra_move_shoot1_detrans
};

static int sounds[NUM_SOUNDS];

#pragma endregion

#pragma region ========================== Utility functions =========================

// Create the guts of the ssithra arrow.
static void MssithraArrowInit(edict_t* arrow) //mxd. Named 'create_ssithra_arrow' in original logic.
{
	arrow->s.modelindex = (byte)gi.modelindex("models/objects/exarrow/tris.fm");

	arrow->movetype = MOVETYPE_FLYMISSILE;
	arrow->solid = SOLID_BBOX;
	arrow->classname = "mssithra_Arrow";
	arrow->clipmask = MASK_SHOT;
	arrow->s.effects |= (EF_ALWAYS_ADD_EFFECTS | EF_CAMERA_NO_CLIP);
	arrow->s.scale = 1.5f;
	arrow->svflags |= SVF_ALWAYS_SEND;

	arrow->touch = MssithraArrowTouch;

	VectorSet(arrow->mins, -1.0f, -1.0f, -1.0f);
	VectorSet(arrow->maxs,  1.0f,  1.0f,  1.0f);
}

edict_t* MssithraArrowReflect(edict_t* self, edict_t* other, vec3_t vel) //mxd. Named 'MssithraAlphaArrowReflect' in original logic.
{
	edict_t* arrow = G_Spawn();

	MssithraArrowInit(arrow);
	VectorCopy(self->s.origin, arrow->s.origin);
	arrow->owner = self->owner;

	arrow->think = G_FreeEdict;
	arrow->nextthink = self->nextthink;

	VectorCopy(vel, arrow->velocity);
	VectorNormalize2(vel, arrow->movedir);
	AnglesFromDir(arrow->movedir, arrow->s.angles);
	arrow->s.angles[YAW] += 90.0f;

	arrow->reflect_debounce_time = self->reflect_debounce_time - 1;
	arrow->reflected_time = self->reflected_time;

	G_LinkMissile(arrow);
	G_SetToFree(self);

	gi.CreateEffect(&arrow->s, FX_M_EFFECTS, CEF_OWNERS_ORIGIN | CEF_FLAG6, NULL, "bv", FX_MSSITHRA_ARROW, arrow->velocity);
	gi.CreateEffect(&arrow->s, FX_LIGHTNING_HIT, CEF_OWNERS_ORIGIN, NULL, "t", vel);

	return arrow;
}

static qboolean MssithraCheckMood(edict_t* self) //mxd. Named 'mssithraCheckMood' in original logic.
{
	if (self->monsterinfo.aiflags & AI_OVERRIDE_GUIDE)
		return false;

	self->mood_think(self);

	switch (self->ai_mood)
	{
		case AI_MOOD_ATTACK:
			QPostMessage(self, ((self->ai_mood_flags & AI_MOOD_FLAG_MISSILE) ? MSG_MISSILE : MSG_MELEE), PRI_DIRECTIVE, NULL);
			return true;

		case AI_MOOD_PURSUE:
		case AI_MOOD_NAVIGATE:
			QPostMessage(self, MSG_RUN, PRI_DIRECTIVE, NULL);
			return true;

		case AI_MOOD_WALK:
			QPostMessage(self, MSG_WALK, PRI_DIRECTIVE, NULL);
			return true;

		case AI_MOOD_STAND:
			QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
			return true;
	}

	return false;
}

#pragma endregion

#pragma region ========================== Message handlers ==========================

static void MssithraStandMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'mssithra_stand' in original logic.
{
	SetAnim(self, ANIM_IDLE1);
}

static void MssithraPainMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'mssithra_pain' in original logic.
{
	//FIXME: make part fly dir the vector from hit loc to sever loc.
	if (self->dead_state == DEAD_DEAD) // Dead but still being hit.
		return;

	int temp;
	int damage;
	qboolean force_pain;
	ParseMsgParms(msg, "eeiii", &temp, &temp, &force_pain, &damage, &temp);

	if (!force_pain || irand(0, 10) < 5 || self->groundentity == NULL || self->pain_debounce_time > level.time || self->curAnimID == ANIM_CLAW1)
		return;

	self->pain_debounce_time = level.time + 2.0f;
	gi.sound(self, CHAN_VOICE, sounds[irand(SND_PAIN1, SND_PAIN2)], 1.0f, ATTN_NORM, 0.0f);

	if (irand(0, 10) == 0)
		SetAnim(self, ANIM_ROAR1); // Make him tougher? More aggressive?
}

static void MssithraDeathMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'mssithra_death' in original logic.
{
	self->svflags |= SVF_DEADMONSTER;
	self->msgHandler = DyingMsgHandler;

	if (self->dead_state == DEAD_DEAD)
		return;

	self->dead_state = DEAD_DEAD;

	gi.sound(self, CHAN_VOICE, sounds[SND_DIE], 1.0f, ATTN_NORM, 0.0f);

	SetAnim(self, ANIM_DEATH1);

	// Remove the life bar once dead.
	M_ShowLifeMeter(0, 0);
	self->post_think = NULL;
	self->next_post_think = -1.0f;
}

static void MssithraMeleeMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'mssithra_melee' in original logic.
{
	if (!M_ValidTarget(self, self->enemy))
	{
		QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
		return;
	}

	vec3_t diff;
	VectorSubtract(self->s.origin, self->enemy->s.origin, diff);
	const float dist = VectorLength(diff);
	const float melee_range = 64.0f;

	const float min_seperation = self->maxs[0] + self->enemy->maxs[0];

	if (dist < min_seperation + melee_range) // A hit.
		SetAnim(self, ANIM_CLAW1);
}

//mxd. Also used as MSG_RUN handler.
static void MssithraMissileMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'mssithra_missile' in original logic.
{
	//NEWSTUFF: jump closer to claw, loop shooting anims.
	if (!M_ValidTarget(self, self->enemy))
	{
		QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
		return;
	}

	if (irand(0, (SKILL + 1) * 2) > 0)
		SetAnim(self, ANIM_SHOOT_TRANS);
	else
		SetAnim(self, ANIM_IDLE1);
}

#pragma endregion

#pragma region ========================== Edict callbacks ===========================

static void MssithraArrowExplodeThink(edict_t* self) //mxd. Named 'mssithra_missile_explode' in original logic.
{
	gi.CreateEffect(NULL, FX_M_EFFECTS, 0, self->s.origin, "bv", FX_MSSITHRA_EXPLODE, self->movedir);

	const float damage = flrand(8.0f, 16.0f); //mxd. int / irand() in original logic.
	T_DamageRadius(self, self->owner, self->owner, MSSITHRA_ARROW_DMG_RADIUS, damage, damage / 2.0f, DAMAGE_ATTACKER_IMMUNE, MOD_DIED);

	G_FreeEdict(self);
}

static void MssithraArrowTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surface) //mxd. Named 'mssithraAlphaArrowTouch' in original logic.
{
	// Are we reflecting?
	if (self->reflect_debounce_time > 0 && EntReflecting(other, true, true))
	{
		Create_rand_relect_vect(self->velocity, self->velocity);
		Vec3ScaleAssign(MSSITHRA_ARROW_SPEED / 2.0f, self->velocity);
		MssithraArrowReflect(self, other, self->velocity);

		return;
	}

	if (plane != NULL) //mxd. Original logic null-checks plane->normal (always true).
		VectorCopy(plane->normal, self->movedir);

	if (other->takedamage != DAMAGE_NO)
	{
		self->dmg = irand(MSSITHRA_DMG_MIN * 2, MSSITHRA_DMG_MAX * 2);
		MssithraArrowExplodeThink(self);
	}
	else
	{
		gi.sound(self, CHAN_WEAPON, sounds[SND_INWALL], 0.5f, ATTN_NORM, 0.0f);

		VectorClear(self->velocity);
		self->s.effects |= EF_ALTCLIENTFX;
		self->dmg = irand(MSSITHRA_DMG_MIN, MSSITHRA_DMG_MAX);

		self->think = MssithraArrowExplodeThink;
		self->nextthink = level.time + flrand(0.5f, 1.5f);
	}
}

static void MssithraPostThink(edict_t* self) //mxd. Named 'mssithra_postthink' in original logic.
{
	// Only display a life-meter if we have an enemy.
	if (self->enemy != NULL)
	{
		if (self->mssithra_healthbar_buildup < self->max_health)
		{
			M_ShowLifeMeter(self->mssithra_healthbar_buildup, self->mssithra_healthbar_buildup);
			self->mssithra_healthbar_buildup += self->max_health / 10; //mxd. '+= 50' in original logic (way too slow).
		}
		else
		{
			M_ShowLifeMeter(self->health, self->max_health);
		}
	}

	self->next_post_think = level.time + 0.05f;
}

#pragma endregion

#pragma region ========================== Action functions ==========================

void mssithra_dead(edict_t* self)
{
	//FIXME: maybe allow dead bodies to be chopped? Make BBOX small?
	self->msgHandler = DeadMsgHandler;
	self->dead_state = DEAD_DEAD;
	self->flags |= FL_DONTANIMATE;

	self->think = NULL;
	self->nextthink = THINK_NEVER; //mxd. '0' in original logic. Changed for consistency sake.

	gi.linkentity(self);
}

void mssithra_swipe(edict_t* self) //mxd. Named 'mssithraSwipe' in original logic.
{
	if (self->enemy == NULL) // If the player gets gibbed, enemy can be NULL.
		return;

	vec3_t diff;
	VectorSubtract(self->s.origin, self->enemy->s.origin, diff);
	const float dist = VectorLength(diff);

	const float min_seperation = self->maxs[0] + self->enemy->maxs[0] + 45.0f;

	if (dist < min_seperation && AI_IsInfrontOf(self, self->enemy)) // A hit.
	{
		gi.sound(self, CHAN_WEAPON, sounds[SND_SWIPEHIT], 1.0f, ATTN_NORM, 0.0f);

		const vec3_t offset = { 35.0f, 0.0f, 32.0f };
		vec3_t origin;
		VectorGetOffsetOrigin(offset, self->s.origin, self->s.angles[YAW], origin);

		vec3_t angles;
		VectorCopy(self->s.angles, angles);
		angles[YAW] += DEGREE_90;

		vec3_t forward;
		AngleVectors(angles, forward, NULL, NULL);

		T_Damage(self->enemy, self, self, forward, origin, vec3_origin, MSSITHRA_DMG_SWIPE, 0, DAMAGE_DISMEMBER, MOD_DIED);

		// Knockdown player?
		if (self->enemy->health > 0 && irand(0, 5) == 0 && Q_stricmp(self->enemy->classname, "player") == 0) // Else don't gib? //TODO: check self->enemy->client instead?
			P_KnockDownPlayer(&self->enemy->client->playerinfo);
	}
	else
	{
		gi.sound(self, CHAN_WEAPON, sounds[SND_SWIPE], 1.0f, ATTN_NORM, 0.0f);
	}
}

void mssithra_arrow(edict_t* self) //mxd. Named 'mssithraArrow' in original logic.
{
	//FIXME: adjust for up/down.
	if (self->enemy == NULL)
		return;

	if (self->enemy->health <= 0)
	{
		self->enemy = NULL;

		if (!MssithraCheckMood(self)) //mxd. Inlined mssithra_decide_stand().
			SetAnim(self, ANIM_IDLE1);

		return;
	}

	if (self->monsterinfo.attack_finished > level.time)
		return;

	gi.sound(self, CHAN_WEAPON, sounds[SND_ARROW], 1.0f, ATTN_NORM, 0.0f);
	self->monsterinfo.attack_finished = level.time + 0.4f;

	vec3_t firing_pos;
	VectorCopy(self->s.origin, firing_pos);
	firing_pos[2] += self->maxs[2] * 0.5f;

	vec3_t forward;
	vec3_t right;
	AngleVectors(self->s.angles, forward, right, NULL);

	VectorMA(firing_pos, 72.0f, forward, firing_pos);
	VectorMA(firing_pos, 16.0f, right, firing_pos);
	firing_pos[2] += 24.0f;

	vec3_t target_pos;
	VectorRandomCopy(self->enemy->s.origin, target_pos, 16.0f);

	vec3_t diff;
	VectorSubtract(target_pos, firing_pos, diff);

	vec3_t fire_dir;
	VectorNormalize2(diff, fire_dir);

	// Increase the spread for lower skill levels.
	float spread;

	switch (SKILL)
	{
		case (int)SKILL_EASY:
			spread = 0.35f;
			break;

		case (int)SKILL_MEDIUM:
			spread = 0.2f;
			break;

		case (int)SKILL_HARD:
		default:
			spread = 0.1f;
			break;
	}

	for (int i = 0; i < 3; i++) //TODO: spawn 5 arrows on Hard/Hard+?
	{
		edict_t* arrow = G_Spawn();

		MssithraArrowInit(arrow);
		arrow->reflect_debounce_time = MAX_REFLECT;

		VectorCopy(firing_pos, arrow->s.origin);
		VectorCopy(self->movedir, arrow->movedir);

		AngleVectors(self->s.angles, NULL, right, NULL);

		switch (i)
		{
			case 2:
				VectorScale(right, spread, right);
				break;

			case 0:
				VectorScale(right, -spread, right);
				break;

			case 1:
			default:
				VectorClear(right);
				break;
		}

		VectorAdd(fire_dir, right, arrow->movedir);
		VectorNormalize(arrow->movedir);
		VectorScale(arrow->movedir, MSSITHRA_ARROW_SPEED, arrow->velocity);

		vectoangles(arrow->velocity, arrow->s.angles);
		arrow->s.angles[YAW] += 90.0f;

		arrow->owner = self;
		arrow->enemy = self->enemy;

		arrow->nextthink = level.time + 5.0f;
		arrow->think = G_FreeEdict;

		G_LinkMissile(arrow);

		gi.CreateEffect(&arrow->s, FX_M_EFFECTS, CEF_OWNERS_ORIGIN | CEF_FLAG6, NULL, "bv", FX_MSSITHRA_ARROW, arrow->velocity);
	}
}

void mssithra_check_mood(edict_t* self) //mxd. Added action function version.
{
	MssithraCheckMood(self);
}

void mssithra_growl(edict_t* self) //mxd. Named 'mmssithraRandomGrowlSound' in original logic.
{
	gi.sound(self, CHAN_VOICE, sounds[irand(SND_GROWL1, SND_GROWL3)], 1.0f, ATTN_NORM, 0.0f);
}

void mssithra_shoot_loop(edict_t* self) //mxd. Named 'mssithra_ShotLoop' in original logic.
{
	SetAnim(self, ANIM_SHOOT_LOOP);
}

void mssithra_check_shoot_loop(edict_t* self) //mxd. Named 'mssithraCheckShotLoop' in original logic.
{
	//FIXME: Check to keep shooting or to stop shooting.
	if (irand(0, (SKILL + 1) * 2) == 0) // Otherwise, just keep playing current animation.
		SetAnim(self, ANIM_SHOOT_DETRANS); //TODO: never do this when health < 50% (or 33%)?
}

#pragma endregion

void MssithraStaticsInit(void)
{
	static ClassResourceInfo_t res_info; //mxd. Made local static.

	classStatics[CID_MSSITHRA].msgReceivers[MSG_STAND] = MssithraStandMsgHandler;
	classStatics[CID_MSSITHRA].msgReceivers[MSG_MISSILE] = MssithraMissileMsgHandler;
	classStatics[CID_MSSITHRA].msgReceivers[MSG_MELEE] = MssithraMeleeMsgHandler;
	classStatics[CID_MSSITHRA].msgReceivers[MSG_DEATH] = MssithraDeathMsgHandler;
	classStatics[CID_MSSITHRA].msgReceivers[MSG_PAIN] = MssithraPainMsgHandler;
	classStatics[CID_MSSITHRA].msgReceivers[MSG_RUN] = MssithraMissileMsgHandler;

	res_info.numAnims = NUM_ANIMS;
	res_info.animations = animations;
	res_info.modelIndex = gi.modelindex("models/monsters/mutantsithra/tris.fm");

	sounds[SND_PAIN1] = gi.soundindex("monsters/mssithra/pain1.wav");
	sounds[SND_PAIN2] = gi.soundindex("monsters/mssithra/pain2.wav");
	sounds[SND_DIE] = gi.soundindex("monsters/mssithra/death1.wav");
	sounds[SND_SWIPE] = gi.soundindex("monsters/mssithra/swipe.wav");
	sounds[SND_SWIPEHIT] = gi.soundindex("monsters/mssithra/swipehit.wav");
	sounds[SND_ARROW] = gi.soundindex("weapons/RedRainPowerFire.wav");
	sounds[SND_AEXPLODE] = gi.soundindex("weapons/FlyingFistImpact.wav");
	sounds[SND_GROWL1] = gi.soundindex("monsters/mssithra/growl1.wav");
	sounds[SND_GROWL2] = gi.soundindex("monsters/mssithra/growl2.wav");
	sounds[SND_GROWL3] = gi.soundindex("monsters/mssithra/growl3.wav");
	sounds[SND_ROAR] = gi.soundindex("monsters/mssithra/roar.wav");
	sounds[SND_INWALL] = gi.soundindex("weapons/staffhitwall.wav");

	res_info.numSounds = NUM_SOUNDS;
	res_info.sounds = sounds;

	classStatics[CID_MSSITHRA].resInfo = &res_info;
}

// QUAKED monster_mssithra (1 .5 0) (-36 -36 0) (36 36 96) AMBUSH ASLEEP 4 8 16 32 64 FIXED WANDER MELEE_LEAD STALK COWARD EXTRA1 EXTRA2 EXTRA3 EXTRA4
// Mutant Ssithra.

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
// mintel					- Monster intelligence - this basically tells a monster how many buoys away an enemy has to be for it to give up (default 16).
// melee_range				- How close the player has to be for the monster to go into melee. If this is zero, the monster will never melee.
//							  If it is negative, the monster will try to keep this distance from the player.
//							  If the monster has a backup, he'll use it if too close, otherwise, a negative value here means the monster will just stop
//							  running at the player at this distance (default 100).
//							 Examples:
//								melee_range = 60 - monster will start swinging it player is closer than 60.
//								melee_range = 0 - monster will never do a melee attack.
//								melee_range = -100 - monster will never do a melee attack and will back away (if it has that ability) when player gets too close.
// missile_range			- Maximum distance the player can be from the monster to be allowed to use it's ranged attack (default 400).
// min_missile_range		- Minimum distance the player can be from the monster to be allowed to use it's ranged attack (default 100).
// bypass_missile_chance	- Chance that a monster will NOT fire it's ranged attack, even when it has a clear shot. This, in effect, will make the monster
//							  come in more often than hang back and fire. A percentage (0 = always fire/never close in, 100 = never fire/always close in) - must be whole number (default 25).
// jump_chance				- Every time the monster has the opportunity to jump, what is the chance (out of 100) that he will... (100 = jump every time) - must be whole number (default 25).
// wakeup_distance			- How far (max) the player can be away from the monster before it wakes up. This means that if the monster can see the player,
//							  at what distance should the monster actually notice him and go for him (default 1024).
// NOTE: A value of zero will result in defaults, if you actually want zero as the value, use -1.
void SP_monster_mssithra(edict_t* self)
{
	// Generic Monster Initialization.
	if (!M_WalkmonsterStart(self)) // Failed initialization.
		return;

	self->msgHandler = DefaultMsgHandler;
	self->materialtype = MAT_FLESH;
	self->flags |= FL_IMMUNE_SLIME;

	if (self->health <= 0)
		self->health = MSSITHRA_HEALTH;

	// Apply to the end result (whether designer set or not).
	self->health = MonsterHealth(self->health);
	self->max_health = self->health;

	self->mass = MSSITHRA_MASS;
	self->yaw_speed = 20.0f;
	self->viewheight = 88;
	self->monsterinfo.aiflags |= (AI_BRUTAL | AI_AGRESSIVE);
	self->movetype = PHYSICSTYPE_STEP;
	self->solid = SOLID_BBOX;

	VectorClear(self->knockbackvel);

	VectorCopy(STDMinsForClass[self->classID], self->mins);
	VectorCopy(STDMaxsForClass[self->classID], self->maxs);

	self->monsterinfo.otherenemyname = "obj_barrel";
	self->s.modelindex = (byte)classStatics[CID_MSSITHRA].resInfo->modelIndex;
	self->s.skinnum = 0;

	self->mssithra_healthbar_buildup = 0;
	self->svflags |= SVF_BOSS;

	if (self->monsterinfo.scale == 0.0f)
	{
		self->monsterinfo.scale = MODEL_SCALE + 0.25f;
		self->s.scale = self->monsterinfo.scale;
	}

	self->post_think = MssithraPostThink;
	self->next_post_think = level.time + FRAMETIME; //mxd. Use define.

	// Setup my mood function.
	MG_InitMoods(self);
	QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);

	// Turn the goofy bolts off!
	self->s.fmnodeinfo[MESH__BOLTS].flags |= FMNI_NO_DRAW; //TODO: enable during firing animation?
}