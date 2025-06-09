//
// sc_FieldDef.cpp
//
// Copyright 1998 Raven Software
//

#include "sc_FieldDef.h"
#include "sc_CScript.h"
#include "sc_EntityVar.h"
#include "sc_FloatVar.h"
#include "sc_IntVar.h"
#include "sc_VectorVar.h"
#include "sc_Utility.h"
#include "Vector.h"

// Fields are just yucky now - once H2 finals, I'm going to change them completely.

#define SPEC_X				(-1)
#define SPEC_Y				(-2)
#define SPEC_Z				(-3)
#define SPEC_DELTA_ANGLES	(-4)
#define SPEC_P_ORIGIN		(-5)

static field_t script_fields[] =
{
	{ "x",				SPEC_X,							F_FLOAT,	FFL_NONE },
	{ "y",				SPEC_Y,							F_FLOAT,	FFL_NONE },
	{ "z",				SPEC_Z,							F_FLOAT,	FFL_NONE },
	{ "origin",			FOFS(s.origin),					F_VECTOR,	FFL_RELINK }, //mxd. Needs re-linking...
	{ "movetype",		FOFS(movetype),					F_INT,		FFL_NONE },
	{ "start_origin",	FOFS(moveinfo.start_origin),	F_VECTOR,	FFL_NONE },
	{ "distance",		FOFS(moveinfo.distance),		F_FLOAT,	FFL_NONE },
	{ "owner",			FOFS(owner),					F_EDICT,	FFL_NONE },
	{ "wait",			FOFS(wait),						F_FLOAT,	FFL_NONE },
	{ "velocity",		FOFS(velocity),					F_VECTOR,	FFL_NONE },
	{ "angle_velocity",	FOFS(avelocity),				F_VECTOR,	FFL_NONE },
	{ "team_chain",		FOFS(teamchain),				F_EDICT,	FFL_NONE },
	{ "yaw_speed",		FOFS(yaw_speed),				F_FLOAT,	FFL_NONE },
	{ "modelindex",		FOFS(s.modelindex),				F_INT,		FFL_NONE },
	{ "count",			FOFS(count),					F_INT,		FFL_NONE },
	{ "solid",			FOFS(solid),					F_INT,		FFL_RELINK }, //mxd. Needs re-linking...
	{ "angles",			FOFS(s.angles),					F_VECTOR,	FFL_NONE },
	{ "start_angles",	FOFS(moveinfo.start_angles),	F_VECTOR,	FFL_NONE },
	{ "state",			FOFS(moveinfo.state),			F_INT,		FFL_NONE },
	{ "c_mode",			FOFS(monsterinfo.c_mode),		F_INT,		FFL_NONE },
	{ "skinnum",		FOFS(s.skinnum),				F_INT,		FFL_NONE },
	{ "ideal_yaw",		FOFS(ideal_yaw),				F_FLOAT,	FFL_NONE },
	{ "delta_angles",	SPEC_DELTA_ANGLES,				F_VECTOR,	FFL_NONE },
	{ "p_origin",		SPEC_P_ORIGIN,					F_VECTOR,	FFL_NONE },
	{ "takedamage",		FOFS(takedamage),				F_INT,		FFL_NONE },

	{ nullptr, 0, F_INT, FFL_NONE }
};

FieldDef::FieldDef(CScript* script)
{
	strcpy_s(name, sizeof(name), script->ReadString()); //mxd. strcpy -> strcpy_s.
	type = static_cast<VariableType>(script->ReadByte());

	for (const field_t* field = script_fields; field->name != nullptr; field++)
	{
		if (strcmp(name, field->name) == 0)
		{
			offset = field->ofs;
			field_type = field->type;
			field_flags = field->flags; //mxd

			break;
		}
	}
}

FieldDef::FieldDef(FILE* f, CScript* script)
{
	fread(name, 1, sizeof(name), f);
	fread(&type, 1, sizeof(type), f);

	int index;
	fread(&index, 1, sizeof(index), f);

	if (script != nullptr && index != -1)
		script->SetFieldIndex(index, this);

	for (const field_t* field = script_fields; field->name != nullptr; field++)
	{
		if (strcmp(name, field->name) == 0)
		{
			offset = field->ofs;
			field_type = field->type;
			field_flags = field->flags; //mxd

			break;
		}
	}
}

void FieldDef::Write(FILE* f, const CScript* script)
{
	constexpr int index = RLID_FIELDDEF;
	fwrite(&index, 1, sizeof(index), f);

	fwrite(name, 1, sizeof(name), f);
	fwrite(&type, 1, sizeof(type), f);

	const int field_index = ((script != nullptr) ? script->LookupFieldIndex(this) : -1);
	fwrite(&field_index, 1, sizeof(field_index), f);
}

byte* FieldDef::GetOffset(const Variable* var) const
{
	byte* dest = nullptr;

	switch (offset)
	{
		case SPEC_X:
		case SPEC_Y:
		case SPEC_Z:
			break;

		case SPEC_DELTA_ANGLES:
		{
			const edict_t* ent = var->GetEdictValue();
			if (ent != nullptr && ent->client != nullptr)
				dest = reinterpret_cast<byte*>(&ent->client->ps.pmove.delta_angles);
		} break;

		case SPEC_P_ORIGIN:
		{
			const edict_t* ent = var->GetEdictValue();
			if (ent != nullptr && ent->client != nullptr)
				dest = reinterpret_cast<byte*>(&ent->client->playerinfo.origin);
		} break;

		default:
		{
			edict_t* ent = var->GetEdictValue();
			if (ent != nullptr)
				dest = reinterpret_cast<byte*>(ent) + offset;
		} break;
	}

	return dest;
}

Variable* FieldDef::GetValue(Variable* var) const
{
	switch (field_type)
	{
		case F_INT:
			return new IntVar("", GetIntValue(var));

		case F_FLOAT:
			return new FloatVar("", GetFloatValue(var));

		case F_EDICT:
			return new EntityVar(GetEdictValue(var));

		case F_VECTOR:
		{
			vec3_t vec;
			GetVectorValue(var, vec);
			return new VectorVar(vec);
		}

		default:
			return nullptr;
	}
}

int FieldDef::GetIntValue(Variable* var) const
{
	byte* dest = GetOffset(var);

	if (field_type != F_INT || dest == nullptr)
	{
		vec3_t data;

		switch (offset)
		{
			case SPEC_X:
				var->GetVectorValue(data);
				return static_cast<int>(data[0]);

			case SPEC_Y:
				var->GetVectorValue(data);
				return static_cast<int>(data[1]);

			case SPEC_Z:
				var->GetVectorValue(data);
				return static_cast<int>(data[2]);

			default:
				return 0;
		}
	}

	return *reinterpret_cast<int*>(dest);
}

float FieldDef::GetFloatValue(Variable* var) const
{
	byte* dest = GetOffset(var);

	if (field_type != F_FLOAT || dest == nullptr)
	{
		vec3_t data;

		switch (offset)
		{
			case SPEC_X:
				var->GetVectorValue(data);
				return data[0];

			case SPEC_Y:
				var->GetVectorValue(data);
				return data[1];

			case SPEC_Z:
				var->GetVectorValue(data);
				return data[2];

			default:
				return 0.0f;
		}
	}

	return *reinterpret_cast<float*>(dest);
}

void FieldDef::GetVectorValue(Variable* var, vec3_t& value) const
{
	byte* dest = GetOffset(var);

	if (field_type != F_VECTOR || dest == nullptr)
		VectorCopy(vec3_origin, value);
	else
		VectorCopy(*reinterpret_cast<vec3_t*>(dest), value);
}

edict_t* FieldDef::GetEdictValue(Variable* var) const
{
	byte* dest = GetOffset(var);

	if (field_type != F_EDICT || dest == nullptr)
		return nullptr;

	return *reinterpret_cast<edict_t**>(dest);
}

const char* FieldDef::GetStringValue(Variable* var) const
{
	return "";
}

void FieldDef::SetValue(Variable* var, Variable* value) const
{
	byte* dest = GetOffset(var);

	if (dest == nullptr)
	{
		switch (offset)
		{
			case SPEC_X:
			{
				vec3_t data;
				var->GetVectorValue(data);
				data[0] = value->GetFloatValue();
				auto* new_var = new VectorVar(data);
				*var = new_var;
				delete new_var;
			} break;

			case SPEC_Y:
			{
				vec3_t data;
				var->GetVectorValue(data);
				data[1] = value->GetFloatValue();
				auto* new_var = new VectorVar(data);
				*var = new_var;
				delete new_var;
			} break;

			case SPEC_Z:
			{
				vec3_t data;
				var->GetVectorValue(data);
				data[2] = value->GetFloatValue();
				auto* new_var = new VectorVar(data);
				*var = new_var;
				delete new_var;
			} break;

			default:
				break;
		}
	}
	else
	{
		switch (field_type)
		{
			case F_INT:
				*reinterpret_cast<int*>(dest) = value->GetIntValue();
				break;

			case F_FLOAT:
				*reinterpret_cast<float*>(dest) = value->GetFloatValue();
				break;

			case F_EDICT:
				*reinterpret_cast<edict_t**>(dest) = value->GetEdictValue();
				break;

			case F_VECTOR:
				value->GetVectorValue(*reinterpret_cast<vec3_t*>(dest));
				break;

			default:
				break;
		}
	}

	//BUGFIX: mxd. Entity needs to be relinked when origin, mins, maxs, or solid changes.
	if (field_flags & FFL_RELINK)
	{
		edict_t* ent = var->GetEdictValue();

		if (ent != nullptr)
			gi.linkentity(ent);
		else
			Com_Printf("FieldDef::SetValue: failed to relink field %i!\n", field_type);
	}
}