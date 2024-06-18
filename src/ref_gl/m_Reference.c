//
// m_Reference.c
//
// Copyright 1998 Raven Software
//

#include "Reference.h"
#include "gl_local.h"
#include "fmodel.h"

int GetReferencedID(const struct model_s* model)
{
	const fmdl_t* temp = model->extradata;

	//mxd. H2 Toolkit code checks for model->model_type, decompiled code checks for model->skeletal_model...
	if (model->skeletal_model && temp->referenceType > REF_NULL && temp->referenceType < NUM_REFERENCED)
		return temp->referenceType;

	return REF_NULL;
}