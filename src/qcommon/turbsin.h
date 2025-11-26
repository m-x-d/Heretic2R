//
// turbsin.h
//
// Copyright 1998 Raven Software
//

#pragma once

#define TURBSCALE	(256.0f / ANGLE_360) //mxd. Replaced (2 * M_PI) with ANGLE_360

//mxd. Helper defines...
#define TURBSIN_V0(v0, v1, time)	((int)((((v0) * 2.3f + (v1)) * 0.015f + (time) * 3.0f) * TURBSCALE) & 255)
#define TURBSIN_V1(v0, v1, time)	((int)((((v1) * 2.3f + (v0)) * 0.015f + (time) * 6.0f) * TURBSCALE) & 255)

extern float turbsin[];
