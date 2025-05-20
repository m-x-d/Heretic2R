//
// sc_CScript.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "sc_Event.h"
#include "sc_FieldDef.h"
#include "sc_StringVar.h"
#include "sc_List.h"
#include "sc_Signaler.h"
#include "g_local.h"

#define MAX_INDEX	100

enum ScriptConditionT
{
	COND_READY,
	COND_COMPLETED,
	COND_SUSPENDED,
	COND_WAIT_ALL,
	COND_WAIT_ANY,
	COND_WAIT_TIME,
};

class Event;

class CScript
{
	char Name[MAX_PATH] = { 0 };
	byte* Data = nullptr;
	ScriptConditionT ScriptCondition = COND_COMPLETED;
	int ConditionInfo = 0;
	int Length = 0;
	int Position = 0;
	List<Variable*> LocalVariables;
	List<Variable*> ParameterVariables;
	List<Variable*> Stack;
	List<Signaler*> Signalers;
	List<Variable*> Waiting;
	List<StringVar*> ParameterValues;
	List<Event*> Events;
	Variable* VarIndex[MAX_INDEX] = { nullptr };
	FieldDef* Fields[MAX_INDEX] = { nullptr };
	edict_t* owner = nullptr;
	edict_t* other = nullptr;
	edict_t* activator = nullptr;
	int DebugFlags = 0;

	void Free(bool do_data);
	void Clear(bool do_data);

	Variable* ReadDeclaration(int& index);

	void PushStack(Variable* v);
	Variable* PopStack();

	void HandleGlobal(bool assignment);
	void HandleLocal(bool assignment);
	void HandleParameter(bool assignment);
	void HandleField();
	void HandleGoto();
	Variable* HandleSpawn();
	Variable* HandleBuiltinFunction();
	void HandlePush();
	void HandlePop();
	void HandleAssignment();
	void HandleAdd();
	void HandleSubtract();
	void HandleMultiply();
	void HandleDivide();
	void HandleDebug();
	void HandleDebugStatement();
	void HandleAddAssignment();
	void HandleSubtractAssignment();
	void HandleMultiplyAssignment();
	void HandleDivideAssignment();
	bool HandleWait(bool for_all);
	bool HandleTimeWait();
	void HandleIf();

	void HandlePrint();
	void HandlePlaySound();
	void HandleFeature(bool enable);
	void HandleCacheSound();

	void HandleMove();
	void HandleRotate();
	void HandleUse();
	void HandleTrigger(bool enable);
	void HandleAnimate();
	void HandleCopyPlayerAttributes();
	void HandleSetViewAngles();
	void HandleSetCacheSize();

	void Move(edict_t* ent, const vec3_t dest);
	void Rotate(edict_t* ent);

	void ProcessEvents();

	void AddSignaler(edict_t* edict, Variable* var, SignalT signal_type);
	void FinishWait(edict_t* which, bool execute);

	void Error(const char* format, ...);
	void StartDebug();
	void EndDebug();

	Variable* FindLocal(const char* var_name);
	bool NewLocal(Variable* which);
	Variable* FindParameter(const char* param_name);
	bool NewParameter(Variable* which);

public:
	CScript(const char* script_name, edict_t* new_owner);
	CScript(FILE* f);
	~CScript();

	void LoadFile();
	void Write(FILE* f);

	Variable* LookupVar(const int index) const
	{
		return VarIndex[index];
	}

	int LookupVarIndex(const Variable* var) const;

	void SetVarIndex(const int index, Variable* var)
	{
		VarIndex[index] = var;
	}

	FieldDef* LookupField(const int index) const
	{
		return Fields[index];
	}

	int LookupFieldIndex(const FieldDef* field) const;

	void SetFieldIndex(const int index, FieldDef* field)
	{
		Fields[index] = field;
	}

	void SetParameter(const char* value);

	byte ReadByte();
	int ReadInt();
	float ReadFloat();
	char* ReadString();

	void Move_Done(edict_t* ent);
	void Rotate_Done(edict_t* ent);

	void AddEvent(Event* which);
	void ClearTimeWait();
	void CheckSignalers(edict_t* which, SignalT signal_type);
	bool CheckWait();
	
	void DebugLine(const char* format, ...);

	void Think();
	ScriptConditionT Execute(edict_t* new_other, edict_t* new_activator);
};