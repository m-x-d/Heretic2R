//
// g_svcmds.c
//
// Copyright 1998 Raven Software
//

#include "g_local.h"

#define MAX_IPFILTERS	1024
#define FILTERBAN		(int)filterban->value //mxd

typedef struct
{
	uint mask;
	uint compare;
} ipfilter_t;

static ipfilter_t ipfilters[MAX_IPFILTERS];
static int num_ipfilters;

static qboolean StringToFilter(char* s, ipfilter_t* f)
{
	char num[128];
	byte ip[4];
	byte ip_mask[4];

	for (int i = 0; i < 4 && *s != 0; i++, s++)
	{
		if (*s < '0' || *s > '9')
		{
			gi.cprintf(NULL, PRINT_HIGH, "Bad filter address: %s\n", s);
			return false;
		}

		int j = 0;
		while (*s >= '0' && *s <= '9')
			num[j++] = *s++;

		num[j] = 0;
		ip[i] = (byte)Q_atoi(num);

		if (ip[i] != 0)
			ip_mask[i] = 255;
	}

	f->mask = *(uint*)ip_mask;
	f->compare = *(uint*)ip;

	return true;
}

// filterban <0 or 1>
// If 1 (the default), then ip addresses matching the current list will be prohibited from entering the game.
// If 0, then only addresses matching the list will be allowed. This lets you easily set up a private game,
// or a game that only allows players from your local network.
qboolean SV_FilterPacket(char* from)
{
	byte ip_mask[4];

	char* s = from;
	for (int i = 0; i < 4 && *s != 0 && *s != ':'; i++, s++)
	{
		while (*s >= '0' && *s <= '9')
		{
			ip_mask[i] = ip_mask[i] * 10 + (byte)(*s - '0');
			s++;
		}
	}

	const uint mask = *(uint*)ip_mask;
	for (int i = 0; i < num_ipfilters; i++)
		if ((mask & ipfilters[i].mask) == ipfilters[i].compare)
			return FILTERBAN;

	return !FILTERBAN;
}

#pragma region ========================== Server commands ==========================

static void SVCmd_Test_f(void) //mxd. Named 'Svcmd_Test_f' in original version.
{
	gi.cprintf(NULL, PRINT_HIGH, "Svcmd_Test_f()\n");
}

// Add IP address to the filter list.
static void SVCmd_AddIP_f(void)
{
	if (gi.argc() < 3)
	{
		gi.cprintf(NULL, PRINT_HIGH, "Usage: addip <ip-mask>\n");
		return;
	}

	int index;
	for (index = 0; index < num_ipfilters; index++)
		if (ipfilters[index].compare == 0xffffffff)
			break; // Free spot.

	if (index == num_ipfilters)
	{
		if (num_ipfilters == MAX_IPFILTERS)
		{
			gi.cprintf(NULL, PRINT_HIGH, "IP filter list is full\n");
			return;
		}

		num_ipfilters++;
	}

	if (!StringToFilter(gi.argv(2), &ipfilters[index]))
		ipfilters[index].compare = 0xffffffff;
}

/*
=================
SV_RemoveIP_f
=================
*/
void SVCmd_RemoveIP_f (void)
{
	ipfilter_t	f;
	int			i, j;

	if (gi.argc() < 3) {
		gi.cprintf(NULL, PRINT_HIGH, "Usage:  sv removeip <ip-mask>\n");
		return;
	}

	if (!StringToFilter (gi.argv(2), &f))
		return;

	for (i=0 ; i<num_ipfilters; i++)
		if (ipfilters[i].mask == f.mask
		&& ipfilters[i].compare == f.compare)
		{
			for (j=i+1 ; j<num_ipfilters; j++)
				ipfilters[j-1] = ipfilters[j];
			num_ipfilters--;
			gi.cprintf (NULL, PRINT_HIGH, "Removed.\n");
			return;
		}
	gi.cprintf (NULL, PRINT_HIGH, "Didn't find %s.\n", gi.argv(2));
}

/*
=================
SV_ListIP_f
=================
*/
void SVCmd_ListIP_f (void)
{
	int		i;
	byte	b[4];

	gi.cprintf (NULL, PRINT_HIGH, "Filter list:\n");
	for (i=0 ; i<num_ipfilters; i++)
	{
		*(unsigned *)b = ipfilters[i].compare;
		gi.cprintf (NULL, PRINT_HIGH, "%3i.%3i.%3i.%3i\n", b[0], b[1], b[2], b[3]);
	}
}

/*
=================
SV_WriteIP_f
=================
*/
void SVCmd_WriteIP_f (void)
{
	FILE	*f;
	char	name[MAX_OSPATH];
	byte	b[4];
	int		i;
	cvar_t	*game;

	game = gi.cvar("game", "", 0);

	if (!*game->string)
		sprintf (name, "%s/listip.cfg", GAMEVERSION);
	else
		sprintf (name, "%s/listip.cfg", game->string);

	gi.cprintf (NULL, PRINT_HIGH, "Writing %s.\n", name);

	f = fopen (name, "wb");
	if (!f)
	{
		gi.cprintf (NULL, PRINT_HIGH, "Couldn't open %s\n", name);
		return;
	}
	
	fprintf(f, "set filterban %d\n", (int)filterban->value);

	for (i=0 ; i<num_ipfilters; i++)
	{
		*(unsigned *)b = ipfilters[i].compare;
		fprintf (f, "sv addip %i.%i.%i.%i\n", b[0], b[1], b[2], b[3]);
	}
	
	fclose (f);
}

#pragma endregion

/*
=================
ServerCommand

ServerCommand will be called when an "sv" command is issued.
The game can issue gi.argc() / gi.argv() commands to get the rest
of the parameters
=================
*/
void	ServerCommand (void)
{
	char	*cmd;

	cmd = gi.argv(1);
	if (Q_stricmp (cmd, "test") == 0)
		SVCmd_Test_f ();
	else if (Q_stricmp (cmd, "addip") == 0)
		SVCmd_AddIP_f ();
	else if (Q_stricmp (cmd, "removeip") == 0)
		SVCmd_RemoveIP_f ();
	else if (Q_stricmp (cmd, "listip") == 0)
		SVCmd_ListIP_f ();
	else if (Q_stricmp (cmd, "writeip") == 0)
		SVCmd_WriteIP_f ();
	else
		gi.cprintf (NULL, PRINT_HIGH, "Unknown server command \"%s\"\n", cmd);
}