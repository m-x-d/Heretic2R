//
// launcher_main.c -- Heretic2R launcher (Linux). Replaces win32 WinMain.
//
// Copyright 1998 Raven Software
//

#include "sys_unix.h"

int main(int argc, char** argv)
{
	return Quake2Main_Unix(argc, argv);
}
