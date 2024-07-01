#include "q_Typedef.h"

typedef struct G_SkeletalJoint_s
{
	int children;		// must be the first field
	float destAngles[3];
	float angVels[3];
	float angles[3];
	int changed[3];
	qboolean inUse;
} G_SkeletalJoint_t;