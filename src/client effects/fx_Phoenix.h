//
// fx_Phoenix.h
//
// Copyright 2025 m-x-d
//

#pragma once

#include "Client Effects.h"

extern qboolean FXPhoenixExplosionBallThink(client_entity_t* explosion, centity_t* owner);
extern client_entity_t* CreatePhoenixSmallExplosion(vec3_t ball_origin);