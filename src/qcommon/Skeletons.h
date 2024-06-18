#include <stdlib.h> // for size_t

#define JN_YAW_CHANGED		0x00000001
#define JN_PITCH_CHANGED	0x00000002
#define JN_ROLL_CHANGED		0x00000004

// Skeleton types
enum {
	SKEL_NULL = -1,
	SKEL_RAVEN = 0,
	SKEL_BOX,
	SKEL_BEETLE,
	SKEL_ELFLORD,
	SKEL_PLAGUE_ELF,
	SKEL_CORVUS,
	NUM_SKELETONS
};

// Raven Skeletal joints
enum {
	RAVEN_LOWERBACK = 0,
	RAVEN_UPPERBACK,
	RAVEN_HEAD,
	NUM_JOINTS_RAVEN
};

// Box Skeletal joints
enum {
	BOX_CENTER = 0,
	NUM_JOINTS_BOX
};

// Beetle Skeletal joints
enum {
	BEETLE_NECK = 0,
	BEETLE_HEAD,
	NUM_JOINTS_BEETLE
};

// Elflord Skeletal joints
enum {
	ELFLORD_,
	ELFLORD__,
	NUM_JOINTS_ELFLORD
};

// Plague Elf Skeletal joints
enum {
	PLAGUE_ELF_LOWERBACK,
	PLAGUE_ELF_UPPERBACK,
	PLAGUE_ELF_HEAD,
	NUM_JOINTS_PLAGUE_ELF
};

// Corvus Skeletal joints
enum {
	CORVUS_LOWERBACK,
	CORVUS_UPPERBACK,
	CORVUS_HEAD,
	NUM_JOINTS_CORVUS
};

#define NO_SWAP_FRAME -1
#define NULL_ROOT_JOINT -1

#define MAX_ARRAYED_SKELETAL_JOINTS 255	// has max of 65,535 (if this remains at 255, net code can be changed to reflect)
#define MAX_ARRAYED_JOINT_NODES	(MAX_ARRAYED_SKELETAL_JOINTS - 1)

#define MAX_JOINTS_PER_SKELETON	8	// arbitrary small number
#define MAX_JOINT_NODES_PER_SKELETON (MAX_JOINTS_PER_SKELETON - 1)

extern char *skeletonRootNames[];
extern int skeletonRNameOffsets[];
extern char *skeletonJointNames[];
extern int skeletonNameOffsets[];
extern int numJointsInSkeleton[];
extern char *skeletonEffectorNames[];
extern int skeletonENameOffsets[];
extern int numNodesInSkeleton[];

typedef void (*CreateSkeleton_t)(void *skeletalJoints, size_t jointSize, struct ArrayedListNode_s *jointNodes, int rootIndex);

extern CreateSkeleton_t SkeletonCreators[NUM_SKELETONS];