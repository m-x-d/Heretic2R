//
// menu_addressbook.c
//
// Copyright 1998 Raven Software
//

#include "client.h"
#include "menu_addressbook.h"

cvar_t* m_banner_address;

#define NUM_ADDRESSBOOK_ENTRIES	8 // 9 in Q2

static menuframework_t s_addressbook_menu;
static menufield_t s_addressbook_fields[NUM_ADDRESSBOOK_ENTRIES];

static void AddressBook_MenuInit(void)
{
	s_addressbook_menu.nitems = 0;

	for (int i = 0; i < NUM_ADDRESSBOOK_ENTRIES; i++)
	{
		char buffer[20];
		Com_sprintf(buffer, sizeof(buffer), "adr%d", i);

		const cvar_t* adr = Cvar_Get(buffer, "", CVAR_ARCHIVE);

		s_addressbook_fields[i].generic.type = MTYPE_FIELD;
		s_addressbook_fields[i].generic.name = NULL;
		s_addressbook_fields[i].generic.callback = NULL;
		s_addressbook_fields[i].generic.x = 0;
		s_addressbook_fields[i].generic.y = i * 18;
		s_addressbook_fields[i].generic.width = 240; // H2
		s_addressbook_fields[i].generic.localdata[0] = i;
		s_addressbook_fields[i].cursor = (int)strlen(adr->string); // 0 in Q2
		s_addressbook_fields[i].length = 60;
		s_addressbook_fields[i].visible_length = 24; // 30 in Q2

		strcpy_s(s_addressbook_fields[i].buffer, sizeof(s_addressbook_fields[i].buffer), adr->string); //mxd. strcpy -> strcpy_s

		Menu_AddItem(&s_addressbook_menu, &s_addressbook_fields[i]);
	}

	Menu_Center(&s_addressbook_menu); // H2
}

static void AddressBook_MenuDraw(void)
{
	char title[MAX_QPATH];

	// Draw menu BG.
	re.BookDrawPic(0, 0, "book/back/b_conback8.bk", cls.m_menuscale);

	if (cls.m_menualpha == 0.0f)
		return;

	// Draw menu title.
	Com_sprintf(title, sizeof(title), "\x03%s", m_banner_address->string);
	const int x = M_GetMenuLabelX(re.BF_Strlen(title));
	const int y = M_GetMenuOffsetY(&s_addressbook_menu);
	re.DrawBigFont(x, y, title, cls.m_menualpha);

	// Draw menu items.
	s_addressbook_menu.x = M_GetMenuLabelX(s_addressbook_menu.width);
	Menu_Center(&s_addressbook_menu);
	Menu_Draw(&s_addressbook_menu);
}

// Q2 counterpart
static const char* AddressBook_MenuKey(const int key)
{
	// Save entered addresses?
	if (key == K_ESCAPE)
	{
		for (int i = 0; i < NUM_ADDRESSBOOK_ENTRIES; i++)
		{
			char buffer[20];
			Com_sprintf(buffer, sizeof(buffer), "adr%d", i);
			Cvar_Set(buffer, s_addressbook_fields[i].buffer);
		}
	}

	return Default_MenuKey(&s_addressbook_menu, key);
}

// Q2 counterpart
void M_Menu_AddressBook_f(void)
{
	AddressBook_MenuInit();
	M_PushMenu(AddressBook_MenuDraw, AddressBook_MenuKey);
}