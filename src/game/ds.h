#ifndef __DS
#define __DS

#include "g_save.h" //mxd
#include "pcode.h"
#include "list.h"

class CScript;

enum ScriptConditionT
{ 
	COND_READY,
	COND_COMPLETED, 
	COND_SUSPENDED, 
	COND_WAIT_ALL, 
	COND_WAIT_ANY,
	COND_WAIT_TIME,
};

#define MAX_INDEX		100

#define INSTRUCTION_MAX 500

//==========================================================================

class Variable
{
protected:
	char		Name[VAR_LENGTH];
	VariableType	Type;

public:
						Variable(char *NewName = "", VariableType NewType = TYPE_UNKNOWN);
						Variable(FILE *FH, CScript *Script);
	virtual void		Write(FILE *FH, CScript *Script, int ID = -1);
	char				*GetName(void) { return Name; }
	VariableType			GetType(void) { return Type; }
	virtual int			GetIntValue(void) { return 0; }
	virtual float		GetFloatValue(void) { return 0.0; }
	virtual void		GetVectorValue(vec3_t &VecValue) { VecValue[0] = VecValue[1] = VecValue[2] = 0.0; }
	virtual edict_t		*GetEdictValue(void) { return NULL; }
	virtual char		*GetStringValue(void) { return ""; }
	virtual void		ReadValue(CScript *Script) {}
	virtual void		Debug(CScript *Script);
	virtual void		Signal(edict_t *Which) { }
	virtual void		ClearSignal(void) { }

	virtual Variable *operator	+(Variable *VI) { return NULL; }
	virtual Variable *operator	-(Variable *VI) { return NULL; }
	virtual Variable *operator	*(Variable *VI) { return NULL; }
	virtual Variable *operator	/(Variable *VI) { return NULL; }
	virtual void	operator	=(Variable *VI) { }
	virtual bool	operator	==(Variable *VI) { return false; }
	virtual bool	operator	!=(Variable *VI) { return false; }
	virtual bool	operator	<(Variable *VI) { return false; }
	virtual bool	operator	<=(Variable *VI) { return false; }
	virtual bool	operator	>(Variable *VI) { return false; }
	virtual bool	operator	>=(Variable *VI) { return false; }
};

//==========================================================================

class IntVar : public Variable
{
protected:
	int	Value;

public:
						IntVar(char *Name = "", int InitValue = 0);
						IntVar(FILE *FH, CScript *Script);
	virtual void		Write(FILE *FH, CScript *Script, int ID = -1);
	virtual int			GetIntValue(void) { return Value; }
	virtual float		GetFloatValue(void) { return (float)Value; }
	virtual void		ReadValue(CScript *Script);
	virtual void		Debug(CScript *Script);
	virtual void		Signal(edict_t *Which);
	virtual void		ClearSignal(void);

	virtual Variable *operator	+(Variable *VI);
	virtual Variable *operator	-(Variable *VI);
	virtual Variable *operator	*(Variable *VI);
	virtual Variable *operator	/(Variable *VI);
	virtual void	operator	=(Variable *VI);

	virtual bool	operator	==(Variable *VI) { return Value == VI->GetIntValue(); }
	virtual bool	operator	!=(Variable *VI) { return Value != VI->GetIntValue(); }
	virtual bool	operator	<(Variable *VI) { return Value < VI->GetIntValue(); }
	virtual bool	operator	<=(Variable *VI) { return Value <= VI->GetIntValue(); }
	virtual bool	operator	>(Variable *VI) { return Value > VI->GetIntValue(); }
	virtual bool	operator	>=(Variable *VI) { return Value >= VI->GetIntValue(); }
};

//==========================================================================

class FloatVar : public Variable
{
protected:
	float	Value;

public:
						FloatVar(char *Name = "", float InitValue = 0.0);
						FloatVar(FILE *FH, CScript *Script);
	virtual void		Write(FILE *FH, CScript *Script, int ID = -1);
	virtual int			GetIntValue(void) { return (int)Value; }
	virtual float		GetFloatValue(void) { return Value; }
	virtual void		ReadValue(CScript *Script);
	virtual void		Debug(CScript *Script);

	virtual Variable *operator	+(Variable *VI);
	virtual Variable *operator	-(Variable *VI);
	virtual Variable *operator	*(Variable *VI);
	virtual Variable *operator	/(Variable *VI);
	virtual void	operator	=(Variable *VI);

	virtual bool	operator	==(Variable *VI) { return Value == VI->GetFloatValue(); }
	virtual bool	operator	!=(Variable *VI) { return Value != VI->GetFloatValue(); }
	virtual bool	operator	<(Variable *VI) { return Value < VI->GetFloatValue(); }
	virtual bool	operator	<=(Variable *VI) { return Value <= VI->GetFloatValue(); }
	virtual bool	operator	>(Variable *VI) { return Value > VI->GetFloatValue(); }
	virtual bool	operator	>=(Variable *VI) { return Value >= VI->GetFloatValue(); }
};

//==========================================================================

class VectorVar : public Variable
{
protected:
	vec3_t	Value;

public:
						VectorVar(char *Name = "", float InitValueX = 0.0, float InitValueY = 0.0, float InitValueZ = 0.0);
						VectorVar(vec3_t NewValue);
						VectorVar(FILE *FH, CScript *Script);
	virtual void		Write(FILE *FH, CScript *Script, int ID = -1);
	virtual void		GetVectorValue(vec3_t &VecValue);
	virtual void		ReadValue(CScript *Script);
	virtual void		Debug(CScript *Script);

	virtual Variable *operator	+(Variable *VI);
	virtual Variable *operator	-(Variable *VI);
	virtual Variable *operator	*(Variable *VI);
	virtual Variable *operator	/(Variable *VI);
	virtual void	operator	=(Variable *VI);
	virtual bool	operator	==(Variable *VI);
	virtual bool	operator	!=(Variable *VI);
	virtual bool	operator	<(Variable *VI);
	virtual bool	operator	<=(Variable *VI);
	virtual bool	operator	>(Variable *VI);
	virtual bool	operator	>=(Variable *VI);
};

//==========================================================================

class EntityVar : public Variable
{
protected:
	edict_t *Value;

public:
						EntityVar(char *Name = "", int InitValue = 0);
						EntityVar(edict_t *Which);
						EntityVar(FILE *FH, CScript *Script);
	virtual void		Write(FILE *FH, CScript *Script, int ID = -1);
	virtual int			GetIntValue(void);
	virtual edict_t		*GetEdictValue(void) { return Value; }
	virtual void		ReadValue(CScript *Script);
	virtual void		Debug(CScript *Script);

	virtual void	operator	=(Variable *VI);
	virtual bool	operator	==(Variable *VI);
	virtual bool	operator	!=(Variable *VI);
};

//==========================================================================

class StringVar : public Variable
{
protected:
	char Value[VAR_LENGTH];

public:
						StringVar(char *Name = "", char *InitValue = "");
						StringVar(FILE *FH, CScript *Script);
	virtual void		Write(FILE *FH, CScript *Script, int ID = -1);
	virtual char		*GetStringValue(void) { return Value; }
	virtual void		ReadValue(CScript *Script);
};

//==========================================================================

class VariableVar : public Variable
{
protected:
	Variable		*Value;

public:
						VariableVar(char *Name = "");
						VariableVar(FILE *FH, CScript *Script);
	virtual void		Write(FILE *FH, CScript *Script, int ID = -1);
	virtual int			GetIntValue(void) { return Value->GetIntValue(); }
	virtual float		GetFloatValue(void) { return Value->GetFloatValue(); }
	virtual void		GetVectorValue(vec3_t &VecValue) { Value->GetVectorValue(VecValue); }
	virtual edict_t		*GetEdictValue(void) { return Value->GetEdictValue(); }
	virtual char		*GetStringValue(void) { return Value->GetStringValue(); }
	virtual void		ReadValue(CScript *Script);
	virtual void		Debug(CScript *Script);
	virtual void		Signal(edict_t *Which) { Value->Signal(Which); }
	virtual void		ClearSignal(void) { Value->ClearSignal(); }

	virtual Variable *operator	+(Variable *VI) { return (*Value) + VI; }
	virtual Variable *operator	-(Variable *VI) { return (*Value) - VI; }
	virtual Variable *operator	*(Variable *VI) { return (*Value) * VI; }
	virtual Variable *operator	/(Variable *VI) { return (*Value) / VI; }
	virtual void	operator	=(Variable *VI) { (*Value) = VI; }

	virtual bool	operator	==(Variable *VI) { return (*Value) == VI; }
	virtual bool	operator	!=(Variable *VI) { return (*Value) != VI; }
	virtual bool	operator	<(Variable *VI) { return (*Value) < VI; }
	virtual bool	operator	<=(Variable *VI) { return (*Value) <= VI; }
	virtual bool	operator	>(Variable *VI) { return (*Value) > VI; }
	virtual bool	operator	>=(Variable *VI) { return (*Value) >= VI; }
};

//==========================================================================

class FieldDef;

class FieldVariableVar : public Variable
{
protected:
	Variable		*Value;
	FieldDef		*Field;

public:
						FieldVariableVar(char *Name = "");
						FieldVariableVar(FILE *FH, CScript *Script);
	virtual void		Write(FILE *FH, CScript *Script, int ID = -1);
	virtual int			GetIntValue(void);
	virtual float		GetFloatValue(void);
	virtual void		GetVectorValue(vec3_t &VecValue);
	virtual edict_t		*GetEdictValue(void);
	virtual char		*GetStringValue(void);
	virtual void		ReadValue(CScript *Script);
	virtual void		Debug(CScript *Script);
	virtual void		Signal(edict_t *Which) { Value->Signal(Which); }
	virtual void		ClearSignal(void) { Value->ClearSignal(); }

	virtual Variable *operator	+(Variable *VI);
	virtual Variable *operator	-(Variable *VI);
	virtual Variable *operator	*(Variable *VI);
	virtual Variable *operator	/(Variable *VI);
	virtual void	operator	=(Variable *VI);

	virtual bool	operator	==(Variable *VI);
	virtual bool	operator	!=(Variable *VI);
	virtual bool	operator	<(Variable *VI);
	virtual bool	operator	<=(Variable *VI);
	virtual bool	operator	>(Variable *VI);
	virtual bool	operator	>=(Variable *VI);
};

//==========================================================================

enum SignalT
{
	SIGNAL_MOVE,
	SIGNAL_ROTATE,
	SIGNAL_ANIMATE,
};

class Signaler
{
private:
	edict_t		*Edict;
	Variable	*Var;
	SignalT		SignalType;

public:
								Signaler(edict_t *NewEdict, Variable *NewVar, SignalT NewSignalType);
								Signaler(FILE *FH, CScript *Script);
								~Signaler(void);
	virtual void				Write(FILE *FH, CScript *Script);
			bool				Test(edict_t *Which, SignalT WhichType);
			edict_t				*GetEdict(void) { return Edict; }
			Variable			*GetVar(void) { return Var; }
			SignalT				GetType(void) { return SignalType; }
			bool	operator	==(Signaler *SI);
};

//==========================================================================

class FieldDef
{
private:
	char			Name[VAR_LENGTH];
	VariableType		Type;
	int				Offset;
	fieldtype_t		FieldType;

public:
						FieldDef(CScript *Script);
						FieldDef(FILE *FH, CScript *Script);
			void		Write(FILE *FH, CScript *Script);
			byte		*GetOffset(Variable *Var);
			Variable	*GetValue(Variable *Var);
			int			GetIntValue(Variable *Var);
			float		GetFloatValue(Variable *Var);
			void		GetVectorValue(Variable *Var, vec3_t &VecValue);
			edict_t		*GetEdictValue(Variable *Var);
			char		*GetStringValue(Variable *Var);
			void		SetValue(Variable *Var, Variable *Value);
};

//==========================================================================

enum EventT
{
	EVENT_MOVE_DONE,
	EVENT_ROTATE_DONE,
	EVENT_SCRIPT_WAIT,
	EVENT_SCRIPT_EXECUTE,
};

class Event
{
protected:
	float		Time;
	EventT		Type;
	int			Priority;

public:
						Event(float NewTime, EventT NewType);
						Event(FILE *FH, CScript *Script);
	virtual void		Write(FILE *FH, CScript *Script, int ID = -1);
			float		GetTime(void) { return Time; }
			EventT		GetType(void) { return Type; }
			int			GetPriority(void) { return Priority; }
	virtual bool		Process(CScript *Script);
};

//==========================================================================

class MoveDoneEvent : public Event
{
private:
	edict_t	*Ent;

public:
						MoveDoneEvent(float NewTime, edict_t *NewEnt);
						MoveDoneEvent(FILE *FH, CScript *Script);
	virtual void		Write(FILE *FH, CScript *Script, int ID = -1);
	virtual bool		Process(CScript *Script);
};

//==========================================================================

class RotateDoneEvent : public Event
{
private:
	edict_t	*Ent;

public:
						RotateDoneEvent(float NewTime, edict_t *NewEnt);
						RotateDoneEvent(FILE *FH, CScript *Script);
	virtual void		Write(FILE *FH, CScript *Script, int ID = -1);
	virtual bool		Process(CScript *Script);
};

//==========================================================================

class ExecuteEvent : public Event
{
private:
	edict_t	*Other;
	edict_t	*Activator;

public:
						ExecuteEvent(float NewTime, edict_t *NewOther = NULL, edict_t *NewActivator = NULL);
						ExecuteEvent(FILE *FH, CScript *Script);
	virtual void		Write(FILE *FH, CScript *Script, int ID = -1);
	virtual bool		Process(CScript *Script);
};

//==========================================================================

class WaitEvent : public Event
{
public:
						WaitEvent(float NewTime);
						WaitEvent(FILE *FH, CScript *Script);
	virtual void		Write(FILE *FH, CScript *Script, int ID = -1);
	virtual bool		Process(CScript *Script);
};

//==========================================================================

class CScript 
{
private:
	char				Name[MAX_PATH];
	unsigned char		*Data;
	ScriptConditionT	ScriptCondition;
	int					ConditionInfo;
	int					Length;
	int					Position;
	List<Variable *>	LocalVariables;
	List<Variable *>	ParameterVariables;
	List<Variable *>	Stack;
	List<Signaler *>	Signalers;
	List<Variable *>	Waiting;
	List<StringVar *>   ParameterValues;
	List<Event *>		Events;
	Variable			*VarIndex[MAX_INDEX];
	FieldDef			*Fields[MAX_INDEX];
	edict_t				*owner, *other, *activator;
	int					DebugFlags;

public:
						CScript(char *ScriptName, edict_t *new_owner);
						CScript(FILE *FH);
						~CScript(void);

	void				LoadFile(void);
	void				Free(bool DoData);
	void				Clear(bool DoData);
	void				Write(FILE *FH);

	Variable			*LookupVar(int Index) { return VarIndex[Index]; }
	int					LookupVarIndex(Variable *Var);
	void				SetVarIndex(int Index, Variable *Var) { VarIndex[Index] = Var; }
	FieldDef			*LookupField(int Index) { return Fields[Index]; }
	int					LookupFieldIndex(FieldDef *Field);
	void				SetFieldIndex(int Index, FieldDef *Field) { Fields[Index] = Field; }
	void				SetParameter(char *Value);

	unsigned char		ReadByte(void);
	int					ReadInt(void);
	float				ReadFloat(void);
	char				*ReadString(void);
	Variable			*ReadDeclaration(int &Index);

	void				PushStack(Variable *VI);
	Variable			*PopStack(void);

	void				HandleGlobal(bool Assignment);
	void				HandleLocal(bool Assignment);
	void				HandleParameter(bool Assignment);
	void				HandleField(void);
	void				HandleGoto(void);
	Variable			*HandleSpawn(void);
	Variable			*HandleBuiltinFunction(void);
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

	void				Move_Done(edict_t *ent);
	void				Move(edict_t *ent, vec3_t Dest);
	void				Rotate_Done(edict_t *ent);
	void				Rotate(edict_t *ent);

	void				AddEvent(Event *Which);
	void				ProcessEvents(void);
	void				ClearTimeWait(void);

	void				AddSignaler(edict_t *Edict, Variable *Var, SignalT SignalType);
	void				CheckSignalers(edict_t *Which, SignalT SignalType);
	bool				CheckWait(void);
	void				FinishWait(edict_t *Which, bool NoExecute);
	void				Error (char *error, ...);
	void				StartDebug(void);
	void				EndDebug(void);
	void				DebugLine (char *debugtext, ...);

	void				Think(void);
	ScriptConditionT	Execute(edict_t *new_other, edict_t *new_activator);

	Variable			*FindLocal(char *Name);
	bool				NewLocal(Variable *Which);
	Variable			*FindParameter(char *Name);
	bool				NewParameter(Variable *Which);
};

//==========================================================================

extern List<Variable *> GlobalVariables;

#endif
