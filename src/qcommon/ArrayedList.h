//
// ArrayedList.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "q_Typedef.h"

typedef struct ArrayedListNode_s
{
	int data;
	int next;
	qboolean in_use; //mxd. 'int inUse' in original logic.
} ArrayedListNode_t;

#define ARRAYEDLISTNODE_NULL (-1)

//TODO: un-inline?
_inline int GetFreeNode(ArrayedListNode_t* array, const int max)
{
	for (int i = 0; i < max; i++)
	{
		if (!array[i].in_use)
		{
			array[i].in_use = true;
			return i;
		}
	}

	return -1;
}

//TODO: un-inline?
_inline void FreeNode(ArrayedListNode_t* array, const int index)
{
	array[index].in_use = false;
}