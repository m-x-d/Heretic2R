//
// fx_remotecamera.c
//
// Heretic II
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Client Entities.h"
#include "ce_DefaultMessageHandler.h"
#include "Particle.h"
#include "ResourceManager.h"
#include "FX.h"
#include "Vector.h"
#include "random.h"
#include "motion.h"
#include "Utilities.h"

static qboolean FXRemoteCameraThink(struct client_entity_s *Self,centity_t *Owner)
{
	

	return(false);
}

void FXRemoteCamera(centity_t *Owner,int Type,int Flags,vec3_t Origin)
{
	short			TargetEntNum;
	centity_t		*TargetEnt;
	vec3_t			Forward,
					Angles;
	client_entity_t	*RemoteCamera;

	fxi.GetEffect(Owner,Flags,clientEffectSpawners[FX_REMOTE_CAMERA].formatString,&TargetEntNum);

	TargetEnt=(&fxi.server_entities[TargetEntNum]);

	VectorSubtract(Owner->origin,TargetEnt->origin,Forward);

	VectorNormalize(Forward);

	vectoangles(Forward,Angles);

	RemoteCamera=ClientEntity_new(Type,Flags|CEF_NOMOVE,Origin,NULL,5000);

	RemoteCamera->radius=10.0;

	VectorCopy(Owner->origin,fxi.cl->refdef.vieworg);

	RemoteCamera->Update=FXRemoteCameraThink;
	
	AddEffect(NULL,RemoteCamera);
}
