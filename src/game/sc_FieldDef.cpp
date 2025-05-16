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
	{ "x",				SPEC_X,							F_FLOAT },
	{ "y",				SPEC_Y,							F_FLOAT },
	{ "z",				SPEC_Z,							F_FLOAT },
	{ "origin",			FOFS(s.origin),					F_VECTOR },
	{ "movetype",		FOFS(movetype),					F_INT },
	{ "start_origin",	FOFS(moveinfo.start_origin),	F_VECTOR },
	{ "distance",		FOFS(moveinfo.distance),		F_FLOAT },
	{ "owner",			FOFS(owner),					F_EDICT },
	{ "wait",			FOFS(wait),						F_FLOAT },
	{ "velocity",		FOFS(velocity),					F_VECTOR },
	{ "angle_velocity",	FOFS(avelocity),				F_VECTOR },
	{ "team_chain",		FOFS(teamchain),				F_EDICT },
	{ "yaw_speed",		FOFS(yaw_speed),				F_FLOAT },
	{ "modelindex",		FOFS(s.modelindex),				F_INT },
	{ "count",			FOFS(count),					F_INT },
	{ "solid",			FOFS(solid),					F_INT },
	{ "angles",			FOFS(s.angles),					F_VECTOR },
	{ "start_angles",	FOFS(moveinfo.start_angles),	F_VECTOR },
	{ "state",			FOFS(moveinfo.state),			F_INT },
	{ "c_mode",			FOFS(monsterinfo.c_mode),		F_INT },
	{ "skinnum",		FOFS(s.skinnum),				F_INT },
	{ "ideal_yaw",		FOFS(ideal_yaw),				F_FLOAT },
	{ "delta_angles",	SPEC_DELTA_ANGLES,				F_VECTOR },
	{ "p_origin",		SPEC_P_ORIGIN,					F_VECTOR },
	{ "takedamage",		FOFS(takedamage),				F_INT },

	{ NULL,				0,								F_INT }
};

FieldDef::FieldDef(CScript* Script)
{
	field_t* Field;
	bool	Found;

	strcpy(Name, Script->ReadString());
	Type = (VariableType)Script->ReadByte();

	FieldType = F_IGNORE;
	Offset = -1;

	Found = false;
	for (Field = script_fields; Field->name; Field++)
	{
		if (strcmp(Name, Field->name) == 0)
		{
			Offset = Field->ofs;
			FieldType = Field->type;
			Found = true;

			break;
		}
	}

	if (!Found)
	{
#ifdef _DEVEL
		Com_Printf("Unknown field '%s'\n", Name);
#endif //_DEVEL
	}
}

FieldDef::FieldDef(FILE* FH, CScript* Script)
{
	int		index;
	bool	Found;
	field_t* Field;

	fread(Name, 1, sizeof(Name), FH);
	fread(&Type, 1, sizeof(Type), FH);

	fread(&index, 1, sizeof(index), FH);
	if (Script && index != -1)
	{
		Script->SetFieldIndex(index, this);
	}

	FieldType = F_IGNORE;
	Offset = -1;

	Found = false;
	for (Field = script_fields; Field->name; Field++)
	{
		if (strcmp(Name, Field->name) == 0)
		{
			Offset = Field->ofs;
			FieldType = Field->type;
			Found = true;

			break;
		}
	}

	if (!Found)
	{
#ifdef _DEVEL
		Com_Printf("Unknown field '%s'\n", Name);
#endif //_DEVEL
	}
}

void FieldDef::Write(FILE* FH, CScript* Script)
{
	int index;

	index = RLID_FIELDDEF;
	fwrite(&index, 1, sizeof(index), FH);

	fwrite(Name, 1, sizeof(Name), FH);
	fwrite(&Type, 1, sizeof(Type), FH);

	index = -1;
	if (Script)
	{
		index = Script->LookupFieldIndex(this);
	}
	fwrite(&index, 1, sizeof(index), FH);
}

byte* FieldDef::GetOffset(Variable* Var)
{
	edict_t* ent;
	byte* b, * Dest;

	Dest = NULL;

	switch (Offset)
	{
		case SPEC_X:
			break;
		case SPEC_Y:
			break;
		case SPEC_Z:
			break;
		case SPEC_DELTA_ANGLES:
			ent = Var->GetEdictValue();
			if (ent && ent->client)
			{
				Dest = (byte*)&ent->client->ps.pmove.delta_angles;
			}
			break;

		case SPEC_P_ORIGIN:
			ent = Var->GetEdictValue();
			if (ent && ent->client)
			{
				Dest = (byte*)&ent->client->playerinfo.origin;
			}
			break;

		default:
			ent = Var->GetEdictValue();
			if (ent)
			{
				b = (byte*)ent;
				Dest = b + Offset;
			}
			break;
	}

	return Dest;
}

Variable* FieldDef::GetValue(Variable* Var)
{
	vec3_t vec;

	switch (FieldType)
	{
		case F_INT:
			return new IntVar("", GetIntValue(Var));
			break;

		case F_FLOAT:
			return new FloatVar("", GetFloatValue(Var));
			break;

		case F_EDICT:
			return new EntityVar(GetEdictValue(Var));
			break;

		case F_VECTOR:
			GetVectorValue(Var, vec);
			return new VectorVar(vec);
			break;
	}

	return NULL;
}

int FieldDef::GetIntValue(Variable* Var)
{
	byte* Dest;
	vec3_t	data;

	Dest = GetOffset(Var);

	if (FieldType != F_INT || !Dest)
	{
		switch (Offset)
		{
			case SPEC_X:
				Var->GetVectorValue(data);
				return (int)data[0];
				break;
			case SPEC_Y:
				Var->GetVectorValue(data);
				return (int)data[1];
				break;
			case SPEC_Z:
				Var->GetVectorValue(data);
				return (int)data[2];
				break;
		}

		return 0.0;
	}

	return *(int*)(Dest);
}

float FieldDef::GetFloatValue(Variable* Var)
{
	byte* Dest;
	vec3_t	data;

	Dest = GetOffset(Var);

	if (FieldType != F_FLOAT || !Dest)
	{
		switch (Offset)
		{
			case SPEC_X:
				Var->GetVectorValue(data);
				return data[0];
				break;
			case SPEC_Y:
				Var->GetVectorValue(data);
				return data[1];
				break;
			case SPEC_Z:
				Var->GetVectorValue(data);
				return data[2];
				break;
		}

		return 0.0;
	}

	return *(float*)(Dest);
}

void FieldDef::GetVectorValue(Variable* Var, vec3_t& VecValue)
{
	byte* Dest;

	Dest = GetOffset(Var);

	if (FieldType != F_VECTOR || !Dest)
	{
		VectorCopy(vec3_origin, VecValue);
		return;
	}

	VectorCopy(*(vec3_t*)(Dest), VecValue);
}

edict_t* FieldDef::GetEdictValue(Variable* Var)
{
	byte* Dest;

	Dest = GetOffset(Var);

	if (FieldType != F_EDICT || !Dest)
	{
		return NULL;
	}

	return *(edict_t**)(Dest);
}

char* FieldDef::GetStringValue(Variable* Var)
{
	return "";
}

void FieldDef::SetValue(Variable* Var, Variable* Value)
{
	byte* Dest;
	vec3_t		data;
	VectorVar* new_var;

	Dest = GetOffset(Var);
	if (Dest == NULL)
	{
		switch (Offset)
		{
			case SPEC_X:
				Var->GetVectorValue(data);
				data[0] = Value->GetFloatValue();
				new_var = new VectorVar(data);
				*Var = new_var;
				delete new_var;
				break;
			case SPEC_Y:
				Var->GetVectorValue(data);
				data[1] = Value->GetFloatValue();
				new_var = new VectorVar(data);
				*Var = new_var;
				delete new_var;
				break;
			case SPEC_Z:
				Var->GetVectorValue(data);
				data[2] = Value->GetFloatValue();
				new_var = new VectorVar(data);
				*Var = new_var;
				delete new_var;
				break;
		}

		return;
	}

	switch (FieldType)
	{
		case F_INT:
			*(int*)(Dest) = Value->GetIntValue();
			break;
		case F_FLOAT:
			*(float*)(Dest) = Value->GetFloatValue();
			break;
		case F_EDICT:
			*(edict_t**)(Dest) = Value->GetEdictValue();
			break;
		case F_VECTOR:
			Value->GetVectorValue(*(vec3_t*)(Dest));
			break;
	}
}