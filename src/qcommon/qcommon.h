#ifndef QCOMMON_H
#define QCOMMON_H

// qcommon.h -- definitions common between client and server, but not game.dll

#include "../game/q_shared.h"


#define	BASEDIRNAME	"base"

#define VERSION_MAJOR		"1"
#define VERSION_MINOR		"06"
#define VERSION_LOCAL		"01"
#define VERSION_DATE		"0504"
#define VERSION_ITERATION	"01"

#define VERSIONDISP		(VERSION_MAJOR"."VERSION_MINOR)
#define VERSIONFULL		(VERSION_MAJOR"."VERSION_MINOR"."VERSION_LOCAL"."VERSION_DATE"."VERSION_ITERATION)

#ifdef _HERETIC2_
#define GAME_DECLSPEC    __declspec(dllexport)
#else 
#define GAME_DECLSPEC    __declspec(dllimport)
#endif

#define NO_BLOOD 0


#ifdef WIN32

#ifdef NDEBUG
#define BUILDSTRING "RELEASE"
#else
#define BUILDSTRING "DEBUG"
#endif

#ifdef _M_IX86
#define	CPUSTRING	"x86"
#endif

#endif

//============================================================================

typedef struct sizebuf_s
{
	qboolean	allowoverflow;	// if false, do a Com_Error
	qboolean	overflowed;		// set to true if the buffer size failed
	byte	*data;
	int		maxsize;
	int		cursize;
	int		readcount;
} sizebuf_t;

void SZ_Init (sizebuf_t *buf, byte *data, int length);
void SZ_Clear (sizebuf_t *buf);
void *SZ_GetSpace (sizebuf_t *buf, int length);
void SZ_Write (sizebuf_t *buf, void *data, int length);
void SZ_Print (sizebuf_t *buf, char *data);	// strcats onto the sizebuf

//============================================================================

struct usercmd_s;
struct entity_state_s;

unsigned char GetB(unsigned char * buf, int i);
void SetB(unsigned char * buf, int i);

void MSG_WriteChar (sizebuf_t *sb, int c);
void MSG_WriteByte (sizebuf_t *sb, int c);
void MSG_WriteShort (sizebuf_t *sb, int c);
void MSG_WriteLong (sizebuf_t *sb, int c);
void MSG_WriteFloat (sizebuf_t *sb, float f);
void MSG_WriteString (sizebuf_t *sb, char *s);
void MSG_WriteCoord (sizebuf_t *sb, float f);
void MSG_WritePos (sizebuf_t *sb, vec3_t pos);
void MSG_WriteAngle (sizebuf_t *sb, float f);
void MSG_WriteAngle16 (sizebuf_t *sb, float f);
void MSG_WriteDeltaUsercmd (sizebuf_t *sb, struct usercmd_s *from, struct usercmd_s *cmd);
void ParseEffectToSizeBuf(sizebuf_t *sizebuf, char *format, va_list marker);
void MSG_WriteEntityHeaderBits(sizebuf_t *msg, unsigned char *bf, unsigned char *bfNonZero);
void MSG_WriteDeltaEntity (struct entity_state_s *from, struct entity_state_s *to
						   , sizebuf_t *msg, qboolean force);
void MSG_WriteDir (sizebuf_t *sb, vec3_t vector);
void MSG_WriteDirMag (sizebuf_t *sb, vec3_t dir);
void MSG_WriteYawPitch (sizebuf_t *sb, vec3_t vector);
void MSG_WriteShortYawPitch (sizebuf_t *sb, vec3_t vector);

void	MSG_BeginReading (sizebuf_t *sb);

int		MSG_ReadChar (sizebuf_t *sb);
int		MSG_ReadByte (sizebuf_t *sb);
int		MSG_ReadShort (sizebuf_t *sb);
int		MSG_ReadLong (sizebuf_t *sb);
float	MSG_ReadFloat (sizebuf_t *sb);
char	*MSG_ReadString (sizebuf_t *sb);
char	*MSG_ReadStringLine (sizebuf_t *sb);

float	MSG_ReadCoord (sizebuf_t *sb);
void	MSG_ReadPos (sizebuf_t *sb, vec3_t pos);
float	MSG_ReadAngle (sizebuf_t *sb);
float	MSG_ReadAngle16 (sizebuf_t *sb);
void	MSG_ReadDeltaUsercmd (sizebuf_t *sb, struct usercmd_s *from, struct usercmd_s *cmd);

void	MSG_ReadDir (sizebuf_t *sb, vec3_t vector);
void	MSG_ReadDirMag (sizebuf_t *sb, vec3_t dir);
void	MSG_ReadYawPitch (sizebuf_t *sb, vec3_t vector);
void	MSG_ReadShortYawPitch (sizebuf_t *sb, vec3_t vector);

void	MSG_ReadData (sizebuf_t *sb, void *buffer, int size);
void	MSG_ReadJoints(sizebuf_t *msg_read, entity_state_t *ent);
void	MSG_ReadEffects(sizebuf_t *msg_read, EffectsBuffer_t *fxBuf);

//============================================================================

extern	qboolean		bigendien;
extern int		sz_line;
extern char		*sz_filename;
#define set_sz_data	sz_filename = __FILE__; sz_line = __LINE__; 

//============================================================================


int	COM_Argc (void);
char *COM_Argv (int arg);	// range and null checked
void COM_ClearArgv (int arg);
int COM_CheckParm (char *parm);
void COM_AddParm (char *parm);

void COM_Init (void);
void COM_InitArgv (int argc, char **argv);

char *CopyString (char *in);

//============================================================================

void Info_Print (char *s);


/* crc.h */

void CRC_Init(unsigned short *crcvalue);
void CRC_ProcessByte(unsigned short *crcvalue, byte data);
unsigned short CRC_Value(unsigned short crcvalue);
int CRC_Block (byte *start, int count);




/*
==============================================================

PROTOCOL

==============================================================
*/

// protocol.h -- communications protocols

#define	PROTOCOL_VERSION	51

//=========================================

#define	PORT_MASTER			28900
#define	PORT_CLIENT			28901
#define	PORT_SERVER			28910
#define	PORT_GAMESPY		28911	// port that gamespy will communicate to our server on
#define	PORT_GAMESPYMASTER	27900	// port we communicate to gamespy's master server

//=========================================

#define	UPDATE_BACKUP		16	// copies of entity_state_t to keep buffered
								// must be power of two
#define	UPDATE_MASK			(UPDATE_BACKUP-1)

#define MAX_PACKET_ENTITIES	64


//==================
// the svc_strings[] array in cl_parse.c should mirror this
//==================

//
// server to client
//
enum svc_ops_e
{
	svc_bad,

	svc_layout,
	svc_inventory,
	svc_client_effect,

	// the rest are private to the client and server
	svc_nop,
	svc_disconnect,
	svc_reconnect,
	svc_sound,					// <see code>
	svc_print,					// [byte] id [string] null terminated string
	svc_gamemsg_print, 			// [short] id (top 3 bits flags)
	svc_stufftext,				// [string] stuffed into client's console buffer, should be \n terminated
	svc_serverdata,				// [long] protocol ...
	svc_configstring,			// [short] [string]
	svc_spawnbaseline,		
	svc_centerprint,			// [string] to put in center of the screen
	svc_gamemsg_centerprint,  	// line number of [string] in strings.txt file
	svc_gamemsgvar_centerprint,	// line number of [string] in strings.txt file, along with var to insert
	svc_levelmsg_centerprint, 	// line number of [string] in strings.txt file
	svc_captionprint,			// line number of [string] in strings.txt file
	svc_obituary,			// line number of [string] in strings.txt file
	svc_download,				// [short] size [size bytes]
	svc_playerinfo,				// variable
	svc_packetentities,			// [...]
	svc_deltapacketentities,	// [...]
	svc_frame,
	svc_removeentities,
	svc_changeCDtrack,
	svc_framenum,				//only sent on world spawn, before client effects get through, so we can ensure client time is right
	svc_demo_client_effect,		//only used to send down persistant effects at the start of a demo
	svc_special_client_effect,	//almost the same as svc_client_effect, except its got an extra size short at the top.
	svc_gamemsgdual_centerprint, //send down two message numbers, to combine into one text string
	svc_nameprint,				//allow a client to print a message across the network without adding its name, instead, just its client number
};

//==============================================

//
// client to server
//
enum clc_ops_e
{
	clc_bad,
	clc_nop, 		
	clc_move,				// [[usercmd_t]
	clc_userinfo,			// [[userinfo string]
	clc_stringcmd,			// [string] message
	clc_startdemo			// start a demo - please send me all persistant effects
};

//==============================================

// player_state_t communication delta flags.

// For every 8 bits of PLAYER_DEL_BYTES, a bit of PLAYER_DEL_NZBYTES is required.

#define PLAYER_DEL_BYTES		(17)
#define PLAYER_DELNZ_BYTES		(3)

#define	PS_VIEWANGLES			(0)
#define PS_FRAMEINFO1			(1) // (1,0)=sent one, upper=lower (1,1)=sent both, (0,0)=sent neither
#define PS_FRAMEINFO2			(2)
#define	PS_M_ORIGIN_XY			(3)
#define	PS_M_ORIGIN_Z			(4)
#define	PS_M_VELOCITY_XY		(5)
#define	PS_M_VELOCITY_Z			(6)
#define PS_FWDVEL				(7)

#define PS_LOWERSEQ				(8)
#define PS_LOWERMOVE_INDEX		(9)
#define PS_AUTOTARGETENTITY		(10)
#define PS_GROUNDPLANE_INFO1	(11) // (0,0) = zaxis, (1,0)=(0,0,0),(1,1)=all three sent
#define PS_GROUNDPLANE_INFO2	(12)
#define PS_IDLETIME				(13)
#define PS_UPPERSEQ				(14)
#define PS_UPPERMOVE_INDEX		(15)

#define	PS_M_TYPE				(16)
#define	PS_M_TIME				(17)
#define	PS_M_FLAGS				(18)
#define	PS_W_FLAGS				(19)
#define	PS_M_GRAVITY			(20)
#define	PS_M_DELTA_ANGLES		(21)
#define	PS_REMOTE_VIEWANGLES	(22)
#define	PS_REMOTE_VIEWORIGIN	(23)

#define PS_REMOTE_ID			(24)
#define PS_VIEWHEIGHT			(25)
#define	PS_OFFSETANGLES			(26)
#define	PS_FOV					(27)
#define	PS_RDFLAGS				(28)
#define PS_FOG_DENSITY			(29)
#define PS_MAP_PERCENTAGE		(30)
#define PS_MISSION1				(31)

#define PS_MISSION2				(32)
#define PS_MINSMAXS				(33)
#define PS_INVENTORY			(34)
#define PS_GROUNDBITS_NNGE		(35)
#define PS_GROUNDBITS_GC		(36)
#define PS_GROUNDBITS_SURFFLAGS (37)
#define PS_WATERLEVEL			(38)
#define PS_WATERTYPE			(39)

#define PS_WATERHEIGHT			(40)
#define PS_GRABLOC0				(41)
#define PS_GRABLOC1				(42)
#define PS_GRABLOC2				(43)
#define PS_GRABANGLE			(44)
#define PS_SIDEVEL				(45)
#define PS_UPVEL				(46)
#define PS_FLAGS				(47)

#define PS_EDICTFLAGS			(48)
#define PS_UPPERIDLE			(49)
#define PS_LOWERIDLE			(50)
#define PS_WEAPON				(51)
#define PS_DEFENSE				(52)
#define PS_LASTWEAPON			(53)
#define PS_LASTDEFENSE			(54)	
#define PS_WEAPONREADY			(55)

#define PS_SWITCHTOWEAPON		(56)
#define PS_NEWWEAPON			(57)
#define PS_WEAP_AMMO_INDEX		(58)
#define PS_DEF_AMMO_INDEX		(59)
#define PS_ARMORTYPE			(60)
#define PS_BOWTYPE				(61)
#define PS_STAFFLEVEL			(62)
#define PS_HELLTYPE				(63)

#define PS_HANDFXTYPE			(64)
#define PS_PLAGUELEVEL			(65)
#define PS_SKINTYPE				(66)
#define PS_ALTPARTS				(67)
#define PS_WEAPONCHARGE			(68)
#define PS_DEADFLAG				(69)
#define PS_IDEAL_YAW			(70)
#define PS_DMFLAGS				(71)

#define PS_OLDVELOCITY_Z		(72)
#define PS_STAT_BIT_0			(73)	// 1st of a contiguous block.
#define PS_STAT_BIT_47			(74+47)	// Rest of the block.
#define PS_CINEMATIC			(122)
#define PS_PIV					(123)
#define PS_METEORCOUNT			(124)
#define	PS_M_CAMERA_DELTA_ANGLES (125)
#define PS_POWERUP_TIMER		(126)
#define PS_QUICKTURN_RATE		(127)

#define PS_ADVANCEDSTAFF		(128)

//==============================================

// user_cmd_t communication delta flags.

// ms and light allways sent, the others are optional

#define	CM_ANGLE1 				(1<<0)
#define	CM_ANGLE2 				(1<<1)
#define	CM_ANGLE3 				(1<<2)
#define	CM_AIMANGLE1 			(1<<3)
#define	CM_AIMANGLE2 			(1<<4)
#define	CM_AIMANGLE3 			(1<<5)
#define	CM_CAMERAVIEWORIGIN1	(1<<6)
#define	CM_CAMERAVIEWORIGIN2	(1<<7)
#define	CM_CAMERAVIEWORIGIN3	(1<<8)
#define	CM_CAMERAVIEWANGLES1	(1<<9)
#define	CM_CAMERAVIEWANGLES2	(1<<10)
#define	CM_CAMERAVIEWANGLES3	(1<<11)
#define	CM_FORWARD				(1<<12)
#define	CM_SIDE					(1<<13)
#define	CM_UP					(1<<14)
#define	CM_BUTTONS				(1<<15)

//==============================================

// a sound without an ent or pos will be a local only sound
#define	SND_VOLUME		(1<<0)		// a byte
#define	SND_ATTENUATION	(1<<1)		// a byte
#define	SND_POS			(1<<2)		// three coordinates
#define	SND_ENT			(1<<3)		// a short 0-2: channel, 3-12: entity
#define	SND_OFFSET		(1<<4)		// a byte, msec offset from frame start
#define SND_PRED_INFO	(1<<5)		// a byte and a float.

#define DEFAULT_SOUND_PACKET_VOLUME	1.0
#define DEFAULT_SOUND_PACKET_ATTENUATION 1.0

//==============================================

// entity_state_t communication delta flags

// For every 8 bits of PLAYER_DEL_BYTES, a bit of PLAYER_DEL_NZBYTES is required.

#define ENT_DEL_BYTES		(5)
#define ENT_DELNZ_BYTES		(1)

#define	U_FRAME8			(0)
#define	U_FRAME16			(1)
#define	U_ORIGIN12			(2)
#define	U_ORIGIN3			(3)
#define	U_ANGLE1			(4)
#define	U_ANGLE2			(5)
#define	U_ANGLE3			(6)
#define U_SWAPFRAME			(7)

#define	U_EFFECTS8			(8)
#define	U_EFFECTS16			(9)
#define U_RENDERFX8			(10)
#define	U_RENDERFX16		(11)
#define	U_CLIENT_EFFECTS	(12)
#define U_FM_INFO			(13)
#define	U_REMOVE			(14)
#define	U_ENT_FREED			(15)

#define	U_COLOR_R			(16)
#define	U_COLOR_G			(17)
#define	U_COLOR_B			(18)
#define	U_COLOR_A			(19)
#define	U_SKIN8				(20)
#define	U_SKIN16			(21)
#define	U_MODEL				(22)
#define U_SCALE				(23)

#define	U_SOUND				(24)
#define	U_SOLID				(25)
#define U_JOINTED			(26)
#define	U_ABSLIGHT			(27)
#define	U_OLDORIGIN			(28)
#define U_USAGE_COUNT		(29)
#define	U_NUMBER16			(30)
#define	U_BMODEL			(31)

#define U_CLIENTNUM			(32)

#define U_FM_HIGH			(1<<7)					// Means more then the first 7 updates

#define U_FM_FRAME			(1<<0)					// Individual bits for each update
#define U_FM_FRAME16		(1<<1)
#define U_FM_COLOR_R		(1<<2)
#define U_FM_COLOR_G		(1<<3)
#define U_FM_COLOR_B		(1<<4)
#define U_FM_COLOR_A		(1<<5)
#define U_FM_FLAGS			(1<<6)
#define U_FM_SKIN			(1<<7)

/*
==============================================================

CMD

Command text buffering and command execution

==============================================================
*/

/*

Any number of commands can be added in a frame, from several different sources.
Most commands come from either keybindings or console line input, but remote
servers can also send across commands and entire text files can be execed.

The + command line options are also added to the command buffer.

The game starts with a Cbuf_AddText ("exec quake.rc\n"); Cbuf_Execute ();

*/

#define	EXEC_NOW	0		// don't return until completed
#define	EXEC_INSERT	1		// insert at current position, but don't run yet
#define	EXEC_APPEND	2		// add to end of the command buffer

void Cbuf_Init (void);
// allocates an initial text buffer that will grow as needed

void Cbuf_AddText (char *text);
// as new commands are generated from the console or keybindings,
// the text is added to the end of the command buffer.

void Cbuf_InsertText (char *text);
// when a command wants to issue other commands immediately, the text is
// inserted at the beginning of the buffer, before any remaining unexecuted
// commands.

void Cbuf_ExecuteText (int exec_when, char *text);
// this can be used in place of either Cbuf_AddText or Cbuf_InsertText

void Cbuf_AddEarlyCommands (qboolean clear);
// adds all the +set commands from the command line

qboolean Cbuf_AddLateCommands (void);
// adds all the remaining + commands from the command line
// Returns true if any late commands were added, which
// will keep the demoloop from immediately starting

void Cbuf_Execute (void);
// Pulls off \n terminated lines of text from the command buffer and sends
// them through Cmd_ExecuteString.  Stops when the buffer is empty.
// Normally called once per frame, but may be explicitly invoked.
// Do not call inside a command function!

void Cbuf_CopyToDefer (void);
void Cbuf_InsertFromDefer (void);
// These two functions are used to defer any pending commands while a map
// is being loaded

//===========================================================================

/*

Command execution takes a null terminated string, breaks it into tokens,
then searches for a command or variable that matches the first token.

*/

typedef void (*xcommand_t) (void);

void	Cmd_Init (void);

void	Cmd_AddCommand (char *cmd_name, xcommand_t function);
// called by the init functions of other parts of the program to
// register commands and functions to call for them.
// The cmd_name is referenced later, so it should not be in temp memory
// if function is NULL, the command will be forwarded to the server
// as a clc_stringcmd instead of executed locally
void	Cmd_RemoveCommand (char *cmd_name);

qboolean Cmd_Exists (char *cmd_name);
// used by the cvar code to check for cvar / command name overlap

char 	*Cmd_CompleteCommand (char *partial);
// attempts to match a partial command for automatic command line completion
// returns NULL if nothing fits

char 	*Cmd_CompleteCommandNext (char *partial, char *last);
// similar to above, but returns the next value after last

int		Cmd_Argc (void);
char	*Cmd_Argv (int arg);
char	*Cmd_Args (void);
// The functions that execute commands get their parameters with these
// functions. Cmd_Argv () will return an empty string, not a NULL
// if arg > argc, so string operations are allways safe.

void	Cmd_TokenizeString (char *text, qboolean macroExpand);
// Takes a null terminated string.  Does not need to be /n terminated.
// breaks the string up into arg tokens.

void	Cmd_ExecuteString (char *text);
// Parses a single line of text into arguments and tries to execute it
// as if it was typed at the console

void	Cmd_ForwardToServer (void);
// adds the current command line as a clc_stringcmd to the client message.
// things like godmode, noclip, etc, are commands directed to the server,
// so when they are typed in at the console, they will need to be forwarded.


/*
==============================================================

CVAR

==============================================================
*/

/*

cvar_t variables are used to hold scalar or string variables that can be changed or displayed at the console or prog code as well as accessed directly
in C code.

The user can access cvars from the console in three ways:
r_draworder			prints the current value
r_draworder 0		sets the current value to 0
set r_draworder 0	as above, but creates the cvar if not present
Cvars are restricted from having the same names as commands to keep this
interface from being ambiguous.
*/

extern	cvar_t	*cvar_vars;

float ClampCvar( float min, float max, float value );

cvar_t *Cvar_Get (char *var_name, char *value, int flags);
// creates the variable if it doesn't exist, or returns the existing one
// if it exists, the value will not be changed, but flags will be ORed in
// that allows variables to be unarchived without needing bitflags

cvar_t 	*Cvar_Set (char *var_name, char *value);
// will create the variable if it doesn't exist

cvar_t *Cvar_ForceSet (char *var_name, char *value);
// will set the variable even if NOSET or LATCH

cvar_t 	*Cvar_FullSet (char *var_name, char *value, int flags);

void	Cvar_SetValue (char *var_name, float value);
// expands value to a string and calls Cvar_Set

float	Cvar_VariableValue (char *var_name);
// returns 0 if not defined or non numeric

char	*Cvar_VariableString (char *var_name);
// returns an empty string if not defined

char 	*Cvar_CompleteVariable (char *partial);
// attempts to match a partial variable name for command line completion
// returns NULL if nothing fits

char	*Cvar_CompleteVariableNext(char *partial, char *last);
// similar to above, except that it goes to next match if any

void	Cvar_GetLatchedVars (void);
// any CVAR_LATCHED variables that have been set will now take effect

qboolean Cvar_Command (void);
// called by Cmd_ExecuteString when Cmd_Argv(0) doesn't match a known
// command.  Returns true if the command was a variable reference that
// was handled. (print or change)

void 	Cvar_WriteVariables (char *path);
// appends lines containing "set variable value" for all variables
// with the archive flag set to true.

void	Cvar_Init (void);

char	*Cvar_Userinfo (void);
// returns an info string containing all the CVAR_USERINFO cvars

char	*Cvar_Serverinfo (void);
// returns an info string containing all the CVAR_SERVERINFO cvars

extern	qboolean	userinfo_modified;
// this is set each time a CVAR_USERINFO variable is changed
// so that the client knows to send it to the server

// Screen flash set
void Activate_Screen_Flash(int color);

// Screen flash unset
void Deactivate_Screen_Flash(void);

// return screen flash value
int Is_Screen_Flashing(void);  

// set up a screen shaking
void Activate_Screen_Shake(float intensity, float duration, float current_time, int flags);
// reset screen shakings
void Reset_Screen_Shake(void);

qboolean Get_Crosshair(vec3_t origin, byte *type);

// called by the camera code to determine our camera offset
void Perform_Screen_Shake(vec3_t, float current_time);

/*
==============================================================

NET

==============================================================
*/

// net.h -- quake's interface to the networking layer

#define	PORT_ANY	-1

// FIXME: this really shouldn't have to be changed to 2000 but some maps (SSTOWN4) were causing it to crash
//#define	MAX_MSGLEN		1400		// max length of a message
#define	MAX_MSGLEN		2500		// max length of a message
#define	PACKET_HEADER	10			// two ints and a short

typedef enum {NA_LOOPBACK, NA_BROADCAST, NA_IP, NA_IPX, NA_BROADCAST_IPX} netadrtype_t;

typedef enum {NS_CLIENT, NS_SERVER, NS_GAMESPY} netsrc_t;

extern	int		gamespy_port;

typedef struct
{
	netadrtype_t	type;

	byte	ip[4];
	byte	ipx[10];

	unsigned short	port;
} netadr_t;

void		NET_Init (void);
void		NET_Shutdown (void);
void		NET_TotalShutdown (void);

void		NET_Config (qboolean multiplayer);

qboolean	NET_GetPacket (netsrc_t sock, netadr_t *net_from, sizebuf_t *net_message);
void		NET_SendPacket (netsrc_t sock, int length, void *data, netadr_t to);

qboolean	NET_CompareAdr (netadr_t a, netadr_t b);
qboolean	NET_CompareBaseAdr (netadr_t a, netadr_t b);
qboolean	NET_IsLocalAddress (netadr_t adr);
char		*NET_AdrToString (netadr_t a);
qboolean	NET_StringToAdr (char *s, netadr_t *a);

//============================================================================

#define	OLD_AVG		0.99		// total = oldtotal*OLD_AVG + new*(1-OLD_AVG)

#define	MAX_LATENT	32

typedef struct
{
	qboolean	fatal_error;

	netsrc_t	sock;

	int			dropped;			// between last packet and previous

	int			last_received;		// for timeouts
	int			last_sent;			// for retransmits

	netadr_t	remote_address;
	int			qport;				// qport value to write when transmitting

// sequencing variables
	int			incoming_sequence;
	int			incoming_acknowledged;
	int			incoming_reliable_acknowledged;	// single bit

	int			incoming_reliable_sequence;		// single bit, maintained local

	int			outgoing_sequence;
	int			reliable_sequence;			// single bit
	int			last_reliable_sequence;		// sequence number of last send

// reliable staging and holding areas
	sizebuf_t	message;		// writing buffer to send to server
	byte		message_buf[MAX_MSGLEN-16];		// leave space for header

// message is copied to this buffer when it is first transfered
	int			reliable_length;
	byte		reliable_buf[MAX_MSGLEN-16];	// unacked reliable message
} netchan_t;

extern	netadr_t	net_from;
extern	sizebuf_t	net_message;
extern	byte		net_message_buffer[MAX_MSGLEN];


void Netchan_Init (void);
void Netchan_Setup (netsrc_t sock, netchan_t *chan, netadr_t adr, int qport);

qboolean Netchan_NeedReliable (netchan_t *chan);
int Netchan_Transmit (netchan_t *chan, int length, byte *data);
void Netchan_OutOfBand (int net_socket, netadr_t adr, int length, byte *data);
void Netchan_OutOfBandPrint (int net_socket, netadr_t adr, char *format, ...);
qboolean Netchan_Process (netchan_t *chan, sizebuf_t *msg);

qboolean Netchan_CanReliable (netchan_t *chan);


/*
==============================================================

CMODEL

==============================================================
*/


#include "../qcommon/qfiles.h"

cmodel_t	*CM_LoadMap (char *name, qboolean clientload, unsigned *checksum);
cmodel_t	*CM_InlineModel (char *name);	// *1, *2, etc

int			CM_NumClusters (void);
int			CM_NumInlineModels (void);
char		*CM_EntityString (void);

// creates a clipping hull for an arbitrary box
int			CM_HeadnodeForBox (vec3_t mins, vec3_t maxs);


// returns an ORed contents mask
int			CM_PointContents (vec3_t p, int headnode);
int			CM_TransformedPointContents (vec3_t p, int headnode, vec3_t origin, vec3_t angles);

void		CM_BoxTrace (vec3_t start, vec3_t end,
						  vec3_t mins, vec3_t maxs,
						  int headnode, int brushmask, trace_t *return_trace);
void		CM_TransformedBoxTrace (vec3_t start, vec3_t end,
						  vec3_t mins, vec3_t maxs,
						  int headnode, int brushmask,
						  vec3_t origin, vec3_t angles, trace_t *return_trace);

byte		*CM_ClusterPVS (int cluster);
byte		*CM_ClusterPHS (int cluster);

int			CM_PointLeafnum (vec3_t p);

// call with topnode set to the headnode, returns with topnode
// set to the first node that splits the box
int			CM_BoxLeafnums (vec3_t mins, vec3_t maxs, int *list,
							int listsize, int *topnode);

int			CM_LeafContents (int leafnum);
int			CM_LeafCluster (int leafnum);
int			CM_LeafArea (int leafnum);

void		CM_SetAreaPortalState (int portalnum, qboolean open);
qboolean	CM_AreasConnected (int area1, int area2);

int			CM_WriteAreaBits (byte *buffer, int area);
qboolean	CM_HeadnodeVisible (int headnode, byte *visbits);

void		CM_WritePortalState (FILE *f);
void		CM_ReadPortalState (FILE *f);

/*
==============================================================

PLAYER MOVEMENT CODE

Common between server and client so prediction matches

==============================================================
*/
				  
void Pmove(pmove_t *pmove, qboolean server);

/*
==============================================================

FILESYSTEM

==============================================================
*/

void		FS_InitFilesystem (void);
char		*FS_GetPath (char *name);
void		FS_SetGamedir (char *dir);
char		*FS_Gamedir (void);
char		*FS_Userdir (void);
char		*FS_NextPath (char *prevpath);
void		FS_ExecAutoexec (void);

int			FS_FOpenFile (char *filename, FILE **file);
void		FS_FCloseFile (FILE *f);
// note: this can't be called from another DLL, due to MS libc issues

int			FS_LoadFile (char *path, void **buffer);
// a null buffer will just return the file length without loading
// a -1 length is not present

void		FS_Read (void *buffer, int len, FILE *f);
// properly handles partial reads

void		FS_FreeFile (void *buffer);

void		FS_CreatePath (char *path);

/*
==============================================================

MISC

==============================================================
*/

#define CFX_CULLING_DIST 1000.0f

#define	ERR_FATAL	0		// exit the entire game with a popup window
#define	ERR_DROP	1		// print to console and disconnect from game
#define	ERR_QUIT	2		// not an error, just a normal exit

#define	EXEC_NOW	0		// don't return until completed
#define	EXEC_INSERT	1		// insert at current position, but don't run yet
#define	EXEC_APPEND	2		// add to end of the command buffer

#define	PRINT_ALL		0
#define PRINT_DEVELOPER	1	// only print when "developer 1"

void		Com_BeginRedirect (int target, char *buffer, int buffersize, void (*flush));
void		Com_EndRedirect (void);
void 		Com_Printf (char *fmt, ...);
void 		Com_DPrintf (char *fmt, ...);
void 		Com_Error (int code, char *fmt, ...);
void 		Com_Quit (void);
int			Com_ServerState (void);
void		Com_SetServerState (int state);

unsigned	Com_BlockChecksum (void *buffer, int length);
byte		COM_BlockSequenceCheckByte (byte *base, int length, int sequence);

extern	cvar_t	*developer;
extern	cvar_t	*dedicated;
extern	cvar_t	*host_speeds;
extern	cvar_t	*log_stats;
extern	cvar_t	*player_dll;

extern	cvar_t	*allow_download;
extern	cvar_t	*allow_download_maps;
extern	cvar_t	*allow_download_players;
extern	cvar_t	*allow_download_models;
extern	cvar_t	*allow_download_sounds;

extern	FILE *log_stats_file;

// host_speeds times
#ifdef _DEVEL
extern	__int64	time_before_game;
extern	__int64	time_after_game;
extern	__int64	time_before_ref;
extern	__int64	time_after_ref;
#endif	// _DEVEL

void Z_Free (void *ptr);
void *Z_Malloc (int size);			// returns 0 filled memory
void *Z_TagMalloc (int size, int tag);
void Z_FreeTags (int tag);

void Qcommon_Init (int argc, char **argv);
void Qcommon_Frame (int msec);

#define NUMVERTEXNORMALS	162
extern	vec3_t	bytedirs[NUMVERTEXNORMALS];

/*
==============================================================

NON-PORTABLE SYSTEM SERVICES

==============================================================
*/

void	Sys_Init (void);

void	Sys_AppActivate (void);

void	Sys_UnloadGame (void);
void	*Sys_GetGameAPI (void *parms);
// loads the game dll and calls the api init function

char	*Sys_ConsoleInput (void);
void	Sys_ConsoleOutput (char *string);
void	Sys_SendKeyEvents (void);
void	Sys_Error (char *error, ...);
void	Sys_Quit (void);
char	*Sys_GetClipboardData( void );
void	Sys_CopyProtect (void);

/*
==============================================================

CLIENT / SERVER SYSTEMS

==============================================================
*/

void CL_Init (void);
void CL_Drop (void);
void CL_Shutdown (void);
void CL_Frame (int msec);
void SCR_BeginLoadingPlaque (void);

void SV_Init (void);
void SV_Shutdown (char *finalmsg, qboolean reconnect);
void SV_Frame (int msec);

#endif // QCOMMON_H