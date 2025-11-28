//
// sc_Utility.h -- Utility logic shared by Script System.
//
// Copyright 1998 Raven Software
//

#pragma once

#include "sc_List.h"
#include "sc_Variable.h"
#include "sc_Signaler.h"

typedef enum RestoreListID_s
{
	RLID_UNDEFINED = -1, //mxd
	RLID_INTVAR = 1,
	RLID_FLOATVAR,
	RLID_VECTORVAR,
	RLID_ENTITYVAR,
	RLID_STRINGVAR,
	RLID_VARIABLEVAR,
	RLID_FIELDVARIABLEVAR,
	RLID_SIGNALER,
	RLID_MOVEDONEEVENT,
	RLID_ROTATEDONEEVENT,
	RLID_EXECUTEEVENT,
	RLID_WAITEVENT,
	RLID_SCRIPT,
	RLID_FIELDDEF,
} RestoreListID_t;

extern List<Variable*> GlobalVariables;
extern List<CScript*> Scripts;

extern void ReadEnt(edict_t** to, FILE* f);
extern void WriteEnt(edict_t** to, FILE* f);
extern void* RestoreObject(FILE* f, CScript* data);
extern void script_signaler(edict_t* which, SignalT signal_type);
extern "C" void animate_signaler(edict_t* which);
extern Variable* FindGlobal(const char* name);
extern bool NewGlobal(Variable* var);

template<class T> size_t tWrite(T* ptr, FILE* f, const int n = 1)
{
	return fwrite(ptr, n, sizeof(T), f);
}

template<class T> size_t tRead(T* ptr, FILE* f, const int n = 1)
{
	return fread(ptr, n, sizeof(T), f);
}