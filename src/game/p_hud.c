#include "g_local.h"
#include "random.h"
#include "vector.h"
#include "g_playstats.h"
#include "g_itemstats.h"
#include "qcommon.h"

extern player_export_t	playerExport;	// interface to player DLL.

qboolean PossessCorrectItem(edict_t *ent, gitem_t *item);

#if 0
	// cursor positioning
	xl <value>
	xr <value>
	yb <value>
	yt <value>

	// drawing
	statpic <name>
	pic <stat>
	num <fieldwidth> <stat>
	string <stat>

	// control
	if <stat>
	ifeq <stat> <value>
	ifbit <stat> <value>
	endif

#endif

char *single_statusbar = 
"yb	-74 "
"xl 16 "		// green mana
"bar 8 16 60 "

"yb -44 "

"xl	40 "
"pic 4 "		// Weapon

"xl 76 "		// Ammo
"pic 2 "
"am "

"xr -152 "		// Armour
"pic 34 "
"arm "

"xr -112 "
"pic 0 "
"hnum "			// Health

"if 6 "
"yb -44 "
"xr -72 "
"pic 6 "		// Defence
"endif "

"yb	-74 "
"xr -32 "
"bar 11 16 60 "		// blue mana

" yt 16 "

"if 28 "		
" xl 32 "
" bar 26 60 16 " 	// Lung time left
"endif "

"if 25 "
" xr -96 "
" bar 23 60 16 "	// Powerup time left
"endif "

"yt	16 "		

"xc 0 "				// Inventory Puzzle Item 1
"pici 18 "

"xc 40 "			// Puzzle 2
"pici 19 "

"xc 80 "			// Puzzle 3
"pici 20 "

"xc 120 "			// Puzzle 4
"pici 21 "

"if 31 "		
" xl 32 "
" gbar 29 "			// Boss Life Meter
"endif "

//"hstring 0 -10 2 Name__________Score__Ping__Time "
//"aclient 0 0 3 0 4 300 12 "
;

char *dm_statusbar =
"yb	-74 "
"xl 16 "		// green mana
"bar 8 16 60 "

"yb -44 "

"xl	40 "
"pic 4 "		// Weapon

"xl 76 "		// Ammo
"pic 2 "
"am "

"xr -152 "		// Armour
"pic 34 "
"arm "

"xr -112 "
"pic 0 "
"hnum "			// Health

"yb -44 "
"xr -72 "
"pic 6 "		// Defence

"yb	-74 "
"xr -32 "
"bar 11 16 60 " // blue mana

" yt 16 "

"if 28 "		
" xl 32 "
" bar 26 60 16 " // Lung time left
"endif "

"if 25 "
" xr -96 "
" bar 23 60 16 " // Powerup time left
"endif "

#if	0
"xc 0 "			// Frag 
"num 3 15 "
#endif
;

/*
======================================================================

INTERMISSION

======================================================================
*/

void MoveClientToIntermission(edict_t *ent, qboolean log_file)
{
	if(deathmatch->value)
		ent->client->playerinfo.showscores = true;

	VectorCopy(level.intermission_origin,ent->s.origin);

	ent->client->ps.pmove.origin[0] = level.intermission_origin[0]*8;
	ent->client->ps.pmove.origin[1] = level.intermission_origin[1]*8;
	ent->client->ps.pmove.origin[2] = level.intermission_origin[2]*8;

	VectorCopy(level.intermission_angle,ent->client->ps.viewangles);
	
	ent->client->ps.pmove.pm_type = PM_INTERMISSION;
	ent->client->ps.rdflags &= ~RDF_UNDERWATER;
	
	// Clean up powerup info.

	ent->client->invincible_framenum = 0;

	ent->viewheight = 0;
	ent->s.modelindex = 0;
	ent->s.effects = 0;
	ent->s.sound = 0;
	ent->solid = SOLID_NOT;

	// Add the layout.

	if(deathmatch->value)
	{
		DeathmatchScoreboardMessage(ent,NULL,log_file);
	
		gi.unicast(ent,true);
	}
}

void BeginIntermission(edict_t *targ)
{
	int		i;
	edict_t	*ent,
			*client;

	// Already activated?

	if(level.intermissiontime)
		return;

	game.autosaved = false;

	// Respawn any dead clients.

	for (i=0 ; i<maxclients->value ; i++)
	{
		client = g_edicts + 1 + i;
		if (!client->inuse)
			continue;
		if (client->health <= 0)
			respawn(client);
	}

	level.intermissiontime = level.time;
	level.changemap = targ->map;

	if (!deathmatch->value)
	{
		// Go immediately to the next level if not deathmatch.

		level.exitintermission = 1;

		return;
	}

	level.exitintermission = 0;

	// Find an intermission spot.

	ent = G_Find (NULL, FOFS(classname), "info_player_intermission");
	
	if (!ent)
	{	
		// The map creator forgot to put in an intermission point.

		ent = G_Find (NULL, FOFS(classname), "info_player_start");
		
		if (!ent)
			ent = G_Find (NULL, FOFS(classname), "info_player_deathmatch");
	}
	else
	{	
		// Chose one of four spots.

		i = irand(0, 3);
		
		while (i--)
		{
			ent = G_Find (ent, FOFS(classname), "info_player_intermission");
			
			if (!ent)
			{	
				// Wrap around the list.

				ent = G_Find (ent, FOFS(classname), "info_player_intermission");
			}
		}
	}

	VectorCopy (ent->s.origin, level.intermission_origin);
	VectorCopy (ent->s.angles, level.intermission_angle);

	// Move all clients to the intermission point.

	for(i=0;i<maxclients->value;i++)
	{
		client=g_edicts+1+i;

		if(!client->inuse)
			continue;

		if (!i)
			MoveClientToIntermission(client, true);
		else
			MoveClientToIntermission(client, false);
	}
}


/*
==================
DeathmatchScoreboardMessage

==================
*/
#define MAX_STRING_SIZE 1400

typedef struct
{
	int			sorted;
	int			scores;
} team_sort_t;

typedef struct
{
	char		teamname[200];
	int			teamscore;
	int			count_for_team;
	team_sort_t	team_sort[MAX_CLIENTS];
} team_scores_t;

void DeathmatchScoreboardMessage (edict_t *ent, edict_t *killer, qboolean log_file)
{
	char		entry[MAX_STRING_SIZE];
	char		value[512];
	int			game_type = 0;
	char		name[MAX_QPATH];
	char		string[MAX_STRING_SIZE];
	int			stringlength;
	int			i, j, k, z;
	int			sorted[MAX_CLIENTS];
	int			sortedscores[MAX_CLIENTS];
	int			score, total, max_team, total_team, max_team_display, real_total;
	int			hours, secs, mins;
	int			x, y;
	qboolean	bubble;
	gclient_t	*cl;
	edict_t		*cl_ent;
	char		*p;
	team_scores_t	team_scores[MAX_CLIENTS];
	team_scores_t	temp_point;
	FILE		*f;
	cvar_t		*host_name;
	char		*game_types[3] = 
	{{"Cooperative"},
	{"Death Match"},
	{"Team Play Death Match"}};

	string[0] = 0;
	stringlength = 0;
	total = 0;

	if (log_file && log_file_name->string[0])
	{
		Com_sprintf (name, sizeof(name), "%s/%s", gi.FS_Userdir(), log_file_name->string);

		Com_Printf ("Dumping end game log to %s\n", name);
		gi.FS_CreatePath (name);
		f = fopen (name, "a");
		if (!f)
		{
			Com_Printf ("ERROR: couldn't open.\n");
			log_file = false;
		}
		else
		{
			if (coop->value)
				game_type = 0;
			else
			if (deathmatch->value)
			{
				game_type = 1;
				if ((int)dmflags->value & (DF_MODELTEAMS | DF_SKINTEAMS))
					game_type = 2;
			}
		   
			secs = level.time;
			hours = secs / (60*60);
			secs -= hours * (60*60);
			mins = secs / 60;
			secs -= mins * 60;
			host_name = gi.cvar("hostname", "", 0);
			fprintf (f, "%s\n", log_file_header->string);
			fprintf (f, "%sMap Name : %s\n", log_file_line_header->string, level.mapname);
			fprintf (f, "%sHost Name : %s\n", log_file_line_header->string, host_name->string);
			fprintf (f, "%sGame type : %s\n", log_file_line_header->string, game_types[game_type]);
			fprintf (f, "%sGame Duration : %02i:%02i:%02i\n%s\n", log_file_line_header->string, hours, mins, secs, log_file_line_header->string);
		}
	}
	// we might have no file name, but logging is set to go
	else
		log_file = false;

	// sort the clients by score and team if we are playing team play.
	// then resort them by team score.
	if ((int)(dmflags->value) & (DF_MODELTEAMS | DF_SKINTEAMS))
	{
		// ensure we have an empty table
		memset(team_scores,0,sizeof(team_scores));
		max_team = 0;
		total_team = 0;

		for(i = 0; i < game.maxclients; i++)
		{
			cl_ent = g_edicts + 1 + i;
			if (!cl_ent->inuse)
				continue;

			// determine score and team type
			score = game.clients[i].resp.score;
			strcpy(value, Info_ValueForKey (cl_ent->client->playerinfo.pers.userinfo, "skin"));

			if (!value[0])
				continue;

			p = strchr(value, '/');

			if (p== NULL)
				p=&value[0];
			else
			// didn't find a team name
			if ((int)(dmflags->value) & DF_SKINTEAMS)
				p++;
			else
			{
				*p = 0;					
				p = &value[0];
			}

			// now find a place in the team list to insert it
			for(j = 0; j<total_team;j++)
			{
				// is it the same as our current one ?
   				if(!stricmp(team_scores[j].teamname, p))
   				{
					break;
   				}
			}

			// find the position within the team mates array we should be, given score
			for(k=0; k<team_scores[j].count_for_team;k++)
			{
				if (team_scores[j].team_sort[k].scores < score)
					break;
			}

			// shuffle all the othe scores down if they need to be
			for(x = team_scores[j].count_for_team ; x >k ; x--)
				team_scores[j].team_sort[x] = team_scores[j].team_sort[x-1];

			// insert us into this team/sorted player slot within the team structure
			strcpy(team_scores[j].teamname, p);
			team_scores[j].teamscore +=	score;
			team_scores[j].team_sort[k].scores = score;
			team_scores[j].team_sort[k].sorted = i;
			team_scores[j].count_for_team++;

			if (j==total_team)
				total_team++;

		}

		//determine how many of each team gets displayed
		if (total_team)
			max_team_display = 10/total_team;
		else
			max_team_display = 0;

		if (max_team_display < 1)
			max_team_display = 1;

		// now order the teams into team score order - nasty little bubble sort here
		do
		{
			bubble = false;
			for (i=0; i<total_team-1; i++)
			{
				if (team_scores[i].teamscore < team_scores[i+1].teamscore)
				{
					bubble = true;
					temp_point = team_scores[i];
					team_scores[i] = team_scores[i+1];
					team_scores[i+1] = temp_point;
				}
				
			}
		} while (bubble);
		
		// now display the data
		real_total = total_team;
		if (total_team > 10)
			total_team = 10;

		y = 32;
		for(i = 0, k = 0; i < total_team; i++)
		{
			x = (k >= 5) ? 180 : 0;
			if (k == 5)
				y = 32;

			Com_sprintf (entry, sizeof(entry), "tm %i %i %i %s ",x,y, team_scores[i].teamscore, team_scores[i].teamname);
			j = strlen(entry);
			if(stringlength + j > MAX_STRING_SIZE)
				break;

			strcpy (string + stringlength, entry);
			stringlength += j;
			y += 16;

			for (j = 0; j < max_team_display; j++)
			{
				// don't try and print more than there are for this team
				if (j >= team_scores[i].count_for_team)
					continue;

				x = (k >= 5) ? 180 : 0;
				cl = &game.clients[team_scores[i].team_sort[j].sorted];
				cl_ent = g_edicts + 1 + team_scores[i].team_sort[j].sorted;
				// Send the layout.
				Com_sprintf (entry, sizeof(entry), "client %i %i %i %i %i %i ",
					x, y, team_scores[i].team_sort[j].sorted,team_scores[i].team_sort[j].scores , cl->ping, (level.framenum - cl->resp.enterframe) / 600);
				z = strlen(entry);
				if(stringlength + z > MAX_STRING_SIZE)
					break;

				strcpy (string + stringlength, entry);
				stringlength += z;
				y += 32;
				k++;
				if (k == 5)
					y = 32;
			}
			y += 8;
		}
		if (log_file)
		{
			for(i = 0, k = 0; i < real_total; i++)
			{
				fprintf (f, "%s%sTeam %s\n",log_file_line_header->string,log_file_line_header->string,team_scores[i].teamname);
				fprintf (f, "%sTeam Score %i\n%s\n",log_file_line_header->string,team_scores[i].teamscore,log_file_line_header->string);

				for (j = 0; j < team_scores[i].count_for_team; j++)
				{
					cl = &game.clients[team_scores[i].team_sort[j].sorted];
					cl_ent = g_edicts + 1 + team_scores[i].team_sort[j].sorted;
					fprintf (f, "%sClient %s\n", log_file_line_header->string, cl_ent->client->playerinfo.pers.netname);
					fprintf (f, "%sScore %i\n", log_file_line_header->string, team_scores[i].team_sort[j].scores);
					fprintf (f, "%sPing %i\n", log_file_line_header->string, cl->ping);
					fprintf (f, "%sTime %i\n%s\n", log_file_line_header->string, (level.framenum - cl->resp.enterframe) / 600, log_file_line_header->string);
				}
			}
		}
	}
	// Sort the clients by score - for normal deathmatch play.
	else
	{
		for(i = 0; i < game.maxclients; i++)
		{
			cl_ent = g_edicts + 1 + i;
			if (!cl_ent->inuse)
				continue;

			score = game.clients[i].resp.score;
			for(j = 0; j < total; j++)
			{
				if(score > sortedscores[j])
					break;
			}
			for(k = total; k > j; k--)
			{
				sorted[k] = sorted[k - 1];
				sortedscores[k] = sortedscores[k - 1];
			}
			sorted[j] = i;
			sortedscores[j] = score;
			total++;
		}
		real_total = total;
		if(total > 12)
		{
			total = 12;
		}
		// now display the data

		y = 32;
		for(i = 0; i < total; i++)
		{
			cl = &game.clients[sorted[i]];
			cl_ent = g_edicts + 1 + sorted[i];
			x = (i >= 6) ? 160 : 0;
			if (i == 6)
				y = 32;

			// Send the layout.
			Com_sprintf (entry, sizeof(entry), "client %i %i %i %i %i %i ",
				x, y, sorted[i], cl->resp.score, cl->ping, (level.framenum - cl->resp.enterframe) / 600);
			j = strlen(entry);
			if(stringlength + j > MAX_STRING_SIZE)
			{
				break;
			}
			strcpy (string + stringlength, entry);
			stringlength += j;
			y += 32;
		}
		if (log_file)
		{
			for (i = 0; i<real_total; i++)
			{
				cl = &game.clients[sorted[i]];
				cl_ent = g_edicts + 1 + sorted[i];
				fprintf (f, "%sClient %s\n", log_file_line_header->string, cl_ent->client->playerinfo.pers.netname);
				fprintf (f, "%sScore %i\n", log_file_line_header->string, cl->resp.score);
				fprintf (f, "%sPing %i\n", log_file_line_header->string, cl->ping);
				fprintf (f, "%sTime %i\n%s\n", log_file_line_header->string, (level.framenum - cl->resp.enterframe) / 600, log_file_line_header->string);
			}
		}
	}

	// Print level name and exit rules.
	gi.WriteByte (svc_layout);
	gi.WriteString (string);

	// close any file that needs to be 
	if (log_file)
	{
		fprintf (f, "%s\n", log_file_footer->string);
		fclose(f);
	}
}

/*
==================
DeathmatchScoreboard

Draw instead of help message.
Note that it isn't that hard to overflow the 1400 byte message limit!
==================
*/
void DeathmatchScoreboard (edict_t *ent)
{
	DeathmatchScoreboardMessage (ent, ent->enemy, false);
	gi.unicast (ent, true);
}


/*
==================
Cmd_Score_f

Display the scoreboard
==================
*/
void Cmd_Score_f (edict_t *ent)
{
	if (!deathmatch->value)
		return;

	if (ent->client->playerinfo.showscores)
	{
		ent->client->playerinfo.showscores = false;
		return;
	}

	ent->client->playerinfo.showscores = true;
	DeathmatchScoreboard (ent);
}

//=======================================================================

/*
===============
G_GetShrineTime
===============
*/

short GetShrineTime(float time)
{
	float		duration;
	short		result;

	duration = time - level.time;
	if(duration < 0.0)
	{
		return(0);
	}
	result = (short)ceil(duration);
	return(result);
}

static char *healthicons[2] =
{
	"icons/i_health.m8",
	"icons/i_health2.m8",
};

// ************************************************************************************************
// G_SetStats
// ----------
// ************************************************************************************************

void G_SetStats (edict_t *ent)
{
	int					i, count;
	gitem_t				*item;
	gclient_t			*pi;
	player_state_t		*ps;
	client_persistant_t	*pers;
	float				time;

	pi = ent->client;
	ps = &ent->client->ps;
	pers = &ent->client->playerinfo.pers;

	// ********************************************************************************************
	// Frags
	// ********************************************************************************************

	ps->stats[STAT_FRAGS] = pi->resp.score;

	// ********************************************************************************************
	// Health.
	// ********************************************************************************************

	ps->stats[STAT_HEALTH_ICON]=gi.imageindex (healthicons[Q_ftol(level.time*2)&1]);
	ps->stats[STAT_HEALTH]=ent->health;

	// ********************************************************************************************
	// Weapon / defence.
	// ********************************************************************************************

	ps->stats[STAT_WEAPON_ICON]=gi.imageindex(pers->weapon->icon);
	if (pers->defence)
		ps->stats[STAT_DEFENCE_ICON]=gi.imageindex(pers->defence->icon);

	// ********************************************************************************************
	// Weapon ammo.
	// ********************************************************************************************

	if(pers->weapon->ammo&&pers->weapon->count_width)
	{
		item=P_FindItem(pers->weapon->ammo);
		ps->stats[STAT_AMMO_ICON]=gi.imageindex(item->icon);
		ps->stats[STAT_AMMO] = pers->inventory.Items[ITEM_INDEX(item)];
	}
	else
	{
		ps->stats[STAT_AMMO_ICON] = 0;
	}
	
	// ********************************************************************************************
	// Offensive mana.
	// ********************************************************************************************

	ps->stats[STAT_OFFMANA_ICON]=gi.imageindex("icons/green-mana.m8");
	ps->stats[STAT_OFFMANA_BACK]=gi.imageindex("icons/green-mana2.m8");
	item = P_FindItem("Off-mana");
	ps->stats[STAT_OFFMANA] = (pers->inventory.Items[ITEM_INDEX(item)] * 100) / MAX_OFF_MANA;
	if(ps->stats[STAT_OFFMANA] < 0)
		ps->stats[STAT_OFFMANA] = 0;

	// ********************************************************************************************
	// Defensive mana.
	// ********************************************************************************************
	
	ps->stats[STAT_DEFMANA_ICON]=gi.imageindex("icons/blue-mana.m8");
	ps->stats[STAT_DEFMANA_BACK]=gi.imageindex("icons/blue-mana2.m8");
	item = P_FindItem("Def-mana");
	ps->stats[STAT_DEFMANA] = (pers->inventory.Items[ITEM_INDEX(item)] * 100) / MAX_DEF_MANA;
	if(ps->stats[STAT_DEFMANA] < 0)
		ps->stats[STAT_DEFMANA] = 0;

	// ********************************************************************************************
	// Shrine timers.
	// ********************************************************************************************

	ps->stats[STAT_POWERUP_BACK] = gi.imageindex("icons/powerup2.m8");
	ps->stats[STAT_POWERUP_ICON] = gi.imageindex("icons/powerup.m8");
	ps->stats[STAT_POWERUP_TIMER] = (GetShrineTime(pi->playerinfo.powerup_timer) * 100) / POWERUP_DURATION;
	// Cheating sets the powerup timer to something huge, so let's avoid a crash here.
	if (ps->stats[STAT_POWERUP_TIMER] > 100)
		ps->stats[STAT_POWERUP_TIMER]=100;

	ps->stats[STAT_LUNG_BACK] =	gi.imageindex("icons/breath2.m8");
	ps->stats[STAT_LUNG_ICON] =	gi.imageindex("icons/breath.m8");
	ps->stats[STAT_LUNG_TIMER] = 0;
	if((ent->waterlevel > 2) && !(ent->flags & FL_INLAVA))
	{
		// Make negative if we have lungs powerup.
		if(pi->playerinfo.lungs_timer)
		{
			time = pi->playerinfo.lungs_timer + ent->air_finished - level.time;
			if(time > 0)
			{
				ps->stats[STAT_LUNG_TIMER] = -(time * 100) / (HOLD_BREATH_TIME + LUNGS_DURATION);
			}
		}
		else
		{
			time = ent->air_finished - level.time;
			if(time > 0)
			{
				ps->stats[STAT_LUNG_TIMER] = (time * 100) / HOLD_BREATH_TIME;
			}
		}
	}

	// ********************************************************************************************
	// Armour items.
	// ********************************************************************************************	

	ps->stats[STAT_ARMOUR_ICON] = 0;
	ps->stats[STAT_ARMOUR] = 0;
	if(pers->armortype == ARMOR_TYPE_SILVER)
	{
		ps->stats[STAT_ARMOUR_ICON] = gi.imageindex("icons/arm_silver.m32");
		ps->stats[STAT_ARMOUR] = (pi->playerinfo.pers.armor_count * 100) / MAX_SILVER_ARMOR;
	}
	if(pers->armortype == ARMOR_TYPE_GOLD)
	{
		ps->stats[STAT_ARMOUR_ICON] = gi.imageindex("icons/arm_gold.m32");
		ps->stats[STAT_ARMOUR] = (pi->playerinfo.pers.armor_count * 250) / MAX_GOLD_ARMOR;
	}

	// ********************************************************************************************
	// Puzzle items.
	// ********************************************************************************************	

	ps->stats[STAT_PUZZLE_ITEM1] = 0;
	ps->stats[STAT_PUZZLE_ITEM2] = 0;
	ps->stats[STAT_PUZZLE_ITEM3] = 0;
	ps->stats[STAT_PUZZLE_ITEM4] = 0;

	// Scan through inventory to handle puzzle pieces.

	item = playerExport.p_itemlist;
	count = STAT_PUZZLE_ITEM1;
	ps->stats[STAT_PUZZLE_COUNT] = 0;
	for(i = 0; i < MAX_ITEMS; i++, item++)
	{
		if((item->flags & IT_PUZZLE) && pers->inventory.Items[i])
		{
			 if(count > STAT_PUZZLE_ITEM4)
			 {
				 break;
			 }
			 else
			 {
				ps->stats[count] = gi.imageindex (item->icon);
				ps->stats[STAT_PUZZLE_COUNT]++;
				if(PossessCorrectItem(ent, item))
				{
					ps->stats[count] |= 0x8000;
				}
				count++;
			 }
		} 
	}

	// ********************************************************************************************
	// Layouts.
	// ********************************************************************************************

	ent->client->ps.stats[STAT_LAYOUTS] = 0;

	// Inventory gets activated when player is in a use puzzle trigger field.

	if(ent->target_ent)
	{
		if(!strcmp(ent->target_ent->classname, "trigger_playerusepuzzle"))
		{
			ps->stats[STAT_LAYOUTS] |= 4;
		}
	}

	if (ent->client->playerinfo.showpuzzleinventory)
	{
		// Show puzzle inventory.

		ps->stats[STAT_LAYOUTS] |= 4;
	}

	if (deathmatch->value)
	{
		if (pers->health <= 0 || level.intermissiontime	|| ent->client->playerinfo.showscores)
			ps->stats[STAT_LAYOUTS] |= 1;
	}
	else
	{
		if (ent->client->playerinfo.showscores)
		    ps->stats[STAT_LAYOUTS] |= 1;
	}
}

// end
