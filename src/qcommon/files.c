//
// files.c -- Heretic 2 filesystem
//
// Copyright 1998 Raven Software
//

// All of Quake's data access is through a hierarchical file system, but the contents of the file system
// can be transparently merged from several sources.

// The "base directory" is the path to the directory holding the quake.exe and all game directories.
// The sys_* files pass this to host_init in quakeparms_t->basedir. This can be overridden with the "-basedir" command line param
// to allow code debugging in a different directory.
// The base directory is only used during filesystem initialization.

// The "game directory" is the first tree on the search path and directory that all generated files
// (savegames, screenshots, demos, config files) will be saved to. This can be overridden with the "-game" command line parameter.
// The game directory can never be changed while quake is executing.
// This is a precaution against having a malicious server instruct clients to write files over areas they shouldn't.

#include "qcommon.h"
#include "qfiles.h"

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

int FS_FileLength(FILE* f)
{
	NOT_IMPLEMENTED
	return 0;
}

// Q2 counterpart
// For some reason, other dll's can't just cal fclose() on files returned by FS_FOpenFile...
void FS_FCloseFile(FILE* f)
{
	fclose(f);
}

int FS_FOpenFile(char* filename, FILE** file)
{
	NOT_IMPLEMENTED
	return 0;
}

void FS_Read(void* buffer, int len, FILE* file)
{
	NOT_IMPLEMENTED
}

// Q2 counterpart
// Filenames are relative to the quake search path.
// Passing a null buffer will return the file length without loading it.
int FS_LoadFile(char* path, void** buffer)
{
	FILE* file;

	const int len = FS_FOpenFile(path, &file);
	if (file == NULL)
	{
		if (buffer != NULL)
			*buffer = NULL;

		return -1;
	}
 
	if (buffer == NULL)
	{
		fclose(file);
		return len;
	}
 
	byte* buf = Z_Malloc(len);
	*buffer = buf;

	FS_Read(buf, len, file);
	fclose(file);

	return len;
}

// Q2 counterpart
void FS_FreeFile(void* buffer)
{
	Z_Free(buffer);
}

static int pakfile_comparer(const void* f1, const void* f2)
{
	const packfile_t* pf1 = f1;
	const packfile_t* pf2 = f2;

	return Q_stricmp(pf1->name, pf2->name);
}

static pack_t* FS_LoadPackFile(char* packfile)
{
	static dpackfile_t info[MAX_FILES_IN_PACK]; //mxd. Made static

	FILE* packhandle;
	dpackheader_t header;

	if (fopen_s(&packhandle, packfile, "rb") != 0) //mxd. fopen -> fopen_s
		return NULL;

	fread(&header, 1, sizeof(header), packhandle);
	if (header.ident != IDPAKHEADER)
		Com_Error(ERR_FATAL, "%s is not a packfile", packfile);

	const int numpackfiles = header.dirlen / (int)sizeof(dpackfile_t);

	if (numpackfiles > MAX_FILES_IN_PACK)
		Com_Error(ERR_FATAL, "%s has %i files", packfile, numpackfiles);

	packfile_t* newfiles = Z_Malloc(numpackfiles * (int)sizeof(packfile_t));

	fseek(packhandle, header.dirofs, SEEK_SET);
	fread(info, 1, header.dirlen, packhandle);

	// CRC the directory to check for modifications.
	//Com_BlockChecksum(info, header.dirlen); //mxd. Skipped: checksum is unused

	// Parse the directory
	for (int i = 0; i < numpackfiles; i++)
	{
		strcpy_s(newfiles[i].name, sizeof(newfiles[i].name), info[i].name); //mxd. strcpy -> strcpy_s
		newfiles[i].filepos = info[i].filepos;
		newfiles[i].filelen = info[i].filelen;
	}

	// H2: sort newfiles by name
	qsort(newfiles, numpackfiles, sizeof(packfile_t), pakfile_comparer);

	// Create pack_t
	pack_t* pack = Z_Malloc(sizeof(pack_t));
	strcpy_s(pack->filename, sizeof(pack->filename), packfile); //mxd. strcpy -> strcpy_s
	pack->handle = packhandle;
	pack->numfiles = numpackfiles;
	pack->files = newfiles;

	Com_Printf("Added packfile %s (%i files)\n", packfile, numpackfiles);

	return pack;
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

// Called to find where to write a file (demos, savegames, etc).
char* FS_Gamedir(void)
{
	return fs_gamedir;
}

void FS_SetGamedir(char* dir)
{
	NOT_IMPLEMENTED
}

char* FS_Userdir(void)
{
	return fs_userdir->string;
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