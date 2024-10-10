//
// sv_entities.c
//
// Copyright 1998 Raven Software
//

#include "server.h"
#include "cmodel.h"
#include "EffectFlags.h"
#include "Vector.h"

static byte fatpvs[8192];

void SV_WriteFrameToClient(client_t* client, sizebuf_t* msg)
{
	NOT_IMPLEMENTED
}

// Q2 counterpart
// The client will interpolate the view position, so we can't use a single PVS point.
static void SV_FatPVS(vec3_t org)
{
	int leafs[64];
	vec3_t mins;
	vec3_t maxs;

	for (int i = 0; i < 3; i++)
	{
		mins[i] = org[i] - 8;
		maxs[i] = org[i] + 8;
	}

	const int count = CM_BoxLeafnums(mins, maxs, leafs, 64, NULL);
	if (count < 1)
		Com_Error(ERR_FATAL, "SV_FatPVS: count < 1");

	const int longs = (CM_NumClusters() + 31) >> 5;

	// Convert leafs to clusters.
	for (int i = 0; i < count; i++)
		leafs[i] = CM_LeafCluster(leafs[i]);

	memcpy(fatpvs, CM_ClusterPVS(leafs[0]), longs << 2);

	// Or in all the other leaf bits
	for (int i = 1; i < count; i++)
	{
		int j;

		for (j = 0; j < i; j++)
			if (leafs[i] == leafs[j])
				break;

		if (j != i)
			continue; // Already have the cluster we want

		byte* src = CM_ClusterPVS(leafs[i]);

		for (j = 0; j < longs; j++)
			((long*)fatpvs)[j] |= ((long*)src)[j];
	}
}

// Add entity to the circular client_entities array.
static void AddClientEntity(const client_t* client, client_frame_t* frame, edict_t* ent, const int index) //mxd. Added to avoid gotos in SV_BuildClientFrame
{
	entity_state_t* state = &svs.client_entities[svs.next_client_entities % svs.num_client_entities];

	if (ent->s.number != index)
	{
		Com_DPrintf("FIXING ENT->S.NUMBER!!!\n");
		ent->s.number = (short)index;
	}

	*state = ent->s;

	// Don't mark players missiles as solid.
	if (ent->owner == client->edict)
		state->solid = SOLID_NOT;

	svs.next_client_entities++;
	frame->num_entities++;
}

// Decides which entities are going to be visible to the client, and copies off the playerstate and areabits.
void SV_BuildClientFrame(client_t* client)
{
	vec3_t org;

	const edict_t* cl_ent = client->edict;

	if (cl_ent->client == NULL)
		return; // Not in game yet.

	// This is the frame we are creating.
	client_frame_t* frame = &client->frames[sv.framenum & UPDATE_MASK];
	frame->senttime = svs.realtime;

	// Find the client's PVS.
	if (cl_ent->client->ps.remote_id >= 0) // H2
	{
		for (int i = 0; i < 3; i++)
			org[i] = cl_ent->client->ps.remote_vieworigin[i] * 0.125f;
	}
	else
	{
		for (int i = 0; i < 3; i++)
			org[i] = (float)client->lastcmd.camera_vieworigin[i] * 0.125f;
	}

	const int leafnum = CM_PointLeafnum(org);
	const int clientarea = CM_LeafArea(leafnum);
	const int clientcluster = CM_LeafCluster(leafnum);

	// Calculate the visible areas.
	frame->areabytes = CM_WriteAreaBits(frame->areabits, clientarea);

	// Grab the current player_state_t.
	frame->ps = cl_ent->client->ps;

	SV_FatPVS(org);
	CM_ClusterPHS(clientcluster);

	// Build up the list of visible entities.
	frame->num_entities = 0;
	frame->first_entity = svs.next_client_entities;

	const int ent_id = 1 << ((cl_ent->s.number - 1) & 31); // H2

	for (int e = 1; e < ge->num_edicts; e++)
	{
		edict_t* ent = EDICT_NUM(e);

		if (ent->just_deleted && (ent->client_sent & ent_id) != 0)
		{
			AddClientEntity(client, frame, ent, e); //mxd
			continue;
		}

		// Ignore ents without visible models.
		if (ent->svflags & SVF_NOCLIENT || !ent->inuse) // H2: new !ent->inuse check
			continue;

		if (ent->svflags & SVF_ALWAYS_SEND) // H2
		{
			ent->client_sent |= ent_id;
			AddClientEntity(client, frame, ent, e); //mxd

			continue;
		}

		// Ignore ents without visible models unless they have an effect.
		if (!ent->s.modelindex && !ent->s.effects && !ent->s.sound)
			continue;

		vec3_t delta;
		VectorSubtract(org, ent->s.origin, delta);
		const float dist = VectorLength(delta);

		// Ignore if not touching a PV leaf.
		if (ent == cl_ent || dist <= 100.0f) // H2: extra 'delta' check.
		{
			ent->client_sent |= ent_id;
			AddClientEntity(client, frame, ent, e); //mxd

			continue;
		}

		// Check area. Doors can legally straddle two areas, so we may need to check another one.
		if (!CM_AreasConnected(clientarea, ent->areanum) && (ent->areanum2 == 0 || !CM_AreasConnected(clientarea, ent->areanum2)))
			continue; // Blocked by a door.

		if (ent->num_clusters == -1)
		{
			// Too many leafs for individual check, go by headnode.
			if (!CM_HeadnodeVisible(ent->headnode, fatpvs))
				continue;
		}
		else
		{
			// Check individual leafs.
			int cluster = 0;
			for (; cluster < ent->num_clusters; cluster++)
			{
				const int leaf = ent->clusternums[cluster];
				if (fatpvs[leaf >> 3] & (1 << (leaf & 7)))
					break;
			}

			if (cluster == ent->num_clusters)
				continue; // Not visible.
		}

		// Add ent to the circular client_entities array?
		if (ent->s.modelindex != 0 || (ent->s.effects & EF_NODRAW_ALWAYS_SEND) || dist <= 2000.0f)
		{
			ent->client_sent |= ent_id;
			AddClientEntity(client, frame, ent, e); //mxd
		}
	}
}