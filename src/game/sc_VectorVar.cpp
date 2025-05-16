//
// sc_VectorVar.cpp
//
// Copyright 1998 Raven Software
//

#include "sc_VectorVar.h"
#include "sc_CScript.h"
#include "sc_Utility.h"
#include "Vector.h"

VectorVar::VectorVar(char* Name, float InitValueX, float InitValueY, float InitValueZ)
	:Variable(Name, TYPE_VECTOR)
{
	Value[0] = InitValueX;
	Value[1] = InitValueY;
	Value[2] = InitValueZ;
}

VectorVar::VectorVar(vec3_t NewValue)
	:Variable("", TYPE_VECTOR)
{
	VectorCopy(NewValue, Value);
}

VectorVar::VectorVar(FILE* FH, CScript* Script)
	:Variable(FH, Script)
{
	fread(&Value, 1, sizeof(Value), FH);
}

void VectorVar::Write(FILE* FH, CScript* Script, int ID)
{
	Variable::Write(FH, Script, RLID_VECTORVAR);

	fwrite(Value, 1, sizeof(Value), FH);
}

void VectorVar::GetVectorValue(vec3_t& VecValue)
{
	VecValue[0] = Value[0];
	VecValue[1] = Value[1];
	VecValue[2] = Value[2];
}

void VectorVar::ReadValue(CScript* Script)
{
	Value[0] = Script->ReadFloat();
	Value[1] = Script->ReadFloat();
	Value[2] = Script->ReadFloat();
}

void VectorVar::Debug(CScript* Script)
{
	Variable::Debug(Script);

	Script->DebugLine("      Vector Value: [%0.f, %0.f, %0.f]\n", Value[0], Value[1], Value[2]);
}

Variable* VectorVar::operator +(Variable* VI)
{
	vec3_t V2, NewV;

	if (VI->GetType() == TYPE_INT || VI->GetType() == TYPE_FLOAT)
	{
		V2[0] = V2[1] = V2[2] = VI->GetFloatValue();
	}
	else
	{
		VI->GetVectorValue(V2);
	}

	NewV[0] = Value[0] + V2[0];
	NewV[1] = Value[1] + V2[1];
	NewV[2] = Value[2] + V2[2];

	return new VectorVar("", NewV[0], NewV[1], NewV[2]);
}

Variable* VectorVar::operator -(Variable* VI)
{
	vec3_t	V2, NewV;

	if (VI->GetType() == TYPE_INT || VI->GetType() == TYPE_FLOAT)
	{
		V2[0] = V2[1] = V2[2] = VI->GetFloatValue();
	}
	else
	{
		VI->GetVectorValue(V2);
	}

	NewV[0] = Value[0] - V2[0];
	NewV[1] = Value[1] - V2[1];
	NewV[2] = Value[2] - V2[2];

	return new VectorVar("", NewV[0], NewV[1], NewV[2]);
}

Variable* VectorVar::operator *(Variable* VI)
{
	vec3_t V2, NewV;

	if (VI->GetType() == TYPE_INT || VI->GetType() == TYPE_FLOAT)
	{
		V2[0] = V2[1] = V2[2] = VI->GetFloatValue();
	}
	else
	{
		VI->GetVectorValue(V2);
	}

	NewV[0] = Value[0] * V2[0];
	NewV[1] = Value[1] * V2[1];
	NewV[2] = Value[2] * V2[2];

	return new VectorVar("", NewV[0], NewV[1], NewV[2]);
}

Variable* VectorVar::operator /(Variable* VI)
{
	vec3_t V2, NewV;

	if (VI->GetType() == TYPE_INT || VI->GetType() == TYPE_FLOAT)
	{
		V2[0] = V2[1] = V2[2] = VI->GetFloatValue();
	}
	else
	{
		VI->GetVectorValue(V2);
	}

	NewV[0] = Value[0] / V2[0];
	NewV[1] = Value[1] / V2[1];
	NewV[2] = Value[2] / V2[2];

	return new VectorVar("", NewV[0], NewV[1], NewV[2]);
}

void VectorVar::operator =(Variable* VI)
{
	VI->GetVectorValue(Value);
}

bool VectorVar::operator ==(Variable* VI)
{
	vec3_t vec;

	if (VI->GetType() == TYPE_INT || VI->GetType() == TYPE_FLOAT)
	{
		return VectorLength(Value) == VI->GetFloatValue();
	}
	else if (VI->GetType() == TYPE_VECTOR)
	{
		VI->GetVectorValue(vec);

		return (VectorCompare(Value, vec) == 1);	// VC6 gives a warning about converting int to bool
	}

	return false;
}

bool VectorVar::operator !=(Variable* VI)
{
	vec3_t vec;

	if (VI->GetType() == TYPE_INT || VI->GetType() == TYPE_FLOAT)
	{
		return VectorLength(Value) != VI->GetFloatValue();
	}
	else if (VI->GetType() == TYPE_VECTOR)
	{
		VI->GetVectorValue(vec);

		return !VectorCompare(Value, vec);
	}

	return false;
}

bool VectorVar::operator <(Variable* VI)
{
	vec3_t	vec;
	float	compare;

	if (VI->GetType() == TYPE_INT || VI->GetType() == TYPE_FLOAT)
	{
		compare = VI->GetFloatValue();
	}
	else if (VI->GetType() == TYPE_VECTOR)
	{
		VI->GetVectorValue(vec);
		compare = VectorLength(vec);
	}
	else
	{
		return false;
	}

	return VectorLength(Value) < compare;
}

bool VectorVar::operator <=(Variable* VI)
{
	vec3_t	vec;
	float	compare;

	if (VI->GetType() == TYPE_INT || VI->GetType() == TYPE_FLOAT)
	{
		compare = VI->GetFloatValue();
	}
	else if (VI->GetType() == TYPE_VECTOR)
	{
		VI->GetVectorValue(vec);
		compare = VectorLength(vec);
	}
	else
	{
		return false;
	}

	return VectorLength(Value) <= compare;
}

bool VectorVar::operator >(Variable* VI)
{
	vec3_t	vec;
	float	compare;

	if (VI->GetType() == TYPE_INT || VI->GetType() == TYPE_FLOAT)
	{
		compare = VI->GetFloatValue();
	}
	else if (VI->GetType() == TYPE_VECTOR)
	{
		VI->GetVectorValue(vec);
		compare = VectorLength(vec);
	}
	else
	{
		return false;
	}

	return VectorLength(Value) > compare;
}

bool VectorVar::operator >=(Variable* VI)
{
	vec3_t	vec;
	float	compare;

	if (VI->GetType() == TYPE_INT || VI->GetType() == TYPE_FLOAT)
	{
		compare = VI->GetFloatValue();
	}
	else if (VI->GetType() == TYPE_VECTOR)
	{
		VI->GetVectorValue(vec);
		compare = VectorLength(vec);
	}
	else
	{
		return false;
	}

	return VectorLength(Value) >= compare;
}