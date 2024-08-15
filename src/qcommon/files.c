//
// files.c -- Heretic 2 filesystem
//
// Copyright 1998 Raven Software
//

#include "qcommon.h"

char fs_gamedir[MAX_OSPATH];
cvar_t* fs_basedir;
cvar_t* fs_userdir; // New in H2
cvar_t* fs_gamedirvar;

typedef struct
{
	char name[MAX_QPATH];
	int filepos;
	int filelen;
} packfile_t;

// In-memory representation
typedef struct pack_s
{
	char filename[MAX_OSPATH];
	FILE* handle;
	int numfiles;
	packfile_t* files;
} pack_t;

typedef struct searchpath_s
{
	char filename[MAX_OSPATH];
	pack_t* pack; // Only one of filename / pack will be used
	struct searchpath_s* next;
} searchpath_t;

searchpath_t* fs_searchpaths;
searchpath_t* fs_base_searchpaths; // Without gamedirs

static pack_t* FS_LoadPackFile(char* packfile)
{
	NOT_IMPLEMENTED
	return NULL;
}

// Sets fs_gamedir, adds the directory to the head of the path, loads and adds Htic2-0.pak Htic2-1.pak ... (not necessarily in this order!)
static void FS_AddGameDirectory(char* dir)
{
	strcpy_s(fs_gamedir, sizeof(fs_gamedir), dir); //mxd. strcpy -> strcpy_s

	// Add the directory to the search path?
	const cvar_t* pakfirst = Cvar_Get("pakfirst", "0", CVAR_ARCHIVE); // H2: new 'pakfirst' logic
	if ((int)pakfirst->value)
	{
		searchpath_t* search = Z_Malloc(sizeof(searchpath_t));
		strcpy_s(search->filename, sizeof(search->filename), dir); //mxd. strcpy -> strcpy_s
		search->next = fs_searchpaths;
		fs_searchpaths = search;
	}

	// Add any pak files in the format Htic2-0.pak, Htic2-1.pak, ...
	for (int i = 0; i < 10; i++)
	{
		char pakfile[MAX_OSPATH];
		Com_sprintf(pakfile, sizeof(pakfile), "%s/Htic2-%i.pak", dir, i);

		pack_t* pak = FS_LoadPackFile(pakfile);
		if (pak != NULL)
		{
			searchpath_t* search = Z_Malloc(sizeof(searchpath_t));
			search->pack = pak;
			search->next = fs_searchpaths;
			fs_searchpaths = search;
		}
	}

	// Add the directory to the search path?
	if (!(int)pakfirst->value) // H2: new 'pakfirst' logic
	{
		searchpath_t* search = Z_Malloc(sizeof(searchpath_t));
		strcpy_s(search->filename, sizeof(search->filename), dir); //mxd. strcpy -> strcpy_s
		search->next = fs_searchpaths;
		fs_searchpaths = search;
	}
}

char* FS_Gamedir(void)
{
	NOT_IMPLEMENTED
	return NULL;
}

void FS_SetGamedir(char* dir)
{
	NOT_IMPLEMENTED
}

void FS_ExecAutoexec(void)
{
	NOT_IMPLEMENTED
}

static void FS_Link_f(void)
{
	NOT_IMPLEMENTED
}

static void FS_Path_f(void)
{
	NOT_IMPLEMENTED
}

void FS_InitFilesystem(void)
{
	Cmd_AddCommand("path", FS_Path_f);
	Cmd_AddCommand("link", FS_Link_f);
	//Missing in H2: Cmd_AddCommand("dir", FS_Dir_f);

	int arg_index = COM_CheckParm("-pakfirst"); // New in H2
	if (arg_index > 0)
		Cvar_SetValue("pakfirst", 1.0f);
	else
		Cvar_SetValue("pakfirst", 0.0f);

	// basedir <path>
	// Allows the game to run from outside the data tree
	arg_index = COM_CheckParm("-basedir"); // New in H2
	if (arg_index > 0)
	{
		fs_basedir = Cvar_Get("basedir", COM_Argv(arg_index + 1), CVAR_NOSET);
	}
	else
	{
		char workdir[256];

		_getcwd(workdir, sizeof(workdir));
		const uint len = strlen(workdir);

		if (workdir[len - 1] == '\\')
			workdir[len - 1] = 0;

		fs_basedir = Cvar_Get("basedir", workdir, CVAR_NOSET); // "C:\Games\Heretic2"
	}

	//mxd. Skip fs_cddir / "-cddir" command line arg logic

	// Start up with 'base' by default
	FS_AddGameDirectory(va("%s/"BASEDIRNAME, fs_basedir->string));

	// Any set gamedirs will be freed up to here
	fs_base_searchpaths = fs_searchpaths;

	// Check for game override
	arg_index = COM_CheckParm("-game"); // New in H2
	if (arg_index > 0)
		fs_gamedirvar = Cvar_Get("game", COM_Argv(arg_index + 1), CVAR_LATCH | CVAR_SERVERINFO);
	else
		fs_gamedirvar = Cvar_Get("game", "", CVAR_LATCH | CVAR_SERVERINFO);

	if (fs_gamedirvar->string[0])
		FS_SetGamedir(fs_gamedirvar->string);

	// New in H2: set user directory
	arg_index = COM_CheckParm("-userdir");
	if (arg_index > 0)
	{
		fs_userdir = Cvar_Get("userdir", COM_Argv(arg_index + 1), CVAR_LATCH | CVAR_SERVERINFO);
	}
	else
	{
		char buffer[MAX_OSPATH];
		char userdir[MAX_OSPATH];

		Com_sprintf(buffer, sizeof(buffer), "%s", FS_Gamedir());
		buffer[strlen(buffer) - strlen(BASEDIRNAME)] = 0; //TODO: does gamedir ALWAYS end with 'base'? Can't we just use fs_basedir here?
		Com_sprintf(userdir, sizeof(userdir), "%suser", buffer);

		fs_userdir = Cvar_Get("userdir", userdir, 0); // "C:\Games\Heretic2/user"
	}
}