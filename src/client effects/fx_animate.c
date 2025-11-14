//
// fx_animate.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Vector.h"
#include "Utilities.h"
#include "Random.h"

typedef struct FXAnimModel
{
	char* model_name;
	struct model_s* model;
	float radius;
	float alpha;
	int num_frames;
	int default_frame;
	int update_time; //mxd. In ms.
} FXAnimModel_t;

// Animating chunks.
static FXAnimModel_t fx_anim_models[NUM_FX_ANIM] =
{
	// FX_ANIM_BANNER
	{
		.model_name = "models/objects/banner/wall/tris.fm",
		.model = NULL,
		.radius = 220.0f,
		.alpha = 1.0f,
		.num_frames = 15,
		.default_frame = 1,
		.update_time = 500 //mxd
	},

	// FX_ANIM_CANDELABRUM
	{
		.model_name = "models/objects/lights/candelabrum/tris.fm",
		.model = NULL,
		.radius = 100.0f,
		.alpha = 1.0f,
		.num_frames = 7,
		.default_frame = 1,
		.update_time = 25 //mxd
	},

	// FX_ANIM_CHANDELIER2
	{
		.model_name = "models/objects/chandelier/chan2/tris.fm",
		.model = NULL,
		.radius = 100.0f,
		.alpha = 1.0f,
		.num_frames = 7,
		.default_frame = 7,
		.update_time = 100 //mxd
	},

	// FX_ANIM_FLAME - NEVER USED
	{
		.model_name = NULL,
		.model = NULL,
		.radius = 100.0f,
		.alpha = 0.5f,
		.num_frames = 1,
		.default_frame = 0,
		.update_time = 100 //mxd
	},

	// FX_ANIM_FIRE - NEVER USED
	{
		.model_name = NULL,
		.model = NULL,
		.radius = 100.0f,
		.alpha = 0.5f,
		.num_frames = 1,
		.default_frame = 0,
		.update_time = 100 //mxd
	},

	// FX_ANIM_BANNERONPOLE
	{
		.model_name = "models/objects/banner/onpole/tris.fm",
		.model = NULL,
		.radius = 100.0f,
		.alpha = 1.0f,
		.num_frames = 21,
		.default_frame = 0,
		.update_time = 100 //mxd
	},

	// FX_ANIM_FLAGONPOLE
	{
		.model_name = "models/objects/flags/onpole/tris.fm",
		.model = NULL,
		.radius = 100.0f,
		.alpha = 1.0f,
		.num_frames = 80,
		.default_frame = 0,
		.update_time = 100 //mxd
	},

	// FX_ANIM_COCOON
	{
		.model_name = "models/objects/eggs/cocoon/tris.fm",
		.model = NULL,
		.radius = 100.0f,
		.alpha = 1.0f,
		.num_frames = 20,
		.default_frame = 0,
		.update_time = 1000 //mxd
	},

	// FX_ANIM_LABPARTSCONTAINER
	{
		.model_name = "models/objects/labs/container1/tris.fm",
		.model = NULL,
		.radius = 100.0f,
		.alpha = 1.0f,
		.num_frames = 30,
		.default_frame = 0,
		.update_time = 100 //mxd //TODO: speed-up animation (to 25?), add random delays between heartbeats (via custom Update handler)?
	},

	// FX_ANIM_LABTRAY
	{
		.model_name = "models/objects/labs/tray/tris.fm",
		.model = NULL,
		.radius = 100.0f,
		.alpha = 1.0f,
		.num_frames = 15,
		.default_frame = 0,
		.update_time = 200 //mxd
	},

	// FX_ANIM_EYEBALLJAR
	{
		.model_name = "models/objects/labs/container2/tris.fm",
		.model = NULL,
		.radius = 100.0f,
		.alpha = 1.0f,
		.num_frames = 80,
		.default_frame = 0,
		.update_time = 100 //mxd
	},

	// FX_ANIM_HANGING_OGLE
	{
		.model_name = "models/objects/torture/ogle/tris.fm",
		.model = NULL,
		.radius = 100.0f,
		.alpha = 1.0f,
		.num_frames = 100,
		.default_frame = 0,
		.update_time = 100 //mxd
	} 
};

void PreCacheFXAnimate(void)
{
	for (int i = 0; i < NUM_FX_ANIM; i++)
		if (fx_anim_models[i].model_name != NULL)
			fx_anim_models[i].model = fxi.RegisterModel(fx_anim_models[i].model_name);
}

static qboolean AnimationUpdate(struct client_entity_s* self, centity_t* owner)
{
	//mxd. Update frame interpolation settings.
	self->framelerp_time = fx_time;
	self->r.oldframe = self->r.frame;

	self->r.frame++;

	if (self->r.frame >= self->NoOfAnimFrames)
		self->r.frame = 0;

	return true;
}

void FXAnimate(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	byte anim;
	byte scale;
	byte skinnum;
	vec3_t angles;
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_ANIMATE].formatString, &anim, &scale, &skinnum, angles);

	const byte anim_type = (anim & 0x7f);
	FXAnimModel_t* anim_settings = &fx_anim_models[anim_type];

	client_entity_t* self = ClientEntity_new(type, flags | CEF_FRAME_LERP, origin, NULL, anim_settings->update_time); //mxd. +CEF_FRAME_LERP; 100 -> update_time.
	VectorDegreesToRadians(angles, self->r.angles);

	self->r.model = &anim_settings->model;
	self->r.scale = (float)scale * 0.02f;
	self->r.skinnum = skinnum;

	self->alpha = anim_settings->alpha;
	self->radius = anim_settings->radius;
	self->NoOfAnimFrames = anim_settings->num_frames;
	self->r.frame = anim_settings->default_frame;
	self->r.oldframe = self->r.frame; //mxd

	if (anim & 0x80)
	{
		self->Update = AnimationUpdate;
		self->nextThinkTime = fx_time + irand(40, 500); // So they don't all start on frame 0 at the same time.
	}
	else
	{
		// Don`t animate and think less.
		self->Update = KeepSelfAI;
		self->updateTime = 1000;
	}

	AddEffect(owner, self);
}