//
// main.c -- Heretic2R launcher.
//
// Copyright 1998 Raven Software
//

#include "Quake2Main.h"

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
	return Quake2Main(hInstance, hPrevInstance, lpCmdLine, nShowCmd);
}