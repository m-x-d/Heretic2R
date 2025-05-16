//
// sc_Utility.h -- Utility logic shared by Script System.
//
// Copyright 1998 Raven Software
//

#pragma once

#include "sc_List.h"
#include "sc_Variable.h" //mxd
#include "sc_Signaler.h"

#define RLID_INTVAR				1
#define RLID_FLOATVAR			2
#define RLID_VECTORVAR			3
#define RLID_ENTITYVAR			4
#define RLID_STRINGVAR			5
#define RLID_VARIABLEVAR		6
#define RLID_FIELDVARIABLEVAR	7
#define RLID_SIGNALER			8
#define RLID_MOVEDONEEVENT		9
#define RLID_ROTATEDONEEVENT	10
#define RLID_EXECUTEEVENT		11
#define RLID_WAITEVENT			12
#define RLID_SCRIPT				13
#define RLID_FIELDDEF			14

typedef struct RestoreList_s
{
	int ID;
	void* (*alloc_func)(FILE*, void*);
} RestoreList_t;

extern List<Variable*> GlobalVariables;
extern List<CScript*> Scripts;
extern RestoreList_t ScriptRL[];

extern void ReadEnt(edict_t** to, FILE* f);
extern void WriteEnt(edict_t** to, FILE* f);
extern void* RestoreObject(FILE* f, RestoreList_t* list, void* data);
extern void script_signaler(edict_t* which, SignalT SignalType);
extern void animate_signaler(edict_t* which);
extern Variable* FindGlobal(char* Name);
extern bool NewGlobal(Variable* Which);

template<class T> size_t tWrite(T* ptr, FILE* FH, int n = 1)
{
	return fwrite(ptr, n, sizeof(T), FH);
}

template<class T> size_t tRead(T* ptr, FILE* FH, int n = 1)
{
	return fread(ptr, n, sizeof(T), FH);
}