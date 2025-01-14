//
// fx_debris_local.h
//
// Copyright 2025 m-x-d
//

#pragma once

#include "q_Typedef.h"

static qboolean FXDebris_Update(struct client_entity_s* self, centity_t* owner);
static qboolean FXFleshDebris_Update(struct client_entity_s* self, centity_t* owner);
static qboolean FXBodyPart_Update(struct client_entity_s* self, centity_t* owner);

static void FXDebris_Collision(client_entity_t* self, CE_Message_t* msg);
static void FXBodyPart_Spawn(const centity_t* owner, int body_part, vec3_t origin, float ke, int frame, int type, byte modelindex, int flags, centity_t* harpy);
static void FXBodyPart_Throw(const centity_t* owner, int body_part, vec3_t origin, float ke, int frame, int type, byte modelindex, int flags, centity_t* harpy);
static qboolean FXBodyPartAttachedUpdate(struct client_entity_s* self, const centity_t* owner);