//
// qcommon.h -- definitions common between client and server, but not game.dll
//
// Copyright 1998 Raven Software
//

#pragma once

#include <setjmp.h>
#include "q_shared.h"
#include "Heretic2.h" //mxd. Moved to separate include, to pull only necessary stuff into launcher...

#define	BASEDIRNAME			"base"

#define VERSION_MAJOR		"1"
#define VERSION_MINOR		"06"
#define VERSION_LOCAL		"01"
#define VERSION_DATE		"0504"
#define VERSION_ITERATION	"01"

#define VERSIONDISP			"0.9 RC1" // Shown in console. //mxd. (VERSION_MAJOR"."VERSION_MINOR) in original logic.
#define VERSIONFULL			(VERSION_MAJOR"."VERSION_MINOR"."VERSION_LOCAL"."VERSION_DATE"."VERSION_ITERATION) // Stored in version cvar.

#define GAME_NAME			"Heretic2R" //mxd
#define GAME_FULLNAME		GAME_NAME" "VERSIONDISP //mxd

#ifdef _WIN32
	#ifdef NDEBUG
		#define BUILDSTRING "RELEASE"
	#else
		#define BUILDSTRING "DEBUG"
	#endif

	#ifdef _WIN64
		#define CPUSTRING	"x64"
	#else
		#define CPUSTRING	"x86"
	#endif
#endif

typedef struct sizebuf_s
{
	qboolean allowoverflow;	// If false, do a Com_Error.
	qboolean overflowed;	// Set to true if the buffer size failed.
	byte* data;
	int maxsize;
	int cursize;
	int readcount;
} sizebuf_t;

extern void SZ_Init(sizebuf_t* buf, byte* data, int length);
extern void SZ_Clear(sizebuf_t* buf);
extern void* SZ_GetSpace(sizebuf_t* buf, int length);
extern void SZ_Write(sizebuf_t* buf, const void* data, int length);
extern void SZ_Print(sizebuf_t* buf, const char* data); // strcats onto the sizebuf.

extern byte GetB(const byte* buf, int bit);
extern void SetB(byte* buf, int bit);

extern void MSG_WriteChar(sizebuf_t* sb, int c);
extern void MSG_WriteByte(sizebuf_t* sb, int c);
extern void MSG_WriteShort(sizebuf_t* sb, int c);
extern void MSG_WriteLong(sizebuf_t* sb, int c);
extern void MSG_WriteFloat(sizebuf_t* sb, float f);
extern void MSG_WriteString(sizebuf_t* sb, const char* s);
extern void MSG_WriteCoord(sizebuf_t* sb, float f);
extern void MSG_WritePos(sizebuf_t* sb, const vec3_t pos);
extern void MSG_WriteAngle(sizebuf_t* sb, float f);
extern void MSG_WriteAngle16(sizebuf_t* sb, float f);
extern void MSG_WriteDeltaUsercmd(sizebuf_t* sb, const struct usercmd_s* from, const struct usercmd_s* cmd);
extern void ParseEffectToSizeBuf(sizebuf_t* sb, const char* format, va_list marker);
extern void MSG_WriteEntityHeaderBits(sizebuf_t* msg, const byte* bf, byte* bfNonZero);
extern void MSG_WriteDeltaEntity(const struct entity_state_s* from, struct entity_state_s* to, sizebuf_t* msg, qboolean force);
extern void MSG_WriteDir(sizebuf_t* sb, const vec3_t dir);
extern void MSG_WriteDirMag(sizebuf_t* sb, const vec3_t dir);
extern void MSG_WriteYawPitch(sizebuf_t* sb, const vec3_t vector);
extern void MSG_WriteShortYawPitch(sizebuf_t* sb, const vec3_t vector);

extern void MSG_BeginReading(sizebuf_t* sb);

extern int MSG_ReadChar(sizebuf_t* sb);
extern int MSG_ReadByte(sizebuf_t* sb);
extern int MSG_ReadShort(sizebuf_t* sb);
extern int MSG_ReadLong(sizebuf_t* sb);
extern float MSG_ReadFloat(sizebuf_t* sb);
extern char* MSG_ReadString(sizebuf_t* sb);
extern char* MSG_ReadStringLine(sizebuf_t* sb);

extern float MSG_ReadCoord(sizebuf_t* sb);
extern void MSG_ReadPos(sizebuf_t* sb, vec3_t pos);
extern float MSG_ReadAngle(sizebuf_t* sb);
extern float MSG_ReadAngle16(sizebuf_t* sb);
extern void MSG_ReadDeltaUsercmd(sizebuf_t* sb, const struct usercmd_s* from, struct usercmd_s* move);

extern void MSG_ReadDir(sizebuf_t* sb, vec3_t dir);
extern void MSG_ReadDirMag(sizebuf_t* sb, vec3_t dir);
extern void MSG_ReadYawPitch(sizebuf_t* sb, vec3_t dir);
extern void MSG_ReadShortYawPitch(sizebuf_t* sb, vec3_t dir);

extern void MSG_ReadData(sizebuf_t* sb, void* data, int size);
extern void MSG_ReadJoints(sizebuf_t* sb, entity_state_t* ent);
extern void MSG_ReadEffects(sizebuf_t* sb, EffectsBuffer_t* fxBuf);

extern int COM_Argc(void);
extern char* COM_Argv(int arg); // Range and null checked.
extern void COM_ClearArgv(int arg);
extern int COM_CheckParm(const char* parm);

extern char* CopyString(const char* in);
extern void Info_Print(const char* s);

#pragma region ========================== PROTOCOL ==========================

// Communications protocols.

#define PROTOCOL_VERSION	51

#define PORT_MASTER			28900
#define PORT_CLIENT			28901
#define PORT_SERVER			28910

#define UPDATE_BACKUP		16 // Copies of entity_state_t to keep buffered. Must be power of two.
#define UPDATE_MASK			(UPDATE_BACKUP - 1)

// Server to client. The svc_strings[] array in cl_parse.c should mirror this.
enum svc_ops_e
{
	svc_bad,

	svc_layout,
	svc_inventory,
	svc_client_effect,

	// The rest are private to the client and server.
	svc_nop,
	svc_disconnect,
	svc_reconnect,
	svc_sound,					// <see code>
	svc_print,					// [byte] id [string] null terminated string.
	svc_gamemsg_print, 			// [short] id (top 3 bits flags).
	svc_stufftext,				// [string] stuffed into client's console buffer, should be \n terminated.
	svc_serverdata,				// [long] protocol ...
	svc_configstring,			// [short] [string].
	svc_spawnbaseline,
	svc_centerprint,			// [string] to put in center of the screen.
	svc_gamemsg_centerprint,  	// Line number of [string] in strings.txt file.
	svc_gamemsgvar_centerprint,	// Line number of [string] in strings.txt file, along with var to insert.
	svc_levelmsg_centerprint, 	// Line number of [string] in strings.txt file.
	svc_captionprint,			// Line number of [string] in strings.txt file.
	svc_obituary,				// Line number of [string] in strings.txt file.
	svc_download,				// [short] size [size bytes].
	svc_playerinfo,				// Variable.
	svc_packetentities,			// [...]
	svc_deltapacketentities,	// [...]
	svc_frame,
	svc_removeentities,
	svc_changeCDtrack,
	svc_framenum,				// Only sent on world spawn, before client effects get through, so we can ensure client time is right.
	svc_demo_client_effect,		// Only used to send down persistent effects at the start of a demo.
	svc_special_client_effect,	// Almost the same as svc_client_effect, except its got an extra size short at the top.
	svc_gamemsgdual_centerprint, // Send down two message numbers, to combine into one text string.
	svc_nameprint,				// Allow a client to print a message across the network without adding its name, instead, just its client number.
};

// Client to server.
enum clc_ops_e
{
	clc_bad,
	clc_nop,
	clc_move,		// [usercmd_t]
	clc_userinfo,	// [string] userinfo.
	clc_stringcmd,	// [string] message.
	clc_startdemo	// Start a demo - please send me all persistent effects.
};

// player_state_t communication delta flags. For every 8 bits of PLAYER_DEL_BYTES, a bit of PLAYER_DEL_NZBYTES is required.
#define PLAYER_DEL_BYTES		17
#define PLAYER_DELNZ_BYTES		3

#define PS_VIEWANGLES			0
#define PS_FRAMEINFO1			1 // (1,0) = sent one, upper = lower (1,1) = sent both, (0,0) = sent neither.
#define PS_FRAMEINFO2			2
#define PS_M_ORIGIN_XY			3
#define PS_M_ORIGIN_Z			4
#define PS_M_VELOCITY_XY		5
#define PS_M_VELOCITY_Z			6
#define PS_FWDVEL				7

#define PS_LOWERSEQ				8
#define PS_LOWERMOVE_INDEX		9
#define PS_AUTOTARGETENTITY		10
#define PS_GROUNDPLANE_INFO1	11 // (0,0) = zaxis, (1,0) = (0,0,0), (1,1) = all three sent.
#define PS_GROUNDPLANE_INFO2	12
#define PS_IDLETIME				13
#define PS_UPPERSEQ				14
#define PS_UPPERMOVE_INDEX		15

#define PS_M_TYPE				16
#define PS_M_TIME				17
#define PS_M_FLAGS				18
#define PS_W_FLAGS				19
#define PS_M_GRAVITY			20
#define PS_M_DELTA_ANGLES		21
#define PS_REMOTE_VIEWANGLES	22
#define PS_REMOTE_VIEWORIGIN	23

#define PS_REMOTE_ID			24
#define PS_VIEWHEIGHT			25
#define PS_OFFSETANGLES			26
#define PS_FOV					27
#define PS_RDFLAGS				28
#define PS_FOG_DENSITY			29
#define PS_MAP_PERCENTAGE		30
#define PS_MISSION1				31

#define PS_MISSION2				32
#define PS_MINSMAXS				33
#define PS_INVENTORY			34
#define PS_GROUNDBITS_NNGE		35
#define PS_GROUNDBITS_GC		36
#define PS_GROUNDBITS_SURFFLAGS 37
#define PS_WATERLEVEL			38
#define PS_WATERTYPE			39

#define PS_WATERHEIGHT			40
#define PS_GRABLOC0				41
#define PS_GRABLOC1				42
#define PS_GRABLOC2				43
#define PS_GRABANGLE			44
#define PS_SIDEVEL				45
#define PS_UPVEL				46
#define PS_FLAGS				47

#define PS_EDICTFLAGS			48
#define PS_UPPERIDLE			49
#define PS_LOWERIDLE			50
#define PS_WEAPON				51
#define PS_DEFENSE				52
#define PS_LASTWEAPON			53
#define PS_LASTDEFENSE			54
#define PS_WEAPONREADY			55

#define PS_SWITCHTOWEAPON		56
#define PS_NEWWEAPON			57
#define PS_WEAP_AMMO_INDEX		58
#define PS_DEF_AMMO_INDEX		59
#define PS_ARMORTYPE			60
#define PS_BOWTYPE				61
#define PS_STAFFLEVEL			62
#define PS_HELLTYPE				63

#define PS_HANDFXTYPE			64
#define PS_PLAGUELEVEL			65
#define PS_SKINTYPE				66
#define PS_ALTPARTS				67
#define PS_WEAPONCHARGE			68
#define PS_DEADFLAG				69
#define PS_IDEAL_YAW			70
#define PS_DMFLAGS				71

#define PS_OLDVELOCITY_Z		72
#define PS_STAT_BIT_0			73			// 1st of a contiguous block.
#define PS_STAT_BIT_47			(74 + 47)	// Rest of the block.
#define PS_CINEMATIC			122
#define PS_PIV					123
#define PS_METEORCOUNT			124
#define PS_M_CAMERA_DELTA_ANGLES 125
#define PS_POWERUP_TIMER		126
#define PS_QUICKTURN_RATE		127

#define PS_ADVANCEDSTAFF		128

// user_cmd_t communication delta flags. ms and light always sent, the others are optional.
#define CM_ANGLE1 				(1 << 0)
#define CM_ANGLE2 				(1 << 1)
#define CM_ANGLE3 				(1 << 2)
#define CM_AIMANGLE1 			(1 << 3)
#define CM_AIMANGLE2 			(1 << 4)
#define CM_AIMANGLE3 			(1 << 5)
#define CM_CAMERAVIEWORIGIN1	(1 << 6)
#define CM_CAMERAVIEWORIGIN2	(1 << 7)
#define CM_CAMERAVIEWORIGIN3	(1 << 8)
#define CM_CAMERAVIEWANGLES1	(1 << 9)
#define CM_CAMERAVIEWANGLES2	(1 << 10)
#define CM_CAMERAVIEWANGLES3	(1 << 11)
#define CM_FORWARD				(1 << 12)
#define CM_SIDE					(1 << 13)
#define CM_UP					(1 << 14)
#define CM_BUTTONS				(1 << 15)

// A sound without an ent or pos will be a local only sound.
#define SND_VOLUME				(1 << 0) // A byte.
#define SND_ATTENUATION			(1 << 1) // A byte.
#define SND_POS					(1 << 2) // Three coordinates.
#define SND_ENT					(1 << 3) // A short 0-2: channel, 3-12: entity.
#define SND_OFFSET				(1 << 4) // A byte, msec offset from frame start.
#define SND_PRED_INFO			(1 << 5) // A byte and a float.

#define DEFAULT_SOUND_PACKET_VOLUME			1.0f
#define DEFAULT_SOUND_PACKET_ATTENUATION	1.0f

// entity_state_t communication delta flags.
#define U_FRAME8			0
#define U_FRAME16			1
#define U_ORIGIN12			2
#define U_ORIGIN3			3
#define U_ANGLE1			4
#define U_ANGLE2			5
#define U_ANGLE3			6
#define U_SWAPFRAME			7

#define U_EFFECTS8			8
#define U_EFFECTS16			9
#define U_RENDERFX8			10
#define U_RENDERFX16		11
#define U_CLIENT_EFFECTS	12
#define U_FM_INFO			13
#define U_REMOVE			14
#define U_ENT_FREED			15

#define U_COLOR_R			16
#define U_COLOR_G			17
#define U_COLOR_B			18
#define U_COLOR_A			19
#define U_SKIN8				20
#define U_SKIN16			21
#define U_MODEL				22
#define U_SCALE				23

#define U_SOUND				24
#define U_SOLID				25
#define U_JOINTED			26
#define U_ABSLIGHT			27
#define U_OLDORIGIN			28
#define U_USAGE_COUNT		29
#define U_NUMBER16			30
#define U_BMODEL			31

#define U_CLIENTNUM			32

#define U_FM_HIGH			(1 << 7) // Means more then the first 7 updates.

#define U_FM_FRAME			(1 << 0) // Individual bits for each update.
#define U_FM_FRAME16		(1 << 1)
#define U_FM_COLOR_R		(1 << 2)
#define U_FM_COLOR_G		(1 << 3)
#define U_FM_COLOR_B		(1 << 4)
#define U_FM_COLOR_A		(1 << 5)
#define U_FM_FLAGS			(1 << 6)
#define U_FM_SKIN			(1 << 7)

#pragma endregion

#pragma region ========================== CMD ==========================

// Command text buffering and command execution.

// Any number of commands can be added in a frame, from several different sources.
// Most commands come from either keybindings or console line input, but remote
// servers can also send across commands and entire text files can be execed.
// The + command line options are also added to the command buffer.
// The game starts with a Cbuf_AddText("exec quake.rc\n"); Cbuf_Execute();

#define EXEC_NOW	0	// Don't return until completed.
#define EXEC_INSERT	1	// Insert at current position, but don't run yet.
#define EXEC_APPEND	2	// Add to end of the command buffer.

// Allocates an initial text buffer that will grow as needed.
extern void Cbuf_Init(void);

// As new commands are generated from the console or keybindings, the text is added to the end of the command buffer.
extern void Cbuf_AddText(const char* text);

// When a command wants to issue other commands immediately, the text is
// inserted at the beginning of the buffer, before any remaining unexecuted commands.
extern void Cbuf_InsertText(const char* text);

// Adds all the +set commands from the command line.
extern void Cbuf_AddEarlyCommands(qboolean clear);

// Adds all the remaining + commands from the command line.
// Returns true if any late commands were added, which will keep the demoloop from immediately starting.
extern qboolean Cbuf_AddLateCommands(void);

// Pulls off \n terminated lines of text from the command buffer and sends them through Cmd_ExecuteString.
// Stops when the buffer is empty. Normally called once per frame, but may be explicitly invoked.
// Do not call inside a command function!
extern void Cbuf_Execute(void);

// These two functions are used to defer any pending commands while a map is being loaded.
extern void Cbuf_CopyToDefer(void);
extern void Cbuf_InsertFromDefer(void);

//===========================================================================

// Command execution takes a null terminated string, breaks it into tokens,
// then searches for a command or variable that matches the first token.

typedef void (*xcommand_t)(void);

extern void Cmd_Init(void);

// Called by the init functions of other parts of the program to register commands and functions to call for them.
// The cmd_name is referenced later, so it should not be in temp memory.
// If function is NULL, the command will be forwarded to the server as a clc_stringcmd instead of executed locally.
Q2DLL_DECLSPEC extern void Cmd_AddCommand(const char* cmd_name, xcommand_t function);

Q2DLL_DECLSPEC extern void Cmd_RemoveCommand(const char* cmd_name);

// Attempts to match a partial command for automatic command line completion. Returns NULL if nothing fits.
extern const char* Cmd_CompleteCommand(const char* partial);

// Similar to above, but returns the next value after last.
extern const char* Cmd_CompleteCommandNext(const char* partial, const char* last);

// The functions that execute commands get their parameters with these functions.
// Cmd_Argv () will return an empty string, not a NULL if arg > argc, so string operations are always safe.
Q2DLL_DECLSPEC extern int Cmd_Argc(void);
Q2DLL_DECLSPEC extern char* Cmd_Argv(int arg);
extern char* Cmd_Args(void);

// Takes a null terminated string. Does not need to be /n terminated. Breaks the string up into arg tokens.
extern void Cmd_TokenizeString(char* text, qboolean macro_expand);

// Parses a single line of text into arguments and tries to execute it as if it was typed at the console.
extern void Cmd_ExecuteString(char* text);

// Adds the current command line as a clc_stringcmd to the client message.
// Things like godmode, noclip, etc, are commands directed to the server,
// so when they are typed in at the console, they will need to be forwarded.
extern void Cmd_ForwardToServer(void);

#pragma endregion

#pragma region ========================== CVAR ==========================

// cvar_t variables are used to hold scalar or string variables that can be changed or displayed at the console or prog code
// as well as accessed directly in C code.

// The user can access cvars from the console in three ways:
// r_draworder			Prints the current value.
// r_draworder 0		Sets the current value to 0.
// set r_draworder 0	Same as above, but creates the cvar if not present.

// Cvars are restricted from having the same names as commands to keep this interface from being ambiguous.

extern cvar_t* cvar_vars;

//mxd. Cvar_Get() defined in q_shared.h

// Will create the variable if it doesn't exist.
Q2DLL_DECLSPEC extern cvar_t* Cvar_Set(const char* var_name, const char* value);

// Will set the variable even if NOSET or LATCH.
extern cvar_t* Cvar_ForceSet(const char* var_name, const char* value);

extern cvar_t* Cvar_FullSet(const char* var_name, const char* value, int flags);

// Expands value to a string and calls Cvar_Set.
Q2DLL_DECLSPEC extern void Cvar_SetValue(const char* var_name, float value);

// Returns 0 if not defined or non numeric.
extern float Cvar_VariableValue(const char* var_name);

// Returns an empty string if not defined.
extern char* Cvar_VariableString(const char* var_name);

// Attempts to match a partial variable name for command line completion returns NULL if nothing fits.
extern const char* Cvar_CompleteVariable(const char* partial);

// Similar to above, except that it goes to next match if any.
extern const char* Cvar_CompleteVariableNext(const char* partial, const char* last);

// Any CVAR_LATCHED variables that have been set will now take effect.
extern void Cvar_GetLatchedVars(void);

// Called by Cmd_ExecuteString when Cmd_Argv(0) doesn't match a known command.
// Returns true if the command was a variable reference that was handled (print or change).
extern qboolean Cvar_Command(void);

// Appends lines containing "set variable value" for all variables with the archive flag set to true.
extern void Cvar_WriteVariables(FILE* f); //mxd. const char* path -> FILE* f.

extern void Cvar_Init(void);

// Returns an info string containing all the CVAR_USERINFO cvars.
extern char* Cvar_Userinfo(void);

// Returns an info string containing all the CVAR_SERVERINFO cvars.
extern char* Cvar_Serverinfo(void);

// This is set each time a CVAR_USERINFO variable is changed so that the client knows to send it to the server.
extern qboolean userinfo_modified;

// Screen flash set.
extern void Activate_Screen_Flash(int color);

// Screen flash unset.
extern void Deactivate_Screen_Flash(void);

// Return screen flash value.
extern int Is_Screen_Flashing(void);

// Set up screen shaking.
extern void Activate_Screen_Shake(float intensity, float duration, float current_time, int flags);

// Reset screen shake.
extern void Reset_Screen_Shake(void);

// Called by the camera code to determine our camera offset.
extern void Perform_Screen_Shake(vec3_t out, float current_time);

#pragma endregion

#pragma region ========================== NET ==========================
// Quake's interface to the networking layer.

#define	PORT_ANY		(-1)

// FIXME: this really shouldn't have to be changed to 2000 but some maps (SSTOWN4) were causing it to crash.
#define MAX_MSGLEN		2500	// Max length of a message. //mxd. 1400 in Q2
#define PACKET_HEADER	10		// Two ints and a short.

typedef enum
{
	NA_LOOPBACK,
	NA_BROADCAST,
	NA_IP,
	NA_IPX,
	NA_BROADCAST_IPX
} netadrtype_t;

typedef enum
{
	NS_CLIENT,
	NS_SERVER,
	NS_GAMESPY //TODO: remove
} netsrc_t;

// H2: net debugging settings.
extern cvar_t* net_sendrate;
extern cvar_t* net_receiverate;
extern cvar_t* net_latency;

typedef struct
{
	netadrtype_t type;
	byte ip[4];
	byte ipx[10];
	ushort port;
} netadr_t;

extern void NET_Init(void);
extern void NET_Shutdown(void);
extern void NET_Config(qboolean multiplayer);

extern qboolean NET_GetPacket(netsrc_t sock, netadr_t* n_from, sizebuf_t* n_message);
extern void NET_SendPacket(netsrc_t sock, int length, const void* data, const netadr_t* to); //mxd. Changed 'to' type to netadr_t*.

extern qboolean NET_CompareAdr(const netadr_t* a, const netadr_t* b); //mxd. Changed args type to netadr_t*.
extern qboolean NET_CompareBaseAdr(const netadr_t* a, const netadr_t* b); //mxd. Changed args type to netadr_t*.
extern qboolean NET_IsLocalAddress(const netadr_t* a); //mxd. Changed arg type to netadr_t*.
extern char* NET_AdrToString(const netadr_t* a); //mxd. Changed arg type to netadr_t*.
extern qboolean NET_StringToAdr(const char* s, netadr_t* a);

//============================================================================

typedef struct
{
	qboolean fatal_error;

	netsrc_t sock;

	int dropped;	// Between last packet and previous.

	int last_received;	// For timeouts.
	int last_sent;		// For retransmits.

	netadr_t remote_address;
	int qport;	// qport value to write when transmitting.

	// Sequencing variables.
	int incoming_sequence;
	int incoming_acknowledged;
	int incoming_reliable_acknowledged;	// Single bit.

	int incoming_reliable_sequence;		// Single bit, maintained local.

	int outgoing_sequence;
	int reliable_sequence;		// Single bit.
	int last_reliable_sequence;	// Sequence number of last send.

	// Reliable staging and holding areas.
	sizebuf_t message;					// Writing buffer to send to server.
	byte message_buf[MAX_MSGLEN - 16];	// Leave space for header.

	// Message is copied to this buffer when it is first transferred.
	int reliable_length;
	byte reliable_buf[MAX_MSGLEN - 16];	// Unpacked reliable message.
} netchan_t;

extern netadr_t net_from;
extern sizebuf_t net_message;
extern byte net_message_buffer[MAX_MSGLEN];

extern void Netchan_Init(void);
extern void Netchan_Setup(netsrc_t sock, netchan_t* chan, const netadr_t* adr, int port); //mxd. Changed 'adr' arg type to netadr_t*.

extern qboolean Netchan_NeedReliable(const netchan_t* chan);
extern int Netchan_Transmit(netchan_t* chan, int length, const byte* data); // Q2: void return type.
extern void Netchan_OutOfBandPrint(int net_socket, const netadr_t* adr, const char* format, ...); //mxd. Changed 'adr' arg type to netadr_t*.
extern qboolean Netchan_Process(netchan_t* chan, sizebuf_t* msg);

#pragma endregion

#pragma region ========================== PLAYER MOVEMENT CODE ==========================

// Common between server and client so prediction matches.
extern void Pmove(pmove_t* pmove, qboolean server);

#pragma endregion

#pragma region ========================== FILESYSTEM ==========================

extern cvar_t * fs_gamedirvar; //mxd
extern qboolean file_from_pak; //mxd

extern void FS_InitFilesystem(void);
extern char* FS_GetPath(const char* name); // H2
extern void FS_SetGamedir(const char* dir);
extern char* FS_Gamedir(void);
extern char* FS_Userdir(void);
extern char* FS_NextPath(const char* prevpath);
extern void FS_ExecAutoexec(void);

Q2DLL_DECLSPEC extern int FS_FOpenFile(const char* filename, FILE** file);
Q2DLL_DECLSPEC extern void FS_FCloseFile(FILE* f); // Note: this can't be called from another DLL, due to MS libc issues.

// A null buffer will just return the file length without loading. A -1 length is not present.
Q2DLL_DECLSPEC extern int FS_LoadFile(const char* path, void** buffer);

// Properly handles partial reads.
extern void FS_Read(void* buffer, int len, FILE* file);
extern int FS_FileLength(FILE* f); //mxd. Made public.

Q2DLL_DECLSPEC extern void FS_FreeFile(void* buffer);
extern void FS_CreatePath(char* path);

#pragma endregion

#pragma region ========================== MISC ==========================

#define CFX_CULLING_DIST 1000.0f

extern void Com_BeginRedirect(int target, char* buffer, int buffersize, void (*flush)(int, char*));
extern void Com_EndRedirect(void);
Q2DLL_DECLSPEC extern void Com_DPrintf(const char* fmt, ...);
H2R_NORETURN Q2DLL_DECLSPEC extern void Com_Error(int code, const char* fmt, ...);
H2R_NORETURN extern void Com_Quit(void);
extern int Com_ServerState(void);
extern void Com_SetServerState(int state);

extern uint Com_BlockChecksum(void* buffer, int length);
extern byte COM_BlockSequenceCheckByte(const byte* base, int length, int sequence);

extern cvar_t* developer;
extern cvar_t* dedicated;
extern cvar_t* host_speeds;
extern cvar_t* log_stats;
extern cvar_t* player_dll;

extern cvar_t* allow_download;
extern cvar_t* allow_download_maps;
extern cvar_t* allow_download_players;
extern cvar_t* allow_download_models;
extern cvar_t* allow_download_sounds;

extern FILE* log_stats_file;

Q2DLL_DECLSPEC extern void Z_Free(void* ptr);
Q2DLL_DECLSPEC extern void* Z_Malloc(int size); // Returns 0-filled memory.
extern void* Z_TagMalloc(int size, int tag);
extern void Z_FreeTags(int tag);

extern void Qcommon_Init(int argc, char** argv);
extern void Qcommon_Frame(int usec);

#pragma endregion

#pragma region ========================== NON-PORTABLE SYSTEM SERVICES ==========================

extern void Sys_Init(void);

extern char* Sys_ConsoleInput(void);
extern void Sys_ConsoleOutput(const char* string);
H2R_NORETURN extern void Sys_Quit(void);

#pragma endregion

#pragma region ========================== CLIENT / SERVER SYSTEMS ==========================

extern jmp_buf abortframe; //mxd

extern void CL_Init(void);
extern void CL_Drop(void);
extern void CL_Shutdown(void);
extern void CL_Frame(int packetdelta, int renderdelta, int timedelta, qboolean packetframe, qboolean renderframe); // YQ2: extra args

extern void SV_Init(void);
extern void SV_Shutdown(const char* finalmsg, qboolean reconnect);
extern void SV_Frame(int usec);

#pragma endregion