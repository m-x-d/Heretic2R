//
// fx_debris_local.h
//
// Copyright 2025 mxd
//

#pragma once

#include "q_Typedef.h"

static qboolean Debris_Update(client_entity_t* self, centity_t* owner);
static qboolean FleshDebris_Update(client_entity_t* self, centity_t* owner);
static qboolean BodyPart_Update(client_entity_t* self, centity_t* owner);

static void Debris_Collision(client_entity_t* self, CE_Message_t* msg);
static void BodyPart_Spawn(const centity_t* owner, int body_part, vec3_t origin, float ke, int frame, int type, byte modelindex, int flags, centity_t* harpy);
static void BodyPart_Throw(const centity_t* owner, int body_part, vec3_t origin, float ke, int frame, int type, byte modelindex, int flags, centity_t* harpy);
static qboolean BodyPartAttachedUpdate(client_entity_t* self, centity_t* owner);