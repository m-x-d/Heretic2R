//
// sc_Main.cpp
//
// Copyright 1998 Raven Software
//

#include "sc_Main.h"
#include "sc_CScript.h"
#include "sc_ExecuteEvent.h"
#include "sc_List.h"
#include "sc_Utility.h"

#define SCRIPT_SAVE_VERSION 2

extern "C" void ProcessScripts(void)
{
	if (Scripts.Size() > 0)
		for (List<CScript*>::Iter is = Scripts.Begin(); is != Scripts.End(); ++is)
			(*is)->Think();
}

extern "C" void ShutdownScripts(qboolean Complete)
{
	List<CScript*>::Iter	is;
	List<Variable*>::Iter	iv;
	int						i;
	edict_t* ent;

	while (Scripts.Size())
	{
		is = Scripts.Begin();
		delete (*is);

		Scripts.Erase(is);
	}

	for (i = 0, ent = g_edicts; i < globals.num_edicts; i++, ent++)
	{
		ent->Script = NULL;
	}

	if (Complete)
	{
		while (GlobalVariables.Size())
		{
			iv = GlobalVariables.Begin();
			delete (*iv);

			GlobalVariables.Erase(iv);
		}
	}
}

extern "C" void SaveScripts(FILE* FH, qboolean DoGlobals)
{
	int						size;
	List<CScript*>::Iter	is;
	List<Variable*>::Iter	iv;

	size = SCRIPT_SAVE_VERSION;
	fwrite(&size, 1, sizeof(size), FH);

	if (DoGlobals)
	{
		size = GlobalVariables.Size();
		fwrite(&size, 1, sizeof(size), FH);

		for (iv = GlobalVariables.Begin(); iv != GlobalVariables.End(); iv++)
		{
			(*iv)->Write(FH, NULL);
		}
	}
	else
	{
		size = Scripts.Size();
		fwrite(&size, 1, sizeof(size), FH);

		for (is = Scripts.Begin(); is != Scripts.End(); is++)
		{
			(*is)->Write(FH);
		}
	}
}

extern "C" void LoadScripts(FILE* FH, qboolean DoGlobals)
{
	int		size, i;
	edict_t* ent;

	fread(&size, 1, sizeof(size), FH);
	if (size != SCRIPT_SAVE_VERSION)
	{
		gi.error("LoadScripts(): Expecting version %d, found version %d", SCRIPT_SAVE_VERSION, size);
	}

	if (DoGlobals)
	{
		ShutdownScripts(true);

		fread(&size, 1, sizeof(size), FH);

		for (i = 0; i < size; i++)
		{
			GlobalVariables.PushBack((Variable*)RestoreObject(FH, ScriptRL, NULL));
		}
	}
	else
	{
		ShutdownScripts(false);

		for (i = 0, ent = g_edicts; i < globals.num_edicts; i++, ent++)
		{
			ent->Script = NULL;
		}

		fread(&size, 1, sizeof(size), FH);

		for (i = 0; i < size; i++)
		{
			Scripts.PushBack((CScript*)RestoreObject(FH, ScriptRL, NULL));
		}
	}
}

void script_use(edict_t* ent, edict_t* other, edict_t* activator)
{
	ent->Script->AddEvent(new ExecuteEvent(level.time, other, activator));
}

/*QUAKED script_runner (.5 .5 .5) (-8 -8 -8) (8 8 8)
set Script to the name of the script to run when triggered
use parm1 through parm16 to send parameters to the script
*/
extern "C" void SP_script_runner(edict_t* ent)
{
	char	temp[MAX_PATH];
	int		i;

	sprintf(temp, "ds/%s.os", st.script);
	ent->Script = new CScript(temp, ent);
	Scripts.PushBack(ent->Script);

	for (i = 0; i < NUM_PARMS; i++)
	{
		if (st.parms[i])
		{
			ent->Script->SetParameter(st.parms[i]);
		}
		else
		{
			break;
		}
	}

	ent->movetype = PHYSICSTYPE_NONE;
	ent->solid = SOLID_NOT;
	ent->svflags |= SVF_NOCLIENT;
	ent->use = script_use;

	//	gi.setmodel (ent, ent->model);
	//	gi.linkentity (ent);
}

/*QUAKE script_parms (.5 .5 .5) ?
target the script_runner object
use parm1 through parm16 to send parameters to the script
*/
extern "C" void SP_parms(edict_t* ent)
{
}