#ifndef FX_DEBRIS_H
#define FX_DEBRIS_H

#include "q_Typedef.h"

void FXDebris_SpawnChunks(int type, int flags, vec3_t origin, int num, int material, 
	vec3_t dir, float ke,vec3_t mins,float scale, qboolean altskin);
void FXBodyPart_Spawn(centity_t *owner, int BodyPart, vec3_t origin, float ke, int frame, int type, byte modelindex, int flags, centity_t *harpy);
qboolean FXDebris_Update(struct client_entity_s *self, centity_t *owner);
qboolean FXFleshDebris_Update(struct client_entity_s *self, centity_t *owner);
qboolean FXBodyPart_Update(struct client_entity_s *self,centity_t *owner);

#endif // FX_DEBRIS_H