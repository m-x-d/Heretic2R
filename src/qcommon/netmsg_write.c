//
// netmsg_write.c
//
// Copyright 1998 Raven Software
//

#include "client.h"
#include "server.h"
#include "anorms.h"
#include "ArrayedList.h"
#include "Vector.h"
#include "g_Skeleton.h"

void SetB(byte* buf, const int bit)
{
	buf[bit / 8] |= 1 << (bit % 8);
}

byte GetB(const byte* buf, const int bit)
{
	if (buf != NULL)
		return buf[bit / 8] & (1 << (bit % 8));

	return 0;
}

static void UnsetB(byte* buf, const int bit)
{
	buf[bit / 8] &= ~(1 << (bit % 8));
}

// Q2 counterpart
void MSG_WriteChar(sizebuf_t* sb, const int c)
{
	byte* buf = SZ_GetSpace(sb, 1);
	*buf = (char)c;
}

// Q2 counterpart
void MSG_WriteByte(sizebuf_t* sb, const int c)
{
	byte* buf = SZ_GetSpace(sb, 1);
	buf[0] = (byte)c;
}

// Q2 counterpart
void MSG_WriteShort(sizebuf_t* sb, const int c)
{
	byte* buf = SZ_GetSpace(sb, 2);
	buf[0] = (byte)(c);
	buf[1] = (byte)(c >> 8);
}

// Q2 counterpart
void MSG_WriteLong(sizebuf_t* sb, const int c)
{
	byte* buf = SZ_GetSpace(sb, 4);
	buf[0] = (byte)(c);
	buf[1] = (byte)(c >> 8);
	buf[2] = (byte)(c >> 16);
	buf[3] = (byte)(c >> 24);
}

void MSG_WriteFloat(sizebuf_t* sb, const float f)
{
	SZ_Write(sb, &f, 4); // H2: no endian conversion.
}

// Q2 counterpart
void MSG_WriteString(sizebuf_t* sb, const char* s)
{
	if (s != NULL)
		SZ_Write(sb, s, (int)strlen(s) + 1);
	else
		SZ_Write(sb, "", 1);
}

// Q2 counterpart
void MSG_WriteCoord(sizebuf_t* sb, const float f)
{
	MSG_WriteShort(sb, (int)(f * 8.0f));
}

// Q2 counterpart
void MSG_WritePos(sizebuf_t* sb, const vec3_t pos)
{
	MSG_WriteShort(sb, (int)(pos[0] * 8.0f));
	MSG_WriteShort(sb, (int)(pos[1] * 8.0f));
	MSG_WriteShort(sb, (int)(pos[2] * 8.0f));
}

// Q2 counterpart
void MSG_WriteAngle(sizebuf_t* sb, const float f)
{
	MSG_WriteByte(sb, (int)(f * 256.0f / 360.0f) & 255);
}

// Q2 counterpart
void MSG_WriteAngle16(sizebuf_t* sb, const float f)
{
	MSG_WriteShort(sb, ANGLE2SHORT(f));
}

void MSG_WriteDeltaUsercmd(sizebuf_t* sb, const usercmd_t* from, const usercmd_t* cmd)
{
	// Send the movement message.
	int bits = 0;

	if (cmd->angles[0] != from->angles[0])
		bits |= CM_ANGLE1;

	if (cmd->angles[1] != from->angles[1])
		bits |= CM_ANGLE2;

	if (cmd->angles[2] != from->angles[2])
		bits |= CM_ANGLE3;

	if (cmd->aimangles[0] != from->aimangles[0])
		bits |= CM_AIMANGLE1;

	if (cmd->aimangles[1] != from->aimangles[1])
		bits |= CM_AIMANGLE2;

	if (cmd->aimangles[2] != from->aimangles[2])
		bits |= CM_AIMANGLE3;

	if (cmd->camera_vieworigin[0] != from->camera_vieworigin[0])
		bits |= CM_CAMERAVIEWORIGIN1;

	if (cmd->camera_vieworigin[1] != from->camera_vieworigin[1])
		bits |= CM_CAMERAVIEWORIGIN2;

	if (cmd->camera_vieworigin[2] != from->camera_vieworigin[2])
		bits |= CM_CAMERAVIEWORIGIN3;

	if (cmd->camera_viewangles[0] != from->camera_viewangles[0])
		bits |= CM_CAMERAVIEWANGLES1;

	if (cmd->camera_viewangles[1] != from->camera_viewangles[1])
		bits |= CM_CAMERAVIEWANGLES2;

	if (cmd->camera_viewangles[2] != from->camera_viewangles[2])
		bits |= CM_CAMERAVIEWANGLES3;

	if (cmd->forwardmove != from->forwardmove)
		bits |= CM_FORWARD;

	if (cmd->sidemove != from->sidemove)
		bits |= CM_SIDE;

	if (cmd->upmove != from->upmove)
		bits |= CM_UP;

	if (cmd->buttons != from->buttons)
		bits |= CM_BUTTONS;

	MSG_WriteShort(sb, bits); // Q2: MSG_WriteByte

	if (bits & CM_ANGLE1)
		MSG_WriteShort(sb, cmd->angles[0]);

	if (bits & CM_ANGLE2)
		MSG_WriteShort(sb, cmd->angles[1]);

	if (bits & CM_ANGLE3)
		MSG_WriteShort(sb, cmd->angles[2]);

	if (bits & CM_AIMANGLE1)
		MSG_WriteShort(sb, cmd->aimangles[0]);

	if (bits & CM_AIMANGLE2)
		MSG_WriteShort(sb, cmd->aimangles[1]);

	if (bits & CM_AIMANGLE3)
		MSG_WriteShort(sb, cmd->aimangles[2]);

	if (bits & CM_CAMERAVIEWORIGIN1)
		MSG_WriteShort(sb, cmd->camera_vieworigin[0]);

	if (bits & CM_CAMERAVIEWORIGIN2)
		MSG_WriteShort(sb, cmd->camera_vieworigin[1]);

	if (bits & CM_CAMERAVIEWORIGIN3)
		MSG_WriteShort(sb, cmd->camera_vieworigin[2]);

	if (bits & CM_CAMERAVIEWANGLES1)
		MSG_WriteShort(sb, cmd->camera_viewangles[0]);

	if (bits & CM_CAMERAVIEWANGLES2)
		MSG_WriteShort(sb, cmd->camera_viewangles[1]);

	if (bits & CM_CAMERAVIEWANGLES3)
		MSG_WriteShort(sb, cmd->camera_viewangles[2]);

	if (bits & CM_FORWARD)
		MSG_WriteShort(sb, cmd->forwardmove);

	if (bits & CM_SIDE)
		MSG_WriteShort(sb, cmd->sidemove);

	if (bits & CM_UP)
		MSG_WriteShort(sb, cmd->upmove);

	if (bits & CM_BUTTONS)
		MSG_WriteShort(sb, cmd->buttons);

	MSG_WriteByte(sb, cmd->msec);
	MSG_WriteByte(sb, cmd->lightlevel);
}

void ParseEffectToSizeBuf(sizebuf_t* sb, const char* format, va_list marker) // H2
{
	while (format != NULL && *format != 0)
	{
		switch (*format)
		{
			case 'b':
				MSG_WriteByte(sb, va_arg(marker, byte));
				break;

			case 'd':
				MSG_WriteDir(sb, *va_arg(marker, vec3_t*));
				break;

			case 'f':
				MSG_WriteFloat(sb, (float)va_arg(marker, double)); // float variadic arg is promoted to double --mxd.
				break;

			case 'i':
				MSG_WriteLong(sb, va_arg(marker, int));
				break;

			case 'p':
			case 'v':
				MSG_WritePos(sb, *va_arg(marker, vec3_t*));
				break;

			case 's':
				MSG_WriteShort(sb, va_arg(marker, short));
				break;

			case 't':
				MSG_WriteShortYawPitch(sb, *va_arg(marker, vec3_t*));
				break;

			case 'u':
				MSG_WriteDirMag(sb, *va_arg(marker, vec3_t*));
				break;

			case 'x':
				MSG_WriteYawPitch(sb, *va_arg(marker, vec3_t*));
				break;

			default:
				break;
		}

		format++;
	}
}

void MSG_WriteEntityHeaderBits(sizebuf_t* msg, const byte* bf, byte* bfNonZero)
{
	*bfNonZero = 0;

	for (int i = 0; i < NUM_ENTITY_HEADER_BITS; i++)
		if (bf[i] != 0)
			SetB(bfNonZero, i);

	MSG_WriteByte(msg, *bfNonZero);

	for (int i = 0; i < NUM_ENTITY_HEADER_BITS; i++)
		if (GetB(bfNonZero, i))
			MSG_WriteByte(msg, bf[i]);
}

static void MSG_WriteEffects(sizebuf_t* sb, EffectsBuffer_t* fxBuf) // H2
{
	if (fxBuf->freeBlock < 256)
	{
		MSG_WriteByte(sb, fxBuf->numEffects);
		MSG_WriteByte(sb, fxBuf->freeBlock);
	}
	else
	{
		fxBuf->numEffects |= FX_BUF_SIZE16;
		MSG_WriteByte(sb, fxBuf->numEffects);
		MSG_WriteShort(sb, fxBuf->freeBlock);
	}

	SZ_Write(sb, fxBuf->buf, fxBuf->freeBlock);
}

static float ClampAngleRad(const float angle) // H2
{
	float result = fmodf(angle, ANGLE_360);

	if (result >= ANGLE_180)
		result -= ANGLE_360;

	if (result <= -ANGLE_180)
		result += ANGLE_360;

	return result;
}

//mxd. Parsed by MSG_ReadJoints().
static void MSG_WriteJoints(sizebuf_t* sb, const int joint_index) // H2
{
	const G_SkeletalJoint_t* joint = &ge->skeletalJoints[joint_index];

	// Write child joints first.
	int child = joint->children;
	while (child != -1)
	{
		MSG_WriteJoints(sb, ge->jointNodes[child].data);
		child = ge->jointNodes[child].next;
	}

	// Check for changes.
	int flags = 0;
	int axis[3] = { 0, 0, 0 };

	for (int i = 0; i < 3; i++)
	{
		if (joint->changed[i])
		{
			flags |= 1 << i;

			if (joint->angVels[i] > 0.0f)
				axis[i] = 1;
			else if (joint->angVels[i] < 0.0f)
				axis[i] = 2;
		}
	}

	// No changes...
	if (flags == 0)
		return;

	// Write changes.
	MSG_WriteByte(sb, flags | (axis[PITCH] + (axis[YAW] + axis[ROLL] * 3) * 3) * 8);
	MSG_WriteByte(sb, joint_index);

	for (int i = 0; i < 3; i++)
	{
		if (joint->changed[i])
		{
			const float angle = ClampAngleRad(joint->destAngles[i]);
			const int byteangle = ClampI((int)(angle * RAD_TO_BYTEANGLE + 128.0f), 0, 255);
			MSG_WriteByte(sb, byteangle);
		}
	}
}

// Writes part of a packetentities message.
// Can delta from either a baseline or a previous packet_entity.
//mxd. Parsed by CL_ParseDelta().
void MSG_WriteDeltaEntity(const entity_state_t* from, entity_state_t* to, sizebuf_t* msg, const qboolean force)
{
	byte bits[NUM_ENTITY_HEADER_BITS];

	if (to->number < 1 || to->number >= MAX_NETWORKABLE_EDICTS)
		Com_Error(ERR_FATAL, "MSG_WriteDeltaEntity: invalid entity number (%i)", to->number);

	// Send an update.
	memset(bits, 0, sizeof(bits));

	if (to->number >= 256)
		SetB(bits, U_NUMBER16);

	// Origin
	if (to->origin[0] != from->origin[0] || to->origin[1] != from->origin[1])
		SetB(bits, U_ORIGIN12);

	if (to->origin[2] != from->origin[2])
		SetB(bits, U_ORIGIN3);

	// Angles
	if (to->angles[0] != from->angles[0])
		SetB(bits, U_ANGLE1);

	if (to->angles[1] != from->angles[1])
		SetB(bits, U_ANGLE2);

	if (to->angles[2] != from->angles[2])
		SetB(bits, U_ANGLE3);

	// Skinnum
	if (to->skinnum != from->skinnum)
	{
		if (to->skinnum < 256)
		{
			SetB(bits, U_SKIN8);
		}
		else if (to->skinnum < 0x10000)
		{
			SetB(bits, U_SKIN16);
		}
		else
		{
			SetB(bits, U_SKIN8);
			SetB(bits, U_SKIN16);
		}
	}

	// Clientnum
	if (to->clientnum != from->clientnum)
		SetB(bits, U_CLIENTNUM);

	// Scale
	if (to->scale != from->scale)
		SetB(bits, U_SCALE);

	// Frame
	if (to->frame != from->frame)
		SetB(bits, (to->frame < 256 ? U_FRAME8 : U_FRAME16));

	// Effects
	if (to->effects != from->effects)
	{
		if (to->effects < 256)
		{
			SetB(bits, U_EFFECTS8);
		}
		else if (to->effects < 0x8000)
		{
			SetB(bits, U_EFFECTS16);
		}
		else
		{
			SetB(bits, U_EFFECTS8);
			SetB(bits, U_EFFECTS16);
		}
	}

	// Renderfx
	if (to->renderfx != from->renderfx)
	{
		if (to->renderfx < 256)
		{
			SetB(bits, U_RENDERFX8);
		}
		else if (to->renderfx < 0x8000)
		{
			SetB(bits, U_RENDERFX16);
		}
		else
		{
			SetB(bits, U_RENDERFX8);
			SetB(bits, U_RENDERFX16);
		}
	}

	// Solid
	if (to->solid != from->solid)
		SetB(bits, U_SOLID);

	//mxd. Bounding box.
	if (SV_PROTOCOL == H2R_PROTOCOL_VERSION && (!VectorCompare(to->mins, from->mins) || !VectorCompare(to->maxs, from->maxs)))
		SetB(bits, U_BBOX);

	// Modleindex
	if (to->modelindex != from->modelindex)
		SetB(bits, U_MODEL);

	// Sound
	if (to->sound != from->sound)
		SetB(bits, U_SOUND);

	// Bmodel
	if (force)
	{
		SetB(bits, U_OLDORIGIN);

		if (Vec3NotZero(to->bmodel_origin))
			SetB(bits, U_BMODEL);
	}

	// Flexmodel node infos
	int fmnodeinfo_usage_flags = 0;

	for (int i = 1; i < MAX_FM_MESH_NODES; i++)
	{
		if (to->fmnodeinfo[i].frame != from->fmnodeinfo[i].frame ||
			to->fmnodeinfo[i].color.c != from->fmnodeinfo[i].color.c ||
			to->fmnodeinfo[i].flags != from->fmnodeinfo[i].flags ||
			to->fmnodeinfo[i].skin != from->fmnodeinfo[i].skin)
		{
			SetB(bits, U_FM_INFO);

			const int flag = i - 1;
			if (flag < 7)
				fmnodeinfo_usage_flags |= 1 << flag;
			else
				fmnodeinfo_usage_flags |= (1 << (flag + 1)) + U_FM_HIGH;
		}
	}

	// ClientEffects
	if (to->clientEffects.numEffects > 0)
		SetB(bits, U_CLIENT_EFFECTS);

	if ((to->effects & 4) != 0)
		SetB(bits, U_JOINTED);

	if ((to->effects & 8) != 0)
		SetB(bits, U_SWAPFRAME);

	// Color
	if (to->color.r != from->color.r)
		SetB(bits, U_COLOR_R);

	if (to->color.g != from->color.g)
		SetB(bits, U_COLOR_G);

	if (to->color.b != from->color.b)
		SetB(bits, U_COLOR_B);

	if (to->color.a != from->color.a)
		SetB(bits, U_COLOR_A);

	// Abslight
	if (to->absLight.r != from->absLight.r || to->absLight.g != from->absLight.g || to->absLight.b != from->absLight.b)
		SetB(bits, U_ABSLIGHT);

	// UsageCount
	if (to->usageCount != from->usageCount)
		SetB(bits, U_USAGE_COUNT);

	// Write the message
	if (!force)
	{
		UnsetB(bits, U_NUMBER16);

		int index;
		for (index = 0; index < NUM_ENTITY_HEADER_BITS; index++)
			if (bits[index] != 0)
				break;

		if (index == NUM_ENTITY_HEADER_BITS)
			return; // Nothing to send!

		if (to->number >= 256)
			SetB(bits, U_NUMBER16);
	}

	byte header;
	MSG_WriteEntityHeaderBits(msg, bits, &header);

	if (GetB(bits, U_NUMBER16))
		MSG_WriteShort(msg, to->number);
	else
		MSG_WriteByte(msg, to->number);

	if (GetB(bits, U_MODEL))
		MSG_WriteByte(msg, to->modelindex);

	if (GetB(bits, U_BMODEL))
		MSG_WritePos(msg, to->bmodel_origin);

	if (GetB(bits, U_FRAME8))
		MSG_WriteByte(msg, to->frame);
	else if (GetB(bits, U_FRAME16))
		MSG_WriteShort(msg, to->frame);

	if (GetB(bits, U_SKIN8) && GetB(bits, U_SKIN16))
		MSG_WriteLong(msg, to->skinnum);
	else if (GetB(bits, U_SKIN16))
		MSG_WriteShort(msg, to->skinnum);
	else if (GetB(bits, U_SKIN8))
		MSG_WriteByte(msg, to->skinnum);

	if (GetB(bits, U_CLIENTNUM))
		MSG_WriteByte(msg, to->clientnum);

	if (GetB(bits, U_SCALE))
		MSG_WriteByte(msg, (int)(to->scale * 100.0f));

	if (GetB(bits, U_EFFECTS8) && GetB(bits, U_EFFECTS16))
		MSG_WriteLong(msg, to->effects);
	else if (GetB(bits, U_EFFECTS16))
		MSG_WriteShort(msg, to->effects);
	else if (GetB(bits, U_EFFECTS8))
		MSG_WriteByte(msg, to->effects);

	if (GetB(bits, U_RENDERFX8) && GetB(bits, U_RENDERFX16))
		MSG_WriteLong(msg, to->renderfx);
	else if (GetB(bits, U_RENDERFX16))
		MSG_WriteShort(msg, to->renderfx);
	else if (GetB(bits, U_RENDERFX8))
		MSG_WriteByte(msg, to->renderfx);

	if (GetB(bits, U_ORIGIN12))
	{
		MSG_WriteCoord(msg, to->origin[0]);
		MSG_WriteCoord(msg, to->origin[1]);
	}

	if (GetB(bits, U_ORIGIN3))
		MSG_WriteCoord(msg, to->origin[2]);

	if (GetB(bits, U_ANGLE1))
		MSG_WriteAngle(msg, to->angles[0]);

	if (GetB(bits, U_ANGLE2))
		MSG_WriteAngle(msg, to->angles[1]);

	if (GetB(bits, U_ANGLE3))
		MSG_WriteAngle(msg, to->angles[2]);

	if (GetB(bits, U_OLDORIGIN))
		MSG_WritePos(msg, to->old_origin);

	if (GetB(bits, U_SOUND))
	{
		MSG_WriteByte(msg, to->sound);
		MSG_WriteByte(msg, to->sound_data);
	}

	if (GetB(bits, U_SOLID))
		MSG_WriteShort(msg, to->solid);

	if (GetB(bits, U_BBOX)) //mxd
	{
		MSG_WritePos(msg, to->mins);
		MSG_WritePos(msg, to->maxs);
	}

	if (GetB(bits, U_FM_INFO))
	{
		MSG_WriteByte(msg, fmnodeinfo_usage_flags & 255);
		if (fmnodeinfo_usage_flags & U_FM_HIGH)
			MSG_WriteByte(msg, fmnodeinfo_usage_flags >> 8);

		for (int i = 1; i < MAX_FM_MESH_NODES; i++)
		{
			const int flag = (i < 8 ? i - 1 : i); // Skip U_FM_HIGH flag
			if (((1 << flag) & fmnodeinfo_usage_flags) == 0)
				continue;

			int fm_flags = 0;

			if (to->fmnodeinfo[i].frame != from->fmnodeinfo[i].frame)
			{
				fm_flags |= U_FM_FRAME;
				if (to->fmnodeinfo[i].frame > 255)
					fm_flags |= U_FM_FRAME16;
			}

			if (to->fmnodeinfo[i].color.r != from->fmnodeinfo[i].color.r)
				fm_flags |= U_FM_COLOR_R;

			if (to->fmnodeinfo[i].color.g != from->fmnodeinfo[i].color.g)
				fm_flags |= U_FM_COLOR_G;

			if (to->fmnodeinfo[i].color.b != from->fmnodeinfo[i].color.b)
				fm_flags |= U_FM_COLOR_B;

			if (to->fmnodeinfo[i].color.a != from->fmnodeinfo[i].color.a)
				fm_flags |= U_FM_COLOR_A;

			if (to->fmnodeinfo[i].flags != from->fmnodeinfo[i].flags)
				fm_flags |= U_FM_FLAGS;

			if (to->fmnodeinfo[i].skin != from->fmnodeinfo[i].skin)
				fm_flags |= U_FM_SKIN;

			MSG_WriteByte(msg, fm_flags);

			if (fm_flags & U_FM_FRAME)
				MSG_WriteByte(msg, to->fmnodeinfo[i].frame & 255);

			if (fm_flags & U_FM_FRAME16)
				MSG_WriteByte(msg, to->fmnodeinfo[i].frame >> 8);

			if (fm_flags & U_FM_COLOR_R)
				MSG_WriteByte(msg, to->fmnodeinfo[i].color.r);

			if (fm_flags & U_FM_COLOR_G)
				MSG_WriteByte(msg, to->fmnodeinfo[i].color.g);

			if (fm_flags & U_FM_COLOR_B)
				MSG_WriteByte(msg, to->fmnodeinfo[i].color.b);

			if (fm_flags & U_FM_COLOR_A)
				MSG_WriteByte(msg, to->fmnodeinfo[i].color.a);

			if (fm_flags & U_FM_FLAGS)
				MSG_WriteByte(msg, to->fmnodeinfo[i].flags);

			if (fm_flags & U_FM_SKIN)
				MSG_WriteByte(msg, to->fmnodeinfo[i].skin);
		}
	}

	if (GetB(bits, U_CLIENT_EFFECTS))
		MSG_WriteEffects(msg, &to->clientEffects);

	if (GetB(bits, U_JOINTED))
	{
		MSG_WriteJoints(msg, to->rootJoint);
		MSG_WriteByte(msg, 0);
	}

	if (GetB(bits, U_SWAPFRAME))
		MSG_WriteShort(msg, to->swapFrame);

	if (GetB(bits, U_COLOR_R))
		MSG_WriteByte(msg, to->color.r);

	if (GetB(bits, U_COLOR_G))
		MSG_WriteByte(msg, to->color.g);

	if (GetB(bits, U_COLOR_B))
		MSG_WriteByte(msg, to->color.b);

	if (GetB(bits, U_COLOR_A))
		MSG_WriteByte(msg, to->color.a);

	if (GetB(bits, U_ABSLIGHT))
	{
		MSG_WriteByte(msg, to->absLight.r);
		MSG_WriteByte(msg, to->absLight.g);
		MSG_WriteByte(msg, to->absLight.b);
	}

	if (GetB(bits, U_USAGE_COUNT))
		MSG_WriteByte(msg, to->usageCount);
}

void MSG_WriteDir(sizebuf_t* sb, const vec3_t dir)
{
	if (dir == NULL)
	{
		Com_DPrintf("ERROR : NULL direction passed into MSG_WriteDir\n"); // H2
		MSG_WriteByte(sb, 0);

		return;
	}

	float bestd = 0.0f;
	int best = 0;

	for (int i = 0; i < NUMVERTEXNORMALS; i++)
	{
		const float d = DotProduct(dir, bytedirs[i]);
		if (d > bestd)
		{
			bestd = d;
			best = i;
		}
	}

	MSG_WriteByte(sb, best);
}

void MSG_WriteDirMag(sizebuf_t* sb, const vec3_t dir) // H2
{
	if (dir == NULL)
	{
		Com_DPrintf("ERROR : NULL direction passed into MSG_WriteDirMag\n"); // H2
		MSG_WriteByte(sb, 0);
		MSG_WriteByte(sb, 0);

		return;
	}

	float bestd = 0.0f;
	int best = 0;

	for (int i = 0; i < NUMVERTEXNORMALS; i++)
	{
		const float d = DotProduct(dir, bytedirs[i]);
		if (d > bestd)
		{
			bestd = d;
			best = i;
		}
	}

	MSG_WriteByte(sb, best);

	const float mag = Clamp(VectorLength(dir) * 0.1f, 1.0f, 255.0f);
	MSG_WriteByte(sb, (int)mag);
}

void MSG_WriteYawPitch(sizebuf_t* sb, const vec3_t vector) // H2
{
	if (vector != NULL)
	{
		vec3_t angles;
		vectoangles(vector, angles);

		MSG_WriteByte(sb, (int)((angles[YAW]  + 180.0f) * (255.0f / 360.0f)));
		MSG_WriteByte(sb, (int)((angles[PITCH] + 90.0f) * (255.0f / 180.0f)));
	}
	else
	{
		Com_DPrintf("ERROR, null direction passed into MSG_WriteYawPitch\n");
		MSG_WriteByte(sb, 0);
		MSG_WriteByte(sb, 0);
	}
}

void MSG_WriteShortYawPitch(sizebuf_t* sb, const vec3_t vector) // H2
{
	if (vector != NULL)
	{
		vec3_t angles;
		vectoangles(vector, angles);

		MSG_WriteShort(sb, (int)(angles[0] * 8.0f));
		MSG_WriteShort(sb, (int)(angles[1] * 8.0f));
	}
	else
	{
		Com_DPrintf("ERROR, null direction passed into MSG_WriteShortYawPitch\n");
		MSG_WriteShort(sb, 0);
		MSG_WriteShort(sb, 0);
	}
}