//
// fx_PhoenixLocal.h
//
// Copyright 2025 mxd
//

#pragma once

#include "Client Effects.h"

static qboolean PhoenixMissilePowerThink(client_entity_t* missile, centity_t* owner);
static void PhoenixExplodePower(int type, int flags, const vec3_t origin, const vec3_t dir);