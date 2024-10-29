#include "Client Effects.h"
#include "Client Entities.h"
#include "Particle.h"
#include "ResourceManager.h"
#include "FX.h"
#include "Vector.h"
#include "Random.h"
#include "Utilities.h"
#include "q_Sprite.h"

#define NUM_EXPLODE_PARTS	256
#define EXP_RANGE			16.0
#define EXP_SPEED			192.0

static struct model_s *armorhit_models[1];

void PreCacheArmorHit(void)
{
	armorhit_models[0] = fxi.RegisterModel("sprites/fx/firestreak.sp2");
}


// we hit someone with armor - do a pretty effect
// ripped off unashamedly from Josh's extremely cool streak effect. One of the coolest effects I've seen in a long time Josh. Good work Dude.
void FXCreateArmorHit(centity_t *owner,int Type,int Flags,vec3_t Origin)
{
	client_entity_t	*TrailEnt;
	vec3_t			dir;
	int				i;

	fxi.GetEffect(owner,Flags,clientEffectSpawners[FX_ARMOR_HIT].formatString, &dir);

   	//Spawn a hit explosion of lines
   	i = GetScaledCount(6, 0.85);

   	while (i--)
   	{
   		TrailEnt=ClientEntity_new(Type, Flags & ~CEF_NO_DRAW, Origin, 0, 500);

   		TrailEnt->r.model = armorhit_models;
   		
   		TrailEnt->r.spriteType = SPRITE_LINE;

   		TrailEnt->r.flags |= RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
   		TrailEnt->r.color.c = 0xFFFFFFFF;
   		TrailEnt->r.scale = flrand(1.0, 2.5);
   		TrailEnt->alpha = flrand(1.0, 0.75);
   		TrailEnt->d_alpha = -2.0;
   		TrailEnt->d_scale = -1.0;

   		TrailEnt->r.color.r = irand(128, 255);
   		TrailEnt->r.color.g = irand(64, 255);
   		TrailEnt->r.color.b = irand(64, 255);
   		TrailEnt->r.color.a = 64 + irand(16, 128);

   		VectorRandomCopy(dir, TrailEnt->velocity, 1.0);
   		
   		VectorCopy(Origin, TrailEnt->r.endpos);
   		VectorMA(TrailEnt->r.endpos, irand(6,8), TrailEnt->velocity, TrailEnt->r.startpos);

   		VectorScale(TrailEnt->velocity, irand(50,100), TrailEnt->velocity);

   		AddEffect(NULL, TrailEnt);
	}

}

void CreateExplosionParticles(client_entity_t *this)
{
	int					i, count;
	client_particle_t	*p;
	paletteRGBA_t		color;

	count = GetScaledCount(NUM_EXPLODE_PARTS, 0.9);
	for(i = 0; i < count; i++)
	{
		color.r = irand(127, 255);
		color.g = irand(127, 255);
		color.b = 0;
		color.a = 255;

		p = ClientParticle_new(PART_4x4_WHITE, color, 500);

		VectorSet(p->origin, flrand(-EXP_RANGE, EXP_RANGE), flrand(-EXP_RANGE, EXP_RANGE), flrand(-EXP_RANGE, EXP_RANGE));
		VectorSet(p->velocity, flrand(-EXP_SPEED, EXP_SPEED), flrand(-EXP_SPEED, EXP_SPEED), flrand(-EXP_SPEED, EXP_SPEED));

		AddParticleToList(this, p);
	}
}

void GenericExplosion1(centity_t *owner, int type, int flags, vec3_t origin)
{
	client_entity_t *effect;

	effect = ClientEntity_new(type, flags, origin, NULL, 500);

	effect->alpha = 0.75;
	effect->flags |= CEF_NO_DRAW;

	AddEffect(NULL, effect);	// add the effect as independent world effect

	CreateExplosionParticles(effect);
}

void GenericExplosion2(centity_t *owner, int type, int flags, vec3_t origin)
{
	client_entity_t *effect;

	effect = ClientEntity_new(type, flags, origin, NULL, 500);

	effect->flags |= CEF_NO_DRAW;

	AddEffect(NULL, effect);	// add the effect as independent world effect

	CreateExplosionParticles(effect);
}

// end