//
// sv_entities.c
//
// Copyright 1998 Raven Software
//

#include "server.h"
#include "client.h"
#include "cmodel.h"
#include "EffectFlags.h"
#include "Vector.h"

static byte fatpvs[MAX_MAP_LEAFS / 8];

// Writes a delta update of an entity_state_t list to the message.
static void SV_EmitPacketEntities(const client_frame_t* from, const client_frame_t* to, sizebuf_t* msg, const uint ent_id) // H2: extra 'ent_id' arg.
{
	entity_state_t* newent;
	entity_state_t* oldent;
	int newnum;
	int oldnum;

	MSG_WriteByte(msg, svc_packetentities);

	const int from_num_entities = (from != NULL ? from->num_entities : 0);
	int newindex = 0;
	int oldindex = 0;

	while (newindex < to->num_entities || oldindex < from_num_entities)
	{
		if (newindex < to->num_entities)
		{
			newent = &svs.client_entities[(to->first_entity + newindex) % svs.num_client_entities];
			newnum = newent->number;
		}
		else
		{
			newent = NULL; //mxd
			newnum = 9999;
		}

		if (oldindex < from_num_entities)
		{
			oldent = &svs.client_entities[(from->first_entity + oldindex) % svs.num_client_entities];
			oldnum = oldent->number;
		}
		else
		{
			oldent = NULL; //mxd
			oldnum = 9999;
		}

		const edict_t* ent = EDICT_NUM(newnum);

		if (newnum == oldnum)
		{
			if (ent->client_sent & ent_id && ent->just_deleted) // H2
			{
				// Remove old entity.
				byte header;
				byte header_bits[NUM_ENTITY_HEADER_BITS] = { 0 };

				SetB(header_bits, U_REMOVE);
				if (oldnum > 255)
					SetB(header_bits, U_NUMBER16);

				SetB(header_bits, U_ENT_FREED);
				MSG_WriteEntityHeaderBits(msg, header_bits, &header);

				if (GetB(header_bits, U_NUMBER16))
					MSG_WriteShort(msg, oldnum);
				else
					MSG_WriteByte(msg, oldnum);
			}
			else
			{
				// Delta update from old position.
				// Because the force param is false, this will not result in any bytes being emitted if the entity has not changed at all.
				// Note that players are always 'newentities', this always updates their oldorigin and prevents warping.
				MSG_WriteDeltaEntity(oldent, newent, msg, false);
			}

			oldindex++;
			newindex++;
		}
		else if (newnum < oldnum)
		{
			if (ent->client_sent & ent_id && ent->just_deleted) // H2
			{
				// Remove new entity.
				byte header;
				byte header_bits[NUM_ENTITY_HEADER_BITS] = { 0 };

				SetB(header_bits, U_REMOVE);
				if (newnum > 255)
					SetB(header_bits, U_NUMBER16);

				SetB(header_bits, U_ENT_FREED);
				MSG_WriteEntityHeaderBits(msg, header_bits, &header);

				if (GetB(header_bits, U_NUMBER16))
					MSG_WriteShort(msg, newnum);
				else
					MSG_WriteByte(msg, newnum);
			}
			else
			{
				// This is a new entity, send it from the baseline.
				MSG_WriteDeltaEntity(&sv.baselines[newnum], newent, msg, true);
			}

			newindex++;
		}
		else // newnum > oldnum
		{
			if (ent->just_deleted) // H2
			{
				// The old entity isn't present in the new message.
				byte header;
				byte header_bits[NUM_ENTITY_HEADER_BITS] = { 0 };

				SetB(header_bits, U_REMOVE);
				if (oldnum > 255)
					SetB(header_bits, U_NUMBER16);

				MSG_WriteEntityHeaderBits(msg, header_bits, &header);

				if (GetB(header_bits, U_NUMBER16))
					MSG_WriteShort(msg, oldnum);
				else
					MSG_WriteByte(msg, oldnum);
			}

			oldindex++;
		}
	}

	MSG_WriteShort(msg, 0);	// End of packetentities.
}

static void SV_WritePlayerstateToClient(const client_frame_t* from, const client_frame_t* to, sizebuf_t* msg)
{
	const player_state_t* ops;
	player_state_t dummy;
	byte buffer[PLAYER_DEL_BYTES];

	const player_state_t* ps = &to->ps;

	if (from == NULL)
	{
		memset(&dummy, 0, sizeof(dummy));
		ops = &dummy;
	}
	else
	{
		ops = &from->ps;
	}

	memset(buffer, 0, sizeof(buffer)); // H2

	// Determine what needs to be sent.
	if (ps->pmove.pm_type != ops->pmove.pm_type)
		SetB(buffer, PS_M_TYPE);

	if (ps->pmove.origin[0] != ops->pmove.origin[0] || ps->pmove.origin[1] != ops->pmove.origin[1])
		SetB(buffer, PS_M_ORIGIN_XY);

	if (ps->pmove.origin[2] != ops->pmove.origin[2])
		SetB(buffer, PS_M_ORIGIN_Z);

	if (ps->pmove.velocity[0] != ops->pmove.velocity[0] || ps->pmove.velocity[1] != ops->pmove.velocity[1])
		SetB(buffer, PS_M_VELOCITY_XY);

	if (ps->pmove.velocity[2] != ops->pmove.velocity[2])
		SetB(buffer, PS_M_VELOCITY_Z);

	if (ps->pmove.pm_time != ops->pmove.pm_time)
		SetB(buffer, PS_M_TIME);

	if (ps->pmove.pm_flags != ops->pmove.pm_flags)
		SetB(buffer, PS_M_FLAGS);

	if (ps->pmove.w_flags != ops->pmove.w_flags)
		SetB(buffer, PS_W_FLAGS);

	if (ps->pmove.gravity != ops->pmove.gravity)
		SetB(buffer, PS_M_GRAVITY);

	if (ps->pmove.delta_angles[0] != ops->pmove.delta_angles[0] ||
		ps->pmove.delta_angles[1] != ops->pmove.delta_angles[1] ||
		ps->pmove.delta_angles[2] != ops->pmove.delta_angles[2])
	{
		SetB(buffer, PS_M_DELTA_ANGLES);
	}

	if (ps->pmove.camera_delta_angles[0] != ops->pmove.camera_delta_angles[0] ||
		ps->pmove.camera_delta_angles[1] != ops->pmove.camera_delta_angles[1] ||
		ps->pmove.camera_delta_angles[2] != ops->pmove.camera_delta_angles[2])
	{
		SetB(buffer, PS_M_CAMERA_DELTA_ANGLES);
	}

	if (!VectorCompare(ps->viewangles, ops->viewangles))
		SetB(buffer, PS_VIEWANGLES);

	if (!VectorCompare(ps->remote_viewangles, ops->remote_viewangles))
		SetB(buffer, PS_REMOTE_VIEWANGLES);

	if (!VectorCompare(ps->remote_vieworigin, ops->remote_vieworigin))
		SetB(buffer, PS_REMOTE_VIEWORIGIN);

	if (ps->remote_id != ops->remote_id)
		SetB(buffer, PS_REMOTE_ID);

	if (ps->viewheight != ops->viewheight)
		SetB(buffer, PS_VIEWHEIGHT);

	if (!VectorCompare(ps->offsetangles, ops->offsetangles))
		SetB(buffer, PS_OFFSETANGLES);

	if (ps->fov != ops->fov)
		SetB(buffer, PS_FOV);

	if (ps->rdflags != ops->rdflags)
		SetB(buffer, PS_RDFLAGS);

	if (ps->AutotargetEntityNum != ops->AutotargetEntityNum)
		SetB(buffer, PS_AUTOTARGETENTITY);

	if (ps->map_percentage != ops->map_percentage)
		SetB(buffer, PS_MAP_PERCENTAGE);

	if (ps->fog_density != ops->fog_density)
		SetB(buffer, PS_FOG_DENSITY);

	if (ps->mission_num1 != ops->mission_num1)
		SetB(buffer, PS_MISSION1);

	if (ps->mission_num2 != ops->mission_num2)
		SetB(buffer, PS_MISSION2);

	for (int i = 0; i < MAX_STATS; i++)
		if (ps->stats[i] != ops->stats[i])
			SetB(buffer, PS_STAT_BIT_0 + i);

	if (!VectorCompare(ps->mins, ops->mins) || !VectorCompare(ps->maxs, ops->maxs))
		SetB(buffer, PS_MINSMAXS);

	if (ps->NoOfItems != 0)
		SetB(buffer, PS_INVENTORY);

	if (ps->NonNullgroundentity != ops->NonNullgroundentity)
		SetB(buffer, PS_GROUNDBITS_NNGE);

	if (ps->GroundPlane.normal[2] < 0.99f)
	{
		SetB(buffer, PS_GROUNDPLANE_INFO1);

		if (VectorLength(ps->GroundPlane.normal) >= 0.1f && !VectorCompare(ps->GroundPlane.normal, ops->GroundPlane.normal))
			SetB(buffer, PS_GROUNDPLANE_INFO2);
	}

	if ((ops->GroundContents ^ ps->GroundContents) & MASK_CURRENT)
		SetB(buffer, PS_GROUNDBITS_GC);

	if (ps->GroundSurface.flags != ops->GroundSurface.flags)
		SetB(buffer, PS_GROUNDBITS_SURFFLAGS);

	if (ps->watertype != ops->watertype)
		SetB(buffer, PS_WATERTYPE);

	if (ps->waterlevel != ops->waterlevel)
		SetB(buffer, PS_WATERLEVEL);

	if (ps->waterheight != ops->waterheight)
		SetB(buffer, PS_WATERHEIGHT);

	if (ps->grabloc[0] != ops->grabloc[0])
		SetB(buffer, PS_GRABLOC0);

	if (ps->grabloc[1] != ops->grabloc[1])
		SetB(buffer, PS_GRABLOC1);

	if (ps->grabloc[2] != ops->grabloc[2])
		SetB(buffer, PS_GRABLOC2);

	if (ps->fwdvel != ops->fwdvel)
		SetB(buffer, PS_FWDVEL);

	if (ps->sidevel != ops->sidevel)
		SetB(buffer, PS_SIDEVEL);

	if (ps->upvel != ops->upvel)
		SetB(buffer, PS_UPVEL);

	if (ps->flags != ops->flags)
		SetB(buffer, PS_FLAGS);

	if (ps->edictflags != ops->edictflags)
		SetB(buffer, PS_EDICTFLAGS);

	if (ps->oldvelocity_z != ops->oldvelocity_z)
		SetB(buffer, PS_OLDVELOCITY_Z);

	if (ps->upperseq != ops->upperseq)
		SetB(buffer, PS_UPPERSEQ);

	if (ps->lowerseq != ops->lowerseq)
		SetB(buffer, PS_LOWERSEQ);

	if (ps->upperframe == ps->lowerframe)
	{
		if (ps->upperframe != ops->upperframe || ps->lowerframe != ops->lowerframe)
			SetB(buffer, PS_FRAMEINFO1);
	}
	else if (ps->upperframe != ops->upperframe || ps->lowerframe != ops->lowerframe)
	{
		SetB(buffer, PS_FRAMEINFO1);
		SetB(buffer, PS_FRAMEINFO2);
	}

	if (ps->upperidle != ops->upperidle)
		SetB(buffer, PS_UPPERIDLE);

	if (ps->loweridle != ops->loweridle)
		SetB(buffer, PS_LOWERIDLE);

	if (ps->uppermove_index != ops->uppermove_index)
		SetB(buffer, PS_UPPERMOVE_INDEX);

	if (ps->lowermove_index != ops->lowermove_index)
		SetB(buffer, PS_LOWERMOVE_INDEX);

	if (ps->weapon != ops->weapon)
		SetB(buffer, PS_WEAPON);

	if (ps->defense != ops->defense)
		SetB(buffer, PS_DEFENSE);

	if (ps->lastweapon != ops->lastweapon)
		SetB(buffer, PS_LASTWEAPON);

	if (ps->lastdefense != ops->lastdefense)
		SetB(buffer, PS_LASTDEFENSE);

	if (ps->weaponready != ops->weaponready)
		SetB(buffer, PS_WEAPONREADY);

	if (ps->switchtoweapon != ops->switchtoweapon)
		SetB(buffer, PS_SWITCHTOWEAPON);

	if (ps->newweapon != ops->newweapon)
		SetB(buffer, PS_NEWWEAPON);

	if (ps->weap_ammo_index != ops->weap_ammo_index)
		SetB(buffer, PS_WEAP_AMMO_INDEX);

	if (ps->def_ammo_index != ops->def_ammo_index)
		SetB(buffer, PS_DEF_AMMO_INDEX);

	if (ps->weaponcharge != ops->weaponcharge)
		SetB(buffer, PS_WEAPONCHARGE);

	if (ps->armortype != ops->armortype)
		SetB(buffer, PS_ARMORTYPE);

	if (ps->bowtype != ops->bowtype)
		SetB(buffer, PS_BOWTYPE);

	if (ps->stafflevel != ops->stafflevel)
		SetB(buffer, PS_STAFFLEVEL);

	if (ps->helltype != ops->helltype)
		SetB(buffer, PS_HELLTYPE);

	if (ps->meteor_count != ops->meteor_count)
		SetB(buffer, PS_METEORCOUNT);

	if (ps->handfxtype != ops->handfxtype)
		SetB(buffer, PS_HANDFXTYPE);

	if (ps->plaguelevel != ops->plaguelevel)
		SetB(buffer, PS_PLAGUELEVEL);

	if (ps->skintype != ops->skintype)
		SetB(buffer, PS_SKINTYPE);

	if (ps->altparts != ops->altparts)
		SetB(buffer, PS_ALTPARTS);

	if (ps->deadflag != ops->deadflag)
		SetB(buffer, PS_DEADFLAG);

	if (ps->ideal_yaw != ops->ideal_yaw)
		SetB(buffer, PS_IDEAL_YAW);

	if (ps->idletime != ops->idletime)
		SetB(buffer, PS_IDLETIME);

	if (ps->powerup_timer != ops->powerup_timer)
		SetB(buffer, PS_POWERUP_TIMER);

	if (ps->quickturn_rate != ops->quickturn_rate)
		SetB(buffer, PS_QUICKTURN_RATE);

	if (ps->dmflags != ops->dmflags)
		SetB(buffer, PS_DMFLAGS);

	if (ps->advancedstaff != ops->advancedstaff)
		SetB(buffer, PS_ADVANCEDSTAFF);

	if (ps->cinematicfreeze != ops->cinematicfreeze)
		SetB(buffer, PS_CINEMATIC);

	if (ps->PIV != ops->PIV)
		SetB(buffer, PS_PIV);

	// Write it.
	MSG_WriteByte(msg, svc_playerinfo);

	byte send_buffer[PLAYER_DELNZ_BYTES];
	memset(send_buffer, 0, sizeof(send_buffer));

	for (int i = 0; i < PLAYER_DEL_BYTES; i++)
		if (buffer[i] != 0)
			SetB(send_buffer, i);

	for (int i = 0; i < PLAYER_DELNZ_BYTES; i++)
		MSG_WriteByte(msg, send_buffer[i]);

	for (int i = 0; i < PLAYER_DEL_BYTES; i++)
		if (GetB(send_buffer, i))
			MSG_WriteByte(msg, buffer[i]);

	// Write the pmove_state_t.
	if (GetB(buffer, PS_M_TYPE))
		MSG_WriteByte(msg, ps->pmove.pm_type);

	if (GetB(buffer, PS_M_ORIGIN_XY))
	{
		MSG_WriteShort(msg, ps->pmove.origin[0]);
		MSG_WriteShort(msg, ps->pmove.origin[1]);
	}

	if (GetB(buffer, PS_M_ORIGIN_Z))
		MSG_WriteShort(msg, ps->pmove.origin[2]);

	if (GetB(buffer, PS_M_VELOCITY_XY))
	{
		MSG_WriteShort(msg, ps->pmove.velocity[0]);
		MSG_WriteShort(msg, ps->pmove.velocity[1]);
	}

	if (GetB(buffer, PS_M_VELOCITY_Z))
		MSG_WriteShort(msg, ps->pmove.velocity[2]);

	if (GetB(buffer, PS_M_TIME))
		MSG_WriteByte(msg, ps->pmove.pm_time);

	if (GetB(buffer, PS_M_FLAGS))
		MSG_WriteByte(msg, ps->pmove.pm_flags);

	if (GetB(buffer, PS_W_FLAGS))
		MSG_WriteByte(msg, ps->pmove.w_flags);

	if (GetB(buffer, PS_M_GRAVITY))
		MSG_WriteShort(msg, ps->pmove.gravity);

	if (GetB(buffer, PS_M_DELTA_ANGLES))
	{
		MSG_WriteShort(msg, ps->pmove.delta_angles[0]);
		MSG_WriteShort(msg, ps->pmove.delta_angles[1]);
		MSG_WriteShort(msg, ps->pmove.delta_angles[2]);
	}

	if (GetB(buffer, PS_M_CAMERA_DELTA_ANGLES))
	{
		MSG_WriteShort(msg, ps->pmove.camera_delta_angles[0]);
		MSG_WriteShort(msg, ps->pmove.camera_delta_angles[1]);
		MSG_WriteShort(msg, ps->pmove.camera_delta_angles[2]);
	}

	if (GetB(buffer, PS_VIEWANGLES))
	{
		MSG_WriteAngle16(msg, ps->viewangles[0]);
		MSG_WriteAngle16(msg, ps->viewangles[1]);
		MSG_WriteAngle16(msg, ps->viewangles[2]);
	}

	if (GetB(buffer, PS_REMOTE_VIEWANGLES))
	{
		MSG_WriteAngle16(msg, ps->remote_viewangles[0]);
		MSG_WriteAngle16(msg, ps->remote_viewangles[1]);
		MSG_WriteAngle16(msg, ps->remote_viewangles[2]);
	}

	if (GetB(buffer, PS_REMOTE_VIEWORIGIN))
	{
		MSG_WriteShort(msg, Q_ftol(ps->remote_vieworigin[0]));
		MSG_WriteShort(msg, Q_ftol(ps->remote_vieworigin[1]));
		MSG_WriteShort(msg, Q_ftol(ps->remote_vieworigin[2]));
	}

	if (GetB(buffer, PS_REMOTE_ID))
		MSG_WriteLong(msg, ps->remote_id);

	if (GetB(buffer, PS_VIEWHEIGHT))
		MSG_WriteShort(msg, ps->viewheight);

	if (GetB(buffer, PS_OFFSETANGLES))
	{
		MSG_WriteAngle16(msg, ps->offsetangles[0]);
		MSG_WriteAngle16(msg, ps->offsetangles[1]);
		MSG_WriteAngle16(msg, ps->offsetangles[2]);
	}

	if (GetB(buffer, PS_FOV))
		MSG_WriteByte(msg, Q_ftol(ps->fov));

	if (GetB(buffer, PS_RDFLAGS))
		MSG_WriteByte(msg, ps->rdflags);

	if (GetB(buffer, PS_AUTOTARGETENTITY))
		MSG_WriteShort(msg, ps->AutotargetEntityNum);

	if (GetB(buffer, PS_MAP_PERCENTAGE))
		MSG_WriteByte(msg, ps->map_percentage);

	if (GetB(buffer, PS_FOG_DENSITY))
		MSG_WriteFloat(msg, ps->fog_density);

	if (GetB(buffer, PS_MISSION1))
		MSG_WriteShort(msg, ps->mission_num1);

	if (GetB(buffer, PS_MISSION2))
		MSG_WriteShort(msg, ps->mission_num2);

	for (int i = 0; i < MAX_STATS; i++)
		if (GetB(buffer, PS_STAT_BIT_0 + i))
			MSG_WriteShort(msg, ps->stats[i]);
 
	if (GetB(buffer, PS_MINSMAXS))
	{
		MSG_WriteShort(msg, Q_ftol(ps->mins[0] * 8.0f));
		MSG_WriteShort(msg, Q_ftol(ps->mins[1] * 8.0f));
		MSG_WriteShort(msg, Q_ftol(ps->mins[2] * 8.0f));

		MSG_WriteShort(msg, Q_ftol(ps->maxs[0] * 8.0f));
		MSG_WriteShort(msg, Q_ftol(ps->maxs[1] * 8.0f));
		MSG_WriteShort(msg, Q_ftol(ps->maxs[2] * 8.0f));
	}

	if (GetB(buffer, PS_INVENTORY))
	{
		MSG_WriteByte(msg, ps->NoOfItems);

		for (int i = 0; i < ps->NoOfItems; i++)
		{
			MSG_WriteByte(msg, ps->inventory_changes[i]);
			MSG_WriteByte(msg, ps->inventory_remaining[i]);
		}
	}

	if (GetB(buffer, PS_GROUNDBITS_NNGE))
		MSG_WriteByte(msg, ps->NonNullgroundentity);

	if (GetB(buffer, PS_GROUNDPLANE_INFO1) && GetB(buffer, PS_GROUNDPLANE_INFO2))
	{
		MSG_WriteFloat(msg, ps->GroundPlane.normal[0]);
		MSG_WriteFloat(msg, ps->GroundPlane.normal[1]);
		MSG_WriteFloat(msg, ps->GroundPlane.normal[2]);
	}

	if (GetB(buffer, PS_GROUNDBITS_GC))
		MSG_WriteByte(msg, ps->GroundContents & MASK_CURRENT);

	if (GetB(buffer, PS_GROUNDBITS_SURFFLAGS))
		MSG_WriteLong(msg, ps->GroundSurface.flags);

	if (GetB(buffer, PS_WATERTYPE))
		MSG_WriteLong(msg, ps->watertype);

	if (GetB(buffer, PS_WATERLEVEL))
		MSG_WriteLong(msg, ps->waterlevel);

	if (GetB(buffer, PS_WATERHEIGHT))
		MSG_WriteFloat(msg, ps->waterheight);

	if (GetB(buffer, PS_GRABLOC0))
		MSG_WriteFloat(msg, ps->grabloc[0]);

	if (GetB(buffer, PS_GRABLOC1))
		MSG_WriteFloat(msg, ps->grabloc[1]);

	if (GetB(buffer, PS_GRABLOC2))
		MSG_WriteFloat(msg, ps->grabloc[2]);

	if (GetB(buffer, PS_GRABANGLE))
		MSG_WriteFloat(msg, ps->grabangle);

	if (GetB(buffer, PS_FWDVEL))
		MSG_WriteFloat(msg, ps->fwdvel);

	if (GetB(buffer, PS_SIDEVEL))
		MSG_WriteFloat(msg, ps->sidevel);

	if (GetB(buffer, PS_UPVEL))
		MSG_WriteFloat(msg, ps->upvel);

	if (GetB(buffer, PS_FLAGS))
		MSG_WriteByte(msg, ps->flags);

	if (GetB(buffer, PS_EDICTFLAGS))
		MSG_WriteLong(msg, ps->edictflags);

	if (GetB(buffer, PS_OLDVELOCITY_Z))
		MSG_WriteFloat(msg, ps->oldvelocity_z);

	if (GetB(buffer, PS_UPPERSEQ))
		MSG_WriteByte(msg, ps->upperseq);

	if (GetB(buffer, PS_LOWERSEQ))
		MSG_WriteByte(msg, ps->lowerseq);

	if (GetB(buffer, PS_FRAMEINFO1))
		MSG_WriteShort(msg, ps->lowerframe);

	if (GetB(buffer, PS_FRAMEINFO2))
		MSG_WriteShort(msg, ps->upperframe);

	if (GetB(buffer, PS_UPPERIDLE))
		MSG_WriteByte(msg, ps->upperidle);

	if (GetB(buffer, PS_LOWERIDLE))
		MSG_WriteByte(msg, ps->loweridle);

	if (GetB(buffer, PS_UPPERMOVE_INDEX))
		MSG_WriteByte(msg, ps->uppermove_index);

	if (GetB(buffer, PS_LOWERMOVE_INDEX))
		MSG_WriteByte(msg, ps->lowermove_index);

	if (GetB(buffer, PS_WEAPON))
		MSG_WriteByte(msg, ps->weapon);

	if (GetB(buffer, PS_DEFENSE))
		MSG_WriteByte(msg, ps->defense);

	if (GetB(buffer, PS_LASTWEAPON))
		MSG_WriteByte(msg, ps->lastweapon);

	if (GetB(buffer, PS_LASTDEFENSE))
		MSG_WriteByte(msg, ps->lastdefense);

	if (GetB(buffer, PS_WEAPONREADY))
		MSG_WriteByte(msg, ps->weaponready);

	if (GetB(buffer, PS_SWITCHTOWEAPON))
		MSG_WriteByte(msg, ps->switchtoweapon);

	if (GetB(buffer, PS_NEWWEAPON))
		MSG_WriteByte(msg, ps->newweapon);

	if (GetB(buffer, PS_WEAP_AMMO_INDEX))
		MSG_WriteByte(msg, ps->weap_ammo_index);

	if (GetB(buffer, PS_DEF_AMMO_INDEX))
		MSG_WriteByte(msg, ps->def_ammo_index);

	if (GetB(buffer, PS_WEAPONCHARGE))
		MSG_WriteByte(msg, ps->weaponcharge);

	if (GetB(buffer, PS_ARMORTYPE))
		MSG_WriteByte(msg, ps->armortype);

	if (GetB(buffer, PS_BOWTYPE))
		MSG_WriteByte(msg, ps->bowtype);

	if (GetB(buffer, PS_STAFFLEVEL))
		MSG_WriteByte(msg, ps->stafflevel);

	if (GetB(buffer, PS_HELLTYPE))
		MSG_WriteByte(msg, ps->helltype);

	if (GetB(buffer, PS_METEORCOUNT))
		MSG_WriteByte(msg, ps->meteor_count);

	if (GetB(buffer, PS_HANDFXTYPE))
		MSG_WriteByte(msg, ps->handfxtype);

	if (GetB(buffer, PS_PLAGUELEVEL))
		MSG_WriteByte(msg, ps->plaguelevel);

	if (GetB(buffer, PS_SKINTYPE))
		MSG_WriteShort(msg, ps->skintype);

	if (GetB(buffer, PS_ALTPARTS))
		MSG_WriteLong(msg, (int)ps->altparts);

	if (GetB(buffer, PS_DEADFLAG))
		MSG_WriteLong(msg, ps->deadflag);

	if (GetB(buffer, PS_IDEAL_YAW))
		MSG_WriteFloat(msg, ps->ideal_yaw);

	MSG_WriteFloat(msg, ps->leveltime);

	if (GetB(buffer, PS_IDLETIME))
		MSG_WriteFloat(msg, ps->idletime);

	if (GetB(buffer, PS_POWERUP_TIMER))
		MSG_WriteFloat(msg, ps->powerup_timer);

	if (GetB(buffer, PS_QUICKTURN_RATE))
		MSG_WriteFloat(msg, ps->quickturn_rate);

	if (GetB(buffer, PS_DMFLAGS))
		MSG_WriteLong(msg, ps->dmflags);

	if (GetB(buffer, PS_ADVANCEDSTAFF))
		MSG_WriteByte(msg, ps->advancedstaff);

	if (GetB(buffer, PS_CINEMATIC))
		MSG_WriteByte(msg, ps->cinematicfreeze);

	if (GetB(buffer, PS_PIV))
		MSG_WriteLong(msg, ps->PIV);
}

void SV_WriteFrameToClient(client_t* client, sizebuf_t* msg)
{
	client_frame_t* oldframe;
	int lastframe;

	// This is the frame we are creating.
	client_frame_t* frame = &client->frames[sv.framenum & UPDATE_MASK];

	// Client is asking for a retransmit, or client hasn't gotten a good message through in a long time.
	if (client->lastframe <= 0 || sv.framenum - client->lastframe >= (UPDATE_BACKUP - 3))
	{
		oldframe = NULL;
		lastframe = -1;
	}
	else
	{
		// We have a valid message to delta from.
		oldframe = &client->frames[client->lastframe & UPDATE_MASK];
		lastframe = client->lastframe;
	}

	MSG_WriteByte(msg, svc_frame);
	MSG_WriteLong(msg, sv.framenum);
	MSG_WriteLong(msg, lastframe); // What we are delta'ing from.

	// Send over the areabits.
	MSG_WriteByte(msg, frame->areabytes);
	SZ_Write(msg, frame->areabits, frame->areabytes);

	// Delta-encode the playerstate.
	SV_WritePlayerstateToClient(oldframe, frame, msg);

	// Delta-encode the entities.
	const int ent_id = 1 << ((client->edict->s.number - 1) & 31); // H2
	SV_EmitPacketEntities(oldframe, frame, msg, ent_id);
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