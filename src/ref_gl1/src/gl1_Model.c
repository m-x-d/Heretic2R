//
// gl1_Model.c
//
// Copyright 1998 Raven Software
//

#include "gl1_Local.h"
#include "gl1_FlexModel.h"
#include "gl1_Image.h"
#include "Hunk.h"

int registration_sequence;

model_t* loadmodel;
static int modfilelen;

static byte mod_novis[MAX_MAP_LEAFS / 8];

#define MAX_MOD_KNOWN 512
static model_t mod_known[MAX_MOD_KNOWN];
static int mod_numknown;

// The inline ('*1', '*2', ...) models from the current map are kept separate.
static model_t mod_inline[MAX_MOD_KNOWN];

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

static void Mod_LoadBookModel(model_t* mod, const void* buffer) // H2
{
	const book_t* book_in = buffer;
	book_t* book_out = Hunk_Alloc(modfilelen);

	if (book_in->bheader.version != BOOK_VERSION)
		ri.Sys_Error(ERR_DROP, "Mod_LoadBookModel: '%s' has wrong version number (%i should be %i)!", mod->name, book_in->bheader.version, BOOK_VERSION); //mxd. Sys_Error() -> ri.Sys_Error().

	if (book_in->bheader.num_segments > MAX_FRAMES)
		ri.Sys_Error(ERR_DROP, "Mod_LoadBookModel: '%s' has too many frames (%i > %i)!", mod->name, book_in->bheader.num_segments, MAX_FRAMES); //mxd. Sys_Error() -> ri.Sys_Error().

	// Copy everything.
	memcpy(book_out, book_in, book_in->bheader.num_segments * sizeof(bookframe_t) + sizeof(bookheader_t));

	// Pre-load frame images.
	bookframe_t* frame = &book_out->bframes[0];
	for (int i = 0; i < book_out->bheader.num_segments; i++, frame++)
	{
		char frame_name[MAX_QPATH];
		Com_sprintf(frame_name, sizeof(frame_name), "Book/%s", frame->name);

		mod->skins[i] = R_FindImage(frame_name, it_pic);
	}

	// Set model type.
	mod->type = mod_book;
}

static void Mod_LoadSpriteModel(model_t* mod, const void* buffer)
{
	NOT_IMPLEMENTED
}

#pragma region ========================== BRUSHMODEL LOADING ==========================

static void Mod_LoadBrushModel(model_t* mod, void* buffer)
{
	NOT_IMPLEMENTED
}

#pragma endregion

static model_t* Mod_ForName(const char* name, const qboolean crash)
{
	int i;
	model_t* mod;

	if (!name[0])
		ri.Sys_Error(ERR_DROP, "Mod_ForName: NULL name");

	// Inline models are grabbed only from worldmodel.
	if (name[0] == '*')
	{
		const int index = (int)strtol(name + 1, NULL, 10); //mxd. atoi -> strtol
		if (index < 1 || r_worldmodel == NULL || index >= r_worldmodel->numsubmodels)
			ri.Sys_Error(ERR_DROP, "Mod_ForName: bad inline model number");

		return &mod_inline[index];
	}

	// Search the currently loaded models.
	for (i = 0, mod = &mod_known[0]; i < mod_numknown; i++, mod++)
		if (mod->name[0] != 0 && strcmp(mod->name, name) == 0)
			return mod;

	// Find a free model slot.
	for (i = 0, mod = &mod_known[0]; i < mod_numknown; i++, mod++)
		if (mod->name[0] == 0)
			break; // Free slot.

	if (i == mod_numknown)
	{
		if (mod_numknown == MAX_MOD_KNOWN)
			ri.Sys_Error(ERR_DROP, "Mod_ForName: mod_numknown == MAX_MOD_KNOWN");

		mod_numknown++;
	}

	strcpy_s(mod->name, sizeof(mod->name), name); //mxd. strcpy -> strcpy_s

	// Load the file.
	char* buf;
	modfilelen = ri.FS_LoadFile(mod->name, (void**)&buf);

	if (buf == NULL)
	{
		if (crash)
			ri.Sys_Error(ERR_DROP, "Mod_ForName: '%s' not found", mod->name);

		memset(mod, 0, sizeof(mod->name));
		return NULL;
	}

	loadmodel = mod;

	// H2: check for FlexModel header...
	if (modfilelen > 6 && Q_strncasecmp(buf, "header", 6) == 0)
	{
		int datasize = 0x200000;
		if (strstr(name, "players/") || strstr(name, "models/player/"))
			datasize = 0x400000;

		mod->extradata = Hunk_Begin(datasize);
		Mod_LoadFlexModel(mod, buf, modfilelen);
	}
	else
	{
		// Call the appropriate loader.
		switch (LittleLong(*(uint*)buf))
		{
			// Missing: case IDALIASHEADER
			case IDSPRITEHEADER:
				mod->extradata = Hunk_Begin(0x10000);
				Mod_LoadSpriteModel(mod, buf);
				break;

			case IDBOOKHEADER: // H2
				mod->extradata = Hunk_Begin(0x10000);
				Mod_LoadBookModel(mod, buf);
				break;

			case IDBSPHEADER:
				mod->extradata = Hunk_Begin(0x1000000);
				Mod_LoadBrushModel(mod, buf);
				break;

			default:
				ri.Sys_Error(ERR_DROP, "Mod_ForName: unknown file id for '%s'", mod->name);
				break;
		}
	}

	mod->extradatasize = Hunk_End();
	ri.FS_FreeFile(buf);

	return mod;
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