#ifndef PLACEMENT_H
#define PLACEMENT_H

typedef float vec3_t[3];

typedef struct Placement_s
{
	vec3_t origin;
	vec3_t direction;
	vec3_t up;
} Placement_t;

#endif