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

extern "C" void ShutdownScripts(const qboolean complete)
{
	while (Scripts.Size() > 0)
	{
		List<CScript*>::Iter script = Scripts.Begin();
		delete (*script);

		Scripts.Erase(script);
	}

	edict_t* ent = &g_edicts[0];
	for (int i = 0; i < globals.num_edicts; i++, ent++)
		ent->Script = nullptr;

	if (complete)
	{
		while (GlobalVariables.Size() > 0)
		{
			List<Variable*>::Iter var = GlobalVariables.Begin();
			delete (*var);

			GlobalVariables.Erase(var);
		}
	}
}

extern "C" void SaveScripts(FILE* f, const qboolean do_globals)
{
	constexpr int version = SCRIPT_SAVE_VERSION;
	fwrite(&version, 1, sizeof(version), f);

	if (do_globals)
	{
		const int size = GlobalVariables.Size();
		fwrite(&size, 1, sizeof(size), f);

		for (List<Variable*>::Iter var = GlobalVariables.Begin(); var != GlobalVariables.End(); ++var)
			(*var)->Write(f, nullptr);
	}
	else
	{
		const int size = Scripts.Size();
		fwrite(&size, 1, sizeof(size), f);

		for (List<CScript*>::Iter script = Scripts.Begin(); script != Scripts.End(); ++script)
			(*script)->Write(f);
	}
}

extern "C" void LoadScripts(FILE* f, const qboolean do_globals)
{
	int version;
	fread(&version, 1, sizeof(version), f);

	if (version != SCRIPT_SAVE_VERSION)
		gi.error("LoadScripts(): Expecting version %d, found version %d", SCRIPT_SAVE_VERSION, version);

	if (do_globals)
	{
		ShutdownScripts(true);

		int size;
		fread(&size, 1, sizeof(size), f);

		for (int i = 0; i < size; i++)
			GlobalVariables.PushBack(static_cast<Variable*>(RestoreObject(f, ScriptRL, nullptr)));
	}
	else
	{
		ShutdownScripts(false);

		edict_t* ent = &g_edicts[0];
		for (int i = 0; i < globals.num_edicts; i++, ent++)
			ent->Script = nullptr;

		int size;
		fread(&size, 1, sizeof(size), f);

		for (int i = 0; i < size; i++)
			Scripts.PushBack(static_cast<CScript*>(RestoreObject(f, ScriptRL, nullptr)));
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