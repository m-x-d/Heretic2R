//
// Heretic2.h
//
// Copyright 1998 Raven Software
//

#pragma once

#ifdef _HERETIC2_
	#define GAME_DECLSPEC __declspec(dllexport)
#else 
	#define GAME_DECLSPEC __declspec(dllimport)
#endif