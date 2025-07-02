//
// gl1_Model.c
//
// Copyright 1998 Raven Software
//

#include "gl1_Local.h"
#include "gl1_Model.h"

int registration_sequence;

static byte mod_novis[MAX_MAP_LEAFS / 8];

#define MAX_MOD_KNOWN 512
static model_t mod_known[MAX_MOD_KNOWN];
static int mod_numknown;

void Mod_Modellist_f(void)
{
	NOT_IMPLEMENTED
}

// Q2 counterpart
void Mod_Init(void)
{
	memset(mod_novis, 0xff, sizeof(mod_novis));
}

static void Mod_Free(model_t* mod)
{
	NOT_IMPLEMENTED
}

// Q2 counterpart
void Mod_FreeAll(void)
{
	for (int i = 0; i < mod_numknown; i++)
		if (mod_known[i].extradatasize)
			Mod_Free(&mod_known[i]);
}

void R_BeginRegistration(const char* model)
{
	NOT_IMPLEMENTED
}

struct model_s* R_RegisterModel(const char* name)
{
	NOT_IMPLEMENTED
	return NULL;
}

void R_EndRegistration(void)
{
	NOT_IMPLEMENTED
}