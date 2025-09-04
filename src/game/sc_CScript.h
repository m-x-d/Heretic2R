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
	char name[MAX_PATH] = {};
	byte* data = nullptr;
	ScriptConditionT script_condition = COND_COMPLETED;
	int condition_info = 0;
	int length = 0;
	int position = 0;
	List<Variable*> local_variables;
	List<Variable*> parameter_variables;
	List<Variable*> stack_variables;
	List<Variable*> waiting_variables;
	List<Signaler*> signalers;
	List<StringVar*> parameter_values;
	List<Event*> events;
	Variable* variable_index[MAX_INDEX] = { nullptr };
	FieldDef* fielddefs[MAX_INDEX] = { nullptr };
	edict_t* owner = nullptr;
	edict_t* other = nullptr;
	edict_t* activator = nullptr;
	int debug_flags = 0;

	void Free(bool do_data);
	void Clear();

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

	[[noreturn]] void Error(const char* format, ...);
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
		return variable_index[index];
	}

	int LookupVarIndex(const Variable* var) const;

	void SetVarIndex(const int index, Variable* var)
	{
		variable_index[index] = var;
	}

	FieldDef* LookupField(const int index) const
	{
		return fielddefs[index];
	}

	int LookupFieldIndex(const FieldDef* field) const;

	void SetFieldIndex(const int index, FieldDef* field)
	{
		fielddefs[index] = field;
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