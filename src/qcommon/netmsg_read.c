#include "qcommon.h"
#include "assert.h"
#include "vector.h"

//
// reading functions
//

void MSG_BeginReading (sizebuf_t *msg)
{
	msg->readcount = 0;
}

// returns -1 if no more characters are available
int MSG_ReadChar (sizebuf_t *msg_read)
{
	int	c;
	
	if (msg_read->readcount+1 > msg_read->cursize)
		c = -1;
	else
		c = (signed char)msg_read->data[msg_read->readcount];
	msg_read->readcount++;
	
	return c;
}

int MSG_ReadByte (sizebuf_t *msg_read)
{
	int	c;
	
	if (msg_read->readcount+1 > msg_read->cursize)
		c = -1;
	else
		c = (unsigned char)msg_read->data[msg_read->readcount];
	msg_read->readcount++;
	
	return c;
}

int MSG_ReadShort (sizebuf_t *msg_read)
{
	int	c;
	
	if (msg_read->readcount+2 > msg_read->cursize)
		c = -1;
	else		
		c = (short)(msg_read->data[msg_read->readcount]
		+ (msg_read->data[msg_read->readcount+1]<<8));
	
	msg_read->readcount += 2;
	
	return c;
}

int MSG_ReadLong (sizebuf_t *msg_read)
{
	int	c;
	
	if (msg_read->readcount+4 > msg_read->cursize)
		c = -1;
	else
		c = msg_read->data[msg_read->readcount]
		+ (msg_read->data[msg_read->readcount+1]<<8)
		+ (msg_read->data[msg_read->readcount+2]<<16)
		+ (msg_read->data[msg_read->readcount+3]<<24);
	
	msg_read->readcount += 4;
	
	return c;
}

float MSG_ReadFloat (sizebuf_t *msg_read)
{
	union
	{
		byte	b[4];
		float	f;
		int	l;
	} dat;
	
	if (msg_read->readcount+4 > msg_read->cursize)
	{
		assert(0);
		dat.f = -1;
	}
	else
	{
		dat.b[0] =	msg_read->data[msg_read->readcount];
		dat.b[1] =	msg_read->data[msg_read->readcount+1];
		dat.b[2] =	msg_read->data[msg_read->readcount+2];
		dat.b[3] =	msg_read->data[msg_read->readcount+3];
	}
	msg_read->readcount += 4;
	
	dat.l = LittleLong (dat.l);

	return dat.f;	
}

char *MSG_ReadString (sizebuf_t *msg_read)
{
	static char	string[2048];
	int		l,c;
	
	l = 0;
	do
	{
		c = MSG_ReadChar (msg_read);
		if (c == -1 || c == 0)
			break;
		string[l] = c;
		l++;
	} while (l < sizeof(string)-1);
	
	string[l] = 0;
	
	return string;
}

char *MSG_ReadStringLine (sizebuf_t *msg_read)
{
	static char	string[2048];
	int		l,c;
	
	l = 0;
	do
	{
		c = MSG_ReadChar (msg_read);
		if (c == -1 || c == 0 || c == '\n')
			break;
		string[l] = c;
		l++;
	} while (l < sizeof(string)-1);
	
	string[l] = 0;
	
	return string;
}

float MSG_ReadCoord (sizebuf_t *msg_read)
{
	return MSG_ReadShort(msg_read) * (1.0/8);
}

void MSG_ReadPos (sizebuf_t *msg_read, vec3_t pos)
{
	if(msg_read->readcount+6 > msg_read->cursize)
	{
		assert(0);
	}

	pos[0] = MSG_ReadShort(msg_read) * (1.0/8);
	pos[1] = MSG_ReadShort(msg_read) * (1.0/8);
	pos[2] = MSG_ReadShort(msg_read) * (1.0/8);
}

float MSG_ReadAngle (sizebuf_t *msg_read)
{
	return MSG_ReadChar(msg_read) * (360.0/256);
}

float MSG_ReadAngle16 (sizebuf_t *msg_read)
{
	return SHORT2ANGLE(MSG_ReadShort(msg_read));
}

void MSG_ReadDeltaUsercmd (sizebuf_t *msg_read, usercmd_t *from, usercmd_t *move)
{
	int bits;

	memcpy (move, from, sizeof(*move));

	// Read delta bits.

	bits=MSG_ReadShort(msg_read);
		
	// Read angles.
	
	if (bits & CM_ANGLE1)
		move->angles[0] = MSG_ReadShort (msg_read);
	if (bits & CM_ANGLE2)
		move->angles[1] = MSG_ReadShort (msg_read);
	if (bits & CM_ANGLE3)
		move->angles[2] = MSG_ReadShort (msg_read);

	// Read aimangles.
	
	if (bits & CM_AIMANGLE1)
		move->aimangles[0] = MSG_ReadShort (msg_read);
	if (bits & CM_AIMANGLE2)
		move->aimangles[1] = MSG_ReadShort (msg_read);
	if (bits & CM_AIMANGLE3)
		move->aimangles[2] = MSG_ReadShort (msg_read);

	// Read camera vieworigin.

	if (bits & CM_CAMERAVIEWORIGIN1)
		move->camera_vieworigin[0] = MSG_ReadShort (msg_read);
	if (bits & CM_CAMERAVIEWORIGIN2)
		move->camera_vieworigin[1] = MSG_ReadShort (msg_read);
	if (bits & CM_CAMERAVIEWORIGIN3)
		move->camera_vieworigin[2] = MSG_ReadShort (msg_read);

	// Read camera viewangles.

	if (bits & CM_CAMERAVIEWANGLES1)
		move->camera_viewangles[0] = MSG_ReadShort (msg_read);
	if (bits & CM_CAMERAVIEWANGLES2)
		move->camera_viewangles[1] = MSG_ReadShort (msg_read);
	if (bits & CM_CAMERAVIEWANGLES3)
		move->camera_viewangles[2] = MSG_ReadShort (msg_read);
	
	// Read movement.
	
	if (bits & CM_FORWARD)
		move->forwardmove = MSG_ReadShort (msg_read);
	if (bits & CM_SIDE)
		move->sidemove = MSG_ReadShort (msg_read);
	if (bits & CM_UP)
		move->upmove = MSG_ReadShort (msg_read);
	
	// Read buttons.

	if (bits & CM_BUTTONS)
		move->buttons = MSG_ReadShort (msg_read);

	// Read time to run command.

	move->msec = MSG_ReadByte (msg_read);

	// Read lightlevel.

	move->lightlevel = MSG_ReadByte (msg_read);
}


void MSG_ReadData (sizebuf_t *msg_read, void *data, int len)
{
	int		i;

	for (i=0 ; i<len ; i++)
		((byte *)data)[i] = MSG_ReadByte (msg_read);
}

#define NUMVERTEXNORMALS	162
vec3_t	bytedirs[NUMVERTEXNORMALS] =
{
#include "anorms.h"
};

void MSG_ReadDir (sizebuf_t *sb, vec3_t dir)
{
	int		b;

	// Read in index into vector table
	b = MSG_ReadByte (sb);
	if (b >= NUMVERTEXNORMALS)
	{
		assert(0);
		Com_Error (ERR_DROP, "MSF_ReadDir: out of range");
	}
	VectorCopy (bytedirs[b], dir);
}

void MSG_ReadDirMag (sizebuf_t *sb, vec3_t dir)
{
	int		b;

	// Read in index into vector table
	b = MSG_ReadByte (sb);
	if (b >= NUMVERTEXNORMALS)
	{
		assert(0);
		Com_Error (ERR_DROP, "MSF_ReadDirMag: out of range");
	}
	VectorCopy (bytedirs[b], dir);

	// Scale by magnitude
	b = MSG_ReadByte (sb);
	Vec3ScaleAssign(10.0 * b, dir);
}

void MSG_ReadShortYawPitch(sizebuf_t *sb, vec3_t dir)
{
	vec3_t	angles;

	if(sb->readcount+4 > sb->cursize)
	{
		assert(0);
	}

	angles[0] = MSG_ReadShort(sb) * (1.0/8);
	angles[1] = MSG_ReadShort(sb) * (1.0/8);
	angles[2] = 0;

	angles[YAW] = angles[YAW] * ANGLE_TO_RAD;
	angles[PITCH] = angles[PITCH] * ANGLE_TO_RAD;
	DirFromAngles(angles, dir);
}

void MSG_ReadYawPitch(sizebuf_t *sb, vec3_t dir)
{
	int		yb, pb;
	float	yaw, pitch;
	vec3_t	angles;

	yb = MSG_ReadByte(sb);
	pb = MSG_ReadByte(sb);

	// Convert to signed degrees
	yaw = (yb * (360.0 / 255.0)) - 180.0;
	pitch = (pb * (180.0 / 255.0)) - 90.0;

	// Convert to radians
	angles[YAW] = yaw * ANGLE_TO_RAD;
	angles[PITCH] = pitch * ANGLE_TO_RAD;
	DirFromAngles(angles, dir);
}

void MSG_ReadEffects(sizebuf_t *msg_read, EffectsBuffer_t *fxBuf)
{
	size_t	bufTypeSize;
	int		len;

	fxBuf->numEffects += MSG_ReadByte(msg_read);

	assert(fxBuf->numEffects >= 0);

	if(fxBuf->numEffects < 0)
	{
		Com_Error (ERR_DROP, "MSG_ReadEffects: number of effects < 0");
		return;
	}

	if(fxBuf->numEffects == 0)
	{
		return;
	}

	if(fxBuf->numEffects & 0x80)
	{
		fxBuf->numEffects &= ~0x80;
		bufTypeSize = sizeof(short);
		len=MSG_ReadShort(msg_read);
	}
	else
	{
		bufTypeSize = sizeof(byte);
		len=MSG_ReadByte(msg_read);
	}

	assert(len > 0);

	if(fxBuf->numEffects <= 0)
	{
		Com_Error (ERR_DROP, "MSG_ReadEffects: bufSize not > 0");
		return;
	}

	MSG_ReadData(msg_read, fxBuf->buf+fxBuf->bufSize, len);

	fxBuf->bufSize+=len;
}