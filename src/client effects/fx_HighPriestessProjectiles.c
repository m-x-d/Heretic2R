//
// fx_HighPriestessProjectiles.c -- Named 'fx_hpproj.c' in original logic --mxd.
//
// Copyright 1998 Raven Software
//

#include "fx_HighPriestessProjectiles.h" //mxd
#include "Client Effects.h"
#include "Particle.h"
#include "Vector.h"
#include "Random.h"
#include "Utilities.h"
#include "ce_DLight.h"
#include "q_Sprite.h"
#include "g_playstats.h"

#define PRIESTESS_TELEPORT_LINEHEIGHT 764

static struct model_s* hpproj_models[11];

void PreCacheHPMissile(void)
{
	// Projectile head sprites.
	hpproj_models[0] = fxi.RegisterModel("sprites/fx/hpproj1_1.sp2");
	hpproj_models[1] = fxi.RegisterModel("sprites/fx/hpproj1_2.sp2"); //TODO: unused?

	// Trail segments.
	hpproj_models[2] = fxi.RegisterModel("sprites/fx/segment_trail.sp2");
	hpproj_models[6] = fxi.RegisterModel("sprites/fx/segment_trail_y.sp2");
	hpproj_models[7] = fxi.RegisterModel("sprites/fx/segment_trail_wt.sp2");

	// Halos.
	hpproj_models[3] = fxi.RegisterModel("sprites/fx/hp_halo.sp2");
	hpproj_models[4] = fxi.RegisterModel("sprites/lens/halo1.sp2");

	// Light Bug model.
	hpproj_models[5] = fxi.RegisterModel("models/objects/lights/bug/tris.fm");

	// Rocks.
	hpproj_models[8] = fxi.RegisterModel("models/debris/stone/schunk1/tris.fm");
	hpproj_models[9] = fxi.RegisterModel("models/debris/stone/schunk2/tris.fm");

	// Lightning bolts.
	hpproj_models[10] = fxi.RegisterModel("sprites/fx/hp_lightning.sp2");
}

static qboolean HPTeleportStartTrailUpdate(client_entity_t* self, centity_t* owner) //mxd. Named 'FXHPTeleportLineThink' in original logic.
{
	if (self->alpha <= 0.0f)
		return false;

	self->r.endpos[2] += 32.0f;

	if (self->r.endpos[2] - self->r.startpos[2] > PRIESTESS_TELEPORT_LINEHEIGHT)
	{
		if (self->SpawnInfo == 0)
			fxi.Activate_Screen_Flash((int)color_white.c);

		self->SpawnInfo = 1;
		self->d_alpha = -1.0f;
		self->r.scale += 16.0f;

		return true;
	}

	self->r.scale += 1.5f;

	// Spawn some particles and some rock chunks.
	const int num_particles = GetScaledCount(8, 0.7f);

	for (int i = 0; i < num_particles; i++)
	{
		client_particle_t* p = ClientParticle_new(PART_4x4_WHITE, color_white, 2500);

		// Chance to make them brown.
		if (irand(0, 1))
		{
			const byte col = (byte)irand(0, 64);
			COLOUR_SET(p->color, col, col, 0); //mxd
		}
		else
		{
			const byte col = (byte)irand(32, 128);
			COLOUR_SET(p->color, col, col, col); //mxd
		}

		p->origin[0] += flrand(-24.0f, 24.0f); //mxd. irand() in original version. Why?
		p->origin[1] += flrand(-24.0f, 24.0f); //mxd. irand() in original version. Why?
		p->origin[2] -= 36;

		p->scale = flrand(0.25f, 1.0f);

		p->acceleration[0] = flrand(-75.0f, 75.0f); //mxd. irand() in original version. Why?
		p->acceleration[1] = flrand(-75.0f, 75.0f); //mxd. irand() in original version. Why?
		p->acceleration[2] = flrand(400.0f, 500.0f); //mxd. irand() in original version. Why?

		p->d_alpha = -2.0f;

		AddParticleToList(self, p);
	}

	const int num_rocks = GetScaledCount(5, 0.7f);

	for (int i = 0; i < num_rocks; i++)
	{
		client_entity_t* rock = ClientEntity_new(FX_HP_MISSILE, CEF_DONT_LINK, self->origin, NULL, 1000);

		rock->radius = 500.0f;
		rock->r.model = &hpproj_models[irand(8, 9)];
		rock->r.scale = flrand(0.1f, 0.5f);

		VectorCopy(self->r.origin, rock->r.origin);
		rock->r.origin[0] += flrand(-32.0f, 32.0f); //mxd. irand() in original version. Why?
		rock->r.origin[1] += flrand(-32.0f, 32.0f); //mxd. irand() in original version. Why?
		rock->r.origin[2] -= 36.0f;

		rock->velocity[2] = 1.0f;

		rock->acceleration[0] = flrand(-150.0f, 150.0f); //mxd. irand() in original version. Why?
		rock->acceleration[1] = flrand(-150.0f, 150.0f); //mxd. irand() in original version. Why?
		rock->acceleration[2] = flrand(300.0f, 600.0f - (self->r.scale * 100.0f)); //mxd. irand() in original version. Why?

		rock->d_alpha = -0.25f;

		AddEffect(NULL, rock);
	}

	return true;
}

static qboolean HPTeleportEndTrailUpdate(client_entity_t* self, centity_t* owner) //mxd. Named 'FXHPTeleportLineThink2' in original logic.
{
	if (self->alpha > 0.0f && self->r.scale > 0.0f)
	{
		self->r.scale -= 4.0f;
		return true;
	}

	return false;
}

static qboolean HPMissile12LightUpdate(client_entity_t* self, centity_t* owner) //mxd. Named 'FXHPMissileSpawnerThink' in original logic.
{
	if (self->LifeTime < fx_time)
		return false;

	client_entity_t* trail = ClientEntity_new(FX_HP_MISSILE, CEF_DONT_LINK, self->origin, NULL, 1000);

	trail->radius = 500.0f;
	trail->r.flags = RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	trail->r.model = &hpproj_models[3]; // Halo sprite.
	trail->r.scale = 0.1f;

	trail->d_alpha = -2.5f;
	trail->d_scale = 4.0f;

	VectorCopy(self->origin, trail->origin);
	VectorRandomSet(trail->velocity, 16.0f); //mxd. Set using irand() in original version. Why?

	AddEffect(NULL, trail);

	return true;
}

static qboolean HPMissile3LightUpdate(client_entity_t* self, centity_t* owner) //mxd. Named 'FXHPMissileSpawnerThink2' in original logic.
{
	if (self->LifeTime < fx_time)
		return false;

	client_entity_t* trail = ClientEntity_new(FX_HP_MISSILE, CEF_DONT_LINK, self->origin, NULL, 1000);

	trail->radius = 500.0f;
	trail->r.flags = RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	trail->r.model = &hpproj_models[4];
	COLOUR_SET(trail->r.color, 229, 250, 88);

	trail->alpha = 0.5f;
	trail->r.scale = 0.25f;

	trail->d_alpha = -2.5f;
	trail->d_scale = 2.0f;

	VectorCopy(self->origin, trail->origin);
	VectorRandomSet(trail->velocity, 16.0f); //mxd. Set using irand() in original version. Why?

	AddEffect(NULL, trail);

	return true;
}

static qboolean HPHaloDie(client_entity_t* self, centity_t* owner)
{
	return (self->r.scale > 0.0f && self->alpha > 0.0f);
}

static qboolean HPMissile4LightUpdate(client_entity_t* self, centity_t* owner) //mxd. Named 'FXHPMissileSpawnerThink2' in original logic.
{
	if (self->LifeTime < fx_time)
	{
		self->Update = HPHaloDie;
		self->d_scale = -2.0f;
		self->d_alpha = -1.0f;

		return true;
	}

	if (self->d_scale == 0.0f)
	{
		self->r.scale = flrand(1.75f, 2.25f);
		return true;
	}

	if (self->r.scale >= 2.0f)
		self->d_scale = 0.0f;

	if (self->alpha > 0.5f)
	{
		self->d_alpha = 0.0f;
		self->alpha = 0.5f;
	}

	return true;
}

static qboolean HPMissile1TrailLineUpdate(client_entity_t* self, centity_t* owner) //mxd. Named 'FXHPTrailThink' in original logic.
{
	if (self->alpha > 0.1f && self->r.scale > 0.0f)
	{
		self->r.scale -= 0.1f;
		return true;
	}

	return false;
}

static qboolean HPMissile2TrailLineUpdate(client_entity_t* self, centity_t* owner) //mxd. Named 'FXHPTrailThink2' in original logic.
{
	if (self->alpha > 0.1f && self->r.scale > 0.0f)
	{
		self->r.scale -= 0.15f;
		return true;
	}

	return false;
}

static qboolean HPMissile3TrailLineUpdate(client_entity_t* self, centity_t* owner) //mxd. Named 'FXHPTrailThink3' in original logic.
{
	if (self->alpha > 0.1f && self->r.scale > 0.0f)
	{
		self->r.scale -= 0.25f;
		return true;
	}

	return false;
}

static qboolean HPMissile3HaloUpdate(client_entity_t* self, centity_t* owner) //mxd. Named 'FXHPBugThink' in original logic.
{
	self->r.scale = flrand(0.2f, 0.4f);
	self->alpha = flrand(0.3f, 0.5f);

	return true;
}

static qboolean HPMissile1TrailUpdate(client_entity_t* self, centity_t* owner) //mxd. Named 'FXHPMissileTrailThink' in original logic. 
{
	self->r.scale = flrand(0.35f, 0.65f);

	client_entity_t* trail = ClientEntity_new(FX_HP_MISSILE, CEF_DONT_LINK, owner->origin, NULL, 17);

	trail->radius = 500.0f;

	VectorCopy(owner->origin, trail->origin);

	trail->r.flags = RF_TRANSLUCENT | RF_TRANS_ADD_ALPHA;
	trail->r.model = &hpproj_models[2]; // Segment trail sprite.

	trail->r.spriteType = SPRITE_LINE;
	trail->r.tile = 1.0f;
	trail->r.scale = 2.5f;

	VectorCopy(self->startpos, trail->r.startpos);
	VectorCopy(owner->origin, trail->r.endpos);

	trail->d_alpha = -2.5f;
	trail->Update = HPMissile1TrailLineUpdate;

	AddEffect(NULL, trail);

	VectorCopy(owner->origin, self->startpos);

	return true;
}

static qboolean HPMissile2TrailUpdate(client_entity_t* self, centity_t* owner) //mxd. Named 'FXHPMissileTrailThink2' in original logic.
{
	self->r.scale = flrand(0.35f, 0.55f);

	client_entity_t* trail = ClientEntity_new(FX_HP_MISSILE, CEF_DONT_LINK, owner->origin, NULL, 17);

	trail->radius = 500.0f;

	VectorCopy(owner->origin, trail->origin);

	trail->r.flags = (RF_TRANSLUCENT | RF_TRANS_ADD_ALPHA);
	trail->r.model = &hpproj_models[2]; // Segment trail sprite.

	trail->r.spriteType = SPRITE_LINE;
	trail->r.tile = 1.0f;
	trail->r.scale = 2.0f; //mxd. BUGFIX: set to 2.0, then set to 1.0 below in original version.

	VectorCopy(self->startpos, trail->r.startpos);
	VectorCopy(owner->origin, trail->r.endpos);

	trail->d_alpha = -4.0f;
	trail->d_scale = 0.0f;
	trail->Update = HPMissile2TrailLineUpdate;

	AddEffect(NULL, trail);

	VectorCopy(owner->origin, self->startpos);

	return true;
}

static qboolean HPMissile3TrailUpdate(client_entity_t* self, centity_t* owner) //mxd. Named 'FXHPMissileTrailThink3' in original logic.
{
	client_entity_t* trail = ClientEntity_new(FX_HP_MISSILE, CEF_DONT_LINK, owner->origin, NULL, 17);

	trail->radius = 500.0f;

	VectorCopy(owner->origin, trail->origin);

	trail->r.flags = RF_TRANSLUCENT | RF_TRANS_ADD_ALPHA;
	trail->r.model = &hpproj_models[6]; // Yellow segment trail sprite.

	trail->r.spriteType = SPRITE_LINE;
	trail->r.tile = 1.0f;
	trail->r.scale = 2.0f; //mxd. BUGFIX: set to 2.0, then set to 1.0 below in original version.

	VectorCopy(self->startpos, trail->r.startpos);
	VectorCopy(owner->origin, trail->r.endpos);

	trail->d_alpha = -2.0f;
	trail->d_scale = 0.0f;
	trail->Update = HPMissile3TrailLineUpdate;

	AddEffect(NULL, trail);

	VectorCopy(owner->origin, self->startpos);

	return true;
}

static void HPMissileExplode(client_entity_t* self, const centity_t* owner)
{
	const vec3_t dir = { 1.0f, 1.0f, 1.0f };
	const int count = GetScaledCount(irand(6, 8), 0.8f);

	for (int i = 0; i < count; i++)
	{
		const int next_think_time = (i == count - 1 ? 500 : 1000); //mxd
		client_entity_t* smoke_puff = ClientEntity_new(FX_HP_MISSILE, 0, owner->origin, NULL, next_think_time);

		smoke_puff->r.model = &hpproj_models[3]; // Halo sprite.
		smoke_puff->r.scale = flrand(0.5f, 1.0f);
		smoke_puff->d_scale = flrand(-1.0f, -1.5f);

		smoke_puff->r.flags = (RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA);

		VectorRandomCopy(dir, smoke_puff->velocity, flrand(64.0f, 128.0f));
		VectorSet(smoke_puff->acceleration, flrand(-100.0f, -50.0f), flrand(-100.0f, -50.0f), flrand(-250.0f, -100.0f));

		smoke_puff->d_alpha = -0.2f;
		smoke_puff->radius = 20.0f;

		AddEffect(NULL, smoke_puff);
	}
}

static void HPBugExplode(client_entity_t* self, const centity_t* owner)
{
	const vec3_t dir = { 1.0f, 1.0f, 1.0f };
	const int count = GetScaledCount(irand(12, 16), 0.8f);

	for (int i = 0; i < count; i++)
	{
		const int next_think_time = (i == count - 1 ? 500 : 1000); //mxd
		client_entity_t* smoke_puff = ClientEntity_new(FX_HP_MISSILE, 0, owner->origin, NULL, next_think_time);

		smoke_puff->r.model = &hpproj_models[0]; // Projectile head sprite.
		smoke_puff->r.scale = flrand(0.5f, 1.0f);
		smoke_puff->d_scale = -2.0f;

		smoke_puff->r.flags = (RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA);

		VectorRandomCopy(dir, smoke_puff->velocity, flrand(16.0f, 64.0f));
		VectorSet(smoke_puff->acceleration, flrand(-200.0f, 200.0f), flrand(-200.0f, 200.0f), flrand(-100.0f, -60.0f));

		smoke_puff->d_alpha = -0.4f;
		smoke_puff->radius = 20.0f;

		AddEffect(NULL, smoke_puff);
	}
}

void FXHPMissileCreateWarp(const int type, const vec3_t origin)
{
	client_entity_t* halo = ClientEntity_new(type, CEF_DONT_LINK, origin, NULL, 2000);

	halo->radius = 500.0f;
	halo->r.model = &hpproj_models[3]; // Halo sprite.
	halo->r.color.c = 0xffff5555;
	halo->r.flags = (RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA);
	halo->r.scale = 0.1f;
	halo->d_scale = 2.0f;
	halo->d_alpha = -2.0f;

	AddEffect(NULL, halo);
}

static qboolean PriestessLinkedEntityUpdatePlacement(client_entity_t* self, centity_t* owner)
{
	LinkedEntityUpdatePlacement(self, owner);
	VectorCopy(self->r.origin, self->r.startpos);

	return true;
}

#pragma region ========================== Missile spawner functions (mxd -- split into separate functions from FXHPMissile()) ==========================

static void SpawnHPMissile1(centity_t* owner, const int type, const vec3_t origin, const vec3_t velocity, const paletteRGBA_t light_color) //mxd
{
	FXHPMissileCreateWarp(type, origin);

	client_entity_t* trail = ClientEntity_new(type, CEF_OWNERS_ORIGIN | CEF_DONT_LINK, origin, NULL, 20);

	VectorCopy(velocity, trail->up);

	trail->radius = 500.0f;
	trail->r.model = &hpproj_models[3]; // Halo sprite.
	trail->r.color.c = 0x00999999;
	trail->r.flags = RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	trail->r.scale = 0.5f;
	trail->dlight = CE_DLight_new(light_color, 150.0f, 0.0f);

	trail->Update = HPMissile1TrailUpdate;
	trail->AddToView = PriestessLinkedEntityUpdatePlacement;

	VectorCopy(origin, trail->startpos);

	AddEffect(owner, trail);
	HPMissile1TrailUpdate(trail, owner);
}

static void SpawnHPMissile2(centity_t* owner, const int type, const vec3_t origin, const vec3_t velocity, const paletteRGBA_t light_color) //mxd
{
	FXHPMissileCreateWarp(type, origin);

	client_entity_t* trail = ClientEntity_new(type, CEF_OWNERS_ORIGIN | CEF_DONT_LINK, origin, NULL, 20);

	VectorCopy(velocity, trail->up);

	trail->radius = 500.0f;
	trail->r.model = &hpproj_models[3]; // Halo sprite.
	trail->r.color.c = 0x00999999;
	trail->r.flags = RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	trail->r.scale = 0.45f;
	trail->dlight = CE_DLight_new(light_color, 150.0f, 0.0f);

	trail->Update = HPMissile2TrailUpdate;
	trail->AddToView = PriestessLinkedEntityUpdatePlacement;

	VectorCopy(owner->origin, trail->startpos);

	AddEffect(owner, trail);
	HPMissile2TrailUpdate(trail, owner);
}

static void SpawnHPMissile3(centity_t* owner, const int type, const vec3_t origin, vec3_t velocity, const paletteRGBA_t bug_color) //mxd
{
	client_entity_t* bug = ClientEntity_new(type, CEF_OWNERS_ORIGIN | CEF_DONT_LINK, origin, NULL, 20);

	bug->radius = 500.0f;
	bug->r.model = &hpproj_models[5]; // Light bug model.

	VectorNormalize(velocity);
	AnglesFromDir(velocity, bug->r.angles);
	bug->r.angles[PITCH] -= ANGLE_90;

	bug->r.color.c = 0xff999999;
	bug->r.scale = flrand(0.3f, 0.4f);
	bug->dlight = CE_DLight_new(bug_color, 50.0f, 0.0f);

	bug->AddToView = LinkedEntityUpdatePlacement;
	bug->Update = HPMissile3TrailUpdate;

	VectorCopy(owner->origin, bug->startpos);

	AddEffect(owner, bug);
	HPMissile3TrailUpdate(bug, owner);

	// Create the halo to follow the bug.
	client_entity_t* halo = ClientEntity_new(type, CEF_OWNERS_ORIGIN | CEF_DONT_LINK, origin, NULL, 20);

	halo->radius = 500.0f;
	halo->r.model = &hpproj_models[4]; // Halo sprite.
	halo->r.flags = RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	halo->r.scale = flrand(0.3f, 0.4f);
	COLOUR_SET(halo->r.color, 229, 250, 88);
	halo->alpha = 0.5f;

	halo->AddToView = LinkedEntityUpdatePlacement;
	halo->Update = HPMissile3HaloUpdate;

	VectorCopy(origin, halo->startpos);

	AddEffect(owner, halo);
	HPMissile3HaloUpdate(halo, owner);
}

static void SpawnHPMissile4(const int type, const vec3_t origin, vec3_t velocity) //mxd
{
	vec3_t angle;
	const int count = irand(5, 8);

	for (int i = 0; i < count; i++)
	{
		client_entity_t* trail = ClientEntity_new(type, CEF_DONT_LINK, origin, NULL, 2000);

		trail->radius = 500.0f;
		trail->r.flags = RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		trail->r.model = &hpproj_models[7];

		trail->r.spriteType = SPRITE_LINE;
		trail->r.tile = 1.0f;
		trail->r.scale = flrand(1.0f, 2.0f); //mxd. BUGFIX: 1. was irand(), 2. was set to 1.0 below in original logic...

		angle[PITCH] = flrand(0.0f, 720.0f); //mxd. Was irand() in original logic.
		angle[YAW] = flrand(0.0f, 720.0f); //mxd. Was irand() in original logic.
		angle[ROLL] = flrand(0.0f, 720.0f); //mxd. Was irand() in original logic.

		AngleVectors(angle, velocity, NULL, NULL);
		VectorMA(origin, 512.0f, velocity, trail->r.endpos);

		VectorCopy(origin, trail->r.startpos);
		VectorCopy(origin, trail->r.origin);

		trail->d_alpha = -2.0f;
		trail->d_scale = 0.0f;

		AddEffect(NULL, trail);
	}
}

static void SpawnHPMissile5(const int type, const vec3_t origin, vec3_t velocity) //mxd
{
	const int count = irand(2, 3);

	for (int i = 0; i < count; i++)
	{
		// Find the number of bends in the bolt.
		const int bends = irand(3, 6);

		// Find the distance for each segment (totalDist/bends).
		vec3_t bolt_dir;
		VectorSubtract(velocity, origin, bolt_dir);
		VectorNormalize(bolt_dir);

		// Get an offset angle from the goal direction.
		vec3_t bolt_angle;
		vectoangles(bolt_dir, bolt_angle);

		bolt_angle[PITCH] += flrand(-35.0f, 35.0f); //mxd. Was irand() in original logic.
		bolt_angle[YAW] += flrand(-35.0f, 35.0f); //mxd. Was irand() in original logic.

		vec3_t bolt_dest;
		VectorCopy(bolt_dir, bolt_dest);
		AngleVectors(bolt_angle, bolt_dir, NULL, NULL);

		vec3_t old_pos;
		VectorCopy(origin, old_pos);

		const float width = flrand(2.0f, 6.0f); //mxd. Was irand() in original logic.
		const float alpha = flrand(-4.0f, -8.0f); //mxd. Was irand() in original logic.

		for (int j = 1; j < bends + 1; j++)
		{
			client_entity_t* bolt = ClientEntity_new(type, CEF_DONT_LINK, origin, NULL, 2000);

			if (j == 1) //BUGFIX: was if (!j) in original version (which never happens). //TODO: test this!
			{
				VectorCopy(origin, bolt->r.startpos);
				VectorCopy(origin, bolt->r.endpos);
			}

			bolt->radius = 500.0f;
			bolt->r.flags = RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
			bolt->r.model = &hpproj_models[10];

			bolt->r.spriteType = SPRITE_LINE;
			bolt->r.scale = width;

			VectorCopy(velocity, bolt->r.origin);

			if (j == bends)
			{
				VectorCopy(old_pos, bolt->r.startpos);
				VectorCopy(velocity, bolt->r.endpos);

				bolt->r.endpos[2] += flrand(-4.0f, 4.0f); //mxd. Was irand() in original logic.
			}
			else
			{
				vec3_t hunt_dir;
				VectorSubtract(velocity, old_pos, hunt_dir);
				float bolt_step = VectorNormalize(hunt_dir);
				bolt_step /= (float)(bends + 1 - j);

				VectorScale(bolt_dir, 2.5f / (float)bends, bolt_dir);
				VectorAdd(bolt_dir, hunt_dir, bolt_dir);
				VectorNormalize(bolt_dir);

				VectorCopy(old_pos, bolt->r.startpos);

				bolt->r.tile = ceilf(bolt_step / 128.0f);

				VectorMA(bolt->r.startpos, bolt_step, bolt_dir, bolt->r.endpos);
				VectorCopy(bolt->r.endpos, old_pos);
			}

			bolt->d_alpha = alpha;
			bolt->d_scale = 0.0f;

			AddEffect(NULL, bolt);
		}
	}
}

static void SpawnHPMissile12Explode(const centity_t* owner, const int type, const vec3_t origin) //mxd
{
	client_entity_t* explosion = ClientEntity_new(type, CEF_NO_DRAW | CEF_DONT_LINK, origin, NULL, 2000);
	AddEffect(NULL, explosion);
	HPMissileExplode(explosion, owner);
}

static void SpawnHPMissile3Explode(const centity_t* owner, const int type, const vec3_t origin) //mxd
{
	client_entity_t* explosion = ClientEntity_new(type, CEF_NO_DRAW | CEF_DONT_LINK, origin, NULL, 2000);
	AddEffect(NULL, explosion);
	HPBugExplode(explosion, owner);
}

static void SpawnHPMissile12Light(const int type, const vec3_t origin, const paletteRGBA_t light_color) //mxd
{
	client_entity_t* light = ClientEntity_new(type, CEF_NO_DRAW | CEF_DONT_LINK, origin, NULL, 20);

	light->radius = 500.0f;
	light->r.model = &hpproj_models[3]; // Halo sprite.
	light->r.color = color_white;
	light->r.flags = RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	light->LifeTime = fx_time + 4000;
	light->dlight = CE_DLight_new(light_color, 200.0f, 0.0f);
	light->Update = HPMissile12LightUpdate;

	AddEffect(NULL, light);
}

static void SpawnHPMissile3Light(const int type, const vec3_t origin, const paletteRGBA_t bug_color) //mxd
{
	client_entity_t* light = ClientEntity_new(type, CEF_NO_DRAW | CEF_DONT_LINK, origin, NULL, 20);

	light->radius = 500.0f;
	light->LifeTime = fx_time + 2000;
	light->dlight = CE_DLight_new(bug_color, 10.0f, 0.0f);
	light->Update = HPMissile3LightUpdate;

	AddEffect(NULL, light);
}

static void SpawnHPMissile4Light(const int type, const vec3_t origin) //mxd
{
	fxi.Activate_Screen_Flash((int)color_white.c);

	// Create the large halo that scales up and fades out.
	client_entity_t* halo = ClientEntity_new(type, CEF_DONT_LINK, origin, NULL, 4000);

	halo->radius = 500.0f;
	halo->r.flags = (RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA);
	halo->r.model = &hpproj_models[4]; // Halo sprite.
	halo->r.scale = 0.1f;
	halo->d_alpha = -1.0f;
	halo->d_scale = 4.0f;
	halo->LifeTime = fx_time + 4000;

	AddEffect(NULL, halo);

	// Create the sparkling center.
	client_entity_t* spark = ClientEntity_new(type, CEF_DONT_LINK, origin, NULL, 20);

	spark->radius = 500.0f;
	spark->r.flags = (RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA);

	spark->r.model = &hpproj_models[4];
	spark->r.scale = 0.1f;
	spark->alpha = 0.1f;
	spark->d_alpha = 0.5f;
	spark->d_scale = 2.0f;
	spark->LifeTime = fx_time + 4000;
	spark->dlight = CE_DLight_new(color_white, 250.0f, 0.0f);
	spark->Update = HPMissile4LightUpdate;

	AddEffect(NULL, spark);

	// Shake the screen.
	fxi.Activate_Screen_Shake(4.0f, 5500.0f, (float)fxi.cl->time, SHAKE_ALL_DIR); // 'current_time' MUST be cl.time, because that's what used by Perform_Screen_Shake() to calculate effect intensity/timing... --mxd.
}

static void SpawnHPTeleportStart(const int type, const vec3_t origin) //mxd
{
	client_entity_t* trail = ClientEntity_new(type, CEF_DONT_LINK, origin, NULL, 20);

	trail->r.flags = (RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA);
	trail->r.model = &hpproj_models[7]; // Trail segment sprite.
	trail->r.spriteType = SPRITE_LINE;
	trail->r.tile = 1.0f;
	trail->r.scale = 2.0f;

	VectorCopy(origin, trail->r.startpos);
	trail->r.startpos[2] -= 128.0f;

	VectorCopy(origin, trail->r.endpos);
	trail->r.endpos[2] += 32.0f;

	trail->d_alpha = 0.0f;
	trail->d_scale = 0.0f;
	trail->dlight = CE_DLight_new(color_white, 250.0f, 0.0f);
	trail->Update = HPTeleportStartTrailUpdate;

	AddEffect(NULL, trail);
}

static void SpawnHPTeleportEnd(const int type, const vec3_t origin) //mxd
{
	client_entity_t* trail = ClientEntity_new(type, CEF_DONT_LINK, origin, NULL, 20);

	trail->r.flags = RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	trail->r.model = &hpproj_models[7]; // Trail segment sprite.
	trail->r.spriteType = SPRITE_LINE;
	trail->r.tile = 1.0f;
	trail->r.scale = 64.0f; //mxd. BUGFIX??? Was set to 1.0 below in original version.

	VectorCopy(origin, trail->r.startpos);
	trail->r.startpos[2] -= 128.0f;

	VectorCopy(origin, trail->r.endpos);
	trail->r.endpos[2] += 512.0f;

	trail->d_alpha = -1.0f;
	trail->d_scale = 0.0f;
	trail->dlight = CE_DLight_new(color_white, 250.0f, 0.0f);
	trail->Update = HPTeleportEndTrailUpdate;

	AddEffect(NULL, trail);
}

#pragma endregion

void FXHPMissile(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	vec3_t velocity;
	byte fx_type;
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_HP_MISSILE].formatString, &velocity, &fx_type);

	const paletteRGBA_t light_color = { .r = 0, .g = 0, .b = 255, .a = 255 };
	const paletteRGBA_t bug_color = { .r = 229, .g = 250, .b = 88, .a = 255 };

	switch (fx_type)
	{
		case HPMISSILE1: // Blue swirling, homing missiles.
			SpawnHPMissile1(owner, type, origin, velocity, light_color); //mxd
			break;


		case HPMISSILE2: // Normal trails off projectiles.
			SpawnHPMissile2(owner, type, origin, velocity, light_color); //mxd
			break;

		case HPMISSILE3: // Light bugs/mines.
			SpawnHPMissile3(owner, type, origin, velocity, bug_color); //mxd
			break;

		case HPMISSILE4: // Light shafts coming from the priestess' staff.
			SpawnHPMissile4(type, origin, velocity); //mxd
			break;

		case HPMISSILE5: // The power bolts she shoots from her staff.
			SpawnHPMissile5(type, origin, velocity); //mxd
			break;

		case HPMISSILE1_EXPLODE: // Normal explosions.
		case HPMISSILE2_EXPLODE:
			SpawnHPMissile12Explode(owner, type, origin); //mxd
			break;

		case HPMISSILE3_EXPLODE: // Light bug explosion.
			SpawnHPMissile3Explode(owner, type, origin); //mxd
			break;

		case HPMISSILE1_LIGHT: // Light the emanates from her staff when casting effects.
		case HPMISSILE2_LIGHT:
			SpawnHPMissile12Light(type, origin, light_color); //mxd
			break;

		case HPMISSILE3_LIGHT: // Light the emanates from her staff when casting bugs.
			SpawnHPMissile3Light(type, origin, bug_color); //mxd
			break;

		case HPMISSILE4_LIGHT: // Light for the massive light attack.
			SpawnHPMissile4Light(type, origin); //mxd
			break;

		case HPMISSILE5_LIGHT: // Empty
			break;

		case HPTELEPORT_START: // Starting effect for her teleport.
			SpawnHPTeleportStart(type, origin); //mxd
			break;

		case HPTELEPORT_END: // Ending effect for her teleport.
			SpawnHPTeleportEnd(type, origin); //mxd
			break;

		default:
			Com_DPrintf("ERROR FXHPMissile: No available effect processor! (EFFECT ID %d)\n", fx_type);
			break;
	}
}