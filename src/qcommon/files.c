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

static char fs_gamedir[MAX_OSPATH];
static cvar_t* fs_basedir;
static cvar_t* fs_userdir; // H2
cvar_t* fs_configsdir; //mxd
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

typedef struct filelink_s
{
	struct filelink_s* next;
	char* from;
	int fromlength;
	char* to;
} filelink_t;

static filelink_t* fs_links;

typedef struct searchpath_s
{
	char filename[MAX_OSPATH];
	pack_t* pack; // Only one of filename / pack will be used
	struct searchpath_s* next;
} searchpath_t;

static searchpath_t* fs_searchpaths;
static searchpath_t* fs_base_searchpaths; // Without gamedirs

qboolean file_from_pak = false; //mxd. int in Q2

// Q2 counterpart
int FS_FileLength(FILE* f)
{
	const int pos = ftell(f);
	fseek(f, 0, SEEK_END);

	const int end = ftell(f);
	fseek(f, pos, SEEK_SET);

	return end;
}

// Creates any directories needed to store the given filename
void FS_CreatePath(char* path)
{
	for (char* ofs = path + 1; *ofs != 0; ofs++)
	{
		if (*ofs == '/' || *ofs == '\\') // H2: extra '\\' check.
		{
			// Create the directory
			*ofs = 0;
			Sys_Mkdir(path);
			*ofs = '/';
		}
	}
}

// Returns OS path for given filename.
char* FS_GetPath(const char* name) // H2
{
	char* path = FS_NextPath(NULL);
	while (path != NULL)
	{
		const char* filepath = Sys_FindFirst(va("%s/%s", path, name), 0, 0);
		Sys_FindClose();

		if (filepath != NULL)
			break;

		path = FS_NextPath(path);
	}

	return path;
}

// Q2 counterpart
// For some reason, other dll's can't just call fclose() on files returned by FS_FOpenFile...
void FS_FCloseFile(FILE* f)
{
	fclose(f);
}

// Finds the file in the search path. Returns file size and an open FILE*.
// Used for streaming data out of either a pak file or a separate file.
int FS_FOpenFile(const char* filename, FILE** file)
{
	char filepath[MAX_OSPATH];
	char netpath[MAX_OSPATH];

	file_from_pak = false;

	// H2: fix separator chars, trim end spaces...
	strcpy_s(filepath, sizeof(filepath), filename);

	const int len = (int)strlen(filename);
	for (int i = 0; i < len; i++)
		if (filepath[i] == '\\')
			filepath[i] = '/';

	for (int i = len - 1; i >= 0 && filepath[i] == ' '; i--)
		filepath[i] = 0;

	// Check for links first
	for (const filelink_t* link = fs_links; link != NULL; link = link->next)
	{
		if (!strncmp(filename, link->from, link->fromlength))
		{
			Com_sprintf(netpath, sizeof(netpath), "%s%s", link->to, filename + link->fromlength);

			if (fopen_s(file, netpath, "rb") == 0) //mxd. fopen -> fopen_s
			{
				Com_DPrintf("link file: %s\n", netpath);
				return FS_FileLength(*file);
			}

			return -1;
		}
	}

	// Search through the path, one element at a time
	for (searchpath_t* search = fs_searchpaths; ; search = search->next)
	{
		if (search == NULL)
		{
			Com_DPrintf("FindFile: can't find %s\n", filename);
			*file = NULL;

			return -1;
		}

		// Is the element a pak file?
		if (search->pack != NULL)
		{
			// Look through all the pak file elements
			pack_t* pak = search->pack;
			if (pak->numfiles < 1)
				continue;

			// H2: do binary search instead of iteration, because pak filenames are sorted alphabetically.
			int start = 0;
			int end = pak->numfiles;

			do
			{
				const int index = (start + end) / 2;
				const int cmp = Q_stricmp(filepath, pak->files[index].name);

				if (cmp == 0)
				{
					// Found it!
					file_from_pak = true;
					Com_DDPrintf(2, "PackFile: %s : %s\n", pak->filename, filename); //mxd. Com_DPrintf() -> Com_DDPrintf(), to reduce console spam when developer 1.

					// Open a new file on the pakfile
					if (fopen_s(file, pak->filename, "rb") != 0) //mxd. fopen -> fopen_s
						Com_Error(ERR_FATAL, "Couldn't reopen %s", pak->filename);

					fseek(*file, pak->files[index].filepos, SEEK_SET);

					return pak->files[index].filelen;
				}

				if (cmp > 0)
					start = index + 1;
				else // cmp < 0
					end = index;
			} while (start < end);
		}
		else
		{
			// Check a file in the directory tree
			Com_sprintf(netpath, sizeof(netpath), "%s/%s", search->filename, filename);

			if (fopen_s(file, netpath, "rb") != 0) //mxd. fopen -> fopen_s
				continue;

			Com_DDPrintf(2, "FindFile: %s\n", netpath); //mxd. Com_DPrintf() -> Com_DDPrintf(), to reduce console spam when developer 1.

			return FS_FileLength(*file);
		}
	}
}

// Q2 counterpart (original H2 logic)
// Properly handles partial reads
void FS_Read(void* buffer, const int len, FILE* file)
{
	byte* buf = buffer;

	// Read in chunks for progress bar
	int remaining = len;
	while (remaining > 0)
	{
		const int block = min(remaining, 0x10000); // Read in blocks of 64k
		const int read = (int)fread(buf, 1, block, file);

		//mxd. Skip logic related to reading from a CD...
		if (read < 1)
			Com_Error(ERR_FATAL, "FS_Read: %i bytes read", read);

		// Do some progress bar thing here...
		remaining -= read;
		buf += read;
	}
}

// Q2 counterpart
// Filenames are relative to the quake search path.
// Passing a null buffer will return the file length without loading it.
int FS_LoadFile(const char* path, void** buffer)
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
qboolean FS_CopyFile(const char* src, const char* dst) //mxd. CopyFile() in Q2 (defined in sv_ccmds.c). Renamed to avoid collision with CopyFile defined in winbase.h (cause H2 game.h includes <windows.h>). Added return type.
{
	static byte buffer[65536]; //mxd. Made static
	FILE* f1;
	FILE* f2;

	Com_DDPrintf(2, "FS_CopyFile (%s, %s)\n", src, dst); //mxd. Com_DPrintf() -> Com_DDPrintf(), to reduce console spam when developer 1.

	if (fopen_s(&f1, src, "rb") != 0) //mxd. fopen -> fopen_s
		return false;

	if (fopen_s(&f2, dst, "wb") != 0) //mxd. fopen -> fopen_s
	{
		fclose(f1);
		return false;
	}

	while (true)
	{
		const uint len = fread(buffer, 1, sizeof(buffer), f1);
		if (len == 0)
			break;

		fwrite(buffer, 1, len, f2);
	}

	fclose(f1);
	fclose(f2);

	return true;
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

	// Add any pak files in the format Htic2-0.pak .. Htic2-9.pak.
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

qboolean FS_IsValidDirName(const char* dir) //mxd. Added to reduce code duplication...
{
	if (strstr(dir, "..") != NULL || strchr(dir, '/') != NULL || strchr(dir, '\\') != NULL || strchr(dir, ':') != NULL)
		return false;

	return true;
}

// Q2 counterpart
// Sets the gamedir and path to a different directory.
void FS_SetGamedir(const char* dir)
{
	if (!FS_IsValidDirName(dir)) //mxd. Use FS_IsValidDirName().
	{
		Com_Printf("Gamedir should be a single filename, not a path\n");
		return;
	}

	// Free up any current game dir info.
	while (fs_searchpaths != fs_base_searchpaths)
	{
		if (fs_searchpaths->pack != NULL)
		{
			fclose(fs_searchpaths->pack->handle);
			Z_Free(fs_searchpaths->pack->files);
			Z_Free(fs_searchpaths->pack);
		}

		searchpath_t* next = fs_searchpaths->next;
		Z_Free(fs_searchpaths);
		fs_searchpaths = next;
	}

	// Flush all data, so it will be forced to reload.
	if (dedicated != NULL && !(int)dedicated->value)
		Cbuf_AddText("vid_restart\nsnd_restart\n");

	Com_sprintf(fs_gamedir, sizeof(fs_gamedir), "%s/%s", fs_basedir->string, dir);

	if (strcmp(dir, BASEDIRNAME) == 0 || *dir == 0)
	{
		Cvar_FullSet("gamedir", "", CVAR_SERVERINFO | CVAR_NOSET);
		Cvar_FullSet("game", "", CVAR_LATCH | CVAR_SERVERINFO);
	}
	else
	{
		Cvar_FullSet("gamedir", dir, CVAR_SERVERINFO | CVAR_NOSET);
		//mxd. Skip fs_cddir logic.
		FS_AddGameDirectory(va("%s/%s", fs_basedir->string, dir));
	}
}

char* FS_Userdir(void)
{
	return fs_userdir->string;
}

void FS_ExecAutoexec(void)
{
	char name[MAX_OSPATH]; // Q2: MAX_QPATH

	const char* dir = Cvar_VariableString("gamedir");
	if (*dir == 0)
		dir = BASEDIRNAME;

	Com_sprintf(name, sizeof(name), "%s/%s/autoexec.cfg", fs_basedir->string, dir);

	if (Sys_FindFirst(name, 0, SFF_SUBDIR | SFF_HIDDEN | SFF_SYSTEM))
		Cbuf_AddText("exec autoexec.cfg\n");

	Sys_FindClose();
}

// Q2 counterpart
// Creates a filelink_t.
static void FS_Link_f(void)
{
	if (Cmd_Argc() != 3)
	{
		Com_Printf("USAGE: link <from> <to>\n");
		return;
	}

	// See if the link already exists.
	const char* from = Cmd_Argv(1); //mxd
	const char* to = Cmd_Argv(2); //mxd

	filelink_t** prev = &fs_links;
	for (filelink_t* l = fs_links; l != NULL; l = l->next)
	{
		if (strcmp(l->from, from) == 0)
		{
			Z_Free(l->to);

			if (*to != 0)
			{
				l->to = CopyString(to);
			}
			else
			{
				// Delete it.
				*prev = l->next;
				Z_Free(l->from);
				Z_Free(l);
			}

			return;
		}

		prev = &l->next;
	}

	// Create a new link.
	filelink_t* l = Z_Malloc(sizeof(filelink_t));
	l->next = &fs_links[0];
	fs_links = l;
	l->from = CopyString(from);
	l->fromlength = (int)strlen(l->from);
	l->to = CopyString(to);
}

// Q2 counterpart
static void FS_Path_f(void)
{
	Com_Printf("Current search path:\n");
	for (const searchpath_t* s = fs_searchpaths; s != NULL; s = s->next)
	{
		if (s == fs_base_searchpaths)
			Com_Printf("----------\n");

		if (s->pack != NULL)
			Com_Printf("%s (%i files)\n", s->pack->filename, s->pack->numfiles);
		else
			Com_Printf("%s\n", s->filename);
	}

	Com_Printf("\nLinks:\n");
	for (const filelink_t* l = fs_links; l != NULL; l = l->next)
		Com_Printf("%s : %s\n", l->from, l->to);
}

// Q2 counterpart
// Allows enumerating all of the directories in the search path
char* FS_NextPath(const char* prevpath)
{
	if (prevpath == NULL)
		return fs_gamedir;

	const char* prev = fs_gamedir;
	for (searchpath_t* s = fs_searchpaths; s != NULL; s = s->next)
	{
		if (s->pack)
			continue;

		if (prevpath == prev)
			return s->filename;

		prev = s->filename;
	}

	return NULL;
}

//mxd. Copy base\config\*.cfg to "x:\Users\[User]\Saved Games\Heretic2R\configs".
static void CopyConfigs(const char* src_dir, const char* dst_dir)
{
	if (Q_stricmp(src_dir, dst_dir) == 0) // Don't copy stuff over itself...
		return;

	char mask[MAX_OSPATH];
	Com_sprintf(mask, sizeof(mask), "%s/*.cfg", src_dir);

	const char* src_filename = Sys_FindFirst(mask, 0, 0);

	while (src_filename != NULL)
	{
		char dst_filename[MAX_OSPATH];
		Com_sprintf(dst_filename, sizeof(dst_filename), "%s%s", dst_dir, strrchr(src_filename, '/'));

		if (!Sys_IsFile(dst_filename)) // Skip if already exists.
		{
			FS_CreatePath(dst_filename);
			FS_CopyFile(src_filename, dst_filename);
		}

		src_filename = Sys_FindNext(0, 0);
	}

	Sys_FindClose();
}

void FS_InitFilesystem(void)
{
	Cmd_AddCommand("path", FS_Path_f);
	Cmd_AddCommand("link", FS_Link_f);
	//Missing in H2: Cmd_AddCommand("dir", FS_Dir_f);

	int arg_index = COM_CheckParm("-pakfirst"); // H2
	if (arg_index > 0)
		Cvar_SetValue("pakfirst", 1.0f);
	else
		Cvar_SetValue("pakfirst", 0.0f);

	// basedir <path>
	// Allows the game to run from outside the data tree.
	arg_index = COM_CheckParm("-basedir"); // H2
	if (arg_index > 0)
	{
		fs_basedir = Cvar_Get("basedir", COM_Argv(arg_index + 1), CVAR_NOSET);
	}
	else
	{
		char workdir[MAX_OSPATH];
		Sys_GetWorkingDir(workdir, sizeof(workdir)); //mxd. _getcwd() -> Sys_GetWorkingDir().

		fs_basedir = Cvar_Get("basedir", workdir, CVAR_NOSET); // "C:\Games\Heretic2"
	}

	//mxd. Skip fs_cddir / "-cddir" command line arg logic.

	// Start up with 'base' by default.
	FS_AddGameDirectory(va("%s/"BASEDIRNAME, fs_basedir->string));

	// Any set gamedirs will be freed up to here.
	fs_base_searchpaths = fs_searchpaths;

	// Check for game override.
	arg_index = COM_CheckParm("-game"); // H2
	if (arg_index > 0)
		fs_gamedirvar = Cvar_Get("game", COM_Argv(arg_index + 1), CVAR_LATCH | CVAR_SERVERINFO);
	else
		fs_gamedirvar = Cvar_Get("game", "", CVAR_LATCH | CVAR_SERVERINFO);

	if (fs_gamedirvar->string[0] != 0)
		FS_SetGamedir(fs_gamedirvar->string);

	// H2: set user directory.
	arg_index = COM_CheckParm("-userdir");
	qboolean use_modern_userdir = false; //mxd

	if (arg_index > 0)
	{
		fs_userdir = Cvar_Get("userdir", COM_Argv(arg_index + 1), CVAR_LATCH | CVAR_SERVERINFO);
	}
	else
	{
		char buffer[MAX_OSPATH];
		char userdir[MAX_OSPATH];

		//mxd. Change userdir location from "c:/Games/Heretic2/user" to "c:\Users\[User]\Saved Games\Heretic2R/user".
		if (Sys_GetOSUserDir(buffer, sizeof(buffer)))
		{
			strcat_s(buffer, sizeof(buffer), "/Heretic2R");
			use_modern_userdir = true;
		}
		else // Fall back to original logic...
		{
			Com_sprintf(buffer, sizeof(buffer), "%s", fs_basedir->string);
		}

		//mxd. Original logic gets userdir by subtracting BASEDIRNAME length from FS_Gamedir(), then appending "user" to it, which resulted in strange or incorrect userdir path for any mod... 
		char* gamedir = Cvar_VariableString("gamedir");

		if (*gamedir == 0)
			Com_sprintf(userdir, sizeof(userdir), "%s/user", buffer);
		else
			Com_sprintf(userdir, sizeof(userdir), "%s/user_%s", buffer, gamedir);

		fs_userdir = Cvar_Get("userdir", userdir, 0); // "C:\Games\Heretic2/user"
	}

	//mxd. Copy configs from [gamedir]/config to Heretic2R\[user]\configs?
	char vanilla_cfg_path[MAX_OSPATH];
	sprintf_s(vanilla_cfg_path, sizeof(vanilla_cfg_path), "%s/config", FS_GetPath("config"));

	if (use_modern_userdir)
	{
		char cfg_path[MAX_OSPATH];
		sprintf_s(cfg_path, sizeof(cfg_path), "%s/configs", fs_userdir->string);

		fs_configsdir = Cvar_Get("configsdir", cfg_path, CVAR_NOSET);
		CopyConfigs(vanilla_cfg_path, cfg_path);
	}
	else
	{
		fs_configsdir = Cvar_Get("configsdir", vanilla_cfg_path, CVAR_NOSET);
	}
}