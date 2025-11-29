//
// sc_Main.cpp
//
// Copyright 1998 Raven Software
//

#include "sc_Main.h"
#include "sc_CScript.h"
#include "sc_ExecuteEvent.h"
#include "sc_Utility.h"

#define SCRIPT_SAVE_VERSION 2

extern "C" void ProcessScripts(void)
{
	for (CScript* script : Scripts)
		script->Think();
}

extern "C" void ShutdownScripts(const qboolean complete)
{
	for (const CScript* script : Scripts)
		delete script;
	Scripts.clear();

	edict_t* ent = &g_edicts[0];
	for (int i = 0; i < globals.num_edicts; i++, ent++)
		ent->Script = nullptr;

	if (complete)
	{
		for (const auto& pair : GlobalVariables)
			delete pair.second;
		GlobalVariables.clear();
	}
}

extern "C" void SaveScripts(FILE* f, const qboolean do_globals)
{
	constexpr int version = SCRIPT_SAVE_VERSION;
	fwrite(&version, 1, sizeof(version), f);

	if (do_globals)
	{
		const uint size = GlobalVariables.size();
		fwrite(&size, 1, sizeof(size), f);

		for (const auto& pair : GlobalVariables)
			pair.second->Write(f, nullptr);
	}
	else
	{
		const uint size = Scripts.size();
		fwrite(&size, 1, sizeof(size), f);

		for (CScript* script : Scripts)
			script->Write(f);
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

		uint size;
		fread(&size, 1, sizeof(size), f);

		for (uint i = 0; i < size; i++)
		{
			Variable* var = static_cast<Variable*>(RestoreObject(f, nullptr));
			GlobalVariables[var->GetName()] = var;
		}
	}
	else
	{
		ShutdownScripts(false);

		edict_t* ent = &g_edicts[0];
		for (int i = 0; i < globals.num_edicts; i++, ent++)
			ent->Script = nullptr;

		uint size;
		fread(&size, 1, sizeof(size), f);

		for (uint i = 0; i < size; i++)
			Scripts.push_back(static_cast<CScript*>(RestoreObject(f, nullptr)));
	}
}

extern "C" void ScriptUse(edict_t* ent, edict_t* other, edict_t* activator) //mxd. Named 'ScriptUse' in original logic.
{
	ent->Script->AddEvent(new ExecuteEvent(level.time, other, activator));
}

// QUAKED script_runner (.5 .5 .5) (-8 -8 -8) (8 8 8)
// Set Script to the name of the script to run when triggered.
// Use parm1 through parm16 to send parameters to the script.
extern "C" void SP_script_runner(edict_t* ent)
{
	char script_path[MAX_OSPATH]; //mxd. MAX_PATH in original logic.
	sprintf_s(script_path, "ds/%s.os", st.script); //mxd. sprintf -> sprintf_s.

	ent->Script = new CScript(script_path, ent);
	Scripts.push_back(ent->Script);

	for (uint i = 0; i < std::size(st.parms) && st.parms[i] != nullptr; i++)
		ent->Script->SetParameter(st.parms[i]);

	ent->movetype = PHYSICSTYPE_NONE;
	ent->solid = SOLID_NOT;
	ent->svflags |= SVF_NOCLIENT;
	ent->use = ScriptUse;
}