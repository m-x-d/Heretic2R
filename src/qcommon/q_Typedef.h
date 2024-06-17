//
// q_Typedef.h
//
// Copyright 1998 Raven Software
//

#pragma once

typedef float vec_t;		//TODO: remove
typedef vec_t vec2_t[2];	//TODO: unused?
typedef vec_t vec3_t[3];
typedef double vec3d_t[3];	//TODO: unused?
typedef vec_t vec5_t[5];	//TODO: unused?

typedef float matrix3_t[3][3];
typedef float matrix3d_t[3][3];	//TODO: unused?

typedef unsigned short ushort;	//mxd. Way shorter than "unsigned short"...
typedef unsigned uint;			//mxd. Shorter than "size_t", way shorter than "unsigned int"...
typedef unsigned long ulong;	//mxd. Way shorter than "unsigned long"...
typedef	int	fixed4_t;		//TODO: unused?
typedef	int	fixed8_t;		//TODO: unused?
typedef	int	fixed16_t;		//TODO: unused?

typedef unsigned char 		byte;
typedef enum {false, true}	qboolean;

typedef struct edict_s edict_t;

typedef struct paletteRGBA_s
{
	union
	{
		struct
		{
			byte r;
			byte g;
			byte b;
			byte a;
		};
		uint c;
		byte c_array[4];
	};
} paletteRGBA_t;