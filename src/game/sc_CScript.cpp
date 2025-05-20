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

CScript::CScript(const char* script_name, edict_t* new_owner)
{
	Clear(true);

	owner = new_owner;
	strcpy_s(name, script_name); //mxd. strcpy -> strcpy_s.

	LoadFile();
}

CScript::CScript(FILE* f)
{
	int index;
	int size;

	Clear(true);

	fread(name, 1, sizeof(name), f);
	LoadFile();

	fread(&script_condition, 1, sizeof(script_condition), f);
	fread(&condition_info, 1, sizeof(condition_info), f);
	fread(&length, 1, sizeof(length), f);
	fread(&position, 1, sizeof(position), f);
	fread(&debug_flags, 1, sizeof(debug_flags), f);

	fread(&index, 1, sizeof(index), f);
	if (index != -1)
	{
		owner = &g_edicts[index];
		owner->Script = this;
	}

	fread(&index, 1, sizeof(index), f);
	if (index != -1)
		other = &g_edicts[index];

	fread(&index, 1, sizeof(index), f);
	if (index != -1)
		activator = &g_edicts[index];

	fread(&size, 1, sizeof(size), f);
	for (int i = 0; i < size; i++)
		RestoreObject(f, ScriptRL, this); // Fields - they'll put themselves in.

	fread(&size, 1, sizeof(size), f);
	for (int i = 0; i < size; i++)
	{
		fread(&index, 1, sizeof(index), f);

		char var_name[VAR_LENGTH];
		fread(var_name, 1, VAR_LENGTH, f);

		variable_index[index] = FindGlobal(var_name);
	}

	fread(&size, 1, sizeof(size), f);
	for (int i = 0; i < size; i++)
		local_variables.PushBack(static_cast<Variable*>(RestoreObject(f, ScriptRL, this)));

	fread(&size, 1, sizeof(size), f);
	for (int i = 0; i < size; i++)
		parameter_variables.PushBack(static_cast<Variable*>(RestoreObject(f, ScriptRL, this)));

	fread(&size, 1, sizeof(size), f);
	for (int i = 0; i < size; i++)
		stack_variables.PushBack(static_cast<Variable*>(RestoreObject(f, ScriptRL, this)));

	fread(&size, 1, sizeof(size), f);
	for (int i = 0; i < size; i++)
		waiting_variables.PushBack(static_cast<Variable*>(RestoreObject(f, ScriptRL, this)));

	fread(&size, 1, sizeof(size), f);
	for (int i = 0; i < size; i++)
		signalers.PushBack(static_cast<Signaler*>(RestoreObject(f, ScriptRL, this)));

	fread(&size, 1, sizeof(size), f);
	for (int i = 0; i < size; i++)
		parameter_values.PushBack(static_cast<StringVar*>(RestoreObject(f, ScriptRL, this)));

	fread(&size, 1, sizeof(size), f);
	for (int i = 0; i < size; i++)
		events.PushBack(static_cast<Event*>(RestoreObject(f, ScriptRL, this)));
}

CScript::~CScript()
{
	Free(true);
}

void CScript::LoadFile()
{
	length = gi.FS_LoadFile(name, reinterpret_cast<void**>(&data));

	if (length == -1)
	{
		Com_Printf("***********************************************\n");
		Com_Printf("Could not open script %s\n", name);
		Com_Printf("***********************************************\n");
	}
	else
	{
		const int version = ReadInt();

		if (version != SCRIPT_VERSION)
		{
			Com_Printf("***********************************************\n");
			Com_Printf("Bad script version for %s: found %d, expecting %d\n", name, version, SCRIPT_VERSION);
			Com_Printf("***********************************************\n");
		}
		else
		{
			script_condition = COND_READY;
		}
	}
}

void CScript::Free(const bool do_data) //TODO: do_data always true, remove?
{
	if (do_data && data != nullptr)
	{
		gi.FS_FreeFile(data);
		data = nullptr;
	}

	while (local_variables.Size())
	{
		List<Variable*>::Iter var = local_variables.Begin();
		delete *var;

		local_variables.Erase(var);
	}

	while (parameter_variables.Size())
	{
		List<Variable*>::Iter var = parameter_variables.Begin();
		delete *var;

		parameter_variables.Erase(var);
	}

	while (stack_variables.Size())
	{
		List<Variable*>::Iter var = stack_variables.Begin();
		delete *var;

		stack_variables.Erase(var);
	}

	while (waiting_variables.Size())
	{
		List<Variable*>::Iter var = waiting_variables.Begin();
		delete *var;

		waiting_variables.Erase(var);
	}

	while (signalers.Size())
	{
		List<Signaler*>::Iter signaler = signalers.Begin();
		delete *signaler;

		signalers.Erase(signaler);
	}

	while (parameter_values.Size())
	{
		List<StringVar*>::Iter param_val = parameter_values.Begin();
		delete *param_val;

		parameter_values.Erase(param_val);
	}

	while (events.Size())
	{
		List<Event*>::Iter event = events.Begin();
		delete *event;

		events.Erase(event);
	}

	for (const auto& fielddef : fielddefs)
		delete fielddef;

	Clear(do_data);
}

void CScript::Clear(const bool do_data) //TODO: unused arg, remove.
{
	data = nullptr;
	owner = nullptr;
	other = nullptr;
	activator = nullptr;

	memset(fielddefs, 0, sizeof(fielddefs));
	memset(variable_index, 0, sizeof(variable_index));

	debug_flags = 0;
	memset(name, 0, sizeof(name));

	script_condition = COND_COMPLETED;
	condition_info = 0;
	position = 0;
	length = 0;
}

void CScript::Write(FILE* f)
{
	int index = RLID_SCRIPT;
	fwrite(&index, 1, sizeof(index), f);

	fwrite(name, 1, sizeof(name), f);
	fwrite(&script_condition, 1, sizeof(script_condition), f);
	fwrite(&condition_info, 1, sizeof(condition_info), f);
	fwrite(&length, 1, sizeof(length), f);
	fwrite(&position, 1, sizeof(position), f);
	fwrite(&debug_flags, 1, sizeof(debug_flags), f);

	index = ((owner != nullptr) ? owner - g_edicts : -1);
	fwrite(&index, 1, sizeof(index), f);

	index = ((other != nullptr) ? other - g_edicts : -1);
	fwrite(&index, 1, sizeof(index), f);

	index = ((activator != nullptr) ? activator - g_edicts : -1);
	fwrite(&index, 1, sizeof(index), f);

	int size = 0;
	for (const auto& fielddef : fielddefs)
		if (fielddef != nullptr)
			size++;
	fwrite(&size, 1, sizeof(size), f);

	for (const auto& fielddef : fielddefs)
		if (fielddef != nullptr)
			fielddef->Write(f, this);

	size = 0;
	for (List<Variable*>::Iter var = GlobalVariables.Begin(); var != GlobalVariables.End(); ++var)
		if (LookupVarIndex(*var) != -1)
			size++;
	fwrite(&size, 1, sizeof(size), f);
	for (List<Variable*>::Iter var = GlobalVariables.Begin(); var != GlobalVariables.End(); ++var)
	{
		index = LookupVarIndex(*var);
		if (index != -1)
		{
			fwrite(&index, 1, sizeof(index), f);
			fwrite((*var)->GetName(), 1, VAR_LENGTH, f);
		}
	}

	size = local_variables.Size();
	fwrite(&size, 1, sizeof(size), f);
	for (List<Variable*>::Iter var = local_variables.Begin(); var != local_variables.End(); ++var)
		(*var)->Write(f, this);

	size = parameter_variables.Size();
	fwrite(&size, 1, sizeof(size), f);
	for (List<Variable*>::Iter var = parameter_variables.Begin(); var != parameter_variables.End(); ++var)
		(*var)->Write(f, this);

	size = stack_variables.Size();
	fwrite(&size, 1, sizeof(size), f);
	for (List<Variable*>::Iter var = stack_variables.Begin(); var != stack_variables.End(); ++var)
		(*var)->Write(f, this);

	size = waiting_variables.Size();
	fwrite(&size, 1, sizeof(size), f);
	for (List<Variable*>::Iter var = waiting_variables.Begin(); var != waiting_variables.End(); ++var)
		(*var)->Write(f, this);

	size = signalers.Size();
	fwrite(&size, 1, sizeof(size), f);
	for (List<Signaler*>::Iter signaler = signalers.Begin(); signaler != signalers.End(); ++signaler)
		(*signaler)->Write(f, this);

	size = parameter_values.Size();
	fwrite(&size, 1, sizeof(size), f);
	for (List<StringVar*>::Iter param_val = parameter_values.Begin(); param_val != parameter_values.End(); ++param_val)
		(*param_val)->Write(f, this);

	size = events.Size();
	fwrite(&size, 1, sizeof(size), f);
	for (List<Event*>::Iter event = events.Begin(); event != events.End(); ++event)
		(*event)->Write(f, this);
}

int CScript::LookupVarIndex(const Variable* var) const
{
	for (int i = 0; i < MAX_INDEX; i++)
		if (variable_index[i] == var)
			return i;

	return -1;
}

int	CScript::LookupFieldIndex(const FieldDef* field) const
{
	for (int i = 0; i < MAX_INDEX; i++)
		if (fielddefs[i] == field)
			return i;

	return -1;
}

void CScript::SetParameter(const char* value)
{
	parameter_values.PushBack(new StringVar("parm", value));
}

byte CScript::ReadByte()
{
	return data[position++];
}

int CScript::ReadInt()
{
	union
	{
		int value;
		byte byte_value[4];
	};

	byte_value[0] = ReadByte();
	byte_value[1] = ReadByte();
	byte_value[2] = ReadByte();
	byte_value[3] = ReadByte();

	return value;
}

float CScript::ReadFloat()
{
	union
	{
		float value;
		byte byte_value[4];
	};

	byte_value[0] = ReadByte();
	byte_value[1] = ReadByte();
	byte_value[2] = ReadByte();
	byte_value[3] = ReadByte();

	return value;
}

char* CScript::ReadString()
{
	char* start = reinterpret_cast<char*>(&data[position]); //TODO: but what if start is '\0'?
	while (ReadByte() != 0) {}

	return start;
}

Variable* CScript::ReadDeclaration(int& index)
{
	const char* var_name = ReadString();
	const int var_type = ReadByte();
	index = ReadInt();

	Variable* var = nullptr;

	switch (var_type)
	{
		case TYPE_INT:
			var = new IntVar(var_name);
			break;

		case TYPE_FLOAT:
			var = new FloatVar(var_name);
			break;

		case TYPE_VECTOR:
			var = new VectorVar(var_name);
			break;

		case TYPE_ENTITY:
			var = new EntityVar(var_name);
			break;

		case TYPE_STRING:
			var = new StringVar(var_name);
			break;

		default: // TYPE_UNKNOWN.
			break;
	}

	if (index >= MAX_INDEX)
		Error("Index out of range: %d > %d", index, MAX_INDEX);

	variable_index[index] = var;

	return var;
}

void CScript::PushStack(Variable* v)
{
	if (v == nullptr)
		Error("Illegal push");

	stack_variables.PushBack(v);
}

Variable* CScript::PopStack()
{
	if (stack_variables.Size() > 0)
	{
		const List<Variable*>::Iter var_iter = --stack_variables.End();
		Variable* var = *var_iter;
		stack_variables.PopBack();

		return var;
	}

	return nullptr;
}

void CScript::HandleGlobal(const bool assignment)
{
	int index;
	Variable* var = ReadDeclaration(index);

	if (assignment)
		var->ReadValue(this);

	if (!NewGlobal(var))
	{
		variable_index[index] = FindGlobal(var->GetName());
		delete var;
	}
}

void CScript::HandleLocal(const bool assignment)
{
	int index;
	Variable* var = ReadDeclaration(index);

	if (assignment)
		var->ReadValue(this);

	NewLocal(var);
}

void CScript::HandleParameter(const bool assignment)
{
	int index;
	Variable* var = ReadDeclaration(index);

	if (assignment)
		var->ReadValue(this);

	NewParameter(var);
}

void CScript::HandleField()
{
	auto* field = new FieldDef(this);

	const int index = ReadInt();
	if (index < 0 || index >= MAX_INDEX)
		Error("Index for field out of range: %d > %d\n", index, MAX_INDEX);

	fielddefs[index] = field;
}

void CScript::HandleGoto()
{
	position = ReadInt();
}

Variable* CScript::HandleSpawn()
{
	edict_t* ent = G_Spawn();

	for (int count = ReadByte(); count > 0; count--)
	{
		const Variable* var_name = PopStack();
		const Variable* var_value = PopStack();

		if (var_name == nullptr || var_value == nullptr)
			Error("Invalid stack for HandleSpawn()");

		const char* name_value = var_name->GetStringValue();

		for (const field_t* f = fields; f->name != nullptr; f++)
		{
			if (Q_stricmp(f->name, name_value) != 0)
				continue;

			byte* b;

			if (f->flags & FFL_SPAWNTEMP)
				b = reinterpret_cast<byte*>(&st);
			else
				b = reinterpret_cast<byte*>(ent);

			switch (f->type)
			{
				case F_LSTRING:
					*reinterpret_cast<char**>(b + f->ofs) = ED_NewString(var_value->GetStringValue());
					break;

				case F_VECTOR:
					var_value->GetVectorValue(*reinterpret_cast<vec3_t*>(b + f->ofs));
					break;

				case F_INT:
					*reinterpret_cast<int*>(b + f->ofs) = var_value->GetIntValue();
					break;

				case F_FLOAT:
					*reinterpret_cast<float*>(b + f->ofs) = var_value->GetFloatValue();
					break;

				case F_ANGLEHACK:
					reinterpret_cast<float*>(b + f->ofs)[0] = 0.0f;
					reinterpret_cast<float*>(b + f->ofs)[1] = var_value->GetFloatValue();
					reinterpret_cast<float*>(b + f->ofs)[2] = 0.0f;
					break;

				default: //mxd. Added default case.
					break;
			}

			break;
		}
	}

	ED_CallSpawn(ent);

	return new EntityVar(ent);
}

Variable* CScript::HandleBuiltinFunction()
{
	Variable* var = nullptr;
	const int index = ReadByte();

	switch (index)
	{
		case FUNC_FIND_ENTITY_WITH_TARGET:
		{
			const Variable* value = PopStack();
			edict_t* search = G_Find(nullptr, FOFS(targetname), value->GetStringValue());
			var = new EntityVar(search);
			delete value;
		} break;

		case FUNC_SIN:
		{
			const Variable* value = PopStack();
			var = new FloatVar("", sinf(DEG2RAD(value->GetFloatValue())));
			delete value;
		} break;

		case FUNC_COS:
		{
			const Variable* value = PopStack();
			var = new FloatVar("", cosf(DEG2RAD(value->GetFloatValue())));
			delete value;
		} break;

		case FUNC_FIND_ENTITY_WITH_SCRIPT:
		{
			const Variable* value = PopStack();
			edict_t* search = G_Find(nullptr, FOFS(scripttarget), value->GetStringValue());
			var = new EntityVar(search);
			delete value;
		} break;

		case FUNC_SPAWN:
			var = HandleSpawn();
			break;

		case FUNC_GET_OTHER:
			var = new EntityVar(other);
			break;

		case FUNC_GET_ACTIVATOR:
			var = new EntityVar(activator);
			break;
	}

	return var;
}

void CScript::HandlePush()
{
	Variable* var;
	const int type = ReadByte();

	switch (type)
	{
		case PUSH_CONST_INT:
			var = new IntVar();
			var->ReadValue(this);
			break;

		case PUSH_CONST_FLOAT:
			var = new FloatVar();
			var->ReadValue(this);
			break;

		case PUSH_CONST_VECTOR:
			var = new VectorVar();
			var->ReadValue(this);
			break;

		case PUSH_CONST_ENTITY:
			var = new EntityVar();
			var->ReadValue(this);
			break;

		case PUSH_CONST_STRING:
			var = new StringVar();
			var->ReadValue(this);
			break;

		case PUSH_VAR:
			var = new VariableVar();
			dynamic_cast<VariableVar*>(var)->ReadValue(this);
			break;

		case PUSH_VAR_WITH_FIELD:
			var = new FieldVariableVar();
			dynamic_cast<FieldVariableVar*>(var)->ReadValue(this);
			break;

		case PUSH_FUNCTION:
			var = HandleBuiltinFunction();
			break;

		default: //mxd. Added default case.
			return;
	}

	PushStack(var);
}

void CScript::HandlePop()
{
	const Variable* var = PopStack();
	delete var;
}

void CScript::HandleAssignment()
{
	Variable* assignee = PopStack();
	Variable* value = PopStack();

	if (value == nullptr || assignee == nullptr)
		Error("Invalid stack for Add");

	*assignee = value;

	delete assignee;
	delete value;
}

void CScript::HandleAdd()
{
	Variable* v1 = PopStack();
	Variable* v2 = PopStack();

	if (v1 == nullptr || v2 == nullptr)
		Error("Invalid stack for Add");

	PushStack(*v1 + v2);

	delete v1;
	delete v2;
}

void CScript::HandleSubtract()
{
	Variable* v1 = PopStack();
	Variable* v2 = PopStack();

	if (v1 == nullptr || v2 == nullptr)
		Error("Invalid stack for Subtract");

	PushStack(*v1 - v2);

	delete v1;
	delete v2;
}

void CScript::HandleMultiply()
{
	Variable* v1 = PopStack();
	Variable* v2 = PopStack();

	if (v1 == nullptr || v2 == nullptr)
		Error("Invalid stack for Multiply");

	PushStack(*v1 * v2);

	delete v1;
	delete v2;
}

void CScript::HandleDivide()
{
	Variable* v1 = PopStack();
	Variable* v2 = PopStack();

	if (v1 == nullptr || v2 == nullptr)
		Error("Invalid stack for Divide");

	PushStack(*v1 / v2);

	delete v1;
	delete v2;
}

void CScript::HandleDebug()
{
	int flags = ReadByte();

	if (flags != 0)
	{
		if (flags & DEBUG_ENABLE)
		{
			flags &= ~DEBUG_ENABLE;
			debug_flags |= flags;
		}
		else
		{
			debug_flags &= ~flags;
		}

		return;
	}

	StartDebug();

	if (parameter_variables.Size() > 0)
	{
		DebugLine("   Parameters:\n");
		for (List<Variable*>::Iter var = parameter_variables.Begin(); var != parameter_variables.End(); ++var)
			(*var)->Debug(this);
	}

	if (GlobalVariables.Size() > 0)
	{
		DebugLine("   Global Variables:\n");
		for (List<Variable*>::Iter var = GlobalVariables.Begin(); var != GlobalVariables.End(); ++var)
			(*var)->Debug(this);
	}

	if (local_variables.Size() > 0)
	{
		DebugLine("   Local Variables:\n");
		for (List<Variable*>::Iter var = local_variables.Begin(); var != local_variables.End(); ++var)
			(*var)->Debug(this);
	}

	EndDebug();
}

void CScript::HandleDebugStatement()
{
	DebugLine("%s\n", ReadString());
}

void CScript::HandleAddAssignment()
{
	Variable* assignee = PopStack();
	Variable* value = PopStack();

	if (value == nullptr || assignee == nullptr)
		Error("Invalid stack for AddAssignment");

	*assignee = *assignee + value;

	delete assignee;
	delete value;
}

void CScript::HandleSubtractAssignment()
{
	Variable* assignee = PopStack();
	Variable* value = PopStack();

	if (value == nullptr || assignee == nullptr)
		Error("Invalid stack for SubtractAssignment");

	*assignee = *assignee - value;

	delete assignee;
	delete value;
}

void CScript::HandleMultiplyAssignment()
{
	Variable* assignee = PopStack();
	Variable* value = PopStack();

	if (value == nullptr || assignee == nullptr)
		Error("Invalid stack for MultiplyAssignment");

	*assignee = *assignee * value;

	delete assignee;
	delete value;
}

void CScript::HandleDivideAssignment()
{
	Variable* assignee = PopStack();
	Variable* value = PopStack();

	if (value == nullptr || assignee == nullptr)
		Error("Invalid stack for DivideAssignment");

	*assignee = *assignee / value;

	delete assignee;
	delete value;
}

bool CScript::HandleWait(const bool for_all)
{
	int count = ReadByte();

	condition_info = ((count & WAIT_CLEAR) ? WAIT_CLEAR : 0);
	count &= ~WAIT_CLEAR;

	for (; count > 0; count--)
	{
		Variable* var = PopStack();
		if (var == nullptr)
			Error("Invalid stack for HandleWait");

		waiting_variables.PushBack(var);
	}

	script_condition = (for_all ? COND_WAIT_ALL : COND_WAIT_ANY);

	if (CheckWait())
	{
		FinishWait(nullptr, false);
		return false;
	}

	return true;
}

bool CScript::HandleTimeWait()
{
	const Variable* var = PopStack();

	if (var == nullptr)
		Error("Invalid stack for Time Wait");

	const float next_time = level.time + var->GetFloatValue();

	if (next_time <= level.time)
		return false;

	AddEvent(new WaitEvent(next_time));
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