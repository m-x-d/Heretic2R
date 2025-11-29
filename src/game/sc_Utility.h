//
// sc_Utility.h -- Utility logic shared by Script System.
//
// Copyright 1998 Raven Software
//

#pragma once

#include <list>
#include <string>
#include <unordered_map>
#include "sc_Variable.h"
#include "sc_Signaler.h"

extern std::unordered_map<std::string, Variable*> GlobalVariables;
extern std::list<CScript*> Scripts;

extern void ReadEnt(edict_t** to, FILE* f);
extern void WriteEnt(edict_t** to, FILE* f);
extern void* RestoreObject(FILE* f, CScript* data);
extern void script_signaler(edict_t* which, SignalT signal_type);
extern "C" void animate_signaler(edict_t* which);
extern Variable* FindGlobal(const char* name);
extern bool NewGlobal(Variable* var);