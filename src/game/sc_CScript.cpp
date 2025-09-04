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

	if (var == nullptr) //mxd
		Error("Invalid var '%s' type: %i", var_name, var_type);

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
	if (stack_variables.Size() == 0)
		Error("Illegal pop"); //mxd. Throw error instead of returning nullptr.

	const List<Variable*>::Iter var_iter = --stack_variables.End();
	Variable* var = *var_iter;
	stack_variables.PopBack();

	return var;
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

void CScript::HandleIf()
{
	const int condition = ReadByte();
	const int location = ReadInt();

	Variable* v2 = PopStack();
	Variable* v1 = PopStack();

	if (v1 == nullptr || v2 == nullptr)
		Error("Invalid stack for If");

	bool result = false;

	switch (condition)
	{
		case COND_EQUAL:
			result = (*v1 == v2);
			break;

		case COND_LESS_THAN:
			result = (*v1 < v2);
			break;

		case COND_LESS_THAN_EQUAL:
			result = (*v1 <= v2);
			break;

		case COND_GREATER_THAN:
			result = (*v1 > v2);
			break;

		case COND_GREATER_THAN_EQUAL:
			result = (*v1 >= v2);
			break;

		case COND_NOT_EQUAL:
			result = (*v1 != v2);
			break;
	}

	if (!result)
		position = location;
}

void CScript::HandlePrint()
{
	const char* text_value;
	int text_index = -1; //mxd. Initialize.

	const Variable* ent_var = nullptr;
	const Variable* level_var = nullptr;
	const edict_t* ent = nullptr;
	int print_level = PRINT_HIGH;

	const int flags = ReadByte();
	const Variable* text_var = PopStack();

	if (text_var == nullptr)
		Error("Invalid stack for Print");

	if (text_var->GetType() == TYPE_STRING)
	{
		text_value = text_var->GetStringValue();
	}
	else
	{
		text_index = text_var->GetIntValue();
		text_value = message_text[text_index].string;
	}

	if (flags & PRINT_LEVEL)
	{
		level_var = PopStack();

		if (level_var == nullptr)
			Error("Invalid stack for Print");

		print_level = level_var->GetIntValue();
	}

	if (flags & PRINT_ENTITY)
	{
		ent_var = PopStack();

		if (ent_var == nullptr)
			Error("Invalid stack for Print");

		ent = ent_var->GetEdictValue();
	}

	if (!static_cast<int>(sv_jumpcinematic->value) || !static_cast<int>(sv_cinematicfreeze->value))
	{
		if ((flags & PRINT_CAPTIONED) && text_index > 0) //mxd. Add text_index check.
		{
			if (ent != nullptr)
				gi.captionprintf(ent, static_cast<short>(text_index)); // Send the ID for the text to the single player.
			else
				gi.bcaption(PRINT_HIGH, static_cast<short>(text_index)); // Send the ID for the text to all players.
		}
		else if ((flags & PRINT_CENTERED) && text_index > 0) //mxd. Add text_index check.
		{
			if (ent != nullptr)
				gi.levelmsg_centerprintf(ent, static_cast<short>(text_index)); // Send the ID over the net rather than the string itself...
		}
		else
		{
			if (ent != nullptr)
				gi.cprintf(ent, print_level, text_value);
			else
				gi.bprintf(print_level, text_value);
		}
	}

	delete text_var;
	delete ent_var;
	delete level_var;
}

void CScript::HandlePlaySound()
{
	const Variable* time_delay_var = nullptr;
	const Variable* channel_var = nullptr;
	const Variable* attenuation_var = nullptr;
	const Variable* volume_var = nullptr;
	const Variable* entity_var = nullptr;

	edict_t* ent = nullptr;
	float volume = 1.0f;
	float attenuation = ATTN_NORM;
	int channel = CHAN_VOICE;
	float time_delay = 0.0f;

	const int flags = ReadByte();

	const Variable* sound_name_var = PopStack();

	if (sound_name_var == nullptr)
		Error("Invalid stack for PlaySound");

	const char* sound_name = sound_name_var->GetStringValue();

	if (flags & PLAY_SOUND_TIMEDELAY)
	{
		time_delay_var = PopStack();

		if (time_delay_var == nullptr)
			Error("Invalid stack for PlaySound");

		time_delay = time_delay_var->GetFloatValue();
	}

	if (flags & PLAY_SOUND_CHANNEL)
	{
		channel_var = PopStack();

		if (channel_var == nullptr)
			Error("Invalid stack for PlaySound");

		channel = channel_var->GetIntValue();
	}

	if (flags & PLAY_SOUND_ATTENUATION)
	{
		attenuation_var = PopStack();

		if (attenuation_var == nullptr)
			Error("Invalid stack for PlaySound");

		attenuation = attenuation_var->GetFloatValue();
	}

	if (flags & PLAY_SOUND_VOLUME)
	{
		volume_var = PopStack();

		if (volume_var == nullptr)
			Error("Invalid stack for PlaySound");

		volume = volume_var->GetFloatValue();
	}

	if (flags & PLAY_SOUND_ENTITY)
	{
		entity_var = PopStack();

		if (entity_var == nullptr)
			Error("Invalid stack for PlaySound");

		ent = entity_var->GetEdictValue();
	}

	// In cinematic freezes, all sounds should be at full volume.
	if (static_cast<int>(sv_cinematicfreeze->value))
	{
		attenuation = ATTN_NONE;
		cinematic_sounds[cinematic_sounds_count].ent = ent;
		cinematic_sounds[cinematic_sounds_count].channel = channel;

		if (cinematic_sounds_count < MAX_CINEMATIC_SOUNDS - 1)
			cinematic_sounds_count++;
	}

	if (!static_cast<int>(sv_jumpcinematic->value) || !static_cast<int>(sv_cinematicfreeze->value))
		gi.sound(ent, channel, gi.soundindex(sound_name), volume, attenuation, time_delay);

	delete sound_name_var;
	delete time_delay_var;
	delete channel_var;
	delete attenuation_var;
	delete volume_var;
	delete entity_var;
}

void CScript::HandleFeature(const bool enable)
{
	const int feature_type = ReadByte();

	switch (feature_type)
	{
		case FEATURE_TRIGGER:
			HandleTrigger(enable);
			break;

		case FEATURE_CINEMATICS:
			if (enable)
			{
				cinematic_sounds_count = 0;
				Cvar_Set("sv_cinematicfreeze", "1");
				RemoveNonCinematicEntities();
			}
			else
			{
				if (sv_jumpcinematic->value == 2.0f) // Jump sent from client.
				{
					Cvar_Set("sv_jumpcinematic", "0");
					const int null_snd = gi.soundindex("misc/null.wav");

					gi.bcaption(PRINT_HIGH, 270); // Send the ID for the text to all players.

					// Stop all cinematic sounds.
					for (int i = 0; i < cinematic_sounds_count; i++)
						if (cinematic_sounds[i].ent != nullptr) // Does the entity still exist?
							gi.sound(cinematic_sounds[i].ent, cinematic_sounds[i].channel, null_snd, 1.0f, ATTN_NORM, 0.0f);
				}

				Cvar_Set("sv_cinematicfreeze", "0");
				ReinstateNonCinematicEntities();
			}
			break;

		default: // FEATURE_AMBIENT_SOUNDS, FEATURE_PLAGUE_SKINS.
			break;
	}
}

void CScript::HandleCacheSound()
{
	const Variable* sound_name_var = PopStack();

	if (sound_name_var == nullptr)
		Error("Invalid stack for HandleChacheSound");

	if (!static_cast<int>(sv_jumpcinematic->value) || !static_cast<int>(sv_cinematicfreeze->value))
		gi.soundindex(sound_name_var->GetStringValue());

	delete sound_name_var;
}

void CScript::HandleMove()
{
	const int flags = ReadByte();

	Variable* signaler_var = ((flags & MOVE_SIGNALER) ? PopStack() : nullptr);
	const Variable* move_rate_var = ((flags & MOVE_RATE) ? PopStack() : nullptr);
	const Variable* move_duration_var = ((flags & MOVE_DURATION) ? PopStack() : nullptr);
	const Variable* move_amount_var = PopStack();
	const Variable* entity_var = PopStack();

	edict_t* ent = entity_var->GetEdictValue();

	if (ent != nullptr)
	{
		vec3_t vec;
		move_amount_var->GetVectorValue(vec);

		if (move_rate_var == nullptr && move_duration_var == nullptr)
		{
			Vec3AddAssign(vec, ent->s.origin);

			if (ent->chain != nullptr)
				Vec3AddAssign(vec, ent->chain->s.origin);
		}
		else
		{
			vec3_t dest;

			if (flags & MOVE_ABSOLUTE)
				VectorCopy(vec, dest);
			else
				VectorAdd(ent->s.origin, vec, dest);

			vec3_t diff;
			VectorSubtract(ent->s.origin, dest, diff);

			float move_rate;
			if (move_rate_var != nullptr && move_duration_var != nullptr)
			{
				move_rate = move_rate_var->GetFloatValue();
				const float dist = move_rate * move_duration_var->GetFloatValue();

				VectorNormalize(diff);
				VectorMA(ent->s.origin, dist, diff, dest);
			}
			else if (move_rate_var != nullptr)
			{
				move_rate = move_rate_var->GetFloatValue();
			}
			else
			{
				move_rate = VectorLength(diff) / move_duration_var->GetFloatValue(); //BUGFIX: mxd. Divided by move_rate_var->GetFloatValue() in original logic (which is nullptr here).
			}

			ent->moveinfo.decel = move_rate;
			ent->moveinfo.accel = move_rate;
			ent->moveinfo.speed = move_rate;

			if (debug_flags & DEBUG_MOVE)
			{
				StartDebug();
				DebugLine("   Moving Entity %d\n", entity_var->GetIntValue());
				DebugLine("      From (%7.3f, %7.3f, %7.3f)\n", ent->s.origin[0], ent->s.origin[1], ent->s.origin[2]);
				DebugLine("      To   (%7.3f, %7.3f, %7.3f)\n", dest[0], dest[1], dest[2]);
				EndDebug();
			}

			if (signaler_var != nullptr)
				AddSignaler(ent, signaler_var, SIGNAL_MOVE);

			Move(ent, dest);
		}
	}

	delete move_amount_var;
	delete entity_var;
	delete move_rate_var;
	delete move_duration_var;
}

void CScript::HandleRotate()
{
	const int flags = ReadByte();

	Variable* signaler_var = ((flags & ROTATE_SIGNALER) ? PopStack() : nullptr);
	const Variable* rotation_rate_var = ((flags & ROTATE_RATE) ? PopStack() : nullptr);
	const Variable* rotation_duration_var = ((flags & ROTATE_DURATION) ? PopStack() : nullptr);
	const Variable* rotation_amount_var = PopStack();
	const Variable* entity_var = PopStack();

	vec3_t vec;
	rotation_amount_var->GetVectorValue(vec);

	edict_t* ent = entity_var->GetEdictValue();

	if (ent != nullptr)
	{
		if (rotation_rate_var == nullptr && rotation_duration_var == nullptr)
		{
			Vec3AddAssign(vec, ent->s.angles);

			if (ent->chain != nullptr)
				Vec3AddAssign(vec, ent->chain->s.angles);
		}
		else
		{
			vec3_t dest;

			if (flags & MOVE_ABSOLUTE)
				VectorCopy(vec, dest);
			else
				VectorAdd(ent->s.angles, vec, dest);

			vec3_t diff;
			VectorSubtract(ent->s.angles, dest, diff);

			if (rotation_rate_var != nullptr && rotation_duration_var != nullptr)
			{
				ent->moveinfo.speed = rotation_rate_var->GetFloatValue();
				const float dist = rotation_rate_var->GetFloatValue() * rotation_duration_var->GetFloatValue();

				VectorNormalize(diff);
				VectorMA(ent->s.angles, dist, diff, dest);
			}
			else if (rotation_rate_var != nullptr)
			{
				ent->moveinfo.speed = rotation_rate_var->GetFloatValue();
			}
			else
			{
				ent->moveinfo.speed = VectorLength(diff) / rotation_duration_var->GetFloatValue();
			}

			VectorCopy(dest, ent->moveinfo.start_angles);
			VectorCopy(dest, ent->moveinfo.end_angles);

			if (debug_flags & DEBUG_ROTATE)
			{
				StartDebug();
				DebugLine("   Rotating Entity %d\n", entity_var->GetIntValue());
				DebugLine("      From (%7.3f, %7.3f, %7.3f)\n", ent->s.angles[0], ent->s.angles[1], ent->s.angles[2]);
				DebugLine("      To   (%7.3f, %7.3f, %7.3f)\n", ent->moveinfo.end_angles[0], ent->moveinfo.end_angles[1], ent->moveinfo.end_angles[2]);
				EndDebug();
			}

			if (signaler_var != nullptr)
				AddSignaler(ent, signaler_var, SIGNAL_ROTATE);

			Rotate(ent);
		}
	}

	delete rotation_amount_var;
	delete entity_var;
	delete rotation_rate_var;
	delete rotation_duration_var;
}

void CScript::HandleUse()
{
	const Variable* entity_var = PopStack();
	edict_t* use_ent = entity_var->GetEdictValue();

	if (use_ent != nullptr && use_ent->use != nullptr)
		use_ent->use(use_ent, other, activator);

	delete entity_var;
}

void CScript::HandleTrigger(const bool enable)
{
	const Variable* entity_var = PopStack();
	edict_t* trigger_ent = entity_var->GetEdictValue();

	if (trigger_ent != nullptr)
	{
		if (enable)
		{
			trigger_ent->solid = SOLID_TRIGGER;
			trigger_ent->use = TriggerMultipleUse;
			gi.linkentity(trigger_ent);
		}
		else
		{
			trigger_ent->solid = SOLID_NOT;
			trigger_ent->use = nullptr;
		}
	}

	delete entity_var; //mxd. Missing in original logic.
}

void CScript::HandleAnimate()
{
	const int flags = ReadByte();

	const Variable* source_var = ((flags & ANIMATE_SOURCE) ? PopStack() : nullptr);
	Variable* signaler_var = ((flags & ANIMATE_SIGNALER) ? PopStack() : nullptr);

	const Variable* moving_var;
	vec3_t moving_val;

	if (flags & ANIMATE_MOVING)
	{
		moving_var = PopStack();
		moving_var->GetVectorValue(moving_val);
	}
	else
	{
		moving_var = nullptr;
		VectorCopy(vec3_origin, moving_val);
	}

	const Variable* turning_var;
	vec3_t turning_val;

	if (flags & ANIMATE_TURNING)
	{
		turning_var = PopStack();
		turning_var->GetVectorValue(turning_val);
	}
	else
	{
		turning_var = nullptr;
		VectorCopy(vec3_origin, turning_val);
	}

	const Variable* repeat_var;
	int repeat_val;

	if (flags & ANIMATE_REPEAT)
	{
		repeat_var = PopStack();
		repeat_val = repeat_var->GetIntValue();
	}
	else
	{
		repeat_var = nullptr;
		repeat_val = 0;
	}

	const Variable* action_var = PopStack();
	const int action_val = action_var->GetIntValue();

	const Variable* entity_var = PopStack();
	edict_t* ent = entity_var->GetEdictValue();

	if (ent != nullptr)
	{
		void (*signaler_routine)(edict_t*) = nullptr;

		if (signaler_var != nullptr)
		{
			AddSignaler(ent, signaler_var, SIGNAL_ANIMATE);
			signaler_routine = animate_signaler;
		}

		QPostMessage(ent, static_cast<G_MsgID_e>(msg_animtype[action_val]), PRI_DIRECTIVE, "iiige", static_cast<int>(moving_val[0]), static_cast<int>(turning_val[0]), repeat_val, signaler_routine, activator);
	}

	delete action_var;
	delete entity_var;
	delete source_var;
	delete repeat_var;
	delete turning_var;
	delete moving_var;
}

void CScript::HandleCopyPlayerAttributes()
{
	const Variable* destination_var = PopStack();
	if (destination_var == nullptr)
		Error("Invalid stack for HandleCopyPlayerAttributes()");

	edict_t* destination_ent = destination_var->GetEdictValue();

	const Variable* player_var = PopStack();
	if (player_var == nullptr)
		Error("Invalid stack for HandleCopyPlayerAttributes()");

	const edict_t* player_ent = player_var->GetEdictValue();

	CinematicSwapPlayer(player_ent, destination_ent);
}

void CScript::HandleSetViewAngles()
{
	const Variable* angles_var = PopStack();
	if (angles_var == nullptr)
		Error("Invalid stack for HandleSetViewAngles()");

	vec3_t hold_angles;
	angles_var->GetVectorValue(hold_angles);

	const Variable* player_var = PopStack();
	if (player_var == nullptr)
		Error("Invalid stack for HandleSetViewAngles()");

	edict_t* player_ent = player_var->GetEdictValue();

	// Set angles.
	player_ent->client->ps.pmove.delta_angles[PITCH] = 0;
	player_ent->client->ps.pmove.delta_angles[YAW] = ANGLE2SHORT(hold_angles[YAW] - player_ent->client->resp.cmd_angles[YAW]);
	player_ent->client->ps.pmove.delta_angles[ROLL] = 0;

	player_ent->s.angles[PITCH] = 0.0f;
	player_ent->s.angles[YAW] = hold_angles[YAW];
	player_ent->s.angles[ROLL] = 0.0f;
}

void CScript::HandleSetCacheSize()
{
	const Variable* cache_size_var = PopStack();
	if (cache_size_var == nullptr)
		Error("Invalid stack for HandleSetCacheSize()");
}

void CScript::Move_Done(edict_t* ent)
{
	VectorClear(ent->velocity);
	VectorCopy(ent->moveinfo.end_origin, ent->s.origin);
}

void CScript::Move(edict_t* ent, const vec3_t dest)
{
	VectorCopy(dest, ent->moveinfo.end_origin);

	VectorSubtract(ent->moveinfo.end_origin, ent->s.origin, ent->moveinfo.dir);
	ent->moveinfo.remaining_distance = VectorNormalize(ent->moveinfo.dir);

	float frames;

	if (ent->moveinfo.remaining_distance <= 0.0f)
		frames = 0.0f;
	else
		frames = floorf(ent->moveinfo.remaining_distance / ent->moveinfo.speed / FRAMETIME) + 1.0f;

	VectorScale(ent->moveinfo.dir, ent->moveinfo.remaining_distance / frames / FRAMETIME, ent->velocity);

	AddEvent(new MoveDoneEvent(level.time + frames * FRAMETIME, ent));
}

void CScript::Rotate_Done(edict_t* ent)
{
	VectorClear(ent->avelocity);
}

void CScript::Rotate(edict_t* ent)
{
	float frames;

	if (ent->moveinfo.speed <= 0.0f)
	{
		frames = 0.0f;
		VectorClear(ent->avelocity);
	}
	else
	{
		vec3_t dest_delta;
		VectorSubtract(ent->moveinfo.start_angles, ent->s.angles, dest_delta);
		const float distance = VectorNormalize(dest_delta);

		frames = floorf(distance / ent->moveinfo.speed / FRAMETIME) + 1.0f;
		VectorScale(dest_delta, distance / frames / FRAMETIME, ent->avelocity);
	}

	AddEvent(new RotateDoneEvent(level.time + frames * FRAMETIME, ent));
}

void CScript::AddEvent(Event* which)
{
	if (events.Size() > 0)
	{
		const float time = which->GetTime();
		List<Event*>::Iter ei;

		// Insert by event time.
		for (ei = events.Begin(); ei != events.End(); ++ei)
			if ((*ei)->GetTime() > time)
				break;

		events.Insert(ei, which);
	}
	else
	{
		events.PushBack(which);
	}
}

void CScript::ProcessEvents()
{
	while (events.Size() > 0)
	{
		List<Event*>::Iter event = events.Begin();

		if (!(*event)->Process(this))
			break;

		delete *event;
		events.Erase(event);
	}
}

void CScript::ClearTimeWait()
{
	if (script_condition == COND_WAIT_TIME)
		script_condition = COND_READY;
}

void CScript::AddSignaler(edict_t* edict, Variable* var, const SignalT signal_type)
{
	auto* new_signaler = new Signaler(edict, var, signal_type);

	// Note that this check does not need to be in there - signalers are very flexible, but if used incorrectly,
	// they can result in weird behavior - this check prevents more than one command using the same signal variable prior to a wait command.
	for (List<Signaler*>::Iter signaler = signalers.Begin(); signaler != signalers.End(); ++signaler)
		if (*(*signaler) == new_signaler)
			Error("AddSignaler: variable '%s' is being used for multiple signals", var->GetName());

	signalers.PushBack(new_signaler);
}

void CScript::CheckSignalers(edict_t* which, const SignalT signal_type)
{
	bool check_wait = false;

	if (signalers.Size() > 0)
	{
		List<Signaler*>::Iter next;

		for (List<Signaler*>::Iter signaler = signalers.Begin(); signaler != signalers.End(); signaler = next)
		{
			next = signaler;
			++next;

			if ((*signaler)->Test(which, signal_type))
			{
				delete *signaler;
				signalers.Erase(signaler);

				check_wait = true;
			}
		}
	}

	if (check_wait && (script_condition == COND_WAIT_ANY || script_condition == COND_WAIT_ALL) && CheckWait())
		FinishWait(which, true);
}

bool CScript::CheckWait()
{
	int needed_vars;

	if (script_condition == COND_WAIT_ALL)
		needed_vars = waiting_variables.Size();
	else if (script_condition == COND_WAIT_ANY)
		needed_vars = 1;
	else if (script_condition == COND_READY)
		return true;
	else // COND_WAIT_TIME, COND_COMPLETED, COND_SUSPENDED.
		return false;

	int num_vars = 0;

	if (waiting_variables.Size() > 0)
	{
		for (List<Variable*>::Iter var = waiting_variables.Begin(); var != waiting_variables.End(); ++var)
			if ((*var)->GetIntValue() != 0)
				num_vars++;
	}

	if (num_vars == needed_vars)
	{
		script_condition = COND_READY;
		return true;
	}

	return false;
}

void CScript::FinishWait(edict_t* which, const bool execute) //mxd. Second var named 'no_execute' in original logic.
{
	if (waiting_variables.Size() > 0)
	{
		for (List<Variable*>::Iter var = waiting_variables.Begin(); var != waiting_variables.End(); ++var)
		{
			if (condition_info == WAIT_CLEAR)
				(*var)->ClearSignal();

			delete *var;
		}
	}

	waiting_variables.Erase(waiting_variables.Begin(), waiting_variables.End());

	if (execute)
		Execute(which, nullptr);
}

[[noreturn]] void CScript::Error(const char* format, ...)
{
	va_list argptr;
	char text[1024];

	va_start(argptr, format);
	vsprintf_s(text, format, argptr); //mxd. vsprintf -> vsprintf_s.
	va_end(argptr);

	gi.error(text);
}

void CScript::StartDebug()
{
	DebugLine("-------------------------------\n");
	DebugLine("Script: %s\n", name);
	DebugLine("   DEBUG at %d\n", position);
}

void CScript::EndDebug()
{
	DebugLine("-------------------------------\n");
}

void CScript::DebugLine(const char* format, ...)
{
	va_list argptr;
	char text[1024];

	va_start(argptr, format);
	vsprintf_s(text, format, argptr); //mxd. vsprintf -> vsprintf_s.
	va_end(argptr);

	Com_Printf("%s", text);

#ifdef _DEBUG
	OutputDebugString(text);
#endif
}

void CScript::Think()
{
	ProcessEvents();
}

ScriptConditionT CScript::Execute(edict_t* new_other, edict_t* new_activator)
{
	if (script_condition != COND_READY)
		return script_condition;

	if (debug_flags & DEBUG_TIME)
	{
		StartDebug();
		DebugLine("   Current Time: %10.1f\n", level.time);
		EndDebug();
	}

	if (new_other != nullptr)
		other = new_other;

	if (new_activator != nullptr)
		activator = new_activator;

	int instructions_count = 0;
	bool done = false;

	while (!done)
	{
		instructions_count++;

		if (instructions_count > MAX_INSTRUCTIONS)
			Error("Runaway loop for script");

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
				done = true;
				break;

			case CODE_SUSPEND:
				done = true;
				break;

			case CODE_DEBUG:
				HandleDebug();
				break;

			case CODE_WAIT_SECONDS:
				done = HandleTimeWait();
				break;

			case CODE_WAIT_ALL:
				done = HandleWait(true);
				break;

			case CODE_WAIT_ANY:
				done = HandleWait(false);
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
				done = true;
				break;
		}

		if (position >= length)
		{
			done = true;
			script_condition = COND_COMPLETED;
		}
	}

	return script_condition;
}

Variable* CScript::FindLocal(const char* var_name)
{
	if (local_variables.Size() > 0)
	{
		for (List<Variable*>::Iter var = local_variables.Begin(); var != local_variables.End(); ++var)
			if (strcmp(var_name, (*var)->GetName()) == 0)
				return *var;
	}

	return nullptr;
}

bool CScript::NewLocal(Variable* which)
{
	if (FindLocal(which->GetName()) == nullptr)
	{
		local_variables.PushBack(which);
		return true;
	}

	// Already exists.
	return false;
}

Variable* CScript::FindParameter(const char* param_name)
{
	if (parameter_variables.Size() > 0)
	{
		for (List<Variable*>::Iter var = parameter_variables.Begin(); var != parameter_variables.End(); ++var)
			if (strcmp(param_name, (*var)->GetName()) == 0)
				return *var;
	}

	return nullptr;
}

bool CScript::NewParameter(Variable* which)
{
	if (FindParameter(which->GetName()) != nullptr)
		return false; // Already exists.

	parameter_variables.PushBack(which);

	if (parameter_values.Size() == 0)
		Error("Missing Parameter");

	const StringVar* parm_value = *parameter_values.Begin();
	parameter_values.Erase(parameter_values.Begin());

	Variable* temp;

	switch (which->GetType())
	{
		case TYPE_ENTITY:
			temp = new EntityVar(G_Find(nullptr, FOFS(targetname), parm_value->GetStringValue()));
			break;

		case TYPE_INT:
			temp = new IntVar("parm", strtol(parm_value->GetStringValue(), nullptr, 10)); //mxd. atol -> strtol.
			break;

		case TYPE_FLOAT:
			temp = new FloatVar("parm", static_cast<float>(strtod(parm_value->GetStringValue(), nullptr))); //mxd. atof -> strtod.
			break;

		case TYPE_VECTOR:
		{
			vec3_t vec;
			sscanf_s(parm_value->GetStringValue(), "%f %f %f", &vec[0], &vec[1], &vec[2]); //mxd. sscanf -> sscanf_s.
			temp = new VectorVar("parm", vec[0], vec[1], vec[2]);
		} break;

		default:
			delete parm_value;
			return false;
	}

	*which = temp;

	delete temp;
	delete parm_value;

	return true;
}