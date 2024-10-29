//
// Heretic II
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Client Entities.h"
#include "Particle.h"
#include "ResourceManager.h"
#include "FX.h"
#include "Random.h"
#include "Vector.h"
#include "string.h"
#include "Utilities.h"
#include "g_playstats.h"

#define PLAYER_FADE_TIME  14
#define PLAYER_FADE_AMOUNT  255/ PLAYER_FADE_TIME
#define MANA_RAD			100.0
#define MANA_HEIGHT	32.0
#define ARMOR_RAD			35.0
#define LUNG_RAD			20.0
#define STAFF_RADIUS		20.0
#define NUM_OF_MANA_PARTS 16
#define NUM_OF_ARMOR_PARTS 30
#define NUM_OF_STAFF_PARTS 6				   
#define NUM_OF_LUNG_PARTS 15
#define TOTAL_STAFF_EFFECTS 30
#define STAFF_EFFECTS_HEIGHT 70
#define STAFF_EFFECTS_START_HEIGHT -30
#define STAFF_HEIGHT_ADD (STAFF_EFFECTS_HEIGHT - STAFF_EFFECTS_START_HEIGHT) / TOTAL_STAFF_EFFECTS
#define STAFF_GREEN_ADD -(255 / (TOTAL_STAFF_EFFECTS -8))
#define STAFF_BLUE_ADD STAFF_GREEN_ADD * 2
#define ACCEL_SCALE 3.1
#define LIGHTNING_START		20
#define LIGHTNING_MINIMUM	0.15
#define LIGHTNING_MAXIMUM	0.3
#define LIGHTNING_SPLIT_RAD 1.3
#define TOTAL_HEALTH_EFFECTS 8
#define BALL_RAD			40
#define BALL_PART_SCALE		10.0
#define BALL_PART_NUM		4
#define BALL_EX_PART_NUM	20
#define LIGHTNING_DUR		100
#define FLIGHT_RAD		20
#define TOTAL_FLIGHT_EFFECTS 60
#define FLIGHT_EFFECTS_HEIGHT 50
#define FLIGHT_EFFECTS_START_HEIGHT -35
#define FLIGHT_HEIGHT_ADD (FLIGHT_EFFECTS_HEIGHT - FLIGHT_EFFECTS_START_HEIGHT) / (TOTAL_FLIGHT_EFFECTS - 24)
#define NUM_OF_FLIGHT_PARTS 3
#define FLIGHT_PLAY_RAD 12
#define NUM_OF_REFLECT_PARTS 20
#define REFLECT_RAD		31
#define TOTAL_REFLECT_EFFECTS 50
#define REFLECT_EFFECTS_HEIGHT 50
#define REFLECT_EFFECTS_START_HEIGHT -35
#define REFLECT_HEIGHT_ADD (REFLECT_EFFECTS_HEIGHT - REFLECT_EFFECTS_START_HEIGHT) / (TOTAL_REFLECT_EFFECTS - 24)
#define TOTAL_POWERUP_EFFECTS 38
#define POWERUP_EFFECTS_HEIGHT 60
#define POWERUP_EFFECTS_START_HEIGHT -30
#define NUM_OF_POWERUP_PARTS 22
#define POWERUP_RAD		6
#define POWERUP_HEIGHT_ADD (POWERUP_EFFECTS_HEIGHT - POWERUP_EFFECTS_START_HEIGHT) / (TOTAL_POWERUP_EFFECTS - 24)

#define NUM_SHRINE_MODELS 2
static struct model_s *shrine_models[NUM_SHRINE_MODELS];

typedef struct
{
	int	red;
	int	green;
	int	blue;
} tint_tab_t;

tint_tab_t tint_tab[9] =
{
	{ 96, 255, 96 },
	{ 96, 255, 255 },
	{ 128, 128, 255 },
	{ 96, 255, 255 },
	{ 255, 255, 96 },
	{ 255, 255, 255 },
	{ 255, 96, 96 },
	{ 255, 96, 96 },
	{ 255, 96, 96 },
};

/*
Main Routines
*/

void PreCacheShrine()
{
	shrine_models[0] = fxi.RegisterModel("sprites/lens/halo1.sp2");
	shrine_models[1] = fxi.RegisterModel("sprites/fx/halo.sp2");
}

/*
----------------------------------------

Main Player routine  - make large player, and fade it into us

----------------------------------------
*/


// kill the expanding player model when it fades away
static qboolean shrine_player_update(struct client_entity_s *self, centity_t *owner)
{
	if (!(--self->SpawnInfo))
	{
		self->r.fmnodeinfo = NULL;
		return(false);		
	}

	return(true);
}

// Create a model of the player, and have it expand and fade, with a tint
void FXShrinePlayerEffect(centity_t *owner, int type, int flags, vec3_t origin)
{
	client_entity_t  	*shrine_fx;
	byte					shrine_type;

	// no longer used - causes really hard to find crashes.
	return;
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_SHRINE_PLAYER].formatString, &shrine_type);

	// push the model slightly up,since we create the model at a larger scale than the original
	origin[2] += 60.0;

	// create the player shrine effect around the player to begin with
 	shrine_fx = ClientEntity_new(type, flags, origin, NULL, 100);
	shrine_fx->radius = 20.0F;
	if (owner->current.effects & EF_CHICKEN)
		shrine_fx->r.model = shrine_models + 1;
	else
		shrine_fx->r.model = shrine_models;
	shrine_fx->d_scale = -1.2;
	shrine_fx->r.scale = 3.0;
	shrine_fx->velocity[2] = -35.5;
	shrine_fx->r.flags = RF_TRANSLUCENT ;
	shrine_fx->r.fmnodeinfo = &owner->current.fmnodeinfo[0];
	shrine_fx->Update = shrine_player_update;
	shrine_fx->SpawnInfo = PLAYER_FADE_TIME;
	shrine_fx->d_alpha = 0.45;
	shrine_fx->alpha = 0.05;
	shrine_fx->r.frame = owner->current.frame;
	shrine_fx->r.color.r = tint_tab[shrine_type].red;
	shrine_fx->r.color.g = tint_tab[shrine_type].green;
	shrine_fx->r.color.b = tint_tab[shrine_type].blue;
	shrine_fx->AddToView = LinkedEntityUpdatePlacement;
	VectorDegreesToRadians(&owner->current.angles[0],shrine_fx->r.angles);

  	AddEffect(owner, shrine_fx);

}


/*
----------------------------------------

Mana effect routine

----------------------------------------
*/

static qboolean FXShrineManaThink(struct client_entity_s *self, centity_t *owner)
{
	client_particle_t	*ce;
	paletteRGBA_t		color;
	vec3_t				vel, org;
	int					i;
	int					count;

	if (!(--self->SpawnInfo))
	{
		return(false);		
	}

	if (self->SpawnInfo >4)
	{
		count = GetScaledCount(NUM_OF_MANA_PARTS, 0.7);
		for(i = 0; i < count; i++)
		{
			// Calc spherical offset around left hand ref point
			VectorSet(vel, flrand(-1.0, 1.0), flrand(-1.0, 1.0), flrand(-1.0, 1.0));
			if(Vec3IsZero(vel))
				org[2] = 1.0;			// Safety in case flrand gens all zeros (VERY unlikely)
			VectorNormalize(vel);
			VectorScale(vel, MANA_RAD, vel);

			color.c = 0xffffffff;
			if (i&1)	// green parts
				ce = ClientParticle_new(PART_16x16_SPARK_G, color, 500);
			else		// blue parts
				ce = ClientParticle_new(PART_16x16_SPARK_B, color, 500);
			VectorCopy(vel, ce->origin);
			ce->origin[2] += MANA_HEIGHT;

			VectorScale(vel, -0.125, ce->velocity);
			VectorScale(vel, -8.0, ce->acceleration);
			ce->scale = 20.0F;
			ce->d_scale = -30.0F;
			ce->color.a = 1;
			ce->d_alpha = 400;
			AddParticleToList(self, ce);
		}
	}
	return(true);
}

// make the mana effect go off
void FXShrineManaEffect(centity_t *owner, int type, int flags, vec3_t origin)
{
	client_entity_t		*glow;

	glow = ClientEntity_new(type, flags | CEF_NO_DRAW |CEF_ADDITIVE_PARTS, origin, 0, 100);

	VectorClear(glow->origin);
	glow->Update = FXShrineManaThink;
	glow->SpawnInfo = 17;
	glow->AddToView = LinkedEntityUpdatePlacement;
	
	AddEffect(owner, glow);

}

/*
----------------------------------------

Armor effect routine

----------------------------------------
*/


static qboolean FXShrineArmorThink(struct client_entity_s *self, centity_t *owner)
{
	client_particle_t	*ce;
	paletteRGBA_t		color;
	vec3_t				vel, org;
	int					i;
	int					count;

	if (!(--self->SpawnInfo))
	{
		return(false);		
	}

	if (self->SpawnInfo >4)
	{
		count = GetScaledCount(NUM_OF_ARMOR_PARTS, 0.7);
		for(i = 0; i < count; i++)
		{
			// Calc spherical offset around left hand ref point
			VectorSet(vel, flrand(-1.0, 1.0), flrand(-1.0, 1.0), flrand(-1.0, 1.0));
			if(Vec3IsZero(vel))
				org[2] = 1.0;			// Safety in case flrand gens all zeros (VERY unlikely)
			VectorNormalize(vel);
			VectorScale(vel, ARMOR_RAD, vel);

			color.c = 0xffffffff;	// The particle is blue
			// which type of particle do we use ?  Gold or Silver armor ?
			if (self->flags & CEF_FLAG6)
				ce = ClientParticle_new(PART_16x16_ORANGE_PUFF, color, 500);
			else
				ce = ClientParticle_new(PART_16x16_BLUE_PUFF, color, 500);
			ce->acceleration[2] = 0.0; 
			VectorCopy(vel, ce->origin);
			ce->scale = 30.0F;
			ce->d_scale = -60.0F;
			ce->color.a = 1;
			ce->d_alpha = 400;
			AddParticleToList(self, ce);
		}
	}
	return(true);
}

void FXShrineArmorEffect(centity_t *owner, int type, int flags, vec3_t origin)
{
	client_entity_t		*glow;

	glow = ClientEntity_new(type, flags | CEF_NO_DRAW |CEF_ADDITIVE_PARTS, origin, 0, 100);

	VectorClear(glow->origin);
	glow->Update = FXShrineArmorThink;
	glow->SpawnInfo = 18;
	glow->AddToView = LinkedEntityUpdatePlacement;
	
	AddEffect(owner, glow);

}

/*
----------------------------------------

Lungs effect routines

----------------------------------------
*/

static qboolean FXShrineLungsThink(struct client_entity_s *self, centity_t *owner)
{
	client_particle_t	*ce;
	paletteRGBA_t		color;
	vec3_t				vel;
	int					i;
	int					count;

	if (!(--self->SpawnInfo))
	{
		return(false);		
	}

	if (self->SpawnInfo >10)
	{
		count = GetScaledCount(NUM_OF_LUNG_PARTS, 0.7);
		for(i = 0; i < count; i++)
		{
			// Calc spherical offset around left hand ref point
			VectorSet(vel, flrand(-1.0, 1.0), flrand(-1.0, 1.0), 0.01);
			VectorNormalize(vel);
			VectorScale(vel, flrand(1.0,LUNG_RAD), vel);
			vel[2] = -30.0;

		  	color.c = 0xffffff40;

			ce = ClientParticle_new(PART_32x32_STEAM, color, 450);

			VectorCopy(vel, ce->origin);
			VectorSet(ce->velocity, 0.0, 0.0, flrand(10.0, 40.0));
			VectorSet(ce->acceleration, 0.0, 0.0, flrand(300.0, 1000.0));
			ce->scale = 8.0F;
			AddParticleToList(self, ce);
		}
	}
	return(true);
}

void FXShrineLungsEffect(centity_t *owner, int type, int flags, vec3_t origin)
{
	client_entity_t		*glow;
														  
	glow = ClientEntity_new(type, flags | CEF_NO_DRAW , origin, 0, 50);

	VectorClear(glow->origin);
	glow->Update = FXShrineLungsThink;
	glow->SpawnInfo = 40;
	glow->AddToView = LinkedEntityUpdatePlacement;
	
	AddEffect(owner, glow);

}

/*
----------------------------------------

Light effect routines

----------------------------------------
*/

// create the light effect - a big old halo that fades away
void FXShrineLightEffect(centity_t *owner, int type, int flags, vec3_t origin)
{
	client_entity_t		*glow;

	glow = ClientEntity_new(type, flags, origin, 0, 2000);

	VectorClear(glow->origin);
	glow->r.model = shrine_models;
	glow->r.flags |= RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	glow->AddToView = LinkedEntityUpdatePlacement;
 	glow->d_scale = 5.0;
 	glow->r.scale = 0.1;
	glow->d_alpha = -0.45;
	glow->alpha = 1.0;
	
	AddEffect(owner, glow);
}

/*
----------------------------------------

Staff Powerup Routines

----------------------------------------
*/


// create the two circles that ring the player
static qboolean FXShrineStaffThink(struct client_entity_s *self, centity_t *owner)
{
	client_particle_t	*ce;
	vec3_t				vel;
	int					count;
	float					curAng;
	float					new_y;
	float					rad;

	if (!(--self->SpawnInfo))
	{
		return(false);		
	}
	rad = ((TOTAL_STAFF_EFFECTS)-self->SpawnInfo) * (STAFF_RADIUS/(TOTAL_STAFF_EFFECTS-8));

	if (self->SpawnInfo >8)
	{
		// figure out how many particles we are going to use
		count = GetScaledCount(self->SpawnDelay, 0.7);
		// create the ring of particles that goes up
		for(curAng = 0.0F; curAng < (M_PI * 2.0F); curAng += (M_PI * 2.0F) / count )
		{

			VectorSet(vel, rad * cos(curAng), rad * sin(curAng), self->SpawnData);

			if(self->flags & CEF_FLAG6)
				ce = ClientParticle_new(irand(PART_16x16_FIRE1, PART_16x16_FIRE3), self->r.color, self->LifeTime);
			else
				ce = ClientParticle_new(PART_16x16_SPARK_B, self->r.color, self->LifeTime);

			ce->acceleration[2] = 0.0; 
			VectorCopy(vel, ce->origin);
			ce->color.a = 118;
			ce->scale = 16.0F;
			AddParticleToList(self, ce);
		}
		// create the ring of particles that goes down
		new_y = -self->SpawnData;
		for(curAng = 0.0F; curAng < (M_PI * 2.0F); curAng += (M_PI * 2.0F) / count )
		{

			VectorSet(vel, rad * cos(curAng), rad * sin(curAng), new_y);

			if(self->flags & CEF_FLAG6)
				ce = ClientParticle_new(irand(PART_16x16_FIRE1, PART_16x16_FIRE3), self->r.color, self->LifeTime);
			else
				ce = ClientParticle_new(PART_16x16_SPARK_B, self->r.color, self->LifeTime);
			ce->acceleration[2] = 0.0; 
			VectorCopy(vel, ce->origin);
			ce->color.a = 118;
			ce->scale = 16.0F;
			AddParticleToList(self, ce);
		}
	}

	// update the particles colors
	if (self->r.color.g > 0)
		self->r.color.g += STAFF_GREEN_ADD;
	else
		self->r.color.g = 0;

	if (self->r.color.b > 0)
		self->r.color.b += STAFF_BLUE_ADD;
	else
		self->r.color.b = 0;

	// move the rings up/down next frame
	self->SpawnData += STAFF_HEIGHT_ADD;

	return(true);
}

// start up the staff power up effect
void FXShrineStaffEffect(centity_t *owner, int type, int flags, vec3_t origin)
{
	client_entity_t		*glow;
	int					line_count, particle_life;
						 
	if(r_detail->value >= DETAIL_HIGH)
	{
		line_count = NUM_OF_STAFF_PARTS;
		particle_life = 2400;
	}
	else
	if(r_detail->value >= DETAIL_NORMAL)
	{
		line_count = NUM_OF_STAFF_PARTS - 2;
		particle_life = 1900;
	}
	else
	{
		line_count = NUM_OF_STAFF_PARTS - 3;
		particle_life = 1500;
	}
	glow = ClientEntity_new(type, flags | CEF_NO_DRAW | CEF_ADDITIVE_PARTS, origin, 0,50);

	VectorClear(glow->origin);
	glow->Update = FXShrineStaffThink;
	glow->SpawnInfo = TOTAL_STAFF_EFFECTS;
	glow->AddToView = LinkedEntityUpdatePlacement;
	glow->SpawnData = STAFF_EFFECTS_START_HEIGHT;
	glow->SpawnDelay = line_count;
	glow->LifeTime = particle_life;
	
	AddEffect(owner, glow);
}


/*
----------------------------------------

Health Lightning routines

----------------------------------------
*/

// recursively called to create the lightning effect
void FXLightningSplit(struct client_entity_s *self, vec3_t org, vec3_t dir, int rand_chance, float stop_height)
{
	client_particle_t		*ce;
	float						start_height;
	vec3_t					dir2;
	vec3_t					dir_effect;
	paletteRGBA_t			color;
	vec3_t					org2;
	float						stop_height2;
	int						lightning_part_duration;

	VectorCopy(org, org2);

	start_height = org[2];
	color.c = 0xffffff;

	// create lightning particle
	while (org2[2] > stop_height)
	{
		// if we are the smaller split type, make our life longer, since we fade that much faster than the big particles
		if (stop_height != -30)
			lightning_part_duration = self->updateTime ;
		else
			lightning_part_duration = self->updateTime / 1.2 ;

		// create the individual particle
		ce = ClientParticle_new(PART_16x16_LIGHTNING, color, lightning_part_duration);
		ce->acceleration[2] = 0.0; 
		VectorCopy(org2, ce->origin);

		// figure out what our scale and alpaha should be based on whether its a small split or not
		if (stop_height != -30)
		{
			ce->scale = 0.5;
			ce->color.a = 108;
		}
		else
		{
			ce->scale = 1.2;
			ce->color.a = 255;
		}
		AddParticleToList(self, ce);

		// decide if we are splitting
		if (!(irand(0,rand_chance)))
		{
			// decide a new direction for the two halves
			dir[1] = flrand(-LIGHTNING_SPLIT_RAD, LIGHTNING_SPLIT_RAD);
			dir[0] = flrand(-LIGHTNING_SPLIT_RAD, LIGHTNING_SPLIT_RAD);

			// create the new split
			dir2[2] = dir[2];
			dir2[1] = -dir[1];
			dir2[0] = -dir[0];
			rand_chance += 5;

			// if we've split a fair few times already, don't again
			if (rand_chance > 125)
				rand_chance = 100000;

			// decide if this is a faint split or not
			if (irand(0,5))
				stop_height2 = org[2]-25;
			else
				stop_height2 = stop_height;

			// create split
			FXLightningSplit(self, org2, dir2, rand_chance, stop_height2);
		}
		// update the direction we want to go in
		else
		{
			// figure out values to add to dir
			VectorSet(dir_effect, flrand(-LIGHTNING_MINIMUM,LIGHTNING_MINIMUM), flrand(-LIGHTNING_MINIMUM,LIGHTNING_MINIMUM) ,0);

			// add to direction
			Vec3AddAssign(dir_effect,dir);
			// cap direction
			if (dir[1] > LIGHTNING_MAXIMUM)
				dir[1] -= LIGHTNING_MINIMUM;
			else
			if (dir[1] < -LIGHTNING_MAXIMUM)
				dir[1] += LIGHTNING_MINIMUM;

			if (dir[0] > LIGHTNING_MAXIMUM)
				dir[0] -= LIGHTNING_MINIMUM;
			else
			if (dir[0] < -LIGHTNING_MAXIMUM)
				dir[0] += LIGHTNING_MINIMUM;
		}
		// update position
		org2[2] += dir[2];
		org2[1] += dir[1];
		org2[0] += dir[0];
	}
}

// create the lightning lines
void FXCreateLightning(struct client_entity_s *self, centity_t *owner)
{
	vec3_t					org;
	vec3_t					dir;
	int						lightning_count;
	float						curAng;
	extern int				ref_soft;

	// create the lightning lines
	if (ref_soft)
		lightning_count = 1;
	else
		lightning_count = irand(2,3);

	for(curAng = 0.0F; curAng < (M_PI * 2.0F); curAng += (M_PI * 2.0F) / lightning_count )
	{
		// setup the start of a lightning line
		VectorSet(org, LIGHTNING_START * cos(curAng), LIGHTNING_START * sin(curAng),self->SpawnData);
		// setup intital direction
		VectorSet(dir, flrand(-LIGHTNING_MINIMUM,LIGHTNING_MINIMUM), flrand(-LIGHTNING_MINIMUM,LIGHTNING_MINIMUM) ,-0.5);
		// create inital line of lightning
		FXLightningSplit(self, org, dir, 60, -30.0f);
	}

	self->SpawnData -= 5;

	// if the owner of the lightning is the main client then flash the screen
	if (owner->current.number == fxi.cl->playernum + 1)
	{
		// give a quick screen flash
		fxi.Activate_Screen_Flash(0x80ffd0c0);
		// make our screen shake a bit
		// values are : a, b, c, d
		// a = amount of maximum screen shake, in pixels
		// b = duration of screen shake in milli seconds
		// c = current time - in milli seconds - if this routine is called from the server, remember this
		// d = dir of shake - see game_stats.h for definitions
		fxi.Activate_Screen_Shake(4,800, fxi.cl->time, SHAKE_ALL_DIR);
	}
}


// make the lightning effect re-occur
static qboolean FXShrineHealthThink(struct client_entity_s *self, centity_t *owner)
{

	if (!(--self->SpawnInfo))
	{
		return(false);		
	}

	// create the lightning lines
	FXCreateLightning(self, owner);

	self->updateTime = flrand(1, (TOTAL_HEALTH_EFFECTS - self->SpawnInfo)) * 100.0; 

	return(true);
}

// create initial lightning line
void FXShrineHealthEffect(centity_t *owner, int type, int flags, vec3_t origin)
{
	client_entity_t		*glow;
	// create the entity that the particles are attached to
	glow = ClientEntity_new(type, flags | CEF_NO_DRAW | CEF_ADDITIVE_PARTS, origin, 0, LIGHTNING_DUR+20);
	glow->SpawnInfo = TOTAL_HEALTH_EFFECTS;
	glow->AddToView = LinkedEntityUpdatePlacement;
	glow->Update = FXShrineHealthThink;
	glow->SpawnData = 70;
	AddEffect(owner, glow);

	// start off the lightning effect
	FXCreateLightning(glow, owner);

}

/*
----------------------------------------

Reflect effect routines

----------------------------------------
*/

// create the two circles that ring the player
static qboolean FXShrineReflectThink(struct client_entity_s *self, centity_t *owner)
{
	client_particle_t	*ce;
	int					count, i;
	float					new_y;
	float					ang;
	float					offset_ang;
	paletteRGBA_t			color;

	if (!(--self->SpawnInfo))
	{
		return(false);		
	}

	if (self->SpawnInfo >24)
	{
		// create the ring of particles that goes up
		color.c = 0xffffff;
		color.a = 0xff;

		// figure out how many particles we are going to use
		count = GetScaledCount(NUM_OF_FLIGHT_PARTS, 0.7);
		offset_ang = 6.28 / count;
		ang = self->Scale;
		for (i=0; i< count; i++)
		{
			ang += offset_ang;;

			ce = ClientParticle_new(PART_16x16_SPARK_B, color, 450);
			ce->acceleration[2] = 0.0; 
			VectorSet(ce->origin, FLIGHT_RAD * cos(ang), FLIGHT_RAD * sin(ang), self->SpawnData);
			ce->scale = 16.0F;
			AddParticleToList(self, ce);
		}
		// create the ring of particles that goes down
		new_y = -self->SpawnData;

		ang = self->Scale;
		for (i=0; i< count; i++)
		{
			ang += offset_ang;;

			ce = ClientParticle_new(PART_16x16_SPARK_B, color, 450);
			ce->acceleration[2] = 0.0; 
			VectorSet(ce->origin, FLIGHT_RAD * cos(ang), FLIGHT_RAD * sin(ang), new_y);
			ce->scale = 16.0F;
			AddParticleToList(self, ce);
		}
		// put the sparkle on us
		if(self->SpawnInfo > 10)
		{
			count = irand(3,7);
			for (i=0; i<count; i++)
			{
				ce = ClientParticle_new(PART_16x16_STAR, color, 280);
				ce->acceleration[2] = 0.0; 
				VectorSet(ce->origin, flrand(-FLIGHT_PLAY_RAD,FLIGHT_PLAY_RAD), flrand(-FLIGHT_PLAY_RAD,FLIGHT_PLAY_RAD), flrand(-30,30) );
				ce->scale = 0.3;
				ce->d_scale = flrand(40.0F, 60.0f);
				AddParticleToList(self, ce);
			}
		}

	}

	// move the rings up/down next frame
	self->SpawnData += FLIGHT_HEIGHT_ADD;
	self->Scale += 0.15;

	return(true);
}

// create the entity the flight loops are on
void FXShrineReflectEffect(centity_t *owner, int type, int flags, vec3_t origin)
{
	client_entity_t		*glow;
														  
	glow = ClientEntity_new(type, flags | CEF_NO_DRAW | CEF_ADDITIVE_PARTS, origin, 0, 25);

	VectorClear(glow->origin);
	glow->Update = FXShrineReflectThink;
	glow->SpawnInfo = TOTAL_FLIGHT_EFFECTS;
	glow->AddToView = LinkedEntityUpdatePlacement;
	glow->SpawnData = FLIGHT_EFFECTS_START_HEIGHT;
	glow->Scale = 0;
	
	AddEffect(owner, glow);

}

/*
----------------------------------------

Ghosting effect Routines

----------------------------------------
*/

// make the glow go away
static qboolean FXShrineGlowThink(struct client_entity_s *self, centity_t *owner)
{
	if (!(--self->SpawnInfo))
	{
		return(false);		
	}

	self->d_alpha = -0.45;
 	self->d_scale = -1.0;
	return(true);
}

// create the little glow bits..
static qboolean FXShrineGhostThink(struct client_entity_s *self, centity_t *owner)
{
	int					i;
	client_entity_t		*glow;
	vec3_t				origin;

	if (!(--self->SpawnInfo))
	{
		return(false);		
	}

	for( i=0; i< irand(3,5); i++)
	{
		VectorSet(origin, flrand(-20, 20), flrand(-20,20), flrand(-30,40)); 

		glow = ClientEntity_new(FX_SHRINE_GHOST, CEF_OWNERS_ORIGIN, origin, 0, 300);

		glow->r.model = shrine_models;
		glow->r.flags |= RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		glow->SpawnInfo = 2;
		glow->AddToView = OffsetLinkedEntityUpdatePlacement;
	 	glow->d_scale = flrand(0.5, 2.0);
	 	glow->r.scale = 0.1;
		glow->d_alpha = 1.0;
		glow->alpha = 0.1;
		glow->Update = FXShrineGlowThink;
		
		AddEffect(owner, glow);
	}
	return(true);
}

// create the inital ghost controlling entity
void FXShrineGhostEffect(centity_t *owner, int type, int flags, vec3_t origin)
{
	client_entity_t		*glow;
	glow = ClientEntity_new(type, flags|CEF_NO_DRAW, origin, 0, 70);

	glow->SpawnInfo = 20;
	glow->Update = FXShrineGhostThink;
	
	AddEffect(owner, glow);
}

/*
----------------------------------------

Speed effect routine

----------------------------------------
*/

// create the two circles that ring the player
static qboolean FXShrineSpeedThink(struct client_entity_s *self, centity_t *owner)
{
	client_particle_t	*ce;
	int					count, i;
	float					ang;
	float					offset_ang;
	vec3_t				angles;
	paletteRGBA_t			color;

	if (!(--self->SpawnInfo))
	{
		return(false);		
	}

	color.c = 0xffffff;
	color.a = 0xff;

	// figure out how many particles we are going to use
	count = GetScaledCount(NUM_OF_REFLECT_PARTS, 0.7);
	offset_ang = 6.28 / count;
	ang = 0;
	for (i=0; i< count; i++)
	{
		ce = ClientParticle_new(PART_32x32_STEAM, self->r.color, 380);
		ce->acceleration[2] = 0.0; 
		VectorSet(angles, ang, self->Scale, 0);
		DirFromAngles(angles, ce->origin);
		Vec3ScaleAssign(REFLECT_RAD, ce->origin);
		ce->scale = 7.0F;
		if (self->SpawnInfo <18)
			ce->color.a = (self->SpawnInfo) * (255 / 18);
		AddParticleToList(self, ce);
		ang += offset_ang;;
	}
	self->Scale += 0.25;

	// put the sparkle on us
	if(self->SpawnInfo > 10)
	{
		count = irand(3,7);
		for (i=0; i<count; i++)
		{
			ce = ClientParticle_new(PART_16x16_STAR, color, 280);
			ce->acceleration[2] = 0.0; 
			VectorSet(ce->origin, flrand(-FLIGHT_PLAY_RAD,FLIGHT_PLAY_RAD), flrand(-FLIGHT_PLAY_RAD,FLIGHT_PLAY_RAD), flrand(-30,30) );
			ce->scale = 0.3;
			ce->d_scale = flrand(40.0F, 60.0f);
			AddParticleToList(self, ce);
		}
	}

	return(true);
}

void FXShrineSpeedEffect(centity_t *owner, int type, int flags, vec3_t origin)
{
	client_entity_t		*glow;
														  
	glow = ClientEntity_new(type, flags | CEF_NO_DRAW, origin, 0, 30);

	VectorClear(glow->origin);
	glow->Update = FXShrineSpeedThink;
	glow->SpawnInfo = TOTAL_REFLECT_EFFECTS;
	glow->AddToView = LinkedEntityUpdatePlacement;
	glow->SpawnData = REFLECT_EFFECTS_START_HEIGHT;
	glow->r.color.c = 0x604040;
	glow->r.color.a = 255;
	glow->Scale = 0;
	
	AddEffect(owner, glow);
}


/*
----------------------------------------

Weapons Power Up effect routine

----------------------------------------
*/

// create the two circles that ring the player
static qboolean FXShrinePowerupThink(struct client_entity_s *self, centity_t *owner)
{
	client_particle_t	*ce;
	int					count, i;
	float					ang;
	float					offset_ang;
	paletteRGBA_t			color;

	if (!(--self->SpawnInfo))
	{
		return(false);		
	}

	if (self->SpawnInfo >24)
	{
		// create the ring of particles that goes up

		color.c = 0xffffff;
		color.a = 0xff;

		// figure out how many particles we are going to use
		count = GetScaledCount(NUM_OF_POWERUP_PARTS, 0.7);
		offset_ang = 6.28 / count;
		ang = 0;
		for (i=0; i< count; i++)
		{
			ce = ClientParticle_new(PART_16x16_SPARK_G, color, 350);
			ce->acceleration[2] = 0.0; 
			VectorSet(ce->origin, POWERUP_RAD * cos(ang), POWERUP_RAD * sin(ang), self->SpawnData);
			ce->scale = 12.0F;
			AddParticleToList(self, ce);
			VectorScale(ce->origin, 25, ce->velocity);
			ce->velocity[2] = 0;
			ang += offset_ang;;
		}

		// put the sparkle on us
		if(self->SpawnInfo > 10)
		{
			count = irand(3,7);
			for (i=0; i<count; i++)
			{
				ce = ClientParticle_new(PART_16x16_STAR, color, 280);
				ce->acceleration[2] = 0.0; 
				VectorSet(ce->origin, flrand(-FLIGHT_PLAY_RAD,FLIGHT_PLAY_RAD), flrand(-FLIGHT_PLAY_RAD,FLIGHT_PLAY_RAD), flrand(-30,30) );
				ce->scale = 0.3;
				ce->d_scale = flrand(40.0F, 60.0f);
				AddParticleToList(self, ce);
			}
		}

	}

	// move the rings up/down next frame
	self->SpawnData += POWERUP_HEIGHT_ADD;

	return(true);
}

void FXShrinePowerUpEffect(centity_t *owner, int type, int flags, vec3_t origin)
{
	client_entity_t		*glow;
														  
	glow = ClientEntity_new(type, flags | CEF_NO_DRAW | CEF_ADDITIVE_PARTS, origin, 0, 75);

	glow->Update = FXShrinePowerupThink;
	glow->SpawnInfo = TOTAL_POWERUP_EFFECTS;
	glow->AddToView = LinkedEntityUpdatePlacement;
	glow->SpawnData = POWERUP_EFFECTS_START_HEIGHT;
	
	AddEffect(owner, glow);
}

/*
----------------------------------------

Persistant shrine ball effect

----------------------------------------
*/

enum
{
	SHRINEBALL_HEAL = 0,
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



static short ShrineParticle[12][2] =
{
	{	PART_16x16_SPARK_I,		PART_16x16_LIGHTNING	},	// SHRINEBALL_HEAL,
	{	PART_16x16_SPARK_G,		PART_16x16_SPARK_B		},	// SHRINEBALL_MANA,
	{	PART_16x16_WATERDROP,	PART_32x32_WFALL	},	// SHRINEBALL_LUNGS,
	{	PART_4x4_WHITE,			PART_32x32_ALPHA_GLOBE	},	// SHRINEBALL_LIGHT,
	{	PART_16x16_SPARK_G,		PART_16x16_SPARK_G		},	// SHRINEBALL_POWERUP,
	{	PART_8x8_BLUE_DIAMOND,	PART_8x8_BLUE_X			},	// SHRINEBALL_ARMOR,
	{	PART_16x16_SPARK_Y,		PART_16x16_SPARK_Y		},	// SHRINEBALL_ARMOR_GOLD,
	{	PART_8x8_RED_X,			PART_8x8_CYAN_DIAMOND	},	// SHRINEBALL_RANDOM
	{	PART_8x8_CYAN_DIAMOND,	PART_8x8_CYAN_X			},	// SHRINEBALL_REFLECT,
	{	PART_16x16_SPARK_C,		PART_32x32_FIREBALL		},	// SHRINEBALL_STAFF
	{	PART_32x32_WFALL,		PART_16x16_STAR			},	// SHRINEBALL_GHOST,
	{	PART_32x32_STEAM,		PART_32x32_STEAM		},	// SHRINEBALL_SPEED,
};


// make the shrine glow ball effect shimmer, and give off steam
static qboolean FXShrineBallThink(struct client_entity_s *self, centity_t *owner)
{
	client_particle_t		*ce;
	paletteRGBA_t			color;
	int						part = 0;
	vec3_t					rad, fwd;
	int						i;
	int						count;

	count = GetScaledCount(BALL_PART_NUM, 0.4);

	for (i=0; i<count; i++)
	{
		assert(self->SpawnInfo >= 0 && self->SpawnInfo < SHRINEBALL_MAX);
		if (self->SpawnInfo == SHRINEBALL_RANDOM)
		{
			part = irand(PART_16x16_SPARK_B, PART_16x16_SPARK_Y);
		}
		else
			part = ShrineParticle[self->SpawnInfo][irand(0,1)];

		if (part == PART_32x32_STEAM)
	  		color.c = 0xffffff40;
		else
			color.c = 0xffffffff;

		ce = ClientParticle_new(part, color, 1000);

		if(part != PART_32x32_WFALL && part != PART_16x16_WATERDROP && part != PART_32x32_STEAM)
			ce->type |= PFL_ADDITIVE;

		rad[PITCH] = flrand(0, 360.0);
		rad[YAW] = flrand(0, 360.0);
		rad[ROLL] = 0.0;
		AngleVectors(rad, fwd, NULL, NULL);
		VectorScale(fwd, BALL_RAD, ce->velocity);
		VectorScale(ce->velocity, -1.0, ce->acceleration);
		ce->color.a = 245;
		ce->scale = BALL_PART_SCALE;
		ce->d_scale = -0.5*BALL_PART_SCALE;
		AddParticleToList(self, ce);
	}

	self->updateTime = 150;
	return(true);
}

// create the floating ball in the middle of the shrine
void FXShrineBall(centity_t *owner, int type, int flags, vec3_t origin)
{
	client_entity_t		*glow;
	vec3_t				offset;	
	byte				shrinetype;
	// go get the normalised direction of the shrine object
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_SHRINE_BALL].formatString, &offset, &shrinetype);

	// don't put out effects for shrines that don't exist
	if(shrinetype >= SHRINEBALL_MAX)
		return;

	// move our starting point out a bit
	Vec3ScaleAssign(25,offset);
	offset[2] = -10;
	Vec3AddAssign(offset, origin); 

	// create the dummy entity, so particles can be attached
	// | CEF_ADDITIVE_PARTS
	glow = ClientEntity_new(type, (flags | CEF_NO_DRAW | CEF_VIEWSTATUSCHANGED) & ~CEF_NOMOVE , origin, 0, 20);
	glow->SpawnInfo = shrinetype;
	glow->Update = FXShrineBallThink;
	glow->radius = 100;

	AddEffect(owner, glow);
}

/*
----------------------------------------

Exploding shrine ball effect

----------------------------------------
*/

// explode the ball in the middle of the shrine
void FXShrineBallExplode(centity_t *owner, int type, int flags, vec3_t origin)
{
	client_particle_t	*ce;
	client_entity_t		*burst;
	vec3_t				offset;	
	int					i, count;
	paletteRGBA_t		color;
	byte				shrinetype;
	int					part;
	vec3_t				rad, fwd;

	color.c = 0xffffffff;

	// go get the normalised direction of the shrine object
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_SHRINE_BALL_EXPLODE].formatString, &offset, &shrinetype);

	// move our starting point out a bit
	Vec3ScaleAssign(25,offset);
	offset[2] = -10;
	Vec3AddAssign(offset, origin); 

	// create the dummy entity, so particles can be attached
	burst = ClientEntity_new(type, (flags | CEF_NO_DRAW | CEF_CHECK_OWNER) & ~CEF_NOMOVE , origin, 0, 1200);
	burst->radius = 100;
	AddEffect(owner, burst);

	assert(shrinetype >= 0 && shrinetype <= SHRINEBALL_MAX);
	count = GetScaledCount(BALL_EX_PART_NUM, 0.4);
	// create a bunch of exploding particles 
	for (i=0; i< count; i++)
	{
		rad[PITCH] = flrand(0, 360.0);
		rad[YAW] = flrand(0, 360.0);
		rad[ROLL] = 0.0;

		if (shrinetype == SHRINEBALL_RANDOM)
		{
			part = irand(PART_16x16_SPARK_B, PART_16x16_SPARK_Y);
		}
		else 
			part = ShrineParticle[shrinetype][irand(0,1)];

		ce = ClientParticle_new(part, color, 1150);

		if(part != PART_32x32_WFALL && part != PART_16x16_WATERDROP)
			ce->type |= PFL_ADDITIVE;
		
		AngleVectors(rad, fwd, NULL, NULL);
		VectorScale(fwd, BALL_RAD, ce->velocity);
		VectorScale(ce->velocity, -0.7, ce->acceleration);
		ce->color.a = 245;
		ce->scale = BALL_PART_SCALE;
		ce->d_scale = -0.5*BALL_PART_SCALE;
		AddParticleToList(burst, ce);
	}

	// Add an additional flash as well.
	// ...and a big-ass flash
	burst = ClientEntity_new(-1, flags, origin, NULL, 250);
	burst->r.model = shrine_models + 1;
	burst->r.flags |= RF_TRANS_ADD | RF_TRANS_ADD_ALPHA | RF_TRANSLUCENT;// | RF_FULLBRIGHT;
	burst->r.frame = 1;
	burst->radius=64;
	burst->r.scale=1.0;
	burst->d_alpha=-4.0;
	burst->d_scale=-4.0;
	AddEffect(NULL, burst);
}


