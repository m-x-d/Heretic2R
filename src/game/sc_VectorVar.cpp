//
// sc_VectorVar.cpp
//
// Copyright 1998 Raven Software
//

#include "sc_VectorVar.h"
#include "sc_CScript.h"
#include "sc_Utility.h"
#include "Vector.h"

VectorVar::VectorVar(const char* new_name, const float x, const float y, const float z) : Variable(new_name, TYPE_VECTOR)
{
	VectorSet(value, x, y, z);
}

VectorVar::VectorVar(vec3_t new_value) : Variable("", TYPE_VECTOR)
{
	VectorCopy(new_value, value);
}

VectorVar::VectorVar(FILE* f, CScript* script) : Variable(f, script)
{
	fread(&value, 1, sizeof(value), f);
}

void VectorVar::Write(FILE* f, CScript* script, int id)
{
	Variable::Write(f, script, RLID_VECTORVAR);
	fwrite(value, 1, sizeof(value), f);
}

void VectorVar::GetVectorValue(vec3_t& dest_value) const
{
	VectorCopy(value, dest_value);
}

void VectorVar::ReadValue(CScript* script)
{
	value[0] = script->ReadFloat();
	value[1] = script->ReadFloat();
	value[2] = script->ReadFloat();
}

void VectorVar::Debug(CScript* script)
{
	Variable::Debug(script);
	script->DebugLine("      Vector Value: [%0.f, %0.f, %0.f]\n", value[0], value[1], value[2]);
}

Variable* VectorVar::operator +(Variable* v)
{
	vec3_t other;

	if (v->GetType() == TYPE_INT || v->GetType() == TYPE_FLOAT)
	{
		const float val = v->GetFloatValue();
		VectorSet(other, val, val, val);
	}
	else
	{
		v->GetVectorValue(other);
	}

	Vec3AddAssign(value, other);

	return new VectorVar("", other[0], other[1], other[2]);
}

Variable* VectorVar::operator -(Variable* v)
{
	vec3_t other;

	if (v->GetType() == TYPE_INT || v->GetType() == TYPE_FLOAT)
	{
		const float val = v->GetFloatValue();
		VectorSet(other, val, val, val);
	}
	else
	{
		v->GetVectorValue(other);
	}

	Vec3SubtractAssign(value, other);

	return new VectorVar("", other[0], other[1], other[2]);
}

Variable* VectorVar::operator *(Variable* v)
{
	vec3_t other;

	if (v->GetType() == TYPE_INT || v->GetType() == TYPE_FLOAT)
	{
		const float val = v->GetFloatValue();
		VectorSet(other, val, val, val);
	}
	else
	{
		v->GetVectorValue(other);
	}

	VectorScaleByVector(value, other, other);

	return new VectorVar("", other[0], other[1], other[2]);
}

Variable* VectorVar::operator /(Variable* v)
{
	vec3_t other;

	if (v->GetType() == TYPE_INT || v->GetType() == TYPE_FLOAT)
	{
		const float val = v->GetFloatValue();
		VectorSet(other, val, val, val);
	}
	else
	{
		v->GetVectorValue(other);
	}

	for (int i = 0; i < 3; i++)
		other[i] = value[i] / other[i];

	return new VectorVar("", other[0], other[1], other[2]);
}

void VectorVar::operator =(Variable* v)
{
	v->GetVectorValue(value);
}

bool VectorVar::operator ==(Variable* v)
{
	if (v->GetType() == TYPE_INT || v->GetType() == TYPE_FLOAT)
		return VectorLength(value) == v->GetFloatValue();

	if (v->GetType() == TYPE_VECTOR)
	{
		vec3_t vec;
		v->GetVectorValue(vec);

		return VectorCompare(value, vec); // VC6 gives a warning about converting int to bool // VC2022 doesn't -- mxd.
	}

	return false;
}

bool VectorVar::operator !=(Variable* v)
{
	if (v->GetType() == TYPE_INT || v->GetType() == TYPE_FLOAT)
		return VectorLength(value) != v->GetFloatValue();

	if (v->GetType() == TYPE_VECTOR)
	{
		vec3_t vec;
		v->GetVectorValue(vec);

		return !VectorCompare(value, vec);
	}

	return false;
}

bool VectorVar::operator <(Variable* v)
{
	if (v->GetType() == TYPE_INT || v->GetType() == TYPE_FLOAT)
		return VectorLength(value) < v->GetFloatValue();

	if (v->GetType() == TYPE_VECTOR)
	{
		vec3_t vec;
		v->GetVectorValue(vec);

		return VectorLength(value) < VectorLength(vec);
	}

	return false;
}

bool VectorVar::operator <=(Variable* v)
{
	if (v->GetType() == TYPE_INT || v->GetType() == TYPE_FLOAT)
		return VectorLength(value) <= v->GetFloatValue();

	if (v->GetType() == TYPE_VECTOR)
	{
		vec3_t vec;
		v->GetVectorValue(vec);

		return VectorLength(value) <= VectorLength(vec);
	}

	return false;
}

bool VectorVar::operator >(Variable* v)
{
	if (v->GetType() == TYPE_INT || v->GetType() == TYPE_FLOAT)
		return VectorLength(value) > v->GetFloatValue();

	if (v->GetType() == TYPE_VECTOR)
	{
		vec3_t vec;
		v->GetVectorValue(vec);

		return VectorLength(value) > VectorLength(vec);
	}

	return false;
}

bool VectorVar::operator >=(Variable* v)
{
	if (v->GetType() == TYPE_INT || v->GetType() == TYPE_FLOAT)
		return VectorLength(value) >= v->GetFloatValue();

	if (v->GetType() == TYPE_VECTOR)
	{
		vec3_t vec;
		v->GetVectorValue(vec);

		return VectorLength(value) >= VectorLength(vec);
	}

	return false;
}