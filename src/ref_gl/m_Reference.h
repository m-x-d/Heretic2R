//
// m_Reference.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "Reference.h"

typedef struct M_Reference_s
{
	Placement_t placement;
} M_Reference_t; 

int GetReferencedID(const struct model_s* model);