#include "g_Typedef.h"

#define X 0
#define Y 1
#define Z 2

#define LUNG_AIRTIME 12
#define GILL_AIRTIME 18

#define KNOCK_BACK_MULTIPLIER 1000.0

float NormalizeAngle(float angle);
float AddNormalizedAngles(float angle1, float angle2);

extern edict_t *FindNearestVisibleActorInFrustum(edict_t *Finder,vec3_t FinderAngles,
													float nearDist,float farDist,
													double hFOV,double vFOV,
													long Flags,
													vec3_t LOSStartPos,
													vec3_t BBMin,vec3_t BBMax);

extern edict_t *FindSpellTargetInRadius(edict_t *searchent, float radius, vec3_t searchpos,
													vec3_t mins, vec3_t maxs);

extern void GetVectorsToActor(edict_t *self, edict_t *actor, vec3_t vec);
extern void QPlaySound(edict_t *self, int sound, int channel);

extern void SetAnim(edict_t *self, int anim);

extern void SpawnBlood(edict_t *target, edict_t *inflictor, vec3_t point, vec3_t normal, 
	int damage);
extern void CalculateKnockBack(vec3_t dir, float knockback, int flags, float mass, vec3_t vel);
extern void PostKnockBack(edict_t *target, vec3_t dir, float knockback, int flags);
extern void DoDamage(edict_t *target, edict_t *inflictor, edict_t *attacker, vec3_t dir,
	vec3_t point, vec3_t normal, int damage, int knockback, int flags, HitLocation_t hitLocation);
extern void CheckForEnviromentDamage(edict_t *self);
extern void DefaultApplyKnockBack(edict_t *self, vec3_t vel);
extern int CategorizeDirection(int stepDOF, float stepDir);

void StartICScript(char *name);
void GetAimVelocity(edict_t *enemy, vec3_t org, vec_t speed, vec3_t AimAngles, vec3_t out);
void remove_non_cinematic_entites(edict_t *owner);

qboolean ok_to_autotarget(edict_t *shooter, edict_t *target);
qboolean ThinkTime(edict_t *self);


