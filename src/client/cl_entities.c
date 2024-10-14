//
// cl_entities.c -- Entity parsing and management.
//
// Copyright 1998 Raven Software
//

#include "client.h"
#include "cl_effects.h"
#include "cl_skeletons.h"
#include "Angles.h"
#include "ResourceManager.h"
#include "sound.h"

ResourceManager_t cl_FXBufMngr;
int camera_timer; // H2
qboolean viewoffset_changed; // H2

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

static void CL_ParseSkeletonDelta(sizebuf_t* sb, entity_state_t* state) // H2
{
	NOT_IMPLEMENTED
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
		CL_ParseSkeletonDelta(&net_message, to);

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

static void CL_ParsePacketEntities(frame_t* oldframe, frame_t* newframe)
{
	NOT_IMPLEMENTED
}

static qboolean CL_ParsePlayerstate(frame_t* oldframe, frame_t* newframe)
{
	NOT_IMPLEMENTED
	return false;
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

	if (cmd != svc_playerinfo)
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