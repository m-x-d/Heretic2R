#include "ds.h"
#include "c_ai.h" //mxd

#define SCRIPT_SAVE_VERSION 2

List<Variable *>	GlobalVariables;
List<CScript *>		Scripts;

template<class T> size_t tWrite(T *ptr,FILE *FH,int n=1)
{
	return fwrite(ptr,n,sizeof(T),FH);
}

template<class T> size_t tRead(T *ptr,FILE *FH,int n=1)
{
	return fread(ptr,n,sizeof(T),FH);
}

extern "C"
{
	extern void TriggerMultipleUse(edict_t *self, edict_t *other, edict_t *activator);
	extern cvar_t 	*Cvar_Set (const char *var_name, const char *value);
}

int msg_animtype  [NUM_MESSAGES] =
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

#define MAX_CINESNDS 255 //TODO: unused?

typedef struct CinematicSound_s //TODO: unused?
{
	edict_t *ent;
	int channel;
} CinematicSound_t;

CinematicSound_t CinematicSound[MAX_CINESNDS]; //TODO: unused?

int CinematicSound_cnt;	// Count of the current # of sounds executed //TODO: unused?

//==========================================================================

typedef struct RestoreList_s
{
	int ID;
	void *(*alloc_func)(FILE *, void *);
} RestoreList_t;

void *RF_IntVar(FILE *FH, void *Data)
{
	return new IntVar(FH, (CScript *)Data);
}

void *RF_FloatVar(FILE *FH, void *Data)
{
	return new FloatVar(FH, (CScript *)Data);
}

void *RF_VectorVar(FILE *FH, void *Data)
{
	return new VectorVar(FH, (CScript *)Data);
}

void *RF_EntityVar(FILE *FH, void *Data)
{
	return new EntityVar(FH, (CScript *)Data);
}

void *RF_StringVar(FILE *FH, void *Data)
{
	return new StringVar(FH, (CScript *)Data);
}

void *RF_VariableVar(FILE *FH, void *Data)
{
	return new VariableVar(FH, (CScript *)Data);
}

void *RF_FieldVariableVar(FILE *FH, void *Data)
{
	return new FieldVariableVar(FH, (CScript *)Data);
}

void *RF_Signaler(FILE *FH, void *Data)
{
	return new Signaler(FH, (CScript *)Data);
}

void *RF_MoveDoneEvent(FILE *FH, void *Data)
{
	return new MoveDoneEvent(FH, (CScript *)Data);
}

void *RF_RotateDoneEvent(FILE *FH, void *Data)
{
	return new RotateDoneEvent(FH, (CScript *)Data);
}

void *RF_ExecuteEvent(FILE *FH, void *Data)
{
	return new ExecuteEvent(FH, (CScript *)Data);
}

void *RF_WaitEvent(FILE *FH, void *Data)
{
	return new WaitEvent(FH, (CScript *)Data);
}

void *RF_Script(FILE *FH, void *Data)
{
	return new CScript(FH);
}

void *RF_FieldDef(FILE *FH, void *Data)
{
	return new FieldDef(FH, (CScript *)Data);
}

#define RLID_INTVAR				1
#define RLID_FLOATVAR			2
#define RLID_VECTORVAR			3
#define RLID_ENTITYVAR			4
#define RLID_STRINGVAR			5
#define RLID_VARIABLEVAR		6
#define RLID_FIELDVARIABLEVAR	7
#define RLID_SIGNALER			8
#define RLID_MOVEDONEEVENT		9
#define RLID_ROTATEDONEEVENT	10
#define RLID_EXECUTEEVENT		11
#define RLID_WAITEVENT			12
#define RLID_SCRIPT				13
#define RLID_FIELDDEF			14

RestoreList_t ScriptRL[] = 
{
	{ RLID_INTVAR,				RF_IntVar },
	{ RLID_FLOATVAR,			RF_FloatVar },
	{ RLID_VECTORVAR,			RF_VectorVar },
	{ RLID_ENTITYVAR,			RF_EntityVar },
	{ RLID_STRINGVAR,			RF_StringVar },
	{ RLID_VARIABLEVAR,			RF_VariableVar },
	{ RLID_FIELDVARIABLEVAR,	RF_FieldVariableVar },
	{ RLID_SIGNALER,			RF_Signaler },
	{ RLID_MOVEDONEEVENT,		RF_MoveDoneEvent },
	{ RLID_ROTATEDONEEVENT,		RF_RotateDoneEvent },
	{ RLID_EXECUTEEVENT,		RF_ExecuteEvent },
	{ RLID_WAITEVENT,			RF_WaitEvent },
	{ RLID_SCRIPT,				RF_Script },
	{ RLID_FIELDDEF,			RF_FieldDef },

	{ 0,						NULL },
};

void *RestoreObject(FILE *FH, RestoreList_t *RestoreList, void *Data)
{
	int				ID;
	RestoreList_t	*pos;

	fread(&ID, 1, sizeof(ID), FH);

	for(pos = RestoreList; pos->alloc_func; pos++)
	{
		if (pos->ID == ID)
		{
			return pos->alloc_func(FH, Data);
		}
	}

	return NULL;
}

//==========================================================================

void ReadEnt(edict_t **to,FILE *FH)
{
	int index;
	tRead(&index,FH);
	if (index<0||index>=globals.num_edicts)
	{
		assert(index==-1); //else invalid edict number
		*to=0;
	}
	else
		*to=g_edicts+index;
}

void WriteEnt(edict_t **to,FILE *FH)
{
	int index;
	if (*to)
	{
		index=(*to)-g_edicts;
		assert(index>=0&&index<globals.num_edicts); //else invalid edict pointer
	}
	else
		index=-1;
	tWrite(&index,FH);
}

//==========================================================================

extern "C" void ProcessScripts(void)
{
	List<CScript *>::Iter	is;

	if (Scripts.Size())
	{
		for (is=Scripts.Begin();is != Scripts.End();is++)
		{
			(*is)->Think();
		}
	}
}

extern "C" void ShutdownScripts(qboolean Complete)
{
	List<CScript *>::Iter	is;
	List<Variable *>::Iter	iv;
	int						i;
	edict_t					*ent;

	while(Scripts.Size())
	{
		is=Scripts.Begin();
		delete (*is);

		Scripts.Erase(is);
	}

	for(i = 0, ent = g_edicts; i < globals.num_edicts; i++, ent++)
	{
		ent->Script = NULL;
	}

	if (Complete)
	{
		while(GlobalVariables.Size())
		{
			iv=GlobalVariables.Begin();
			delete (*iv);

			GlobalVariables.Erase(iv);
		}
	}
}

extern "C" void SaveScripts(FILE *FH, qboolean DoGlobals)
{
	int						size;
	List<CScript *>::Iter	is;
	List<Variable *>::Iter	iv;

	size = SCRIPT_SAVE_VERSION;
	fwrite(&size, 1, sizeof(size), FH);

	if (DoGlobals)
	{
		size = GlobalVariables.Size();
		fwrite(&size, 1, sizeof(size), FH);

		for (iv=GlobalVariables.Begin();iv != GlobalVariables.End();iv++)
		{
			(*iv)->Write(FH, NULL);
		}
	}
	else
	{
		size = Scripts.Size();
		fwrite(&size, 1, sizeof(size), FH);

		for (is=Scripts.Begin();is != Scripts.End();is++)
		{
			(*is)->Write(FH);
		}
	}
}

extern "C" void LoadScripts(FILE *FH, qboolean DoGlobals)
{
	int		size, i;
	edict_t	*ent;

	fread(&size, 1, sizeof(size), FH);
	if (size != SCRIPT_SAVE_VERSION)
	{
		gi.error("LoadScripts(): Expecting version %d, found version %d", SCRIPT_SAVE_VERSION, size);
	}

	if (DoGlobals)
	{
		ShutdownScripts(true);

		fread(&size, 1, sizeof(size), FH);

		for(i=0;i<size;i++)
		{	
			GlobalVariables.PushBack((Variable *)RestoreObject(FH, ScriptRL, NULL));
		}
	}
	else
	{
		ShutdownScripts(false);

		for(i = 0, ent = g_edicts; i < globals.num_edicts; i++, ent++)
		{
			ent->Script = NULL;
		}

		fread(&size, 1, sizeof(size), FH);

		for(i=0;i<size;i++)
		{	
			Scripts.PushBack((CScript *)RestoreObject(FH, ScriptRL, NULL));
		}
	}
}


void script_use(edict_t *ent, edict_t *other, edict_t *activator)
{
	ent->Script->AddEvent(new ExecuteEvent(level.time, other, activator) );
}

/*QUAKED script_runner (.5 .5 .5) (-8 -8 -8) (8 8 8)
set Script to the name of the script to run when triggered
use parm1 through parm16 to send parameters to the script
*/
extern "C" void SP_script_runner (edict_t *ent)
{
	char	temp[MAX_PATH];
	int		i;

	sprintf(temp,"ds/%s.os",st.script);
	ent->Script = new CScript(temp, ent);
	Scripts.PushBack(ent->Script);

	for(i=0;i<NUM_PARMS;i++)
	{
		if (st.parms[i])
		{
			ent->Script->SetParameter(st.parms[i]);
		}
		else
		{
			break;
		}
	}

	ent->movetype = PHYSICSTYPE_NONE;
	ent->solid = SOLID_NOT;
	ent->svflags |= SVF_NOCLIENT;
	ent->use = script_use;

//	gi.setmodel (ent, ent->model);
//	gi.linkentity (ent);
}

/*QUAKE script_parms (.5 .5 .5) ?
target the script_runner object
use parm1 through parm16 to send parameters to the script
*/
extern "C" void SP_parms (edict_t *ent)
{
}

//==========================================================================

Variable *FindGlobal(char *Name)
{
	List<Variable *>::Iter	iv;

	if (GlobalVariables.Size())
	{
		for (iv=GlobalVariables.Begin();iv != GlobalVariables.End();iv++)
		{
			if (strcmp(Name, (*iv)->GetName()) == 0)
			{
				return *iv;
			}
		}
	}

	return NULL;
}

bool NewGlobal(Variable *Which)
{
	Variable *Check;

	Check = FindGlobal(Which->GetName());
	if (Check)
	{	// already exists
		return false;
	}

	GlobalVariables.PushBack(Which);

	return true;
}

//==========================================================================

Variable::Variable(char *NewName, VariableType NewType)
{
	strcpy(Name,NewName);
	Type = NewType;
}

Variable::Variable(FILE *FH, CScript *Script)
{
	int index;

	fread(Name, 1, sizeof(Name), FH);
	fread(&Type, 1, sizeof(Type), FH);
	fread(&index, 1, sizeof(index), FH);

	if (Script && index != -1)
	{
		Script->SetVarIndex(index, this);
	}
}

void Variable::Write(FILE *FH, CScript *Script, int ID)
{
	int	index = -1;

	fwrite(&ID, 1, sizeof(ID), FH);
	fwrite(Name, 1, sizeof(Name), FH);
	fwrite(&Type, 1, sizeof(Type), FH);

	if (Script)
	{
		index = Script->LookupVarIndex(this);
	}
	fwrite(&index, 1, sizeof(index), FH);
}

void Variable::Debug(CScript *Script)
{
	Script->DebugLine("   Name: %s\n",Name);
}

//==========================================================================

IntVar::IntVar(char *Name, int InitValue)
:Variable(Name, TYPE_INT)
{
	Value = InitValue;
}

IntVar::IntVar(FILE *FH, CScript *Script)
:Variable(FH, Script)
{
	fread(&Value, 1, sizeof(Value), FH);
}

void IntVar::Write(FILE *FH, CScript *Script, int ID)
{
	Variable::Write(FH, Script, RLID_INTVAR);

	fwrite(&Value, 1, sizeof(Value), FH);
}

void IntVar::ReadValue(CScript *Script)
{
	Value = Script->ReadInt();
}

void IntVar::Debug(CScript *Script)
{
	Variable::Debug(Script);

	Script->DebugLine("      Integer Value: %d\n",Value);
}

void IntVar::Signal(edict_t *Which)
{
	Value++;
}

void IntVar::ClearSignal(void)
{
	Value = 0;
}

Variable *IntVar::operator +(Variable *VI)
{
	return new IntVar("",Value + VI->GetIntValue());
}

Variable *IntVar::operator -(Variable *VI)
{
	return new IntVar("",Value - VI->GetIntValue());
}

Variable *IntVar::operator *(Variable *VI)
{
	return new IntVar("",Value * VI->GetIntValue());
}

Variable *IntVar::operator /(Variable *VI)
{
	return new IntVar("",Value / VI->GetIntValue());
}

void IntVar::operator =(Variable *VI)
{
	Value = VI->GetIntValue();
}

//==========================================================================

FloatVar::FloatVar(char *Name, float InitValue)
:Variable(Name, TYPE_FLOAT)
{
	Value = InitValue;
}

FloatVar::FloatVar(FILE *FH, CScript *Script)
:Variable(FH, Script)
{
	fread(&Value, 1, sizeof(Value), FH);
}

void FloatVar::Write(FILE *FH, CScript *Script, int ID)
{
	Variable::Write(FH, Script, RLID_FLOATVAR);

	fwrite(&Value, 1, sizeof(Value), FH);
}

void FloatVar::ReadValue(CScript *Script)
{
	Value = Script->ReadFloat();
}

void FloatVar::Debug(CScript *Script)
{
	Variable::Debug(Script);

	Script->DebugLine("      Float Value: %0.f\n",Value);
}

Variable *FloatVar::operator +(Variable *VI)
{
	return new FloatVar("",Value + VI->GetFloatValue());
}

Variable *FloatVar::operator -(Variable *VI)
{
	return new FloatVar("",Value - VI->GetFloatValue());
}

Variable *FloatVar::operator *(Variable *VI)
{
	return new FloatVar("",Value * VI->GetFloatValue());
}

Variable *FloatVar::operator /(Variable *VI)
{
	return new FloatVar("",Value / VI->GetFloatValue());
}

void FloatVar::operator =(Variable *VI)
{
	Value = VI->GetFloatValue();
}

//==========================================================================

VectorVar::VectorVar(char *Name, float InitValueX, float InitValueY, float InitValueZ)
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

VectorVar::VectorVar(FILE *FH, CScript *Script)
:Variable(FH, Script)
{
	fread(&Value, 1, sizeof(Value), FH);
}

void VectorVar::Write(FILE *FH, CScript *Script, int ID)
{
	Variable::Write(FH, Script, RLID_VECTORVAR);

	fwrite(Value, 1, sizeof(Value), FH);
}

void VectorVar::GetVectorValue(vec3_t &VecValue) 
{ 
	VecValue[0] = Value[0];
	VecValue[1] = Value[1];
	VecValue[2] = Value[2]; 
}

void VectorVar::ReadValue(CScript *Script)
{
	Value[0] = Script->ReadFloat();
	Value[1] = Script->ReadFloat();
	Value[2] = Script->ReadFloat();
}

void VectorVar::Debug(CScript *Script)
{
	Variable::Debug(Script);

	Script->DebugLine("      Vector Value: [%0.f, %0.f, %0.f]\n",Value[0],Value[1],Value[2]);
}

Variable *VectorVar::operator +(Variable *VI)
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

Variable *VectorVar::operator -(Variable *VI)
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

Variable *VectorVar::operator *(Variable *VI)
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

Variable *VectorVar::operator /(Variable *VI)
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

void VectorVar::operator =(Variable *VI)
{
	VI->GetVectorValue(Value);
}

bool VectorVar::operator ==(Variable *VI)
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

bool VectorVar::operator !=(Variable *VI)
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

bool VectorVar::operator <(Variable *VI)
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

bool VectorVar::operator <=(Variable *VI)
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

bool VectorVar::operator >(Variable *VI)
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

bool VectorVar::operator >=(Variable *VI)
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

//==========================================================================

EntityVar::EntityVar(char *Name, int InitValue)
:Variable(Name, TYPE_ENTITY)
{
	if (InitValue == -1)
	{
		Value = NULL;
	}
	else
	{
		Value = &g_edicts[InitValue];
	}
}

EntityVar::EntityVar(edict_t *Which)
:Variable("", TYPE_ENTITY)
{
	Value = Which;
}

EntityVar::EntityVar(FILE *FH, CScript *Script)
:Variable(FH, Script)
{
	int index;

	fread(&index, 1, sizeof(index), FH);

	if (index == -1)
	{
		Value = NULL;
	}
	else
	{
		Value = &g_edicts[index];
	}
}

void EntityVar::Write(FILE *FH, CScript *Script, int ID)
{
	int index;

	Variable::Write(FH, Script, RLID_ENTITYVAR);

	index = GetIntValue();
	fwrite(&index, 1, sizeof(index), FH);
}

void EntityVar::ReadValue(CScript *Script)
{
	int Index;

	Index = Script->ReadInt();
	if (Index == -1)
	{
		Value = NULL;
	}
	else
	{
		Value = &g_edicts[Index];
	}
}

void EntityVar::Debug(CScript *Script)
{
	Variable::Debug(Script);

	Script->DebugLine("      Entity Value: %d\n",GetIntValue());
}

int EntityVar::GetIntValue(void)
{
	if (Value)
	{
		return Value - g_edicts;
	}

	return -1;
}

void EntityVar::operator =(Variable *VI)
{
	Value = VI->GetEdictValue();
}

bool EntityVar::operator ==(Variable *VI)
{ 
	if (VI->GetType() == TYPE_INT)
	{
		return GetIntValue() == VI->GetIntValue();
	}
	else if (VI->GetType() == TYPE_ENTITY)
	{
		return GetEdictValue() == VI->GetEdictValue();
	}
	
	return false;
}

bool EntityVar::operator !=(Variable *VI)
{ 
	if (VI->GetType() == TYPE_INT)
	{
		return GetIntValue() != VI->GetIntValue();
	}
	else if (VI->GetType() == TYPE_ENTITY)
	{
		return GetEdictValue() != VI->GetEdictValue();
	}
	
	return false;
}

//==========================================================================

StringVar::StringVar(char *Name, char *InitValue)
:Variable(Name, TYPE_STRING)
{
	strcpy(Value, InitValue);
}

StringVar::StringVar(FILE *FH, CScript *Script)
:Variable(FH, Script)
{
	fread(&Value, 1, sizeof(Value), FH);
}

void StringVar::Write(FILE *FH, CScript *Script, int ID)
{
	Variable::Write(FH, Script, RLID_STRINGVAR);

	fwrite(&Value, 1, sizeof(Value), FH);
}

void StringVar::ReadValue(CScript *Script)
{
	strcpy(Value, Script->ReadString());
}

//==========================================================================

VariableVar::VariableVar(char *Name)
:Variable(Name, TYPE_UNKNOWN)
{
	Value = NULL;
}

VariableVar::VariableVar(FILE *FH, CScript *Script)
:Variable(FH, Script)
{
	int index;

	fread(&index, 1, sizeof(index), FH);
	Value = Script->LookupVar(index);
}

void VariableVar::Write(FILE *FH, CScript *Script, int ID)
{
	int index;

	Variable::Write(FH, Script, RLID_VARIABLEVAR);

	index = Script->LookupVarIndex(Value);
	fwrite(&index, 1, sizeof(index), FH);
}

void VariableVar::ReadValue(CScript *Script)
{
	int Index;

	Index = Script->ReadInt();

	Value = Script->LookupVar(Index);

	if (Value)
	{
		Type = Value->GetType();
	}
}

void VariableVar::Debug(CScript *Script)
{
	Value->Debug(Script);
}

//==========================================================================

FieldVariableVar::FieldVariableVar(char *Name)
:Variable(Name, TYPE_UNKNOWN)
{
	Value = NULL;
	Field = NULL;
}

FieldVariableVar::FieldVariableVar(FILE *FH, CScript *Script)
:Variable(FH, Script)
{
	int index;

	fread(&index, 1, sizeof(index), FH);
	Value = Script->LookupVar(index);

	fread(&index, 1, sizeof(index), FH);
	Field = Script->LookupField(index);
}

void FieldVariableVar::Write(FILE *FH, CScript *Script, int ID)
{
	int index;

	Variable::Write(FH, Script, RLID_FIELDVARIABLEVAR);

	index = Script->LookupVarIndex(Value);
	fwrite(&index, 1, sizeof(index), FH);
	
	index = Script->LookupFieldIndex(Field);
	fwrite(&index, 1, sizeof(index), FH);
}

void FieldVariableVar::ReadValue(CScript *Script)
{
	int Index;

	Index = Script->ReadInt();
	Value = Script->LookupVar(Index);

	Index = Script->ReadInt();
	Field = Script->LookupField(Index);
}

void FieldVariableVar::Debug(CScript *Script)
{
	Value->Debug(Script);
}

int FieldVariableVar::GetIntValue(void) 
{ 
	return Field->GetIntValue(Value); 
}

float FieldVariableVar::GetFloatValue(void) 
{ 
	return Field->GetFloatValue(Value); 
}

void FieldVariableVar::GetVectorValue(vec3_t &VecValue) 
{ 
	Field->GetVectorValue(Value, VecValue); 
}

edict_t *FieldVariableVar::GetEdictValue(void) 
{ 
	return Field->GetEdictValue(Value); 
}

char *FieldVariableVar::GetStringValue(void) 
{ 
	return Field->GetStringValue(Value); 
}

Variable *FieldVariableVar::operator +(Variable *VI)
{ 
	Variable *Result, *Val;

	Val = Field->GetValue(Value);

	Result = (*Val) + VI;

	delete Val;

	return Result;
}

Variable *FieldVariableVar::operator -(Variable *VI) 
{ 
	Variable *Result, *Val;

	Val = Field->GetValue(Value);

	Result =  (*Val) - VI;

	delete Val;

	return Result;
}

Variable *FieldVariableVar::operator *(Variable *VI) 
{ 
	Variable *Result, *Val;

	Val = Field->GetValue(Value);

	Result = (*Val) * VI;

	delete Val;

	return Result;
}

Variable *FieldVariableVar::operator /(Variable *VI) 
{ 
	Variable *Result, *Val;

	Val = Field->GetValue(Value);

	Result = (*Val) / VI;

	delete Val;

	return Result;
}

void FieldVariableVar::operator =(Variable *VI) 
{ 
	Field->SetValue(Value, VI);
}

bool FieldVariableVar::operator ==(Variable *VI) 
{ 
	return (*Value) == VI; 
}

bool FieldVariableVar::operator !=(Variable *VI) 
{ 
	return (*Value) != VI; 
}

bool FieldVariableVar::operator <(Variable *VI) 
{ 
	return (*Value) < VI; 
}

bool FieldVariableVar::operator <=(Variable *VI) 
{ 
	return (*Value) <= VI; 
}

bool FieldVariableVar::operator >(Variable *VI) 
{ 
	return (*Value) > VI; 
}

bool FieldVariableVar::operator >=(Variable *VI) 
{ 
	return (*Value) >= VI; 
}

//==========================================================================

Signaler::Signaler(edict_t *NewEdict, Variable *NewVar, SignalT NewSignalType)
{
	Edict = NewEdict;
	Var = NewVar;
	SignalType = NewSignalType;
}

Signaler::Signaler(FILE *FH, CScript *Script)
{
	ReadEnt(&Edict,FH);
	tRead(&SignalType, FH);

	Var = (Variable *)RestoreObject(FH, ScriptRL, Script);
}

Signaler::~Signaler(void)
{
	if (Var)
	{
		delete Var;
	}
}

void Signaler::Write(FILE *FH, CScript *Script)
{
	int index;

	index = RLID_SIGNALER;
	fwrite(&index, 1, sizeof(index), FH);

	WriteEnt(&Edict,FH);
	tWrite(&SignalType, FH);

	Var->Write(FH, Script);
}

bool Signaler::Test(edict_t *Which, SignalT WhichType)
{
	if (WhichType != SignalType)
	{
		return false;
	}

	if (Edict != Which)
	{
		return false;
	}
	
	Var->Signal(Which);

	return true;
}

bool Signaler::operator ==(Signaler *SI)
{ 
	if (Var == SI->GetVar())
	{
		return true;
	}

	return false;
}

void script_signaler(edict_t *which, SignalT SignalType)
{
	List<CScript *>::Iter	is;

	if (Scripts.Size())
	{
		for (is=Scripts.Begin();is != Scripts.End();is++)
		{
			(*is)->CheckSignalers(which, SignalType);
		}
	}
}

void move_signaler(edict_t *which)
{
	script_signaler(which, SIGNAL_MOVE);
}

void rotate_signaler(edict_t *which)
{
	script_signaler(which, SIGNAL_ROTATE);
}

void animate_signaler(edict_t *which)
{
	script_signaler(which, SIGNAL_ANIMATE);
}

//==========================================================================

// Fields are just yucky now - once H2 finals, I'm going to change them completely

#define SPEC_X				-1
#define SPEC_Y				-2
#define SPEC_Z				-3
#define SPEC_DELTA_ANGLES	-4
#define SPEC_P_ORIGIN		-5

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

FieldDef::FieldDef(CScript *Script)
{
	field_t *Field;
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
		Com_Printf("Unknown field '%s'\n",Name);
#endif //_DEVEL
	}
}

FieldDef::FieldDef(FILE *FH, CScript *Script)
{
	int		index;
	bool	Found;
	field_t *Field;

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
		Com_Printf("Unknown field '%s'\n",Name);
#endif //_DEVEL
	}
}

void FieldDef::Write(FILE *FH, CScript *Script)
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

byte *FieldDef::GetOffset(Variable *Var)
{
	edict_t	*ent;
	byte	*b, *Dest;

	Dest = NULL;

	switch(Offset)
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
				Dest = (byte *)&ent->client->ps.pmove.delta_angles;
			}
			break;

		case SPEC_P_ORIGIN:
			ent = Var->GetEdictValue();
			if (ent && ent->client)
			{
				Dest = (byte *)&ent->client->playerinfo.origin;
			}
			break;

		default:
			ent = Var->GetEdictValue();
			if (ent)
			{
				b = (byte *)ent;
				Dest = b+Offset;
			}
			break;
	}

	return Dest;
}

Variable *FieldDef::GetValue(Variable *Var)
{
	vec3_t vec;

	switch(FieldType)
	{
		case F_INT:
			return new IntVar("", GetIntValue(Var) );
			break;

		case F_FLOAT:
			return new FloatVar("", GetFloatValue(Var) );
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

int FieldDef::GetIntValue(Variable *Var)
{
	byte	*Dest;
	vec3_t	data;

	Dest = GetOffset(Var);

 	if (FieldType != F_INT || !Dest) 
	{
		switch(Offset)
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

	return *(int *)(Dest);
}

float FieldDef::GetFloatValue(Variable *Var)
{
	byte	*Dest;
	vec3_t	data;

	Dest = GetOffset(Var);

	if (FieldType != F_FLOAT || !Dest)
	{
		switch(Offset)
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

	return *(float *)(Dest);
}

void FieldDef::GetVectorValue(Variable *Var, vec3_t &VecValue)
{
	byte	*Dest;

	Dest = GetOffset(Var);

	if (FieldType != F_VECTOR || !Dest)
	{
		VectorCopy(vec3_origin, VecValue);
		return;
	}

	VectorCopy(*(vec3_t *)(Dest), VecValue);
}

edict_t *FieldDef::GetEdictValue(Variable *Var)
{
	byte	*Dest;

	Dest = GetOffset(Var);

	if (FieldType != F_EDICT || !Dest)
	{
		return NULL;
	}

	return *(edict_t **)(Dest);
}

char *FieldDef::GetStringValue(Variable *Var)
{
	return "";
}

void FieldDef::SetValue(Variable *Var, Variable *Value)
{
	byte		*Dest;
	vec3_t		data;
	VectorVar	*new_var;

	Dest = GetOffset(Var);
	if (Dest == NULL)
	{
		switch(Offset)
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
	
	switch(FieldType)
	{
		case F_INT:
			*(int *)(Dest) = Value->GetIntValue();
			break;
		case F_FLOAT:
			*(float *)(Dest) = Value->GetFloatValue();
			break;
		case F_EDICT:
			*(edict_t **)(Dest) = Value->GetEdictValue();
			break;
		case F_VECTOR:
			Value->GetVectorValue(*(vec3_t *)(Dest));
			break;
	}
}

//==========================================================================

Event::Event(float NewTime, EventT NewType)
{
	Time = floor((NewTime + 0.05) * 10) / 10;		// avoids stupid math rounding errors
	Type = NewType;
}

Event::Event(FILE *FH, CScript *Script)
{
	tRead(&Time,FH);
	tRead(&Type,FH);
	tRead(&Priority,FH);
}

void Event::Write(FILE *FH, CScript *Script, int ID)
{
	fwrite(&ID, 1, sizeof(ID), FH);
	tWrite(&Time,FH);
	tWrite(&Type,FH);
	tWrite(&Priority,FH);
}

bool Event::Process(CScript *Script)
{
	return FALSE;
}

//==========================================================================

MoveDoneEvent::MoveDoneEvent(float NewTime, edict_t *NewEnt)
:Event(NewTime, EVENT_MOVE_DONE)
{
	Ent = NewEnt;

	Priority = 10;
}

MoveDoneEvent::MoveDoneEvent(FILE *FH, CScript *Script)
:Event(FH, Script)
{
	ReadEnt(&Ent,FH);
}

void MoveDoneEvent::Write(FILE *FH, CScript *Script, int ID)
{
	Event::Write(FH, Script, RLID_MOVEDONEEVENT);
	WriteEnt(&Ent,FH);
}

bool MoveDoneEvent::Process(CScript *Script)
{
	if (level.time < Time)
	{
		return FALSE;
	}

	Script->Move_Done(Ent);
	move_signaler(Ent);

	return TRUE;
}

//==========================================================================

RotateDoneEvent::RotateDoneEvent(float NewTime, edict_t *NewEnt)
:Event(NewTime, EVENT_ROTATE_DONE)
{
	Ent = NewEnt;

	Priority = 10;
}

RotateDoneEvent::RotateDoneEvent(FILE *FH, CScript *Script)
:Event(FH, Script)
{
	ReadEnt(&Ent,FH);
}

void RotateDoneEvent::Write(FILE *FH, CScript *Script, int ID)
{
	Event::Write(FH, Script, RLID_ROTATEDONEEVENT);
	WriteEnt(&Ent,FH);
}

bool RotateDoneEvent::Process(CScript *Script)
{
	if (level.time < Time)
	{
		return FALSE;
	}

	Script->Rotate_Done(Ent);
	rotate_signaler(Ent);

	return TRUE;
}

//==========================================================================

ExecuteEvent::ExecuteEvent(float NewTime, edict_t *NewOther, edict_t *NewActivator)
:Event(NewTime, EVENT_SCRIPT_EXECUTE)
{
	Other = NewOther;
	Activator = NewActivator;

	Priority = 0;
}

ExecuteEvent::ExecuteEvent(FILE *FH, CScript *Script)
:Event(FH, Script)
{
	ReadEnt(&Other,FH);
	ReadEnt(&Activator,FH);
}

void ExecuteEvent::Write(FILE *FH, CScript *Script, int ID)
{
	Event::Write(FH, Script, RLID_EXECUTEEVENT);
	WriteEnt(&Other,FH);
	WriteEnt(&Activator,FH);
}

bool ExecuteEvent::Process(CScript *Script)
{
	if (level.time < Time)
	{
		return FALSE;
	}

	if (Script->CheckWait())
	{
		Script->Execute(Other,Activator);
	}

	return TRUE;
}

//==========================================================================

WaitEvent::WaitEvent(float NewTime)
:Event(NewTime, EVENT_SCRIPT_WAIT)
{
	Priority = 0;
}

WaitEvent::WaitEvent(FILE *FH, CScript *Script)
:Event(FH, Script)
{
}

void WaitEvent::Write(FILE *FH, CScript *Script, int ID)
{
	Event::Write(FH, Script, RLID_WAITEVENT);
}

bool WaitEvent::Process(CScript *Script)
{
	if (level.time < Time)
	{
		return FALSE;
	}

	Script->ClearTimeWait();

	if (Script->CheckWait())
	{
		Script->Execute(NULL,NULL);
	}

	return TRUE;
}

//==========================================================================

CScript::CScript(char *ScriptName, edict_t *new_owner)
{
	Clear(true);

	owner = new_owner;
	strcpy(Name, ScriptName);

	LoadFile();
}

CScript::CScript(FILE *FH)
{
	int						index;
	int						size;
	int						i;
	char					name[VAR_LENGTH];

	Clear(true);

	fread(Name, 1, sizeof(Name), FH);
	LoadFile();

	fread(&ScriptCondition, 1, sizeof(ScriptCondition), FH);
	fread(&ConditionInfo, 1, sizeof(ConditionInfo), FH);
	fread(&Length, 1, sizeof(Length), FH);
	fread(&Position, 1, sizeof(Position), FH);
	fread(&DebugFlags, 1, sizeof(DebugFlags), FH);

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
	for(i=0;i<size;i++)
	{	// fields - they'll put themselves in
		RestoreObject(FH, ScriptRL, this);
	}

	fread(&size, 1, sizeof(size), FH);
	for (i=0;i<size;i++)
	{
		fread(&index, 1, sizeof(index), FH);
		fread(name, 1, VAR_LENGTH, FH);

		VarIndex[index] = FindGlobal(name);
	}

	fread(&size, 1, sizeof(size), FH);
	for(i=0;i<size;i++)
	{
		LocalVariables.PushBack((Variable *)RestoreObject(FH, ScriptRL, this));
	}

	fread(&size, 1, sizeof(size), FH);
	for(i=0;i<size;i++)
	{
		ParameterVariables.PushBack((Variable *)RestoreObject(FH, ScriptRL, this));
	}

	fread(&size, 1, sizeof(size), FH);
	for(i=0;i<size;i++)
	{
		Stack.PushBack((Variable *)RestoreObject(FH, ScriptRL, this));
	}

	fread(&size, 1, sizeof(size), FH);
	for(i=0;i<size;i++)
	{
		Waiting.PushBack((Variable *)RestoreObject(FH, ScriptRL, this));
	}

	fread(&size, 1, sizeof(size), FH);
	for(i=0;i<size;i++)
	{
		Signalers.PushBack((Signaler *)RestoreObject(FH, ScriptRL, this));
	}

	fread(&size, 1, sizeof(size), FH);
	for(i=0;i<size;i++)
	{
		ParameterValues.PushBack((StringVar *)RestoreObject(FH, ScriptRL, this));
	}

	fread(&size, 1, sizeof(size), FH);
	for(i=0;i<size;i++)
	{
		Events.PushBack((Event *)RestoreObject(FH, ScriptRL, this));
	}
}

CScript::~CScript(void)
{
	Free(true);
}

void CScript::LoadFile(void)
{
	int Version;

	Length = gi.FS_LoadFile(Name, (void **)&Data);
	if (Length == -1)
	{
		Com_Printf("***********************************************\n");
		Com_Printf("Could not open script %s\n",Name);
		Com_Printf("***********************************************\n");
	}
	else
	{
		Version = ReadInt();

		if (Version != SCRIPT_VERSION)
		{
			Com_Printf("***********************************************\n");
			Com_Printf("Bad script version for %s: found %d, expecting %d\n",Name,Version,SCRIPT_VERSION);
			Com_Printf("***********************************************\n");
		}
		else
		{
			ScriptCondition = COND_READY;
		}
	}
}

void CScript::Free(bool DoData)
{
	int						i;
	List<Variable *>::Iter	iv;
	List<Signaler *>::Iter	is;
	List<StringVar *>::Iter	isv;
	List<Event *>::Iter		iev;

	if (Data && DoData)
	{
		gi.FS_FreeFile(Data);
		Data = NULL;
	}

	while(LocalVariables.Size())
	{
		iv=LocalVariables.Begin();
		delete (*iv);

		LocalVariables.Erase(iv);
	}

	while(ParameterVariables.Size())
	{
		iv=ParameterVariables.Begin();
		delete (*iv);

		ParameterVariables.Erase(iv);
	}

	while(Stack.Size())
	{
		iv=Stack.Begin();
		delete (*iv);

		Stack.Erase(iv);
	}

	while(Waiting.Size())
	{
		iv=Waiting.Begin();
		delete (*iv);

		Waiting.Erase(iv);
	}

	while(Signalers.Size())
	{
		is=Signalers.Begin();
		delete (*is);

		Signalers.Erase(is);
	}

	while(ParameterValues.Size())
	{
		isv=ParameterValues.Begin();
		delete (*isv);

		ParameterValues.Erase(isv);
	}

	while(Events.Size())
	{
		iev=Events.Begin();
		delete (*iev);

		Events.Erase(iev);
	}

	for(i=0;i<MAX_INDEX;i++)
	{
		if (Fields[i])
		{
			delete Fields[i];
		}
	}

	Clear(DoData);
}

void CScript::Clear(bool DoData)
{
	if (DoData)
	{
		Data = NULL;
	}

	owner = other = activator = NULL;

	memset(Fields, 0, sizeof(Fields));
	memset(VarIndex, 0, sizeof(VarIndex));

	DebugFlags = 0;
	memset(Name, 0, sizeof(Name));

	ScriptCondition = COND_COMPLETED;
	ConditionInfo = 0;
	Data = NULL;
	Position = 0;
	Length = 0;
}

void CScript::Write(FILE *FH)
{
	int						index;
	int						size;
	List<Variable *>::Iter	iv;
	List<Signaler *>::Iter	is;
	List<StringVar *>::Iter	isv;
	List<Event *>::Iter		iev;
	int						i;

	index = RLID_SCRIPT;
	fwrite(&index, 1, sizeof(index), FH);

	fwrite(Name, 1, sizeof(Name), FH);
	fwrite(&ScriptCondition, 1, sizeof(ScriptCondition), FH);
	fwrite(&ConditionInfo, 1, sizeof(ConditionInfo), FH);
	fwrite(&Length, 1, sizeof(Length), FH);
	fwrite(&Position, 1, sizeof(Position), FH);
	fwrite(&DebugFlags, 1, sizeof(DebugFlags), FH);

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
	for(i=0,size=0;i<MAX_INDEX;i++)
	{
		if (Fields[i])
		{
			size++;
		}
	}
	fwrite(&size, 1, sizeof(size), FH);
	for(i=0;i<MAX_INDEX;i++)
	{
		if (Fields[i])
		{
			Fields[i]->Write(FH, this);
		}
	}

	size = 0;
	for (iv=GlobalVariables.Begin();iv != GlobalVariables.End();iv++)
	{
		if (LookupVarIndex(*iv) != -1)
		{
			size++;
		}
	}
	fwrite(&size, 1, sizeof(size), FH);
	for (iv=GlobalVariables.Begin();iv != GlobalVariables.End();iv++)
	{
		index = LookupVarIndex(*iv);
		if (index != -1)
		{
			fwrite(&index, 1, sizeof(index), FH);
			fwrite((*iv)->GetName(), 1, VAR_LENGTH, FH);
		}
	}

	size = LocalVariables.Size();
	fwrite(&size, 1, sizeof(size), FH);
	for (iv=LocalVariables.Begin();iv != LocalVariables.End();iv++)
	{
		(*iv)->Write(FH, this);
	}

	size = ParameterVariables.Size();
	fwrite(&size, 1, sizeof(size), FH);
	for (iv=ParameterVariables.Begin();iv != ParameterVariables.End();iv++)
	{
		(*iv)->Write(FH, this);
	}

	size = Stack.Size();
	fwrite(&size, 1, sizeof(size), FH);
	for (iv=Stack.Begin();iv != Stack.End();iv++)
	{
		(*iv)->Write(FH, this);
	}

	size = Waiting.Size();
	fwrite(&size, 1, sizeof(size), FH);
	for (iv=Waiting.Begin();iv != Waiting.End();iv++)
	{
		(*iv)->Write(FH, this);
	}

	size = Signalers.Size();
	fwrite(&size, 1, sizeof(size), FH);
	for (is=Signalers.Begin();is != Signalers.End();is++)
	{
		(*is)->Write(FH, this);
	}


	size = ParameterValues.Size();
	fwrite(&size, 1, sizeof(size), FH);
	for (isv=ParameterValues.Begin();isv != ParameterValues.End();isv++)
	{
		(*isv)->Write(FH, this);
	}

	size = Events.Size();
	fwrite(&size, 1, sizeof(size), FH);
	for (iev=Events.Begin();iev != Events.End();iev++)
	{
		(*iev)->Write(FH, this);
	}
}

int CScript::LookupVarIndex(Variable *Var) 
{
	int i;

	for(i=0;i<MAX_INDEX;i++)
	{
		if (VarIndex[i] == Var)
		{
			return i;
		}
	}

	return -1;
}

int	CScript::LookupFieldIndex(FieldDef *Field)
{
	int i;

	for(i=0;i<MAX_INDEX;i++)
	{
		if (Fields[i] == Field)
		{
			return i;
		}
	}

	return -1;
}

void CScript::SetParameter(char *Value)
{
	ParameterValues.PushBack(new StringVar("parm",Value));
}

unsigned char CScript::ReadByte(void)
{
	return Data[Position++];
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

char *CScript::ReadString(void)
{
	char *Pos;

	Pos = (char *)&Data[Position];

	while(ReadByte())
	{
	}

	return Pos;
}

Variable *CScript::ReadDeclaration(int &Index)
{
	int			Type;
	char		*Name;
	Variable	*RetVal;

	Name = ReadString();
	Type = ReadByte();
	Index = ReadInt();

	RetVal = NULL;
	switch(Type)
	{
		case TYPE_INT:
			RetVal =  new IntVar(Name);
			break;
		case TYPE_FLOAT:
			RetVal =  new FloatVar(Name);
			break;
		case TYPE_VECTOR:
			RetVal =  new VectorVar(Name);
			break;
		case TYPE_ENTITY:
			RetVal =  new EntityVar(Name);
			break;
		case TYPE_STRING:
			RetVal =  new StringVar(Name);
			break;
		case TYPE_UNKNOWN:
			break;
	}

	if (Index >= MAX_INDEX)
	{
		Error("Index out of range: %d > %d",Index,MAX_INDEX);
	}

	VarIndex[Index] = RetVal;

	return RetVal;
}

void CScript::PushStack(Variable *VI)
{
	if (!VI)
	{
		Error("Illegal push");
	}

	Stack.PushBack(VI);
}

Variable *CScript::PopStack(void)
{
	Variable *Value;
	List<Variable *>::Iter	iv;

	if (Stack.Size())
	{
		iv = --Stack.End();
		Value = *iv;
		Stack.PopBack();

		return Value;
	}

	return NULL;
}

void CScript::HandleGlobal(bool Assignment)
{
	Variable	*Var;
	int			Index;

	Var = ReadDeclaration(Index);

	if (Assignment)
	{
		Var->ReadValue(this);
	}

	if (!NewGlobal(Var))
	{
		VarIndex[Index] = FindGlobal(Var->GetName());

		delete Var;
	}
}

void CScript::HandleLocal(bool Assignment)
{
	Variable	*Var;
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
	Variable	*Var;
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
	FieldDef	*NewField;

	NewField = new FieldDef(this);

	Index = ReadInt();
	if (Index < 0 || Index >= MAX_INDEX)
	{
		Error("Index for field out of range: %d > %d\n",Index,MAX_INDEX);
	}

	Fields[Index] = NewField;
}

void CScript::HandleGoto(void)
{
	Position = ReadInt();
}

Variable *CScript::HandleSpawn(void)
{
	int			Count;
	edict_t		*ent;
	Variable	*Name;
	Variable	*Value;
	const field_t	*f;
	const char	*NameValue;
	byte		*b;

	ent = G_Spawn();

	for(Count = ReadByte(); Count; Count--)
	{
		Name = PopStack();
		Value = PopStack();
		if (!Name || !Value)
		{
			Error("Invalid stack for HandleSpawn()");
		}

		NameValue = Name->GetStringValue();

		for (f=fields ; f->name ; f++)
		{
			if (!Q_stricmp(f->name, (char *)NameValue) )
			{
				if (f->flags & FFL_SPAWNTEMP)
				{
					b = (byte *)&st;
				}
				else
				{
					b = (byte *)ent;
				}

				switch (f->type)
				{
					case F_LSTRING:
						*(char **)(b+f->ofs) = ED_NewString (Value->GetStringValue());
						break;
					case F_VECTOR:
						Value->GetVectorValue(*(vec3_t *)(b+f->ofs));
						break;
					case F_INT:
						*(int *)(b+f->ofs) = Value->GetIntValue();
						break;
					case F_FLOAT:
						*(float *)(b+f->ofs) = Value->GetFloatValue();
						break;
					case F_ANGLEHACK:
						((float *)(b+f->ofs))[0] = 0;
						((float *)(b+f->ofs))[1] = Value->GetFloatValue();
						((float *)(b+f->ofs))[2] = 0;
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

Variable *CScript::HandleBuiltinFunction(void)
{
	int			Index;
	edict_t		*Search;
	Variable	*V1;
	Variable	*Var;

	Index = ReadByte();
	switch(Index)
	{
		case FUNC_FIND_ENTITY_WITH_TARGET:
			V1 = PopStack();
			Search = G_Find(NULL, FOFS(targetname), V1->GetStringValue());
			Var = new EntityVar(Search);

			delete V1;
			break;

		case FUNC_SIN:
			V1 = PopStack();
			Var = new FloatVar( "", sin( DEG2RAD(V1->GetFloatValue()) ));

			delete V1;
			break;

		case FUNC_COS:
			V1 = PopStack();
			Var = new FloatVar("", cos( DEG2RAD(V1->GetFloatValue()) ));

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
	Variable	*Var;

	Type = ReadByte();
	switch(Type)
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
			((VariableVar *)Var)->ReadValue(this);
			break;
		case PUSH_VAR_WITH_FIELD:
			Var = new FieldVariableVar();
			((VariableVar *)Var)->ReadValue(this);
			break;
		case PUSH_FUNCTION:
			Var = HandleBuiltinFunction();
			break;
	}

	PushStack(Var);
}

void CScript::HandlePop(void)
{
	Variable *V;

	V = PopStack();
	if (V)
	{
		delete V;
	}
}

void CScript::HandleAssignment(void)
{
	Variable *Value, *Assignee;

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
	Variable *V1, *V2;

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
	Variable *V1, *V2;

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
	Variable *V1, *V2;

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
	Variable *V1, *V2;

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
	List<Variable *>::Iter	iv;
	int						Flags;

	Flags = ReadByte();

	if (Flags)
	{
		if (Flags & DEBUG_ENABLE)
		{
			Flags &= ~DEBUG_ENABLE;
			DebugFlags |= Flags;
		}
		else
		{
			DebugFlags &= ~Flags;
		}
	}
	else
	{
		StartDebug();

		if (ParameterVariables.Size())
		{
			DebugLine("   Parameters:\n");
			for (iv=ParameterVariables.Begin();iv != ParameterVariables.End();iv++)
			{
				(*iv)->Debug(this);
			}
		}

		if (GlobalVariables.Size())
		{
			DebugLine("   Global Variables:\n");
			for (iv=GlobalVariables.Begin();iv != GlobalVariables.End();iv++)
			{
				(*iv)->Debug(this);
			}
		}

		if (LocalVariables.Size())
		{
			DebugLine("   Local Variables:\n");
			for (iv=LocalVariables.Begin();iv != LocalVariables.End();iv++)
			{
				(*iv)->Debug(this);
			}
		}
		EndDebug();
	}
}

void CScript::HandleDebugStatement(void)
{
	DebugLine("%s\n",ReadString());
}

void CScript::HandleAddAssignment(void)
{
	Variable *Value, *Assignee;

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
	Variable *Value, *Assignee;

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
	Variable *Value, *Assignee;

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
	Variable *Value, *Assignee;

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
	Variable	*VI;

	count = ReadByte();
	if (count & WAIT_CLEAR)
	{
		ConditionInfo = WAIT_CLEAR;
	}
	else
	{
		ConditionInfo = 0;
	}

	count &= ~WAIT_CLEAR;

	for(;count;count--)
	{
		VI = PopStack();
		if (!VI)
		{
			Error("Invalid stack for HandleWait");
		}

		Waiting.PushBack(VI);
	}

	if (ForAll)
	{
		ScriptCondition = COND_WAIT_ALL;
	}
	else
	{
		ScriptCondition = COND_WAIT_ANY;
	}

	if (CheckWait())
	{
		FinishWait(NULL,false);

		return false;
	}
	
	return true;
}

bool CScript::HandleTimeWait(void)
{
	Variable	*V;
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

	AddEvent(new WaitEvent(NextTime) );

	ScriptCondition = COND_WAIT_TIME;

	return true;
}

void CScript::HandleIf(void)
{
	int			Condition;
	int			Location;
	Variable	*V1, *V2;
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

	switch(Condition)
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
		Position = Location;
	}
}

void CScript::HandlePrint(void)
{
	int			Flags;
	Variable	*Text, *Entity, *Level;
	char		*TextValue;
	int			LevelValue;
	edict_t		*ent;
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
	Variable	*SoundName, *Entity, *Volume, *Attenuation, *Channel, *TimeDelay;
	char		*SoundValue;
	float		VolumeValue, AttenuationValue, TimeDelayValue;
	int			ChannelValue;
	edict_t		*ent;


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
		CinematicSound[CinematicSound_cnt].ent = ent;
		CinematicSound[CinematicSound_cnt].channel = ChannelValue;

		if (CinematicSound_cnt < MAX_CINESNDS-1 )
			++CinematicSound_cnt; 	
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
	int i,null_snd;

	FeatureType = ReadByte();

	switch(FeatureType)
	{
		case FEATURE_TRIGGER:
			HandleTrigger(Enable);
			break;

		case FEATURE_AMBIENT_SOUNDS:
			break;

		case FEATURE_CINEMATICS:
				if (Enable)
				{
					CinematicSound_cnt = 0;
					Cvar_Set("sv_cinematicfreeze","1");
					RemoveNonCinematicEntities();
				}
				else
				{
					if (sv_jumpcinematic->value == 2)	// Jump sent from client
					{
						Cvar_Set("sv_jumpcinematic","0");
						null_snd = gi.soundindex("misc/null.wav");
						gi.bcaption(PRINT_HIGH, 270);		// Send the ID for the text to all players
						for (i=0;i<CinematicSound_cnt;++i)
						{
							if (CinematicSound[i].ent)	// Does the entity still exist
							{
								gi.sound(CinematicSound[i].ent, CinematicSound[i].channel, 
									null_snd, 1, ATTN_NORM, 0);
									
							}
						}
					}


					Cvar_Set("sv_cinematicfreeze","0");
					ReinstateNonCinematicEntities();
				}
			break;

		case FEATURE_PLAGUE_SKINS:
			break;
	}
}

void CScript::HandleCacheSound(void)
{
	Variable	*SoundName;
	char		*SoundValue;

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
	Variable	*Signaler, *Rate, *Duration, *Amount, *Entity;
	edict_t		*ent;
	vec3_t		Vec,Dest,Diff;
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

			if (DebugFlags & DEBUG_MOVE)
			{
				StartDebug();
				DebugLine("   Moving Entity %d\n",Entity->GetIntValue());
				DebugLine("      From (%7.3f, %7.3f, %7.3f)\n",ent->s.origin[0],ent->s.origin[1],ent->s.origin[2]);
				DebugLine("      To   (%7.3f, %7.3f, %7.3f)\n",Dest[0], Dest[1], Dest[2]);
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
	Variable	*Signaler, *Rate, *Duration, *Amount, *Entity;
	edict_t		*ent;
	vec3_t		Vec,Dest,Diff;
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

			if (DebugFlags & DEBUG_ROTATE)
			{
				StartDebug();
				DebugLine("   Rotating Entity %d\n",Entity->GetIntValue());
				DebugLine("      From (%7.3f, %7.3f, %7.3f)\n",ent->s.angles[0],ent->s.angles[1],ent->s.angles[2]);
				DebugLine("      To   (%7.3f, %7.3f, %7.3f)\n",ent->moveinfo.end_angles[0], ent->moveinfo.end_angles[1], ent->moveinfo.end_angles[2]);
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
	Variable	*Entity;
	edict_t		*use_ent;

	Entity = PopStack();

	use_ent = Entity->GetEdictValue();
	if (use_ent && use_ent->use)
	{
		use_ent->use(use_ent,other,activator);
	}

	delete Entity;
}

void CScript::HandleTrigger(bool Enable)
{
	Variable	*Entity;
	edict_t		*trigger_ent;

	Entity = PopStack();

	trigger_ent = Entity->GetEdictValue();
	if (trigger_ent)
	{
		if (Enable)
		{
			trigger_ent->solid = SOLID_TRIGGER;
			trigger_ent->use = TriggerMultipleUse;
			gi.linkentity (trigger_ent);
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
	Variable	*Signaler, *Moving, *Turning, *Repeat, *Action, *Entity, *Source;
	edict_t		*ent, *SourceEnt;
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

		QPostMessage(ent,(enum G_MsgID_e) msg_animtype[ActionVal], PRI_DIRECTIVE, "iiige",(int)MovingVal[0],(int)TurningVal[0],(int)RepeatVal,SignalerRoutine,activator);
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
	Variable	*Player, *Destination;
	edict_t		*PlayerEnt, *DestinationEnt;

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

	CinematicSwapPlayer (PlayerEnt,DestinationEnt);
}

void CScript::HandleSetViewAngles(void)
{
	Variable	*Player, *Angles;
	edict_t		*PlayerEnt;
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

	PlayerEnt->client->ps.pmove.delta_angles[PITCH]=0;
	PlayerEnt->client->ps.pmove.delta_angles[YAW]=ANGLE2SHORT(HoldAngles[YAW]-PlayerEnt->client->resp.cmd_angles[YAW]);
	PlayerEnt->client->ps.pmove.delta_angles[ROLL]=0;

	PlayerEnt->s.angles[PITCH]=0;
	PlayerEnt->s.angles[YAW]=HoldAngles[YAW];
	PlayerEnt->s.angles[ROLL]=0;
}

void CScript::HandleSetCacheSize(void)
{
	Variable	*CacheSize;

	CacheSize = PopStack();
	if (!CacheSize)
	{
		Error("Invalid stack for HandleSetCacheSize()");
	}
}

void CScript::Move_Done(edict_t *ent)
{
	VectorClear (ent->velocity);

	VectorCopy(ent->moveinfo.end_origin, ent->s.origin);
}

void CScript::Move(edict_t *ent, vec3_t Dest)
{
	float	frames;

	VectorCopy(Dest, ent->moveinfo.end_origin);

	VectorSubtract (ent->moveinfo.end_origin, ent->s.origin, ent->moveinfo.dir);
	ent->moveinfo.remaining_distance = VectorNormalize (ent->moveinfo.dir);
	if (ent->moveinfo.remaining_distance <= 0)
	{
		frames = 0;
	}
	else
	{
		frames = floor((ent->moveinfo.remaining_distance / ent->moveinfo.speed) / FRAMETIME) + 1;
	}

	VectorScale (ent->moveinfo.dir, ent->moveinfo.remaining_distance/frames/FRAMETIME, ent->velocity);

	AddEvent(new MoveDoneEvent(level.time + (frames * FRAMETIME), ent));
}

void CScript::Rotate_Done (edict_t *ent)
{
	VectorClear (ent->avelocity);
}

void CScript::Rotate(edict_t *ent)
{
	float	distance;
	vec3_t	destdelta;
	float	frames;

	VectorSubtract (ent->moveinfo.start_angles, ent->s.angles, destdelta);

	distance = VectorNormalize (destdelta);
	if (ent->moveinfo.speed <= 0)
	{
		frames = 0;
		VectorClear (ent->avelocity);
	}
	else
	{
		frames = floor((distance / ent->moveinfo.speed) / FRAMETIME) + 1;
		VectorScale (destdelta, distance/frames/FRAMETIME, ent->avelocity);
	}

	AddEvent(new RotateDoneEvent(level.time + (frames * FRAMETIME), ent));
}

void CScript::AddEvent(Event *Which)
{
	List<Event *>::Iter	ie;
	float				time;

	if (Events.Size())
	{
		time = Which->GetTime();
		for (ie=Events.Begin();ie != Events.End();ie++)
		{
			if ( (*ie)->GetTime() > time)
			{
				break;
			}
		}
		Events.Insert(ie, Which);
	}
	else
	{
		Events.PushBack(Which);
	}

#ifdef _DEBUG
	float				testtime;

	time = 0;
	for (ie=Events.Begin();ie != Events.End();ie++)
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
	List<Event *>::Iter	ie, next;

	while(Events.Size())
	{
		ie = Events.Begin();

		if ((*ie)->Process(this))
		{
			delete (*ie);
			Events.Erase(ie);
		}
		else
		{
			break;
		}
	}
}

void CScript::ClearTimeWait(void)
{
	if (ScriptCondition == COND_WAIT_TIME)
	{
		ScriptCondition = COND_READY;
	}
}

void CScript::AddSignaler(edict_t *Edict, Variable *Var, SignalT SignalType)
{
	List<Signaler *>::Iter	is;
	Signaler *NewSig;

	NewSig = new Signaler(Edict, Var, SignalType);

	// Note that this check does not need to be in there - signalers are very flexible, but if used
	// incorrectly, they can result in weird behavior - this check prevents more than one command using
	// the same signal varaible prior to a wait command
	for (is=Signalers.Begin();is != Signalers.End();is++)
	{
		if (*(*is) == NewSig)
		{
			Error("Renner Error #1: Variable '%s' is being used for multiple signals", Var->GetName() );
		}
	}

	Signalers.PushBack(NewSig);
}

void CScript::CheckSignalers(edict_t *Which, SignalT SignalType)
{
	List<Signaler *>::Iter	is, next;
	bool					DoCheckWait = false;

	if (Signalers.Size())
	{
		for (is=Signalers.Begin();is != Signalers.End();is = next)
		{
			next = is;
			next++;
			if ((*is)->Test(Which, SignalType))
			{
				delete (*is);
				Signalers.Erase(is);

				DoCheckWait = true;
			}
		}
	}

	if (DoCheckWait && (ScriptCondition == COND_WAIT_ANY || ScriptCondition == COND_WAIT_ALL))
	{
		if (CheckWait())
		{
			FinishWait(Which, true);
		}
	}
}

bool CScript::CheckWait(void)
{
	List<Variable *>::Iter	iv;
	int						count, needed;

	if (ScriptCondition == COND_WAIT_ALL)
	{
		needed = Waiting.Size();
	}
	else if (ScriptCondition == COND_WAIT_ANY)
	{
		needed = 1;
	}
	else if (ScriptCondition == COND_WAIT_TIME)
	{
		return false;
	}
	else if (ScriptCondition == COND_READY)
	{
		return true;
	}
	else
	{
		return false;
	}

	count = 0;
	if (Waiting.Size())
	{
		for (iv=Waiting.Begin();iv != Waiting.End();iv++)
		{
			if ( (*iv)->GetIntValue())
			{
				count++;
			}
		}
	}

	if (count == needed)
	{
		ScriptCondition = COND_READY;

		return true;
	}

	return false;
}

void CScript::FinishWait(edict_t *Which, bool NoExecute)
{
	List<Variable *>::Iter	iv;

	if (Waiting.Size())
	{
		for (iv=Waiting.Begin();iv != Waiting.End();iv++)
		{
			if (ConditionInfo == WAIT_CLEAR)
			{
				(*iv)->ClearSignal();
			}

			delete *iv;
		}
	}
	Waiting.Erase(Waiting.Begin(), Waiting.End() );

	if (NoExecute)
	{
		Execute(Which, NULL);
	}
}

void CScript::Error (char *error, ...)
{
	va_list argptr;
	char	text[1024];
	
	va_start (argptr, error);
	vsprintf (text, error, argptr);
	va_end (argptr);

	gi.error(text);
}

void CScript::StartDebug(void)
{
	DebugLine("-------------------------------\n");
	DebugLine("Script: %s\n",Name);
	DebugLine("   DEBUG at %d\n",Position);
}

void CScript::EndDebug(void)
{
	DebugLine("-------------------------------\n");
}

void CScript::DebugLine (char *debugtext, ...)
{
	va_list argptr;
	char	text[1024];
	
	va_start (argptr, debugtext);
	vsprintf (text, debugtext, argptr);
	va_end (argptr);

	Com_Printf("%s",text);

#ifdef _DEBUG
	OutputDebugString(text);
#endif
}

void CScript::Think(void)
{
	ProcessEvents();
}

ScriptConditionT CScript::Execute(edict_t *new_other, edict_t *new_activator)
{
	bool				Done;
	int					InstructionCount;

	if (ScriptCondition != COND_READY)
	{
		return ScriptCondition;
	}

	if (DebugFlags & DEBUG_TIME)
	{
		StartDebug();
		DebugLine("   Current Time: %10.1f\n",level.time);
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
		if (InstructionCount > INSTRUCTION_MAX)
		{
			Error("Runaway loop for script");
		}

		switch(ReadByte())
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
				ScriptCondition = COND_COMPLETED;
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

		if (Position >= Length)
		{
			Done = true;
			ScriptCondition = COND_COMPLETED;
		}
	}

	return ScriptCondition;
}

Variable *CScript::FindLocal(char *Name)
{
	List<Variable *>::Iter	iv;

	if (LocalVariables.Size())
	{
		for (iv=LocalVariables.Begin();iv != LocalVariables.End();iv++)
		{
			if (strcmp(Name, (*iv)->GetName()) == 0)
			{
				return *iv;
			}
		}
	}

	return NULL;
}

bool CScript::NewLocal(Variable *Which)
{
	Variable *Check;

	Check = FindLocal(Which->GetName());
	if (Check)
	{	// already exists
		return false;
	}

	LocalVariables.PushBack(Which);

	return true;
}

Variable *CScript::FindParameter(char *Name)
{
	List<Variable *>::Iter	iv;

	if (ParameterVariables.Size())
	{
		for (iv=ParameterVariables.Begin();iv != ParameterVariables.End();iv++)
		{
			if (strcmp(Name, (*iv)->GetName()) == 0)
			{
				return *iv;
			}
		}
	}

	return NULL;
}

bool CScript::NewParameter(Variable *Which)
{
	Variable	*Check;
	StringVar	*ParmValue;
	edict_t		*Search;
	Variable	*temp;
	vec3_t		vec;

	Check = FindParameter(Which->GetName());
	if (Check)
	{	// already exists
		return false;
	}

	ParameterVariables.PushBack(Which);

	if (!ParameterValues.Size())
	{
		Error("Missing Parameter");
	}

	ParmValue = *ParameterValues.Begin();
	ParameterValues.Erase(ParameterValues.Begin());

	switch(Which->GetType())
	{
		case TYPE_ENTITY:
			Search = G_Find(NULL, FOFS(targetname), ParmValue->GetStringValue());
			temp = new EntityVar(Search);
			break;

		case TYPE_INT:
			temp = new IntVar("parm",atol(ParmValue->GetStringValue()));
			break;

		case TYPE_FLOAT:
			temp = new FloatVar("parm",atof(ParmValue->GetStringValue()));
			break;

		case TYPE_VECTOR:
			sscanf (ParmValue->GetStringValue(), "%f %f %f", &vec[0], &vec[1], &vec[2]);
			temp = new VectorVar("parm",vec[0],vec[1],vec[2]);
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