//
// fx_debris.c
//
// Heretic II
// Copyright 1998 Raven Software
//

#include "ce_DefaultMessageHandler.h"
#include "Client Effects.h"
#include "Client Entities.h"
#include "Particle.h"
#include "ResourceManager.h"
#include "FX.h"
#include "FX_DustPuff.h"
#include "Vector.h"
#include "Utilities.h"
#include "Angles.h"
#include "random.h"
#include "q_Physics.h"
#include "fx_debris.h"
#include "ce_DLight.h"
#include "g_playstats.h"
#include "Vector.h"

client_entity_t *FXDebris_Throw(vec3_t origin, int material, vec3_t dir, float ke,float scale, int flags, qboolean altskin);
client_entity_t *DoBloodSplash(vec3_t loc, int amount, qboolean yellow_blood);
void ThrowBlood(vec3_t origin, vec3_t tnormal, qboolean dark, qboolean yellow, qboolean trueplane);

void DoBloodTrail(client_entity_t *spawner, int amount);
void GenericSparks(centity_t *owner, int type, int flags, vec3_t origin, vec3_t dir);

static void FXDebris_Collision(client_entity_t *self, CE_Message_t *msg);
static void FXBodyPart_Throw(centity_t *owner, int BodyPart, vec3_t origin, float ke, int frame, int type, byte modelindex, int flags, centity_t *harpy);

qboolean FXDebris_Vanish(struct client_entity_s *self,centity_t *owner);
qboolean FXBodyPartAttachedUpdate(struct client_entity_s *self, centity_t *owner);

extern int ref_soft;
//------------------------------------------------------------------
//	FX Debris base info
//------------------------------------------------------------------

typedef struct DebrisChunk
{
	char	*modelName;
	int		skinNum;
	struct model_s *model;
	float	mass;
} DebrisChunk_t;

int debrisChunkOffsets[NUM_MAT+1]=
{
	0,	// Stone.
	4,	// Grey stone.
	8,	// Cloth.
	12,	// Metal.
	16,	// Flesh.
	22,	// Pottery.
	26,	// Glass.
	31,	// Leaf.
	34,	// Wood
	38,	// Brown Stone
	42,	// Nothing - just smoke
	43,	// Insect chunks
	51,	// one beyond
};

DebrisChunk_t debrisChunks[]=
{
	// Grey stone.

	{"models/debris/stone/schunk1/tris.fm", 0, NULL, 3.0},//0
	{"models/debris/stone/schunk2/tris.fm", 0, NULL, 3.0},
	{"models/debris/stone/schunk3/tris.fm", 0, NULL, 3.0},
	{"models/debris/stone/schunk4/tris.fm", 0, NULL, 3.0},

	// Grey stone.

	{"models/debris/stone/schunk1/tris.fm", 0, NULL, 3.0},//4
	{"models/debris/stone/schunk2/tris.fm", 0, NULL, 3.0},
	{"models/debris/stone/schunk3/tris.fm", 0, NULL, 3.0},
	{"models/debris/stone/schunk4/tris.fm", 0, NULL, 3.0},

	// Cloth.
	{"models/debris/pottery/pot1/tris.fm", 0, NULL, 0.2},//8
	{"models/debris/pottery/pot1/tris.fm", 0, NULL, 0.2},
	{"models/debris/pottery/pot1/tris.fm", 0, NULL, 0.3},
	{"models/debris/pottery/pot1/tris.fm", 0, NULL, 0.4},

	// Metal.

	{"models/debris/metal/mchunk1/tris.fm", 0, NULL, 2.0},//12
	{"models/debris/metal/mchunk2/tris.fm", 0, NULL, 3.0},
	{"models/debris/metal/mchunk3/tris.fm", 0, NULL, 4.0},
	{"models/debris/metal/mchunk4/tris.fm", 0, NULL, 5.0},

	// Flesh.

	{"models/debris/meat/chunk1/tris.fm", 0, NULL, 3.0},//16
	{"models/debris/meat/chunk2/tris.fm", 0, NULL, 3.0},
	{"models/debris/meat/chunk3/tris.fm", 0, NULL, 3.0},
	{"models/debris/meat/chunk4/tris.fm", 0, NULL, 3.0},
	{"models/debris/meat/chunk5/tris.fm", 0, NULL, 3.0},
	{"models/debris/meat/chunk6/tris.fm", 0, NULL, 3.0},

	// Pottery.

	{"models/debris/pottery/pot1/tris.fm", 0, NULL, 2.0},//22
	{"models/debris/pottery/pot2/tris.fm", 0, NULL, 3.0},
	{"models/debris/pottery/pot3/tris.fm", 0, NULL, 2.5},
	{"models/debris/pottery/pot4/tris.fm", 0, NULL, 1.4},

	// Glass - models need different skins
	{"models/debris/wood/splinter1/tris.fm", 0, NULL, 1.8},//26
	{"models/debris/wood/splinter2/tris.fm", 0, NULL, 2.0},
	{"models/debris/wood/splinter3/tris.fm", 0, NULL, 1.9},
	{"models/debris/wood/splinter4/tris.fm", 0, NULL, 1.6},
	{"models/debris/wood/splinter1/tris.fm", 0, NULL, 1.8},


	// Leaf - invalid debris type.
	{"models/debris/pottery/pot1/tris.fm", 0, NULL, 2.0},//31
	{"models/debris/pottery/pot1/tris.fm", 0, NULL, 2.0},
	{"models/debris/pottery/pot1/tris.fm", 0, NULL, 2.0},

	// Wood chunks.
	{"models/debris/wood/splinter1/tris.fm", 0, NULL, 1.8},//34
	{"models/debris/wood/splinter2/tris.fm", 0, NULL, 2.0},
	{"models/debris/wood/splinter3/tris.fm", 0, NULL, 1.9},
	{"models/debris/wood/splinter4/tris.fm", 0, NULL, 1.6},

	// Brown stone.
	{"models/debris/stone/schunk1/tris.fm", 1, NULL, 3.0},//38
	{"models/debris/stone/schunk2/tris.fm", 1, NULL, 3.0},
	{"models/debris/stone/schunk3/tris.fm", 1, NULL, 3.0},
	{"models/debris/stone/schunk4/tris.fm", 1, NULL, 3.0},

	// Nothing - just smoke
	{"models/debris/meat/chunk6/tris.fm", 0, NULL, 3.0},//42

	//Insect Chunks - fixme, use diff skins for diff bugs...
	{"models/debris/pottery/pot1/tris.fm", 2, NULL, 2.0},//43
	{"models/debris/pottery/pot2/tris.fm", 2, NULL, 3.0},
	{"models/debris/pottery/pot3/tris.fm", 2, NULL, 2.5},
	{"models/debris/pottery/pot4/tris.fm", 2, NULL, 1.4},
	{"models/debris/insect/chunk1/tris.fm", 0, NULL, 2.0},
	{"models/debris/insect/chunk2/tris.fm", 0, NULL, 3.0},
	{"models/debris/insect/chunk3/tris.fm", 0, NULL, 2.5},
	{"models/debris/insect/chunk4/tris.fm", 0, NULL, 1.4},
};

float debrisElasticity[NUM_MAT] =
{
	1.3,		// Stone
	1.3,		// Grey Stone
	1.1,		// Cloth
	1.4,		// Metal
	1.2,		// Flesh
	1.3,		// Pottery
	1.3,		// Glass
	1.1,		// Leaf
	1.5,		// Wood
	1.3,		// Brown Stone
	1.,			// Nothing - just smoke
	1.3,		// Insect chunks
};

void InitDebrisStatics()
{
	classStatics[CID_DEBRIS].msgReceivers[MSG_COLLISION] = FXDebris_Collision;
}

void PreCacheDebris()
{
	int i, j, offset;

	for(j = 0; j < NUM_MAT; ++j)
	{
		offset = debrisChunkOffsets[j+1];
		for(i = debrisChunkOffsets[j]; i < offset; ++i)
		{
			debrisChunks[i].model = fxi.RegisterModel(debrisChunks[i].modelName);
		}
	}
}

qboolean flame_out (struct client_particle_s *self, int crap)
{
	if((fxi.cl->time - self->startTime) < (self->duration/2))
	{
		if(!irand(0, 9))
		{
			self->type = irand(PART_16x16_FIRE1, PART_16x16_FIRE3);
			self->type |= PFL_ADDITIVE;//whoo-hoo!  Additive particle!
		}
		return true;
	}

	self->scale += flrand(0.6, 0.8);
	self->type = PART_16x16_MIST;//automatically removes additive drawing
	self->color.r = 10;
	self->color.g = 10;
	self->color.b = 10;
	self->color.a = 255;
//	self->extraUpdate = NULL;
	return true;
}

void DoFireTrail (client_entity_t *spawner)
{
	client_particle_t	*flame;
	int					i, material, count;
	float				radius, master_scale;
	paletteRGBA_t	color;
	float				flame_duration;

	if(spawner->flags&CEF_FLAG7 && spawner->dlight)
	{
		if(spawner->dlight->intensity<=0)
		{//flame out
			spawner->flags &= ~CEF_FLAG6;
			return;
		}
	}

	color.c = 0xe5007fff;

	material = spawner->SpawnInfo & SIF_FLAG_MASK;
	if(material == MAT_FLESH || material == MAT_INSECT || spawner->effectID == FX_BODYPART)
		master_scale = spawner->r.scale * 3.33;
	else	
		master_scale = spawner->r.scale;

	if (r_detail->value < DETAIL_NORMAL)
		flame_duration = 700.0;
	else
		flame_duration = 1000.0;

	count = GetScaledCount(irand(2, 5), 0.3);
	for(i = 0; i < count; i++)
	{
		flame = ClientParticle_new(irand(PART_16x16_FIRE1, PART_16x16_FIRE3), color, flame_duration);

		radius = spawner->r.scale * 2.0;
		VectorSet(flame->origin, flrand(-radius, radius), flrand(-radius, radius), flrand(-4.0F, -2.0F) * spawner->r.scale);
		VectorAdd(flame->origin, spawner->r.origin, flame->origin);

		flame->scale = master_scale;
		VectorSet(flame->velocity, flrand(-1.0, 1.0), flrand(-1, 1.0), flrand(17.0, 20.0));
		flame->acceleration[2] = 32.0 * spawner->r.scale;
		flame->d_scale = flrand(-5.0, -2.5);
		flame->d_alpha = flrand(-200.0, -160.0);

//		flame->extraUpdate = flame_out;

		flame->duration = (255.0 * flame_duration) / -flame->d_alpha;		// time taken to reach zero alpha

		flame->startTime = fxi.cl->time;

		flame->type |= PFL_ADDITIVE;//whoo-hoo!  Additive particle!
		AddParticleToList(spawner, flame);
	}
}

//------------------------------------------------------------------
//	FX Body Part spawn functions
//------------------------------------------------------------------

void FXBodyPart(centity_t *owner,int type, int flags, vec3_t origin)
{
	short			frame;
	short			BodyPart;
	byte			damage;
	byte			modelindex, OwnerEntnum;
	centity_t		*realowner;
	float			ke;

	//increase count on owner so that can have multiple effects?
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_BODYPART].formatString, &frame, &BodyPart, &damage, &modelindex, &OwnerEntnum);

	realowner = &fxi.server_entities[OwnerEntnum];
	
	ke = damage * 10000.0f;
	if(ke < 10000.0f)			
	{
		ke = 10000.0f;
	}

	FXBodyPart_Spawn(realowner, BodyPart, origin, ke, frame, type, modelindex, flags, owner);
}

void FXBodyPart_Spawn(centity_t *owner, int BodyPart, vec3_t origin, float ke, int frame, int type, byte modelindex, int flags, centity_t *harpy)
{
	vec3_t				spray_org;

	FXBodyPart_Throw(owner, BodyPart, origin, ke, frame, type, modelindex, flags, harpy);

	VectorAdd(origin, owner->origin, spray_org);

	if(ke && type!=FX_THROWWEAPON)
	{
		DoBloodSplash(origin, 5, flags&CEF_FLAG8);
	}
}

static void FXBodyPart_Throw(centity_t *owner, int BodyPart, vec3_t origin, float ke, int frame, int type, byte modelindex, int flags, centity_t *harpy)
{//FIXME: make sure parts have correct skins, even node 0!
	int				index, whichnode, material, node_num;
	client_entity_t	*debris;
	vec3_t			start;
	trace_t			trace;
	vec3_t			dir = {0, 0, 1};

	debris = ClientEntity_new(type, 0, origin, NULL, 17);//flags sent as 0

	if(type == FX_THROWWEAPON)
		material = MAT_METAL;//not elastic enough for effect?
	else
	{
		if(flags&CEF_FLAG8)//insect material type
			material = MAT_INSECT;
		else
			material = MAT_FLESH;
	}

	debris->SpawnInfo = material;
	//FIXME: when do I run out of these?- ie, when is debris invalid?

	debris->classID = CID_DEBRIS;
	debris->msgHandler = CE_DefaultMsgHandler;
	if(modelindex == 255)//FIXME: these will tilt up and down with the player's torso!!!
	{	// Player special model.
		debris->r.model = fxi.cl->clientinfo[owner->baseline.number-1].model;
		if (!debris->r.model)
			debris->r.model = fxi.cl->baseclientinfo.model;
	}
	else
	{
		debris->r.model = &fxi.cl->model_draw[modelindex];
	}
	debris->r.fmnodeinfo = FMNodeInfo_new();
	debris->r.frame = frame;//first frame should be parts frame of a flexmodel
//need to copy base skin also
	debris->r.skinnum = owner->current.skinnum;
		
	for(whichnode = 1, node_num = 0; whichnode<=16384; whichnode*=2)//bitwise
	{
		node_num++;
		if(!((int)(BodyPart)&(int)(whichnode)))
		{
			debris->r.fmnodeinfo[node_num].flags |= FMNI_NO_DRAW;
		}
		else
		{
			debris->r.fmnodeinfo[node_num] = owner->current.fmnodeinfo[node_num];//copy skins and flags and colors
			debris->r.fmnodeinfo[node_num].flags &= ~FMNI_NO_DRAW;
		}
	}
	//turn off first node always?
	if(modelindex != 255 || (modelindex == 255 && !(BodyPart & 1)))
		debris->r.fmnodeinfo[0].flags |= FMNI_NO_DRAW;
	else
	{
		debris->r.fmnodeinfo[0] = owner->current.fmnodeinfo[0];//copy skins and flags and colors
		debris->r.fmnodeinfo[0].flags &= ~FMNI_NO_DRAW;
	}

	debris->flags |= (CEF_CLIP_TO_WORLD | CEF_ABSOLUTE_PARTS);
	index = irand(debrisChunkOffsets[material], debrisChunkOffsets[material + 1] - 1);
	if(owner->entity)
		debris->r.skinnum = owner->entity->skinnum;
	else
		debris->r.skinnum = 0;

	debris->radius = 2.0;

	if(harpy)//HACK: harpy took it!
	{
		debris->flags |= CEF_OWNERS_ORIGIN;
		debris->Update = FXBodyPartAttachedUpdate;
		debris->updateTime = 50;
		AddEffect(harpy, debris);
		return;
	}

	VectorRandomCopy(dir, debris->velocity, 0.5F);
	
	if(ke)
	{
		Vec3ScaleAssign(sqrt(ke/debrisChunks[index].mass), debris->velocity);
		debris->color.c = 0xFFFFFFFF;
	}
	else
	{
		ke = irand(10, 100) * 10000.0f;
		Vec3ScaleAssign(sqrt(ke/debrisChunks[index].mass), debris->velocity);
		debris->color.c = 0x00000000;
	}

	debris->acceleration[2] = GetGravity();

	debris->r.angles[0] = flrand(-ANGLE_180, ANGLE_180);
	debris->r.angles[1] = flrand(-ANGLE_90, ANGLE_90);

	debris->elasticity = debrisElasticity[MAT_FLESH];

	debris->Update = FXBodyPart_Update;
	debris->updateTime = 50;
	
	if(r_detail->value == DETAIL_UBERHIGH)
		debris->LifeTime = fxi.cl->time + 10000;
	else if(!r_detail->value)
		debris->LifeTime = fxi.cl->time + 1000;
	else
		debris->LifeTime = fxi.cl->time + 3000;

	if(flags&CEF_FLAG6)//on fire- dynamic light
	{
		// Not-very-perfect way of doing a pointcontents from the FX dll
		VectorCopy(origin, start);
		start[2] += 1;
		fxi.Trace(start, vec3_origin, vec3_origin, origin, 0, 0, &trace);
		if(trace.contents&MASK_WATER)
		{//in water- no flames, pal!
			flags &= ~CEF_FLAG6;
		}
		else
		{
			if(!ref_soft && r_detail->value == DETAIL_HIGH)
			{//uberhigh?
				paletteRGBA_t	color;

				debris->flags |= CEF_FLAG7;//don't spawn blood too, just flames
				color.c = 0xe5007fff;
				debris->dlight = CE_DLight_new(color, 50.0F, -5.0F);
			}

			debris->flags |= CEF_FLAG6;
		}
	}

	AddEffect(NULL,debris);
}


qboolean FXBodyPartAttachedUpdate(struct client_entity_s *self, centity_t *owner)
{
	VectorCopy(owner->lerp_origin, self->r.origin);
	VectorSet(self->r.angles,
		owner->lerp_angles[PITCH] * -1 * ANGLE_TO_RAD,
		owner->lerp_angles[YAW] * ANGLE_TO_RAD,
		owner->lerp_angles[ROLL] * ANGLE_TO_RAD);

	if((self->SpawnInfo&SIF_FLAG_MASK)==MAT_FLESH||(self->SpawnInfo&SIF_FLAG_MASK) == MAT_INSECT)
		DoBloodTrail(self, -1);
	
	if(self->flags&CEF_FLAG6)//on fire- do a fire trail
		DoFireTrail(self);

	return true;
}

qboolean FXBodyPart_Update(struct client_entity_s *self, centity_t *owner)
{
	int curTime = fxi.cl->time;
	float d_time = (curTime - self->lastThinkTime) / 1000.0f;

	if ( curTime > self->LifeTime )
	{
		self->d_alpha = flrand(-0.05, -0.2);
		self->Update = FXDebris_Vanish;
		return true;
	}

	self->r.angles[0] += ANGLE_360*d_time;
	self->r.angles[1] += ANGLE_360*d_time;
	self->r.angles[2] += ANGLE_360*d_time;
	
	self->lastThinkTime = fxi.cl->time;

	if((self->SpawnInfo&SIF_FLAG_MASK)==MAT_FLESH||(self->SpawnInfo&SIF_FLAG_MASK) == MAT_INSECT)
		DoBloodTrail(self, 6);
	
	if(self->flags&CEF_FLAG6)//on fire- do a fire trail
		DoFireTrail(self);

	return true;
}
//------------------------------------------------------------------
//	FX Debris spawn functions
//	CEF_FLAG6 = on fire
//  CEF_FLAG7 = male insect skin on mat_insect (CEF_FLAG7 cleared out and set if has dynamic light for fire)
//  CEF_FLAG8 = use reflection skin
//------------------------------------------------------------------

client_entity_t *FXDebris_Throw(vec3_t origin, int material, vec3_t dir, float ke, float scale, int flags, qboolean altskin)
{
	int				index;
	client_entity_t	*debris;

	debris = ClientEntity_new(-1, 0, origin, NULL, 50);
	debris->SpawnInfo = material;

	index = irand(debrisChunkOffsets[material], debrisChunkOffsets[material + 1] - 1);

	debris->classID = CID_DEBRIS;
	debris->msgHandler = CE_DefaultMsgHandler;

	debris->r.model = &debrisChunks[index].model;

	debris->r.scale = scale;
	debris->r.angles[0] = flrand(-ANGLE_180, ANGLE_180);
	debris->r.angles[1] = flrand(-ANGLE_90, ANGLE_90);

	debris->flags |= CEF_CLIP_TO_WORLD | CEF_ABSOLUTE_PARTS;
	debris->radius = 5.0;

	VectorRandomCopy(dir, debris->velocity, 0.5F);
	Vec3ScaleAssign(sqrt(ke / debrisChunks[index].mass), debris->velocity);

	debris->acceleration[2] = GetGravity();

	debris->elasticity = debrisElasticity[material];
	debris->r.skinnum = debrisChunks[index].skinNum;
	
	if(material == MAT_FLESH||material == MAT_INSECT)	// Flesh need a different update for blood
	{
		debris->Update = FXFleshDebris_Update;
		if(altskin)
		{
			if(index < 47)//using multiskinned pottery chunks
				debris->r.skinnum = 1;//male
		}
	}
	else
	{
		if(material == MAT_GLASS)
		{
			debris->r.skinnum = 1;
			debris->r.flags |= RF_TRANSLUCENT;
		}

		debris->Update = FXDebris_Update;
	}

	//Debris lasts 10 seconds before it slowly goes away
	debris->LifeTime = fxi.cl->time + 1000;

	if(flags&CEF_FLAG6)//on fire- dynamic light
	{
		if(flags&CEF_FLAG7)//high detail, non-ref_soft?
		{//uberhigh?
			paletteRGBA_t	color;

			debris->flags |= CEF_FLAG7;//spawn blood too, not just flames
			color.c = 0xe5007fff;
			debris->dlight = CE_DLight_new(color, 50.0F, -5.0F);
		}

		debris->flags |= CEF_FLAG6;
	}
		
	if(flags&CEF_FLAG8)//reflective
	{
		debris->r.flags |= RF_REFLECTION;
	}

	AddEffect(NULL, debris);

	return(debris);
}

// Flesh debris throws should be a separate effect
// This would save quite a bit of code

void FXDebris_SpawnChunks(int type, int flags, vec3_t origin, int num, int material, vec3_t dir, float ke, vec3_t mins, float scale, qboolean altskin)
{
	int					i;
	vec3_t				holdorigin, start;
	trace_t				trace;
	
	if(flags&CEF_FLAG6)//onfire, check for highdetail, non-ref_soft
	{
		//Not-very-perfect way of doing a pointcontents from the FX dll
		VectorCopy(origin, start);
		start[2] += 1;
		fxi.Trace(start, vec3_origin, vec3_origin, origin, 0, 0, &trace);
		if(trace.contents&MASK_WATER)
		{//in water- no flames, pal!
			flags &= ~CEF_FLAG6;
		}
		else
		{
			if(!ref_soft && r_detail->value == DETAIL_HIGH)
				flags|=CEF_FLAG7;//do dynamic light and blood trail
		}
	}

	for(i = 0; i < num; ++i)
	{
		VectorCopy(origin, holdorigin);
		holdorigin[0] += flrand(-mins[0], mins[0]);
		holdorigin[1] += flrand(-mins[1], mins[1]);
		holdorigin[2] += flrand(-mins[2], mins[2]);

		if (material != MAT_NONE)
		{
			FXDebris_Throw(holdorigin, material, dir, ke, scale, flags, altskin);

			if(flags&CEF_FLAG6 && !irand(0, 3))//onfire
			{//do a big flaming sparky thingy
			}
			if (material == MAT_FLESH)	// Flesh need a different update for blood
			{
				DoBloodSplash(holdorigin, 5, false);
			}
			else if (material == MAT_INSECT)
			{
				DoBloodSplash(holdorigin, 5, true);
			}
			else if(irand(0, 1))
			{
				CreateSinglePuff(holdorigin, 20.0);
			}
		}
		else	// Nothing but smoke
			CreateSinglePuff(holdorigin, 20.0);
	}
}

void FXDebris_SpawnFleshChunks(int type, int flags, vec3_t origin, int num, int material, vec3_t dir, float ke, vec3_t mins, float scale, qboolean altskin)
{
	int					i;
	vec3_t				holdorigin, start;
	trace_t				trace;

	if(flags&CEF_FLAG6)//onfire, check for highdetail, non-ref_soft
	{
		//Not-very-perfect way of doing a pointcontents from the FX dll
		VectorCopy(origin, start);
		start[2] += 1;
		fxi.Trace(start, vec3_origin, vec3_origin, origin, 0, 0, &trace);
		if(trace.contents&MASK_WATER)
		{//in water- no flames, pal!
			flags &= ~CEF_FLAG6;
		}
		else
		{
			if(!ref_soft && r_detail->value == DETAIL_HIGH)
				flags|=CEF_FLAG7;//do dynamic light and blood trail
		}
	}

	for(i = 0; i < num; ++i)
	{
		VectorCopy(origin, holdorigin);
		holdorigin[0] += flrand(-mins[0], mins[0]);
		holdorigin[1] += flrand(-mins[1], mins[1]);
		holdorigin[2] += flrand(-mins[2], mins[2]);
		FXDebris_Throw(holdorigin, material, dir, ke, scale, flags, altskin);
		DoBloodSplash(holdorigin, 5, material == MAT_INSECT);
	}
}


// num = number of chunks to spawn (dependent on size and mass)
// material = type of chunk to explode
// dir = direction of debris (currently always 0.0 0.0 1.0)
// ke = kinetic energy (dependent of damage and number of chunks)
// mins = mins field of edict (so debris is spawned at base)
// scale = size of the spawned chunks (dependent on size)

void FXDebris(centity_t *owner, int type, int flags, vec3_t origin)
{
	byte			num, material, size, mag;
	vec3_t			dir, mins;
	float			ke, scale;

	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_DEBRIS].formatString, &size, &material, mins, &mag);

	scale = size * 10;
	VectorSet(dir, 0.0, 0.0, 1.0);
	Vec3ScaleAssign(mag, mins);
	scale = sqrt(scale) * 0.08;
	if(r_detail->value < DETAIL_NORMAL)
		num = ClampI(size, 1, 5);
	else if(r_detail->value == DETAIL_NORMAL)
		num = ClampI(size, 1, 12);
	else if(r_detail->value == DETAIL_HIGH)
		num = ClampI(size, 1, 16);
	else
		num = ClampI(size, 1, 24);

	ke = 40000.0 + (size * 400.0);

	FXDebris_SpawnChunks(type, flags, origin, num, material, dir, ke, mins, scale, false);
}

void FXFleshDebris(centity_t *owner, int type, int flags, vec3_t origin)
{
	byte			num, material, size, mag;
	vec3_t			dir, mins;
	qboolean		altskin = false;
	float			scale;

	if(flags&CEF_FLAG7)
	{//male insect
		material = MAT_INSECT;
		flags &= ~CEF_FLAG7;
		flags &= ~CEF_FLAG8;
		altskin = true;
	}
	else if(flags&CEF_FLAG8)
	{//normal insect
		material = MAT_INSECT;
		flags &= ~CEF_FLAG8;
	}
	else
		material = MAT_FLESH;

	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_FLESH_DEBRIS].formatString, &size, mins, &mag);

	VectorSet(dir, 0.0, 0.0, 1.0);
	Vec3ScaleAssign(mag, mins);
	if(r_detail->value < DETAIL_NORMAL)
		num = ClampI(size, 1, 5);
	else if(r_detail->value == DETAIL_NORMAL)
		num = ClampI(size, 1, 12);
	else if(r_detail->value == DETAIL_HIGH)
		num = ClampI(size, 1, 16);
	else
		num = ClampI(size, 1, 24);

	scale = mag/24;
	FXDebris_SpawnFleshChunks(type, flags, origin, num, material, dir, 80000.0, mins, scale, altskin);
}

//------------------------------------------------------------------
//	FX Debris message receivers
//------------------------------------------------------------------

static void FXDebris_Collision(client_entity_t *self, CE_Message_t *msg)
{
	trace_t		*trace;
	float		d_time;
	int			material;
	qboolean	dark = false;
	qboolean	yellow = false;

	if(self->flags & CEF_CLIP_TO_WORLD)
	{
		ParseMsgParms(msg, "g", &trace);

		if(trace->startsolid || trace->allsolid || Vec3IsZeroEpsilon(trace->plane.normal))
			return;

		// hit the world
		if(trace->ent == (struct edict_s *)-1)
		{
			if(trace->contents&CONTENTS_SOLID)
			{//hit a solid surface, make noise and leave any decals
				material = self->SpawnInfo & SIF_FLAG_MASK;//again, the SpawnInfo lower 2 bits are material types, >=16 are flags- here we mask out those flags to get the actual materialtype
				switch(material)
				{//fixme: find out what surface this is hitting?
					case MAT_METAL:
						if(self->effectID == FX_THROWWEAPON && (!(irand(0,2))))
						{
							vec3_t	dir;

							VectorSet(dir, 0.0, 0.0, 1.0);
							GenericSparks(NULL, FX_SPARKS, 0, self->r.origin, dir);
						
							if(!irand(0,1))
								fxi.S_StartSound(self->r.origin, -1, CHAN_AUTO, fxi.S_RegisterSound("misc/dropmetal.wav"), 1, ATTN_NORM, 0);
							else
								fxi.S_StartSound(self->r.origin, -1, CHAN_AUTO, fxi.S_RegisterSound("misc/dropmetal1.wav"), 1, ATTN_NORM, 0);
						}
						else
							fxi.S_StartSound(self->r.origin, -1, CHAN_AUTO,
									fxi.S_RegisterSound(va("misc/drophvmtl%c.wav", irand('1', '3'))), 1, ATTN_STATIC, 0);

						//need more hollow sounds for big metal
						break;
					
					case MAT_WOOD:
						if(!irand(0,6))
						{
							if(!irand(0,1))
								fxi.S_StartSound(self->r.origin, -1, CHAN_AUTO, fxi.S_RegisterSound("misc/dropwood.wav"), 1, ATTN_STATIC, 0);
							else
								fxi.S_StartSound(self->r.origin, -1, CHAN_AUTO, fxi.S_RegisterSound("misc/dropwood1.wav"), 1, ATTN_STATIC, 0);
						}
						break;

					case MAT_STONE:
					case MAT_GREYSTONE:
					case MAT_BROWNSTONE:
						if(!irand(0,6))
						{
							if(!irand(0,2))
								fxi.S_StartSound(self->r.origin, -1, CHAN_AUTO, fxi.S_RegisterSound("misc/dropthing.wav"), 1, ATTN_STATIC, 0);
							else
								fxi.S_StartSound(self->r.origin, -1, CHAN_AUTO,
										fxi.S_RegisterSound(va("misc/boulder%c.wav", irand('1', '2'))), 1, ATTN_STATIC, 0);
						}
						break;

					case MAT_FLESH://maybe slide?  Wet sound?
					case MAT_INSECT:
						if(self->flags&CEF_FLAG6)
							dark = true;
						
						if(material==MAT_INSECT)
							yellow = true;

						if(self->effectID == FX_BODYPART)
						{
							fxi.S_StartSound(self->r.origin, -1, CHAN_AUTO,
									fxi.S_RegisterSound(va("misc/fleshdrop%c.wav", irand('1', '3'))), 1, ATTN_STATIC, 0);
							DoBloodSplash(self->r.origin, irand(1, 3), yellow);
							if(!(self->SpawnInfo&SIF_INWATER))
								ThrowBlood(self->r.origin, trace->plane.normal, dark, yellow, false);
						}
						else if (!irand(0,1))//50% to splat
						{
							fxi.S_StartSound(self->r.origin, -1, CHAN_AUTO,
									fxi.S_RegisterSound(va("misc/fleshdrop%c.wav", irand('1', '3'))), 1, ATTN_STATIC, 0);
							if(!(self->SpawnInfo&SIF_INWATER))
								ThrowBlood(self->r.origin, trace->plane.normal, dark, yellow, false);
						}
						break;
					
					case MAT_GLASS:
						if(!irand(0,2))
						{
							fxi.S_StartSound(self->r.origin, -1, CHAN_AUTO,
									fxi.S_RegisterSound(va("misc/dropglass%c.wav", irand('1', '2'))), 1, ATTN_STATIC, 0);
						}
						break;

					default:
						break;
				}
			}
			
			if (trace->plane.normal[2] > GROUND_NORMAL)
			{
				// don't bounce if velocity is small
				if(Q_fabs(self->velocity[2]) < 100.0f || VectorLength(self->velocity) < 100.0f || trace->fraction < 0.075)
				{
					// Set pitch so that chunks lie flat on ground
					self->r.angles[PITCH] = ANGLE_90;

					BecomeStatic(self);			

					self->d_alpha = flrand(-0.1, -0.25);
					self->Update = FXDebris_Vanish;
					return;
				}
			}

			BounceVelocity(self->velocity, trace->plane.normal, self->velocity, self->elasticity);

			d_time = fxi.cls->frametime * trace->fraction;
			// The game might crash with a zero movement. --Pat
			if (d_time)
			{
				Physics_MoveEnt(self, d_time, d_time * d_time * 0.5, trace);
			}
		}
	}
}

//------------------------------------------------------------------
//	FX Debris update
//------------------------------------------------------------------

void FXDarkSmoke(vec3_t origin, float scale, float range);
qboolean FXDebris_Remove(struct client_entity_s *self,centity_t *owner)
{
	return false;
}

qboolean FXDebris_Vanish(struct client_entity_s *self,centity_t *owner)
{
	if(self->SpawnInfo&SIF_INLAVA)
		FXDarkSmoke(self->r.origin, flrand(0.2, 0.5), flrand(30, 50));

	if ((self->alpha < 0.1f)||(self->r.scale < 0.1f))
	{
		if(self->flags&CEF_FLAG6)
		{//let the smoke die out
			self->alpha = 0.0f;
			self->r.scale = 0.0f;
			self->Update = FXDebris_Remove;
			self->updateTime = 1000;
			return true;
		}
		else
			return false;
	}

	if(self->flags&CEF_FLAG6 && !irand(0, 2))//on fire- do a fire trail
	{
		if(irand(0, 0.3)>self->alpha)
		{
			self->dlight = NULL;
			self->flags &= ~CEF_FLAG6;
			self->d_alpha = -0.01;
		}
		else if(irand(0, 0.3)>self->r.scale)
		{
			self->dlight = NULL;
 			self->flags &= ~CEF_FLAG6;
			self->d_scale = -0.01;
		}
		else
			DoFireTrail(self);//FIXME: make them just smoke when still?
	}

	return true;
}

qboolean FXDebris_Update(struct client_entity_s *self,centity_t *owner)
{
	int curTime = fxi.cl->time;
	float d_time = (curTime - self->lastThinkTime) / 1000.0f;

	if ( curTime > self->LifeTime )
	{
		self->d_alpha = flrand(-0.05, -0.2);
		self->Update = FXDebris_Vanish;
		return true;
	}

	self->r.angles[0] += ANGLE_360*d_time;
	self->r.angles[1] += ANGLE_360*d_time;
	
	self->lastThinkTime = fxi.cl->time;

	if(self->flags&CEF_FLAG6)//on fire- do a fire trail
		DoFireTrail(self);

	return true;
}

qboolean FXFleshDebris_Update(struct client_entity_s *self,centity_t *owner)
{
	int curTime = fxi.cl->time;
	float d_time = (curTime - self->lastThinkTime) / 1000.0f;

	
	if(self->flags&CEF_FLAG6)//on fire- do a fire trail
	{
		DoFireTrail(self);
		if(self->flags&CEF_FLAG7)
			DoBloodTrail(self, 2);
	}
	else
		DoBloodTrail(self, 2);


	if(curTime > self->LifeTime)
	{
		self->d_alpha = flrand(-0.05, -0.2);
		self->Update = FXDebris_Vanish;
		return true;
	}

	self->r.angles[0] += ANGLE_360*d_time;
	self->r.angles[1] += ANGLE_360*d_time;
	
	self->lastThinkTime = fxi.cl->time;

	return true;
}

