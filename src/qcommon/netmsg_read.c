//
// netmsg_read.c
//
// Copyright 1998 Raven Software
//

#include "anorms.h" //mxd
#include "qcommon.h"
#include "cl_skeletons.h"
#include "Vector.h"

void MSG_BeginReading(sizebuf_t *sb)
{
	sb->readcount = 0;
}

// Returns -1 if no more characters are available.
int MSG_ReadChar(sizebuf_t* sb)
{
	int	c;

	if (sb->readcount + 1 > sb->cursize)
		c = -1;
	else
		c = (char)sb->data[sb->readcount];

	sb->readcount++;

	return c;
}

int MSG_ReadByte(sizebuf_t* sb)
{
	int	c;

	if (sb->readcount + 1 > sb->cursize)
		c = -1;
	else
		c = sb->data[sb->readcount];

	sb->readcount++;

	return c;
}

int MSG_ReadShort(sizebuf_t* sb)
{
	int	c;

	if (sb->readcount + 2 > sb->cursize)
		c = -1;
	else
		c = (short)(sb->data[sb->readcount]	+ (sb->data[sb->readcount + 1] << 8));

	sb->readcount += 2;

	return c;
}

int MSG_ReadLong(sizebuf_t* sb)
{
	int	c;

	if (sb->readcount + 4 > sb->cursize)
		c = -1;
	else
		c = sb->data[sb->readcount]	+ (sb->data[sb->readcount + 1] << 8) + (sb->data[sb->readcount + 2] << 16) + (sb->data[sb->readcount + 3] << 24);

	sb->readcount += 4;

	return c;
}

float MSG_ReadFloat(sizebuf_t* sb)
{
	union
	{
		byte b[4];
		float f;
		int l;
	} dat;

	if (sb->readcount + 4 > sb->cursize)
	{
		assert(0);
		dat.f = -1;
	}
	else
	{
		dat.b[0] = sb->data[sb->readcount];
		dat.b[1] = sb->data[sb->readcount + 1];
		dat.b[2] = sb->data[sb->readcount + 2];
		dat.b[3] = sb->data[sb->readcount + 3];
	}

	sb->readcount += 4;
	dat.l = LittleLong(dat.l);

	return dat.f;
}

char* MSG_ReadString(sizebuf_t* sb)
{
	static char	string[2048];

	int l = 0;
	do
	{
		const char c = (char)MSG_ReadChar(sb);
		if (c == -1 || c == 0)
			break;

		string[l] = c;
		l++;
	} while (l < (int)sizeof(string) - 1);

	string[l] = 0;

	return string;
}

char* MSG_ReadStringLine(sizebuf_t* sb)
{
	static char	string[2048];

	int l = 0;
	do
	{
		const char c = (char)MSG_ReadChar(sb);
		if (c == -1 || c == 0 || c == '\n')
			break;

		string[l] = c;
		l++;
	} while (l < (int)sizeof(string) - 1);

	string[l] = 0;

	return string;
}

float MSG_ReadCoord(sizebuf_t* sb)
{
	return (float)MSG_ReadShort(sb) * (1.0f / 8.0f);
}

void MSG_ReadPos(sizebuf_t* sb, vec3_t pos)
{
	if (sb->readcount + 6 > sb->cursize)
		assert(0);

	pos[0] = (float)MSG_ReadShort(sb) * (1.0f / 8.0f);
	pos[1] = (float)MSG_ReadShort(sb) * (1.0f / 8.0f);
	pos[2] = (float)MSG_ReadShort(sb) * (1.0f / 8.0f);
}

float MSG_ReadAngle(sizebuf_t* sb)
{
	return (float)MSG_ReadChar(sb) * (360.0f / 256);
}

float MSG_ReadAngle16(sizebuf_t* sb)
{
	return SHORT2ANGLE(MSG_ReadShort(sb));
}

void MSG_ReadDeltaUsercmd(sizebuf_t* sb, const usercmd_t* from, usercmd_t* move)
{
	memcpy(move, from, sizeof(*move));

	// Read delta bits.
	const int bits = MSG_ReadShort(sb);

	// Read angles.
	if (bits & CM_ANGLE1)
		move->angles[0] = (short)MSG_ReadShort(sb);
	if (bits & CM_ANGLE2)
		move->angles[1] = (short)MSG_ReadShort(sb);
	if (bits & CM_ANGLE3)
		move->angles[2] = (short)MSG_ReadShort(sb);

	// Read aimangles.
	if (bits & CM_AIMANGLE1)
		move->aimangles[0] = (short)MSG_ReadShort(sb);
	if (bits & CM_AIMANGLE2)
		move->aimangles[1] = (short)MSG_ReadShort(sb);
	if (bits & CM_AIMANGLE3)
		move->aimangles[2] = (short)MSG_ReadShort(sb);

	// Read camera vieworigin.
	if (bits & CM_CAMERAVIEWORIGIN1)
		move->camera_vieworigin[0] = (short)MSG_ReadShort(sb);
	if (bits & CM_CAMERAVIEWORIGIN2)
		move->camera_vieworigin[1] = (short)MSG_ReadShort(sb);
	if (bits & CM_CAMERAVIEWORIGIN3)
		move->camera_vieworigin[2] = (short)MSG_ReadShort(sb);

	// Read camera viewangles.
	if (bits & CM_CAMERAVIEWANGLES1)
		move->camera_viewangles[0] = (short)MSG_ReadShort(sb);
	if (bits & CM_CAMERAVIEWANGLES2)
		move->camera_viewangles[1] = (short)MSG_ReadShort(sb);
	if (bits & CM_CAMERAVIEWANGLES3)
		move->camera_viewangles[2] = (short)MSG_ReadShort(sb);

	// Read movement.
	if (bits & CM_FORWARD)
		move->forwardmove = (short)MSG_ReadShort(sb);
	if (bits & CM_SIDE)
		move->sidemove = (short)MSG_ReadShort(sb);
	if (bits & CM_UP)
		move->upmove = (short)MSG_ReadShort(sb);

	// Read buttons.
	if (bits & CM_BUTTONS)
		move->buttons = (short)MSG_ReadShort(sb);

	// Read time to run command.
	move->msec = (byte)MSG_ReadByte(sb);

	// Read lightlevel.
	move->lightlevel = (byte)MSG_ReadByte(sb);
}


void MSG_ReadData(sizebuf_t* sb, void* data, const int size)
{
	for (int i = 0; i < size; i++)
		((byte*)data)[i] = (byte)MSG_ReadByte(sb);
}

void MSG_ReadDir(sizebuf_t* sb, vec3_t dir)
{
	// Read in index into vector table.
	const int b = MSG_ReadByte(sb);

	if (b < 0 || b >= NUMVERTEXNORMALS) //mxd. Added lower bound check. MSG_ReadByte() returns -1 on error.
		Com_Error(ERR_DROP, "MSF_ReadDir: out of range");

	VectorCopy(bytedirs[b], dir);
}

void MSG_ReadDirMag(sizebuf_t* sb, vec3_t dir)
{
	// Read in index into vector table
	int b = MSG_ReadByte(sb);

	if (b < 0 || b >= NUMVERTEXNORMALS) //mxd. Added lower bound check. MSG_ReadByte() returns -1 on error.
		Com_Error(ERR_DROP, "MSF_ReadDirMag: out of range");

	VectorCopy(bytedirs[b], dir);

	// Scale by magnitude
	b = MSG_ReadByte(sb);
	Vec3ScaleAssign(10.0f * (float)b, dir);
}

void MSG_ReadShortYawPitch(sizebuf_t* sb, vec3_t dir)
{
	vec3_t angles;

	if (sb->readcount + 4 > sb->cursize)
		assert(0);

	angles[0] = (float)MSG_ReadShort(sb) / 8.0f;
	angles[1] = (float)MSG_ReadShort(sb) / 8.0f;
	angles[2] = 0;

	angles[YAW] *= ANGLE_TO_RAD;
	angles[PITCH] *= ANGLE_TO_RAD;
	DirFromAngles(angles, dir);
}

void MSG_ReadYawPitch(sizebuf_t* sb, vec3_t dir)
{
	vec3_t angles;

	const int yb = MSG_ReadByte(sb);
	const int pb = MSG_ReadByte(sb);

	// Convert to signed degrees
	const float yaw = ((float)yb * (360.0f / 255.0f)) - 180.0f;
	const float pitch = ((float)pb * (180.0f / 255.0f)) - 90.0f;

	// Convert to radians
	angles[YAW] = yaw * ANGLE_TO_RAD;
	angles[PITCH] = pitch * ANGLE_TO_RAD;
	DirFromAngles(angles, dir);
}

void MSG_ReadEffects(sizebuf_t* sb, EffectsBuffer_t* fxBuf)
{
	int len;
	int num_fx = MSG_ReadByte(sb); //mxd. Original logic adds this directly to fxBuf->numEffects, which can potentially create ENTITY_FX_SIZE16 flag out of thin air...

	if (num_fx & FX_BUF_SIZE16)
	{
		num_fx &= ~FX_BUF_SIZE16;
		len = MSG_ReadShort(sb);
	}
	else
	{
		len = MSG_ReadByte(sb);
	}

	fxBuf->numEffects += num_fx;

	if (fxBuf->numEffects < 0 || fxBuf->numEffects >= FX_BUF_MAX_EFFECTS) //mxd. Add upper bound check.
		Com_Error(ERR_DROP, "MSG_ReadEffects: invalid number of effects (%i)", fxBuf->numEffects);

	if (fxBuf->numEffects == 0)
		return;

	if (len > 0)
	{
		MSG_ReadData(sb, &fxBuf->buf[fxBuf->bufSize], len);
		fxBuf->bufSize += len;
	}
	else
	{
		Com_Error(ERR_DROP, "MSG_ReadEffects: invalid buffer size (%i)", len);
	}
}

#ifdef QUAKE2_DLL //mxd. Avoid referencing unneeded stuff in Client Effects...

// Written by MSG_WriteJoints().
void MSG_ReadJoints(sizebuf_t* sb, entity_state_t* ent)
{
	if (ent->rootJoint < 0)
	{
		const int root = ent->number * 3 - 3;
		SK_CreateSkeleton(SKEL_CORVUS, root);
		ent->rootJoint = (short)root;
	}

	while (true)
	{
		const int flags = MSG_ReadByte(sb);
		if (flags == 0)
			break;

		const int index = MSG_ReadByte(sb);
		if (index < 0) //mxd. Added lower bound check. MSG_ReadByte() returns -1 on error.
			Com_Error(ERR_DROP, "MSG_ReadJoints: joint index out of range");

		if ((flags & JN_YAW_CHANGED) != 0)
		{
			const int val = MSG_ReadByte(sb);
			skeletal_joints[index].destAngles[0] = (float)(val - 128) / RAD_TO_BYTEANGLE;

			const int dir = (flags >> 3) % 3;
			if (dir == 1)
				skeletal_joints[index].angVels[0] = ANGLE_45;
			else if (dir == 2)
				skeletal_joints[index].angVels[0] = -ANGLE_45;
			else
				skeletal_joints[index].angVels[0] = 0.0f;
		}

		if ((flags & JN_PITCH_CHANGED) != 0)
		{
			const int val = MSG_ReadByte(sb);
			skeletal_joints[index].destAngles[1] = (float)(val - 128) / RAD_TO_BYTEANGLE;

			const int dir = (flags >> 3) / 3 % 3;
			if (dir == 1)
				skeletal_joints[index].angVels[1] = ANGLE_45;
			else if (dir == 2)
				skeletal_joints[index].angVels[1] = -ANGLE_45;
			else
				skeletal_joints[index].angVels[1] = 0.0f;
		}

		if ((flags & JN_ROLL_CHANGED) != 0)
		{
			const int val = MSG_ReadByte(sb);
			skeletal_joints[index].destAngles[2] = (float)(val - 128) / RAD_TO_BYTEANGLE;

			const int dir = (flags >> 3) / 9;
			if (dir == 1)
				skeletal_joints[index].angVels[2] = ANGLE_45;
			else if (dir == 2)
				skeletal_joints[index].angVels[2] = -ANGLE_45;
			else
				skeletal_joints[index].angVels[2] = 0.0f;
		}
	}
}

#endif