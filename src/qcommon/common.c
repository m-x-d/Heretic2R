//
// common.c -- Misc functions used in client and server
//
// Copyright 1998 Raven Software
//

#include <float.h>
#include <setjmp.h>

#include "anorms.h"
#include "client.h"
#include "cmodel.h"
#include "console.h"
#include "qcommon.h"
#include "keys.h"
#include "screen.h"
#include "snd_dll.h"

#define MAXPRINTMSG		4096
#define MAX_NUM_ARGVS	50

static int com_argc;
static char* com_argv[MAX_NUM_ARGVS + 1];

jmp_buf abortframe; // An ERR_DROP occured, exit the entire frame.

FILE* log_stats_file;

cvar_t* host_speeds; //TODO: unused. Remove?
cvar_t* log_stats;
cvar_t* developer;
static cvar_t* timescale;
static cvar_t* fixedtime;
static cvar_t* logfile_active;	// 1 = buffer log, 2 = flush after each print
static cvar_t* showtrace;
cvar_t* dedicated;
cvar_t* vid_maxfps; // YQ2

// H2:
static cvar_t* fpu_precision;
static cvar_t* hideconprint;
cvar_t* player_dll;

static FILE* logfile;

static int server_state;

#pragma region ========================== CLIENT / SERVER INTERACTIONS ====================

static int rd_target;
static char* rd_buffer;
static int rd_buffersize;
static void	(*rd_flush)(int target, char* buffer);

// Q2 counterpart
void Com_BeginRedirect(const int target, char* buffer, const int buffersize, void (*flush)(int, char*))
{
	if (target == 0 || buffer == NULL || buffersize == 0 || flush == NULL)
		return;

	rd_target = target;
	rd_buffer = buffer;
	rd_buffersize = buffersize;
	rd_flush = flush;

	*rd_buffer = 0;
}

// Q2 counterpart
void Com_EndRedirect(void)
{
	rd_flush(rd_target, rd_buffer);

	rd_target = 0;
	rd_buffer = NULL;
	rd_buffersize = 0;
	rd_flush = NULL;
}

// Both client and server can use this, and it will output to the appropriate place.
void Com_Printf(const char* fmt, ...)
{
	char msg[MAXPRINTMSG];
	va_list argptr;

	va_start(argptr, fmt);
	vsprintf_s(msg, sizeof(msg), fmt, argptr); //mxd. vsprintf -> vsprintf_s
	va_end(argptr);

	Com_ColourPrintf(P_WHITE, "%s", msg); // Changed in H2
}

//mxd. Similar to Q2's Com_Printf
void Com_ColourPrintf(const PalIdx_t colour, const char* fmt, ...)
{
	if (hideconprint == NULL || !(int)hideconprint->value)
	{
		va_list argptr;
		char msg[MAXPRINTMSG];

		va_start(argptr, fmt);
		vsprintf_s(msg, sizeof(msg), fmt, argptr); //mxd. vsprintf -> vsprintf_s
		va_end(argptr);

		con.current_color = TextPalette[colour];

		if (rd_target)
		{
			if ((int)(strlen(msg) + strlen(rd_buffer)) > rd_buffersize - 1)
			{
				rd_flush(rd_target, rd_buffer);
				*rd_buffer = 0;
			}

			strcat_s(rd_buffer, rd_buffersize, msg); //mxd. strcat -> strcat_s
		}
		else
		{
			// Print to console
			Con_Print(msg);

			// Also echo to debugging console
			Sys_ConsoleOutput(msg);

			// Write to logfile?
			if (logfile_active && (int)logfile_active->value)
			{
				if (logfile == NULL)
				{
					char name[MAX_QPATH];
					Com_sprintf(name, sizeof(name), "%s/qconsole.log", FS_Userdir()); // Q2: FS_Gamedir()
					// MISSING: fopen(name, "a"); case
					fopen_s(&logfile, name, "w"); //mxd. fopen -> fopen_s
				}

				if (logfile != NULL)
					fprintf(logfile, "%s", msg);

				if (logfile_active->value > 1.0f)
					fflush(logfile); // Force it to save every time
			}
		}
	}

	con.current_color = TextPalette[P_WHITE];
}

// A Com_Printf that only shows up if the "developer" cvar is set.
void Com_DPrintf(const char* fmt, ...)
{
	va_list argptr;
	static char msg[MAXPRINTMSG]; //mxd. Make static.

	// Don't confuse non-developers with techie stuff...
	if (developer != NULL && (int)developer->value)
	{
		va_start(argptr, fmt);
		vsprintf_s(msg, sizeof(msg), fmt, argptr); //mxd. vsprintf -> vsprintf_s
		va_end(argptr);

		Com_ColourPrintf(P_YELLOW, "%s", msg); // Q2: Com_Printf
	}
}

//mxd. A Com_Printf that only shows up if the "developer" cvar is >= given level.
void Com_DDPrintf(const int level, const char* fmt, ...)
{
	static int level_colors[] = { P_WHITE, P_YELLOW, P_CYAN, P_PURPLE };
	static int max_level = sizeof(level_colors) / sizeof(level_colors[0]);

	va_list argptr;
	static char msg[MAXPRINTMSG];

	// Don't confuse non-developers with techie stuff...
	if (developer != NULL && (int)developer->value >= level)
	{
		va_start(argptr, fmt);
		vsprintf_s(msg, sizeof(msg), fmt, argptr); //mxd. vsprintf -> vsprintf_s
		va_end(argptr);

		Com_ColourPrintf(level_colors[ClampI(level, 0, max_level)], "%s", msg); // Q2: Com_Printf
	}
}

// Both client and server can use this, and it will do the appropriate things.
H2R_NORETURN void Com_Error(const int code, const char* fmt, ...)
{
	va_list argptr;
	static char msg[MAXPRINTMSG];
	static qboolean recursive;

	if (recursive)
		Sys_Error("recursive error after: %s", msg);

	recursive = true;

	va_start(argptr, fmt);
	vsprintf_s(msg, sizeof(msg), fmt, argptr); //mxd. vsprintf -> vsprintf_s
	va_end(argptr);

	switch (code)
	{
		case ERR_DISCONNECT:
			CL_Disconnect_f(); // H2
			break;

		case ERR_DROP:
			Com_Printf("*********************************\nERROR: %s\n****************************** ***\n", msg);
			SV_Shutdown(va("Server crashed: %s\n", msg), false);
			CL_Drop();
			recursive = false;
			longjmp(abortframe, -1);
			break;

		default:
			SV_Shutdown(va("Server fatal crashed: %s\n", msg), false);
			CL_Shutdown();
			break;
	}

	if (logfile != NULL)
	{
		fclose(logfile);
		logfile = NULL;
	}

	Sys_Error("%s", msg);
}

// Both client and server can use this, and it will do the appropriate things.
H2R_NORETURN void Com_Quit(void)
{
	SV_Shutdown("Server quit\n", false);
	// Missing: CL_Shutdown ();

	if (logfile != NULL)
	{
		fclose(logfile);
		logfile = NULL;
	}

	Sys_Quit();
}

int Com_ServerState(void)
{
	return server_state;
}

void Com_SetServerState(const int state)
{
	server_state = state;
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

// Q2 counterpart
void SZ_Clear(sizebuf_t* buf)
{
	buf->cursize = 0;
	buf->overflowed = false;
}

void* SZ_GetSpace(sizebuf_t* buf, const int length)
{
	if (buf->cursize + length > buf->maxsize)
	{
		if (!buf->allowoverflow)
		{
			//mxd. Original code also prints debug_file and debug_line here
			if (buf->maxsize == 0) // H2: extra sanity check
				Com_Error(ERR_FATAL, "SZ_GetSpace: buffer uninitialised");
			else
				Com_Error(ERR_FATAL, "SZ_GetSpace: overflow without allowoverflow set");
		}

		if (length > buf->maxsize)
			Com_Error(ERR_FATAL, "SZ_GetSpace: %i is > full buffer size", length);

		Com_Printf("SZ_GetSpace: overflow\n");
		SZ_Clear(buf);
		buf->overflowed = true;
	}

	void* data = buf->data + buf->cursize;
	buf->cursize += length;

	return data;
}

// Q2 counterpart
void SZ_Write(sizebuf_t* buf, const void* data, const int length)
{
	memcpy(SZ_GetSpace(buf, length), data, length);
}

// Q2 counterpart
void SZ_Print(sizebuf_t* buf, const char* data)
{
	const int len = (int)strlen(data) + 1;
	if (buf->cursize == 0 || buf->data[buf->cursize - 1])
		memcpy(SZ_GetSpace(buf, len), data, len); // No trailing 0.
	else
		memcpy((byte*)SZ_GetSpace(buf, len - 1) - 1, data, len); // Write over trailing 0.
}

#pragma endregion

#pragma region ========================== COMMAND LINE ARGS PROCESSING ====================

// Q2 counterpart
// Returns the position (1 to argc-1) in the program's argument list where the given parameter appears,
// or 0 if not present.
int COM_CheckParm(const char* parm)
{
	for (int i = 1; i < com_argc; i++)
		if (strcmp(parm, com_argv[i]) == 0)
			return i;

	return 0;
}

// Q2 counterpart
int COM_Argc(void)
{
	return com_argc;
}

// Q2 counterpart
char* COM_Argv(const int arg)
{
	if (arg >= 0 && arg < com_argc && com_argv[arg] != NULL)
		return com_argv[arg];

	return "";
}

// Q2 counterpart
void COM_ClearArgv(const int arg)
{
	if (arg >= 0 && arg < com_argc && com_argv[arg] != NULL)
		com_argv[arg] = "";
}

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

// Q2 counterpart
char* CopyString(const char* in)
{
	const int len = (int)strlen(in) + 1;
	char* out = Z_Malloc(len);
	strcpy_s(out, len, in); //mxd. strcpy -> strcpy_s

	return out;
}

// Q2 counterpart
void Info_Print(const char* s)
{
	char key[512];
	char value[512];

	if (*s == '\\')
		s++;

	while (*s != 0)
	{
		char* o = key;
		while (*s != 0 && *s != '\\')
			*o++ = *s++;

		const int l = o - key;
		if (l < 20)
		{
			memset(o, ' ', 20 - l);
			key[20] = 0;
		}
		else
		{
			*o = 0;
		}

		Com_Printf("%s", key);

		if (*s == 0)
		{
			Com_Printf("MISSING VALUE\n");
			return;
		}

		o = value;
		s++;
		while (*s != 0 && *s != '\\')
			*o++ = *s++;

		*o = 0;

		if (*s != 0)
			s++;

		Com_Printf("%s\n", value);
	}
}

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

// Q2 counterpart
void Z_Free(void* ptr)
{
	zhead_t* z = (zhead_t*)ptr - 1;

	if (z->magic != Z_MAGIC)
		Com_Error(ERR_FATAL, "Z_Free: bad magic");

	z->prev->next = z->next;
	z->next->prev = z->prev;

	z_count--;
	z_bytes -= z->size;
	free(z);
}

// Q2 counterpart
static void Z_Stats_f(void)
{
	Com_Printf("Zone memory : %i bytes in %i blocks\n", z_bytes, z_count);
}

// Q2 counterpart
void Z_FreeTags(const int tag)
{
	zhead_t* next;

	for (zhead_t* z = z_chain.next; z != &z_chain; z = next)
	{
		next = z->next;
		if (z->tag == tag)
			Z_Free(z + 1);
	}
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

// For proxy protecting.
byte COM_BlockSequenceCheckByte(const byte* base, int length, const int sequence)
{
	byte chkb[64];

	const byte* p = (byte*)bytedirs + (sequence % sizeof(bytedirs));

	length = min(60, length);
	memcpy(chkb, base, length);

	chkb[length + 0] = (byte)(p[0] ^ (byte)sequence);
	chkb[length + 1] = p[1];
	chkb[length + 2] = (byte)(p[2] ^ (byte)(sequence >> 8));
	chkb[length + 3] = p[3];

	return (byte)Com_BlockChecksum(chkb, length + 4);
}

// Q2 counterpart
// Just throw a fatal error to test error shutdown procedures.
H2R_NORETURN static void Com_Error_f(void)
{
	Com_Error(ERR_FATAL, "%s", Cmd_Argv(1));
}

void Qcommon_Init(const int argc, char** argv)
{
	if (setjmp(abortframe) != 0)
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

	Cbuf_AddText("exec default.cfg\n");
	Cbuf_AddText("exec config.cfg\n");

	Cbuf_AddEarlyCommands(true);
	Cbuf_Execute();

	// Init commands and vars.
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
	vid_maxfps = Cvar_Get("vid_maxfps", "60", CVAR_ARCHIVE); // YQ2

	// H2:
	fpu_precision = Cvar_Get("fpu_precision", "1", 0);
	//sys_copyfail = Cvar_Get("sys_copyfail", "You must have the Heretic II CD in the drive to play.", 0); //mxd. Relevant logic skipped
	hideconprint = Cvar_Get("hideconprint", "0", 0);
	player_dll = Cvar_Get("player_dll", "Player", 0);

	const char* s = va("%s:  %s %s %s", VERSIONFULL, CPUSTRING, __DATE__, BUILDSTRING);
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

	Com_ColourPrintf(P_HEADER, "\n==== "GAME_FULLNAME" initialized ====\n\n"); // Q2: Com_Printf //mxd. Use define.
}

void Qcommon_Frame(int usec) //mxd. msec -> usec.
{
	static int packetdelta = 1000000; // Time since last packetframe in microseconds.
	static int renderdelta = 1000000; // Time since last renderframe in microseconds.
	static int clienttimedelta = 0; // Accumulated time since last client run.
	static int servertimedelta = 0; // Accumulated time since last server run.

	uint control_word; //mxd

	if (setjmp(abortframe))
		return; // An ERR_DROP was thrown.

	// H2: set fpu precision (done in WinMain in Q2). //TODO: is this relevant? _controlfp logic is removed in YQ2.
	if (fpu_precision->value == 0.0f)
		_controlfp_s(&control_word, _PC_24, _MCW_PC);
	else if (fpu_precision->value == 1.0f)
		_controlfp_s(&control_word, _PC_53, _MCW_PC);
	else
		_controlfp_s(&control_word, _PC_64, _MCW_PC);

	if (log_stats->modified)
	{
		log_stats->modified = false;

		if (log_stats_file != NULL)
		{
			fclose(log_stats_file);
			log_stats_file = NULL;
		}

		if ((int)log_stats->value && fopen_s(&log_stats_file, "stats.log", "w") == 0) //mxd. fopen -> fopen_s
			fprintf(log_stats_file, "entities,dlights,parts,frame time\n");
	}

	if ((int)fixedtime->value)
		usec = (int)fixedtime->value;
	else if ((int)timescale->value)
		usec *= (int)timescale->value;

	if ((int)showtrace->value)
	{
		Com_Printf("%4i traces  %4i points\n", c_traces, c_pointcontents);
		c_traces = 0;
		c_brush_traces = 0;
		c_pointcontents = 0;
	}

	// We can render 1000 frames at maximum, because the minimum frametime of the client is 1 millisecond.
	if (vid_maxfps->value > 999 || vid_maxfps->value < 1)
		Cvar_SetValue("vid_maxfps", 999);

	if (cl_maxfps->value > 250)
		Cvar_SetValue("cl_maxfps", 250);

	const float rfps = vid_maxfps->value;
	const float pfps = min(cl_maxfps->value, rfps); // We can't have more packet frames than render frames, so limit pfps to rfps.

	// Calculate timings.
	packetdelta += usec;
	renderdelta += usec;
	clienttimedelta += usec;
	servertimedelta += usec;

	qboolean packetframe = true; // Runs the server and the client, but not the renderer. The minimal interval is about 10000 microseconds. If run more often the movement prediction in pmove.c breaks. That's the Q2 variant of the famous 125hz bug.
	qboolean renderframe = true; // Runs the renderer, but not the client or the server. The minimal interval is about 1000 microseconds.

	if (!(int)cl_timedemo->value && cl.cinematictime == 0) // H2: extra cl.cinematictime check.
	{
		// Render frames.
		if (renderdelta < (int)(1000000.0f / rfps))
			renderframe = false;

		// Network frames.
		const int packettargetdelta = (int)(1000000.0f / pfps);

		// "packetdelta + renderdelta/2 >= packettargetdelta" if now we're closer to when we want to run the next packetframe than we'd (probably) be after the next render frame.
		if (!renderframe || packetdelta + renderdelta / 2 < packettargetdelta)
			packetframe = false;
	}

	// Dedicated server terminal console.
	while (true)
	{
		char* s = Sys_ConsoleInput();
		if (s == NULL)
			break;

		Cbuf_AddText(va("%s\n", s));
	}

	// H2: no 'host_speeds' logic.
	Cbuf_Execute();

	// Run the server frame.
	if (packetframe)
	{
		SV_Frame(servertimedelta);
		servertimedelta = 0;
	}

	// Run the client frame.
	if (packetframe || renderframe)
	{
		CL_Frame(packetdelta, renderdelta, clienttimedelta, packetframe, renderframe);
		clienttimedelta = 0;

		// Reset deltas and mark frame.
		if (packetframe)
			packetdelta = 0;

		if (renderframe)
			renderdelta = 0;
	}
}