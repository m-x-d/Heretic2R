//
// Heretic2.h
//
// Copyright 1998 Raven Software
//

#pragma once

#ifdef QUAKE2_DLL
	#define GAME_DECLSPEC __declspec(dllexport)
#else 
	#define GAME_DECLSPEC __declspec(dllimport)
#endif