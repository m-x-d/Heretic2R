//
// common.c -- Misc functions used in client and server
//
// Copyright 1998 Raven Software
//

#include <setjmp.h>
#include "qcommon.h"
#include "keys.h"
#include "screen.h"
#include "snd_dll.h"

#define MAX_NUM_ARGVS	50

int com_argc;
char* com_argv[MAX_NUM_ARGVS + 1];

jmp_buf abortframe; // An ERR_DROP occured, exit the entire frame.

cvar_t* host_speeds;
cvar_t* log_stats;
cvar_t* developer;
cvar_t* timescale;
cvar_t* fixedtime;
cvar_t* logfile_active;	// 1 = buffer log, 2 = flush after each print
cvar_t* showtrace;
cvar_t* dedicated;

// New in H2:
cvar_t* fpu_precision;
cvar_t* hideconprint;
cvar_t* player_dll;

#pragma region ========================== CLIENT / SERVER INTERACTIONS ====================

void Com_Printf(char* fmt, ...)
{
	NOT_IMPLEMENTED
}

void Com_ColourPrintf(PalIdx_t colour, char* msg, ...)
{
	NOT_IMPLEMENTED
}

void Com_Error(int code, char* fmt, ...)
{
	NOT_IMPLEMENTED
}

void Com_Quit(void)
{
	NOT_IMPLEMENTED
}

#pragma endregion

#pragma region ========================== SIZEBUFFER HANDLING =============================

// Q2 counterpart
void SZ_Init(sizebuf_t* buf, byte* data, const int length)
{
	memset(buf, 0, sizeof(sizebuf_t));
	buf->data = data;
	buf->maxsize = length;
}

#pragma endregion

#pragma region ========================== COMMAND LINE ARGS PROCESSING ====================

// Q2 counterpart
static void COM_InitArgv(const int argc, char** argv)
{
	if (argc > MAX_NUM_ARGVS)
		Com_Error(ERR_FATAL, "argc > MAX_NUM_ARGVS");

	com_argc = argc;
	for (int i = 0; i < argc; i++)
	{
		if (argv[i] == NULL || strlen(argv[i]) >= MAX_TOKEN_CHARS)
			com_argv[i] = "";
		else
			com_argv[i] = argv[i];
	}
}

#pragma endregion

#pragma region ========================== ZONE MEMORY ALLOCATION ==========================

#define Z_MAGIC		0x1d1d

typedef struct zhead_s
{
	struct zhead_s* prev;
	struct zhead_s* next;
	short magic;
	short tag; // For group free
	int size;
} zhead_t;

zhead_t z_chain;
int z_count;
int z_bytes;

static void Z_Stats_f(void)
{
	NOT_IMPLEMENTED
}

// Q2 counterpart
void* Z_TagMalloc(int size, const int tag)
{
	size += sizeof(zhead_t);
	zhead_t* z = malloc(size);

	if (z == NULL)
		Com_Error(ERR_FATAL, "Z_Malloc: failed on allocation of %i bytes", size);

	memset(z, 0, size);

	z_count++;
	z_bytes += size;
	z->magic = Z_MAGIC;
	z->tag = (short)tag;
	z->size = size;

	z->next = z_chain.next;
	z->prev = &z_chain;
	z_chain.next->prev = z;
	z_chain.next = z;

	return z + 1;
}

// Q2 counterpart
void* Z_Malloc(const int size)
{
	return Z_TagMalloc(size, 0);
}

#pragma endregion

static void Com_Error_f(void)
{
	NOT_IMPLEMENTED
}

void Qcommon_Init(int argc, char** argv)
{
	if (setjmp(abortframe))
		Sys_Error("Error during initialization");

	z_chain.prev = &z_chain;
	z_chain.next = &z_chain;

	// Prepare enough of the subsystems to handle cvar and command buffer management
	COM_InitArgv(argc, argv);

	// Missing in H2: Swap_Init();
	Cbuf_Init();

	Cmd_Init();
	Cvar_Init();

	Key_Init();

	// We need to add the early commands twice, because a basedir or cddir needs to be set before execing config files,
	// but we want other params to override the settings of the config files.
	Cbuf_AddEarlyCommands(false);
	Cbuf_Execute();

	FS_InitFilesystem();
	SndDll_Init(); // New in H2

	Cbuf_AddText("exec default.cfg\n");
	Cbuf_AddText("exec config.cfg\n");

	Cbuf_AddEarlyCommands(true);
	Cbuf_Execute();

	// Init commands and vars
	Cmd_AddCommand("z_stats", Z_Stats_f);
	Cmd_AddCommand("error", Com_Error_f);

	host_speeds = Cvar_Get("host_speeds", "0", 0);
	log_stats = Cvar_Get("log_stats", "0", 0);
	developer = Cvar_Get("developer", "0", 0);
	timescale = Cvar_Get("timescale", "1", 0);
	fixedtime = Cvar_Get("fixedtime", "0", 0);
	logfile_active = Cvar_Get("logfile", "0", 0);
	showtrace = Cvar_Get("showtrace", "0", 0);
	dedicated = Cvar_Get("dedicated", "0", CVAR_NOSET);

	// New in H2:
	fpu_precision = Cvar_Get("fpu_precision", "1", 0);
	//sys_copyfail = Cvar_Get("sys_copyfail", "You must have the Heretic II CD in the drive to play.", 0); //mxd. Relevant logic skipped
	hideconprint = Cvar_Get("hideconprint", "0", 0);
	player_dll = Cvar_Get("player_dll", "Player", 0);

	char* s = va("%s:  %s %s %s", VERSIONFULL, CPUSTRING, __DATE__, BUILDSTRING);
	Cvar_Get("version", s, CVAR_SERVERINFO | CVAR_NOSET);

	// Q2: allow_download_ cvars inited in SV_Init()
	allow_download = Cvar_Get("allow_download", "1", CVAR_ARCHIVE);
	allow_download_maps = Cvar_Get("allow_download_maps", "1", CVAR_ARCHIVE);
	allow_download_players = Cvar_Get("allow_download_players", "1", CVAR_ARCHIVE);
	allow_download_models = Cvar_Get("allow_download_models", "1", CVAR_ARCHIVE);
	allow_download_sounds = Cvar_Get("allow_download_sounds", "1", CVAR_ARCHIVE);

	if ((int)dedicated->value)
		Cmd_AddCommand("quit", Com_Quit);

	Sys_Init();

	NET_Init();
	Netchan_Init();

	SV_Init();
	CL_Init();

	// Add + commands from command line.
	if (!Cbuf_AddLateCommands())
	{
		// If the user didn't give any commands, run default action.
		if ((int)dedicated->value)
			Cbuf_AddText("dedicated_start\n");
		else
			Cbuf_AddText("demo_loop\n"); // Q2: "d1\n"

		Cbuf_Execute();
	}
	else
	{
		// The user asked for something explicit, so drop the loading plaque.
		SCR_EndLoadingPlaque();
	}

	Com_ColourPrintf(P_HEADER, "====== Heretic 2 Initialized ======\n\n"); // Q2: Com_Printf
}

void Qcommon_Frame(int msec)
{
	NOT_IMPLEMENTED
}