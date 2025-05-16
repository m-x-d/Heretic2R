//
// sc_CScript.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include <stdio.h>
#include "sc_Event.h"
#include "sc_FieldDef.h"
#include "sc_StringVar.h"
#include "sc_List.h"
#include "sc_Signaler.h"

#define MAX_INDEX		100
#define INSTRUCTION_MAX 500

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
private:
	char				Name[260]; //TODO: MAX_PATH in original logic.
	unsigned char* Data;
	ScriptConditionT	ScriptCondition;
	int					ConditionInfo;
	int					Length;
	int					Position;
	List<Variable*>	LocalVariables;
	List<Variable*>	ParameterVariables;
	List<Variable*>	Stack;
	List<Signaler*>	Signalers;
	List<Variable*>	Waiting;
	List<StringVar*>   ParameterValues;
	List<Event*>		Events;
	Variable* VarIndex[MAX_INDEX];
	FieldDef* Fields[MAX_INDEX];
	edict_t* owner, * other, * activator;
	int					DebugFlags;

public:
	CScript(char* ScriptName, edict_t* new_owner);
	CScript(FILE* FH);
	~CScript(void);

	void				LoadFile(void);
	void				Free(bool DoData);
	void				Clear(bool DoData);
	void				Write(FILE* FH);

	Variable* LookupVar(int Index) { return VarIndex[Index]; }
	int					LookupVarIndex(Variable* Var);
	void				SetVarIndex(int Index, Variable* Var) { VarIndex[Index] = Var; }
	FieldDef* LookupField(int Index) { return Fields[Index]; }
	int					LookupFieldIndex(FieldDef* Field);
	void				SetFieldIndex(int Index, FieldDef* Field) { Fields[Index] = Field; }
	void				SetParameter(char* Value);

	unsigned char		ReadByte(void);
	int					ReadInt(void);
	float				ReadFloat(void);
	char* ReadString(void);
	Variable* ReadDeclaration(int& Index);

	void				PushStack(Variable* VI);
	Variable* PopStack(void);

	void				HandleGlobal(bool Assignment);
	void				HandleLocal(bool Assignment);
	void				HandleParameter(bool Assignment);
	void				HandleField(void);
	void				HandleGoto(void);
	Variable* HandleSpawn(void);
	Variable* HandleBuiltinFunction(void);
	void				HandlePush(void);
	void				HandlePop(void);
	void				HandleAssignment(void);
	void				HandleAdd(void);
	void				HandleSubtract(void);
	void				HandleMultiply(void);
	void				HandleDivide(void);
	void				HandleDebug(void);
	void				HandleDebugStatement(void);
	void				HandleAddAssignment(void);
	void				HandleSubtractAssignment(void);
	void				HandleMultiplyAssignment(void);
	void				HandleDivideAssignment(void);
	bool				HandleWait(bool ForAll);
	bool				HandleTimeWait(void);
	void				HandleIf(void);

	void				HandlePrint(void);
	void				HandlePlaySound(void);
	void				HandleFeature(bool Enable);
	void				HandleCacheSound(void);

	void				HandleMove(void);
	void				HandleRotate(void);
	void				HandleUse(void);
	void				HandleTrigger(bool Enable);
	void				HandleAnimate(void);
	void				HandleCopyPlayerAttributes(void);
	void				HandleSetViewAngles(void);
	void				HandleSetCacheSize(void);

	void				Move_Done(edict_t* ent);
	void				Move(edict_t* ent, vec3_t Dest);
	void				Rotate_Done(edict_t* ent);
	void				Rotate(edict_t* ent);

	void				AddEvent(Event* Which);
	void				ProcessEvents(void);
	void				ClearTimeWait(void);

	void				AddSignaler(edict_t* Edict, Variable* Var, SignalT SignalType);
	void				CheckSignalers(edict_t* Which, SignalT SignalType);
	bool				CheckWait(void);
	void				FinishWait(edict_t* Which, bool NoExecute);
	void				Error(char* error, ...);
	void				StartDebug(void);
	void				EndDebug(void);
	void				DebugLine(char* debugtext, ...);

	void				Think(void);
	ScriptConditionT	Execute(edict_t* new_other, edict_t* new_activator);

	Variable* FindLocal(char* Name);
	bool				NewLocal(Variable* Which);
	Variable* FindParameter(char* Name);
	bool				NewParameter(Variable* Which);
};
