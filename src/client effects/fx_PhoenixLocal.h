//
// fx_PhoenixLocal.h
//
// Copyright 2025 m-x-d
//

#pragma once

#include "Client Effects.h"

static qboolean FXPhoenixMissilePowerThink(client_entity_t* missile, centity_t* owner);
static void FXPhoenixExplodePower(centity_t* owner, int type, int flags, const vec3_t origin, const vec3_t dir);