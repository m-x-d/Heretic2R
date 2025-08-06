//
// menu_joinserver.h
//
// Copyright 1998 Raven Software
//

#include <windows.h>
#include "client.h"
#include "cl_strings.h"
#include "menu_joinserver.h"
#include "menu_addressbook.h"

cvar_t* m_banner_join;
cvar_t* m_item_refresh;

#define MAX_LOCAL_SERVERS	8

static menuframework_t s_joinserver_menu;

static menuaction_t s_joinserver_address_book_action;
static menuaction_t s_joinserver_search_action;
static menuaction_t s_joinserver_server_actions[MAX_LOCAL_SERVERS];

static int m_num_servers;

// User-readable information.
static char local_server_names[MAX_LOCAL_SERVERS][80];

// Network address.
static netadr_t local_server_netadr[MAX_LOCAL_SERVERS];

// Q2 counterpart
static void AddressBookFunc(void* data)
{
	M_Menu_AddressBook_f();
}

static void SearchLocalGames(void)
{
	m_num_servers = 0;

	for (int i = 0; i < MAX_LOCAL_SERVERS; i++)
		strcpy_s(local_server_names[i], sizeof(local_server_names[i]), MENU_EMPTY); //mxd. strcpy -> strcpy_s

	//mxd. Skip M_DrawTextBox() logic (searching for local servers takes less than a second nowadays).

	// Send out info packets.
	CL_PingServers_f();
}

// Q2 counterpart
static void SearchLocalGamesFunc(void* data)
{
	SearchLocalGames();
}

static void JoinServerFunc(void* self)
{
	char buffer[128];

	const int index = (menuaction_t*)self - s_joinserver_server_actions;
	if (index >= m_num_servers || Q_stricmp(local_server_names[index], MENU_EMPTY) == 0)
		return;

	se.StopAllSounds_Sounding(); // H2
	Com_sprintf(buffer, sizeof(buffer), "connect %s\n", NET_AdrToString(&local_server_netadr[index]));
	Cbuf_AddText(buffer);
	M_ForceMenuOff();
}

void JoinServer_MenuInit(void)
{
	static char name_address[MAX_QPATH];
	static char name_refresh[MAX_QPATH];

	s_joinserver_menu.nitems = 0;

	Com_sprintf(name_address, sizeof(name_address), "\x02%s", m_banner_address->string);
	s_joinserver_address_book_action.generic.type = MTYPE_ACTION;
	s_joinserver_address_book_action.generic.name = name_address;
	s_joinserver_address_book_action.generic.width = re.BF_Strlen(name_address);
	s_joinserver_address_book_action.generic.flags = QMF_LEFT_JUSTIFY;
	s_joinserver_address_book_action.generic.x = 0;
	s_joinserver_address_book_action.generic.y = 0;
	s_joinserver_address_book_action.generic.callback = AddressBookFunc;

	Com_sprintf(name_refresh, sizeof(name_refresh), "\x02%s", m_item_refresh->string);
	s_joinserver_search_action.generic.type = MTYPE_ACTION;
	s_joinserver_search_action.generic.name = name_refresh;
	s_joinserver_search_action.generic.width = re.BF_Strlen(name_refresh);
	s_joinserver_search_action.generic.flags = QMF_LEFT_JUSTIFY;
	s_joinserver_search_action.generic.x = 0;
	s_joinserver_search_action.generic.y = 20;
	s_joinserver_search_action.generic.callback = SearchLocalGamesFunc;

	for (int i = 0; i < MAX_LOCAL_SERVERS; i++)
	{
		menuaction_t* item = &s_joinserver_server_actions[i];
		item->generic.type = MTYPE_ACTION;
		strcpy_s(local_server_names[i], sizeof(local_server_names[i]), MENU_EMPTY);
		item->generic.name = local_server_names[i];
		item->generic.width = re.BF_Strlen(local_server_names[i]);
		item->generic.flags = QMF_LEFT_JUSTIFY | QMF_MULTILINE | QMF_SELECT_SOUND;
		item->generic.x = 0;
		item->generic.y = 60 + i * 20;
		item->generic.callback = JoinServerFunc;
	}

	Menu_AddItem(&s_joinserver_menu, &s_joinserver_address_book_action);
	Menu_AddItem(&s_joinserver_menu, &s_joinserver_search_action);

	for (int i = 0; i < MAX_LOCAL_SERVERS; i++)
		Menu_AddItem(&s_joinserver_menu, &s_joinserver_server_actions[i]);

	Menu_Center(&s_joinserver_menu);
	SearchLocalGames();
}

static void JoinServer_MenuDraw(void)
{
	char title[MAX_QPATH];

	// Draw menu BG.
	Menu_DrawBG("book/back/b_conback8.bk", cls.m_menuscale);

	if (cls.m_menualpha == 0.0f)
		return;

	// Draw menu title.
	Com_sprintf(title, sizeof(title), "\x03%s", m_banner_join->string);
	const int x = M_GetMenuLabelX(re.BF_Strlen(title));
	const int y = M_GetMenuOffsetY(&s_joinserver_menu);
	re.DrawBigFont(x, y, title, cls.m_menualpha);

	// Update server name items width.
	for (int i = 0; i < MAX_LOCAL_SERVERS; i++)
		s_joinserver_server_actions[i].generic.width = re.BF_Strlen(local_server_names[i]);

	// Draw menu items.
	s_joinserver_menu.x = M_GetMenuLabelX(s_joinserver_menu.width);
	Menu_Draw(&s_joinserver_menu);
}

// Q2 counterpart
static const char* JoinServer_MenuKey(const int key)
{
	return Default_MenuKey(&s_joinserver_menu, key);
}

// Q2 counterpart
void M_Menu_JoinServer_f(void)
{
	JoinServer_MenuInit();
	M_PushMenu(JoinServer_MenuDraw, JoinServer_MenuKey);
}

// Q2 counterpart
void M_AddToServerList(const netadr_t* adr, const char* info)
{
	if (m_num_servers == MAX_LOCAL_SERVERS)
		return;

	while (*info == ' ')
		info++;

	// Ignore if duplicated.
	for (int i = 0; i < m_num_servers; i++)
		if (strcmp(info, local_server_names[i]) == 0)
			return;

	local_server_netadr[m_num_servers] = *adr;
	strncpy_s(local_server_names[m_num_servers], sizeof(local_server_names[0]), info, sizeof(local_server_names[0]) - 1); //mxd. strncpy -> strncpy_s
	m_num_servers++;
}