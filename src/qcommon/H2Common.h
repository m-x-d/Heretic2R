//
// H2Common.h
//
// Copyright 1998 Raven Software
//

#pragma once

#ifdef H2COMMON_STATIC
	#define H2COMMON_API
#else
	#ifdef H2COMMON
		#define H2COMMON_API __declspec(dllexport)
	#else
		#define H2COMMON_API __declspec(dllimport)
	#endif
#endif

//TODO: remove when appropriate headers are added...

//Defined in q_shared.h:
#ifndef M_PI
	#define M_PI		3.14159265358979323846	// matches value in gcc v2 math.h
#endif
