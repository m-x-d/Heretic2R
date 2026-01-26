//
// fx_remotecamera.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Utilities.h"
#include "Vector.h"

void FXRemoteCamera(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	short target_ent_num;
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_REMOTE_CAMERA].formatString, &target_ent_num); //TODO: target_ent_num is unused.

	client_entity_t* remote_camera = ClientEntity_new(type, flags | CEF_NOMOVE, origin, NULL, 5000);

	remote_camera->radius = 10.0f;
	VectorCopy(owner->origin, fxi.cl->refdef.vieworg);

	AddEffect(NULL, remote_camera);
}