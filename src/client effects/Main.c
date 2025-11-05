//
// Main.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "LightStyles.h" //mxd
#include "Particle.h"
#include "ResourceManager.h"
#include "Skeletons.h"
#include "Vector.h"
#include "Utilities.h" //mxd
#include "ce_DLight.h"
#include "q_Physics.h"
#include "g_playstats.h"

#define CLFX_DECLSPEC	__declspec(dllexport)

// Important! This is the string that determines if you can join a server - ie you have the right Client Effects.dll.
static char clfx_string[128] = "Heretic II v1.06"; //mxd. Renamed from client_string to avoid collisions with the var defined in client.h.

client_fx_import_t fxi;

cvar_t* cl_camera_under_surface;
cvar_t* r_farclipdist;
cvar_t* r_nearclipdist;
cvar_t* r_detail;
cvar_t* clfx_gravity;
cvar_t* vid_ref;
cvar_t* cl_timedemo;

static cvar_t* fx_numinview;
static cvar_t* fx_numactive;
static cvar_t* cl_lerpdist2;

qboolean ref_soft; //mxd. int in original logic.
int numprocessedparticles;
int numrenderedparticles;

qboolean fx_FreezeWorld = false;

static int num_owned_inview;

static void Clear(void)
{
	if (clientEnts != NULL)
		RemoveEffectList(&clientEnts);

	centity_t* owner = fxi.server_entities;
	for (int i = 0; i < MAX_NETWORKABLE_EDICTS; i++, owner++)
	{
		if (owner->effects != NULL)
			RemoveOwnedEffectList(owner);

		if (owner->current.clientEffects.buf != NULL)
		{
			ResMngr_DeallocateResource(fxi.FXBufMngr, owner->current.clientEffects.buf, ENTITY_FX_BUF_SIZE * sizeof(char));
			owner->current.clientEffects.buf = NULL;
		}
	}

	CL_ClearLightStyles();

	//mxd. Regenerate default {1, 1, 1} cl_lightstyle values. Fixes black screen after loading map in SP from console. Happens in original logic because:
	// 1. cl_lightstyle is zeroed out on map change by CL_ClearLightStyles().
	// 2. cl_paused is enabled when opening console in SP, so lightstypes update (fxe.UpdateEffects() -> CL_RunLightStyles()) is skipped in CL_Frame()).
	CL_RunLightStyles(); //TODO: player model hideable meshes (armor, bow etc.) are not updated.

	ClearCircularList(); //mxd
}

static void Init(void)
{
	InitParticleMngr();
	InitFMNodeInfoMngr();
	InitEntityMngr();
	CE_InitMsgMngr();
	InitDLightMngr();

	for (int i = 0; i < NUM_CLASSIDS; i++)
		ce_class_statics_inits[i]();

	clientEnts = NULL;

	cl_camera_under_surface = Cvar_Get("cl_camera_under_surface", "0", 0);
	r_farclipdist = Cvar_Get("r_farclipdist", FAR_CLIP_DIST, 0);
	r_nearclipdist = Cvar_Get("r_nearclipdist", NEAR_CLIP_DIST, 0);
	r_detail = Cvar_Get("r_detail", DETAIL_DEFAULT, CVAR_ARCHIVE);
	fx_numinview = Cvar_Get("fx_numinview", "0", 0);
	fx_numactive = Cvar_Get("fx_numactive", "0", 0);
	clfx_gravity = Cvar_Get("clfx_gravity", GRAVITY_STRING, 0);
	cl_timedemo = Cvar_Get("timedemo", "0", 0);

	cl_lerpdist2 = Cvar_Get("cl_lerpdist2", "10000", 0);
	vid_ref = Cvar_Get("vid_ref", "soft", CVAR_ARCHIVE);

	Clear();
}

static void ShutDown(void)
{
	Clear();

	ReleaseParticleMngr();
	ReleaseEntityMngr();
	ReleaseFMNodeInfoMngr();
	ReleaseDLightMngr();
	CE_ReleaseMsgMngr();
}

static void AddEffects(const qboolean freeze)
{
	int	num_free_inview = 0;

	// If the world is frozen then the client effects, particularly the particles shouldn't update.
	fx_FreezeWorld = freeze;

	if (clientEnts != NULL)
		num_free_inview += AddEffectsToView(&clientEnts, NULL);

	// Add all effects which are attached to entities, that have no model.
	centity_t* owner = fxi.server_entities;
	for (int i = 0; i < MAX_NETWORKABLE_EDICTS; i++, owner++)
		if (owner->effects != NULL && (owner->current.effects & EF_ALWAYS_ADD_EFFECTS)) // Think something else need to be done... maybe a list of centities with effects.
			num_owned_inview += AddEffectsToView(&owner->effects, owner);

	CL_AddLightStyles();

	if ((int)fx_numinview->value)
		Com_DPrintf("Active CE : free %d, owned %d. Particles : processed %d, rendered %d\n", num_free_inview, num_owned_inview, numprocessedparticles, numrenderedparticles);
}

static void PostRenderUpdate(void)
{
	int	num_free_active = 0;
	int	num_owned_active = 0;

	numprocessedparticles = 0;
	numrenderedparticles = 0;

	if (clientEnts != NULL)
		num_free_active += UpdateEffects(&clientEnts, NULL);

	centity_t* owner = fxi.server_entities;
	for (int i = 0; i < MAX_NETWORKABLE_EDICTS; i++, owner++)
		if (owner->effects != NULL) // Think something else need to be done... maybe a list of centities with effects.
			num_owned_active += UpdateEffects(&owner->effects, owner);

	CL_RunLightStyles();

	if ((int)fx_numactive->value)
		Com_DPrintf("Active CE : free %d, owned %d\n", num_free_active, num_owned_active);
}

static int DummyEffectParams(centity_t* ent, const int flags, const int effect)
{
	sizebuf_t* msg_read;
	sizebuf_t temp_buf;

	const char* format = clientEffectSpawners[effect].formatString;

	if (format == NULL)
		return 0;

	EffectsBuffer_t* fx_buf = NULL;

	if (ent != NULL && !(flags & (CEF_BROADCAST | CEF_MULTICAST)))
	{
		if (*fxi.cl_effectpredict == 0)
			fx_buf = &ent->current.clientEffects;
		else
			fx_buf = fxi.clientPredEffects;

		msg_read = &temp_buf;

		memset(msg_read, 0, sizeof(*msg_read));

		msg_read->data = fx_buf->buf;
		msg_read->cursize = fx_buf->bufSize;
		msg_read->maxsize = fx_buf->bufSize;
		msg_read->readcount = fx_buf->freeBlock;
	}
	else
	{
		msg_read = fxi.net_message;
	}

	int count = 0;

	while (format[count] != 0)
	{
		vec3_t v;

		switch (format[count])
		{
			case 'b':
				MSG_ReadByte(msg_read);
				break;

			case 's':
				MSG_ReadShort(msg_read);
				break;

			case 'i':
				MSG_ReadLong(msg_read);
				break;

			case 'f':
				MSG_ReadFloat(msg_read);
				break;

			case 'p':
			case 'v':
				MSG_ReadPos(msg_read, v);
				break;

			case 'u':
				MSG_ReadDirMag(msg_read, v);
				break;

			case 'd':
				MSG_ReadDir(msg_read, v);
				break;

			case 'x':
				MSG_ReadYawPitch(msg_read, v);
				break;

			case 't':
				MSG_ReadShortYawPitch(msg_read, v);
				break;

			default:
				assert(0);
				return 0;
		}

		count++;
	}

	if (ent != NULL && !(flags & (CEF_BROADCAST | CEF_MULTICAST)))
		fx_buf->freeBlock = msg_read->readcount;

	return count;
}

// Insert the logic in this could use a good cleaning...
//mxd. Written by SV_CreateEffectEvent() / SV_CreatePersistantEffect().
static void ParseEffects(centity_t* owner)
{
	int index;
	int num;
	int flags;
	sizebuf_t* msg_read;
	sizebuf_t temp_buf;

	EffectsBuffer_t* fx_buf = NULL;
	int last_effect = -1;
	int event_id = 0;
	centity_t* temp_owner = owner;

	if (owner != NULL)
	{
		// Where do we pull the effect from?
		if (*fxi.cl_effectpredict == 0)
		{
			// Effects received as part of entity_state_t from server.
			fx_buf = &owner->current.clientEffects;
		}
		else
		{
			// Predicted effects are pulled from here...
			fx_buf = fxi.clientPredEffects;

			// We are dealing with predicted effects, so reset freeblock for reading, as writing
			// will have left it at the end of the written data.
			fx_buf->freeBlock = 0;
		}

		num = fx_buf->numEffects;

		msg_read = &temp_buf;
		memset(msg_read, 0, sizeof(*msg_read));
		msg_read->data = fx_buf->buf;
		msg_read->cursize = msg_read->maxsize = fx_buf->bufSize;
	}
	else
	{
		msg_read = fxi.net_message;
		num = MSG_ReadByte(msg_read);
	}

	if (num < 0)
	{
		fxi.Com_Error(ERR_DROP, "ParseClientEffects: number of effects < 0");
		return;
	}

	for (int i = 0; i < num; i++)
	{
		qboolean effect_is_from_server = false;

		if (owner != NULL)
			msg_read->readcount = fx_buf->freeBlock;

		ushort effect = (ushort)MSG_ReadShort(msg_read);

		if (effect & EFFECT_PRED_INFO)
		{
			// If EFFECT_PRED_INFO bit is set (only on effects sent by server, never on predicted effects),
			// read a byte that uniquely identifies the client effect in the player code.
			event_id = MSG_ReadByte(msg_read);
			effect &= ~EFFECT_PRED_INFO;
			effect_is_from_server = true;
		}

		if (effect & EFFECT_FLAGS) //mxd. Use EFFECT_FLAGS define.
		{
			effect &= ~EFFECT_FLAGS;
			flags = MSG_ReadByte(msg_read);
		}
		else
		{
			flags = 0;
		}

		if (flags & (CEF_BROADCAST | CEF_MULTICAST))
		{
			if (flags & CEF_ENTNUM16)
				index = MSG_ReadShort(msg_read);
			else
				index = MSG_ReadByte(msg_read);

			if (index != 0) // 0 should indicate the world.
			{
				assert(index > 0 && index < MAX_NETWORKABLE_EDICTS);
				temp_owner = &fxi.server_entities[index];
			}
		}

		vec3_t position;
		if (flags & CEF_OWNERS_ORIGIN)
		{
			if (temp_owner != NULL)
				VectorCopy(temp_owner->origin, position);
			else
				VectorClear(position);
		}
		else
		{
			MSG_ReadPos(msg_read, position);

			if (temp_owner != NULL && !(flags & CEF_BROADCAST))
				Vec3AddAssign(temp_owner->origin, position);
		}

		if (effect >= NUM_FX)
		{
			fxi.Com_Error(ERR_DROP, "ParseClientEffects: bad effect %d last effect %d", effect, last_effect);
			return;
		}

		if (owner != NULL && !(flags & (CEF_BROADCAST | CEF_MULTICAST)))
			fx_buf->freeBlock = msg_read->readcount;

		// Do we want to start this client-effect if client-prediction has already started it?
		if (*fxi.cl_effectpredict == 0 && (int)fxi.cl_predict->value && effect_is_from_server &&
			fxi.EffectEventIdTimeArray[event_id] <= *fxi.leveltime && fxi.EffectEventIdTimeArray[event_id] != 0.0f)
		{
			// The client-effect has already been started by client-prediction, so just skip it.
			DummyEffectParams(owner, flags, effect);
		}
		else
		{
			// Start the client-effect.
			clientEffectSpawners[effect].SpawnCFX(temp_owner, effect, flags, position);
		}

		if (effect_is_from_server && fxi.EffectEventIdTimeArray[event_id] <= *fxi.leveltime)
			fxi.EffectEventIdTimeArray[event_id] = 0.0f;

		if (flags & (CEF_BROADCAST | CEF_MULTICAST))
			temp_owner = NULL;

		last_effect = effect;
	}

	if (owner != NULL) // Free the buffer allocated in CL_ParseDelta and passed onto owner->current.
	{
		fx_buf->freeBlock = 0;
		ResMngr_DeallocateResource(fxi.FXBufMngr, fx_buf->buf, ENTITY_FX_BUF_SIZE * sizeof(char));
		fx_buf->buf = NULL;
		fx_buf->numEffects = 0;
		fx_buf->bufSize = 0;
	}
}

static void RemoveEffectsFromCent(centity_t* cent)
{
	if (cent->effects != NULL)
		RemoveOwnedEffectList(cent);
}

static void AddServerEntities(const frame_t* frame)
{
	static entity_t sv_ents[MAX_SERVER_ENTITIES];
	static fmnodeinfo_t sv_ents_fmnodeinfos[MAX_SERVER_ENTITIES][MAX_FM_MESH_NODES];

	int effects;
	int renderfx;
	int clientnum;
	qboolean is_predicted_player;

	const int maxclients = Q_atoi(fxi.cl->configstrings[CS_MAXCLIENTS]);

	// Have to do this here, since the init is loaded once, and the graphics dll might be reloaded.
	ref_soft = (strcmp("soft", vid_ref->string)) ? 0 : 1;

	fxi.cl->PIV = 0;

	PrepAddEffectsToView();
	num_owned_inview = 0;

	// Mace balls rotate at a fixed rate.
	const float macerotate = anglemod((float)fxi.cl->time / 700.0f);

	// Brush models can auto animate their frames.
	const int autoanim = fxi.cl->time / 500;

	memset(sv_ents, 0, sizeof(sv_ents));

	int num_ents_to_add = frame->num_entities;
	if (num_ents_to_add > MAX_SERVER_ENTITIES)
	{
		Com_Printf("Overflow:  Too many (%d : %d) server entities to add to view\n", num_ents_to_add, MAX_SERVER_ENTITIES);
		num_ents_to_add = MAX_SERVER_ENTITIES;
	}

	entity_t* ent = sv_ents;
	for (int pnum = 0; pnum < num_ents_to_add; pnum++)
	{
		entity_state_t* s1 = &fxi.parse_entities[(frame->parse_entities + pnum) & (MAX_PARSE_ENTITIES - 1)];
		centity_t* cent = &fxi.server_entities[s1->number];

		cent->s1 = s1;

		if ((int)fxi.cl_predict->value && s1->number == fxi.cl->playernum + 1)
			is_predicted_player = true; // We are dealing with the client's model under prediction.
		else
			is_predicted_player = false; // We are dealing with a non predicted model (i.e. everything except the client's model).

		// Setup effects, renderfx, skinnum and clientnum stuff.
		if (is_predicted_player)
		{
			cent->current.effects = fxi.predictinfo->effects;
			effects = fxi.predictinfo->effects;

			cent->current.renderfx = fxi.predictinfo->renderfx;
			renderfx = fxi.predictinfo->renderfx;

			ent->skinnum = fxi.predictinfo->skinnum;
			clientnum = fxi.predictinfo->clientnum;
		}
		else
		{
			effects = s1->effects;
			renderfx = s1->renderfx;
			ent->skinnum = s1->skinnum;
			clientnum = s1->clientnum;
		}

		// Set frame.
		if (effects & EF_ANIM_ALL)
			ent->frame = autoanim;
		else if (effects & EF_ANIM_ALLFAST)
			ent->frame = fxi.cl->time / 100;
		else
			ent->frame = s1->frame;

		// Handle flex-model nodes.
		ent->fmnodeinfo = sv_ents_fmnodeinfos[pnum];

		if (is_predicted_player)
			memcpy(ent->fmnodeinfo, fxi.predictinfo->fmnodeinfo, sizeof(s1->fmnodeinfo));
		else
			memcpy(ent->fmnodeinfo, s1->fmnodeinfo, sizeof(s1->fmnodeinfo));

		//TODO: What's going on here?
		if (is_predicted_player)
		{
			// Deal with predicted origin.
			ent->backlerp = 1.0f - fxi.predictinfo->playerLerp;
			VectorCopy(cent->origin, ent->origin);
		}
		else
		{
			ent->oldframe = cent->prev.frame;
			ent->backlerp = 1.0f - fxi.cl->lerpfrac;

			// Interpolate origin.
			vec3_t dist;
			VectorSubtract(cent->current.origin, cent->prev.origin, dist);
			const float lerpdist_scaler = (effects & EF_FAST_MOVER ? 2.0f : 1.0f); //mxd. Special handling for monster_harpy. Can exceed cl_lerpdist2 during harpy_dive_end_move()... 

			if (VectorLengthSquared(dist) <= cl_lerpdist2->value * lerpdist_scaler)
				VectorMA(cent->prev.origin, 1.0f - ent->backlerp, dist, ent->origin);
			else
				VectorCopy(cent->current.origin, ent->origin); // Assume entity teleported.

			VectorCopy(ent->origin, cent->origin);
		}

		VectorCopy(ent->origin, ent->oldorigin);
		VectorCopy(cent->origin, cent->lerp_origin);

		// Set model. 
		if (s1->modelindex == 255)
		{
			// Use custom model and skin for player.
			clientinfo_t* ci = &fxi.cl->clientinfo[clientnum];

			ent->model = ci->model;

			const int skinnum = (ent->skinnum < SKIN_MAX ? ent->skinnum : 0);
			ent->skin = ci->skin[skinnum];

			// To allow for multiple skins on various parts, I'm going to send across a pointer to the whole skin array.
			ent->skins = &ci->skin[0];

			if (ent->skin == NULL || ent->model == NULL)
			{
				ent->model = fxi.cl->baseclientinfo.model;
				ent->skin = fxi.cl->baseclientinfo.skin[skinnum];
			}
		}
		else
		{
			ent->model = &fxi.cl->model_draw[s1->modelindex];
			ent->skin = NULL; // No custom skin.
		}

		ent->scale = s1->scale;
		ent->color = (s1->color.c != 0 ? s1->color : color_white);

		ent->absLight.r = s1->absLight.r;
		ent->absLight.g = s1->absLight.g;
		ent->absLight.b = s1->absLight.b;

		// Set render effects (fullbright, translucent, etc).
		ent->flags = renderfx;

		// Calculate angles.
		if (effects & EF_MACE_ROTATE)
		{
			// Mace balls auto-rotate.
			ent->angles[0] = macerotate * 2.0f;
			ent->angles[1] = macerotate;
			ent->angles[2] = 0.0f;
		}
		else
		{
			// Interpolate angles.
			if (is_predicted_player)
			{
				// The correct angle values have already been generated by prediction and written into the client's predictinfo_t structure.
				for (int i = 0; i < 3; i++)
				{
					cent->current.angles[i] = fxi.predictinfo->currAngles[i];
					cent->prev.angles[i] = fxi.predictinfo->prevAngles[i];

					const float a1 = cent->current.angles[i];
					const float a2 = cent->prev.angles[i];
					ent->angles[i] = LerpAngle(a2, a1, fxi.predictinfo->angleLerp); //mxd. playerLerp -> angleLerp (playerLerp interpolates at 10 FPS, currAngles/prevAngles are updated each packetframe (at 30 FPS by default)).
				}
			}
			else
			{
				// Get the angle values from the usual source, for all entities, including the player.
				for (int i = 0; i < 3; i++)
				{
					const float a1 = cent->current.angles[i];
					const float a2 = cent->prev.angles[i];
					ent->angles[i] = LerpAngle(a2, a1, fxi.cl->lerpfrac);
				}
			}

			VectorDegreesToRadians(ent->angles, ent->angles);
			VectorCopy(ent->angles, cent->lerp_angles);
		}

		ent->rootJoint = ((effects & EF_JOINTED) ? s1->rootJoint : NULL_ROOT_JOINT);

		if (is_predicted_player)
		{
			// The correct frame and swapframe values have already been generated by prediction
			// and written into the client's predictinfo_t structure.
			cent->prev.frame = (short)fxi.predictinfo->prevFrame;
			cent->current.frame = (short)fxi.predictinfo->currFrame;
			cent->prev.swapFrame = (short)fxi.predictinfo->prevSwapFrame;
			cent->current.swapFrame = (short)fxi.predictinfo->currSwapFrame;

			ent->oldframe = cent->prev.frame;
			ent->frame = cent->current.frame;

			if ((effects & EF_SWAPFRAME) && cent->current.swapFrame != cent->current.frame)
			{
				ent->swapFrame = cent->current.swapFrame;
				ent->oldSwapFrame = cent->prev.swapFrame;

				// Yuck... but need to stop crashes for the demo. //TODO: no longer needed?
				if (ent->oldSwapFrame == NO_SWAP_FRAME)
					ent->oldSwapFrame = ent->oldframe;
			}
			else
			{
				ent->swapFrame = NO_SWAP_FRAME;
				cent->prev.swapFrame = NO_SWAP_FRAME;
			}
		}
		else
		{
			// Always get the frame and swapframe values from the usual source, for all entities, including the player.
			if ((effects & EF_SWAPFRAME) && s1->swapFrame != s1->frame)
			{
				ent->swapFrame = s1->swapFrame;
				ent->oldSwapFrame = cent->prev.swapFrame;

				// Yuck... but need to stop crashes for the demo. //TODO: no longer needed?
				if (ent->oldSwapFrame == NO_SWAP_FRAME)
					ent->oldSwapFrame = ent->oldframe;
			}
			else
			{
				ent->swapFrame = NO_SWAP_FRAME;
				cent->prev.swapFrame = NO_SWAP_FRAME;
			}
		}

		ent->referenceInfo = cent->referenceInfo;

		if (cent->current.clientEffects.numEffects > 0)
		{
			*fxi.cl_effectpredict = 0;
			ParseEffects(cent);
		}

		if (s1->number > 0 && s1->number <= maxclients)
		{
			fxi.cl->PIV |= 1 << (s1->number - 1);
			VectorCopy(ent->origin, fxi.cl->clientinfo[s1->number - 1].origin);
		}

		// Add player's packet_entity_t to refresh list of entity_t's and save the entity_t pointer	in PlayerEntPtr.
		if (s1->number == fxi.cl->playernum + 1)
		{
			if ((int)fxi.cl_predict->value && fxi.clientPredEffects->numEffects > 0)
			{
				*fxi.cl_effectpredict = 1;
				ParseEffects(cent);
				*fxi.cl_effectpredict = 0;
			}

			// This is the player.
			if (*fxi.PlayerAlpha < 1.0f)
			{
				ent->color.a = (byte)(*fxi.PlayerAlpha * (float)ent->color.a);
				ent->flags |= RF_TRANSLUCENT;
			}
			else
			{
				// Color has already been copied from s1.
				ent->flags &= ~RF_TRANSLUCENT;
			}

			if (s1->modelindex > 0)
				AddEntityToView(ent);

			*fxi.PlayerEntPtr = ent;

			// So client effects can play with owners entity.
			cent->entity = ent;

			if (cent->effects != NULL && !(effects & (EF_DISABLE_ALL_CFX | EF_ALWAYS_ADD_EFFECTS)))
				num_owned_inview += AddEffectsToView(&cent->effects, cent);

			ent++;
		}
		else
		{
			// Cull (any eligible) entire models before they get rendered
			if (s1->modelindex > 0)
			{
				vec3_t dir;
				VectorSubtract(ent->origin, fxi.cl->refdef.vieworg, dir);

				ent->depth = VectorNormalize(dir);

				AddEntityToView(ent);
				cent->entity = ent; // So client effects can play with owners entity.

				ent++;
			}

			if (cent->effects != NULL && !(effects & (EF_DISABLE_ALL_CFX | EF_ALWAYS_ADD_EFFECTS)))
				num_owned_inview += AddEffectsToView(&cent->effects, cent);
		}
	}
}

//mxd. Dynamically resolved in clfx_dll.c
CLFX_DECLSPEC client_fx_export_t GetfxAPI(const client_fx_import_t import)
{
	client_fx_export_t export;

	fxi = import;

	export.api_version = FX_API_VERSION;
	export.client_string = clfx_string;

	export.Init = Init;
	export.ShutDown = ShutDown;
	export.Clear = Clear;

	export.RegisterSounds = RegisterSounds;
	export.RegisterModels = RegisterModels;

	export.AddPacketEntities = AddServerEntities;	// In the client code in the executable the following functions are called first.
	export.AddEffects = AddEffects;					// Secondly...
	export.ParseClientEffects = ParseEffects;		// Thirdly (if any independent effects exist).
	export.UpdateEffects = PostRenderUpdate;		// Lastly.

	export.SetLightstyle = CL_SetLightstyle;

	export.GetLMI = GetLMI;
	export.GetLMIMax = GetLMIMax;

	export.RemoveClientEffects = RemoveEffectsFromCent;

	return export;
}