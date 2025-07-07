//
// gl1_Model.c
//
// Copyright 1998 Raven Software
//

#include "gl1_Local.h"
#include "gl1_FlexModel.h"
#include "gl1_Image.h"

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

static model_t* Mod_ForName(const char* name, const qboolean crash)
{
	NOT_IMPLEMENTED
}

void R_BeginRegistration(const char* model)
{
	NOT_IMPLEMENTED
}

struct model_s* R_RegisterModel(const char* name)
{
	char img_name[MAX_OSPATH];

	model_t* mod = Mod_ForName(name, false);

	if (mod == NULL)
		return NULL;

	mod->registration_sequence = registration_sequence;

	switch (mod->type) //mxd. No mod_alias case.
	{
		case mod_brush:
			for (int i = 0; i < mod->numtexinfo; i++)
				mod->texinfo[i].image->registration_sequence = registration_sequence;
			break;

		case mod_sprite:
		{
			dsprite_t* sprout = mod->extradata;
			for (int i = 0; i < sprout->numframes; i++)
			{
				Com_sprintf(img_name, sizeof(img_name), "Sprites/%s", sprout->frames[i].name); // H2: extra "Sprites/" prefix
				mod->skins[i] = R_FindImage(img_name, it_sprite);
			}
		} break;

		case mod_fmdl: // H2
			Mod_RegisterFlexModel(mod);
			break;

		case mod_book: // H2
		{
			book_t* book = mod->extradata;
			bookframe_t* bframe = book->bframes;
			for (int i = 0; i < book->bheader.num_segments; i++, bframe++)
			{
				Com_sprintf(img_name, sizeof(img_name), "Book/%s", bframe->name);
				mod->skins[i] = R_FindImage(img_name, it_pic);
			}
		} break;

		default:
			ri.Sys_Error(ERR_DROP, "R_RegisterModel '%s' failed\n", name); //mxd. Sys_Error() -> ri.Sys_Error().
			return NULL;
	}

	return mod;
}

void R_EndRegistration(void)
{
	NOT_IMPLEMENTED
}