//
// sc_CScript.cpp
//
// Copyright 1998 Raven Software
//

#include "sc_CScript.h"
#include "c_ai.h"
#include "sc_EntityVar.h"
#include "sc_FieldVariableVar.h"
#include "sc_FloatVar.h"
#include "sc_IntVar.h"
#include "sc_MoveDoneEvent.h"
#include "sc_RotateDoneEvent.h"
#include "sc_VariableVar.h"
#include "sc_VectorVar.h"
#include "sc_WaitEvent.h"
#include "sc_Utility.h"
#include "Vector.h"
#include "g_local.h"

static int msg_animtype[NUM_MESSAGES] =
{
	MSG_C_ACTION1,
	MSG_C_ACTION2,
	MSG_C_ACTION3,
	MSG_C_ACTION4,
	MSG_C_ACTION5,
	MSG_C_ACTION6,
	MSG_C_ACTION7,
	MSG_C_ACTION8,
	MSG_C_ACTION9,
	MSG_C_ACTION10,
	MSG_C_ACTION11,
	MSG_C_ACTION12,
	MSG_C_ACTION13,
	MSG_C_ACTION14,
	MSG_C_ACTION15,
	MSG_C_ACTION16,
	MSG_C_ACTION17,
	MSG_C_ACTION18,
	MSG_C_ACTION19,
	MSG_C_ACTION20,
	MSG_C_ATTACK1,
	MSG_C_ATTACK2,
	MSG_C_ATTACK3,
	MSG_C_BACKPEDAL1,
	MSG_C_DEATH1,
	MSG_C_DEATH2,
	MSG_C_DEATH3,
	MSG_C_DEATH4,
	MSG_C_GIB1,
	MSG_C_IDLE1,
	MSG_C_IDLE2,
	MSG_C_IDLE3,
	MSG_C_IDLE4,
	MSG_C_IDLE5,
	MSG_C_IDLE6,
	MSG_C_JUMP1,
	MSG_C_PAIN1,
	MSG_C_PAIN2,
	MSG_C_PAIN3,
	MSG_C_PIVOTLEFTGO,
	MSG_C_PIVOTLEFT,
	MSG_C_PIVOTLEFTSTOP,
	MSG_C_PIVOTRIGHTGO,
	MSG_C_PIVOTRIGHT,
	MSG_C_PIVOTRIGHTSTOP,
	MSG_C_RUN1,
	MSG_C_STEPLEFT,
	MSG_C_STEPRIGHT,
	MSG_C_THINKAGAIN,
	MSG_C_TRANS1,
	MSG_C_TRANS2,
	MSG_C_TRANS3,
	MSG_C_TRANS4,
	MSG_C_TRANS5,
	MSG_C_TRANS6,
	MSG_C_WALKSTART,
	MSG_C_WALK1,
	MSG_C_WALK2,
	MSG_C_WALK3,
	MSG_C_WALK4,
	MSG_C_WALKSTOP1,
	MSG_C_WALKSTOP2,
	MSG_C_ATTACK4,
	MSG_C_ATTACK5,
};

#define MAX_CINEMATIC_SOUNDS	255 //mxd. Named 'MAX_CINESNDS' in original logic.
#define MAX_INSTRUCTIONS		500 //mxd. Named 'INSTRUCTION_MAX' in original logic.

typedef struct CinematicSound_s
{
	edict_t* ent;
	int channel;
} CinematicSound_t;

static CinematicSound_t cinematic_sounds[MAX_CINEMATIC_SOUNDS]; //mxd. Named 'CinematicSound' in original logic.
static int cinematic_sounds_count; // Count of the current # of sounds executed. //mxd. Named 'CinematicSound_cnt' in original logic.

extern "C"
{
	extern void TriggerMultipleUse(edict_t* self, edict_t* other, edict_t* activator);
	extern cvar_t* Cvar_Set(const char* var_name, const char* value);
}

CScript::CScript(const char* ScriptName, edict_t* new_owner)
{
	Clear(true);

	owner = new_owner;
	strcpy(name, ScriptName);

	LoadFile();
}

CScript::CScript(FILE* FH)
{
	int						index;
	int						size;
	int						i;
	char					name[VAR_LENGTH];

	Clear(true);

	fread(name, 1, sizeof(name), FH);
	LoadFile();

	fread(&script_condition, 1, sizeof(script_condition), FH);
	fread(&condition_info, 1, sizeof(condition_info), FH);
	fread(&length, 1, sizeof(length), FH);
	fread(&position, 1, sizeof(position), FH);
	fread(&debug_flags, 1, sizeof(debug_flags), FH);

	fread(&index, 1, sizeof(index), FH);
	if (index != -1)
	{
		owner = &g_edicts[index];
		owner->Script = this;
	}

	fread(&index, 1, sizeof(index), FH);
	if (index != -1)
	{
		other = &g_edicts[index];
	}

	fread(&index, 1, sizeof(index), FH);
	if (index != -1)
	{
		activator = &g_edicts[index];
	}

	fread(&size, 1, sizeof(size), FH);
	for (i = 0; i < size; i++)
	{	// fields - they'll put themselves in
		RestoreObject(FH, ScriptRL, this);
	}

	fread(&size, 1, sizeof(size), FH);
	for (i = 0; i < size; i++)
	{
		fread(&index, 1, sizeof(index), FH);
		fread(name, 1, VAR_LENGTH, FH);

		variable_index[index] = FindGlobal(name);
	}

	fread(&size, 1, sizeof(size), FH);
	for (i = 0; i < size; i++)
	{
		local_variables.PushBack((Variable*)RestoreObject(FH, ScriptRL, this));
	}

	fread(&size, 1, sizeof(size), FH);
	for (i = 0; i < size; i++)
	{
		parameter_variables.PushBack((Variable*)RestoreObject(FH, ScriptRL, this));
	}

	fread(&size, 1, sizeof(size), FH);
	for (i = 0; i < size; i++)
	{
		stack_variables.PushBack((Variable*)RestoreObject(FH, ScriptRL, this));
	}

	fread(&size, 1, sizeof(size), FH);
	for (i = 0; i < size; i++)
	{
		waiting_variables.PushBack((Variable*)RestoreObject(FH, ScriptRL, this));
	}

	fread(&size, 1, sizeof(size), FH);
	for (i = 0; i < size; i++)
	{
		signalers.PushBack((Signaler*)RestoreObject(FH, ScriptRL, this));
	}

	fread(&size, 1, sizeof(size), FH);
	for (i = 0; i < size; i++)
	{
		parameter_values.PushBack((StringVar*)RestoreObject(FH, ScriptRL, this));
	}

	fread(&size, 1, sizeof(size), FH);
	for (i = 0; i < size; i++)
	{
		events.PushBack((Event*)RestoreObject(FH, ScriptRL, this));
	}
}

CScript::~CScript(void)
{
	Free(true);
}

void CScript::LoadFile(void)
{
	int Version;

	length = gi.FS_LoadFile(name, (void**)&data);
	if (length == -1)
	{
		Com_Printf("***********************************************\n");
		Com_Printf("Could not open script %s\n", name);
		Com_Printf("***********************************************\n");
	}
	else
	{
		Version = ReadInt();

		if (Version != SCRIPT_VERSION)
		{
			Com_Printf("***********************************************\n");
			Com_Printf("Bad script version for %s: found %d, expecting %d\n", name, Version, SCRIPT_VERSION);
			Com_Printf("***********************************************\n");
		}
		else
		{
			script_condition = COND_READY;
		}
	}
}

void CScript::Free(bool DoData)
{
	int						i;
	List<Variable*>::Iter	iv;
	List<Signaler*>::Iter	is;
	List<StringVar*>::Iter	isv;
	List<Event*>::Iter		iev;

	if (data && DoData)
	{
		gi.FS_FreeFile(data);
		data = NULL;
	}

	while (local_variables.Size())
	{
		iv = local_variables.Begin();
		delete (*iv);

		local_variables.Erase(iv);
	}

	while (parameter_variables.Size())
	{
		iv = parameter_variables.Begin();
		delete (*iv);

		parameter_variables.Erase(iv);
	}

	while (stack_variables.Size())
	{
		iv = stack_variables.Begin();
		delete (*iv);

		stack_variables.Erase(iv);
	}

	while (waiting_variables.Size())
	{
		iv = waiting_variables.Begin();
		delete (*iv);

		waiting_variables.Erase(iv);
	}

	while (signalers.Size())
	{
		is = signalers.Begin();
		delete (*is);

		signalers.Erase(is);
	}

	while (parameter_values.Size())
	{
		isv = parameter_values.Begin();
		delete (*isv);

		parameter_values.Erase(isv);
	}

	while (events.Size())
	{
		iev = events.Begin();
		delete (*iev);

		events.Erase(iev);
	}

	for (i = 0; i < MAX_INDEX; i++)
	{
		if (fielddefs[i])
		{
			delete fielddefs[i];
		}
	}

	Clear(DoData);
}

void CScript::Clear(bool DoData)
{
	if (DoData)
	{
		data = NULL;
	}

	owner = other = activator = NULL;

	memset(fielddefs, 0, sizeof(fielddefs));
	memset(variable_index, 0, sizeof(variable_index));

	debug_flags = 0;
	memset(name, 0, sizeof(name));

	script_condition = COND_COMPLETED;
	condition_info = 0;
	data = NULL;
	position = 0;
	length = 0;
}

void CScript::Write(FILE* FH)
{
	int						index;
	int						size;
	List<Variable*>::Iter	iv;
	List<Signaler*>::Iter	is;
	List<StringVar*>::Iter	isv;
	List<Event*>::Iter		iev;
	int						i;

	index = RLID_SCRIPT;
	fwrite(&index, 1, sizeof(index), FH);

	fwrite(name, 1, sizeof(name), FH);
	fwrite(&script_condition, 1, sizeof(script_condition), FH);
	fwrite(&condition_info, 1, sizeof(condition_info), FH);
	fwrite(&length, 1, sizeof(length), FH);
	fwrite(&position, 1, sizeof(position), FH);
	fwrite(&debug_flags, 1, sizeof(debug_flags), FH);

	index = -1;
	if (owner)
	{
		index = owner - g_edicts;
	}
	fwrite(&index, 1, sizeof(index), FH);

	index = -1;
	if (other)
	{
		index = other - g_edicts;
	}
	fwrite(&index, 1, sizeof(index), FH);

	index = -1;
	if (activator)
	{
		index = activator - g_edicts;
	}
	fwrite(&index, 1, sizeof(index), FH);

	size = 0;
	for (i = 0, size = 0; i < MAX_INDEX; i++)
	{
		if (fielddefs[i])
		{
			size++;
		}
	}
	fwrite(&size, 1, sizeof(size), FH);
	for (i = 0; i < MAX_INDEX; i++)
	{
		if (fielddefs[i])
		{
			fielddefs[i]->Write(FH, this);
		}
	}

	size = 0;
	for (iv = GlobalVariables.Begin(); iv != GlobalVariables.End(); iv++)
	{
		if (LookupVarIndex(*iv) != -1)
		{
			size++;
		}
	}
	fwrite(&size, 1, sizeof(size), FH);
	for (iv = GlobalVariables.Begin(); iv != GlobalVariables.End(); iv++)
	{
		index = LookupVarIndex(*iv);
		if (index != -1)
		{
			fwrite(&index, 1, sizeof(index), FH);
			fwrite((*iv)->GetName(), 1, VAR_LENGTH, FH);
		}
	}

	size = local_variables.Size();
	fwrite(&size, 1, sizeof(size), FH);
	for (iv = local_variables.Begin(); iv != local_variables.End(); iv++)
	{
		(*iv)->Write(FH, this);
	}

	size = parameter_variables.Size();
	fwrite(&size, 1, sizeof(size), FH);
	for (iv = parameter_variables.Begin(); iv != parameter_variables.End(); iv++)
	{
		(*iv)->Write(FH, this);
	}

	size = stack_variables.Size();
	fwrite(&size, 1, sizeof(size), FH);
	for (iv = stack_variables.Begin(); iv != stack_variables.End(); iv++)
	{
		(*iv)->Write(FH, this);
	}

	size = waiting_variables.Size();
	fwrite(&size, 1, sizeof(size), FH);
	for (iv = waiting_variables.Begin(); iv != waiting_variables.End(); iv++)
	{
		(*iv)->Write(FH, this);
	}

	size = signalers.Size();
	fwrite(&size, 1, sizeof(size), FH);
	for (is = signalers.Begin(); is != signalers.End(); is++)
	{
		(*is)->Write(FH, this);
	}


	size = parameter_values.Size();
	fwrite(&size, 1, sizeof(size), FH);
	for (isv = parameter_values.Begin(); isv != parameter_values.End(); isv++)
	{
		(*isv)->Write(FH, this);
	}

	size = events.Size();
	fwrite(&size, 1, sizeof(size), FH);
	for (iev = events.Begin(); iev != events.End(); iev++)
	{
		(*iev)->Write(FH, this);
	}
}

int CScript::LookupVarIndex(const Variable* Var) const
{
	int i;

	for (i = 0; i < MAX_INDEX; i++)
	{
		if (variable_index[i] == Var)
		{
			return i;
		}
	}

	return -1;
}

int	CScript::LookupFieldIndex(const FieldDef* Field) const
{
	int i;

	for (i = 0; i < MAX_INDEX; i++)
	{
		if (fielddefs[i] == Field)
		{
			return i;
		}
	}

	return -1;
}

void CScript::SetParameter(const char* Value)
{
	parameter_values.PushBack(new StringVar("parm", Value));
}

unsigned char CScript::ReadByte(void)
{
	return data[position++];
}

int CScript::ReadInt(void)
{
	union
	{
		int				oldvalue;
		unsigned char	newvalue[4];
	};

	newvalue[0] = ReadByte();
	newvalue[1] = ReadByte();
	newvalue[2] = ReadByte();
	newvalue[3] = ReadByte();

	return oldvalue;
}

float CScript::ReadFloat(void)
{
	union
	{
		float			oldvalue;
		unsigned char	newvalue[4];
	};

	newvalue[0] = ReadByte();
	newvalue[1] = ReadByte();
	newvalue[2] = ReadByte();
	newvalue[3] = ReadByte();

	return oldvalue;
}

char* CScript::ReadString(void)
{
	char* Pos;

	Pos = (char*)&data[position];

	while (ReadByte())
	{
	}

	return Pos;
}

Variable* CScript::ReadDeclaration(int& Index)
{
	int			Type;
	char* Name;
	Variable* RetVal;

	Name = ReadString();
	Type = ReadByte();
	Index = ReadInt();

	RetVal = NULL;
	switch (Type)
	{
		case TYPE_INT:
			RetVal = new IntVar(Name);
			break;
		case TYPE_FLOAT:
			RetVal = new FloatVar(Name);
			break;
		case TYPE_VECTOR:
			RetVal = new VectorVar(Name);
			break;
		case TYPE_ENTITY:
			RetVal = new EntityVar(Name);
			break;
		case TYPE_STRING:
			RetVal = new StringVar(Name);
			break;
		case TYPE_UNKNOWN:
			break;
	}

	if (Index >= MAX_INDEX)
	{
		Error("Index out of range: %d > %d", Index, MAX_INDEX);
	}

	variable_index[Index] = RetVal;

	return RetVal;
}

void CScript::PushStack(Variable* VI)
{
	if (!VI)
	{
		Error("Illegal push");
	}

	stack_variables.PushBack(VI);
}

Variable* CScript::PopStack(void)
{
	Variable* Value;
	List<Variable*>::Iter	iv;

	if (stack_variables.Size())
	{
		iv = --stack_variables.End();
		Value = *iv;
		stack_variables.PopBack();

		return Value;
	}

	return NULL;
}

void CScript::HandleGlobal(bool Assignment)
{
	Variable* Var;
	int			Index;

	Var = ReadDeclaration(Index);

	if (Assignment)
	{
		Var->ReadValue(this);
	}

	if (!NewGlobal(Var))
	{
		variable_index[Index] = FindGlobal(Var->GetName());

		delete Var;
	}
}

void CScript::HandleLocal(bool Assignment)
{
	Variable* Var;
	int			Index;

	Var = ReadDeclaration(Index);

	if (Assignment)
	{
		Var->ReadValue(this);
	}

	NewLocal(Var);
}

void CScript::HandleParameter(bool Assignment)
{
	Variable* Var;
	int			Index;

	Var = ReadDeclaration(Index);

	if (Assignment)
	{
		Var->ReadValue(this);
	}

	NewParameter(Var);
}

void CScript::HandleField(void)
{
	int			Index;
	FieldDef* NewField;

	NewField = new FieldDef(this);

	Index = ReadInt();
	if (Index < 0 || Index >= MAX_INDEX)
	{
		Error("Index for field out of range: %d > %d\n", Index, MAX_INDEX);
	}

	fielddefs[Index] = NewField;
}

void CScript::HandleGoto(void)
{
	position = ReadInt();
}

Variable* CScript::HandleSpawn(void)
{
	int			Count;
	edict_t* ent;
	Variable* Name;
	Variable* Value;
	const field_t* f;
	const char* NameValue;
	byte* b;

	ent = G_Spawn();

	for (Count = ReadByte(); Count; Count--)
	{
		Name = PopStack();
		Value = PopStack();
		if (!Name || !Value)
		{
			Error("Invalid stack for HandleSpawn()");
		}

		NameValue = Name->GetStringValue();

		for (f = fields; f->name; f++)
		{
			if (!Q_stricmp(f->name, (char*)NameValue))
			{
				if (f->flags & FFL_SPAWNTEMP)
				{
					b = (byte*)&st;
				}
				else
				{
					b = (byte*)ent;
				}

				switch (f->type)
				{
					case F_LSTRING:
						*(char**)(b + f->ofs) = ED_NewString(Value->GetStringValue());
						break;
					case F_VECTOR:
						Value->GetVectorValue(*(vec3_t*)(b + f->ofs));
						break;
					case F_INT:
						*(int*)(b + f->ofs) = Value->GetIntValue();
						break;
					case F_FLOAT:
						*(float*)(b + f->ofs) = Value->GetFloatValue();
						break;
					case F_ANGLEHACK:
						((float*)(b + f->ofs))[0] = 0;
						((float*)(b + f->ofs))[1] = Value->GetFloatValue();
						((float*)(b + f->ofs))[2] = 0;
						break;
					case F_IGNORE:
						break;

					case F_RGBA:
						break;
					case F_RGB:
						break;
				}
				break;
			}
		}
	}

	ED_CallSpawn(ent);

	return new EntityVar(ent);
}

Variable* CScript::HandleBuiltinFunction(void)
{
	int			Index;
	edict_t* Search;
	Variable* V1;
	Variable* Var;

	Index = ReadByte();
	switch (Index)
	{
		case FUNC_FIND_ENTITY_WITH_TARGET:
			V1 = PopStack();
			Search = G_Find(NULL, FOFS(targetname), V1->GetStringValue());
			Var = new EntityVar(Search);

			delete V1;
			break;

		case FUNC_SIN:
			V1 = PopStack();
			Var = new FloatVar("", sin(DEG2RAD(V1->GetFloatValue())));

			delete V1;
			break;

		case FUNC_COS:
			V1 = PopStack();
			Var = new FloatVar("", cos(DEG2RAD(V1->GetFloatValue())));

			delete V1;
			break;

		case FUNC_FIND_ENTITY_WITH_SCRIPT:
			V1 = PopStack();
			Search = G_Find(NULL, FOFS(scripttarget), V1->GetStringValue());
			Var = new EntityVar(Search);

			delete V1;
			break;

		case FUNC_SPAWN:
			Var = HandleSpawn();
			break;

		case FUNC_GET_OTHER:
			Var = new EntityVar(other);
			break;

		case FUNC_GET_ACTIVATOR:
			Var = new EntityVar(activator);
			break;
	}

	return Var;
}

void CScript::HandlePush(void)
{
	int			Type;
	Variable* Var;

	Type = ReadByte();
	switch (Type)
	{
		case PUSH_CONST_INT:
			Var = new IntVar();
			Var->ReadValue(this);
			break;
		case PUSH_CONST_FLOAT:
			Var = new FloatVar();
			Var->ReadValue(this);
			break;
		case PUSH_CONST_VECTOR:
			Var = new VectorVar();
			Var->ReadValue(this);
			break;
		case PUSH_CONST_ENTITY:
			Var = new EntityVar();
			Var->ReadValue(this);
			break;
		case PUSH_CONST_STRING:
			Var = new StringVar();
			Var->ReadValue(this);
			break;
		case PUSH_VAR:
			Var = new VariableVar();
			((VariableVar*)Var)->ReadValue(this);
			break;
		case PUSH_VAR_WITH_FIELD:
			Var = new FieldVariableVar();
			((VariableVar*)Var)->ReadValue(this); //TODO: (FieldVariableVar*)?
			break;
		case PUSH_FUNCTION:
			Var = HandleBuiltinFunction();
			break;
	}

	PushStack(Var);
}

void CScript::HandlePop(void)
{
	Variable* V;

	V = PopStack();
	if (V)
	{
		delete V;
	}
}

void CScript::HandleAssignment(void)
{
	Variable* Value, * Assignee;

	Assignee = PopStack();
	Value = PopStack();
	if (Value == NULL || Assignee == NULL)
	{
		Error("Invalid stack for Add");
	}

	(*Assignee) = Value;

	delete Assignee;
	delete Value;
}

void CScript::HandleAdd(void)
{
	Variable* V1, * V2;

	V1 = PopStack();
	V2 = PopStack();
	if (V1 == NULL || V2 == NULL)
	{
		Error("Invalid stack for Add");
	}

	PushStack((*V1) + V2);

	delete V1;
	delete V2;
}

void CScript::HandleSubtract(void)
{
	Variable* V1, * V2;

	V1 = PopStack();
	V2 = PopStack();
	if (V1 == NULL || V2 == NULL)
	{
		Error("Invalid stack for Subtract");
	}

	PushStack((*V1) - V2);

	delete V1;
	delete V2;
}

void CScript::HandleMultiply(void)
{
	Variable* V1, * V2;

	V1 = PopStack();
	V2 = PopStack();
	if (V1 == NULL || V2 == NULL)
	{
		Error("Invalid stack for Multiply");
	}

	PushStack((*V1) * V2);

	delete V1;
	delete V2;
}

void CScript::HandleDivide(void)
{
	Variable* V1, * V2;

	V1 = PopStack();
	V2 = PopStack();
	if (V1 == NULL || V2 == NULL)
	{
		Error("Invalid stack for Divide");
	}

	PushStack((*V1) / V2);

	delete V1;
	delete V2;
}

void CScript::HandleDebug(void)
{
	List<Variable*>::Iter	iv;
	int						Flags;

	Flags = ReadByte();

	if (Flags)
	{
		if (Flags & DEBUG_ENABLE)
		{
			Flags &= ~DEBUG_ENABLE;
			debug_flags |= Flags;
		}
		else
		{
			debug_flags &= ~Flags;
		}
	}
	else
	{
		StartDebug();

		if (parameter_variables.Size())
		{
			DebugLine("   Parameters:\n");
			for (iv = parameter_variables.Begin(); iv != parameter_variables.End(); iv++)
			{
				(*iv)->Debug(this);
			}
		}

		if (GlobalVariables.Size())
		{
			DebugLine("   Global Variables:\n");
			for (iv = GlobalVariables.Begin(); iv != GlobalVariables.End(); iv++)
			{
				(*iv)->Debug(this);
			}
		}

		if (local_variables.Size())
		{
			DebugLine("   Local Variables:\n");
			for (iv = local_variables.Begin(); iv != local_variables.End(); iv++)
			{
				(*iv)->Debug(this);
			}
		}
		EndDebug();
	}
}

void CScript::HandleDebugStatement(void)
{
	DebugLine("%s\n", ReadString());
}

void CScript::HandleAddAssignment(void)
{
	Variable* Value, * Assignee;

	Assignee = PopStack();
	Value = PopStack();
	if (Value == NULL || Assignee == NULL)
	{
		Error("Invalid stack for AddAssignment");
	}

	(*Assignee) = (*Assignee) + Value;

	delete Assignee;
	delete Value;
}

void CScript::HandleSubtractAssignment(void)
{
	Variable* Value, * Assignee;

	Assignee = PopStack();
	Value = PopStack();
	if (Value == NULL || Assignee == NULL)
	{
		Error("Invalid stack for SubtractAssignment");
	}

	(*Assignee) = (*Assignee) - Value;

	delete Assignee;
	delete Value;
}

void CScript::HandleMultiplyAssignment(void)
{
	Variable* Value, * Assignee;

	Assignee = PopStack();
	Value = PopStack();
	if (Value == NULL || Assignee == NULL)
	{
		Error("Invalid stack for MultiplyAssignment");
	}

	(*Assignee) = (*Assignee) * Value;

	delete Assignee;
	delete Value;
}

void CScript::HandleDivideAssignment(void)
{
	Variable* Value, * Assignee;

	Assignee = PopStack();
	Value = PopStack();
	if (Value == NULL || Assignee == NULL)
	{
		Error("Invalid stack for DivideAssignment");
	}

	(*Assignee) = (*Assignee) / Value;

	delete Assignee;
	delete Value;
}

bool CScript::HandleWait(bool ForAll)
{
	int			count;
	Variable* VI;

	count = ReadByte();
	if (count & WAIT_CLEAR)
	{
		condition_info = WAIT_CLEAR;
	}
	else
	{
		condition_info = 0;
	}

	count &= ~WAIT_CLEAR;

	for (; count; count--)
	{
		VI = PopStack();
		if (!VI)
		{
			Error("Invalid stack for HandleWait");
		}

		waiting_variables.PushBack(VI);
	}

	if (ForAll)
	{
		script_condition = COND_WAIT_ALL;
	}
	else
	{
		script_condition = COND_WAIT_ANY;
	}

	if (CheckWait())
	{
		FinishWait(NULL, false);

		return false;
	}

	return true;
}

bool CScript::HandleTimeWait(void)
{
	Variable* V;
	float		NextTime;

	V = PopStack();
	if (!V)
	{
		Error("Invalid stack for Time Wait");
	}

	NextTime = level.time + V->GetFloatValue();
	if (NextTime <= level.time)
	{
		return false;
	}

	AddEvent(new WaitEvent(NextTime));

	script_condition = COND_WAIT_TIME;

	return true;
}

void CScript::HandleIf(void)
{
	int			Condition;
	int			Location;
	Variable* V1, * V2;
	bool		Result;

	Condition = ReadByte();
	Location = ReadInt();

	V2 = PopStack();
	V1 = PopStack();

	if (V1 == NULL || V2 == NULL)
	{
		Error("Invalid stack for If");
	}

	Result = false;

	switch (Condition)
	{
		case COND_EQUAL:
			if ((*V1) == V2)
			{
				Result = true;
			}
			break;
		case COND_LESS_THAN:
			if ((*V1) < V2)
			{
				Result = true;
			}
			break;
		case COND_LESS_THAN_EQUAL:
			if ((*V1) <= V2)
			{
				Result = true;
			}
			break;
		case COND_GREATER_THAN:
			if ((*V1) > V2)
			{
				Result = true;
			}
			break;
		case COND_GREATER_THAN_EQUAL:
			if ((*V1) >= V2)
			{
				Result = true;
			}
			break;
		case COND_NOT_EQUAL:
			if ((*V1) != V2)
			{
				Result = true;
			}
			break;
	}

	if (!Result)
	{
		position = Location;
	}
}

void CScript::HandlePrint(void)
{
	int			Flags;
	Variable* Text, * Entity, * Level;
	const char* TextValue;
	int			LevelValue;
	edict_t* ent;
	int			TextIndex;

	Entity = Level = NULL;
	LevelValue = PRINT_HIGH;
	ent = NULL;

	Flags = ReadByte();

	Text = PopStack();
	if (!Text)
	{
		Error("Invalid stack for Print");
	}
	if (Text->GetType() == TYPE_STRING)
	{
		TextValue = Text->GetStringValue();
	}
	else
	{
		TextIndex = Text->GetIntValue();
		TextValue = message_text[TextIndex].string;
	}

	if (Flags & PRINT_LEVEL)
	{
		Level = PopStack();
		if (!Level)
		{
			Error("Invalid stack for Print");
		}
		LevelValue = Level->GetIntValue();
	}

	if (Flags & PRINT_ENTITY)
	{
		Entity = PopStack();
		if (!Entity)
		{
			Error("Invalid stack for Print");
		}
		ent = Entity->GetEdictValue();
	}

	if (!sv_jumpcinematic->value || !sv_cinematicfreeze->value)
	{
		if (Flags & PRINT_CAPTIONED)
		{
			if (ent)
			{
				gi.captionprintf(ent, TextIndex);		// Send the ID for the text to the single player
			}
			else
			{
				gi.bcaption(PRINT_HIGH, TextIndex);		// Send the ID for the text to all players
			}
		}
		else if (Flags & PRINT_CENTERED)
		{
			if (ent)
			{
				gi.levelmsg_centerprintf(ent, TextIndex);			// Send the ID over the net rather than the string itself...
			}
		}
		else
		{
			if (ent)
			{
				gi.cprintf(ent, LevelValue, TextValue);
			}
			else
			{
				gi.bprintf(LevelValue, TextValue);
			}
		}
	}

	delete Text;
	if (Entity)
	{
		delete Entity;
	}
	if (Level)
	{
		delete Level;
	}
}

void CScript::HandlePlaySound(void)
{
	int			Flags;
	Variable* SoundName, * Entity, * Volume, * Attenuation, * Channel, * TimeDelay;
	const char* SoundValue;
	float		VolumeValue, AttenuationValue, TimeDelayValue;
	int			ChannelValue;
	edict_t* ent;


	Entity = Volume = Attenuation = Channel = TimeDelay = NULL;
	ent = NULL;
	VolumeValue = 1.0;
	AttenuationValue = ATTN_NORM;
	ChannelValue = CHAN_VOICE;

	TimeDelayValue = 0.0;

	Flags = ReadByte();

	SoundName = PopStack();
	if (!SoundName)
	{
		Error("Invalid stack for PlaySound");
	}
	SoundValue = SoundName->GetStringValue();

	if (Flags & PLAY_SOUND_TIMEDELAY)
	{
		TimeDelay = PopStack();
		if (!TimeDelay)
		{
			Error("Invalid stack for PlaySound");
		}
		TimeDelayValue = TimeDelay->GetFloatValue();
	}

	if (Flags & PLAY_SOUND_CHANNEL)
	{
		Channel = PopStack();
		if (!Channel)
		{
			Error("Invalid stack for PlaySound");
		}
		ChannelValue = Channel->GetIntValue();
	}

	if (Flags & PLAY_SOUND_ATTENUATION)
	{
		Attenuation = PopStack();
		if (!Attenuation)
		{
			Error("Invalid stack for PlaySound");
		}
		AttenuationValue = Attenuation->GetFloatValue();
	}

	if (Flags & PLAY_SOUND_VOLUME)
	{
		Volume = PopStack();
		if (!Volume)
		{
			Error("Invalid stack for PlaySound");
		}
		VolumeValue = Volume->GetFloatValue();
	}

	if (Flags & PLAY_SOUND_ENTITY)
	{
		Entity = PopStack();
		if (!Entity)
		{
			Error("Invalid stack for PlaySound");
		}
		ent = Entity->GetEdictValue();
	}

	if (sv_cinematicfreeze->value)		// In cinematic freezes, all sounds should be full volume.  Thus is it written.
	{
		AttenuationValue = ATTN_NONE;
		cinematic_sounds[cinematic_sounds_count].ent = ent;
		cinematic_sounds[cinematic_sounds_count].channel = ChannelValue;

		if (cinematic_sounds_count < MAX_CINEMATIC_SOUNDS - 1)
			++cinematic_sounds_count;
	}

	if (!sv_jumpcinematic->value || !sv_cinematicfreeze->value)
	{
		gi.sound(ent, ChannelValue, gi.soundindex(SoundValue), VolumeValue, AttenuationValue, TimeDelayValue);
	}

	delete SoundName;
	if (Entity)
	{
		delete Entity;
	}
	if (Volume)
	{
		delete Volume;
	}
	if (Attenuation)
	{
		delete Attenuation;
	}
	if (Channel)
	{
		delete Channel;
	}
	if (TimeDelay)
	{
		delete TimeDelay;
	}
}

void CScript::HandleFeature(bool Enable)
{
	int FeatureType;
	int i, null_snd;

	FeatureType = ReadByte();

	switch (FeatureType)
	{
		case FEATURE_TRIGGER:
			HandleTrigger(Enable);
			break;

		case FEATURE_AMBIENT_SOUNDS:
			break;

		case FEATURE_CINEMATICS:
			if (Enable)
			{
				cinematic_sounds_count = 0;
				Cvar_Set("sv_cinematicfreeze", "1");
				RemoveNonCinematicEntities();
			}
			else
			{
				if (sv_jumpcinematic->value == 2)	// Jump sent from client
				{
					Cvar_Set("sv_jumpcinematic", "0");
					null_snd = gi.soundindex("misc/null.wav");
					gi.bcaption(PRINT_HIGH, 270);		// Send the ID for the text to all players
					for (i = 0; i < cinematic_sounds_count; ++i)
					{
						if (cinematic_sounds[i].ent)	// Does the entity still exist
						{
							gi.sound(cinematic_sounds[i].ent, cinematic_sounds[i].channel,
								null_snd, 1, ATTN_NORM, 0);

						}
					}
				}


				Cvar_Set("sv_cinematicfreeze", "0");
				ReinstateNonCinematicEntities();
			}
			break;

		case FEATURE_PLAGUE_SKINS:
			break;
	}
}

void CScript::HandleCacheSound(void)
{
	Variable* SoundName;
	const char* SoundValue;

	SoundName = PopStack();
	if (!SoundName)
	{
		Error("Invalid stack for HandleChacheSound");
	}
	SoundValue = SoundName->GetStringValue();

	if (!sv_jumpcinematic->value || !sv_cinematicfreeze->value)
	{
		gi.soundindex(SoundValue);
	}

	delete SoundName;
}

void CScript::HandleMove(void)
{
	int			Flags;
	Variable* Signaler, * Rate, * Duration, * Amount, * Entity;
	edict_t* ent;
	vec3_t		Vec, Dest, Diff;
	vec_t		Length;

	Signaler = Rate = Duration = NULL;

	Flags = ReadByte();

	if (Flags & MOVE_SIGNALER)
	{
		Signaler = PopStack();
	}

	if (Flags & MOVE_RATE)
	{
		Rate = PopStack();
	}

	if (Flags & MOVE_DURATION)
	{
		Duration = PopStack();
	}

	Amount = PopStack();
	Entity = PopStack();

	Amount->GetVectorValue(Vec);

	ent = Entity->GetEdictValue();
	if (ent)
	{
		if (!Rate && !Duration)
		{
			VectorAdd(ent->s.origin, Vec, ent->s.origin);
			if (ent->chain)
			{
				VectorAdd(ent->chain->s.origin, Vec, ent->chain->s.origin);
			}
		}
		else
		{
			if (!(Flags & MOVE_ABSOLUTE))
			{
				VectorAdd(ent->s.origin, Vec, Dest);
			}
			else
			{
				VectorCopy(Vec, Dest);
			}

			VectorSubtract(ent->s.origin, Dest, Diff);
			Length = VectorLength(Diff);

			if (Rate && Duration)
			{
				ent->moveinfo.decel = ent->moveinfo.accel = ent->moveinfo.speed = Rate->GetFloatValue();
				Length = Rate->GetFloatValue() * Duration->GetFloatValue();
				VectorNormalize(Diff);
				VectorMA(ent->s.origin, Length, Diff, Dest);
			}
			else if (Rate)
			{
				ent->moveinfo.decel = ent->moveinfo.accel = ent->moveinfo.speed = Rate->GetFloatValue();
			}
			else
			{
				ent->moveinfo.decel = ent->moveinfo.accel = ent->moveinfo.speed = Length / Duration->GetFloatValue();
			}

			if (debug_flags & DEBUG_MOVE)
			{
				StartDebug();
				DebugLine("   Moving Entity %d\n", Entity->GetIntValue());
				DebugLine("      From (%7.3f, %7.3f, %7.3f)\n", ent->s.origin[0], ent->s.origin[1], ent->s.origin[2]);
				DebugLine("      To   (%7.3f, %7.3f, %7.3f)\n", Dest[0], Dest[1], Dest[2]);
				EndDebug();
			}

			if (Signaler)
			{
				AddSignaler(ent, Signaler, SIGNAL_MOVE);
			}

			Move(ent, Dest);
		}
	}

	delete Amount;
	delete Entity;
	//	Signaling routine will handle this
	//	if (Signaler)
	//	{
	//		delete Signaler;
	//	}
	if (Rate)
	{
		delete Rate;
	}
	if (Duration)
	{
		delete Duration;
	}
}

void CScript::HandleRotate(void)
{
	int			Flags;
	Variable* Signaler, * Rate, * Duration, * Amount, * Entity;
	edict_t* ent;
	vec3_t		Vec, Dest, Diff;
	vec_t		Length;

	Signaler = Rate = Duration = NULL;

	Flags = ReadByte();

	if (Flags & ROTATE_SIGNALER)
	{
		Signaler = PopStack();
	}

	if (Flags & ROTATE_RATE)
	{
		Rate = PopStack();
	}

	if (Flags & ROTATE_DURATION)
	{
		Duration = PopStack();
	}

	Amount = PopStack();
	Entity = PopStack();

	Amount->GetVectorValue(Vec);

	ent = Entity->GetEdictValue();
	if (ent)
	{
		if (!Rate && !Duration)
		{
			VectorAdd(ent->s.angles, Vec, ent->s.angles);
			if (ent->chain)
			{
				VectorAdd(ent->chain->s.angles, Vec, ent->chain->s.angles);
			}
		}
		else
		{
			if (!(Flags & MOVE_ABSOLUTE))
			{
				VectorAdd(ent->s.angles, Vec, Dest);
			}
			else
			{
				VectorCopy(Vec, Dest);
			}

			VectorSubtract(ent->s.angles, Dest, Diff);
			Length = VectorLength(Diff);

			if (Rate && Duration)
			{
				ent->moveinfo.speed = Rate->GetFloatValue();
				Length = Rate->GetFloatValue() * Duration->GetFloatValue();
				VectorNormalize(Diff);
				VectorMA(ent->s.angles, Length, Diff, Dest);
			}
			else if (Rate)
			{
				ent->moveinfo.speed = Rate->GetFloatValue();
			}
			else
			{
				ent->moveinfo.speed = Length / Duration->GetFloatValue();
			}

			VectorCopy(Dest, ent->moveinfo.start_angles);
			VectorCopy(Dest, ent->moveinfo.end_angles);

			if (debug_flags & DEBUG_ROTATE)
			{
				StartDebug();
				DebugLine("   Rotating Entity %d\n", Entity->GetIntValue());
				DebugLine("      From (%7.3f, %7.3f, %7.3f)\n", ent->s.angles[0], ent->s.angles[1], ent->s.angles[2]);
				DebugLine("      To   (%7.3f, %7.3f, %7.3f)\n", ent->moveinfo.end_angles[0], ent->moveinfo.end_angles[1], ent->moveinfo.end_angles[2]);
				EndDebug();
			}

			if (Signaler)
			{
				AddSignaler(ent, Signaler, SIGNAL_ROTATE);
			}
			Rotate(ent);
		}
	}

	delete Amount;
	delete Entity;
	//	Signaling routine will handle this
	//	if (Signaler)
	//	{
	//		delete Signaler;
	//	}
	if (Rate)
	{
		delete Rate;
	}
	if (Duration)
	{
		delete Duration;
	}
}

void CScript::HandleUse(void)
{
	Variable* Entity;
	edict_t* use_ent;

	Entity = PopStack();

	use_ent = Entity->GetEdictValue();
	if (use_ent && use_ent->use)
	{
		use_ent->use(use_ent, other, activator);
	}

	delete Entity;
}

void CScript::HandleTrigger(bool Enable)
{
	Variable* Entity;
	edict_t* trigger_ent;

	Entity = PopStack();

	trigger_ent = Entity->GetEdictValue();
	if (trigger_ent)
	{
		if (Enable)
		{
			trigger_ent->solid = SOLID_TRIGGER;
			trigger_ent->use = TriggerMultipleUse;
			gi.linkentity(trigger_ent);
		}
		else
		{
			trigger_ent->solid = SOLID_NOT;
			trigger_ent->use = NULL;
		}
	}
}

void CScript::HandleAnimate(void)
{
	int			Flags;
	Variable* Signaler, * Moving, * Turning, * Repeat, * Action, * Entity, * Source;
	edict_t* ent, * SourceEnt;
	vec3_t		MovingVal;
	vec3_t		TurningVal;
	int			RepeatVal, ActionVal;

	void		(*SignalerRoutine)(edict_t*);

	SignalerRoutine = NULL;
	Signaler = Moving = Turning = Repeat = Action = Entity = Source = NULL;
	SourceEnt = NULL;
	VectorCopy(vec3_origin, MovingVal);
	VectorCopy(vec3_origin, TurningVal);
	RepeatVal = 0;

	Flags = ReadByte();

	if (Flags & ANIMATE_SOURCE)
	{
		Source = PopStack();
		SourceEnt = Source->GetEdictValue();
	}

	if (Flags & ANIMATE_SIGNALER)
	{
		Signaler = PopStack();
	}

	if (Flags & ANIMATE_MOVING)
	{
		Moving = PopStack();
		Moving->GetVectorValue(MovingVal);
	}

	if (Flags & ANIMATE_TURNING)
	{
		Turning = PopStack();
		Turning->GetVectorValue(TurningVal);
	}

	if (Flags & ANIMATE_REPEAT)
	{
		Repeat = PopStack();
		RepeatVal = Repeat->GetIntValue();
	}


	Action = PopStack();
	ActionVal = Action->GetIntValue();

	Entity = PopStack();
	ent = Entity->GetEdictValue();

	if (ent)
	{
		if (Signaler)
		{
			AddSignaler(ent, Signaler, SIGNAL_ANIMATE);
			SignalerRoutine = animate_signaler;
		}

		QPostMessage(ent, (enum G_MsgID_e)msg_animtype[ActionVal], PRI_DIRECTIVE, "iiige", (int)MovingVal[0], (int)TurningVal[0], (int)RepeatVal, SignalerRoutine, activator);
	}

	delete Action;
	delete Entity;
	if (Source)
	{
		delete Source;
	}
	//	Signaling routine will handle this
	//	if (Signaler)
	//	{
	//		delete Signaler;
	//	}
	if (Repeat)
	{
		delete Repeat;
	}
	if (Turning)
	{
		delete Turning;
	}
	if (Moving)
	{
		delete Moving;
	}
}


void CScript::HandleCopyPlayerAttributes(void)
{
	Variable* Player, * Destination;
	edict_t* PlayerEnt, * DestinationEnt;

	Destination = PopStack();
	if (!Destination)
	{
		Error("Invalid stack for HandleCopyPlayerAttributes()");
	}
	DestinationEnt = Destination->GetEdictValue();

	Player = PopStack();
	if (!Player)
	{
		Error("Invalid stack for HandleCopyPlayerAttributes()");
	}
	PlayerEnt = Player->GetEdictValue();

	CinematicSwapPlayer(PlayerEnt, DestinationEnt);
}

void CScript::HandleSetViewAngles(void)
{
	Variable* Player, * Angles;
	edict_t* PlayerEnt;
	vec3_t		vec;
	vec3_t		HoldAngles;

	Angles = PopStack();
	if (!Angles)
	{
		Error("Invalid stack for HandleSetViewAngles()");
	}
	Angles->GetVectorValue(vec);

	Player = PopStack();
	if (!Player)
	{
		Error("Invalid stack for HandleSetViewAngles()");
	}
	PlayerEnt = Player->GetEdictValue();

	// use PlayerEnt and vec
	// set angles
	Angles->GetVectorValue(HoldAngles);

	PlayerEnt->client->ps.pmove.delta_angles[PITCH] = 0;
	PlayerEnt->client->ps.pmove.delta_angles[YAW] = ANGLE2SHORT(HoldAngles[YAW] - PlayerEnt->client->resp.cmd_angles[YAW]);
	PlayerEnt->client->ps.pmove.delta_angles[ROLL] = 0;

	PlayerEnt->s.angles[PITCH] = 0;
	PlayerEnt->s.angles[YAW] = HoldAngles[YAW];
	PlayerEnt->s.angles[ROLL] = 0;
}

void CScript::HandleSetCacheSize(void)
{
	Variable* CacheSize;

	CacheSize = PopStack();
	if (!CacheSize)
	{
		Error("Invalid stack for HandleSetCacheSize()");
	}
}

void CScript::Move_Done(edict_t* ent)
{
	VectorClear(ent->velocity);

	VectorCopy(ent->moveinfo.end_origin, ent->s.origin);
}

void CScript::Move(edict_t* ent, const vec3_t Dest)
{
	float	frames;

	VectorCopy(Dest, ent->moveinfo.end_origin);

	VectorSubtract(ent->moveinfo.end_origin, ent->s.origin, ent->moveinfo.dir);
	ent->moveinfo.remaining_distance = VectorNormalize(ent->moveinfo.dir);
	if (ent->moveinfo.remaining_distance <= 0)
	{
		frames = 0;
	}
	else
	{
		frames = floor((ent->moveinfo.remaining_distance / ent->moveinfo.speed) / FRAMETIME) + 1;
	}

	VectorScale(ent->moveinfo.dir, ent->moveinfo.remaining_distance / frames / FRAMETIME, ent->velocity);

	AddEvent(new MoveDoneEvent(level.time + (frames * FRAMETIME), ent));
}

void CScript::Rotate_Done(edict_t* ent)
{
	VectorClear(ent->avelocity);
}

void CScript::Rotate(edict_t* ent)
{
	float	distance;
	vec3_t	destdelta;
	float	frames;

	VectorSubtract(ent->moveinfo.start_angles, ent->s.angles, destdelta);

	distance = VectorNormalize(destdelta);
	if (ent->moveinfo.speed <= 0)
	{
		frames = 0;
		VectorClear(ent->avelocity);
	}
	else
	{
		frames = floor((distance / ent->moveinfo.speed) / FRAMETIME) + 1;
		VectorScale(destdelta, distance / frames / FRAMETIME, ent->avelocity);
	}

	AddEvent(new RotateDoneEvent(level.time + (frames * FRAMETIME), ent));
}

void CScript::AddEvent(Event* Which)
{
	List<Event*>::Iter	ie;
	float				time;

	if (events.Size())
	{
		time = Which->GetTime();
		for (ie = events.Begin(); ie != events.End(); ie++)
		{
			if ((*ie)->GetTime() > time)
			{
				break;
			}
		}
		events.Insert(ie, Which);
	}
	else
	{
		events.PushBack(Which);
	}

#ifdef _DEBUG
	float				testtime;

	time = 0;
	for (ie = events.Begin(); ie != events.End(); ie++)
	{
		testtime = (*ie)->GetTime();
		if (testtime < time)
		{
			DebugBreak();
		}
	}
#endif
}

void CScript::ProcessEvents(void)
{
	List<Event*>::Iter	ie, next;

	while (events.Size())
	{
		ie = events.Begin();

		if ((*ie)->Process(this))
		{
			delete (*ie);
			events.Erase(ie);
		}
		else
		{
			break;
		}
	}
}

void CScript::ClearTimeWait(void)
{
	if (script_condition == COND_WAIT_TIME)
	{
		script_condition = COND_READY;
	}
}

void CScript::AddSignaler(edict_t* Edict, Variable* Var, SignalT SignalType)
{
	List<Signaler*>::Iter	is;
	Signaler* NewSig;

	NewSig = new Signaler(Edict, Var, SignalType);

	// Note that this check does not need to be in there - signalers are very flexible, but if used
	// incorrectly, they can result in weird behavior - this check prevents more than one command using
	// the same signal varaible prior to a wait command
	for (is = signalers.Begin(); is != signalers.End(); is++)
	{
		if (*(*is) == NewSig)
		{
			Error("Renner Error #1: Variable '%s' is being used for multiple signals", Var->GetName());
		}
	}

	signalers.PushBack(NewSig);
}

void CScript::CheckSignalers(edict_t* Which, SignalT SignalType)
{
	List<Signaler*>::Iter	is, next;
	bool					DoCheckWait = false;

	if (signalers.Size())
	{
		for (is = signalers.Begin(); is != signalers.End(); is = next)
		{
			next = is;
			next++;
			if ((*is)->Test(Which, SignalType))
			{
				delete (*is);
				signalers.Erase(is);

				DoCheckWait = true;
			}
		}
	}

	if (DoCheckWait && (script_condition == COND_WAIT_ANY || script_condition == COND_WAIT_ALL))
	{
		if (CheckWait())
		{
			FinishWait(Which, true);
		}
	}
}

bool CScript::CheckWait(void)
{
	List<Variable*>::Iter	iv;
	int						count, needed;

	if (script_condition == COND_WAIT_ALL)
	{
		needed = waiting_variables.Size();
	}
	else if (script_condition == COND_WAIT_ANY)
	{
		needed = 1;
	}
	else if (script_condition == COND_WAIT_TIME)
	{
		return false;
	}
	else if (script_condition == COND_READY)
	{
		return true;
	}
	else
	{
		return false;
	}

	count = 0;
	if (waiting_variables.Size())
	{
		for (iv = waiting_variables.Begin(); iv != waiting_variables.End(); iv++)
		{
			if ((*iv)->GetIntValue())
			{
				count++;
			}
		}
	}

	if (count == needed)
	{
		script_condition = COND_READY;

		return true;
	}

	return false;
}

void CScript::FinishWait(edict_t* Which, bool NoExecute)
{
	List<Variable*>::Iter	iv;

	if (waiting_variables.Size())
	{
		for (iv = waiting_variables.Begin(); iv != waiting_variables.End(); iv++)
		{
			if (condition_info == WAIT_CLEAR)
			{
				(*iv)->ClearSignal();
			}

			delete* iv;
		}
	}
	waiting_variables.Erase(waiting_variables.Begin(), waiting_variables.End());

	if (NoExecute)
	{
		Execute(Which, NULL);
	}
}

void CScript::Error(const char* error, ...)
{
	va_list argptr;
	char	text[1024];

	va_start(argptr, error);
	vsprintf(text, error, argptr);
	va_end(argptr);

	gi.error(text);
}

void CScript::StartDebug(void)
{
	DebugLine("-------------------------------\n");
	DebugLine("Script: %s\n", name);
	DebugLine("   DEBUG at %d\n", position);
}

void CScript::EndDebug(void)
{
	DebugLine("-------------------------------\n");
}

void CScript::DebugLine(const char* debugtext, ...)
{
	va_list argptr;
	char	text[1024];

	va_start(argptr, debugtext);
	vsprintf(text, debugtext, argptr);
	va_end(argptr);

	Com_Printf("%s", text);

#ifdef _DEBUG
	OutputDebugString(text);
#endif
}

void CScript::Think(void)
{
	ProcessEvents();
}

ScriptConditionT CScript::Execute(edict_t* new_other, edict_t* new_activator)
{
	bool				Done;
	int					InstructionCount;

	if (script_condition != COND_READY)
	{
		return script_condition;
	}

	if (debug_flags & DEBUG_TIME)
	{
		StartDebug();
		DebugLine("   Current Time: %10.1f\n", level.time);
		EndDebug();
	}

	if (new_other)
	{
		other = new_other;
	}
	if (new_activator)
	{
		activator = new_activator;
	}

	InstructionCount = 0;
	Done = false;
	while (!Done)
	{
		InstructionCount++;
		if (InstructionCount > MAX_INSTRUCTIONS)
		{
			Error("Runaway loop for script");
		}

		switch (ReadByte())
		{
			case CODE_NEW_GLOBAL:
				HandleGlobal(false);
				break;
			case CODE_NEW_GLOBAL_PLUS_ASSIGNMENT:
				HandleGlobal(true);
				break;
			case CODE_NEW_LOCAL:
				HandleLocal(false);
				break;
			case CODE_NEW_LOCAL_PLUS_ASSIGNMENT:
				HandleLocal(true);
				break;
			case CODE_NEW_PARAMETER:
				HandleParameter(false);
				break;
			case CODE_NEW_PARAMETER_PLUS_DEFAULT:
				HandleParameter(true);
				break;
			case CODE_FIELD:
				HandleField();
				break;
			case CODE_ASSIGNMENT:
				HandleAssignment();
				break;
			case CODE_ADD:
				HandleAdd();
				break;
			case CODE_SUBTRACT:
				HandleSubtract();
				break;
			case CODE_MULTIPLY:
				HandleMultiply();
				break;
			case CODE_DIVIDE:
				HandleDivide();
				break;
			case CODE_ADD_ASSIGNMENT:
				HandleAddAssignment();
				break;
			case CODE_SUBTRACT_ASSIGNMENT:
				HandleSubtractAssignment();
				break;
			case CODE_MULTIPLY_ASSIGNMENT:
				HandleMultiplyAssignment();
				break;
			case CODE_DIVIDE_ASSIGNMENT:
				HandleDivideAssignment();
				break;
			case CODE_GOTO:
				HandleGoto();
				break;
			case CODE_PUSH:
				HandlePush();
				break;
			case CODE_POP:
				HandlePop();
				break;
			case CODE_IF:
				HandleIf();
				break;
			case CODE_EXIT:
				script_condition = COND_COMPLETED;
				Done = true;
				break;
			case CODE_SUSPEND:
				//ScriptCondition = COND_SUSPENDED;
				Done = true;
				break;
			case CODE_DEBUG:
				HandleDebug();
				break;
			case CODE_WAIT_SECONDS:
				Done = HandleTimeWait();
				break;
			case CODE_WAIT_ALL:
				Done = HandleWait(true);
				break;
			case CODE_WAIT_ANY:
				Done = HandleWait(false);
				break;
			case CODE_MOVE:
				HandleMove();
				break;
			case CODE_ROTATE:
				HandleRotate();
				break;
			case CODE_USE:
				HandleUse();
				break;
			case CODE_COPY_PLAYER_ATTRIBUTES:
				HandleCopyPlayerAttributes();
				break;
			case CODE_SET_VIEW_ANGLES:
				HandleSetViewAngles();
				break;
			case CODE_SET_CACHE_SIZE:
				HandleSetCacheSize();
				break;
			case CODE_ANIMATE:
				HandleAnimate();
				break;
			case CODE_PRINT:
				HandlePrint();
				break;
			case CODE_PLAY_SOUND:
				HandlePlaySound();
				break;
			case CODE_ENABLE:
				HandleFeature(true);
				break;
			case CODE_DISABLE:
				HandleFeature(false);
				break;
			case CODE_DEBUG_STATEMENT:
				HandleDebugStatement();
				break;
			case CODE_CACHE_SOUND:
				HandleCacheSound();
				break;
			default:
				Done = true;
				break;
		}

		if (position >= length)
		{
			Done = true;
			script_condition = COND_COMPLETED;
		}
	}

	return script_condition;
}

Variable* CScript::FindLocal(const char* Name)
{
	List<Variable*>::Iter	iv;

	if (local_variables.Size())
	{
		for (iv = local_variables.Begin(); iv != local_variables.End(); iv++)
		{
			if (strcmp(Name, (*iv)->GetName()) == 0)
			{
				return *iv;
			}
		}
	}

	return NULL;
}

bool CScript::NewLocal(Variable* Which)
{
	Variable* Check;

	Check = FindLocal(Which->GetName());
	if (Check)
	{	// already exists
		return false;
	}

	local_variables.PushBack(Which);

	return true;
}

Variable* CScript::FindParameter(const char* Name)
{
	List<Variable*>::Iter	iv;

	if (parameter_variables.Size())
	{
		for (iv = parameter_variables.Begin(); iv != parameter_variables.End(); iv++)
		{
			if (strcmp(Name, (*iv)->GetName()) == 0)
			{
				return *iv;
			}
		}
	}

	return NULL;
}

bool CScript::NewParameter(Variable* Which)
{
	Variable* Check;
	StringVar* ParmValue;
	edict_t* Search;
	Variable* temp;
	vec3_t		vec;

	Check = FindParameter(Which->GetName());
	if (Check)
	{	// already exists
		return false;
	}

	parameter_variables.PushBack(Which);

	if (!parameter_values.Size())
	{
		Error("Missing Parameter");
	}

	ParmValue = *parameter_values.Begin();
	parameter_values.Erase(parameter_values.Begin());

	switch (Which->GetType())
	{
		case TYPE_ENTITY:
			Search = G_Find(NULL, FOFS(targetname), ParmValue->GetStringValue());
			temp = new EntityVar(Search);
			break;

		case TYPE_INT:
			temp = new IntVar("parm", atol(ParmValue->GetStringValue()));
			break;

		case TYPE_FLOAT:
			temp = new FloatVar("parm", atof(ParmValue->GetStringValue()));
			break;

		case TYPE_VECTOR:
			sscanf(ParmValue->GetStringValue(), "%f %f %f", &vec[0], &vec[1], &vec[2]);
			temp = new VectorVar("parm", vec[0], vec[1], vec[2]);
			break;

		default:
			delete ParmValue;
			return false;
			break;
	}

	(*Which) = temp;

	delete temp;
	delete ParmValue;

	return true;
}