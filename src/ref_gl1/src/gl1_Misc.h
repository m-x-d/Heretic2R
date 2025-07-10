//
// gl1_Misc.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "ref.h"

extern void R_ScreenShot_f(void);
extern void R_Strings_f(void);

extern void R_SetDefaultState(void);
extern void R_DrawNullModel(void);

extern void R_TransformVector(const vec3_t v, vec3_t out);
extern void R_RotateForEntity(const entity_t* e);

extern void R_HandleTransparency(const entity_t* e);
extern void R_CleanupTransparency(const entity_t* e);