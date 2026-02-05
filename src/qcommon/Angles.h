//
// Angles.h
//
// Copyright 1998 Raven Software
//

#pragma once

// Angle indexes. Originally defined in q_shared.h.
#define PITCH			0 // up / down
#define YAW				1 // left / right
#define ROLL			2 // fall over

// Angles in radians [https://www.rapidtables.com/convert/number/degrees-to-radians.html].
#define ANGLE_0			0.0f
#define ANGLE_1			0.017453292f
#define ANGLE_5			0.087266462f
#define ANGLE_10		0.174532925f
#define ANGLE_15		0.261799387f
#define ANGLE_20		0.392699081f
#define ANGLE_30		0.523598775f
#define ANGLE_35		0.610865238f //mxd
#define ANGLE_40		0.698131701f //mxd
#define ANGLE_45		0.785398163f
#define ANGLE_60		1.047197551f
#define ANGLE_85		1.483529864f //mxd
#define ANGLE_90		1.570796327f
#define ANGLE_120		2.094395102f
#define ANGLE_135		2.35619449f
#define ANGLE_180		3.141592653f
#define ANGLE_225		3.926990817f
#define ANGLE_270		4.71238898f
#define ANGLE_315		5.497787144f
#define ANGLE_360		6.283185307f

// Conversion routines.
#define ANGLE_TO_RAD	ANGLE_1
#define RAD_TO_ANGLE	(180.0f / ANGLE_180)

#define BYTEANGLE_TO_RAD	(ANGLE_360 / 255.0f) //mxd
#define DEG_TO_BYTEANGLE	(255.0f / 360.0f) //mxd

//mxd. Used by skeletal joints logic.
#define JOINT_TO_BYTEANGLE	(254.0f / ANGLE_90)
#define BYTEANGLE_TO_JOINT	(ANGLE_90 / 254.0f)