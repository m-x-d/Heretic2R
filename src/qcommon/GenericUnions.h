#ifndef GENERICUNIONS_H
#define GENERICUNIONS_H

#include "q_shared.h"

// don't add anything to this union which is greater than 4 bytes in size
typedef union GenericUnion4_u
{
	byte t_byte;
	short t_short;
	int t_int;
	unsigned int t_uint;
	float t_float;
	float *t_float_p;
	struct edict_s *t_edict_p;
	void *t_void_p;
	paletteRGBA_t t_RGBA;
} GenericUnion4_t;

// don't add anything to this union which is greater than 8 bytes in size
typedef union GenericUnion8_u
{
	GenericUnion4_t	u4;
	long t_long;
	unsigned long t_ulong;
	double t_double;
} GenericUnion8_t;

// don't add anything to this union which is greater than 8 bytes in size
typedef union GenericUnion12_u
{
	GenericUnion8_t u8;
	vec3_t t_vec3;
} GenericUnion12_t;

#endif