typedef float vec3_t[3];

#define DECALFLAG_ACTIVE	1	// actually, any flag at all counts as active
#define DECALFLAG_ORIENTED	2	// otherwise screen is used for up, right, normal
#define DECALFLAG_ANIMATE	4	// required for animation to occur, removed at end
#define DECALFLAG_TIMED		8	// remove after removetime set

extern struct cvar_s		*cl_testdecals;

extern void CL_AddDecals(void);
extern void V_TestDecals (void);
extern void CL_UpdateDecals(void);



