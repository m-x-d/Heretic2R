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
#define ANGLE_40		0.6981317f //mxd
#define ANGLE_45		0.785398163f
#define ANGLE_50		0.872664626f //mxd
#define ANGLE_60		1.047197551f
#define ANGLE_72		1.256637061f //mxd. Unused.
#define ANGLE_75		1.308996939f //mxd.
#define ANGLE_90		1.570796327f
#define ANGLE_120		2.094395102f
#define ANGLE_135		2.35619449f
#define ANGLE_144		2.513274123f //mxd. Unused.
#define ANGLE_180		3.141592653f
#define ANGLE_225		3.926990817f
#define ANGLE_270		4.71238898f
#define ANGLE_315		5.497787144f
#define ANGLE_360		6.283185307f

// Angles in degrees
#define DEGREE_0		0.0f
#define DEGREE_180		180.0f
#define DEGREE_45		(DEGREE_180 / 4.0f)
#define DEGREE_90		(DEGREE_180 / 2.0f)
#define DEGREE_135		(DEGREE_90 + DEGREE_45)
#define DEGREE_270		(DEGREE_180 + DEGREE_90)
#define DEGREE_360		(DEGREE_180 * 2.0f)

#define DEGREE_225		(DEGREE_180 + DEGREE_45)
#define DEGREE_315		(DEGREE_270 + DEGREE_45)

#define DEGREE_30		(DEGREE_180 / 6.0f)
#define DEGREE_60		(DEGREE_180 / 3.0f)
#define DEGREE_120		(DEGREE_360 / 3.0f)

#define DEGREE_1		(DEGREE_180 / 180.0f)
#define DEGREE_5		(DEGREE_180 / 36.0f)
#define DEGREE_10		(DEGREE_180 / 18.0f)
#define DEGREE_15		(DEGREE_180 / 12.0f)
#define DEGREE_20		(DEGREE_180 / 8.0f)

// Conversion routines
#define ANGLE_TO_RAD	ANGLE_1
#define RAD_TO_ANGLE	(180.0f / ANGLE_180)

#define SHORT_TO_ANGLE	(360.0f / 65536.0f)

//mxd. Used by skeletal joints logic.
#define RAD_TO_BYTEANGLE (161.7014176816138f) //TODO: == 254 / ANGLE_90? Why 254?..

//mxd
#define BYTEANGLE_TO_RAD	(ANGLE_360 / 255.0f)