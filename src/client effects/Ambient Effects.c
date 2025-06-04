//
// Ambient Effects.c
//
// Copyright 1998 Raven Software
//

#include "Ambient Effects.h"
#include "Client Effects.h"
#include "Particle.h"
#include "Random.h"

void DoWaterSplash(client_entity_t* effect, const paletteRGBA_t color, int count)
{
	count = min(500, count);

	for (int i = 0; i < count; i++)
	{
		client_particle_t* p = ClientParticle_new(PART_16x16_WATERDROP, color, 1000);

		p->d_alpha = 0.0f;
		p->d_scale = -1.0f;
		p->velocity[0] = flrand(-20.0f, 20.0f);
		p->velocity[1] = flrand(-20.0f, 20.0f);
		p->velocity[2] = flrand(20.0f, 30.0f);

		AddParticleToList(effect, p);
	}
}

void WaterSplash(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	int cnt;
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_SPLASH].formatString, &cnt);

	client_entity_t* effect = ClientEntity_new(type, flags, origin, NULL, 500);
	effect->flags |= CEF_NO_DRAW | CEF_NOMOVE;

	AddEffect(NULL, effect);

	DoWaterSplash(effect, color_white, cnt); //mxd. Use color_white.
}

#if 0 //TODO: unused. Implement? Use Q2 fly logic as reference?

#define FLY_EFFECT_THINK_TIME 100 // creates new flies every 0.1 sec

// This needs work. . .
qboolean CreateFlyParticles(client_entity_t *this, centity_t *owner)
{
#define NUMVERTEXNORMALS	162
#define	BEAMLENGTH			16
//	int		n;
	int		count;
//	int		starttime;

//	int fly_stoptime;

	int			i;
	client_particle_t *p;
	float		angle;
	float		sr, sp, sy, cr, cp, cy;
	vec3_t		forward;
	float		dist = 64;
	float		ltime;
	static float avertexnormals [NUMVERTEXNORMALS][3] =
	{
	#include "anorms.h"
	};
	static vec3_t avelocities [NUMVERTEXNORMALS];

/*	fly_stoptime = this->startTime + this->duration;

	if (fly_stoptime < fxi.cl->time)
	{
		starttime = fxi.cl->time;
		fly_stoptime = fxi.cl->time + 60000;
	}
	else
	{
		starttime = fly_stoptime - 60000;
	}

	n = fxi.cl->time - starttime;
	if (n < 20000)
		count = n * 162 / 20000.0;
	else
	{
		n = fly_stoptime - fxi.cl->time;
		if (n < 20000)
			count = n * 162 / 20000.0;
		else
			count = 162;
	}*/

	//

//	if (count > NUMVERTEXNORMALS)
		count = NUMVERTEXNORMALS;

	if (!avelocities[0][0])
	{
		for (i=0 ; i<NUMVERTEXNORMALS*3 ; i++)
			avelocities[0][i] = flrand(0.0, 2.55);
	}

	ltime = (float)fxi.cl->time / 1000.0;
	for (i=0 ; i<count ; i+=2)
	{
		p = ResMngr_AllocateResource(&ParticleMngr, sizeof(*this->p_root));

		AddParticleToList(this, p);

		angle = ltime * avelocities[i][0];
		sy = sin(angle);
		cy = cos(angle);
		angle = ltime * avelocities[i][1];
		sp = sin(angle);
		cp = cos(angle);
		angle = ltime * avelocities[i][2];
		sr = sin(angle);
		cr = cos(angle);
	
		forward[0] = cp*cy;
		forward[1] = cp*sy;
		forward[2] = -sp;

		p->startTime = fxi.cl->time;
		p->duration = 50;

		dist = sin(ltime + i)*64;
		p->origin[0] = avertexnormals[i][0]*dist + forward[0]*BEAMLENGTH;
		p->origin[1] = avertexnormals[i][1]*dist + forward[1]*BEAMLENGTH;
		p->origin[2] = avertexnormals[i][2]*dist + forward[2]*BEAMLENGTH;

		VectorClear (p->velocity);
		VectorClear (p->acceleration);

		p->color.c = 0x000000FF;

		p->d_alpha = -100;

		p->scale = 1.0;
		p->d_scale = 0.0;
	}

	return true; // never goes away. . .
}

void FlyEffect(centity_t *owner, int type, int flags, vec3_t origin)
{
	client_entity_t *effect;

	effect = ClientEntity_new(type, flags, origin, NULL, FLY_EFFECT_THINK_TIME);

	effect->flags |= CEF_NO_DRAW;

	effect->Update = CreateFlyParticles;

	AddEffect(owner, effect);
}

#endif