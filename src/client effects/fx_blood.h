//
// fx_blood.h
//
// Copyright 2025 mxd
//

#pragma once

#include "Client Entities.h"

extern client_entity_t* DoBloodSplash(vec3_t loc, int amount, qboolean yellow_blood);
extern void DoBloodTrail(client_entity_t* spawner, int amount);
extern void ThrowBlood(const vec3_t torigin, const vec3_t tnormal, qboolean dark, qboolean yellow, qboolean trueplane);