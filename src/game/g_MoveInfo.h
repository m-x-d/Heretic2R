#ifndef G_MOVEINFO_H
#define G_MOVEINFO_H

typedef struct MoveInfo_s
{
	int			type;

	int			stepDOF;
	int			state;
	int			readiness;

	float		yawOffset;		// offset from facing used to determine actual angle moving toward (radians)
	float		pivotDirection; // 1 is left, -1 is right
	float		destYaw;		// facing the ent is turning towards (radians)

	int			blockCount;
	float		yawAtLastCollision;

} MoveInfo_t;

extern MoveInfo_t *MoveInfo_new();
extern void MoveInfo_delete(MoveInfo_t *toDelete);

#endif // G_MOVEINFO_H
