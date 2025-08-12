//
// cl_entities.c -- Entity parsing and management.
//
// Copyright 1998 Raven Software
//

#include "client.h"
#include "cl_effects.h"
#include "cl_skeletons.h"
#include "Angles.h"
#include "cmodel.h"
#include "EffectFlags.h"
#include "menu.h"
#include "Reference.h"
#include "ResourceManager.h"
#include "Vector.h"

ResourceManager_t cl_FXBufMngr;
int camera_timer; // H2
qboolean viewoffset_changed; // H2

static vec3_t look_angles;

//mxd. Very similar to LERPedReferences_new().
static LERPedReferences_t* AllocateLERPedReference(const int ref_type)
{
	LERPedReferences_t* ref = ResMngr_AllocateResource(&cl_FXBufMngr, sizeof(LERPedReferences_t));

	ref->refType = ref_type;
	ref->jointIDs = jointIDs[ref_type];
	ref->lastUpdate = -(REF_MINCULLTIME * 2.0f);

	memset(ref->references, 0, sizeof(ref->references));
	memset(ref->oldReferences, 0, sizeof(ref->oldReferences));

	return ref;
}

static void DeallocateLERPedReference(void* data) // H2
{
	ResMngr_DeallocateResource(&cl_FXBufMngr, data, sizeof(LERPedReferences_t));
}

void CL_ClearSkeletalEntities(void) // H2
{
	se.StopAllSounds();

	if (fxe.Clear != NULL)
		fxe.Clear();

	centity_t* ent = &cl_entities[0];
	for (int i = 0; i < MAX_NETWORKABLE_EDICTS; i++, ent++)
	{
		if (ent->referenceInfo != NULL)
			DeallocateLERPedReference(ent->referenceInfo);

		memset(ent, 0, sizeof(centity_t));

		ent->baseline.skeletalType = -1;
		ent->baseline.rootJoint = -1;
		ent->baseline.swapFrame = -1;
		ent->current.skeletalType = -1;
		ent->current.rootJoint = -1;
		ent->current.swapFrame = -1;
		ent->prev.skeletalType = -1;
		ent->prev.rootJoint = -1;
		ent->prev.swapFrame = -1;
	}

	memset(skeletal_joints, 0, sizeof(skeletal_joints));
	memset(joint_nodes, 0, sizeof(joint_nodes));
}

// NOTE: mxd. Not used anywhere. Needed to match original quake2.dll exported functions list...
Q2DLL_DECLSPEC qboolean CL_CheckEntity(const int index, const centity_t* ent)
{
	return (index >= 0 && index < MAX_NETWORKABLE_EDICTS && &cl_entities[index] == ent && cl_entities[index].flags != 0); //mxd. Added index sanity checks.
}

// NOTE: mxd. Not used anywhere. Needed to match original quake2.dll exported functions list...
Q2DLL_DECLSPEC centity_t* CL_FindOwner(const int index)
{
	if (index >= 0 && index < MAX_NETWORKABLE_EDICTS) //mxd. Added index sanity checks.
		return &cl_entities[index];

	return NULL;
}

// Returns the entity number and the header bits.
int CL_ParseEntityBits(byte* bf, byte* bfNonZero)
{
	bfNonZero[0] = (byte)MSG_ReadByte(&net_message);

	for (int i = 0; i < NUM_ENTITY_HEADER_BITS; i++)
	{
		if (GetB(bfNonZero, i))
			bf[i] = (byte)MSG_ReadByte(&net_message);
		else
			bf[i] = 0;
	}

	if (GetB(bf, U_NUMBER16))
		return MSG_ReadShort(&net_message);

	return MSG_ReadByte(&net_message);
}

// Can go from either a baseline or a previous packet_entity.
//mxd. Written by MSG_WriteDeltaEntity().
void CL_ParseDelta(const entity_state_t* from, entity_state_t* to, const int number, const byte* bits)
{
	assert(from != NULL); //mxd

	// Set everything to the state we are delta'ing from
	*to = *from;

	// H2: missing VectorCopy(from->origin, to->old_origin);
	to->number = (short)number;

	if (GetB(bits, U_MODEL))
		to->modelindex = (byte)MSG_ReadByte(&net_message);

	if (GetB(bits, U_BMODEL))
	{
		to->bmodel_origin[0] = MSG_ReadCoord(&net_message);
		to->bmodel_origin[1] = MSG_ReadCoord(&net_message);
		to->bmodel_origin[2] = MSG_ReadCoord(&net_message);
	}

	if (GetB(bits, U_FRAME8))
	{
		to->frame = (short)MSG_ReadByte(&net_message);
		if (number == cl.playernum + 1)
			cl.frame.playerstate.frame = to->frame;
	}

	if (GetB(bits, U_FRAME16))
	{
		to->frame = (short)MSG_ReadShort(&net_message);
		if (number == cl.playernum + 1)
			cl.frame.playerstate.frame = to->frame;
	}

	if (GetB(bits, U_SKIN8) && GetB(bits, U_SKIN16))
		to->skinnum = MSG_ReadLong(&net_message);
	else if (GetB(bits, U_SKIN16))
		to->skinnum = MSG_ReadShort(&net_message);
	else if (GetB(bits, U_SKIN8))
		to->skinnum = MSG_ReadByte(&net_message);

	if (GetB(bits, U_CLIENTNUM))
	{
		to->clientnum = (short)MSG_ReadByte(&net_message);
		if (number == cl.playernum + 1)
			cl.frame.playerstate.clientnum = to->clientnum;
	}

	if (GetB(bits, U_SCALE))
		to->scale = (float)(MSG_ReadByte(&net_message)) * 0.01f;

	if (GetB(bits, U_EFFECTS8) || GetB(bits, U_EFFECTS16))
	{
		if (GetB(bits, U_EFFECTS8) && GetB(bits, U_EFFECTS16))
			to->effects = MSG_ReadLong(&net_message);
		else if (GetB(bits, U_EFFECTS16))
			to->effects = MSG_ReadShort(&net_message);
		else // U_EFFECTS8
			to->effects = MSG_ReadByte(&net_message);

		if (number == cl.playernum + 1)
			cl.frame.playerstate.effects = to->effects;
	}

	if (GetB(bits, U_RENDERFX8) || GetB(bits, U_RENDERFX16))
	{
		if (GetB(bits, U_RENDERFX8) && GetB(bits, U_RENDERFX16))
			to->renderfx = MSG_ReadLong(&net_message);
		else if (GetB(bits, U_RENDERFX16))
			to->renderfx = MSG_ReadShort(&net_message);
		else // U_RENDERFX8
			to->renderfx = MSG_ReadByte(&net_message);

		if (number == cl.playernum + 1)
			cl.frame.playerstate.renderfx = to->renderfx;
	}

	if (GetB(bits, U_ORIGIN12))
	{
		to->origin[0] = MSG_ReadCoord(&net_message);
		to->origin[1] = MSG_ReadCoord(&net_message);
	}

	if (GetB(bits, U_ORIGIN3))
		to->origin[2] = MSG_ReadCoord(&net_message);

	if (GetB(bits, U_ANGLE1))
	{
		to->angles[0] = MSG_ReadAngle(&net_message);
		if (number == cl.playernum + 1)
			cl.frame.playerstate.angles[0] = to->angles[0];
	}

	if (GetB(bits, U_ANGLE2))
	{
		to->angles[1] = MSG_ReadAngle(&net_message);
		if (number == cl.playernum + 1)
			cl.frame.playerstate.angles[1] = to->angles[1];
	}

	if (GetB(bits, U_ANGLE3))
	{
		to->angles[2] = MSG_ReadAngle(&net_message);
		if (number == cl.playernum + 1)
			cl.frame.playerstate.angles[2] = to->angles[2];
	}

	if (GetB(bits, U_OLDORIGIN))
		MSG_ReadPos(&net_message, to->old_origin);

	if (GetB(bits, U_SOUND))
	{
		to->sound = (byte)MSG_ReadByte(&net_message);
		to->sound_data = (byte)MSG_ReadByte(&net_message);
	}

	if (GetB(bits, U_SOLID))
		to->solid = (short)MSG_ReadShort(&net_message);

	if (GetB(bits, U_FM_INFO))
	{
		int fmnodeinfo_usage_flags = MSG_ReadByte(&net_message);

		if ((fmnodeinfo_usage_flags & U_FM_HIGH) != 0)
			fmnodeinfo_usage_flags += MSG_ReadByte(&net_message) << 8;

		for (int i = 1; i < MAX_FM_MESH_NODES; i++)
		{
			const int flag = (i < 8 ? i - 1 : i); // Skip U_FM_HIGH flag
			if (((1 << flag) & fmnodeinfo_usage_flags) == 0)
				continue;

			const int fm_flags = MSG_ReadByte(&net_message);

			if (fm_flags & U_FM_FRAME)
			{
				to->fmnodeinfo[i].frame = MSG_ReadByte(&net_message);
				if (number == cl.playernum + 1)
					cl.frame.playerstate.fmnodeinfo[i].frame = to->fmnodeinfo[i].frame;
			}

			if (fm_flags & U_FM_FRAME16)
			{
				to->fmnodeinfo[i].frame += MSG_ReadByte(&net_message) << 8;
				if (number == cl.playernum + 1)
					cl.frame.playerstate.fmnodeinfo[i].frame = to->fmnodeinfo[i].frame;
			}

			if (fm_flags & U_FM_COLOR_R)
			{
				to->fmnodeinfo[i].color.r = (byte)MSG_ReadByte(&net_message);
				if (number == cl.playernum + 1)
					cl.frame.playerstate.fmnodeinfo[i].color.r = to->fmnodeinfo[i].color.r;
			}

			if (fm_flags & U_FM_COLOR_G)
			{
				to->fmnodeinfo[i].color.g = (byte)MSG_ReadByte(&net_message);
				if (number == cl.playernum + 1)
					cl.frame.playerstate.fmnodeinfo[i].color.g = to->fmnodeinfo[i].color.g;
			}

			if (fm_flags & U_FM_COLOR_B)
			{
				to->fmnodeinfo[i].color.b = (byte)MSG_ReadByte(&net_message);
				if (number == cl.playernum + 1)
					cl.frame.playerstate.fmnodeinfo[i].color.b = to->fmnodeinfo[i].color.b;
			}

			if (fm_flags & U_FM_COLOR_A)
			{
				to->fmnodeinfo[i].color.a = (byte)MSG_ReadByte(&net_message);
				if (number == cl.playernum + 1)
					cl.frame.playerstate.fmnodeinfo[i].color.a = to->fmnodeinfo[i].color.a;
			}

			if (fm_flags & U_FM_FLAGS)
			{
				to->fmnodeinfo[i].flags = (byte)MSG_ReadByte(&net_message);
				if (number == cl.playernum + 1)
					cl.frame.playerstate.fmnodeinfo[i].flags = to->fmnodeinfo[i].flags;
			}

			if (fm_flags & U_FM_SKIN)
			{
				to->fmnodeinfo[i].skin = (byte)MSG_ReadByte(&net_message);
				if (number == cl.playernum + 1)
					cl.frame.playerstate.fmnodeinfo[i].skin = to->fmnodeinfo[i].skin;
			}
		}
	}

	if (GetB(bits, U_CLIENT_EFFECTS))
	{
		EffectsBuffer_t* fx_buf = &cl_entities[number].current.clientEffects;

		if (fx_buf->buf == NULL)
		{
			fx_buf->buf = (byte*)ResMngr_AllocateResource(&fx_buffer_manager, ENTITY_FX_BUF_SIZE);
			fx_buf->numEffects = 0;
			fx_buf->bufSize = 0; //mxd. Increased by MSG_ReadEffects().
			fx_buf->freeBlock = 0;
		}

		MSG_ReadEffects(&net_message, fx_buf);

		if (cls.disable_screen && fx_buf->buf != NULL)
		{
			ResMngr_DeallocateResource(&fx_buffer_manager, fx_buf->buf, ENTITY_FX_BUF_SIZE);
			fx_buf->buf = NULL;
			fx_buf->numEffects = 0;
			fx_buf->bufSize = 0;
			fx_buf->freeBlock = 0;
		}
	}

	if (GetB(bits, U_JOINTED))
		MSG_ReadJoints(&net_message, to);

	if (GetB(bits, U_SWAPFRAME))
	{
		to->swapFrame = (short)MSG_ReadShort(&net_message);
		if (number == cl.playernum + 1)
			cl.frame.playerstate.swapFrame = to->swapFrame;
	}

	if (GetB(bits, U_COLOR_R))
		to->color.r = (byte)MSG_ReadByte(&net_message);

	if (GetB(bits, U_COLOR_G))
		to->color.g = (byte)MSG_ReadByte(&net_message);

	if (GetB(bits, U_COLOR_B))
		to->color.b = (byte)MSG_ReadByte(&net_message);

	if (GetB(bits, U_COLOR_A))
		to->color.a = (byte)MSG_ReadByte(&net_message);

	if (GetB(bits, U_ABSLIGHT))
	{
		to->absLight.r = (byte)MSG_ReadByte(&net_message);
		to->absLight.g = (byte)MSG_ReadByte(&net_message);
		to->absLight.b = (byte)MSG_ReadByte(&net_message);
	}

	if (GetB(bits, U_USAGE_COUNT))
		to->usageCount = (byte)MSG_ReadByte(&net_message);
}

// Parses deltas from the given base and adds the resulting entity to the current frame.
static void CL_DeltaEntity(frame_t* frame, const int newnum, const entity_state_t* old, const byte* bits)
{
	centity_t* ent = &cl_entities[newnum];
	entity_state_t* state = &cl_parse_entities[cl.parse_entities & (MAX_PARSE_ENTITIES - 1)];
	cl.parse_entities++;
	frame->num_entities++;

	CL_ParseDelta(old, state, newnum, bits);

	if ((ent->flags & CF_INUSE) && ent->prev.usageCount != state->usageCount) // H2
	{
		fxe.RemoveClientEffects(ent);

		if (ent->prev.rootJoint != -1)
		{
			SK_ClearJoints(ent->prev.rootJoint);
			ent->baseline.rootJoint = -1;
			ent->current.rootJoint = -1;
			ent->prev.rootJoint = -1;
		}

		if (ent->referenceInfo != NULL)
		{
			DeallocateLERPedReference(ent->referenceInfo);
			ent->referenceInfo = NULL;
		}
	}

	// Some data changes will force no lerping.
	if (GetB(bits, U_OLDORIGIN))
		ent->serverframe = -99;

	if (GetB(bits, U_MODEL)) // H2
	{
		struct model_s* model;

		if (ent->current.effects & EF_PLAYER)
		{
			if (cl.clientinfo[ent->current.number].model != NULL)
				model = *cl.clientinfo[ent->current.number].model;
			else
				model = *cl.baseclientinfo.model;
		}
		else
		{
			model = cl.model_draw[ent->current.modelindex];
		}

		if (ent->referenceInfo != NULL)
		{
			DeallocateLERPedReference(ent->referenceInfo);
			ent->referenceInfo = NULL;
		}

		if (model != NULL)
		{
			const int id = re.GetReferencedID(model);
			if (id != -1)
				ent->referenceInfo = AllocateLERPedReference(id);
		}
	}

	if (ent->serverframe != cl.frame.serverframe - 1)
	{
		// Duplicate the current state so lerping doesn't hurt anything.
		ent->prev = *state;
		VectorCopy(state->old_origin, ent->lerp_origin);
	}
	else
	{
		// Shuffle the last state to previous.
		ent->prev = ent->current;
	}

	ent->serverframe = cl.frame.serverframe;

	if (ent->current.clientEffects.buf != NULL)
	{
		ent->prev.clientEffects.buf = ent->current.clientEffects.buf;
		ent->prev.clientEffects.numEffects = ent->current.clientEffects.numEffects;
		ent->prev.clientEffects.freeBlock = ent->current.clientEffects.freeBlock;
		ent->prev.clientEffects.bufSize = ent->current.clientEffects.bufSize;
	}

	ent->current = *state;

	if (ent->prev.clientEffects.buf != NULL)
	{
		ent->current.clientEffects.buf = ent->prev.clientEffects.buf;
		ent->current.clientEffects.freeBlock = ent->prev.clientEffects.freeBlock;
		ent->current.clientEffects.numEffects = ent->prev.clientEffects.numEffects;
		ent->current.clientEffects.bufSize = ent->prev.clientEffects.bufSize;

		ent->prev.clientEffects.buf = NULL;
		ent->prev.clientEffects.numEffects = 0;
		ent->prev.clientEffects.freeBlock = 0;
		ent->prev.clientEffects.bufSize = 0;
	}
}

// An svc_packetentities has just been parsed, deal with the rest of the data stream.
//mxd. Written by SV_EmitPacketEntities().
static void CL_ParsePacketEntities(const frame_t* oldframe, frame_t* newframe)
{
	struct model_s* model;
	byte bits[NUM_ENTITY_HEADER_BITS];
	entity_state_t* oldstate;
	int oldnum;

	newframe->parse_entities = cl.parse_entities;
	newframe->num_entities = 0;

	memset(bits, 0, sizeof(bits));

	// Delta from the entities present in oldframe.
	int oldindex = 0;
	byte unused = 0;

	if (oldframe == NULL || oldframe->num_entities <= 0)
	{
		oldnum = 99999;
		oldstate = NULL; //mxd
	}
	else
	{
		oldstate = &cl_parse_entities[oldframe->parse_entities & (MAX_PARSE_ENTITIES - 1)];
		oldnum = oldstate->number;
	}

	while (true)
	{
		const int newnum = CL_ParseEntityBits(bits, &unused);

		if (newnum >= MAX_EDICTS)
			Com_Error(ERR_DROP, "CL_ParsePacketEntities: bad number:%i", newnum);

		if (net_message.readcount > net_message.cursize)
			Com_Error(ERR_DROP, "CL_ParsePacketEntities: end of message");

		if (newnum == 0)
			break;

		while (oldnum < newnum)
		{
			// One or more entities from the old packet are unchanged.
			if ((int)cl_shownet->value == 3)
				Com_Printf("   unchanged: %i\n", oldnum);

			if ((cl_entities[oldnum].flags & CF_INUSE) && !(cl_entities[oldnum].flags & CF_SERVER_CULLED)) // H2. Extra flags checks
				CL_DeltaEntity(newframe, oldnum, oldstate, NULL);

			oldindex++;

			if (oldindex >= oldframe->num_entities)
			{
				oldnum = 99999;
			}
			else
			{
				oldstate = &cl_parse_entities[(oldframe->parse_entities + oldindex) & (MAX_PARSE_ENTITIES - 1)];
				oldnum = oldstate->number;
			}
		}

		centity_t* ent = &cl_entities[newnum];

		if (GetB(bits, U_REMOVE))
		{
			// The entity present in oldframe is not in the current frame.
			if ((int)cl_shownet->value == 3)
				Com_Printf("   remove: %i\n", newnum);

			ent->flags |= CF_SERVER_CULLED; // H2

			if (GetB(bits, U_ENT_FREED)) // H2
			{
				ent->flags &= ~CF_INUSE;
				fxe.RemoveClientEffects(ent);

				if (ent->prev.rootJoint != -1)
				{
					SK_ClearJoints(ent->prev.rootJoint);
					ent->baseline.rootJoint = -1;
					ent->current.rootJoint = -1;
					ent->prev.rootJoint = -1;
				}

				if (ent->referenceInfo != NULL)
				{
					DeallocateLERPedReference(ent->referenceInfo);
					ent->referenceInfo = NULL;
				}
			}

			if (oldnum == newnum)
			{
				oldindex++;

				if (oldindex >= oldframe->num_entities)
				{
					oldnum = 99999;
				}
				else
				{
					oldstate = &cl_parse_entities[(oldframe->parse_entities + oldindex) & (MAX_PARSE_ENTITIES - 1)];
					oldnum = oldstate->number;
				}
			}

			continue;
		}

		if (oldnum == newnum)
		{
			// Delta from previous state.
			if ((int)cl_shownet->value == 3)
				Com_Printf("   delta: %i\n", newnum);

			CL_DeltaEntity(newframe, newnum, oldstate, bits);
			ent->flags &= ~CF_SERVER_CULLED; // H2
			oldindex++;

			if (oldindex >= oldframe->num_entities)
			{
				oldnum = 99999;
			}
			else
			{
				oldstate = &cl_parse_entities[(oldframe->parse_entities + oldindex) & (MAX_PARSE_ENTITIES - 1)];
				oldnum = oldstate->number;
			}
		}
		else if (newnum < oldnum)
		{
			// Delta from baseline.
			if ((int)cl_shownet->value == 3)
				Com_Printf("   baseline: %i\n", newnum);

			CL_DeltaEntity(newframe, newnum, &ent->baseline, bits);

			ent->flags &= ~CF_SERVER_CULLED; // H2
			ent->flags |= CF_INUSE; // H2

			if (ent->current.effects & EF_PLAYER) // H2
			{
				if (cl.clientinfo[ent->current.number].model != NULL)
					model = *cl.clientinfo[ent->current.number].model;
				else
					model = *cl.baseclientinfo.model;
			}
			else
			{
				model = cl.model_draw[ent->current.modelindex];
			}

			if (ent->referenceInfo != NULL) // H2
			{
				DeallocateLERPedReference(ent->referenceInfo);
				ent->referenceInfo = NULL;
			}

			if (model != NULL) // H2
			{
				const int id = re.GetReferencedID(model);
				if (id != -1)
					ent->referenceInfo = AllocateLERPedReference(id);
			}
		}
	}

	// Any remaining entities in the old frame are copied over.
	while (oldnum != 99999)
	{
		// One or more entities from the old packet are unchanged.
		if ((int)cl_shownet->value == 3)
			Com_Printf("   unchanged: %i\n", oldnum);

		if ((cl_entities[oldnum].flags & CF_INUSE) && !(cl_entities[oldnum].flags & CF_SERVER_CULLED)) // H2. Extra flags checks
			CL_DeltaEntity(newframe, oldnum, oldstate, NULL);

		oldindex++;

		if (oldindex >= oldframe->num_entities)
			break;

		oldstate = &cl_parse_entities[(oldframe->parse_entities + oldindex) & (MAX_PARSE_ENTITIES - 1)];
		oldnum = oldstate->number;
	}
}

//mxd. Written by SV_WritePlayerstateToClient().
static qboolean CL_ParsePlayerstate(const frame_t* oldframe, frame_t* newframe)
{
	byte nonzero_bits[PLAYER_DELNZ_BYTES];
	byte flags[PLAYER_DEL_BYTES];

	player_state_t* state = &newframe->playerstate;

	// Clear to old value before delta parsing.
	if (oldframe != NULL)
		memcpy(state, &oldframe->playerstate, sizeof(player_state_t));
	else
		memset(state, 0, sizeof(player_state_t));

	for (int i = 0; i < PLAYER_DELNZ_BYTES; i++)
		nonzero_bits[i] = (byte)MSG_ReadByte(&net_message);

	for (int i = 0; i < PLAYER_DEL_BYTES; i++)
	{
		if (GetB(nonzero_bits, i))
			flags[i] = (byte)MSG_ReadByte(&net_message);
		else
			flags[i] = 0;
	}

	// Parse the pmove_state_t.
	if (GetB(flags, PS_M_TYPE))
		state->pmove.pm_type = MSG_ReadByte(&net_message);

	if (cl.attractloop)
		state->pmove.pm_type = PM_FREEZE;

	if (GetB(flags, PS_M_ORIGIN_XY))
	{
		state->pmove.origin[0] = (short)MSG_ReadShort(&net_message);
		state->pmove.origin[1] = (short)MSG_ReadShort(&net_message);
	}

	if (GetB(flags, PS_M_ORIGIN_Z))
		state->pmove.origin[2] = (short)MSG_ReadShort(&net_message);

	if (GetB(flags, PS_M_VELOCITY_XY))
	{
		state->pmove.velocity[0] = (short)MSG_ReadShort(&net_message);
		state->pmove.velocity[1] = (short)MSG_ReadShort(&net_message);
	}

	if (GetB(flags, PS_M_VELOCITY_Z))
		state->pmove.velocity[2] = (short)MSG_ReadShort(&net_message);

	if (GetB(flags, PS_M_TIME))
		state->pmove.pm_time = (byte)MSG_ReadByte(&net_message);

	if (GetB(flags, PS_M_FLAGS))
	{
		state->pmove.pm_flags = (byte)MSG_ReadByte(&net_message);
		pred_pm_flags = state->pmove.pm_flags;
	}

	if (GetB(flags, PS_W_FLAGS))
	{
		state->pmove.w_flags = (byte)MSG_ReadByte(&net_message);
		pred_pm_w_flags = state->pmove.w_flags;
	}

	if (GetB(flags, PS_M_GRAVITY))
		state->pmove.gravity = (short)MSG_ReadShort(&net_message);

	if (GetB(flags, PS_M_DELTA_ANGLES))
		for (int i = 0; i < 3; i++)
			state->pmove.delta_angles[i] = (short)MSG_ReadShort(&net_message);

	if (GetB(flags, PS_M_CAMERA_DELTA_ANGLES))
		for (int i = 0; i < 3; i++)
			state->pmove.camera_delta_angles[i] = (short)MSG_ReadShort(&net_message);

	if (GetB(flags, PS_VIEWANGLES))
		for (int i = 0; i < 3; i++)
			state->viewangles[i] = MSG_ReadAngle16(&net_message);

	if (GetB(flags, PS_REMOTE_VIEWANGLES))
		for (int i = 0; i < 3; i++)
			state->remote_viewangles[i] = MSG_ReadAngle16(&net_message);

	if (GetB(flags, PS_REMOTE_VIEWORIGIN))
		for (int i = 0; i < 3; i++)
			state->remote_vieworigin[i] = (float)MSG_ReadShort(&net_message);

	if (GetB(flags, PS_REMOTE_ID))
		state->remote_id = MSG_ReadLong(&net_message);

	if (GetB(flags, PS_VIEWHEIGHT))
		state->viewheight = (short)MSG_ReadShort(&net_message);

	if (GetB(flags, PS_OFFSETANGLES))
	{
		for (int i = 0; i < 3; i++)
			state->offsetangles[i] = MSG_ReadAngle16(&net_message);
	}
	else
	{
		VectorClear(state->offsetangles);
		VectorClear(cl.playerinfo.offsetangles);
	}

	if (GetB(flags, PS_FOV))
		state->fov = (float)MSG_ReadByte(&net_message);

	if (GetB(flags, PS_RDFLAGS))
		state->rdflags = MSG_ReadByte(&net_message);

	if (GetB(flags, PS_AUTOTARGETENTITY))
		state->AutotargetEntityNum = MSG_ReadShort(&net_message);

	if (GetB(flags, PS_MAP_PERCENTAGE))
		state->map_percentage = (byte)MSG_ReadByte(&net_message);

	if (GetB(flags, PS_FOG_DENSITY))
		state->fog_density = MSG_ReadFloat(&net_message);

	if (GetB(flags, PS_MISSION1))
		state->mission_num1 = (short)MSG_ReadShort(&net_message);

	if (GetB(flags, PS_MISSION2))
		state->mission_num2 = (short)MSG_ReadShort(&net_message);

	for (int i = 0; i < MAX_STATS; i++)
		if (GetB(flags, PS_STAT_BIT_0 + i))
			state->stats[i] = (short)MSG_ReadShort(&net_message);

	if (GetB(flags, PS_MINSMAXS))
	{
		for (int i = 0; i < 3; i++)
			state->mins[i] = (float)MSG_ReadShort(&net_message) * 0.125f;

		for (int i = 0; i < 3; i++)
			state->maxs[i] = (float)MSG_ReadShort(&net_message) * 0.125f;
	}

	if (GetB(flags, PS_INVENTORY))
	{
		state->NoOfItems = (byte)MSG_ReadByte(&net_message);

		for (int i = 0; i < state->NoOfItems; i++)
		{
			state->inventory_changes[i] = (byte)MSG_ReadByte(&net_message);
			state->inventory_remaining[i] = (byte)MSG_ReadByte(&net_message);
		}
	}

	if (GetB(flags, PS_GROUNDBITS_NNGE))
		state->NonNullgroundentity = (byte)MSG_ReadByte(&net_message);

	if (GetB(flags, PS_GROUNDPLANE_INFO1) && GetB(flags, PS_GROUNDPLANE_INFO2))
	{
		for (int i = 0; i < 3; i++)
			state->GroundPlane.normal[i] = MSG_ReadFloat(&net_message);
	}
	else if (GetB(flags, PS_GROUNDPLANE_INFO1))
	{
		VectorClear(state->GroundPlane.normal);
	}
	else
	{
		VectorSet(state->GroundPlane.normal, 0, 0, 1);
	}

	if (GetB(flags, PS_GROUNDBITS_GC))
		state->GroundContents = (MSG_ReadByte(&net_message) << 16);

	if (GetB(flags, PS_GROUNDBITS_SURFFLAGS))
		state->GroundSurface.flags = MSG_ReadLong(&net_message);

	if (GetB(flags, PS_WATERTYPE))
		state->watertype = MSG_ReadLong(&net_message);

	if (GetB(flags, PS_WATERLEVEL))
		state->waterlevel = MSG_ReadLong(&net_message);

	if (GetB(flags, PS_WATERHEIGHT))
		state->waterheight = MSG_ReadFloat(&net_message);

	if (GetB(flags, PS_GRABLOC0))
		state->grabloc[0] = MSG_ReadFloat(&net_message);

	if (GetB(flags, PS_GRABLOC1))
		state->grabloc[1] = MSG_ReadFloat(&net_message);

	if (GetB(flags, PS_GRABLOC2))
		state->grabloc[2] = MSG_ReadFloat(&net_message);

	if (GetB(flags, PS_GRABANGLE))
		state->grabangle = MSG_ReadFloat(&net_message);

	if (GetB(flags, PS_FWDVEL))
		state->fwdvel = MSG_ReadFloat(&net_message);

	if (GetB(flags, PS_SIDEVEL))
		state->sidevel = MSG_ReadFloat(&net_message);

	if (GetB(flags, PS_UPVEL))
		state->upvel = MSG_ReadFloat(&net_message);

	if (GetB(flags, PS_FLAGS))
		state->flags = MSG_ReadByte(&net_message);

	if (GetB(flags, PS_EDICTFLAGS))
		state->edictflags = MSG_ReadLong(&net_message);

	if (GetB(flags, PS_OLDVELOCITY_Z))
		state->oldvelocity_z = MSG_ReadFloat(&net_message);

	if (GetB(flags, PS_UPPERSEQ))
		state->upperseq = MSG_ReadByte(&net_message);

	if (GetB(flags, PS_LOWERSEQ))
		state->lowerseq = MSG_ReadByte(&net_message);

	if (GetB(flags, PS_FRAMEINFO1) && GetB(flags, PS_FRAMEINFO2))
	{
		state->lowerframe = MSG_ReadShort(&net_message);
		state->upperframe = MSG_ReadShort(&net_message);
	}
	else if (GetB(flags, PS_FRAMEINFO1))
	{
		state->lowerframe = MSG_ReadShort(&net_message);
		state->upperframe = state->lowerframe;
	}

	if (GetB(flags, PS_UPPERIDLE))
		state->upperidle = (byte)MSG_ReadByte(&net_message);

	if (GetB(flags, PS_LOWERIDLE))
		state->loweridle = (byte)MSG_ReadByte(&net_message);

	if (GetB(flags, PS_UPPERMOVE_INDEX))
		state->uppermove_index = MSG_ReadByte(&net_message);

	if (GetB(flags, PS_LOWERMOVE_INDEX))
		state->lowermove_index = MSG_ReadByte(&net_message);

	if (GetB(flags, PS_WEAPON))
		state->weapon = (byte)MSG_ReadByte(&net_message);

	if (GetB(flags, PS_DEFENSE))
		state->defense = (byte)MSG_ReadByte(&net_message);

	if (GetB(flags, PS_LASTWEAPON))
		state->lastweapon = (byte)MSG_ReadByte(&net_message);

	if (GetB(flags, PS_LASTDEFENSE))
		state->lastdefense = (byte)MSG_ReadByte(&net_message);

	if (GetB(flags, PS_WEAPONREADY))
		state->weaponready = MSG_ReadByte(&net_message);

	if (GetB(flags, PS_SWITCHTOWEAPON))
		state->switchtoweapon = MSG_ReadByte(&net_message);

	if (GetB(flags, PS_NEWWEAPON))
		state->newweapon = (byte)MSG_ReadByte(&net_message);

	if (GetB(flags, PS_WEAP_AMMO_INDEX))
		state->weap_ammo_index = MSG_ReadByte(&net_message);

	if (GetB(flags, PS_DEF_AMMO_INDEX))
		state->def_ammo_index = MSG_ReadByte(&net_message);

	if (GetB(flags, PS_WEAPONCHARGE))
		state->weaponcharge = (byte)MSG_ReadByte(&net_message);

	if (GetB(flags, PS_ARMORTYPE))
		state->armortype = MSG_ReadByte(&net_message);

	if (GetB(flags, PS_BOWTYPE))
		state->bowtype = (byte)MSG_ReadByte(&net_message);

	if (GetB(flags, PS_STAFFLEVEL))
		state->stafflevel = (byte)MSG_ReadByte(&net_message);

	if (GetB(flags, PS_HELLTYPE))
		state->helltype = (byte)MSG_ReadByte(&net_message);

	if (GetB(flags, PS_METEORCOUNT))
		state->meteor_count = (byte)MSG_ReadByte(&net_message);

	if (GetB(flags, PS_HANDFXTYPE))
		state->handfxtype = (byte)MSG_ReadByte(&net_message);

	if (GetB(flags, PS_PLAGUELEVEL))
		state->plaguelevel = (byte)MSG_ReadByte(&net_message);

	if (GetB(flags, PS_SKINTYPE))
		state->skintype = (short)MSG_ReadShort(&net_message);

	if (GetB(flags, PS_ALTPARTS))
		state->altparts = MSG_ReadLong(&net_message);

	if (GetB(flags, PS_DEADFLAG))
		state->deadflag = MSG_ReadLong(&net_message);

	if (GetB(flags, PS_IDEAL_YAW))
		state->ideal_yaw = MSG_ReadFloat(&net_message);

	state->leveltime = MSG_ReadFloat(&net_message);

	if (GetB(flags, PS_IDLETIME))
		state->idletime = MSG_ReadFloat(&net_message);

	if (GetB(flags, PS_POWERUP_TIMER))
		state->powerup_timer = MSG_ReadFloat(&net_message);

	if (GetB(flags, PS_QUICKTURN_RATE))
		state->quickturn_rate = MSG_ReadFloat(&net_message);

	if (GetB(flags, PS_DMFLAGS))
		state->dmflags = MSG_ReadLong(&net_message);

	if (GetB(flags, PS_ADVANCEDSTAFF))
		state->advancedstaff = (byte)MSG_ReadByte(&net_message);

	if (GetB(flags, PS_CINEMATIC))
	{
		state->cinematicfreeze = (byte)MSG_ReadByte(&net_message);
		Cvar_Set("cl_cinematicfreeze", (state->cinematicfreeze ? "1" : "0"));
	}

	if (GetB(flags, PS_PIV))
		state->PIV = MSG_ReadLong(&net_message);

	return (GetB(flags, PS_OFFSETANGLES) != 0);
}

void CL_ParseFrame(void)
{
	frame_t* old;

	memset(&cl.frame, 0, sizeof(cl.frame));

	cl.frame.serverframe = MSG_ReadLong(&net_message);
	cl.frame.deltaframe = MSG_ReadLong(&net_message);
	cl.frame.servertime = cl.frame.serverframe * 100;

	// H2: missing: BIG HACK to let old demos continue to work
	if ((int)cl_shownet->value == 3)
		Com_Printf("   frame:%i  delta:%i\n", cl.frame.serverframe, cl.frame.deltaframe);

	// If the frame is delta compressed from data that we no longer have available,
	// we must suck up the rest of the frame, but not use it, then ask for a non-compressed message.
	if (cl.frame.deltaframe <= 0)
	{
		cl.frame.valid = true; // Uncompressed frame.
		cls.demowaiting = false; // We can start recording now.
		old = NULL;
	}
	else
	{
		old = &cl.frames[cl.frame.deltaframe & UPDATE_MASK];
		if (!old->valid)
			Com_Printf("Delta from invalid frame (not supposed to happen!).\n"); // Should never happen.

		if (old->serverframe != cl.frame.deltaframe)
		{
			// The frame that the server did the delta from is too old, so we can't reconstruct it properly.
			Com_Printf("Delta frame too old.\n"); 
		}
		else if (cl.parse_entities - old->parse_entities > MAX_PARSE_ENTITIES - 128)
		{
			Com_Printf("Delta parse_entities too old.\n");
		}
		else
		{
			// Valid delta parse.
			cl.frame.valid = true; 
		}
	}

	// Clamp time.
	cl.time = ClampI(cl.time, cl.frame.servertime - 100, cl.frame.servertime);

	// Read areabits.
	const int len = MSG_ReadByte(&net_message);
	MSG_ReadData(&net_message, cl.frame.areabits, len);

	// Read playerinfo.
	int cmd = MSG_ReadByte(&net_message);
	SHOWNET(svc_strings[cmd]);

	if (cmd != svc_playerinfo)
		Com_Error(ERR_DROP, "CL_ParseFrame: not playerinfo");

	viewoffset_changed = CL_ParsePlayerstate(old, &cl.frame); // H2: extra return from CL_ParsePlayerstate.

	// Read packet entities.
	cmd = MSG_ReadByte(&net_message);
	SHOWNET(svc_strings[cmd]);

	if (cmd != svc_packetentities)
		Com_Error(ERR_DROP, "CL_ParseFrame: not packetentities");

	CL_ParsePacketEntities(old, &cl.frame);

	// Save the frame off in the backup array for later delta comparisons.
	cl.frames[cl.frame.serverframe & UPDATE_MASK] = cl.frame;

	if (cl.frame.valid)
	{
		// Getting a valid frame message ends the connection process.
		if (cls.state != ca_active)
		{
			cls.state = ca_active;
			cl.force_refdef = true;

			for (int i = 0; i < 3; i++)
			{
				cl.predicted_origin[i] = (float)cl.frame.playerstate.pmove.origin[i] * 0.125f;
				cl.predicted_angles[i] = cl.frame.playerstate.viewangles[i];
				cl.viewangles[i] = (float)cl.frame.playerstate.pmove.camera_delta_angles[i] * SHORT_TO_ANGLE; // H2
			}

			if (cl.refresh_prepped && cls.disable_servercount != cl.servercount)
				SCR_EndLoadingPlaque(); // Get rid of loading plaque.

			CL_ResetPlayerInfo(); // H2
			Reset_Screen_Shake(); // H2

			//mxd. Un-pause the game (game will still be paused when loading map from console) (Q1-style behaviour).
			Cvar_SetValue("paused", 0.0f);
		}

		cl.sound_prepped = true; // Can start mixing ambient sounds.
		CL_CheckPredictionError();
	}
}

static void CL_UpdateWallDistances(void) // H2
{
#define NUM_WALL_CHECKS	(sizeof(cl.wall_dist) / sizeof(cl.wall_dist[0]))

	static int eax_presets[7][4] = // [world preset][sound preset]
	{
		{ EAX_ENVIRONMENT_GENERIC,		EAX_ENVIRONMENT_ROOM,		EAX_ENVIRONMENT_HALLWAY,		EAX_ENVIRONMENT_ALLEY }, // EAX_GENERIC
		{ EAX_ENVIRONMENT_QUARRY,		EAX_ENVIRONMENT_CAVE,		EAX_ENVIRONMENT_STONECORRIDOR,	EAX_ENVIRONMENT_ALLEY }, // EAX_ALL_STONE
		{ EAX_ENVIRONMENT_ARENA,		EAX_ENVIRONMENT_LIVINGROOM,	EAX_ENVIRONMENT_HALLWAY,		EAX_ENVIRONMENT_ALLEY }, // EAX_ARENA
		{ EAX_ENVIRONMENT_CITY,			EAX_ENVIRONMENT_ROOM,		EAX_ENVIRONMENT_SEWERPIPE,		EAX_ENVIRONMENT_ALLEY }, // EAX_CITY_AND_SEWERS
		{ EAX_ENVIRONMENT_CITY,			EAX_ENVIRONMENT_STONEROOM,	EAX_ENVIRONMENT_STONECORRIDOR,	EAX_ENVIRONMENT_ALLEY }, // EAX_CITY_AND_ALLEYS
		{ EAX_ENVIRONMENT_FOREST,		EAX_ENVIRONMENT_ROOM,		EAX_ENVIRONMENT_HALLWAY,		EAX_ENVIRONMENT_ALLEY }, // EAX_FOREST
		{ EAX_ENVIRONMENT_PSYCHOTIC,	EAX_ENVIRONMENT_PSYCHOTIC,	EAX_ENVIRONMENT_PSYCHOTIC,		EAX_ENVIRONMENT_PSYCHOTIC }, // EAX_PSYCHOTIC
	};

	vec3_t start;
	vec3_t end;
	trace_t tr;

	VectorCopy(PlayerEntPtr->origin, start);
	VectorCopy(PlayerEntPtr->origin, end);

	switch (cl.wall_check)
	{
		case 0:
			end[2] += 800.0f;
			break;

		case 1:
			end[0] += 800.0f;
			end[2] += 100.0f;
			break;

		case 2:
			end[1] -= 800.0f;
			end[2] += 100.0f;
			break;

		case 3:
			end[1] += 800.0f;
			end[2] += 100.0f;
			break;

		case 4:
			end[0] -= 800.0f;
			end[2] += 100.0f;
			break;

		default:
			return;
	}

	CL_Trace(start, NULL, NULL, end, MASK_PLAYERSOLID | CONTENTS_WORLD_ONLY, CONTENTS_DETAIL, &tr);
	Vec3SubtractAssign(start, tr.endpos);
	cl.wall_dist[cl.wall_check] = VectorLength(tr.endpos);

	int sound_preset;
	if (cl.wall_dist[1] + cl.wall_dist[3] < 250.0f || cl.wall_dist[2] + cl.wall_dist[4] < 250.0f)
		sound_preset = 1;
	else
		sound_preset = 0;

	if (cl.wall_dist[0] <= 790.0f)
		sound_preset += 2;

	const int world_preset = Q_ftol(EAX_default->value);

	Cvar_SetValue("EAX_preset", (float)eax_presets[world_preset][sound_preset]);
	cl.wall_check = (cl.wall_check + 1) % NUM_WALL_CHECKS;
}

// Q2 counterpart
static void vectoangles2(vec3_t value1, vec3_t angles)
{
	if (value1[0] != 0.0f || value1[1] != 0.0f)
	{
		float yaw = atan2f(value1[1], value1[0]) * RAD_TO_ANGLE;
		if (yaw < 0.0f)
			yaw += 360.0f;

		const float forward = sqrtf(value1[0] * value1[0] + value1[1] * value1[1]);
		float pitch = atan2f(value1[2], forward) * RAD_TO_ANGLE;
		if (pitch < 0.0f)
			pitch += 360.0f;

		VectorSet(angles, -pitch, yaw, 0.0f);
	}
	else if (value1[2] > 0.0f)
	{
		VectorSet(angles , -90.0f, 0.0f, 0.0f);
	}
	else
	{
		VectorSet(angles, -270.0f, 0.0f, 0.0f);
	}
}

static void CL_UpdateCameraOrientation(const float cam_fwd_offset, const qboolean interpolate) // H2 //mxd. Flipped 'interpolate' arg logic.
{
#define MAX_CAMERA_TIMER	500
#define MASK_CAMERA			(CONTENTS_SOLID | CONTENTS_ILLUSIONARY | CONTENTS_CAMERABLOCK)

	typedef enum
	{
		CM_DEFAULT,			// When on land.
		CM_DIVE,			// When swimming underwater.
		CM_SWIM,			// When swimming on water surface.
		CM_LIQUID_DEATH,	// When died in lava/slime (but not in water).
	} cam_mode_e;

	static cam_mode_e cam_mode;
	static qboolean cam_timer_reset;
	static vec3_t old_vieworg;
	static vec3_t old_viewangles;
	static vec3_t prev_start;
	static vec3_t prev_prev_start;
	static vec3_t prev_end;
	static vec3_t prev_prev_end;

	if (cls.state != ca_active)
		return;

	const vec3_t mins = { -1.0f, -1.0f, -1.0f };
	const vec3_t maxs = {  1.0f,  1.0f,  1.0f };
	const vec3_t mins_2 = { -3.0f, -3.0f, -3.0f };
	const vec3_t maxs_2 = {  3.0f,  3.0f,  3.0f };

	const int water_flags = ((int)cl_predict->value ? cl.playerinfo.pm_w_flags : cl.frame.playerstate.pmove.w_flags);
	const int waterlevel =  ((int)cl_predict->value ? cl.playerinfo.waterlevel : cl.frame.playerstate.waterlevel);

	float fwd_offset = cam_fwd_offset;

	if ((int)cl_camera_fpmode->value)
	{
		fwd_offset += cl_camera_fpheight->value;
		cam_transparency = cl_camera_fptrans->value;
	}
	else
	{
		fwd_offset += cl_camera_fpoffs->value + 16.0f;
		cam_transparency = cl_playertrans->value;
	}

	vec3_t forward;
	vec3_t up;
	AngleVectors(look_angles, forward, NULL, up);

	const cam_mode_e prev_cam_mode = cam_mode;

	vec3_t start;
	vec3_t end;
	VectorCopy(PlayerEntPtr->origin, start);

	if (water_flags != 0)
	{
		if (water_flags & (WF_SURFACE | WF_DIVE | WF_DIVING))
		{
			VectorCopy(start, end);

			start[2] += 100.0f;
			end[2] -= 100.0f;

			trace_t tr;
			CL_Trace(start, mins, maxs, end, MASK_WATER | CONTENTS_CAMERABLOCK, CONTENTS_DETAIL | CONTENTS_TRANSLUCENT, &tr);

			if (tr.fraction != 1.0f)
			{
				start[2] = tr.endpos[2];
				VectorMA(start, fwd_offset - 9.5f, up, end);

				cam_mode = CM_SWIM;
			}
		}
		else
		{
			VectorMA(start, fwd_offset, forward, end);
			cam_mode = CM_DIVE;
		}
	}
	else
	{
		VectorMA(start, fwd_offset, up, end);

		trace_t tr;
		CL_Trace(start, mins_2, maxs_2, end, MASK_CAMERA, CONTENTS_DETAIL | CONTENTS_TRANSLUCENT, &tr);

		if (tr.fraction != 1.0f)
			VectorCopy(tr.endpos, end);

		end[2] -= 4.0f;

		if (CL_PMpointcontents(end) & MASK_WATER)
		{
			vec3_t tmp_end;
			VectorCopy(start, tmp_end);

			start[2] += 100.0f;
			tmp_end[2] -= 100.0f;

			CL_Trace(start, mins, maxs, tmp_end, MASK_WATER | CONTENTS_CAMERABLOCK, CONTENTS_DETAIL | CONTENTS_TRANSLUCENT, &tr);

			if (tr.fraction != 1.0f)
			{
				start[2] = tr.endpos[2] * 2.0f - end[2] + 4.0f;
				VectorMA(start, fwd_offset, up, end);

				cam_mode = CM_LIQUID_DEATH;
			}
		}
		else
		{
			VectorMA(start, fwd_offset, up, end);
			cam_mode = CM_DEFAULT;
		}
	}

	// Interpolate position when switching camera mode.
	if (prev_cam_mode != cam_mode)
	{
		if (!cam_timer_reset)
		{
			camera_timer = 0;
			cam_timer_reset = true;
		}

		VectorCopy(prev_start, prev_prev_start);
		VectorCopy(prev_end, prev_prev_end);
	}

	if (cam_timer_reset)
	{
		if (camera_timer >= MAX_CAMERA_TIMER)
		{
			cam_timer_reset = false;
		}
		else
		{
			VectorCopy(start, prev_start);
			VectorCopy(end, prev_end);

			const float lerp = (float)camera_timer / (float)MAX_CAMERA_TIMER;
			for (int i = 0; i < 3; i++)
			{
				start[i] = prev_prev_start[i] + (prev_start[i] - prev_prev_start[i]) * lerp;
				end[i] = prev_prev_end[i] + (prev_end[i] - prev_prev_end[i]) * lerp;
			}
		}
	}
	else
	{
		VectorCopy(start, prev_start);
		VectorCopy(end, prev_end);
	}

	trace_t trace;
	CL_Trace(start, mins_2, maxs_2, end, MASK_CAMERA, CONTENTS_DETAIL | CONTENTS_TRANSLUCENT, &trace);

	if (trace.fraction != 1.0f)
		VectorCopy(trace.endpos, end);

	float viewdist;
	if ((int)cl_camera_fpmode->value)
		viewdist = -cl_camera_fpdist->value;
	else
		viewdist = -cl_camera_viewdist->value;

	vec3_t end_2;
	VectorMA(end, viewdist, forward, end_2);

	if ((water_flags & WF_SWIMFREE) && (CL_PMpointcontents(end) & MASK_WATER))
	{
		CL_Trace(end_2, mins, maxs, end, MASK_WATER | CONTENTS_CAMERABLOCK, CONTENTS_DETAIL | CONTENTS_TRANSLUCENT, &trace);

		if (!trace.startsolid && trace.fraction != 1.0f)
			for (int i = 0; i < 3; i++)
				end_2[i] = end[i] + (trace.endpos[i] - end[i]) * 0.9f;
	}

	vec3_t end_3;
	if (!(int)cl_camera_clipdamp->value)
		VectorCopy(end_2, end_3);

	if (interpolate)
	{
		float damp_factor = 0.0f;

		if ((int)cl_camera_fpmode->value)
		{
			damp_factor = 1.0f;
		}
		else if (cl_camera_dampfactor->value != 0.0f)
		{
			damp_factor = fabsf(look_angles[PITCH]);
			damp_factor = min(89.0f, damp_factor);
			damp_factor /= 89.0f;

			damp_factor = (1.0f - cl_camera_dampfactor->value) * damp_factor * damp_factor * damp_factor + cl_camera_dampfactor->value;
		}

		for (int i = 0; i < 3; i++)
			end_2[i] = old_vieworg[i] + (end_2[i] - old_vieworg[i]) * damp_factor;
	}

	CL_Trace(end, mins, maxs, end_2, MASK_CAMERA, CONTENTS_DETAIL | CONTENTS_TRANSLUCENT, &trace);

	if (trace.fraction != 1.0f)
	{
		if (cl_camera_clipdamp->value == 1.0f)
		{
			VectorCopy(trace.endpos, end_2);
		}
		else
		{
			VectorCopy(end_3, end_2);

			CL_Trace(end, mins, maxs, end_2, MASK_CAMERA, CONTENTS_DETAIL | CONTENTS_TRANSLUCENT, &trace);

			if (trace.fraction != 1.0f)
				VectorCopy(trace.endpos, end_2);
		}
	}

	if (waterlevel == 0 || (water_flags & ~WF_SWIMFREE) || (water_flags == 0 && in_down.state == 0))
	{
		const float roll_scaler = 1.0f - fabsf(look_angles[PITCH] / 89.0f);
		const vec3_t v = { mins[0], mins[1], -1.0f - roll_scaler * 2.0f };

		CL_Trace(end, v, maxs, end_2, MASK_WATER | CONTENTS_CAMERABLOCK, CONTENTS_DETAIL | CONTENTS_TRANSLUCENT, &trace);

		if (trace.fraction != 1.0f)
			VectorCopy(trace.endpos, end_2);
	}

	vec3_t diff;
	VectorSubtract(end, end_2, diff);

	if (cl_camera_viewmax->value < VectorLength(diff))
	{
		VectorNormalize(diff);
		VectorMA(end, -cl_camera_viewmax->value, diff, end_2);
	}

	// Copy calculated angles to refdef.
	vec3_t viewangles;
	VectorSubtract(end, end_2, viewangles);
	VectorNormalize(viewangles);

	vectoangles2(viewangles, cl.refdef.viewangles);

	VectorCopy(end_2, cl.refdef.vieworg);

	if ((int)cl_camera_freeze->value)
	{
		VectorCopy(old_viewangles, cl.refdef.viewangles);
		VectorCopy(old_vieworg, cl.refdef.vieworg);
	}
	else
	{
		VectorCopy(cl.refdef.viewangles, old_viewangles);
		VectorCopy(cl.refdef.vieworg, old_vieworg);
	}

	// Apply screen shake.
	vec3_t shake_amount;
	Perform_Screen_Shake(shake_amount, (float)cl.time);
	Vec3AddAssign(shake_amount, cl.refdef.vieworg);

	VectorCopy(PlayerEntPtr->origin, cl.refdef.clientmodelorg);
}

// Sets cl.refdef view values.
static void CL_CalcViewValues(void)
{
	static vec3_t old_viewoffset;
	static float frame_delta;

	const float lerp = cl.lerpfrac;
	const player_state_t* ps = &cl.frame.playerstate;

	const int frame = (cl.frame.serverframe - 1) & UPDATE_MASK;
	frame_t* oldframe = &cl.frames[frame];
	qboolean no_cam_lerp = false;

	if (oldframe->serverframe != cl.frame.serverframe - 1 || !oldframe->valid)
		oldframe = &cl.frame; // Previous frame was dropped or invalid.

	// See if the player entity was teleported this frame. //mxd. Original H2 logic doesn't seem to do abs() here. A bug?
	if (abs(oldframe->playerstate.pmove.origin[0] - ps->pmove.origin[0]) > 256 * 8 ||
		abs(oldframe->playerstate.pmove.origin[1] - ps->pmove.origin[1]) > 256 * 8 ||
		abs(oldframe->playerstate.pmove.origin[2] - ps->pmove.origin[2]) > 256 * 8)
	{
		// Don't interpolate.
		oldframe = &cl.frame;
		no_cam_lerp = true;
	}

	player_state_t* ops = &oldframe->playerstate;

	// Calculate the origin.
	if (ps->remote_id < 0 && ps->pmove.pm_type != PM_INTERMISSION)
	{
		if ((int)cl_predict->value)
		{
			VectorCopy(cl.predicted_origin, PlayerEntPtr->origin);
		}
		else
		{
			for (int c = 0; c < 3; c++)
				PlayerEntPtr->origin[c] = ((float)ops->pmove.origin[c] + (float)(ps->pmove.origin[c] - ops->pmove.origin[c]) * lerp) * 0.125f;
		}

		VectorCopy(PlayerEntPtr->origin, PlayerEntPtr->oldorigin);
	}

	if (viewoffset_changed)
	{
		vec3_t viewoffset;

		if ((int)cl_predict->value)
			VectorSubtract(cl.playerinfo.offsetangles, ps->offsetangles, viewoffset);
		else
			VectorSubtract(ps->offsetangles, ops->offsetangles, viewoffset);

		for (int i = 0; i < 3; i++)
		{
			if (viewoffset[i] != old_viewoffset[i])
			{
				cl.inputangles[i] += viewoffset[i];
				cl.viewangles[i] += viewoffset[i];
			}
		}

		VectorCopy(viewoffset, old_viewoffset);
		viewoffset_changed = false;
	}

	if (ps->pmove.pm_type == PM_INTERMISSION)
	{
		for (int i = 0; i < 3; i++)
		{
			cl.refdef.vieworg[i] = (float)ps->pmove.origin[i] * 0.125f;
			cl.refdef.viewangles[i] = ps->viewangles[i];
		}
	}
	else
	{
		if (ps->pmove.pm_type == PM_FREEZE)
		{
			for (int i = 0; i < 3; i++)
				cl.predicted_angles[i] = LerpAngle(ops->viewangles[i], ps->viewangles[i], lerp);

			VectorCopy(cl.predicted_angles, look_angles);
		}
		else if (in_lookaround.state & 1)
		{
			for (int i = 0; i < 3; i++)
				look_angles[i] = cl.lookangles[i] + (float)ps->pmove.delta_angles[i] * SHORT_TO_ANGLE;
		}
		else
		{
			VectorCopy(cl.predicted_angles, look_angles);
		}

		if (ps->remote_id < 0) // When not looking through a remote camera.
		{
			const float cam_fwd_offset = (float)ops->viewheight + (float)(ps->viewheight - ops->viewheight) * lerp;

			if (no_cam_lerp)
			{
				CL_UpdateCameraOrientation(cam_fwd_offset, false);
			}
			else
			{
				frame_delta += cls.rframetime * vid_maxfps->value; //mxd. cls.frametime * cl_maxfps->value in original logic.
				const int num_frames = (int)roundf(frame_delta);

				for (int i = 0; i < num_frames; i++)
					CL_UpdateCameraOrientation(cam_fwd_offset, true);

				frame_delta -= (float)num_frames;
			}
		}
		else
		{
			if (ps->remote_id != ops->remote_id)
			{
				ops->remote_id = ps->remote_id;

				VectorCopy(ps->remote_vieworigin, ops->remote_vieworigin);
				VectorCopy(ps->remote_viewangles, ops->remote_viewangles);
			}

			// Just use interpolated values.
			for (int i = 0; i < 3; i++)
			{
				cl.refdef.vieworg[i] = (ops->remote_vieworigin[i] + (ps->remote_vieworigin[i] - ops->remote_vieworigin[i]) * lerp) * 0.125f;
				cl.refdef.viewangles[i] = LerpAngle(ops->remote_viewangles[i], ps->remote_viewangles[i], lerp);
			}
		}
	}

	AngleVectors(cl.refdef.viewangles, cl.v_forward, cl.v_right, cl.v_up);

	VectorCopy(cl.refdef.viewangles, cl.camera_viewangles);
	VectorCopy(cl.refdef.vieworg, cl.camera_vieworigin);

	// H2: Update EAX preset.
	if (CL_PMpointcontents(cl.camera_vieworigin) & CONTENTS_WATER)
	{
		if (cl_camera_under_surface->value != 1.0f && !(int)menus_active->value && se.SetEaxEnvironment != NULL)
		{
			Cvar_SetValue("EAX_preset", EAX_ENVIRONMENT_UNDERWATER); // H2_1.07: 22 -> 44.
			se.SetEaxEnvironment(EAX_ENVIRONMENT_UNDERWATER);
		}

		cl_camera_under_surface->value = 1.0f;
	}
	else
	{
		if (!(int)menus_active->value && se.SetEaxEnvironment != NULL)
		{
			CL_UpdateWallDistances();
			se.SetEaxEnvironment((int)EAX_preset->value);
		}

		cl_camera_under_surface->value = 0.0f;
	}

	// H2: Apply screen shake.
	vec3_t shake;
	Perform_Screen_Shake(shake, (float)cl.time);
	Vec3AddAssign(shake, cl.refdef.vieworg);

	// Interpolate field of view.
	cl.refdef.fov_x = ops->fov + (ps->fov - ops->fov) * lerp;
}

// Emits all entities, particles, and lights to the refresh.
void CL_AddEntities(void)
{
	if (cls.state != ca_active)
		return;

	if (cl.time > cl.frame.servertime)
		cl.lerpfrac = 1.0f;
	else if (cl.time < cl.frame.servertime - 100)
		cl.lerpfrac = 0.0f;
	else
		cl.lerpfrac = 1.0f - (float)(cl.frame.servertime - cl.time) * 0.01f;

	if ((int)cl_timedemo->value)
		cl.lerpfrac = 1.0f;

	fxe.AddPacketEntities(&cl.frame);
	fxe.AddEffects((qboolean)cl_freezeworld->value);

	CL_CalcViewValues();
}

// Q2 counterpart
// The sound code makes callbacks to the client for entitiy position information, so entities can be dynamically re-spatialized.
Q2DLL_DECLSPEC void CL_GetEntitySoundOrigin(const int ent, vec3_t org) //mxd. No longer used by sound dll. Kept to maintain compatibility with original logic.
{
	if (ent >= 0 && ent < MAX_EDICTS)
		VectorCopy(cl_entities[ent].lerp_origin, org);
	else
		Com_Error(ERR_DROP, "CL_GetEntitySoundOrigin: bad ent");

	// FIXME: bmodel issues...
}

void CL_Trace(const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, const int brushmask, const int flags, trace_t* t) // H2
{
	t->ent = NULL;

	if (mins == NULL)
		mins = vec3_origin;

	if (maxs == NULL)
		maxs = vec3_origin;

	if (flags & CONTENTS_DETAIL)
	{
		CM_BoxTrace(start, end, mins, maxs, 0, brushmask, t);

		if (t->fraction < 1.0f)
			t->ent = (struct edict_s*)(-1);

		if (t->startsolid || t->allsolid)
			return;
	}
	else
	{
		memset(t, 0, sizeof(trace_t));
	}

	if (flags & CONTENTS_TRANSLUCENT)
	{
		if (brushmask & CONTENTS_CAMERABLOCK)
			trace_ignore_camera = true;

		if (brushmask & CONTENTS_WATER)
			trace_check_water = true;

		CL_ClipMoveToEntities(start, mins, maxs, end, t);

		if (brushmask & CONTENTS_CAMERABLOCK)
			trace_ignore_camera = false;

		if (brushmask & CONTENTS_WATER)
			trace_check_water = false;
	}
}