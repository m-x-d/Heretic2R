//
// H2Common.h
//
// Copyright 1998 Raven Software
//

#pragma once

#ifdef H2COMMON
	#define H2COMMON_API __declspec(dllexport)
#else
	#define H2COMMON_API __declspec(dllimport)
#endif