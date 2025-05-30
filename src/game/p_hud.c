//
// p_hud.c
//
// Copyright 1998 Raven Software
//

#include "p_hud.h" //mxd
#include "g_playstats.h"
#include "g_itemstats.h"
#include "p_client.h" //mxd
#include "p_dll.h" //mxd
#include "Random.h"
#include "Vector.h"
#include "qcommon.h"
#include "g_local.h"

#pragma region ========================== Statusbar layouts ==========================

/*
Cursor positioning:
	xl <value>
	xr <value>
	yb <value>
	yt <value>

Drawing:
	statpic <name>
	pic <stat>
	num <fieldwidth> <stat>
	string <stat>

Control:
	if <stat>
	ifeq <stat> <value>
	ifbit <stat> <value>
	endif
*/

char* single_statusbar =
"yb -74 "
"xl 16 "		// Green mana.
"bar 8 16 60 "

"yb -44 "

"xl 40 "
"pic 4 "		// Weapon.

"xl 76 "		// Ammo.
"pic 2 "
"am "

"xr -152 "		// Armour.
"pic 34 "
"arm "

"xr -112 "
"pic 0 "
"hnum "			// Health.

"if 6 "
"yb -44 "
"xr -72 "
"pic 6 "		// Defence.
"endif "

"yb -74 "
"xr -32 "
"bar 11 16 60 "	// Blue mana.

" yt 16 "

"if 28 "
" xl 32 "
" bar 26 60 16 "	// Lung time left.
"endif "

"if 25 "
" xr -96 "
" bar 23 60 16 "	// Powerup time left.
"endif "

"yt 16 "

"xc 0 "		// Inventory Puzzle Item 1.
"pici 18 "

"xc 40 "	// Inventory Puzzle Item 2.
"pici 19 "

"xc 80 "	// Inventory Puzzle Item 3.
"pici 20 "

"xc 120 "	// Inventory Puzzle Item 4.
"pici 21 "

"if 31 "
" xl 32 "
" gbar 29 "	// Boss Life Meter.
"endif ";

char* dm_statusbar =
"yb -74 "
"xl 16 "		// Green mana.
"bar 8 16 60 "

"yb -44 "

"xl 40 "
"pic 4 "		// Weapon.

"xl 76 "		// Ammo.
"pic 2 "
"am "

"xr -152 "		// Armour.
"pic 34 "
"arm "

"xr -112 "
"pic 0 "
"hnum "			// Health.

"yb -44 "
"xr -72 "
"pic 6 "		// Defence.

"yb -74 "
"xr -32 "
"bar 11 16 60 " // Blue mana.

" yt 16 "

"if 28 "
" xl 32 "
" bar 26 60 16 " // Lung time left.
"endif "

"if 25 "
" xr -96 "
" bar 23 60 16 " // Powerup time left.
"endif ";

#pragma endregion

#pragma region ========================== INTERMISSION ==========================

void MoveClientToIntermission(edict_t* client, const qboolean log_file)
{
	VectorCopy(level.intermission_origin, client->s.origin);

	for (int i = 0; i < 3; i++)
		client->client->ps.pmove.origin[i] = (short)(level.intermission_origin[i] * 8.0f);

	VectorCopy(level.intermission_angle, client->client->ps.viewangles);

	client->client->ps.pmove.pm_type = PM_INTERMISSION;
	client->client->ps.rdflags &= ~RDF_UNDERWATER;

	// Clean up powerup info.
	client->client->invincible_framenum = 0;

	client->viewheight = 0;
	client->s.modelindex = 0;
	client->s.effects = 0;
	client->s.sound = 0;
	client->solid = SOLID_NOT;

	// Add the layout.
	if (DEATHMATCH)
	{
		client->client->playerinfo.showscores = true;
		DeathmatchScoreboardMessage(log_file);
		gi.unicast(client, true);
	}
}

void BeginIntermission(const edict_t* target_changelevel)
{
	// Already activated?
	if (level.intermissiontime > 0.0f)
		return;

	game.autosaved = false;

	// Respawn any dead clients.
	for (int i = 0; i < MAXCLIENTS; i++)
	{
		edict_t* client = &g_edicts[i + 1];

		if (client->inuse && client->health <= 0)
			ClientRespawn(client);
	}

	level.intermissiontime = level.time;
	level.changemap = target_changelevel->map;

	if (!DEATHMATCH)
	{
		// Go immediately to the next level if not deathmatch.
		level.exitintermission = true;
		return;
	}

	level.exitintermission = false;

	// Find an intermission spot.
	edict_t* spot = G_Find(NULL, FOFS(classname), "info_player_intermission");

	if (spot == NULL)
	{
		// The map creator forgot to put in an intermission point.
		spot = G_Find(NULL, FOFS(classname), "info_player_start");

		if (spot == NULL)
			spot = G_Find(NULL, FOFS(classname), "info_player_deathmatch");
	}
	else
	{
		// Chose one of four spots.
		for (int i = 0; i < irand(0, 3); i++)
		{
			spot = G_Find(spot, FOFS(classname), "info_player_intermission");

			if (spot == NULL)
				spot = G_Find(spot, FOFS(classname), "info_player_intermission"); // Wrap around the list.
		}
	}

	VectorCopy(spot->s.origin, level.intermission_origin);
	VectorCopy(spot->s.angles, level.intermission_angle);

	// Move all clients to the intermission point.
	for (int i = 0; i < MAXCLIENTS; i++)
	{
		edict_t* client = &g_edicts[i + 1];

		if (client->inuse)
			MoveClientToIntermission(client, i == 0);
	}
}

#pragma endregion

#pragma region ========================== DM SCOREBOARD ==========================

typedef struct
{
	int sorted;
	int scores;
} team_sort_t;

typedef struct
{
	char team_name[200];
	int team_score;
	int count_for_team;
	team_sort_t	team_sort[MAX_CLIENTS];
} team_scores_t;

void DeathmatchScoreboardMessage(qboolean log_file)
{
#define MAX_STRING_SIZE 1400

	const char* game_types[3] = { "Cooperative", "Death Match", "Team Play Death Match" };
	FILE* logfile = NULL; //mxd. Initialize to avoid compiler warning.

	if (log_file && log_file_name->string[0] != 0)
	{
		char name[MAX_QPATH];
		Com_sprintf(name, sizeof(name), "%s/%s", gi.FS_Userdir(), log_file_name->string);

		Com_Printf("Dumping end game log to %s\n", name);
		gi.FS_CreatePath(name);

		if (fopen_s(&logfile, name, "a") == 0) //mxd. fopen -> fopen_s.
		{
			int game_type = 0; // COOP.

			if (DEATHMATCH)
				game_type = ((DMFLAGS & (DF_MODELTEAMS | DF_SKINTEAMS)) ? 2 : 1); // TDM/DM.

			int secs = (int)level.time;
			const int hours = secs / (60 * 60);
			secs -= hours * (60 * 60);
			const int mins = secs / 60;
			secs -= mins * 60;

			const cvar_t* host_name = gi.cvar("hostname", "", 0);

			fprintf(logfile, "%s\n", log_file_header->string);
			fprintf(logfile, "%sMap Name : %s\n", log_file_line_header->string, level.mapname);
			fprintf(logfile, "%sHost Name : %s\n", log_file_line_header->string, host_name->string);
			fprintf(logfile, "%sGame Type : %s\n", log_file_line_header->string, game_types[game_type]);
			fprintf(logfile, "%sGame Duration : %02i:%02i:%02i\n%s\n", log_file_line_header->string, hours, mins, secs, log_file_line_header->string);
		}
		else
		{
			Com_Printf("ERROR: couldn't open.\n");
			log_file = false;
		}
	}
	else
	{
		// We might have no file name, but logging is set to go.
		log_file = false;
	}

	char string[MAX_STRING_SIZE] = { 0 };
	int string_length = 0;

	// Sort the clients by score and team if we are playing team play, then resort them by team score.
	if (DMFLAGS & (DF_MODELTEAMS | DF_SKINTEAMS))
	{
		// Ensure we have an empty table
		team_scores_t team_scores[MAX_CLIENTS] = { 0 };
		int total_teams = 0;

		for (int i = 0; i < game.maxclients; i++)
		{
			const edict_t* cl_ent = &g_edicts[i + 1];
			if (!cl_ent->inuse)
				continue;

			// Determine score and team type.
			const int score = game.clients[i].resp.score;

			char value[512];
			strcpy_s(value, sizeof(value), Info_ValueForKey(cl_ent->client->playerinfo.pers.userinfo, "skin")); //mxd. strcpy -> strcpy_s

			if (value[0] == 0)
				continue;

			char* p = strchr(value, '/');

			if (p == NULL)
			{
				p = &value[0];
			}
			else if (DMFLAGS & DF_SKINTEAMS) // Didn't find a team name.
			{
				p++;
			}
			else
			{
				*p = 0;
				p = &value[0];
			}

			// Now find a place in the team list to insert it.
			int team_index;
			for (team_index = 0; team_index < total_teams; team_index++)
				if (Q_stricmp(team_scores[team_index].team_name, p) == 0) // Is it the same as our current one?
					break;

			// Find the position within the team mates array we should be, given score.
			int score_index;
			team_scores_t* cur_score = &team_scores[team_index]; //mxd
			for (score_index = 0; score_index < cur_score->count_for_team; score_index++)
				if (cur_score->team_sort[score_index].scores < score)
					break;

			// Shuffle all the other scores down if they need to be.
			for (int c = cur_score->count_for_team; c > score_index; c--)
				cur_score->team_sort[c] = cur_score->team_sort[c - 1];

			// Insert us into this team/sorted player slot within the team structure.
			strcpy_s(cur_score->team_name, sizeof(cur_score->team_name), p); //mxd. strcpy -> strcpy_s
			cur_score->team_score += score;
			cur_score->team_sort[score_index].scores = score;
			cur_score->team_sort[score_index].sorted = i;
			cur_score->count_for_team++;

			if (team_index == total_teams)
				total_teams++;
		}

		// Now order the teams into team score order - nasty little bubble sort here.
		qboolean bubble;
		do
		{
			bubble = false;
			for (int i = 0; i < total_teams - 1; i++)
			{
				if (team_scores[i].team_score < team_scores[i + 1].team_score)
				{
					bubble = true;
					const team_scores_t temp_point = team_scores[i];
					team_scores[i] = team_scores[i + 1];
					team_scores[i + 1] = temp_point;
				}

			}
		} while (bubble);

		// Determine how many of each team gets displayed.
		const int max_team_display = ((total_teams > 0) ? max(1, 10 / total_teams) : 0);

		// Now display the data.
		const int real_total = total_teams;
		total_teams = min(10, total_teams);

		int y = 32;
		for (int i = 0, k = 0; i < total_teams; i++)
		{
			int x = (k >= 5 ? 180 : 0);
			if (k == 5)
				y = 32;

			char entry[MAX_STRING_SIZE];
			Com_sprintf(entry, sizeof(entry), "tm %i %i %i %s ", x, y, team_scores[i].team_score, team_scores[i].team_name);

			const int len = (int)strlen(entry);
			if (string_length + len > MAX_STRING_SIZE)
				break;

			strcpy_s(&string[string_length], sizeof(string) - string_length - 1, entry); //mxd. strcpy -> strcpy_s
			string_length += len;
			y += 16;

			for (int j = 0; j < max_team_display; j++)
			{
				// Don't try and print more than there are for this team.
				if (j >= team_scores[i].count_for_team)
					continue;

				x = (k >= 5 ? 180 : 0);

				const team_sort_t* cur_team = &team_scores[i].team_sort[j]; //mxd
				const gclient_t* cl = &game.clients[cur_team->sorted];

				// Send the layout.
				Com_sprintf(entry, sizeof(entry), "client %i %i %i %i %i %i ", x, y, cur_team->sorted, cur_team->scores, cl->ping, (level.framenum - cl->resp.enterframe) / 600);

				const int entry_len = (int)strlen(entry);
				if (string_length + entry_len > MAX_STRING_SIZE)
					break;

				strcpy_s(&string[string_length], sizeof(string) - string_length - 1, entry); //mxd. strcpy -> strcpy_s

				string_length += entry_len;
				y += 32;
				k++;

				if (k == 5)
					y = 32;
			}

			y += 8;
		}

		if (log_file)
		{
			for (int i = 0; i < real_total; i++)
			{
				team_scores_t* cur_team = &team_scores[i]; //mxd
				fprintf(logfile, "%s%sTeam %s\n", log_file_line_header->string, log_file_line_header->string, cur_team->team_name);
				fprintf(logfile, "%sTeam Score %i\n%s\n", log_file_line_header->string, cur_team->team_score, log_file_line_header->string);

				for (int j = 0; j < cur_team->count_for_team; j++)
				{
					const gclient_t* cl = &game.clients[cur_team->team_sort[j].sorted];
					const edict_t* cl_ent = &g_edicts[cur_team->team_sort[j].sorted + 1];

					fprintf(logfile, "%sClient %s\n", log_file_line_header->string, cl_ent->client->playerinfo.pers.netname);
					fprintf(logfile, "%sScore %i\n", log_file_line_header->string, cur_team->team_sort[j].scores);
					fprintf(logfile, "%sPing %i\n", log_file_line_header->string, cl->ping);
					fprintf(logfile, "%sTime %i\n%s\n", log_file_line_header->string, (level.framenum - cl->resp.enterframe) / 600, log_file_line_header->string);
				}
			}
		}
	}
	else // Sort the clients by score - for normal deathmatch play.
	{
		int total = 0;
		int sorted[MAX_CLIENTS] = { 0 }; //mxd. Initialize.
		int sorted_scores[MAX_CLIENTS] = { 0 }; //mxd. Initialize.

		for (int i = 0; i < game.maxclients; i++)
		{
			const edict_t* cl_ent = &g_edicts[i + 1];
			if (!cl_ent->inuse)
				continue;

			int score = game.clients[i].resp.score;

			int insert_index;
			for (insert_index = 0; insert_index < total; insert_index++)
				if (score > sorted_scores[insert_index])
					break;

			for (int k = total; k > insert_index; k--)
			{
				sorted[k] = sorted[k - 1];
				sorted_scores[k] = sorted_scores[k - 1];
			}

			sorted[insert_index] = i;
			sorted_scores[insert_index] = score;
			total++;
		}

		int real_total = total;
		total = min(12, total);

		// Now display the data.
		int y = 32;

		for (int i = 0; i < total; i++)
		{
			gclient_t* cl = &game.clients[sorted[i]];

			int x = (i >= 6 ? 160 : 0);
			if (i == 6)
				y = 32;

			// Send the layout.
			char entry[MAX_STRING_SIZE];
			Com_sprintf(entry, sizeof(entry), "client %i %i %i %i %i %i ", x, y, sorted[i], cl->resp.score, cl->ping, (level.framenum - cl->resp.enterframe) / 600);

			int len = (int)strlen(entry);
			if (string_length + len > MAX_STRING_SIZE)
				break;

			strcpy_s(&string[string_length], sizeof(string) - string_length - 1, entry); //mxd. strcpy -> strcpy_s
			string_length += len;
			y += 32;
		}

		if (log_file)
		{
			for (int i = 0; i < real_total; i++)
			{
				gclient_t* cl = &game.clients[sorted[i]];
				edict_t* cl_ent = &g_edicts[sorted[i] + 1];

				fprintf(logfile, "%sClient %s\n", log_file_line_header->string, cl_ent->client->playerinfo.pers.netname);
				fprintf(logfile, "%sScore %i\n", log_file_line_header->string, cl->resp.score);
				fprintf(logfile, "%sPing %i\n", log_file_line_header->string, cl->ping);
				fprintf(logfile, "%sTime %i\n%s\n", log_file_line_header->string, (level.framenum - cl->resp.enterframe) / 600, log_file_line_header->string);
			}
		}
	}

	// Print level name and exit rules.
	gi.WriteByte(svc_layout);
	gi.WriteString(string);

	// Close any file that needs to be.
	if (log_file)
	{
		fprintf(logfile, "%s\n", log_file_footer->string);
		fclose(logfile);
	}
}

// Display the scoreboard.
void Cmd_Score_f(const edict_t* ent)
{
	if (!DEATHMATCH)
		return;

	if (!ent->client->playerinfo.showscores)
	{
		// Draw instead of help message. Note that it isn't that hard to overflow the 1400 byte message limit!
		DeathmatchScoreboardMessage(false);
		gi.unicast(ent, true);
	}

	ent->client->playerinfo.showscores = !ent->client->playerinfo.showscores;
}

#pragma endregion

#pragma region ========================== STATUSBAR LAYOUT LOGIC ==========================

static short GetShrineTime(const float time)
{
	const float duration = time - level.time;
	if (duration >= 0.0f)
		return (short)(ceilf(duration));

	return 0;
}

void G_SetStats(const edict_t* ent)
{
	static char* health_icons[2] = //TODO: i_health.m8 and i_health2.m8 files are identical!
	{
		"icons/i_health.m8",
		"icons/i_health2.m8",
	};

	const gclient_t* pi = ent->client;
	player_state_t* ps = &ent->client->ps;
	const client_persistant_t* pers = &ent->client->playerinfo.pers;

	// Frags.
	ps->stats[STAT_FRAGS] = (short)pi->resp.score;

	// Health.
	ps->stats[STAT_HEALTH_ICON] = (short)gi.imageindex(health_icons[Q_ftol(level.time * 2.0f) & 1]);
	ps->stats[STAT_HEALTH] = (short)ent->health;

	// Weapon / defense.
	ps->stats[STAT_WEAPON_ICON] = (short)gi.imageindex(pers->weapon->icon);
	if (pers->defence != NULL)
		ps->stats[STAT_DEFENCE_ICON] = (short)gi.imageindex(pers->defence->icon);

	// Weapon ammo.
	if (pers->weapon->ammo != NULL && pers->weapon->count_width > 0)
	{
		const gitem_t* ammo = P_FindItem(pers->weapon->ammo);

		ps->stats[STAT_AMMO_ICON] = (short)gi.imageindex(ammo->icon);
		ps->stats[STAT_AMMO] = (short)pers->inventory.Items[ITEM_INDEX(ammo)];
	}
	else
	{
		ps->stats[STAT_AMMO_ICON] = 0;
	}

	// Offensive mana.
	ps->stats[STAT_OFFMANA_ICON] = (short)gi.imageindex("icons/green-mana.m8");
	ps->stats[STAT_OFFMANA_BACK] = (short)gi.imageindex("icons/green-mana2.m8");

	const gitem_t* mana_off = P_FindItem("Off-mana");
	ps->stats[STAT_OFFMANA] = (short)((pers->inventory.Items[ITEM_INDEX(mana_off)] * 100) / MAX_OFF_MANA);
	ps->stats[STAT_OFFMANA] = max(0, ps->stats[STAT_OFFMANA]);

	// Defensive mana.
	ps->stats[STAT_DEFMANA_ICON] = (short)gi.imageindex("icons/blue-mana.m8");
	ps->stats[STAT_DEFMANA_BACK] = (short)gi.imageindex("icons/blue-mana2.m8");

	const gitem_t* mana_def = P_FindItem("Def-mana");
	ps->stats[STAT_DEFMANA] = (short)((pers->inventory.Items[ITEM_INDEX(mana_def)] * 100) / MAX_DEF_MANA);
	if (ps->stats[STAT_DEFMANA] < 0)
		ps->stats[STAT_DEFMANA] = 0;

	// Shrine timers.
	ps->stats[STAT_POWERUP_BACK] = (short)gi.imageindex("icons/powerup2.m8");
	ps->stats[STAT_POWERUP_ICON] = (short)gi.imageindex("icons/powerup.m8");
	ps->stats[STAT_POWERUP_TIMER] = (short)((GetShrineTime(pi->playerinfo.powerup_timer) * 100) / (short)POWERUP_DURATION);

	// Cheating sets the powerup timer to something huge, so let's avoid a crash here.
	ps->stats[STAT_POWERUP_TIMER] = min(100, ps->stats[STAT_POWERUP_TIMER]);

	// Lungs timer.
	ps->stats[STAT_LUNG_BACK] = (short)gi.imageindex("icons/breath2.m8");
	ps->stats[STAT_LUNG_ICON] = (short)gi.imageindex("icons/breath.m8");
	ps->stats[STAT_LUNG_TIMER] = 0;

	if (ent->waterlevel > 2 && !(ent->flags & FL_INLAVA))
	{
		// Make negative if we have lungs powerup.
		if (pi->playerinfo.lungs_timer > 0.0f)
		{
			const float time = pi->playerinfo.lungs_timer + ent->air_finished - level.time;
			if (time > 0)
				ps->stats[STAT_LUNG_TIMER] = (short)(-(time * 100.0f) / (HOLD_BREATH_TIME + LUNGS_DURATION));
		}
		else
		{
			const float time = ent->air_finished - level.time;
			if (time > 0)
				ps->stats[STAT_LUNG_TIMER] = (short)((time * 100.0f) / HOLD_BREATH_TIME);
		}
	}

	// Armour items.
	ps->stats[STAT_ARMOUR_ICON] = 0;
	ps->stats[STAT_ARMOUR] = 0;

	if (pers->armortype == ARMOR_TYPE_SILVER)
	{
		ps->stats[STAT_ARMOUR_ICON] = (short)gi.imageindex("icons/arm_silver.m32");
		ps->stats[STAT_ARMOUR] = (short)((pi->playerinfo.pers.armor_count * 100.0f) / MAX_SILVER_ARMOR);
	}
	else if (pers->armortype == ARMOR_TYPE_GOLD)
	{
		ps->stats[STAT_ARMOUR_ICON] = (short)gi.imageindex("icons/arm_gold.m32");
		ps->stats[STAT_ARMOUR] = (short)((pi->playerinfo.pers.armor_count * 250.0f) / MAX_GOLD_ARMOR);
	}

	// Puzzle items.
	ps->stats[STAT_PUZZLE_ITEM1] = 0;
	ps->stats[STAT_PUZZLE_ITEM2] = 0;
	ps->stats[STAT_PUZZLE_ITEM3] = 0;
	ps->stats[STAT_PUZZLE_ITEM4] = 0;

	// Scan through inventory to handle puzzle pieces.
	ps->stats[STAT_PUZZLE_COUNT] = 0;

	int count = STAT_PUZZLE_ITEM1;
	gitem_t* item = playerExport.p_itemlist;
	for (int i = 0; i < MAX_ITEMS; i++, item++)
	{
		if ((item->flags & IT_PUZZLE) && pers->inventory.Items[i] > 0)
		{
			ps->stats[count] = (short)gi.imageindex(item->icon);
			ps->stats[STAT_PUZZLE_COUNT]++;

			if (PossessCorrectItem(ent, item))
				ps->stats[count] |= 0x8000;

			count++;
		}

		if (count > STAT_PUZZLE_ITEM4)
			break;
	}

	// Layouts.
	ent->client->ps.stats[STAT_LAYOUTS] = 0;

	// Inventory gets activated when player is in a use puzzle trigger field.
	if (ent->target_ent != NULL && strcmp(ent->target_ent->classname, "trigger_playerusepuzzle") == 0)
		ps->stats[STAT_LAYOUTS] |= 4;

	// Show puzzle inventory?
	if (ent->client->playerinfo.showpuzzleinventory)
		ps->stats[STAT_LAYOUTS] |= 4;

	// Show scoreboard?
	if (ent->client->playerinfo.showscores || (DEATHMATCH && (pers->health <= 0 || level.intermissiontime > 0.0f)))
		ps->stats[STAT_LAYOUTS] |= 1;
}

#pragma endregion