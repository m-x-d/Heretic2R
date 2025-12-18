//
// fx_WaterSplash.h -- Named Ambient Effects.h in original logic.
//
// Copyright 1998 Raven Software
//

#pragma once

#include "Client Entities.h"

extern void DoWaterSplash(client_entity_t* effect, paletteRGBA_t color, int count);
extern void FXWaterSplash(centity_t* owner, int type, int flags, vec3_t origin);