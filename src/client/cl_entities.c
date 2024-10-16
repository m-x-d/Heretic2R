//
// cl_entities.c -- Entity parsing and management.
//
// Copyright 1998 Raven Software
//

#include "client.h"
#include "cl_effects.h"
#include "cl_skeletons.h"
#include "Angles.h"
#include "EffectFlags.h"
#include "Reference.h"
#include "ResourceManager.h"
#include "sound.h"
#include "Vector.h"

ResourceManager_t cl_FXBufMngr;
int camera_timer; // H2
qboolean viewoffset_changed; // H2

//TODO: ref_type can potentially be in 0 .. 3 range. Why only joint_id_types[0] is defined?
static LERPedReferences_t* AllocateLERPedReference(const int ref_type)
{
	static int joint_ids[] = { 1, 1, -1, -1, 1, 1, 1 };
	static int* joint_id_types[] = { joint_ids, NULL, NULL, NULL };

	LERPedReferences_t* ref = ResMngr_AllocateResource(&cl_FXBufMngr, sizeof(LERPedReferences_t));

	ref->refType = ref_type;
	ref->jointIDs = joint_id_types[ref_type];
	ref->lastUpdate = -2.0f;

	memset(ref->references, 0, sizeof(ref->references));
	memset(ref->oldReferences, 0, sizeof(ref->oldReferences));

	return ref;
}

static void DeallocateLERPedReference(void* data) // H2
{
	NOT_IMPLEMENTED
}

void CL_ClearSkeletalEntities(void) // H2
{
	S_StopAllSounds();

	if (fxe.Clear != NULL)
		fxe.Clear();

	centity_t* ent = cl_entities;
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
void CL_ParseDelta(const entity_state_t* from, entity_state_t* to, const int number, const byte* bits)
{
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
		to->scale = (float)MSG_ReadByte(&net_message) * 0.01f;

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
			fx_buf->bufSize = 0;
			fx_buf->freeBlock = 0;
		}

		MSG_ReadEffects(&net_message, fx_buf);

		if (cls.disable_screen != 0.0f && fx_buf->buf != NULL)
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

	if ((ent->flags & 1) && ent->prev.usageCount != state->usageCount) // H2
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

		if (newnum  >= MAX_EDICTS)
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

			if ((cl_entities[oldnum].flags & 1) && !(cl_entities[oldnum].flags & 2)) // H2. Extra flags checks //TODO: what are those flags?
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

			ent->flags |= 2; // H2

			if (GetB(bits, U_ENT_FREED)) // H2
			{
				ent->flags &= ~1;
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
			ent->flags &= ~2; // H2
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
			ent->flags &= ~2; // H2

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

			ent->flags |= 1; // H2

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

		if ((cl_entities[oldnum].flags & 1) && !(cl_entities[oldnum].flags & 2)) // H2. Extra flags checks //TODO: what are those flags?
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
		}

		cl.sound_prepped = true; // Can start mixing ambient sounds.
		CL_CheckPredictionError();
	}
}

void CL_AddEntities(void)
{
	NOT_IMPLEMENTED
}

void CL_GetEntitySoundOrigin(int ent, vec3_t org)
{
	NOT_IMPLEMENTED
}

void CL_Trace(vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end, int brushmask, int flags, trace_t* t)
{
	NOT_IMPLEMENTED
}