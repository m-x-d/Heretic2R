//
// gl_model.c -- model loading and caching
//
// Copyright 1998 Raven Software
//

#include "gl_local.h"

byte mod_novis[MAX_MAP_LEAFS / 8];

int registration_sequence;

void Mod_Modellist_f(void)
{
	NOT_IMPLEMENTED
}

// Q2 counterpart
void Mod_Init(void)
{
	memset(mod_novis, 0xff, sizeof(mod_novis));
}

void R_BeginRegistration(char* model)
{
	NOT_IMPLEMENTED
}

struct model_s* R_RegisterModel(char* name)
{
	NOT_IMPLEMENTED
	return NULL;
}

void R_EndRegistration(void)
{
	NOT_IMPLEMENTED
}