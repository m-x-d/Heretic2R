//
// fx_Shrine.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Particle.h"
#include "Random.h"
#include "Utilities.h"
#include "Vector.h"
#include "g_playstats.h"

#define MANA_RAD						100.0f
#define MANA_HEIGHT						32.0f
#define ARMOR_RAD						35.0f
#define LUNG_RAD						20.0f
#define STAFF_RADIUS					20.0f

#define NUM_OF_MANA_PARTS				16
#define NUM_OF_ARMOR_PARTS				30
#define NUM_OF_STAFF_PARTS				6
#define NUM_OF_LUNG_PARTS				15

#define TOTAL_STAFF_EFFECTS				30
#define STAFF_EFFECTS_HEIGHT			70
#define STAFF_EFFECTS_START_HEIGHT		(-30)
#define STAFF_HEIGHT_ADD				((STAFF_EFFECTS_HEIGHT - STAFF_EFFECTS_START_HEIGHT) / TOTAL_STAFF_EFFECTS)
#define STAFF_GREEN_ADD					(-(255 / (TOTAL_STAFF_EFFECTS - 8)))
#define STAFF_BLUE_ADD					(STAFF_GREEN_ADD * 2)

#define LIGHTNING_START					20
#define LIGHTNING_MINIMUM				0.15f
#define LIGHTNING_MAXIMUM				0.3f
#define LIGHTNING_SPLIT_RAD				1.3f

#define TOTAL_HEALTH_EFFECTS			8

#define BALL_RAD						40
#define BALL_PART_SCALE					10.0f
#define BALL_PART_NUM					4
#define BALL_EX_PART_NUM				20

#define LIGHTNING_DUR					100
#define FLIGHT_RAD						20
#define TOTAL_FLIGHT_EFFECTS			60
#define FLIGHT_EFFECTS_HEIGHT			50
#define FLIGHT_EFFECTS_START_HEIGHT		(-35)
#define FLIGHT_HEIGHT_ADD				((FLIGHT_EFFECTS_HEIGHT - FLIGHT_EFFECTS_START_HEIGHT) / (TOTAL_FLIGHT_EFFECTS - 24))
#define NUM_OF_FLIGHT_PARTS				3
#define FLIGHT_PLAY_RAD					12

#define NUM_OF_REFLECT_PARTS			20
#define TOTAL_REFLECT_EFFECTS			50
#define REFLECT_RAD						31
#define REFLECT_EFFECTS_START_HEIGHT	(-35)

#define NUM_OF_POWERUP_PARTS			22
#define TOTAL_POWERUP_EFFECTS			38
#define POWERUP_EFFECTS_HEIGHT			60
#define POWERUP_EFFECTS_START_HEIGHT	(-30)
#define POWERUP_RAD						6
#define POWERUP_HEIGHT_ADD				((POWERUP_EFFECTS_HEIGHT - POWERUP_EFFECTS_START_HEIGHT) / (TOTAL_POWERUP_EFFECTS - 24))

static struct model_s* shrine_models[2];

void PreCacheShrine(void)
{
	shrine_models[0] = fxi.RegisterModel("sprites/lens/halo1.sp2");
	shrine_models[1] = fxi.RegisterModel("sprites/fx/halo.sp2");
}

// No longer used - causes really hard to find crashes.
void FXShrinePlayerEffect(centity_t* owner, int type, int flags, vec3_t origin) { } //TODO: remove?

#pragma region ========================== MANA EFFECT ROUTINES ==========================

static qboolean ShrineManaThink(struct client_entity_s* self, centity_t* owner)
{
	if (--self->SpawnInfo == 0)
		return false;

	if (self->SpawnInfo < 5)
		return true;

	const int count = GetScaledCount(NUM_OF_MANA_PARTS, 0.7f);

	for (int i = 0; i < count; i++)
	{
		// Calc spherical offset around left hand ref point.
		vec3_t vel;
		VectorRandomSet(vel, 1.0f);

		if (Vec3IsZero(vel))
			vel[2] = 1.0f; // Safety in case flrand gens all zeros (VERY unlikely). //BUGFIX: mxd. Original version sets unused vector instead.

		VectorNormalize(vel);
		VectorScale(vel, MANA_RAD, vel);

		const int particle_type = ((i & 1) ? PART_16x16_SPARK_G : PART_16x16_SPARK_B); //mxd. Green or blue particles.
		client_particle_t* ce = ClientParticle_new(particle_type, color_white, 500);

		ce->scale = 20.0f;
		ce->d_scale = -30.0f;
		ce->color.a = 1;
		ce->d_alpha = 400.0f;
		VectorSet(ce->origin, vel[0], vel[1], vel[2] + MANA_HEIGHT);
		VectorScale(vel, -0.125f, ce->velocity);
		VectorScale(vel, -8.0f, ce->acceleration);

		AddParticleToList(self, ce);
	}

	return true;
}

// Make the mana effect go off.
void FXShrineManaEffect(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	client_entity_t* fx_spawner = ClientEntity_new(type, (int)(flags | CEF_NO_DRAW | CEF_ADDITIVE_PARTS), origin, NULL, 100);

	VectorClear(fx_spawner->origin);
	fx_spawner->SpawnInfo = 17;
	fx_spawner->AddToView = LinkedEntityUpdatePlacement;
	fx_spawner->Update = ShrineManaThink;

	AddEffect(owner, fx_spawner);
}

#pragma endregion

#pragma region ========================== ARMOR EFFECT ROUTINES ==========================

static qboolean ShrineArmorThink(struct client_entity_s* self, centity_t* owner)
{
	if (--self->SpawnInfo == 0)
		return false;

	if (self->SpawnInfo < 5)
		return true;

	// Which type of particle do we use? Gold or Silver armor?
	const int particle_type = ((self->flags & CEF_FLAG6) ? PART_16x16_ORANGE_PUFF : PART_16x16_BLUE_PUFF); //mxd
	const int count = GetScaledCount(NUM_OF_ARMOR_PARTS, 0.7f);

	for (int i = 0; i < count; i++)
	{
		// Calc spherical offset around left hand ref point.
		vec3_t vel;
		VectorRandomSet(vel, 1.0f);

		if (Vec3IsZero(vel))
			vel[2] = 1.0f; // Safety in case flrand gens all zeros (VERY unlikely). //BUGFIX: mxd. Original version sets unused vector instead.

		VectorNormalize(vel);
		VectorScale(vel, ARMOR_RAD, vel);

		client_particle_t* ce = ClientParticle_new(particle_type, color_white, 500);

		ce->scale = 30.0f;
		ce->d_scale = -60.0f;
		ce->color.a = 1;
		ce->d_alpha = 400.0f;
		ce->acceleration[2] = 0.0f;
		VectorCopy(vel, ce->origin);

		AddParticleToList(self, ce);
	}

	return true;
}

void FXShrineArmorEffect(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	client_entity_t* glow = ClientEntity_new(type, (int)(flags | CEF_NO_DRAW | CEF_ADDITIVE_PARTS), origin, NULL, 100);

	VectorClear(glow->origin);
	glow->SpawnInfo = 18;
	glow->AddToView = LinkedEntityUpdatePlacement;
	glow->Update = ShrineArmorThink;

	AddEffect(owner, glow);
}

#pragma endregion

#pragma region ========================== LUNGS EFFECT ROUTINES ==========================

static qboolean ShrineLungsThink(struct client_entity_s* self, centity_t* owner)
{
	if (--self->SpawnInfo == 0)
		return false;

	if (self->SpawnInfo < 11)
		return true;

	const paletteRGBA_t color = { .c = 0xffffff40 };
	const int count = GetScaledCount(NUM_OF_LUNG_PARTS, 0.7f);

	for (int i = 0; i < count; i++)
	{
		// Calc spherical offset around left hand ref point.
		vec3_t vel;
		VectorSet(vel, flrand(-1.0f, 1.0f), flrand(-1.0f, 1.0f), 0.01f);
		VectorNormalize(vel);
		VectorScale(vel, flrand(1.0f, LUNG_RAD), vel);
		vel[2] = -30.0f;

		client_particle_t* ce = ClientParticle_new(PART_32x32_STEAM, color, 450);

		VectorCopy(vel, ce->origin);
		ce->velocity[2] = flrand(10.0f, 40.0f);
		ce->acceleration[2] = flrand(300.0f, 1000.0f);
		ce->scale = 8.0f;

		AddParticleToList(self, ce);
	}

	return true;
}

void FXShrineLungsEffect(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	client_entity_t* glow = ClientEntity_new(type, (int)(flags | CEF_NO_DRAW), origin, NULL, 50);

	VectorClear(glow->origin);
	glow->SpawnInfo = 40;
	glow->AddToView = LinkedEntityUpdatePlacement;
	glow->Update = ShrineLungsThink;

	AddEffect(owner, glow);
}

#pragma endregion

#pragma region ========================== LIGHT EFFECT ROUTINES ==========================

// Create the light effect - a big old halo that fades away.
void FXShrineLightEffect(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	client_entity_t* glow = ClientEntity_new(type, flags, origin, NULL, 2000);

	VectorClear(glow->origin);
	glow->r.model = &shrine_models[0];
	glow->r.flags = RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	glow->d_scale = 5.0f;
	glow->r.scale = 0.1f;
	glow->d_alpha = -0.45f;
	glow->AddToView = LinkedEntityUpdatePlacement;

	AddEffect(owner, glow);
}

#pragma endregion

#pragma region ========================== STAFF POWERUP EFFECT ROUTINES ==========================

// Create the two circles that ring the player.
static qboolean ShrineStaffThink(struct client_entity_s* self, centity_t* owner)
{
	if (--self->SpawnInfo == 0)
		return false;

	const float radius = (float)(TOTAL_STAFF_EFFECTS - self->SpawnInfo) * (STAFF_RADIUS / (TOTAL_STAFF_EFFECTS - 8));

	if (self->SpawnInfo > 8)
	{
		// Figure out how many particles we are going to use.
		const int count = GetScaledCount(self->SpawnDelay, 0.7f);

		// Create rings of particles that goes up and down.
		const float angle_step = ANGLE_360 / (float)count;

		float cur_angle = 0.0f;
		while (cur_angle < ANGLE_360)
		{
			for (int c = 0; c < 2; c++)
			{
				const float dir_z = (c == 0 ? 1.0f : -1.0f); //mxd
				const vec3_t vel = { radius * cosf(cur_angle), radius * sinf(cur_angle), self->SpawnData * dir_z };

				const int particle_type = ((self->flags & CEF_FLAG6) ? irand(PART_16x16_FIRE1, PART_16x16_FIRE3) : PART_16x16_SPARK_B);
				client_particle_t* ce = ClientParticle_new(particle_type, self->r.color, self->LifeTime);

				VectorCopy(vel, ce->origin);
				ce->acceleration[2] = 0.0f;
				ce->color.a = 118;
				ce->scale = 16.0f;

				AddParticleToList(self, ce);
			}

			cur_angle += angle_step;
		}
	}

	// Update the particles colors.
	if (self->r.color.g > 0)
		self->r.color.g += (byte)STAFF_GREEN_ADD;

	if (self->r.color.b > 0)
		self->r.color.b += (byte)STAFF_BLUE_ADD;

	// Move the rings up/down next frame.
	self->SpawnData += STAFF_HEIGHT_ADD;

	return true;
}

// Start up the staff power up effect.
void FXShrineStaffEffect(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	int line_count;
	int particle_life;

	if (R_DETAIL >= DETAIL_HIGH) //TODO: separate case for DETAIL_UBERHIGH.
	{
		line_count = NUM_OF_STAFF_PARTS;
		particle_life = 2400;
	}
	else if (R_DETAIL == DETAIL_NORMAL)
	{
		line_count = NUM_OF_STAFF_PARTS - 2;
		particle_life = 1900;
	}
	else
	{
		line_count = NUM_OF_STAFF_PARTS - 3;
		particle_life = 1500;
	}

	client_entity_t* glow = ClientEntity_new(type, (int)(flags | CEF_NO_DRAW | CEF_ADDITIVE_PARTS), origin, NULL, 50);

	VectorClear(glow->origin);
	glow->SpawnInfo = TOTAL_STAFF_EFFECTS;
	glow->SpawnData = STAFF_EFFECTS_START_HEIGHT;
	glow->SpawnDelay = line_count;
	glow->LifeTime = particle_life;
	glow->AddToView = LinkedEntityUpdatePlacement;
	glow->Update = ShrineStaffThink;

	AddEffect(owner, glow);
}

#pragma endregion

#pragma region ========================== HEALTH LIGHTNING ROUTINES ==========================

// Recursively called to create the lightning effect.
static void LightningSplit(struct client_entity_s* self, vec3_t org, vec3_t dir, int rand_chance, const float stop_height)
{
	vec3_t cur_org;
	VectorCopy(org, cur_org);

	const qboolean small_split = (stop_height == 30.0f); //mxd

	// If we are the smaller split type, make our life longer, since we fade that much faster than the big particles.
	const int lightning_part_duration = (small_split ? (int)((float)self->updateTime / 1.2f) : self->updateTime);

	// Figure out what our scale and alpha should be based on whether its a small split or not.
	const float particle_scale = (small_split ? 1.2f : 0.5f); //mxd
	const byte particle_alpha = (small_split ? 255 : 108); //mxd

	// Create lightning particle.
	while (cur_org[2] > stop_height)
	{
		// Create the individual particle.
		client_particle_t* ce = ClientParticle_new(PART_16x16_LIGHTNING, color_white, lightning_part_duration); //TODO: mxd. Original logic used 0xffffff (white with 0 alpha). Intentional or not?

		ce->scale = particle_scale;
		ce->color.a = particle_alpha;
		ce->acceleration[2] = 0.0f;
		VectorCopy(cur_org, ce->origin);

		AddParticleToList(self, ce);

		// Decide if we are splitting.
		if (irand(0, rand_chance) == 0)
		{

			// Decide a new direction for the two halves.
			dir[1] = flrand(-LIGHTNING_SPLIT_RAD, LIGHTNING_SPLIT_RAD);
			dir[0] = flrand(-LIGHTNING_SPLIT_RAD, LIGHTNING_SPLIT_RAD);

			// Create the new split.
			vec3_t new_dir = { -dir[0], -dir[1], dir[2] };
			rand_chance += 5;

			// If we've split a fair few times already, don't again.
			if (rand_chance > 125)
				rand_chance = 100000;

			// Decide if this is a faint split or not.
			const float new_stop_height = (irand(0, 5) ? org[2] - 25.0f : stop_height);

			// Create split.
			LightningSplit(self, cur_org, new_dir, rand_chance, new_stop_height);
		}
		else // Update the direction we want to go in.
		{
			// Figure out values to add to dir.
			const vec3_t dir_effect = { flrand(-LIGHTNING_MINIMUM, LIGHTNING_MINIMUM), flrand(-LIGHTNING_MINIMUM, LIGHTNING_MINIMUM), 0.0f };

			// Add to direction.
			Vec3AddAssign(dir_effect, dir);

			// Cap XY direction.
			for (int i = 0; i < 2; i++)
			{
				if (dir[i] > LIGHTNING_MAXIMUM)
					dir[i] -= LIGHTNING_MINIMUM;
				else if (dir[i] < -LIGHTNING_MAXIMUM)
					dir[i] += LIGHTNING_MINIMUM;
			}
		}

		// Update position.
		Vec3AddAssign(dir, cur_org);
	}
}

// Create the lightning lines.
static void CreateLightning(struct client_entity_s* self, const centity_t* owner)
{
	// Create the lightning lines.
	const int lightning_count = (ref_soft ? 1 : irand(2, 3));
	const float angle_increment = ANGLE_360 / (float)lightning_count; //mxd

	float cur_angle = 0.0f;
	while (cur_angle < ANGLE_360)
	{
		// Setup the start of a lightning line.
		vec3_t org = { LIGHTNING_START * cosf(cur_angle), LIGHTNING_START * sinf(cur_angle), self->SpawnData };

		// Setup initial direction.
		vec3_t dir = { flrand(-LIGHTNING_MINIMUM, LIGHTNING_MINIMUM), flrand(-LIGHTNING_MINIMUM, LIGHTNING_MINIMUM), -0.5f };

		// Create initial line of lightning.
		LightningSplit(self, org, dir, 60, -30.0f);

		cur_angle += angle_increment;
	}

	self->SpawnData -= 5.0f;

	// If the owner of the lightning is the main client then flash the screen.
	if (owner->current.number == fxi.cl->playernum + 1)
	{
		// Give a quick screen flash.
		fxi.Activate_Screen_Flash((int)0x80ffd0c0);

		// Make our screen shake a bit.
		fxi.Activate_Screen_Shake(4.0f, 800.0f, (float)fxi.cl->time, SHAKE_ALL_DIR);
	}
}

// Make the lightning effect re-occur.
static qboolean ShrineHealthThink(struct client_entity_s* self, centity_t* owner)
{
	if (--self->SpawnInfo == 0)
		return false;

	// Create the lightning lines.
	CreateLightning(self, owner);

	self->updateTime = irand(1, TOTAL_HEALTH_EFFECTS - self->SpawnInfo) * 100;

	return true;
}

// Create initial lightning line.
void FXShrineHealthEffect(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	// Create the entity that the particles are attached to.
	client_entity_t* glow = ClientEntity_new(type, (int)(flags | CEF_NO_DRAW | CEF_ADDITIVE_PARTS), origin, NULL, LIGHTNING_DUR + 20);

	glow->SpawnInfo = TOTAL_HEALTH_EFFECTS;
	glow->SpawnData = 70;
	glow->AddToView = LinkedEntityUpdatePlacement;
	glow->Update = ShrineHealthThink;

	AddEffect(owner, glow);

	// Start off the lightning effect.
	CreateLightning(glow, owner);
}

#pragma endregion

#pragma region ========================== REFLECT EFFECT ROUTINES ==========================

// Create the two circles that ring the player.
static qboolean ShrineReflectThink(struct client_entity_s* self, centity_t* owner)
{
	if (--self->SpawnInfo == 0)
		return false;

	// Create the ring of particles that goes up.
	if (self->SpawnInfo > 24)
	{
		// Figure out how many particles we are going to use.
		const int count = GetScaledCount(NUM_OF_FLIGHT_PARTS, 0.7f);
		const float angle_offset = 6.28f / (float)count;

		// Create the ring of particles that goes up and down.
		float angle = self->Scale;
		for (int i = 0; i < count; i++)
		{
			angle += angle_offset;

			for (int c = 0; c < 2; c++)
			{
				const float dir_z = (c == 0 ? 1.0f : -1.0f); //mxd
				client_particle_t* ce = ClientParticle_new(PART_16x16_SPARK_B, color_white, 450);

				ce->acceleration[2] = 0.0f;
				VectorSet(ce->origin, FLIGHT_RAD * cosf(angle), FLIGHT_RAD * sinf(angle), self->SpawnData * dir_z);
				ce->scale = 16.0f;

				AddParticleToList(self, ce);
			}
		}

		// Put the sparkle on us.
		if (self->SpawnInfo > 10)
		{
			for (int i = 0; i < irand(3, 7); i++)
			{
				client_particle_t* ce = ClientParticle_new(PART_16x16_STAR, color_white, 280);

				ce->acceleration[2] = 0.0f;
				VectorSet(ce->origin, flrand(-FLIGHT_PLAY_RAD, FLIGHT_PLAY_RAD), flrand(-FLIGHT_PLAY_RAD, FLIGHT_PLAY_RAD), flrand(-30.0f, 30.0f));
				ce->scale = 0.3f;
				ce->d_scale = flrand(40.0f, 60.0f);

				AddParticleToList(self, ce);
			}
		}
	}

	// Move the rings up/down next frame.
	self->SpawnData += FLIGHT_HEIGHT_ADD;
	self->Scale += 0.15f;

	return true;
}

// Create the entity the flight loops are on.
void FXShrineReflectEffect(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	client_entity_t* glow = ClientEntity_new(type, (int)(flags | CEF_NO_DRAW | CEF_ADDITIVE_PARTS), origin, NULL, 25);

	VectorClear(glow->origin);
	glow->SpawnInfo = TOTAL_FLIGHT_EFFECTS;
	glow->SpawnData = FLIGHT_EFFECTS_START_HEIGHT;
	glow->AddToView = LinkedEntityUpdatePlacement;
	glow->Update = ShrineReflectThink;

	AddEffect(owner, glow);
}

#pragma endregion

#pragma region ========================== GHOSTING EFFECT ROUTINES ==========================

// Make the glow go away.
static qboolean ShrineGlowThink(struct client_entity_s* self, centity_t* owner)
{
	if (--self->SpawnInfo == 0)
		return false;

	self->d_alpha = -0.45f;
	self->d_scale = -1.0f;

	return true;
}

// Create the little glow bits.
static qboolean ShrineGhostThink(struct client_entity_s* self, centity_t* owner)
{
	if (--self->SpawnInfo == 0)
		return false;

	for (int i = 0; i < irand(3, 5); i++)
	{
		const vec3_t origin = { flrand(-20.0f, 20.0f), flrand(-20.0f, 20.0f), flrand(-30.0f, 40.0f) };
		client_entity_t* glow = ClientEntity_new(FX_SHRINE_GHOST, CEF_OWNERS_ORIGIN, origin, NULL, 300);

		glow->r.model = &shrine_models[0];
		glow->r.flags = RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		glow->SpawnInfo = 2;
		glow->d_scale = flrand(0.5f, 2.0f);
		glow->r.scale = 0.1f;
		glow->d_alpha = 1.0f;
		glow->alpha = 0.1f;
		glow->AddToView = OffsetLinkedEntityUpdatePlacement;
		glow->Update = ShrineGlowThink;

		AddEffect(owner, glow);
	}

	return true;
}

// Create the initial ghost controlling entity.
void FXShrineGhostEffect(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	client_entity_t* glow = ClientEntity_new(type, (int)(flags | CEF_NO_DRAW), origin, NULL, 70);

	glow->SpawnInfo = 20;
	glow->Update = ShrineGhostThink;

	AddEffect(owner, glow);
}

#pragma endregion

#pragma region ========================== SPEED EFFECT ROUTINES ==========================

// Create the two circles that ring the player.
static qboolean ShrineSpeedThink(struct client_entity_s* self, centity_t* owner)
{
	if (--self->SpawnInfo == 0)
		return false;

	// Figure out how many particles we are going to use.
	const int count = GetScaledCount(NUM_OF_REFLECT_PARTS, 0.7f);
	const float offset_angle = 6.28f / (float)count;
	float angle = 0.0f;

	for (int i = 0; i < count; i++)
	{

		client_particle_t* ce = ClientParticle_new(PART_32x32_STEAM, self->r.color, 380);

		const vec3_t angles = { angle, self->Scale, 0.0f };
		DirFromAngles(angles, ce->origin);
		Vec3ScaleAssign(REFLECT_RAD, ce->origin);
		ce->acceleration[2] = 0.0f;
		ce->scale = 7.0f;

		if (self->SpawnInfo < 18)
			ce->color.a = (byte)(self->SpawnInfo * (255 / 18));

		AddParticleToList(self, ce);

		angle += offset_angle;
	}

	self->Scale += 0.25f;

	// Put the sparkle on us.
	if (self->SpawnInfo > 10)
	{
		for (int i = 0; i < irand(3, 7); i++)
		{
			client_particle_t* ce = ClientParticle_new(PART_16x16_STAR, color_white, 280);

			ce->acceleration[2] = 0.0f;
			VectorSet(ce->origin, flrand(-FLIGHT_PLAY_RAD, FLIGHT_PLAY_RAD), flrand(-FLIGHT_PLAY_RAD, FLIGHT_PLAY_RAD), flrand(-30.0f, 30.0f));
			ce->scale = 0.3f;
			ce->d_scale = flrand(40.0f, 60.0f);

			AddParticleToList(self, ce);
		}
	}

	return true;
}

void FXShrineSpeedEffect(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	client_entity_t* glow = ClientEntity_new(type, (int)(flags | CEF_NO_DRAW), origin, NULL, 30);

	VectorClear(glow->origin);
	glow->SpawnInfo = TOTAL_REFLECT_EFFECTS;
	glow->SpawnData = REFLECT_EFFECTS_START_HEIGHT;
	glow->r.color.c = 0x604040ff;
	glow->AddToView = LinkedEntityUpdatePlacement;
	glow->Update = ShrineSpeedThink;

	AddEffect(owner, glow);
}

#pragma endregion

#pragma region ========================== WEAPONS POWER UP EFFECT ROUTINES ==========================

// Create the two circles that ring the player.
static qboolean ShrinePowerupThink(struct client_entity_s* self, centity_t* owner)
{
	if (--self->SpawnInfo == 0)
		return false;

	if (self->SpawnInfo > 24)
	{
		// Create the ring of particles that goes up.

		// Figure out how many particles we are going to use.
		const int count = GetScaledCount(NUM_OF_POWERUP_PARTS, 0.7f);
		const float offset_angle = 6.28f / (float)count;
		float angle = 0.0f;

		for (int i = 0; i < count; i++)
		{
			client_particle_t* ce = ClientParticle_new(PART_16x16_SPARK_G, color_white, 350);

			VectorSet(ce->origin, POWERUP_RAD * cosf(angle), POWERUP_RAD * sinf(angle), self->SpawnData);
			VectorScale(ce->origin, 25.0f, ce->velocity);
			ce->velocity[2] = 0.0f;
			ce->acceleration[2] = 0.0f;
			ce->scale = 12.0f;

			AddParticleToList(self, ce);

			angle += offset_angle;
		}

		// Put the sparkle on us.
		if (self->SpawnInfo > 10)
		{
			for (int i = 0; i < irand(3, 7); i++)
			{
				client_particle_t* ce = ClientParticle_new(PART_16x16_STAR, color_white, 280);

				ce->acceleration[2] = 0.0f;
				VectorSet(ce->origin, flrand(-FLIGHT_PLAY_RAD, FLIGHT_PLAY_RAD), flrand(-FLIGHT_PLAY_RAD, FLIGHT_PLAY_RAD), flrand(-30.0f, 30.0f));
				ce->scale = 0.3f;
				ce->d_scale = flrand(40.0f, 60.0f);

				AddParticleToList(self, ce);
			}
		}
	}

	// Move the rings up/down next frame.
	self->SpawnData += POWERUP_HEIGHT_ADD;

	return true;
}

void FXShrinePowerupEffect(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	client_entity_t* glow = ClientEntity_new(type, (int)(flags | CEF_NO_DRAW | CEF_ADDITIVE_PARTS), origin, NULL, 75);

	glow->SpawnInfo = TOTAL_POWERUP_EFFECTS;
	glow->SpawnData = POWERUP_EFFECTS_START_HEIGHT;
	glow->AddToView = LinkedEntityUpdatePlacement;
	glow->Update = ShrinePowerupThink;

	AddEffect(owner, glow);
}

#pragma endregion

#pragma region ========================== PERSISTENT SHRINE BALL EFFECT ==========================

enum
{
	SHRINEBALL_HEAL,
	SHRINEBALL_MANA,
	SHRINEBALL_LUNGS,
	SHRINEBALL_LIGHT,
	SHRINEBALL_POWERUP,
	SHRINEBALL_ARMOR,
	SHRINEBALL_ARMOR_GOLD,
	SHRINEBALL_RANDOM,
	SHRINEBALL_REFLECT,
	SHRINEBALL_STAFF,
	SHRINEBALL_GHOST,
	SHRINEBALL_SPEED,

	SHRINEBALL_MAX
};

static short shrine_particles[][2] =
{
	{	PART_16x16_SPARK_I,		PART_16x16_LIGHTNING	},	// SHRINEBALL_HEAL,
	{	PART_16x16_SPARK_G,		PART_16x16_SPARK_B		},	// SHRINEBALL_MANA,
	{	PART_16x16_WATERDROP,	PART_32x32_WFALL		},	// SHRINEBALL_LUNGS,
	{	PART_4x4_WHITE,			PART_32x32_ALPHA_GLOBE	},	// SHRINEBALL_LIGHT,
	{	PART_16x16_SPARK_G,		PART_16x16_SPARK_G		},	// SHRINEBALL_POWERUP,
	{	PART_8x8_BLUE_DIAMOND,	PART_8x8_BLUE_X			},	// SHRINEBALL_ARMOR,
	{	PART_16x16_SPARK_Y,		PART_16x16_SPARK_Y		},	// SHRINEBALL_ARMOR_GOLD,
	{	PART_8x8_RED_X,			PART_8x8_CYAN_DIAMOND	},	// SHRINEBALL_RANDOM
	{	PART_8x8_CYAN_DIAMOND,	PART_8x8_CYAN_X			},	// SHRINEBALL_REFLECT,
	{	PART_16x16_SPARK_C,		PART_32x32_FIREBALL		},	// SHRINEBALL_STAFF
	{	PART_32x32_WFALL,		PART_16x16_STAR			},	// SHRINEBALL_GHOST,
	{	PART_32x32_STEAM,		PART_32x32_STEAM		},	// SHRINEBALL_SPEED
};

// Make the shrine glow ball effect shimmer, and give off steam.
static qboolean ShrineBallThink(struct client_entity_s* self, centity_t* owner)
{
	int part;
	const int count = GetScaledCount(BALL_PART_NUM, 0.4f);

	for (int i = 0; i < count; i++)
	{
		assert(self->SpawnInfo >= 0 && self->SpawnInfo < SHRINEBALL_MAX);

		if (self->SpawnInfo == SHRINEBALL_RANDOM)
			part = irand(PART_16x16_SPARK_B, PART_16x16_SPARK_Y);
		else
			part = shrine_particles[self->SpawnInfo][irand(0, 1)];

		client_particle_t* ce = ClientParticle_new(part, color_white, 1000);

		if (part != PART_32x32_WFALL && part != PART_16x16_WATERDROP && part != PART_32x32_STEAM)
			ce->type |= PFL_ADDITIVE;

		const vec3_t rad = { flrand(0.0f, 360.0f), flrand(0.0f, 360.0f), 0.0f };

		vec3_t fwd;
		AngleVectors(rad, fwd, NULL, NULL);
		VectorScale(fwd, BALL_RAD, ce->velocity);
		VectorScale(ce->velocity, -1.0f, ce->acceleration);
		ce->color.a = 245;
		ce->scale = BALL_PART_SCALE;
		ce->d_scale = -0.5f * BALL_PART_SCALE;

		AddParticleToList(self, ce);
	}

	self->updateTime = 150;

	return true;
}

// Create the floating ball in the middle of the shrine.
void FXShrineBall(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	// Get the normalized direction of the shrine object.
	vec3_t offset;
	byte shrine_type;
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_SHRINE_BALL].formatString, &offset, &shrine_type);

	// Don't put out effects for shrines that don't exist.
	if (shrine_type >= SHRINEBALL_MAX)
		return;

	// Move our starting point out a bit.
	Vec3ScaleAssign(25.0f, offset);
	offset[2] = -10.0f;
	Vec3AddAssign(offset, origin);

	// Create the dummy entity, so particles can be attached.
	client_entity_t* glow = ClientEntity_new(type, (int)(flags | CEF_NO_DRAW | CEF_VIEWSTATUSCHANGED) & ~CEF_NOMOVE, origin, NULL, 20);

	glow->radius = 100.0f;
	glow->SpawnInfo = shrine_type;
	glow->Update = ShrineBallThink;

	AddEffect(owner, glow);
}

#pragma endregion

#pragma region ========================== EXPLODING SHRINE BALL EFFECT ==========================

// Explode the ball in the middle of the shrine.
void FXShrineBallExplode(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	// Get the normalized direction of the shrine object.
	vec3_t offset;
	byte shrine_type;
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_SHRINE_BALL_EXPLODE].formatString, &offset, &shrine_type);

	// Move our starting point out a bit.
	Vec3ScaleAssign(25.0f, offset);
	offset[2] = -10.0f;
	Vec3AddAssign(offset, origin);

	// Create the dummy entity, so particles can be attached.
	client_entity_t* burst = ClientEntity_new(type, (int)(flags | CEF_NO_DRAW | CEF_CHECK_OWNER) & ~CEF_NOMOVE, origin, NULL, 1200);
	burst->radius = 100.0f;
	AddEffect(owner, burst);

	assert(shrine_type < SHRINEBALL_MAX); //mxd. '<= SHRINEBALL_MAX' in original logic.

	const int count = GetScaledCount(BALL_EX_PART_NUM, 0.4f);

	// Create a bunch of exploding particles.
	for (int i = 0; i < count; i++)
	{
		int part;

		if (shrine_type == SHRINEBALL_RANDOM)
			part = irand(PART_16x16_SPARK_B, PART_16x16_SPARK_Y);
		else
			part = shrine_particles[shrine_type][irand(0, 1)];

		client_particle_t* ce = ClientParticle_new(part, color_white, 1150);

		if (part != PART_32x32_WFALL && part != PART_16x16_WATERDROP)
			ce->type |= PFL_ADDITIVE;

		const vec3_t rad = { flrand(0.0f, 360.0f), flrand(0.0f, 360.0f), 0.0f };

		vec3_t fwd;
		AngleVectors(rad, fwd, NULL, NULL);
		VectorScale(fwd, BALL_RAD, ce->velocity);
		VectorScale(ce->velocity, -0.7f, ce->acceleration);
		ce->color.a = 245;
		ce->scale = BALL_PART_SCALE;
		ce->d_scale = -0.5f * BALL_PART_SCALE;

		AddParticleToList(burst, ce);
	}

	// Add an additional flash as well.
	client_entity_t* flash = ClientEntity_new(-1, flags, origin, NULL, 250);

	flash->radius = 64.0f;
	flash->r.model = &shrine_models[1];
	flash->r.flags = RF_TRANS_ADD | RF_TRANS_ADD_ALPHA | RF_TRANSLUCENT;
	flash->r.frame = 1;
	flash->d_alpha = -4.0f;
	flash->d_scale = -4.0f;

	AddEffect(NULL, flash);
}

#pragma endregion