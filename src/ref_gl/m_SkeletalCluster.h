typedef int qboolean;

typedef struct M_SkeletalCluster_s
{
	int children;		// must be the first field
	int numVerticies;
	int *verticies;
	qboolean inUse;
} M_SkeletalCluster_t;

