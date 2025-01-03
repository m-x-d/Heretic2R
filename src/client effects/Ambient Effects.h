//
// Ambient Effects.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "Client Entities.h"

// Protos for ambientfx.
extern void DoWaterSplash(client_entity_t* effect, paletteRGBA_t color, int count);
extern void WaterSplash(centity_t* owner, int type, int flags, const vec3_t origin);